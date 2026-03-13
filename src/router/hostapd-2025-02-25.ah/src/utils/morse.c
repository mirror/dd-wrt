/*
 * Copyright 2022 Morse Micro
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"

#include <sys/stat.h>
#include <assert.h>
#include "utils/common.h"
#include "drivers/driver.h"
#include "drivers/nl80211_copy.h"
#include "common/hw_features_common.h"

#include "morse.h"
#include "config.h"

#define IEEE80211_CHAN_1MHZ (1)
#define IEEE80211_CHAN_2MHZ (2)
#define IEEE80211_CHAN_4MHZ (4)
#define IEEE80211_CHAN_8MHZ (8)
#define IEEE80211_CHAN_16MHZ (16)
#define START_FREQ_5GHZ (5000)
#define END_FREQ_5GHZ (5900)
#define HT_FREQ_TO_HT_CHAN(ht_freq) ((ht_freq - START_FREQ_5GHZ) / 5)
#define IN_LOWER_HT40_RANGE(ht_chan) (ht_chan >= 38 && ht_chan <= 62 ? 1 : 0)
#define IN_MIDDLE_HT40_RANGE(ht_chan) (ht_chan >= 102 && ht_chan <= 134 ? 1 : 0)
#define IN_HIGHER_HT40_RANGE(ht_chan) (ht_chan >= 151 && ht_chan <= 175 ? 1 : 0)

#define S1G_CHAN_SEP_KHZ (500)

#define PRIM_CHAN_INDEX_MIN (0)

struct s1g_ht_chan_pair {
	int s1g_channel;
	int ht_channel;
	int bw;
};

static const int vht80_chans[] = {
	42, 58, 106, 122, 155, 171
};
static const unsigned int num_vht80_chans = ARRAY_SIZE(vht80_chans);

static const int vht160_chans[] = {
	50, 114, 163
};
static const unsigned int num_vht160_chans = ARRAY_SIZE(vht160_chans);

static const int ht_40_pri_1mhz_offset[] = {
	-2, 2,
};

static const int vht_80_pri_1mhz_offset[] = {
	-6, -2, 2, 6
};

static const int vht_160_pri_1mhz_offset[] = {
	-14, -10, -6, -2, 2, 6, 10, 14
};

static const int s1g_pri_1mhz_offset_default[] = {
	-1, 1
};

static const int s1g_pri_1mhz_offset_jp[] = {
	-13, -11
};

static const int s1g_overlap_chan_jp[] = {
	4, 8, 38
};

/** Implements the channelisation scheme used in most regions (AU, US etc.) */
static const struct s1g_ht_chan_pair s1g_ht_chan_pairs_default[] = {
	/* nulls for alignment */
	{-1, -1, -1},
	{1, 132, 1},
	{2, 134, 2},
	{3, 136, 1},
	{4, -1, -1},	/* unmapped */
	{5, 36, 1},
	{6, 38, 2},
	{7, 40, 1},
	{8, 42, 4},
	{9, 44, 1},
	{10, 46, 2},
	{11, 48, 1},
	{12, 50, 8},
	{13, 52, 1},
	{14, 54, 2},
	{15, 56, 1},
	{16, 58, 4},
	{17, 60, 1},
	{18, 62, 2},
	{19, 64, 1},
	{20, -1, 16},	/* unmapped */
	{21, 100, 1},
	{22, 102, 2},
	{23, 104, 1},
	{24, 106, 4},
	{25, 108, 1},
	{26, 110, 2},
	{27, 112, 1},
	{28, 114, 8},
	{29, 116, 1},
	{30, 118, 2},
	{31, 120, 1},
	{32, 122, 4},
	{33, 124, 1},
	{34, 126, 2},
	{35, 128, 1},
	{36, -1, -1},	/* unmapped */
	{37, 149, 1},
	{38, 151, 2},
	{39, 153, 1},
	{40, 155, 4},
	{41, 157, 1},
	{42, 159, 2},
	{43, 161, 1},
	{44, 163, 8},
	{45, 165, 1},
	{46, 167, 2},
	{47, 169, 1},
	{48, 171, 4},
	{49, 173, 1},
	{50, 175, 2},
	{51, 177, 1},
};

/** Implements the JP specific channelisation scheme */
static const struct s1g_ht_chan_pair s1g_ht_chan_pairs_jp[] = {
	/* nulls for alignment */
	{-1, -1, -1,},	/* unmapped */
	{1, -1, -1},	/* unmapped */
	{2, 38, 2},
	{3, -1, -1},	/* unmapped */
	{4, 54, 2},
	{5, -1, -1},	/* unmapped */
	{6, 46, 2},
	{7, -1, -1},	/* unmapped */
	{8, 62, 2},
	{9, 108, 1},
	{10, -1, -1},	/* unmapped */
	{11, -1, -1},	/* unmapped */
	{12, -1, -1},	/* unmapped */
	{13, 36, 1},
	{14, -1, -1},	/* unmapped */
	{15, 40, 1},
	{16, -1, -1},	/* unmapped */
	{17, 44, 1},
	{18, -1, -1},	/* unmapped */
	{19, 48, 1},
	{20, -1, -1},	/* unmapped */
	{21, 64, 1},
	{22, -1, -1},	/* unmapped */
	{23, -1, -1},	/* unmapped */
	{24, -1, -1},	/* unmapped */
	{25, -1, -1},	/* unmapped */
	{26, -1, -1},	/* unmapped */
	{27, -1, -1},	/* unmapped */
	{28, -1, -1},	/* unmapped */
	{29, -1, -1},	/* unmapped */
	{30, -1, -1},	/* unmapped */
	{31, -1, -1},	/* unmapped */
	{32, -1, -1},	/* unmapped */
	{33, -1, -1},	/* unmapped */
	{34, -1, -1},	/* unmapped */
	{35, -1, -1},	/* unmapped */
	{36, 42, 4},
	{37, -1, -1},	/* unmapped */
	{38, 58, 4},
	{39, -1, -1},	/* unmapped */
	{40, -1, -1},	/* unmapped */
	{41, -1, -1},	/* unmapped */
	{42, -1, -1},	/* unmapped */
	{43, -1, -1},	/* unmapped */
	{44, -1, -1},	/* unmapped */
	{45, -1, -1},	/* unmapped */
	{46, -1, -1},	/* unmapped */
	{47, -1, -1},	/* unmapped */
	{48, -1, -1},	/* unmapped */
	{49, -1, -1},	/* unmapped */
	{50, -1, -1},	/* unmapped */
	{51, -1, -1},	/* unmapped */
};

/** Pointer to the configured channelisation pair map */
static const struct s1g_ht_chan_pair *s1g_ht_chan_pairs = s1g_ht_chan_pairs_default;

void morse_set_s1g_ht_chan_pairs(const char *cc)
{
	if (cc && strncmp("JP", cc, COUNTRY_CODE_LEN) == 0)
		s1g_ht_chan_pairs = s1g_ht_chan_pairs_jp;
	else
		s1g_ht_chan_pairs = s1g_ht_chan_pairs_default;
}

#define S1G_CHAN_COUNT (52)
#define S1G_CHAN_MIN (1L)
#define S1G_CHAN_MAX (S1G_CHAN_COUNT - 1L)

int morse_s1g_verify_ht_chan_pairs(void)
{
	int c;

	for (c = S1G_CHAN_MIN; c < S1G_CHAN_COUNT; c++) {
		if (s1g_ht_chan_pairs == NULL ||
			s1g_ht_chan_pairs[c].s1g_channel != c) {
			return -1;
		}
	}

	return 0;
}
/* Country code of supported countries, extendable in future
 * mapping to countries in struct ah_class.
 */
static const char *ah_country[] = {
	[MORSE_AU] = "AU",
	[MORSE_CA] = "CA",
	[MORSE_EU] = "EU",
	[MORSE_GB] = "GB",
	[MORSE_IN] = "IN",
	[MORSE_JP] = "JP",
	[MORSE_KR] = "KR",
	[MORSE_NZ] = "NZ",
	[MORSE_SG] = "SG",
	[MORSE_US] = "US",
};

static const struct ah_class us1 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 1,
	.s1g_op_class_idx = 1,
	.global_op_class = 68,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"US", "CA"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(1) |
		S1G_CHAN_ENABLED_FLAG(3) |
		S1G_CHAN_ENABLED_FLAG(5) |
		S1G_CHAN_ENABLED_FLAG(7) |
		S1G_CHAN_ENABLED_FLAG(9) |
		S1G_CHAN_ENABLED_FLAG(11) |
		S1G_CHAN_ENABLED_FLAG(13) |
		S1G_CHAN_ENABLED_FLAG(15) |
		S1G_CHAN_ENABLED_FLAG(17) |
		S1G_CHAN_ENABLED_FLAG(19) |
		S1G_CHAN_ENABLED_FLAG(21) |
		S1G_CHAN_ENABLED_FLAG(23) |
		S1G_CHAN_ENABLED_FLAG(25) |
		S1G_CHAN_ENABLED_FLAG(27) |
		S1G_CHAN_ENABLED_FLAG(29) |
		S1G_CHAN_ENABLED_FLAG(31) |
		S1G_CHAN_ENABLED_FLAG(33) |
		S1G_CHAN_ENABLED_FLAG(35) |
		S1G_CHAN_ENABLED_FLAG(37) |
		S1G_CHAN_ENABLED_FLAG(39) |
		S1G_CHAN_ENABLED_FLAG(41) |
		S1G_CHAN_ENABLED_FLAG(43) |
		S1G_CHAN_ENABLED_FLAG(45) |
		S1G_CHAN_ENABLED_FLAG(47) |
		S1G_CHAN_ENABLED_FLAG(49) |
		S1G_CHAN_ENABLED_FLAG(51)
	),
};

