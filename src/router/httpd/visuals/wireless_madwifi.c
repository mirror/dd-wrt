#ifdef HAVE_MADWIFI
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
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"

unsigned char madbuf[24 * 1024];

static const char *ieee80211_ntoa(const uint8_t mac[IEEE80211_ADDR_LEN])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x",
		     mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

int
ej_active_wireless_if(webs_t wp, int argc, char_t ** argv,
		      char *ifname, int cnt, int turbo, int macmask)
{
	// unsigned char buf[24 * 1024];

	unsigned char *cp;
	int s, len;
	struct iwreq iwr;
	char nb[32];
	sprintf(nb, "%s_bias", ifname);
	int bias = atoi(nvram_default_get(nb, "0"));
	if (!ifexists(ifname)) {
		printf("IOCTL_STA_INFO ifresolv %s failed!\n", ifname);
		return cnt;
	}
	int state = get_radiostate(ifname);

	if (state == 0 || state == -1) {
		printf("IOCTL_STA_INFO radio %s not enabled!\n", ifname);
		return cnt;
	}
	s = getsocket();
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		return cnt;
	}
	(void)memset(&iwr, 0, sizeof(struct iwreq));
	(void)strncpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name));

	iwr.u.data.pointer = (void *)&madbuf[0];
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		fprintf(stderr, "IOCTL_STA_INFO for %s failed!\n", ifname);
		closesocket();
		return cnt;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		// fprintf(stderr,"IOCTL_STA_INFO len<struct %s failed!\n",ifname);
		closesocket();
		return cnt;
	}
	cp = madbuf;
	do {
		struct ieee80211req_sta_info *si;
		uint8_t *vp;

		si = (struct ieee80211req_sta_info *)cp;
		vp = (u_int8_t *)(si + 1);

		if (cnt)
			websWrite(wp, ",");
		cnt++;
		char mac[32];

		strcpy(mac, ieee80211_ntoa(si->isi_macaddr));
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
		if (si->isi_noise == 0) {
			si->isi_noise = -95;
		}
		int qual = (si->isi_noise + si->isi_rssi) * 124 + 11600;
		qual /= 10;
		char *type="";
		if (si->isi_athflags & IEEE80211_ATHC_WDS)
		    type="WDS:";
		    
		if (si->isi_rates
		    && ((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) !=
			0)
		    && ((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) !=
			0)) {
			websWrite(wp,
				  "'%s','%s%s','%s','%3dM','%3dM','%d','%d','%d','%d'",
				  mac, type,ifname, UPTIME(si->isi_uptime),
				  ((si->isi_rates[si->isi_txrate] &
				    IEEE80211_RATE_VAL) / 2) * turbo,
				  ((si->isi_rates[si->isi_rxrate] &
				    IEEE80211_RATE_VAL) / 2) * turbo,
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		} else {
			websWrite(wp,
				  "'%s','%s%s','%s','N/A','N/A','%d','%d','%d','%d'",
				  mac, type,ifname, UPTIME(si->isi_uptime),
				  si->isi_noise + si->isi_rssi + bias,
				  si->isi_noise + bias, si->isi_rssi, qual);
		}
		cp += si->isi_len;
		len -= si->isi_len;
	}
	while (len >= sizeof(struct ieee80211req_sta_info));
	closesocket();

	return cnt;
}

extern int
ej_active_wireless_if_11n(webs_t wp, int argc, char_t ** argv,
			  char *ifname, int cnt, int turbo, int macmask);

extern char *getiflist(void);

