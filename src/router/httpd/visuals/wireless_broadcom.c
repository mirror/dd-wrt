/*
 * wireless_broadcom.c
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

typedef struct wl_rateset3 {
	uint32 count; /* # rates in this set */
	uint8 rates[16]; /* rates in 500 kbit/s units w/hi bit set if basic */
} wl_rateset3_t;

#ifdef WL_STA_ANT_MAX
typedef struct {
	uint16 ver; /* version of this struct */
	uint16 len; /* length in bytes of this structure */
	uint16 cap; /* sta's advertised capabilities */
	uint32 flags; /* flags defined below */
	uint32 idle; /* time since data pkt rx'd from sta */
	struct ether_addr ea; /* Station address */
	wl_rateset3_t rateset; /* rateset in use */
	uint32 in; /* seconds elapsed since associated */
	uint32 listen_interval_inms; /* Min Listen interval in ms for this STA */
	uint32 tx_pkts; /* # of user packets transmitted (unicast) */
	uint32 tx_failures; /* # of user packets failed */
	uint32 rx_ucast_pkts; /* # of unicast packets received */
	uint32 rx_mcast_pkts; /* # of multicast packets received */
	uint32 tx_rate; /* Rate used by last tx frame */
	uint32 rx_rate; /* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds; /* # of packet decrypted successfully */
	uint32 rx_decrypt_failures; /* # of packet decrypted unsuccessfully */
	uint32 tx_tot_pkts; /* # of user tx pkts (ucast + mcast) */
	uint32 rx_tot_pkts; /* # of data packets recvd (uni + mcast) */
	uint32 tx_mcast_pkts; /* # of mcast pkts txed */
	uint64 tx_tot_bytes; /* data bytes txed (ucast + mcast) */
	uint64 rx_tot_bytes; /* data bytes recvd (ucast + mcast) */
	uint64 tx_ucast_bytes; /* data bytes txed (ucast) */
	uint64 tx_mcast_bytes; /* # data bytes txed (mcast) */
	uint64 rx_ucast_bytes; /* data bytes recvd (ucast) */
	uint64 rx_mcast_bytes; /* data bytes recvd (mcast) */
	int8 rssi[WL_STA_ANT_MAX]; /* per antenna rssi */
	int8 nf[WL_STA_ANT_MAX]; /* per antenna noise floor */
	uint16 aid; /* association ID */
	uint16 ht_capabilities; /* advertised ht caps */
	uint16 vht_flags; /* converted vht flags */
	uint32 tx_pkts_retry_cnt; /* # of frames where a retry was
					 * necessary (obsolete)
					 */
	uint32 tx_pkts_retry_exhausted; /* # of user frames where a retry
					 * was exhausted
					 */
	int8 rx_lastpkt_rssi[WL_STA_ANT_MAX]; /* Per antenna RSSI of last
						 * received data frame.
						 */
	/* TX WLAN retry/failure statistics:
	 * Separated for host requested frames and WLAN locally generated frames.
	 * Include unicast frame only where the retries/failures can be counted.
	 */
	uint32 tx_pkts_total; /* # user frames sent successfully */
	uint32 tx_pkts_retries; /* # user frames retries */
	uint32 tx_pkts_fw_total; /* # FW generated sent successfully */
	uint32 tx_pkts_fw_retries; /* # retries for FW generated frames */
	uint32 tx_pkts_fw_retry_exhausted; /* # FW generated where a retry
						 * was exhausted
						 */
	uint32 rx_pkts_retried; /* # rx with retry bit set */
	uint32 tx_rate_fallback; /* lowest fallback TX rate */
} sta_info_compat4_t;

