/*
 * wl_ath9k.c 
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

#include <string.h>
#include <unistd.h>

#include <typedefs.h>
#include <bcmutils.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>
#include <math.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

// netlink:
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include <linux/nl80211.h>

// dummy TBD 
int getassoclist_ath9k(char *ifname, unsigned char *list)
{
	unsigned int *count = (unsigned int *)list;
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	unsigned char *l = (unsigned char *)list;
	mac80211_info = mac80211_assoclist(ifname);
	l += 4;
	count[0] = 0;
	for (wc = mac80211_info->wci; wc; wc = wc->next) {

		ether_atoe(wc->mac, l);
		l += 6;
		count[0]++;
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return count[0];
}

// dummy TBD 
int getRssi_ath9k(char *ifname, unsigned char *mac)
{
	unsigned char rmac[32];
	ether_etoa(mac, rmac);

	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		if (!strcmp(wc->mac, rmac)) {
			free_wifi_clients(mac80211_info->wci);
			free(mac80211_info);
			return wc->signal;
		}
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return 0;
}

// dummy TBD 
int getUptime_ath9k(char *ifname, unsigned char *mac)
{
	unsigned char rmac[32];
	ether_etoa(mac, rmac);
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		if (!strcmp(wc->mac, rmac)) {
			free_wifi_clients(mac80211_info->wci);
			free(mac80211_info);
			return wc->uptime;
		}
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return 0;
}

// dummy TBD 
int getNoise_ath9k(char *ifname, unsigned char *mac)
{
	unsigned char rmac[32];
	ether_etoa(mac, rmac);
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		if (!strcmp(wc->mac, rmac)) {
			free_wifi_clients(mac80211_info->wci);
			free(mac80211_info);
			return wc->noise;
		}
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return 0;
}

// dummy TBD  erstmal alles zum spielen
/*
static struct wifi_channels *list_channelsext_ath9k(const char *ifname, char country[2],int max_bandwidth_khz, unsigned char band)
{
	// get_ath9k_phy_idx
	int i;
	fprintf(stderr, "list fake channels for %s\n", ifname);
	int count=0;

	struct wifi_channels *list =
	    (struct wifi_channels *)safe_malloc(sizeof(struct wifi_channels) *
					   (13+1+166-36+1));
	(void)memset(list, 0, (sizeof(struct wifi_channels)*(3+1+166-36+1)));
	for (i=0;i<13;i++) {
		list[count].channel = i+1;
		list[count].freq = 5*(i+1)+2407;
		list[count].noise = -95;
		if (i>3)
			list[count].ht40minus = 1;
		if (i<10)
			list[count].ht40plus = 1;
		list[count].outdoor = 1;
		list[count].dfs = 0;
		if (!strcmp(country,"DE"))
			list[count].max_eirp = 20;
		else
			list[count].max_eirp = 27;
		list[count].no_ofdm = 0;
		count++;
		}
	list[count].channel = 14;
	list[count].freq = 2484;
	list[count].noise = -95;
	list[count].no_ofdm = 1;
	list[count].outdoor = 1;
	list[count].no_ofdm = 1;
	count++;
		for (i=36;i<145;i=i+4) {
			list[count].channel = i;
			list[count].freq = i*5+5000;
			if (i!=36)
				list[count].ht40minus = 1;
			list[count].ht40plus = 1;
			list[count].max_eirp = 27;
			count++;
			}
		for (i=149;i<201;i=i+4) {
			list[count].channel = i;
			list[count].freq = i*5+5000;
			if (i!=197)
				list[count].ht40plus = 1;
			list[count].ht40minus = 1;
			list[count].max_eirp = 27;
			count++;
			}

	list[count++].freq = -1;
	return list;
}
*/
/*
struct wifi_channels *list_channels_ath9k(char *devnr, char country[2],int max_bandwidth_khz, unsigned char band)
{
	return list_channelsext_ath9k(devnr, country,max_bandwidth_khz,band);
}
*/