static const struct ah_class us2 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 2,
	.s1g_op_class_idx = 2,
	.global_op_class = 69,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"US", "CA"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(2) |
		S1G_CHAN_ENABLED_FLAG(6) |
		S1G_CHAN_ENABLED_FLAG(10) |
		S1G_CHAN_ENABLED_FLAG(14) |
		S1G_CHAN_ENABLED_FLAG(18) |
		S1G_CHAN_ENABLED_FLAG(22) |
		S1G_CHAN_ENABLED_FLAG(26) |
		S1G_CHAN_ENABLED_FLAG(30) |
		S1G_CHAN_ENABLED_FLAG(34) |
		S1G_CHAN_ENABLED_FLAG(38) |
		S1G_CHAN_ENABLED_FLAG(42) |
		S1G_CHAN_ENABLED_FLAG(46) |
		S1G_CHAN_ENABLED_FLAG(50)
	),
};

static const struct ah_class us3 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 3,
	.s1g_op_class_idx = 3,
	.global_op_class = 70,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"US", "CA"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(8) |
		S1G_CHAN_ENABLED_FLAG(16) |
		S1G_CHAN_ENABLED_FLAG(24) |
		S1G_CHAN_ENABLED_FLAG(32) |
		S1G_CHAN_ENABLED_FLAG(40) |
		S1G_CHAN_ENABLED_FLAG(48)
	),
};

static const struct ah_class us4 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 4,
	.s1g_op_class_idx = 4,
	.global_op_class = 71,
	.s1g_width = IEEE80211_CHAN_8MHZ,
	.cc_list = {"US", "CA"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(12) |
		S1G_CHAN_ENABLED_FLAG(28) |
		S1G_CHAN_ENABLED_FLAG(44)
	),
};

static const struct ah_class eu6 = {
	.s1g_freq_start = 863000,
	.s1g_op_class = 6,
	.s1g_op_class_idx = 6,
	.global_op_class = 66,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"EU", "GB"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(1) |
		S1G_CHAN_ENABLED_FLAG(3) |
		S1G_CHAN_ENABLED_FLAG(5) |
		S1G_CHAN_ENABLED_FLAG(7) |
		S1G_CHAN_ENABLED_FLAG(9)
	),
};

static const struct ah_class eu7 = {
	.s1g_freq_start = 863000,
	.s1g_op_class = 7,
	.s1g_op_class_idx = 7,
	.global_op_class = 67,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"EU", "GB"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(2) |
		S1G_CHAN_ENABLED_FLAG(6)
	),
};

static const struct ah_class jp8 = {
	.s1g_freq_start = 916500,
	.s1g_op_class = 8,
	.s1g_op_class_idx = 8,
	.global_op_class = 73,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"JP"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(9) |
		S1G_CHAN_ENABLED_FLAG(13) |
		S1G_CHAN_ENABLED_FLAG(15) |
		S1G_CHAN_ENABLED_FLAG(17) |
		S1G_CHAN_ENABLED_FLAG(19) |
		S1G_CHAN_ENABLED_FLAG(21)
	),
};

static const struct ah_class jp9 = {
	.s1g_freq_start = 922500,
	.s1g_op_class = 9,
	.s1g_op_class_idx = 9,
	.global_op_class = 64,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"JP"},
	.chans = (
			S1G_CHAN_ENABLED_FLAG(2) |
			S1G_CHAN_ENABLED_FLAG(6)
	),
};

static const struct ah_class jp10 = {
	.s1g_freq_start = 922500,
	.s1g_op_class = 10,
	.s1g_op_class_idx = 10,
	.global_op_class = 64,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"JP"},
	.chans = (
			S1G_CHAN_ENABLED_FLAG(4) |
			S1G_CHAN_ENABLED_FLAG(8)
	),
};

static const struct ah_class jp11 = {
	.s1g_freq_start = 906500,
	.s1g_op_class = 11,
	.s1g_op_class_idx = 11,
	.global_op_class = 65,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"JP"},
	.chans = S1G_CHAN_ENABLED_FLAG(36),
};

static const struct ah_class jp12 = {
	.s1g_freq_start = 906500,
	.s1g_op_class = 12,
	.s1g_op_class_idx = 12,
	.global_op_class = 65,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"JP"},
	.chans = S1G_CHAN_ENABLED_FLAG(38),
};

static const struct ah_class kr14 = {
	.s1g_freq_start = 917500,
	.s1g_op_class = 14,
	.s1g_op_class_idx = 14,
	.global_op_class = 74,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"KR"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(1) |
		S1G_CHAN_ENABLED_FLAG(3) |
		S1G_CHAN_ENABLED_FLAG(5) |
		S1G_CHAN_ENABLED_FLAG(7) |
		S1G_CHAN_ENABLED_FLAG(9) |
		S1G_CHAN_ENABLED_FLAG(11)
	),
};

static const struct ah_class kr15 = {
	.s1g_freq_start = 917500,
	.s1g_op_class = 15,
	.s1g_op_class_idx = 15,
	.global_op_class = 75,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"KR"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(2) |
		S1G_CHAN_ENABLED_FLAG(6) |
		S1G_CHAN_ENABLED_FLAG(10)
	),
};

static const struct ah_class kr16 = {
	.s1g_freq_start = 917500,
	.s1g_op_class = 16,
	.s1g_op_class_idx = 16,
	.global_op_class = 76,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"KR"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(8)
	),
};

static const struct ah_class sg17 = {
	.s1g_freq_start = 863000,
	.s1g_op_class = 17,
	.s1g_op_class_idx = 17,
	.global_op_class = 66,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"SG"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(7) |
		S1G_CHAN_ENABLED_FLAG(9) |
		S1G_CHAN_ENABLED_FLAG(11)
	),
};

static const struct ah_class sg18 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 18,
	.s1g_op_class_idx = 18,
	.global_op_class = 68,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"SG"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(37) |
		S1G_CHAN_ENABLED_FLAG(39) |
		S1G_CHAN_ENABLED_FLAG(41) |
		S1G_CHAN_ENABLED_FLAG(43) |
		S1G_CHAN_ENABLED_FLAG(45)
	),
};

static const struct ah_class sg19 = {
	.s1g_freq_start = 863000,
	.s1g_op_class = 19,
	.s1g_op_class_idx = 19,
	.global_op_class = 67,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"SG"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(10)
	),
};

static const struct ah_class sg20 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 20,
	.s1g_op_class_idx = 20,
	.global_op_class = 69,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"SG"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(38) |
		S1G_CHAN_ENABLED_FLAG(42)
	),
};

static const struct ah_class sg21 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 21,
	.s1g_op_class_idx = 21,
	.global_op_class = 70,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"SG"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(40)
	),
};

static const struct ah_class au22 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 22,
	.s1g_op_class_idx = 22,
	.global_op_class = 68,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"AU"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(27) |
		S1G_CHAN_ENABLED_FLAG(29) |
		S1G_CHAN_ENABLED_FLAG(31) |
		S1G_CHAN_ENABLED_FLAG(33) |
		S1G_CHAN_ENABLED_FLAG(35) |
		S1G_CHAN_ENABLED_FLAG(37) |
		S1G_CHAN_ENABLED_FLAG(39) |
		S1G_CHAN_ENABLED_FLAG(41) |
		S1G_CHAN_ENABLED_FLAG(43) |
		S1G_CHAN_ENABLED_FLAG(45) |
		S1G_CHAN_ENABLED_FLAG(47) |
		S1G_CHAN_ENABLED_FLAG(49) |
		S1G_CHAN_ENABLED_FLAG(51)
	),
};

static const struct ah_class au23 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 23,
	.s1g_op_class_idx = 23,
	.global_op_class = 69,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"AU"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(30) |
		S1G_CHAN_ENABLED_FLAG(34) |
		S1G_CHAN_ENABLED_FLAG(38) |
		S1G_CHAN_ENABLED_FLAG(42) |
		S1G_CHAN_ENABLED_FLAG(46) |
		S1G_CHAN_ENABLED_FLAG(50)
	),
};

static const struct ah_class au24 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 24,
	.s1g_op_class_idx = 24,
	.global_op_class = 70,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"AU"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(32) |
		S1G_CHAN_ENABLED_FLAG(40) |
		S1G_CHAN_ENABLED_FLAG(48)
	),
};

static const struct ah_class au25 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 25,
	.s1g_op_class_idx = 25,
	.global_op_class = 71,
	.s1g_width = IEEE80211_CHAN_8MHZ,
	.cc_list = {"AU"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(44)
	),
};

static const struct ah_class nz26 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 26,
	.s1g_op_class_idx = 26,
	.global_op_class = 68,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"NZ"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(27) |
		S1G_CHAN_ENABLED_FLAG(29) |
		S1G_CHAN_ENABLED_FLAG(31) |
		S1G_CHAN_ENABLED_FLAG(33) |
		S1G_CHAN_ENABLED_FLAG(35) |
		S1G_CHAN_ENABLED_FLAG(37) |
		S1G_CHAN_ENABLED_FLAG(39) |
		S1G_CHAN_ENABLED_FLAG(41) |
		S1G_CHAN_ENABLED_FLAG(43) |
		S1G_CHAN_ENABLED_FLAG(45) |
		S1G_CHAN_ENABLED_FLAG(47) |
		S1G_CHAN_ENABLED_FLAG(49) |
		S1G_CHAN_ENABLED_FLAG(51)
	),
};

