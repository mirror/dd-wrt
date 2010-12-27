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

#include "unl.h"
#include "mac80211regulatory.h"
#include "linux/nl80211.h"

#include "wlutils.h"

// some defenitions from hostapd
typedef uint16_t u16;
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

static struct unl unl;

static void print_wifi_clients(struct wifi_client_info *wci);
void free_wifi_clients(struct wifi_client_info *wci);
static struct wifi_client_info *add_to_wifi_clients(struct wifi_client_info *list_root);
static int mac80211_cb_survey(struct nl_msg *msg, void *data);

static void __attribute__((constructor)) mac80211_init(void) {
	unl_genl_init(&unl, "nl80211");
} 



void mac_addr_n2a(char *mac_addr, unsigned char *arg)
{
	int i, l;
#define ETH_ALEN        6

	l = 0;
	for (i = 0; i < ETH_ALEN ; i++) {
		if (i == 0) {
			sprintf(mac_addr+l, "%02x", arg[i]);
			l += 2;
		} else {
			sprintf(mac_addr+l, ":%02x", arg[i]);
			l += 3;
		}
	}
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

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_SURVEY_INFO])
		return -1;

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX,
			     tb[NL80211_ATTR_SURVEY_INFO],
			     survey_policy))
		return -1;

	if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY])
		return -1;

	return 0;
}


static int mac80211_cb_survey(struct nl_msg *msg, void *data)
{
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	int freq;
	struct mac80211_info *mac80211_info = data;

	if (parse_survey(msg, sinfo))
		goto out;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	if (sinfo[NL80211_SURVEY_INFO_IN_USE])
		{

		if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME] &&
			sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]) {

			mac80211_info->channel_active_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
			mac80211_info->channel_busy_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);

		}

		if (sinfo[NL80211_SURVEY_INFO_NOISE]) {
			mac80211_info->noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
		}
	}
	return 0;

out:
	nlmsg_free(msg);
	return NL_SKIP;
}

static void getNoise_mac80211_internal(char *interface,struct mac80211_info *mac80211_info) {
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
	return(-199);
}

