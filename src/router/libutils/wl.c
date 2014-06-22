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
#include <syslog.h>
#include <fcntl.h>
//#include <math.h>
#ifdef HAVE_ATH9K
#include <glob.h>
#endif

struct nvram_tuple router_defaults[] = {
	{0, 0, 0}
};

/*
 * DD-WRT addition (loaned from radauth) 
 */

#ifndef HAVE_MADWIFI
u_int ieee80211_mhz2ieee(u_int freq)
{
	if (freq == 2484)
		return 14;
	if (freq < 2484 && freq > 2407)
		return (freq - 2407) / 5;
	if (freq < 2412) {
		int d = ((((int)freq) - 2412) / 5) + 256;
		return d;
	}
	if (freq < 2502)
		return 14;
	if (freq < 2512)
		return 15;
	if (freq < 4990 && freq > 4940)
		return ((freq * 10) + (((freq % 5) == 2) ? 5 : 0) - 49400) / 5;
	else if (freq >= 4800 && freq < 5005)
		return (freq - 4000) / 5;
	if (freq < 5000)
		return 15 + ((freq - (2512)) / 20);

	return (freq - (5000)) / 5;
}

unsigned int ieee80211_ieee2mhz(unsigned int chan)
{
	if (chan == 14)
		return 2484;
	if (chan < 14)
		return ((2407) + chan * 5);
	else if (chan < 27)
		return ((2512) + ((chan - 15) * 20));
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

int has_5ghz(char *prefix)
{
	if (!strcmp(prefix, "wl0"))
		return 0;
	return 1;
}

int has_2ghz(char *prefix)
{
	if (!strcmp(prefix, "wl0"))
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

int wifi_getchannel(char *ifname)
{
	struct iwreq wrq;
	double freq;
	int channel;

	(void)memset(&wrq, 0, sizeof(struct iwreq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	ioctl(getsocket(), SIOCGIWFREQ, &wrq);
	closesocket();
	int i;

	freq = (float)wrq.u.freq.m;
	if (freq < 1000.0f) {
		return (int)freq;
	}

	freq = (double)wrq.u.freq.m;
	for (i = 0; i < wrq.u.freq.e; i++)
		freq *= 10.0;
	freq /= 1000000.0;
	cprintf("wifi channel %f\n", freq);
	channel = ieee80211_mhz2ieee(freq);

	return channel;
}

int wifi_getfreq(char *ifname)
{
	struct iwreq wrq;
	double freq;

	(void)memset(&wrq, 0, sizeof(struct iwreq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	ioctl(getsocket(), SIOCGIWFREQ, &wrq);
	closesocket();

	int i;

	freq = (float)wrq.u.freq.m;
	if (freq < 1000.0f) {
		return ieee80211_ieee2mhz((unsigned int)freq);
	}
	freq = (double)wrq.u.freq.m;
	for (i = 0; i < wrq.u.freq.e; i++)
		freq *= 10.0;
	freq /= 1000000.0;
	cprintf("wifi channel %f\n", freq);
	return freq;
}

float wifi_getrate(char *ifname)
{
	struct iwreq wrq;

	(void)memset(&wrq, 0, sizeof(struct iwreq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
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

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
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
		unsigned short MCS:7;	// MCS
		unsigned short BW:1;	//channel bandwidth 20MHz or 40 MHz
		unsigned short ShortGI:1;
		unsigned short STBC:2;	//SPACE
		unsigned short rsv:3;
		unsigned short MODE:2;	// Use definition MODE_xxx.
	} field;
	unsigned short word;
} MACHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_ENTRY {
	unsigned char ApIdx;
	unsigned char Addr[6];
	unsigned char Aid;
	unsigned char Psm;	// 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char MimoPs;	// 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
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
	RT_802_11_MAC_ENTRY Entry[128];	//MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;

typedef struct STAINFO {
	char mac[6];
	char rssi;
	char noise;
	char ifname[32];
} STAINFO;

int OidQueryInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength)
{
	struct iwreq wrq;

	strcpy(wrq.ifr_name, DeviceName);
	wrq.u.data.length = PtrLength;
	wrq.u.data.pointer = (caddr_t) ptr;
	wrq.u.data.flags = OidQueryCode;

	return (ioctl(socket_id, (SIOCIWFIRSTPRIV + 0x0E), &wrq));
}

#include "stapriv.h"
#include "oid.h"

STAINFO *getRaStaInfo(char *ifname)
{
	char G_bRadio = 1;	//TRUE

	int ConnectStatus = 0;
	unsigned char BssidQuery[6];

	if (!nvram_nmatch("sta", "%s_mode", ifname)
	    && !nvram_nmatch("apsta", "%s_mode", ifname)
	    && !nvram_nmatch("apstawet", "%s_mode", ifname)) {
		return NULL;
	}
	char *ifn = "ra0";
	if (!strcmp(ifname, "wl1"))
		ifn = "ba0";

	if (nvram_nmatch("apsta", "%s_mode", ifname)
	    || nvram_nmatch("apstawet", "%s_mode", ifname)) {
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
		memset(&BssidQuery, 0x00, sizeof(BssidQuery));
		OidQueryInformation(OID_802_11_BSSID, s, ifn, &BssidQuery, sizeof(BssidQuery));
		long RSSI;
		int nNoiseDbm;
		unsigned char lNoise;	// this value is (ULONG) in Ndis driver (NOTICE!!!)

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

#define RTPRIV_IOCTL_GET_MAC_TABLE		(SIOCIWFIRSTPRIV + 0x0F)

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
		(void)memset(&iwr, 0, sizeof(struct iwreq));
		(void)strncpy(iwr.ifr_name, getRADev(ifname), sizeof(iwr.ifr_name));

		iwr.u.data.pointer = (caddr_t) & table;
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

int getRssi(char *ifname, unsigned char *mac)
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
		(void)memset(&iwr, 0, sizeof(struct iwreq));
		(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));

		iwr.u.data.pointer = (caddr_t) & table;
		if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE, &iwr) < 0) {
			ignore = 1;
		}
	}

	STAINFO *sta = getRaStaInfo(ifname);

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
}

int getNoise(char *ifname, unsigned char *mac)
{

	return -95;

}

int getUptime(char *ifname, unsigned char *mac)
{
	return 0;
}

void radio_off(int idx)
{
	if (idx == 0)
		eval("iwpriv", "ra0", "set", "RadioOn=0");
	else
		eval("iwpriv", "ba0", "set", "RadioOn=0");
	led_control(LED_WLAN0, LED_OFF);
}

void radio_on(int idx)
{
	if (idx == 0)
		eval("iwpriv", "ra0", "set", "RadioOn=1");
	else
		eval("iwpriv", "ba0", "set", "RadioOn=1");
	led_control(LED_WLAN0, LED_ON);
}

#else
#ifdef WL_CHANSPEC_BW_8080

static const uint8 wf_chspec_bw_mhz[] = { 5, 10, 20, 40, 80, 160, 160 };

#define WF_NUM_BW \
	(sizeof(wf_chspec_bw_mhz)/sizeof(uint8))

/* 40MHz channels in 5GHz band */
static const uint8 wf_5g_40m_chans[] = { 38, 46, 54, 62, 102, 110, 118, 126, 134, 142, 151, 159 };

#define WF_NUM_5G_40M_CHANS \
	(sizeof(wf_5g_40m_chans)/sizeof(uint8))

/* 80MHz channels in 5GHz band */
static const uint8 wf_5g_80m_chans[] = { 42, 58, 106, 122, 138, 155 };

#define WF_NUM_5G_80M_CHANS \
	(sizeof(wf_5g_80m_chans)/sizeof(uint8))

/* 160MHz channels in 5GHz band */
static const uint8 wf_5g_160m_chans[] = { 50, 114 };

#define WF_NUM_5G_160M_CHANS \
	(sizeof(wf_5g_160m_chans)/sizeof(uint8))

static uint8 center_chan_to_edge(uint bw)
{
	/* edge channels separated by BW - 10MHz on each side
	 * delta from cf to edge is half of that,
	 * MHz to channel num conversion is 5MHz/channel
	 */
	return (uint8) (((bw - 20) / 2) / 5);
}

static uint8 channel_low_edge(uint center_ch, uint bw)
{
	return (uint8) (center_ch - center_chan_to_edge(bw));
}

/* return control channel given center channel and side band */
static uint8 channel_to_ctl_chan(uint center_ch, uint bw, uint sb)
{
	return (uint8) (channel_low_edge(center_ch, bw) + sb * 4);
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
#define WL_CHANSPEC_CTL_SB_MASK		0x0300
#define WL_CHANSPEC_CTL_SB_LOWER	0x0100
#define WL_CHANSPEC_CTL_SB_UPPER	0x0200

#define WL_CHANSPEC_BW_MASK		0x0C00
#define WL_CHANSPEC_BW_40		0x0C00

#define CHSPEC_CHANNEL(chspec)	((uint8)(chspec & 0xff))

#define CHSPEC_IS40(chspec)	(((chspec) & WL_CHANSPEC_BW_MASK) == WL_CHANSPEC_BW_40)
#define CHSPEC_SB_UPPER(chspec)	((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_UPPER)
#define CHSPEC_SB_LOWER(chspec)	((chspec & WL_CHANSPEC_CTL_SB_MASK) == WL_CHANSPEC_CTL_SB_LOWER)
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
int has_beamforming(char *prefix)
{
	wlc_rev_info_t rev;
	int c = 0;
	if (!strcmp(prefix, "wl0"))
		c = 0;
	else if (!strcmp(prefix, "wl1"))
		c = 1;

	char *name = get_wl_instance_name(c);
	wl_ioctl(name, WLC_GET_REVINFO, &rev, sizeof(rev));

	if (rev.corerev < 40)
		return 0;	/* TxBF unsupported */

	return 1;

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

	char abbrev[WLC_CNTRY_BUF_SZ] = "";	/* default.. current locale */
	wl_uint32_list_t *list;

	memset(buf, 0, WLC_IOCTL_MAXLEN);
	strcpy(buf, "chanspecs");
	buflen = strlen(buf) + 1;

	chanspec = (chanspec_t *) (buf + buflen);
	*chanspec = c;
	buflen += (sizeof(chanspec_t));

	strncpy(buf + buflen, abbrev, WLC_CNTRY_BUF_SZ);

	buflen += WLC_CNTRY_BUF_SZ;

	list = (wl_uint32_list_t *) (buf + buflen);
	list->count = WL_NUMCHANSPECS;
	buflen += sizeof(uint32) * (WL_NUMCHANSPECS + 1);

	if ((ret = wl_ioctl(ifname, WLC_GET_VAR, &buf[0], buflen)))
		if (ret < 0)
			return 0;

	int wl = get_wl_instance(ifname);

	list = (wl_uint32_list_t *) buf;

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
			if (nvram_nmatch("uu", "wl%d_nctrlsb", wl) || nvram_nmatch("ul", "wl%d_nctrlsb", wl) || nvram_nmatch("lu", "wl%d_nctrlsb", wl) || nvram_nmatch("ll", "wl%d_nctrlsb", wl))
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

#else				//HAVE_80211AC

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
int has_5ghz(char *prefix)
{
#ifdef HAVE_QTN
	if (!strcmp(prefix,"wl1"))
	    return 1;
#endif
	if (strstr(nvram_nget("%s_bandlist", prefix), "a"))
		return 1;

	return 0;
}

int has_2ghz(char *prefix)
{
	if (strstr(nvram_nget("%s_bandlist", prefix), "b"))
		return 1;

	return 0;
}

int wifi_getchannel(char *ifn)
{
	channel_info_t ci;
	char name[32];
	sprintf(name, "%s_ifname", ifn);
	char *ifname = nvram_safe_get(name);

	memset(&ci, 0, sizeof(ci));
	wl_ioctl(ifname, WLC_GET_CHANNEL, &ci, sizeof(ci));
	if (ci.scan_channel > 0) {
		return ci.scan_channel;
	} else if (ci.hw_channel > 0) {
		return ci.hw_channel;
	} else
		return -1;
}

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
	// int ap;
	// if ((wl_ioctl(name, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap) 
	// {
	int ret;
	unsigned int num;

	num = (sizeof(*list) - 4) / 6;	/* Maximum number of entries in the
					 * buffer */
	memcpy(list, &num, 4);	/* First 4 bytes are the number of ent. */

	ret = wl_ioctl(name, WLC_GET_ASSOCLIST, list, 8192);
	unsigned int *count = (unsigned int *)list;

	// }else
	// {
	// char buf[WLC_IOCTL_MAXLEN];
	/*
	 * wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	 * memset(buf,0,WLC_IOCTL_MAXLEN); if (wl_ioctl(name, WLC_GET_BSS_INFO,
	 * bss_info, WLC_IOCTL_MAXLEN)<0)return 0; struct maclist *l = (struct
	 * maclist*)list; 
	 */
	/*
	 * sta_info_t *sta_info = (sta_info_t *) buf;
	 * memset(buf,0,WLC_IOCTL_MAXLEN); if (wl_ioctl(name, WLC_GET_VAR,
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

	num = (sizeof(*list) - 4) / 6;	/* Maximum number of entries in the
					 * buffer */
	memcpy(list, &num, 4);	/* First 4 bytes are the number of ent. */

	ret = wl_ioctl(name, WLC_GET_WDSLIST, list, 8192);

	return (ret);
}

#if !defined(HAVE_RT2880) && !defined(HAVE_RT61)
int getNoise(char *ifname, unsigned char *macname)
{
	unsigned int noise;

	// rssi = 0;
	// char buf[WLC_IOCTL_MAXLEN];
	wl_ioctl(ifname, WLC_GET_PHY_NOISE, &noise, sizeof(noise));

	/*
	 * wl_bss_info_t *bss_info = (wl_bss_info_t *) buf;
	 * memset(buf,0,WLC_IOCTL_MAXLEN);
	 * 
	 * wl_ioctl(name, WLC_GET_BSS_INFO, bss_info, WLC_IOCTL_MAXLEN); if
	 * ((wl_ioctl(name, WLC_GET_AP, &ap, sizeof(ap)) < 0) || ap) { if
	 * (wl_ioctl(name, WLC_GET_PHY_NOISE, &noise, sizeof(noise)) < 0) noise = 
	 * 0; } else { // somehow the structure doesn't fit here rssi = buf[82];
	 * noise = buf[84]; } 
	 */
	return noise;
}

int getUptime(char *ifname, unsigned char *mac)
{
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

/*
 * Atheros 
 */

#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int set80211priv(struct iwreq *iwr, const char *ifname, int op, void *data, size_t len)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))

	memset(iwr, 0, sizeof(struct iwreq));
	strncpy(iwr->ifr_name, ifname, IFNAMSIZ);
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
			IOCTL_ERR(IEEE80211_IOCTL_SETPARAM),
			IOCTL_ERR(IEEE80211_IOCTL_GETPARAM),
			IOCTL_ERR(IEEE80211_IOCTL_SETMODE),
			IOCTL_ERR(IEEE80211_IOCTL_GETMODE),
			IOCTL_ERR(IEEE80211_IOCTL_SETWMMPARAMS),
			IOCTL_ERR(IEEE80211_IOCTL_GETWMMPARAMS),
			IOCTL_ERR(IEEE80211_IOCTL_SETCHANLIST),
			IOCTL_ERR(IEEE80211_IOCTL_GETCHANLIST),
			IOCTL_ERR(IEEE80211_IOCTL_CHANSWITCH),
			IOCTL_ERR(IEEE80211_IOCTL_GETCHANINFO),
			IOCTL_ERR(IEEE80211_IOCTL_SETOPTIE),
			IOCTL_ERR(IEEE80211_IOCTL_GETOPTIE),
			IOCTL_ERR(IEEE80211_IOCTL_SETMLME),
			IOCTL_ERR(IEEE80211_IOCTL_SETKEY),
			IOCTL_ERR(IEEE80211_IOCTL_DELKEY),
			IOCTL_ERR(IEEE80211_IOCTL_ADDMAC),
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

#define MEGA	1e6

float wifi_getrate(char *ifname)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)
	    && (nvram_nmatch("ap", "%s_mode", ifname)
		|| nvram_nmatch("wdsap", "%s_mode", ifname))) {
		if (nvram_nmatch("b-only", "%s_net_mode", ifname))
			return 11.0;
		if (nvram_nmatch("g-only", "%s_net_mode", ifname))
			return 54.0;
		if (nvram_nmatch("a-only", "%s_net_mode", ifname))
			return 54.0;
		if (nvram_nmatch("bg-mixed", "%s_net_mode", ifname))
			return 54.0;
		if (nvram_nmatch("2040", "%s_channelbw", ifname)
		    || nvram_nmatch("40", "%s_channelbw", ifname)) {
			return (float)(HTTxRate40_400(mac80211_get_maxmcs(ifname))) * MEGA;
		} else {
			return (float)(HTTxRate20_400(mac80211_get_maxmcs(ifname))) * MEGA;
		}
	} else
#endif
	{

		struct iwreq wrq;

		(void)memset(&wrq, 0, sizeof(struct iwreq));
		strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
		ioctl(getsocket(), SIOCGIWRATE, &wrq);
		return wrq.u.bitrate.value;
	}
}

/*
 * For doing log10/exp10 without libm 
 */
#define LOG10_MAGIC	1.25892541179

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
	while (fin > 1.000001) {	/* Eliminate rounding errors, take ceil */
		res += 1;
		fin /= LOG10_MAGIC;
	}
	return (res);
}

int isEMP(char *ifname)		//checks if its usually a emp card (no concrete detection possible)
{
	int vendor;
	int product;
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "ath%d", &devcount);
	sprintf(readid, "/proc/sys/dev/wifi%d/idvendor", devcount);
	FILE *in = fopen(readid, "rb");
	vendor = 0;
	if (in) {
		fscanf(in, "%d", &vendor);
		fclose(in);
	}
	sprintf(readid, "/proc/sys/dev/wifi%d/idproduct", devcount);
	in = fopen(readid, "rb");
	product = 0;
	if (in) {
		fscanf(in, "%d", &product);
		fclose(in);
	}
	if (vendor == 0x168c && product == 0x2062)
		return 1;
	if (vendor == 0x168c && product == 0x2063)	//will include more suspicius cards. 
		return 1;
	return 0;

}

int isXR36(char *ifname)	//checks if its usually a emp card (no concrete detection possible)
{
	int vendor;
	int product;
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "ath%d", &devcount);
	sprintf(readid, "/proc/sys/dev/wifi%d/idvendor", devcount);
	FILE *in = fopen(readid, "rb");
	vendor = 0;
	if (in) {
		fscanf(in, "%d", &vendor);
		fclose(in);
	}
	sprintf(readid, "/proc/sys/dev/wifi%d/idproduct", devcount);
	in = fopen(readid, "rb");
	product = 0;
	if (in) {
		fscanf(in, "%d", &product);
		fclose(in);
	}
	if (vendor == 0x0777 && product == 0x3c03)	//XR3.3/XR3.6/XR3.7 share the same pci id's
		return 1;
	return 0;

}

