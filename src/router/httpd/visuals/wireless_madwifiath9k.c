/*
 * wireless_madwifiath9k.c 
 *
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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
int ej_active_wireless_if_ath9k(webs_t wp, int argc, char_t ** argv, char *ifname, int cnt, int turbo, int macmask)
{
	char mac[32];
	struct mac80211_info *mac80211_info;
	struct wifi_client_info *wc;
	char nb[32];
	int bias, qual, it, div = 1, mul = 1;
	int co = 0;
	sprintf(nb, "%s_bias", ifname);
	bias = nvram_default_geti(nb, 0);

	if (is_ath5k(ifname)) {
		sprintf(nb, "%s_channelbw", ifname);
		int channelbw = atoi(nvram_default_get(nb, "0"));
		if (channelbw == 40)
			mul = 2;
		if (channelbw == 10)
			div = 2;
		if (channelbw == 5)
			div = 4;
	}
	// sprintf(it, "inactivity_time", ifname);
//      it = nvram_default_geti("inacttime", 300000);

	mac80211_info = mac80211_assoclist(ifname);
	for (wc = mac80211_info->wci; wc; wc = wc->next) {
		strncpy(mac, wc->mac, 31);
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

		if (cnt)
			websWrite(wp, ",");

		int ht = 0;
		int sgi = 0;
		int vht = 0;
		char info[32];

		if (!wc->rx_is_ht)
			ht = 5;
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
		if (wc->rx_is_short_gi || wc->is_short_gi)
			sgi = 1;

		if (ht == 5 && sgi)
			ht = 0;
		if (ht == 5 && vht)
			ht = 0;
		if (ht == 5)
			strcpy(info, "LEGACY");
		else
			strcpy(info, vht ? "VHT" : "HT");
		char *bwinfo[] = { "20", "40", "80", "160", "80+80" };
		if (ht < 5 && ht >= 0)
			sprintf(info, "%s%s", info, bwinfo[ht]);
		if (wc->ht40intol)
			sprintf(info, "%si", info);
		if (sgi)
			sprintf(info, "%s%s", info, "SGI");
		if (wc->islzo)
			sprintf(info, "%s %s", info, "LZ");
		char str[64] = { 0 };

		websWrite(wp, "'%s','%s','%s','%dM','%dM','%s','%d','%d','%d','%d'", mac, wc->ifname, UPTIME(wc->uptime, str), wc->txrate / 10 * mul / div, wc->rxrate / 10 * mul / div, info, wc->signal + bias,
			  wc->noise + bias, wc->signal - wc->noise, qual);
		cnt++;
//              }
	}
	free_wifi_clients(mac80211_info->wci);
	free(mac80211_info);
	return cnt;
}

void ej_get_busy(webs_t wp, int argc, char_t ** argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	if (is_ath9k(prefix)) {
		unsigned long long busy = getBusy_mac80211(prefix);
		websWrite(wp, "%llu ms", busy);
	}
}

void ej_get_active(webs_t wp, int argc, char_t ** argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	if (is_ath9k(prefix)) {
		unsigned long long active = getActive_mac80211(prefix);
		websWrite(wp, "%llu ms", active);
	}
}

void ej_get_quality(webs_t wp, int argc, char_t ** argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	if (is_ath9k(prefix)) {
		unsigned long long active = getActive_mac80211(prefix);
		unsigned long long busy = getBusy_mac80211(prefix);
		unsigned long long quality = 100 - ((busy * 100) / active);
		websWrite(wp, "%llu%%", quality);
	}
}

void ej_show_busy(webs_t wp, int argc, char_t ** argv)
{
	char *prefix = nvram_safe_get("wifi_display");
	if (is_ath9k(prefix)) {
		unsigned long long busy = getBusy_mac80211(prefix);
		unsigned long long active = getActive_mac80211(prefix);
		if (busy != (unsigned long long)(-1)) {
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(status_wireless.busy)</script></div>\n");
			websWrite(wp, "<span id=\"wl_busy\">%llu ms</span>&nbsp;\n", busy);
			websWrite(wp, "</div>\n");
		}
		if (active != (unsigned long long)(-1)) {
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(status_wireless.active)</script></div>\n");
			websWrite(wp, "<span id=\"wl_active\">%llu ms</span>&nbsp;\n", active);
			websWrite(wp, "</div>\n");
		}
		if (active != (unsigned long long)(-1) && busy != (unsigned long long)(-1)) {
			unsigned long long quality = 100 - ((busy * 100) / active);
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(status_wireless.quality)</script></div>\n");
			websWrite(wp, "<span id=\"wl_quality\">%llu%%</span>&nbsp;\n", quality);
			websWrite(wp, "</div>\n");
		}
	}
}

static struct nla_policy survey_policy[NL80211_SURVEY_INFO_MAX + 1] = {
	[NL80211_SURVEY_INFO_FREQUENCY] = {.type = NLA_U32},
	[NL80211_SURVEY_INFO_NOISE] = {.type = NLA_U8},
	[NL80211_SURVEY_INFO_CHANNEL_TIME] = {.type = NLA_U64},
	[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY] = {.type = NLA_U64},
};

static int parse_survey(struct nl_msg *msg, struct nlattr **sinfo)
{
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[NL80211_ATTR_SURVEY_INFO])
		return -1;

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX, tb[NL80211_ATTR_SURVEY_INFO], survey_policy))
		return -1;

	if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY])
		return -1;

	return 0;
}

struct survey_data {
	int first_survey;
	webs_t wp;
};

static int cb_survey(struct nl_msg *msg, void *data)
{
	struct survey_data *d = (struct survey_data *)data;
	struct nlattr *sinfo[NL80211_SURVEY_INFO_MAX + 1];
	struct nlattr *tb[NL80211_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	int freq;
	int noise = -1;
	int64_t active = -1;
	int64_t busy = -1;
	int64_t rx_time = -1;
	int64_t tx_time = -1;
	int64_t quality = -1;

	nla_parse(tb, NL80211_ATTR_MAX, genlmsg_attrdata(gnlh, 0), genlmsg_attrlen(gnlh, 0), NULL);
	if (!tb[NL80211_ATTR_SURVEY_INFO])
		return NL_SKIP;

	if (nla_parse_nested(sinfo, NL80211_SURVEY_INFO_MAX, tb[NL80211_ATTR_SURVEY_INFO], survey_policy))
		return NL_SKIP;

	if (!sinfo[NL80211_SURVEY_INFO_FREQUENCY])
		return NL_SKIP;

	freq = nla_get_u32(sinfo[NL80211_SURVEY_INFO_FREQUENCY]);
	if (sinfo[NL80211_SURVEY_INFO_NOISE]) {
		noise = nla_get_u8(sinfo[NL80211_SURVEY_INFO_NOISE]);
	}
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME])
		active = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME]);
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY])
		busy = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_BUSY]);

	if (active > 0)
		quality = 100 - ((busy * 100) / active);

	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX])
		rx_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_RX]);
	if (sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX])
		tx_time = nla_get_u64(sinfo[NL80211_SURVEY_INFO_CHANNEL_TIME_TX]);

	if (sinfo[NL80211_SURVEY_INFO_IN_USE])
		websWrite(d->wp, "%c\"[%d]\"", !d->first_survey ? ' ' : ',', freq);
	else
		websWrite(d->wp, "%c\"%d\"", !d->first_survey ? ' ' : ',', freq);

	websWrite(d->wp, ",\"%d\"", ieee80211_mhz2ieee(freq));
	d->first_survey = 1;

	if (noise != -1)
		websWrite(d->wp, ",\"%d\"", noise);
	else
		websWrite(d->wp, ",\"N/A\"");

	if (quality != -1)
		websWrite(d->wp, ",\"%d\"", quality);
	else
		websWrite(d->wp, ",\"N/A\"");

	if (active != -1)
		websWrite(d->wp, ",\"%d\"", active);
	else
		websWrite(d->wp, ",\"N/A\"");

	if (busy != -1)
		websWrite(d->wp, ",\"%d\"", busy);
	else
		websWrite(d->wp, ",\"N/A\"");

	if (rx_time != -1)
		websWrite(d->wp, ",\"%d\"", rx_time);
	else
		websWrite(d->wp, ",\"N/A\"");

	if (tx_time != -1)
		websWrite(d->wp, ",\"%d\"\n", tx_time);
	else
		websWrite(d->wp, ",\"N/A\"\n");

out:
	return NL_SKIP;
}

void ej_dump_channel_survey(webs_t wp, int argc, char_t ** argv)
{
	struct nl_msg *surveymsg;
	struct unl unl;
	struct survey_data data;
	int wdev;
	data.first_survey = 0;
	data.wp = wp;
	char *interface = nvram_safe_get("wifi_display");
	wdev = if_nametoindex(interface);
	eval("iw", "dev", interface, "scan");
	unl_genl_init(&unl, "nl80211");
	surveymsg = unl_genl_msg(&unl, NL80211_CMD_GET_SURVEY, true);
	NLA_PUT_U32(surveymsg, NL80211_ATTR_IFINDEX, wdev);
	unl_genl_request(&unl, surveymsg, cb_survey, &data);
	unl_free(&unl);
	return;

nla_put_failure:
	nlmsg_free(surveymsg);
	unl_free(&unl);
	return;

}
#endif
