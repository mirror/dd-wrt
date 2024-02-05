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
#include <utils.h>
#include <wlutils.h>
#include <shutils.h>
#include <utils.h>
#include <bcmnvram.h>
#include <syslog.h>
#include <fcntl.h>
#include <glob.h>
#include <channelcache.h>
#include <sys/socket.h>
#include <linux/if.h>

struct site_survey_list *open_site_survey(char *name)
{
	FILE *fp;

	if (name == NULL || *(name) == 0)
		eval("site_survey");
	else {
		eval("site_survey", name);
	}

	struct site_survey_list *local_site_survey_lists = malloc(sizeof(struct site_survey_list) * SITE_SURVEY_NUM);
	bzero(local_site_survey_lists, sizeof(struct site_survey_list) * SITE_SURVEY_NUM);

	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&local_site_survey_lists[0], sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1, fp);
		fclose(fp);
		return local_site_survey_lists;
	}
	return NULL;
}

int getValueFromPath(char *path, int dev, char *fmt, int *err)
{
	char *globstring;
	int value = 0;
	if (err)
		*err = -1;
	asprintf(&globstring, path, dev);
	FILE *fp = fopen(globstring, "rb");
	if (fp) {
		if (err)
			*err = 0;
		fscanf(fp, fmt, &value);
		fclose(fp);
	}
	free(globstring);
	return value;
}

/*
 * DD-WRT addition (loaned from radauth) 
 */

int ieee80211_mhz2ieee(int freq)
{
	if (freq == 2484)
		return 14;
	if (freq == 2407)
		return 0;
	if (freq < 2484 && freq > 2407)
		return (freq - 2407) / 5;
	if (freq < 2412) {
		return ((freq - 2407) / 5) + 256;
	}
	if (freq < 2502 && freq > 2484)
		return 14;
	if (freq < 2512 && freq > 2484)
		return 15;
	if (freq > 2484 && freq < 4000)
		return (15 + ((freq - 2512) / 20)) & 0xff;
	if (freq < 4990 && freq > 4940)
		return ((freq * 10) + (((freq % 5) == 2) ? 5 : 0) - 49400) / 5;
	// 5000 will become  channel 200
	if (freq >= 4800 && freq < 5005)
		return (freq - 4000) / 5;
	if (freq < 5000)
		return 15 + ((freq - 2512) / 20);
	if (freq == 58320)
		return 1;
	if (freq == 60480)
		return 2;
	if (freq == 62640)
		return 3;
	if (freq == 64800)
		return 4;

	return ((freq - 5000) / 5) % 0xff;
}

#ifdef HAVE_MVEBU
int is_wrt3200(void)
{
	FILE *fp = fopen("/proc/device-tree/model", "r");
	if (fp) {
		char modelstr[32];
		fread(modelstr, 1, 31, fp);
		if (strstr(modelstr, "WRT3200ACM")) {
			fclose(fp);
			return 1;
		}
		fclose(fp);
	}
	return 0;
}
#endif

#ifdef HAVE_MADWIFI
int has_2ghz(const char *prefix)
{
	INITVALUECACHE();
#ifdef HAVE_MVEBU
	if (is_wrt3200() && is_mvebu(prefix) && !strncmp(prefix, "wlan0", 5)) {
		RETURNVALUE(0);
	}
#endif
	if (is_mac80211(prefix)) {
		RETURNVALUE(mac80211_check_band(prefix, 2));
	}

	RETURNVALUE(has_athmask(dn, 0x8));
	EXITVALUECACHE();

	return ret;
}

int has_5ghz(const char *prefix)
{
	INITVALUECACHE();
	if (has_ad(prefix)) {
		RETURNVALUE(0);
	}
	if (is_mac80211(prefix)) {
		RETURNVALUE(mac80211_check_band(prefix, 5));
	}

	RETURNVALUE(has_athmask(dn, 0x1));
	EXITVALUECACHE();
	return ret;
}

#endif

#if !defined(HAVE_MADWIFI)
int has_vht160(const char *prefix)
{
#ifdef HAVE_RT2880
	char *dev = getWifiDeviceName(prefix, NULL);
	if (dev && !strcmp(dev, "MT7615 802.11ac"))
		return 1;
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *next;
	char *ifname = nvram_nget("%s_ifname", prefix);

	if (wl_iovar_get(ifname, "cap", (void *)caps, sizeof(caps)))
		return 0; // error
	foreach(cap, caps, next)
	{
		if (!strcmp(cap, "160")) {
			return 1;
		}
	}
#endif
	return 0;
}

int has_vht80plus80(const char *prefix)
{
	//                      char *dev = getWifiDeviceName(prefix);
	//                      if (dev && !strcmp(dev, "MT7615 802.11ac"))
	//                          return 1;

	return 0;
}
#endif

#ifndef HAVE_MADWIFI

unsigned int ieee80211_ieee2mhz(unsigned int chan)
{
	if (chan == 14)
		return 2484;
	if (chan < 14)
		return ((2407) + chan * 5);
	else if (chan < 27)
		return ((2512) + ((chan - 15) * 20));
	if (chan >= 182 && chan <= 196)
		return ((4000) + (chan * 5));
	else
		return ((5000) + (chan * 5));
}

#if defined(HAVE_RT2880) || defined(HAVE_RT61)
char *getRADev(char *prefix)
{
	char *ifname = NULL;

	if (!strcmp(prefix, "wl0"))
		ifname = "ra0";
	if (!strcmp(prefix, "wl0.1"))
		ifname = "ra1";
	if (!strcmp(prefix, "wl0.2"))
		ifname = "ra2";
	if (!strcmp(prefix, "wl0.3"))
		ifname = "ra3";
	if (!strcmp(prefix, "wl0.4"))
		ifname = "ra4";
	if (!strcmp(prefix, "wl0.5"))
		ifname = "ra5";
	if (!strcmp(prefix, "wl0.6"))
		ifname = "ra6";
	if (!strcmp(prefix, "wl0.7"))
		ifname = "ra7";

	if (!strcmp(prefix, "wl1"))
		ifname = "ba0";
	if (!strcmp(prefix, "wl1.1"))
		ifname = "ba1";
	if (!strcmp(prefix, "wl1.2"))
		ifname = "ba2";
	if (!strcmp(prefix, "wl1.3"))
		ifname = "ba3";
	if (!strcmp(prefix, "wl1.4"))
		ifname = "ba4";
	if (!strcmp(prefix, "wl1.5"))
		ifname = "ba5";
	if (!strcmp(prefix, "wl1.6"))
		ifname = "ba6";
	if (!strcmp(prefix, "wl1.7"))
		ifname = "ba7";

	if (ifname)
		return ifname;
	else
		return prefix;
}

int has_5ghz(const char *prefix)
{
	if (!strcmp(prefix, "wl0"))
		return 0;
	if (!strcmp(prefix, "ra0"))
		return 0;
	return 1;
}

int has_2ghz(const char *prefix)
{
	if (!strcmp(prefix, "wl0"))
		return 1;
	if (!strcmp(prefix, "ra0"))
		return 1;
	return 0;
}

int getchannels(unsigned int *list, char *ifname)
{
	if (!strcmp(ifname, "ra0")) {
		list[0] = 1;
		list[1] = 2;
		list[2] = 3;
		list[3] = 4;
		list[4] = 5;
		list[5] = 6;
		list[6] = 7;
		list[7] = 8;
		list[8] = 9;
		list[9] = 10;
		list[10] = 11;
		list[11] = 12;
		list[12] = 13;
		list[13] = 14;
#ifdef HAVE_BUFFALO
		// temporal solution, needs to be done right
		if (nvram_match("region", "EU"))
			return 11;
		else if (nvram_match("region", "RU"))
			return 11;
		else if (nvram_match("region", "US"))
			return 11;
		else if (nvram_match("region", "JP"))
			return 13;
		else
			return 11;
#else
		return 14;
#endif
	} else {
		int i;
		int cnt = 0;
#ifdef HAVE_BUFFALO
		if (nvram_match("region", "EU")) {
			for (i = 0; i < 4; i++)
				list[cnt++] = 36 + (i * 4);
		} else if (nvram_match("region", "RU")) {
			for (i = 0; i < 4; i++)
				list[cnt++] = 36 + (i * 4);
		} else if (nvram_match("region", "US")) {
			for (i = 0; i < 4; i++)
				list[cnt++] = 36 + (i * 4);
			for (i = 0; i < 5; i++)
				list[cnt++] = 149 + (i * 4);
		} else if (nvram_match("region", "JP")) {
			for (i = 0; i < 8; i++)
				list[cnt++] = 36 + (i * 4);
			for (i = 0; i < 11; i++)
				list[cnt++] = 100 + (i * 4);
		} else {
			for (i = 0; i < 4; i++)
				list[cnt++] = 36 + (i * 4);
		}

#else
		for (i = 0; i < 8; i++)
			list[cnt++] = 36 + (i * 4);

		for (i = 0; i < 11; i++)
			list[cnt++] = 100 + (i * 4);

		for (i = 0; i < 7; i++)
			list[cnt++] = 149 + (i * 4);
#endif
		return cnt;
	}
}

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
#include <linux/socket.h>
#include <linux/if.h>
#define __user
#include "wireless.h"

static int wrqfreq_to_int(struct iwreq *wrq)
{
	int freq, i;
	freq = wrq->u.freq.m;
	if (freq < 1000) {
		return freq;
	}
	int divisor = 1000000;
	int e = wrq->u.freq.e;
	for (i = 0; i < e; i++)
		divisor /= 10;
	if (divisor)
		freq /= divisor;
	return freq;
}

struct wifi_interface *wifi_getfreq(char *ifname)
{
	struct iwreq wrq;
	int freq;

	(void)bzero(&wrq, sizeof(struct iwreq));
	strlcpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);
	ioctl(getsocket(), SIOCGIWFREQ, &wrq);
	closesocket();
	freq = wrq.u.freq.m;
	struct wifi_interface *interface = (struct wifi_interface *)malloc(sizeof(struct wifi_interface));
	if (freq < 1000) {
		interface->freq = ieee80211_ieee2mhz(freq);
		return interface;
	}
	interface->freq = wrqfreq_to_int(&wrq);
	return interface;
}

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
int wifi_getchannel(char *ifname)
{
	struct wifi_interface *interface = wifi_getfreq(ifname);
	int channel = ieee80211_mhz2ieee(interface->freq);
	free(interface);
	return channel;
}
#endif
long long wifi_getrate(char *ifname)
{
	struct iwreq wrq;

	(void)bzero(&wrq, sizeof(struct iwreq));
	strlcpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);
	ioctl(getsocket(), SIOCGIWRATE, &wrq);
	closesocket();
	return wrq.u.bitrate.value;
}

int get_radiostate(char *ifname)
{
	if (nvram_nmatch("disabled", "%s_net_mode", ifname))
		return 0;
	if (!strcmp(ifname, "wl0"))
		ifname = "ra0";
	if (!strcmp(ifname, "wl1"))
		ifname = "ba0";
	struct ifreq ifr;
	int skfd = getsocket();

	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		closesocket();
		return -1;
	}
	if ((ifr.ifr_flags & IFF_UP)) {
		closesocket();
		return 1;
	}
	closesocket();
	return 0;
}

static const char *ieee80211_ntoa(const uint8_t mac[6])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

typedef union _MACHTTRANSMIT_SETTING {
	struct {
		unsigned short MCS : 7; // MCS
		unsigned short BW : 1; //channel bandwidth 20MHz or 40 MHz
		unsigned short ShortGI : 1;
		unsigned short STBC : 2; //SPACE
		unsigned short rsv : 3;
		unsigned short MODE : 2; // Use definition MODE_xxx.
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char ApIdx;
	unsigned char Addr[6];
	unsigned char Aid;
	unsigned char Psm; // 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char MimoPs; // 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char AvgRssi0;
	char AvgRssi1;
	char AvgRssi2;
	unsigned int ConnectedTime;
	MACHTTRANSMIT_SETTING TxRate;
	//#ifdef RTMP_RBUS_SUPPORT
	unsigned int LastRxRate;
	int StreamSnr[3];
	int SoundingRespSnr[3];
	//#endif // RTMP_RBUS_SUPPORT //
} RT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[128]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;

int OidQueryInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t)ptr;
	wrq.u.data.flags = OidQueryCode;

	return (ioctl(socket_id, (SIOCIWFIRSTPRIV + 0x0E), &wrq));
}

#include "stapriv.h"
#include "oid.h"

STAINFO *getRaStaInfo(char *ifname)
{
	char G_bRadio = 1; //TRUE

	int ConnectStatus = 0;
	unsigned char BssidQuery[6];

	if (!nvram_nmatch("sta", "%s_mode", ifname) && !nvram_nmatch("apsta", "%s_mode", ifname) &&
	    !nvram_nmatch("apstawet", "%s_mode", ifname)) {
		return NULL;
	}
	char *ifn = "ra0";
	if (!strcmp(ifname, "wl1"))
		ifn = "ba0";

	if (nvram_nmatch("apsta", "%s_mode", ifname) || nvram_nmatch("apstawet", "%s_mode", ifname)) {
		ifn = "apcli0";
		if (!strcmp(ifname, "wl1"))
			ifn = "apcli1";
	}

	int s;

	s = getsocket();
	if (OidQueryInformation(OID_GEN_MEDIA_CONNECT_STATUS, s, ifn, &ConnectStatus, sizeof(ConnectStatus)) < 0) {
		return NULL;
	}
	if (OidQueryInformation(RT_OID_802_11_RADIO, s, ifn, &G_bRadio, sizeof(G_bRadio)) < 0) {
		return NULL;
	}
	if (G_bRadio && ConnectStatus == 1) {
		bzero(&BssidQuery, sizeof(BssidQuery));
		OidQueryInformation(OID_802_11_BSSID, s, ifn, &BssidQuery, sizeof(BssidQuery));
		long RSSI;
		int nNoiseDbm;
		unsigned char lNoise; // this value is (ULONG) in Ndis driver (NOTICE!!!)

		OidQueryInformation(RT_OID_802_11_RSSI, s, ifn, &RSSI, sizeof(RSSI));
		OidQueryInformation(RT_OID_802_11_QUERY_NOISE_LEVEL, s, ifn, &lNoise, sizeof(lNoise));
		nNoiseDbm = lNoise;
		nNoiseDbm -= 143;

		STAINFO *ret = safe_malloc(sizeof(STAINFO));

		memcpy(ret->mac, BssidQuery, 6);
		strcpy(ret->ifname, ifn);
		ret->rssi = RSSI;
		ret->noise = -nNoiseDbm;
		return ret;
	}
	return NULL;
}

#define RTPRIV_IOCTL_GET_MAC_TABLE (SIOCIWFIRSTPRIV + 0x0F)

int getassoclist(char *ifname, unsigned char *list)
{
	struct iwreq iwr;
	unsigned int *count = (unsigned int *)list;

	RT_802_11_MAC_TABLE table = { 0 };
	int s, i;

	if (nvram_nmatch("disabled", "%s_net_mode", ifname)) {
		return 0;
	}

	int state = get_radiostate(ifname);
	int ignore = 0;

	if (state == 0 || state == -1) {
		ignore = 1;
	}
	if (!ignore) {
		s = getsocket();
		if (s < 0) {
			ignore = 1;
		}
		(void)bzero(&iwr, sizeof(struct iwreq));
		(void)strlcpy(iwr.ifr_name, getRADev(ifname), sizeof(iwr.ifr_name) - 1);

		iwr.u.data.pointer = (caddr_t)&table;
		if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE, &iwr) < 0) {
			ignore = 1;
		}
	}

	unsigned char *l = (unsigned char *)list;

	count[0] = 0;
	l += 4;
	STAINFO *sta = getRaStaInfo(ifname);

	if (sta != NULL) {
		memcpy(l, sta->mac, 6);
		l += 6;
		count[0]++;
		free(sta);
	}
	if (!ignore && table.Num < 128)
		for (i = 0; i < table.Num; i++) {
			memcpy(l, &table.Entry[i].Addr, 6);
			if (l[0] == 0 && l[1] == 0 && l[2] == 0 && l[3] == 0 && l[4] == 0 && l[5] == 0)
				break;
			l += 6;
			count[0]++;
		}

	return count[0];
}

int getWifiInfo(char *ifname, unsigned char *mac, int field)
{
	struct iwreq iwr;
	RT_802_11_MAC_TABLE table = { 0 };
	int s, i;

	if (nvram_nmatch("disabled", "%s_net_mode", ifname)) {
		return 0;
	}

	int state = get_radiostate(ifname);
	int ignore = 0;

	if (state == 0 || state == -1) {
		ignore = 1;
	}
	if (!ignore) {
		s = getsocket();
		if (s < 0) {
			ignore = 1;
		}
		(void)bzero(&iwr, sizeof(struct iwreq));
		(void)strlcpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name) - 1);

		iwr.u.data.pointer = (caddr_t)&table;
		if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE, &iwr) < 0) {
			ignore = 1;
		}
	}

	STAINFO *sta = getRaStaInfo(ifname);
	switch (field) {
	case INFO_RSSI:
		if (sta != NULL) {
			if (!memcmp(mac, sta->mac, 6)) {
				int retu = sta->rssi;

				free(sta);
				return -95 + retu;
			}
			free(sta);
		}
		if (!ignore && table.Num < 128)
			for (i = 0; i < table.Num; i++) {
				if (!memcmp(mac, &table.Entry[i].Addr, 6)) {
					return -95 + table.Entry[i].AvgRssi0;
				}
			}
		return 0;
	case INFO_NOISE:
		return -95;
	case INFO_UPTIME:
		return 0;
	}
	return 0;
}

void radio_off(int idx)
{
	switch (idx) {
	case -1:
		idx = 1;
	case 0:
		if (!nvram_match("wl0_net_mode", "disabled")) {
			eval("iwpriv", "ra0", "set", "RadioOn=0");
			eval("iwpriv", "ra0", "set", "WlanLed=0");
		}
	case 1:
		if (!nvram_match("wl1_net_mode", "disabled") && idx == 1) {
			eval("iwpriv", "ba0", "set", "RadioOn=0");
			eval("iwpriv", "ba0", "set", "WlanLed=0");
		}
	}
	led_control(LED_WLAN0, LED_OFF);
}

void radio_on(int idx)
{
	switch (idx) {
	case -1:
		idx = 1;
	case 0:
		if (!nvram_match("wl0_net_mode", "disabled")) {
			eval("iwpriv", "ra0", "set", "RadioOn=1");
			eval("iwpriv", "ra0", "set", "WlanLed=1");
		}
	case 1:
		if (!nvram_match("wl1_net_mode", "disabled") && idx == 1) {
			eval("iwpriv", "ba0", "set", "RadioOn=1");
			eval("iwpriv", "ba0", "set", "WlanLed=1");
		}
	}
	led_control(LED_WLAN0, LED_ON);
}

#else
#ifdef WL_CHANSPEC_BW_8080

static const uint8 wf_chspec_bw_mhz[] = { 5, 10, 20, 40, 80, 160, 160 };

#define WF_NUM_BW (sizeof(wf_chspec_bw_mhz) / sizeof(uint8))

/* 40MHz channels in 5GHz band */
static const uint8 wf_5g_40m_chans[] = { 38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159 };

#define WF_NUM_5G_40M_CHANS (sizeof(wf_5g_40m_chans) / sizeof(uint8))

/* 80MHz channels in 5GHz band */
static const uint8 wf_5g_80m_chans[] = { 42, 58, 106, 122, 138, 155 };

#define WF_NUM_5G_80M_CHANS (sizeof(wf_5g_80m_chans) / sizeof(uint8))

/* 160MHz channels in 5GHz band */
static const uint8 wf_5g_160m_chans[] = { 50, 114 };

#define WF_NUM_5G_160M_CHANS (sizeof(wf_5g_160m_chans) / sizeof(uint8))

static uint8 center_chan_to_edge(uint bw)
{
	/* edge channels separated by BW - 10MHz on each side
	 * delta from cf to edge is half of that,
	 * MHz to channel num conversion is 5MHz/channel
	 */
	return (uint8)(((bw - 20) / 2) / 5);
}

static uint8 channel_low_edge(uint center_ch, uint bw)
{
	return (uint8)(center_ch - center_chan_to_edge(bw));
}

/* return control channel given center channel and side band */
static uint8 channel_to_ctl_chan(uint center_ch, uint bw, uint sb)
{
	return (uint8)(channel_low_edge(center_ch, bw) + sb * 4);
}

/* convert bandwidth from chanspec to MHz */
static uint bw_chspec_to_mhz(chanspec_t chspec)
{
	uint bw;

	bw = (chspec & WL_CHANSPEC_BW_MASK) >> WL_CHANSPEC_BW_SHIFT;
	return (bw >= WF_NUM_BW ? 0 : wf_chspec_bw_mhz[bw]);
}

