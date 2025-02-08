/*
 * mac80211site_survey.c 
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

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <glob.h>

#include <wlutils.h>
#include <unl.h>

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <ctype.h>
#include <ddnvram.h>
#include <shutils.h>
#include <wlutils.h>

#include <nl80211.h>

#include "mac80211site_survey.h"

static int sscount = 0;
static int rate_count = 0;

static int *noise;
static unsigned long long *active;
static unsigned long long *busy;

static const char *global_ifname;
static struct scan_params scan_params;

struct ie_print {
	const char *name;
	void (*print)(const uint8_t type, uint8_t len, const uint8_t *data);
	uint8_t minlen, maxlen;
	uint8_t flags;
};
static void print_ssid(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_mesh_ssid(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_supprates(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_ds(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_tim(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_rsn(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_ht_capa(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_ht_op(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_vht_capa(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_vht_oper(const uint8_t type, uint8_t len, const uint8_t *data);
static void print_capabilities(const uint8_t type, uint8_t len, const uint8_t *data);
static int print_bss_handler(struct nl_msg *msg, void *arg);
static void print_rsn_ie(const char *defcipher, const char *defauth, uint8_t len, const uint8_t *data, int type);
static void print_ies(unsigned char *ie, int ielen, bool unknown, enum print_ie_type ptype);

static void tab_on_first(bool *first);

static const struct ie_print ieprinters[] = {
	[0] = { "SSID", print_ssid, 0, 32, BIT(PRINT_SCAN) | BIT(PRINT_LINK), },
	[1] = { "Supported rates", print_supprates, 0, 255, BIT(PRINT_SCAN), },
	[3] = { "TIM", print_tim, 4, 255, BIT(PRINT_SCAN), },
	[3] = { "DS Parameter set", print_ds, 1, 1, BIT(PRINT_SCAN), },
	[45] = { "HT capabilities", print_ht_capa, 26, 26, BIT(PRINT_SCAN), },
	[61] = { "HT operation", print_ht_op, 22, 22, BIT(PRINT_SCAN), },
	[48] = { "RSN", print_rsn, 2, 255, BIT(PRINT_SCAN), },
	[50] = { "Extended supported rates", print_supprates, 0, 255, BIT(PRINT_SCAN), },
	[191] = { "VHT capabilities", print_vht_capa, 12, 255, BIT(PRINT_SCAN), },
	[192] = { "VHT operation", print_vht_oper, 5, 255, BIT(PRINT_SCAN), },
	[114] = { "MESH ID", print_mesh_ssid, 0, 32, BIT(PRINT_SCAN) | BIT(PRINT_LINK), },
};

static void fillENC(const char *text)
{
	char *buf;
	char var[64];
	char *next;
	buf = site_survey_lists[sscount].ENCINFO;
	foreach(var, buf, next)
	{
		if (!strcmp(var, text))
			return;
	}
	strspcattach(buf, text);
}

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
	[NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = { .type = NLA_U64 },
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = { .type = NLA_U64 },
};

static int parse_survey(struct nl_msg *msg, struct nlattr **sinfo)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_SURVEY_INFO])
		return -1;

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX, tb[NL80211_ATTR_SURVEY_INFO], survey_policy))
		return -1;

	if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY])
		return -1;

	return 0;
}

static int cb_survey(struct nl_msg *msg, void *data)
{
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	int freq;

	if (parse_survey(msg, sinfo))
		goto out;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	if (sinfo[NL80211_SURVEY_INFO_NOISE]) {
		int8_t lnoise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
		noise[freq] = lnoise;
		// int channel=ieee80211_mhz2ieee(freq);
		// noise[channel] = lnoise;
	} else {
		noise[freq] = -95;
	}
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME] && sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]) {
		active[freq] = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
		busy[freq] = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);
	} else {
		active[freq] = 0;
		busy[freq] = 0;
	}

out:
	return NL_SKIP;
}

static void lgetnoise(struct unl *unl, int wdev)
{
	struct nl_msg *surveymsg;

	surveymsg = unl_genl_msg(unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(surveymsg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(unl, surveymsg, cb_survey, NULL);
	return;

nla_put_failure:
	nlmsg_free(surveymsg);
	return;
}

static void print_mcs_index(const __u8 *mcs)
{
	unsigned int mcs_bit, prev_bit = -2, prev_cont = 0;

	for (mcs_bit = 0; mcs_bit <= 76; mcs_bit++) {
		unsigned int mcs_octet = mcs_bit / 8;
		unsigned int MCS_RATE_BIT = 1 << mcs_bit % 8;
		bool mcs_rate_idx_set;

		mcs_rate_idx_set = !!(mcs[mcs_octet] & MCS_RATE_BIT);

		if (!mcs_rate_idx_set)
			continue;

		if (prev_bit != mcs_bit - 1) {
			prev_cont = 0;
		} else if (!prev_cont) {
			prev_cont = 1;
		}
		if (mcs_bit != 32)
			prev_bit = mcs_bit;
		else
			prev_cont = 1;
	}
	if (prev_cont) {
		if (prev_bit == 7)
			rate_count = 150;
		if (prev_bit == 15)
			rate_count = 300;
		if (prev_bit == 23)
			rate_count = 450;
		if (prev_bit == 31)
			rate_count = 600;
	}
}

static void print_ht_mcs(const __u8 *mcs)
{
	site_survey_lists[sscount].extcap |= CAP_HT; // ht

	/* As defined in 7.3.2.57.4 Supported MCS Set field */
	unsigned int tx_max_num_spatial_streams, max_rx_supp_data_rate;
	bool tx_mcs_set_defined, tx_mcs_set_equal, tx_unequal_modulation;

	max_rx_supp_data_rate = ((mcs[10] >> 8) & ((mcs[11] & 0x3) << 8));
	tx_mcs_set_defined = !!(mcs[12] & (1 << 0));
	tx_mcs_set_equal = !(mcs[12] & (1 << 1));
	tx_max_num_spatial_streams = ((mcs[12] >> 2) & 3) + 1;
	tx_unequal_modulation = !!(mcs[12] & (1 << 4));

	/* XXX: else see 9.6.0e.5.3 how to get this I think */

	if (tx_mcs_set_defined) {
		if (tx_mcs_set_equal) {
			print_mcs_index(mcs);
		} else {
			print_mcs_index(mcs);
		}
	} else {
		print_mcs_index(mcs);
	}
}