static int mac80211_cb_stations(struct nl_msg *msg,void *data) {
	// struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
	struct nlattr *rinfo[NL80211_RATE_INFO_MAX + 1];
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	char mac_addr[20], dev[20];
	struct mac80211_info *mac80211_info = data;
	mac80211_info->wci = add_to_wifi_clients(mac80211_info->wci);
	// struct nlattr *sinfo[NL80211_STA_INFO_MAX + 1];
    static struct nla_policy stats_policy[NL80211_STA_INFO_MAX + 1] = {
        [NL80211_STA_INFO_INACTIVE_TIME] = { .type = NLA_U32 },
        [NL80211_STA_INFO_RX_BYTES] = { .type = NLA_U32 },
        [NL80211_STA_INFO_TX_BYTES] = { .type = NLA_U32 },
        [NL80211_STA_INFO_RX_PACKETS] = { .type = NLA_U32 },
        [NL80211_STA_INFO_TX_PACKETS] = { .type = NLA_U32 },
        [NL80211_STA_INFO_SIGNAL] = { .type = NLA_U8 },
        [NL80211_STA_INFO_TX_BITRATE] = { .type = NLA_NESTED },
        [NL80211_STA_INFO_LLID] = { .type = NLA_U16 },
        [NL80211_STA_INFO_PLID] = { .type = NLA_U16 },
        [NL80211_STA_INFO_PLINK_STATE] = { .type = NLA_U8 },
    };

    static struct nla_policy rate_policy[NL80211_RATE_INFO_MAX + 1] = {
        [NL80211_RATE_INFO_BITRATE] = { .type = NLA_U16 },
        [NL80211_RATE_INFO_MCS] = { .type = NLA_U8 },
        [NL80211_RATE_INFO_40_MHZ_WIDTH] = { .type = NLA_FLAG },
        [NL80211_RATE_INFO_SHORT_GI] = { .type = NLA_FLAG },
    };
	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[NL80211_ATTR_STA_INFO]) {
		fprintf(stderr, "sta stats missing!\n");
		return NL_SKIP;
	}
	if (nla_parse_nested(sinfo, NL80211_STA_INFO_MAX,
			     tb[NL80211_ATTR_STA_INFO],
			     stats_policy)) {
		fprintf(stderr, "failed to parse nested attributes!\n");
		return NL_SKIP;
	}
	mac_addr_n2a(mac_addr, nla_data(tb[NL80211_ATTR_MAC]));
	if_indextoname(nla_get_u32(tb[NL80211_ATTR_IFINDEX]), dev);
	printf("Station %s (on %s)", mac_addr, dev);
	strcpy(mac80211_info->wci->mac, mac_addr);
	strcpy(mac80211_info->wci->ifname, dev);
	mac80211_info->wci->noise=mac80211_info->noise;

	if (sinfo[NL80211_STA_INFO_INACTIVE_TIME]) {
		mac80211_info->wci->inactive_time=nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]);
		printf("\n\tinactive time:\t%u ms",
			nla_get_u32(sinfo[NL80211_STA_INFO_INACTIVE_TIME]));
	}
	if (sinfo[NL80211_STA_INFO_RX_BYTES]) {
		mac80211_info->wci->rx_bytes=nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]);
		printf("\n\trx bytes:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_BYTES]));
	}
	if (sinfo[NL80211_STA_INFO_RX_PACKETS]) {
		mac80211_info->wci->rx_packets=nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]);
		printf("\n\trx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_RX_PACKETS]));
	}
	if (sinfo[NL80211_STA_INFO_TX_BYTES]) {
		mac80211_info->wci->tx_bytes=nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]);
		printf("\n\ttx bytes:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_BYTES]));
	}
	if (sinfo[NL80211_STA_INFO_TX_PACKETS]) {
		mac80211_info->wci->tx_packets=nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]);
		printf("\n\ttx packets:\t%u",
			nla_get_u32(sinfo[NL80211_STA_INFO_TX_PACKETS]));
	}
	if (sinfo[NL80211_STA_INFO_SIGNAL]) {
		mac80211_info->wci->signal=(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]);
		printf("\n\tsignal:  \t%d dBm",
			(int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]));
		}

	if (sinfo[NL80211_STA_INFO_TX_BITRATE]) {
		if (nla_parse_nested(rinfo, NL80211_RATE_INFO_MAX,
				     sinfo[NL80211_STA_INFO_TX_BITRATE], rate_policy)) {
			fprintf(stderr, "failed to parse nested rate attributes!\n");
		} else {
			printf("\n\ttx bitrate:\t");
			if (rinfo[NL80211_RATE_INFO_BITRATE]) {
				mac80211_info->wci->txrate =nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
				int rate = nla_get_u16(rinfo[NL80211_RATE_INFO_BITRATE]);
				printf("%d.%d MBit/s", rate / 10, rate % 10);
			}

			if (rinfo[NL80211_RATE_INFO_MCS]) {
				mac80211_info->wci->mcs=nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]);
				printf(" MCS %d", nla_get_u8(rinfo[NL80211_RATE_INFO_MCS]));
				}
			if (rinfo[NL80211_RATE_INFO_40_MHZ_WIDTH]) {
				mac80211_info->wci->is_40mhz=1;
				printf(" 40Mhz");
				}
			if (rinfo[NL80211_RATE_INFO_SHORT_GI]) {
				mac80211_info->wci->is_short_gi=1;
				printf(" short GI");
				}
		}
	}
return(0);
}

struct mac80211_info *mac80211_assoclist(char *interface) {
	struct nl_msg *msg;
	glob_t globbuf;
	char globstring[1024];
	int globresult;
	struct mac80211_info *mac80211_info = calloc(1, sizeof(struct mac80211_info));