int isFXXN_PRO(char *ifname)	//checks if its usualla a DBII Networks FxxN-PRO card (no correct detection possible)
{
	char cproduct[30];
	char cvendor[30];
	char readid[64];
	int devcount;

	strcpy(readid, ifname);
	sscanf(readid, "ath%d", &devcount);
	sprintf(readid, "/sys/class/ieee80211/phy%d/device/subsystem_vendor", devcount);
	FILE *in = fopen(readid, "rb");
	if (in) {
		fscanf(in, "%s\n", cvendor);
		fclose(in);
	}
	sprintf(readid, "/sys/class/ieee80211/phy%d/device/subsystem_device", devcount);
	in = fopen(readid, "rb");
	if (in) {
		fscanf(in, "%s\n", cproduct);
		fclose(in);
	}

	if (!strcmp(cvendor, "0x168c") && !strcmp(cproduct, "0x2096")) {	//F36N-PRO / F64N-PRO shares the same id's
		return 1;
	}
	return 0;
}

int isSR71E(char *ifname)
{

	char cproduct[30];
	char cvendor[30];
	char readid[64];
	int devcount;

	strcpy(readid, ifname);
	sscanf(readid, "ath%d", &devcount);
	sprintf(readid, "/sys/class/ieee80211/phy%d/device/subsystem_vendor", devcount);
	FILE *in = fopen(readid, "rb");
	if (in) {
		fscanf(in, "%s\n", cvendor);
		fclose(in);
	}
	sprintf(readid, "/sys/class/ieee80211/phy%d/device/subsystem_device", devcount);
	in = fopen(readid, "rb");
	if (in) {
		fscanf(in, "%s\n", cproduct);
		fclose(in);
	}

	if (!strcmp(cvendor, "0x0777") && !strcmp(cproduct, "0x4e05")) {	// SR71-E
		return 1;
	}
	return 0;

}

