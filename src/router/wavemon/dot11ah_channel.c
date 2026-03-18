/*
 * Copyright Morse Micro 2022
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
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "dot11ah_channel.h"
#include <stdio.h>
#include <string.h>

static const country_channel_map_t us_channel_map = {
	.country = "US",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 48,
	.ah_vals = {
		/* 1Mhz */
		{132, 1, 902.5, 1},
		{136, 3, 903.5, 1},
		{36, 5, 904.5, 1},
		{40, 7, 905.5, 1},
		{44, 9, 906.5, 1},
		{48, 11, 907.5, 1},
		{52, 13, 908.5, 1},
		{56, 15, 909.5, 1},
		{60, 17, 910.5, 1},
		{64, 19, 911.5, 1},
		{100, 21, 912.5, 1},
		{104, 23, 913.5, 1},
		{108, 25, 914.5, 1},
		{112, 27, 915.5, 1},
		{116, 29, 916.5, 1},
		{120, 31, 917.5, 1},
		{124, 33, 918.5, 1},
		{128, 35, 919.5, 1},
		{149, 37, 920.5, 1},
		{153, 39, 921.5, 1},
		{157, 41, 922.5, 1},
		{161, 43, 923.5, 1},
		{165, 45, 924.5, 1},
		{169, 47, 925.5, 1},
		{173, 49, 926.5, 1},
		{177, 51, 927.5, 1},
		/* 2MHz */
		{134, 2, 903, 2},
		{38, 6, 905, 2},
		{46, 10, 907, 2},
		{54, 14, 909, 2},
		{62, 18, 911, 2},
		{102, 22, 913, 2},
		{110, 26, 915, 2},
		{118, 30, 917, 2},
		{126, 34, 919, 2},
		{151, 38, 921, 2},
		{159, 42, 923, 2},
		{167, 46, 925, 2},
		{175, 50, 927, 2},
		/* 4MHz */
		{42, 8, 906, 4},
		{58, 16, 910, 4},
		{106, 24, 914, 4},
		{122, 32, 918, 4},
		{155, 40, 922, 4},
		{171, 48, 926, 4},
		/* 8MHz */
		{50, 12, 908, 8},
		{114, 28, 916, 8},
		{163, 44, 924, 8},
	}
};

static const country_channel_map_t au_2020_channel_map = {
	.country = "AU",
	.chan_schemes = CHAN_SCHEME_80211_2020,
	.num_mapped_channels = 23,
	.ah_vals = {
		/* 1Mhz */
		{112, 27, 915.5, 1},
		{116, 29, 916.5, 1},
		{120, 31, 917.5, 1},
		{124, 33, 918.5, 1},
		{128, 35, 919.5, 1},
		{149, 37, 920.5, 1},
		{153, 39, 921.5, 1},
		{157, 41, 922.5, 1},
		{161, 43, 923.5, 1},
		{165, 45, 924.5, 1},
		{169, 47, 925.5, 1},
		{173, 49, 926.5, 1},
		{177, 51, 927.5, 1},
		/* 2Mhz */
		{118, 30, 917, 2},
		{126, 34, 919, 2},
		{151, 38, 921, 2},
		{159, 42, 923, 2},
		{167, 46, 925, 2},
		{175, 50, 927, 2},
		/* 4 Mhz */
		{122, 32, 918, 4},
		{155, 40, 922, 4},
		{171, 48, 926, 4},
		/* 8 Mhz */
		{163, 44, 924, 8}
	}
};

static const country_channel_map_t au_revmf_channel_map = {
	.country = "AU",
	.chan_schemes = CHAN_SCHEME_80211_REVMF | CHAN_SCHEME_80211_2024,
	.num_mapped_channels = 26,
	.ah_vals = {
		/* 1 MHz */
		{36, 28, 916.0, 1},
		{40, 30, 917.0, 1},
		{44, 32, 918.0, 1},
		{48, 34, 919.0, 1},
		{52, 36, 920.0, 1},
		{56, 38, 921.0, 1},
		{60, 40, 922.0, 1},
		{64, 42, 923.0, 1},
		{116, 44, 924.0, 1},
		{120, 46, 925.0, 1},
		{124, 48, 926.0, 1},
		{128, 50, 927.0, 1},
		/* 2 MHz */
		{38, 29, 916.5, 2},
		{46, 33, 918.5, 2},
		{54, 37, 920.5, 2},
		{62, 41, 922.5, 2},
		{118, 45, 924.5, 2},
		{126, 49, 926.5, 2},
		/* 4 MHz */
		{42, 31, 917.5, 4},
		{58, 39, 921.5, 4},
		{122, 47, 925.5, 4},
		/* 8 MHz */
		{50, 35, 919.5, 8},
		{114, 43, 923.5, 8},

		/* These are the extra channels. */
		/* 4 MHz */
		{155, 51, 919.5, 4},
		{171, 59, 923.5, 4},
		/* 8 MHz */
		{163, 55, 921.5, 8},
	}
};

