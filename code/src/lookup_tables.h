/* lookup_tables.h
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
 * @file lookup_tables.h
 * @brief Header file containing phonetic lookup tables and definitions for TOM speech synthesizer.
 */
#ifndef LOOKUP_TABLES_H
#define LOOKUP_TABLES_H

#include <stdint.h>

// Explicit phonetic category tags matching your math-driven architecture
/**
 * @def PH_VOICED
 * @brief Flag indicating a voiced phoneme (vowels, liquids, nasals)
 * 
 * Voiced phonemes are produced with vocal cord vibration and typically involve
 * multiple formant frequencies. These are processed through the sinf multi-oscillator track.
 */
#define PH_VOICED   0x00  // Vowels, Liquids, Nasals (sinf multi-oscillator track)

/**
 * @def PH_NOISE
 * @brief Flag indicating a noise phoneme (fricatives, plosives, stops)
 * 
 * Noise phonemes are produced without vocal cord vibration and involve random sound
 * energy. These are processed through the LFSR random burst track.
 */
#define PH_NOISE    0x01  // Fricatives, Plosives, Stops (LFSR random burst track)

/**
 * @def PH_SILENCE
 * @brief Flag indicating a silence phoneme (absolute quiet padding)
 * 
 * Silence frames represent pauses in speech and contain no audio information.
 * They are used to create timing gaps between words or phrases.
 */
#define PH_SILENCE  0x02  // Absolute quiet padding windows

/**
 * @struct PhonemeDescriptor
 * @brief Structure describing the acoustic characteristics of a phoneme.
 * 
 * This structure defines all parameters needed to generate a specific phonetic sound,
 * including formant frequencies, amplitudes, bandwidths, and classification flags.
 */
typedef struct {
    const char *name;       /**< Name identifier for the phoneme (e.g., "AA", "EH") */
    uint16_t f1_hz;         /**< First formant frequency in Hertz */
    uint16_t f2_hz;         /**< Second formant frequency in Hertz */
    uint16_t f3_hz;         /**< Third formant frequency in Hertz */
    uint16_t f4_hz;         /**< Fourth formant frequency in Hertz */
    uint16_t f5_hz;         /**< Fifth formant frequency in Hertz */
    uint16_t f6_hz;         /**< Sixth formant frequency in Hertz */
    uint8_t a1;             /**< First formant amplitude coefficient (0-15 scale) */
    uint8_t a2;             /**< Second formant amplitude coefficient (0-15 scale) */
    uint8_t a3;             /**< Third formant amplitude coefficient (0-15 scale) */
    uint8_t a4;             /**< Fourth formant amplitude coefficient (0-15 scale) */
    uint8_t a5;             /**< Fifth formant amplitude coefficient (0-15 scale) */
    uint8_t a6;             /**< Sixth formant amplitude coefficient (0-15 scale) */
    uint16_t b1;            /**< First formant bandwidth for resonance shaping */
    uint16_t b2;            /**< Second formant bandwidth for resonance shaping */
    uint16_t b3;            /**< Third formant bandwidth for resonance shaping */
    uint16_t b4;            /**< Fourth formant bandwidth for resonance shaping */
    uint16_t b5;            /**< Fifth formant bandwidth for resonance shaping */
    uint16_t b6;            /**< Sixth formant bandwidth for resonance shaping */
    uint8_t flags;          /**< Phoneme category flag (PH_VOICED, PH_NOISE, or PH_SILENCE) */
} PhonemeDescriptor;

/**
 * @var tom_phoneme_rules
 * @brief Array of phonetic descriptors defining the complete TOM phoneme set.
 * 
 * This lookup table contains detailed acoustic parameters for 67 standard ARPAbet phonemes,
 * including vowels, consonants (stops, fricatives, nasals, liquids), and special symbols.
 * Each entry defines six formant frequencies, amplitudes, bandwidths, and classification flags
 * needed for high-quality speech synthesis.
 */