int wifi_gettxpower(char *ifname)
{
	int poweroffset = wifi_gettxpoweroffset(ifname);
	struct iwreq wrq;

	(void)memset(&wrq, 0, sizeof(struct iwreq));

	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);

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
	poweroffset = 12;	//?? guess
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
#ifdef HAVE_ATH9K
	if (isFXXN_PRO(ifname)) {
		if (nvram_nmatch("1", "%s_cardtype", ifname))
			return 5;
		if (nvram_nmatch("2", "%s_cardtype", ifname))
			return 5;
	} else if (isSR71E(ifname)) {
		return 6;
	}
#endif

	int vendor;
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "ath%d", &devcount);
	sprintf(readid, "/proc/sys/dev/wifi%d/poweroffset", devcount);
	FILE *in = fopen(readid, "rb");

	vendor = 0;
	if (in) {
		vendor = atoi(fgets(readid, sizeof(readid), in));
		fclose(in);
	}
	poweroffset = vendor;
	if (poweroffset < 0 || poweroffset > 20)
		poweroffset = 0;
#endif
	char *manpoweroffset;
	manpoweroffset = nvram_nget("%s_poweroffset", ifname);
	if (strlen(manpoweroffset)) {
		poweroffset = atoi(manpoweroffset);
	}
	return poweroffset;
}

