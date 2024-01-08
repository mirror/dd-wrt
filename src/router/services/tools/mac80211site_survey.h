/*
 * mac80211site_survey.h
 * Copyright (C) 2011 Christian Scheele <chris@dd-wrt.com>
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
 * mostly copied from iw (scan.c, utils.c, iw.h) thanks that this exists:
 * Copyright (c) 2007, 2008    Johannes Berg
 * Copyright (c) 2007      Andy Lutomirski
 * Copyright (c) 2007      Mike Kershaw
 * Copyright (c) 2008-2009     Luis R. Rodriguez
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

#define WLAN_CAPABILITY_ESS (1 << 0)
#define WLAN_CAPABILITY_IBSS (1 << 1)
#define WLAN_CAPABILITY_CF_POLLABLE (1 << 2)
#define WLAN_CAPABILITY_CF_POLL_REQUEST (1 << 3)
#define WLAN_CAPABILITY_PRIVACY (1 << 4)
#define WLAN_CAPABILITY_SHORT_PREAMBLE (1 << 5)
#define WLAN_CAPABILITY_PBCC (1 << 6)
#define WLAN_CAPABILITY_CHANNEL_AGILITY (1 << 7)
#define WLAN_CAPABILITY_SPECTRUM_MGMT (1 << 8)
#define WLAN_CAPABILITY_QOS (1 << 9)
#define WLAN_CAPABILITY_SHORT_SLOT_TIME (1 << 10)
#define WLAN_CAPABILITY_APSD (1 << 11)
#define WLAN_CAPABILITY_DSSS_OFDM (1 << 13)

#define BIT(x) (1ULL << (x))

#define IEEE80211_COUNTRY_EXTENSION_ID 201

#define ARRAY_SIZE(ar) (sizeof(ar) / sizeof(ar[0]))

union ieee80211_country_ie_triplet {
	struct {
		__u8 first_channel;
		__u8 num_channels;
		__s8 max_power;
	} __attribute__((packed)) chans;
	struct {
		__u8 reg_extension_id;
		__u8 reg_class;
		__u8 coverage_class;
	} __attribute__((packed)) ext;
} __attribute__((packed));

struct ieee80211_mtik_ie_data {
	__u8 data1[2]; /* unknown yet 0x011e */
	__u8 flags; /* 4(100) - wds, 1(1) - nstream, 8(1000) - pooling, 0 - none */
	__u8 data2[3]; /* unknown yet fill with zero */
	__u8 version[4]; /* little endian version. Use 0x1f660902 */
	__u8 pad1; /* a kind of padding, 0xff */
	__u8 namelen; /* length of radio name. Change with caution. 0x0f is safe value */
	__u8 radioname[15]; /* Radio name */
	__u8 pad2[5]; /* unknown. fill with zero */
} __attribute__((packed));

struct ieee80211_mtik_ie {
	__u8 oui[3]; /* 0x00, 0x50, 0xf2 */
	__u8 type; /* OUI type */
	__u16 version; /* spec revision */
	struct ieee80211_mtik_ie_data iedata;
} __attribute__((packed));

struct aironet_ie {
	uint8 load;
	uint8 hops;
	uint8 device;
	uint8 refresh_rate;
	uint16 cwmin;
	uint16 cwmax;
	uint8 flags;
	uint8 distance;
	char name[16]; /* AP or Client's machine name */
	uint16 num_assoc; /* number of clients associated */
	uint16 radiotype;
} __attribute__((packed));

enum print_ie_type {
	PRINT_SCAN,
	PRINT_LINK,
};

struct scan_params {
	bool unknown;
	enum print_ie_type type;
	bool show_both_ie_sets;
};

static unsigned char wfa_oui[3] = { 0x50, 0x6f, 0x9a };
static unsigned char wifi_oui[3] = { 0x00, 0x50, 0xf2 };
static unsigned char brcm_oui[3] = { 0x00, 0x10, 0x18 };
static unsigned char mtik_oui[3] = { 0x00, 0x0c, 0x42 };
static unsigned char ieee80211_oui[3] = { 0x00, 0x0f, 0xac };

/* typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
*/

static struct site_survey_list *site_survey_lists;