static const struct ah_class nz27 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 27,
	.s1g_op_class_idx = 27,
	.global_op_class = 69,
	.s1g_width = IEEE80211_CHAN_2MHZ,
	.cc_list = {"NZ"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(30) |
		S1G_CHAN_ENABLED_FLAG(34) |
		S1G_CHAN_ENABLED_FLAG(38) |
		S1G_CHAN_ENABLED_FLAG(42) |
		S1G_CHAN_ENABLED_FLAG(46) |
		S1G_CHAN_ENABLED_FLAG(50)
	),
};

static const struct ah_class nz28 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 28,
	.s1g_op_class_idx = 28,
	.global_op_class = 70,
	.s1g_width = IEEE80211_CHAN_4MHZ,
	.cc_list = {"NZ"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(32) |
		S1G_CHAN_ENABLED_FLAG(40) |
		S1G_CHAN_ENABLED_FLAG(48)
	),
};

static const struct ah_class nz29 = {
	.s1g_freq_start = 902000,
	.s1g_op_class = 29,
	.s1g_op_class_idx = 29,
	.global_op_class = 71,
	.s1g_width = IEEE80211_CHAN_8MHZ,
	.cc_list = {"NZ"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(44)
	),
};

static const struct ah_class eu30 = {
	.s1g_freq_start = 901400,
	.s1g_op_class = 30,
	.s1g_op_class_idx = 30,
	.global_op_class = 77,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"ZZ"}, /* Setting to ZZ till operating class 77 is supported */
	.chans = (
		S1G_CHAN_ENABLED_FLAG(31)|
		S1G_CHAN_ENABLED_FLAG(33)|
		S1G_CHAN_ENABLED_FLAG(35)
	),
};

static const struct ah_class in31 = {
	.s1g_freq_start = 863000,
	.s1g_op_class = 6,
	.s1g_op_class_idx = 31,
	.global_op_class = 66,
	.s1g_width = IEEE80211_CHAN_1MHZ,
	.cc_list = {"IN"},
	.chans = (
		S1G_CHAN_ENABLED_FLAG(5)|
		S1G_CHAN_ENABLED_FLAG(7)|
		S1G_CHAN_ENABLED_FLAG(9)
	),
};

static const struct ah_class
		*s1g_op_classes[] = {
	NULL,
	&us1,
	&us2,
	&us3,
	&us4,
	NULL,
	&eu6,
	&eu7,
	&jp8,
	&jp9,
	&jp10,
	&jp11,
	&jp12,
	NULL,
	&kr14,
	&kr15,
	&kr16,
	&sg17,
	&sg18,
	&sg19,
	&sg20,
	&sg21,
	&au22,
	&au23,
	&au24,
	&au25,
	&nz26,
	&nz27,
	&nz28,
	&nz29,
	&eu30,
	&in31,
};

const unsigned int S1G_OP_CLASSES_LEN = ARRAY_SIZE(s1g_op_classes);

/*
 * Classify an operating class number as S1G local, global or invalid.
 * Also returns a mapping for local operating classes (if ch_map NOT NULL).
 */
enum s1g_op_class_type morse_s1g_op_class_valid(u8 s1g_op_class, const struct ah_class **class)
{
	/* known global range */
	if ((s1g_op_class >= 64) && (s1g_op_class <= 77)) {
		return OP_CLASS_S1G_GLOBAL;
	}

	/* local index */
	if (s1g_op_class < S1G_OP_CLASSES_LEN) {
		const struct ah_class *ah_class = s1g_op_classes[s1g_op_class];
		if (ah_class) {
			if (class) {
				*class = ah_class;
			}
			return OP_CLASS_S1G_LOCAL;
		}
	}

	return OP_CLASS_INVALID;
}

/**
 * morse_s1g_op_class_has_cc - Check if an S1G class is allowed for a country code
 *
 * @class: S1G class structure
 * @cc:    country code
 *
 * Returns: true if the country code is supported
 */
static bool morse_s1g_op_class_has_cc(const struct ah_class *class, const char *cc)
{
	unsigned int i = 0;

	while (i < ARRAY_SIZE(class->cc_list)) {
		if (cc[0] == class->cc_list[i][0] && cc[1] == class->cc_list[i][1])
			return true;
		i++;
	}

	return false;
}

static const struct ah_class *morse_s1g_op_class_global_search(u8 op_class, char *cc)
{
	unsigned int i;

	for (i = 0; i < S1G_OP_CLASSES_LEN; i++) {
		const struct ah_class *class = s1g_op_classes[i];

		if (class) {
			if (class->global_op_class == op_class) {
				if (cc) {
					if (morse_s1g_op_class_has_cc(class, cc))
						return class;
				} else {
					return class;
				}
			}
		}
	}
	return NULL;
}

/**
 * @brief Get the s1g class corresponding to a global op class, country code and s1g channel.

 * @note Some regions have multiple s1g classes per global class which causes
 *	ambiguity - this function can be used to get around that.
 *
 * @global_op_class: global operating class
 * @cc:              country code
 * @s1g_chan:        S1G channel index
 *
 * Returns: an S1G class structure, or NULL if not found
 */
static const struct ah_class *morse_s1g_op_class_global_search_validate_chan(int global_op_class,
									     char *cc,
									     int s1g_chan)
{
	unsigned int i;

	if (cc == NULL)
		return NULL;

	for (i = 0; i < S1G_OP_CLASSES_LEN; i++) {
		const struct ah_class *class = s1g_op_classes[i];

		if (class) {
			if (class->global_op_class == global_op_class) {
				if (morse_s1g_op_class_has_cc(class, cc) &&
				    (S1G_CHAN_ENABLED_FLAG(s1g_chan) & class->chans))
					return class;
			}
		}
	}
	return NULL;
}

static const struct ah_class *morse_s1g_op_class_global_search_cc(u8 op_class, char *cc)
{
	if (cc == NULL)
		return NULL;

	return morse_s1g_op_class_global_search(op_class, cc);
}

/* Check if a channel is valid for a class.
 * Returns the channel if valid, 0 if not.
 */
static int morse_s1g_op_class_channel_valid(
		const struct ah_class *class, int s1g_chan, bool report_error)
{
	int ret;

	if (!class) {
		wpa_printf(MSG_ERROR, "No S1G class found");
		return MORSE_INVALID_CHANNEL;
	}

	if (s1g_chan < S1G_CHAN_MIN || s1g_chan > S1G_CHAN_MAX) {
		wpa_printf(MSG_ERROR,
				"S1G channel %d not in valid range (min:%ld, max:%ld)",
				s1g_chan, S1G_CHAN_MIN, S1G_CHAN_MAX);
		return MORSE_INVALID_CHANNEL;
	}

	ret = (S1G_CHAN_ENABLED_FLAG(s1g_chan) & class->chans) ? s1g_chan : MORSE_INVALID_CHANNEL;
	if (ret == MORSE_INVALID_CHANNEL && report_error)
		wpa_printf(MSG_ERROR,
			"Channel %d not found for s1g/global operating class %d/%d",
			s1g_chan, class->s1g_op_class, class->global_op_class);

	return ret;
}

int morse_cc_get_sec_channel_offset(int sec_chan_offset, char *cc)
{
	int sec_chan = 0;

	if (strncmp(cc, "JP" , COUNTRY_CODE_LEN) == 0) {
		if (sec_chan_offset == HT_INFO_HT_PARAM_SECONDARY_CHNL_BELOW)
			sec_chan = s1g_pri_1mhz_offset_jp[0];
		else if (sec_chan_offset == HT_INFO_HT_PARAM_SECONDARY_CHNL_ABOVE)
			sec_chan = s1g_pri_1mhz_offset_jp[1];
	} else {
		if (sec_chan_offset == HT_INFO_HT_PARAM_SECONDARY_CHNL_BELOW)
			sec_chan = s1g_pri_1mhz_offset_default[0];
		else if (sec_chan_offset == HT_INFO_HT_PARAM_SECONDARY_CHNL_ABOVE)
			sec_chan = s1g_pri_1mhz_offset_default[1];
	}

	return sec_chan;
}

/* Convert ht channel to s1g channel */
int morse_ht_chan_to_s1g_chan(int ht_chan)
{
	unsigned i;

	if (s1g_ht_chan_pairs == NULL ||
		ht_chan < 0)
		return MORSE_S1G_RETURN_ERROR;

	for (i = S1G_CHAN_MIN; i < S1G_CHAN_COUNT; i++)
		if (s1g_ht_chan_pairs[i].ht_channel == ht_chan)
			return s1g_ht_chan_pairs[i].s1g_channel;

	return MORSE_S1G_RETURN_ERROR;
}

/* Convert ht frequency to s1g channel */
int morse_ht_freq_to_s1g_chan(int ht_freq)
{
		if (ht_freq < START_FREQ_5GHZ || ht_freq > END_FREQ_5GHZ)
			return MORSE_S1G_RETURN_ERROR;

		return morse_ht_chan_to_s1g_chan(HT_FREQ_TO_HT_CHAN(ht_freq));
}

