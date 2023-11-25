/*
 * Copyright (C) 2006-2018, Marvell International Ltd.
 *
 * This software file (the "File") is distributed by Marvell International
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991
 * (the "License").  You may use, redistribute and/or modify this File in
 * accordance with the terms and conditions of the License, a copy of which
 * is available by writing to the Free Software Foundation, Inc.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about
 * this warranty disclaimer.
 */

/* Description:  This file implements common utility functions. */

#include <linux/etherdevice.h>

#include "sysadpt.h"
#include "core.h"
#include "utils.h"

static unsigned short phy_rate[][5] = {
	{2,   13,  15,  27,  30},   /* 0  */
	{4,   26,  29,  54,  60},   /* 1  */
	{11,  39,  43,  81,  90},   /* 2  */
	{22,  52,  58,  108, 120},  /* 3  */
	{44,  78,  87,  162, 180},  /* 4  */
	{12,  104, 115, 216, 240},  /* 5  */
	{18,  117, 130, 243, 270},  /* 6  */
	{24,  130, 144, 270, 300},  /* 7  */
	{36,  26,  29,  54,  60},   /* 8  */
	{48,  52,  58,  108, 120},  /* 9  */
	{72,  78,  87,  162, 180},  /* 10 */
	{96,  104, 116, 216, 240},  /* 11 */
	{108, 156, 173, 324, 360},  /* 12 */
	{0,   208, 231, 432, 480},  /* 13 */
	{0,   234, 260, 486, 540},  /* 14 */
	{0,   260, 289, 540, 600},  /* 15 */
	{0,   39,  43,  81,  90},   /* 16 */
	{0,   78,  87,  162, 180},  /* 17 */
	{0,   117, 130, 243, 270},  /* 18 */
	{0,   156, 173, 324, 360},  /* 19 */
	{0,   234, 260, 486, 540},  /* 20 */
	{0,   312, 347, 648, 720},  /* 21 */
	{0,   351, 390, 729, 810},  /* 22 */
	{0,   390, 433, 810, 900},  /* 23 */
};

/* 20Mhz: Nss1_LGI, Nss1_SGI, Nss2_LGI, Nss2_SGI, Nss3_LGI, Nss3_SGI */
static unsigned short phy_rate_11ac20M[][6] = {
	{13,  15,  26,  29,  39,  44},   /* 0 */
	{26,  29,  52,  58,  78,  87},   /* 1 */
	{39,  44,  78,  87,  117, 130},  /* 2 */
	{52,  58,  104, 116, 156, 174},  /* 3 */
	{78,  87,  156, 174, 234, 260},  /* 4 */
	{104, 116, 208, 231, 312, 347},  /* 5 */
	{117, 130, 234, 260, 351, 390},  /* 6 */
	{130, 145, 260, 289, 390, 434},  /* 7 */
	{156, 174, 312, 347, 468, 520},  /* 8 */
	/* Nss 1 and Nss 2 mcs9 not valid */
	{2,   2,   2,   2,   520, 578},  /* 9 */
};

/* 40Mhz: Nss1_LGI, Nss1_SGI, Nss2_LGI, Nss2_SGI, Nss3_LGI, Nss3_SGI */
static unsigned short phy_rate_11ac40M[][6] = {
	{27,  30,  54,  60,  81,   90},   /* 0 */
	{54,  60,  108, 120, 162,  180},  /* 1 */
	{81,  90,  162, 180, 243,  270},  /* 2 */
	{108, 120, 216, 240, 324,  360},  /* 3 */
	{162, 180, 324, 360, 486,  540},  /* 4 */
	{216, 240, 432, 480, 648,  720},  /* 5 */
	{243, 270, 486, 540, 729,  810},  /* 6 */
	{270, 300, 540, 600, 810,  900},  /* 7 */
	{324, 360, 648, 720, 972,  1080}, /* 8 */
	{360, 400, 720, 800, 1080, 1200}, /* 9 */
};

/* 80Mhz: Nss1_LGI, Nss1_SGI, Nss2_LGI, Nss2_SGI, Nss3_LGI, Nss3_SGI */
static unsigned short phy_rate_11ac80M[][6] = {
	{59,  65,  117,  130,  175,  195},  /* 0 */
	{117, 130, 234,  260,  351,  390},  /* 1 */
	{175, 195, 351,  390,  527,  585},  /* 2 */
	{234, 260, 468,  520,  702,  780},  /* 3 */
	{351, 390, 702,  780,  1053, 1170}, /* 4 */
	{468, 520, 936,  1040, 1404, 1560}, /* 5 */
	{527, 585, 1053, 1170, 2,    2},    /* 6, Nss 3 mcs6 not valid */
	{585, 650, 1170, 1300, 1755, 1950}, /* 7 */
	{702, 780, 1404, 1560, 2106, 2340}, /* 8 */
	{780, 867, 1560, 1733, 2340, 2600}, /* 9 */
};

