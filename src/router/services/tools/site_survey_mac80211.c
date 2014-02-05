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

#include "wlutils.h"
#include "unl.h"

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
#include <bcmnvram.h>

#include "linux/nl80211.h"

#include "mac80211site_survey.h"

static struct unl unl;

static void __attribute__((constructor)) mac80211_init(void)
{
	unl_genl_init(&unl, "nl80211");
}

static int sscount = 0;
static int rate_count = 0;

static int noise[6075];

static struct scan_params scan_params;

struct ie_print {
	const char *name;
	void (*print) (const uint8_t type, uint8_t len, const uint8_t * data);
	uint8_t minlen, maxlen;
	uint8_t flags;
};
static void print_ssid(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_supprates(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_ds(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_tim(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_country(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_powerconstraint(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_rsn(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_erp(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_ht_capa(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_ht_op(const uint8_t type, uint8_t len, const uint8_t * data);
static void print_capabilities(const uint8_t type, uint8_t len, const uint8_t * data);
static int print_bss_handler(struct nl_msg *msg, void *arg);
static void print_rsn_ie(const char *defcipher, const char *defauth, uint8_t len, const uint8_t * data);
static void print_ies(unsigned char *ie, int ielen, bool unknown, enum print_ie_type ptype);

static void tab_on_first(bool * first);

static const struct ie_print ieprinters[] = {
	[0] = {"SSID", print_ssid, 0, 32, BIT(PRINT_SCAN) | BIT(PRINT_LINK),},
	[1] = {"Supported rates", print_supprates, 0, 255, BIT(PRINT_SCAN),},
	[3] = {"DS Parameter set", print_ds, 1, 1, BIT(PRINT_SCAN),},
	[5] = {"TIM", print_tim, 4, 255, BIT(PRINT_SCAN),},
	[7] = {"Country", print_country, 3, 255, BIT(PRINT_SCAN),},
	[32] = {"Power constraint", print_powerconstraint, 1, 1, BIT(PRINT_SCAN),},
	[42] = {"ERP", print_erp, 1, 255, BIT(PRINT_SCAN),},
	[45] = {"HT capabilities", print_ht_capa, 26, 26, BIT(PRINT_SCAN),},
	[61] = {"HT operation", print_ht_op, 22, 22, BIT(PRINT_SCAN),},
	[48] = {"RSN", print_rsn, 2, 255, BIT(PRINT_SCAN),},
	[50] = {"Extended supported rates", print_supprates, 0, 255,
		BIT(PRINT_SCAN),},
	[127] = {"Extended capabilities", print_capabilities, 0, 255,
		 BIT(PRINT_SCAN),},
};

static void fillENC(const char *text, const char *space)
{
	char *buf;
	buf = site_survey_lists[sscount].ENCINFO;
	buf += strlen(buf);
	buf += sprintf(buf, "%s%s", text, space);
}

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = {.type = NLA_U32},
	[NL80211_SURVEY_INFO_NOISE] = {.type = NLA_U8},
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = {.type = NLA_U64},
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = {.type = NLA_U64},
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
	}

out:
	return NL_SKIP;
}

static void lgetnoise(int wdev)
{
	static struct nl_msg *surveymsg;

	surveymsg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(surveymsg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, surveymsg, cb_survey, NULL);
	return;

nla_put_failure:
	nlmsg_free(surveymsg);
	return;
}

static __u32 compute_ampdu_length(__u8 exponent)
{
	switch (exponent) {
	case 0:
		return 8191;	/* (2 ^(13 + 0)) -1 */
	case 1:
		return 16383;	/* (2 ^(13 + 1)) -1 */
	case 2:
		return 32767;	/* (2 ^(13 + 2)) -1 */
	case 3:
		return 65535;	/* (2 ^(13 + 3)) -1 */
	default:
		return 0;
	}
}

static const char *print_ampdu_space(__u8 space)
{
	switch (space) {
	case 0:
		return "No restriction";
	case 1:
		return "1/4 usec";
	case 2:
		return "1/2 usec";
	case 3:
		return "1 usec";
	case 4:
		return "2 usec";
	case 5:
		return "4 usec";
	case 6:
		return "8 usec";
	case 7:
		return "16 usec";
	default:
		return "BUG (spacing more than 3 bits!)";
	}
}

void print_ampdu_length(__u8 exponent)
{
	__u32 max_ampdu_length;

	max_ampdu_length = compute_ampdu_length(exponent);

	if (max_ampdu_length) {
		printf("\t\tMaximum RX AMPDU length %d bytes (exponent: 0x0%02x)\n", max_ampdu_length, exponent);
	} else {
		printf("\t\tMaximum RX AMPDU length: unrecognized bytes " "(exponent: %d)\n", exponent);
	}
}

void print_ampdu_spacing(__u8 spacing)
{
	printf("\t\tMinimum RX AMPDU time spacing: %s (0x%02x)\n", print_ampdu_space(spacing), spacing);
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
			if (prev_bit != -2)
				printf("%d, ", prev_bit);
			else
				printf(" ");
			printf("%d", mcs_bit);
			prev_cont = 0;
		} else if (!prev_cont) {
			printf("-");
			prev_cont = 1;
		}

		prev_bit = mcs_bit;
	}

	if (prev_cont) {
		printf("%d", prev_bit);
		if (prev_bit == 7)
			rate_count = 150;
		if (prev_bit == 15)
			rate_count = 300;
		if (prev_bit == 23)
			rate_count = 450;
	}
	printf("\n");
}

static void print_ht_mcs(const __u8 *mcs)
{
	/* As defined in 7.3.2.57.4 Supported MCS Set field */
	unsigned int tx_max_num_spatial_streams, max_rx_supp_data_rate;
	bool tx_mcs_set_defined, tx_mcs_set_equal, tx_unequal_modulation;

	max_rx_supp_data_rate = ((mcs[10] >> 8) & ((mcs[11] & 0x3) << 8));
	tx_mcs_set_defined = !!(mcs[12] & (1 << 0));
	tx_mcs_set_equal = !(mcs[12] & (1 << 1));
	tx_max_num_spatial_streams = ((mcs[12] >> 2) & 3) + 1;
	tx_unequal_modulation = !!(mcs[12] & (1 << 4));

	if (max_rx_supp_data_rate)
		printf("\t\tHT Max RX data rate: %d Mbps\n", max_rx_supp_data_rate);
	/* XXX: else see 9.6.0e.5.3 how to get this I think */

	if (tx_mcs_set_defined) {
		if (tx_mcs_set_equal) {
			printf("\t\tHT TX/RX MCS rate indexes supported:");
			print_mcs_index(mcs);
		} else {
			printf("\t\tHT RX MCS rate indexes supported:");
			print_mcs_index(mcs);

			if (tx_unequal_modulation)
				printf("\t\tTX unequal modulation supported\n");
			else
				printf("\t\tTX unequal modulation not supported\n");

			printf("\t\tHT TX Max spatial streams: %d\n", tx_max_num_spatial_streams);

			printf("\t\tHT TX MCS rate indexes supported may differ\n");
		}
	} else {
		printf("\t\tHT RX MCS rate indexes supported:");
		print_mcs_index(mcs);
		printf("\t\tHT TX MCS rate indexes are undefined\n");
	}
}

static void print_wifi_wpa(const uint8_t type, uint8_t len, const uint8_t * data)
{
	fillENC("WPA", " ");
	print_rsn_ie("TKIP", "IEEE 802.1X", len, data);
}

static bool print_wifi_wmm_param(const uint8_t * data, uint8_t len)
{
	int i;
	static const char *aci_tbl[] = { "BE", "BK", "VI", "VO" };

	if (len < 19)
		goto invalid;

	if (data[0] != 1) {
		printf("Parameter: not version 1: ");
		return false;
	}

	printf("\t * Parameter version 1");

	data++;

	if (data[0] & 0x80)
		printf("\n\t\t * u-APSD");

	data += 2;

	for (i = 0; i < 4; i++) {
		printf("\n\t\t * %s:", aci_tbl[(data[0] >> 5) & 3]);
		if (data[4] & 0x10)
			printf(" acm");
		printf(" CW %d-%d", (1 << (data[1] & 0xf)) - 1, (1 << (data[1] >> 4)) - 1);
		printf(", AIFSN %d", data[0] & 0xf);
		if (data[2] | data[3])
			printf(", TXOP %d usec", (data[2] + (data[3] << 8)) * 32);
		data += 4;
	}

	printf("\n");
	return true;

invalid:
	printf("invalid: ");
	return false;
}

static void print_wifi_wmm(const uint8_t type, uint8_t len, const uint8_t * data)
{
	int i;

	switch (data[0]) {
	case 0x00:
		printf(" information:");
		break;
	case 0x01:
		if (print_wifi_wmm_param(data + 1, len - 1))
			return;
		break;
	default:
		printf(" type %d:", data[0]);
		break;
	}

	for (i = 1; i < len; i++)
		printf(" %.02x", data[i]);
	printf("\n");
}

static const char *wifi_wps_dev_passwd_id(uint16_t id)
{
	switch (id) {
	case 0:
		return "Default (PIN)";
	case 1:
		return "User-specified";
	case 2:
		return "Machine-specified";
	case 3:
		return "Rekey";
	case 4:
		return "PushButton";
	case 5:
		return "Registrar-specified";
	default:
		return "??";
	}
}

static void print_wifi_wps(const uint8_t type, uint8_t len, const uint8_t * data)
{
	bool first = true;
	__u16 subtype, sublen;

	while (len >= 4) {
		subtype = (data[0] << 8) + data[1];
		sublen = (data[2] << 8) + data[3];
		if (sublen > len)
			break;

		switch (subtype) {
		case 0x104a:
			tab_on_first(&first);
			printf("\t * Version: %d.%d\n", data[4] >> 4, data[4] & 0xF);
			break;
		case 0x1011:
			tab_on_first(&first);
			printf("\t * Device name: %.*s\n", sublen, data + 4);
			break;
		case 0x1012:{
				uint16_t id;
				tab_on_first(&first);
				if (sublen != 2) {
					printf("\t * Device Password ID: (invalid " "length %d)\n", sublen);
					break;
				}
				id = data[4] << 8 | data[5];
				printf("\t * Device Password ID: %u (%s)\n", id, wifi_wps_dev_passwd_id(id));
				break;
			}
		case 0x1021:
			tab_on_first(&first);
			printf("\t * Manufacturer: %.*s\n", sublen, data + 4);
			break;
		case 0x1023:
			tab_on_first(&first);
			printf("\t * Model: %.*s\n", sublen, data + 4);
			break;
		case 0x1024:
			tab_on_first(&first);
			printf("\t * Model Number: %.*s\n", sublen, data + 4);
			break;
		case 0x103b:{
				__u8 val = data[4];
				tab_on_first(&first);
				printf("\t * Response Type: %d%s\n", val, val == 3 ? " (AP)" : "");
				break;
			}
		case 0x103c:{
				__u8 val = data[4];
				tab_on_first(&first);
				printf("\t * RF Bands: 0x%x\n", val);
				break;
			}
		case 0x1041:{
				__u8 val = data[4];
				tab_on_first(&first);
				printf("\t * Selected Registrar: 0x%x\n", val);
				break;
			}
		case 0x1042:
			tab_on_first(&first);
			printf("\t * Serial Number: %.*s\n", sublen, data + 4);
			break;
		case 0x1044:{
				__u8 val = data[4];
				tab_on_first(&first);
				printf("\t * Wi-Fi Protected Setup State: %d%s%s\n", val, val == 1 ? " (Unconfigured)" : "", val == 2 ? " (Configured)" : "");
				break;
			}
		case 0x1054:{
				tab_on_first(&first);
				if (sublen != 8) {
					printf("\t * Primary Device Type: (invalid " "length %d)\n", sublen);
					break;
				}
				printf("\t * Primary Device Type: " "%u-%02x%02x%02x%02x-%u\n", data[4] << 8 | data[5], data[6], data[7], data[8], data[9], data[10] << 8 | data[11]);
				break;
			}
		case 0x1057:{
				__u8 val = data[4];
				tab_on_first(&first);
				printf("\t * AP setup locked: 0x%.2x\n", val);
				break;
			}
		case 0x1008:
		case 0x1053:{
				__u16 meth = (data[4] << 8) + data[5];
				bool comma = false;
				tab_on_first(&first);
				printf("\t * %sConfig methods:", subtype == 0x1053 ? "Selected Registrar " : "");
#define T(bit, name) do {		\
	if (meth & (1<<bit)) {		\
		if (comma)		\
			printf(",");	\
		comma = true;		\
		printf(" " name);	\
	} } while (0)
				T(0, "USB");
				T(1, "Ethernet");
				T(2, "Label");
				T(3, "Display");
				T(4, "Ext. NFC");
				T(5, "Int. NFC");
				T(6, "NFC Intf.");
				T(7, "PBC");
				T(8, "Keypad");
				printf("\n");
				break;
#undef T
			}
		default:{
				const __u8 *subdata = data + 4;
				__u16 tmplen = sublen;

				tab_on_first(&first);
				printf("\t * Unknown TLV (%#.4x, %d bytes):", subtype, tmplen);
				while (tmplen) {
					printf(" %.2x", *subdata);
					subdata++;
					tmplen--;
				}
				printf("\n");
				break;
			}
		}

		data += sublen + 4;
		len -= sublen + 4;
	}

	if (len != 0) {
		printf("\t\t * bogus tail data (%d):", len);
		while (len) {
			printf(" %.2x", *data);
			data++;
			len--;
		}
		printf("\n");
	}
}

static void tab_on_first(bool * first)
{
	if (!*first)
		printf("\t");
	else
		*first = false;
}

static void print_ssid(const uint8_t type, uint8_t len, const uint8_t * data)
{
	memcpy(site_survey_lists[sscount].SSID, data, len);
}

static int print_bss_handler(struct nl_msg *msg, void *arg)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *bss[NL80211_BSS_MAX + 1];
	char mac_addr[20], dev[20];
	static struct nla_policy bss_policy[NL80211_BSS_MAX + 1] = {
		[NL80211_BSS_TSF] = {.type = NLA_U64},
		[NL80211_BSS_FREQUENCY] = {.type = NLA_U32},
		[NL80211_BSS_BSSID] = {},
		[NL80211_BSS_BEACON_INTERVAL] = {.type = NLA_U16},
		[NL80211_BSS_CAPABILITY] = {.type = NLA_U16},
		[NL80211_BSS_INFORMATION_ELEMENTS] = {},
		[NL80211_BSS_SIGNAL_MBM] = {.type = NLA_U32},
		[NL80211_BSS_SIGNAL_UNSPEC] = {.type = NLA_U8},
		[NL80211_BSS_STATUS] = {.type = NLA_U32},
		[NL80211_BSS_SEEN_MS_AGO] = {.type = NLA_U32},
		[NL80211_BSS_BEACON_IES] = {},
	};
	struct scan_params *params = arg;
	rate_count = 0;
	memset(site_survey_lists[sscount].ENCINFO, 0, 128);
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
	printf("BSS %s (on %s)", mac_addr, dev);
	strcpy(site_survey_lists[sscount].BSSID, mac_addr);

	if (bss[NL80211_BSS_STATUS]) {
		switch (nla_get_u32(bss[NL80211_BSS_STATUS])) {
		case NL80211_BSS_STATUS_AUTHENTICATED:
			printf(" -- authenticated");
			break;
		case NL80211_BSS_STATUS_ASSOCIATED:
			printf(" -- associated");
			break;
		case NL80211_BSS_STATUS_IBSS_JOINED:
			printf(" -- joined");
			break;
		default:
			printf(" -- unknown status: %d", nla_get_u32(bss[NL80211_BSS_STATUS]));
			break;
		}
	}
	printf("\n");

	if (bss[NL80211_BSS_TSF]) {
		unsigned long long tsf;
		tsf = (unsigned long long)nla_get_u64(bss[NL80211_BSS_TSF]);
		printf("\tTSF: %llu usec (%llud, %.2lld:%.2llu:%.2llu)\n", tsf, tsf / 1000 / 1000 / 60 / 60 / 24, (tsf / 1000 / 1000 / 60 / 60) % 24, (tsf / 1000 / 1000 / 60) % 60, (tsf / 1000 / 1000) % 60);
	}
	if (bss[NL80211_BSS_FREQUENCY]) {
		printf("\tfreq: %d\n", nla_get_u32(bss[NL80211_BSS_FREQUENCY]));
		site_survey_lists[sscount].frequency = nla_get_u32(bss[NL80211_BSS_FREQUENCY]);
	}
	if (bss[NL80211_BSS_BEACON_INTERVAL]) {
		printf("\tbeacon interval: %d\n", nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]));
		site_survey_lists[sscount].beacon_period = nla_get_u16(bss[NL80211_BSS_BEACON_INTERVAL]);
	}
	if (bss[NL80211_BSS_CAPABILITY]) {
		__u16 capa = nla_get_u16(bss[NL80211_BSS_CAPABILITY]);
		site_survey_lists[sscount].capability = capa;
		printf("\tcapability:");
		if (capa & WLAN_CAPABILITY_ESS)
			printf(" ESS");
		if (capa & WLAN_CAPABILITY_IBSS)
			printf(" IBSS");
		if (capa & WLAN_CAPABILITY_PRIVACY)
			printf(" Privacy");
		if (capa & WLAN_CAPABILITY_SHORT_PREAMBLE)
			printf(" ShortPreamble");
		if (capa & WLAN_CAPABILITY_PBCC)
			printf(" PBCC");
		if (capa & WLAN_CAPABILITY_CHANNEL_AGILITY)
			printf(" ChannelAgility");
		if (capa & WLAN_CAPABILITY_SPECTRUM_MGMT)
			printf(" SpectrumMgmt");
		if (capa & WLAN_CAPABILITY_QOS)
			printf(" QoS");
		if (capa & WLAN_CAPABILITY_SHORT_SLOT_TIME)
			printf(" ShortSlotTime");
		if (capa & WLAN_CAPABILITY_APSD)
			printf(" APSD");
		if (capa & WLAN_CAPABILITY_DSSS_OFDM)
			printf(" DSSS-OFDM");
		printf(" (0x%.4x)\n", capa);
	}
	if (bss[NL80211_BSS_SIGNAL_MBM]) {
		int s = nla_get_u32(bss[NL80211_BSS_SIGNAL_MBM]);
		printf("\tsignal: %d.%.2d dBm\n", s / 100, s % 100);
		site_survey_lists[sscount].RSSI = s / 100;
	}
	if (bss[NL80211_BSS_SIGNAL_UNSPEC]) {
		unsigned char s = nla_get_u8(bss[NL80211_BSS_SIGNAL_UNSPEC]);
		printf("\tsignal: %d/100\n", s);
	}
	if (bss[NL80211_BSS_SEEN_MS_AGO]) {
		int age = nla_get_u32(bss[NL80211_BSS_SEEN_MS_AGO]);
		printf("\tlast seen: %d ms ago\n", age);
	}

	if (bss[NL80211_BSS_INFORMATION_ELEMENTS] && show--) {
		if (bss[NL80211_BSS_BEACON_IES])
			printf("\tInformation elements from Probe Response " "frame:\n");
		print_ies(nla_data(bss[NL80211_BSS_INFORMATION_ELEMENTS]), nla_len(bss[NL80211_BSS_INFORMATION_ELEMENTS]), params->unknown, params->type);
	}
	if (bss[NL80211_BSS_BEACON_IES] && show--) {
		printf("\tInformation elements from Beacon frame:\n");
		print_ies(nla_data(bss[NL80211_BSS_BEACON_IES]), nla_len(bss[NL80211_BSS_BEACON_IES]), params->unknown, params->type);
	}
	site_survey_lists[sscount].rate_count = rate_count;
	int freq = site_survey_lists[sscount].frequency;
	site_survey_lists[sscount].phy_noise = noise[freq];
	if (site_survey_lists[sscount].channel == 0) {
		site_survey_lists[sscount].channel = ieee80211_mhz2ieee(site_survey_lists[sscount].frequency);
	}
	sscount++;

	return NL_SKIP;
}

static const char *country_env_str(char environment)
{
	switch (environment) {
	case 'I':
		return "Indoor only";
	case 'O':
		return "Outdoor only";
	case ' ':
		return "Indoor/Outdoor";
	default:
		return "bogus";
	}
}

static void print_ie(const struct ie_print *p, const uint8_t type, uint8_t len, const uint8_t * data)
{
	int i;

	if (!p->print)
		return;

	printf("\t%s:", p->name);
	if (len < p->minlen || len > p->maxlen) {
		if (len > 1) {
			printf(" <invalid: %d bytes:", len);
			for (i = 0; i < len; i++)
				printf(" %.02x", data[i]);
			printf(">\n");
		} else if (len)
			printf(" <invalid: 1 byte: %.02x>\n", data[0]);
		else
			printf(" <invalid: no data>\n");
		return;
	}

	p->print(type, len, data);
}

static const struct ie_print wifiprinters[] = {
	[1] = {"WPA", print_wifi_wpa, 2, 255, BIT(PRINT_SCAN),},
	[2] = {"WMM", print_wifi_wmm, 1, 255, BIT(PRINT_SCAN),},
	[4] = {"WPS", print_wifi_wps, 0, 255, BIT(PRINT_SCAN),},
};

static void print_vendor(unsigned char len, unsigned char *data, bool unknown, enum print_ie_type ptype)
{
	int i;

	if (len < 3) {
		printf("\tVendor specific: <too short> data:");
		for (i = 0; i < len; i++)
			printf(" %.02x", data[i]);
		printf("\n");
		return;
	}

	if (len >= 4 && memcmp(data, wifi_oui, 3) == 0) {
		if (data[3] < ARRAY_SIZE(wifiprinters) && wifiprinters[data[3]].name && wifiprinters[data[3]].flags & BIT(ptype)) {
			print_ie(&wifiprinters[data[3]], data[3], len - 4, data + 4);
			return;
		}
		if (!unknown)
			return;
		printf("\tWiFi OUI %#.2x, data:", data[3]);
		for (i = 0; i < len - 4; i++)
			printf(" %.02x", data[i + 4]);
		printf("\n");
		return;
	}

	if (!unknown)
		return;

	printf("\tVendor specific: OUI %.2x:%.2x:%.2x, data:", data[0], data[1], data[2]);
	for (i = 3; i < len; i++)
		printf(" %.2x", data[i]);
	printf("\n");
}

static void print_ies(unsigned char *ie, int ielen, bool unknown, enum print_ie_type ptype)
{
	while (ielen >= 2 && ielen >= ie[1]) {
		if (ie[0] < ARRAY_SIZE(ieprinters) && ieprinters[ie[0]].name && ieprinters[ie[0]].flags & BIT(ptype)) {
			print_ie(&ieprinters[ie[0]], ie[0], ie[1], ie + 2);
		} else if (ie[0] == 221 /* vendor */ ) {
			print_vendor(ie[1], ie + 2, unknown, ptype);
		} else if (unknown) {
			int i;

			printf("\tUnknown IE (%d):", ie[0]);
			for (i = 0; i < ie[1]; i++)
				printf(" %.2x", ie[2 + i]);
			printf("\n");
		}
		ielen -= ie[1] + 2;
		ie += ie[1] + 2;
	}
}

static void print_supprates(const uint8_t type, uint8_t len, const uint8_t * data)
{
	int i;

	printf(" ");

	for (i = 0; i < len; i++) {
		int r = data[i] & 0x7f;
		printf("%d.%d%s _%d_", r / 2, 5 * (r & 1), data[i] & 0x80 ? "*" : "", rate_count);
		rate_count++;
	}
	printf("\n");
}

static void print_ds(const uint8_t type, uint8_t len, const uint8_t * data)
{
	printf(" channel %d\n", data[0]);
	site_survey_lists[sscount].channel = data[0];
}

static void print_tim(const uint8_t type, uint8_t len, const uint8_t * data)
{
	printf(" DTIM Count %u DTIM Period %u Bitmap Control 0x%x " "Bitmap[0] 0x%x", data[0], data[1], data[2], data[3]);
	if (len - 4)
		printf(" (+ %u octet%s)", len - 4, len - 4 == 1 ? "" : "s");
	printf("\n");
}

static void print_country(const uint8_t type, uint8_t len, const uint8_t * data)
{
	printf(" %.*s", 2, data);

	printf("\tEnvironment: %s\n", country_env_str(data[2]));

	data += 3;
	len -= 3;

	if (len < 3) {
		printf("\t\tNo country IE triplets present\n");
		return;
	}

	while (len >= 3) {
		int end_channel;
		union ieee80211_country_ie_triplet *triplet = (void *)data;

		if (triplet->ext.reg_extension_id >= IEEE80211_COUNTRY_EXTENSION_ID) {
			printf
			    ("\t\tExtension ID: %d Regulatory Class: %d Coverage class: %d (up to %dm)\n",
			     triplet->ext.reg_extension_id, triplet->ext.reg_class, triplet->ext.coverage_class, triplet->ext.coverage_class * 450);

			data += 3;
			len -= 3;
			continue;
		}

		/* 2 GHz */
		if (triplet->chans.first_channel <= 14)
			end_channel = triplet->chans.first_channel + (triplet->chans.num_channels - 1);
		else
			end_channel = triplet->chans.first_channel + (4 * (triplet->chans.num_channels - 1));

		printf("\t\tChannels [%d - %d] @ %d dBm\n", triplet->chans.first_channel, end_channel, triplet->chans.max_power);

		data += 3;
		len -= 3;
	}

	return;
}

static void print_powerconstraint(const uint8_t type, uint8_t len, const uint8_t * data)
{
	printf(" %d dB\n", data[0]);
}

static void print_erp(const uint8_t type, uint8_t len, const uint8_t * data)
{
	if (data[0] == 0x00)
		printf(" <no flags>");
	if (data[0] & 0x01)
		printf(" NonERP_Present");
	if (data[0] & 0x02)
		printf(" Use_Protection");
	if (data[0] & 0x04)
		printf(" Barker_Preamble_Mode");
	printf("\n");
}

static void print_cipher(const uint8_t * data)
{
	if (memcmp(data, wifi_oui, 3) == 0) {
		switch (data[3]) {
		case 0:
			printf("Use group cipher suite");
			break;
		case 1:
			printf("WEP-40");
			fillENC("WEP-40", " ");
			break;
		case 2:
			printf("TKIP");
			fillENC("TKIP", " ");
			break;
		case 4:
			printf("CCMP");
			fillENC("CCMP", " ");
			break;
		case 5:
			printf("WEP-104");
			fillENC("WEP-104", " ");
			break;
		default:
			printf("%.02x-%.02x-%.02x:%d", data[0], data[1], data[2], data[3]);
			break;
		}
	} else if (memcmp(data, ieee80211_oui, 3) == 0) {
		switch (data[3]) {
		case 0:
			printf("Use group cipher suite");
			break;
		case 1:
			printf("WEP-40");
			fillENC("WEP-40", " ");
			break;
		case 2:
			printf("TKIP");
			fillENC("TKIP", " ");
			break;
		case 4:
			printf("CCMP");
			fillENC("CCMP", " ");
			break;
		case 5:
			printf("WEP-104");
			fillENC("WEP-104", " ");
			break;
		case 6:
			printf("AES-128-CMAC");
			fillENC("AES-128-CMAC", " ");
			break;
		default:
			printf("%.02x-%.02x-%.02x:%d", data[0], data[1], data[2], data[3]);
			break;
		}
	} else
		printf("%.02x-%.02x-%.02x:%d", data[0], data[1], data[2], data[3]);
}

static void print_auth(const uint8_t * data)
{
	if (memcmp(data, wifi_oui, 3) == 0) {
		switch (data[3]) {
		case 1:
			printf("IEEE 802.1X");
			fillENC("IEEE 802.1X", " ");
			break;
		case 2:
			printf("PSK");
			fillENC("PSK", " ");
			break;
		default:
			printf("%.02x-%.02x-%.02x:%d", data[0], data[1], data[2], data[3]);
			break;
		}
	} else if (memcmp(data, ieee80211_oui, 3) == 0) {
		switch (data[3]) {
		case 1:
			printf("IEEE 802.1X");
			fillENC("IEEE 802.1X", " ");
			break;
		case 2:
			printf("PSK");
			fillENC("PSK", " ");
			break;
		case 3:
			printf("FT/IEEE 802.1X");
			fillENC("FT/IEEE 8021X", " ");
			break;
		case 4:
			printf("FT/PSK");
			fillENC("FT/PSK", " ");
			break;
		case 5:
			printf("IEEE 802.1X/SHA-256");
			fillENC("IEEE 8021X/SHA-256", " ");
			break;
		case 6:
			printf("PSK/SHA-256");
			fillENC("PSK/SHA-256", " ");
			break;
		default:
			printf("%.02x-%.02x-%.02x:%d", data[0], data[1], data[2], data[3]);
			break;
		}
	} else
		printf("%.02x-%.02x-%.02x:%d", data[0], data[1], data[2], data[3]);
}

static void print_rsn_ie(const char *defcipher, const char *defauth, uint8_t len, const uint8_t * data)
{
	bool first = true;
	__u16 version, count, capa;
	int i;

	version = data[0] + (data[1] << 8);
	tab_on_first(&first);
	printf("\t * Version: %d\n", version);

	data += 2;
	len -= 2;

	if (len < 4) {
		tab_on_first(&first);
		printf("\t * Group cipher: %s\n", defcipher);
		printf("\t * Pairwise ciphers: %s\n", defcipher);
		fillENC(defcipher, " ");
		return;
	}

	tab_on_first(&first);
	printf("\t * Group cipher: ");
	print_cipher(data);
	printf("\n");

	data += 4;
	len -= 4;

	if (len < 2) {
		tab_on_first(&first);
		printf("\t * Pairwise ciphers: %s\n", defcipher);
		return;
	}

	count = data[0] | (data[1] << 8);
	if (2 + (count * 4) > len)
		goto invalid;

	tab_on_first(&first);
	printf("\t * Pairwise ciphers:");
	for (i = 0; i < count; i++) {
		printf(" ");
		print_cipher(data + 2 + (i * 4));
	}
	printf("\n");

	data += 2 + (count * 4);
	len -= 2 + (count * 4);

	if (len < 2) {
		tab_on_first(&first);
		printf("\t * Authentication suites: %s\n", defauth);
		fillENC(defauth, " ");
		return;
	}

	count = data[0] | (data[1] << 8);
	if (2 + (count * 4) > len)
		goto invalid;

	tab_on_first(&first);
	printf("\t * Authentication suites:");
	for (i = 0; i < count; i++) {
		printf(" ");
		print_auth(data + 2 + (i * 4));
	}
	printf("\n");

	data += 2 + (count * 4);
	len -= 2 + (count * 4);

	if (len >= 2) {
		capa = data[0] | (data[1] << 8);
		tab_on_first(&first);
		printf("\t * Capabilities:");
		if (capa & 0x0001)
			printf(" PreAuth");
		if (capa & 0x0002)
			printf(" NoPairwise");
		switch ((capa & 0x000c) >> 2) {
		case 0:
			break;
		case 1:
			printf(" 2-PTKSA-RC");
			break;
		case 2:
			printf(" 4-PTKSA-RC");
			break;
		case 3:
			printf(" 16-PTKSA-RC");
			break;
		}
		switch ((capa & 0x0030) >> 4) {
		case 0:
			break;
		case 1:
			printf(" 2-GTKSA-RC");
			break;
		case 2:
			printf(" 4-GTKSA-RC");
			break;
		case 3:
			printf(" 16-GTKSA-RC");
			break;
		}
		if (capa & 0x0040)
			printf(" MFP-required");
		if (capa & 0x0080)
			printf(" MFP-capable");
		if (capa & 0x0200)
			printf(" Peerkey-enabled");
		if (capa & 0x0400)
			printf(" SPP-AMSDU-capable");
		if (capa & 0x0800)
			printf(" SPP-AMSDU-required");
		printf(" (0x%.4x)\n", capa);
		data += 2;
		len -= 2;
	}

	if (len >= 2) {
		int pmkid_count = data[0] | (data[1] << 8);

		if (len >= 2 + 16 * pmkid_count) {
			tab_on_first(&first);
			printf("\t * %d PMKIDs\n", pmkid_count);
			/* not printing PMKID values */
			data += 2 + 16 * pmkid_count;
			len -= 2 + 16 * pmkid_count;
		} else
			goto invalid;
	}

	if (len >= 4) {
		tab_on_first(&first);
		printf("\t * Group mgmt cipher suite: ");
		print_cipher(data);
		printf("\n");
		data += 4;
		len -= 4;
	}

invalid:
	if (len != 0) {
		printf("\t\t * bogus tail data (%d):", len);
		while (len) {
			printf(" %.2x", *data);
			data++;
			len--;
		}
		printf("\n");
	}
}

static void print_rsn(const uint8_t type, uint8_t len, const uint8_t * data)
{
	fillENC("WPA2", " ");
	print_rsn_ie("CCMP", "IEEE 802.1X", len, data);
}

static void print_ht_capability(__u16 cap)
{
#define PRINT_HT_CAP(_cond, _str) \
	do { \
		if (_cond) \
			printf("\t\t\t" _str "\n"); \
	} while (0)

	printf("\t\tCapabilities: 0x%02x\n", cap);

	PRINT_HT_CAP((cap & BIT(0)), "RX LDPC");
	PRINT_HT_CAP((cap & BIT(1)), "HT20/HT40");
	if (cap & BIT(1))
		fillENC("HT20/HT40", " ");
	PRINT_HT_CAP(!(cap & BIT(1)), "HT20");
	if (!(cap & BIT(1)))
		fillENC("HT20", " ");

	PRINT_HT_CAP(((cap >> 2) & 0x3) == 0, "Static SM Power Save");
	PRINT_HT_CAP(((cap >> 2) & 0x3) == 1, "Dynamic SM Power Save");
	PRINT_HT_CAP(((cap >> 2) & 0x3) == 3, "SM Power Save disabled");

	PRINT_HT_CAP((cap & BIT(4)), "RX Greenfield");
	PRINT_HT_CAP((cap & BIT(5)), "RX HT20 SGI");
	PRINT_HT_CAP((cap & BIT(6)), "RX HT40 SGI");
	PRINT_HT_CAP((cap & BIT(7)), "TX STBC");

	PRINT_HT_CAP(((cap >> 8) & 0x3) == 0, "No RX STBC");
	PRINT_HT_CAP(((cap >> 8) & 0x3) == 1, "RX STBC 1-stream");
	PRINT_HT_CAP(((cap >> 8) & 0x3) == 2, "RX STBC 2-streams");
	PRINT_HT_CAP(((cap >> 8) & 0x3) == 3, "RX STBC 3-streams");

	PRINT_HT_CAP((cap & BIT(10)), "HT Delayed Block Ack");

	PRINT_HT_CAP(!(cap & BIT(11)), "Max AMSDU length: 3839 bytes");
	PRINT_HT_CAP((cap & BIT(11)), "Max AMSDU length: 7935 bytes");

	/*
	 * For beacons and probe response this would mean the BSS
	 * does or does not allow the usage of DSSS/CCK HT40.
	 * Otherwise it means the STA does or does not use
	 * DSSS/CCK HT40.
	 */
	PRINT_HT_CAP((cap & BIT(12)), "DSSS/CCK HT40");
	PRINT_HT_CAP(!(cap & BIT(12)), "No DSSS/CCK HT40");

	/* BIT(13) is reserved */

	PRINT_HT_CAP((cap & BIT(14)), "40 MHz Intolerant");

	PRINT_HT_CAP((cap & BIT(15)), "L-SIG TXOP protection");
#undef PRINT_HT_CAP
}

static void print_ht_capa(const uint8_t type, uint8_t len, const uint8_t * data)
{
	printf("\n");
	print_ht_capability(data[0] | (data[1] << 8));
	print_ampdu_length(data[2] & 3);
	print_ampdu_spacing((data[2] >> 2) & 7);
	print_ht_mcs(data + 3);
}

static void print_ht_op(const uint8_t type, uint8_t len, const uint8_t * data)
{
	static const char *offset[4] = {
		"no secondary",
		"above",
		"[reserved!]",
		"below",
	};
	static const char *protection[4] = {
		"no",
		"nonmember",
		"20 MHz",
		"non-HT mixed",
	};
	static const char *sta_chan_width[2] = {
		"20 MHz",
		"any",
	};

	printf("\n");
	printf("\t\t * primary channel: %d\n", data[0]);
	printf("\t\t * secondary channel offset: %s\n", offset[data[1] & 0x3]);
	printf("\t\t * STA channel width: %s\n", sta_chan_width[(data[1] & 0x4) >> 2]);
	printf("\t\t * RIFS: %d\n", (data[1] & 0x8) >> 3);
	printf("\t\t * HT protection: %s\n", protection[data[2] & 0x3]);
	printf("\t\t * non-GF present: %d\n", (data[2] & 0x4) >> 2);
	printf("\t\t * OBSS non-GF present: %d\n", (data[2] & 0x10) >> 4);
	printf("\t\t * dual beacon: %d\n", (data[4] & 0x40) >> 6);
	printf("\t\t * dual CTS protection: %d\n", (data[4] & 0x80) >> 7);
	printf("\t\t * STBC beacon: %d\n", data[5] & 0x1);
	printf("\t\t * L-SIG TXOP Prot: %d\n", (data[5] & 0x2) >> 1);
	printf("\t\t * PCO active: %d\n", (data[5] & 0x4) >> 2);
	printf("\t\t * PCO phase: %d\n", (data[5] & 0x8) >> 3);
}

static void print_capabilities(const uint8_t type, uint8_t len, const uint8_t * data)
{
	int i, base, bit;
	bool first = true;

	for (i = 0; i < len; i++) {
		base = i * 8;

		for (bit = 0; bit < 8; bit++) {
			if (!(data[i] & (1 << bit)))
				continue;

			if (!first)
				printf(",");
			else
				first = false;

			switch (bit + base) {
			case 0:
				printf(" HT Information Exchange Supported");
				break;
			case 1:
				printf(" On-demand Beacon");
				break;
			case 2:
				printf(" Extended Channel Switching");
				break;
			case 3:
				printf(" Wave Indication");
				break;
			case 4:
				printf(" PSMP Capability");
				break;
			case 5:
				printf(" Service Interval Granularity");
				break;
			case 6:
				printf(" S-PSMP Capability");
				break;
			default:
				printf(" %d", bit);
				break;
			}
		}
	}

	printf("\n");
}

//  end of iw copied code
void mac80211_scan(char *interface)
{
	struct nl_msg *msg;
	int wdev;
	wdev = if_nametoindex(interface);
	lgetnoise(wdev);
	memset(&scan_params, 0, sizeof(scan_params));
	scan_params.type = PRINT_SCAN;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SCAN, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, msg, print_bss_handler, &scan_params);
	// nlmsg_free(surveymsg);
	return;
nla_put_failure:
	nlmsg_free(msg);
}

