/* wave_generator.c
 *
 * Copyright 2026 Alan Crispin <crispinalan@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
/**
 * @file wave_generator.c
 * @brief Audio waveform generation engine for TOM speech synthesizer.
 */
#include "wave_generator.h"
#include "lookup_tables.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>  // Added for rand() function

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// --- CONFIGURABLE PARAMETERS ---
static float FREQUENCY_INTERP_FACTOR = 0.1f; 
static float AMPLITUDE_INTERP_FACTOR = 0.05f; 
static float MAX_AMPLITUDE_CLAMP = 0.35f; 
static float SMOOTHING_WEIGHT_STEP = 1.0f/64.0f; 

// Plosive burst characteristics
static float PLOSIVE_DECAY_FACTOR = 35.0f; 
static float PLOSIVE_AMPLITUDE_MULTIPLIER = 24.0f; 

// Fricative characteristics 
static float FRICATIVE_DECAY_RATE = 12.0f; 
static float FRICATIVE_NOISE_AMPLITUDE = 1.0f; 

// Pre-emphasis filter parameters
static float PREEMPHASIS_ALPHA = 0.85f; 

// Additional clipping prevention parameters
static float CLIPPING_PROTECTION_FACTOR = 0.9f; 
static float VOICE_AMPLITUDE_BOOST = 1.3f; 

/**
 * @struct TOMEngineState
 * @brief Internal state structure for the TOM audio engine.
 */
typedef struct {
    float ph0; 
    float ph1; float ph2; float ph3; float ph4; float ph5; float ph6;
    uint16_t noise_lfsr; 
    float fricative_phase; 
    int is_first_frame; 
    float last_unvoiced_amplitude; 
    float env_peak; 
    // For better transition handling
    float prev_voiced_signal;
    float prev_unvoiced_signal;
    int last_phoneme_type;
} TOMEngineState; // --- CORE ENGINE STATE ---

/**
 * @struct InterpolationState
 * @brief State structure for interpolation between frames.
 */
typedef struct {
    float prev_f1_freq; float prev_f2_freq; float prev_f3_freq;
    float prev_f4_freq; float prev_f5_freq; float prev_f6_freq;
    float prev_f1_amp; float prev_f2_amp; float prev_f3_amp;
    float prev_f4_amp; float prev_f5_amp; float prev_f6_amp;
    int frame_valid; 
    float prev_mix_weight; 
    int was_silence; 
    // Transition state
    int last_frame_type;
} InterpolationState; // --- INTERPOLATION STATE ---

/**
 * @var state
 * @brief Global TOM engine state.
 */
static TOMEngineState state;
/**
 * @var interp_state
 * @brief Global interpolation state for smooth transitions between frames.
 */
static InterpolationState interp_state = {0};

/**
 * @fn init_wave_generator(void)
 * @brief Initializes the wave generator engine and resets all internal states.
 * 
 * Sets up default values for all audio generation parameters including phase,
 * noise generators, and amplitude tracking variables.
 */
void init_wave_generator(void) {
    state.ph0 = 0.0f; 
    state.ph1 = 0.0f; state.ph2 = 0.0f; state.ph3 = 0.0f;
    state.ph4 = 0.0f; state.ph5 = 0.0f; state.ph6 = 0.0f;
     
    state.noise_lfsr = 0xACE1; 
    state.fricative_phase = 0.0f;
    state.is_first_frame = 1;
    state.last_unvoiced_amplitude = 0.0f;
    state.env_peak = 0.0f;
    state.prev_voiced_signal = 0.0f;
    state.prev_unvoiced_signal = 0.0f;
    state.last_phoneme_type = PH_SILENCE;
    
    memset(&interp_state, 0, sizeof(InterpolationState));
}

/**
 * @fn generate_lfsr_noise(void)
 * @brief Generates pseudo-random noise using Linear Feedback Shift Register (LFSR).
 * 
 * Creates a stream of pseudo-random numbers suitable for generating white noise
 * in unvoiced phoneme synthesis.
 * 
 * @return 8-bit signed integer representing the generated noise value (-128 to +127)
 */