static void print_wifi_wpa(const uint8_t type, uint8_t len, const uint8_t *data)
{
	print_rsn_ie("TKIP", "IEEE 802.1X", len, data, 0);
}

static void print_ssid(const uint8_t type, uint8_t len, const uint8_t *data)
{
	if (!(site_survey_lists[sscount].extcap & CAP_MESH))
		memcpy(site_survey_lists[sscount].SSID, data, len);
}

static void print_mesh_ssid(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].extcap |= CAP_MESH;
	memcpy(site_survey_lists[sscount].SSID, data, len);
}

static int print_bss_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *bss[NL80211_BSS_MAX + 1];
	char mac_addr[20], dev[20];
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_TSF] = { .type = NLA_U64 },
		[NL80211_BSS_FREQUENCY] = { .type = NLA_U32 },
		[NL80211_BSS_BSSID] = {},
		[NL80211_BSS_BEACON_INTERVAL] = { .type = NLA_U16 },
		[NL80211_BSS_CAPABILITY] = { .type = NLA_U16 },
		[NL80211_BSS_INFORMATION_ELEMENTS] = {},
		[NL80211_BSS_SIGNAL_MBM] = { .type = NLA_U32 },
		[NL80211_BSS_SIGNAL_UNSPEC] = { .type = NLA_U8 },
		[NL80211_BSS_STATUS] = { .type = NLA_U32 },
		[NL80211_BSS_SEEN_MS_AGO] = { .type = NLA_U32 },
		[NL80211_BSS_BEACON_IES] = {},
	};
	struct scan_params *params = arg;
	rate_count = 0;
	bzero(site_survey_lists[sscount].ENCINFO, 128);
	int show = params->show_both_ie_sets ? 2 : 1;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_BSS]) {
		fprintf(stderr, "bss info missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(bss, NL80211_BSS_MAX, tb[NL80211_ATTR_BSS], bss_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}

	if (!bss[NL80211_BSS_BSSID])
		return NL_SKIP;
	ether_etoa(nla_data(bss[NL80211_BSS_BSSID]), mac_addr);
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	strcpy(site_survey_lists[sscount].BSSID, mac_addr);

	if (bss[NL80211_BSS_FREQUENCY]) {
		site_survey_lists[sscount].frequency = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	}
	if (bss[NL80211_BSS_BEACON_INTERVAL]) {
		site_survey_lists[sscount].beacon_period = nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]);
	}
	if (bss[NL80211_BSS_CAPABILITY]) {
		__u16 capa = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
		site_survey_lists[sscount].capability = capa;
	}
	if (bss[NL80211_BSS_SIGNAL_MBM]) {
		int s = nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
		site_survey_lists[sscount].RSSI = s / 100;
	}
	if (bss[NL80211_BSS_SEEN_MS_AGO]) {
		int age = nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]);
	}

	if (bss[NL80211_BSS_INFORMATION_ELEMENTS] && show--) {
		print_ies(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]),
			  params->unknown, params->type);
	}
	if (bss[NL80211_BSS_BEACON_IES] && show--) {
		print_ies(nla_data(bss[NL80211_BSS_BEACON_IES]), nla_len(bss[NL80211_BSS_BEACON_IES]), params->unknown,
			  params->type);
	}
	site_survey_lists[sscount].rate_count = rate_count;
	int freq = site_survey_lists[sscount].frequency;
	site_survey_lists[sscount].phy_noise = noise[freq];
	site_survey_lists[sscount].active = active[freq];
	site_survey_lists[sscount].busy = busy[freq];
	if ((site_survey_lists[sscount].channel & 0xff) == 0) {
		site_survey_lists[sscount].channel |=
			(ieee80211_mhz2ieee(global_ifname, site_survey_lists[sscount].frequency) & 0xff);
	}
	sscount++;

	return NL_SKIP;
}

