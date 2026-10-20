/* scheduler.c
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
 * @file scheduler.c
 * @brief Speech scheduling and phoneme frame generation functions.
 */
 
#include "scheduler.h"
#include "lookup_tables.h"
#include <string.h>
#include <stdio.h>
#include <stdint.h>

/**
 * @struct SchedulerContext
 * @brief Context structure for speech scheduling parameters.
 */
typedef struct {
 unsigned char speed;
 unsigned char pitch;
 unsigned char mouth;
 unsigned char throat;
} SchedulerContext;

/**
 * @var ctx
 * @brief Static scheduler context used throughout the scheduling process.
 */
static SchedulerContext ctx;

/**
 * @fn init_speech_scheduler(unsigned char speed, unsigned char pitch, unsigned char mouth, unsigned char throat)
 * @brief Initializes the speech scheduler with given parameters.
 * 
 * Sets up the global scheduler context with control values for speed, pitch,
 * mouth position and throat position that will be used during phoneme scheduling.
 * 
 * @param speed Speed factor (0-255) - lower values increase speaking rate
 * @param pitch Pitch factor (0-255) - higher values raise pitch
 * @param mouth Mouth shape parameter for articulation
 * @param throat Throat position parameter for resonance
 */
void init_speech_scheduler(unsigned char speed, unsigned char pitch, unsigned char mouth, unsigned char throat) {
 ctx.speed = speed; 
 ctx.pitch = pitch; 
 ctx.mouth = mouth; 
 ctx.throat = throat;
}

/**
 * @fn find_phoneme_rules(const char *name)
 * @brief Finds phoneme descriptor rules by name.
 * 
 * Searches through the global phoneme rule table to find matching phoneme descriptors.
 * 
 * @param name Name of the phoneme to search for
 * @return Pointer to PhonemeDescriptor if found, NULL otherwise
 */
static const PhonemeDescriptor* find_phoneme_rules(const char *name) {
 int total_phonemes = TOTAL_TOM_PHONEMES;
 for (int i = 0; i < total_phonemes; i++) {
 if (strcmp(tom_phoneme_rules[i].name, name) == 0) {
 return &tom_phoneme_rules[i];
 }
 }
 return NULL;
}

/**
 * @fn base_token(const char *token, char *base)
 * @brief Extracts the base token from a phonetic symbol.
 * 
 * Removes stress markers (0, 1, or 2) from the end of tokens to get their base form.
 * 
 * @param token Input phonetic token with potential stress indicator
 * @param base Output buffer for cleaned base token
 */
static void base_token(const char *token, char *base) {
 size_t n = strlen(token);
 if (n > 0 && token[n - 1] >= '0' && token[n - 1] <= '2') --n;
 if (n >= 16) n = 15;
 memcpy(base, token, n);
 base[n] = '\0';
}

/**
 * @fn is_vowel(const char *token)
 * @brief Determines if a given phonetic token represents a vowel.
 * 
 * Checks whether the provided phoneme symbol corresponds to a vowel sound,
 * excluding consonants like M, N, L, R, W, and Y that are considered non-vowels.
 * 
 * @param token Phonetic token to check
 * @return 1 if token is a vowel, 0 otherwise
 */
static int is_vowel(const char *token) {
 char base[16];
 base_token(token, base);
 const PhonemeDescriptor *rules = find_phoneme_rules(base);
 return (rules && rules->flags == PH_VOICED && strncmp(base, "M", 1) != 0 && strncmp(base, "N", 1) != 0 && strncmp(base, "L", 1) != 0 && strncmp(base, "R", 1) != 0 && strncmp(base, "W", 1) != 0 && strncmp(base, "Y", 1) != 0);
}
/**
 * @fn syllabify_phonemes(const char *input, char *output, size_t output_size)
 * @brief Performs syllable segmentation on phonetic input.
 * 
 * Identifies natural syllable boundaries in a sequence of phonetic symbols
 * by analyzing vowel positions and consonant clusters to improve speech quality.
 * 
 * @param input Input phonetic string (space-separated)
 * @param output Buffer to store the syllabified result
 * @param output_size Size of the output buffer
 * @return 1 on success, 0 on failure
 */