#endif
typedef struct {
	uint16 ver; /* version of this struct */
	uint16 len; /* length in bytes of this structure */
	uint16 cap; /* sta's advertised capabilities */
	uint32 flags; /* flags defined below */
	uint32 idle; /* time since data pkt rx'd from sta */
	struct ether_addr ea; /* Station address */
	wl_rateset3_t rateset; /* rateset in use */
	uint32 in; /* seconds elapsed since associated */
	uint32 listen_interval_inms; /* Min Listen interval in ms for this STA */
	uint32 tx_pkts; /* # of packets transmitted */
	uint32 tx_failures; /* # of packets failed */
	uint32 rx_ucast_pkts; /* # of unicast packets received */
	uint32 rx_mcast_pkts; /* # of multicast packets received */
	uint32 tx_rate; /* Rate of last successful tx frame */
	uint32 rx_rate; /* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds; /* # of packet decrypted successfully */
	uint32 rx_decrypt_failures; /* # of packet decrypted unsuccessfully */
} sta_info_compat3_t;

typedef struct wl_rateset2_compat {
	uint32 count; /* # rates in this set */
	uint8 rates[255]; /* rates in 500 kbit/s units w/hi bit set if basic */
} wl_rateset2_compat_t;

typedef struct {
	uint16 ver; /* version of this struct */
	uint16 len; /* length in bytes of this structure */
	uint16 cap; /* sta's advertised capabilities */
	uint32 flags; /* flags defined below */
	uint32 idle; /* time since data pkt rx'd from sta */
	struct ether_addr ea; /* Station address */
	wl_rateset2_compat_t rateset; /* rateset in use */
	uint32 in; /* seconds elapsed since associated */
	uint32 listen_interval_inms; /* Min Listen interval in ms for this STA */
	uint32 tx_pkts; /* # of packets transmitted */
	uint32 tx_failures; /* # of packets failed */
	uint32 rx_ucast_pkts; /* # of unicast packets received */
	uint32 rx_mcast_pkts; /* # of multicast packets received */
	uint32 tx_rate; /* Rate of last successful tx frame */
	uint32 rx_rate; /* Rate of last successful rx frame */
	uint32 rx_decrypt_succeeds; /* # of packet decrypted successfully */
	uint32 rx_decrypt_failures; /* # of packet decrypted unsuccessfully */
} sta_info_compat2_t;

#define RSSI_TMP "/tmp/.rssi"
#define ASSOCLIST_CMD "wl assoclist"
#define RSSI_CMD "wl rssi"
#define NOISE_CMD "wl noise"

