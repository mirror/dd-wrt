/*
 * mac80211info.c 
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <shutils.h>
#include <utils.h>
#include <pthread.h>
#include <errno.h>

#include "unl.h"
#include "mac80211regulatory.h"
#include <nl80211.h>

#include "wlutils.h"
#include <utils.h>
#include <channelcache.h>

// some defenitions from hostapd
typedef uint16_t u16;
typedef uint32_t u32;
#define BIT(x) (1ULL << (x))
#define HT_CAP_INFO_LDPC_CODING_CAP ((u16)BIT(0))
#define HT_CAP_INFO_SUPP_CHANNEL_WIDTH_SET ((u16)BIT(1))
#define HT_CAP_INFO_SMPS_MASK ((u16)(BIT(2) | BIT(3)))
#define HT_CAP_INFO_SMPS_STATIC ((u16)0)
#define HT_CAP_INFO_SMPS_DYNAMIC ((u16)BIT(2))
#define HT_CAP_INFO_SMPS_DISABLED ((u16)(BIT(2) | BIT(3)))
#define HT_CAP_INFO_GREEN_FIELD ((u16)BIT(4))
#define HT_CAP_INFO_SHORT_GI20MHZ ((u16)BIT(5))
#define HT_CAP_INFO_SHORT_GI40MHZ ((u16)BIT(6))
#define HT_CAP_INFO_TX_STBC ((u16)BIT(7))
#define HT_CAP_INFO_RX_STBC_MASK ((u16)(BIT(8) | BIT(9)))
#define HT_CAP_INFO_RX_STBC_1 ((u16)BIT(8))
#define HT_CAP_INFO_RX_STBC_12 ((u16)BIT(9))
#define HT_CAP_INFO_RX_STBC_123 ((u16)(BIT(8) | BIT(9)))
#define HT_CAP_INFO_DELAYED_BA ((u16)BIT(10))
#define HT_CAP_INFO_MAX_AMSDU_SIZE ((u16)BIT(11))
#define HT_CAP_INFO_DSSS_CCK40MHZ ((u16)BIT(12))
#define HT_CAP_INFO_PSMP_SUPP ((u16)BIT(13))
#define HT_CAP_INFO_40MHZ_INTOLERANT ((u16)BIT(14))
#define HT_CAP_INFO_LSIG_TXOP_PROTECT_SUPPORT ((u16)BIT(15))

#define VHT_CAP_MAX_MPDU_LENGTH_7991 ((u32)BIT(0))
#define VHT_CAP_MAX_MPDU_LENGTH_11454 ((u32)BIT(1))
#define VHT_CAP_SUPP_CHAN_WIDTH_160MHZ ((u32)BIT(2))
#define VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ ((u32)BIT(3))
#define VHT_CAP_RXLDPC ((u32)BIT(4))
#define VHT_CAP_SHORT_GI_80 ((u32)BIT(5))
#define VHT_CAP_SHORT_GI_160 ((u32)BIT(6))
#define VHT_CAP_TXSTBC ((u32)BIT(7))
#define VHT_CAP_RXSTBC_1 ((u32)BIT(8))
#define VHT_CAP_RXSTBC_2 ((u32)BIT(9))
#define VHT_CAP_RXSTBC_3 ((u32)BIT(8) | BIT(9))
#define VHT_CAP_RXSTBC_4 ((u32)BIT(10))
#define VHT_CAP_SU_BEAMFORMER_CAPABLE ((u32)BIT(11))
#define VHT_CAP_SU_BEAMFORMEE_CAPABLE ((u32)BIT(12))
#define VHT_CAP_MU_BEAMFORMER_CAPABLE ((u32)BIT(19))
#define VHT_CAP_MU_BEAMFORMEE_CAPABLE ((u32)BIT(20))
#define VHT_CAP_VHT_TXOP_PS ((u32)BIT(21))
#define VHT_CAP_HTC_VHT ((u32)BIT(22))
#define VHT_CAP_MAX_A_MPDU_LENGTH_EXPONENT ((u32)BIT(23))
#define VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB ((u32)BIT(27))
#define VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB ((u32)BIT(26) | BIT(27))
#define VHT_CAP_RX_ANTENNA_PATTERN ((u32)BIT(28))
#define VHT_CAP_TX_ANTENNA_PATTERN ((u32)BIT(29))

#ifndef HAVE_MICRO
static pthread_mutex_t mutex_unl = PTHREAD_MUTEX_INITIALIZER;
static char *lastlock;
static char *lastunlock;
#define lock() pthread_mutex_lock(&mutex_unl)
#define unlock() pthread_mutex_unlock(&mutex_unl)
#else
#define lock()
#define unlock()
#endif

void mac80211_lock(void)
{
	lock();
}

void mac80211_unlock(void)
{
	unlock();
}

static struct unl unl;
static bool bunl = false;

static void print_wifi_clients(struct wifi_client_info *wci);
void free_wifi_clients(struct wifi_client_info *wci);
static struct wifi_client_info *add_to_wifi_clients(struct wifi_client_info *list_root);
static int mac80211_cb_survey(struct nl_msg *msg, void *data);

static void __attribute__((constructor)) mac80211_init(void)
{
	if (!bunl) {
		int ret = unl_genl_init(&unl, "nl80211");
		if (!ret) {
			bunl = 1;
		}
	}
}

void special_mac80211_init(void)
{
	if (bunl) {
		unl_free(&unl);
		memset(&unl, 0, sizeof(unl));
	}
	if (!unl.family) {
		if (!unl_genl_init(&unl, "nl80211"))
		    bunl = 1;
	}
}

static int phy_lookup_by_number(int idx)
{
	int err;
	int phy = getValueFromPath("/sys/class/ieee80211/phy%d/index", idx, "%d", &err);
	if (err)
		return -1;
	return phy;
}

int mac80211_get_phyidx_by_vifname(const char *vif)
{
	return (phy_lookup_by_number(get_ath9k_phy_ifname(vif)));
}

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = { .type = NLA_U32 },
	[NL80211_SURVEY_INFO_NOISE] = { .type = NLA_U8 },
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = { .type = NLA_U64 },
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = { .type = NLA_U64 },
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
	if (sinfo[NL80211_SURVEY_INFO_IN_USE]) {
		mac80211_info->channel_active_time = (unsigned long long)-1;
		mac80211_info->channel_busy_time = (unsigned long long)-1;
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

		if (sinfo[NL80211_SURVEY_INFO_NOISE]) {
			mac80211_info->noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
#ifdef HAVE_MVEBU
			mac80211_info->noise -= 10;
#endif
		}

		if (sinfo[NL80211_SURVEY_INFO_FREQUENCY])
			mac80211_info->frequency = freq;
	}
	if (!mac80211_info->noise) {
#ifdef HAVE_MVEBU
		mac80211_info->noise = -104;
#else
		mac80211_info->noise = -95;
#endif
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

struct mac80211_info *getcurrentsurvey_mac80211(const char *interface, struct mac80211_info *mac80211_info)
{
	mac80211_init();
	lock();
	struct nl_msg *msg;
	int wdev = if_nametoindex(interface);
	bzero(mac80211_info, sizeof(struct mac80211_info));
#ifdef HAVE_MVEBU
	mac80211_info->noise = -104;
#else
	mac80211_info->noise = -95;
#endif
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, msg, mac80211_cb_survey, mac80211_info);
	unlock();
	return mac80211_info;

nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return NULL;
}

#ifdef HAVE_ATH10K
#if 0
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
	if (!isb)
		return;
	unsigned int sifs_pipeline;
	if ((int)distance == -1)
		return;
	if (slot == 0)		// too low value. 
		return;

	if (!isb) {
		ack *= macclk;	// 88Mhz is the core clock of AR9880
		cts *= macclk;
		slot *= macclk;
		sifs *= macclk;
	} else {
		sifs_pipeline = (sifs * 150) - ((400 * 150) / 1000);
		sifs = (sifs * 80) - 11;
	}
	if (!isb && ack > 0x3fff) {
		fprintf(stderr, "invalid ack 0x%08x, max is 0x3fff. truncate it\n", ack);
		ack = 0x3fff;
		cts = 0x3fff;
	} else if (isb && ack > 0xff) {
		fprintf(stderr, "invalid ack 0x%08x, max is 0xff. truncate it\n", ack);
		ack = 0xff;
		cts = 0xff;
	}

	unsigned int oldack;
	if (isb)
		oldack = get_ath10kreg(dev, 0x6000) & 0xff;
	else
		oldack = get_ath10kreg(dev, 0x8014) & 0x3fff;
	if (oldack != ack) {
		if (isb) {
			set_ath10kreg(dev, 0x0040, slot);	// slot timing
			set_ath10kreg(dev, 0xf56c, sifs_pipeline);
			set_ath10kreg(dev, 0xa000, sifs);
			unsigned int mask = get_ath10kreg(dev, 0x6000);
			mask &= 0xffff0000;
			set_ath10kreg(dev, 0x6000, mask | ack | (cts << 8));
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
		//      baseslot = get_ath10kreg(ifname, 0x0040);
		ack = get_ath10kreg(ifname, 0x6000) & 0xff;
		sifs = get_ath10kreg(ifname, 0xa000);
		sifs += 11;
		sifs /= 80;
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
#endif
#if 0
int getFrequency_mac80211(char *interface)
{
	lock();
	struct nl_msg *msg;
	struct mac80211_info mac80211_info;
	int wdev = if_nametoindex(interface);
	bzero(&mac80211_info, sizeof(mac80211_info));
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, msg, mac80211_cb_survey, &mac80211_info);
	unlock();
	return mac80211_info.frequency;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
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
	char ifname[32];
	strcpy(ifname, interface);
	char *c = strchr(ifname, '.');
	if (c)
		c[0] = 0;
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);

	if (nvram_nmatch("0", "%s_distance", ifname)) {
		char str[64];
		sprintf(str, "/sys/kernel/debug/ieee80211/phy%d/ath9k/ack_to", phy);
		FILE *fp = fopen(str, "rb");
		if (fp) {
			int ack;
			int rawack;
			char state[32];
			fscanf(fp, "%d %d %s", &rawack, &ack, state);
			fclose(fp);
			ack = (ack + 2) / 3; // do the coverage class
			ack = (ack + 1) / 2;
			unlock();
			return ack;
		}
		sprintf(str, "/sys/kernel/debug/ieee80211/phy%d/ath5k/ack_to", phy);
		fp = fopen(str, "rb");
		if (fp) {
			int ack;
			int rawack;
			char state[32];
			fscanf(fp, "%d %d %s", &rawack, &ack, state);
			fclose(fp);
			ack = (ack + 2) / 3; // do the coverage class
			ack = (ack + 1) / 2;
			unlock();
			return ack;
		}
	}

	if (phy == -1) {
		unlock();
		return 0;
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
	if (!msg) {
		unlock();
		return 0;
	}
	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]) {
		coverage = nla_get_u8(tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]);
	}
	nlmsg_free(msg);
	unlock();
	return coverage;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

int mac80211_get_maxpower(char *interface)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *tb_freq[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *nl_band;
	struct nlattr *nl_freq;
	static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
		[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },   [NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_NO_IR] = { .type = NLA_FLAG }, [__NL80211_FREQUENCY_ATTR_NO_IBSS] = { .type = NLA_FLAG },
		[NL80211_FREQUENCY_ATTR_RADAR] = { .type = NLA_FLAG }, [NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
	};
	int rem_band, rem_freq, rem_rate, rem_mode, rem_cmd, rem_ftype, rem_if;
	int phy;
	char ifname[32];
	strcpy(ifname, interface);
	char *c = strchr(ifname, '.');
	if (c)
		c[0] = 0;
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);

	if (phy == -1) {
		unlock();
		return 0;
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
	if (!msg) {
		unlock();
		return 0;
	}
	int maxpower = 0;
	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (tb[NL80211_ATTR_WIPHY_BANDS]) {
		nla_for_each_nested(nl_band, tb[NL80211_ATTR_WIPHY_BANDS], rem_band) {
			nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(nl_band), nla_len(nl_band), NULL);
			if (tb_band[NL80211_BAND_ATTR_FREQS]) {
				nla_for_each_nested(nl_freq, tb_band[NL80211_BAND_ATTR_FREQS], rem_freq) {
					nla_parse(tb_freq, NL80211_FREQUENCY_ATTR_MAX, nla_data(nl_freq), nla_len(nl_freq),
						  freq_policy);
					if (!tb_freq[NL80211_FREQUENCY_ATTR_FREQ])
						continue;
					if (tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] &&
					    !tb_freq[NL80211_FREQUENCY_ATTR_DISABLED]) {
						int p = nla_get_u32(tb_freq[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]) / 100;
						if (p > maxpower)
							maxpower = p;
					}
				}
			}
		}
	}

	nlmsg_free(msg);
	unlock();
	return maxpower;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

struct statdata {
	struct mac80211_info *mac80211_info;
	int iftype;
};

static void get_chain_signal(struct nlattr *attr_list, char *signals, size_t size)
{
	struct nlattr *attr;
	int rem;
	memset(signals, 0, size);
	if (!attr_list)
		return;
	int cnt = 0;
	nla_for_each_nested(attr, attr_list, rem) {
		signals[cnt++] = nla_get_u8(attr);
		if (cnt == size)
			break;
	}
}

static int mac80211_cb_stations(struct nl_msg *msg, void *data)
{
	// struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nl80211_sta_flag_update *sta_flags;
	char dev[20];
	struct statdata *d = data;
	struct mac80211_info *mac80211_info = d->mac80211_info;
	mac80211_info->wci = add_to_wifi_clients(mac80211_info->wci);
	// struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
		[NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_BEACON_RX] = { .type = NLA_U64 },
		[NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_SIGNAL_AVG] = { .type = NLA_U8 },
		[NL80211_STA_INFO_T_OFFSET] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_BITRATE] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
		[NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
		[NL80211_STA_INFO_TX_RETRIES] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_FAILED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_BEACON_LOSS] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_DROP_MISC] = { .type = NLA_U64 },
		[NL80211_STA_INFO_STA_FLAGS] = { .minlen = sizeof(struct nl80211_sta_flag_update) },
		[NL80211_STA_INFO_LOCAL_PM] = { .type = NLA_U32 },
		[NL80211_STA_INFO_PEER_PM] = { .type = NLA_U32 },
		[NL80211_STA_INFO_NONPEER_PM] = { .type = NLA_U32 },
		[NL80211_STA_INFO_CHAIN_SIGNAL] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_CHAIN_SIGNAL_AVG] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_TID_STATS] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_BSS_PARAM] = { .type = NLA_NESTED },
		[NL80211_STA_INFO_RX_DURATION] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_DURATION] = { .type = NLA_U64 },
		[NL80211_STA_INFO_ACK_SIGNAL] = { .type = NLA_U8 },
		[NL80211_STA_INFO_ACK_SIGNAL_AVG] = { .type = NLA_U8 },
		[NL80211_STA_INFO_AIRTIME_LINK_METRIC] = { .type = NLA_U32 },
		[NL80211_STA_INFO_CONNECTED_TO_AS] = { .type = NLA_U8 },
		[NL80211_STA_INFO_CONNECTED_TO_GATE] = { .type = NLA_U8 },
		[NL80211_STA_INFO_RX_COMPRESSED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_TX_COMPRESSED] = { .type = NLA_U32 },
		[NL80211_STA_INFO_RX_COMPRESSED_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_TX_COMPRESSED_BYTES64] = { .type = NLA_U64 },
		[NL80211_STA_INFO_RADIONAME] = { .type = NLA_STRING },
	};

	static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
		[NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
		[NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
#ifdef NL80211_VHT_CAPABILITY_LEN
		[NL80211_RATE_INFO_BITRATE32] = { .type = NLA_U32 },
		[NL80211_RATE_INFO_80_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_80P80_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_160_MHZ_WIDTH] = { .type = NLA_FLAG },
		[NL80211_RATE_INFO_VHT_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_VHT_NSS] = { .type = NLA_U8 },
		//              [NL80211_RATE_INFO_10_MHZ_WIDTH] = {.type = NLA_FLAG},
		//              [NL80211_RATE_INFO_5_MHZ_WIDTH] = {.type = NLA_FLAG},
		[NL80211_RATE_INFO_HE_MCS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_HE_NSS] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_HE_GI] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_HE_DCM] = { .type = NLA_U8 },
		[NL80211_RATE_INFO_HE_RU_ALLOC] = { .type = NLA_U8 },
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
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	memcpy(mac80211_info->wci->etheraddr, nla_data(tb[NL80211_ATTR_MAC]), 6);
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
	if (sinfo[NL80211_STA_INFO_RX_COMPRESSED]) {
		mac80211_info->wci->rx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_RX_COMPRESSED]);
		if (mac80211_info->wci->rx_packets)
			mac80211_info->wci->islzo = 1;
	}
	if (sinfo[NL80211_STA_INFO_TX_BYTES]) {
		mac80211_info->wci->tx_bytes = nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]);
	}
	if (sinfo[NL80211_STA_INFO_TX_PACKETS]) {
		mac80211_info->wci->tx_packets = nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);
	}
	if (sinfo[NL80211_STA_INFO_TX_COMPRESSED]) {
		mac80211_info->wci->tx_compressed = nla_get_u32(sinfo[NL80211_STA_INFO_TX_COMPRESSED]);
	}
	get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL], mac80211_info->wci->chaininfo, sizeof(mac80211_info->wci->chaininfo));
	if (sinfo[NL80211_STA_INFO_SIGNAL_AVG]) {
		mac80211_info->wci->signal = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL_AVG]);
	}
	get_chain_signal(sinfo[NL80211_STA_INFO_CHAIN_SIGNAL_AVG], mac80211_info->wci->chaininfo_avg, sizeof(mac80211_info->wci->chaininfo_avg));

	if (sinfo[NL80211_STA_INFO_DATA_ACK_SIGNAL_AVG]) {
		mac80211_info->wci->signal_avg = (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_DATA_ACK_SIGNAL_AVG]);
		if (mac80211_info->wci->signal_avg > 0)
			mac80211_info->wci->signal_avg = -1;
	} else {
		mac80211_info->wci->signal_avg = -1;
	}
	if (sinfo[NL80211_STA_INFO_CONNECTED_TIME]) {
		mac80211_info->wci->uptime = nla_get_u32(sinfo[NL80211_STA_INFO_CONNECTED_TIME]);
	}
	if (sinfo[NL80211_STA_INFO_RADIONAME]) {
		strcpy(mac80211_info->wci->radioname, nla_data(sinfo[NL80211_STA_INFO_RADIONAME]));
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
				mac80211_info->wci->is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH]) {
				mac80211_info->wci->is_160mhz = 1;
				mac80211_info->wci->is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH]) {
				mac80211_info->wci->is_80p80mhz = 1;
				mac80211_info->wci->is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_VHT_MCS]) {
				mac80211_info->wci->is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_HE_MCS]) {
				mac80211_info->wci->is_he = 1;
			}
#endif
			if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
				mac80211_info->wci->is_short_gi = 1;
			}
		}
	}
#ifdef HAVE_ATH10K
	if (d->iftype && sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]) {
		unsigned int tx = nla_get_u32(sinfo[NL80211_STA_INFO_EXPECTED_THROUGHPUT]);
		tx = tx * 1000;
		tx = tx / 1024;
		mac80211_info->wci->txrate = tx / 100;
	}
#endif
	if (sinfo[NL80211_STA_INFO_STA_FLAGS]) {
		sta_flags = (struct nl80211_sta_flag_update *)nla_data(sinfo[NL80211_STA_INFO_STA_FLAGS]);

		if (sta_flags->mask & BIT(NL80211_STA_FLAG_HT40INTOL)) {
			if (sta_flags->set & BIT(NL80211_STA_FLAG_HT40INTOL))
				mac80211_info->wci->ht40intol = 1;
			else
				mac80211_info->wci->ht40intol = 0;
		}
		if (sta_flags->mask & BIT(NL80211_STA_FLAG_PM)) {
			if (sta_flags->set & BIT(NL80211_STA_FLAG_PM))
				mac80211_info->wci->ps = 1;
			else
				mac80211_info->wci->ps = 0;
		}
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
				mac80211_info->wci->rx_is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_160_MHZ_WIDTH]) {
				mac80211_info->wci->rx_is_160mhz = 1;
				mac80211_info->wci->rx_is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_80P80_MHZ_WIDTH]) {
				mac80211_info->wci->rx_is_80p80mhz = 1;
				mac80211_info->wci->rx_is_vht = 1;
			}
			if (rinfo[NL80211_RATE_INFO_VHT_MCS]) {
				mac80211_info->wci->rx_is_vht = 1;
			}

			if (rinfo[NL80211_RATE_INFO_HE_MCS]) {
				mac80211_info->wci->rx_is_he = 1;
			}
#endif

			if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
				mac80211_info->wci->rx_is_short_gi = 1;
			}
		}
	}
	return (NL_SKIP);
}

struct mac80211_info *mac80211_assoclist(char *interface)
{
	mac80211_init();
	struct nl_msg *msg;
	glob_t globbuf;
	char *globstring;
	int globresult;
	struct statdata data;
	lock();
	data.mac80211_info = calloc(1, sizeof(struct mac80211_info));
	if (interface)
		asprintf(&globstring, "/sys/class/ieee80211/phy*/device/net/%s*", interface);
	else
		asprintf(&globstring, "/sys/class/ieee80211/phy*/device/net/*");
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	free(globstring);
	int i;
	char *history = NULL;
	for (i = 0; i < globbuf.gl_pathc; i++) {
		char *ifname;
		ifname = strrchr(globbuf.gl_pathv[i], '/');
		if (!ifname)
			continue;
		if (history) {
			char *next;
			char ifcheck[64];
			foreach(ifcheck, history, next)
			{
				if (!strcmp(ifcheck, ifname + 1))
					goto skip;
			}
		}
		// get noise for the actual interface
		getNoise_mac80211_internal(ifname + 1, data.mac80211_info);
		msg = unl_genl_msg(&unl, NL80211_CMD_GET_STATION, true);
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(ifname + 1));
		data.iftype = 0;
		if (is_ath10k(ifname + 1))
			data.iftype = 1;
		if (is_ath11k(ifname + 1))
			data.iftype = 1;
		unl_genl_request(&unl, msg, mac80211_cb_stations, &data);

		char *oldhistory = history;
		if (oldhistory) {
			asprintf(&history, "%s %s", oldhistory, ifname + 1);
			free(oldhistory);
			oldhistory = NULL;
		} else {
			asprintf(&history, "%s", ifname + 1);
		}
