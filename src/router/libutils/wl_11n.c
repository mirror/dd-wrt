/*
 * Wireless network adapter utilities
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: wl.c,v 1.3 2005/11/11 09:26:19 seg Exp $
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

#include "wireless.h"
#undef WPA_OUI
#undef WME_OUI
#include "../madwifi.dev/madwifi_mimo.dev/core/net80211/ieee80211.h"
#include "../madwifi.dev/madwifi_mimo.dev/core/net80211/ieee80211_crypto.h"
#include "../madwifi.dev/madwifi_mimo.dev/core/net80211/ieee80211_ioctl.h"

struct wifi_channels {
	int channel;
	int freq;
	int noise;
};







static struct wifi_channels *list_channelsext(const char *ifname, int allchans)
{
	struct ieee80211req_chaninfo chans;
	struct ieee80211req_chaninfo achans;
	const struct ieee80211_channel *c;
	int i;

	fprintf (stderr, "list channels for %s\n", ifname);
	if (do80211priv
	    (ifname, IEEE80211_IOCTL_GETCHANINFO, &chans, sizeof(chans)) < 0) {
		fprintf(stderr, "unable to get channel information\n");
		return NULL;
	}
	if (!allchans) {
		uint8_t active[64];

		if (do80211priv
		    (ifname, IEEE80211_IOCTL_GETCHANLIST, &active,
		     sizeof(active)) < 0) {
			fprintf(stderr, "unable to get active channel list\n");
			return NULL;
		}
		memset(&achans, 0, sizeof(achans));
		for (i = 0; i < chans.ic_nchans; i++) {
			c = &chans.ic_chans[i];
			if (isset(active, c->ic_ieee) || allchans)
				achans.ic_chans[achans.ic_nchans++] = *c;
		}
	} else
		achans = chans;

	struct wifi_channels *list =
	    (struct wifi_channels *)malloc(sizeof(struct wifi_channels) *
					   (achans.ic_nchans + 1));

	char wl_mode[16];
	char wl_turbo[16];

	sprintf(wl_mode, "%s_net_mode", ifname);
	sprintf(wl_turbo, "%s_channelbw", ifname);
	int l = 0;
int up=0;
char sb[32];
sprintf(sb,"%s_nctrlsb",ifname);
if (nvram_match(sb,"upper"))
    up=1;

	for (i = 0; i < achans.ic_nchans; i++) {

		// filter out A channels if mode isnt A-Only or mixed
		if (IEEE80211_IS_CHAN_A(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "a-only") && nvram_invmatch(wl_mode, "mixed") && nvram_invmatch(wl_mode, "na-only"))
				continue;
		}
		if (IEEE80211_IS_CHAN_11NA_HT20(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "na-only")
			    && nvram_invmatch(wl_mode, "mixed") && nvram_invmatch(wl_turbo, "20") && nvram_invmatch(wl_turbo, "2040"))
				{
				continue;
				}
		}
		if (IEEE80211_IS_CHAN_11NA_HT40PLUS(&achans.ic_chans[i]) || IEEE80211_IS_CHAN_11NA_HT40MINUS(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "na-only")
			    && nvram_invmatch(wl_mode, "mixed") && nvram_invmatch(wl_turbo, "40") && nvram_invmatch(wl_turbo, "2040"))
				{
				continue;
				}
			if (up && !IEEE80211_IS_CHAN_11N_CTL_U_CAPABLE(&achans.ic_chans[i]))
			    continue;
			if (!up && !IEEE80211_IS_CHAN_11N_CTL_L_CAPABLE(&achans.ic_chans[i]))
			    continue;
		}
		if (IEEE80211_IS_CHAN_11NG_HT20(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "ng-only")
			    && nvram_invmatch(wl_mode, "mixed") && nvram_invmatch(wl_turbo, "20") && nvram_invmatch(wl_turbo, "2040"))
				{
				continue;
				}
		}
		if (IEEE80211_IS_CHAN_11NG_HT40PLUS(&achans.ic_chans[i]) || IEEE80211_IS_CHAN_11NG_HT40MINUS(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "ng-only")
			    && nvram_invmatch(wl_mode, "mixed") && nvram_invmatch(wl_turbo, "40") && nvram_invmatch(wl_turbo, "2040"))
				{
				continue;
				}
			if (up && !IEEE80211_IS_CHAN_11N_CTL_U_CAPABLE(&achans.ic_chans[i]))
			    continue;
			if (!up && !IEEE80211_IS_CHAN_11N_CTL_L_CAPABLE(&achans.ic_chans[i]))
			    continue;
		}
		// filter out B/G channels if mode isnt g-only, b-only or mixed
		if (IEEE80211_IS_CHAN_ANYG(&achans.ic_chans[i])
		    || IEEE80211_IS_CHAN_B(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "g-only")
			    && nvram_invmatch(wl_mode, "mixed")
			    && nvram_invmatch(wl_mode, "b-only")
			    && nvram_invmatch(wl_mode, "bg-mixed") 
			    && nvram_invmatch(wl_mode, "ng-only"))
				continue;
		}

		list[l].channel = achans.ic_chans[i].ic_ieee;
		list[l].freq = achans.ic_chans[i].ic_freq;
		list[l].noise = -95;	// achans.ic_chans[i].ic_noise;
		l++;
	}

	list[l].freq = -1;
	return list;
}

struct wifi_channels *list_channels_11n(char *devnr)
{
	return list_channelsext(devnr, 1);
	/*
	 * char csign[64]; char channel[64]; char ppp[64]; char freq[64]; char
	 * dum1[64]; char dum2[64]; char dum3[64]; char dum4[64];
	 * 
	 * char cmd[64]; sprintf (cmd, "iwlist %s chan>/tmp/.channels", devnr);
	 * system (cmd); FILE *in = fopen ("/tmp/.channels", "rb"); if (in ==
	 * NULL) return NULL; fscanf (in, "%s %s %s %s %s %s %s %s", csign,
	 * channel, ppp, freq, dum1, dum2, dum3, dum4); int ch = atoi (channel);
	 * int i; struct wifi_channels *list = (struct wifi_channels *) malloc
	 * (sizeof (struct wifi_channels) * (ch+1) ); for (i = 0; i < ch; i++) {
	 * fscanf (in, "%s %s %s %s %s", csign, channel, ppp, freq, dum1); if
	 * (!strcmp (csign, "Current")) break; list[i].channel = atoi (channel);
	 * list[i].freq = strdup (freq); channelcount++; } fclose (in); return
	 * list; 
	 */
}


