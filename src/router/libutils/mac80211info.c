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

#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdbool.h>
#include <glob.h>
#include <bcmnvram.h>

#include "unl.h"
#include "mac80211regulatory.h"
#include <nl80211.h>

#include "wlutils.h"
#include <utils.h>

// some defenitions from hostapd
typedef uint16_t u16;
typedef uint32_t u32;
#define BIT(x) (1ULL<<(x))
#define HT_CAP_INFO_LDPC_CODING_CAP     ((u16) BIT(0))
#define HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET  ((u16) BIT(1))
#define HT_CAP_INFO_SMPS_MASK           ((u16) (BIT(2) | BIT(3)))
#define HT_CAP_INFO_SMPS_STATIC         ((u16) 0)
#define HT_CAP_INFO_SMPS_DYNAMIC        ((u16) BIT(2))
#define HT_CAP_INFO_SMPS_DISABLED       ((u16) (BIT(2) | BIT(3)))
#define HT_CAP_INFO_GREEN_FIELD         ((u16) BIT(4))
#define HT_CAP_INFO_SHORT_GI20MHZ       ((u16) BIT(5))
#define HT_CAP_INFO_SHORT_GI40MHZ       ((u16) BIT(6))
#define HT_CAP_INFO_TX_STBC         ((u16) BIT(7))
#define HT_CAP_INFO_RX_STBC_MASK        ((u16) (BIT(8) | BIT(9)))
#define HT_CAP_INFO_RX_STBC_1           ((u16) BIT(8))
#define HT_CAP_INFO_RX_STBC_12          ((u16) BIT(9))
#define HT_CAP_INFO_RX_STBC_123         ((u16) (BIT(8) | BIT(9)))
#define HT_CAP_INFO_DELAYED_BA          ((u16) BIT(10))
#define HT_CAP_INFO_MAX_AMSDU_SIZE      ((u16) BIT(11))
#define HT_CAP_INFO_DSSS_CCK40MHZ       ((u16) BIT(12))
#define HT_CAP_INFO_PSMP_SUPP           ((u16) BIT(13))
#define HT_CAP_INFO_40MHZ_INTOLERANT        ((u16) BIT(14))
#define HT_CAP_INFO_LSIG_TXOP_PROTECT_SUPPORT   ((u16) BIT(15))

#define VHT_CAP_MAX_MPDU_LENGTH_7991                ((u32) BIT(0))
#define VHT_CAP_MAX_MPDU_LENGTH_11454               ((u32) BIT(1))
#define VHT_CAP_SUPP_CHAN_WIDTH_160MHZ              ((u32) BIT(2))
#define VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ     ((u32) BIT(3))
#define VHT_CAP_RXLDPC                              ((u32) BIT(4))
#define VHT_CAP_SHORT_GI_80                         ((u32) BIT(5))
#define VHT_CAP_SHORT_GI_160                        ((u32) BIT(6))
#define VHT_CAP_TXSTBC                              ((u32) BIT(7))
#define VHT_CAP_RXSTBC_1                            ((u32) BIT(8))
#define VHT_CAP_RXSTBC_2                            ((u32) BIT(9))
#define VHT_CAP_RXSTBC_3                            ((u32) BIT(8) | BIT(9))
#define VHT_CAP_RXSTBC_4                            ((u32) BIT(10))
#define VHT_CAP_SU_BEAMFORMER_CAPABLE               ((u32) BIT(11))
#define VHT_CAP_SU_BEAMFORMEE_CAPABLE               ((u32) BIT(12))
#define VHT_CAP_BEAMFORMER_ANTENNAS_MAX             ((u32) BIT(13) | BIT(14))
#define VHT_CAP_SOUNDING_DIMENTION_MAX              ((u32) BIT(16) | BIT(17))
#define VHT_CAP_MU_BEAMFORMER_CAPABLE               ((u32) BIT(19))
#define VHT_CAP_MU_BEAMFORMEE_CAPABLE               ((u32) BIT(20))
#define VHT_CAP_VHT_TXOP_PS                         ((u32) BIT(21))
#define VHT_CAP_HTC_VHT                             ((u32) BIT(22))
#define VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT          ((u32) BIT(23))
#define VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB   ((u32) BIT(27))
#define VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB     ((u32) BIT(26) | BIT(27))
#define VHT_CAP_RX_ANTENNA_PATTERN                  ((u32) BIT(28))
#define VHT_CAP_TX_ANTENNA_PATTERN                  ((u32) BIT(29))

struct unl unl;
bool bunl;

static void print_wifi_clients(struct wifi_client_info *wci);
void free_wifi_clients(struct wifi_client_info *wci);
static struct wifi_client_info *add_to_wifi_clients(struct wifi_client_info *list_root);
static int mac80211_cb_survey(struct nl_msg *msg, void *data);

static void __attribute__((constructor)) mac80211_init(void)
{
	if (!bunl) {
		int ret = unl_genl_init(&unl, "nl80211");
		bunl = 1;
	}
}

static int phy_lookup_by_number(int idx)
{
	char buf[200];
	int fd, pos;

	snprintf(buf, sizeof(buf), "/sys/class/ieee80211/phy%d/index", idx);

	fd = open(buf, O_RDONLY);
	if (fd < 0)
		return -1;
	pos = read(fd, buf, sizeof(buf) - 1);
	if (pos < 0) {
		close(fd);
		return -1;
	}
	buf[pos] = '\0';
	close(fd);
	return atoi(buf);
}