int active_wireless_if(webs_t wp, int argc, char_t **argv, char *iface, char *visible, int *cnt, int globalcnt)
{
	int rssi = 0, noise = 0;
	char *mode;
	char mac[30];
	char line[80];
	int macmask;
	macmask = atoi(argv[0]);
	if (strcmp(iface, "qtn") && !ifexists(iface))
		return globalcnt;
	char wlmode[32];
	char *displayname = iface;
	if (!strncmp(displayname, "eth", 3))
		displayname = visible;
	sprintf(wlmode, "%s_mode", visible);
	mode = nvram_safe_get(wlmode);
	unsigned char buf[WLC_IOCTL_MAXLEN];
	int ht = 0;
	int sgi = 0;
	int vht = 0;
	int i40 = 0;
	int chain_rssi[4] = { 0, 0, 0, 0 };

	bzero(buf, WLC_IOCTL_MAXLEN); // get_wdev
	int r;
#ifdef HAVE_QTN
	if (has_qtn(iface))
		r = getassoclist_qtn(iface, buf);
	else
#endif
		r = getassoclist(iface, buf);

	if (r < 0)
		return globalcnt;
	struct maclist *maclist = (struct maclist *)buf;
	int i;

	for (i = 0; i < maclist->count; i++) {
		ether_etoa((uint8 *)&maclist->ea[i], mac);

		rssi = 0;
		noise = 0;
		// get rssi value
		char rxrate[32];
		char txrate[32];
		char time[32];
		strcpy(rxrate, "N/A");
		strcpy(txrate, "N/A");
		strcpy(time, "N/A");
#ifdef HAVE_QTN
		if (has_qtn(iface)) {
			rssi = getRssiIndex_qtn(iface, i);
			noise = getNoiseIndex_qtn(iface, i);
			int rx = getRXRate_qtn(iface, i);
			int tx = getTXRate_qtn(iface, i);
			if (tx > 0)
				sprintf(txrate, "%dM", tx);

			if (rx > 0)
				sprintf(rxrate, "%dM", rx);
		} else {
#endif
			char cmd[64];
			if (strcmp(mode, "ap") && strcmp(mode, "apsta") && strcmp(mode, "apstawet"))
				rssi = getRssi(iface, NULL);
			else
				rssi = getRssi(iface, mac);
			noise = getNoise(iface, NULL);
#ifdef HAVE_QTN
		}
#endif
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
		if (globalcnt)
			websWrite(wp, ",");
		*cnt = (*cnt) + 1;
		globalcnt++;
		char info[32];
		strcpy(info, "N/A");
#ifdef HAVE_QTN
		if (!has_qtn(iface)) {
#endif
#ifndef WL_STA_SCBSTATS
#define WL_STA_SCBSTATS 0x4000 /* Per STA debug stats */
#endif
#ifdef WL_STA_ANT_MAX
			sta_info_compat4_t *sta4;
#endif
			sta_info_compat3_t *sta3;
			sta_info_compat2_t *sta2;
			char *param;
			int buflen;
			char buf[WLC_IOCTL_MEDLEN];
			strcpy(buf, "sta_info");
			buflen = strlen(buf) + 1;
			param = (char *)(buf + buflen);
			memcpy(param, (char *)&maclist->ea[i], ETHER_ADDR_LEN);
			char str[64] = { 0 };
			if (!wl_ioctl(iface, WLC_GET_VAR, &buf[0], WLC_IOCTL_MEDLEN)) {
				/* display the sta info */
				sta2 = (sta_info_compat2_t *)buf;
				switch (sta2->ver) {
				case 2:

					sta2 = (sta_info_compat2_t *)buf;
					if (sta2->flags & WL_STA_SCBSTATS) {
						int tx = sta2->tx_rate;
						int rx = sta2->rx_rate;
						if (tx > 0)
							sprintf(txrate, "%dM", tx / 1000);

						if (rx > 0)
							sprintf(rxrate, "%dM", rx / 1000);
						strcpy(time, UPTIME(sta2->in, str, sizeof(str)));
					}
					break;
				case 3:
					sta3 = (sta_info_compat3_t *)buf;
					if (sta3->flags & WL_STA_SCBSTATS) {
						int tx = sta3->tx_rate;
						int rx = sta3->rx_rate;
						if (tx > 0)
							sprintf(txrate, "%dM", tx / 1000);

						if (rx > 0)
							sprintf(rxrate, "%dM", rx / 1000);
						strcpy(time, UPTIME(sta3->in, str, sizeof(str)));
					}
					sprintf(info, "LEGACY");
					if (sta3->flags & WL_STA_N_CAP)
						sprintf(info, "HT20");
					if (sta3->flags & WL_STA_PS)
						strcat(info, "PS");
					break;
#ifdef WL_STA_ANT_MAX
				case 4:
				case 5:
					sta4 = (sta_info_compat4_t *)buf;
					if (sta4->flags & WL_STA_SCBSTATS) {
						int tx = sta4->tx_rate;
						int rx = sta4->rx_rate;
						if (tx > 0)
							sprintf(txrate, "%dM", tx / 1000);

						if (rx > 0)
							sprintf(rxrate, "%dM", rx / 1000);
						strcpy(time, UPTIME(sta4->in, str, sizeof(str)));
					}
					chain_rssi[0] = sta4->rssi[0];
					chain_rssi[1] = sta4->rssi[1];
					chain_rssi[2] = sta4->rssi[2];
					chain_rssi[3] = sta4->rssi[3];
					info[0] = 0;
					ht = 0;
					sgi = 0;
					vht = 0;
					i40 = 0;
					if (sta4->flags & WL_STA_N_CAP) {
						ht = 1;
						if (sta4->ht_capabilities) {
							if (sta4->ht_capabilities & WL_STA_CAP_40MHZ)
								ht = 2;
							if (sta4->ht_capabilities & WL_STA_CAP_SHORT_GI_20)
								sgi = 1;
							if (sta4->ht_capabilities & WL_STA_CAP_SHORT_GI_40) {
								sgi = 1;
								ht = 2;
							}
							if (sta4->ht_capabilities & WL_STA_CAP_40MHZ_INTOLERANT) {
								i40 = 1;
							}
						}
					} else {
						ht = 0;
					}

					if (sta4->flags & WL_STA_VHT_CAP) {
						vht = 1;
						if (sta4->vht_flags & WL_STA_SGI80) {
							sgi = 1;
							ht = 3;
						}
						if (sta4->vht_flags & WL_STA_SGI160) {
							sgi = 1;
							ht = 4;
						}
					}
					if (vht)
						sprintf(info, "VHT");
					else
						sprintf(info, "HT");

					switch (ht) {
					case 0:
						sprintf(info, "LEGACY");
						break;
					case 1:
						strcat(info, "20");
						break;
					case 2:
						strcat(info, "40");
						break;
					case 3:
						strcat(info, "80");
						break;
					case 4:
						strcat(info, "160");
						break;
					}
					if (sgi)
						strcat(info, "SGI");
					if (i40)
						strcat(info, "[HT40i]");
					if (sta4->flags & WL_STA_PS)
						strcat(info, "[PS]");
					break;
#endif
				}
			}
#ifdef HAVE_QTN
		}
#endif

		/*
		 * if (!strcmp (mode, "ap")) { noise = getNoise(iface,NULL); // null
		 * only for broadcom }
		 */
		int signal = rssi;
		int qual = 0;
		if (signal >= -50)
			qual = 1000;
		else if (signal <= -100)
			qual = 0;
		else
			qual = (signal + 100) * 20;
		websWrite(wp, "'%s','','%s','%s','%s','%s','%s','%d','%d','%d','%d','%d','%d','%d','%d','%s','%s'", mac,
			  displayname, time, txrate, rxrate, info, rssi, noise, rssi - noise, qual, chain_rssi[0], chain_rssi[1],
			  chain_rssi[2], chain_rssi[3], nvram_nget("%s_label", iface), iface);
	}

	return globalcnt;
}

