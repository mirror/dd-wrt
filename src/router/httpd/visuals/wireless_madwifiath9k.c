/*
 * wireless_madwifiath9k.c 
 *
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
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
#ifdef HAVE_ATH9K
#define VISUALSOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
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

#include <broadcom.h>
#include <bcmparams.h>
#include <utils.h>
#include <wlutils.h>
#include "wireless_generic.c"
#include <nl80211.h>
#include <unl.h>
#include <net/if.h>
#include <dd_list.h>

int active_wireless_if_ath9k(webs_t wp, int argc, char_t **argv, char *ifname, int *cnt, int globalcnt, int turbo, int macmask)
{
	char mac[32];
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	int divider = 1;
	char nb[32];
	int bias, qual, div = 1, mul = 1;
	snprintf(nb, sizeof(nb), "%s_bias", ifname);
	bias = nvram_default_geti(nb, 0);
	snprintf(nb, sizeof(nb), "%s_channelbw", ifname);
	int channelbw = atoi(nvram_default_get(nb, "0"));
	if (is_ath5k(ifname)) {
		if (channelbw == 40)
			mul = 2;
	}
	if (channelbw == 10)
		div = 2;
	if (channelbw == 5)
		div = 4;
	if (channelbw == 2)
		div = 8;
	// sprintf(it, "inactivity_time", ifname);
	//      it = nvram_default_geti("inacttime", 300000);

	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		char tmp[64];
		snprintf(tmp, sizeof(tmp), "%s.sta", ifname);
		if (strcmp(wc->ifname, ifname) && strncmp(wc->ifname, tmp, strlen(tmp)))
			continue;
		ether_etoa(wc->etheraddr, mac);
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
		int signal = wc->signal;
		if (signal >= -50)
			qual = 1000;
		else if (signal <= -100)
			qual = 0;
		else
			qual = (signal + 100) * 20;
		if (globalcnt)
			websWrite(wp, ",");
		int ht = 0;
		int sgi = 0;
		int vht = 0;
		int he = 0;
		char info[32];
		if (!wc->rx_is_ht && !wc->is_ht)
			ht = 8;
		if (wc->rx_is_40mhz || wc->is_40mhz)
			ht = 1;
		if (wc->rx_is_80mhz || wc->is_80mhz)
			ht = 2;
		if (wc->rx_is_160mhz || wc->is_160mhz)
			ht = 3;
		if (wc->rx_is_80p80mhz || wc->is_80p80mhz)
			ht = 4;
		if (wc->rx_is_vht || wc->is_vht)
			vht = 1;
		if (wc->rx_is_he || wc->is_he)
			he = 1;
		if (wc->rx_is_short_gi || wc->is_short_gi)
			sgi = 1;
		if (ht == 8 && sgi)
			ht = 0;
		if (ht == 8 && (vht || he))
			ht = 0;
		if (ht == 8)
			strcpy(info, "LEGACY");
		else
			strcpy(info, he ? "HE" : vht ? "VHT" : "HT");
		if (div == 2)
			ht = 7;
		if (div == 4)
			ht = 6;
		if (div == 8)
			ht = 5;
		char *bwinfo[] = { "20", "40", "80", "160", "80+80", "2.5", "5", "10" };
		if (ht < 8 && ht >= 0)
			snprintf(info, sizeof(info), "%s%s", info, bwinfo[ht]);
		if (sgi)
			snprintf(info, sizeof(info), "%s%s", info, "SGI");
		if (wc->islzo)
			snprintf(info, sizeof(info), "%s %s", info, "LZ");
		if (wc->ht40intol)
			snprintf(info, sizeof(info), "%s[ht40i]", info);
		if (wc->ps)
			snprintf(info, sizeof(info), "%s[PS]", info);
		char str[64] = { 0 };
		char *radioname = wc->radioname;
		if (!*(radioname))
			radioname = "";
		websWrite(wp, "'%s','%s','%s','%s','%dM','%dM','%s','%d','%d','%d','%d','%d','%d','%d','%d','%s','%s'", mac,
			  radioname, wc->ifname, UPTIME(wc->uptime, str, sizeof(str)), wc->txrate / 10 * mul / div,
			  wc->rxrate / 10 * mul / div, info, wc->signal + bias, wc->noise + bias, wc->signal - wc->noise, qual,
			  wc->chaininfo_avg[0], wc->chaininfo_avg[1], wc->chaininfo_avg[2], wc->chaininfo_avg[3],
			  nvram_nget("%s_label", wc->ifname), wc->ifname);
		*cnt = (*cnt) + 1;
		globalcnt++;
		//              }
	}
	free_wifi_clients(mac80211_info->wci);
	debug_free(mac80211_info);
	return globalcnt;
}

EJ_VISIBLE void ej_get_busy(webs_t wp, int argc, char_t **argv)
{
	struct mac80211_info info;
	char *prefix = nvram_safe_get("wifi_display");
	if (has_nolivesurvey(prefix))
		return;
	if (is_mac80211(prefix)) {
		if (nvram_nmatch("disabled", "%s_net_mode", prefix))
			return;
		if (getcurrentsurvey_mac80211(prefix, &info)) {
			unsigned long long busy = info.channel_busy_time;
			websWrite(wp, "%llu ms", busy);
		}
	}
}

EJ_VISIBLE void ej_get_active(webs_t wp, int argc, char_t **argv)
{
	struct mac80211_info info;
	char *prefix = nvram_safe_get("wifi_display");
	if (has_nolivesurvey(prefix))
		return;
	if (is_mac80211(prefix)) {
		if (nvram_nmatch("disabled", "%s_net_mode", prefix))
			return;
		if (getcurrentsurvey_mac80211(prefix, &info)) {
			unsigned long long active = info.channel_active_time;
			websWrite(wp, "%llu ms", active);
		}
	}
}

EJ_VISIBLE void ej_get_quality(webs_t wp, int argc, char_t **argv)
{
	struct mac80211_info info;
	char *prefix = nvram_safe_get("wifi_display");
	if (has_nolivesurvey(prefix))
		return;
	if (is_mac80211(prefix)) {
		if (nvram_nmatch("disabled", "%s_net_mode", prefix))
			return;
		if (getcurrentsurvey_mac80211(prefix, &info)) {
			long long active = info.channel_active_time;
			long long busy = info.channel_busy_time;
			long long quality = 100;
			if (active > 0) {
				quality = 100 - ((busy * 100) / active);
			}
			websWrite(wp, "%llu%%", quality < 0 ? 0 : quality);
		}
	}
}

EJ_VISIBLE void ej_show_busy(webs_t wp, int argc, char_t **argv)
{
	struct mac80211_info info;
	char *prefix = nvram_safe_get("wifi_display");
	if (has_nolivesurvey(prefix))
		return;
	if (is_mac80211(prefix)) {
		if (nvram_nmatch("disabled", "%s_net_mode", prefix))
			return;
		if (nvram_nmatch("disabled", "%s_mode", prefix))
			return;
		if (getcurrentsurvey_mac80211(prefix, &info)) {
			long long active = info.channel_active_time;
			long long busy = info.channel_busy_time;
			if (busy != -1) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(
					wp,
					"<div class=\"label\"><script type=\"text/javascript\">Capture(status_wireless.busy)</script></div>\n");
				websWrite(wp, "<span id=\"wl_busy\">%llu ms</span>&nbsp;\n", busy);
				websWrite(wp, "</div>\n");
			}
			if (active != -1) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(
					wp,
					"<div class=\"label\"><script type=\"text/javascript\">Capture(status_wireless.active)</script></div>\n");
				websWrite(wp, "<span id=\"wl_active\">%llu ms</span>&nbsp;\n", active);
				websWrite(wp, "</div>\n");
			}
			if (busy != -1 && active != -1 && active > 0) {
				long long quality = 100;
				if (active > 0)
					quality = 100 - ((busy * 100) / active);
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(
					wp,
					"<div class=\"label\"><script type=\"text/javascript\">Capture(status_wireless.quality)</script></div>\n");
				websWrite(wp, "<span id=\"wl_quality\">%llu%%</span>&nbsp;\n", quality < 0 ? 0 : quality);
				websWrite(wp, "</div>\n");
			}
		}
	}
}

EJ_VISIBLE void ej_dump_channel_survey(webs_t wp, int argc, char_t **argv)
{
	struct frequency *f, *ftmp;
	int first_survey = 0;
	char *interface = nvram_safe_get("wifi_display");
	DD_LIST_HEAD(frequencies);
	if (getsurveystats(&frequencies, NULL, interface, NULL, 2, 20))
		return;
	dd_list_for_each_entry(f, &frequencies, list)
	{
		if (!f->active_count && !f->busy_count && !f->noise_count)
			continue;
		if (f->in_use)
			websWrite(wp, "%c\"[%d]\"", !first_survey ? ' ' : ',', f->freq);
		else
			websWrite(wp, "%c\"%d\"", !first_survey ? ' ' : ',', f->freq);
		websWrite(wp, ",\"%d\"", ieee80211_mhz2ieee(f->freq));
		first_survey = 1;
		if (f->noise_count)
			websWrite(wp, ",\"%d\"", f->noise / f->noise_count);
		else
			websWrite(wp, ",\"-95\"");
		if (f->active_count && f->busy_count)
			websWrite(wp, ",\"%lld\"", 100 - ((f->busy * 100) / f->active));
		else
			websWrite(wp, ",\"\"");
		if (f->active_count)
			websWrite(wp, ",\"%lld\"", f->active);
		else
			websWrite(wp, ",\"\"");
		if (f->busy_count)
			websWrite(wp, ",\"%lld\"", f->busy);
		else
			websWrite(wp, ",\"\"");
		if (f->rx_time_count)
			websWrite(wp, ",\"%lld\"", f->rx_time);
		else
			websWrite(wp, ",\"\"");
		if (f->tx_time_count)
			websWrite(wp, ",\"%lld\"\n", f->tx_time);
		else
			websWrite(wp, ",\"\"\n");
	}
	dd_list_for_each_entry_safe(f, ftmp, &frequencies, list)
	{
		dd_list_del(&f->list);
		debug_free(f);
	}
}

EJ_VISIBLE void ej_channel_survey(webs_t wp, int argc, char_t **argv)
{
	if (has_channelsurvey(nvram_safe_get("wifi_display"))) {
		if (nvram_nmatch("disabled", "%s_net_mode", nvram_safe_get("wifi_display")))
			return;
		websWrite(
			wp,
			"document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" name=\\\"channel_survey\\\" value=\\\"\" + sbutton.csurvey + \"\\\" onclick=\\\"OpenChannelSurvey()\\\" />\");\n");
	}
}

#endif