/* 160Mhz: Nss1_LGI, Nss1_SGI, Nss2_LGI, Nss2_SGI, Nss3_LGI, Nss3_SGI */
static unsigned short phy_rate_11ac160M[][6] = {
	{117,   130,  234,  260,  351,  390},  /* 0 */
	{234,   260,  468,  520,  702,  780},  /* 1 */
	{351,   390,  702,  780,  1053, 1170}, /* 2 */
	{468,   520,  936,  1040, 1404, 1560}, /* 3 */
	{702,   780,  1404, 1560, 2106, 2340}, /* 4 */
	{936,   1040, 1872, 2080, 2808, 3120}, /* 5 */
	{1053,  1170, 2106, 2340, 3159, 3510}, /* 6 */
	{1170,  1300, 2340, 2600, 3510, 3900}, /* 7 */
	{1404,  1560, 2808, 3120, 4212, 4680}, /* 8 */
	{1560,  1733, 2130, 3467, 4680, 5200}, /* 9 */
};

int utils_get_phy_rate(u8 format, u8 bandwidth, u8 short_gi, u8 mcs_id)
{
	u8 index = 0;
	u8 nss_11ac = 0;
	u8 rate_11ac = 0;

	if (format == TX_RATE_FORMAT_11N) {
		index = (bandwidth << 1) | short_gi;
		index++;
	} else if (format == TX_RATE_FORMAT_11AC) {
		rate_11ac = mcs_id & 0xf; /* 11ac, mcs_id[3:0]: rate     */
		nss_11ac = mcs_id >> 4;	  /* 11ac, mcs_id[6:4]: nss code */
		index = (nss_11ac << 1) | short_gi;
	}

	if (format != TX_RATE_FORMAT_11AC)
		return (phy_rate[mcs_id][index] / 2);

	if (bandwidth == TX_RATE_BANDWIDTH_20)
		return (phy_rate_11ac20M[rate_11ac][index] / 2);
	else if (bandwidth == TX_RATE_BANDWIDTH_40)
		return (phy_rate_11ac40M[rate_11ac][index] / 2);
	else if (bandwidth == TX_RATE_BANDWIDTH_80)
		return (phy_rate_11ac80M[rate_11ac][index] / 2);
	else
		return (phy_rate_11ac160M[rate_11ac][index] / 2);
}

u8 utils_get_rate_id(u8 rate)
{
	switch (rate) {
	case 10:   /* 1 Mbit/s or 12 Mbit/s */
		return 0;
	case 20:   /* 2 Mbit/s */
		return 1;
	case 55:   /* 5.5 Mbit/s */
		return 2;
	case 110:  /* 11 Mbit/s */
		return 3;
	case 220:  /* 22 Mbit/s */
		return 4;
	case 0xb:  /* 6 Mbit/s */
		return 5;
	case 0xf:  /* 9 Mbit/s */
		return 6;
	case 0xe:  /* 18 Mbit/s */
		return 8;
	case 0x9:  /* 24 Mbit/s */
		return 9;
	case 0xd:  /* 36 Mbit/s */
		return 10;
	case 0x8:  /* 48 Mbit/s */
		return 11;
	case 0xc:  /* 54 Mbit/s */
		return 12;
	case 0x7:  /* 72 Mbit/s */
		return 13;
	}

	return 0;
}

u32 utils_get_init_tx_rate(struct mwl_priv *priv, struct ieee80211_conf *conf,
			   struct ieee80211_sta *sta)
{
	u32 tx_rate;
	u16 format, nss, bw, rate_mcs;

	if (sta->vht_cap.vht_supported)
		format = TX_RATE_FORMAT_11AC;
	else if (sta->ht_cap.ht_supported)
		format = TX_RATE_FORMAT_11N;
	else
		format = TX_RATE_FORMAT_LEGACY;

	switch (priv->antenna_tx) {
	case ANTENNA_TX_1:
		nss = 1;
		break;
	case ANTENNA_TX_2:
		nss = 2;
		break;
	case ANTENNA_TX_3:
	case ANTENNA_TX_4_AUTO:
		nss = 3;
		break;
	default:
		nss = sta->rx_nss;
		break;
	}
	if (nss > sta->rx_nss)
		nss = sta->rx_nss;

