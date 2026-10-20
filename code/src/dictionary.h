/* dictionary.h
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
 * @file dictionary.h
 * @brief Header file containing phonetic dictionary and translation functions.
 */
#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <string.h>
#include <ctype.h>
#include <stddef.h> // Includes size_t type-definition cleanly

/**
 * @struct PhoneticEntry
 * @brief Structure representing a phonetic entry with text and phoneme representation.
 */
typedef struct {
    const char *text;
    const char *phoneme;
} PhoneticEntry;

/**
 * @var dictionary
 * @brief Static array of phonetic entries for common words, dates, and phrases.
 * 
 * This dictionary contains mappings from English text to their phonetic representations,
 * including days of the week, months, ordinal numbers, cardinal numbers, time expressions,
 * and other common terms used in speech synthesis.
 */
static const PhoneticEntry dictionary[] = {
    {"MONDAY",    "M AA1 N D EY0"}, 
    {"TUESDAY",   "T UW1 Z D EY0"},
    {"WEDNESDAY", "W EH1 N Z D EY0"},
    {"THURSDAY",  "TH ER1 Z D EY0"},
    {"FRIDAY",    "F R AY1 D EY0"},
    {"SATURDAY",  "S AE1 T ER0 D EY0"},
    {"SUNDAY",    "S AH1 N D EY0"}, 
    
    {"JANUARY",   "JH AE1 N Y UX0 EH0 R IY0"},
    {"FEBRUARY",  "F EH1 B R UX0 EH0 R IY0"},
    {"MARCH",     "M AA1 R CH"},
    {"APRIL",     "EY1 P R IH0 L"},
    {"MAY",       "M EY1"},
    {"JUNE",      "J UX1 N"},
    {"JULY",      "J UX0 L AY1"},
    {"AUGUST",    "AO1 G AH0 S T"},
    {"SEPTEMBER", "S EH0 P T EH1 M B ER0"}, 
    {"OCTOBER",   "AA0 K T OW1 B ER0"}, 
    {"NOVEMBER",  "N OW0 V EH1 M B ER0"},
    {"DECEMBER",  "D IH0 S EH1 M B ER0"},   
  
        
    //ordinal numbers (e.g., "first", "second") with stress (dates)
	{"1ST",       "F ER1 S T"},
	{"2ND",       "S EH1 K AH0 N D"},
	{"3RD",       "TH ER1 D"},
	{"4TH",       "F OW1 R TH"},
	//{"5TH",       "F AY1 V TH"},
	{"5TH",       "F IH1 F TH"}, 
	{"6TH",       "S IH1 K S TH"},
	{"7TH",       "S EH1 V AH0 N TH"},
	{"8TH",       "EY1 T HH"},
	{"9TH",       "N AE1 N TH"},
	{"10TH",      "T EH1 N TH"},
	{"11TH",      "EH1 L EH1 V AH0 N TH"},
	{"12TH",      "TW EH1 L F TH"},
	{"13TH",      "TH ER1 T IY0 N TH"},
	{"14TH",      "F AO1 R T IY0 N TH"},
	{"15TH",      "F AY1 F T IY0 N TH"},
	{"16TH",      "S IH1 K S T IY0 N TH"},
	{"17TH",      "S EV AH1 N T IY0 N TH"},
	{"18TH",      "EY1 T IY0 N TH"},
	{"19TH",      "N AE1 N T IY0 N TH"},
	{"20TH",      "T W EH1 N T IY0 TH"},
	{"21ST",      "T W EH1 N T IY0 OW1 N TH"},
	{"22ND",      "T W EH1 N T IY0 T EY1 N D"},
	{"23RD",      "T W EH1 N T IY0 TH ER1 D"},
	{"24TH",      "T W EH1 N T IY0 F OW1 R TH"},
	{"25TH",      "T W EH1 N T IY0 F AY1 V TH"},
	{"26TH",      "T W EH1 N T IY0 S IH1 K S TH"},
	{"27TH",      "T W EH1 N T IY0 SEV AH1 N TH"},
	{"28TH",      "T W EH1 N T IY0 EY1 T HH"},
	{"29TH",      "T W EH1 N T IY0 N AE1 N TH"},	
	{"30TH",      "TH ER1 T IY0 IX0 TH"},
	{"31ST",      "TH ER1 T IY0 S T"},
		
	//cardinal numbers with stress (time)
	{"1",  "W AH1 N"},
	{"2",  "T UW1"},
	{"TWO",  "T UW1"},
	//{"2", "T UW0"}
	//{"2", "T AH0"},
	{"3",  "TH R IY1"},
	{"4",  "F AO1 R"},
	{"5",  "F AY1 V"},
	{"6",  "S IH1 K S"},
	{"SIX",  "S IH1 K S"},
	{"7",  "S EH1 V AH0 N"},
	{"8",  "EY1 T"},
	{"9",  "N AY1 N"},
	{"10", "T EH1 N"},
	{"11", "IH0 L EH1 V AH0 N"},
	{"12", "T W EH1 L V"},
	{"13", "TH ER2 T IY1 N"},
	{"14", "F AO2 R T IY1 N"},
	{"15", "F IH2 F T IY1 N"},
	{"16", "S IH2 K S T IY1 N"},
	{"17", "S EH2 V AH0 N T IY1 N"},
	{"18", "EY2 T IY1 N"},
	{"19", "N AY2 N T IY1 N"},

	{"20", "T W EH1 N T IY0"},
	{"21", "T W EH1 N T IY0 W AH1 N"},
	{"22", "T W EH1 N T IY0 T UW1"},
	{"23", "T W EH1 N T IY0 TH R IY1"},
	{"24", "T W EH1 N T IY0 F AO1 R"},
	{"25", "T W EH1 N T IY0 F AY1 V"},
	{"26", "T W EH1 N T IY0 S IH1 K S"},
	{"27", "T W EH1 N T IY0 S EH1 V AH0 N"},
	{"28", "T W EH1 N T IY0 EY1 T"},
	{"29", "T W EH1 N T IY0 N AY1 N"},

	{"30", "TH ER1 D IY0"},
	{"THIRTY", "TH ER1 D IY0"},
	{"31", "TH ER1 D IY0 W AH1 N"},
	{"32", "TH ER1 D IY0 T UW1"},
	{"33", "TH ER1 D IY0 TH R IY1"},
	{"34", "TH ER1 D IY0 F AO1 R"},
	{"35", "TH ER1 D IY0 F AY1 V"},
	{"36", "TH ER1 D IY0 S IH1 K S"},
	{"37", "TH ER1 D IY0 S EH1 V AH0 N"},
	{"38", "TH ER1 D IY0 EY1 T"},
	{"39", "TH ER1 D IY0 N AY1 N"},

	{"40", "F AO1 R T IY0"},
	{"41", "F AO1 R T IY0 W AH1 N"},
	{"42", "F AO1 R T IY0 T UW1"},
	{"43", "F AO1 R T IY0 TH R IY1"},
	{"44", "F AO1 R T IY0 F AO1 R"},
	{"45", "F AO1 R T IY0 F AY1 V"},
	{"46", "F AO1 R T IY0 S IH1 K S"},
	{"47", "F AO1 R T IY0 S EH1 V AH0 N"},
	{"48", "F AO1 R T IY0 EY1 T"},
	{"49", "F AO1 R T IY0 N AY1 N"},

	{"50", "F IH1 F T IY0"},
	{"51", "F IH1 F T IY0 W AH1 N"},
	{"52", "F IH1 F T IY0 T UW1"},
	{"53", "F IH1 F T IY0 TH R IY1"},
	{"54", "F IH1 F T IY0 F AO1 R"},
	{"55", "F IH1 F T IY0 F AY1 V"},
	{"56", "F IH1 F T IY0 S IH1 K S"},
	{"57", "F IH1 F T IY0 S EH1 V AH0 N"},
	{"58", "F IH1 F T IY0 EY1 T"},
	{"59", "F IH1 F T IY0 N AY1 N"},
	
	{"ZERO", "Z IH1 R OW0"},
	{"OH", "OW1"},	
	{"THE",  "DH AH0"},
	{"TIME", "T AY1 M"},	
	{"AM", "EY1 EH1 M"},
	{"PM", "P IY1 EH1 M"},
    
    {"HAPPY",    "HH AE1 P IY0"},  
    {"BIRTHDAY", "B ER1 TH D EY0"},  
    {"MEETING",  "M IY1 T IH0 N G"},
    {"DOCTOR", "D AA1 K T ER0"},
    {"DENTIST", "D EH1 N T IH0 S T"},
    {"APPOINTMENT", "AH1 P OY1 N T M EH0 N T"},
	{"REMINDER", "R IH1 M AY0 N D ER0"},
	{"HOLIDAY", "HH AA1 L IH0 D EY0"},
	{"WORK", "W ER1 K"},
    {"WORLD",    "W ER1 L D"},
    {"HELLO",    "HH AH0 L OW1"},
    {"MY",       "M AY1"},
    {"NAME",     "N EY1 M"},
    {"IS",       "IH1 Z"},
    {"TOM",      "T AA1 M"}
};