int get_wififreq(char *ifname, int freq)
{
#ifdef HAVE_NS3
	return -2000;
#endif

	if (isEMP(ifname)) {
		if (nvram_nmatch("4", "%s_cardtype", ifname))
			return freq - 2400;
	}
#ifdef HAVE_ATH9K
	if (isFXXN_PRO(ifname)) {
		if (nvram_nmatch("1", "%s_cardtype", ifname)) {
			if (freq < 5180 || freq > 5580)
				return -1;
			return freq - 1830;
		}
		if (nvram_nmatch("2", "%s_cardtype", ifname)) {
			if (freq < 5180 || freq > 5730)
				return -1;
			return freq + 720;
		}
	}
#endif

	char *var = NULL;
	if (ifname) {
		char localvar[32];
		sprintf(localvar, "%s_offset", ifname);
		var = nvram_get(localvar);
	}
	if (var) {
		return freq + atoi(var);
	}
	int vendor;
	int devcount;
	char readid[64];

	strcpy(readid, ifname);
	sscanf(readid, "ath%d", &devcount);
	sprintf(readid, "/proc/sys/dev/wifi%d/vendor", devcount);
	FILE *in = fopen(readid, "rb");

	vendor = 0;
	if (in) {
		vendor = atoi(fgets(readid, sizeof(readid), in));
		fclose(in);
	}
	switch (vendor) {
	case 9:		// ubnt xr9
		if (freq < 2427 || freq > 2442)
			return -1;
		return freq - (2427 - 907);
		break;
	case 4:		// ubnt sr9
		if (freq < 2422 || freq > 2437)
			return -1;
		return (2422 + 922) - freq;
		break;
	case 13:
		return freq - (5540 - 3540);	// xr3 general 3,5 ghz
	case 1328:
		return freq - (5540 - 2840);	// xr3 special 2.8 ghz
	case 1336:
		if (nvram_nmatch("2", "%s_cardtype", ifname))
			return freq - (5765 - 3658);	// xr3 3.7 ghz
		else
			return freq - (5540 - 3340);	// xr3 special 3.3/3.6 ghz
	case 7:
		if (freq < 2427 || freq > 2442)
			return -1;
		return freq - (2427 - 763);	// xr7 
	case 14:
		return freq - (5540 - 4516);	// xr4 
		// case 24:
		// return -(5540-4540); //sr4 
	case 23:		// reserved for XR2.3 until spec is known
	case 26:		// reserved for XR2.6 until spec is known

	default:
		return freq;
		break;
	}
	return freq;
}

