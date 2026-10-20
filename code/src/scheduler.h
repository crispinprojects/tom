/* scheduler.h
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
 * @file scheduler.h
 * @brief Header file containing speech scheduling and phoneme frame generation functions.
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>
#include "wave_generator.h"

/**
 * @fn syllabify_phonemes(const char *input, char *output, size_t output_size)
 * @brief Performs syllable segmentation on phonetic input.
 * 
 * This function analyzes a sequence of phonetic symbols and identifies natural
 * syllable boundaries by examining vowel positions and consonant clusters. It adds
 * appropriate silence markers to indicate where syllables should be separated for
 * improved speech quality and articulation.
 * 
 * @param input Input phonetic string (space-separated) to be syllabified
 * @param output Buffer to store the resulting syllabified phonetic string
 * @param output_size Size of the output buffer in bytes
 * @return 1 on successful syllabification, 0 on failure or error conditions
 */
int syllabify_phonemes(const char *input, char *output, size_t output_size);

/**
 * @fn init_speech_scheduler(unsigned char speed, unsigned char pitch, unsigned char mouth, unsigned char throat)
 * @brief Initializes the speech scheduler with control parameters.
 * 
 * Sets up global scheduling context variables that will be used during phoneme frame
 * generation. These parameters affect the overall characteristics of synthesized speech,
 * including speaking rate, pitch level, and articulatory position settings.
 * 
 * @param speed Speed factor (0-255) - lower values increase speaking rate (default: 72)
 * @param pitch Pitch factor (0-255) - higher values raise pitch (default: 64)
 * @param mouth Mouth shape parameter for articulation control
 * @param throat Throat position parameter for resonance shaping
 */
void init_speech_scheduler(unsigned char speed, unsigned char pitch, unsigned char mouth, unsigned char throat);

/**
 * @fn schedule_phoneme_frames(const char *phoneme_string, SpeechFrame *output_frames_array)
 * @brief Schedules phoneme frames for audio generation.
 * 
 * Converts a phonetic string into an array of speech frames with appropriate timing,
 * frequency, and amplitude values based on the scheduler's context parameters. Each
 * frame contains detailed information about formant frequencies, amplitudes, pitch,
 * and phoneme type needed to generate accurate synthetic speech.
 * 
 * @param phoneme_string Input phonetic string containing space-separated phonemes
 * @param output_frames_array Array of SpeechFrame structures to populate with scheduling results
 * @return Number of frames successfully scheduled (maximum 2000)
 */
int schedule_phoneme_frames(const char *phoneme_string, SpeechFrame *output_frames_array);

#endif