void ej_active_wireless(webs_t wp, int argc, char_t ** argv)
{
	int c = getdevicecount();
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
	for (i = 0; i < c; i++) {
		sprintf(devs, "ath%d", i);
		sprintf(turbo, "%s_channelbw", devs);
		if (nvram_match(turbo, "40"))
			t = 2;
		else
			t = 1;
{
memdebug_enter();
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
		if (is_ath11n(devs))
			cnt =
			    ej_active_wireless_if_11n(wp, argc, argv, devs, cnt,
						      t, macmask);
		else
#endif
			cnt =
			    ej_active_wireless_if(wp, argc, argv, devs, cnt, t,
						  macmask);
memdebug_leave_info("active_wireless_if call");
}

		char vif[32];

		sprintf(vif, "%s_vifs", devs);
		char var[80], *next;
		char *vifs = nvram_get(vif);

{
memdebug_enter();
		if (vifs != NULL)
			foreach(var, vifs, next) {
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
			if (is_ath11n(devs))
				cnt =
				    ej_active_wireless_if_11n(wp, argc, argv,
							      var, cnt, t,
							      macmask);
			else
#endif
				cnt =
				    ej_active_wireless_if(wp, argc, argv, var,
							  cnt, t, macmask);
			}
memdebug_leave_info("active_wireless_if call");
}
	}
{
memdebug_enter();

	// show wds links
	for (i = 0; i < c; i++) {

		int s;

		for (s = 1; s <= 10; s++) {
			char wdsvarname[32] = { 0 };
			char wdsdevname[32] = { 0 };
			char wdsmacname[32] = { 0 };
			char *dev;
			char *hwaddr;
			char var[80];

			sprintf(wdsvarname, "ath%d_wds%d_enable", i, s);
			sprintf(wdsdevname, "ath%d_wds%d_if", i, s);
			sprintf(wdsmacname, "ath%d_wds%d_hwaddr", i, s);
			sprintf(turbo, "ath%d_channelbw", i);
			if (nvram_match(turbo, "40"))
				t = 2;
			else
				t = 1;

			dev = nvram_safe_get(wdsdevname);
			if (dev == NULL || strlen(dev) == 0)
				continue;
			if (nvram_match(wdsvarname, "0"))
				continue;
#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)
			if (is_ath11n(devs))
				cnt =
				    ej_active_wireless_if_11n(wp, argc, argv,
							      dev, cnt, t,
							      macmask);
			else
#endif
				cnt =
				    ej_active_wireless_if(wp, argc, argv, dev,
							  cnt, t, macmask);
		}
	}
memdebug_leave_info("active_wireless_wds call");
}
}

int get_distance(void)
{
	char path[64];
	int ifcount, distance = 0;

	strcpy(path, nvram_safe_get("wifi_display"));
	sscanf(path, "ath%d", &ifcount);
	sprintf(path, "/proc/sys/dev/wifi%d/distance", ifcount);
	FILE *in = fopen(path, "rb");

	if (in != NULL) {
		fscanf(in, "%d", &distance);
		fclose(in);
	}

/*	sprintf(path, "/proc/sys/dev/wifi%d/timingoffset", ifcount);
	in = fopen(path, "rb");

	if (in != NULL) {
		fscanf(in, "%d", &tim);
		fclose(in);
	}
	ack -= tim * 2;
*/
	return distance;
}

int get_acktiming(void)
{
	char path[64];
	int ifcount, ack = 0;

	strcpy(path, nvram_safe_get("wifi_display"));
	sscanf(path, "ath%d", &ifcount);
	sprintf(path, "/proc/sys/dev/wifi%d/acktimeout", ifcount);
	FILE *in = fopen(path, "rb");

	if (in != NULL) {
		fscanf(in, "%d", &ack);
		fclose(in);
	}

/*	sprintf(path, "/proc/sys/dev/wifi%d/timingoffset", ifcount);
	in = fopen(path, "rb");

	if (in != NULL) {
		fscanf(in, "%d", &tim);
		fclose(in);
	}
	ack -= tim * 2;
*/
	return ack;
}

void ej_show_acktiming(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>\n",
		  live_translate("share.acktiming"));
	int ack = get_acktiming();
	int distance = get_distance();

	websWrite(wp, "<span id=\"wl_ack\">%d&#181;s (%dm)</span> &nbsp;\n",
		  ack, distance);
	websWrite(wp, "</div>\n");
}

void ej_update_acktiming(webs_t wp, int argc, char_t ** argv)
{
	int ack = get_acktiming();
	int distance = get_distance();

	websWrite(wp, "%d&#181;s (%dm)", ack, distance);
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
	float rate = wifi_getrate(nvram_safe_get("wifi_display"));
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
	sprintf(mode, "%s_channelbw", nvram_safe_get("wifi_display"));
	if (nvram_match(mode, "40"))
		rate *= 2;
	if (rate > 0.0) {
		websWrite(wp, "%g %cb/s", rate / divisor, scale);
	} else
		websWrite(wp, "%s", live_translate("share.auto"));

}

void ej_get_curchannel(webs_t wp, int argc, char_t ** argv)
{
	int channel = wifi_getchannel(nvram_safe_get("wifi_display"));

	if (channel > 0 && channel < 1000) {
		websWrite(wp, "%d (%d MHz)", channel,
			  get_wififreq(nvram_safe_get("wifi_display"),
				       wifi_getfreq(nvram_safe_get
						    ("wifi_display"))));
	} else
		// websWrite (wp, "unknown");
		websWrite(wp, "%s", live_translate("share.unknown"));
	return;
}

void ej_active_wds(webs_t wp, int argc, char_t ** argv)
{
}

#endif
