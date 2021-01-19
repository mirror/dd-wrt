/*******************************************************************************
Copyright (C) 2015 Annapurna Labs Ltd.

This file may be licensed under the terms of the Annapurna Labs Commercial
License Agreement.

Alternatively, this file can be distributed under the terms of the GNU General
Public License V2 as published by the Free Software Foundation and can be
found at http://www.gnu.org/licenses/gpl-2.0.html

Alternatively, redistribution and use in source and binary forms, with or
without modification, are permitted provided that the following conditions are
met:

    *     Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

    *     Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/**
 * @defgroup grouppll PLL control
 * @ingroup group_ring
 *
 *  @{
 * @file   al_hal_pll_map.h
 *
 * @brief Header file for the PLL HAL driver frequency mapping
 *
 */

#ifndef __AL_HAL_PLL_MAP_H__
#define __AL_HAL_PLL_MAP_H__

#include "al_hal_pll.h"

/* *INDENT-OFF* */
#ifdef __cplusplus
extern "C" {
#endif
/* *INDENT-ON* */

struct al_pll_freq_map_ent {
	enum al_pll_freq	freq;
	uint32_t		freq_val;
	uint32_t		nf;
	uint32_t		nr;
	uint32_t		od;
	uint32_t		bwadj;
};

static const struct al_pll_freq_map_ent al_pll_freq_map_100[] = {
	/* freq,		freq_val,	nf,	nr,	od,	bwadj */
	{ AL_PLL_FREQ_1400_000,	1400000,	27,	0,	1,	16 },
	{ AL_PLL_FREQ_533_333,	533333,		31,	0,	5,	15 },
	{ AL_PLL_FREQ_800_000,	800000,		31,	0,	3,	15 },
	{ AL_PLL_FREQ_1000_000,	1000000,	19,	0,	1,	9 },
	{ AL_PLL_FREQ_1200_000,	1200000,	23,	0,	1,	11 },
	{ AL_PLL_FREQ_1500_000,	1500000,	29,	0,	1,	14 },
	{ AL_PLL_FREQ_1600_000,	1600000,	31,	0,	1,	15 },
	{ AL_PLL_FREQ_1700_000,	1700000,	33,	0,	1,	16 },
	{ AL_PLL_FREQ_1800_000,	1800000,	35,	0,	1,	17 },
	{ AL_PLL_FREQ_1900_000,	1900000,	37,	0,	1,	18 },
	{ AL_PLL_FREQ_2000_000,	2000000,	19,	0,	0,	9 },
//	{ AL_PLL_FREQ_2100_000,	2100000,	20,	0,	0,	9 },
//	{ AL_PLL_FREQ_2200_000,	2200000,	21,	0,	0,	10 },
//	{ AL_PLL_FREQ_2300_000,	2300000,	22,	0,	0,	10 },
//	{ AL_PLL_FREQ_2400_000,	2400000,	23,	0,	0,	11 },
//	{ AL_PLL_FREQ_2500_000,	2500000,	24,	0,	0,	11 },
//	{ AL_PLL_FREQ_2600_000,	2600000,	25,	0,	0,	12 },
//	{ AL_PLL_FREQ_2700_000,	2700000,	26,	0,	0,	12 },
//	{ AL_PLL_FREQ_2800_000,	2800000,	27,	0,	0,	13 },
//	{ AL_PLL_FREQ_3000_000,	3000000,	29,	0,	0,	14 },
//	{ AL_PLL_FREQ_3200_000,	3200000,	31,	0,	0,	15 },
};

static const struct al_pll_freq_map_ent al_pll_freq_map_125[] = {
	/* freq,		freq_val,	nf,	nr,	od,	bwadj */
	{ AL_PLL_FREQ_1250_000,	1250000,	9,	0,	0,	4 },
	{ AL_PLL_FREQ_1500_000,	1500000,	11,	0,	0,	5 },
	{ AL_PLL_FREQ_1625_000,	1625000,	12,	0,	0,	6 },
	{ AL_PLL_FREQ_1750_000,	1750000,	13,	0,	0,	6 },
	{ AL_PLL_FREQ_2000_000,	2000000,	15,	0,	0,	7 },
//	{ AL_PLL_FREQ_2125_000,	2125000,	16,	0,	0,	8 },
//	{ AL_PLL_FREQ_2250_000,	2250000,	17,	0,	0,	8 },
//	{ AL_PLL_FREQ_2375_000,	2375000,	18,	0,	0,	9 },
//	{ AL_PLL_FREQ_2500_000,	2500000,	19,	0,	0,	9 },
//	{ AL_PLL_FREQ_2625_000,	2625000,	20,	0,	0,	10 },
//	{ AL_PLL_FREQ_2750_000,	2750000,	21,	0,	0,	10 },
//	{ AL_PLL_FREQ_2875_000,	2875000,	22,	0,	0,	11 },
//	{ AL_PLL_FREQ_3000_000,	3000000,	23,	0,	0,	11 },
	{ AL_PLL_FREQ_533_333,	533333,		127,	14,	1,	63 },
	{ AL_PLL_FREQ_666_666,	666666,		63,	2,	3,	31 },
	{ AL_PLL_FREQ_916_666,	916666,		43,	2,	1,	21 },
	{ AL_PLL_FREQ_933_333,	933333,		223,	14,	1,	111 },
	{ AL_PLL_FREQ_1062_500,	1062500,	84,	4,	1,	42 },
	{ AL_PLL_FREQ_800_000,	800000,		63,	4,	1,	31 },
};

static const struct al_pll_freq_map_ent al_pll_freq_map_25[] = {
	/* freq,		freq_val,	nf,	nr,	od,	bwadj */
	{ AL_PLL_FREQ_1250_000,	1250000,	49,	0,	0,	24 },
	{ AL_PLL_FREQ_1500_000,	1500000,	59,	0,	0,	29 },
	{ AL_PLL_FREQ_1625_000,	1625000,	64,	0,	0,	32 },
	{ AL_PLL_FREQ_1750_000,	1750000,	69,	0,	0,	34 },
	{ AL_PLL_FREQ_2000_000,	2000000,	79,	0,	0,	39 },
//	{ AL_PLL_FREQ_2125_000,	2125000,	84,	0,	0,	42 },
//	{ AL_PLL_FREQ_2250_000,	2250000,	89,	0,	0,	44 },
//	{ AL_PLL_FREQ_2375_000,	2375000,	94,	0,	0,	47 },
//	{ AL_PLL_FREQ_2500_000,	2500000,	99,	0,	0,	49 },
//	{ AL_PLL_FREQ_2625_000,	2625000,	104,	0,	0,	52 },
//	{ AL_PLL_FREQ_2750_000,	2750000,	109,	0,	0,	54 },
//	{ AL_PLL_FREQ_2875_000,	2875000,	114,	0,	0,	57 },
//	{ AL_PLL_FREQ_3000_000,	3000000,	119,	0,	0,	59 },
	{ AL_PLL_FREQ_533_333,	533333,		127,	2,	1,	63 },
	{ AL_PLL_FREQ_666_666,	666666,		159,	2,	1,	79 },
	{ AL_PLL_FREQ_916_666,	916666,		219,	2,	1,	109 },
	{ AL_PLL_FREQ_933_333,	933333,		223,	2,	1,	111 },
	{ AL_PLL_FREQ_1062_500,	1062500,	84,	0,	1,	42 },
	{ AL_PLL_FREQ_800_000,	800000,		63,	0,	1,	31 },
};

/* *INDENT-OFF* */
#ifdef __cplusplus
}
#endif
/* *INDENT-ON* */
#endif

/** @} end of PLL group */

