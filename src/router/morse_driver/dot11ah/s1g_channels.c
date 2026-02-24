/*
 * Copyright 2017-2024 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/types.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include <linux/crc32.h>
#include <linux/ieee80211.h>
#include "s1g_ieee80211.h"

#include "dot11ah.h"
#include "tim.h"
#include "../morse.h"
#include "s1g_channels_rules.c"
#include "channel_alphas.h"

#define CHANNEL_FLAGS_BW_MASK 0x7c000

/**
 * Morse dot11ah S1G channel list.
 *
 * Includes mapping to 5G channels
 */
struct morse_dot11ah_ch_map {
	char alpha[3];
	int (*prim_1mhz_channel_loc_to_idx)(int op_bw_mhz,
					    int pr_bw_mhz,
					    int pr_chan_num,
					    int chan_centre_freq_num,
					    int chan_loc);
	int (*calculate_primary_s1g)(int op_bw_mhz,
				     int pr_bw_mhz,
				     int chan_centre_freq_num,
				     int pr_1mhz_chan_idx);
	int (*s1g_op_chan_pri_chan_to_5g)(int s1g_op_chan,
					  int s1g_pri_chan);
	int (*get_pri_1mhz_chan)(int primary_channel,
				 int primary_channel_width_mhz,
				 bool pri_1_mhz_loc_upper);
	int (*transform_overlapping_5g_chan)(int chan_5g);
	u32 num_mapped_channels;
	struct morse_dot11ah_channel *s1g_channels;
};

static int prim_1mhz_channel_loc_to_idx_default(int op_bw_mhz, int pr_bw_mhz, int pr_chan_num,
						int chan_centre_freq_num, int chan_loc)
{
	switch (op_bw_mhz) {
	case 1:
		return 0;
	case 2:
		return chan_loc;
	case 4:
		if (pr_bw_mhz == 1)
			return (((pr_chan_num - chan_centre_freq_num) + 3) / 2);
		else
			return (((pr_chan_num - chan_centre_freq_num) + 2) / 2) + chan_loc;
	case 8:
		if (pr_bw_mhz == 1)
			return (((pr_chan_num - chan_centre_freq_num) + 7) / 2);
		else
			return (((pr_chan_num - chan_centre_freq_num) + 6) / 2) + chan_loc;
	default:
		return -ENOENT;
	}
}

static int prim_1mhz_channel_loc_to_idx_jp(int op_bw_mhz, int pr_bw_mhz, int pr_chan_num,
					   int chan_centre_freq_num, int chan_loc)
{
	switch (op_bw_mhz) {
	case 1:
		return 0;
	case 2:
		return chan_loc;
	case 4:
		if (chan_centre_freq_num == 36) {
			return pr_chan_num == 13 ? 0 :
			       pr_chan_num == 15 ? 1 :
			       pr_chan_num == 17 ? 2 :
			       pr_chan_num == 19 ? 3 :
			       pr_chan_num == 2  ? 0 + chan_loc :
			       pr_chan_num == 6  ? 2 + chan_loc : -EINVAL;
		} else if (chan_centre_freq_num == 38) {
			return pr_chan_num == 15 ? 0 :
			       pr_chan_num == 17 ? 1 :
			       pr_chan_num == 19 ? 2 :
			       pr_chan_num == 21 ? 3 :
			       pr_chan_num == 4  ? 0 + chan_loc :
			       pr_chan_num == 8  ? 2 + chan_loc : -EINVAL;
		} else {
			return -EINVAL;
		}
	case 8:
	default:
		return -EINVAL;
	}
}

static int calculate_primary_s1g_channel_default(int op_bw_mhz, int pr_bw_mhz,
						 int chan_centre_freq_num, int pr_1mhz_chan_idx)
{
	int chan_loc = (pr_1mhz_chan_idx % 2);

	switch (op_bw_mhz) {
	case 1:
		return chan_centre_freq_num;
	case 2:
		if (pr_bw_mhz == 1)
			return chan_centre_freq_num + ((chan_loc == 0) ? -1 : 1);
		else
			return chan_centre_freq_num;
	case 4:
		if (pr_bw_mhz == 1)
			return ((2 * pr_1mhz_chan_idx) - 3) + chan_centre_freq_num;
		else
			return ((pr_1mhz_chan_idx / 2) * 4) - 2 + chan_centre_freq_num;
	case 8:
		if (pr_bw_mhz == 1)
			return ((2 * pr_1mhz_chan_idx) - 7) + chan_centre_freq_num;
		else
			return ((pr_1mhz_chan_idx / 2) * 4) - 6 + chan_centre_freq_num;
	}

	return -EINVAL;
}