uint8 wf_chspec_ctlchan(chanspec_t chspec)
{
	uint center_chan;
	uint bw_mhz;
	uint sb;

	/* Is there a sideband ? */
	if (CHSPEC_IS20(chspec)) {
		return CHSPEC_CHANNEL(chspec);
	} else {
		sb = CHSPEC_CTL_SB(chspec) >> WL_CHANSPEC_CTL_SB_SHIFT;

		if (CHSPEC_IS8080(chspec)) {
			bw_mhz = 80;

			if (sb < 4) {
				center_chan = CHSPEC_CHAN1(chspec);
			} else {
				center_chan = CHSPEC_CHAN2(chspec);
				sb -= 4;
			}

			/* convert from channel index to channel number */
			center_chan = wf_5g_80m_chans[center_chan];
		} else {
			bw_mhz = bw_chspec_to_mhz(chspec);
			center_chan = CHSPEC_CHANNEL(chspec) >> WL_CHANSPEC_CHAN_SHIFT;
		}

		return (channel_to_ctl_chan(center_chan, bw_mhz, sb));
	}
}

static int getcenterchannel(chanspec_t chspec)
{
	const char *band;
	uint ctl_chan;

	/* ctl channel */
	return wf_chspec_ctlchan(chspec);
}

#else

#ifndef CHSPEC_IS40
#define WL_CHANSPEC_CTL_SB_MASK 0x0300
#define WL_CHANSPEC_CTL_SB_LOWER 0x0100
#define WL_CHANSPEC_CTL_SB_UPPER 0x0200

#define WL_CHANSPEC_BW_MASK 0x0C00
#define WL_CHANSPEC_BW_40 0x0C00

#define CHSPEC_CHANNEL(chspec) ((uint8)(chspec & 0xff))

#define CHSPEC_IS40(chspec) (((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40)
#define CHSPEC_SB_UPPER(chspec) ((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_UPPER)
#define CHSPEC_SB_LOWER(chspec) ((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LOWER)
#endif

static int getcenterchannel(chanspec_t chspec)
{
	const char *band;
	uint ctl_chan;
	int channel;
	channel = CHSPEC_CHANNEL(chspec);
	/* check for non-default band spec */

	if (CHSPEC_IS40(chspec)) {
		if (CHSPEC_SB_UPPER(chspec)) {
			channel += 2;
		} else {
			channel -= 2;
		}
	}
	return channel;
}
#endif
#ifdef HAVE_80211AC
int has_beamforming(const char *prefix)
{
	wlc_rev_info_t rev;
	int c = 0;
	if (!strcmp(prefix, "wl0"))
		c = 0;
	else if (!strcmp(prefix, "wl1"))
		c = 1;
	else if (!strcmp(prefix, "wl2"))
		c = 2;

	char *name = get_wl_instance_name(c);
	wl_ioctl(name, WLC_GET_REVINFO, &rev, sizeof(rev));

	if (rev.corerev < 40)
		return 0; /* TxBF unsupported */

	return 1;
}

int has_mumimo(const char *prefix)
{
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *next;
	int c = 0;
	if (!strcmp(prefix, "wl0"))
		c = 0;
	else if (!strcmp(prefix, "wl1"))
		c = 1;
	else if (!strcmp(prefix, "wl2"))
		c = 2;

	char *name = get_wl_instance_name(c);
	if (wl_iovar_get(name, "cap", (void *)caps, sizeof(caps)))
		return -1;

	foreach(cap, caps, next)
	{
		if (!strcmp(cap, "multi-user-beamformer"))
			return 1;
	}
	return 0;
}
#endif