static const country_channel_map_t nz_channel_map = {
	.country = "NZ",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 23,
	.ah_vals = {
		/* 1Mhz */
		{112, 27, 915.5, 1},
		{116, 29, 916.5, 1},
		{120, 31, 917.5, 1},
		{124, 33, 918.5, 1},
		{128, 35, 919.5, 1},
		{149, 37, 920.5, 1},
		{153, 39, 921.5, 1},
		{157, 41, 922.5, 1},
		{161, 43, 923.5, 1},
		{165, 45, 924.5, 1},
		{169, 47, 925.5, 1},
		{173, 49, 926.5, 1},
		{177, 51, 927.5, 1},
		/* 2Mhz */
		{118, 30, 917, 2},
		{126, 34, 919, 2},
		{151, 38, 921, 2},
		{159, 42, 923, 2},
		{167, 46, 925, 2},
		{175, 50, 927, 2},
		/* 4 Mhz */
		{122, 32, 918, 4},
		{155, 40, 922, 4},
		{171, 48, 926, 4},
		/* 8 Mhz */
		{163, 44, 924, 8}
	}
};

static const country_channel_map_t eu_channel_map = {
	.country = "EU",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 8,
	.ah_vals = {
		/* 1Mhz */
		{132, 1, 863.5, 1},
		{136, 3, 864.5, 1},
		{36, 5, 865.5, 1},
		{40, 7, 866.5, 1},
		{44, 9, 867.5, 1},
		{120, 31, 916.9, 1},
		{124, 33, 917.9, 1},
		{128, 35, 918.9, 1},
	}
};

static const country_channel_map_t in_channel_map = {
	.country = "IN",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 3,
	.ah_vals = {
		/* 1Mhz */
		{36, 5, 865.5, 1},
		{40, 7, 866.5, 1},
		{44, 9, 867.5, 1},
	}
};

static const country_channel_map_t jp_channel_map = {
	.country = "JP",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 12,
	.ah_vals = {
		/* 1 MHz */
		{32,  9, 921, 1},
		{36, 13, 923, 1},
		{40, 15, 924, 1},
		{44, 17, 925, 1},
		{48, 19, 926, 1},
		{64, 21, 927, 1},
		/* 2 MHz*/
		{38, 2, 923.5, 2},
		{54, 4, 924.5, 2},
		{46, 6, 925.5, 2},
		{62, 8, 926.5, 2},
		/* 4MHz */
		{42, 36, 924.5, 4},
		{58, 38, 925.5, 4},
	}
};

static const country_channel_map_t kr_channel_map = {
	.country = "KR",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 10,
	.ah_vals = {
		/* 1 Mhz */
		{132, 1, 918, 1},
		{136, 3, 919, 1},
		{36, 5, 920, 1},
		{40, 7, 921, 1},
		{44, 9, 922, 1},
		{48, 11, 923, 1},
		/* 2Mhz */
		{134, 2, 918.5, 2},
		{38, 6, 920.5, 2},
		{46, 10, 922.5, 2},
		/* 4Mhz */
		{42, 8, 921.5, 4},
	}
};

static const country_channel_map_t sg_channel_map = {
	.country = "SG",
	.chan_schemes = CHAN_SCHEME_ALL,
	.num_mapped_channels = 12,
	.ah_vals = {
		/* 1 Mhz */
		{40, 7, 866.5, 1},
		{44, 9, 867.5, 1},
		{48, 11, 868.5, 1},
		{149, 37, 920.5, 1},
		{153, 39, 921.5, 1},
		{157, 41, 922.5, 1},
		{161, 43, 923.5, 1},
		{165, 45, 924.5, 1},
		/* 2Mhz */
		{46, 10, 868, 2},
		{151, 38, 921, 2},
		{159, 42, 923, 2},
		/* 4Mhz */
		{155, 40, 922, 4}
	}
};

static const country_channel_map_t *mapped_channel[] = {
	&us_channel_map,
	&au_2020_channel_map,
	&au_revmf_channel_map,
	&nz_channel_map,
	&eu_channel_map,
	&in_channel_map,
	&jp_channel_map,
	&kr_channel_map,
	&sg_channel_map
};

#define CHANNEL_MAP_SIZE (sizeof(mapped_channel) / sizeof(*mapped_channel))

static enum chan_scheme morse_get_chan_scheme()
{
	FILE *f = fopen("/sys/module/dot11ah/parameters/channelization_scheme", "r");
	if (!f) {
		/* This indicates an old driver. */
		return CHAN_SCHEME_80211_2020;
	}

	int r = fgetc(f);
	fclose(f);
	switch (r) {
	case '1':
		return CHAN_SCHEME_80211_2020;
	case '2':
		return CHAN_SCHEME_80211_2024;
	case '3':
		return CHAN_SCHEME_80211_REVMF;
	default:
		/* A driver that has something we don't understand;
		 * default to what we think would be the driver default
		 * for now.
		 */
		return CHAN_SCHEME_80211_REVMF;
	};
}

void s1g_get_country(char *buf)
{
	FILE *f = fopen("/sys/module/morse/parameters/country", "r");
	if (!f) {
		buf[0] = '\0';
		return;
	}

	if (fread(buf, 1, 2, f) != 2) {
		buf[0] = '\0';
	}

	fclose(f);
}

const country_channel_map_t *set_s1g_channel_map(void)
{
	char curr_country[3] = {0};
	enum chan_scheme curr_scheme = morse_get_chan_scheme();

	s1g_get_country(curr_country);

	for (int i = 0; i < CHANNEL_MAP_SIZE; i++) {
		const country_channel_map_t *map = mapped_channel[i];
		if (map->chan_schemes & curr_scheme && !strncmp(curr_country, map->country, strlen(map->country))) {
			return map;
		}
	}

	return NULL;
}
