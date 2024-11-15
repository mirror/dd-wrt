/*
 * wireless_madwifi.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"

unsigned char madbuf[24 * 1024];

static const char *ieee80211_ntoa(const uint8_t mac[IEEE80211_ADDR_LEN])
{
	static char a[18];
	int i;

	i = snprintf(a, sizeof(a), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
	return (i < 17 ? NULL : a);
}

int active_wireless_if(webs_t wp, int argc, char_t **argv, char *ifname, int *cnt, int globalcnt, int turbo, int macmask)
{
	// unsigned char buf[24 * 1024];

	unsigned char *cp;
	int s, len;
	struct iwreq iwr;
	char nb[32];
	sprintf(nb, "%s_bias", ifname);
	int bias = nvram_default_geti(nb, 0);
	if (!ifexists(ifname)) {
		printf("IOCTL_STA_INFO ifresolv %s failed!\n", ifname);
		return globalcnt;
	}
	int state = get_radiostate(ifname);

	if (state == 0 || state == -1) {
		printf("IOCTL_STA_INFO radio %s not enabled!\n", ifname);
		return globalcnt;
	}
	s = getsocket();
	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		return globalcnt;
	}
	(void)bzero(&iwr, sizeof(struct iwreq));
	(void)strlcpy(iwr.ifr_name, ifname, sizeof(iwr.ifr_name) - 1);

	iwr.u.data.pointer = (void *)&madbuf[0];
	iwr.u.data.length = 24 * 1024;
	if (ioctl(s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0) {
		fprintf(stderr, "IOCTL_STA_INFO for %s failed!\n", ifname);
		dd_closesocket();
		return globalcnt;
	}
	len = iwr.u.data.length;
	if (len < sizeof(struct ieee80211req_sta_info)) {
		// fprintf(stderr,"IOCTL_STA_INFO len<struct %s failed!\n",ifname);
		dd_closesocket();
		return globalcnt;
	}
	cp = madbuf;
	int bufcount = 0;
	do {
		struct ieee80211req_sta_info *si;
		uint8_t *vp;

		si = (struct ieee80211req_sta_info *)cp;
		vp = (u_int8_t *)(si + 1);

		if (globalcnt)
			websWrite(wp, ",");

		*cnt = (*cnt) + 1;
		globalcnt++;
		char mac[32];

		strlcpy(mac, ieee80211_ntoa(si->isi_macaddr), 31);
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
		if (si->isi_noise == 0) {
			si->isi_noise = -95;
		}

		int signal = si->isi_noise + si->isi_rssi;
		int qual = 0;
		if (signal >= -50)
			qual = 1000;
		else if (signal <= -100)
			qual = 0;
		else
			qual = (signal + 100) * 20;

		char *type = "";
		if (si->isi_athflags & IEEE80211_ATHC_WDS)
			type = "WDS:";
		char str[64] = { 0 };
		if (si->isi_rates && ((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) != 0) &&
		    ((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) != 0)) {
			websWrite(
				wp,
				"'%s','%s','%s%s','%s','%3dM','%3dM','N/A','%d','%d','%d','%d','0','0','0','0','0','0','0','0','%s','%s'",
				mac, si->radioname, type, ifname, UPTIME(si->isi_uptime, str, sizeof(str)),
				((si->isi_rates[si->isi_txrate] & IEEE80211_RATE_VAL) / 2) * turbo,
				((si->isi_rates[si->isi_rxrate] & IEEE80211_RATE_VAL) / 2) * turbo,
				si->isi_noise + si->isi_rssi + bias, si->isi_noise + bias, si->isi_rssi, qual,
				nvram_nget("%s_label", ifname), ifname);
		} else {
			websWrite(
				wp,
				"'%s','%s','%s%s','%s','N/A','N/A','N/A','%d','%d','%d','%d','0','0','0','0','0','0','0','0','%s','%s'",
				mac, si->radioname, type, ifname, UPTIME(si->isi_uptime, str, sizeof(str)),
				si->isi_noise + si->isi_rssi + bias, si->isi_noise + bias, si->isi_rssi, qual,
				nvram_nget("%s_label", ifname), ifname);
		}
		bufcount += si->isi_len;
		cp += si->isi_len;
		len -= si->isi_len;
	} while (len >= sizeof(struct ieee80211req_sta_info) && bufcount < (sizeof(madbuf) - sizeof(struct ieee80211req_sta_info)));
	dd_closesocket();

	return globalcnt;
}

#if defined(HAVE_ATH9K)
extern int active_wireless_if_ath9k(webs_t wp, int argc, char_t **argv, char *ifname, int *cnt, int globalcnt, int turbo,
				    int macmask);
#endif
static int assoc_count[16];

EJ_VISIBLE void ej_assoc_count(webs_t wp, int argc, char_t **argv)
{
	assoc_count_prefix(wp, "wlan");
}

EJ_VISIBLE void ej_active_wireless(webs_t wp, int argc, char_t **argv)
{
	int c = getdevicecount();
	char devs[32];
	int i;
	char turbo[32];
	int t;
	int cnt = 0;
	int global = 0;
	int macmask;
	int gotassocs = 0;
	memset(assoc_count, 0, sizeof(assoc_count));
	macmask = atoi(argv[0]);
	for (i = 0; i < c; i++) {
		sprintf(devs, "wlan%d", i);
		sprintf(turbo, "%s_channelbw", devs);
		if (nvram_matchi(turbo, 40))
			t = 2;
		else
			t = 1;
		if (is_mac80211(devs)) {
			if (has_ad(devs)) {
				global = active_wireless_if_ath9k(wp, argc, argv, "giwifi0", &assoc_count[cnt], global, t, macmask);
			} else {
				if (nvram_nmatch("1", "%s_owe", devs)) {
					char owe[64];
					sprintf(owe, "%s_owe", devs);
					global = active_wireless_if_ath9k(wp, argc, argv, owe, &assoc_count[cnt], global, t,
									  macmask);

				} else
					global = active_wireless_if_ath9k(wp, argc, argv, devs, &assoc_count[cnt], global, t,
									  macmask);
			}
			gotassocs = 1;
		}
		if (!gotassocs) {
			global = active_wireless_if(wp, argc, argv, devs, &assoc_count[cnt], global, t, macmask);
		}
		cnt++;
		char vif[32];

		sprintf(vif, "%s_vifs", devs);
		char var[80], *next;
		char *vifs = nvram_safe_get(vif);
		if (*vifs) {
			foreach(var, vifs, next)
			{
				if (!is_mac80211(devs)) {
					global = active_wireless_if(wp, argc, argv, var, &assoc_count[cnt], global, t, macmask);
				} else {
					if (nvram_nmatch("1", "%s_owe", var)) {
						char owe[64];
						sprintf(owe, "%s_owe", var);
						global = active_wireless_if_ath9k(wp, argc, argv, owe, &assoc_count[cnt], global, t,
										  macmask);

					} else
						global = active_wireless_if_ath9k(wp, argc, argv, var, &assoc_count[cnt], global, t,
										  macmask);
				}
				cnt++;
			}
		}
	}

	// show wds links
	for (i = 0; i < c; i++) {
		sprintf(devs, "wlan%d", i);
		if (!is_mac80211(devs)) {
			int s;

			for (s = 1; s <= 10; s++) {
				char wdsvarname[32] = { 0 };
				char wdsdevname[32] = { 0 };
				char wdsmacname[32] = { 0 };
				char *dev;
				char *hwaddr;
				char var[80];

				sprintf(wdsvarname, "wlan%d_wds%d_enable", i, s);
				sprintf(wdsdevname, "wlan%d_wds%d_if", i, s);
				sprintf(wdsmacname, "wlan%d_wds%d_hwaddr", i, s);
				sprintf(turbo, "wlan%d_channelbw", i);
				if (nvram_matchi(turbo, 40))
					t = 2;
				else
					t = 1;

				dev = nvram_safe_get(wdsdevname);
				if (dev == NULL || *(dev) == 0)
					continue;
				if (nvram_matchi(wdsvarname, 0))
					continue;
				global = active_wireless_if(wp, argc, argv, dev, &assoc_count[cnt], global, t, macmask);
				cnt++;
			}
		}
	}
}

static int get_distance(char *ifname)
{
	char path[64];
	int ifcount, distance = 0;

	strcpy(path, ifname);
	sscanf(path, "wlan%d", &ifcount);
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

static int get_acktiming(char *ifname)
{
	char path[64];
	int ifcount, ack = 0;

	strcpy(path, ifname);
	sscanf(path, "wlan%d", &ifcount);
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

EJ_VISIBLE void ej_update_acktiming(webs_t wp, int argc, char_t **argv)
{
	unsigned int ack, distance;
	char *ifname = nvram_safe_get("wifi_display");
	if (nvram_nmatch("disabled", "%s_net_mode", ifname)) {
		websWrite(wp, "N/A");
		return;
	}
	char ifn[32];
	strcpy(ifn, ifname);
	char *c = strchr(ifn, '.');
	if (c)
		c[0] = 0;
	if (is_ath10k(ifname) && nvram_nmatch("0", "%s_distance", ifn)) {
		int phy = mac80211_get_phyidx_by_vifname(ifn);
		char str[64];
		sprintf(str, "/sys/kernel/debug/ieee80211/phy%d/ath10k/cur_ack", phy);
		FILE *fp = fopen(str, "rb");
		int rawack;
		fscanf(fp, "%d", &rawack);
		fclose(fp);
		int slt = 9;
		int hwdelay = 0;
		int div = 1;
		if (nvram_nmatch("10", "%s_channelbw", ifn)) {
			slt = 9 * 2;
			div = 2;
		}
		if (nvram_nmatch("5", "%s_channelbw", ifn)) {
			slt = 9 * 4;
			div = 4;
		}
		if (nvram_nmatch("2", "%s_channelbw", ifn)) {
			slt = 9 * 8;
			div = 8;
		}
		// fw contains a internal tolerance value which is added, we consider it for accurate measurement
		hwdelay += (slt * 2) + (3 * div);
		if (hwdelay < rawack) {
			ack = rawack - hwdelay; // hw delay
			ack /= div; // check if this devision is required for wave-2
			if (!ack)
				ack = 1;
		} else {
			ack = rawack - 21; //fallback
		}
		distance = (300 * ack) / 2;
	} else if (is_mac80211(ifname) || is_mvebu(ifname)) {
		int coverage = mac80211_get_coverageclass(ifname);
		ack = coverage * 3;
		/* See handle_distance() for an explanation where the '450' comes from */
		distance = coverage * 450;
	} else {
		ack = get_acktiming(ifname);
		distance = get_distance(ifname);
	}
	if (ack <= 0 || distance <= 0)
		websWrite(wp, "N/A");
	else
		websWrite(wp, "%d&#181;s (%dm)", ack, distance);
}