skip:;
	}
	if (history)
		free(history);
	// print_wifi_clients(mac80211_info->wci);
	// free_wifi_clients(mac80211_info->wci);
	globfree(&globbuf);
	unlock();
	return (data.mac80211_info);
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return (data.mac80211_info);
}

char *mac80211_get_caps(const char *interface, int shortgi, int greenfield, int ht40, int ldpc, int smps)
{
	mac80211_init();
	struct nl_msg *msg;
	struct nlattr *caps, *bands, *band;
	int rem;
	u16 cap;
	char *capstring = NULL;
	int phy;
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return strdup("");
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return strdup("");
	}
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		caps = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_HT_CAPA);
		if (!caps)
			continue;
		cap = nla_get_u16(caps);
		asprintf(&capstring, "%s%s%s%s%s%s%s%s%s%s%s%s%s", ((cap & HT_CAP_INFO_LDPC_CODING_CAP) && ldpc ? "[LDPC]" : ""),
			 (((cap & HT_CAP_INFO_SHORT_GI20MHZ) && shortgi) ? "[SHORT-GI-20]" : ""),
			 (((cap & HT_CAP_INFO_SHORT_GI40MHZ) && shortgi) ? "[SHORT-GI-40]" : ""),
			 (cap & HT_CAP_INFO_TX_STBC ? "[TX-STBC]" : ""), (((cap >> 8) & 0x3) == 1 ? "[RX-STBC1]" : ""),
			 (((cap >> 8) & 0x3) == 2 ? "[RX-STBC12]" : ""), (((cap >> 8) & 0x3) == 3 ? "[RX-STBC123]" : ""),
			 ((cap & HT_CAP_INFO_DSSS_CCK40MHZ) ? "[DSSS_CCK-40]" : ""),
			 ((cap & HT_CAP_INFO_GREEN_FIELD && greenfield) ? "[GF]" : ""),
			 (cap & HT_CAP_INFO_DELAYED_BA ? "[DELAYED-BA]" : ""),
			 ((((cap >> 2) & 0x3) == 0 && !is_brcmfmac(interface)) ? smps == 1 ? "[SMPS-STATIC]" : "" : ""),
			 (((cap >> 2) & 0x3) == 1 ? smps == 2 ? "[SMPS-DYNAMIC]" : "" : ""),
			 (cap & HT_CAP_INFO_MAX_AMSDU_SIZE ? "[MAX-AMSDU-7935]" : ""));
	}