static int calculate_primary_s1g_channel_jp(int op_bw_mhz, int pr_bw_mhz, int chan_centre_freq_num,
					    int pr_1mhz_chan_idx)
{
	int offset;

	switch (op_bw_mhz) {
	case 1:
		return chan_centre_freq_num;
	case 2:
		if (pr_bw_mhz == 1) {
			offset = pr_1mhz_chan_idx ? 13 : 11;
			return (chan_centre_freq_num + offset);
		} else {
			return chan_centre_freq_num;
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
		return (offset > 0) ? (chan_centre_freq_num - offset) : -EINVAL;
	default:
		return -ENOENT;
	}
}

static int s1g_op_chan_pri_chan_to_5g_default(int s1g_op_chan, int s1g_pri_chan)
{
	return morse_dot11ah_s1g_chan_to_5g_chan(s1g_pri_chan);
}

static int s1g_op_chan_pri_chan_to_5g_jp(int s1g_op_chan, int s1g_pri_chan)
{
	int ht20mhz_offset;
	/* In the JP regulatory, some primary channels have duplicate
	 * entries so to get the correct 5g value, the op chan
	 * must be considered.
	 */
	if ((s1g_op_chan == 4 ||
	     s1g_op_chan == 8 ||
	     s1g_op_chan == 38) &&
	    s1g_pri_chan != 21) {
		ht20mhz_offset = 12;
	} else {
		ht20mhz_offset = 0;
	}

	return morse_dot11ah_s1g_chan_to_5g_chan(s1g_pri_chan) + ht20mhz_offset;
}

static int transform_overlapping_5g_chan_jp(int chan_5g)
{
	int ht20mhz_offset = 0;

	/* In the JP regulatory, some S1G primary channels map to multiple 5G channels.
	 * Only the first mapping is maintained, so transform the overlapping channels back
	 * to the known set.
	 */
	if (chan_5g == 52 || chan_5g == 56 || chan_5g == 60)
		ht20mhz_offset = 12;

	return chan_5g - ht20mhz_offset;
}

static int get_pri_1mhz_chan_default(int primary_channel,
			      int primary_channel_width_mhz, bool pri_1_mhz_loc_upper)
{
	if (primary_channel_width_mhz == 2)
		return primary_channel += pri_1_mhz_loc_upper ? 1 : -1;
	else if (primary_channel_width_mhz == 1)
		return primary_channel;
	else
		return -EINVAL;
}

static int get_pri_1mhz_chan_jp(int primary_channel,
			     int primary_channel_width_mhz, bool pri_1_mhz_loc_upper)
{
	if (primary_channel_width_mhz == 2) {
		switch (primary_channel) {
		case 2:
			return pri_1_mhz_loc_upper ? 15 : 13;
		case 4:
			return pri_1_mhz_loc_upper ? 17 : 15;
		case 6:
			return pri_1_mhz_loc_upper ? 19 : 17;
		case 8:
			return pri_1_mhz_loc_upper ? 21 : 19;
		default:
			return -ENOENT;
		}
	} else if (primary_channel_width_mhz == 1) {
		return primary_channel;
	} else {
		return -EINVAL;
	}
}

/* AU map */
static const struct morse_dot11ah_ch_map mors_au_map = {
		.alpha = CHANNEL_ALPHA_AU,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(au_s1g_channels),
		.s1g_channels = au_s1g_channels,
};

/* NZ map */
static const struct morse_dot11ah_ch_map mors_nz_map = {
		.alpha = CHANNEL_ALPHA_NZ,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(nz_s1g_channels),
		.s1g_channels = nz_s1g_channels,
};

/* CA map */
static const struct morse_dot11ah_ch_map mors_ca_map = {
		.alpha = CHANNEL_ALPHA_CA,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(ca_s1g_channels),
		.s1g_channels = ca_s1g_channels,
};

/* EU map */
static const struct morse_dot11ah_ch_map mors_eu_map = {
		.alpha = CHANNEL_ALPHA_EU,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(eu_s1g_channels),
		.s1g_channels = eu_s1g_channels,
};

/* GB map */
static const struct morse_dot11ah_ch_map mors_gb_map = {
		.alpha = CHANNEL_ALPHA_GB,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(gb_s1g_channels),
		.s1g_channels = gb_s1g_channels,
};

/* IN map */
static const struct morse_dot11ah_ch_map mors_in_map = {
		.alpha = CHANNEL_ALPHA_IN,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(in_s1g_channels),
		.s1g_channels = in_s1g_channels,
};

/* JP map */
static const struct morse_dot11ah_ch_map mors_jp_map = {
		.alpha = CHANNEL_ALPHA_JP,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_jp,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_jp,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_jp,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_jp,
		.transform_overlapping_5g_chan = &transform_overlapping_5g_chan_jp,
		.num_mapped_channels = ARRAY_SIZE(jp_s1g_channels),
		.s1g_channels = jp_s1g_channels,
};

/* KR map */
static const struct morse_dot11ah_ch_map mors_kr_map = {
		.alpha = CHANNEL_ALPHA_KR,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(kr_s1g_channels),
		.s1g_channels = kr_s1g_channels,
};

/* SG map */
static const struct morse_dot11ah_ch_map mors_sg_map = {
		.alpha = CHANNEL_ALPHA_SG,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(sg_s1g_channels),
		.s1g_channels = sg_s1g_channels,
};

/* US map */
static const struct morse_dot11ah_ch_map mors_us_map = {
		.alpha = CHANNEL_ALPHA_US,
		.prim_1mhz_channel_loc_to_idx = &prim_1mhz_channel_loc_to_idx_default,
		.calculate_primary_s1g = &calculate_primary_s1g_channel_default,
		.s1g_op_chan_pri_chan_to_5g = &s1g_op_chan_pri_chan_to_5g_default,
		.get_pri_1mhz_chan = &get_pri_1mhz_chan_default,
		.transform_overlapping_5g_chan = NULL,
		.num_mapped_channels = ARRAY_SIZE(us_s1g_channels),
		.s1g_channels = us_s1g_channels,
};

const struct morse_dot11ah_ch_map *mapped_channels[] = {
	&mors_au_map,
	&mors_ca_map,
	&mors_eu_map,
	&mors_gb_map,
	&mors_in_map,
	&mors_jp_map,
	&mors_kr_map,
	&mors_nz_map,
	&mors_sg_map,
	&mors_us_map,
};

static const struct morse_dot11ah_ch_map *__mors_s1g_map;

int morse_dot11ah_channel_set_map(const char *alpha)
{
	int i;

	if (WARN_ON(!alpha))
		return -ENOENT;

	for (i = 0; i < ARRAY_SIZE(mapped_channels); i++)
		if (!strncmp(mapped_channels[i]->alpha, alpha, strlen(alpha)))
			__mors_s1g_map = mapped_channels[i];

	if (!__mors_s1g_map)
		return -ENOENT;

	if (WARN_ON(!__mors_s1g_map->prim_1mhz_channel_loc_to_idx))
		return -ENOENT;

	if (WARN_ON(!__mors_s1g_map->calculate_primary_s1g))
		return -ENOENT;

	if (WARN_ON(!__mors_s1g_map->prim_1mhz_channel_loc_to_idx))
		return -ENOENT;

	if (WARN_ON(!__mors_s1g_map->get_pri_1mhz_chan))
		return -ENOENT;

	return 0;
}

/* Convert a regional ISO alpha-2 string to a morse_region */
static enum morse_dot11ah_region morse_reg_get_region(const char *alpha)
{
	if (!alpha)
		return REGION_UNSET;

	if (!strcmp(alpha, "AU"))
		return MORSE_AU;

	if (!strcmp(alpha, "CA"))
		return MORSE_CA;

	if (!strcmp(alpha, "EU"))
		return MORSE_EU;

	if (!strcmp(alpha, "GB"))
		return MORSE_GB;

	if (!strcmp(alpha, "IN"))
		return MORSE_IN;

	if (!strcmp(alpha, "JP"))
		return MORSE_JP;

	if (!strcmp(alpha, "KR"))
		return MORSE_KR;

	if (!strcmp(alpha, "NZ"))
		return MORSE_NZ;

	if (!strcmp(alpha, "SG"))
		return MORSE_SG;

	if (!strcmp(alpha, "US"))
		return MORSE_US;

	return REGION_UNSET;
}

static struct morse_dot11ah_channel *lookup_s1g_chan_from_5g_chan(int chan_5g)
{
	int ch;

	if (__mors_s1g_map->transform_overlapping_5g_chan)
		chan_5g = __mors_s1g_map->transform_overlapping_5g_chan(chan_5g);

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++) {
		if (chan_5g == __mors_s1g_map->s1g_channels[ch].hw_value_map)
			return &__mors_s1g_map->s1g_channels[ch];
	}

	return NULL;
}

