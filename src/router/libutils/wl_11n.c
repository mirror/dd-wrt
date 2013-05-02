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

static struct wifi_channels *list_channelsext(const char *ifname, int allchans)
{
	struct ieee80211req_chaninfo chans;
	struct ieee80211req_chaninfo achans;
	const struct ieee80211_channel *c;
	int i;

	fprintf(stderr, "list channels for %s\n", ifname);
	if (do80211priv(ifname, IEEE80211_IOCTL_GETCHANINFO, &chans, sizeof(chans)) < 0) {
		fprintf(stderr, "unable to get channel information\n");
		return NULL;
	}
	if (!allchans) {
		uint8_t active[64];

		if (do80211priv(ifname, IEEE80211_IOCTL_GETCHANLIST, &active, sizeof(active)) < 0) {
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

	struct wifi_channels *list = (struct wifi_channels *)safe_malloc(sizeof(struct wifi_channels) * (achans.ic_nchans + 1));
	(void)memset(list, 0, (sizeof(struct wifi_channels) * ((achans.ic_nchans + 1))));

	char wl_mode[16];
	char wl_turbo[16];

	sprintf(wl_mode, "%s_net_mode", ifname);
	sprintf(wl_turbo, "%s_channelbw", ifname);
	int l = 0;
	int up = 0;
	char sb[32];
	sprintf(sb, "%s_nctrlsb", ifname);
	if (nvram_match(sb, "upper"))
		up = 1;

	for (i = 0; i < achans.ic_nchans; i++) {
#ifdef HAVE_BUFFALO
		if (achans.ic_chans[i].ic_flags & IEEE80211_CHAN_RADARFOUND)	//filter channels with detected radar
			continue;
#endif
		// filter out A channels if mode isnt A-Only or mixed
		if (IEEE80211_IS_CHAN_5GHZ(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "a-only")
			    && nvram_invmatch(wl_mode, "mixed")
			    && nvram_invmatch(wl_mode, "n5-only")
			    && nvram_invmatch(wl_mode, "na-only")) {
//                              fprintf(stderr,"5 Ghz %d is not compatible to a-only/mixed/na-only %X\n",achans.ic_chans[i].ic_freq,achans.ic_chans[i].ic_flags);
				continue;
			}
			if (nvram_match(wl_turbo, "40")
			    && (nvram_match(wl_mode, "n5-only")
				|| nvram_match(wl_mode, "mixed")
				|| nvram_match(wl_mode, "na-only"))) {
				if (up && !IEEE80211_IS_CHAN_11NA_HT40PLUS(&achans.ic_chans[i]))
					continue;
				if (!up && !IEEE80211_IS_CHAN_11NA_HT40MINUS(&achans.ic_chans[i]))
					continue;
			}
		}
		// filter out B/G channels if mode isnt g-only, b-only or mixed
		if (IEEE80211_IS_CHAN_2GHZ(&achans.ic_chans[i])) {
			if (nvram_invmatch(wl_mode, "g-only")
			    && nvram_invmatch(wl_mode, "mixed")
			    && nvram_invmatch(wl_mode, "b-only")
			    && nvram_invmatch(wl_mode, "n2-only")
			    && nvram_invmatch(wl_mode, "n-only")
			    && nvram_invmatch(wl_mode, "bg-mixed")
			    && nvram_invmatch(wl_mode, "ng-only")) {
				fprintf(stderr, "%s:%d\n", __func__, __LINE__);
				continue;
			}
#ifdef HAVE_BUFFALO_SA
			if (nvram_default_match("region", "SA", "")
			    && (!strcmp(getUEnv("region"), "AP") || !strcmp(getUEnv("region"), "US"))
			    && achans.ic_chans[i].ic_ieee > 11 && achans.ic_chans[i].ic_ieee <= 14)
				continue;
#endif
			if (nvram_match(wl_turbo, "40")
			    && (nvram_match(wl_mode, "n2-only")
				|| nvram_match(wl_mode, "n-only")
				|| nvram_match(wl_mode, "mixed")
				|| nvram_match(wl_mode, "ng-only"))) {
				if (up && !IEEE80211_IS_CHAN_11NG_HT40PLUS(&achans.ic_chans[i])) {
					fprintf(stderr, "%s:%d\n", __func__, __LINE__);
					continue;
				}
				if (!up && !IEEE80211_IS_CHAN_11NG_HT40MINUS(&achans.ic_chans[i])) {
					fprintf(stderr, "%s:%d\n", __func__, __LINE__);
					continue;
				}
			}
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
	 * int i; struct wifi_channels *list = (struct wifi_channels *) safe_malloc
	 * (sizeof (struct wifi_channels) * (ch+1) ); for (i = 0; i < ch; i++) {
	 * fscanf (in, "%s %s %s %s %s", csign, channel, ppp, freq, dum1); if
	 * (!strcmp (csign, "Current")) break; list[i].channel = atoi (channel);
	 * list[i].freq = strdup (freq); channelcount++; } fclose (in); return
	 * list; 
	 */
}

int getassoclist_11n(char *ifname, unsigned char *list)
{
	unsigned char *buf;

	buf = safe_malloc(24 * 1024);
	memset(buf, 0, 1024 * 24);
	unsigned char *cp;
	int len;
	struct iwreq iwr;
	int s;
	char type[32];
	char netmode[32];
	unsigned int *count = (unsigned int *)list;

	sprintf(type, "%s_mode", ifname);
	sprintf(netmode, "%s_net_mode", ifname);
	if (nvram_match(netmode, "disabled")) {
		free(buf);
		return 0;
	}
	int mincount = 0;

	if (nvram_match(type, "wdssta") || nvram_match(type, "sta")
	    || nvram_match(type, "wet")) {
		int assoc = isAssociated(ifname);

		if (!assoc) {
			free(buf);
			return 0;
		}
		char mac[6];

		getAssocMAC(ifname, mac);
		memcpy(&list[4], mac, 6);
		count[0] = 1;
		mincount = 1;
	}
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		free(buf);
		mincount = 1;
		return mincount;
	}
	(void)memset(&iwr, 0, sizeof(iwr));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.length = 1024 * 24;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		close(s);
		free(buf);
		return mincount;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		close(s);
		free(buf);
		return mincount;
	}

	cp = buf;
	unsigned char *l = (unsigned char *)list;

	count[0] = 0;
	l += 4;
	do {
		struct ieee80211req_sta_info *si;

		si = (struct ieee80211req_sta_info *)cp;
		memcpy(l, &si->isi_macaddr[0], 6);
		if (l[0] == 0 && l[1] == 0 && l[2] == 0 && l[3] == 0 && l[4] == 0 && l[5] == 0)
			break;
		l += 6;
		count[0]++;
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	close(s);
	free(buf);

	return mincount > count[0] ? mincount : count[0];
}

int getRssi_11n(char *ifname, unsigned char *mac)
{
	unsigned char *buf = safe_malloc(24 * 1024);

	memset(buf, 0, 1024 * 24);
	unsigned char *cp;
	int len;
	struct iwreq iwr;
	int s;
	char nb[32];
	sprintf(nb, "%s_bias", ifname);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		free(buf);
		return 0;
	}
	(void)memset(&iwr, 0, sizeof(iwr));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.length = 1024 * 24;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		close(s);
		free(buf);
		return 0;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		close(s);
		free(buf);
		return 0;
	}