EJ_VISIBLE void ej_assoc_count(webs_t wp, int argc, char_t **argv)
{
	assoc_count_prefix(wp, "wl");
}

EJ_VISIBLE void ej_active_wireless(webs_t wp, int argc, char_t **argv)
{
	int c = get_wl_instances();
	int i;
	int cnt = 0;
	int global = 0;
	memset(assoc_count, 0, sizeof(assoc_count));

	for (i = 0; i < c; i++) {
		char wlif[32];

		sprintf(wlif, "wl%d", i);
		global = active_wireless_if(wp, argc, argv, get_wl_instance_name(i), wlif, &assoc_count[cnt], global);
		cnt++;
		char *next;
		char var[80];
		char *vifs = nvram_nget("wl%d_vifs", i);

		if (vifs == NULL)
			return;

		foreach(var, vifs, next)
		{
			global = active_wireless_if(wp, argc, argv, var, var, &assoc_count[cnt], global);
			cnt++;
		}
	}
}

EJ_VISIBLE void ej_get_currate(webs_t wp, int argc, char_t **argv)
{
	int rate = 0;
	char name[32];

	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));
	char *ifname = nvram_safe_get(name);

	wl_ioctl(ifname, WLC_GET_RATE, &rate, sizeof(rate));

	if (rate > 0)
		websWrite(wp, "%d%s Mbit/s", (rate / 2), (rate & 1) ? ".5" : "");
	else
		websWrite(wp, "%s", live_translate(wp, "share.unknown"));

	return;
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
	channel_info_t ci;
	char name[32];

	char *prefix = nvram_safe_get("wifi_display");
	sprintf(name, "%s_ifname", prefix);
	char *ifname = nvram_safe_get(name);
	wl_bss_info_t *bi;
	char buf[WLC_IOCTL_MAXLEN + 4];
	*(unsigned int *)&buf[0] = WLC_IOCTL_MAXLEN;
	bzero(&ci, sizeof(ci));
	wl_ioctl(ifname, WLC_GET_CHANNEL, &ci, sizeof(ci));
	wl_ioctl(ifname, WLC_GET_BSS_INFO, &buf[0], WLC_IOCTL_MAXLEN);
	bi = (wl_bss_info_t *)(&buf[4]);

	if (ci.scan_channel > 0) {
		websWrite(wp, "%d (scanning)", ci.scan_channel);
	} else {
		if (ci.hw_channel == 0) {
			websWrite(wp, "%s", live_translate(wp, "share.unknown"));
			return;
		}
		switch ((bi->chanspec & 0x3800)) {
		case 0:
		case 0x800:
		case 0x1000:
			websWrite(wp, "%d", bi->chanspec & 0xff);
			break;
		case 0x0C00: // for older version
		case 0x1800:
		case 0x2000:
		case 0x2800:
		case 0x3000:
			if (!bi->ctl_ch)
				websWrite(wp, "%d", bi->chanspec & 0xff);
			else
				websWrite(wp, "%d + %d", bi->ctl_ch, bi->chanspec & 0xff);
		}
	}
	return;
}