#if KERNEL_VERSION(5, 10, 11) > MAC80211_VERSION_CODE
u8 ch_flag_to_chan_bw(enum morse_dot11ah_channel_flags flags)
#else
u8 ch_flag_to_chan_bw(enum ieee80211_channel_flags flags)
#endif
{
	int bw = flags & CHANNEL_FLAGS_BW_MASK;

	switch (bw) {
	case IEEE80211_CHAN_1MHZ:
		return 1;
	case IEEE80211_CHAN_2MHZ:
		return 2;
	case IEEE80211_CHAN_4MHZ:
		return 4;
	case IEEE80211_CHAN_8MHZ:
		return 8;
	default:
		return 0;
	}
}
EXPORT_SYMBOL(ch_flag_to_chan_bw);

/* Return s1g frequency in HZ given s1g chan number */
u32 morse_dot11ah_s1g_chan_to_s1g_freq(int chan_s1g)
{
	int ch;

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++) {
		const struct ieee80211_channel_s1g *chan = &__mors_s1g_map->s1g_channels[ch].ch;

		if (chan_s1g == chan->hw_value)
			return KHZ_TO_HZ(ieee80211_channel_to_khz(chan));
	}

	return false;
}
EXPORT_SYMBOL(morse_dot11ah_s1g_chan_to_s1g_freq);