int morse_s1g_chan_to_ht20_prim_chan(int s1g_op_channel, int s1g_prim_1MHz_channel, char *cc)
{
	int ht_chan;
	int offset;

	if (strncmp(cc, "JP", COUNTRY_CODE_LEN) == 0) {
		ht_chan = morse_s1g_chan_to_ht_chan(s1g_prim_1MHz_channel);
		offset = morse_ht_chan_offset_jp(s1g_op_channel, s1g_prim_1MHz_channel, 0);

		if (ht_chan < 0 || offset < 0)
			return MORSE_S1G_RETURN_ERROR;
		else
			return ht_chan + offset;
	}
	else
		return morse_s1g_chan_to_ht_chan(s1g_prim_1MHz_channel);
}

/* Get ht channel offset value for Japan from s1g/ht channel */
int morse_ht_chan_offset_jp(int chan, int primary_chan, int ht)
{
	unsigned int i;
	int ht20mhz_offset = 0;

	/* Get ht20 channel offset value from ht channel */
	if (ht) {
		if ((chan > MORSE_JP_HT20_NON_OVERLAP_CHAN_START) &&
			(chan <= MORSE_JP_HT20_NON_OVERLAP_CHAN_END))
			return MORSE_JP_HT20_NON_OVERLAP_CHAN_OFFSET;
		else
			return ht20mhz_offset;
	}

	/* Get ht20 channel offset value from S1G channel */
	if (s1g_ht_chan_pairs == NULL ||
	   (chan < S1G_CHAN_MIN ||
	    chan > S1G_CHAN_MAX))
		return MORSE_INVALID_CHANNEL;

	/* In the JP regulatory, some primary channels have duplicate
	 * entries so to get the correct 5g value, the op chan
	 * must be considered.
	 */
	for (i = 0; i < (int)ARRAY_SIZE(s1g_overlap_chan_jp); i++) {
		if (chan == s1g_overlap_chan_jp[i] &&
		    primary_chan != MORSE_JP_S1G_NON_OVERLAP_CHAN) {
			ht20mhz_offset = MORSE_JP_HT20_NON_OVERLAP_CHAN_OFFSET;
			break;
		} else {
			ht20mhz_offset = 0;
		}
	}

	return ht20mhz_offset;
}

/* Convert s1g channel to ht channel */
int morse_s1g_chan_to_ht_chan(int s1g_chan)
{
	if (s1g_ht_chan_pairs == NULL ||
		(s1g_chan < S1G_CHAN_MIN || s1g_chan > S1G_CHAN_MAX))
		return MORSE_S1G_RETURN_ERROR;

	return s1g_ht_chan_pairs[s1g_chan].ht_channel;
}

/* Convert s1g channel to bandwidth */
int morse_s1g_chan_to_bw(int s1g_chan)
{
	if (s1g_ht_chan_pairs == NULL ||
		(s1g_chan < S1G_CHAN_MIN || s1g_chan > S1G_CHAN_MAX))
		return MORSE_S1G_RETURN_ERROR;

	return s1g_ht_chan_pairs[s1g_chan].bw;
}

/*
 * Convert an operating class to a country code.
 * This function is used only for testing.
 * If the operating class has multiple country codes, the first configured country code is returned.
 */
int morse_s1g_op_class_to_country(u8 s1g_op_class, char *cc)
{
	const struct ah_class *class;

	if (!cc)
		return MORSE_S1G_RETURN_ERROR;

	if (morse_s1g_op_class_valid(s1g_op_class, &class) != OP_CLASS_S1G_LOCAL)
		return MORSE_S1G_RETURN_ERROR;

	if (!class)
		return MORSE_S1G_RETURN_ERROR;

	cc[0] = class->cc_list[0][0];
	cc[1] = class->cc_list[0][1];

	return MORSE_SUCCESS;
}

int morse_s1g_country_to_global_op_class(char *cc)
{
	unsigned int i;

	if (!cc)
		return MORSE_S1G_RETURN_ERROR;

	for (i = 0; i < S1G_OP_CLASSES_LEN; i++) {
		const struct ah_class *class = s1g_op_classes[i];

		if (!class)
			continue;

		if (morse_s1g_op_class_has_cc(class, cc))
			return class->global_op_class;
	}

	return MORSE_S1G_RETURN_ERROR;
}

/* Convert an operating class to channel width */
int morse_s1g_op_class_to_ch_width(u8 s1g_op_class)
{
	const struct ah_class *class;

	switch (morse_s1g_op_class_valid(s1g_op_class, &class)) {
		case OP_CLASS_S1G_LOCAL:
			return class->s1g_width;
		case OP_CLASS_S1G_GLOBAL:
			class = morse_s1g_op_class_global_search(s1g_op_class, NULL);
			if (class)
				return class->s1g_width;
			break;
		default:
			break;
	}

	return MORSE_S1G_RETURN_ERROR;
}

/* Convert an operating class and s1g channel to frequency (kHz) */
int morse_s1g_op_class_chan_to_freq(u8 s1g_op_class, int s1g_chan)
{
	const struct ah_class *class;

	if (morse_s1g_op_class_valid(s1g_op_class, &class) != OP_CLASS_S1G_LOCAL)
		return MORSE_S1G_RETURN_ERROR;
	if (!class)
		return MORSE_S1G_RETURN_ERROR;
	if (morse_s1g_op_class_channel_valid(class, s1g_chan, true) == MORSE_INVALID_CHANNEL)
		return MORSE_S1G_RETURN_ERROR;

	return class->s1g_freq_start + (s1g_chan * S1G_CHAN_SEP_KHZ);
}

/* Convert ht channel and s1g operating class to s1g frequency (kHz) */
int morse_s1g_op_class_ht_chan_to_s1g_freq(u8 s1g_op_class, int ht_chan)
{
	int s1g_chan = morse_ht_chan_to_s1g_chan(ht_chan);

	if (s1g_chan == MORSE_S1G_RETURN_ERROR)
		return MORSE_S1G_RETURN_ERROR;

	return morse_s1g_op_class_chan_to_freq(s1g_op_class, s1g_chan);
}

int morse_s1g_op_class_ht_freq_to_s1g_freq(u8 s1g_op_class, int ht_freq)
{
	unsigned int s1g_chan;

	if (s1g_ht_chan_pairs == NULL ||
			ht_freq < 0)
		return MORSE_S1G_RETURN_ERROR;

	/* for all S1G channels */
	for (s1g_chan = S1G_CHAN_MIN; s1g_chan < S1G_CHAN_COUNT; s1g_chan++) {
		/* check if the frequency for the mapped HT chan matches */
		if (ieee80211_channel_to_frequency(s1g_ht_chan_pairs[s1g_chan].ht_channel,
				NL80211_BAND_5GHZ) == ht_freq) {
			/* found matching s1g chan, determine frequency */
			return morse_s1g_op_class_chan_to_freq(s1g_op_class, s1g_chan);
		}
	}

	return MORSE_S1G_RETURN_ERROR;
}

/* Convert a country and ht frequency into a s1g frequency (kHz) */
int morse_cc_ht_freq_to_s1g_freq(char *cc, int ht_freq)
{
	unsigned int s1g_chan;
	unsigned int i;
	u8 op_class = 0;
	const struct ah_class *class;

	if (s1g_ht_chan_pairs == NULL)
		return MORSE_S1G_RETURN_ERROR;


	/* for all S1G channels */
	for (s1g_chan = S1G_CHAN_MIN; s1g_chan < S1G_CHAN_COUNT; s1g_chan++) {
		/* check if the frequency for the mapped HT chan matches */
		if (ieee80211_channel_to_frequency(s1g_ht_chan_pairs[s1g_chan].ht_channel,
				NL80211_BAND_5GHZ) == ht_freq) {
			/* found matching s1g chan */
			break;
		}
	}
	/* no matching s1g chan */
	if (s1g_chan >= S1G_CHAN_COUNT)
		return MORSE_S1G_RETURN_ERROR;

	/* find matching class */
	for (i = 0; i < S1G_OP_CLASSES_LEN; i++) {
		class = s1g_op_classes[i];
		if (!class)
			continue;

		/* check channel valid for class, matching BW, matching country */
		if ((class->chans & S1G_CHAN_ENABLED_FLAG(s1g_chan)) &&
				class->s1g_width == morse_s1g_chan_to_bw(s1g_chan) &&
				morse_s1g_op_class_has_cc(class, cc)) {
			op_class = class->s1g_op_class;
			break;
		}
	}
	if (!op_class)
		return MORSE_S1G_RETURN_ERROR;

	return morse_s1g_op_class_chan_to_freq(op_class, s1g_chan);
}

/* Return the first valid channel from an s1g operating class */
int morse_s1g_op_class_first_chan(u8 s1g_op_class)
{
	int i;
	const struct ah_class *class;

	if (morse_s1g_op_class_valid(s1g_op_class, &class) != OP_CLASS_S1G_LOCAL)
		return MORSE_S1G_RETURN_ERROR;

	if (!class)
		return MORSE_S1G_RETURN_ERROR;

	for (i = S1G_CHAN_MIN; i < S1G_CHAN_COUNT; i++) {
		if (class->chans & S1G_CHAN_ENABLED_FLAG(i))
			return i;
	}

	return MORSE_S1G_RETURN_ERROR;
}