int getchannels(unsigned int *retlist, char *ifname)
{
#ifdef HAVE_80211AC
	char buf[WLC_IOCTL_MAXLEN];
	int buflen;
	int ret;
	int count = 0;
	int i;
	chanspec_t c = 0, *chanspec;

	char abbrev[WLC_CNTRY_BUF_SZ] = ""; /* default.. current locale */
	wl_uint32_list_t *list;

	bzero(buf, WLC_IOCTL_MAXLEN);
	strcpy(buf, "chanspecs");
	buflen = strlen(buf) + 1;

	chanspec = (chanspec_t *)(buf + buflen);
	*chanspec = c;
	buflen += (sizeof(chanspec_t));

	strncpy(buf + buflen, abbrev, WLC_CNTRY_BUF_SZ);

	buflen += WLC_CNTRY_BUF_SZ;

	list = (wl_uint32_list_t *)(buf + buflen);
	list->count = WL_NUMCHANSPECS;
	buflen += sizeof(uint32) * (WL_NUMCHANSPECS + 1);

	if ((ret = wl_ioctl(ifname, WLC_GET_VAR, &buf[0], buflen)))
		if (ret < 0)
			return 0;

	int wl = get_wl_instance(ifname);

	list = (wl_uint32_list_t *)buf;

	int mask = 0;
	int bw = atoi(nvram_nget("wl%d_nbw", wl));
	if (!bw)
		bw = 20;

	int spec = 0;
#ifdef WL_CHANSPEC_BW_8080
	if (nvram_nmatch("8080", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_8080;
	else if (nvram_nmatch("160", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_160;
	else if (nvram_nmatch("80", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_80;
	else
#endif
		if (nvram_nmatch("40", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_40;
	else if (nvram_nmatch("20", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_20;
	else if (nvram_nmatch("0", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_20;
	else if (nvram_nmatch("10", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_10;
#ifdef WL_CHANSPEC_BW_5
	else if (nvram_nmatch("5", "wl%d_nbw", wl))
		mask = WL_CHANSPEC_BW_5;
#endif

	if (bw > 20) {
#ifdef WL_CHANSPEC_CTL_SB_UU
		if (bw == 80) {
			if (nvram_nmatch("lower", "wl%d_nctrlsb", wl) || nvram_nmatch("upper", "wl%d_nctrlsb", wl))
				nvram_nset("ll", "wl%d_nctrlsb", wl);
		}
		if (nvram_nmatch("uu", "wl%d_nctrlsb", wl))
			spec = WL_CHANSPEC_CTL_SB_UU;
		else if (nvram_nmatch("ul", "wl%d_nctrlsb", wl))
			spec = WL_CHANSPEC_CTL_SB_UL;
		else if (nvram_nmatch("lu", "wl%d_nctrlsb", wl))
			spec = WL_CHANSPEC_CTL_SB_LU;
		else if (nvram_nmatch("ll", "wl%d_nctrlsb", wl))
			spec = WL_CHANSPEC_CTL_SB_LL;
#endif
		if (bw == 40) {
			if (nvram_nmatch("uu", "wl%d_nctrlsb", wl) || nvram_nmatch("ul", "wl%d_nctrlsb", wl) ||
			    nvram_nmatch("lu", "wl%d_nctrlsb", wl) || nvram_nmatch("ll", "wl%d_nctrlsb", wl))
				nvram_nset("lower", "wl%d_nctrlsb", wl);
		}
		if (nvram_nmatch("lower", "wl%d_nctrlsb", wl))
			spec = WL_CHANSPEC_CTL_SB_LOWER;
		else if (nvram_nmatch("upper", "wl%d_nctrlsb", wl))
			spec = WL_CHANSPEC_CTL_SB_UPPER;
	}

	for (i = 0; i < list->count; i++) {
		c = list->element[i];
		int cspec = c & 0x700;
		int cbw = c & 0x3800;
		//        fprintf(stderr,"wl%d: %X spec %d, cbw %d\n",wl,c,cbw,cspec);
		if ((cbw == mask) && (cspec == spec)) {
			//        fprintf(stderr,"take wl%d: %X spec %d, cbw %d\n",wl,c,cbw,cspec);

			int channel = getcenterchannel(c);

			int a;
			int inlist = 0;
			for (a = 0; a < count; a++)
				if (retlist[a] == channel) {
					inlist = 1;
					break;
				}
			if (!inlist) {
				retlist[count++] = channel;
			}
		}
	}
	int a;
	// sort
	for (a = 0; a < count; a++) {
		for (i = 0; i < count - 1; i++) {
			if (retlist[i + 1] < retlist[i]) {
				int cc = retlist[i + 1];
				retlist[i + 1] = retlist[i];
				retlist[i] = cc;
			}
		}
	}

	return count;

#else //HAVE_80211AC

	// int ret, num;
	// num = (sizeof (*list) - 4) / 6; /* Maximum number of entries in the
	// buffer */
	// memcpy (list, &num, 4); /* First 4 bytes are the number of ent. */

	// ret = wl_ioctl (name, WLC_GET_VALID_CHANNELS, list, 128);
	// fprintf(stderr,"get channels\n");
	char exec[64];

	sprintf(exec, "wl -i %s channels", ifname);
	FILE *in = popen(exec, "r");

	int chan;
	int count = 0;

	while (!feof(in) && fscanf(in, "%d", &chan) == 1) {
		retlist[count++] = chan;
	}
	pclose(in);
#ifdef BUFFALO_JP
	return count - 1;
#else
	return count;
#endif

#endif
}

#ifdef TEST

void main(int argc, char *argv[])
{
	char buf[1024];
	getchannels(buf, "eth1");
}
#endif
int has_5ghz(const char *prefix)
{
#ifdef HAVE_QTN
	if (!strcmp(prefix, "wl1"))
		return 1;
#endif
	if (strstr(nvram_nget("%s_bandlist", prefix), "a"))
		return 1;

	return 0;
}

int has_2ghz(const char *prefix)
{
	if (strstr(nvram_nget("%s_bandlist", prefix), "b"))
		return 1;

	return 0;
}

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880)

int wifi_getchannel(char *ifn)
{
	channel_info_t ci;
	char name[32];
	sprintf(name, "%s_ifname", ifn);
	char *ifname = nvram_safe_get(name);

	bzero(&ci, sizeof(ci));
	wl_ioctl(ifname, WLC_GET_CHANNEL, &ci, sizeof(ci));
	if (ci.scan_channel > 0) {
		return ci.scan_channel;
	} else if (ci.hw_channel > 0) {
		return ci.hw_channel;
	} else
		return -1;
}
#endif
int wl_getbssid(char *wl, char *mac)
{
	int ret;
	struct ether_addr ea;

	wl_ioctl(wl, WLC_GET_BSSID, &ea, ETHER_ADDR_LEN);
	ether_etoa(&ea, mac);
	return 0;
}

int getassoclist(char *name, unsigned char *list)
{
#ifdef HAVE_QTN
	if (has_qtn(name))
		return getassoclist_qtn(name, list);
#endif

	// int ap;
	// if ((wl_ioctl(name, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap)
	// {
	int ret;
	unsigned int num;

	num = (sizeof(*list) - 4) / 6; /* Maximum number of entries in the
					 * buffer */
	memcpy(list, &num, 4); /* First 4 bytes are the number of ent. */

	ret = wl_ioctl(name, WLC_GET_ASSOCLIST, list, 8192);
	unsigned int *count = (unsigned int *)list;

	// }else
	// {
	// char buf[WLC_IOCTL_MAXLEN];
	/*
	 * wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	 * bzero(buf,WLC_IOCTL_MAXLEN); if (wl_ioctl(name, WLC_GET_BSS_INFO,
	 * bss_info, WLC_IOCTL_MAXLEN)<0)return 0; struct maclist *l = (struct
	 * maclist*)list; 
	 */
	/*
	 * sta_info_t *sta_info = (sta_info_t *) buf;
	 * bzero(buf,WLC_IOCTL_MAXLEN); if (wl_ioctl(name, WLC_GET_VAR,
	 * sta_info, WLC_IOCTL_MAXLEN)<0)return 0;
	 * 
	 * struct maclist *l = (struct maclist*)list; l->count=1;
	 * memcpy(&l->ea,&sta_info->ea,6);
	 */
	if (ret < 0)
		return -1;
	return count[0];
}
#endif

int getwdslist(char *name, unsigned char *list)
{
	int ret, num;

	num = (sizeof(*list) - 4) / 6; /* Maximum number of entries in the
					 * buffer */
	memcpy(list, &num, 4); /* First 4 bytes are the number of ent. */

	ret = wl_ioctl(name, WLC_GET_WDSLIST, list, 8192);

	return (ret);
}

#if !defined(HAVE_RT2880) && !defined(HAVE_RT61)

typedef struct {
	uint32 val;
	struct ether_addr ea;
} rssi_val_t;

int getWifiInfo(char *ifname, unsigned char *macname, int field)
{
	unsigned int noise, rssi;
	rssi_val_t rssi_get;

	switch (field) {
	case INFO_NOISE:
		wl_ioctl(ifname, WLC_GET_PHY_NOISE, &noise, sizeof(noise));
		return noise;
		break;
	case INFO_RSSI:
		if (!macname) {
			wl_ioctl(ifname, WLC_GET_RSSI, &rssi, sizeof(rssi));
			return rssi;
		} else {
			ether_atoe(macname, &rssi_get.ea);
			wl_ioctl(ifname, WLC_GET_RSSI, &rssi_get, sizeof(rssi_get));
			return rssi_get.val;
		}
		break;
	}
	return 0;
}

#endif

#else
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
#include "../madwifi.dev/madwifi.dev/net80211/ieee80211.h"
#include "../madwifi.dev/madwifi.dev/net80211/ieee80211_crypto.h"
#include "../madwifi.dev/madwifi.dev/net80211/ieee80211_ioctl.h"

static int wrqfreq_to_int(struct iwreq *wrq)
{
	int freq, i;
	freq = wrq->u.freq.m;
	if (freq < 1000) {
		return freq;
	}
	int divisor = 1000000;
	int e = wrq->u.freq.e;
	for (i = 0; i < e; i++)
		divisor /= 10;
	if (divisor)
		freq /= divisor;
	return freq;
}

/*
 * Atheros 
 */

#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int set80211priv(struct iwreq *iwr, const char *ifname, int op, void *data, size_t len)
{
#define N(a) (sizeof(a) / sizeof(a[0]))

	bzero(iwr, sizeof(struct iwreq));
	strlcpy(iwr->ifr_name, ifname, IFNAMSIZ - 1);
	if (len < IFNAMSIZ) {
		/*
		 * Argument data fits inline; put it there.
		 */
		memcpy(iwr->u.name, data, len);
	} else {
		/*
		 * Argument data too big for inline transfer; setup a
		 * parameter block instead; the kernel will transfer
		 * the data for the driver.
		 */
		iwr->u.data.pointer = data;
		iwr->u.data.length = len;
	}

	if (ioctl(getsocket(), op, iwr) < 0) {
		static const char *opnames[] = {
			IOCTL_ERR(IEEE80211_IOCTL_SETPARAM),	 IOCTL_ERR(IEEE80211_IOCTL_GETPARAM),
			IOCTL_ERR(IEEE80211_IOCTL_SETMODE),	 IOCTL_ERR(IEEE80211_IOCTL_GETMODE),
			IOCTL_ERR(IEEE80211_IOCTL_SETWMMPARAMS), IOCTL_ERR(IEEE80211_IOCTL_GETWMMPARAMS),
			IOCTL_ERR(IEEE80211_IOCTL_SETCHANLIST),	 IOCTL_ERR(IEEE80211_IOCTL_GETCHANLIST),
			IOCTL_ERR(IEEE80211_IOCTL_CHANSWITCH),	 IOCTL_ERR(IEEE80211_IOCTL_GETCHANINFO),
			IOCTL_ERR(IEEE80211_IOCTL_SETOPTIE),	 IOCTL_ERR(IEEE80211_IOCTL_GETOPTIE),
			IOCTL_ERR(IEEE80211_IOCTL_SETMLME),	 IOCTL_ERR(IEEE80211_IOCTL_SETKEY),
			IOCTL_ERR(IEEE80211_IOCTL_DELKEY),	 IOCTL_ERR(IEEE80211_IOCTL_ADDMAC),
			IOCTL_ERR(IEEE80211_IOCTL_DELMAC),
		};
		op -= SIOCIWFIRSTPRIV;
		if (0 <= op && op < N(opnames))
			perror(opnames[op]);
		else
			perror("ioctl[unknown???]");
		return -1;
	}
	return 0;
#undef N
}

int do80211priv(const char *ifname, int op, void *data, size_t len)
{
	struct iwreq iwr;

	if (set80211priv(&iwr, ifname, op, data, len) < 0)
		return -1;
	if (len < IFNAMSIZ)
		memcpy(data, iwr.u.name, len);
	return iwr.u.data.length;
}

#define KILO 1000

long long wifi_getrate(char *ifname)
{
	if (is_mac80211(ifname) && !is_mvebu(ifname)) {
		char physical[32];
		bzero(physical, sizeof(physical));
		strncpy(physical, ifname, 5);
		if (nvram_nmatch("b-only", "%s_net_mode", physical))
			return 11000 * KILO;
		if (nvram_nmatch("g-only", "%s_net_mode", physical))
			return 54000 * KILO;
		if (nvram_nmatch("a-only", "%s_net_mode", physical))
			return 54000 * KILO;
		if (nvram_nmatch("bg-mixed", "%s_net_mode", physical))
			return 54000 * KILO;
		struct wifi_interface *interface = mac80211_get_interface(ifname);
		if (!interface)
			return -1;
		long long rate;
		int sgi = has_shortgi(physical);
		int vhtmcs = -1;
		//fprintf(stderr,"sgi %d, width %d\n",sgi, interface->width);
		if (nvram_nmatch("mixed", "%s_net_mode", physical) || //
		    nvram_nmatch("ac-only", "%s_net_mode", physical) || //
		    nvram_nmatch("ax-only", "%s_net_mode", physical) || //
		    nvram_nmatch("xacn-mixed", "%s_net_mode", physical) || //
		    nvram_nmatch("1", "%s_turbo_qam", physical) || //
		    nvram_nmatch("acn-mixed", "%s_net_mode", physical)) //
			vhtmcs = mac80211_get_maxvhtmcs(physical);
		int mcs = mac80211_get_maxmcs(physical);
		int novht = 0;
		if (is_ath10k(ifname) && has_2ghz(physical))
			novht = nvram_nmatch("0", "%s_turbo_qam", physical);
		sgi = sgi ? nvram_nmatch("1", "%s_shortgi", physical) : 0;
		switch (interface->width) {
		case 2:
			rate = 54000;
			break;
		case 5:
			rate = 54000 / 4;
			break;
		case 10:
			rate = 54000 / 2;
			break;
		case 20:
		case 40:
		case 80:
		case 160:
			rate = VHTTxRate(mcs, novht ? -1 : vhtmcs, sgi, interface->width);
			break;
		case 8080:
			rate = VHTTxRate(mcs, novht ? -1 : vhtmcs, sgi, 160);
			break;
		default:
			rate = 54000;
		}
		free(interface);
		return rate * KILO;
	} else {
		struct iwreq wrq;

		(void)bzero(&wrq, sizeof(struct iwreq));
		strlcpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);
		ioctl(getsocket(), SIOCGIWRATE, &wrq);
		return wrq.u.bitrate.value;
	}
}

/*
 * For doing log10/exp10 without libm 
 */
#define LOG10_MAGIC 1.25892541179

int iw_mwatt2dbm(int in)
{
	/*
	 * Version without libm : slower 
	 */
	double fin = (double)in;
	int res = 0;

	/*
	 * Split integral and floating part to avoid accumulating rounding errors 
	 */
	while (fin > 10.0) {
		res += 10;
		fin /= 10.0;
	}
	while (fin > 1.000001) { /* Eliminate rounding errors, take ceil */
		res += 1;
		fin /= LOG10_MAGIC;
	}
	return (res);
}

static int checkid(char *ifname, int vendorid,
		   int productid) //checks if its usually a emp card (no concrete detection possible)
{
#ifdef HAVE_MVEBU
	return 0;
#endif
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "wlan%d", &devcount);
	int vendor = getValueFromPath("/proc/sys/dev/wifi%d/idvendor", devcount, "%d", NULL);
	int product = getValueFromPath("/proc/sys/dev/wifi%d/idproduct", devcount, "%d", NULL);
	if (vendor == vendorid && product == productid) //XR3.3/XR3.6/XR3.7 share the same pci id's
		return 1;
	return 0;
}

int isEMP(char *ifname) //checks if its usually a emp card (no concrete detection possible)
{
	if (checkid(ifname, 0x168c, 0x2062))
		return 1;
	if (checkid(ifname, 0x168c,
		    0x2063)) //will include more suspicius cards.
		return 1;
	return 0;
}

int isXR36(char *ifname) //checks if its usually a emp card (no concrete detection possible)
{
	return checkid(ifname, 0x0777, 0x3c03);
}

int isFXXN_PRO(char *ifname) //checks if its usualla a DBII Networks FxxN-PRO card (no correct detection possible)
{
	char readid[64];
	int devcount;

	strcpy(readid, ifname);
	sscanf(readid, "wlan%d", &devcount);
	int cvendor = getValueFromPath("/sys/class/ieee80211/phy%d/device/subsystem_vendor", devcount, "0x%x", NULL);
	int cproduct = getValueFromPath("/sys/class/ieee80211/phy%d/device/subsystem_device", devcount, "0x%x", NULL);
	if (cvendor == 0x168c && cproduct == 0x2096) { //F36N-PRO / F64N-PRO shares the same id's
		return 1;
	} else if (cvendor == 0xdb11 && cproduct == 0x0f50) { // F50N-PRO
		return 2;
	}
	return 0;
}

int isSR71E(char *ifname)
{
	char readid[64];
	int devcount;

	strcpy(readid, ifname);
	sscanf(readid, "wlan%d", &devcount);

	int cvendor = getValueFromPath("/sys/class/ieee80211/phy%d/device/subsystem_vendor", devcount, "0x%x", NULL);
	int cproduct = getValueFromPath("/sys/class/ieee80211/phy%d/device/subsystem_device", devcount, "0x%x", NULL);

	if (cvendor == 0x0777 && cproduct == 0x4e05) { // SR71-E
		return 1;
	}
	if (cvendor == 0x0777 && cproduct == 0x4082) { // SR71
		return 1;
	}
	if (cvendor == 0x168c && cproduct == 0x2082) { // SR71-A
		return 1;
	}
	if (cvendor == 0x0777 && cproduct == 0x4005) { // SR71-15
		return 1;
	}
	if (cvendor == 0x0777 && cproduct == 0x4002) { // SR71-12
		return 1;
	}
	return 0;
}

int isDL4600(char *ifname)
{
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "wlan%d", &devcount);
	int vendor = getValueFromPath("/proc/sys/dev/wifi%d/idvendor", devcount, "%d", NULL);
	int product = getValueFromPath("/proc/sys/dev/wifi%d/idproduct", devcount, "%d", NULL);
	if (vendor == 0x1C14 && product == 0x19)
		return 1;
	return 0;
}

int wifi_gettxpower(char *ifname)
{
	int poweroffset = wifi_gettxpoweroffset(ifname);
	struct iwreq wrq;

	(void)bzero(&wrq, sizeof(struct iwreq));

	strlcpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);

	ioctl(getsocket(), SIOCGIWTXPOW, &wrq);
	closesocket();
	struct iw_param *txpower = &wrq.u.txpower;

	if (txpower->disabled) {
		return 0;
	} else {
		/*
		 * Check for relative values 
		 */
		if (txpower->flags & IW_TXPOW_RELATIVE) {
			if (txpower->value < 0)
				return 0;
			return txpower->value + poweroffset;
		} else {
			int dbm = 0;

			/*
			 * Convert everything to dBm 
			 */
			if (txpower->flags & IW_TXPOW_MWATT)
				dbm = iw_mwatt2dbm(txpower->value);
			else
				dbm = txpower->value;
			if (dbm < 0)
				return 0;
			return dbm + poweroffset;
		}
	}
}

int wifi_gettxpoweroffset(char *ifname)
{
	int poweroffset = 0;

#ifdef HAVE_ALPHA
	poweroffset = 10;
#elif HAVE_EOC1650
	poweroffset = 0;
#elif HAVE_BWRG1000
	poweroffset = 12; //?? guess
#elif HAVE_EAP3660
	poweroffset = 8;
#elif HAVE_EOC2610
	poweroffset = 8;
#elif HAVE_ECB3500
	poweroffset = 8;
#elif HAVE_EOC5610
	poweroffset = 0;
#elif HAVE_NS2
	poweroffset = 10;
#elif HAVE_LC2
	poweroffset = 10;
#elif HAVE_BS2
	poweroffset = 0;
#elif HAVE_PICO2
	poweroffset = 0;
#elif HAVE_PICO2HP
	poweroffset = 10;
#elif HAVE_MS2
	poweroffset = 10;
#elif HAVE_BS2HP
	poweroffset = 10;
#elif HAVE_NS5
	poweroffset = 5;
#elif HAVE_PICO5
	poweroffset = 5;
#elif HAVE_NS3
	poweroffset = 5;
#elif HAVE_LC5
	poweroffset = 5;
#elif HAVE_DLM101
	poweroffset = 5;
#elif HAVE_BS5
	poweroffset = 5;
#elif HAVE_LS5
	poweroffset = 5;
#else
	if (isEMP(ifname)) {
		if (nvram_nmatch("2", "%s_cardtype", ifname))
			return 8;
		if (nvram_nmatch("3", "%s_cardtype", ifname))
			return 8;
		if (nvram_nmatch("5", "%s_cardtype", ifname))
			return 8;
		if (nvram_nmatch("6", "%s_cardtype", ifname))
			return 7;
		if (nvram_nmatch("7", "%s_cardtype", ifname))
			return 10;
	}

	if (isDL4600(ifname))
		return 10;

#ifdef HAVE_ATH9K
	if (isFXXN_PRO(ifname))
		return 5;
	else if (isSR71E(ifname))
		return 7;
#endif
	int devcount;
	int err;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "wlan%d", &devcount);
	poweroffset = getValueFromPath("/proc/sys/dev/wifi%d/poweroffset", devcount, "%d", &err);
	if (err || poweroffset < 0 || poweroffset > 20)
		poweroffset = 0;
#endif
	char *manpoweroffset;
	manpoweroffset = nvram_nget("%s_poweroffset", ifname);
	if (*(manpoweroffset)) {
		poweroffset = atoi(manpoweroffset);
	}
	return poweroffset;
}

int get_freqoffset(char *ifname)
{
#ifdef HAVE_NS3
	return -2000;
#endif

	if (isEMP(ifname)) {
		if (nvram_nmatch("4", "%s_cardtype", ifname))
			return -2400;
	}

	if (isDL4600(ifname))
		return -705;

#ifdef HAVE_ATH9K
	if (isFXXN_PRO(ifname) == 1) {
		if (nvram_nmatch("1", "%s_cardtype", ifname)) {
			return -1830;
		}
		if (nvram_nmatch("2", "%s_cardtype", ifname)) {
			return 720;
		}
	}
#endif

	char *var = NULL;
	if (ifname) {
		char localvar[32];
		sprintf(localvar, "%s_offset", ifname);
		var = nvram_safe_get(localvar);
	}
	if (var && *var) {
		return atoi(var);
	}
	int vendor;
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "wlan%d", &devcount);
	vendor = getValueFromPath("/proc/sys/dev/wifi%d/vendor", devcount, "%d", NULL);

	switch (vendor) {
	case 9: // ubnt xr9
		return -(2427 - 907);
		break;
	case 4: // ubnt sr9
		return -(2422 - 922);
		break;
	case 13:
		return -(5540 - 3540); // xr3 general 3,5 ghz
	case 1328:
		return -(5540 - 2840); // xr3 special 2.8 ghz
	case 1336:
		if (nvram_nmatch("2", "%s_cardtype", ifname))
			return -(5765 - 3658); // xr3 3.7 ghz
		else
			return -(5540 - 3340); // xr3 special 3.3/3.6 ghz
	case 7:
		return -(2427 - 763); // xr7
	case 14:
		return -(5540 - 4516); // xr4
		// case 24:
		// return -(5540-4540); //sr4
	case 23: // reserved for XR2.3 until spec is known
	case 26: // reserved for XR2.6 until spec is known

	default:
		return 0;
		break;
	}
	return 0;
}

int get_wififreq(char *ifname, int freq)
{
	return freq + get_freqoffset(ifname);
}

struct wifi_interface *wifi_getfreq(char *ifname)
{
	struct iwreq wrq;

	if (has_ad(ifname)) {
		struct wifi_interface *interface = (struct wifi_interface *)malloc(sizeof(struct wifi_interface));
		bzero(interface, sizeof(struct wifi_interface));
		interface->freq = atoi(nvram_safe_get("wlan2_channel"));
		interface->center1 = -1;
		interface->center2 = -1;
		return interface;
	}
	if (is_mac80211(ifname)) {
		return mac80211_get_interface(ifname);
	}

	(void)bzero(&wrq, sizeof(struct iwreq));
	strlcpy(wrq.ifr_name, ifname, IFNAMSIZ - 1);
	ioctl(getsocket(), SIOCGIWFREQ, &wrq);
	closesocket();
	struct wifi_interface *interface = (struct wifi_interface *)malloc(sizeof(struct wifi_interface));
	bzero(interface, sizeof(struct wifi_interface));
	interface->freq = wrqfreq_to_int(&wrq);
	interface->center1 = -1;
	interface->center2 = -1;
	return interface;
}

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)

int wifi_getchannel(char *ifname)
{
	struct wifi_interface *interface = wifi_getfreq(ifname);
	if (!interface)
		return 0;
	int channel = ieee80211_mhz2ieee(interface->freq);
	free(interface);
	return channel;
}
#endif

int get_radiostate(char *ifname)
{
	if (nvram_nmatch("disabled", "%s_net_mode", ifname))
		return 0;
	if (nvram_nmatch("disabled", "%s_mode", ifname))
		return 0;
	if (!has_ad(ifname)) {
		if (is_mac80211(ifname)) {
			int state = getValueFromPath("/sys/kernel/debug/ieee80211/phy%d/ath9k/diag", get_ath9k_phy_ifname(ifname),
						     "0x%x", NULL);
			if (state == 0x00000003)
				return 0;
		}
	}
	struct ifreq ifr;
	int skfd = getsocket();

	strlcpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name) - 1);
	if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
		closesocket();
		return -1;
	}
	closesocket();
	if ((ifr.ifr_flags & IFF_UP)) {
		return 1;
	}
	return 0;
}

static int iw_get_ext(int skfd, /* Socket to the kernel */
		      const char *ifname, /* Device name */
		      int request, /* WE ID */
		      struct iwreq *pwrq)
{ /* Fixed part of the
				 * request */
	/*
	 * Set device name 
	 */
	strlcpy(pwrq->ifr_name, ifname, IFNAMSIZ - 1);
	/*
	 * Do the request 
	 */
	return (ioctl(skfd, request, pwrq));
}

int isAssociated(char *ifname)
{
	struct iwreq wrq;
	int i;

	if (iw_get_ext(getsocket(), ifname, SIOCGIWAP, &wrq) >= 0) {
		for (i = 0; i < 6; i++)
			if (wrq.u.ap_addr.sa_data[i] != 0) {
				closesocket();
				return 1;
			}
	}
	closesocket();
	return 0;
}

int getAssocMAC(char *ifname, char *mac)
{
	struct iwreq wrq;
	int i;
	int ret = -1;

	if (iw_get_ext(getsocket(), ifname, SIOCGIWAP, &wrq) >= 0) {
		for (i = 0; i < 6; i++)
			if (wrq.u.ap_addr.sa_data[i] != 0)
				ret = 0;
	}
	if (!ret) {
		for (i = 0; i < 6; i++)
			mac[i] = wrq.u.ap_addr.sa_data[i];
	}
	closesocket();
	return ret;
}

#ifdef HAVE_ATH9K

static unsigned int get_ath10kreg(char *ifname, unsigned int reg)
{
	int phy = get_ath9k_phy_ifname(ifname);
	char file[64];
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_addr", phy);
	FILE *fp = fopen(file, "wb");
	if (fp == NULL)
		return 0;
	fprintf(fp, "0x%x", reg);
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

static void set_ath10kreg(char *ifname, unsigned int reg, unsigned int value)
{
	char file[64];
	int phy = get_ath9k_phy_ifname(ifname);
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_addr", phy);
	FILE *fp = fopen(file, "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "0x%x", reg);
	fclose(fp);
	sprintf(file, "/sys/kernel/debug/ieee80211/phy%d/ath10k/reg_value", phy);
	fp = fopen(file, "wb");
	if (fp == NULL)
		return;
	fprintf(fp, "0x%x", value);
	fclose(fp);
}

void radio_on_off_ath9k(int idx, int on)
{
	char debugstring[64];
	int fp;
	char secmode[16];
	char tpt[16];
	char prefix[32];
	sprintf(prefix, "wlan%d", idx);
	int needrestart = 1;
	if (on) {
		char pid[64];
		sprintf(pid, "/var/run/%s_hostapd.pid", prefix);
		if (check_pidfromfile(pid, "hostapd"))
			needrestart = 0;
		sprintf(pid, "/var/run/%s_wpa_supplicant.pid", prefix);
		if (check_pidfromfile(pid, "wpa_supplicant"))
			needrestart = 0;
		if (needrestart) {
			eval("startservice", "restarthostapd_ifneeded", "-f");
			eval("restart", "dnsmasq");
			eval("startservice", "resetleds", "-f");
			eval("startservice", "postnetwork", "-f");
		}
	} else {
		char pid[64];
		sprintf(pid, "/var/run/%s_hostapd.pid", prefix);
		FILE *file = fopen(pid, "rb");
		if (file) {
			int p;
			fscanf(file, "%d", &p);
			fclose(file);
			kill(p, SIGTERM);
			unlink(pid);
		}
		sprintf(pid, "/var/run/%s_wpa_supplicant.pid", prefix);
		file = fopen(pid, "rb");
		if (file) {
			int p;
			fscanf(file, "%d", &p);
			fclose(file);
			kill(p, SIGTERM);
			unlink(pid);
		}
	}

	if (is_ath10k(prefix)) {
		unsigned int pcu_diag_reg = 0x24048;
		if (has_wave2(prefix))
			pcu_diag_reg = 0x311b4;
		unsigned int value = get_ath10kreg(prefix, pcu_diag_reg);
		if (on)
			value &= ~((1 << 5) | (1 << 6));
		else
			value |= ((1 << 5) | (1 << 6)); // disable rx and tx
		set_ath10kreg(prefix, pcu_diag_reg, value);
	} else if (is_ath11k(prefix)) {
		/*		unsigned int pcu_diag_reg = 0x24048;
		if (has_wave2(prefix))
			pcu_diag_reg = 0x311b4;
		unsigned int value = get_ath10kreg(prefix, pcu_diag_reg);
		if (on)
			value &= ~((1 << 5) | (1 << 6));
		else
			value |= ((1 << 5) | (1 << 6));	// disable rx and tx
		set_ath10kreg(prefix, pcu_diag_reg, value);*/
	} else {
		sprintf(debugstring, "/sys/kernel/debug/ieee80211/phy%d/ath9k/diag", get_ath9k_phy_idx(idx));
		fp = open(debugstring, O_WRONLY);
		if (fp) {
			if (on)
				write(fp, "0", sizeof("0") - 1);
			else
				write(fp, "3", sizeof("3") - 1);
			fprintf(stderr, "ath9k radio %d: phy%d wlan%d\n", on, get_ath9k_phy_idx(idx), idx);
			close(fp);
		}
	}
	// LED
#ifdef HAVE_WZRHPAG300NH
	if (idx == 0)
		sprintf(debugstring, "/sys/class/leds/wireless_generic_1/trigger");
	else
		sprintf(debugstring, "/sys/class/leds/wireless_generic_21/trigger");
#else
	if (is_ath10k(prefix))
		sprintf(debugstring, "/sys/class/leds/ath10k-phy%d/trigger", get_ath9k_phy_idx(idx));
	else if (is_ath11k(prefix))
		sprintf(debugstring, "/sys/class/leds/ath11k-phy%d/trigger", get_ath9k_phy_idx(idx));
	else
		sprintf(debugstring, "/sys/class/leds/ath9k-phy%d/trigger", get_ath9k_phy_idx(idx));
#endif
	fp = open(debugstring, O_WRONLY);
	if (fp) {
		if (on) {
			sprintf(tpt, "phy%dtpt", get_ath9k_phy_idx(idx));
			write(fp, tpt, strlen(tpt));
			sprintf(secmode, "wlan%d_akm", idx);
			if (nvram_exists(secmode) && !nvram_match(secmode, "disabled")) {
				// needs refinements
				if (idx == 0)
					led_control(LED_SEC0, LED_ON);
				else if (idx == 1)
					led_control(LED_SEC1, LED_ON);
			}
		} else {
			write(fp, "none", sizeof("none") - 1);
#ifdef HAVE_WZRHPAG300NH
			if (idx == 0) {
				led_control(LED_SEC0, LED_OFF);
			} else if (idx == 1) {
				led_control(LED_SEC1, LED_OFF);
			}
#endif
		}
		close(fp);
	}
}

#endif

int has_athmask(int devnum, int mask)
{
	int err;
	int modes = getValueFromPath("/proc/sys/dev/wifi%d/wirelessmodes", devnum, "%d", &err);
	if (!err && (modes & mask) == mask)
		return 1;
	else
		return 0;
}

static struct wifi_channels *list_channelsext(const char *ifname, int allchans)
{
	struct ieee80211req_chaninfo chans;
	struct ieee80211req_chaninfo achans;
	const struct ieee80211_channel *c;
	int i;
	struct wifi_channels *list = getcache(ifname, nvram_nget("%s_regdomain", ifname));
	if (list)
		return list;

	// fprintf (stderr, "list channels for %s\n", ifname);
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
		bzero(&achans, sizeof(achans));
		for (i = 0; i < chans.ic_nchans; i++) {
			c = &chans.ic_chans[i];
			if (isset(active, c->ic_ieee) || allchans)
				achans.ic_chans[achans.ic_nchans++] = *c;
		}
	} else
		achans = chans;

	// fprintf(stderr,"channel number %d\n", achans.ic_nchans);
	list = (struct wifi_channels *)calloc(sizeof(struct wifi_channels) * (achans.ic_nchans + 1), 1);

	char wl_mode[16];
	char wl_turbo[16];

	sprintf(wl_mode, "%s_net_mode", ifname);
	sprintf(wl_turbo, "%s_channelbw", ifname);
	int l = 0;

	for (i = 0; i < achans.ic_nchans; i++) {
		// fprintf(stderr,"channel number %d of %d\n", i,achans.ic_nchans);

		// filter out A channels if mode isnt A-Only or mixed
		if (IEEE80211_IS_CHAN_A(&achans.ic_chans[i])) {
#ifdef HAVE_WHRAG108
			if (!strcmp(ifname, "wlan1"))
				continue;
#endif
#ifdef HAVE_TW6600
			if (!strcmp(ifname, "wlan1"))
				continue;
#endif
			if (nvram_invmatch(wl_mode, "a-only") && nvram_invmatch(wl_mode, "mixed"))
				continue;
		}
		// filter out B/G channels if mode isnt g-only, b-only or mixed
		if (IEEE80211_IS_CHAN_ANYG(&achans.ic_chans[i]) || IEEE80211_IS_CHAN_B(&achans.ic_chans[i])) {
#ifdef HAVE_WHRAG108
			if (!strcmp(ifname, "wlan0"))
				continue;
#endif
#ifdef HAVE_TW6600
			if (!strcmp(ifname, "wlan0"))
				continue;
#endif
			if (nvram_invmatch(wl_mode, "g-only") && nvram_invmatch(wl_mode, "mixed") &&
			    nvram_invmatch(wl_mode, "b-only") && nvram_invmatch(wl_mode, "bg-mixed"))
				continue;
		}
		// filter out channels which are not supporting turbo mode if turbo
		// is enabled
		if (!IEEE80211_IS_CHAN_STURBO(&achans.ic_chans[i]) && !IEEE80211_IS_CHAN_DTURBO(&achans.ic_chans[i])) {
			if (nvram_matchi(wl_turbo, 40))
				continue;
		}
		// filter out turbo channels if turbo mode is disabled
		/*
		 * if (IEEE80211_IS_CHAN_STURBO (&achans.ic_chans[i]) ||
		 * IEEE80211_IS_CHAN_DTURBO (&achans.ic_chans[i])) { if (nvram_match
		 * (wl_turbo, "0")) continue; }
		 */
		if (IEEE80211_IS_CHAN_STURBO(&achans.ic_chans[i])) {
			if (!nvram_matchi(wl_turbo, 40))
				continue;
		}

		list[l].channel = achans.ic_chans[i].ic_ieee;
		list[l].freq = achans.ic_chans[i].ic_freq;
		list[l].noise = -95; // achans.ic_chans[i].ic_noise;
		l++;
	}

	list[l].freq = -1;
	addcache(ifname, nvram_nget("%s_regdomain", ifname), list);
	return list;
}
void invalidate_channelcache(void)
{
	_invalidate_channelcache();
}
struct wifi_channels *list_channels(char *devnr)
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

int getWifiInfo(char *ifname, unsigned char *mac, int field)
{
	if (is_mac80211(ifname)) {
		return getWifiInfo_ath9k(ifname, mac, field);
	}
	unsigned char *buf = calloc(24 * 1024, 1);

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
	(void)bzero(&iwr, sizeof(iwr));
	(void)strlcpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name) - 1);
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

	bzero(maccmp, 6);
	do {
		struct ieee80211req_sta_info *si;

		si = (struct ieee80211req_sta_info *)cp;
		if (!memcmp(&si->isi_macaddr[0], mac, 6)) {
			close(s);

			int result = 0;

			char turbo[32];
			char *ifn = strdup(ifname);
			char *s = strchr(ifn, '.');
			if (s)
				*s = 0;
			sprintf(turbo, "%s_channelbw", ifn);
			free(ifn);
			int t;
			if (nvram_matchi(turbo, 40))
				t = 20;
			else
				t = 10;

			switch (field) {
			case INFO_RSSI:
				result = si->isi_noise + si->isi_rssi + nvram_default_geti(nb, 0);
			case INFO_NOISE:
				result = si->isi_noise + nvram_default_geti(nb, 0);
			case INFO_UPTIME:
				result = si->isi_uptime;
			case INFO_RXRATE:
				result = ((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) / 2) * t;
			case INFO_TXRATE:
				result = ((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) / 2) * t;
			default:
				result = 0;
			}
			free(buf);
			return result;
		}
		if (!memcmp(&si->isi_macaddr[0], mac, 6))
			break;
		cp += si->isi_len;
		len -= si->isi_len;
	} while (len >= sizeof(struct ieee80211req_sta_info));
	close(s);
	free(buf);
	return 0;
}

int getassoclist(char *ifname, unsigned char *list)
{
	if (is_mac80211(ifname)) {
		return getassoclist_ath9k(ifname, list);
	}
	unsigned char *buf;

	buf = calloc(24 * 1024, 1);
	unsigned char *cp;
	int len;
	struct iwreq iwr;
	int s;
	unsigned int *count = (unsigned int *)list;

	if (nvram_nmatch("disabled", "%s_net_mode", ifname)) {
		free(buf);
		return 0;
	}
	int mincount = 0;

	if (nvram_nmatch("wdssta", "%s_mode", ifname) || nvram_nmatch("sta", "%s_mode", ifname) ||
	    nvram_nmatch("wdssta_mtik", "%s_mode", ifname) || nvram_nmatch("wet", "%s_mode", ifname)) {
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
	(void)bzero(&iwr, sizeof(iwr));
	(void)strlcpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name) - 1);
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
	} while (len >= sizeof(struct ieee80211req_sta_info));
	close(s);
	free(buf);

	return mincount > count[0] ? mincount : count[0];
}