EJ_VISIBLE void ej_show_acktiming(webs_t wp, int argc, char_t **argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	if (nvram_nmatch("disabled", "%s_net_mode", prefix))
		return;
	if (nvram_nmatch("disabled", "%s_mode", prefix))
		return;
	char buf[128];
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\">%s</div>\n", tran_string(buf, sizeof(buf), "share.acktiming"));
	websWrite(wp, "<span id=\"wl_ack\">\n");
	ej_update_acktiming(wp, argc, argv);
	websWrite(wp, "</span> &nbsp;\n");
	websWrite(wp, "</div>\n");
}

extern long long wifi_getrate(char *ifname);

#define KILO 1000
#define MEGA 1000000
#define GIGA 1000000000

EJ_VISIBLE void ej_get_currate(webs_t wp, int argc, char_t **argv)
{
	char mode[32];
	char *ifname = nvram_safe_get("wifi_display");
	int state = get_radiostate(ifname);
	if (state == 0 || state == -1) {
		websWrite(wp, "%s", live_translate(wp, "share.disabled"));
		return;
	}
	long long rate = wifi_getrate(ifname);
	char scale;
	long long divisor;

	if (rate >= MEGA) {
		scale = 'M';
		divisor = MEGA;
	} else {
		scale = 'k';
		divisor = KILO;
	}
	sprintf(mode, "%s_channelbw", ifname);
	if (!is_mac80211(ifname)) {
		if (nvram_matchi(mode, 40))
			rate *= 2;
	}
	if (rate > 0) {
		long long ext = (rate % divisor) / 100000;
		if (ext)
			websWrite(wp, "%lld.%lld %cbit/s", rate / divisor, ext, scale);
		else
			websWrite(wp, "%lld %cbit/s", rate / divisor, scale);
	} else
		websWrite(wp, "%s", live_translate(wp, "share.auto"));
}