static int8_t generate_lfsr_noise(void) {
    uint16_t lsb = state.noise_lfsr & 1;
    state.noise_lfsr >>= 1;
    if (lsb) state.noise_lfsr ^= 0xB400;
    return (int8_t)(state.noise_lfsr >> 8);
}

/**
 * @fn smooth_frequency_interpolation(SpeechFrame *current_frame, SpeechFrame *prev_frame)
 * @brief Applies frequency smoothing between consecutive speech frames.
 * 
 * Smooths transitions in formant frequencies to prevent abrupt changes that could
 * cause audible artifacts during speech synthesis.
 * 
 * @param current_frame Current frame being processed
 * @param prev_frame Previous frame for interpolation reference
 */
static void smooth_frequency_interpolation(SpeechFrame *current_frame, SpeechFrame *prev_frame) 
{
    if (current_frame->type == PH_VOICED && prev_frame->type != PH_SILENCE) {
        current_frame->f1.frequency = (uint16_t)((float)current_frame->f1.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + (float)prev_frame->f1.frequency * FREQUENCY_INTERP_FACTOR);
        current_frame->f2.frequency = (uint16_t)((float)current_frame->f2.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + (float)prev_frame->f2.frequency * FREQUENCY_INTERP_FACTOR);
        current_frame->f3.frequency = (uint16_t)((float)current_frame->f3.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + (float)prev_frame->f3.frequency * FREQUENCY_INTERP_FACTOR);
        current_frame->f4.frequency = (uint16_t)((float)current_frame->f4.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + (float)prev_frame->f4.frequency * FREQUENCY_INTERP_FACTOR);
        current_frame->f5.frequency = (uint16_t)((float)current_frame->f5.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + (float)prev_frame->f5.frequency * FREQUENCY_INTERP_FACTOR);
        current_frame->f6.frequency = (uint16_t)((float)current_frame->f6.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + (float)prev_frame->f6.frequency * FREQUENCY_INTERP_FACTOR);
    }
}

/**
 * @fn apply_preemphasis_filter(float input_sample, float* prev_output)
 * @brief Applies pre-emphasis filtering to enhance high-frequency content.
 * 
 * Implements a simple first-order high-pass filter to boost higher frequencies
 * in the audio signal for better clarity and intelligibility.
 * 
 * @param input_sample Input sample value
 * @param prev_output Pointer to previous output value (for feedback)
 * @return Filtered output value
 */
static float apply_preemphasis_filter(float input_sample, float* prev_output) {
    float output = input_sample - PREEMPHASIS_ALPHA * (*prev_output);
    *prev_output = input_sample;
    return output;
}

/**
 * @fn apply_master_normalization(float raw_sample)
 * @brief Applies dynamic range compression and normalization.
 * 
 * Normalizes audio amplitude to prevent clipping while maintaining consistent volume levels.
 * 
 * @param raw_sample Input sample value
 * @return Normalized output sample value
 */
static float apply_master_normalization(float raw_sample) {
    float abs_val = fabsf(raw_sample);
    if (abs_val > state.env_peak) {
        state.env_peak = abs_val;
    } else {
        state.env_peak += (abs_val - state.env_peak) * 0.0005f; 
    }
    float dynamic_gain = 1.0f;
    if (state.env_peak > 90.0f) {
        dynamic_gain = 90.0f / state.env_peak;
    }
    return raw_sample * dynamic_gain;
}

/**
 * @fn generate_speech_wave(char *output_buffer, SpeechFrame *frames, int frame_count, float volume_factor)
 * @brief Generates full waveform audio from scheduled speech frames.
 * 
 * Processes an array of speech frames to produce a complete audio buffer in 8-bit PCM format.
 * Handles all aspects of synthesis including formant generation, mixing voiced and unvoiced components,
 * noise processing for plosives and fricatives, and final output formatting.
 * 
 * @param output_buffer Buffer where generated waveform data will be stored
 * @param frames Array of speech frames to process
 * @param frame_count Number of frames in the input array
 * @param volume_factor Multiplier to adjust overall audio amplitude
 * @return Number of bytes written to output buffer
 */
