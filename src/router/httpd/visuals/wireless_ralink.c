/*
 * wireless_ralink.c
 *
 * Copyright (C) 2005 - 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <cymac.h>
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

#include "wireless_copy.h"

#ifdef HAVE_DANUBE
#define RT_BIG_ENDIAN
#endif

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

		// Last RX Rate
//              unsigned int mcs = pe->LastRxRate & 0x7F;
//              websWrite(wp, T("<td>MCS %d<br>%2dM, %cGI<br>%s%s</td>"),
//                              mcs,  ((lastRxRate>>7) & 0x1)? 40: 20,
//                              ((lastRxRate>>8) & 0x1)? 'S': 'L',
//                              phyMode[(lastRxRate>>14) & 0x3],
//                              ((lastRxRate>>9) & 0x3)? ", STBC": " ");

typedef union _HTTRANSMIT_SETTING {
	struct {
		unsigned short MCS:7;	// MCS
		unsigned short BW:1;	//channel bandwidth 20MHz or 40 MHz
		unsigned short ShortGI:1;
		unsigned short STBC:2;	//SPACE
		unsigned short rsv:3;
		unsigned short MODE:2;	// 0: CCK, 1:OFDM, 2:Mixedmode, 3:GreenField
	} field;
	unsigned short word;
} HTTRANSMIT_SETTING, *PHTTRANSMIT_SETTING;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[128];	//MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;
#define BW_20       0
#define BW_40       1
// SHORTGI
#define GI_400      1		// only support in HT mode
#define GI_800      0

#define RT_OID_802_11_QUERY_LAST_RX_RATE            0x0613
#define	RT_OID_802_11_QUERY_LAST_TX_RATE			0x0632

#define RTPRIV_IOCTL_GET_MAC_TABLE		(SIOCIWFIRSTPRIV + 0x0F)
#define RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT					(SIOCIWFIRSTPRIV + 0x1F)	// modified by Red@Ralink, 2009/09/30

typedef struct STAINFO {
	char mac[6];
	char rssi;
	char noise;
	char ifname[32];
} STAINFO;

static char bGetHTTxRateByBW_GI_MCS(int nBW, int nGI, int nMCS, double *dRate)
{
	//fprintf(stderr, "bGetHTTxRateByBW_GI_MCS()\n");
	// no TxRate for (BW = 20, GI = 400, MCS = 32) & (BW = 20, GI = 400, MCS = 32)
	if (((nBW == BW_20) && (nGI == GI_400) && (nMCS == 32)) || ((nBW == BW_20) && (nGI == GI_800) && (nMCS == 32))) {
		return 0;
	}

	if (nMCS == 32)
		nMCS = 25;

	if (nBW == BW_20 && nGI == GI_800)
		*dRate = HTTxRate20_800(nMCS);
	else if (nBW == BW_20 && nGI == GI_400)
		*dRate = HTTxRate20_400(nMCS);
	else if (nBW == BW_40 && nGI == GI_800)
		*dRate = HTTxRate40_800(nMCS);
	else if (nBW == BW_40 && nGI == GI_400)
		*dRate = HTTxRate40_400(nMCS);
	else
		return 0;	//false

	//fprintf(stderr, "dRate=%.1f\n", *dRate);
	return 1;		//true
}

static void TxRxRateFor11n(HTTRANSMIT_SETTING * HTSetting, double *fLastTxRxRate)
{
	double b_mode[] = { 1, 2, 5.5, 11 };
	float g_Rate[] = { 6, 9, 12, 18, 24, 36, 48, 54 };

	switch (HTSetting->field.MODE) {
	case 0:
		if (HTSetting->field.MCS >= 0 && HTSetting->field.MCS <= 3)
			*fLastTxRxRate = b_mode[HTSetting->field.MCS];
		else if (HTSetting->field.MCS >= 8 && HTSetting->field.MCS <= 11)
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
		if (0 == bGetHTTxRateByBW_GI_MCS(HTSetting->field.BW, HTSetting->field.ShortGI, HTSetting->field.MCS, fLastTxRxRate)) {
			*fLastTxRxRate = 0;
		}
		break;
	default:
		*fLastTxRxRate = 0;
		break;
	}
}

extern int OidQueryInformation(unsigned long OidQueryCode, int socket_id, char *DeviceName, void *ptr, unsigned long PtrLength);

static void DisplayLastTxRxRateFor11n(char *ifname, int s, int nID, double *fLastTxRxRate)
{
	unsigned long lHTSetting;
	HTTRANSMIT_SETTING HTSetting;
	OidQueryInformation(nID, s, ifname, &lHTSetting, sizeof(lHTSetting));
	memset(&HTSetting, 0x00, sizeof(HTSetting));
	memcpy(&HTSetting, &lHTSetting, sizeof(HTSetting));
	TxRxRateFor11n(&HTSetting, fLastTxRxRate);
}

int ej_active_wireless_if(webs_t wp, int argc, char_t ** argv, char *ifname, int cnt, int turbo, int macmask)
{

	static RT_802_11_MAC_TABLE table = { 0 };

	unsigned char *cp;
	int s, len, i;
	struct iwreq iwr;
	int ignore = 0;

	if (!ifexists(ifname)) {
		printf("IOCTL_STA_INFO ifresolv %s failed!\n", ifname);
		return cnt;
	}
	int state = get_radiostate(ifname);

	if (state == 0 || state == -1) {
		return cnt;
	}
	s = getsocket();
	if (s < 0) {
		return cnt;
	}
	(void)memset(&iwr, 0, sizeof(struct iwreq));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));

	iwr.u.data.pointer = (caddr_t) & table;
	iwr.u.data.length = sizeof(table);
	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE_STRUCT, &iwr) < 0) {
		ignore = 1;
	}

	if (!ignore && table.Num < 128)
		for (i = 0; i < table.Num; i++) {
			if (cnt)
				websWrite(wp, ",");
			cnt++;
			char mac[32];
			strcpy(mac, ieee80211_ntoa(table.Entry[i].Addr));
			if (nvram_match("maskmac", "1") && macmask) {
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
				int qual = table.Entry[i].AvgRssi0 * 124 + 11600;
				qual /= 10;
				HTTRANSMIT_SETTING HTSetting;
				double rate = 1;
				char rx[32];
				char tx[32];
				memset(&HTSetting, 0x00, sizeof(HTSetting));
				memcpy(&HTSetting, &table.Entry[i].TxRate, sizeof(HTSetting));
				TxRxRateFor11n(&HTSetting, &rate);
				snprintf(tx, 8, "%.1f", rate);

				memset(&HTSetting, 0x00, sizeof(HTSetting));
				HTSetting.field.MCS = table.Entry[i].LastRxRate & 0x7F;
				HTSetting.field.BW = (table.Entry[i].LastRxRate >> 7) & 0x1;
				HTSetting.field.ShortGI = (table.Entry[i].LastRxRate >> 8) & 0x1;
				HTSetting.field.STBC = (table.Entry[i].LastRxRate >> 9) & 0x3;
				HTSetting.field.MODE = (table.Entry[i].LastRxRate >> 14) & 0x3;
				TxRxRateFor11n(&HTSetting, &rate);
				snprintf(rx, 8, "%.1f", rate);

				websWrite(wp,
					  "'%s','%s','%s','%s','%s','%d','%d','%d','%d'", mac, ifname, UPTIME(table.Entry[i].ConnectedTime), tx, rx, table.Entry[i].AvgRssi0, -95, (table.Entry[i].AvgRssi0 - (-95)), qual);
			}
		}
	STAINFO *sta = getRaStaInfo("wl0");

	if (sta) {
		char mac[32];

		int qual = sta->rssi * 124 + 11600;
		double rate = 1;
		char rx[32];
		char tx[32];
		DisplayLastTxRxRateFor11n(getRADev("wl0"), s, RT_OID_802_11_QUERY_LAST_RX_RATE, &rate);
		snprintf(rx, 8, "%.1f", rate);
		DisplayLastTxRxRateFor11n(getRADev("wl0"), s, RT_OID_802_11_QUERY_LAST_TX_RATE, &rate);
		snprintf(tx, 8, "%.1f", rate);

		qual /= 10;
		strcpy(mac, ieee80211_ntoa(sta->mac));
		websWrite(wp, "'%s','%s','N/A','%s','%s','%d','%d','%d','%d'", mac, sta->ifname, tx, rx, sta->rssi, sta->noise, (sta->rssi - (sta->noise)), qual);
		free(sta);

	}

	sta = getRaStaInfo("wl1");

	if (sta) {
		char mac[32];

		int qual = sta->rssi * 124 + 11600;
		double rate = 1;
		char rx[32];
		char tx[32];
		DisplayLastTxRxRateFor11n(getRADev("wl1"), s, RT_OID_802_11_QUERY_LAST_RX_RATE, &rate);
		snprintf(rx, 8, "%.1f", rate);
		DisplayLastTxRxRateFor11n(getRADev("wl1"), s, RT_OID_802_11_QUERY_LAST_TX_RATE, &rate);
		snprintf(tx, 8, "%.1f", rate);

		qual /= 10;
		strcpy(mac, ieee80211_ntoa(sta->mac));
		websWrite(wp, "'%s','%s','N/A','%s','%s','%d','%d','%d','%d'", mac, sta->ifname, tx, rx, sta->rssi, sta->noise, (sta->rssi - (sta->noise)), qual);
		free(sta);

	}

	closesocket();
	return cnt;
}

extern char *getiflist(void);

void ej_active_wireless(webs_t wp, int argc, char_t ** argv)
{
	int i;
	int cnt = 0;
	char turbo[32];
	int t;
	int macmask;

#ifdef FASTWEB
	ejArgs(argc, argv, "%d", &macmask);
#else
	if (ejArgs(argc, argv, "%d", &macmask) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	t = 1;
	cnt = ej_active_wireless_if(wp, argc, argv, getRADev("wl0"), cnt, t, macmask);
	cnt = ej_active_wireless_if(wp, argc, argv, getRADev("wl1"), cnt, t, macmask);

}

extern float wifi_getrate(char *ifname);

#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9

void ej_get_currate(webs_t wp, int argc, char_t ** argv)
{
	char mode[32];
	int state = get_radiostate(nvram_safe_get("wifi_display"));

	if (state == 0 || state == -1) {
		websWrite(wp, "%s", live_translate("share.disabled"));
		return;
	}
	float rate = wifi_getrate(getRADev(nvram_safe_get("wifi_display")));
	char scale;
	int divisor;

	if (rate >= GIGA) {
		scale = 'G';
		divisor = GIGA;
	} else {
		if (rate >= MEGA) {
			scale = 'M';
			divisor = MEGA;
		} else {
			scale = 'k';
			divisor = KILO;
		}
	}
	if (rate > 0.0) {
		websWrite(wp, "%g %cb/s", rate / divisor, scale);
	} else
		websWrite(wp, "%s", live_translate("share.auto"));

}

void ej_show_acktiming(webs_t wp, int argc, char_t ** argv)
{
	return;
}

void ej_update_acktiming(webs_t wp, int argc, char_t ** argv)
{
	return;
}

void ej_get_curchannel(webs_t wp, int argc, char_t ** argv)
{
	int channel = wifi_getchannel(getRADev(nvram_safe_get("wifi_display")));

	if (channel > 0 && channel < 1000) {
		websWrite(wp, "%d (%d MHz)", channel, wifi_getfreq(getRADev(nvram_safe_get("wifi_display"))));
	} else
		// websWrite (wp, "unknown");
		websWrite(wp, "%s", live_translate("share.unknown"));
	return;
}

void ej_active_wds(webs_t wp, int argc, char_t ** argv)
{
}

#endif