EJ_VISIBLE void ej_get_curchannel(webs_t wp, int argc, char_t **argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	char base[32];
	strncpy(base, prefix, 32);
	strchr(base, '.');
	int channel = wifi_getchannel(base);
	if (channel >= 0 && channel < 1000) {
		struct wifi_interface *interface = wifi_getfreq(base);
		int width = nvram_ngeti("%s_channelbw", base);
		if (!interface) {
			websWrite(wp, "%s", live_translate(wp, "share.unknown"));
			return;
		}

		int freq = get_wififreq(base,
					interface->freq); // translation for special frequency devices
		if (is_mac80211(base)) {
			websWrite(wp, "%d", ieee80211_mhz2ieee(interface->freq));
			if (interface->center1 != -1 && interface->center1 != interface->freq)
				websWrite(wp, " + %d", ieee80211_mhz2ieee(interface->center1));
			if (interface->center2 != -1 && interface->center1 != interface->freq)
				websWrite(wp, " + %d", ieee80211_mhz2ieee(interface->center2));
		} else {
			websWrite(wp, "%d", channel);
		}
		websWrite(wp, " (%d MHz", freq);
		char *vht = "HT";

		char *netmode = nvram_nget("%s_net_mode", base);
		if (has_qam256(base) && freq < 4000 && nvram_nmatch("1", "%s_turbo_qam", base)) {
			vht = "VHT";
		} else if (freq < 4000) {
			vht = "HT";
		} else if (has_ac(base)) {
			if (!strcmp(netmode, "acn-mixed") || //
			    !strcmp(netmode, "ac-only") || //
			    !strcmp(netmode, "ax-only") || //
			    !strcmp(netmode, "xacn-mixed") || //
			    !strcmp(netmode, "mixed")) {
				vht = "VHT";
			}
		}

		if (has_ax(base)) {
			if (!strcmp(netmode, "xacn-mixed") || //
			    !strcmp(netmode, "ax-only") || //
			    !strcmp(netmode, "axg-only") || //
			    !strcmp(netmode, "mixed")) {
				vht = "HE";
			}
		}

		if (is_mac80211(base)) {
			int ht = has_ht(base);
			switch (interface->width) {
			case 10:
			case 5:
			case 2:
				websWrite(wp, " NOHT");
				break;
			case 20:

				if (ht) {
					if (width == 2)
						websWrite(wp, " %s2.5", vht);
					else if (width == 5)
						websWrite(wp, " %s5", vht);
					else if (width == 10)
						websWrite(wp, " %s10", vht);
					else
						websWrite(wp, " %s20", vht);
				} else {
					/* for legacy chipsets we use the old naming */
					if (width == 2)
						websWrite(wp, " Eighth");
					else if (width == 5)
						websWrite(wp, " Quarter");
					else if (width == 10)
						websWrite(wp, " Half");
					else
						websWrite(wp, " LEGACY");
				}
				break;
			case 40:
				if (ht)
					websWrite(wp, " %s40", vht);
				else
					websWrite(wp,
						  " Turbo"); //ath5k turbo mode
				break;
			case 80:
				websWrite(wp, " %s80", vht);
				break;
			case 8080:
				websWrite(wp, " %s80+80", vht);
				break;
			case 160:
				websWrite(wp, " %s160", vht);
				break;
			}
		}
		websWrite(wp, ")");
		debug_free(interface);

	} else
		websWrite(wp, "%s", live_translate(wp, "share.unknown"));
	return;
}