void radio_off(int idx)
{
#ifdef HAVE_ATH9K
	if (idx == -1) {
		int cc = getdevicecount();
		int i;
		for (i = 0; i < cc; i++) {
			radio_on_off_ath9k(i, 0);
		}
		led_control(LED_WLAN0, LED_OFF);
		led_control(LED_WLAN1, LED_OFF);
	} else {
		radio_on_off_ath9k(idx, 0);
		if (idx == 0)
			led_control(LED_WLAN0, LED_OFF);
		if (idx == 1)
			led_control(LED_WLAN1, LED_OFF);
	}
#endif
	if (idx != -1) {
#ifdef HAVE_MVEBU

#else
		writevaproc("1", "/proc/sys/dev/wifi%d/silent", idx);
		writevaproc("1", "/proc/sys/dev/wifi%d/ledon",
			    idx); // switch off led
#endif
		if (idx == 0)
			led_control(LED_WLAN0, LED_OFF);
		if (idx == 1)
			led_control(LED_WLAN1, LED_OFF);
	} else {
		int cc = getdevicecount();
		int i;
		for (i = 0; i < cc; i++) {
#ifdef HAVE_MVEBU

#else
			writevaproc("1", "/proc/sys/dev/wifi%d/silent", i);
			writevaproc("1", "/proc/sys/dev/wifi%d/ledon",
				    i); // switch off led
#endif
		}
		led_control(LED_WLAN0, LED_OFF);
		led_control(LED_WLAN1, LED_OFF);
	}
#ifdef HAVE_ATH9K
#endif
}

void radio_on(int idx)
{
#ifdef HAVE_ATH9K
	if (idx == -1) {
		int cc = getdevicecount();
		int i;
		for (i = 0; i < cc; i++) {
			radio_on_off_ath9k(i, 1);
		}
		led_control(LED_WLAN0, LED_ON);
		led_control(LED_WLAN1, LED_ON);
	} else {
		radio_on_off_ath9k(idx, 1);
		if (idx == 0)
			led_control(LED_WLAN0, LED_ON);
		if (idx == 1)
			led_control(LED_WLAN1, LED_ON);
	}
#endif
	if (idx != -1) {
#ifdef HAVE_MVEBU

#else
		writevaproc("0", "/proc/sys/dev/wifi%d/silent", idx);
#endif
		if (idx == 0)
			led_control(LED_WLAN0, LED_ON);
		if (idx == 1)
			led_control(LED_WLAN1, LED_ON);
	} else {
		int cc = getdevicecount();
		int i;
		for (i = 0; i < cc; i++) {
			writevaproc("0", "/proc/sys/dev/wifi%d/silent", i);
		}
		led_control(LED_WLAN0, LED_ON);
		led_control(LED_WLAN1, LED_ON);
	}
}

int gettxantenna(char *ifname)
{
	if (is_mac80211(ifname)) {
#ifdef HAVE_CARLSONWIRELESS
		if (!registered_has_cap(20))
			return (1);
#endif
		return (mac80211_get_avail_tx_antenna(ifname));
	} else
		return (7);
}

int getrxantenna(char *ifname)
{
	if (is_mac80211(ifname)) {
#ifdef HAVE_CARLSONWIRELESS
		if (!registered_has_cap(20))
			return (1);
#endif
		return (mac80211_get_avail_rx_antenna(ifname));
	} else
		return (7);
}

#endif

#if (defined(HAVE_RT2880) || defined(HAVE_RT61)) && !defined(HAVE_MT76)

int has_mimo(const char *prefix)
{
	return 1;
}

int has_ac(const char *prefix)
{
	if (!strncmp(prefix, "ba", 2) || !strncmp(prefix, "wl1", 3)) {
		FILE *fp = fopen("/sys/bus/pci/devices/0000:01:00.0/device", "rb");
		if (!fp)
			return 0;
		char id[32];
		fscanf(fp, "%s", id);
		fclose(fp);
		if (!strcmp(id, "0x7662"))
			return 1;
		if (!strcmp(id, "0x7615"))
			return 1;
	}

	if (!strncmp(prefix, "ra", 2) || !strncmp(prefix, "wl0", 3)) {
		FILE *fp = fopen("/sys/bus/pci/devices/0000:01:00.0/device", "rb");
		if (!fp)
			return 0;
		char id[32];
		fscanf(fp, "%s", id);
		fclose(fp);
		if (!strcmp(id, "0x7662"))
			return 1;
		if (!strcmp(id, "0x7615"))
			return 1;
	}

	if (!strncmp(prefix, "ba", 2) || !strncmp(prefix, "wl1", 3)) {
		FILE *fp = fopen("/sys/bus/pci/devices/0000:02:00.0/device", "rb");
		if (!fp)
			return 0;
		char id[32];
		fscanf(fp, "%s", id);
		fclose(fp);
		if (!strcmp(id, "0x7662"))
			return 1;
		if (!strcmp(id, "0x7615"))
			return 1;
	}
	return 0;
}
#else

int has_mimo(const char *prefix)
{
#ifdef HAVE_QTN
	if (!strcmp(prefix, "wl1"))
		return 1;
#endif
	char mimo[32];
	sprintf(mimo, "%s_phytypes", prefix);
	char *phy = nvram_safe_get(mimo);
	if (strchr(phy, 'n') || strchr(phy, 'h') || strchr(phy, 's') || strchr(phy, 'v'))
		return 1;
	else
		return 0;
}

#if !defined(HAVE_ATH10K) && !defined(HAVE_BRCMFMAC) && !defined(HAVE_MT76)
int has_ac(const char *prefix)
{
#ifdef HAVE_ATH9K
	return 0;
#else
#ifdef HAVE_QTN
	if (!strcmp(prefix, "wl1"))
		return 1;
#endif
	char mimo[32];
	sprintf(mimo, "%s_phytypes", prefix);
	char *phy = nvram_safe_get(mimo);
	if (strchr(phy, 'v'))
		return 1;
	else
		return 0;
#endif
}
#endif
#endif

#ifdef HAVE_QTN
int has_qtn(const char *prefix)
{
	if (!strcmp(prefix, "qtn"))
		return 1;
	if (!strncmp(prefix, "wl1", 3))
		return 1;
	return 0;
}
#endif

struct wifidevices {
	char *name; //chipset name for gui
	int flags; //see below
	unsigned short vendor;
	unsigned short device;
	unsigned short subvendor; //optional
	unsigned short subdevice;
	char *wmac; // indicates a wisoc based chipset. ahb soc name must be defined here
};
#define NONE 0x0
#define CHANNELSURVEY 0x1 // driver supports channelsurvey feature
#define QBOOST \
	0x2 // qboost is a tdma like protocol. i just added this feature to ath10k for doing some experiments. its only supported on 10.4 based firmwares (9984, ipq40xx etc)
#define TDMA 0x4 // older chipsets to not support tdma, just some sort of polling. so we need this flag
#define BEACONVAP100 0x8 // limit if vaps are configured, beacon minimum must be 100ms (chipset specific)
#define SURVEY_NOPERIOD 0x10 // survey is non periodic
#define CHWIDTH_5_10_MHZ 0x20
#define CHWIDTH_25_MHZ 0x40
#define QAM256 0x80
#define FWSWITCH 0x100
#define SPECTRAL 0x200
#define WAVE2 0x400
#define CUSTOMFW 0x800
#define DUALBAND 0x1000
#define AUTOBURST 0x2000
#define QAM256BUG 0x4000

