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
#ifndef __UCLIBC__
//#define __DEFINED_float_t
#endif
#include <string.h>
#include <unistd.h>
#include <typedefs.h>
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
	char *l = (char *)list;
	mac80211_info = mac80211_assoclist(ifname);
	l += 4;
	count[0] = 0;
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		memcpy(l, wc->etheraddr, 6);
		l += 6;
		count[0]++;
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return count[0];
}

// dummy TBD
int getWifiInfo_ath9k(char *ifname, unsigned char *mac, int field)
{
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		if (!memcmp(wc->etheraddr, mac, 6)) {
			free_wifi_clients(mac80211_info->wci);
			free(mac80211_info);
			switch (field) {
			case INFO_RSSI:
				return wc->signal;
			case INFO_NOISE:
				return wc->noise;
			case INFO_UPTIME:
				return wc->uptime;
			case INFO_RXRATE:
				return wc->rxrate;
			case INFO_TXRATE:
				return wc->txrate;
			default:
				return 0;
			}
		}
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return 0;
}
