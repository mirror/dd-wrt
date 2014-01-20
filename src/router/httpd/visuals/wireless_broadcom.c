/*
 * wireless_broadcom.c
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

typedef struct wl_rateset_compat {
	uint32 count;		/* # rates in this set */
	uint8 rates[16];	/* rates in 500kbps units w/hi bit set if basic */
} wl_rateset_compat_t;

typedef struct {
	uint16 ver;		/* version of this struct */
	uint16 len;		/* length in bytes of this structure */
	uint16 cap;		/* sta's advertised capabilities */
	uint32 flags;		/* flags defined below */
	uint32 idle;		/* time since data pkt rx'd from sta */
	struct ether_addr ea;	/* Station address */
	wl_rateset_compat_t rateset;	/* rateset in use */
	uint32 in;		/* seconds elapsed since associated */
	uint32 listen_interval_inms;	/* Min Listen interval in ms for this STA */
	uint32 tx_pkts;		/* # of packets transmitted */
	uint32 tx_failures;	/* # of packets failed */
	uint32 rx_ucast_pkts;	/* # of unicast packets received */
	uint32 rx_mcast_pkts;	/* # of multicast packets received */
	uint32 tx_rate;		/* Rate of last successful tx frame */
	uint32 rx_rate;		/* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds;	/* # of packet decrypted successfully */
	uint32 rx_decrypt_failures;	/* # of packet decrypted unsuccessfully */
} sta_info_compat_t;

typedef struct wl_rateset_old_compat {
	uint32 count;		/* # rates in this set */
	uint8 rates[255];	/* rates in 500kbps units w/hi bit set if basic */
} wl_rateset_compat_old_t;

typedef struct {
	uint16 ver;		/* version of this struct */
	uint16 len;		/* length in bytes of this structure */
	uint16 cap;		/* sta's advertised capabilities */
	uint32 flags;		/* flags defined below */
	uint32 idle;		/* time since data pkt rx'd from sta */
	struct ether_addr ea;	/* Station address */
	wl_rateset_compat_old_t rateset;	/* rateset in use */
	uint32 in;		/* seconds elapsed since associated */
	uint32 listen_interval_inms;	/* Min Listen interval in ms for this STA */
	uint32 tx_pkts;		/* # of packets transmitted */
	uint32 tx_failures;	/* # of packets failed */
	uint32 rx_ucast_pkts;	/* # of unicast packets received */
	uint32 rx_mcast_pkts;	/* # of multicast packets received */
	uint32 tx_rate;		/* Rate of last successful tx frame */
	uint32 rx_rate;		/* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds;	/* # of packet decrypted successfully */
	uint32 rx_decrypt_failures;	/* # of packet decrypted unsuccessfully */
} sta_info_compat_old_t;

#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"