int generate_speech_wave(char *output_buffer, SpeechFrame *frames, int frame_count, float volume_factor) {
    int buffer_ptr = 0;
    float sample_rate = 22050.0f; 
    
    state.ph0 = 0.0f; 
    state.ph1 = 0.0f; state.ph2 = 0.0f; state.ph3 = 0.0f;
    state.ph4 = 0.0f; state.ph5 = 0.0f; state.ph6 = 0.0f;
    state.fricative_phase = 0.0f;
    state.is_first_frame = 1;
    state.last_unvoiced_amplitude = 0.0f;
    state.env_peak = 0.0f;
    
    printf("DEBUG: Generating %d frames\n", frame_count); 
    float prev_preemphasis_output = 0.0f; 
    
    for (int f = 0; f < frame_count; f++) {
        SpeechFrame *current_frame = &frames[f]; 
        
        if (f == 0) {
            interp_state.prev_f1_freq = (float)current_frame->f1.frequency;
            interp_state.prev_f2_freq = (float)current_frame->f2.frequency;
            interp_state.prev_f3_freq = (float)current_frame->f3.frequency;
            interp_state.prev_f4_freq = (float)current_frame->f4.frequency;
            interp_state.prev_f5_freq = (float)current_frame->f5.frequency;
            interp_state.prev_f6_freq = (float)current_frame->f6.frequency;
            interp_state.prev_f1_amp = (float)current_frame->f1.amplitude;
            interp_state.prev_f2_amp = (float)current_frame->f2.amplitude;
            interp_state.prev_f3_amp = (float)current_frame->f3.amplitude;
            interp_state.prev_f4_amp = (float)current_frame->f4.amplitude;
            interp_state.prev_f5_amp = (float)current_frame->f5.amplitude;
            interp_state.prev_f6_amp = (float)current_frame->f6.amplitude;
            interp_state.prev_mix_weight = 0.0f; 
            interp_state.frame_valid = 1;
            interp_state.was_silence = (current_frame->type == PH_SILENCE);
            interp_state.last_frame_type = current_frame->type;
        }
        
        if (f > 0 && f < frame_count - 1) {
            smooth_frequency_interpolation(current_frame, &frames[f-1]);
            
            // IMPROVEMENT 1: Instant Vowel Attacks out of Silence Gaps
            // Prevents the "funnel-shaped" slow amplitude rise after a pause
            if (frames[f-1].type == PH_SILENCE && current_frame->type == PH_VOICED) {		 
                // FIXED: Enforce master clipping safeguards directly on the instant attack values
                if (current_frame->f1.amplitude > 255) current_frame->f1.amplitude = 255;
                if (current_frame->f2.amplitude > 255) current_frame->f2.amplitude = 255;
                if (current_frame->f3.amplitude > 255) current_frame->f3.amplitude = 255;
                if (current_frame->f4.amplitude > 255) current_frame->f4.amplitude = 255;
                if (current_frame->f5.amplitude > 255) current_frame->f5.amplitude = 255;
                if (current_frame->f6.amplitude > 255) current_frame->f6.amplitude = 255;	 
                 
            } else {
                float prev_f1_amp = frames[f-1].f1.amplitude;
                float prev_f2_amp = frames[f-1].f2.amplitude;
                float prev_f3_amp = frames[f-1].f3.amplitude;
                float prev_f4_amp = frames[f-1].f4.amplitude;
                float prev_f5_amp = frames[f-1].f5.amplitude;
                float prev_f6_amp = frames[f-1].f6.amplitude;
                
                current_frame->f1.amplitude = (uint16_t)((float)current_frame->f1.amplitude * (1.0f - AMPLITUDE_INTERP_FACTOR) + prev_f1_amp * AMPLITUDE_INTERP_FACTOR);
                current_frame->f2.amplitude = (uint16_t)((float)current_frame->f2.amplitude * (1.0f - AMPLITUDE_INTERP_FACTOR) + prev_f2_amp * AMPLITUDE_INTERP_FACTOR);
                current_frame->f3.amplitude = (uint16_t)((float)current_frame->f3.amplitude * (1.0f - AMPLITUDE_INTERP_FACTOR) + prev_f3_amp * AMPLITUDE_INTERP_FACTOR);
                current_frame->f4.amplitude = (uint16_t)((float)current_frame->f4.amplitude * (1.0f - AMPLITUDE_INTERP_FACTOR) + prev_f4_amp * AMPLITUDE_INTERP_FACTOR);
                current_frame->f5.amplitude = (uint16_t)((float)current_frame->f5.amplitude * (1.0f - AMPLITUDE_INTERP_FACTOR) + prev_f5_amp * AMPLITUDE_INTERP_FACTOR);
                current_frame->f6.amplitude = (uint16_t)((float)current_frame->f6.amplitude * (1.0f - AMPLITUDE_INTERP_FACTOR) + prev_f6_amp * AMPLITUDE_INTERP_FACTOR);
            }
            
            if (current_frame->f1.amplitude > 255) current_frame->f1.amplitude = 255;
            if (current_frame->f2.amplitude > 255) current_frame->f2.amplitude = 255;
            if (current_frame->f3.amplitude > 255) current_frame->f3.amplitude = 255;
        }
        
        float stored_f1_freq = current_frame->f1.frequency;
        float stored_f2_freq = current_frame->f2.frequency;
        float stored_f3_freq = current_frame->f3.frequency;
        float stored_f4_freq = current_frame->f4.frequency;
        float stored_f5_freq = current_frame->f5.frequency;
        float stored_f6_freq = current_frame->f6.frequency;
        float stored_f1_amp = (float)current_frame->f1.amplitude;
        float stored_f2_amp = (float)current_frame->f2.amplitude;
        float stored_f3_amp = (float)current_frame->f3.amplitude;
        float stored_f4_amp = (float)current_frame->f4.amplitude;
        float stored_f5_amp = (float)current_frame->f5.amplitude;
        float stored_f6_amp = (float)current_frame->f6.amplitude;
        
        if (f > 0 && !state.is_first_frame) {
            current_frame->f1.frequency = (int)((float)current_frame->f1.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + interp_state.prev_f1_freq * FREQUENCY_INTERP_FACTOR);
            current_frame->f2.frequency = (int)((float)current_frame->f2.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + interp_state.prev_f2_freq * FREQUENCY_INTERP_FACTOR);
            current_frame->f3.frequency = (int)((float)current_frame->f3.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + interp_state.prev_f3_freq * FREQUENCY_INTERP_FACTOR);
            current_frame->f4.frequency = (int)((float)current_frame->f4.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + interp_state.prev_f4_freq * FREQUENCY_INTERP_FACTOR);
            current_frame->f5.frequency = (int)((float)current_frame->f5.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + interp_state.prev_f5_freq * FREQUENCY_INTERP_FACTOR);
            current_frame->f6.frequency = (int)((float)current_frame->f6.frequency * (1.0f - FREQUENCY_INTERP_FACTOR) + interp_state.prev_f6_freq * FREQUENCY_INTERP_FACTOR);
        }
        
        float intonation_factor = 1.0f - ((float)f / (float)frame_count) * 0.12f;
        float adjusted_pitch = (float)current_frame->pitch * intonation_factor;
        
        int samples_to_render = 256; 
        if (adjusted_pitch > 0) {
            samples_to_render = (int)(sample_rate / adjusted_pitch);
            if (samples_to_render < 8) samples_to_render = 8;
            if (samples_to_render > 1024) samples_to_render = 1024;
        }
        if (samples_to_render <= 2) samples_to_render = 2; 
        
        // Determine mix weight for voiced/unvoiced signal combination
        float target_mix_weight = 1.0f; 
        if (current_frame->pitch == 0 || (current_frame->f1.amplitude == 0 && current_frame->f2.amplitude == 0)) {
            target_mix_weight = 0.0f; 
        } else if (current_frame->f1.amplitude < 10 || current_frame->f1.frequency == 300) {
            target_mix_weight = 0.0f; 
        }
        
        // NEW: Detect if the current frame belongs to an aspirated phoneme like HH
        int is_aspirated = (current_frame->type == PH_VOICED && current_frame->f1.frequency == 500 && current_frame->f2.frequency == 1500 && current_frame->f4.amplitude == 8);
        
        // Force mix weight lower for aspirated sounds to allow noise blending
        if (is_aspirated) {
            target_mix_weight = 0.35f; 
        }

        int is_unvoiced = (target_mix_weight == 0.0f);
        int is_plosive = (current_frame->f1.frequency <= 500 && current_frame->f2.frequency <= 1500 && 
                        current_frame->f1.frequency > 410 && current_frame->f4.amplitude == 8 && is_unvoiced);
        
        // Handle transition from voiced to unvoiced (especially stops to fricatives)
        int has_transition = 0;
        if (f > 0) {
            SpeechFrame *prev_frame = &frames[f-1];
            if ((prev_frame->type == PH_VOICED && current_frame->type == PH_NOISE) || 
                (prev_frame->type == PH_NOISE && current_frame->type == PH_VOICED)) {
                // This is a transition - we want to smooth it
                has_transition = 1;
            }
        }
        
        if (f > 0 && frames[f-1].type == PH_SILENCE && current_frame->type != PH_SILENCE) {
            state.fricative_phase = 0.0f;
        }
        
        if (f > 0 && frames[f-1].type != PH_VOICED && current_frame->type == PH_VOICED) {
            state.ph0 = 0.0f; state.ph1 = 0.0f; state.ph2 = 0.0f;
            state.ph3 = 0.0f; state.ph4 = 0.0f; state.ph5 = 0.0f; state.ph6 = 0.0f;
        }
        
        if (state.is_first_frame && f > 0) {
            if (frames[f-1].type == PH_SILENCE && current_frame->type != PH_SILENCE) {
                interp_state.prev_mix_weight = 0.0f;
            }
        }
        
        for (int s = 0; s < samples_to_render; s++) {
            // Smooth the mix weight transition
            if (interp_state.prev_mix_weight < target_mix_weight) {
                interp_state.prev_mix_weight += SMOOTHING_WEIGHT_STEP;
                if (interp_state.prev_mix_weight > target_mix_weight) interp_state.prev_mix_weight = target_mix_weight;
            } else if (interp_state.prev_mix_weight > target_mix_weight) {
                interp_state.prev_mix_weight -= SMOOTHING_WEIGHT_STEP;
                if (interp_state.prev_mix_weight < target_mix_weight) interp_state.prev_mix_weight = target_mix_weight;
            }
            
            float d0 = (adjusted_pitch > 0.0f) ? (adjusted_pitch * 256.0f) / sample_rate : (84.0f * 256.0f) / sample_rate;
            state.ph0 += d0;
            
            if (state.ph0 >= 256.0f) {
                state.ph0 -= 256.0f;
                state.ph1 = 0.0f; state.ph2 = 0.0f; state.ph3 = 0.0f;
            }
            
            float d1 = (2.0f * (float)M_PI * (float)current_frame->f1.frequency) / sample_rate;
            float d2 = (2.0f * (float)M_PI * (float)current_frame->f2.frequency) / sample_rate;
            float d3 = (2.0f * (float)M_PI * (float)current_frame->f3.frequency) / sample_rate;
            float d4 = (2.0f * (float)M_PI * (float)current_frame->f4.frequency) / sample_rate;
            float d5 = (2.0f * (float)M_PI * (float)current_frame->f5.frequency) / sample_rate;
            float d6 = (2.0f * (float)M_PI * (float)current_frame->f6.frequency) / sample_rate;
            
            state.ph1 += d1; state.ph2 += d2; state.ph3 += d3;
            state.ph4 += d4; state.ph5 += d5; state.ph6 += d6;
            
            while (state.ph1 >= (2.0f * (float)M_PI)) state.ph1 -= (2.0f * (float)M_PI);
            while (state.ph2 >= (2.0f * (float)M_PI)) state.ph2 -= (2.0f * (float)M_PI);
            while (state.ph3 >= (2.0f * (float)M_PI)) state.ph3 -= (2.0f * (float)M_PI);
            while (state.ph4 >= (2.0f * (float)M_PI)) state.ph4 -= (2.0f * (float)M_PI);
            while (state.ph5 >= (2.0f * (float)M_PI)) state.ph5 -= (2.0f * (float)M_PI);
            while (state.ph6 >= (2.0f * (float)M_PI)) state.ph6 -= (2.0f * (float)M_PI);
            
            float max_amp_sum = (float)(current_frame->f1.amplitude + current_frame->f2.amplitude + current_frame->f3.amplitude + current_frame->f4.amplitude + current_frame->f5.amplitude + current_frame->f6.amplitude);
            if (max_amp_sum <= 0.0f) max_amp_sum = 1.0f;
            
            float sine1 = sinf(state.ph1) * 127.0f;
            float sine2 = sinf(state.ph2) * 127.0f;
            float sine3 = sinf(state.ph3) * 127.0f;
            float sine4 = sinf(state.ph4) * 127.0f;
            float sine5 = sinf(state.ph5) * 127.0f;
            float rect6 = (state.ph6 >= (float)M_PI) ? 32.0f : -32.0f;
            
            float v1 = sine1 * (float)current_frame->f1.amplitude;
            float v2 = sine2 * (float)current_frame->f2.amplitude;
            float v3 = sine3 * (float)current_frame->f3.amplitude;
            float v4 = sine4 * (float)current_frame->f4.amplitude;
            float v5 = sine5 * (float)current_frame->f5.amplitude;
            float v6 = rect6 * (float)current_frame->f6.amplitude;
            
            if (current_frame->f1.bandwidth > 0) v1 *= expf(-((float)current_frame->f1.frequency / (float)current_frame->f1.bandwidth));
            if (current_frame->f2.bandwidth > 0) v2 *= expf(-((float)current_frame->f2.frequency / (float)current_frame->f2.bandwidth));
            if (current_frame->f3.bandwidth > 0) v3 *= expf(-((float)current_frame->f3.frequency / (float)current_frame->f3.bandwidth));
            if (current_frame->f4.bandwidth > 0) v4 *= expf(-((float)current_frame->f4.frequency / (float)current_frame->f4.bandwidth));
            if (current_frame->f5.bandwidth > 0) v5 *= expf(-((float)current_frame->f5.frequency / (float)current_frame->f5.bandwidth));
            if (current_frame->f6.bandwidth > 0) v6 *= expf(-((float)current_frame->f6.frequency / (float)current_frame->f6.bandwidth));
            
            float voiced_signal = (v1 + v2 + v3 + v4 + v5 + v6) / max_amp_sum;
            voiced_signal *= 0.85f * expf(-(state.ph0 / 256.0f) * 4.5f);
            
            int8_t noise = generate_lfsr_noise();
            float normalized_amplitude = (float)current_frame->f2.amplitude / 15.0f;
            float unvoiced_signal = 0.0f;
            
            if (is_plosive) {
                float plosive_burst = expf(-((float)s / (float)samples_to_render) * PLOSIVE_DECAY_FACTOR);
                unvoiced_signal = (float)noise * normalized_amplitude * PLOSIVE_AMPLITUDE_MULTIPLIER * plosive_burst;
            } else if (current_frame->type == PH_NOISE || is_aspirated) {
                state.fricative_phase += 1.0f / sample_rate;
                if (state.fricative_phase > 1.0f) state.fricative_phase = 0.0f;
                
                float fricative_decay = expf(-state.fricative_phase * FRICATIVE_DECAY_RATE);
                
                // NEW: Soften noise scaling factor exclusively when handling HH frames
                float noise_scale = is_aspirated ? 0.25f : FRICATIVE_NOISE_AMPLITUDE;
                unvoiced_signal = (float)noise * normalized_amplitude * noise_scale * fricative_decay;
                
                if (s < 12 && f > 0 && frames[f-1].type == PH_SILENCE) {
                    unvoiced_signal *= ((float)s / 12.0f);
                }
            } else {
                // For voiced fricatives, we still want some noise to make them sound natural
                if (current_frame->type == PH_VOICED && current_frame->pitch > 0) {
                    // Add a subtle noise component for voiced fricatives like /v/ and /z/
                    unvoiced_signal = (float)noise * normalized_amplitude * 0.1f;
                } else {
                    unvoiced_signal = 0.0f;
                }
            }

            float mixed_sample = 0.0f;
            
            // Enhanced transition handling for better fricative/stop transitions
            if (current_frame->pitch == 0 || (current_frame->f1.amplitude == 0 && current_frame->f2.amplitude == 0)) {
                mixed_sample = 0.0f;
            } else {
                mixed_sample = (voiced_signal * interp_state.prev_mix_weight) + (unvoiced_signal * (1.0f - interp_state.prev_mix_weight));
            }
            
            // Apply transition smoothing for voiced/unvoiced transitions
            if (has_transition && s < samples_to_render/4) {  // Smooth first quarter of transition
                float transition_factor = ((float)s / (samples_to_render/4.0f));
                mixed_sample = mixed_sample * (1.0f - transition_factor) + state.prev_voiced_signal * transition_factor;
            }
            
            float final_sample = mixed_sample * CLIPPING_PROTECTION_FACTOR * VOICE_AMPLITUDE_BOOST * volume_factor; // Apply volume factor here
            float normalized_output = apply_master_normalization(final_sample);
            float emphasis_output = apply_preemphasis_filter(normalized_output, &prev_preemphasis_output);
            
            // === HARD CLIPPING PROTECTION IN FINAL OUTPUT STAGE ===
            if (emphasis_output > 255.0f * MAX_AMPLITUDE_CLAMP) 
                emphasis_output = 255.0f * MAX_AMPLITUDE_CLAMP;
            if (emphasis_output < -255.0f * MAX_AMPLITUDE_CLAMP) 
                emphasis_output = -255.0f * MAX_AMPLITUDE_CLAMP;
                
            int output_value = (int)(emphasis_output + 128);
            
            // === MORE ROBUST MIXING WITH BETTER TRANSITIONS ===
            if (output_value > 255) output_value = 255;
            if (output_value < 0) output_value = 0;
            
            // Add dithering to reduce quantization noise - this is the key addition
            static float dither = 0.0f;  // Static variable for consistent dither pattern
            int final_output_value = output_value + (int)dither;
            
            // Clip final value after adding dither
            if (final_output_value > 255) final_output_value = 255;
            if (final_output_value < 0) final_output_value = 0;
            
            output_buffer[buffer_ptr++] = (char)final_output_value;
            
            // Update dither for next sample - this creates the dithering effect
            dither = ((float)rand() / RAND_MAX) * 0.5f - 0.25f;  // Range: -0.25 to +0.25
            
            // Store signals for transition smoothing (for next frame)
            if (s == samples_to_render - 1) {
                state.prev_voiced_signal = voiced_signal;
                state.prev_unvoiced_signal = unvoiced_signal;
            }
        }
        
        interp_state.prev_f1_freq = stored_f1_freq;
        interp_state.prev_f2_freq = stored_f2_freq;
        interp_state.prev_f3_freq = stored_f3_freq;
        interp_state.prev_f4_freq = stored_f4_freq;
        interp_state.prev_f5_freq = stored_f5_freq;
        interp_state.prev_f6_freq = stored_f6_freq;
        interp_state.prev_f1_amp = stored_f1_amp;
        interp_state.prev_f2_amp = stored_f2_amp;
        interp_state.prev_f3_amp = stored_f3_amp;
        interp_state.prev_f4_amp = stored_f4_amp;
        interp_state.prev_f5_amp = stored_f5_amp;
        interp_state.prev_f6_amp = stored_f6_amp;
        state.is_first_frame = 0;
    }
    
    return buffer_ptr;
}