static void print_ie(const struct ie_print *p, const uint8_t type, uint8_t len, const uint8_t *data)
{
	int i;

	if (!p->print)
		return;

	if (len < p->minlen || len > p->maxlen) {
		return;
	}

	p->print(type, len, data);
}

static const struct ie_print wifiprinters[] = {
	[1] = { "WPA", print_wifi_wpa, 2, 255, BIT(PRINT_SCAN), },
};

static inline void print_wifi_owe(const uint8_t type, uint8_t len, const uint8_t *data)
{
	printf("OWE");
	fillENC("OWE");
}

static const struct ie_print wfa_printers[] = {
	[28] = { "OWE", print_wifi_owe, 1, 255, BIT(PRINT_SCAN), },
};

static void print_aironet(unsigned char len, unsigned char *data, bool unknown, enum print_ie_type ptype)
{
	struct aironet_ie *ie = (struct aironet_ie *)data;
	site_survey_lists[sscount].numsta = ie->num_assoc;
	memcpy(site_survey_lists[sscount].radioname, ie->name, 15);
}

static void print_vendor(unsigned char len, unsigned char *data, bool unknown, enum print_ie_type ptype)
{
	int i;

	if (len < 3) {
		return;
	}
	if (len >= 4 && !memcmp(data, brcm_oui, 3)) {
		if (data[3] == 2) {
			site_survey_lists[sscount].numsta = data[4];
			if (data[6] & 0x80)
				site_survey_lists[sscount].extcap |= CAP_DWDS;
		}
	}
	if (len >= 4 && !memcmp(data, mtik_oui, 3)) {
		struct ieee80211_mtik_ie *ie = (struct ieee80211_mtik_ie *)data;
		if (ie->iedata.namelen <= 15) {
			memcpy(site_survey_lists[sscount].radioname, ie->iedata.radioname, ie->iedata.namelen);
		}
		if (ie->iedata.flags & 0x4)
			site_survey_lists[sscount].extcap |= CAP_MTIKWDS;
	}
	if (len >= 4 && memcmp(data, wifi_oui, 3) == 0) {
		if (data[3] < ARRAY_SIZE(wifiprinters) && wifiprinters[data[3]].name && wifiprinters[data[3]].flags & BIT(ptype)) {
			print_ie(&wifiprinters[data[3]], data[3], len - 4, data + 4);
			return;
		}
		if (!unknown)
			return;
		return;
	}

	if (len >= 4 && memcmp(data, wfa_oui, 3) == 0) {
		if (data[3] < ARRAY_SIZE(wfa_printers) && wfa_printers[data[3]].name && wfa_printers[data[3]].flags & BIT(ptype)) {
			print_ie(&wfa_printers[data[3]], data[3], len - 4, data + 4);
			return;
		}
		if (!unknown)
			return;
		return;
	}

	if (!unknown)
		return;
}
static void __print_he_capa(const __u16 *mac_cap, const __u16 *phy_cap, const __u16 *mcs_set, size_t mcs_len, const __u8 *ppet,
			    int ppet_len, bool indent)
{
	size_t mcs_used;
	const char *pre = indent ? "\t" : "";

	if (phy_cap[0] & BIT(1 + 8)) {
		site_survey_lists[sscount].channel |= 0x1000;
		fillENC("HE40");
	}
	if (phy_cap[0] & BIT(2 + 8)) {
		site_survey_lists[sscount].channel |= 0x1000;
		site_survey_lists[sscount].channel |= 0x100;
		fillENC("HE40");
		fillENC("HE80");
	}
	if (phy_cap[0] & BIT(3 + 8)) {
		site_survey_lists[sscount].channel |= 0x200;
		fillENC("HE160");
	}
	if (phy_cap[0] & BIT(4 + 8)) {
		site_survey_lists[sscount].channel |= 0x200;
		fillENC("HE80+80");
	}

	int antennacount = 0;
	mcs_used = 0;
	__u8 phy_cap_support[] = { BIT(1) | BIT(2), BIT(3), BIT(4) };
	/* Supports more, but overflow? Abort. */
	if (2 * sizeof(mcs_set[0]) >= mcs_len)
		return;

	int k;
	for (k = 0; k < 8; k++) {
		__u16 mcs = mcs_set[0];
		mcs >>= k * 2;
		mcs &= 0x3;
		if (mcs != 3) {
			antennacount++;
		}
	}
	rate_count = 150 * antennacount;
}