int ej_active_wireless_if(webs_t wp, int argc, char_t ** argv, char *iface, char *visible, int cnt)
{
	int rssi = 0, noise = 0;
	FILE *fp2;
	char *mode;
	char mac[30];
	char line[80];
	int macmask;
	macmask = atoi(argv[0]);
	if (!ifexists(iface))
		return cnt;
	unlink(RSSI_TMP);
	char wlmode[32];

	sprintf(wlmode, "%s_mode", visible);
	mode = nvram_safe_get(wlmode);
	unsigned char buf[WLC_IOCTL_MAXLEN];

	memset(buf, 0, WLC_IOCTL_MAXLEN);	// get_wdev
	int r = getassoclist(iface, buf);

	if (r < 0)
		return cnt;
	struct maclist *maclist = (struct maclist *)buf;
	int i;

	for (i = 0; i < maclist->count; i++) {
		ether_etoa((uint8 *) & maclist->ea[i], mac);

		rssi = 0;
		noise = 0;
		// get rssi value
		if (strcmp(mode, "ap") && strcmp(mode, "apsta")
		    && strcmp(mode, "apstawet"))
			sysprintf("wl -i %s rssi > %s", iface, RSSI_TMP);
		else
			sysprintf("wl -i %s rssi \"%s\" > %s", iface, mac, RSSI_TMP);

		// get noise value if not ap mode
		// if (strcmp (mode, "ap"))
		// snprintf (cmd, sizeof (cmd), "wl -i %s noise >> %s", iface,
		// RSSI_TMP);
		// system2 (cmd); // get RSSI value for mac

		fp2 = fopen(RSSI_TMP, "r");
		if (fgets(line, sizeof(line), fp2) != NULL) {

			// get rssi
			if (sscanf(line, "%d", &rssi) != 1)
				continue;
			noise = getNoise(iface, NULL);
			/*
			 * if (strcmp (mode, "ap") && fgets (line, sizeof (line), fp2) != 
			 * NULL && sscanf (line, "%d", &noise) != 1) continue;
			 */
			// get noise for client/wet mode

			fclose(fp2);
		}
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
		if (cnt)
			websWrite(wp, ",");
		cnt++;
		int rxrate[32];
		int txrate[32];
		int time[32];
		strcpy(rxrate, "N/A");
		strcpy(txrate, "N/A");
		strcpy(time, "N/A");
#ifndef WL_STA_SCBSTATS
#define WL_STA_SCBSTATS		0x4000	/* Per STA debug stats */
#endif
		sta_info_compat_t *sta;
		sta_info_compat_old_t *staold;
		char *param;
		int buflen;
		char buf[WLC_IOCTL_MEDLEN];
		strcpy(buf, "sta_info");
		buflen = strlen(buf) + 1;
		param = (char *)(buf + buflen);
		memcpy(param, (char *)&maclist->ea[i], ETHER_ADDR_LEN);
		if (!wl_ioctl(iface, WLC_GET_VAR, &buf[0], WLC_IOCTL_MEDLEN)) {
			/* display the sta info */
			sta = (sta_info_compat_t *) buf;
			if (sta->ver == 2) {
				staold = (sta_info_compat_old_t *) buf;
				if (staold->flags & WL_STA_SCBSTATS) {
					int tx = staold->tx_rate;
					int rx = staold->rx_rate;
					if (tx > 0)
						sprintf(txrate, "%dM", tx / 1000);

					if (rx > 0)
						sprintf(rxrate, "%dM", rx / 1000);
					strcpy(time, UPTIME(staold->in));
				}
			} else {	// sta->ver == 3
				if (sta->flags & WL_STA_SCBSTATS) {
					int tx = sta->tx_rate;
					int rx = sta->rx_rate;
					if (tx > 0)
						sprintf(txrate, "%dM", tx / 1000);

					if (rx > 0)
						sprintf(rxrate, "%dM", rx / 1000);
					strcpy(time, UPTIME(sta->in));
				}
			}
		}

		/*
		 * if (!strcmp (mode, "ap")) { noise = getNoise(iface,NULL); // null
		 * only for broadcom }
		 */
		int qual = rssi * 124 + 11600;
		qual /= 10;
		websWrite(wp, "'%s','%s','%s','%s','%s','%d','%d','%d','%d'", mac, iface, time, txrate, rxrate, rssi, noise, rssi - noise, qual);
	}
	unlink(RSSI_TMP);

	return cnt;
}

void ej_active_wireless(webs_t wp, int argc, char_t ** argv)
{
	int cnt = 0;
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char wlif[32];

		sprintf(wlif, "wl%d", i);
		cnt = ej_active_wireless_if(wp, argc, argv, get_wl_instance_name(i), wlif, cnt);
		char *next;
		char var[80];
		char *vifs = nvram_nget("wl%d_vifs", i);

		if (vifs == NULL)
			return;

		foreach(var, vifs, next) {
			cnt = ej_active_wireless_if(wp, argc, argv, var, var, cnt);
		}
	}
}

void ej_get_currate(webs_t wp, int argc, char_t ** argv)
{
	int rate = 0;
	char name[32];

	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));
	char *ifname = nvram_safe_get(name);

	wl_ioctl(ifname, WLC_GET_RATE, &rate, sizeof(rate));

	if (rate > 0)
		websWrite(wp, "%d%s Mbps", (rate / 2), (rate & 1) ? ".5" : "");
	else
		websWrite(wp, "%s", live_translate("share.unknown"));

	return;
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
	channel_info_t ci;
	char name[32];

	char *prefix = nvram_safe_get("wifi_display");
	sprintf(name, "%s_ifname", prefix);
	char *ifname = nvram_safe_get(name);

	memset(&ci, 0, sizeof(ci));
	wl_ioctl(ifname, WLC_GET_CHANNEL, &ci, sizeof(ci));
	if (ci.scan_channel > 0) {
		websWrite(wp, "%d (scanning)", ci.scan_channel);
	} else if (ci.hw_channel > 0) {
		if (has_mimo(prefix)
		    && (nvram_nmatch("n-only", "%s_net_mode", prefix)
			|| nvram_nmatch("mixed", "%s_net_mode", prefix)
			|| nvram_nmatch("na-only", "%s_net_mode", prefix)
			|| nvram_nmatch("n2-only", "%s_net_mode", prefix)
			|| nvram_nmatch("n5-only", "%s_net_mode", prefix)
			|| nvram_nmatch("ac-only", "%s_net_mode", prefix)
			|| nvram_nmatch("ng-only", "%s_net_mode", prefix))
		    && (nvram_nmatch("ap", "%s_mode", prefix)
			|| nvram_nmatch("wdsap", "%s_mode", prefix)
			|| nvram_nmatch("infra", "%s_mode", prefix))) {
			if (nvram_nmatch("40", "%s_nbw", prefix)) {
				websWrite(wp, "%d + ", nvram_nmatch("upper", "%s_nctrlsb", prefix) ? ci.hw_channel + 2 : ci.hw_channel - 2);
			}
			if (nvram_nmatch("80", "%s_nbw", prefix)) {
				int channel = ci.hw_channel - 6;
				if (nvram_nmatch("ll", "%s_nctrlsb", prefix))
					websWrite(wp, "%d + ", channel);
				if (nvram_nmatch("lu", "%s_nctrlsb", prefix))
					websWrite(wp, "%d + ", channel + 4);
				if (nvram_nmatch("ul", "%s_nctrlsb", prefix))
					websWrite(wp, "%d + ", channel + 8);
				if (nvram_nmatch("uu", "%s_nctrlsb", prefix))
					websWrite(wp, "%d + ", channel + 12);

			}
		}
		if (nvram_nmatch("40", "%s_nbw", prefix)) {
			websWrite(wp, "%d", nvram_nmatch("upper", "%s_nctrlsb", prefix) ? ci.hw_channel - 2 : ci.hw_channel + 2);
		} else {
			websWrite(wp, "%d", ci.hw_channel);
		}
	} else
		// websWrite (wp, "unknown");
		websWrite(wp, "%s", live_translate("share.unknown"));
	return;

}