#define PCI_ANY 0
#ifdef HAVE_ATH5K
#define CHANNELSURVEY5K 0x1
#else
#define CHANNELSURVEY5K 0
#endif
static struct wifidevices wdevices[] = {
	{ "Ubiquiti XR5", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3005,
	  NULL }, //UBNT XR5 offset 10
	{ "Ubiquiti XR5", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x7777, 0x3005,
	  NULL }, //UBNT XR5 offset 10
	{ "Ubiquiti XR4", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3004, NULL }, //UBNT XR4
	{ "Ubiquiti SRX", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3006,
	  NULL }, //UBNT XR7 offset 10
	{ "Ubiquiti XR7", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3007,
	  NULL }, //UBNT XR7 offset 10
	{ "Ubiquiti XR2-2.3", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3b02,
	  NULL }, //UBNT XR2.3
	{ "Ubiquiti XR2-2.6", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3c02,
	  NULL }, //UBNT XR2.6 offset 10
	{ "Ubiquiti XR2", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3002, NULL }, //UBNT XR2
	{ "Ubiquiti XR2", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x7777, 0x3002, NULL }, //UBNT XR2
	{ "Ubiquiti XR3", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3003,
	  NULL }, //UBNT XR3 offset 10
	{ "Ubiquiti XR3-3.6", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3c03,
	  NULL }, //UBNT XR3-3.6 offset 10
	{ "Ubiquiti XR3-2.8", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3b03,
	  NULL }, //UBNT XR3-2.8
	{ "Ubiquiti XR9", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x3009,
	  NULL }, //UBNT XR9 offset 10
	{ "Ubiquiti UB5", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x0777, 0x1107, NULL }, //UBNT UB5
	{ "Ubiquiti SR71A", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0027, 0x0777, 0x4082,
	  NULL }, //UBNT SR71A offset 10
	{ "Ubiquiti SR71", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0027, 0x0777, 0x2082,
	  NULL }, //UBNT SR71 offset 10
	{ "Ubiquiti SR71-E", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x0777, 0x4e05,
	  NULL }, //UBNT SR71-E offset 10
	{ "Ubiquiti SR71-15", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x0777, 0x4005,
	  NULL }, //UBNT SR71-15 offset 10
	{ "Ubiquiti SR71-12", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x0777, 0x4002,
	  NULL }, //UBNT SR71-12 offset 10
	{ "Ubiquiti SR9", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x0777, 0x2009, NULL }, //UBNT SR9
	{ "Ubiquiti SR9", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x7777, 0x2009,
	  NULL }, //UBNT SR9 offset 12
	{ "Ubiquiti SR4", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x7777, 0x2004,
	  NULL }, //UBNT SR4 offset 6
	{ "Ubiquiti SR4", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x0777, 0x2004,
	  NULL }, //UBNT SR4 offset 6
	{ "Ubiquiti SR4C", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x0777, 0x1004,
	  NULL }, //UBNT SR4C offset 6
	{ "Ubiquiti SR4C", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x7777, 0x1004,
	  NULL }, //UBNT SR4C offset 6
	{ "Ubiquiti SRC", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x168c, 0x1042,
	  NULL }, //UBNT SRC offset 1
	{ "Ubiquiti SR2", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x168c, 0x2041,
	  NULL }, //UBNT SR2 offset 10
	{ "Ubiquiti SR5", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x168c, 0x2042,
	  NULL }, //UBNT SR5 offset 7
	{ "Ubiquiti SR2", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, 0x168c, 0x2051, NULL }, //UBNT SR2
	{ "Ubiquiti SR71-X", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x0777, 0x4f05, NULL }, //UBNT SR2
	{ "Senao NMP8601 AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x168c, 0x2063, NULL }, //NMP
	{ "Alfa Networks AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x17f9, 0x000d,
	  NULL }, //alfa
	{ "Seano NMP8601 AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x168c, 0x2062, NULL }, //NMP
	{ "Gigabyte / AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x1458, 0xe901,
	  NULL }, //Gigabyte
	{ "Alfa Networks AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x168d, 0x1031,
	  NULL }, //Alfa
	{ "Alfa Networks AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x168d, 0x10a2,
	  NULL }, //Alfa
	{ "DoodleLabs DB-F15-PRO", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0xdb11, 0xf50,
	  NULL }, //dbii F50-pro-i
	{ "DoodleLabs DL4600", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x1c14, 0x19, NULL }, //dl4600
	{ "Mikrotik R52nM", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x198c, 0x4201, NULL },
	{ "Mikrotik R52nM", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x19b6, 0x5201, NULL },
	{ "Mikrotik R5H", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x19b6, 0x2201, NULL },
	{ "Mikrotik R5H", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x19b6, 0x2203, NULL },
	{ "Mikrotik R11e-5HnD", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0033, 0x19b6, 0xd014,
	  NULL },
	{ "Mikrotik R11e-5HnDr2", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0033, 0x19b6, 0xd057,
	  NULL },
	{ "Mikrotik R11e-2HPnD", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0033, 0x19b6, 0xd016,
	  NULL },
	{ "Mikrotik R11e-5HacD", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x003c, 0x19b6, 0xd075,
	  NULL },
	{ "Mikrotik R11e-5HacT", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x003c, 0x19b6, 0xd03c,
	  NULL },
	{ "Wistron DCMA-82", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, 0x185f, 0x1600, NULL },
	{ "Wistron DNMA-92", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, 0x168c, 0x2096, NULL },
	{ "AR5210 802.11a", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0007, PCI_ANY, PCI_ANY, NULL },
	{ "AR5210 802.11a", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0207, PCI_ANY, PCI_ANY, NULL },
	{ "AR5211 802.11a", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0011, PCI_ANY, PCI_ANY, NULL },
	{ "AR5211 802.11ab", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0012, PCI_ANY, PCI_ANY, NULL },
	{ "AR5212", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0013, PCI_ANY, PCI_ANY, NULL },
	{ "AR5212", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x1014, PCI_ANY, PCI_ANY, NULL },
	{ "AR2413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001a, PCI_ANY, PCI_ANY, NULL },
	{ "AR5413", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001b, PCI_ANY, PCI_ANY, NULL },
	{ "AR542x", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001c, PCI_ANY, PCI_ANY, NULL },
	{ "AR2417", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x001d, PCI_ANY, PCI_ANY, NULL },
	{ "AR5513", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0020, PCI_ANY, PCI_ANY, NULL },
	{ "AR5416 802.11n", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0023, PCI_ANY, PCI_ANY, NULL },
	{ "AR5418 802.11n", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0024, PCI_ANY, PCI_ANY, NULL },
	{ "AR9160 802.11n", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0027, PCI_ANY, PCI_ANY, NULL },
	{ "AR922X 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0029, PCI_ANY, PCI_ANY, NULL },
	{ "AR928X 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x002a, PCI_ANY, PCI_ANY, NULL },
	{ "AR9285 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x002b, PCI_ANY, PCI_ANY, NULL },
	{ "AR2427 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x002c, PCI_ANY, PCI_ANY, NULL },
	{ "AR9227 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x002d, PCI_ANY, PCI_ANY, NULL },
	{ "AR9287 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x002e, PCI_ANY, PCI_ANY, NULL },
	{ "AR93xx 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0030, PCI_ANY, PCI_ANY, NULL },
	{ "AR9485 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0032, PCI_ANY, PCI_ANY, NULL },
	{ "AR958x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0033, PCI_ANY, PCI_ANY, NULL },
	{ "AR9462 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0034, PCI_ANY, PCI_ANY, NULL },
	{ "QCA9565 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0036, PCI_ANY, PCI_ANY, NULL },
	{ "AR9485 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x0037, PCI_ANY, PCI_ANY, NULL },
	{ "AR5002X", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0x9013, PCI_ANY, PCI_ANY, NULL },
	{ "AR5006X", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0xff19, PCI_ANY, PCI_ANY, NULL },
	{ "AR2425", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0xff1b, PCI_ANY, PCI_ANY, NULL },
	{ "AR5008", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0xff1c, PCI_ANY, PCI_ANY, NULL },
	{ "AR922x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c, 0xff1d, PCI_ANY, PCI_ANY, NULL },
	{ "QCA988x 802.11ac", SPECTRAL | FWSWITCH | QAM256 | QAM256BUG | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c,
	  0x003c, PCI_ANY, PCI_ANY, NULL },
	{ "QCA6174 802.11ac", SPECTRAL | QAM256 | CHANNELSURVEY, 0x168c, 0x003e, PCI_ANY, PCI_ANY, NULL },
	{ "QCA99X0 802.11ac", WAVE2 | SPECTRAL | FWSWITCH | QAM256 | QAM256BUG | CHANNELSURVEY | CHWIDTH_5_10_MHZ | QBOOST | TDMA,
	  0x168c, 0x0040, PCI_ANY, PCI_ANY, NULL },
	{ "QCA6164 802.11ac", SPECTRAL | QAM256 | CHANNELSURVEY, 0x168c, 0x0041, PCI_ANY, PCI_ANY, NULL },
	{ "QCA9377 802.11ac", SPECTRAL | QAM256 | CHANNELSURVEY, 0x168c, 0x0042, PCI_ANY, PCI_ANY, NULL },
	{ "QCA9984 802.11ac",
	  DUALBAND | WAVE2 | SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ | QBOOST | TDMA | BEACONVAP100, 0x168c,
	  0x0046, PCI_ANY, PCI_ANY, NULL },
	{ "QCA9887 802.11ac", SPECTRAL | FWSWITCH | QAM256 | QAM256BUG | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, 0x168c,
	  0x0050, PCI_ANY, PCI_ANY, NULL },
	{ "QCA9888 802.11ac",
	  WAVE2 | SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ | QBOOST | TDMA | BEACONVAP100, 0x168c, 0x0056,
	  PCI_ANY, PCI_ANY, NULL },
	{ "QCN9074 802.11ax", WAVE2 | SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | BEACONVAP100, 0x17cb, 0x1104, PCI_ANY, PCI_ANY,
	  NULL },
	{ "QCN9224 802.11ax", WAVE2 | SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | BEACONVAP100, 0x17cb, 0x1109, PCI_ANY, PCI_ANY,
	  NULL },
	{ "WCN6855 802.11ax", WAVE2 | SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | BEACONVAP100, 0x17cb, 0x1103, PCI_ANY, PCI_ANY,
	  NULL },
	{ "QCA6390 802.11ax", WAVE2 | SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | BEACONVAP100, 0x17cb, 0x1101, PCI_ANY, PCI_ANY,
	  NULL },
	{ "Ubiquiti QCA9888 802.11ac", SPECTRAL | FWSWITCH | QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x0777, 0x11ac, PCI_ANY,
	  PCI_ANY, NULL },
	{ "MWL88W8964 802.11ac", QAM256 | CHANNELSURVEY, 0x11ab, 0x2b40, PCI_ANY, PCI_ANY, NULL },
	{ "MWL88W8864 802.11ac", QAM256 | CHANNELSURVEY, 0x11ab, 0x2a55, PCI_ANY, PCI_ANY, NULL },
	{ "MWL88W8897 802.11ac", QAM256 | CHANNELSURVEY, 0x11ab, 0x2b38, PCI_ANY, PCI_ANY, NULL },
	{ "WIL6210 802.11ad", NONE, 0x1ae9, 0x0310, PCI_ANY, PCI_ANY, NULL },
	{ "MWLSD8887 802.11ac", CHANNELSURVEY, 0x02df, 0x9135, PCI_ANY, PCI_ANY, NULL },
	{ "MT7615E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7615, PCI_ANY, PCI_ANY, NULL },
	{ "MT7663 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7663, PCI_ANY, PCI_ANY, NULL },
	{ "MT7611 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7611, PCI_ANY, PCI_ANY, NULL },
	{ "MT7915E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7915, PCI_ANY, PCI_ANY, NULL },
	{ "MT7916E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7916, PCI_ANY, PCI_ANY, NULL },
	{ "MT7921E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7961, PCI_ANY, PCI_ANY, NULL },
	{ "MT7922E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x7922, PCI_ANY, PCI_ANY, NULL },
	{ "MT7921E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x0608, PCI_ANY, PCI_ANY, NULL },
	{ "MT7922E 802.11ac", QAM256 | CHANNELSURVEY | CHWIDTH_5_10_MHZ, 0x14c3, 0x0616, PCI_ANY, PCI_ANY, NULL },
	{ "MT7610E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7610, PCI_ANY, PCI_ANY, NULL },
	{ "MT7630E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7630, PCI_ANY, PCI_ANY, NULL },
	{ "MT7650E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7650, PCI_ANY, PCI_ANY, NULL },
	{ "MT7662E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7662, PCI_ANY, PCI_ANY, NULL },
	{ "MT7612E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7612, PCI_ANY, PCI_ANY, NULL },
	{ "MT7602E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7602, PCI_ANY, PCI_ANY, NULL },
	{ "MT7603E 802.11ac", QAM256 | CHANNELSURVEY, 0x14c3, 0x7603, PCI_ANY, PCI_ANY, NULL },
	{ "MT7620 802.11n", CHANNELSURVEY, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY, "rt2880_wmac" },
	{ "RT3091 802.11n", CHANNELSURVEY, 0x1814, 0x3091, PCI_ANY, PCI_ANY, NULL },
	{ "BCM4350 802.11ac", NONE, 0x14e4, 0x4350, PCI_ANY, PCI_ANY, NULL },
	{ "BCM4356 802.11ac", NONE, 0x14e4, 0x4356, PCI_ANY, PCI_ANY, NULL },
	{ "BCM4358 802.11ac", NONE, 0x14e4, 0x4358, PCI_ANY, PCI_ANY, NULL },
	{ "BCM4365 802.11ac", QAM256 | CHANNELSURVEY, 0x14e4, 0x4365, PCI_ANY, PCI_ANY, NULL },
	{ "BCM4366 802.11ac", QAM256 | CHANNELSURVEY, 0x14e4, 0x4366, PCI_ANY, PCI_ANY, NULL },
	{ "BCM4371 802.11ac", NONE, 0x14e4, 0x4371, PCI_ANY, PCI_ANY, NULL },
	{ "BCM43570 802.11ac", NONE, 0x14e4, 43570, PCI_ANY, PCI_ANY, NULL },
	{ "BCM43566 802.11ac", NONE, 0x14e4, 43566, PCI_ANY, PCI_ANY, NULL },
	{ "BCM43567 802.11ac", NONE, 0x14e4, 43567, PCI_ANY, PCI_ANY, NULL },
	{ "BCM43569 802.11ac", NONE, 0x14e4, 43569, PCI_ANY, PCI_ANY, NULL },
	{ "BCM43602 802.11ac", QAM256 | SURVEY_NOPERIOD, 0x14e4, 43602, PCI_ANY, PCI_ANY, NULL },
	{ "BCM43664 802.11ac", QAM256 | NONE, 0x14e4, 43664, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8723de 802.11n", NONE, 0x10ec, 0xd723, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8822ce 802.11ac", NONE, 0x10ec, 0xc822, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8822be 802.11ac", NONE, 0x10ec, 0xb822, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8192DE 802.11n", NONE, 0x10ec, 0x8153, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8192DE 802.11n", NONE, 0x10ec, 0x002b, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8188E 802.11n", NONE, 0x10ec, 0x8179, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8192C 802.11n", NONE, 0x10ec, 0x8191, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8188C 802.11n", NONE, 0x10ec, 0x8178, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8188C 802.11n", NONE, 0x10ec, 0x8177, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8188C 802.11n", NONE, 0x10ec, 0x8176, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8192EE 802.11n", NONE, 0x10ec, 0x818b, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8192S 802.11n", NONE, 0x10ec, 0x8192, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8191S 802.11n", NONE, 0x10ec, 0x8171, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8191S 802.11n", NONE, 0x10ec, 0x8172, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8191S 802.11n", NONE, 0x10ec, 0x8173, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8191S 802.11n", NONE, 0x10ec, 0x8174, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8723E 802.11n", NONE, 0x10ec, 0x8723, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8723BE 802.11n", NONE, 0x10ec, 0xB723, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8821ae 802.11ac", NONE, 0x10ec, 0x8812, PCI_ANY, PCI_ANY, NULL },
	{ "RTL8821ae 802.11ac", NONE, 0x10ec, 0x8821, PCI_ANY, PCI_ANY, NULL },
#ifdef HAVE_IWLWIFI
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2701, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2702, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2711, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2712, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2721, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2722, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2731, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2732, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2741, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x103c, 0x2741, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2742, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2751, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2752, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2753, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2754, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2761, NULL },
	{ "IWL2200", NONE, 0x8086, 0x1043, 0x8086, 0x2762, NULL },
	{ "IWL2200", NONE, 0x8086, 0x104f, PCI_ANY, PCI_ANY, NULL },
	{ "IWL2200", NONE, 0x8086, 0x4020, PCI_ANY, PCI_ANY, NULL },
	{ "IWL2200", NONE, 0x8086, 0x4021, PCI_ANY, PCI_ANY, NULL },
	{ "IWL2200", NONE, 0x8086, 0x4023, PCI_ANY, PCI_ANY, NULL },
	{ "IWL2200", NONE, 0x8086, 0x4024, PCI_ANY, PCI_ANY, NULL },

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2520, NULL }, /* IN 2100A mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2521, NULL }, /* IN 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2524, NULL }, /* IN 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2525, NULL }, /* IN 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2526, NULL }, /* IN 2100A mPCI Gen A3 */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2522, NULL }, /* IN 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2523, NULL }, /* IN 2100 mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2527, NULL }, /* IN 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2528, NULL }, /* IN 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2529, NULL }, /* IN 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x252B, NULL }, /* IN 2100 mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x252C, NULL }, /* IN 2100 mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x252D, NULL }, /* IN 2100 mPCI 3A */

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2550, NULL }, /* IB 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2551, NULL }, /* IB 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2553, NULL }, /* IB 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2554, NULL }, /* IB 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2555, NULL }, /* IB 2100 mPCI 3B */

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2560, NULL }, /* DE 2100A mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2562, NULL }, /* DE 2100A mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2563, NULL }, /* DE 2100A mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2561, NULL }, /* DE 2100 mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2565, NULL }, /* DE 2100 mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2566, NULL }, /* DE 2100 mPCI 3A */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2567, NULL }, /* DE 2100 mPCI 3A */

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2570, NULL }, /* GA 2100 mPCI 3B */

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2580, NULL }, /* TO 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2582, NULL }, /* TO 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2583, NULL }, /* TO 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2581, NULL }, /* TO 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2585, NULL }, /* TO 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2586, NULL }, /* TO 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2587, NULL }, /* TO 2100 mPCI 3B */

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2590, NULL }, /* SO 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2592, NULL }, /* SO 2100A mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2591, NULL }, /* SO 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2593, NULL }, /* SO 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2596, NULL }, /* SO 2100 mPCI 3B */
	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x2598, NULL }, /* SO 2100 mPCI 3B */

	{ "IWL2100", NONE, 0x8086, 0x1043, 0x8086, 0x25A0, NULL }, /* HP 2100 mPCI 3B */

	{ "IWL3945", NONE, 0x8086, 0x4222, PCI_ANY, PCI_ANY, NULL },
	{ "IWL3945", NONE, 0x8086, 0x4227, PCI_ANY, PCI_ANY, NULL },

	{ "IWL4965", NONE, 0x8086, 0x4229, PCI_ANY, PCI_ANY, NULL },
	{ "IWL4965", NONE, 0x8086, 0x4230, PCI_ANY, PCI_ANY, NULL },

	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1201, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1301, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1204, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1304, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1205, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1305, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1206, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1306, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1221, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1321, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1224, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1324, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1225, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1325, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1226, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4232, PCI_ANY, 0x1326, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1211, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1311, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1214, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1314, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1215, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1315, NULL }, /* Half Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1216, NULL }, /* Mini Card */
	{ "IWL5100", NONE, 0x8086, 0x4237, PCI_ANY, 0x1316, NULL }, /* Half Mini Card */

	/* 5300 Series WiFi */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1021, NULL }, /* Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1121, NULL }, /* Half Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1024, NULL }, /* Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1124, NULL }, /* Half Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1001, NULL }, /* Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1101, NULL }, /* Half Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1004, NULL }, /* Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4235, PCI_ANY, 0x1104, NULL }, /* Half Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4236, PCI_ANY, 0x1011, NULL }, /* Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4236, PCI_ANY, 0x1111, NULL }, /* Half Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4236, PCI_ANY, 0x1014, NULL }, /* Mini Card */
	{ "IWL5300", NONE, 0x8086, 0x4236, PCI_ANY, 0x1114, NULL }, /* Half Mini Card */

	/* 5350 Series WiFi/WiMax */
	{ "IWL5350", NONE, 0x8086, 0x423A, PCI_ANY, 0x1001, NULL }, /* Mini Card */
	{ "IWL5350", NONE, 0x8086, 0x423A, PCI_ANY, 0x1021, NULL }, /* Mini Card */
	{ "IWL5350", NONE, 0x8086, 0x423B, PCI_ANY, 0x1011, NULL }, /* Mini Card */

	/* 5150 Series Wifi/WiMax */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1201, NULL }, /* Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1301, NULL }, /* Half Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1206, NULL }, /* Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1306, NULL }, /* Half Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1221, NULL }, /* Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1321, NULL }, /* Half Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423C, PCI_ANY, 0x1326, NULL }, /* Half Mini Card */

	{ "IWL5150", NONE, 0x8086, 0x423D, PCI_ANY, 0x1211, NULL }, /* Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423D, PCI_ANY, 0x1311, NULL }, /* Half Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423D, PCI_ANY, 0x1216, NULL }, /* Mini Card */
	{ "IWL5150", NONE, 0x8086, 0x423D, PCI_ANY, 0x1316, NULL }, /* Half Mini Card */

	/* 6x00 Series */
	{ "IWL6000", NONE, 0x8086, 0x422B, PCI_ANY, 0x1101, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422B, PCI_ANY, 0x1108, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422B, PCI_ANY, 0x1121, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422B, PCI_ANY, 0x1128, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422C, PCI_ANY, 0x1301, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422C, PCI_ANY, 0x1306, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422C, PCI_ANY, 0x1307, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422C, PCI_ANY, 0x1321, NULL },
	{ "IWL6000", NONE, 0x8086, 0x422C, PCI_ANY, 0x1326, NULL },
	{ "IWL6000", NONE, 0x8086, 0x4238, PCI_ANY, 0x1111, NULL },
	{ "IWL6000", NONE, 0x8086, 0x4238, PCI_ANY, 0x1118, NULL },
	{ "IWL6000", NONE, 0x8086, 0x4239, PCI_ANY, 0x1311, NULL },
	{ "IWL6000", NONE, 0x8086, 0x4239, PCI_ANY, 0x1316, NULL },

	/* 6x05 Series */
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1301, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1306, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1307, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1308, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1321, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1326, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1328, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0085, PCI_ANY, 0x1311, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0085, PCI_ANY, 0x1318, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0085, PCI_ANY, 0x1316, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0xC020, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0085, PCI_ANY, 0xC220, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0085, PCI_ANY, 0xC228, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x4820, NULL },
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1304, NULL }, /* low 5GHz active */
	{ "IWL6005", NONE, 0x8086, 0x0082, PCI_ANY, 0x1305, NULL }, /* high 5GHz active */

	/* 6x30 Series */
	{ "IWL1030", NONE, 0x8086, 0x008A, PCI_ANY, 0x5305, NULL },
	{ "IWL1030", NONE, 0x8086, 0x008A, PCI_ANY, 0x5307, NULL },
	{ "IWL1030", NONE, 0x8086, 0x008A, PCI_ANY, 0x5325, NULL },
	{ "IWL1030", NONE, 0x8086, 0x008A, PCI_ANY, 0x5327, NULL },
	{ "IWL1030", NONE, 0x8086, 0x008B, PCI_ANY, 0x5315, NULL },
	{ "IWL1030", NONE, 0x8086, 0x008B, PCI_ANY, 0x5317, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0090, PCI_ANY, 0x5211, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0090, PCI_ANY, 0x5215, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0090, PCI_ANY, 0x5216, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5201, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5205, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5206, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5207, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5221, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5225, NULL },
	{ "IWL6030", NONE, 0x8086, 0x0091, PCI_ANY, 0x5226, NULL },

	/* 6x50 WiFi/WiMax Series */
	{ "IWL6050", NONE, 0x8086, 0x0087, PCI_ANY, 0x1301, NULL },
	{ "IWL6050", NONE, 0x8086, 0x0087, PCI_ANY, 0x1306, NULL },
	{ "IWL6050", NONE, 0x8086, 0x0087, PCI_ANY, 0x1321, NULL },
	{ "IWL6050", NONE, 0x8086, 0x0087, PCI_ANY, 0x1326, NULL },
	{ "IWL6050", NONE, 0x8086, 0x0089, PCI_ANY, 0x1311, NULL },
	{ "IWL6050", NONE, 0x8086, 0x0089, PCI_ANY, 0x1316, NULL },

	/* 6150 WiFi/WiMax Series */
	{ "IWL6150", NONE, 0x8086, 0x0885, PCI_ANY, 0x1305, NULL },
	{ "IWL6150", NONE, 0x8086, 0x0885, PCI_ANY, 0x1307, NULL },
	{ "IWL6150", NONE, 0x8086, 0x0885, PCI_ANY, 0x1325, NULL },
	{ "IWL6150", NONE, 0x8086, 0x0885, PCI_ANY, 0x1327, NULL },
	{ "IWL6150", NONE, 0x8086, 0x0886, PCI_ANY, 0x1315, NULL },
	{ "IWL6150", NONE, 0x8086, 0x0886, PCI_ANY, 0x1317, NULL },

	/* 1000 Series WiFi */
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1205, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1305, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1225, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1325, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0084, PCI_ANY, 0x1215, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0084, PCI_ANY, 0x1315, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1206, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1306, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1226, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0083, PCI_ANY, 0x1326, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0084, PCI_ANY, 0x1216, NULL },
	{ "IWL1000", NONE, 0x8086, 0x0084, PCI_ANY, 0x1316, NULL },

	/* 100 Series WiFi */
	{ "IWL100", NONE, 0x8086, 0x08AE, PCI_ANY, 0x1005, NULL },
	{ "IWL100", NONE, 0x8086, 0x08AE, PCI_ANY, 0x1007, NULL },
	{ "IWL100", NONE, 0x8086, 0x08AF, PCI_ANY, 0x1015, NULL },
	{ "IWL100", NONE, 0x8086, 0x08AF, PCI_ANY, 0x1017, NULL },
	{ "IWL100", NONE, 0x8086, 0x08AE, PCI_ANY, 0x1025, NULL },
	{ "IWL100", NONE, 0x8086, 0x08AE, PCI_ANY, 0x1027, NULL },

	/* 130 Series WiFi */
	{ "IWL130", NONE, 0x8086, 0x0896, PCI_ANY, 0x5005, NULL },
	{ "IWL130", NONE, 0x8086, 0x0896, PCI_ANY, 0x5007, NULL },
	{ "IWL130", NONE, 0x8086, 0x0897, PCI_ANY, 0x5015, NULL },
	{ "IWL130", NONE, 0x8086, 0x0897, PCI_ANY, 0x5017, NULL },
	{ "IWL130", NONE, 0x8086, 0x0896, PCI_ANY, 0x5025, NULL },
	{ "IWL130", NONE, 0x8086, 0x0896, PCI_ANY, 0x5027, NULL },

	/* 2x00 Series */
	{ "IWL2000", NONE, 0x8086, 0x0890, PCI_ANY, 0x4022, NULL },
	{ "IWL2000", NONE, 0x8086, 0x0891, PCI_ANY, 0x4222, NULL },
	{ "IWL2000", NONE, 0x8086, 0x0890, PCI_ANY, 0x4422, NULL },
	{ "IWL2000", NONE, 0x8086, 0x0890, PCI_ANY, 0x4822, NULL },

	/* 2x30 Series */
	{ "IWL2030", NONE, 0x8086, 0x0887, PCI_ANY, 0x4062, NULL },
	{ "IWL2030", NONE, 0x8086, 0x0888, PCI_ANY, 0x4262, NULL },
	{ "IWL2030", NONE, 0x8086, 0x0887, PCI_ANY, 0x4462, NULL },

	/* 6x35 Series */
	{ "IWL6035", NONE, 0x8086, 0x088E, PCI_ANY, 0x4060, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088E, PCI_ANY, 0x406A, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088F, PCI_ANY, 0x4260, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088F, PCI_ANY, 0x426A, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088E, PCI_ANY, 0x4460, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088E, PCI_ANY, 0x446A, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088E, PCI_ANY, 0x4860, NULL },
	{ "IWL6035", NONE, 0x8086, 0x088F, PCI_ANY, 0x5260, NULL },

	/* 105 Series */
	{ "IWL105", NONE, 0x8086, 0x0894, PCI_ANY, 0x0022, NULL },
	{ "IWL105", NONE, 0x8086, 0x0895, PCI_ANY, 0x0222, NULL },
	{ "IWL105", NONE, 0x8086, 0x0894, PCI_ANY, 0x0422, NULL },
	{ "IWL105", NONE, 0x8086, 0x0894, PCI_ANY, 0x0822, NULL },

	/* 135 Series */
	{ "IWL135", NONE, 0x8086, 0x0892, PCI_ANY, 0x0062, NULL },
	{ "IWL135", NONE, 0x8086, 0x0893, PCI_ANY, 0x0262, NULL },
	{ "IWL135", NONE, 0x8086, 0x0892, PCI_ANY, 0x0462, NULL },
	/* 7260 Series */
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4070, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4072, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4170, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4C60, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4C70, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4060, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x406A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4160, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4062, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4162, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4270, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4272, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4260, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x426A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4262, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4470, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4472, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4460, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x446A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4462, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4870, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x486E, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4A70, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4A6E, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4A6C, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4570, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4560, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4370, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4360, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x5070, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x5072, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x5170, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x5770, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4020, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x402A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0x4220, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0x4420, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC070, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC072, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC170, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC060, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC06A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC160, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC062, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC162, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC770, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC760, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC270, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xCC70, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xCC60, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC272, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC260, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC26A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC262, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC470, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC472, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC460, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC462, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC570, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC560, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC370, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC360, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC020, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC02A, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B2, PCI_ANY, 0xC220, NULL },
	{ "IWL7260", NONE, 0x8086, 0x08B1, PCI_ANY, 0xC420, NULL },

	/* 3160 Series */
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0070, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0072, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0170, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0172, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0060, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0062, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B4, PCI_ANY, 0x0270, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B4, PCI_ANY, 0x0272, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0470, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x0472, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B4, PCI_ANY, 0x0370, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8070, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8072, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8170, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8172, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8060, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8062, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B4, PCI_ANY, 0x8270, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B4, PCI_ANY, 0x8370, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B4, PCI_ANY, 0x8272, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8470, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x8570, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x1070, NULL },
	{ "IWL3160", NONE, 0x8086, 0x08B3, PCI_ANY, 0x1170, NULL },

	/* 3165 Series */
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x4010, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x4012, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3166, PCI_ANY, 0x4212, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x4410, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x4510, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x4110, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3166, PCI_ANY, 0x4310, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3166, PCI_ANY, 0x4210, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x8010, NULL },
	{ "IWL3165", NONE, 0x8086, 0x3165, PCI_ANY, 0x8110, NULL },

	/* 3168 Series */
	{ "IWL3168", NONE, 0x8086, 0x24FB, PCI_ANY, 0x2010, NULL },
	{ "IWL3168", NONE, 0x8086, 0x24FB, PCI_ANY, 0x2110, NULL },
	{ "IWL3168", NONE, 0x8086, 0x24FB, PCI_ANY, 0x2050, NULL },
	{ "IWL3168", NONE, 0x8086, 0x24FB, PCI_ANY, 0x2150, NULL },
	{ "IWL3168", NONE, 0x8086, 0x24FB, PCI_ANY, 0x0000, NULL },

	/* 7265 Series */
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5010, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5110, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5100, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5310, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5302, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5210, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5C10, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5012, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5412, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5410, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5510, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5400, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x1010, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5000, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x500A, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5200, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5002, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5102, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5202, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9010, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9012, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x900A, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9110, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9112, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x9210, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x9200, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9510, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x9310, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9410, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5020, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x502A, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5420, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5090, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5190, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5590, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5290, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5490, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x5F10, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x5212, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095B, PCI_ANY, 0x520A, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9000, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9400, NULL },
	{ "IWL7265", NONE, 0x8086, 0x095A, PCI_ANY, 0x9E10, NULL },

	/* 8000 Series */
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0010, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1010, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x10B0, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0130, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1130, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0132, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1132, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0110, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x01F0, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0012, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1012, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1110, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0050, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0250, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1050, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0150, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x1150, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F4, PCI_ANY, 0x0030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F4, PCI_ANY, 0x1030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xC010, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xC110, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xD010, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xC050, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xD050, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xD0B0, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0xB0B0, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x8010, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x8110, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x9010, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x9110, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F4, PCI_ANY, 0x8030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F4, PCI_ANY, 0x9030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F4, PCI_ANY, 0xC030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F4, PCI_ANY, 0xD030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x8130, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x9130, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x8132, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x9132, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x8050, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x8150, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x9050, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x9150, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0004, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0044, NULL },
	{ "IWL4165", NONE, 0x8086, 0x24F5, PCI_ANY, 0x0010, NULL },
	{ "IWL4165", NONE, 0x8086, 0x24F6, PCI_ANY, 0x0030, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0810, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0910, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0850, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0950, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0930, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24F3, PCI_ANY, 0x0000, NULL },
	{ "IWL8260", NONE, 0x8086, 0x24F3, PCI_ANY, 0x4010, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0010, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0110, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x1110, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x1130, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0130, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x1010, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x10D0, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0050, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0150, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x9010, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x8110, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x8050, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x8010, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0810, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x9110, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x8130, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0910, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0930, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0950, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0850, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x1014, NULL },
	{ "IWL8275", NONE, 0x8086, 0x24FD, PCI_ANY, 0x3E02, NULL },
	{ "IWL8275", NONE, 0x8086, 0x24FD, PCI_ANY, 0x3E01, NULL },
	{ "IWL8275", NONE, 0x8086, 0x24FD, PCI_ANY, 0x1012, NULL },
	{ "IWL8275", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0012, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x0014, NULL },
	{ "IWL8265", NONE, 0x8086, 0x24FD, PCI_ANY, 0x9074, NULL },

	/* 9000 Series */
	{ "IWL9260", NONE, 0x8086, 0x2526, PCI_ANY, 0x1550, NULL },
	{ "IWL9560", NONE, 0x8086, 0x2526, PCI_ANY, 0x1551, NULL },
	{ "IWL9560", NONE, 0x8086, 0x2526, PCI_ANY, 0x1552, NULL },
	{ "IWL9xxx", NONE, 0x8086, 0x2526, PCI_ANY, PCI_ANY, NULL },
	{ "IWL9560", NONE, 0x8086, 0x271B, PCI_ANY, PCI_ANY, NULL },
	{ "IWL9260", NONE, 0x8086, 0x271C, PCI_ANY, 0x0214, NULL },
	{ "IWL9560", NONE, 0x8086, 0x271C, PCI_ANY, PCI_ANY, NULL },
	{ "IWL9560", NONE, 0x8086, 0x30DC, PCI_ANY, PCI_ANY, NULL },
	{ "IWL9560", NONE, 0x8086, 0x31DC, PCI_ANY, PCI_ANY, NULL },
	{ "IWL9560", NONE, 0x8086, 0x9DF0, PCI_ANY, PCI_ANY, NULL },
	{ "IWL9560", NONE, 0x8086, 0xA370, PCI_ANY, PCI_ANY, NULL },

	/* Qu devices */
	{ "IWLQu", NONE, 0x8086, 0x02F0, PCI_ANY, PCI_ANY, NULL },
	{ "IWLQu", NONE, 0x8086, 0x06F0, PCI_ANY, PCI_ANY, NULL },

	{ "IWLQu", NONE, 0x8086, 0x34F0, PCI_ANY, PCI_ANY, NULL },
	{ "IWLQu", NONE, 0x8086, 0x3DF0, PCI_ANY, PCI_ANY, NULL },
	{ "IWLQu", NONE, 0x8086, 0x4DF0, PCI_ANY, PCI_ANY, NULL },

	{ "IWLQu", NONE, 0x8086, 0x43F0, PCI_ANY, PCI_ANY, NULL },
	{ "IWLQu", NONE, 0x8086, 0xA0F0, PCI_ANY, PCI_ANY, NULL },

	{ "IWLQu", NONE, 0x8086, 0x2720, PCI_ANY, PCI_ANY, NULL },

	{ "IWLAX200", NONE, 0x8086, 0x2723, PCI_ANY, PCI_ANY, NULL },

	{ "IWLAX211", NONE, 0x8086, 0x2725, PCI_ANY, 0x0090, NULL },
	{ "IWLAX210", NONE, 0x8086, 0x2725, PCI_ANY, 0x0020, NULL },
	{ "IWLAX210", NONE, 0x8086, 0x2725, PCI_ANY, 0x0310, NULL },
	{ "IWLAX210", NONE, 0x8086, 0x2725, PCI_ANY, 0x0510, NULL },
	{ "IWLAX210", NONE, 0x8086, 0x2725, PCI_ANY, 0x0A10, NULL },
	{ "IWLAX411", NONE, 0x8086, 0x2725, PCI_ANY, 0x00B0, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x2726, PCI_ANY, 0x0090, NULL },
	{ "IWLAX411", NONE, 0x8086, 0x2726, PCI_ANY, 0x00B0, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x2726, PCI_ANY, 0x0510, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7A70, PCI_ANY, 0x0090, NULL },
	{ "IWLAX411", NONE, 0x8086, 0x7A70, PCI_ANY, 0x00B0, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7A70, PCI_ANY, 0x0310, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7A70, PCI_ANY, 0x0510, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7A70, PCI_ANY, 0x0A10, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7AF0, PCI_ANY, 0x0090, NULL },
	{ "IWLAX411", NONE, 0x8086, 0x7AF0, PCI_ANY, 0x00B0, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7AF0, PCI_ANY, 0x0310, NULL },
	{ "IWLAX211", NONE, 0x8086, 0x7AF0, PCI_ANY, 0x0510, NULL },
	{ "IWLAX411", NONE, 0x8086, 0x7AF0, PCI_ANY, 0x0A10, NULL },
#endif
	{ "AR9100 802.11n", CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY, "ath9k" },
	{ "AR933x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY,
	  "ar933x_wmac" },
	{ "AR934x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY,
	  "ar934x_wmac" },
	{ "QCA955x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY,
	  "qca955x_wmac" },
	{ "QCA953x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY,
	  "qca953x_wmac" },
	{ "QCA956x 802.11n", SPECTRAL | CHANNELSURVEY | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY,
	  "qca956x_wmac" },
	{ "AR231X", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY, "ar231x-wmac.0" },
	{ "AR231X", CHANNELSURVEY5K | CHWIDTH_5_10_MHZ | CHWIDTH_25_MHZ, PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY, "ar231x-wmac.1" },
	{ "IPQ4019 802.11ac", WAVE2 | SPECTRAL | FWSWITCH | CHANNELSURVEY | CHWIDTH_5_10_MHZ | QBOOST | TDMA | BEACONVAP100,
	  PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY, "soc/a000000.wifi" },
	{ "IPQ4019 802.11ac", WAVE2 | SPECTRAL | FWSWITCH | CHANNELSURVEY | CHWIDTH_5_10_MHZ | QBOOST | TDMA | BEACONVAP100,
	  PCI_ANY, PCI_ANY, PCI_ANY, PCI_ANY, "soc/a800000.wifi" },
};

char *getWifiDeviceName(const char *prefix, int *flags)
{
	int devnum;
	int i;
	int device = 0, vendor = 0, subdevice = 0, subvendor = 0;
	int devcount;
	sscanf(prefix, "wlan%d", &devcount);
	vendor = getValueFromPath("/proc/sys/dev/wifi%d/dev_vendor", devcount, "%d", NULL);
	device = getValueFromPath("/proc/sys/dev/wifi%d/dev_device", devcount, "%d", NULL);
	subvendor = getValueFromPath("/proc/sys/dev/wifi%d/idvendor", devcount, "%d", NULL);
	subdevice = getValueFromPath("/proc/sys/dev/wifi%d/idproduct", devcount, "%d", NULL);
#ifdef HAVE_TMK
	char vname[50];
	snprintf(vname, sizeof(vname), "%s_fakename", prefix);
	char *fakename = nvram_safe_get(vname);
	if (*fakename)
		return fakename;
#endif
#ifdef HAVE_ATH9K
	if (!vendor || !device) {
		devnum = get_ath9k_phy_ifname(prefix);
		if (devnum == -1)
			return NULL;
		vendor = getValueFromPath("/sys/class/ieee80211/phy%d/device/vendor", devnum, "0x%x", NULL);
		device = getValueFromPath("/sys/class/ieee80211/phy%d/device/device", devnum, "0x%x", NULL);
		subvendor = getValueFromPath("/sys/class/ieee80211/phy%d/device/subsystem_vendor", devnum, "0x%x", NULL);
		subdevice = getValueFromPath("/sys/class/ieee80211/phy%d/device/subsystem_device", devnum, "0x%x", NULL);
		if (!vendor || !device) {
			for (i = 0; i < sizeof(wdevices) / sizeof(wdevices[0]); i++) {
				if (wdevices[i].wmac) {
					char wpath[128];
					sprintf(wpath, "/sys/devices/platform/%s/ieee80211/phy%d/index", wdevices[i].wmac, devnum);
					FILE *test = fopen(wpath, "rb");
					if (test) {
						fclose(test);
						if (flags)
							*flags = wdevices[i].flags;
						return wdevices[i].name;
					}
				}
			}
		}
	}
#endif
#ifdef HAVE_RT2880
	char basedev[32];
	sprintf(basedev, "wlan%d", devcount);
	if (is_rt2880_wmac(basedev)) {
		*flags = CHANNELSURVEY;
		return "MT7620 WiSOC";
	}
	if (!vendor || !device) {
		if (!strncmp(prefix, "ra", 2) || !strncmp(prefix, "wl0", 3)) {
			FILE *fp = fopen("/sys/bus/pci/devices/0000:01:00.0/device", "rb");
			if (fp) {
				fscanf(fp, "0x%x", &device);
				fclose(fp);
			}
			fp = fopen("/sys/bus/pci/devices/0000:01:00.0/vendor", "rb");
			if (fp) {
				fscanf(fp, "0x%x", &vendor);
				fclose(fp);
			}
		}

		if (!strncmp(prefix, "ba", 2) || !strncmp(prefix, "wl1", 3)) {
			FILE *fp = fopen("/sys/bus/pci/devices/0000:02:00.0/device", "rb");
			if (fp) {
				fscanf(fp, "0x%x", &device);
				fclose(fp);
			}
			fp = fopen("/sys/bus/pci/devices/0000:02:00.0/vendor", "rb");
			if (fp) {
				fscanf(fp, "0x%x", &vendor);
				fclose(fp);
			}
		}
	}
#endif
	if (!vendor || !device) {
		return NULL;
	}
	for (i = 0; i < sizeof(wdevices) / sizeof(wdevices[0]); i++) {
		if (wdevices[i].vendor == vendor && //
		    wdevices[i].device == device && //
		    ((wdevices[i].subvendor == subvendor) || wdevices[i].subvendor == PCI_ANY) && //
		    ((wdevices[i].subdevice == subdevice) || wdevices[i].subdevice == PCI_ANY)) {
			if (flags)
				*flags = wdevices[i].flags;
			return wdevices[i].name;
		}
	}

	for (i = 0; i < sizeof(wdevices) / sizeof(wdevices[0]); i++) {
		if (!wdevices[i].subvendor && wdevices[i].vendor == vendor && wdevices[i].device == device) {
			if (flags)
				*flags = wdevices[i].flags;
			return wdevices[i].name;
		}
	}
	return NULL;
}

#ifdef HAVE_ATH9K
static int flagcheck(const char *prefix, int flag, int nullvalid)
{
	int flags = 0;
	if (!is_mac80211(prefix))
		return 0;
	char *wifiname = getWifiDeviceName(prefix, &flags);
	if (!wifiname && nullvalid)
		return 1;
	return (flags & flag);
}

#define FLAGCHECK(name, flag, nullvalid)                   \
	int has_##name(const char *prefix)                 \
	{                                                  \
		return flagcheck(prefix, flag, nullvalid); \
	}

FLAGCHECK(channelsurvey, CHANNELSURVEY, 1);
FLAGCHECK(nolivesurvey, SURVEY_NOPERIOD, 1);
FLAGCHECK(qboost, QBOOST, 0);
FLAGCHECK(qboost_tdma, TDMA, 0);
FLAGCHECK(wave2, WAVE2, 0);
FLAGCHECK(dualband_cap, DUALBAND, 0);
FLAGCHECK(beacon_limit, BEACONVAP100, 0);
FLAGCHECK(fwswitch, FWSWITCH, 0);
FLAGCHECK(spectral_support, SPECTRAL, 0);

int has_qam256(const char *prefix)
{
	int support = flagcheck(prefix, QAM256, 0);
	int bug = flagcheck(prefix, QAM256BUG, 0);
	if (support && bug && !nvram_nmatch("ddwrt", "%s_fwtype", prefix)) {
		/*
		   Vanilla firmware contains a bug caused by a wrong assert in ratecontrol on 99X0 chipsets 
		   which leads to crashes if vht modes are used in 2.4 ghz. 
		   so we can only allow using this feature on ddwrt fw types
		   on QCA988X/QCA9887 vanilla firmwares its entirely broken at several code locations. this has been fixed
		   in ddwrt firmwares
		 */
		return 0;
	}
	return support;
}

int has_dualband(const char *prefix)
{
	INITVALUECACHE();
	if (!has_dualband_cap(prefix))
		RETURNVALUE(0);
	int phy = mac80211_get_phyidx_by_vifname(prefix);
	char str[64];
	sprintf(str, "/sys/kernel/debug/ieee80211/phy%d/ath10k/bmi_board_id", phy);
	FILE *fp = fopen(str, "rb");
	if (!fp) {
		RETURNVALUE(0);
	} else {
		int bmi;
		fscanf(fp, "%d", &bmi);
		fclose(fp);
		RETURNVALUE(bmi == 11);
	}
	EXITVALUECACHE();
	return ret;
}

int has_quarter(const char *prefix)
{
	int flag = flagcheck(prefix, CHWIDTH_5_10_MHZ, 0);
	if (is_ath10k(prefix) && has_fwswitch(prefix)) {
		if (!nvram_nmatch("ddwrt", "%s_fwtype", prefix))
			return 0;
	}

	return flag;
}

int has_half(const char *prefix)
{
	int flag = flagcheck(prefix, CHWIDTH_5_10_MHZ, 0);
	if (is_ath10k(prefix) && has_fwswitch(prefix)) {
		if (!nvram_nmatch("ddwrt", "%s_fwtype", prefix))
			return 0;
	}

	return flag;
}

int has_subquarter(const char *prefix)
{
	int flag = flagcheck(prefix, CHWIDTH_25_MHZ, 0);
	if (is_ath10k(prefix) && has_fwswitch(prefix)) {
		if (!nvram_nmatch("ddwrt", "%s_fwtype", prefix))
			return 0;
	}

	return flag;
}
#endif

#ifdef HAVE_ATH9K
int getath9kdevicecount(void)
{
	glob_t globbuf;
	int globresult;
	int count = 0;
	globresult = glob("/sys/class/ieee80211/phy*", GLOB_NOSORT, NULL, &globbuf);
	if (globresult == 0)
		count = (int)globbuf.gl_pathc;
	globfree(&globbuf);
	return (count);
}

int get_ath9k_phy_idx(int idx)
{
	// fprintf(stderr,"channel number %d of %d\n", i,achans.ic_nchans);
#ifdef HAVE_MVEBU
	return idx;
#else
	return idx - getifcount("wifi");
#endif
}

int get_ath9k_phy_ifname(const char *ifname)
{
	int devnum;
	if (!ifname)
		return -1;
	if (is_wil6210(ifname))
		return 2;
	if (strncmp(ifname, "wlan", 4))
		return -1;
	if (!sscanf(ifname, "wlan%d", &devnum))
		return -1;
	// fprintf(stderr,"channel number %d of %d\n", i,achans.ic_nchans);
	return get_ath9k_phy_idx(devnum);
}

int is_mac80211(const char *prefix)
{
#ifdef HAVE_MVEBU
	return 1;
#endif
	if (strncmp(prefix, "wlan", 4))
		return 0;
	INITVALUECACHE();
	glob_t globbuf;
	char *globstring;
	int globresult;
	int devnum;
	// get legacy interface count
	// correct index if there are legacy cards arround
	devnum = get_ath9k_phy_ifname(prefix);
	if (devnum == -1) {
		RETURNVALUE(0);
	}
	asprintf(&globstring, "/sys/kernel/debug/ieee80211/phy%d", devnum);
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	free(globstring);
	if (globresult == 0) {
		RETURNVALUE((int)globbuf.gl_pathc);
	} else
		RETURNVALUE(0);
	globfree(&globbuf);
	EXITVALUECACHE();
	return ret;
}

int has_spectralscanning(const char *prefix)
{
	if (!has_spectral_support(prefix))
		return 0;
	INITVALUECACHE();
	glob_t globbuf;
	char *globstring;
	int globresult;
	int devnum;
	// get legacy interface count
	// correct index if there are legacy cards arround
	devnum = get_ath9k_phy_ifname(prefix);
	if (devnum == -1)
		RETURNVALUE(0);
	if (is_ath10k(prefix))
		asprintf(&globstring, "/sys/kernel/debug/ieee80211/phy%d/ath10k/spectral_count", devnum);
	else if (is_ath11k(prefix))
		asprintf(&globstring, "/sys/kernel/debug/ieee80211/phy%d/ath11k/spectral_count", devnum);
	else
		asprintf(&globstring, "/sys/kernel/debug/ieee80211/phy%d/ath9k/spectral_count", devnum);
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	free(globstring);
	if (globresult == 0)
		ret = (int)globbuf.gl_pathc;
	globfree(&globbuf);
	EXITVALUECACHE();
	return ret;
}
#endif

#ifdef HAVE_ATH9K
int has_airtime_fairness(const char *prefix)
{
	return (is_ath10k(prefix) || is_ath11k(prefix) || is_ath10k(prefix) || is_ath9k(prefix) || is_mt7615(prefix) ||
		is_mt7915(prefix) || is_mt7921(prefix) || is_mt7603(prefix) || is_mt76x0(prefix) || is_mt76x2(prefix));
}
#endif

#ifdef HAVE_ATH5K
static int devicecountbydriver_ath5kahb(const char *prefix)
{
	glob_t globbuf;
	char *globstring;
	int globresult;
	int devnum;
	int ret;
	// correct index if there are legacy cards arround... should not...
	devnum = get_ath9k_phy_ifname(prefix);
	if (devnum == -1)
		return 0;

	if (devnum == 0)
		asprintf(&globstring, "/sys/class/ieee80211/phy%d/device/driver/ar231x-wmac.0", devnum);
	else
		asprintf(&globstring, "/sys/class/ieee80211/phy%d/device/driver/ar231x-wmac.1", devnum);

	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	free(globstring);
	if (globresult == 0)
		ret = (int)globbuf.gl_pathc;
	else
		ret = 0;
	globfree(&globbuf);

	return ret;
}

#endif
static int devicecountbydriver(const char *prefix, char *drivername)
{
	glob_t globbuf;
	char *globstring;
	int globresult;
	int devnum;
	int ret;
	// correct index if there are legacy cards arround... should not...
	devnum = get_ath9k_phy_ifname(prefix);
	if (devnum == -1)
		return 0;
	asprintf(&globstring, "/sys/class/ieee80211/phy%d/device/driver/module/drivers/%s", devnum, drivername);
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	free(globstring);
	if (globresult == 0)
		ret = (int)globbuf.gl_pathc;
	else
		ret = 0;
	globfree(&globbuf);

	return ret;
}

#define IS_DRIVER(name, desc)                                   \
	int is_##name(const char *prefix)                       \
	{                                                       \
		INITVALUECACHE();                               \
		RETURNVALUE(devicecountbydriver(prefix, desc)); \
		EXITVALUECACHE();                               \
		return ret;                                     \
	}

#ifdef HAVE_ATH5K
IS_DRIVER(ath5k_pci, "pci:ath5k");

int is_ath5k_ahb(const char *prefix)
{
	INITVALUECACHE();
	RETURNVALUE(devicecountbydriver_ath5kahb(prefix));
	EXITVALUECACHE();
	return ret;
}

int is_ath5k(const char *prefix)
{
	return is_ath5k_pci(prefix) || is_ath5k_ahb(prefix);
}

#endif
#ifdef HAVE_ATH9K
IS_DRIVER(ath9k, "pci:ath9k");
int is_ap8x(char *prefix)
{
	INITVALUECACHE();
	char *dev = getWifiDeviceName(prefix, NULL);
	if (dev && !strcmp(dev, "AR9100 802.11n")) {
		ret = 1;
	}
	RETURNVALUE(ret);
	EXITVALUECACHE();
	return ret;
}

IS_DRIVER(iwlwifi, "pci:iwlwifi");
IS_DRIVER(iwl4965, "pci:iwl4965");
IS_DRIVER(iwl3945, "pci:iwl3945");
#endif
#ifdef HAVE_MVEBU
IS_DRIVER(mvebu, "pci:mwlwifi");
#endif
#ifdef HAVE_ATH10K
IS_DRIVER(ath10k, "pci:ath10k_pci");
#endif
#ifdef HAVE_ATH11K
IS_DRIVER(ath11k, "pci:ath11k_pci");
#endif
#ifdef HAVE_BRCMFMAC
IS_DRIVER(brcmfmac, "pci:brcmfmac");
#endif
#if defined(HAVE_MT76) || defined(HAVE_ATH11K)
IS_DRIVER(mt7615, "pci:mt7615e");
IS_DRIVER(mt7915, "pci:mt7915e");
IS_DRIVER(mt7921, "pci:mt7921e");
IS_DRIVER(mt7603, "pci:mt7603e");
IS_DRIVER(mt76x0, "pci:mt76x0e");
IS_DRIVER(mt76x2, "pci:mt76x2e");
IS_DRIVER(rt2880_wmac, "pci:rt2880_wmac");
IS_DRIVER(rt2880_pci, "pci:rt2880pci");

int is_mt76(const char *prefix)
{
	return (is_mt7615(prefix) || is_mt7915(prefix) || is_mt7921(prefix) || is_mt7603(prefix) || is_mt76x0(prefix) ||
		is_mt76x2(prefix) || is_rt2880_pci(prefix) || is_rt2880_wmac(prefix));
}
#endif

#ifdef HAVE_WPA3
int has_airtime_policy(const char *prefix)
{
	if (is_ath10k(prefix) || is_ath11k(prefix) || is_ath9k(prefix) || is_mt7615(prefix) || is_mt7915(prefix) ||
	    is_mt7921(prefix) || is_mt7603(prefix) || is_mt76x0(prefix) || is_mt76x2(prefix))
		return 1;
	return 0;
}
#endif

#ifdef HAVE_WIL6210
int is_wil6210(const char *prefix)
{
	if (!strcmp(prefix, "giwifi0"))
		return 1;
	if (!strcmp(prefix, "wlan2"))
		return 1;
	return 0;
}

#endif

/* we could read it from mac80211, but this is more complicated */
#ifdef HAVE_ATH9K
int getmaxvaps(const char *prefix)
{
	if (is_ath9k(prefix))
		return 8;
	if (is_ath10k(prefix))
		return 16;
	if (is_ath11k(prefix))
		return 16;
	if (is_ath5k(prefix))
		return 4;
	if (is_mt7615(prefix))
		return 16;
	if (is_mt7915(prefix))
		return 32;
	if (is_mt7921(prefix))
		return 4;
	if (is_mt7603(prefix))
		return 4;
	if (is_mt76x2(prefix))
		return 8;
	if (is_mt76x0(prefix))
		return 8;
	if (is_brcmfmac(prefix))
		return 4;
	if (is_mvebu(prefix))
		return 16;
	if (is_wil6210(prefix))
		return 1;
	if (is_iwlwifi(prefix))
		return 1;
	if (is_iwl4965(prefix))
		return 1;
	if (is_iwl3945(prefix))
		return 1;
	return 8; // default
}

#endif
static int HTtoVHTindex(int mcs)
{
	if (mcs < 8)
		return mcs;
	if (mcs < 16)
		return mcs + 2;
	if (mcs < 24)
		return mcs + 4;
	return mcs + 6;
}

int VHTTxRate(unsigned int mcs, unsigned int vhtmcs, unsigned int sgi, unsigned int bw)
{
	static int vHTTxRate20_800[40] = {
		6500,  13000, 19500, 26000,  39000,  52000,  58500,  65000,  78000,  78000, // MCS 0 -8
		13000, 26000, 39000, 52000,  78000,  104000, 117000, 130000, 156000, 156000, // MCS 8 - 15
		19500, 39000, 58500, 78000,  117000, 156000, 175500, 195000, 234000, 260000, // MCS 16 - 23
		26000, 52000, 78000, 104000, 156000, 208000, 234000, 260000, 312000, 312000 // MCS 24 - 31
	};
	static int vHTTxRate20_400[40] = {
		7200,  14400, 21700, 28900,  43300,  57800,  65000,  72200,  86700,  86700, //
		14444, 28889, 43333, 57778,  86667,  115556, 130000, 144444, 173300, 173300, //
		21700, 43300, 65000, 86700,  130000, 173300, 195000, 216700, 260000, 288900, //
		28900, 57800, 86700, 115600, 173300, 231100, 260000, 288900, 346700, 0 //
	};
	static int vHTTxRate40_800[40] = {
		13500, 27000,  40500,  54000,  81000,  108000, 121500, 135000, 162000, 180000, //
		27000, 54000,  81000,  108000, 162000, 216000, 243000, 270000, 324000, 360000, //
		40500, 81000,  121500, 162000, 243000, 324000, 364500, 405000, 486000, 540000, //
		54000, 108000, 162000, 216000, 324000, 432000, 486000, 540000, 648000, 720000 //
	};
	static int vHTTxRate40_400[40] = {
		15000, 30000,  45000,  60000,  90000,  120000, 135000, 150000, 180000, 200000, //
		30000, 60000,  90000,  120000, 180000, 240000, 270000, 300000, 360000, 400000, //
		45000, 90000,  135000, 180000, 270000, 360000, 405000, 450000, 540000, 600000, //
		60000, 120000, 180000, 240000, 360000, 480000, 540000, 600000, 720000, 800000 //
	};
	static int vHTTxRate80_800[40] = { 29300,  58500,  87800,  117000, 175500, 234000, 263300,  292500,  351000,  390000, //
					   58500,  117000, 175500, 234000, 351000, 468000, 526500,  585000,  702000,  780000,
					   87800,  175500, 263300, 351000, 526500, 702000, 0,	    877500,  1053000, 1170000, //
					   117000, 234000, 351000, 468000, 702000, 936000, 1053000, 1170000, 1404000, 1560000 };
	static int vHTTxRate80_400[40] = { 32500,  65000,  97500,  130000, 195000, 260000,  292500,  325000,  390000,  433300, //
					   65000,  130000, 195000, 260000, 390000, 520000,  585000,  650000,  780000,  866700, //
					   97500,  195000, 292500, 390000, 585000, 780000,  0,	     975000,  1170000, 1300000, //
					   130000, 260000, 390000, 520000, 780000, 1040000, 1170000, 1300000, 1560000, 1733300 };
	static int vHTTxRate160_800[40] = {
		58500,	117000, 175500, 234000, 351000,	 468000,  526500,  585000,  702000,  780000,
		117000, 234000, 351000, 468000, 702000,	 936000,  1053000, 1170000, 1404000, 1560000,
		175500, 351000, 526500, 702000, 1053000, 1404000, 1579500, 1755000, 2106000, 2106000,
		234000, 468000, 702000, 936000, 1404000, 1872000, 2106000, 2340000, 2808000, 3120000,

	};
	static int vHTTxRate160_400[40] = {
		65000,	130000, 195000, 260000,	 390000,  520000,  585000,  650000,  780000,  866700, //
		130000, 260000, 390000, 520000,	 780000,  1040000, 1170000, 1300000, 1560000, 1733300,
		195000, 390000, 585000, 780000,	 1170000, 1560000, 1755000, 1950000, 2340000, 2340000,
		260000, 520000, 780000, 1040000, 1560000, 2080000, 2340000, 2600000, 3120000, 3466700,
	};

	int *table = vHTTxRate20_400;
	//      fprintf(stderr, "sgi %d mcs %d vhtmcs %d\n", sgi, mcs, vhtmcs);
	if (sgi) {
		switch (bw) {
		case 20:
			table = vHTTxRate20_400;
			break;
		case 40:
			table = vHTTxRate40_400;
			break;
		case 80:
			table = vHTTxRate80_400;
			break;
		case 160:
			table = vHTTxRate160_400;
			break;
		}
	} else {
		switch (bw) {
		case 20:
			table = vHTTxRate20_800;
			break;
		case 40:
			table = vHTTxRate40_800;
			break;
		case 80:
			table = vHTTxRate80_800;
			break;
		case 160:
			table = vHTTxRate160_800;
			break;
		}
	}
	if (vhtmcs == -1) {
		vhtmcs = HTtoVHTindex(mcs);
	}

	return table[vhtmcs];
}

#ifndef HAVE_MADWIFI
#if defined(HAVE_NORTHSTAR) || defined(HAVE_80211AC) && !defined(HAVE_BUFFALO)
void setRegulationDomain(char *reg)
{
	char ccode[4] = "";
#define DEFAULT 0
#define EU 1
#define CN 2
#define TW 3
#define JP 4
#define CA 5
#define US 6
#define AU 7
#define RU 8
#define KR 9
#define LA 10
#define BR 11
#define SG 12

	int cntry = DEFAULT;
	struct PAIRS {
		char *code0;
		int rev0;
		char *code1;
		int rev1;
	};

	// need to handle it special on dhd
	struct PAIRS dhd_pairs[] = { { "Q2", 989, "Q2", 989 }, { "EU", 38, "EU", 38 }, { "CN", 65, "CN", 65 },
				     { "TW", 990, "TW", 990 }, { "JP", 44, "JP", 45 }, { "CA", 974, "CA", 974 },
				     { "Q2", 989, "Q2", 989 }, { "AU", 8, "AU", 8 },   { "RU", 993, "RU", 993 },
				     { "KR", 982, "KR", 982 }, { "LA", 6, "LA", 6 },   { "BR", 23, "BR", 23 },
				     { "SG", 994, "SG", 994 } };

	struct PAIRS pairs[] = { { "EU", 66, "EU", 8 },	 { "EU", 66, "EU", 38 }, { "CN", 34, "CN", 41 }, { "TW", 0, "TW", 0 },
				 { "JP", 44, "JP", 45 }, { "CA", 2, "CA", 2 },	 { "US", 0, "US", 0 },	 { "Q1", 27, "AU", 0 },
				 { "RU", 0, "RU", 0 },	 { "KR", 0, "KR", 0 },	 { "LA", 0, "LA", 0 },	 { "BR", 0, "BR", 0 },
				 { "SG", 0, "SG", 0 } };

	char *tmp = nvram_safe_get("wl_reg_mode");
	if (!*tmp)
		tmp = NULL;
	nvram_set("wl0_reg_mode", tmp);
	nvram_set("wl1_reg_mode", tmp);
	nvram_set("wl2_reg_mode", tmp);
	tmp = nvram_safe_get("wl_tpc_db");
	if (!*tmp)
		tmp = NULL;
	nvram_set("wl0_tpc_db", tmp);
	nvram_set("wl1_tpc_db", tmp);
	nvram_set("wl2_tpc_db", tmp);
	int brand = getRouterBrand();
	strncpy(ccode, getIsoName(reg), 3);
	strncpy(ccode, getIsoName(reg), 3);
	if (nvram_match("brcm_unlock", "1")) {
		pairs[DEFAULT].code0 = "ALL";
		pairs[DEFAULT].rev0 = 0;
		pairs[DEFAULT].code1 = "ALL";
		pairs[DEFAULT].rev1 = 0;
	} else {
		cntry = (!strcmp(ccode, "EU") || !strcmp(ccode, "DE") || !strcmp(ccode, "GB") || !strcmp(ccode, "FR") ||
			 !strcmp(ccode, "NL") || !strcmp(ccode, "ES") || !strcmp(ccode, "IT")) ?
				EU :
				0;
		cntry = !strcmp(ccode, "CN") ? CN : cntry;
		cntry = !strcmp(ccode, "US") ? US : cntry;
		cntry = !strcmp(ccode, "JP") ? JP : cntry;
		cntry = !strcmp(ccode, "AU") ? AU : cntry;
		cntry = !strcmp(ccode, "SG") ? SG : cntry;
		cntry = !strcmp(ccode, "BR") ? BR : cntry;
		cntry = !strcmp(ccode, "RU") ? RU : cntry;
		cntry = !strcmp(ccode, "TW") ? TW : cntry;
		cntry = !strcmp(ccode, "CA") ? CA : cntry;
		cntry = !strcmp(ccode, "KR") ? KR : cntry;
		cntry = !strcmp(ccode, "LA") ? LA : cntry;

		switch (brand) {
		case ROUTER_ASUS_AC88U:
		case ROUTER_DLINK_DIR895:
		case ROUTER_DLINK_DIR890:
		case ROUTER_DLINK_DIR885:
		case ROUTER_NETGEAR_R8500:
		case ROUTER_ASUS_AC5300:
		case ROUTER_ASUS_AC3200:
		case ROUTER_ASUS_AC3100:
			memcpy(pairs, dhd_pairs, sizeof(pairs));
			pairs[EU].code0 = "E0";
			pairs[EU].rev0 = 962;
			pairs[EU].code1 = "E0";
			pairs[EU].rev1 = 962;

			pairs[JP].code0 = "JP";
			pairs[JP].rev0 = 72;
			pairs[JP].code1 = "JP";
			pairs[JP].rev1 = 72;

			pairs[CA].code0 = "Q2";
			pairs[CA].rev0 = 992;
			pairs[CA].code1 = "Q2";
			pairs[CA].rev1 = 992;

			pairs[US].code0 = "Q2";
			pairs[US].rev0 = 992;
			pairs[US].code1 = "Q2";
			pairs[US].rev1 = 992;

			break;
		case ROUTER_NETGEAR_R7000P:
			memcpy(pairs, dhd_pairs, sizeof(pairs));
			pairs[EU].code0 = "EU";
			pairs[EU].rev0 = 38;
			pairs[EU].code1 = "E0";
			pairs[EU].rev1 = 938;
			break;
		case ROUTER_NETGEAR_R8000:
			memcpy(pairs, dhd_pairs, sizeof(pairs));
			pairs[EU].code0 = "EU";
			pairs[EU].rev0 = 38;
			pairs[EU].code1 = "EU";
			pairs[EU].rev1 = 39;
			break;
		}
	}

	//fprintf(stderr, "setRegulationDomain ccode: %s rrev: %s\n", ccode, rrev);
	nvram_seti("wl_country_rev", pairs[cntry].rev0);
	nvram_seti("wl0_country_rev", pairs[cntry].rev0);
	nvram_seti("wl1_country_rev", pairs[cntry].rev1);
	nvram_seti("wl2_country_rev", pairs[cntry].rev1);
	nvram_set("wl_country_code", pairs[cntry].code0);
	nvram_set("wl0_country_code", pairs[cntry].code0);
	nvram_set("wl1_country_code", pairs[cntry].code1);
	nvram_set("wl2_country_code", pairs[cntry].code1);

	switch (brand) {
	case ROUTER_D1800H:
		nvram_seti("wl_country_rev",
			   pairs[cntry].rev1); //DH1800 wl0 is 5G so needs to be inverted
		nvram_seti("wl0_country_rev", pairs[cntry].rev1);
		nvram_seti("wl1_country_rev", pairs[cntry].rev0);
		nvram_set("wl_country_code", pairs[cntry].code1);
		nvram_set("wl0_country_code", pairs[cntry].code1);
		nvram_set("wl1_country_code", pairs[cntry].code0);
		nvram_seti("pci/1/1/regrev", pairs[cntry].rev1);
		nvram_seti("pci/2/1/regrev", pairs[cntry].rev0);
		nvram_set("pci/1/1/ccode", pairs[cntry].code1);
		nvram_set("pci/2/1/ccode", pairs[cntry].code0);
		return;
	case ROUTER_LINKSYS_EA6500:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
	case ROUTER_NETGEAR_AC1450:
	case ROUTER_NETGEAR_R6250:
	case ROUTER_NETGEAR_R6300V2:
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
	case ROUTER_NETGEAR_R6700V3:
	case ROUTER_NETGEAR_R7000:
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR865:
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6900:
	case ROUTER_UBNT_UNIFIAC:
		nvram_seti("pci/1/1/regrev", pairs[cntry].rev0);
		nvram_seti("pci/2/1/regrev", pairs[cntry].rev1);
		nvram_set("pci/1/1/ccode", pairs[cntry].code0);
		nvram_set("pci/2/1/ccode", pairs[cntry].code1);
		break;
	case ROUTER_NETGEAR_R8000:
	case ROUTER_NETGEAR_R8500:
		nvram_seti("0:regrev", pairs[cntry].rev1);
		nvram_seti("1:regrev", pairs[cntry].rev0);
		nvram_seti("2:regrev", pairs[cntry].rev1);
		nvram_set("0:ccode", pairs[cntry].code1);
		nvram_set("1:ccode", pairs[cntry].code0);
		nvram_set("2:ccode", pairs[cntry].code1);
		nvram_seti("wl0_country_rev", pairs[cntry].rev1);
		nvram_seti("wl1_country_rev", pairs[cntry].rev0);
		nvram_seti("wl2_country_rev", pairs[cntry].rev1);
		nvram_set("wl0_country_code", pairs[cntry].code1);
		nvram_set("wl1_country_code", pairs[cntry].code0);
		nvram_set("wl2_country_code", pairs[cntry].code1);
		break;
	default:
		nvram_seti("0:regrev", pairs[cntry].rev0);
		nvram_seti("1:regrev", pairs[cntry].rev1);
		nvram_seti("2:regrev", pairs[cntry].rev1);
		nvram_set("0:ccode", pairs[cntry].code0);
		nvram_set("1:ccode", pairs[cntry].code1);
		nvram_set("2:ccode", pairs[cntry].code1);
	}
}
#endif
#endif

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
void set_vifsmac(char *base) // corrects hwaddr and bssid assignment
{
	char *next;
	char var[80];
	char mac[80];
	char *vifs = nvram_nget("%s_vifs", base);

	foreach(var, vifs, next)
	{
		eval("ifconfig", var, "down");
		wl_getbssid(var, mac);
		set_hwaddr(var, mac);
	}
}

#define PHY_TYPE_A 0
#define PHY_TYPE_B 1
#define PHY_TYPE_G 2
#define PHY_TYPE_NULL 0xf

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 1000
#define TXPWR_DEFAULT 70
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

int wlconf_up(char *name)
{
	int phytype, gmode, val, ret;
	char ifinst[32];
	char *next;
	char var[80];

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
	return -1;
#endif
	if (!strncmp(name, "vlan", 4))
		return -1;
	if (!strncmp(name, "br", 2))
		return -1;
#ifdef HAVE_ONLYCLIENT
	if (nvram_match("wl_mode", "ap")) {
		cprintf("this version does only support the client mode\n");
		nvram_set("wl_mode", "sta");
		nvram_async_commit();
	}
#endif
	int instance = get_wl_instance(name);

	if (instance == -1)
		return -1; // no wireless device

	if (nvram_nmatch("disabled", "wl%d_net_mode", instance))
		return -2;

	char prefix[16];
	sprintf(prefix, "wl%d", instance);
	if (nvram_nmatch("infra", "wl%d_mode", instance)) {
		nvram_nset("0", "wl%d_infra", instance);
	} else {
		nvram_nset("1", "wl%d_infra", instance);
	}
#ifdef HAVE_80211AC
	if (has_ac(prefix)) {
		if (nvram_nmatch("1", "wl%d_nband", instance)) {
			if (nvram_nmatch("1", "wl%d_nitro_qam", instance))
				eval("wl", "-i", name, "vht_features",
				     "4"); // nitro qam
			else
				eval("wl", "-i", name, "vht_features", "0");
		}
		if (nvram_nmatch("2", "wl%d_nband", instance)) {
			if (nvram_nmatch("1", "wl%d_nitro_qam", instance))
				eval("wl", "-i", name, "vht_features",
				     "7"); // nitro qam
			else if (nvram_nmatch("1", "wl%d_turbo_qam", instance))
				eval("wl", "-i", name, "vht_features", "3");
			else
				eval("wl", "-i", name, "vht_features", "0");
		}
		if (has_2ghz(prefix)) {
			if (nvram_nmatch("1", "wl%d_turbo_qam", instance))
				eval("wl", "-i", name, "vhtmode", "1");
			else
				eval("wl", "-i", name, "vhtmode", "0");
		} else {
			eval("wl", "-i", name, "vhtmode", "1");
		}
	}
#endif
#if (defined(HAVE_NORTHSTAR) || defined(HAVE_80211AC)) && !defined(HAVE_BUFFALO)
	setRegulationDomain(nvram_safe_get("wl_regdomain"));
#endif
	if (nvram_nmatchi(1, "wl%d_txbf", instance)) {
		if (nvram_nmatchi(1, "wl%d_mumimo", instance)) {
			nvram_nseti(2, "wl%d_txbf_bfr_cap", instance);
			nvram_nseti(2, "wl%d_txbf_bfe_cap", instance);
			nvram_nset("0x8000", "wl%d_mu_features", instance);
		} else {
			nvram_nseti(1, "wl%d_txbf_bfr_cap", instance);
			nvram_nseti(1, "wl%d_txbf_bfe_cap", instance);
			nvram_nseti(0, "wl%d_mu_features", instance);
		}

	} else {
		nvram_nseti(0, "wl%d_txbf_bfr_cap", instance);
		nvram_nseti(0, "wl%d_txbf_bfe_cap", instance);
	}

	ret = eval("wlconf", name, "up");
	sprintf(ifinst, "wl%d", instance);
	set_vifsmac(ifinst);
	ret = eval("wlconf", name, "up");
	/*
	 * eval("wl","radio","off"); eval("wl","atten","0","0","60");
	 * eval("wl","lrl","16"); eval("wl","srl","16");
	 * eval("wl","interference","0"); eval("wl","radio","on");
	 */
	gmode = nvram_ngeti("wl%d_gmode", instance);

	/*
	 * Get current phy type 
	 */
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));

	// set preamble type for b cards
	if (phytype == PHY_TYPE_B || gmode == 0) {
		if (nvram_nmatch("long", "wl%d_plcphdr", instance))
			val = WLC_PLCP_LONG;
		else if (nvram_nmatch("short", "wl%d_plcphdr", instance))
			val = WLC_PLCP_SHORT;
		else
			val = WLC_PLCP_AUTO;
		WL_IOCTL(name, WLC_SET_PLCPHDR, &val, sizeof(val));
	}
	// adjust txpwr and txant
	val = nvram_ngeti("wl%d_txpwr", instance);
	if (val < 1 || val > TXPWR_MAX)
		val = TXPWR_DEFAULT;
	char pwr[8];
	sprintf(pwr, "%d", val);
#ifdef HAVE_TXPWRFIXED
//      eval("wl", "-i", name, "txpwr1", "-m");
#else
	eval("wl", "-i", name, "txpwr1", "-m", "-o", pwr);
#endif
#ifdef HAVE_80211AC
	val = nvram_ngeti("wl%d_txpwrusr", instance);
	if (val == 1)
		eval("wl", "-i", name, "txpwr1", "-1");
#endif
	eval("wl", "-i", name, "roam_delta", nvram_default_get("roam_delta", "15"));

	/*
	 * Set txant 
	 */
	val = nvram_ngeti("wl%d_txant", instance);
	if (val < 0 || val > 3 || val == 2)
		val = 3;
	WL_IOCTL(name, WLC_SET_TXANT, &val, sizeof(val));

	/*
	 * if (nvram_match ("boardtype", "bcm94710dev")) { if (val == 0) val = 1;
	 * if (val == 1) val = 0; } 
	 */
	val = nvram_ngeti("wl%d_antdiv", instance);
	WL_IOCTL(name, WLC_SET_ANTDIV, &val, sizeof(val));

	/*
	 * search for "afterburner" string 
	 */
	char *afterburner = nvram_nget("wl%d_afterburner", instance);

	if (!strcmp(afterburner, "on"))
		eval("wl", "-i", name, "afterburner_override", "1");
	else if (!strcmp(afterburner, "off"))
		eval("wl", "-i", name, "afterburner_override", "0");
	else // auto
		eval("wl", "-i", name, "afterburner_override", "-1");

	char *shortslot = nvram_nget("wl%d_shortslot", instance);

	if (!strcmp(shortslot, "long"))
		eval("wl", "-i", name, "shortslot_override", "0");
	else if (!strcmp(shortslot, "short"))
		eval("wl", "-i", name, "shortslot_override", "1");
	else // auto
		eval("wl", "-i", name, "shortslot_override", "-1");

	// Set ACK Timing. Thx to Nbd
	char *v;
#if defined(HAVE_ACK)
	char sens[32];
	sprintf(sens, "wl%d_distance", instance);
	if ((v = nvram_default_get(sens, "500"))) {
		rw_reg_t reg;
		uint32 shm;

		val = atoi(v);
		if (val == 0) {
			eval("wl", "-i", name, "noack", "1");
			// wlc_noack (0);
		} else {
			eval("wl", "-i", name, "noack", "0");
			// wlc_noack (1);
		}

		if (val) {
			val = 9 + (val / 150) + ((val % 150) ? 1 : 0);
			char strv[32];

			sprintf(strv, "%d", val);
			eval("wl", "-i", name, "acktiming", strv);
		}
	}
#endif

	/*
	 * if (nvram_match("wl0_mode","sta") || nvram_match("wl0_mode","infra"))
	 * { val = 0; WL_IOCTL(name, WLC_SET_WET, &val, sizeof(val)); if
	 * (nvram_match("wl_mode", "infra")){ val = 0; WL_IOCTL(name,
	 * WLC_SET_INFRA, &val, sizeof(val)); } else{ val = 1; WL_IOCTL(name,
	 * WLC_SET_INFRA, &val, sizeof(val)); } } 
	 */

	if (nvram_nmatch("infra", "wl%d_mode", instance)) {
		eval("wl", "-i", name, "infra", "0");
		eval("wl", "-i", name, "ssid", nvram_nget("wl%d_ssid", instance));
	}
	eval("wl", "-i", name, "vlan_mode", "0");
	char *vifs = nvram_nget("%s_vifs", ifinst);
	if (vifs != NULL) {
		foreach(var, vifs, next)
		{
			char tmp[256];
			eval("ifconfig", var, "down");
			char *mac = nvram_nget("%s_hwaddr", var);
			set_hwaddr(var, mac);
			eval("ifconfig", var, "up");
			if (!nvram_nmatch("0", "%s_bridged", var)) {
				br_add_interface(getBridge(var, tmp), var);
			} else {
				ifconfig(var, IFUP, nvram_nget("%s_ipaddr", var), nvram_nget("%s_netmask", var));
			}
		}
		if (nvram_nmatch("apstawet", "wl%d_mode", instance)) {
			foreach(var, vifs, next)
			{
				eval("wl", "-i", var, "down");
				eval("wl", "-i", var, "apsta", "0");
				eval("wl", "-i", var, "up");
				eval("wl", "-i", var, "radioname", nvram_safe_get("router_name"));
			}
			eval("wl", "-i", name, "down");
			eval("wl", "-i", name, "apsta", "1");
			eval("wl", "-i", name, "wet", "1");
			eval("wl", "-i", name, "up");
		}
	}
	eval("ifconfig", name, "up");
	eval("wl", "-i", name, "radioname", nvram_safe_get("router_name"));
	//      eval("startservice", "emf", "-f");
	return ret;
}

int wlconf_down(char *name)
{
	eval("wlconf", name, "down");
	return 0;
}

void radio_off(int idx)
{
	if (pidof("nas") > 0 || pidof("wrt-radauth") > 0) {
		eval("stopservice", "nas", "-f");
	}
	if (idx != -1) {
		fprintf(stderr, "radio_off(%d) interface: %s\n", idx, get_wl_instance_name(idx));
		eval("wl", "-i", get_wl_instance_name(idx), "radio", "off");
		if (idx == 0)
			led_control(LED_WLAN0, LED_OFF);
		if (idx == 1)
			led_control(LED_WLAN1, LED_OFF);
		if (idx == 2)
			led_control(LED_WLAN2, LED_OFF);

	} else {
		int cc = get_wl_instances();
		int ii;

		for (ii = 0; ii < cc; ii++) {
			eval("wl", "-i", get_wl_instance_name(ii), "radio", "off");
		}
		led_control(LED_WLAN0, LED_OFF);
		led_control(LED_WLAN1, LED_OFF);
		led_control(LED_WLAN2, LED_OFF);
	}
	//fix ticket 2991
	eval("startservice", "nas", "-f");
}

void radio_on(int idx)
{
	if (pidof("nas") > 0 || pidof("wrt-radauth") > 0) {
		eval("stopservice", "nas", "-f");
	}
	if (idx != -1) {
		if (!nvram_nmatch("disabled", "wl%d_net_mode", idx)) {
			fprintf(stderr, "radio_on(%d) interface: %s \n", idx, get_wl_instance_name(idx));
			eval("wl", "-i", get_wl_instance_name(idx), "radio", "on");
			wlconf_down(get_wl_instance_name(idx));
			wlconf_up(get_wl_instance_name(idx));
		}

		if (idx == 0)
			led_control(LED_WLAN0, LED_ON);
		if (idx == 1)
			led_control(LED_WLAN1, LED_ON);
		if (idx == 2)
			led_control(LED_WLAN2, LED_ON);

	} else {
		int cc = get_wl_instances();
		int ii;
		for (ii = 0; ii < cc; ii++) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", ii)) {
				eval("wl", "-i", get_wl_instance_name(ii), "radio", "on");
				wlconf_down(get_wl_instance_name(ii));
				wlconf_up(get_wl_instance_name(ii));
			}
		}
		led_control(LED_WLAN0, LED_ON);
		led_control(LED_WLAN1, LED_ON);
		led_control(LED_WLAN2, LED_ON);
	}
	eval("startservice", "nas", "-f");
	eval("startservice", "guest_nas", "-f");
}

/*
 * int wl_probe (char *name) { int ret, val;
 * 
 * if ((ret = wl_ioctl (name, WLC_GET_MAGIC, &val, sizeof (val)))) return
 * ret; if (val != WLC_IOCTL_MAGIC) return -1; if ((ret = wl_ioctl (name,
 * WLC_GET_VERSION, &val, sizeof (val)))) return ret; if (val >
 * WLC_IOCTL_VERSION) return -1;
 * 
 * return ret; } 
 */
// #ifndef HAVE_MSSID
int wl_set_val(char *name, char *var, void *val, int len)
{
	char buf[128];
	int buf_len;

	/*
	 * check for overflow 
	 */
	if ((buf_len = strlen(var)) + 1 + len > sizeof(buf))
		return -1;

	strcpy(buf, var);
	buf_len += 1;

	/*
	 * append int value onto the end of the name string 
	 */
	memcpy(&buf[buf_len], val, len);
	buf_len += len;

	return wl_ioctl(name, WLC_SET_VAR, buf, buf_len);
}

int wl_get_val(char *name, char *var, void *val, int len)
{
	char buf[128];
	int ret;

	/*
	 * check for overflow 
	 */
	if (strlen(var) + 1 > sizeof(buf) || len > sizeof(buf))
		return -1;

	strcpy(buf, var);
	if ((ret = wl_ioctl(name, WLC_GET_VAR, buf, sizeof(buf))))
		return ret;

	memcpy(val, buf, len);
	return 0;
}

int wl_set_int(char *name, char *var, int val)
{
	return wl_set_val(name, var, &val, sizeof(val));
}

int wl_get_int(char *name, char *var, int *val)
{
	return wl_get_val(name, var, val, sizeof(*val));
}

int wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1; /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen); /* copy iovar name including null */
	memcpy((int8 *)bufptr + namelen, param, paramlen);

	err = wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen);

	return (err);
}

// #else
int wl_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1; /* length of iovar name plus null */
	iolen = namelen + paramlen;

	/* check for overflow */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen); /* copy iovar name including null */
	memcpy((int8 *)bufptr + namelen, param, paramlen);

	return wl_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

int wl_iovar_set(char *ifname, char *iovar, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	return wl_iovar_setbuf(ifname, iovar, param, paramlen, smbuf, sizeof(smbuf));
}

int wl_iovar_get(char *ifname, char *iovar, void *bufptr, int buflen)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int ret;

	/* use the return buffer if it is bigger than what we have on the stack */
	if (buflen > sizeof(smbuf)) {
		ret = wl_iovar_getbuf(ifname, iovar, NULL, 0, bufptr, buflen);
	} else {
		ret = wl_iovar_getbuf(ifname, iovar, NULL, 0, smbuf, sizeof(smbuf));
		if (ret == 0)
			memcpy(bufptr, smbuf, buflen);
	}

	return ret;
}

/*
 * set named driver variable to int value
 * calling example: wl_iovar_setint(ifname, "arate", rate) 
 */
int wl_iovar_setint(char *ifname, char *iovar, int val)
{
	return wl_iovar_set(ifname, iovar, &val, sizeof(val));
}

/*
 * get named driver variable to int value and return error indication 
 * calling example: wl_iovar_getint(ifname, "arate", &rate) 
 */
int wl_iovar_getint(char *ifname, char *iovar, int *val)
{
	return wl_iovar_get(ifname, iovar, val, sizeof(int));
}

/*
 * format a bsscfg indexed iovar buffer
 */
static int wl_bssiovar_mkbuf(char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen, unsigned int *plen)
{
	char *prefix = "bsscfg:";
	int8 *p;
	uint prefixlen;
	uint namelen;
	uint iolen;

	prefixlen = strlen(prefix); /* length of bsscfg prefix */
	namelen = strlen(iovar) + 1; /* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/*
	 * check for overflow 
	 */
	if (buflen < 0 || iolen > (uint)buflen) {
		*plen = 0;
		return BCME_BUFTOOSHORT;
	}

	p = (int8 *)bufptr;

	/*
	 * copy prefix, no null 
	 */
	memcpy(p, prefix, prefixlen);
	p += prefixlen;

	/*
	 * copy iovar name including null 
	 */
	memcpy(p, iovar, namelen);
	p += namelen;

	/*
	 * bss config index as first param 
	 */
	memcpy(p, &bssidx, sizeof(int32));
	p += sizeof(int32);

	/*
	 * parameter buffer follows 
	 */
	if (paramlen)
		memcpy(p, param, paramlen);

	*plen = iolen;
	return 0;
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int wl_bssiovar_setbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint iolen;

	err = wl_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;

	return wl_ioctl(ifname, WLC_SET_VAR, bufptr, iolen);
}

/*
 * get named & bss indexed driver variable buffer value
 */
int wl_bssiovar_getbuf(char *ifname, char *iovar, int bssidx, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint iolen;

	err = wl_bssiovar_mkbuf(iovar, bssidx, param, paramlen, bufptr, buflen, &iolen);
	if (err)
		return err;

	return wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen);
}