	cp = buf;
	char maccmp[6];

	memset(maccmp, 0, 6);
	do {
		struct ieee80211req_sta_info *si;

		si = (struct ieee80211req_sta_info *)cp;
		if (!memcmp(&si->isi_macaddr[0], mac, 6)) {
			close(s);
			int rssi = si->isi_noise + si->isi_rssi;

			free(buf);

			return rssi + atoi(nvram_default_get(nb, "0"));
		}
		if (!memcmp(&si->isi_macaddr[0], mac, 6))
			break;
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	close(s);
	free(buf);
	return 0;
}

int getUptime_11n(char *ifname, unsigned char *mac)
{
	unsigned char *buf = safe_malloc(24 * 1024);

	memset(buf, 0, 24 * 1024);
	unsigned char *cp;
	int len;
	struct iwreq iwr;
	int s;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		free(buf);
		return 0;
	}
	(void)memset(&iwr, 0, sizeof(iwr));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		close(s);
		free(buf);
		return 0;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		close(s);
		free(buf);
		return -1;
	}
	cp = buf;
	char maccmp[6];

	memset(maccmp, 0, 6);
	do {
		struct ieee80211req_sta_info *si;

		si = (struct ieee80211req_sta_info *)cp;
		if (!memcmp(&si->isi_macaddr[0], mac, 6)) {
			close(s);
			int uptime = si->isi_uptime;

			free(buf);
			return uptime;
		}
		if (!memcmp(&si->isi_macaddr[0], mac, 6))
			break;
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	close(s);
	free(buf);
	return 0;
}

int getNoise_11n(char *ifname, unsigned char *mac)
{
	unsigned char *buf = safe_malloc(24 * 1024);

	memset(buf, 0, 24 * 1024);
	unsigned char *cp;
	int len;
	struct iwreq iwr;
	int s;
	char nb[32];
	sprintf(nb, "%s_bias", ifname);

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		free(buf);
		return 0;
	}
	(void)memset(&iwr, 0, sizeof(iwr));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));
	iwr.u.data.pointer = (void *)buf;
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		close(s);
		free(buf);
		return 0;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		close(s);
		free(buf);
		return -1;
	}

	cp = buf;
	char maccmp[6];

	memset(maccmp, 0, 6);
	do {
		struct ieee80211req_sta_info *si;

		si = (struct ieee80211req_sta_info *)cp;
		if (!memcmp(&si->isi_macaddr[0], mac, 6)) {
			close(s);
			int noise = si->isi_noise;

			free(buf);
			return noise + atoi(nvram_default_get(nb, "0"));
		}
		if (!memcmp(&si->isi_macaddr[0], mac, 6))
			break;
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	close(s);
	free(buf);
	return 0;
}