/**
 * @def DICTIONARY_SIZE
 * @brief Macro defining the size of the _dictionary array.
 */
#define DICTIONARY_SIZE (sizeof(dictionary) / sizeof(dictionary[0]))


/**
 * @fn translate_word_phrase(char *output, const char *input_phrase)
 * @brief Translates a phrase from text to phonetic representation using the dictionary.
 * 
 * This function takes an input phrase and converts each word into its phonetic equivalent
 * by looking up words in the calendar_dictionary. Unknown words are passed through unchanged.
 * 
 * @param output Buffer to store the resulting phonetic string (space-separated).
 * @param input_phrase Input text phrase to be translated.
 */
static void translate_word_phrase(char *output, const char *input_phrase) {
    char phrase_copy[1024];
    char temp_out[1024]; 
    
    memset(phrase_copy, 0, 1024);
    memset(temp_out, 0, 1024);
    
    strncpy(phrase_copy, input_phrase, 1023);

    char *word = strtok(phrase_copy, " ");
    while (word != NULL) {
        int found = 0;
        // FIXED: Using size_t cleanly resolves the implicit signedness warning block!
        for (size_t i = 0; i < DICTIONARY_SIZE; i++) {
            if (strcmp(dictionary[i].text, word) == 0) {
                strncat(temp_out, dictionary[i].phoneme, 1023 - strlen(temp_out));
                strncat(temp_out, " ", 1023 - strlen(temp_out));
                strncat(temp_out, "SIL ", 1023 - strlen(temp_out)); 
                found = 1;
                break;
            }
        }
        if (!found) {
            strncat(temp_out, word, 1023 - strlen(temp_out));
            strncat(temp_out, " ", 1023 - strlen(temp_out));
        }
        word = strtok(NULL, " ");
    }
    strcpy(output, temp_out);
}
#endif