void print_he_capability(const uint8_t *ie, int len)
{
	const void *mac_cap, *phy_cap, *mcs_set;
	int mcs_len;
	int i = 0;

	mac_cap = &ie[i];
	i += 6;

	phy_cap = &ie[i];
	i += 11;

	mcs_set = &ie[i];
	mcs_len = len - i;

	__print_he_capa(mac_cap, phy_cap - 1, mcs_set, mcs_len, NULL, 0, false);
}

void print_he_operation(const uint8_t *ie, int len)
{
	uint8_t oper_parameters[3] = { ie[0], ie[1], ie[2] };
	uint8_t bss_color = ie[3];
	uint16_t nss_mcs_set = *(uint16_t *)(&ie[4]);
	uint8_t vht_oper_present = oper_parameters[1] & 0x40;
	uint8_t co_hosted_bss_present = oper_parameters[1] & 0x80;
	uint8_t uhb_operation_info_present = oper_parameters[2] & 0x02;
	uint8_t offset = 6;

	if (uhb_operation_info_present) {
		if (len - offset < 5) {
			return;
		} else {
			const uint8_t control = ie[offset + 1];

			switch (control & 0x3) {
			case 0:
				break;
			case 1:
				site_survey_lists[sscount].channel |= 0x1000;
				break;
			case 2:
				site_survey_lists[sscount].channel |= 0x1100;
				break;
			case 3:
				site_survey_lists[sscount].channel |= 0x1200;

				break;
			}
		}
	}
}

static void print_he_capa(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].extcap |= CAP_AX; // AX capable
	print_he_capability(data, len);
}

static void print_he_oper(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].extcap |= CAP_AX; // AX capable
	print_he_operation(data, len);
}

static const struct ie_print ext_printers[] = {
	[35] = { "HE capabilities", print_he_capa, 21, 54, BIT(PRINT_SCAN), },
	[36] = { "HE Operation", print_he_oper, 6, 15, BIT(PRINT_SCAN), },
};