/* Return s1g channel number given 5g channel number and op class */
u16 morse_dot11ah_5g_chan_to_s1g_ch(u8 chan_5g, u8 op_class)
{
	struct morse_dot11ah_channel *chan = lookup_s1g_chan_from_5g_chan(chan_5g);

	return (chan) ? chan->ch.hw_value : 0;
}
EXPORT_SYMBOL(morse_dot11ah_5g_chan_to_s1g_ch);

/* Returns a pointer to the s1g map region string */
const char *morse_dot11ah_get_region_str(void)
{
	return (const char *)__mors_s1g_map->alpha;
}
EXPORT_SYMBOL(morse_dot11ah_get_region_str);

const struct morse_dot11ah_channel *morse_dot11ah_s1g_freq_to_s1g(int freq, int bw)
{
	int ch;
	int _freq;
	int _bw;

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++) {
		_freq = MHZ_TO_HZ(__mors_s1g_map->s1g_channels[ch].ch.center_freq) +
				KHZ_TO_HZ(__mors_s1g_map->s1g_channels[ch].ch.freq_offset);
		_bw = ch_flag_to_chan_bw(__mors_s1g_map->s1g_channels[ch].ch.flags);
		if (freq == _freq && bw == _bw)
			return &__mors_s1g_map->s1g_channels[ch];
	}

	return NULL;
}
EXPORT_SYMBOL(morse_dot11ah_s1g_freq_to_s1g);

const struct morse_dot11ah_channel
*morse_dot11ah_5g_chan_to_s1g(struct ieee80211_channel *chan_5g)
{
	return lookup_s1g_chan_from_5g_chan(chan_5g->hw_value);
}
EXPORT_SYMBOL(morse_dot11ah_5g_chan_to_s1g);