static int morse_ht_chan_ht40(int ht_chan)
{
	if (((ht_chan + 2) % 8 == 0)
			&& (IN_LOWER_HT40_RANGE(ht_chan)
				|| IN_MIDDLE_HT40_RANGE(ht_chan))) {
		return ht_chan;
	}
	if (((ht_chan + 1) % 8 == 0)
			&& IN_HIGHER_HT40_RANGE(ht_chan)) {
		return ht_chan;
	}
	return MORSE_SUCCESS;
}

static int morse_ht_chan_vht80(int ht_chan)
{
	unsigned i;

	for (i = 0; i < num_vht80_chans; i++) {
		if (ht_chan == vht80_chans[i])
			return ht_chan;
	}
	return MORSE_SUCCESS;
}

static int morse_ht_chan_vht160(int ht_chan)
{
	unsigned i;

	for (i = 0; i < num_vht160_chans; i++) {
		if (ht_chan == vht160_chans[i])
			return ht_chan;
	}
	return MORSE_SUCCESS;
}

/* Returns the center channel, taking into account VHT channel offsets */
int morse_ht_chan_to_ht_chan_center(struct hostapd_config *conf, int ht_chan)
{
	int ht_center_chan = 0;

	if (conf->ieee80211ac) {
		switch (conf->vht_oper_chwidth) {
		case CHANWIDTH_USE_HT:
			break;
		case CHANWIDTH_80MHZ:
			if (morse_ht_chan_vht80(ht_chan))
				return ht_chan;
			ht_center_chan = ht_chan -
					vht_80_pri_1mhz_offset[conf->s1g_prim_1mhz_chan_index];
			if (morse_ht_chan_vht80(ht_center_chan))
				return ht_center_chan;
			return MORSE_S1G_RETURN_ERROR;
		case CHANWIDTH_160MHZ:
			if (morse_ht_chan_vht160(ht_chan))
				return ht_chan;
			ht_center_chan = ht_chan -
					vht_160_pri_1mhz_offset[conf->s1g_prim_1mhz_chan_index];
			if (morse_ht_chan_vht160(ht_center_chan))
				return ht_center_chan;
			return MORSE_S1G_RETURN_ERROR;
		default:
			break;
		}
	}

	/* HT40 */
	if (conf->secondary_channel) {
		ht_center_chan = ht_chan - ht_40_pri_1mhz_offset[conf->s1g_prim_1mhz_chan_index];
		if (morse_ht_chan_ht40(ht_center_chan))
			return ht_center_chan;
	}

	return ht_chan;
}

/* Returns the ht channel, taking into account VHT channel offsets */
int morse_ht_center_chan_to_ht_chan(struct hostapd_config *conf, int ht_chan)
{
	if (conf->ieee80211ac) {
		switch (conf->vht_oper_chwidth) {
		case CHANWIDTH_USE_HT:
			break;
		case CHANWIDTH_80MHZ:
			if (morse_ht_chan_vht80(ht_chan))
				return ht_chan +
					vht_80_pri_1mhz_offset[conf->s1g_prim_1mhz_chan_index];
			break;
		case CHANWIDTH_160MHZ:
			if (morse_ht_chan_vht160(ht_chan))
				return ht_chan +
					vht_160_pri_1mhz_offset[conf->s1g_prim_1mhz_chan_index];
			break;
		default:
			break;
		}
	}

	/* HT40 */
	if (conf->secondary_channel) {
		if (morse_ht_chan_ht40(ht_chan))
			return ht_chan + ht_40_pri_1mhz_offset[conf->s1g_prim_1mhz_chan_index];
	}

	return ht_chan;
}

int morse_cc_get_primary_s1g_channel(int op_bw_mhz, int pr_bw_mhz,
					int s1g_op_chan, int pr_1mhz_chan_idx, char *cc)
{
	if (strncmp(cc, "JP", COUNTRY_CODE_LEN) == 0)
		return morse_calculate_primary_s1g_channel_jp(op_bw_mhz, pr_bw_mhz,
							s1g_op_chan, pr_1mhz_chan_idx);
	else
		return morse_calculate_primary_s1g_channel(op_bw_mhz, pr_bw_mhz,
							s1g_op_chan, pr_1mhz_chan_idx);
}

int morse_calculate_primary_s1g_channel_jp(int op_bw_mhz, int pr_bw_mhz, int s1g_op_chan,
						int pr_1mhz_chan_idx)
{
	int offset;

	switch (op_bw_mhz) {
	case 1:
		return s1g_op_chan;
	case 2:
		if (pr_bw_mhz == 1) {
			offset = pr_1mhz_chan_idx ? 13 : 11;
			return (s1g_op_chan + offset);
		} else {
			return s1g_op_chan;
		}
	case 4:
		if (pr_bw_mhz == 1) {
			offset = pr_1mhz_chan_idx == 0 ? 23 :
				pr_1mhz_chan_idx == 1 ? 21 :
				pr_1mhz_chan_idx == 2 ? 19 :
				pr_1mhz_chan_idx == 3 ? 17 : -1;
		} else {
			offset = ((pr_1mhz_chan_idx == 0) || (pr_1mhz_chan_idx == 1)) ? 34 :
				((pr_1mhz_chan_idx == 2) || (pr_1mhz_chan_idx == 3)) ? 30 : -1;
		}
		return (offset > 0) ? (s1g_op_chan - offset) : -EINVAL;
	default:
		return -ENOENT;
	}
}

/* Get primary s1g channel */
int morse_calculate_primary_s1g_channel(int op_bw_mhz, int pr_bw_mhz, int s1g_op_chan,
					int pr_1mhz_chan_idx)
{
	int chan_loc = (pr_1mhz_chan_idx % 2);

	switch (op_bw_mhz) {
	case 1:
		return s1g_op_chan;
	case 2:
		if (pr_bw_mhz == 1)
			return s1g_op_chan + ((chan_loc == 0) ? -1 : 1);
		else
			return s1g_op_chan;
	case 4:
		if (pr_bw_mhz == 1)
			return ((2 * pr_1mhz_chan_idx) - 3) + s1g_op_chan;
		else
			return ((pr_1mhz_chan_idx / 2) * 4) - 2 + s1g_op_chan;
	case 8:
		if (pr_bw_mhz == 1)
			return ((2 * pr_1mhz_chan_idx) - 7) + s1g_op_chan;
		else
			return ((pr_1mhz_chan_idx / 2) * 4) - 6 + s1g_op_chan;
	}

	return MORSE_S1G_RETURN_ERROR;
}

/* Derive operating class struct for the country based on bandwidth and channel */
const struct ah_class *morse_s1g_ch_to_op_class(u8 s1g_bw, char *cc, int s1g_chan)
{
	unsigned int i;

	for (i = 0; i < S1G_OP_CLASSES_LEN; i++) {
		const struct ah_class *class = s1g_op_classes[i];

		if (class && class->s1g_width == s1g_bw) {
			if (!cc)
				return class;
			if (morse_s1g_op_class_has_cc(class, cc)) {
				if (morse_s1g_op_class_channel_valid(class,
							s1g_chan, true) == s1g_chan)
					return class;
			}
		}
	}

	return NULL;
}

/**
 * Check if the primary 1MHz channel index is in correct range for a particular frequency.
 *
 * @param s1g_bw		The S1G bandwidth
 * @param s1g_1mhz_prim_index	Primary 1MHz channel index (0-7 for 8MHz BW, 0-3 for 4MHz,
 *				0-1 for 2MHz, and 0 for 1MHz)
 *
 * @returns true if the s1g_1mhz_prim_index value provided is correct else false
 */
static bool morse_check_valid_s1g_prim_1Mhz_chan_index(u8 s1g_bw, u8 s1g_1mhz_prim_index)
{
	bool valid_s1g_prim_index = (s1g_1mhz_prim_index <= (s1g_bw -1));

	if (!valid_s1g_prim_index)
		wpa_printf(MSG_ERROR, "Not a valid s1g prim index for bw %d",
					s1g_bw);

	return valid_s1g_prim_index;
}

static const char *cc_mismatch_msg = "country code %c%c mismatch against S1G operating class %d";

/*
 * Verify operating class and country code (no channel).
 * Returns S1G local operating class if valid combination, negative if invalid.
 */
int morse_s1g_verify_op_class_country(u8 s1g_op_class, char *cc, u8 s1g_1mhz_prim_index)
{
	const struct ah_class *class;

	/* check if operating class is global or local */
	switch (morse_s1g_op_class_valid(s1g_op_class, &class)) {
		/* global */
		case OP_CLASS_S1G_GLOBAL:
			/* search for matching op class matching global and country code */
			class = morse_s1g_op_class_global_search_cc(s1g_op_class, cc);

			if (class &&
				(morse_check_valid_s1g_prim_1Mhz_chan_index(
						class->s1g_width, s1g_1mhz_prim_index))) {
				/* valid */
				return class->s1g_op_class_idx;
			}
			break;
		case OP_CLASS_S1G_LOCAL:
			/* check country code matches */
			if (cc && class) {
				/* if country code doesn't match */
				if (!morse_s1g_op_class_has_cc(class, cc)) {
					/* and there is some value set */
					if (cc[0] != 0) {
						wpa_printf(MSG_ERROR, cc_mismatch_msg,
							cc[0], cc[1], s1g_op_class);
						return MORSE_S1G_RETURN_ERROR;
					}
				}
			}

			if (class && morse_check_valid_s1g_prim_1Mhz_chan_index(class->s1g_width,
									s1g_1mhz_prim_index))
				return class->s1g_op_class_idx;
			break;
		/* invalid */
		default:
			break;
	}
	return MORSE_S1G_RETURN_ERROR;
}