	if (interface)
		sprintf(globstring, "/sys/class/ieee80211/phy*/device/net/%s*",
			interface);
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
		getNoise_mac80211_internal(ifname + 1,mac80211_info);
		msg = unl_genl_msg(&unl, NL80211_CMD_GET_STATION,true);
		NLA_PUT_U32(msg, NL80211_ATTR_IFINDEX, if_nametoindex(ifname + 1));
		unl_genl_request(&unl, msg, mac80211_cb_stations,mac80211_info);
	}
	// print_wifi_clients(mac80211_info->wci);
	// free_wifi_clients(mac80211_info->wci);
	globfree(&globbuf);


	return(mac80211_info);
nla_put_failure:
	nlmsg_free(msg);
	return(mac80211_info);
}

char *mac80211_get_caps(char *interface) {
	struct nl_msg *msg;
	struct nlattr *caps, *bands, *band;
	int rem;
	u16 cap;
	char *capstring="";
	int wdev,phy;
	wdev = if_nametoindex(interface);
	phy = unl_nl80211_wdev_to_phy(&unl, wdev);


	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return "";
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;

	nla_for_each_nested(band, bands, rem) {
		caps = nla_find(nla_data(band), nla_len(band), NL80211_BAND_ATTR_HT_CAPA);
		if (!caps) continue;
		cap = nla_get_u16(caps);
		asprintf(&capstring,"%s%s%s%s%s%s%s%s"
			,(cap & HT_CAP_INFO_LDPC_CODING_CAP ? "[LDPC]" : "")
			,(cap & HT_CAP_INFO_SHORT_GI20MHZ ? "[SHORT-GI-20]" : "")
			,(cap & HT_CAP_INFO_SHORT_GI40MHZ ? "[SHORT-GI-40]" : "")
			,(cap & HT_CAP_INFO_TX_STBC ? "[TX-STBC]" : "")
			,(((cap >> 8) & 0x3) == 1 ? "[RX-STBC1]" : "")
			,(((cap >> 8) & 0x3) == 2 ? "[RX-STBC12]" : "")
			,(((cap >> 8) & 0x3) == 3 ? "[RX-STBC123]" : "")
			,(cap & HT_CAP_INFO_DSSS_CCK40MHZ ? "[DSSS_CCK-40]" : "")
			);
	}
	printf("%s\n",capstring);
	return capstring;
out:
nla_put_failure:
	nlmsg_free(msg);
	return "";
	}

static struct nla_policy freq_policy[NL80211_FREQUENCY_ATTR_MAX + 1] = {
	[NL80211_FREQUENCY_ATTR_FREQ] = { .type = NLA_U32 },
};

int mac80211_check_band(char *interface,int checkband) {
	struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band,*freqlist,*freq;
	int rem, rem2, freq_mhz;
	int wdev,phy;
	int bandfound=0;
	wdev = if_nametoindex(interface);
	phy = unl_nl80211_wdev_to_phy(&unl, wdev);


	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return 0;
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;

	nla_for_each_nested(band, bands, rem) {
		freqlist = nla_find(nla_data(band), nla_len(band),
				    NL80211_BAND_ATTR_FREQS);
		if (!freqlist)
			continue;
		nla_for_each_nested(freq, freqlist, rem2) {
			nla_parse_nested(tb, NL80211_FREQUENCY_ATTR_MAX,
					 freq, freq_policy);
			if (!tb[NL80211_FREQUENCY_ATTR_FREQ])
				continue;

			if (tb[NL80211_FREQUENCY_ATTR_DISABLED])
				continue;

			freq_mhz = nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
			if ((int) (freq_mhz / 1000) == checkband)
				bandfound=1;
		}
	}
	return bandfound;
out:
nla_put_failure:
	nlmsg_free(msg);
	return 0;
}

struct wifi_channels *mac80211_get_channels(char *interface,char *country,int max_bandwidth_khz, unsigned char checkband) {
	struct nlattr *tb[NL80211_BAND_ATTR_MAX + 1];
	struct nl_msg *msg;
	struct nlattr *bands, *band,*freqlist,*freq;
	struct ieee80211_regdomain *rd;
	struct ieee80211_freq_range regfreq;
	struct ieee80211_power_rule regpower;
	struct wifi_channels *list = NULL;
	int rem, rem2, freq_mhz,wdev,phy,rrc,startfreq,stopfreq,range,regmaxbw,run;
	int regfound=0;
	int htrange=30;
	int chancount=0;
	int count=0;