/*
 * set named & bss indexed driver variable to buffer value
 */
int wl_bssiovar_set(char *ifname, char *iovar, int bssidx, void *param, int paramlen)
{
	char smbuf[WLC_IOCTL_SMLEN];

	return wl_bssiovar_setbuf(ifname, iovar, bssidx, param, paramlen, smbuf, sizeof(smbuf));
}

/*
 * get named & bss indexed driver variable buffer value
 */
int wl_bssiovar_get(char *ifname, char *iovar, int bssidx, void *outbuf, int len)
{
	char smbuf[WLC_IOCTL_SMLEN];
	int err;

	/*
	 * use the return buffer if it is bigger than what we have on the stack 
	 */
	if (len > (int)sizeof(smbuf)) {
		err = wl_bssiovar_getbuf(ifname, iovar, bssidx, NULL, 0, outbuf, len);
	} else {
		bzero(smbuf, sizeof(smbuf));
		err = wl_bssiovar_getbuf(ifname, iovar, bssidx, NULL, 0, smbuf, sizeof(smbuf));
		if (err == 0)
			memcpy(outbuf, smbuf, len);
	}

	return err;
}

/*
 * set named & bss indexed driver variable to int value
 */
int wl_bssiovar_setint(char *ifname, char *iovar, int bssidx, int val)
{
	return wl_bssiovar_set(ifname, iovar, bssidx, &val, sizeof(int));
}

