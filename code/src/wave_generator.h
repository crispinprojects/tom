/* wave_generator.h
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
 * @file wave_generator.h
 * @brief Header file for the TOM speech synthesis waveform generator.
 */
#ifndef WAVE_GENERATOR_H
#define WAVE_GENERATOR_H

#include <stdint.h>

/**
 * @struct Formant
 * @brief Structure representing a single formant in the vocal tract.
 * 
 * A formant is a peak in the frequency spectrum of sound produced by the human vocal tract,
 * characterized by its center frequency, amplitude, and bandwidth.
 */
typedef struct {
    uint16_t frequency;  
    uint16_t amplitude;
    uint16_t bandwidth;   // Added bandwidth field for formant shaping
} Formant;

/**
 * @struct SpeechFrame
 * @brief Structure representing a single frame of speech parameters.
 * 
 * Contains all necessary parameters to generate one time slice of audio,
 * including formant frequencies and amplitudes, pitch information, and phoneme type.
 */
typedef struct {
    Formant f1;
    Formant f2;
    Formant f3;
    Formant f4;
    Formant f5;
    Formant f6;
    uint16_t pitch;
    uint16_t type;      // Hardcoded to an explicit 16-bit width to crush compiling pointer shifts!
} SpeechFrame;

/**
 * @fn init_wave_generator(void)
 * @brief Initializes the wave generator engine.
 * 
 * Must be called before any audio generation functions to set up internal state.
 */
void init_wave_generator(void);

/**
 * @fn generate_speech_wave(char *output_buffer, SpeechFrame *frames, int frame_count, float volume_factor)
 * @brief Generates waveform data from speech frames.
 * 
 * Processes an array of speech frames and generates a complete audio buffer suitable
 * for writing to WAV files or real-time playback.
 * 
 * @param output_buffer Buffer where generated audio samples will be stored (8-bit PCM)
 * @param frames Array of SpeechFrame structures containing synthesis parameters
 * @param frame_count Number of frames in the input array
 * @param volume_factor Factor to adjust overall amplitude of generated sound
 * @return Number of bytes written to output buffer
 */
int generate_speech_wave(char *output_buffer, SpeechFrame *frames, int frame_count, float volume_factor);

#endif