const struct morse_dot11ah_channel
*morse_dot11ah_channel_chandef_to_s1g(struct cfg80211_chan_def *chan_5g)
{
	int hwval;

	if (chan_5g->center_freq1 && chan_5g->center_freq1 != chan_5g->chan->center_freq)
		hwval = ieee80211_frequency_to_channel(chan_5g->center_freq1);
	else
		hwval = ieee80211_frequency_to_channel(chan_5g->chan->center_freq);

	return lookup_s1g_chan_from_5g_chan(hwval);
}
EXPORT_SYMBOL(morse_dot11ah_channel_chandef_to_s1g);

int morse_dot11ah_s1g_chan_to_5g_chan(int chan_s1g)
{
	int ch;

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++)
		if (chan_s1g ==  __mors_s1g_map->s1g_channels[ch].ch.hw_value)
			return __mors_s1g_map->s1g_channels[ch].hw_value_map;

	return -ENOENT;
}
EXPORT_SYMBOL(morse_dot11ah_s1g_chan_to_5g_chan);

int morse_dot11ah_s1g_chan_bw_to_5g_chan(int chan_s1g, int bw_mhz)
{
	int ch;
	int ch_bw;

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++)
		if (chan_s1g ==  __mors_s1g_map->s1g_channels[ch].ch.hw_value) {
			ch_bw = ch_flag_to_chan_bw(__mors_s1g_map->s1g_channels[ch].ch.flags);
			if (ch_bw == bw_mhz)
				return __mors_s1g_map->s1g_channels[ch].hw_value_map;
		}
	return -ENOENT;
}
EXPORT_SYMBOL(morse_dot11ah_s1g_chan_bw_to_5g_chan);

int morse_dot11ah_s1g_op_chan_pri_chan_to_5g(int s1g_op_chan, int s1g_pri_chan)
{
	return __mors_s1g_map->s1g_op_chan_pri_chan_to_5g(s1g_op_chan, s1g_pri_chan);
}
EXPORT_SYMBOL(morse_dot11ah_s1g_op_chan_pri_chan_to_5g);

u32 morse_dot11ah_channel_get_flags(int chan_s1g)
{
	int ch;

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++)
		if (chan_s1g ==  __mors_s1g_map->s1g_channels[ch].ch.hw_value)
			return __mors_s1g_map->s1g_channels[ch].ch.flags;

	/* Could not find the channel? it is safe to set flags = 0 */
	return 0;
}
EXPORT_SYMBOL(morse_dot11ah_channel_get_flags);

int morse_dot11ah_channel_to_freq_khz(int chan)
{
	enum morse_dot11ah_region region;

	region = morse_reg_get_region(__mors_s1g_map->alpha);

	switch (region) {
	case MORSE_AU:
	case MORSE_CA:
	case MORSE_NZ:
	case MORSE_US:
		return 902000 + chan * 500;
	case MORSE_EU:
	case MORSE_GB:
		if (chan < 31)
			return 863000 + chan * 500;
		else
			return 901400 + chan * 500;
	case MORSE_IN:
		return 863000 + chan * 500;
	case MORSE_JP:
		if (chan <= 21) {
			if (chan & 0x1)
				return 916500 + chan * 500;
			else
				return 922500 + chan * 500;
		} else {
			return 906500 + chan * 500;
		}
	case MORSE_KR:
		return 917500 + chan * 500;
	case MORSE_SG:
		if (chan < 31)
			return 863000 + chan * 500;
		else
			return 902000 + chan * 500;
	case REGION_UNSET:
	default:
		break;
	}
	return 0;
}
EXPORT_SYMBOL(morse_dot11ah_channel_to_freq_khz);