u_int ieee80211_mhz2ieee(u_int freq)
{
	if (freq == 2484)
		return 14;
	if (freq == 2407)
		return 0;
	if (freq < 2484 && freq > 2407)
		return (freq - 2407) / 5;
	if (freq < 2412) {
		int d = ((((int)freq) - 2407) / 5) + 256;
		return d;
	}
	if (freq > 2484 && freq < 4000)
		return (freq - 2407) / 5;
	if (freq < 4990 && freq > 4940)
		return ((freq * 10) + (((freq % 5) == 2) ? 5 : 0) - 49400) / 5;
	// 5000 will become  channel 200
	if (freq > 4910 && freq < 5005)
		return (freq - 4000) / 5;
	if (freq < 5000)
		return 15 + ((freq - 2512) / 20);

	return (freq - 5000) / 5;
}

int wifi_getchannel(char *ifname)
{
	int channel;
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		int f = getFrequency_mac80211(ifname);
		return ieee80211_mhz2ieee(f);
	}
#endif

	struct iwreq wrq;

	(void)memset(&wrq, 0, sizeof(struct iwreq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	ioctl(getsocket(), SIOCGIWFREQ, &wrq);
	closesocket();
	int i;

	double freq = (double)wrq.u.freq.m;
	for (i = 0; i < wrq.u.freq.e; i++)
		freq *= 10.0;
	freq /= 1000000.0;
	cprintf("wifi channel %f\n", freq);
	channel = ieee80211_mhz2ieee(freq);

	return channel;
}

int wifi_getfreq(char *ifname)
{
	struct iwreq wrq;

#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		return getFrequency_mac80211(ifname);
	}
#endif

	(void)memset(&wrq, 0, sizeof(struct iwreq));
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	ioctl(getsocket(), SIOCGIWFREQ, &wrq);
	closesocket();

	int i;
	double freq = (double)wrq.u.freq.m;
	for (i = 0; i < wrq.u.freq.e; i++)
		freq *= 10.0;
	freq /= 1000000.0;
	cprintf("wifi channel %f\n", freq);
	return freq;
}