/*
 * void wl_printlasterror(char *name) { char err_buf[WLC_IOCTL_SMLEN];
 * strcpy(err_buf, "bcmerrstr");
 * 
 * fprintf(stderr, "Error: "); if ( wl_ioctl(name, WLC_GET_VAR, err_buf,
 * sizeof (err_buf)) != 0) fprintf(stderr, "Error getting the Errorstring
 * from driver\n"); else fprintf(stderr, err_buf); } 
 */

int get_maxbssid(char *name)
{
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *next;
	char *ifname = nvram_nget("%s_ifname", name);
	if (wl_iovar_get(ifname, "cap", (void *)caps, sizeof(caps)))
		return 4; //minimum is default
	foreach(cap, caps, next)
	{
		if (!strcmp(cap, "mbss16")) {
			return 16;
		}
		if (!strcmp(cap, "mbss8")) {
			return 8;
		}
		if (!strcmp(cap, "mbss4")) {
			return 4;
		}
	}
	return 4;
}

int has_acktiming(const char *name)
{
	char cap[WLC_IOCTL_SMLEN];
	char caps[WLC_IOCTL_MEDLEN];
	char *next;
	char *ifname = nvram_nget("%s_ifname", name);
	if (wl_iovar_get(ifname, "cap", (void *)caps, sizeof(caps)))
		return 0;
	foreach(cap, caps, next)
	{
		if (!strcmp(cap, "acktiming")) {
			return 1;
		}
	}
	return 0;
}

