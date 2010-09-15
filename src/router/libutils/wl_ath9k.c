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

struct wifi_channels {
	int channel;
	int freq;
	int noise;
};

// dummy TBD 
int getassoclist_ath9k(char *ifname, unsigned char *list)
{
	// get_ath9k_phy_idx nicht vergessen
	return(0);
}

// dummy TBD 
int getRssi_ath9k(char *ifname, unsigned char *mac)
{
	// (int8_t)nla_get_u8(sinfo[NL80211_STA_INFO_SIGNAL]));
	return(-65);
}

// dummy TBD 
int getUptime_ath9k(char *ifname, unsigned char *mac)
{
    //todo, no idea yet howto get that
	return(300);
}

// dummy TBD 
int getNoise_ath9k(char *ifname, unsigned char *mac)
{
	// todo
	//  (int8_t)nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]));
	// dummy:
	return(-103);
}


// dummy TBD  erstmal alles zum spielen
static struct wifi_channels *list_channelsext_ath9k(const char *ifname, int allchans)
{
	// get_ath9k_phy_idx
	int i;
	fprintf(stderr, "list fake channels for %s\n", ifname);
	int count=0;

	struct wifi_channels *list =
	    (struct wifi_channels *)safe_malloc(sizeof(struct wifi_channels) *
					   (13+1+166-36+1));
	for (i=0;i<13;i++) {
		list[count].channel = i+1;
		list[count].freq = 5*(i+1)+2407;
		list[count].noise = -95;	// achans.ic_chans[i].ic_noise;
		count++;
	}
	list[count].channel = 14;
	list[count].freq = 2484;
	list[count].noise = -95;	// achans.ic_chans[i].ic_noise;
	count++;
	for (i=36;i<166;i++) {
		list[count].channel = i+1;
		list[count].freq = (i+1)*5+5000;
		list[count].noise = -95;	// achans.ic_chans[i].ic_noise;
		count++;
	}

	list[count++].freq = -1;
	return list;
}

struct wifi_channels *list_channels_ath9k(char *devnr)
{
	return list_channelsext_ath9k(devnr, 1);
}