int get_radiostate(char *ifname)
{
	if (nvram_nmatch("disabled", "%s_net_mode", ifname))
		return 0;
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		char debugstring[64];
		FILE *fp;
		int idx;
		char state[11];

		sprintf(debugstring, "/sys/kernel/debug/ieee80211/phy%d/ath9k/diag", get_ath9k_phy_ifname(ifname));
		fp = fopen(debugstring, "r");
		if (fp) {
			fread(state, sizeof(state) - 1, 1, fp);
			fclose(fp);
			state[10] = '\0';
			if (!strncmp(state, "0x00000003", 10))
				return 0;
		}
	}
#endif
	struct ifreq ifr;
	int skfd = getsocket();

	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
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

static inline int iw_get_ext(int skfd,	/* Socket to the kernel */
			     const char *ifname,	/* Device name */
			     int request,	/* WE ID */
			     struct iwreq *pwrq)
{				/* Fixed part of the
				 * request */
	/*
	 * Set device name 
	 */
	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);
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

void radio_on_off_ath9k(int idx, int on)
{
	char debugstring[64];
	int fp;
	char secmode[16];
	char tpt[8];

	sprintf(debugstring, "/sys/kernel/debug/ieee80211/phy%d/ath9k/diag", get_ath9k_phy_idx(idx));
	fp = open(debugstring, O_WRONLY);
	if (fp) {
		if (on)
			write(fp, "0", strlen("0"));
		else
			write(fp, "3", strlen("3"));
		fprintf(stderr, "ath9k radio %d: phy%d ath%d\n", on, get_ath9k_phy_idx(idx), idx);
		close(fp);
	}
	// LED
#ifdef HAVE_WZRHPAG300NH
	if (idx == 0)
		sprintf(debugstring, "/sys/class/leds/wireless_generic_1/trigger");
	else
		sprintf(debugstring, "/sys/class/leds/wireless_generic_21/trigger");
#else
	sprintf(debugstring, "/sys/class/leds/ath9k-phy%d/trigger", get_ath9k_phy_idx(idx));
#endif
	fp = open(debugstring, O_WRONLY);
	if (fp) {
		if (on) {
			sprintf(tpt, "phy%dtpt", get_ath9k_phy_idx(idx));
			write(fp, tpt, strlen(tpt));
			sprintf(secmode, "ath%d_akm", idx);
			if (nvram_get(secmode) && !nvram_match(secmode, "disabled")) {
				// needs refinements
				if (idx == 0)
					led_control(LED_SEC0, LED_ON);
				else if (idx == 1)
					led_control(LED_SEC1, LED_ON);
			}
		} else {
			write(fp, "none", strlen("none"));
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

int is_ar5008(char *prefix)
{
	char sys[64];
	int devnum;
	sscanf(prefix, "ath%d", &devnum);

	sprintf(sys, "/proc/sys/dev/wifi%d/mimo", devnum);

	if (f_exists(sys))
		return 1;

	return 0;
}

int is_ath11n(char *prefix)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(prefix))
		return 1;
#endif
#ifdef HAVE_MADWIFI_MIMO
	if (is_ar5008(prefix))
		return 1;
#endif
	return 0;
}

int has_athmask(int devnum, int mask)
{
	char sys[64];
	int modes;

	sprintf(sys, "/proc/sys/dev/wifi%d/wirelessmodes", devnum);
	FILE *tmp = fopen(sys, "rb");

	if (tmp == NULL)
		return 0;
	fscanf(tmp, "%d", &modes);
	fclose(tmp);
	if ((modes & mask) == mask)
		return 1;
	else
		return 0;
}

int has_5ghz(char *prefix)
{
	int devnum;
	sscanf(prefix, "ath%d", &devnum);
#ifdef HAVE_ATH9K
	if (is_ath9k(prefix))
		return mac80211_check_band(prefix, 5);
#endif

	return has_athmask(devnum, 0x1);
}

int has_2ghz(char *prefix)
{
	int devnum;
	sscanf(prefix, "ath%d", &devnum);
#ifdef HAVE_ATH9K
	if (is_ath9k(prefix))
		return mac80211_check_band(prefix, 2);
#endif

	return has_athmask(devnum, 0x8);
}

static struct wifi_channels *list_channelsext(const char *ifname, int allchans)
{
	struct ieee80211req_chaninfo chans;
	struct ieee80211req_chaninfo achans;
	const struct ieee80211_channel *c;
	int i;

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
		memset(&achans, 0, sizeof(achans));
		for (i = 0; i < chans.ic_nchans; i++) {
			c = &chans.ic_chans[i];
			if (isset(active, c->ic_ieee) || allchans)
				achans.ic_chans[achans.ic_nchans++] = *c;
		}
	} else
		achans = chans;

	// fprintf(stderr,"channel number %d\n", achans.ic_nchans);
	struct wifi_channels *list = (struct wifi_channels *)safe_malloc(sizeof(struct wifi_channels) * (achans.ic_nchans + 1));
	(void)memset(list, 0, (sizeof(struct wifi_channels) * ((achans.ic_nchans + 1))));

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
			if (!strcmp(ifname, "ath1"))
				continue;
#endif
#ifdef HAVE_TW6600
			if (!strcmp(ifname, "ath1"))
				continue;
#endif
			if (nvram_invmatch(wl_mode, "a-only")
			    && nvram_invmatch(wl_mode, "mixed"))
				continue;
		}
		// filter out B/G channels if mode isnt g-only, b-only or mixed
		if (IEEE80211_IS_CHAN_ANYG(&achans.ic_chans[i])
		    || IEEE80211_IS_CHAN_B(&achans.ic_chans[i])) {
#ifdef HAVE_WHRAG108
			if (!strcmp(ifname, "ath0"))
				continue;
#endif
#ifdef HAVE_TW6600
			if (!strcmp(ifname, "ath0"))
				continue;
#endif
			if (nvram_invmatch(wl_mode, "g-only")
			    && nvram_invmatch(wl_mode, "mixed")
			    && nvram_invmatch(wl_mode, "b-only")
			    && nvram_invmatch(wl_mode, "bg-mixed"))
				continue;
		}
		// filter out channels which are not supporting turbo mode if turbo
		// is enabled
		if (!IEEE80211_IS_CHAN_STURBO(&achans.ic_chans[i])
		    && !IEEE80211_IS_CHAN_DTURBO(&achans.ic_chans[i])) {
			if (nvram_match(wl_turbo, "40"))
				continue;
		}
		// filter out turbo channels if turbo mode is disabled
		/*
		 * if (IEEE80211_IS_CHAN_STURBO (&achans.ic_chans[i]) ||
		 * IEEE80211_IS_CHAN_DTURBO (&achans.ic_chans[i])) { if (nvram_match
		 * (wl_turbo, "0")) continue; }
		 */
		if (IEEE80211_IS_CHAN_STURBO(&achans.ic_chans[i])) {
			if (!nvram_match(wl_turbo, "40"))
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

int getRssi(char *ifname, unsigned char *mac)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		return getRssi_ath9k(ifname, mac);
	}