int syllabify_phonemes(const char *input, char *output, size_t output_size) {
 char copy[2048];
 char tokens[256][16];
 int boundary[256] = {0};
 int count = 0;
 
 if (!input || !output || output_size == 0 || strlen(input) >= sizeof(copy)) return 0;
 strcpy(copy, input);
 
 char *word = strtok(copy, " \t\r\n");
 while (word) {
  if (count >= 256) return 0;
  strcpy(tokens[count++], word);
  word = strtok(NULL, " \t\r\n");
 }
 
 int vowel_indices[256];
 int vowel_count = 0;
 for (int i = 0; i < count; ++i) {
  if (strcmp(tokens[i], "SIL") != 0 && is_vowel(tokens[i])) {
   vowel_indices[vowel_count++] = i;
  }
 }
 
 for (int v = 0; v < vowel_count - 1; ++v) {
  int left_vowel = vowel_indices[v];
  int right_vowel = vowel_indices[v + 1];
  
  int has_sil = 0;
  for (int check = left_vowel + 1; check < right_vowel; check++) {
   if (strcmp(tokens[check], "SIL") == 0) has_sil = 1;
  }
  if (has_sil) continue;
  
  int consonants = right_vowel - left_vowel - 1;
  if (consonants <= 1) {
   boundary[right_vowel] = 1;
  } else {
   boundary[right_vowel - (consonants / 2)] = 1;
  }
 }
 
 size_t used = 0;
 output[0] = '\0';
 for (int i = 0; i < count; ++i) {
  if (boundary[i] && i > 0 && strcmp(tokens[i-1], "SIL") != 0 && strcmp(tokens[i], "SIL") != 0) {
   int n = snprintf(output + used, output_size - used, " SIL ");
   if (n < 0 || (size_t)n >= output_size - used) return 0;
   used += (size_t)n;
  }
  int n = snprintf(output + used, output_size - used, "%s%s", (i && output[used-1] != ' ') ? " " : "", tokens[i]);
  if (n < 0 || (size_t)n >= output_size - used) return 0;
  used += (size_t)n;
 }
 return 1;
}

/**
 * @fn schedule_phoneme_frames(const char *phoneme_string, SpeechFrame *output_frames_array)
 * @brief Schedules phoneme frames for audio generation.
 * 
 * Converts a phonetic string into an array of speech frames with appropriate timing,
 * frequency, and amplitude values based on the scheduler's context parameters.
 * 
 * @param phoneme_string Input phonetic string to schedule
 * @param output_frames_array Array of SpeechFrame structures to populate
 * @return Number of frames successfully scheduled
 */