static int write_site_survey(void);
static int open_site_survey(void);

void mac80211_site_survey(char *interface)
{
	int i;
	int phy, wdev;
	char scaninterface[32];
	char macaddr[32];
	unsigned char hwbuff[16];
	bzero(site_survey_lists, sizeof(site_survey_lists));
	sysprintf("iw dev %s scan", interface);
	mac80211_scan(interface);
	write_site_survey();
	open_site_survey();
	for (i = 0; i < SITE_SURVEY_NUM && site_survey_lists[i].BSSID[0]; i++) {

		if (site_survey_lists[i].SSID[0] == 0) {
			strcpy(site_survey_lists[i].SSID, "hidden");
		}

		fprintf(stderr,
			"[%2d] SSID[%20s] BSSID[%s] channel[%2d] frequency[%4d] rssi[%d] noise[%d] beacon[%d] cap[%x] dtim[%d] rate[%d] enc[%s]\n",
			i, site_survey_lists[i].SSID,
			site_survey_lists[i].BSSID,
			site_survey_lists[i].channel,
			site_survey_lists[i].frequency,
			site_survey_lists[i].RSSI,
			site_survey_lists[i].phy_noise,
			site_survey_lists[i].beacon_period, site_survey_lists[i].capability, site_survey_lists[i].dtim_period, site_survey_lists[i].rate_count, site_survey_lists[i].ENCINFO);
	}

}

static int write_site_survey(void)
{
	FILE *fp;
	if ((fp = fopen(SITE_SURVEY_DB, "w"))) {
		fwrite(&site_survey_lists[0], sizeof(site_survey_lists), 1, fp);
		fclose(fp);
		return 0;
	}
	return 1;
}

static int open_site_survey(void)
{
	FILE *fp;
	bzero(site_survey_lists, sizeof(site_survey_lists));
	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&site_survey_lists[0], sizeof(site_survey_lists), 1, fp);
		fclose(fp);
		return 1;
	}
	return 0;
}

int site_survey_main_mac802211(int argc, char *argv[])
{
	unlink(SITE_SURVEY_DB);
	char *sta = nvram_safe_get("wifi_display");
	mac80211_site_survey(sta);
	return 0;
}
