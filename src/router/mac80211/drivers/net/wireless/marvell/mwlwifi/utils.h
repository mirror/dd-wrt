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

/* Description:  This file defines common utility functions. */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <net/arp.h>
#include <net/ip.h>
#include <net/udp.h>
#include <net/icmp.h>

/* DHCP message types */
#define DHCPDISCOVER    1
#define DHCPOFFER       2
#define DHCPREQUEST     3
#define DHCPDECLINE     4
#define DHCPACK         5
#define DHCPNAK         6
#define DHCPRELEASE     7
#define DHCPINFORM      8

static inline int utils_tid_to_ac(u8 tid)
{
	switch (tid) {
	case 0:
	case 3:
		return IEEE80211_AC_BE;
	case 1:
	case 2:
		return IEEE80211_AC_BK;
	case 4:
	case 5:
		return IEEE80211_AC_VI;
	case 6:
	case 7:
		return IEEE80211_AC_VO;
	default:
		break;
	}

	return -1;
}

static inline void utils_add_basic_rates(int band, struct sk_buff *skb)
{
	struct ieee80211_mgmt *mgmt;
	int len;
	u8 *pos;

	mgmt = (struct ieee80211_mgmt *)skb->data;
	len = skb->len - ieee80211_hdrlen(mgmt->frame_control);
	len -= 4;
	pos = (u8 *)cfg80211_find_ie(WLAN_EID_SUPP_RATES,
				     mgmt->u.assoc_req.variable,
				     len);
	if (pos) {
		pos++;
		len = *pos++;
		while (len) {
			if (band == NL80211_BAND_2GHZ) {
				if ((*pos == 2) || (*pos == 4) ||
				    (*pos == 11) || (*pos == 22))
					*pos |= 0x80;
			} else {
				if ((*pos == 12) || (*pos == 24) ||
				    (*pos == 48))
					*pos |= 0x80;
			}
			pos++;
			len--;
		}
	}
}

static inline int utils_assign_stnid(struct mwl_priv *priv, int macid, u16 aid)
{
	int stnid;
	int i;

	spin_lock_bh(&priv->stnid_lock);
	stnid = priv->available_stnid;
	if (stnid >= priv->stnid_num) {
		spin_unlock_bh(&priv->stnid_lock);
		return 0;
	}
	priv->stnid[stnid].macid = macid;
	priv->stnid[stnid].aid = aid;
	stnid++;
	for (i = stnid; i < priv->stnid_num; i++) {
		if (!priv->stnid[i].aid)
			break;
	}
	priv->available_stnid = i;
	spin_unlock_bh(&priv->stnid_lock);
	return stnid;
}

static inline void utils_free_stnid(struct mwl_priv *priv, u16 stnid)
{
	spin_lock_bh(&priv->stnid_lock);
	if (stnid && (stnid <= priv->stnid_num)) {
		stnid--;
		priv->stnid[stnid].macid = 0;
		priv->stnid[stnid].aid = 0;
		if (priv->available_stnid > stnid)
			priv->available_stnid = stnid;
	}
	spin_unlock_bh(&priv->stnid_lock);
}

int utils_get_phy_rate(u8 format, u8 bandwidth, u8 short_gi, u8 mcs_id);

u8 utils_get_rate_id(u8 rate);

u32 utils_get_init_tx_rate(struct mwl_priv *priv, struct ieee80211_conf *conf,
			   struct ieee80211_sta *sta);

struct mwl_vif *utils_find_vif_bss(struct mwl_priv *priv, u8 *bssid);

struct mwl_sta *utils_find_sta(struct mwl_priv *priv, u8 *addr);

struct mwl_sta *utils_find_sta_by_aid(struct mwl_priv *priv, u16 aid);

struct mwl_sta *utils_find_sta_by_id(struct mwl_priv *priv, u16 stnid);

void utils_dump_data_info(const char *prefix_str, const void *buf, size_t len);

void utils_dump_data_debug(const char *prefix_str, const void *buf, size_t len);

bool utils_is_non_amsdu_packet(const void *packet, bool mac80211);

bool utils_is_arp(const void *packet, bool mac80211, u16 *arp_op);

bool utils_is_icmp_echo(const void *packet, bool mac80211, u8 *type);

bool utils_is_dhcp(const void *packet, bool mac80211, u8 *op, u8 *dhcp_client);

void utils_dump_arp(const void *packet, bool mac80211, size_t len);

void utils_dump_icmp_echo(const void *packet, bool mac80211, size_t len);

void utils_dump_dhcp(const void *packet, bool mac80211, size_t len);

#endif /* _UTILS_H_ */