#define WDS_RSSI_TMP	"/tmp/.rssi"
int ej_active_wds_instance(webs_t wp, int argc, char_t ** argv, int instance, int cnt);
void ej_active_wds(webs_t wp, int argc, char_t ** argv)
{
	int cnt = 0;
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		cnt = ej_active_wds_instance(wp, argc, argv, i, cnt);
	}
}

int ej_active_wds_instance(webs_t wp, int argc, char_t ** argv, int instance, int cnt)
{
	int rssi = 0, i;
	FILE *fp2;
	char *mode;
	char mac[30];
	char line[80];

	// char title[30];
	char wdsvar[30];
	char desc[30];
	int macmask;

	ejArgs(argc, argv, "%d", &macmask);
	unlink(WDS_RSSI_TMP);

	mode = nvram_nget("wl%d_mode", instance);

	if (strcmp(mode, "ap") && strcmp(mode, "apsta")
	    && strcmp(mode, "apstawet"))
		return cnt;
	unsigned char buf[WLC_IOCTL_MAXLEN];
	char *iface = get_wl_instance_name(instance);

	if (!ifexists(iface))
		return cnt;
	int r = getwdslist(iface, buf);

	if (r < 0)
		return cnt;
	struct maclist *maclist = (struct maclist *)buf;
	int e;

	for (e = 0; e < maclist->count; e++) {

		ether_etoa((uint8 *) & maclist->ea[e], mac);

		rssi = 0;
		memset(desc, 0, 30);
		for (i = 1; i <= 10; i++) {
			snprintf(wdsvar, 30, "wl%d_wds%d_hwaddr", instance, i);
			if (nvram_match(wdsvar, mac)) {
				snprintf(wdsvar, 30, "wl%d_wds%d_desc", instance, i);
				snprintf(desc, sizeof(desc), "%s", nvram_get(wdsvar));
				if (!strcmp(nvram_get(wdsvar), ""))
					strcpy(desc, "&nbsp;");
			}
		}

		sysprintf("wl -i %s rssi \"%s\" > %s", iface, mac, RSSI_TMP);

		fp2 = fopen(RSSI_TMP, "r");
		if (fgets(line, sizeof(line), fp2) != NULL) {

			// get rssi
			if (sscanf(line, "%d", &rssi) != 1)
				continue;
			fclose(fp2);
		}
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
		if (cnt)
			websWrite(wp, ",");
		cnt++;
		int noise = getNoise(iface, NULL);

		websWrite(wp, "\"%s\",\"%s\",\"%s\",\"%d\",\"%d\",\"%d\"", mac, iface, desc, rssi, noise, rssi - noise);
	}

	unlink(WDS_RSSI_TMP);
	return cnt;
}

static unsigned int bits_count(unsigned int n)
{
	unsigned int count = 0;

	while (n > 0) {
		if (n & 1)
			count++;
		n >>= 1;
	}

	return count;
}

/* Writes "1" if Tx beamforming is supported. Otherwise, "0" */
int ej_wl_txbf_capable(webs_t wp, int argc, char_t ** argv)
{
	char *name = NULL;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	int txbf_capable = 0;
	wlc_rev_info_t revinfo;
	char *ifname;

	ejArgs(argc, argv, "%s", &ifname);

	name = nvram_nget("%s_ifname", ifname);

	/* Get revision info */
	wl_ioctl(name, WLC_GET_REVINFO, &revinfo, sizeof(revinfo));

	/*
	 * Beamforming is available on core revs >= 40. Currently, 1-2
	 * streams have beamforming.
	 */
	if (revinfo.corerev >= 40) {
		int txchain;

		if (wl_iovar_getint(name, "txchain", &txchain))
			return -1;

		if (bits_count((unsigned int)txchain) > 1) {
			txbf_capable = 1;
		}
	}

	return websWrite(wp, "%d", txbf_capable);
}