int mac80211_get_phyidx_by_vifname(char *vif)
{
	return (phy_lookup_by_number(get_ath9k_phy_ifname(vif)));
}

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = {.type = NLA_U32},
	[NL80211_SURVEY_INFO_NOISE] = {.type = NLA_U8},
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = {.type = NLA_U64},
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = {.type = NLA_U64},
};

int mac80211_parse_survey(struct nl_msg *msg, struct nlattr **sinfo)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_SURVEY_INFO]) {
		fprintf(stderr, "no survey info\n");
		return -1;
	}

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX, tb[NL80211_ATTR_SURVEY_INFO], survey_policy)) {
		fprintf(stderr, "error survey\n");

		return -1;
	}

	if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY]) {
		fprintf(stderr, "no frequency info\n");
		return -1;
	}

	return 0;
}

static int mac80211_cb_survey(struct nl_msg *msg, void *data)
{
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	int freq;
	struct mac80211_info *mac80211_info = data;

	if (mac80211_parse_survey(msg, sinfo))
		goto out;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	if (!mac80211_info->noise)
		mac80211_info->noise = -95;
	if (sinfo[NL80211_SURVEY_INFO_IN_USE]) {

		if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME] && sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]) {
			mac80211_info->channel_active_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
			mac80211_info->channel_busy_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);
		}

		if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX])
			mac80211_info->channel_receive_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX]);

		if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX])
			mac80211_info->channel_transmit_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX]);

		if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY])
			mac80211_info->extension_channel_busy_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_EXT_BUSY]);

		if (sinfo[NL80211_SURVEY_INFO_NOISE])
			mac80211_info->noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);

		if (sinfo[NL80211_SURVEY_INFO_FREQUENCY])
			mac80211_info->frequency = freq;
	}

out:
	return NL_SKIP;
}

static void getNoise_mac80211_internal(char *interface, struct mac80211_info *mac80211_info)
{
	struct nl_msg *msg;
	int wdev = if_nametoindex(interface);

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, msg, mac80211_cb_survey, mac80211_info);
	return;
nla_put_failure:
	nlmsg_free(msg);
	return;
}

int getNoise_mac80211(char *interface)
{
	struct nl_msg *msg;
	struct mac80211_info mac80211_info;
	int wdev = if_nametoindex(interface);
	memset(&mac80211_info, 0, sizeof(mac80211_info));

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, msg, mac80211_cb_survey, &mac80211_info);
	return mac80211_info.noise;

nla_put_failure:
	nlmsg_free(msg);
	return (-199);
}

#ifdef HAVE_ATH10K
int is_beeliner(const char *prefix)
{
	char globstring[1024];
	int devnum;
	devnum = get_ath9k_phy_ifname(prefix);
	if (devnum == -1)
		return 0;

	sprintf(globstring, "/sys/class/ieee80211/phy%d/device/device", devnum);
	FILE *fp = fopen(globstring, "rb");
	if (!fp)
		return 0;
	char buf[32];
	fscanf(fp, "%s", buf);
	fclose(fp);
	if (!strcmp(buf, "0x0040"))
		return 1;
	return 0;
}

unsigned int get_ath10kreg(char *ifname, unsigned int reg)
{
	unsigned int baseaddress = is_beeliner(ifname) ? 0x30000 : 0x20000;
	char file[64];
	int phy = get_ath9k_phy_ifname(ifname);
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_addr", phy);
	FILE *fp = fopen(file, "wb");
	if (fp == NULL)
		return 0;
	fprintf(fp, "0x%x", baseaddress + reg);
	fclose(fp);
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_value", phy);
	fp = fopen(file, "rb");
	if (fp == NULL)
		return 0;
	int value;
	fscanf(fp, "0x%08x:0x%08x", &reg, &value);
	fclose(fp);
	return value;
}