EJ_VISIBLE void ej_active_wds(webs_t wp, int argc, char_t **argv)
{
}

EJ_VISIBLE void ej_get_low_2ghz(webs_t wp, int argc, char_t **argv)
{
	//	if (is_ath5k("wlan0") || is_ath5k("wlan1"))
	//		websWrite(wp, "2192");
	websWrite(wp, "2312");
}

EJ_VISIBLE void ej_get_high_2ghz(webs_t wp, int argc, char_t **argv)
{
	if (has_ar900b("wlan0") || has_ar900b("wlan1")) {
		websWrite(wp, "2492"); // tested on habanero
		return;
	}
	websWrite(wp,
		  "2500"); //some may be able to go up to 2732, but this seems only to affect older ath5k chipsets from our tests
}

EJ_VISIBLE void ej_get_high_5ghz(webs_t wp, int argc, char_t **argv)
{
	if (has_ar900b("wlan0") || has_ar900b("wlan1")) {
		websWrite(wp, "6115"); // tested on habanero
		return;
	}
	if ((is_ath10k("wlan0") && has_wave2("wlan0")) || (is_ath10k("wlan1") && has_wave2("wlan1"))) {
		websWrite(wp, "6180"); // need to find out the real maximum which is way higher than 7 ghz
		return;
	}
	if ((is_ath10k("wlan0")) || (is_ath10k("wlan1"))) {
		websWrite(wp, "6395"); // tested limit for qca988x
		return;
	}

	websWrite(wp, "6100");
}

EJ_VISIBLE void ej_get_low_5ghz(webs_t wp, int argc, char_t **argv)
{
	//	if (is_ath5k("wlan0") || is_ath5k("wlan1"))
	//		websWrite(wp, "4900");
	websWrite(wp, "4800");
}
#endif