out:
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	if (!capstring)
		return strdup("");
	return capstring;
}

#ifndef HAVE_SUPERCHANNEL
int inline issuperchannel(void)
{
	return 0;
}
#else
int issuperchannel(void);

#endif

static int cansuperchannel(const char *prefix)
{
	return (issuperchannel() && nvram_nmatch("0", "%s_regulatory", prefix) && nvram_nmatch("ddwrt", "%s_fwtype", prefix));
}

#if 0
static char *mac80211_get_hecaps(const char *interface)
{
	struct nlattr *tb_band;
	struct nlattr *tb;
	unsigned short mac_cap[3] = { 0 };
	unsigned short phy_cap[6] = { 0 };
	unsigned short mcs_set[6] = { 0 };
	mac80211_init();
	struct nl_msg *msg;
	struct nlattr *caps, *bands, *band;
	size_t len;
	char *capstring = NULL;
	int rem;
	int phy;
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return strdup("");
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	nlmsg_hdr(msg)->nlmsg_flags |= NLM_F_DUMP;
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	nla_put_flag(msg, NL80211_ATTR_SPLIT_WIPHY_DUMP);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return strdup("");
	}

	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		tb_band = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_IFTYPE_DATA);
		fprintf(stderr, "%s:%d %X %d\n", __func__, __LINE__, nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_VHT_CAPA), nla_len(band));
		fprintf(stderr, "%s:%d %X\n", __func__, __LINE__, nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_IFTYPE_DATA));
		if (tb_band) {
			struct nlattr *nl_iftype;
			int rem_band;
			fprintf(stderr, "%s:%d\n", __func__, __LINE__);

			nla_for_each_nested(nl_iftype, tb_band, rem_band) {
				fprintf(stderr, "%s:%d\n", __func__, __LINE__);
				tb = nla_find(nla_data(nl_iftype), nla_len(nl_iftype), NL80211_BAND_IFTYPE_ATTR_HE_CAP_PHY);
				if (tb) {
					len = nla_len(tb);
					fprintf(stderr, "%s:%d len %d\n", __func__, __LINE__, len);

					if (len > sizeof(phy_cap) - 1)
						len = sizeof(phy_cap) - 1;
					memcpy(&((__u8 *)phy_cap)[1], nla_data(tb), len);
				}

			}
		}
	}
	asprintf(&capstring, "%s%s%s%s", phy_cap[0] & (1 << 10) ? "[HE80]" : "", phy_cap[0] & (1 << 10) ? "[HE40]" : "", phy_cap[0] & (1 << 11) ? "[HE160]" : "", phy_cap[0] & (1 << 12) ? "[HE160][HE80+80]" : "");