#endif

#ifdef HAVE_MADWIFI

#elif defined(HAVE_RT2880) || defined(HAVE_RT61)

char *getSTA()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("sta", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i)) {
				if (i == 0)
					return "ra0";
				else
					return "ba0";
			}
		}

		if (nvram_nmatch("apsta", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i)) {
				if (i == 0)
					return "apcli0";
				else
					return "apcli1";
			}
		}
	}
	return NULL;
}

char *getWET()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (!nvram_nmatch("disabled", "wl%d_net_mode", i) && nvram_nmatch("wet", "wl%d_mode", i))
			if (i == 0)
				return "ra0";
			else
				return "ba0";

		if (!nvram_nmatch("disabled", "wl%d_net_mode", i) && nvram_nmatch("apstawet", "wl%d_mode", i))
			if (i == 0)
				return "apcli0";
			else
				return "apcli1";
	}
	return NULL;
}

#else
char *getSTA()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("sta", "wl%d_mode", i) || nvram_nmatch("apsta", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i))
				return get_wl_instance_name(i);
			// else
			// return nvram_nget ("wl%d_ifname", i);
		}
	}
	return NULL;
}

char *getWET()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("wet", "wl%d_mode", i) || nvram_nmatch("apstawet", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i))
				return get_wl_instance_name(i);
			// else
			// return nvram_nget ("wl%d_ifname", i);
		}
	}
	return NULL;
}

#endif

struct wl_assoc_mac *get_wl_assoc_mac(char *ifname, int *c)
{
	struct wl_assoc_mac *wlmac = NULL;
	int count;
	char checkif[12];
	char *prefix = ifname;
	wlmac = NULL;
	count = *c = 0;

	int i;
	int gotit = 0;
	// fprintf(stderr,"assoclist\n");

	if (!ifexists(prefix))
		return NULL;
	unsigned char *buf = malloc(8192);
	struct maclist *maclist = (struct maclist *)buf;
	int cnt = getassoclist(prefix, buf);
	if (cnt > 0) {
		gotit = 1;
		wlmac = realloc(wlmac, sizeof(struct wl_assoc_mac) * (count + cnt));
		int a;
		for (a = 0; a < cnt; a++) {
			bzero(&wlmac[count + a], sizeof(struct wl_assoc_mac));
			unsigned char *m = (unsigned char *)&maclist->ea[a];
			sprintf(wlmac[count + a].mac, "%02X:%02X:%02X:%02X:%02X:%02X", m[0] & 0xff, m[1] & 0xff, m[2] & 0xff,
				m[3] & 0xff, m[4] & 0xff, m[5] & 0xff);
		}
		count += cnt;
		// cprintf("Count of wl assoclist mac is %d\n", count);
		*c = count;
		free(buf);
		return wlmac;
	}
	free(buf);
	return NULL;
}

int getdevicecount(void)
{
#ifdef HAVE_MADWIFI
	int count = 0;
#ifdef HAVE_ATH9K
	count += getath9kdevicecount();
#endif
	count += getifcount("wifi");

	return count;
#else
	return get_wl_instances();
#endif
}

int haswifi(void)
{
	int count = 0;
#ifdef HAVE_NOWIFI
	return 0;
#elif defined(HAVE_ATH9K) || defined(HAVE_MADWIFI)
	count += getdevicecount();
	return (count);
#else
	return 1;
#endif
}