	wdev = if_nametoindex(interface);
	phy = unl_nl80211_wdev_to_phy(&unl, wdev);

	rd=mac80211_get_regdomain(country);

	msg = unl_genl_msg(&unl, NL80211_CMD_GET_WIPHY, false);
	NLA_PUT_U32(msg, NL80211_ATTR_WIPHY, phy);
	if (unl_genl_request_single(&unl, msg, &msg) < 0)
		return NULL;
	bands = unl_find_attr(&unl, msg, NL80211_ATTR_WIPHY_BANDS);
	if (!bands)
		goto out;

	for (run=0;run <2;run++) {
		if (run == 1) {
			list = (struct wifi_channels *)malloc(sizeof(struct wifi_channels) * (chancount+1));
			(void)memset(list, 0, (sizeof(struct wifi_channels)*(chancount+1)));
		}
		nla_for_each_nested(band, bands, rem) {
			freqlist = nla_find(nla_data(band), nla_len(band),
					    NL80211_BAND_ATTR_FREQS);
			if (!freqlist)
				continue;
			nla_for_each_nested(freq, freqlist, rem2) {
				nla_parse_nested(tb, NL80211_FREQUENCY_ATTR_MAX,
						 freq, freq_policy);
				if (!tb[NL80211_FREQUENCY_ATTR_FREQ])
					continue;

				if (tb[NL80211_FREQUENCY_ATTR_DISABLED])
					continue;
				regfound=0;
				if (max_bandwidth_khz == 40)
						range=10;
				else
					// for 10/5mhz this should be fine 
					range=max_bandwidth_khz/2;
				freq_mhz = (int) nla_get_u32(tb[NL80211_FREQUENCY_ATTR_FREQ]);
				for (rrc = 0; rrc < rd->n_reg_rules; rrc++) {
					regfreq  = rd->reg_rules[rrc].freq_range;
					startfreq=(int)((float)(regfreq.start_freq_khz)/1000.0);
					stopfreq=(int)((float)(regfreq.end_freq_khz)/1000.0);
					regmaxbw=(int)((float)(regfreq.max_bandwidth_khz)/1000.0);
					if ((freq_mhz - range) >= startfreq && (freq_mhz + range) <= stopfreq) {
						if (run == 1) {
							regpower = rd->reg_rules[rrc].power_rule;
							list[count].channel = ieee80211_mhz2ieee(freq_mhz);
							list[count].freq = freq_mhz;
							// todo: wenn wir das ueberhaupt noch verwenden
							list[count].noise = 0;
							list[count].max_eirp = (int)((float)(regpower.max_eirp)/100.0);
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
							if (max_bandwidth_khz == 40) {
								if ((freq_mhz - htrange) >= startfreq ) {
									list[count].ht40minus = 1;
								}
								if ((freq_mhz + htrange) <= stopfreq) {
									list[count].ht40plus = 1;
								}
							}
							count++;
						}
						if (run == 0) chancount++;
					}
				}
			}
		}
	}
	if (count) list[count].freq=-1;
	if (rd) 
		free(rd);
	return list;
out:
nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

static struct wifi_client_info *add_to_wifi_clients(struct wifi_client_info *list_root){
		struct wifi_client_info *new = calloc(1, sizeof(struct wifi_client_info));
		if (new == NULL) {
			fprintf(stderr, "add_wifi_clients: Out of memory!\n");
			return(NULL);
			}
		else
			{
			 new->next =  list_root;
			 return(new);
			}
		}

void free_wifi_clients(struct wifi_client_info *wci) {
	while (wci) {
		struct wifi_client_info *next = wci->next;
		free(wci);
		wci = next;
		}
	}