static void print_extension(unsigned char len, unsigned char *ie, bool unknown, enum print_ie_type ptype)
{
	unsigned char tag;

	if (len < 1) {
		return;
	}

	tag = ie[0];
	if (tag < ARRAY_SIZE(ext_printers) && ext_printers[tag].name && ext_printers[tag].flags & BIT(ptype)) {
		print_ie(&ext_printers[tag], tag, len - 1, ie + 1);
		return;
	}
}

static void print_ies(unsigned char *ie, int ielen, bool unknown, enum print_ie_type ptype)
{
	while (ielen >= 2 && ielen >= ie[1]) {
		if (ie[0] < ARRAY_SIZE(ieprinters) && ieprinters[ie[0]].name && ieprinters[ie[0]].flags & BIT(ptype)) {
			print_ie(&ieprinters[ie[0]], ie[0], ie[1], ie + 2);
		} else if (ie[0] == 221 /* vendor */) {
			print_vendor(ie[1], ie + 2, unknown, ptype);
		} else if (ie[0] == 255 /* extension */) {
			print_extension(ie[1], ie + 2, unknown, ptype);
		} else if (ie[0] == 133 /* vendor */) {
			print_aironet(ie[1], ie + 2, unknown, ptype);
		} else if (unknown) {
			int i;
		}
		ielen -= ie[1] + 2;
		ie += ie[1] + 2;
	}
}

static void print_supprates(const uint8_t type, uint8_t len, const uint8_t *data)
{
	int i;

	for (i = 0; i < len; i++) {
		int r = data[i] & 0x7f;
		rate_count++;
	}
}

static void print_ds(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].channel = data[0];
}

static void print_tim(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].dtim_period = data[1];
}

static char *ciphers[] = { "Use group cipher suite", "WEP-40",	    "TKIP",	"",	    "CCMP",	"WEP-104",
			   "AES-128-CMAC",	     "NO-GROUP",    "GCMP-128", "GCMP-256", "CCMP-256", "AES-128-GMAC",
			   "AES-256-GMAC",	     "AES-256-CMAC" };

static void print_cipher(const uint8_t *data)
{
	if (memcmp(data, wifi_oui, 3) == 0) {
		if (data[3] < 6)
			fillENC(ciphers[data[3]]);
	} else if (memcmp(data, ieee80211_oui, 3) == 0) {
		if (data[3] < 14)
			fillENC(ciphers[data[3]]);
	}
}

static void print_auth(const uint8_t *data, int type)
{
	if (memcmp(data, ieee80211_oui, 3) == 0 || memcmp(data, wifi_oui, 3) == 0) {
		switch (data[3]) {
		case 1:
			if (type)
				fillENC("EAP/WPA2");
			else
				fillENC("EAP/WPA");
			break;
			break;
		case 2:
			if (type)
				fillENC("PSK2");
			else
				fillENC("PSK");
			break;
		case 3:
			fillENC("FT/EAP");
			break;
		case 4:
			fillENC("FT/PSK");
			break;
		case 5:
			fillENC("EAP/SHA-256");
			break;
		case 6:
			fillENC("PSK/SHA-256");
			break;
		case 7:
			fillENC("TDLS/TPK");
			break;
		case 8:
			fillENC("SAE/PSK3");
			break;
		case 9:
			fillENC("FT/SAE");
			break;
		case 11:
			fillENC("EAP/SUITE-B");
			break;
		case 12:
			fillENC("EAP/SUITE-B-192");
			break;
		case 14:
			fillENC("FILS/SHA256");
			break;
		case 15:
			fillENC("FILS/SHA384");
			break;
		case 16:
			fillENC("FT-FILS/SHA256");
			break;
		case 17:
			fillENC("FT-FILS/SHA384");
			break;
		case 18:
			fillENC("OWE");
			break;
		default:
			break;
		}
	}
}

