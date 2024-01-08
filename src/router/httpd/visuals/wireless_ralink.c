/*
 * wireless_ralink.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_RT2880
#define VISUALSOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

#include "wireless_generic.c"

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

#ifndef __UCLIBC__
/* Convenience types.  */
typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;

/* Fixed-size types, underlying types depend on word size and compiler.  */
typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#endif
#include "wireless_copy.h"

#ifdef HAVE_DANUBE
#define RT_BIG_ENDIAN
#endif

static const char *ieee80211_ntoa(const uint8_t mac[6])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0],
		     mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

typedef union _MACHTTRANSMIT_SETTING {
	struct {
		unsigned short MCS : 7; // MCS
		unsigned short BW : 1; //channel bandwidth 20MHz or 40 MHz
		unsigned short ShortGI : 1;
		unsigned short STBC : 2; //SPACE
		unsigned short eTxBF : 1;
		unsigned short rsv : 1;
		unsigned short iTxBF : 1;
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
	unsigned int LastRxRate;
	short StreamSnr[3];
	short SoundingRespSnr[3];
#if 0
	short TxPER;
	short reserved;
#endif
} RT_802_11_MAC_ENTRY;

typedef union _HTTRANSMIT_SETTING {
	struct {
		unsigned short MCS : 7; // MCS
		unsigned short BW : 1; //channel bandwidth 20MHz or 40 MHz
		unsigned short ShortGI : 1;
		unsigned short STBC : 2; //SPACE
		unsigned short eTxBF : 1;
		unsigned short rsv : 1;
		unsigned short iTxBF : 1;
		unsigned short
			MODE : 2; // 0: CCK, 1:OFDM, 2:Mixedmode, 3:GreenField
	} field;
	unsigned short word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[128]; //MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;
#define BW_20 0
#define BW_40 1
// SHORTGI
#define GI_400 1 // only support in HT mode
#define GI_800 0

#define RT_OID_802_11_QUERY_LAST_RX_RATE 0x0613
#define RT_OID_802_11_QUERY_LAST_TX_RATE 0x0632

#define RTPRIV_IOCTL_GET_MAC_TABLE (SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT \
	(SIOCIWFIRSTPRIV + 0x1F) // modified by Red@Ralink, 2009/09/30

static char bGetHTTxRateByBW_GI_MCS(int nBW, int nGI, int nMCS, int *dRate)
{
	//fprintf(stderr, "bGetHTTxRateByBW_GI_MCS()\n");
	// no TxRate for (BW = 20, GI = 400, MCS = 32) & (BW = 20, GI = 400, MCS = 32)
	if (((nBW == BW_20) && (nGI == GI_400) && (nMCS == 32)) ||
	    ((nBW == BW_20) && (nGI == GI_800) && (nMCS == 32))) {
		return 0;
	}

	if (nMCS == 32)
		nMCS = 25;

	if (nBW == BW_20 && nGI == GI_800)
		*dRate = VHTTxRate(nMCS, -1, 0, 20);
	else if (nBW == BW_20 && nGI == GI_400)
		*dRate = VHTTxRate(nMCS, -1, 1, 20);
	else if (nBW == BW_40 && nGI == GI_800)
		*dRate = VHTTxRate(nMCS, -1, 0, 40);
	else if (nBW == BW_40 && nGI == GI_400)
		*dRate = VHTTxRate(nMCS, -1, 1, 40);
	else
		return 0; //false

	//fprintf(stderr, "dRate=%.1f\n", *dRate);
	return 1; //true
}

static void TxRxRateFor11n(HTTRANSMIT_SETTING *HTSetting, int *fLastTxRxRate)
{
	int b_mode[] = { 1000, 2000, 5500, 11000 };
	int g_Rate[] = { 6000, 9000, 12000, 18000, 24000, 36000, 48000, 54000 };

	switch (HTSetting->field.MODE) {
	case 0:
		if (HTSetting->field.MCS >= 0 && HTSetting->field.MCS <= 3)
			*fLastTxRxRate = b_mode[HTSetting->field.MCS];
		else if (HTSetting->field.MCS >= 8 &&
			 HTSetting->field.MCS <= 11)
			*fLastTxRxRate = b_mode[HTSetting->field.MCS - 8];
		else
			*fLastTxRxRate = 0;

		break;
	case 1:
		if ((HTSetting->field.MCS >= 0) && (HTSetting->field.MCS < 8))
			*fLastTxRxRate = g_Rate[HTSetting->field.MCS];
		else
			*fLastTxRxRate = 0;

		break;
	case 2:
	case 3:
		if (0 == bGetHTTxRateByBW_GI_MCS(
				 HTSetting->field.BW, HTSetting->field.ShortGI,
				 HTSetting->field.MCS, fLastTxRxRate)) {
			*fLastTxRxRate = 0;
		}
		break;
	default:
		*fLastTxRxRate = 0;
		break;
	}
}

extern int OidQueryInformation(unsigned long OidQueryCode, int socket_id,
			       char *DeviceName, void *ptr,
			       unsigned long PtrLength);

static void DisplayLastTxRxRateFor11n(char *ifname, int s, int nID,
				      int *fLastTxRxRate)
{
	unsigned long lHTSetting;
	HTTRANSMIT_SETTING HTSetting;
	OidQueryInformation(nID, s, ifname, &lHTSetting, sizeof(lHTSetting));
	bzero(&HTSetting, sizeof(HTSetting));
	memcpy(&HTSetting, &lHTSetting, sizeof(HTSetting));
	TxRxRateFor11n(&HTSetting, fLastTxRxRate);
}

EJ_VISIBLE void ej_assoc_count(webs_t wp, int argc, char_t **argv)
{
	assoc_count_prefix(wp, "wl");
}

int active_wireless_if(webs_t wp, int argc, char_t **argv, char *ifname,
		       int *cnt, int globalcnt, int turbo, int macmask)
{
	static RT_802_11_MAC_TABLE table = { 0 };

	unsigned char *cp;
	int s, len, i, qual;
	struct iwreq iwr;
	int ignore = 0;

	if (!ifexists(getRADev(ifname))) {
		printf("IOCTL_STA_INFO ifresolv %s failed!\n", ifname);
		return globalcnt;
	}
	int state = get_radiostate(ifname);

	if (state == 0 || state == -1) {
		return globalcnt;
	}
	s = getsocket();
	if (s < 0) {
		return globalcnt;
	}
	(void)bzero(&iwr, sizeof(struct iwreq));
	(void)strlcpy(iwr.ifr_name, getRADev(ifname), sizeof(iwr.ifr_name) - 1);

	iwr.u.data.pointer = (caddr_t)&table;
	iwr.u.data.length = sizeof(table);
	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		ignore = 1;
	}

	if (!ignore && table.Num < 128)
		for (i = 0; i < table.Num; i++) {
			if (globalcnt)
				websWrite(wp, ",");
			*cnt = (*cnt) + 1;
			globalcnt++;
			char mac[32];
			strcpy(mac, ieee80211_ntoa(table.Entry[i].Addr));
			if (nvram_matchi("maskmac", 1) && macmask) {
				mac[0] = 'x';
				mac[1] = 'x';
				mac[3] = 'x';
				mac[4] = 'x';
				mac[6] = 'x';
				mac[7] = 'x';
				mac[9] = 'x';
				mac[10] = 'x';
			}

			{
				int signal = table.Entry[i].AvgRssi0;
				if (signal >= -50)
					qual = 1000;
				else if (signal <= -100)
					qual = 0;
				else
					qual = (signal + 100) * 20;

				HTTRANSMIT_SETTING HTSetting;
				int rate = 1;
				char rx[32];
				char tx[32];
				bzero(&HTSetting, sizeof(HTSetting));
				memcpy(&HTSetting, &table.Entry[i].TxRate,
				       sizeof(HTSetting));
				TxRxRateFor11n(&HTSetting, &rate);
				snprintf(tx, 8, "%d.%d", rate / 1000,
					 rate % 1000);

				bzero(&HTSetting, sizeof(HTSetting));
				HTSetting.field.MCS =
					table.Entry[i].LastRxRate & 0x7F;
				HTSetting.field.BW =
					(table.Entry[i].LastRxRate >> 7) & 0x1;
				HTSetting.field.ShortGI =
					(table.Entry[i].LastRxRate >> 8) & 0x1;
				HTSetting.field.STBC =
					(table.Entry[i].LastRxRate >> 9) & 0x3;
				HTSetting.field.MODE =
					(table.Entry[i].LastRxRate >> 14) & 0x3;
				TxRxRateFor11n(&HTSetting, &rate);
				snprintf(rx, 8, "%d.%d", rate / 1000,
					 rate % 1000);

				int ht = 0;
				int sgi = 0;
				int vht = 0;
				char info[32];

				if (HTSetting.field.BW)
					ht = 1;
				if (HTSetting.field.ShortGI)
					sgi = 1;

				if (sgi)
					sprintf(info, "SGI");
				if (vht)
					sprintf(info, "VHT");
				else
					sprintf(info, "HT");

				if (ht == 0)
					sprintf(info, "%s20", info);
				if (ht == 1)
					sprintf(info, "%s40", info);
				if (ht == 2)
					sprintf(info, "%s80", info);
				if (ht == 3)
					sprintf(info, "%s160", info);
				char str[64] = { 0 };
				char *radev = getRADev(ifname);

				websWrite(
					wp,
					"'%s','','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%d','%d','0','%s','%s'",
					mac, radev,
					UPTIME(table.Entry[i].ConnectedTime,
					       str, sizeof(str)),
					tx, rx, info, table.Entry[i].AvgRssi0,
					-95, (table.Entry[i].AvgRssi0 - (-95)),
					qual, table.Entry[i].AvgRssi0,
					table.Entry[i].AvgRssi1,
					table.Entry[i].AvgRssi2,
					nvram_nget("%s_label", radev), radev);
			}
		}
	STAINFO *sta = getRaStaInfo(ifname);

	if (sta) {
		char mac[32];
		if (globalcnt)
			websWrite(wp, ",");
		*cnt = (*cnt) + 1;
		globalcnt++;
		int signal = sta->rssi;
		if (signal >= -50)
			qual = 1000;
		else if (signal <= -100)
			qual = 0;
		else
			qual = (signal + 100) * 20;

		int rate = 1;
		char rx[32];
		char tx[32];
		DisplayLastTxRxRateFor11n(getRADev(ifname), s,
					  RT_OID_802_11_QUERY_LAST_RX_RATE,
					  &rate);
		snprintf(rx, 8, "%d.%d", rate / 1000, rate % 1000);
		DisplayLastTxRxRateFor11n(getRADev(ifname), s,
					  RT_OID_802_11_QUERY_LAST_TX_RATE,
					  &rate);
		snprintf(tx, 8, "%d.%d", rate / 1000, rate % 1000);

		strcpy(mac, ieee80211_ntoa(sta->mac));
		websWrite(
			wp,
			"'%s','N/A','%s','N/A','%s','%s','N/A','%d','%d','%d','%d','0','0','0','0'",
			mac, sta->ifname, tx, rx, sta->rssi, sta->noise,
			(sta->rssi - (sta->noise)), qual);
		debug_free(sta);
	}

	closesocket();
	return globalcnt;
}

EJ_VISIBLE void ej_active_wireless(webs_t wp, int argc, char_t **argv)
{
	int i;
	char turbo[32];
	int t;
	int global = 0;
	int macmask = atoi(argv[0]);
	memset(assoc_count, 0, sizeof(assoc_count));
	t = 1;
	global = active_wireless_if(wp, argc, argv, "wl0", &assoc_count[0],
				    global, t, macmask);
	global = active_wireless_if(wp, argc, argv, "wl1", &assoc_count[1],
				    global, t, macmask);
}

extern long long wifi_getrate(char *ifname);

#define KILO 1000
#define MEGA 1000000
#define GIGA 1000000000

EJ_VISIBLE void ej_get_currate(webs_t wp, int argc, char_t **argv)
{
	char mode[32];
	int state = get_radiostate(nvram_safe_get("wifi_display"));

	if (state == 0 || state == -1) {
		websWrite(wp, "%s", live_translate(wp, "share.disabled"));
		return;
	}
	long long rate = wifi_getrate(getRADev(nvram_safe_get("wifi_display")));
	char scale;
	long long divisor;

	if (rate >= MEGA) {
		scale = 'M';
		divisor = MEGA;
	} else {
		scale = 'k';
		divisor = KILO;
	}
	if (rate > 0.0) {
		websWrite(wp, "%lld %cbit/s", rate / divisor, scale);
	} else
		websWrite(wp, "%s", live_translate(wp, "share.auto"));
}

EJ_VISIBLE void ej_show_acktiming(webs_t wp, int argc, char_t **argv)
{
	return;
}

EJ_VISIBLE void ej_update_acktiming(webs_t wp, int argc, char_t **argv)
{
	return;
}

EJ_VISIBLE void ej_get_curchannel(webs_t wp, int argc, char_t **argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	int channel = wifi_getchannel(getRADev(prefix));

	if (channel > 0 && channel < 1000) {
		struct wifi_interface *interface =
			wifi_getfreq(getRADev(prefix));
		if (!interface) {
			websWrite(wp, "%s",
				  live_translate(wp, "share.unknown"));
			return;
		}
		int freq = interface->freq;
		debug_free(interface);
		websWrite(wp, "%d", channel);
		if (has_mimo(prefix) &&
		    (nvram_nmatch("n-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("na-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("n2-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("n5-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("ac-only", "%s_net_mode", prefix) ||
		     nvram_nmatch("acn-mixed", "%s_net_mode", prefix) ||
		     nvram_nmatch("ng-only", "%s_net_mode", prefix)) &&
		    (nvram_nmatch("ap", "%s_mode", prefix) ||
		     nvram_nmatch("wdsap", "%s_mode", prefix) ||
		     nvram_nmatch("infra", "%s_mode", prefix))) {
			if (nvram_nmatch("40", "%s_nbw", prefix)) {
				int ext_chan = 0;

				if (nvram_nmatch("lower", "%s_nctrlsb",
						 prefix) ||
				    nvram_nmatch("ll", "%s_nctrlsb", prefix) ||
				    nvram_nmatch("lu", "%s_nctrlsb", prefix))
					ext_chan = 1;
				if (channel <= 4)
					ext_chan = 1;
				if (channel >= 10)
					ext_chan = 0;

				websWrite(wp, " + %d",
					  !ext_chan ? channel - 4 :
						      channel + 4);
			} else if (nvram_nmatch("80", "%s_nbw", prefix)) {
				if (nvram_nmatch("ll", "%s_nctrlsb", prefix) ||
				    nvram_nmatch("lower", "%s_nctrlsb", prefix))
					websWrite(wp, " + %d", channel + 6);
				if (nvram_nmatch("lu", "%s_nctrlsb", prefix))
					websWrite(wp, " + %d", channel + 2);
				if (nvram_nmatch("ul", "%s_nctrlsb", prefix))
					websWrite(wp, " + %d", channel - 2);
				if (nvram_nmatch("uu", "%s_nctrlsb", prefix) ||
				    nvram_nmatch("upper", "%s_nctrlsb", prefix))
					websWrite(wp, " + %d", channel - 6);
			}
		}
		websWrite(wp, " (%d MHz)", freq);

	} else
		websWrite(wp, "%s", live_translate(wp, "share.unknown"));
	return;
}

EJ_VISIBLE void ej_active_wds(webs_t wp, int argc, char_t **argv)
{
}

#endif