int schedule_phoneme_frames(const char *phoneme_string, SpeechFrame *output_frames_array) {
 char input_copy[512];
 int total_frames = 0;
 int clean_ptr = 0;
 memset(input_copy, 0, 512);
 
 for (int idx = 0; phoneme_string[idx] != '\0' && clean_ptr < 511; idx++) {
  if (phoneme_string[idx] != '[' && phoneme_string[idx] != ']') {
   input_copy[clean_ptr++] = phoneme_string[idx];
  }
 }
 input_copy[clean_ptr] = '\0';

 uint16_t last_vowel_f1 = 500, last_vowel_f2 = 1500, last_vowel_f3 = 2500;
 float duration_remainder = 0.0f;
 
 char *token = strtok(input_copy, " ");
 while (token != NULL && total_frames < 2000) {
  if (strlen(token) == 0) {
   token = strtok(NULL, " ");
   continue;
  }
  int stress_level = 0; 
  char clean_token[16];
  int c_ptr = 0;
  for (int m = 0; token[m] != '\0' && m < 15; m++) {
   if (token[m] == '1') stress_level = 1;
   else if (token[m] == '2') stress_level = 2;
   else if (token[m] == '0') stress_level = 0;
   else clean_token[c_ptr++] = token[m];
  }
  clean_token[c_ptr] = '\0';

  if (strcmp(clean_token, "SIL") == 0) {
   float base_duration = 4.0f;
   float raw_duration = (base_duration * (float)(256 - ctx.speed)) / 128.0f;
   float total_duration_float = raw_duration + duration_remainder;
   int frame_duration = (int)total_duration_float;
   duration_remainder = total_duration_float - (float)frame_duration;
   if (frame_duration <= 0) frame_duration = 1;
   
   for (int k = 0; k < frame_duration && total_frames < 2000; k++) {
    SpeechFrame *frame = &output_frames_array[total_frames];
    memset(frame, 0, sizeof(SpeechFrame));
    frame->type = PH_SILENCE;
    total_frames++;
   }
   token = strtok(NULL, " ");
   continue;
  }
  
  const PhonemeDescriptor *current_target = find_phoneme_rules(clean_token);
  if (current_target != NULL) {
   float base_duration = 14.0f;
   if (current_target->flags == PH_NOISE) base_duration = 6.0f;
   else if (current_target->flags == PH_SILENCE) base_duration = 4.0f;
   
   if (stress_level == 1) base_duration *= 1.5f;
   else if (stress_level == 0 && current_target->flags == PH_VOICED) base_duration *= 0.85f;
   
   float raw_duration = (base_duration * (float)(256 - ctx.speed)) / 128.0f;
   float total_duration_float = raw_duration + duration_remainder;
   int frame_duration = (int)total_duration_float;
   duration_remainder = total_duration_float - (float)frame_duration;
   if (frame_duration <= 0) frame_duration = 1;
   int blend_frames = frame_duration / 3;
   if (blend_frames <= 0) blend_frames = 1;
   
   for (int k = 0; k < frame_duration && total_frames < 2000; k++) {
    float ramp = (float)k / (float)frame_duration;
    SpeechFrame *frame = &output_frames_array[total_frames];
    uint16_t target_f1 = current_target->f1_hz;
    uint16_t target_f2 = current_target->f2_hz;
    uint16_t target_f3 = current_target->f3_hz;
    
    if (current_target->flags == PH_VOICED) {
     if (k < blend_frames) {
      float blend_ramp = (float)k / (float)blend_frames;
      target_f1 = last_vowel_f1 + (uint16_t)((current_target->f1_hz - last_vowel_f1) * blend_ramp);
      target_f2 = last_vowel_f2 + (uint16_t)((current_target->f2_hz - last_vowel_f2) * blend_ramp);
      target_f3 = last_vowel_f3 + (uint16_t)((current_target->f3_hz - last_vowel_f3) * blend_ramp);
     }
    }
    
    frame->f1.frequency = target_f1; frame->f2.frequency = target_f2; frame->f3.frequency = target_f3;
    frame->f4.frequency = current_target->f4_hz; frame->f5.frequency = current_target->f5_hz; frame->f6.frequency = current_target->f6_hz;
    frame->f1.amplitude = current_target->a1; frame->f2.amplitude = current_target->a2; frame->f3.amplitude = current_target->a3;
    frame->f4.amplitude = current_target->a4; frame->f5.amplitude = current_target->a5; frame->f6.amplitude = current_target->a6;
    frame->pitch = (uint16_t)((float)ctx.pitch * (1.02f - (ramp * 0.08f)));
    frame->type = (uint16_t)current_target->flags;
    total_frames++;
   }
   if (current_target->flags == PH_VOICED) {
    last_vowel_f1 = current_target->f1_hz; last_vowel_f2 = current_target->f2_hz; last_vowel_f3 = current_target->f3_hz;
   }
  }
  token = strtok(NULL, " ");
 }
 return total_frames;
}