int morse_dot11ah_freq_khz_bw_mhz_to_chan(u32 freq, u8 bw)
{
	int channel = 0;
	enum morse_dot11ah_region region;

	region = morse_reg_get_region(__mors_s1g_map->alpha);

	switch (region) {
	case MORSE_AU:
	case MORSE_CA:
	case MORSE_NZ:
	case MORSE_US:
		channel = (freq - 902000) /  500;
		break;
	case MORSE_EU:
	case MORSE_GB:
		if (freq > 901400)
			channel = (freq - 901400) / 500;
		else
			channel = (freq - 863000) / 500;
		break;
	case MORSE_IN:
		channel = (freq - 863000) / 500;
		break;
	case MORSE_JP:
		if ((freq % 1000) == 500) {
			/* We are on a 500kHz centre */
			if (bw < 4)
				channel = (freq - 922500) / 500;
			else
				channel = (freq - 906500) / 500;
		} else {
			channel = (freq - 916500) / 500;
		}
		break;
	case MORSE_KR:
		channel = (freq - 917500) / 500;
		break;
	case MORSE_SG:
		if (freq > 902000)
			channel = (freq - 902000) / 500;
		else
			channel = (freq - 863000) / 500;
		break;
	case REGION_UNSET:
	default:
		break;
	}
	return channel;
}
EXPORT_SYMBOL(morse_dot11ah_freq_khz_bw_mhz_to_chan);

int morse_dot11ah_prim_1mhz_chan_loc_to_idx(int op_bw_mhz, int pr_bw_mhz, int pr_chan_num,
					    int chan_centre_freq_num, int chan_loc)
{
	return __mors_s1g_map->prim_1mhz_channel_loc_to_idx(op_bw_mhz,
							    pr_bw_mhz,
							    pr_chan_num,
							    chan_centre_freq_num,
							    chan_loc);
}

int morse_dot11ah_calc_prim_s1g_chan(int op_bw_mhz, int pr_bw_mhz,
						int chan_centre_freq_num, int pr_1mhz_chan_idx)
{
	return __mors_s1g_map->calculate_primary_s1g(op_bw_mhz,
						     pr_bw_mhz,
						     chan_centre_freq_num,
						     pr_1mhz_chan_idx);
}
EXPORT_SYMBOL(morse_dot11ah_calc_prim_s1g_chan);

int morse_dot11ah_get_pri_1mhz_chan(int primary_channel,
	int primary_channel_width_mhz, bool pri_1_mhz_loc_upper)
{
	return __mors_s1g_map->get_pri_1mhz_chan(primary_channel,
					   primary_channel_width_mhz, pri_1_mhz_loc_upper);
}
EXPORT_SYMBOL(morse_dot11ah_get_pri_1mhz_chan);

int morse_dot11ah_ignore_channel(int chan_s1g)
{
	int ch;

	for (ch = 0; ch < __mors_s1g_map->num_mapped_channels; ch++) {
		if (chan_s1g ==  __mors_s1g_map->s1g_channels[ch].ch.hw_value) {
			__mors_s1g_map->s1g_channels[ch].ch.flags |= IEEE80211_CHAN_IGNORE;
			return 0;
		}
	}

	return -ENOENT;
}
EXPORT_SYMBOL(morse_dot11ah_ignore_channel);

int morse_dot11ah_get_num_channels(void)
{
	if (!__mors_s1g_map)
		return 0;
	return __mors_s1g_map->num_mapped_channels;
}
EXPORT_SYMBOL(morse_dot11ah_get_num_channels);

int morse_dot11ah_fill_channel_list(struct morse_channel *list)
{
	int i;
	int count = 0;
	struct morse_channel *chan;
	const struct morse_dot11ah_channel *map_entry;

	if (!list || !__mors_s1g_map)
		return -ENOENT;

	for (i = 0; i < __mors_s1g_map->num_mapped_channels; i++) {
		map_entry = &__mors_s1g_map->s1g_channels[i];
		chan = &list[count];

		/* Skip ignored channels */
		if (map_entry->ch.flags & IEEE80211_CHAN_IGNORE)
			continue;

		chan->frequency_khz = ieee80211_channel_to_khz(&map_entry->ch);
		chan->channel_s1g = map_entry->ch.hw_value;
		chan->channel_5g = map_entry->hw_value_map;
		/* extract the s1g bandwidth from the channel flags */
		chan->bandwidth_mhz = ch_flag_to_chan_bw(map_entry->ch.flags);
		count++;
	}

	return count;
}
EXPORT_SYMBOL(morse_dot11ah_fill_channel_list);