/*
 * Verify operating class, country code and channel.
 * Returns S1G local operating class if valid combination, negative if invalid.
 */
int morse_s1g_verify_op_class_country_channel(u8 s1g_op_class, char *cc, int s1g_chan,
						u8 s1g_1mhz_prim_index)
{
	const struct ah_class *class;

	/* check if operating class is global or local */
	switch (morse_s1g_op_class_valid(s1g_op_class, &class)) {
	/* global */
	case OP_CLASS_S1G_GLOBAL:
		/* search for matching op class - some channels share a global
		 * op class but have different s1g operating classes
		 */
		class = morse_s1g_op_class_global_search_validate_chan(s1g_op_class, cc, s1g_chan);

		if (class) {
			/* lookup channel */
			if (morse_s1g_op_class_channel_valid(class, s1g_chan, true) !=
								MORSE_INVALID_CHANNEL &&
					morse_check_valid_s1g_prim_1Mhz_chan_index(class->s1g_width,
									s1g_1mhz_prim_index))
				return class->s1g_op_class_idx;
		}
		break;
	case OP_CLASS_S1G_LOCAL:
		/* check country code matches */
		if (cc && class) {
			/* if country code doesn't match */
			if (!morse_s1g_op_class_has_cc(class, cc)) {
				/* and there is some value set */
				if (cc[0] != 0) {
					wpa_printf(MSG_ERROR, cc_mismatch_msg,
							cc[0], cc[1], s1g_op_class);
					return MORSE_S1G_RETURN_ERROR;
				}
			}
		}

		/* look up channel */
		if (morse_s1g_op_class_channel_valid(class, s1g_chan, true) !=
							MORSE_INVALID_CHANNEL &&
			morse_check_valid_s1g_prim_1Mhz_chan_index(class->s1g_width,
							s1g_1mhz_prim_index))
			return class->s1g_op_class_idx;
		break;
	/* invalid */
	default:
		wpa_printf(MSG_ERROR,
				"Unknown S1G operating class %d", s1g_op_class);
		break;
	}

	return MORSE_S1G_RETURN_ERROR;
}

int morse_s1g_get_start_freq_for_country(char *cc, int freq, int bw)
{
	int start_freq = 0;
	unsigned int region;

	for (region = 0; region < sizeof(ah_country) / sizeof(char *); region++) {
		if (strncmp(ah_country[region], cc, COUNTRY_CODE_LEN) == 0) {
			switch (region) {
			case MORSE_AU:
			case MORSE_CA:
			case MORSE_NZ:
			case MORSE_US:
				start_freq = 902000;
				break;
			case MORSE_EU:
			case MORSE_GB:
				if (freq > 901400)
					start_freq = 901400;
				else
					start_freq = 863000;
				break;
			case MORSE_IN:
				start_freq = 863000;
				break;
			case MORSE_JP:
				if ((freq % 1000) == 500) {
					/* We are on a 500kHz centre */
					if (bw < 4)
						start_freq = 922500;
					else
						start_freq = 906500;
				} else {
					start_freq = 916500;
				}
				break;
			case MORSE_KR:
				start_freq = 917500;
				break;
			case MORSE_SG:
				if (freq > 902000)
					start_freq = 902000;
				else
					start_freq = 863000;
				break;
			case REGION_UNSET:
			default:
				break;
			}
		}
	}

	return start_freq;
}

#ifdef CONFIG_IEEE80211AH
int morse_s1g_get_primary_channel(struct hostapd_config *conf, int bw)
{
	int ht_center_chan = morse_ht_chan_to_ht_chan_center(conf, conf->channel);
	int s1g_op_chan = morse_ht_chan_to_s1g_chan(ht_center_chan);
	int op_bw = morse_s1g_chan_to_bw(s1g_op_chan);

	/* Can only retrieve the 1Mhz or 2MHz primary channel variant */
	if (bw < 1 || bw > 2)
		return MORSE_INVALID_CHANNEL;

	if (conf->s1g_prim_1mhz_chan_index >= op_bw)
		return MORSE_INVALID_CHANNEL;

	return morse_cc_get_primary_s1g_channel(op_bw,
						bw,
						s1g_op_chan,
						conf->s1g_prim_1mhz_chan_index,
						conf->op_country);
}

/* Validate ht centre channel with index and returns corresponding ht channel */
int morse_validate_ht_channel_with_idx(u8 s1g_op_class, int ht_center_channel,
				int *oper_chwidth, int s1g_prim_1mhz_chan_index,
				struct hostapd_config *conf)
{
	int errors = 0;

	/* Update the operating class to S1G regional specific */
	conf->s1g_op_class = s1g_op_class;

	/* Update primary channel index to common configuration*/
	conf->s1g_prim_1mhz_chan_index = s1g_prim_1mhz_chan_index;

	/* Enable 80211n mode */
	conf->ieee80211n = 1;

	/* Enable 80211ac mode (to allow VHT capabilties mapped to S1G). */
	conf->ieee80211ac = 1;

	*oper_chwidth = morse_s1g_op_class_to_ch_width(conf->s1g_op_class);
	if (*oper_chwidth < 0)
		return MORSE_S1G_RETURN_ERROR;

	if (*oper_chwidth != 1)
		conf->secondary_channel = (conf->s1g_prim_1mhz_chan_index % 2 ? -1 : 1);

	switch (*oper_chwidth) {
	case IEEE80211_CHAN_1MHZ:
		conf->s1g_prim_chwidth = S1G_PRIM_CHWIDTH_1;
		conf->vht_oper_chwidth = 0;
		break;
	case IEEE80211_CHAN_2MHZ:
		conf->s1g_prim_chwidth = S1G_PRIM_CHWIDTH_1;
		conf->ht_capab |= HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET;
		conf->vht_oper_chwidth = 0;
		break;
	case IEEE80211_CHAN_4MHZ:
		conf->s1g_prim_chwidth = S1G_PRIM_CHWIDTH_2;
		conf->ht_capab |= HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET;
		/* Based on the WLAN channel allocation, the HT control
		 * channel is offset by 6 from the VHT80 channel index
		 */
		conf->vht_oper_chwidth = 1;
		conf->vht_oper_centr_freq_seg0_idx = ht_center_channel;
		if (conf->s1g_capab & S1G_CAP0_SGI_4MHZ)
			conf->vht_capab |= VHT_CAP_SHORT_GI_80;
		break;
	case IEEE80211_CHAN_8MHZ:
		conf->s1g_prim_chwidth = S1G_PRIM_CHWIDTH_2;
		conf->ht_capab |= HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET;
		/* valid 8MHz channel - map to 160MHz */
		wpa_printf(MSG_INFO,
			"Automatically configuring VHT due to 160MHz channel selection");
		conf->vht_oper_chwidth = 2;
		/* Based on the WLAN channel allocation, the HT control
		 * channel is offset by 14 from the VHT160 channel index
		 */
		conf->vht_oper_centr_freq_seg0_idx = ht_center_channel;
		if (conf->s1g_capab & S1G_CAP0_SGI_8MHZ)
			conf->vht_capab |= VHT_CAP_SHORT_GI_80 | VHT_CAP_SHORT_GI_160;
		break;
	default:
		errors++;
		break;
	}

	if (errors)
		return MORSE_S1G_RETURN_ERROR;

	return morse_ht_center_chan_to_ht_chan(conf, ht_center_channel);
}

/* Convert s1g frequency to s1g channel */
static int morse_s1g_freq_to_s1g_channel(int s1g_frequency, int s1g_start_frequency,
					const struct ah_class *class)
{
	int channel;

	channel = (s1g_frequency - s1g_start_frequency) / S1G_CHAN_SEP_KHZ;

	return morse_s1g_op_class_channel_valid(class, channel, false);
}

/* Convert s1g frequency to ht frequency */
int morse_s1g_freq_to_ht_freq(int s1g_frequency, const struct ah_class *class, bool report_error)
{
	int s1g_channel, ht_channel;
	int s1g_start_frequency = class->s1g_freq_start;

	/* Get S1G channel from S1G frequency */
	s1g_channel = morse_s1g_freq_to_s1g_channel(s1g_frequency, s1g_start_frequency, class);
	if (s1g_channel < 0) {
		if (report_error)
			wpa_printf(MSG_ERROR, "Failed s1g freq to s1g channel conversion");
		return MORSE_INVALID_CHANNEL;
	}

	/* Convert S1G channel to ht channel */
	ht_channel = morse_s1g_chan_to_ht_chan(s1g_channel);
	if (ht_channel < 0) {
		if (report_error)
			wpa_printf(MSG_ERROR, "Failed s1g channel to ht channel conversion");
		return MORSE_INVALID_CHANNEL;
	}

	/* Get HT frequency */
	return ieee80211_channel_to_frequency(ht_channel, NL80211_BAND_5GHZ);
}

/* Get the HT frequency for an S1G frequency and country code */
int morse_s1g_freq_and_cc_to_ht_freq(int s1g_frequency, const char *cc)
{
	unsigned int i;

	for (i = 0; i < S1G_OP_CLASSES_LEN; i++) {
		const struct ah_class *class = s1g_op_classes[i];

		if (class && morse_s1g_op_class_has_cc(class, cc)) {
			int ht_freq;

			ht_freq = morse_s1g_freq_to_ht_freq(s1g_frequency, class, false);
			if (ht_freq > 0)
				return ht_freq;
		}
	}

	return MORSE_S1G_RETURN_ERROR;
}

