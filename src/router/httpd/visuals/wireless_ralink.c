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
static const char *ieee80211_ntoa(const uint8_t mac[6])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x",
		     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
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
	unsigned char Addr[6];
	unsigned char Aid;
	unsigned char Psm;	// 0:PWR_ACTIVE, 1:PWR_SAVE
	unsigned char MimoPs;	// 0:MMPS_STATIC, 1:MMPS_DYNAMIC, 3:MMPS_Enabled
	char AvgRssi0;
	char AvgRssi1;
	char AvgRssi2;
	unsigned int ConnectedTime;
	MACHTTRANSMIT_SETTING TxRate;
} RT_802_11_MAC_ENTRY;

typedef struct _RT_802_11_MAC_TABLE {
	unsigned long Num;
	RT_802_11_MAC_ENTRY Entry[128];	//MAX_LEN_OF_MAC_TABLE = 32
} RT_802_11_MAC_TABLE;

#define RTPRIV_IOCTL_GET_MAC_TABLE		(SIOCIWFIRSTPRIV + 0x0F)

typedef struct STAINFO {
	char mac[6];
	char rssi;
	char noise;
	char ifname[32];
} STAINFO;

int
ej_active_wireless_if(webs_t wp, int argc, char_t ** argv,
		      char *ifname, int cnt, int turbo, int macmask)
{

	RT_802_11_MAC_TABLE table = { 0 };

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
//    iwr.u.data.length = 24 * 1024;
	if (ioctl(s, RTPRIV_IOCTL_GET_MAC_TABLE, &iwr) < 0) {
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
#if 0
			if (si->isi_rates
			    &&
			    ((si->isi_rates[si->isi_txrate] &
			      IEEE80211_RATE_VAL) != 0)
			    &&
			    ((si->isi_rates[si->isi_rxrate] &
			      IEEE80211_RATE_VAL) != 0)) {
				websWrite(wp,
					  "'%s','%s','%3dM','%3dM','%d','%d','%d'",
					  mac, ifname,
					  ((si->isi_rates[si->isi_txrate] &
					    IEEE80211_RATE_VAL) / 2) * turbo,
					  ((si->isi_rates[si->isi_rxrate] &
					    IEEE80211_RATE_VAL) / 2) * turbo,
					  -95 + table.Entry[i].AvgRssi0, -95,
					  table.Entry[i].AvgRssi0);
			} else
//* 1.24 + 116
#endif
			{
				int qual =
				    table.Entry[i].AvgRssi0 * 124 + 11600;
				qual /= 10;

				websWrite(wp,
					  "'%s','%s','N/A','N/A','N/A','%d','%d','%d','%d'",
					  mac, ifname, table.Entry[i].AvgRssi0,
					  -95,
					  (table.Entry[i].AvgRssi0 - (-95)),
					  qual);
			}
		}
	STAINFO *sta = getRaStaInfo("wl0");

	if (sta) {
		char mac[32];

		int qual = sta->rssi * 124 + 11600;
		qual /= 10;
		strcpy(mac, ieee80211_ntoa(sta->mac));
		websWrite(wp, "'%s','%s','N/A','N/A','N/A','%d','%d','%d','%d'",
			  mac, sta->ifname, sta->rssi, sta->noise,
			  (sta->rssi - (sta->noise)), qual);
		free(sta);

	}

	closesocket();
	return cnt;
}

extern char *getiflist(void);

void ej_active_wireless(webs_t wp, int argc, char_t ** argv)
{
	char devs[32];
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
	sprintf(devs, "ra0");
	t = 1;
	cnt = ej_active_wireless_if(wp, argc, argv, "ra0", cnt, t, macmask);

}

extern float wifi_getrate(char *ifname);

#define KILO	1e3
#define MEGA	1e6
#define GIGA	1e9

void ej_get_currate(webs_t wp, int argc, char_t ** argv)
{
	char mode[32];
	int state = get_radiostate("wl0");

	if (state == 0 || state == -1) {
		websWrite(wp, "%s", live_translate("share.disabled"));
		return;
	}
	float rate = wifi_getrate("ra0");
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
	int channel = wifi_getchannel("ra0");

	if (channel > 0 && channel < 1000) {
		websWrite(wp, "%d (%d MHz)", channel, wifi_getfreq("ra0"));
	} else
		// websWrite (wp, "unknown");
		websWrite(wp, "%s", live_translate("share.unknown"));
	return;
}

void ej_active_wds(webs_t wp, int argc, char_t ** argv)
{
}

#endif