static void print_rsn_ie(const char *defcipher, const char *defauth, uint8_t len, const uint8_t *data, int type)
{
	bool first = true;
	__u16 version, count, capa;
	int i;

	version = data[0] + (data[1] << 8);

	data += 2;
	len -= 2;

	if (len < 4) {
		fillENC(defcipher);
		return;
	}

	print_cipher(data);

	data += 4;
	len -= 4;

	if (len < 2) {
		return;
	}

	count = data[0] | (data[1] << 8);
	if (2 + (count * 4) > len)
		goto invalid;

	for (i = 0; i < count; i++) {
		print_cipher(data + 2 + (i * 4));
	}

	data += 2 + (count * 4);
	len -= 2 + (count * 4);

	if (len < 2) {
		fillENC(defauth);
		return;
	}

	count = data[0] | (data[1] << 8);
	if (2 + (count * 4) > len)
		goto invalid;

	for (i = 0; i < count; i++) {
		print_auth(data + 2 + (i * 4), type);
	}

	data += 2 + (count * 4);
	len -= 2 + (count * 4);

	if (len >= 2) {
		capa = data[0] | (data[1] << 8);
		data += 2;
		len -= 2;
	}

	if (len >= 2) {
		int pmkid_count = data[0] | (data[1] << 8);

		if (len >= 2 + 16 * pmkid_count) {
			/* not printing PMKID values */
			data += 2 + 16 * pmkid_count;
			len -= 2 + 16 * pmkid_count;
		} else
			goto invalid;
	}

	if (len >= 4) {
		print_cipher(data);
		data += 4;
		len -= 4;
	}

invalid:
}

static void print_rsn(const uint8_t type, uint8_t len, const uint8_t *data)
{
	print_rsn_ie("CCMP", "IEEE 802.1X", len, data, 1);
}

void print_vht_info(__u32 capa, const __u8 *mcs)
{
	__u16 tmp;
	int i;
	site_survey_lists[sscount].extcap |= CAP_VHT; // vht

	switch ((capa >> 2) & 3) {
	case 0:
		if (capa & BIT(5)) {
			fillENC("VHT80SGI");
		} else {
			fillENC("VHT80");
		}
		break;
	case 1:
		if (capa & BIT(6)) {
			fillENC("VHT160SGI");
		} else {
			fillENC("VHT160");
		}
		site_survey_lists[sscount].channel |= 0x1100;
		break;
	case 2:
		fillENC("VHT160 VHT80+80");
		site_survey_lists[sscount].channel |= 0x1200;
		break;
	case 3:
	}

	tmp = mcs[4] | (mcs[5] << 8);
	int antennacount = 0;
	for (i = 1; i <= 8; i++) {
		antennacount++;
		switch ((tmp >> ((i - 1) * 2)) & 3) {
		case 0:
		case 1:
		case 2:
			antennacount++;
			break;
		case 3:
			break;
		}
	}
	rate_count = 150 * antennacount;
}

static void print_vht_capa(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].extcap |= CAP_VHT; // vht
	print_vht_info(data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24), data + 4);
}

static void print_vht_oper(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].channel |= 0x1000; //20 or 40
	if (data[0] == 1)
		site_survey_lists[sscount].channel |= 0x100; //80
	if (data[0] == 3 || data[0] == 2)
		site_survey_lists[sscount].channel |= 0x200; //80+80 or 160

	site_survey_lists[sscount].extcap |= CAP_VHT; // vht
}

static void print_ht_capability(__u16 cap)
{
	site_survey_lists[sscount].extcap |= CAP_HT; // vht

	if (cap & BIT(1)) {
		site_survey_lists[sscount].channel |= 0x2000;
		if ((cap & BIT(6))) {
			fillENC("HT40SGI");
		} else {
			fillENC("HT40");
		}
	}
	if (!(cap & BIT(1))) {
		if ((cap & BIT(5))) {
			fillENC("HT20SGI");
		} else {
			fillENC("HT20");
		}
	}
}

static void print_ht_capa(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].extcap |= CAP_HT; // vht
	print_ht_capability(data[0] | (data[1] << 8));
	print_ht_mcs(data + 3);
}