#endif
#ifdef HAVE_MADWIFI_MIMO
	if (is_ar5008(ifname)) {
		return getRssi_11n(ifname, mac);
	}
#endif
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

int getUptime(char *ifname, unsigned char *mac)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		return getUptime_ath9k(ifname, mac);
	}
#endif
#ifdef HAVE_MADWIFI_MIMO
	if (is_ar5008(ifname)) {
		return getUptime_11n(ifname, mac);
	}
#endif
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

int getNoise(char *ifname, unsigned char *mac)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		return getNoise_ath9k(ifname, mac);
	}
#endif
#ifdef HAVE_MADWIFI_MIMO
	if (is_ar5008(ifname)) {
		return getNoise_11n(ifname, mac);
	}
#endif
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

int getassoclist(char *ifname, unsigned char *list)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
		return getassoclist_ath9k(ifname, list);
	}
#endif
#ifdef HAVE_MADWIFI_MIMO
	if (is_ar5008(ifname)) {
		return getassoclist_11n(ifname, list);
	}
#endif
	unsigned char *buf;

	buf = safe_malloc(24 * 1024);
	memset(buf, 0, 1024 * 24);
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

	if (nvram_nmatch("wdssta", "%s_mode", ifname)
	    || nvram_nmatch("sta", "%s_mode", ifname)
	    || nvram_nmatch("wet", "%s_mode", ifname)) {
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

void radio_off(int idx)
{
#ifdef HAVE_ATH9K
#ifdef HAVE_MADWIFI_MIMO
	if (nvram_match("mimo_driver", "ath9k"))
#endif
	{
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
	}
#endif
	if (idx != -1) {
		writevaproc("1", "/proc/sys/dev/wifi%d/silent", idx);
		writevaproc("1", "/proc/sys/dev/wifi%d/ledon", idx);	// switch off led
		if (idx == 0)
			led_control(LED_WLAN0, LED_OFF);
		if (idx == 1)
			led_control(LED_WLAN1, LED_OFF);
	} else {
		int cc = getdevicecount();
		int i;
		for (i = 0; i < cc; i++) {
			writevaproc("1", "/proc/sys/dev/wifi%d/silent", i);
			writevaproc("1", "/proc/sys/dev/wifi%d/ledon", i);	// switch off led
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
#ifdef HAVE_MADWIFI_MIMO
	if (nvram_match("mimo_driver", "ath9k"))
#endif
	{
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
	}
#endif
	if (idx != -1) {

		writevaproc("0", "/proc/sys/dev/wifi%d/silent", idx);
		if (idx == 0)
			led_control(LED_WLAN0, LED_ON);
		if (idx == 1)
			led_control(LED_WLAN1, LED_ON);
	} else {
		int cc = getdevicecount();
		int i;
		for (i = 0; i < cc; i++) {
			writevaproc("0", "/proc/sys/dev/wifi%d/silent", idx);
		}
		led_control(LED_WLAN0, LED_ON);
		led_control(LED_WLAN1, LED_ON);
	}
}

int gettxantenna(char *ifname)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
#ifdef HAVE_CARLSONWIRELESS
		if (!registered_has_cap(20))
			return (1);
#endif
		return (mac80211_get_avail_tx_antenna(get_ath9k_phy_ifname(ifname)));
	} else
#endif
		return (7);
}

int getrxantenna(char *ifname)
{
#ifdef HAVE_ATH9K
	if (is_ath9k(ifname)) {
#ifdef HAVE_CARLSONWIRELESS
		if (!registered_has_cap(20))
			return (1);
#endif
		return (mac80211_get_avail_rx_antenna(get_ath9k_phy_ifname(ifname)));
	} else
#endif
		return (7);
}

#endif

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)