#define WDS_RSSI_TMP "/tmp/.rssi"
int internal_ej_active_wds_instance(webs_t wp, int argc, char_t **argv, int instance, int cnt);
EJ_VISIBLE void ej_active_wds(webs_t wp, int argc, char_t **argv)
{
	int cnt = 0;
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		cnt = internal_ej_active_wds_instance(wp, argc, argv, i, cnt);
	}
}

int internal_ej_active_wds_instance(webs_t wp, int argc, char_t **argv, int instance, int cnt)
{
	int rssi = 0, i;
	char *mode;
	char mac[30];
	char line[80];

	// char title[30];
	char wdsvar[30];
	char desc[30];
	int macmask;
	macmask = atoi(argv[0]);

	mode = nvram_nget("wl%d_mode", instance);

	if (strcmp(mode, "ap") && strcmp(mode, "apsta") && strcmp(mode, "apstawet"))
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
		ether_etoa((uint8 *)&maclist->ea[e], mac);

		rssi = 0;
		bzero(desc, 30);
		for (i = 1; i <= 10; i++) {
			snprintf(wdsvar, sizeof(wdsvar), "wl%d_wds%d_hwaddr", instance, i);
			if (nvram_match(wdsvar, mac)) {
				snprintf(wdsvar, sizeof(wdsvar), "wl%d_wds%d_desc", instance, i);
				snprintf(desc, sizeof(desc), "%s", nvram_safe_get(wdsvar));
				if (!strcmp(nvram_safe_get(wdsvar), ""))
					strcpy(desc, "&nbsp;");
			}
		}

		rssi = getRssi(iface, mac);

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
		if (cnt)
			websWrite(wp, ",");
		cnt++;
		int noise = getNoise(iface, NULL);

		websWrite(wp, "\"%s\",\"%s\",\"%s\",\"%d\",\"%d\",\"%d\"", mac, iface, desc, rssi, noise, rssi - noise);
	}

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

#if 0
/* Writes "1" if Tx beamforming is supported. Otherwise, "0" */
int ej_wl_txbf_capable(webs_t wp, int argc, char_t ** argv)
{
	char *name = NULL;
	char tmp[NVRAM_BUFSIZE], prefix[] = "wlXXXXXXXXXX_";
	int txbf_capable = 0;
	wlc_rev_info_t revinfo;
	char *ifname = argv[0];

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
#endif