static void print_ht_op(const uint8_t type, uint8_t len, const uint8_t *data)
{
	site_survey_lists[sscount].channel |= 0x1000; //20 or 40
	site_survey_lists[sscount].extcap |= CAP_HT; // ht
	if (data[1] & 0x3)
		site_survey_lists[sscount].extcap |= CAP_SECCHANNEL; // sec channel available
}

//  end of iw copied code
void mac80211_scan(struct unl *unl, char *interface)
{
	struct nl_msg *msg;
	int wdev;
	wdev = if_nametoindex(interface);
	lgetnoise(unl, wdev);
	bzero(&scan_params, sizeof(scan_params));
	scan_params.type = PRINT_SCAN;
	msg = unl_genl_msg(unl, NL80211_CMD_GET_SCAN, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(unl, msg, print_bss_handler, &scan_params);
	// nlmsg_free(surveymsg);
	return;
nla_put_failure:
	nlmsg_free(msg);
}

static int write_site_survey(void);
static int local_open_site_survey(void);

void mac80211_site_survey(char *interface)
{
	noise = malloc(7000 * sizeof(int));
	active = malloc(7000 * sizeof(unsigned long long));
	busy = malloc(7000 * sizeof(unsigned long long));
	struct unl unl;
	unl_genl_init(&unl, "nl80211");
	site_survey_lists = malloc(sizeof(struct site_survey_list) * SITE_SURVEY_NUM);
	int i;
	int phy, wdev;
	char scaninterface[32];
	char macaddr[32];
	unsigned char hwbuff[16];
	bzero(site_survey_lists, sizeof(struct site_survey_list) * SITE_SURVEY_NUM);
	for (i = 0; i < SITE_SURVEY_NUM; i++) {
		site_survey_lists[i].numsta = -1;
		site_survey_lists[i].extcap = 0;
	}
	sysprintf("iw dev %s scan>/dev/null 2>&1", interface);
	mac80211_scan(&unl, interface);
	write_site_survey();
	local_open_site_survey();
	for (i = 0; i < SITE_SURVEY_NUM && site_survey_lists[i].frequency; i++) {
		if (site_survey_lists[i].SSID[0] == 0) {
			strcpy(site_survey_lists[i].SSID, "hidden");
		}

		fprintf(stderr,
			"[%2d] SSID[%20s] BSSID[%s] channel[%2d/%4d] frequency[%4d] numsta[%d] rssi[%d] noise[%d] active[%llu] busy[%llu] quality[%llu] beacon[%d] cap[%x] dtim[%d] rate[%d] enc[%s] extcap[0x%02X]\n",
			i, site_survey_lists[i].SSID, site_survey_lists[i].BSSID, site_survey_lists[i].channel & 0xff,
			site_survey_lists[i].channel, site_survey_lists[i].frequency, site_survey_lists[i].numsta,
			site_survey_lists[i].RSSI, site_survey_lists[i].phy_noise, site_survey_lists[i].active,
			site_survey_lists[i].busy,
			site_survey_lists[i].active ? (100 - (site_survey_lists[i].busy * 100 / site_survey_lists[i].active)) : 100,
			site_survey_lists[i].beacon_period, site_survey_lists[i].capability, site_survey_lists[i].dtim_period,
			site_survey_lists[i].rate_count, site_survey_lists[i].ENCINFO, site_survey_lists[i].extcap);
	}
	free(site_survey_lists);
	unl_free(&unl);
	free(busy);
	free(active);
	free(noise);
}

static int write_site_survey(void)
{
	FILE *fp;
	if ((fp = fopen(SITE_SURVEY_DB, "w"))) {
		fwrite(&site_survey_lists[0], sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1, fp);
		fclose(fp);
		return 0;
	}
	return 1;
}

static int local_open_site_survey(void)
{
	FILE *fp;
	bzero(site_survey_lists, sizeof(site_survey_lists) * SITE_SURVEY_NUM);
	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&site_survey_lists[0], sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1, fp);
		fclose(fp);
		return 1;
	}
	return 0;
}

int site_survey_main_mac802211(int argc, char *argv[])
{
	unlink(SITE_SURVEY_DB);
	global_ifname = argc > 1 ? argv[1] : nvram_safe_get("wifi_display");
	mac80211_site_survey(global_ifname);
	return 0;
}