/**
 * Find the 5G-mapped centre frequency of a primary channel
 *
 * @param s1g_op_bw S1G operating bandwidth (MHz)
 * @param s1g_prim_bw S1G primary bandwidth (MHz)
 * @param s1g_op_chan Operating channel
 * @param s1g_prim_1mhz_chan_index Primary S1G 1MHz channel index
 * @param country Country code
 *
 * @return 5G-mapped centre frequency of primary channel (KHz)
 */
static int morse_s1g_chan_get_primary_chan_freq_ht(int s1g_op_bw, int s1g_prim_bw, int s1g_op_chan,
					    int s1g_prim_1mhz_chan_index, char* country)
{
	int s1g_prim_chan;
	int ht_prim_chan;
	int ht_prim_freq;

	s1g_prim_chan = morse_cc_get_primary_s1g_channel(s1g_op_bw, s1g_prim_bw, s1g_op_chan,
							     s1g_prim_1mhz_chan_index, country);
	if (s1g_prim_chan < 0)
		return MORSE_S1G_RETURN_ERROR;

	ht_prim_chan = morse_s1g_chan_to_ht20_prim_chan(s1g_op_chan, s1g_prim_chan, country);
	if (ht_prim_chan < 0)
		return MORSE_S1G_RETURN_ERROR;

	ht_prim_freq = ieee80211_channel_to_frequency(ht_prim_chan, NL80211_BAND_5GHZ);
	return ht_prim_freq ? ht_prim_freq : MORSE_S1G_RETURN_ERROR;
}

bool morse_s1g_is_chan_conf_primary_disabled(struct hostapd_config *conf,
					     struct hostapd_hw_modes *mode, int s1g_op_chan)
{
	int ht20_prim_freq;
	int ht20_sec_freq;
	int s1g_op_bw;
	struct hostapd_channel_data *chan;

	/* Primary 1MHz channel index taken from config means that this function is not capable of
	 * validating operating channel with a different bandwidth to the configuration, as the
	 * 1MHz index will be meaningless and potentially invalid.
	 */
	char* country = conf->op_country;
	int s1g_prim_1mhz_chan_index = conf->s1g_prim_1mhz_chan_index;

	s1g_op_bw = morse_s1g_chan_to_bw(s1g_op_chan);
	if (s1g_op_bw == MORSE_S1G_RETURN_ERROR)
		return true;

	/* HT mapping of S1G 1MHz primary channel, not S1G primary channel. This is the HT primary
	 * channel in the 5G mapping
	 */
	ht20_prim_freq = morse_s1g_chan_get_primary_chan_freq_ht(s1g_op_bw, IEEE80211_CHAN_1MHZ,
								 s1g_op_chan,
								 s1g_prim_1mhz_chan_index,
								 country);
	if (ht20_prim_freq == MORSE_S1G_RETURN_ERROR)
		return true;

	chan = hw_mode_get_channel(mode, ht20_prim_freq, NULL);
	if (!chan || (chan->flag & HOSTAPD_CHAN_DISABLED))
		return true;

	/* 2, 4 and 8MHz channels have an HT 20MHz secondary channel, which maps to the S1G 1MHz
	 * secondary channel. Validate it if S1G primary channel width is 2MHz.
	 */
	if (conf->secondary_channel && conf->s1g_prim_chwidth == S1G_PRIM_CHWIDTH_2) {
		ht20_sec_freq = ht20_prim_freq + conf->secondary_channel * 20;
		chan = hw_mode_get_channel(mode, ht20_sec_freq, NULL);
		if (!chan || (chan->flag & HOSTAPD_CHAN_DISABLED))
			return true;
	}
	return false;
}

/* Validate ECSA primary channel with the current primary and operating channels */
int morse_s1g_csa_validate_primary_chan(struct hostapd_iface *iface, int csa_ht20_frequency)
{
	int oper_chwidth;
	int prim_chwidth;
	int current_ht20_frequency;
	int ht_center_chan = morse_ht_chan_to_ht_chan_center(iface->conf, iface->conf->channel);
	int current_s1g_chan_center = morse_ht_chan_to_s1g_chan(ht_center_chan);

	oper_chwidth = morse_s1g_op_class_to_ch_width(iface->conf->s1g_op_class);
	if (oper_chwidth < 0) {
		wpa_printf(MSG_ERROR,
			"%s: error determining S1G operating channel width from operating class (%d)",
			__func__, iface->conf->s1g_op_class);
		return MORSE_S1G_RETURN_ERROR;
	}

	switch (iface->conf->s1g_prim_chwidth) {
	case S1G_PRIM_CHWIDTH_1:
		prim_chwidth = 1;
		break;
	case S1G_PRIM_CHWIDTH_2:
		prim_chwidth = 2;
		break;
	default:
		wpa_printf(MSG_ERROR, "Error found in config, invalid prim_chwidth");
		return MORSE_S1G_RETURN_ERROR;
	}

	if (prim_chwidth > oper_chwidth) {
		wpa_printf(MSG_ERROR, "Invalid primary channel width");
		return MORSE_S1G_RETURN_ERROR;
	}

	current_ht20_frequency = morse_s1g_chan_get_primary_chan_freq_ht(
					oper_chwidth,
					IEEE80211_CHAN_1MHZ,
					current_s1g_chan_center,
					iface->conf->s1g_prim_1mhz_chan_index,
					iface->conf->op_country
				);
	if (current_ht20_frequency == MORSE_S1G_RETURN_ERROR)
		return MORSE_S1G_RETURN_ERROR;

	if (csa_ht20_frequency == current_ht20_frequency) {
		wpa_printf(MSG_ERROR,
			   "ECSA: Switching to same primary 1Mhz channel not allowed (freq: %d)",
			   current_ht20_frequency);
		return MORSE_S1G_RETURN_ERROR;
	}

	return MORSE_SUCCESS;
}

