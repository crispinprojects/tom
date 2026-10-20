/* main.c
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
 * @file main.c
 * @brief Main entry point for the TOM (Talking Output Machine) speech synthesizer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "wave_generator.h"
#include "scheduler.h"
#include "dictionary.h"

#define MAX_AUDIO_SAMPLES 524288
static char audio_output[MAX_AUDIO_SAMPLES];


/**
 * @fn WriteWav(char* filename, char* buffer, int bufferlength)
 * @brief Writes audio data to a WAV file format.
 * 
 * Creates and writes a standard PCM WAV file containing the specified audio buffer.
 * 
 * @param filename Name of the output WAV file
 * @param buffer Audio data buffer to write
 * @param bufferlength Length in bytes of the audio data
 */
void WriteWav(char* filename, char* buffer, int bufferlength)
{
 unsigned int filesize;
 unsigned int fmtlength = 16;
 unsigned short int format = 1; // PCM
 unsigned short int channels = 1;
 unsigned int samplerate = 22050;
 unsigned short int blockalign = 1;
 unsigned short int bitspersample = 8;
 
 if (filename == NULL) return;
 
 FILE *file = fopen(filename, "wb");
 if (file == NULL) return; 
 
 fwrite("RIFF", 4, 1, file);
 filesize = bufferlength + 12 + 16 + 8 - 8;
 fwrite(&filesize, 4, 1, file);
 fwrite("WAVE", 4, 1, file);
 fwrite("fmt ", 4, 1, file);
 fwrite(&fmtlength, 4, 1, file);
 fwrite(&format, 2, 1, file);
 fwrite(&channels, 2, 1, file);
 fwrite(&samplerate, 4, 1, file);
 fwrite(&samplerate, 4, 1, file); 
 fwrite(&blockalign, 2, 1, file);
 fwrite(&bitspersample, 2, 1, file);
 fwrite("data", 4, 1, file);
 fwrite(&bufferlength, 4, 1, file);
 fwrite(buffer, bufferlength, 1, file);
 fclose(file);
}
/**
 * @fn print_help(void)
 * @brief Prints usage information and command-line options.
 */
void print_help(void) {
 printf("TOM: Talking Output Machine - speech synthesiser\n");
 printf("usage: tom \"Phrase Text\" [options]\n");
 printf("options:\n");
 printf(" -wav filename    Output directly to a WAV audio file\n");
 printf(" -speed value     (default=72)\n");
 printf(" -pitch value     (default=64)\n");
 printf(" -av factor       Audio volume amplification factor (default=1.0)\n");
 printf(" -v               Print version information\n");
}

/**
 * @fn print_version(void)
 * @brief Prints version information for the TOM synthesizer.
 */
void print_version(void) {
    printf("TOM: Talking Output Machine for Text Synthesis version 0.1.0\n");
}

/**
 * @fn main(int argc, char **argv)
 * @brief Main entry point of the TOM speech synthesizer application.
 * 
 * Parses command-line arguments and processes text input to generate synthesized speech.
 * Supports various options including output file specification, speed control,
 * pitch adjustment, and volume amplification.
 * 
 * @param argc Number of command line arguments
 * @param argv Array of command line argument strings
 * @return Exit status (0 for success)
 */
int main(int argc, char **argv) {
 if (argc < 2) {
 print_help();
 return 1;
 } 
 
 unsigned char speed = 72;
 unsigned char pitch = 64;
 float av = 1.0f; // Audio volume factor
 char* wavfilename = NULL; 
 
 char text_raw[512]; 
 memset(text_raw, 0, sizeof(text_raw)); 
 
 int i = 1;
 while (i < argc) {
 if (argv[i][0] == '-') {
 if (strcmp(argv[i], "-speed") == 0 && (i + 1) < argc) {
 speed = (unsigned char)atoi(argv[i + 1]);
 i += 2;
 continue;
 } else if (strcmp(argv[i], "-pitch") == 0 && (i + 1) < argc) {
 pitch = (unsigned char)atoi(argv[i + 1]);
 i += 2;
 continue;
 } else if (strcmp(argv[i], "-wav") == 0 && (i + 1) < argc) {
 wavfilename = argv[i + 1];
 i += 2;
 continue;
 } else if (strcmp(argv[i], "-av") == 0 && (i + 1) < argc) {
 av = (float)atof(argv[i + 1]);
 i += 2;
 continue;
 } else if (strcmp(argv[i], "-v") == 0) {
 print_version();
 return 0; 
 } else {
 print_help();
 return 1;
 }
 } else {
 if (strlen(text_raw) > 0) {
 strncat(text_raw, " ", sizeof(text_raw) - 1 - strlen(text_raw));
 }
 strncat(text_raw, argv[i], sizeof(text_raw) - 1 - strlen(text_raw));
 i++;
 }
 } 
 
 if (strlen(text_raw) == 0) {
 fprintf(stderr, "TOM Error: Input string empty.\n");
 return 1;
 }
 
 printf("Input: %s\n", text_raw);
 printf("Speed: %d, Pitch: %d, Volume Factor: %.2f\n", speed, pitch, av); 
 
 for (int j = 0; text_raw[j] != '\0'; j++) {
 text_raw[j] = (char)toupper((unsigned char)text_raw[j]);
 } 
 
 memset(audio_output, 128, MAX_AUDIO_SAMPLES); 
 
 // initialize subsystem scheduling rules 
 init_speech_scheduler(speed, pitch, 0, 0);
 init_wave_generator(); 
 
 char translated_phonemes[2048];
 translate_word_phrase(translated_phonemes, text_raw); //dictionary 
 printf("Phonetic output (raw): %s\n", translated_phonemes); 
 
 //syllable segmentation processing
 char syllabified_phonemes[2048];
 if (syllabify_phonemes(translated_phonemes, syllabified_phonemes, sizeof(syllabified_phonemes))) {
 printf("Phonetic output (syllabified): %s\n", syllabified_phonemes);
 } else {
 printf("WARNING: Syllabification failed, falling back to raw phonemes.\n");
 strcpy(syllabified_phonemes, translated_phonemes);
 }

 SpeechFrame scheduled_frames[2000];
 memset(scheduled_frames, 0, sizeof(scheduled_frames)); 
 
 // Use the new syllabified phoneme sequence to assign frame arrays
 int total_frames = schedule_phoneme_frames(syllabified_phonemes, scheduled_frames); 
 
 // Generate speech with volume control
 int written_bytes = generate_speech_wave(audio_output, scheduled_frames, total_frames, av); 
 
 if (wavfilename != NULL) {
 WriteWav(wavfilename, audio_output, written_bytes); 
 printf("TOM: Synthesized audio written safely to '%s'.\n", wavfilename);
 } else {
 printf("TOM: Done. (No -wav file option declared)\n");
 }
 
 return 0;
}
