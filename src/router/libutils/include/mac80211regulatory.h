/*
 * mac80211info.c 
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* 
 * mostly copied from crda:
 * Copyright (c) 2008, Luis R. Rodriguez <mcgrof@gmail.com>
 * Copyright (c) 2008, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2008, Michael Green <Michael.Green@Atheros.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/types.h>

/*
 * WARNING: This file needs to be kept in sync with
 *  - the parser (dbparse.py)
 *  - the generator code (db2bin.py)
 */

/* spells "RGDB" */
#define REGDB_MAGIC 0x52474442

/*
 * Only supported version now, start at arbitrary number
 * to have some more magic. We still consider this to be
 * "Version 1" of the file.
 */
#define REGDB_VERSION 19

/*
 * The signature at the end of the file is an RSA-signed
 * SHA-1 hash of the file.
 */

/* db file starts with a struct regdb_file_header */

struct regdb_file_header {
	/* must be REGDB_MAGIC */
	__be32 magic;
	/* must be REGDB_VERSION */
	__be32 version;
	/*
	 * Pointer (offset) into file where country list starts
	 * and number of countries. The country list is sorted
	 * alphabetically to allow binary searching (should it
	 * become really huge). Each country is described by a
	 * struct regdb_file_reg_country.
	 */
	__be32 reg_country_ptr;
	__be32 reg_country_num;
	/* length (in bytes) of the signature at the end of the file */
	__be32 signature_length;
};

struct regdb_file_freq_range {
	__be32 start_freq, /* in kHz */
		end_freq, /* in kHz */
		max_bandwidth; /* in kHz */
};

/*
 * Values of zero mean "not applicable", i.e. the regulatory
 * does not limit a certain value.
 */
struct regdb_file_power_rule {
	/* antenna gain is in mBi (100 * dBi) */
	__be32 max_antenna_gain;
	/* this is in mBm (100 * dBm) */
	__be32 max_eirp;
};

/* must match <linux/nl80211.h> enum nl80211_reg_rule_flags */

enum reg_rule_flags {
	RRF_NO_OFDM = 1 << 0, /* OFDM modulation not allowed */
	RRF_NO_CCK = 1 << 1, /* CCK modulation not allowed */
	RRF_NO_INDOOR = 1 << 2, /* indoor operation not allowed */
	RRF_NO_OUTDOOR = 1 << 3, /* outdoor operation not allowed */
	RRF_DFS = 1 << 4, /* DFS support is required to be
				 * used */
	RRF_PTP_ONLY = 1 << 5, /* this is only for Point To Point
				 * links */
	RRF_PTMP_ONLY = 1 << 6, /* this is only for Point To Multi
				 * Point links */
	RRF_PASSIVE_SCAN = 1 << 7, /* passive scan is required */
	RRF_NO_IBSS = 1 << 8, /* IBSS is not allowed */
};

struct regdb_file_reg_rule {
	/* pointers (offsets) into the file */
	__be32 freq_range_ptr; /* pointer to a struct regdb_file_freq_range */
	__be32 power_rule_ptr; /* pointer to a struct regdb_file_power_rule */
	/* rule flags using enum reg_rule_flags */
	__be32 flags;
};

struct regdb_file_reg_rules_collection {
	__be32 reg_rule_num;
	/* pointers (offsets) into the file. There are reg_rule_num elements
	 * in the reg_rule_ptrs array pointing to struct
	 * regdb_file_reg_rule */
	__be32 reg_rule_ptrs[];
};

struct regdb_file_reg_country {
	__u8 alpha2[2];
	__u8 PAD[2];
	/* pointer (offset) into the file to a struct
	 * regdb_file_reg_rules_collection */
	__be32 reg_collection_ptr;
};

/*
 * Verify that no unexpected padding is added to structures
 * for some reason.
 */

#define ERROR_ON(cond) ((void)sizeof(char[1 - 2 * !!(cond)]))

#define CHECK_STRUCT(name, size) ERROR_ON(sizeof(struct name) != size)

static void check_db_binary_structs(void)
{
	CHECK_STRUCT(regdb_file_header, 20);
	CHECK_STRUCT(regdb_file_freq_range, 12);
	CHECK_STRUCT(regdb_file_power_rule, 8);
	CHECK_STRUCT(regdb_file_reg_rule, 12);
	CHECK_STRUCT(regdb_file_reg_rules_collection, 4);
	CHECK_STRUCT(regdb_file_reg_country, 8);
}

/* from reglib.h */

/* Common regulatory structures, functions and helpers */

/* This matches the kernel's data structures */
struct ieee80211_freq_range {
	__u32 start_freq_khz;
	__u32 end_freq_khz;
	__u32 max_bandwidth_khz;
};

struct ieee80211_power_rule {
	__u32 max_antenna_gain;
	__u32 max_eirp;
};

struct ieee80211_reg_rule {
	struct ieee80211_freq_range freq_range;
	struct ieee80211_power_rule power_rule;
	__u32 flags;
};

struct ieee80211_regdomain {
	__u32 n_reg_rules;
	char alpha2[2];
	struct ieee80211_reg_rule reg_rules[];
};

static int is_world_regdom(const char *alpha2)
{
	if (alpha2[0] == '0' && alpha2[1] == '0')
		return 1;
	return 0;
}

static int isalpha_upper(char letter)
{
	if (letter >= 'A' && letter <= 'Z')
		return 1;
	return 0;
}

static int is_alpha2(const char *alpha2)
{
	if (isalpha_upper(alpha2[0]) && isalpha_upper(alpha2[1]))
		return 1;
	return 0;
}

/* Avoid stdlib */
static int is_len_2(const char *alpha2)
{
	if (alpha2[0] == '\0' || (alpha2[1] == '\0'))
		return 0;
	if (alpha2[2] == '\0')
		return 1;
	return 0;
}

static int is_valid_regdom(const char *alpha2)
{
	if (!is_len_2(alpha2))
		return 0;

	if (!is_alpha2(alpha2) && !is_world_regdom(alpha2))
		return 0;

	return 1;
}

static __u32 max(__u32 a, __u32 b)
{
	return (a > b) ? a : b;
}

static __u32 min(__u32 a, __u32 b)
{
	return (a > b) ? b : a;
}

static void *crda_get_file_ptr(__u8 *db, int dblen, int structlen, __be32 ptr);

/* File reg db entry -> rd converstion utilities */
struct ieee80211_regdomain *country2rd(__u8 *db, int dblen, struct regdb_file_reg_country *country);

struct ieee80211_regdomain *mac80211_get_regdomain(const char *varcountry);