	switch (conf->chandef.width) {
	case NL80211_CHAN_WIDTH_20_NOHT:
	case NL80211_CHAN_WIDTH_20:
		bw = TX_RATE_BANDWIDTH_20;
		break;
	case NL80211_CHAN_WIDTH_40:
		bw = TX_RATE_BANDWIDTH_40;
		break;
	case NL80211_CHAN_WIDTH_80:
		bw = TX_RATE_BANDWIDTH_80;
		break;
	case NL80211_CHAN_WIDTH_160:
		bw = TX_RATE_BANDWIDTH_160;
		break;
	default:
		bw = sta->bandwidth;
		break;
	}
	if (bw > sta->bandwidth)
		bw = sta->bandwidth;

	switch (format) {
	case TX_RATE_FORMAT_LEGACY:
		rate_mcs = 12; /* ignore 11b */
		break;
	case TX_RATE_FORMAT_11N:
		rate_mcs = (nss * 8) - 1;
		break;
	default:
		rate_mcs = ((nss - 1) << 4) | 8;
		break;
	}

	tx_rate = (format | (bw << MWL_TX_RATE_BANDWIDTH_SHIFT) |
		(TX_RATE_INFO_SHORT_GI << MWL_TX_RATE_SHORTGI_SHIFT) |
		(rate_mcs << MWL_TX_RATE_RATEIDMCS_SHIFT));

	return tx_rate;
}

struct mwl_vif *utils_find_vif_bss(struct mwl_priv *priv, u8 *bssid)
{
	struct mwl_vif *mwl_vif;

	spin_lock_bh(&priv->vif_lock);
	list_for_each_entry(mwl_vif, &priv->vif_list, list) {
		if (ether_addr_equal(bssid, mwl_vif->bssid)) {
			spin_unlock_bh(&priv->vif_lock);
			return mwl_vif;
		}
	}
	spin_unlock_bh(&priv->vif_lock);

	return NULL;
}

struct mwl_sta *utils_find_sta(struct mwl_priv *priv, u8 *addr)
{
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);
		if (ether_addr_equal(addr, sta->addr)) {
			spin_unlock_bh(&priv->sta_lock);
			return sta_info;
		}
	}
	spin_unlock_bh(&priv->sta_lock);

	return NULL;
}

struct mwl_sta *utils_find_sta_by_aid(struct mwl_priv *priv, u16 aid)
{
	struct mwl_sta *sta_info;
	struct ieee80211_sta *sta;

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		sta = container_of((void *)sta_info, struct ieee80211_sta,
				   drv_priv);
		if (sta->aid == aid) {
			spin_unlock_bh(&priv->sta_lock);
			return sta_info;
		}
	}
	spin_unlock_bh(&priv->sta_lock);

	return NULL;
}

struct mwl_sta *utils_find_sta_by_id(struct mwl_priv *priv, u16 stnid)
{
	struct mwl_sta *sta_info;

	spin_lock_bh(&priv->sta_lock);
	list_for_each_entry(sta_info, &priv->sta_list, list) {
		if (sta_info->stnid == stnid) {
			spin_unlock_bh(&priv->sta_lock);
			return sta_info;
		}
	}
	spin_unlock_bh(&priv->sta_lock);

	return NULL;
}

void utils_dump_data_info(const char *prefix_str, const void *buf, size_t len)
{
	print_hex_dump(KERN_INFO, prefix_str, DUMP_PREFIX_OFFSET,
		       16, 1, buf, len, true);
}

void utils_dump_data_debug(const char *prefix_str, const void *buf, size_t len)
{
	print_hex_dump(KERN_DEBUG, prefix_str, DUMP_PREFIX_OFFSET,
		       16, 1, buf, len, true);
}

bool utils_is_non_amsdu_packet(const void *packet, bool mac80211)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct iphdr *iph;
	struct udphdr *udph;

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == cpu_to_be16(ETH_P_PAE))
		return true;

	if (*protocol == htons(ETH_P_ARP))
		return true;

	if (*protocol == htons(ETH_P_IP)) {
		data += sizeof(__be16);
		iph = (struct iphdr *)data;
		if (iph->protocol == IPPROTO_ICMP)
			return true;
		if (iph->protocol == IPPROTO_UDP) {
			data += (iph->ihl * 4);
			udph = (struct udphdr *)data;
			if (((udph->source == htons(68)) &&
			    (udph->dest == htons(67))) ||
			    ((udph->source == htons(67)) &&
			    (udph->dest == htons(68))))
				return true;
		}
	}

	return false;
}