void set_ath10kreg(char *ifname, unsigned int reg, unsigned int value)
{
	unsigned int baseaddress = is_beeliner(ifname) ? 0x30000 : 0x20000;
	char file[64];
	int phy = get_ath9k_phy_ifname(ifname);
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_addr", phy);
	FILE *fp = fopen(file, "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "0x%x", baseaddress + reg);
	fclose(fp);
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_value", phy);
	fp = fopen(file, "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "0x%x", value);
	fclose(fp);
}

void set_ath10kdistance(char *dev, unsigned int distance)
{
	unsigned int isb = is_beeliner(dev);
	unsigned int macclk = isb ? 142 : 88;
	unsigned int slot = ((distance + 449) / 450) * 3;
	unsigned int baseslot = 9;
	slot += baseslot;	// base time
	unsigned int sifs = 16;
	unsigned int ack = slot + sifs;
	unsigned int cts = ack;
	if ((int)distance == -1)
		return;
	if (slot == 0)		// too low value. 
		return;

	if (!isb) {
		ack *= macclk;	// 88Mhz is the core clock of AR9880
		cts *= macclk;
		slot *= macclk;
		sifs *= macclk;
	}
	if (!isb && ack > 0x3fff) {
		fprintf(stderr, "invalid ack 0x%08x, max is 0x3fff. truncate it\n", ack);
		ack = 0x3fff;
	} else if (isb && ack > 0xffff) {
		fprintf(stderr, "invalid ack 0x%08x, max is 0xff. truncate it\n", ack);
		ack = 0xffff;
	}

	unsigned int oldack;
	if (isb)
		oldack = get_ath10kreg(dev, 0xf424) & 0xffff;
	else
		oldack = get_ath10kreg(dev, 0x8014) & 0x3fff;

	if (oldack != ack) {
		if (isb) {
			set_ath10kreg(dev, 0x0040, baseslot);
			set_ath10kreg(dev, 0xf420, ((sifs << 8) & 0x1ff00) | (slot & 0xff));
			set_ath10kreg(dev, 0xf424, ack & 0xffff);
		} else {
			set_ath10kreg(dev, 0x1070, slot);
			set_ath10kreg(dev, 0x1030, sifs);
			set_ath10kreg(dev, 0x8014, ((cts << 16) & 0x3fff0000) | (ack & 0x3fff));
		}
	}

}

unsigned int get_ath10kack(char *ifname)
{
	unsigned int isb = is_beeliner(ifname);
	unsigned int macclk = isb ? 142 : 88;
	unsigned int ack, slot, sifs, baseslot = 9;
	/* since qualcom/atheros missed to implement one of the most important features in wireless devices, we need this evil hack here */
	if (isb) {
		baseslot = (get_ath10kreg(ifname, 0x0040));
		ack = (get_ath10kreg(ifname, 0xf424) & 0xffff);
		sifs = ((get_ath10kreg(ifname, 0xf420) >> 8) & 0x1ff);
	} else {
		slot = (get_ath10kreg(ifname, 0x1070)) / macclk;
		ack = (get_ath10kreg(ifname, 0x8014) & 0x3fff) / macclk;
		sifs = (get_ath10kreg(ifname, 0x1030)) / macclk;
	}
	ack -= sifs;
	ack -= baseslot;
	return ack;
}

unsigned int get_ath10kdistance(char *ifname)
{
	unsigned int distance, ack;
	ack = get_ath10kack(ifname);
	distance = ack;
	distance /= 3;
	distance *= 450;
	return distance;
}

#endif
#if 0
int getFrequency_mac80211(char *interface)
{
	struct nl_msg *msg;
	struct mac80211_info mac80211_info;
	int wdev = if_nametoindex(interface);
	memset(&mac80211_info, 0, sizeof(mac80211_info));
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, msg, mac80211_cb_survey, &mac80211_info);
	return mac80211_info.frequency;
nla_put_failure:
	nlmsg_free(msg);
	return (0);
}
#endif
int mac80211_get_coverageclass(char *interface)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	int phy;
	unsigned char coverage = 0;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1)
		return 0;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return 0;
	if (!msg)
		return 0;
	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]) {
		coverage = nla_get_u8(tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]);
	}
	nlmsg_free(msg);
	return coverage;
nla_put_failure:
	nlmsg_free(msg);
	return 0;
}

static int mac80211_cb_stations(struct nl_msg *msg, void *data)
{
	// struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nl80211_sta_flag_update *sta_flags;
	char mac_addr[20], dev[20];
	struct mac80211_info *mac80211_info = data;
	mac80211_info->wci = add_to_wifi_clients(mac80211_info->wci);
	// struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = {.type = NLA_U32},
		[NL80211_STA_INFO_RX_BYTES] = {.type = NLA_U32},
		[NL80211_STA_INFO_TX_BYTES] = {.type = NLA_U32},
		[NL80211_STA_INFO_RX_PACKETS] = {.type = NLA_U32},
		[NL80211_STA_INFO_TX_PACKETS] = {.type = NLA_U32},
		[NL80211_STA_INFO_SIGNAL] = {.type = NLA_U8},
		[NL80211_STA_INFO_TX_BITRATE] = {.type = NLA_NESTED},
		[NL80211_STA_INFO_RX_BITRATE] = {.type = NLA_NESTED},
		[NL80211_STA_INFO_LLID] = {.type = NLA_U16},
		[NL80211_STA_INFO_PLID] = {.type = NLA_U16},
		[NL80211_STA_INFO_PLINK_STATE] = {.type = NLA_U8},
		[NL80211_STA_INFO_CONNECTED_TIME] = {.type = NLA_U32},
		[NL80211_STA_INFO_STA_FLAGS] = {.minlen = sizeof(struct nl80211_sta_flag_update)},
	};
	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = {.type = NLA_U16},
		[NL80211_RATE_INFO_MCS] = {.type = NLA_U8},
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = {.type = NLA_FLAG},
		[NL80211_RATE_INFO_SHORT_GI] = {.type = NLA_FLAG},
#ifdef NL80211_VHT_CAPABILITY_LEN
		[NL80211_RATE_INFO_BITRATE32] = {.type = NLA_U32},
		[NL80211_RATE_INFO_80_MHZ_WIDTH] = {.type = NLA_FLAG},
		[NL80211_RATE_INFO_80P80_MHZ_WIDTH] = {.type = NLA_FLAG},
		[NL80211_RATE_INFO_160_MHZ_WIDTH] = {.type = NLA_FLAG},
		[NL80211_RATE_INFO_VHT_MCS] = {.type = NLA_U8},
		[NL80211_RATE_INFO_VHT_NSS] = {.type = NLA_U8},