out:
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	if (!capstring)
		return strdup("");
	return capstring;
}
#endif

static char *mac80211_get_hecaps(const char *interface)
{
	char *capstring = NULL;
	if (is_ath11k(interface)) {
		asprintf(&capstring, "[HE80][HE40][HE160][HE160][HE80+80]");
	}
	return capstring;
}

char *mac80211_get_vhtcaps(const char *interface, int shortgi, int vht80, int vht160, int vht8080, int su_bf, int mu_bf)
{
	mac80211_init();
	struct nl_msg *msg;
	struct nlattr *caps, *bands, *band;
	int rem;
	u32 cap;
	char *capstring = NULL;
	int phy;
	int has5ghz = has_5ghz(interface) || cansuperchannel(interface);
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return strdup("");
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return strdup("");
	}
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		caps = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_VHT_CAPA);
		if (!caps)
			continue;
		cap = nla_get_u32(caps);
		unsigned int bfantenna = (cap >> 13) & 0x7;
		unsigned int sodimension = (cap >> 16) & 0x7;
#if 0
		if (bfantenna & 4)
			bfantenna &= 4;
		if (bfantenna & 2)
			bfantenna &= 2;

		if (sodimension & 4)
			sodimension &= 4;
		if (sodimension & 2)
			sodimension &= 2;
#endif
		asprintf(&capstring, "%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s[MAX-A-MPDU-LEN-EXP%d]%s%s%s%s%s%s",
			 (cap & VHT_CAP_RXLDPC ? "[RXLDPC]" : ""),
			 (((cap & VHT_CAP_SHORT_GI_80) && shortgi && has5ghz && (vht80 || vht160)) ? "[SHORT-GI-80]" : ""),
			 (((cap & VHT_CAP_SHORT_GI_160) && shortgi && has5ghz && vht160) ? "[SHORT-GI-160]" : ""),
			 (cap & VHT_CAP_TXSTBC ? "[TX-STBC-2BY1]" : ""), (((cap >> 8) & 0x7) == 1 ? "[RX-STBC-1]" : ""),
			 (((cap >> 8) & 0x7) == 2 ? "[RX-STBC-12]" : ""), (((cap >> 8) & 0x7) == 3 ? "[RX-STBC-123]" : ""),
			 (((cap >> 8) & 0x7) == 4 ? "[RX-STBC-1234]" : ""),
			 (((cap & VHT_CAP_SU_BEAMFORMER_CAPABLE) && su_bf) ? "[SU-BEAMFORMER]" : ""),
			 (((cap & VHT_CAP_MU_BEAMFORMER_CAPABLE) && mu_bf) ? "[MU-BEAMFORMER]" : ""),
			 (((cap & VHT_CAP_SU_BEAMFORMEE_CAPABLE) && su_bf) ? "[SU-BEAMFORMEE]" : "")
			 //                       , (((cap & VHT_CAP_MU_BEAMFORMEE_CAPABLE) && mu_bf) ? "[MU-BEAMFORMEE]" : "")
			 ,
			 (cap & VHT_CAP_VHT_TXOP_PS ? "[VHT-TXOP-PS]" : ""), (cap & VHT_CAP_HTC_VHT ? "[HTC-VHT]" : ""),
			 (cap & VHT_CAP_RX_ANTENNA_PATTERN ? "[RX-ANTENNA-PATTERN]" : ""),
			 (cap & VHT_CAP_TX_ANTENNA_PATTERN ? "[TX-ANTENNA-PATTERN]" : ""),
			 ((cap & 3) == 1 ? "[MAX-MPDU-7991]" : ""), ((cap & 3) == 2 ? "[MAX-MPDU-11454]" : ""),
			 (((cap & VHT_CAP_SUPP_CHAN_WIDTH_160MHZ) && has5ghz && vht160) ? "[VHT160]" : ""),
			 (((cap & VHT_CAP_SUPP_CHAN_WIDTH_160_80PLUS80MHZ) && has5ghz && (vht8080 || vht160)) ?
				  "[VHT160-80PLUS80]" :
				  ""),
			 ((cap & VHT_CAP_HTC_VHT) ? ((cap & VHT_CAP_VHT_LINK_ADAPTATION_VHT_UNSOL_MFB) ? "[VHT-LINK-ADAPT2]" : "") :
						    ""),
			 ((cap & VHT_CAP_HTC_VHT) ? ((cap & VHT_CAP_VHT_LINK_ADAPTATION_VHT_MRQ_MFB) ? "[VHT-LINK-ADAPT3]" : "") :
						    ""),
			 ((cap >> 23) & 7),
			 (((cap & VHT_CAP_SU_BEAMFORMEE_CAPABLE) && su_bf) || ((cap & VHT_CAP_MU_BEAMFORMEE_CAPABLE) && mu_bf)) &&
					 (bfantenna & 1) ?
				 "[BF-ANTENNA-2]" :
				 "",
			 (((cap & VHT_CAP_SU_BEAMFORMER_CAPABLE) && su_bf) || ((cap & VHT_CAP_MU_BEAMFORMER_CAPABLE) && mu_bf)) &&
					 (sodimension & 1) ?
				 "[SOUNDING-DIMENSION-2]" :
				 "",
			 (((cap & VHT_CAP_SU_BEAMFORMEE_CAPABLE) && su_bf) || ((cap & VHT_CAP_MU_BEAMFORMEE_CAPABLE) && mu_bf)) &&
					 (bfantenna & 2) ?
				 "[BF-ANTENNA-3]" :
				 "",
			 (((cap & VHT_CAP_SU_BEAMFORMER_CAPABLE) && su_bf) || ((cap & VHT_CAP_MU_BEAMFORMER_CAPABLE) && mu_bf)) &&
					 (sodimension & 2) ?
				 "[SOUNDING-DIMENSION-3]" :
				 "",
			 (((cap & VHT_CAP_SU_BEAMFORMEE_CAPABLE) && su_bf) || ((cap & VHT_CAP_MU_BEAMFORMEE_CAPABLE) && mu_bf)) &&
					 (bfantenna & 4) ?
				 "[BF-ANTENNA-4]" :
				 "",
			 (((cap & VHT_CAP_SU_BEAMFORMER_CAPABLE) && su_bf) || ((cap & VHT_CAP_MU_BEAMFORMER_CAPABLE) && mu_bf)) &&
					 (sodimension & 4) ?
				 "[SOUNDING-DIMENSION-4]" :
				 "");
	}
out:
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	if (!capstring)
		return strdup("");
	return capstring;
}

#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
int has_vht160(const char *interface)
{
	INITVALUECACHEi(interface);
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1, 1, 1, 1, 1, 1);
	if (strstr(vhtcaps, "VHT160")) {
		free(vhtcaps);
		RETURNVALUE(1);
	}
	if (strstr(vhtcaps, "VHT160-80PLUS80")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
	EXITVALUECACHE();
	return ret;
}
#endif