bool utils_is_arp(const void *packet, bool mac80211, u16 *arp_op)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct arphdr *arph;

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == htons(ETH_P_ARP)) {
		data += sizeof(__be16);
		arph = (struct arphdr *)data;
		*arp_op = ntohs(arph->ar_op);
		return true;
	}

	return false;
}

bool utils_is_icmp_echo(const void *packet, bool mac80211, u8 *type)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct iphdr *iph;
	struct icmphdr *icmph;

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == htons(ETH_P_IP)) {
		data += sizeof(__be16);
		iph = (struct iphdr *)data;
		if (iph->protocol == IPPROTO_ICMP) {
			data += (iph->ihl * 4);
			icmph = (struct icmphdr *)data;
			*type = icmph->type;
			return true;
		}
	}

	return false;
}

bool utils_is_dhcp(const void *packet, bool mac80211, u8 *op, u8 *dhcp_client)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct iphdr *iph;
	struct udphdr *udph;

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == htons(ETH_P_IP)) {
		data += sizeof(__be16);
		iph = (struct iphdr *)data;
		if (iph->protocol == IPPROTO_UDP) {
			data += (iph->ihl * 4);
			udph = (struct udphdr *)data;
			if (((udph->source == htons(68)) &&
			    (udph->dest == htons(67))) ||
			    ((udph->source == htons(67)) &&
			    (udph->dest == htons(68)))) {
				data += sizeof(struct udphdr);
				*op = *data;
				ether_addr_copy(dhcp_client, data + 28);
				return true;
			}
		}
	}

	return false;
}

void utils_dump_arp(const void *packet, bool mac80211, size_t len)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct arphdr *arph;

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == htons(ETH_P_ARP)) {
		data += sizeof(__be16);
		arph = (struct arphdr *)data;
		if (arph->ar_op == htons(ARPOP_REQUEST))
			utils_dump_data_info("ARP REQUEST: ", packet, len);
		else if (arph->ar_op == htons(ARPOP_REPLY))
			utils_dump_data_info("ARP REPLY: ", packet, len);
	}
}

void utils_dump_icmp_echo(const void *packet, bool mac80211, size_t len)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct iphdr *iph;
	struct icmphdr *icmph;

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == htons(ETH_P_IP)) {
		data += sizeof(__be16);
		iph = (struct iphdr *)data;
		if (iph->protocol == IPPROTO_ICMP) {
			data += (iph->ihl * 4);
			icmph = (struct icmphdr *)data;
			if (icmph->type == ICMP_ECHO)
				utils_dump_data_info("ECHO REQUEST: ",
						     packet, len);
			else if (icmph->type == ICMP_ECHOREPLY)
				utils_dump_data_info("ECHO REPLY: ",
						     packet, len);
		}
	}
}

void utils_dump_dhcp(const void *packet, bool mac80211, size_t len)
{
	const u8 *data = packet;
	struct ieee80211_hdr *wh;
	__be16 *protocol;
	struct iphdr *iph;
	struct udphdr *udph;
	const char *dhcp_op[8] = {
		"DHCPDISCOVER",
		"DHCPOFFER",
		"DHCPREQUEST",
		"DHCPDECLINE",
		"DHCPACK",
		"DHCPNAK",
		"DHCPRELEASE",
		"DHCPINFORM"
	};

	if (mac80211) {
		/* mac80211 packet */
		wh = (struct ieee80211_hdr *)data;
		data += ieee80211_hdrlen(wh->frame_control) + 6;
		protocol = (__be16 *)data;
	} else {
		/* mac802.3 packet */
		data += (2 * ETH_ALEN);
		protocol = (__be16 *)data;
	}

	if (*protocol == htons(ETH_P_IP)) {
		data += sizeof(__be16);
		iph = (struct iphdr *)data;
		if (iph->protocol == IPPROTO_UDP) {
			data += (iph->ihl * 4);
			udph = (struct udphdr *)data;
			if (((udph->source == htons(68)) &&
			    (udph->dest == htons(67))) ||
			    ((udph->source == htons(67)) &&
			    (udph->dest == htons(68)))) {
				data += sizeof(struct udphdr);
				utils_dump_data_info(dhcp_op[*data - 1],
						     packet, len);
			}
		}
	}
}