/* Validate ECSA parameters and do necessary conversions */
int morse_s1g_validate_csa_params(struct hostapd_iface *iface,	struct csa_settings *settings)
{
	const struct ah_class *class;
	const struct ah_class *prim_class;
	int s1g_prim_channel_index = -1;
	int s1g_op_channel;
	int s1g_prim_channel;
	int s1g_prim_1MHz_channel;
	int s1g_start_freq;
	int ht20_mapped_channel;

	int s1g_bandwidth = settings->freq_params.bandwidth;
	int s1g_prim_bw = settings->freq_params.prim_bandwidth;
	int s1g_prim_frequency = settings->freq_params.freq;
	int s1g_center_frequency = settings->freq_params.center_freq1;

	if (settings->cs_count < 2) {
		wpa_printf(MSG_ERROR, "cs_count must be >= 2");
		return MORSE_S1G_RETURN_ERROR;
	}

	/* Increase the channel switch count by 1 to match the number of Beacons going out
	 * with ECSA IE. The mac80211 function ieee80211_beacon_get first decrements
	 * the channel switch count and then uses it in beacon. So if the user gives cs_count
	 * of 2, actually it sends only 1 Beacon with ECSA IE.
	 */
	if (settings->cs_count < UINT8_MAX)
		settings->cs_count += 1;

	/* Get the start frequency for regdomain based on operating bandwdith*/
	s1g_start_freq = morse_s1g_get_start_freq_for_country(
				iface->conf->op_country, s1g_center_frequency, s1g_bandwidth);
	s1g_op_channel = (s1g_center_frequency - s1g_start_freq) / S1G_CHAN_SEP_KHZ;

	/* Get the start frequency for regdomain based on primary bandwdith*/
	s1g_start_freq = morse_s1g_get_start_freq_for_country(
				iface->conf->op_country, s1g_prim_frequency, s1g_prim_bw);
	s1g_prim_channel = (s1g_prim_frequency - s1g_start_freq) / S1G_CHAN_SEP_KHZ;

	class = morse_s1g_ch_to_op_class(s1g_bandwidth,
				iface->conf->op_country, s1g_op_channel);
	if (!class) {
		wpa_printf(MSG_ERROR, "Failed to derive class from s1g operating bandwidth");
		return MORSE_S1G_RETURN_ERROR;
	}

	prim_class = morse_s1g_ch_to_op_class(s1g_prim_bw,
					iface->conf->op_country, s1g_prim_channel);
	if (!prim_class) {
		wpa_printf(MSG_ERROR, "Failed to derive class from s1g primary bandwidth");
		return MORSE_S1G_RETURN_ERROR;
	}

	/* Derive s1g channel index corresponding to s1g primary channel */
	if (s1g_prim_frequency > s1g_center_frequency)
		s1g_prim_channel_index = ((s1g_bandwidth - 1) +
			((s1g_prim_frequency - s1g_center_frequency) / S1G_CHAN_SEP_KHZ)) / 2;
	else if (s1g_prim_frequency < s1g_center_frequency)
		s1g_prim_channel_index = ((s1g_bandwidth - 1) -
			((s1g_center_frequency - s1g_prim_frequency) / S1G_CHAN_SEP_KHZ)) / 2;
	else if (s1g_prim_frequency == s1g_center_frequency &&
			(s1g_bandwidth == 1 || s1g_bandwidth == 2))
		s1g_prim_channel_index = 0;

	if (s1g_prim_channel_index < 0 || s1g_prim_channel_index > (s1g_bandwidth - 1)) {
		wpa_printf(MSG_ERROR,
			"Invalid bandwidth, freq, center_freq1 combination for country %c%c",
			iface->conf->op_country[0], iface->conf->op_country[1]);
		return MORSE_S1G_RETURN_ERROR;
	}

	/* When Operating BW is 1MHz, set sec_channel_offset to 0 */
	if (s1g_bandwidth == IEEE80211_CHAN_1MHZ) {
		settings->freq_params.sec_channel_offset = 0;
	} else if (settings->freq_params.sec_channel_offset != -1 &&
				settings->freq_params.sec_channel_offset != 1) {
		wpa_printf(MSG_ERROR, "Invalid secondary channel offset %d, s1g_prim_bw=%d",
					settings->freq_params.sec_channel_offset, s1g_prim_bw);
		return MORSE_S1G_RETURN_ERROR;
	}

	/* When primary channel bandwidth is 2MHz, the s1g_prim_channel_index points to
	 * lower side of 2MHz primay channel, so, increment primary channel index to point
	 * to upper side (at odd index) if sec_channel_offset is -1.
	 */
	if (settings->freq_params.sec_channel_offset == -1 && s1g_prim_bw == IEEE80211_CHAN_2MHZ)
		s1g_prim_channel_index++;

	/* Enable VHT so that capabilities matched to S1G can be used. If VHT is enabled, HT must
	 * also be enabled.
	 */
	settings->freq_params.vht_enabled = 1;
	settings->freq_params.ht_enabled = 1;

	switch (s1g_bandwidth) {
	case 8:
		settings->freq_params.bandwidth = 160;
		break;
	case 4:
		settings->freq_params.bandwidth = 80;
		break;
	case 2:
		settings->freq_params.bandwidth = 40;
		break;
	case 1:
		settings->freq_params.bandwidth = 20;
		break;
	default:
		settings->freq_params.bandwidth = 20;
		break;
	}

	s1g_prim_1MHz_channel = morse_cc_get_primary_s1g_channel(s1g_bandwidth,
			IEEE80211_CHAN_1MHZ, s1g_op_channel, s1g_prim_channel_index,
			iface->conf->op_country);


	ht20_mapped_channel = morse_s1g_chan_to_ht20_prim_chan(s1g_op_channel,s1g_prim_1MHz_channel,
				iface->conf->op_country);

	settings->freq_params.center_freq1 = morse_s1g_freq_to_ht_freq(s1g_center_frequency, class,
									true);
	settings->freq_params.freq = ieee80211_channel_to_frequency(ht20_mapped_channel,
									NL80211_BAND_5GHZ);

	if (morse_s1g_csa_validate_primary_chan(iface, settings->freq_params.freq)) {
		wpa_printf(MSG_ERROR, "Block CSA as primary 1MHz channel is same as current");
		return MORSE_S1G_RETURN_ERROR;
	}

	settings->s1g_freq_params.s1g_prim_channel_index_1MHz = s1g_prim_channel_index;
	/*
	 * Advertise local op class in ECSA IE also to be consistent with
	 * S1G Operation IE.
	 * TODO - SW-8116: Updating S1G Op IE also to global would require interop
	 * testing with other vendors, which will be done as part of Q2 2023.
	 */
	settings->s1g_freq_params.s1g_global_op_class = class->s1g_op_class;
	settings->s1g_freq_params.s1g_prim_bw = s1g_prim_bw;
	settings->s1g_freq_params.s1g_oper_bw = s1g_bandwidth;
	settings->s1g_freq_params.s1g_oper_freq = s1g_center_frequency;
	settings->s1g_freq_params.s1g_prim_ch_global_op_class = prim_class->global_op_class;

	return MORSE_SUCCESS;
}

/*
 * Remove duplicate entries in a buffer and arrange in ascending order
 * Eg: For "JP" supported operating classes IE buffer fills the global_op_class
 * values 73, 64, 64, 65, 65 corresponding to each S1G_local_op_class.
 * After processing the elements will be arranged as 64, 65, 73
 */
int morse_remove_duplicates_and_sort_buf(struct wpabuf *buf, int buf_offset)
{
	int i, j, k;
	u8 high_value;
	int ie_len = wpabuf_len(buf);
	u8 *ie_buf = (u8 *)wpabuf_head(buf);

	/* Removes duplicate entries from buffer */
	for (i = buf_offset; i < ie_len; i++) {
		for (j = i + 1; j < ie_len; j++) {
			if (memcmp((ie_buf + i), (ie_buf + j), 1) == 0) {
				for (k = j; k < ie_len; k++)
					memcpy((ie_buf + k), (ie_buf + k + 1), 1);
				j--;
				ie_len--;
				buf->used = ie_len;
			}
		}
	}

	/* Sort buffer in ascending order */
	for (i = buf_offset; i < (int)wpabuf_len(buf); ++i) {
		for (j = i + 1; j < (int)wpabuf_len(buf); ++j) {
			if (memcmp((ie_buf + i), (ie_buf + j), 1) > 0) {
				high_value = *(ie_buf + i);
				memcpy((ie_buf + i), (ie_buf + j), 1);
				*(ie_buf + j) = high_value;
			}
		}
	}

	return 0;
}

int morse_insert_supported_op_class(struct wpabuf *buf, char *cc,
				int s1g_ch_width, int s1g_op_chan)
{
	const struct ah_class *class;
	const struct ah_class *current_class;
	unsigned int i;

	/* Fill current operating class based on oper ie */
	current_class = morse_s1g_ch_to_op_class(s1g_ch_width, cc, s1g_op_chan);
	if (!current_class) {
		wpa_printf(MSG_ERROR,"Failed to derive class from s1g operating bandwidth");
		return MORSE_S1G_RETURN_ERROR;
	}

	wpabuf_put_u8(buf, current_class->global_op_class);

	/* Fill list of all supported operating class for the country */
	for (i = 0; i < S1G_OP_CLASSES_LEN; i++ ) {
		if (morse_s1g_op_class_valid(i, &class) >= 0) {
			if (class) {
				if (morse_s1g_op_class_has_cc(class, cc))
					wpabuf_put_u8(buf, class->global_op_class);
			}
		}
	}

	return MORSE_SUCCESS;
}
#endif /* CONFIG_IEEE80211AH */

#ifdef CONFIG_MORSE_WNM
int morse_wnm_oper(const char *ifname, enum wnm_oper oper)
{
	int ret = -1;

	wpa_printf(MSG_INFO, "morse: wnm_oper %d", oper);

	switch (oper) {
	case WNM_SLEEP_ENTER_CONFIRM:
		ret = morse_set_long_sleep_enabled(ifname, true);
		break;

	case WNM_SLEEP_EXIT_CONFIRM:
		ret = morse_set_long_sleep_enabled(ifname, false);
		break;

	case WNM_SLEEP_ENTER_FAIL:
		wpa_printf(MSG_WARNING, "Failed to enter WNM Sleep");
		ret = 0;
		break;

	case WNM_SLEEP_EXIT_FAIL:
		wpa_printf(MSG_WARNING, "Failed to exit WNM Sleep");
		ret = morse_set_long_sleep_enabled(ifname, false);
		break;

	default:
		wpa_printf(MSG_DEBUG, "Unsupported WNM operation %d", oper);
		break;
	}

	return ret;
}
#endif

#ifdef CONFIG_IEEE80211AH
/*
 * If a frequency is in the S1G band, convert it to an HT frequency
 * for internal processing.
 * The value will be converted back to S1G in the driver, for use by firmware.
 */
int morse_convert_s1g_freq_to_ht_freq(int freq, const char *country)
{
	int ht_freq;

	if (freq < MIN_S1G_FREQ_KHZ || freq > MAX_S1G_FREQ_KHZ)
		return freq;

	wpa_printf(MSG_DEBUG, "Converting s1g freq %d to ht freq", freq);
	if (!country[0]) {
		wpa_printf(MSG_ERROR,
			"Country not configured - cannot convert s1g scan_freq %d",
			freq);
		return freq;
	}

	ht_freq = morse_s1g_freq_and_cc_to_ht_freq(freq, country);
	if (ht_freq <= 0) {
		wpa_printf(MSG_INFO, "Failed to get ht freq for s1g freq %d", freq);
		return freq;
	}

	wpa_printf(MSG_INFO, "Processing s1g freq %d internally as ht freq %d", freq, ht_freq);

	return ht_freq;
}

int morse_s1g_get_first_center_freq_for_country(char *cc)
{
	int freq = 902500;
	unsigned int region;

	for (region = 0; region < sizeof(ah_country)/sizeof(char *); region++) {
		if (strncmp(ah_country[region], cc, COUNTRY_CODE_LEN) == 0) {
			switch (region) {
			case MORSE_AU:
			case MORSE_NZ:
				freq = 915500;
				break;
			case MORSE_CA:
			case MORSE_US:
				freq = 902500;
				break;
			case MORSE_EU:
			case MORSE_GB:
				freq = 863500;
				break;
			case MORSE_IN:
				freq = 865500;
				break;
			case MORSE_JP:
				freq = 923000;
				break;
			case MORSE_KR:
				freq = 918000;
				break;
			case MORSE_SG:
				freq = 866500;
				break;
			case REGION_UNSET:
			default:
				break;
			}

			/* Match found. Stop processing. */
			break;
		}
	}

	return freq;
}
#endif /* CONFIG_IEEE80211AH */