//              [NL80211_RATE_INFO_10_MHZ_WIDTH] = {.type = NLA_FLAG},
//              [NL80211_RATE_INFO_5_MHZ_WIDTH] = {.type = NLA_FLAG},
#endif
	};

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX, tb[NL80211_ATTR_STA_INFO], stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}
	ether_etoa(nla_data(tb[NL80211_ATTR_MAC]), mac_addr);
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	strcpy(mac80211_info->wci->mac, mac_addr);
	strcpy(mac80211_info->wci->ifname, dev);
	mac80211_info->wci->noise = mac80211_info->noise;
	if (strstr(dev, ".sta"))
		mac80211_info->wci->is_wds = 1;
	if (sinfo[NL80211_STA_INFO_INACTIVE_TIME]) {
		mac80211_info->wci->inactive_time = nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]);
	}
	if (sinfo[NL80211_STA_INFO_RX_BYTES]) {
		mac80211_info->wci->rx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]);
	}
	if (sinfo[NL80211_STA_INFO_RX_PACKETS]) {
		mac80211_info->wci->rx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);
	}
	if (sinfo[NL80211_STA_INFO_TX_BYTES]) {
		mac80211_info->wci->tx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]);
	}
	if (sinfo[NL80211_STA_INFO_TX_PACKETS]) {
		mac80211_info->wci->tx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);
	}
	if (sinfo[NL80211_STA_INFO_SIGNAL]) {
		mac80211_info->wci->signal = (int8_t) nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
	}

	if (sinfo[NL80211_STA_INFO_CONNECTED_TIME]) {
		mac80211_info->wci->uptime = nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]);
	}

	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX, sinfo[NL80211_STA_INFO_TX_BITRATE], rate_policy)) {
			fprintf(stderr, "failed to parse nested tx rate attributes!\n");
		} else {
			if (rinfo[NL80211_RATE_INFO_BITRATE]) {
				mac80211_info->wci->txrate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
			}

			if (rinfo[NL80211_RATE_INFO_MCS]) {
				mac80211_info->wci->mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]);
				mac80211_info->wci->is_ht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH]) {
				mac80211_info->wci->is_40mhz = 1;
			}
#ifdef NL80211_VHT_CAPABILITY_LEN
			if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH]) {
				mac80211_info->wci->is_80mhz = 1;
			}
			if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH]) {
				mac80211_info->wci->is_160mhz = 1;
			}
			if (rinfo[NL80211_RATE_INFO_VHT_MCS]) {
				mac80211_info->wci->is_vht = 1;
			}
#endif
			if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
				mac80211_info->wci->is_short_gi = 1;
			}
		}
	}
	if (sinfo[NL80211_STA_INFO_STA_FLAGS]) {
		sta_flags = (struct nl80211_sta_flag_update *)
		    nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);
		if (sta_flags->mask & BIT(8))	// may work later. but not yet
			mac80211_info->wci->ht40intol = 1;

	}
	if (sinfo[NL80211_STA_INFO_RX_BITRATE]) {
		if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX, sinfo[NL80211_STA_INFO_RX_BITRATE], rate_policy)) {
			fprintf(stderr, "failed to parse nested rx rate attributes!\n");
		} else {
			if (rinfo[NL80211_RATE_INFO_BITRATE]) {
				mac80211_info->wci->rxrate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
			}

			if (rinfo[NL80211_RATE_INFO_MCS]) {
				mac80211_info->wci->rx_mcs = nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]);
				mac80211_info->wci->rx_is_ht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH]) {
				mac80211_info->wci->rx_is_40mhz = 1;
			}
#ifdef NL80211_VHT_CAPABILITY_LEN
			if (rinfo[NL80211_RATE_INFO_80_MHZ_WIDTH]) {
				mac80211_info->wci->rx_is_80mhz = 1;
			}
			if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH]) {
				mac80211_info->wci->rx_is_160mhz = 1;
			}
			if (rinfo[NL80211_RATE_INFO_VHT_MCS]) {
				mac80211_info->wci->rx_is_vht = 1;
			}
#endif
			if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
				mac80211_info->wci->rx_is_short_gi = 1;
			}
		}
	}
	return (0);
}

struct mac80211_info *mac80211_assoclist(char *interface)
{
	struct nl_msg *msg;
	glob_t globbuf;
	char globstring[1024];
	int globresult;
	struct mac80211_info *mac80211_info = calloc(1, sizeof(struct mac80211_info));
	if (interface)
		sprintf(globstring, "/sys/class/ieee80211/phy*/device/net/%s*", interface);
	else
		sprintf(globstring, "/sys/class/ieee80211/phy*/device/net/*");
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	int i;
	for (i = 0; i < globbuf.gl_pathc; i++) {
		char *ifname;
		ifname = strrchr(globbuf.gl_pathv[i], '/');
		if (!ifname)
			continue;
		// get noise for the actaul interface
		getNoise_mac80211_internal(ifname + 1, mac80211_info);
		msg = unl_genl_msg(&unl, NL80211_CMD_GET_STATION, true);
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(ifname + 1));
		unl_genl_request(&unl, msg, mac80211_cb_stations, mac80211_info);
	}
	// print_wifi_clients(mac80211_info->wci);
	// free_wifi_clients(mac80211_info->wci);
	globfree(&globbuf);
	return (mac80211_info);