#if defined(HAVE_ATH11K)
int has_he160(const char *interface)
{
	return 0;
#if 0
	//if vht caps do not support vht160, he160 will not work anyway. we dont need this function
	INITVALUECACHEi(interface);
	char *hecaps = mac80211_get_hecaps(interface);
	if (strstr(hecaps, "HE160")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(hecaps);
	EXITVALUECACHE();
	return ret;
#endif
}
#endif

int has_greenfield(const char *interface)
{
	INITVALUECACHEi(interface);
	char *htcaps = mac80211_get_caps(interface, 1, 1, 1, 1, 0);
	if (strstr(htcaps, "[GF]")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(htcaps);
	EXITVALUECACHE();
	return ret;
}

static void *iftype_worker(struct nlattr **tb, void *priv)
{
	struct nlattr *nl_mode;
	int rem_mode;
	if (tb[NL80211_ATTR_SUPPORTED_IFTYPES]) {
		nla_for_each_nested(nl_mode, tb[NL80211_ATTR_SUPPORTED_IFTYPES], rem_mode)
			if (nla_type(nl_mode) == *((int *)priv)) {
				return (void *)1;
			}
	}
	return (void *)0;
}

static void *acktiming_worker(struct nlattr **tb, void *priv)
{
	struct nlattr *nl_mode;
	int rem_mode;
	if (tb[NL80211_ATTR_WIPHY_COVERAGE_CLASS]) {
		return (void *)1;
	}
	return (void *)0;
}

static void *feature_worker(struct nlattr **tb, void *priv)
{
	unsigned int feature = *((int *)priv);
	if (tb[NL80211_ATTR_FEATURE_FLAGS]) {
		unsigned int features = nla_get_u32(tb[NL80211_ATTR_FEATURE_FLAGS]);
		if ((features & feature) == feature) {
			return (void *)1;
		}
	}
	return (void *)0;
}

static void *cipher_worker(struct nlattr **tb, void *priv)
{
	__u32 *ciphers = NULL;
	__u32 *num = (__u32 *)priv;
	if (tb[NL80211_ATTR_CIPHER_SUITES]) {
		*num = nla_len(tb[NL80211_ATTR_CIPHER_SUITES]) / sizeof(__u32);
		if (*num > 0) {
			ciphers = malloc(*num * sizeof(__u32));
			memcpy(ciphers, nla_data(tb[NL80211_ATTR_CIPHER_SUITES]), *num * sizeof(__u32));
		}
	}
	return ciphers;
}

static void *mac80211_has_worker(const char *prefix, void *(*worker)(struct nlattr **tb, void *priv), void *priv)
{
	mac80211_init();
	int phy = get_ath9k_phy_ifname(prefix);
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	void *ret;
	lock();
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	if (!msg) {
		unlock();
		return NULL;
	}
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		goto nla_put_failure;
	}
	gnlh = nlmsg_data(nlmsg_hdr(msg));
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	ret = worker(tb, priv);
found:;
	nlmsg_free(msg);
	unlock();
	return ret;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return NULL;
}

static __u32 *mac80211_get_ciphers(const char *prefix, __u32 *num)
{
	return (__u32 *)mac80211_has_worker(prefix, &cipher_worker, num);
}

static int mac80211_has_feature(const char *prefix, unsigned int feature)
{
	return (long)mac80211_has_worker(prefix, &feature_worker, &feature);
}

int has_smps(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_feature(prefix, NL80211_FEATURE_STATIC_SMPS);
	ret |= mac80211_has_feature(prefix, NL80211_FEATURE_DYNAMIC_SMPS);
	EXITVALUECACHE();
	return ret;
}

int has_dynamic_smps(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_feature(prefix, NL80211_FEATURE_DYNAMIC_SMPS);
	EXITVALUECACHE();
	return ret;
}

int has_static_smps(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_feature(prefix, NL80211_FEATURE_STATIC_SMPS);
	EXITVALUECACHE();
	return ret;
}

int has_uapsd(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_feature(prefix, NL80211_ATTR_SUPPORT_AP_UAPSD);
	EXITVALUECACHE();
	return ret;
}

#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
int has_vht80(const char *interface)
{
	INITVALUECACHEi(interface);
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1, 1, 1, 1, 1, 1);
	if (strstr(vhtcaps, "SHORT-GI-80")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
	EXITVALUECACHE();
	return ret;
}
#endif

#if defined(HAVE_ATH10K) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
int has_ac(const char *prefix)
{
	INITVALUECACHE();
	char *vhtcaps = mac80211_get_vhtcaps(prefix, 1, 1, 1, 1, 1, 1);
	if (*vhtcaps) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
	EXITVALUECACHE();
	return ret;
}
#endif
int has_ht(const char *prefix)
{
	INITVALUECACHE();
	char *htcaps = mac80211_get_caps(prefix, 1, 1, 1, 1, 0);
	if (*htcaps) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(htcaps);
	EXITVALUECACHE();
	return ret;
}

int has_ldpc(const char *prefix)
{
	INITVALUECACHE();
	char *htcaps = mac80211_get_caps(prefix, 1, 1, 1, 1, 0);
	if (strstr(htcaps, "LDPC")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(htcaps);
	EXITVALUECACHE();
	return ret;
}

#ifdef HAVE_WIL6210
int has_ad(const char *prefix)
{
	return (is_wil6210(prefix));
}
#endif
#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
int has_vht80plus80(const char *interface)
{
	INITVALUECACHEi(interface);
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1, 1, 1, 1, 1, 1);
	if (strstr(vhtcaps, "VHT160-80PLUS80")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
	EXITVALUECACHE();
	return ret;
}
#endif

#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
int has_subeamforming(const char *interface)
{
	INITVALUECACHEi(interface);
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1, 1, 1, 1, 1, 1);
	if (strstr(vhtcaps, "SU-BEAMFORMER") || strstr(vhtcaps, "SU-BEAMFORMEE")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
	EXITVALUECACHE();
	return ret;
}
#endif

#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU) || defined(HAVE_BRCMFMAC) || defined(HAVE_MT76)
int has_mubeamforming(const char *interface)
{
	INITVALUECACHEi(interface);
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1, 1, 1, 1, 1, 1);
	if (strstr(vhtcaps, "MU-BEAMFORMER") || strstr(vhtcaps, "MU-BEAMFORMEE")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
	EXITVALUECACHE();
	return ret;
}
#endif

int has_shortgi(const char *interface)
{
	INITVALUECACHEi(interface);
	char *htcaps = mac80211_get_caps(interface, 1, 1, 1, 1, 0);
	if (strstr(htcaps, "SHORT-GI")) {
		free(htcaps);
		RETURNVALUE(1);
	}
	free(htcaps);
#if defined(HAVE_ATH10K) || defined(HAVE_MVEBU)
	char *vhtcaps = mac80211_get_vhtcaps(interface, 1, 1, 1, 1, 1, 1);
	if (strstr(vhtcaps, "SHORT-GI")) {
		ret = 1;
	} else {
		ret = 0;
	}
	free(vhtcaps);
#endif
	EXITVALUECACHE();
	return ret;
}

static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
	[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
	[NL80211_FREQUENCY_ATTR_DISABLED] = { .type = NLA_FLAG },
	[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] = { .type = NLA_U32 },
};

int mac80211_check_band(const char *interface, int checkband)
{
	mac80211_init();
	struct nlattr *tb[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band, *freqlist, *freq;
	int rem, rem2, freq_mhz;
	int phy;
	int bandfound = 0;
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return 0;
	}

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
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
	unlock();
	return bandfound;
out:
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

static int isinlist(struct wifi_channels *list, struct wifi_channels *freq, int range, int bw)
{
	int i = 0;
	struct wifi_channels *chan;
	while ((chan = &list[i++])->freq > 0) {
		if ((bw == 40 && !chan->ht40) || (bw == 80 && !chan->vht80) || (bw == 160 && !chan->vht160))
			continue;
		if (chan->freq == freq->freq + range && chan->band == freq->band) {
			return 1;
		}
	}
	//      fprintf(stderr, "fails freq->freq + range %d bands %d,%d \n", freq->freq + range, chan->band, freq->band);
	return 0;
}

static int check_ranges(char *name, struct wifi_channels *list, struct wifi_channels *chan, int *ranges, int mhz)
{
	int i = 0;
	int range;
	while ((range = ranges[i++]) != -1) {
		//              fprintf(stderr, "[%s] %d range check at %d\n", name, chan->freq, range);
		if (!isinlist(list, chan, range, mhz)) {
			//                      fprintf(stderr, "[%s] %d range check failed at %d\n", name, chan->freq, range);
			return 0;
		}
	}
	//      fprintf(stderr, "[%s] %d success\n", name, chan->freq);
	return 1;
}

#define VHT160RANGE(offset)                                                                                                \
	(int[])                                                                                                            \
	{                                                                                                                  \
		offset + 70, offset + 50, offset - 50, offset - 70, offset + 30, offset + 10, offset - 10, offset - 30, -1 \
	}
#define VHT80RANGE(offset)                                             \
	(int[])                                                        \
	{                                                              \
		offset + 30, offset + 10, offset - 10, offset - 30, -1 \
	}

#define VHTRANGE(width, offset) width == 160 ? VHT160RANGE(offset) : VHT80RANGE(offset)

/* check all channel combinations and sort out incompatible configurations */
static void check_validchannels(struct wifi_channels *list, int bw, int nooverlap)
{
	int i = 0;
	int lastwasupper = 0;
	while (1) {
		struct wifi_channels *chan = &list[i++];
		if (chan->freq == -1)
			break;
		chan->luu = 0;
		chan->ulu = 0;
		chan->uul = 0;
		chan->uuu = 0;
		chan->ull = 0;
		chan->lul = 0;
		chan->llu = 0;
		chan->lll = 0;

		if (bw == 40) {
			if (check_ranges("LOWER", list, chan, (int[]){ -20, -1 }, 40)) {
				chan->luu = 1;
			}
			if (check_ranges("UPPER", list, chan, (int[]){ 20, -1 }, 40)) {
				chan->ull = 1;
			}

			if (nooverlap) {
				if (chan->luu && chan->ull) {
					if (lastwasupper) {
						chan->ull = 0;
						lastwasupper = 0;
					} else {
						chan->luu = 0;
						lastwasupper = 1;
					}
				} else if (chan->luu) {
					lastwasupper = 0;
				} else if (chan->ull) {
					lastwasupper = 1;
				}
			}
		}
		/* first entry in range is the dfs channel which must be considered to ensure its a valid channel */
		if (bw == 80) {
			if (check_ranges("LL", list, chan, VHTRANGE(80, -30), 80)) {
				chan->lul = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("UU", list, chan, VHTRANGE(80, 30), 80)) {
				chan->ulu = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("UL", list, chan, VHTRANGE(80, -10), 80)) {
				chan->luu = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("LU", list, chan, VHTRANGE(80, 10), 80)) {
				chan->ull = 1;
				if (nooverlap)
					goto next;
			}
		}
		if (bw == 160) {
			if (check_ranges("LLL", list, chan, VHTRANGE(160, -70), 160)) {
				chan->lll = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("UUU", list, chan, VHTRANGE(160, 70), 160)) {
				chan->uuu = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("LLU", list, chan, VHTRANGE(160, -50), 160)) {
				chan->llu = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("UUL", list, chan, VHTRANGE(160, 50), 160)) {
				chan->uul = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("LUL", list, chan, VHTRANGE(160, -30), 160)) {
				chan->lul = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("ULU", list, chan, VHTRANGE(160, 30), 160)) {
				chan->ulu = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("LUU", list, chan, VHTRANGE(160, -10), 160)) {
				chan->luu = 1;
				if (nooverlap)
					goto next;
			}
			if (check_ranges("ULL", list, chan, VHTRANGE(160, 10), 160)) {
				chan->ull = 1;
				if (nooverlap)
					goto next;
			}
		}
next:;
	}
}

static struct wifi_channels ghz60channels[] = {
	{ .channel = 1, .freq = 58320, .max_eirp = 40, .hw_eirp = 40 },
	{ .channel = 2, .freq = 60480, .max_eirp = 40, .hw_eirp = 40 },
	{ .channel = 3, .freq = 62640, .max_eirp = 40, .hw_eirp = 40 },
	//      {.channel = 4,.freq = 64800,.max_eirp = 40,.hw_eirp = 40},
	{ .channel = -1, .freq = -1, .max_eirp = -1, .hw_eirp = -1 },
};

struct wifi_channels *mac80211_get_channels(struct unl *local_unl, const char *interface, const char *country,
					    int max_bandwidth_khz, unsigned char checkband, int nocache)
{
	struct nlattr *tb[NL80211_FREQUENCY_ATTR_MAX + 1];
	struct nlattr *tb_band[NL80211_BAND_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band, *freqlist, *freq;
	struct ieee80211_regdomain *rd;
	struct ieee80211_freq_range regfreq;
	struct ieee80211_power_rule regpower;
	struct wifi_channels *list = NULL;
	int rem, rem2, freq_mhz, phy, startfreq, stopfreq, range, regmaxbw, run;
	int regfound = 0;
	int chancount = 0;
	int count = 0;
	char sc[32];
	int skip = 1;
	int rrdcount = 0;
	int super = 0;
	bool width_40 = false;
	bool width_160 = false;
	bool width_80 = false;
	int nooverlap = 1;
	if (nvram_nmatch("1", "%s_overlap", interface))
		nooverlap = 0;

	if (has_ad(interface)) {
		list = (struct wifi_channels *)calloc(sizeof(ghz60channels), 1);
		memcpy(list, ghz60channels, sizeof(ghz60channels));
		return list;
	}
	if (!nocache) {
		list = getcache(interface, country);
		if (list) {
			return list;
		}
	}
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		return NULL;
	}
#ifdef HAVE_SUPERCHANNEL
	sprintf(sc, "%s_regulatory", interface);
	if (issuperchannel() && nvram_default_geti(sc, 1) == 0) {
		super = 1;
		skip = 0;
	}
#endif
	rd = mac80211_get_regdomain(country);
	// for now just leave
	if (rd == NULL) {
		return NULL;
	}

	msg = unl_genl_msg(local_unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(local_unl, msg, &msg) < 0) {
		return NULL;
	}

	bands = unl_find_attr(local_unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands) {
		goto out;
	}
	int lastband = 0;
	int firstchan = 0;
	for (run = 0; run < 2; run++) {
		int bandcounter = 0;
		if (run == 1) {
			list = (struct wifi_channels *)calloc(sizeof(struct wifi_channels) * (chancount + 1), 1);
		}
		nla_for_each_nested(band, bands, rem) {
			nla_parse(tb_band, NL80211_BAND_ATTR_MAX, nla_data(band), nla_len(band), NULL);
			if (tb_band[NL80211_BAND_ATTR_HT_CAPA]) {
				__u16 cap = nla_get_u16(tb_band[NL80211_BAND_ATTR_HT_CAPA]);
				if (cap & BIT(1))
					width_40 = true;
			}

			if (tb_band[NL80211_BAND_ATTR_VHT_CAPA]) {
				__u32 capa;
				width_80 = true;
				capa = nla_get_u32(tb_band[NL80211_BAND_ATTR_VHT_CAPA]);
				switch ((capa >> 2) & 3) {
				case 2:
					/* width_80p80 = true; */
					/* fall through */
				case 1:
					width_160 = true;
					break;
				}
			}

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
				if (max_bandwidth_khz == 40 || max_bandwidth_khz == 80 || max_bandwidth_khz == 160 ||
				    max_bandwidth_khz == 20)
					range = 10;
				else
					// for 10/5mhz this should be fine
					range = max_bandwidth_khz / 2;
				freq_mhz = (int)nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
				int eirp = 0;
				if (tb[NL80211_FREQUENCY_ATTR_MAX_TX_POWER] && !tb[NL80211_FREQUENCY_ATTR_DISABLED])
					eirp = nla_get_u32(tb[NL80211_FREQUENCY_ATTR_MAX_TX_POWER]);
				if (skip == 0)
					rrdcount = 1;
				else
					rrdcount = rd->n_reg_rules;

				//for (rrc = 0; rrc < rrdcount; rrc++)
				{
					int cc;
					int isband = 0;
					if (freq_mhz >= 4800)
						isband = 1;
					if (freq_mhz >= 5500)
						isband = 2;
#ifdef HAVE_MVEBU
					if (is_wrt3200() && phy == 0 && isband == 0)
						continue;
#endif
					startfreq = 0;
					stopfreq = 0;
					int startlowbound = 0;
					int starthighbound = 0;
					int stoplowbound = 0;
					int stophighbound = 0;
					switch (isband) {
					case 0:
						startlowbound = 2200000;
						starthighbound = 4800000;
						stophighbound = 2700000;
						stoplowbound = 0;
						break;
					case 1:
						startlowbound = 4800000;
						starthighbound = 5350000;
						stophighbound = 5350000;
						stoplowbound = 2700000;
						break;
					case 2:
						startlowbound = 5350000;
						starthighbound = 5500000;
						stophighbound = 6200000;
						stoplowbound = 5500000;
						break;
					}
					int flags = 0;
					regmaxbw = 0;
					int band = 0;
					if (super) {
						startfreq = 2200;
						stopfreq = 6200;
						regpower.max_eirp = 40 * 100;
						regmaxbw = 160;
						flags = 0;
					} else {
						int ccidx = 0;
						for (cc = 0; cc < rrdcount; cc++) {
							regfreq = rd->reg_rules[cc].freq_range;
							if (cc &&
							    regfreq.start_freq_khz == rd->reg_rules[cc - 1].freq_range.end_freq_khz)
								ccidx--;
							if (!startfreq && regfreq.start_freq_khz > startlowbound &&
							    regfreq.start_freq_khz < starthighbound) {
								startfreq = regfreq.start_freq_khz / 1000;
							}
							if (regfreq.end_freq_khz <= stophighbound &&
							    regfreq.end_freq_khz > stoplowbound) {
								stopfreq = regfreq.end_freq_khz / 1000;
							}
							if (freq_mhz > regfreq.start_freq_khz / 1000 &&
							    freq_mhz < regfreq.end_freq_khz / 1000) {
								band = ccidx + bandcounter;
								flags = rd->reg_rules[cc].flags;
								regpower = rd->reg_rules[cc].power_rule;
								regmaxbw = regfreq.max_bandwidth_khz / 1000;
								if (ccidx != lastband) {
									firstchan = 0;
								}
								if (!firstchan)
									firstchan = freq_mhz;
								lastband = ccidx;
								int offset = freq_mhz - firstchan;
								if ((offset % max_bandwidth_khz) >= (max_bandwidth_khz - 20))
									bandcounter += 10;
								//                                                              fprintf(stderr, "[%d:%d}: (band %d) %d %d\n", regfreq.start_freq_khz / 1000, freq_mhz, cc, offset%max_bandwidth_khz ,band);
							}
							ccidx++;
						}
					}

					//                                      fprintf(stderr, "pre: run %d, freq %d, startfreq %d stopfreq %d, regmaxbw %d maxbw %d\n", run, freq_mhz, startfreq, stopfreq, regmaxbw, max_bandwidth_khz);

					//                                      regfreq = rd->reg_rules[rrc].freq_range;
					//                                      startfreq = regfreq.start_freq_khz / 1000;
					//                                      stopfreq = regfreq.end_freq_khz / 1000;
					//                                      if (!skip)
					//                                              regmaxbw = 40;
					if (!skip || ((freq_mhz - range) >= startfreq && (freq_mhz + range) <= stopfreq)) {
						if (run == 1) {
#if defined(HAVE_BUFFALO_SA) && defined(HAVE_ATH9K)
							char *sa_region = getUEnv("region");
							if (sa_region != NULL &&
							    (!strcmp(sa_region, "AP") || !strcmp(sa_region, "US")) &&
							    ieee80211_mhz2ieee(freq_mhz) > 11 &&
							    ieee80211_mhz2ieee(freq_mhz) < 14 &&
							    nvram_default_match("region", "SA", ""))
								continue;
#endif
							if (checkband == 2 && freq_mhz > 4000)
								continue;
							if (checkband == 5 && freq_mhz < 4000)
								continue;
							if (max_bandwidth_khz > regmaxbw)
								continue;
							list[count].channel = ieee80211_mhz2ieee(freq_mhz);
							list[count].freq = freq_mhz;
							if (nooverlap && max_bandwidth_khz > 40)
								list[count].band = band;
							// todo: wenn wir das ueberhaupt noch verwenden
							list[count].noise = 0;
							list[count].max_eirp = regpower.max_eirp / 100;
							list[count].hw_eirp = eirp / 100;
							if (flags & RRF_NO_OFDM)
								list[count].no_ofdm = 1;
							if (flags & RRF_NO_CCK)
								list[count].no_cck = 1;
							if (flags & RRF_NO_INDOOR)
								list[count].no_indoor = 1;
							if (flags & RRF_NO_OUTDOOR)
								list[count].no_outdoor = 1;
							if (flags & RRF_DFS)
								list[count].dfs = 1;
							if (flags & RRF_PTP_ONLY)
								list[count].ptp_only = 1;
							if (flags & RRF_PTMP_ONLY)
								list[count].ptmp_only = 1;
							if (flags & RRF_PASSIVE_SCAN)
								list[count].passive_scan = 1;
							if (flags & RRF_NO_IBSS)
								list[count].no_ibss = 1;
							list[count].lll = 0;
							list[count].llu = 0;
							list[count].lul = 0;
							list[count].luu = 0;
							list[count].ull = 0;
							list[count].ulu = 0;
							list[count].uul = 0;
							list[count].uuu = 0;
							//                                                      fprintf(stderr,"freq %d, htrange %d, startfreq %d, stopfreq %d\n", freq_mhz, range, startfreq, stopfreq);
							if (((freq_mhz - range) - (max_bandwidth_khz / 2)) >=
							    startfreq) { // 5510 -         5470
								list[count].lll = 1;
								list[count].llu = 1;
								list[count].lul = 1;
								list[count].luu = 1;
							}
							if (((freq_mhz + range) + (max_bandwidth_khz / 2)) <= stopfreq) {
								list[count].ull = 1;
								list[count].ulu = 1;
								list[count].uul = 1;
								list[count].uuu = 1;
							}
							list[count].ht40 = true;
							list[count].vht80 = true;
							list[count].vht160 = true;
							//                                                      fprintf(stderr, "%d %d %d\n", freq_mhz, band, max_bandwidth_khz);
							if (regmaxbw < 40 && max_bandwidth_khz == 40) {
								list[count].luu = 0;
								list[count].ull = 0;
								list[count].ht40 = false;
							}
							if (regmaxbw < 80 && max_bandwidth_khz == 80) {
								list[count].ull = 0;
								list[count].uul = 0;
								list[count].lul = 0;
								list[count].ulu = 0;
								list[count].vht80 = false;
							}
							if (regmaxbw < 160 && max_bandwidth_khz == 160) {
								list[count].luu = 0;
								list[count].ull = 0;
								list[count].ulu = 0;
								list[count].uul = 0;
								list[count].uuu = 0;
								list[count].lul = 0;
								list[count].llu = 0;
								list[count].lll = 0;
								list[count].vht160 = false;
							}
							if (regmaxbw > 20 && regmaxbw >= max_bandwidth_khz) {
								//      fprintf(stderr, "freq %d, htrange %d, startfreq %d stopfreq %d, regmaxbw %d hw_eirp %d max_eirp %d ht40plus %d ht40minus %d\n", freq_mhz, max_bandwidth_khz,
								//              startfreq, stopfreq, regmaxbw, eirp, regpower.max_eirp, list[count].ht40plus, list[count].ht40minus);
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
	check_validchannels(list, max_bandwidth_khz, nooverlap);
	if (!nocache)
		addcache(interface, country, list);
	return list;
out:
nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

struct wifi_channels *mac80211_get_channels_simple(const char *interface, const char *country, int max_bandwidth_khz,
						   unsigned char checkband)
{
	lock();
	struct wifi_channels *chan = mac80211_get_channels(&unl, interface, country, max_bandwidth_khz, checkband, 1);
	unlock();
	return chan;
}

int can_ht40(const char *interface)
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
	lock();
	chan = mac80211_get_channels(&unl, interface, getIsoName(country), 40, 0xff, 1);
	unlock();
	if (chan) {
		while (chan[i].freq != -1) {
			if (chan[i].luu || chan[i].ull) {
				free(chan);
				return 1;
			}
			i++;
		}
	}
	free(chan);
	return 0;
}

int can_vht80(const char *interface)
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
	lock();
	chan = mac80211_get_channels(&unl, interface, getIsoName(country), 80, 0xff, 1);
	unlock();
	if (chan) {
		while (chan[i].freq != -1) {
			if (chan[i].lul || chan[i].ulu) {
				free(chan);
				return 1;
			}
			i++;
		}
	}
	free(chan);
	return 0;
}

int can_vht160(const char *interface)
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
	lock();
	chan = mac80211_get_channels(&unl, interface, getIsoName(country), 160, 0xff, 1);
	unlock();
	if (chan) {
		while (chan[i].freq != -1) {
			if (chan[i].uuu || chan[i].lll) {
				free(chan);
				return 1;
			}
			i++;
		}
	}
	free(chan);
	return 0;
}

int mac80211_check_valid_frequency(const char *interface, char *country, int freq)
{
	struct wifi_channels *chan;
	int found = 0;
	int i = 0;
	lock();
	chan = mac80211_get_channels(&unl, interface, country, 40, 0xff, 0);
	unlock();
	if (chan) {
		while (chan[i].freq != -1) {
			if (freq == chan[i].freq) {
				return freq;
			}
			i++;
		}
	}
	return (0);
}

static struct wifi_client_info *add_to_wifi_clients(struct wifi_client_info *list_root)
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
	return get_max_mcs_index(mcs);
}

static int get_vht_mcs(__u32 capa, const __u8 *mcs)
{
	__u16 tmp;
	int i;
	int latest = -1;
	tmp = mcs[4] | (mcs[5] << 8);
	for (i = 1; i <= 8; i++) {
		switch ((tmp >> ((i - 1) * 2)) & 3) {
		case 0:
			latest = ((i - 1) * 10) + 7;
			break;
		case 1:
			latest = ((i - 1) * 10) + 8;
			break;
		case 2:
			latest = ((i - 1) * 10) + 9;
			break;
		case 3:
			break;
		}
	}
	return latest;
}

int mac80211_get_maxrate(char *interface)
{
	mac80211_init();
	struct nlattr *tb[NL80211_BITRATE_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band, *ratelist, *rate;
	int rem, rem2;
	int phy;
	int maxrate = 0;
	static struct nla_policy rate_policy[NL80211_BITRATE_ATTR_MAX + 1] = {
		[NL80211_BITRATE_ATTR_RATE] = { .type = NLA_U32 },
		[NL80211_BITRATE_ATTR_2GHZ_SHORTPREAMBLE] = { .type = NLA_FLAG },
	};
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return 0;
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
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
	unlock();
	return maxrate;
out:
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

int mac80211_get_maxmcs(char *interface)
{
	mac80211_init();
	struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band;
	int rem;
	int phy;
	int maxmcs = 0;
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return 0;
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		nla_parse(tb, NL80211_BAND_ATTR_MAX, nla_data(band), nla_len(band), NULL);
		if (tb[NL80211_BAND_ATTR_HT_MCS_SET] && nla_len(tb[NL80211_BAND_ATTR_HT_MCS_SET]) == 16)
			maxmcs = get_ht_mcs(nla_data(tb[NL80211_BAND_ATTR_HT_MCS_SET]));
	}
	nlmsg_free(msg);
	unlock();
	return maxmcs;
out:
	unlock();
	return 0;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

int mac80211_get_maxvhtmcs(char *interface)
{
	mac80211_init();
	struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band;
	int rem;
	int phy;
	int maxmcs = -1;
	lock();
	phy = mac80211_get_phyidx_by_vifname(interface);
	if (phy == -1) {
		unlock();
		return 0;
	}
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;
	nla_for_each_nested(band, bands, rem) {
		nla_parse(tb, NL80211_BAND_ATTR_MAX, nla_data(band), nla_len(band), NULL);
		if (tb[NL80211_BAND_ATTR_VHT_CAPA] && tb[NL80211_BAND_ATTR_VHT_MCS_SET])
			maxmcs = get_vht_mcs(nla_get_u32(tb[NL80211_BAND_ATTR_VHT_CAPA]),
					     nla_data(tb[NL80211_BAND_ATTR_VHT_MCS_SET]));
	}
	nlmsg_free(msg);
	unlock();
	return maxmcs;
out:
	unlock();
	return 0;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

static int mac80211_get_antennas(const char *prefix, int which, int direction);

void mac80211_set_antennas(const char *prefix, uint32_t tx_ant, uint32_t rx_ant)
{
	int maxrxchain = mac80211_get_antennas(prefix, 0, 1);
	int maxtxchain = mac80211_get_antennas(prefix, 0, 0);
	if (has_vht160_2by2(prefix) &&
	    (nvram_nmatch("160", "%s_channelbw", prefix) || nvram_nmatch("8080", "%s_channelbw", prefix)) && tx_ant == 0xf &&
	    rx_ant == 0xf) {
		rx_ant = 0x3;
		tx_ant = 0x3;
	}
	if (maxrxchain > 15 && (maxrxchain & 0xf) == 0)
		rx_ant <<= 4;
	if (maxtxchain > 15 && (maxtxchain & 0xf) == 0)
		tx_ant <<= 4;

	int phy = get_ath9k_phy_ifname(prefix);
	mac80211_init();
	struct nl_msg *msg;
	if (tx_ant == 0 || rx_ant == 0)
		return;
	lock();
	msg = unl_genl_msg(&unl, NL80211_CMD_SET_WIPHY, false);
	if (!msg) {
		unlock();
		return;
	}
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (is_ap8x(prefix) && tx_ant == 5)
		tx_ant = 3;
	if (is_ap8x(prefix) && tx_ant == 4)
		tx_ant = 2;
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_TX, tx_ant);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY_ANTENNA_RX, rx_ant);
	unl_genl_request(&unl, msg, NULL, NULL);
	unlock();
	return;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return;
}

static int mac80211_has_iftype(const char *prefix, enum nl80211_iftype iftype)
{
	return (long)mac80211_has_worker(prefix, &iftype_worker, &iftype);
}

int has_acktiming(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = (long)mac80211_has_worker(prefix, &acktiming_worker, NULL);
	EXITVALUECACHE();
	return ret;
}

int has_ibss(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_iftype(prefix, NL80211_IFTYPE_ADHOC);
	EXITVALUECACHE();
	return ret;
}

int has_no_apmode(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = !mac80211_has_iftype(prefix, NL80211_IFTYPE_AP);
	EXITVALUECACHE();
	return ret;
}

int has_wdsap(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_iftype(prefix, NL80211_IFTYPE_AP_VLAN);
	EXITVALUECACHE();
	return ret;
}

#ifdef HAVE_IPQ6018
int has_apup(const char *prefix)
{
	return has_wdsap(prefix);
}
#else
int has_apup(const char *prefix)
{
	return 0;
}
#endif

#ifdef HAVE_MAC80211_MESH
int has_mesh(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_iftype(prefix, NL80211_IFTYPE_MESH_POINT);
	EXITVALUECACHE();
	return ret;
}

int has_tdma(const char *prefix)
{
	if (!is_mac80211(prefix))
		return 0;
	INITVALUECACHE();
	ret = mac80211_has_iftype(prefix, NL80211_IFTYPE_TDMA);
	EXITVALUECACHE();
	return ret;
}
#endif

static int mac80211_get_antennas(const char *prefix, int which, int direction)
{
	int phy = get_ath9k_phy_ifname(prefix);
	mac80211_init();
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	int ret = 0;
	lock();
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	if (!msg) {
		unlock();
		return 0;
	}
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return 0;
	}
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
	unlock();
	return ret;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return 0;
}

static int match_cipher(const char *prefix, __u32 cipher)
{
	__u32 num;
	if (!is_mac80211(prefix))
		return 0;
	__u32 *ciphers = mac80211_get_ciphers(prefix, &num);
	if (!ciphers)
		return 0;
	int i;
	for (i = 0; i < num; i++) {
		if (ciphers[i] == cipher) {
			free(ciphers);
			return 1;
		}
	}
	free(ciphers);
	return 0;
}

int has_cmac(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac06);
	EXITVALUECACHE();
}

int has_gcmp_128(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac08);
	EXITVALUECACHE();
	return ret;
}

int has_gcmp(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac08);
	EXITVALUECACHE();
	return ret;
}

int has_gcmp_256(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac09);
	EXITVALUECACHE();
	return ret;
}

int has_ccmp_256(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac0a);
	EXITVALUECACHE();
	return ret;
}

int has_gmac_128(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac0b);
	EXITVALUECACHE();
	return ret;
}