void radio_off(int idx)
{
	if (pidof("nas") > 0 || pidof("wrt-radauth") > 0) {
		eval("stopservice", "nas");
	}
	if (idx != -1) {
		fprintf(stderr, "radio_off(%d) interface: %s\n", idx, get_wl_instance_name(idx));
		eval("wl", "-i", get_wl_instance_name(idx), "radio", "off");
		if (idx == 0)
			led_control(LED_WLAN0, LED_OFF);
		if (idx == 1)
			led_control(LED_WLAN1, LED_OFF);

	} else {

		int cc = get_wl_instances();
		int ii;

		for (ii = 0; ii < cc; ii++) {
			eval("wl", "-i", get_wl_instance_name(ii), "radio", "off");
		}
		led_control(LED_WLAN0, LED_OFF);
		led_control(LED_WLAN1, LED_OFF);
	}
	//fix ticket 2991
	eval("startservice", "nas", "-f");
}

void radio_on(int idx)
{
	if (pidof("nas") > 0 || pidof("wrt-radauth") > 0) {
		eval("stopservice", "nas");
	}
	if (idx != -1) {
		if (!nvram_nmatch("disabled", "wl%d_net_mode", idx))
			fprintf(stderr, "radio_on(%d) interface: %s \n", idx, get_wl_instance_name(idx));
		eval("wl", "-i", get_wl_instance_name(idx), "radio", "on");
		if (idx == 0)
			led_control(LED_WLAN0, LED_ON);
		if (idx == 1)
			led_control(LED_WLAN1, LED_ON);

	} else {
		int cc = get_wl_instances();
		int ii;
		for (ii = 0; ii < cc; ii++) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", ii)) {
				eval("wl", "-i", get_wl_instance_name(ii), "radio", "on");
			}
		}
		led_control(LED_WLAN0, LED_ON);
		led_control(LED_WLAN1, LED_ON);
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

// #else
int wl_iovar_getbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	int err;
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	/* length of iovar name plus null */
	iolen = namelen + paramlen;

	/*
	 * check for overflow 
	 */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8 *) bufptr + namelen, param, paramlen);

	err = wl_ioctl(ifname, WLC_GET_VAR, bufptr, buflen);

	return (err);
}

int wl_iovar_setbuf(char *ifname, char *iovar, void *param, int paramlen, void *bufptr, int buflen)
{
	uint namelen;
	uint iolen;

	namelen = strlen(iovar) + 1;	/* length of iovar name plus null */
	iolen = namelen + paramlen;

	/*
	 * check for overflow 
	 */
	if (iolen > buflen)
		return (BCME_BUFTOOSHORT);

	memcpy(bufptr, iovar, namelen);	/* copy iovar name including null */
	memcpy((int8 *) bufptr + namelen, param, paramlen);

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

	/*
	 * use the return buffer if it is bigger than what we have on the stack 
	 */
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

	prefixlen = strlen(prefix);	/* length of bsscfg prefix */
	namelen = strlen(iovar) + 1;	/* length of iovar name + null */
	iolen = prefixlen + namelen + sizeof(int) + paramlen;

	/*
	 * check for overflow 
	 */
	if (buflen < 0 || iolen > (uint) buflen) {
		*plen = 0;
		return BCME_BUFTOOSHORT;
	}

	p = (int8 *) bufptr;

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
		memset(smbuf, 0, sizeof(smbuf));
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

#endif