nla_put_failure:
	nlmsg_free(msg);
	return (mac80211_info);
}

char *mac80211_get_caps(char *interface, int shortgi)
{
	struct nl_msg *msg;
	struct nlattr *caps, *bands, *band;
	int rem;
	u16 cap;
	char *capstring = NULL;
	int phy;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		return strdup("");
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return "";
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		caps = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_HT_CAPA);
		if (!caps)
			continue;
		cap = nla_get_u16(caps);
		asprintf(&capstring, "%s%s%s%s%s%s%s%s", (cap & HT_CAP_INFO_LDPC_CODING_CAP ? "[LDPC]" : "")
			 , (((cap & HT_CAP_INFO_SHORT_GI20MHZ) && shortgi) ? "[SHORT-GI-20]" : "")
			 , (((cap & HT_CAP_INFO_SHORT_GI40MHZ) && shortgi) ? "[SHORT-GI-40]" : "")
			 , (cap & HT_CAP_INFO_TX_STBC ? "[TX-STBC]" : "")
			 , (((cap >> 8) & 0x3) == 1 ? "[RX-STBC1]" : "")
			 , (((cap >> 8) & 0x3) == 2 ? "[RX-STBC12]" : "")
			 , (((cap >> 8) & 0x3) == 3 ? "[RX-STBC123]" : "")
			 , (cap & HT_CAP_INFO_DSSS_CCK40MHZ ? "[DSSS_CCK-40]" : "")
		    );
	}
out:
nla_put_failure:
	nlmsg_free(msg);
	if (!capstring)
		return strdup("");
	return capstring;
}

#ifdef HAVE_ATH10K

char *mac80211_get_vhtcaps(char *interface, int shortgi)
{
	struct nl_msg *msg;
	struct nlattr *caps, *bands, *band;
	int rem;
	u32 cap;
	char *capstring = NULL;
	int phy;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		return strdup("");
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return "";
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		caps = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_VHT_CAPA);
		if (!caps)
			continue;
		cap = nla_get_u32(caps);
		asprintf(&capstring, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s[MAX-A-MPDU-LEN-EXP%d]", (cap & VHT_CAP_RXLDPC ? "[RXLDPC]" : "")
			 , (((cap & VHT_CAP_SHORT_GI_80) && shortgi) ? "[SHORT-GI-80]" : "")
			 , (((cap & VHT_CAP_SHORT_GI_160) && shortgi) ? "[SHORT-GI-160]" : "")
			 , (cap & VHT_CAP_TXSTBC ? "[TX-STBC-2BY1]" : "")
			 , (((cap >> 8) & 0x7) == 1 ? "[RX-STBC1]" : "")
			 , (((cap >> 8) & 0x7) == 2 ? "[RX-STBC12]" : "")
			 , (((cap >> 8) & 0x7) == 3 ? "[RX-STBC123]" : "")
			 , (((cap >> 8) & 0x7) == 4 ? "[RX-STBC1234]" : "")
			 , (cap & VHT_CAP_SU_BEAMFORMER_CAPABLE ? "[SU-BEAMFORMER]" : "")
			 , (cap & VHT_CAP_SU_BEAMFORMEE_CAPABLE ? "[SU-BEAMFORMEE]" : "")
			 , (cap & VHT_CAP_MU_BEAMFORMER_CAPABLE ? "[MU-BEAMFORMER]" : "")
			 , (cap & VHT_CAP_MU_BEAMFORMEE_CAPABLE ? "[MU-BEAMFORMEE]" : "")
			 , (cap & VHT_CAP_VHT_TXOP_PS ? "[VHT-TXOP-PS]" : "")
			 , (cap & VHT_CAP_HTC_VHT ? "[HTC-VHT]" : "")
			 , (cap & VHT_CAP_RX_ANTENNA_PATTERN ? "[RX-ANTENNA-PATTERN]" : "")
			 , (cap & VHT_CAP_TX_ANTENNA_PATTERN ? "[TX-ANTENNA-PATTERN]" : "")
			 , ((cap & 3) == 1 ? "[MAX-MPDU-7991]" : "")
			 , ((cap & 3) == 2 ? "[MAX-MPDU-11454]" : "")
			 , (cap & VHT_CAP_SUPP_CHAN_WIDTH_160MHZ ? "[VHT160]" : "")
			 , (cap & VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ ? "[VHT160-80PLUS80]" : "")
			 , (((cap >> 26) & 3) == 2 ? "[VHT-LINK-ADAPT2]" : "")
			 , (((cap >> 26) & 3) == 3 ? "[VHT-LINK-ADAPT3]" : "")
			 , ((cap >> 23) & 7)
		    );
	}
out:
nla_put_failure:
	nlmsg_free(msg);
	if (!capstring)
		return strdup("");
	return capstring;
}
#endif

int has_shortgi(char *interface)
{
	char *htcaps = mac80211_get_caps(interface, 1);
	if (strstr(htcaps, "SHORT-GI")) {
		free(htcaps);
		return 1;
	}
	free(htcaps);
#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU)
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1);
	if (strstr(vhtcaps, "SHORT-GI")) {
		free(vhtcaps);
		return 1;
	}
	free(vhtcaps);
#endif
	return 0;
}

static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
	[NL80211_FREQUENCY_ATTR_FREQ] = {
					 .type = NLA_U32},
};