int has_gmac_256(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac0c);
	EXITVALUECACHE();
	return ret;
}

int has_cmac_256(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x000fac0d);
	EXITVALUECACHE();
	return ret;
}

int has_sms4(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x00147201);
	EXITVALUECACHE();
	return ret;
}

int has_ckip(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x00409600);
	EXITVALUECACHE();
	return ret;
}

int has_ckip_cmic(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x00409601);
	EXITVALUECACHE();
	return ret;
}

int has_cmic(const char *prefix)
{
	INITVALUECACHE();
	ret = match_cipher(prefix, 0x00409602);
	EXITVALUECACHE();
	return ret;
}

int mac80211_get_avail_tx_antenna(const char *prefix)
{
	int ret = mac80211_get_antennas(prefix, 0, 0);
	if (ret > 15 && (ret & 0x0f) == 0)
		ret >>= 4;
	if (is_ap8x(prefix) && ret == 3)
		ret = 5;
	return (ret);
}

int mac80211_get_avail_rx_antenna(const char *prefix)
{
	int ret = mac80211_get_antennas(prefix, 0, 1);
	if (ret > 15 && (ret & 0x0f) == 0)
		ret >>= 4;
	return ret;
}

int mac80211_get_configured_tx_antenna(const char *prefix)
{
	int ret = mac80211_get_antennas(prefix, 1, 0);
	int avail = mac80211_get_antennas(prefix, 0, 0);

	if (ret > 15 && (ret & 0x0f) == 0)
		ret >>= 4;
	if (avail > 15 && (avail & 0x0f) == 0)
		avail >>= 4;

	if (is_ap8x(prefix) && avail == 3 && ret == 3)
		ret = 5;
	if (is_ap8x(prefix) && avail == 3 && ret == 2)
		ret = 4;
	return (ret);
}