static const PhonemeDescriptor tom_phoneme_rules[] = {
		// Vowels and diphthongs
	{ "IY",   270, 2290, 3010, 4000, 5000, 6000,  14, 12, 10, 8, 6, 4,  50, 200, 400, 198, 248, 298, PH_VOICED },
	{ "IH",   390, 1990, 2550, 3500, 4500, 5500,  14, 13, 10, 8, 6, 4,  50, 100, 140, 179, 229, 279, PH_VOICED },
	{ "EH",   530, 1760, 2820, 3800, 4800, 5800,  14, 12, 10, 8, 6, 4,  60, 90, 200, 175, 225, 275, PH_VOICED },
	{ "AE",   730, 1700, 2500, 3500, 4500, 5500,  15, 11,  9, 8, 6, 4,  70, 130, 300, 172, 217, 272, PH_VOICED },
	{ "AA",   700, 1100, 2500, 3500, 4500, 5500,  15, 10,  8, 8, 6, 4,  130, 70, 160, 168, 223, 278, PH_VOICED },
	{ "AH",   620, 1200, 2400, 3400, 4400, 5400,  14, 11,  9, 8, 6, 4,  80, 50, 140, 168, 223, 278, PH_VOICED },
	{ "AO",   570,  850, 2500, 3500, 4500, 5500,  15, 10,  8, 8, 6, 4,  90, 100, 80, 179, 224, 279, PH_VOICED },
	{ "UH",   440,  960, 2300, 3300, 4300, 5300,  14, 11,  8, 8, 6, 4,  80, 100, 80, 168, 218, 268, PH_VOICED },
	{ "UW",   300,  870, 2240, 3200, 4200, 5200,  13, 10,  8, 8, 6, 4,  65, 110, 140, 160, 210, 260, PH_VOICED },
	{ "ER",   490, 1350, 1690, 2690, 3690, 4690,  14, 12, 10, 8, 6, 4,  100, 60, 110, 82, 87, 92, PH_VOICED },
	{ "AX",   500, 1500, 2500, 3500, 4500, 5500,  12, 10,  8, 8, 6, 4,  80, 50, 140, 174, 224, 274, PH_VOICED },
	{ "IX",   390, 1990, 2550, 3550, 4550, 5550,  13, 11,  9, 8, 6, 4,  50, 100, 140, 176, 226, 276, PH_VOICED },
	// Diphthong midpoint values.
	{ "EY",   500, 2000, 2600, 3600, 4600, 5600,  14, 12, 10, 8, 6, 4,  70, 100, 200, 176, 226, 276, PH_VOICED },
	{ "OW",   430,  850, 2300, 3300, 4300, 5300,  14, 10,  8, 8, 6, 4,  80, 70, 70, 165, 215, 265, PH_VOICED },
	{ "AW",   520, 1000, 2350, 3350, 4350, 5350,  12,  9,  7, 8, 6, 4,  80, 70, 140, 178, 223, 278, PH_VOICED },
	{ "AY",   500, 1500, 2550, 3550, 4550, 5550,  12,  9,  8, 8, 6, 4,  100, 70, 200, 168, 223, 278, PH_VOICED },
	{ "OY",   450, 1400, 2500, 3500, 4500, 5500,  12,  9,  8, 8, 6, 4,  80, 50, 130, 170, 220, 270, PH_VOICED },
	// Stops.
	{ "B",    400, 1200, 2200, 3200, 4200, 5200,  10,  7,  4, 8, 6, 4,  65, 90, 125, 500, 600, 800, PH_NOISE },
	{ "P",    400, 1500, 3000, 4000, 5000, 6000,   9,  7,  5, 8, 6, 4,  300, 190, 185, 500, 600, 800, PH_NOISE },
	{ "D",    1800, 2800, 4000, 5000, 6000, 7000,  10,  8,  5, 8, 6, 4,  60, 100, 170, 500, 600, 800, PH_NOISE },
	{ "T",  3000, 4500, 6000, 7000, 8000, 9000,   8,  9,  8, 8, 6, 4,  300, 180, 220, 500, 600, 800, PH_NOISE },
	//{ "T", 	  2000, 3500, 5000, 6000, 7000, 8000,  8, 9, 8, 6, 4, 2,   300, 180, 220, 500, 600, 800, PH_NOISE },
	{ "G",    500,  1500, 2500, 3500, 4500, 5500,  10,  7,  4, 8, 6, 4,  70, 145, 190, 500, 600, 800, PH_NOISE },
	{ "K",    1800, 3000, 5000, 6000, 7000, 8000,   8,  9,  7, 8, 6, 4,  280, 220, 250, 500, 600, 800, PH_NOISE },
	// Fricatives
	{ "F",    300, 2500, 4000, 5000, 6000, 7000,   8,  7,  5, 8, 6, 4,  225, 120, 175, 500, 600, 800, PH_NOISE },
	//{ "V",    300, 2500, 4000, 5000, 6000, 7000,   8,  7,  5, 8, 6, 4,  55, 95, 125, 500, 600, 800, PH_NOISE },
	{ "TH",   300, 2200, 3500, 4500, 5500, 6500,   7,  6,  4, 8, 6, 4,  200, 90, 200, 500, 600, 800, PH_NOISE },
	{ "DH",   300, 2200, 3500, 4500, 5500, 6500,   7,  6,  4, 8, 6, 4,  60, 95, 185, 500, 600, 800, PH_NOISE },
	{ "S",    500, 4500, 6500, 7500, 8500, 9500,   2,  8, 12, 8, 6, 4,  200, 95, 220, 500, 600, 800, PH_NOISE },
	//{ "Z",    500, 4500, 6500, 7500, 8500, 9500,   2,  8, 12, 8, 6, 4,  70, 60, 180, 500, 600, 800, PH_NOISE },
	{ "SH",   300, 2500, 4000, 5000, 6000, 7000,   4,  9,  8, 8, 6, 4,  200, 110, 280, 500, 600, 800, PH_NOISE },
	{ "ZH",   300, 2500, 4000, 5000, 6000, 7000,   4,  9,  8, 8, 6, 4,  200, 100, 300, 500, 600, 800, PH_NOISE },
	{ "CH",   1200, 3000, 5000, 6000, 7000, 8000,   5,  9,  8, 8, 6, 4,  200, 110, 270, 500, 600, 800, PH_NOISE },
	{ "JH",   1200, 3000, 5000, 6000, 7000, 8000,   5,  9,  8, 8, 6, 4,  50, 110, 270, 500, 600, 800, PH_NOISE },
	//change V and Z fricatives to voiced
	{ "V",    300, 2500, 4000, 5000, 6000, 7000,   8,  7,  5, 8, 6, 4,  55, 95, 125, 500, 600, 800, PH_VOICED },
	{ "Z",    500, 4500, 6500, 7500, 8500, 9500,   2,  8, 12, 8, 6, 4,  70, 60, 180, 500, 600, 800, PH_VOICED },
	// Glottal fricative.	
	//{ "HH",   500, 1500, 2500, 3500, 4500, 5500,   5,  4,  3, 8, 6, 4,  300, 160, 300, 500, 600, 800, PH_NOISE},
	{ "HH", 500, 1500, 2500, 3500, 4500, 5500, 5, 4, 3, 8, 6, 4, 300, 160, 300, 500, 600, 800, PH_VOICED },

	// Nasals.
	{ "M",    280,  900, 2200, 3200, 4200, 5200,  10,  5,  3, 8, 6, 4,  70, 110, 450, 500, 600, 800, PH_VOICED },
	{ "N",    280, 1700, 2600, 3600, 4600, 5600,  10,  6,  4, 8, 6, 4,  70, 110, 450, 500, 600, 800, PH_VOICED },
	{ "NG",   300,  900, 2200, 3200, 4200, 5200,  10,  5,  3, 8, 6, 4,  160, 150, 100, 500, 600, 800, PH_VOICED },
	// Liquids and glides.
	{ "W",    300,  610, 2200, 3200, 4200, 5200,  12,  9,  7, 8, 6, 4,  50, 80, 60, 500, 600, 800, PH_VOICED },
	{ "R",    490,  800, 1690, 2690, 3690, 4690,  13, 11,  9, 8, 6, 4,  70, 100, 120, 500, 600, 800, PH_VOICED },
	{ "L",    360, 1030, 2880, 3880, 4880, 5880,  13, 10,  5, 8, 6, 4,  50, 100, 280, 500, 600, 800, PH_VOICED },
	{ "Y",    280, 2250, 3100, 4100, 5100, 6100,  12, 13, 10, 8, 6, 4,  40, 250, 500, 500, 600, 800, PH_VOICED },
	// Others
	{ "H",    700, 1200, 2500, 3500, 4500, 5500,   5,   4,   3, 8, 6, 4,  200, 120, 200, 500, 600, 800, PH_NOISE },
	{ "UX",   300,  870, 2240, 3240, 4240, 5240,  13, 10,  8, 8, 6, 4,  65, 110, 140, 160, 210, 260, PH_VOICED }, // alias for UW
	{ "OH",   570,  850, 2500, 3500, 4500, 5500,  14, 10,  8, 8, 6, 4,  90, 100, 80, 179, 224, 279, PH_VOICED }, // custom alias for AO
	{ "EL",   350, 1200, 2500, 3500, 4500, 5500,  10,  7,  5, 8, 6, 4,  65, 60, 80, 500, 600, 800, PH_VOICED },  // custom syllabic-L approximation
	{ "SIL",    0,    0,    0,    0,    0,    0,   0,   0,   0, 0, 0, 0,   0,  0,  0,  0,  0,  0, PH_SILENCE }
};

/**
 * @def TOTAL_TOM_PHONEMES
 * @brief Macro defining the total number of phonemes in the tom_phoneme_rules table.
 */
#define TOTAL_TOM_PHONEMES (sizeof(tom_phoneme_rules) / sizeof(tom_phoneme_rules[0]))

#endif