int mac80211_check_band(char *interface, int checkband)
{

	struct nlattr *tb[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band, *freqlist, *freq;
	int rem, rem2, freq_mhz;
	int phy;
	int bandfound = 0;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		return 0;
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return 0;
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		freqlist = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_FREQS);
		if (!freqlist)
			continue;
		nla_for_each_nested(freq, freqlist, rem2) {
			nla_parse_nested(tb, NL80211_FREQUENCY_ATTR_MAX, freq, freq_policy);
			if (!tb[NL80211_FREQUENCY_ATTR_FREQ])
				continue;
			if (tb[NL80211_FREQUENCY_ATTR_DISABLED])
				continue;
			freq_mhz = nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
			if (checkband == 2 && freq_mhz < 3000)
				bandfound = 1;
			if (checkband == 5 && freq_mhz > 3000)
				bandfound = 1;
		}
	}
	nlmsg_free(msg);
	return bandfound;
out:
nla_put_failure:
	nlmsg_free(msg);
	return 0;
}

struct wifi_channels *mac80211_get_channels(char *interface, char *country, int max_bandwidth_khz, unsigned char checkband)
{
	struct nlattr *tb[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band, *freqlist, *freq;
	struct ieee80211_regdomain *rd;
	struct ieee80211_freq_range regfreq;
	struct ieee80211_power_rule regpower;
	struct wifi_channels *list = NULL;
	int rem, rem2, freq_mhz, phy, rrc, startfreq, stopfreq, range, regmaxbw, run;
	int regfound = 0;
	int htrange = 30;
	int chancount = 0;
	int count = 0;
	char sc[32];
	int skip = 1;
	int rrdcount = 0;
	if (max_bandwidth_khz == 80)
		htrange = 60;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1)
		return NULL;
#ifdef HAVE_SUPERCHANNEL
	sprintf(sc, "%s_regulatory", interface);
	if (issuperchannel()
	    && atoi(nvram_default_get(sc, "1")) == 0)
		skip = 0;
#endif
	rd = mac80211_get_regdomain(country);
	// for now just leave 
	if (rd == NULL)
		return list;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return NULL;
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	for (run = 0; run < 2; run++) {
		if (run == 1) {
			list = (struct wifi_channels *)malloc(sizeof(struct wifi_channels) * (chancount + 1));
			(void)memset(list, 0, (sizeof(struct wifi_channels) * (chancount + 1)));
		}
		nla_for_each_nested(band, bands, rem) {
			freqlist = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_FREQS);
			if (!freqlist)
				continue;
			nla_for_each_nested(freq, freqlist, rem2) {
				nla_parse_nested(tb, NL80211_FREQUENCY_ATTR_MAX, freq, freq_policy);
				if (!tb[NL80211_FREQUENCY_ATTR_FREQ])
					continue;
				if (skip && tb[NL80211_FREQUENCY_ATTR_DISABLED])
					continue;
				regfound = 0;
				if (max_bandwidth_khz == 40 || max_bandwidth_khz == 80)
					range = 10;
				else
					// for 10/5mhz this should be fine 
					range = max_bandwidth_khz / 2;
				freq_mhz = (int)nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
				if (skip == 0)
					rrdcount = 1;
				else
					rrdcount = rd->n_reg_rules;
				for (rrc = 0; rrc < rrdcount; rrc++) {
					regfreq = rd->reg_rules[rrc].freq_range;
					startfreq = (int)((float)(regfreq.start_freq_khz) / 1000.0);
					stopfreq = (int)((float)(regfreq.end_freq_khz) / 1000.0);
					regmaxbw = (int)((float)(regfreq.max_bandwidth_khz) / 1000.0);
					if (!skip)
						regmaxbw = 40;
					else
						regmaxbw = (int)((float)(regfreq.max_bandwidth_khz) / 1000.0);
					if (!skip || ((freq_mhz - range) >= startfreq && (freq_mhz + range) <= stopfreq)) {
						if (run == 1) {
							regpower = rd->reg_rules[rrc].power_rule;
#if defined(HAVE_BUFFALO_SA) && defined(HAVE_ATH9K)
							if ((!strcmp(getUEnv("region"), "AP")
							     || !strcmp(getUEnv("region"), "US"))
							    && ieee80211_mhz2ieee(freq_mhz) > 11 && ieee80211_mhz2ieee(freq_mhz) < 14 && nvram_default_match("region", "SA", ""))
								continue;
#endif
							if (checkband == 2 && freq_mhz > 4000)
								continue;
							if (checkband == 5 && freq_mhz < 4000)
								continue;

							list[count].channel = ieee80211_mhz2ieee(freq_mhz);
							list[count].freq = freq_mhz;
							// todo: wenn wir das ueberhaupt noch verwenden
							list[count].noise = 0;
							list[count].max_eirp = (int)((float)(regpower.max_eirp) / 100.0);
							if (rd->reg_rules[rrc].flags & RRF_NO_OFDM)
								list[count].no_ofdm = 1;
							if (rd->reg_rules[rrc].flags & RRF_NO_CCK)
								list[count].no_cck = 1;
							if (rd->reg_rules[rrc].flags & RRF_NO_INDOOR)
								list[count].no_indoor = 1;
							if (rd->reg_rules[rrc].flags & RRF_NO_OUTDOOR)
								list[count].no_outdoor = 1;
							if (rd->reg_rules[rrc].flags & RRF_DFS)
								list[count].dfs = 1;
							if (rd->reg_rules[rrc].flags & RRF_PTP_ONLY)
								list[count].ptp_only = 1;
							if (rd->reg_rules[rrc].flags & RRF_PTMP_ONLY)
								list[count].ptmp_only = 1;
							if (rd->reg_rules[rrc].flags & RRF_PASSIVE_SCAN)
								list[count].passive_scan = 1;
							if (rd->reg_rules[rrc].flags & RRF_NO_IBSS)
								list[count].no_ibss = 1;
							if (regmaxbw == 40 || regmaxbw == 80) {
								if ((freq_mhz - htrange) >= startfreq) {
									list[count].ht40minus = 1;
								}
								if ((freq_mhz + htrange) <= stopfreq) {
									list[count].ht40plus = 1;
								}
							}
							count++;
						}
						if (run == 0)
							chancount++;
					}
				}
			}
		}
	}
	list[count].freq = -1;
	if (rd)
		free(rd);
	nlmsg_free(msg);
	return list;