int mac80211_get_configured_rx_antenna(const char *prefix)
{
	int ret = mac80211_get_antennas(prefix, 1, 1);
	if (ret > 15 && (ret & 0x0f) == 0)
		ret >>= 4;
	return ret;
}

struct wifi_interface *mac80211_get_interface(char *dev)
{
	mac80211_init();
	struct nlattr *tb_msg[NL80211_ATTR_MAX + 1];
	const char *indent = "";
	struct nl_msg *msg;
	struct genlmsghdr *gnlh;
	int ret = 0;
	struct wifi_interface *interface = NULL;
	lock();
	msg = unl_genl_msg(&unl, NL80211_CMD_GET_INTERFACE, false);
	if (!msg) {
		unlock();
		return NULL;
	}
	if (has_ad(dev))
		dev = "giwifi0";
	int devidx = if_nametoindex(dev);
	NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, devidx);
	if (unl_genl_request_single(&unl, msg, &msg) < 0) {
		unlock();
		return NULL;
	}

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
#if 0
	if (tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]) {
		int txp = nla_get_u32(tb_msg[NL80211_ATTR_WIPHY_TX_POWER_LEVEL]);
		interface->txpower = txp;
	}
#endif
	nlmsg_free(msg);
	unlock();
	return interface;
nla_put_failure:
	nlmsg_free(msg);
	unlock();
	return interface;
}

#ifdef TEST
void main(int argc, char *argv[])
{
	mac80211_get_channels("wlan0", "US", 20, 255);
	mac80211_get_channels("wlan1", "DE", 20, 255);
	mac80211_get_channels("wlan1", "DE", 20, 255);
	mac80211_get_channels("wlan0", "US", 20, 255);
	fprintf(stderr, "phy0 %d %d %d %d\n", mac80211_get_avail_tx_antenna(0), mac80211_get_avail_rx_antenna(0),
		mac80211_get_configured_tx_antenna(0), mac80211_get_configured_rx_antenna(0));
	fprintf(stderr, "phy1 %d %d %d %d\n", mac80211_get_avail_tx_antenna(1), mac80211_get_avail_rx_antenna(1),
		mac80211_get_configured_tx_antenna(1), mac80211_get_configured_rx_antenna(1));
}
#endif