out:
nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

int has_ht40(char *interface)
{
	struct wifi_channels *chan;
	int found = 0;
	int i = 0;
	char regdomain[32];
	char *country;
	if (is_ath5k(interface))
		return (0);
	sprintf(regdomain, "%s_regdomain", interface);
	country = nvram_default_get(regdomain, "UNITED_STATES");
	chan = mac80211_get_channels(interface, getIsoName(country), 40, 0xff);
	if (chan != NULL) {
		while (chan[i].freq != -1) {
			if (chan[i].ht40plus || chan[i].ht40minus) {
				free(chan);
				return 1;
			}
			i++;
		}
		free(chan);
	}
	return 0;
}

int mac80211_check_valid_frequency(char *interface, char *country, int freq)
{
	struct wifi_channels *chan;
	int found = 0;
	int i = 0;
	chan = mac80211_get_channels(interface, country, 40, 0xff);
	if (chan != NULL) {
		while (chan[i].freq != -1) {
			if (freq == chan[i].freq) {
				free(chan);
				return freq;
			}
			i++;
		}
		free(chan);
	}
	return (0);
}

static struct wifi_client_info *add_to_wifi_clients(struct
						    wifi_client_info
						    *list_root)
{
	struct wifi_client_info *new = calloc(1, sizeof(struct wifi_client_info));
	if (new == NULL) {
		fprintf(stderr, "add_wifi_clients: Out of memory!\n");
		return (NULL);
	} else {
		new->next = list_root;
		return (new);
	}
}

void free_wifi_clients(struct wifi_client_info *wci)
{
	while (wci) {
		struct wifi_client_info *next = wci->next;
		free(wci);
		wci = next;
	}
}

static int get_max_mcs_index(const __u8 *mcs)
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

		prev_bit = mcs_bit;
	}

	if (prev_cont)
		return prev_bit;
	return 0;
}

static int get_ht_mcs(const __u8 *mcs)
{
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
			return (get_max_mcs_index(mcs));
		} else {
			return (get_max_mcs_index(mcs));
		}
	} else {
		return (get_max_mcs_index(mcs));
	}
}

int mac80211_get_maxrate(char *interface)
{
	struct nlattr *tb[NL80211_BITRATE_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band, *ratelist, *rate;
	int rem, rem2;
	int phy;
	int maxrate = 0;
	static struct nla_policy rate_policy[NL80211_BITRATE_ATTR_MAX + 1] = {
		[NL80211_BITRATE_ATTR_RATE] = {
					       .type = NLA_U32},[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE] = {
													     .type = NLA_FLAG},
	};
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1)
		return 0;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return 0;
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		ratelist = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_RATES);
		if (!ratelist)
			continue;
		nla_for_each_nested(rate, ratelist, rem2) {
			nla_parse_nested(tb, NL80211_BITRATE_ATTR_MAX, rate, rate_policy);
			if (!tb[NL80211_BITRATE_ATTR_RATE])
				continue;
			maxrate = 0.1 * nla_get_u32(tb[NL80211_BITRATE_ATTR_RATE]);
		}
	}
	nlmsg_free(msg);
	return maxrate;
out:
nla_put_failure:
	nlmsg_free(msg);
	return 0;
}

int mac80211_get_maxmcs(char *interface)
{
	struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band;
	int rem;
	int phy;
	int maxmcs = 0;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1)
		return 0;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return 0;
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		nla_parse(tb, NL80211_BAND_ATTR_MAX, nla_data(band), nla_len(band), NULL);
		if (tb[NL80211_BAND_ATTR_HT_MCS_SET]
		    && nla_len(tb[NL80211_BAND_ATTR_HT_MCS_SET]) == 16)
			maxmcs = get_ht_mcs(nla_data(tb[NL80211_BAND_ATTR_HT_MCS_SET]));
	}
	nlmsg_free(msg);
	return maxmcs;
out:
	return 0;
nla_put_failure:
	nlmsg_free(msg);
	return 0;
}

void mac80211_set_antennas(int phy, uint32_t tx_ant, uint32_t rx_ant)
{
	struct nl_msg *msg;
	if (tx_ant == 0 || rx_ant == 0)
		return;
	msg = unl_genl_msg(&unl, NL80211_CMD_SET_WIPHY, false);
	if (!msg)
		return;
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (isap8x() && tx_ant == 5)
		tx_ant = 3;
	if (isap8x() && tx_ant == 4)
		tx_ant = 2;
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_TX, tx_ant);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_RX, rx_ant);
	unl_genl_request(&unl, msg, NULL, NULL);
	return;
nla_put_failure:
	nlmsg_free(msg);
	return;
}

static int mac80211_get_antennas(int phy, int which, int direction)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	int ret = 0;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	if (!msg)
		return 0;
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return 0;
	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (which == 0 && direction == 0) {
		if (tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX])
			ret = ((int)nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_TX]));
	}

	if (which == 0 && direction == 1) {
		if (tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX])
			ret = ((int)nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_AVAIL_RX]));
	}

	if (which == 1 && direction == 0) {
		if (tb[NL80211_ATTR_WIPHY_ANTENNA_TX])
			ret = ((int)nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_TX]));
	}

	if (which == 1 && direction == 1) {
		if (tb[NL80211_ATTR_WIPHY_ANTENNA_RX])
			ret = ((int)nla_get_u32(tb[NL80211_ATTR_WIPHY_ANTENNA_RX]));
	}
	nlmsg_free(msg);
	return ret;
nla_put_failure:
	nlmsg_free(msg);
	return 0;
}

int mac80211_get_avail_tx_antenna(int phy)
{
	int ret = mac80211_get_antennas(phy, 0, 0);
	if (isap8x() && ret == 3)
		ret = 5;
	return (ret);
}

int mac80211_get_avail_rx_antenna(int phy)
{
	return (mac80211_get_antennas(phy, 0, 1));
}

int mac80211_get_configured_tx_antenna(int phy)
{
	int ret = mac80211_get_antennas(phy, 1, 0);
	int avail = mac80211_get_antennas(phy, 0, 0);
	if (isap8x() && avail == 3 && ret == 3)
		ret = 5;
	if (isap8x() && avail == 3 && ret == 2)
		ret = 4;
	return (ret);
}

int mac80211_get_configured_rx_antenna(int phy)
{
	return (mac80211_get_antennas(phy, 1, 1));
}

struct wifi_interface *mac80211_get_interface(char *dev)
{
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	const char *indent = "";
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	int ret = 0;
	struct wifi_interface *interface = NULL;
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_INTERFACE, false);
	if (!msg)
		return NULL;
	int devidx = if_nametoindex(dev);

	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return NULL;

	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb_msg, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (tb_msg[NL80211_ATTR_WIPHY_FREQ]) {
		interface = (struct wifi_interface *)malloc(sizeof(struct wifi_interface));
		interface->freq = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_FREQ]);
		interface->width = 2;
		interface->center1 = -1;
		interface->center2 = -1;

		if (tb_msg[NL80211_ATTR_CHANNEL_WIDTH]) {

			if (tb_msg[NL80211_ATTR_CENTER_FREQ1])
				interface->center1 = nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ1]);
			if (tb_msg[NL80211_ATTR_CENTER_FREQ2])
				interface->center2 = nla_get_u32(tb_msg[NL80211_ATTR_CENTER_FREQ2]);

			switch (nla_get_u32(tb_msg[NL80211_ATTR_CHANNEL_WIDTH])) {
			case NL80211_CHAN_WIDTH_20_NOHT:
				interface->width = 2;
				break;
			case NL80211_CHAN_WIDTH_20:
				interface->width = 20;
				break;
			case NL80211_CHAN_WIDTH_40:
				interface->width = 40;
				if (interface->center1 != -1) {
					if (interface->freq > interface->center1)
						interface->center1 -= 10;
					else
						interface->center1 += 10;
				}
				break;
			case NL80211_CHAN_WIDTH_80:
				interface->width = 80;
				break;
			case NL80211_CHAN_WIDTH_80P80:
				interface->width = 8080;
				break;
			case NL80211_CHAN_WIDTH_160:
				interface->width = 160;
				break;
			case 6:
				interface->width = 5;
				break;
			case 7:
				interface->width = 10;
				break;
			}

		} else if (tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]) {
			enum nl80211_channel_type channel_type;

			channel_type = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_CHANNEL_TYPE]);
			switch (channel_type) {
			case NL80211_CHAN_NO_HT:
				interface->width = 2;
				break;
			case NL80211_CHAN_HT20:
				interface->width = 20;
				break;
			case NL80211_CHAN_HT40MINUS:
				interface->width = 40;
				break;
			case NL80211_CHAN_HT40PLUS:
				interface->width = 40;
				break;
			default:
				interface->width = 40;
				break;
			}

		}

	}
	nlmsg_free(msg);
	return interface;
nla_put_failure:
	nlmsg_free(msg);
	return interface;
}

#ifdef TEST
void main(int argc, char *argv[])
{
	fprintf(stderr, "phy0 %d %d %d %d\n", mac80211_get_avail_tx_antenna(0), mac80211_get_avail_rx_antenna(0), mac80211_get_configured_tx_antenna(0), mac80211_get_configured_rx_antenna(0));
	fprintf(stderr, "phy1 %d %d %d %d\n", mac80211_get_avail_tx_antenna(1), mac80211_get_avail_rx_antenna(1), mac80211_get_configured_tx_antenna(1), mac80211_get_configured_rx_antenna(1));
}

#endif
