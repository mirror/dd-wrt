/*
 * setupassistant.c
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
#ifdef HAVE_BUFFALO

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>
#include <wlutils.h>

void sas_show_wep(webs_t wp, char *prefix);
void sas_show_preshared(webs_t wp, char *prefix);
void sas_show_radius(webs_t wp, char *prefix, int showmacformat, int backup);
void sas_show_netmode(webs_t wp, char *prefix);
void sas_show_channel(webs_t wp, char *dev, char *prefix, int type);
EJ_VISIBLE void ej_sas_show_wireless_single(webs_t wp, char *prefix);
void sas_show_security_single(webs_t wp, int argc, char_t **argv, char *prefix);
void sas_show_security_prefix(webs_t wp, int argc, char_t **argv, char *prefix,
			      int primary);

void sas_show_wpa_setting(webs_t wp, int argc, char_t **argv, char *prefix,
			  char *security_prefix);

char *nvram_selget(webs_t wp, char *name)
{
	if (wp->gozila_action) {
		char *buf = GOZILA_GET(wp, name);

		if (buf) {
			return buf;
			//              return sprintf("%s", buf);
		}
	}
	return nvram_safe_get(name);
}

int nvram_selnmatch(webs_t wp, char *match, const char *fmt, ...)
{
	char varbuf[64];
	va_list args;
	va_start(args, (char *)fmt);
	vsnprintf(varbuf, sizeof(varbuf), fmt, args);
	va_end(args);
	return nvram_selmatch(wp, varbuf, match);
}

static char *selmatch(webs_t wp, char *var, char *is, char *ret)
{
	if (nvram_selmatch(wp, var, is))
		return ret;
	return "";
}

static char *sas_nvram_prefix_get(webs_t wp, const char *name,
				  const char *prefix)
{
	char p[64];
	sprintf(p, "%s_%s", prefix, name);
	return nvram_selget(wp, p);
}

static int sas_nvram_prefix_match(webs_t wp, const char *name,
				  const char *prefix, char *match)
{
	char p[64];
	sprintf(p, "%s_%s", prefix, name);
	return nvram_selmatch(wp, p, match);
}

static char *sas_nvram_nget(webs_t wp, const char *fmt, ...)
{
	char varbuf[64];
	va_list args;
	va_start(args, (char *)fmt);
	vsnprintf(varbuf, sizeof(varbuf), fmt, args);
	va_end(args);
	return nvram_selget(wp, varbuf);
}

static char *sas_nvram_default_get(webs_t wp, char *var, char *def)
{
	if (wp->gozila_action) {
		char *buf = GOZILA_GET(wp, var);
		if (buf) {
			return buf;
		}
	}
	return nvram_default_get(var, def);
	/*char *v = nvram_get(var);
	   if (v == NULL || strlen(v) == 0) {
	   nvram_set(var, def);
	   return def;
	   }
	   return nvram_selget(wp, var); */
}

static int sas_nvram_default_match(webs_t wp, char *var, char *match, char *def)
{
	fprintf(stderr, "[sas_nvram_default_match] %s\n", var);
	if (wp->gozila_action) {
		fprintf(stderr, "[sas_nvram_default_match] %s gozila\n", var);
		char *buf = GOZILA_GET(wp, var);
		if (buf) {
			fprintf(stderr,
				"[sas_nvram_default_match] %s: %s - %s\n", var,
				buf, match);
			return !strcmp(buf, match);
		}
	}
	return nvram_default_match(var, match, def);
	/*char *v = nvram_get(var);
	   if (v == NULL || strlen(v) == 0) {
	   nvram_set(var, def);
	   return !strcmp(match, def);
	   }
	   return nvram_selmatch(wp, var, match); */
}

EJ_VISIBLE void ej_sas_nvram_checked(webs_t wp, int argc, char_t **argv)
{
	if (nvram_selmatch(wp, argv[0], argv[1])) {
		websWrite(wp, "checked=\"checked\"");
	}

	return;
}

EJ_VISIBLE void ej_sas_nvc(webs_t wp, int argc, char_t **argv)
{
	ej_sas_nvram_checked(wp, argc, argv);
	return;
}

EJ_VISIBLE void ej_sas_make_time_list(webs_t wp, int argc, char_t **argv)
{
	int i, st, en;
	char ic[16];

	st = atoi(argv[1]);
	en = atoi(argv[2]);

	for (i = st; i <= en; i++) {
		sprintf(ic, "%d", i);
		websWrite(wp, "<option value=\"%d\" %s >%02d</option>\n", i,
			  nvram_selmatch(wp, argv[0], ic) ?
				  "selected=\"selected\"" :
				  "",
			  i);
	}

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_else_match("wan_proto", "dhcp", "0","1"); %> produces "0"
 * <% nvram_else_match("wan_proto", "static", "0","1"); %> produces "1"
 */
EJ_VISIBLE void ej_sas_nvram_else_match(webs_t wp, int argc, char_t **argv)
{
	if (nvram_selmatch(wp, argv[0], argv[1])) {
		websWrite(wp, argv[2]);
	} else {
		websWrite(wp, argv[3]);
	}
	return;
}

EJ_VISIBLE void ej_sas_nvem(webs_t wp, int argc, char_t **argv)
{
	ej_sas_nvram_else_match(wp, argc, argv);
	return;
}

EJ_VISIBLE void ej_show_sas_stage(webs_t wp, int argc, char_t **argv)
{
	do_ej(METHOD_GET, NULL, "sas_stage_1.asp", wp);
}

EJ_VISIBLE void ej_show_sas_wan_setting(webs_t wp, int argc, char_t **argv)
{
	char *type;

	type = GOZILA_GET(wp, "wan_proto");
	if (type == NULL)
		type = nvram_safe_get("wan_proto");
	char ejname[32];
	snprintf(ejname, 31, "sas_%s.asp", type);
	do_ej(METHOD_GET, NULL, ejname, wp);
}

char *ej_get_sas_stage(webs_t wp, int argc, char_t **argv)
{
	char *stage;

	stage = GOZILA_GET(wp, "sas_stage");

	if (stage == NULL)
		stage = "0";

	if (atoi(stage) == 0) {
		stage = "1";
	}

	return stage;
}

EJ_VISIBLE void ej_visible_css(webs_t wp, int argc, char_t **argv)
{
	if (strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, "display: none;");
	}
}

EJ_VISIBLE void ej_print_sas_stage(webs_t wp, int argc, char_t **argv)
{
	char *stage;
	stage = ej_get_sas_stage(wp, argc, argv);
	websWrite(wp, stage);
}

int internal_ej_sas_stage_is_visible(webs_t wp, int argc, char_t **argv)
{
	char *stage;
	stage = ej_get_sas_stage(wp, argc, argv);
	if (strcmp(stage, argv[0])) {
		return 1;
	} else {
		return 0;
	}
}

EJ_VISIBLE void ej_sas_stage_visible_css(webs_t wp, int argc, char_t **argv)
{
	if (internal_ej_sas_stage_is_visible(wp, argc, argv) == 1) {
		websWrite(wp, "display: none");
	}
}

EJ_VISIBLE void ej_do_sas_stage_menu(webs_t wp, int argc, char_t **argv)
{
	int i;
	static char labels[5][32] = { "", "sas.internet_connection",
				      "sas.network_settings",
				      "sas.wireless_settings",
				      "sas.other_settings" };
	char *stage;

	stage = ej_get_sas_stage(wp, argc, argv);

	websWrite(wp, "<div id=\"sas_menu\">\n");
	websWrite(wp, "<div class=\"wrapper\">\n");
	websWrite(wp, "<ul>\n");
	for (i = 1; i < 5; i++) {
		if (i + 1 < 5) {
			if (atoi(stage) == i) {
				websWrite(
					wp,
					"<li class=\"active\"><span><strong class=\"step_%s\">%s</strong></span></li>\n",
					stage,
					live_translate(wp,
						       labels[atoi(stage)]));
			} else {
				websWrite(
					wp,
					"<li><span><strong class=\"step_%i\">%s</strong></span></li>\n",
					i, live_translate(wp, labels[i]));
			}
		} else {
			if (atoi(stage) == i) {
				websWrite(
					wp,
					"<li class=\"active last\"><span class=\"last\"><strong class=\"step_%s\">%s</strong></span></li>\n",
					stage,
					live_translate(wp,
						       labels[atoi(stage)]));
			} else {
				websWrite(
					wp,
					"<li class=\"last\"><span class=\"last\"><strong class=\"step_%i\">%s</strong></span></li>\n",
					i, live_translate(wp, labels[i]));
			}
		}
	}
	websWrite(wp, "</ul>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");
}

EJ_VISIBLE void ej_sas_show_wireless(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_MADWIFI
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wlan%d", i);
		ej_sas_show_wireless_single(wp, buf);
		sas_show_security_single(wp, argc, argv, buf);
	}
#else
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wl%d", i);
		ej_sas_show_wireless_single(wp, buf);
		sas_show_security_single(wp, argc, argv, buf);
	}
#endif
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% get_single_ip("lan_ipaddr","1"); %> produces "168"
 */
char *sas_get_single_ip(webs_t wp, char *label, int position)
{
	char *c;
	char name[32];
	char d[32];
	char *g;

	if (wp->gozila_action) {
		sprintf(name, "%s_%i", label, position);
		g = GOZILA_GET(wp, name);
		fprintf(stderr, "[sas_get_single_ip] %s %s %i\n", name, g,
			position);
		if (g) {
			return g;
		} else {
			return "0";
		}
	}

	c = nvram_selget(wp, label);
	if (c) {
		if (!strcmp(c, PPP_PSEUDO_IP) || !strcmp(c, PPP_PSEUDO_GW))
			c = "0.0.0.0";
		else if (!strcmp(c, PPP_PSEUDO_NM))
			c = "255.255.255.0";

		sprintf(d, "%i", get_single_ip(c, position));
		return d;
	} else {
		return "0";
	}
}

EJ_VISIBLE void ej_sas_get_single_ip(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, sas_get_single_ip(wp, argv[0], atoi(argv[1])));
	return;
}

EJ_VISIBLE void ej_sas_get_single_nm(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, sas_get_single_ip(wp, argv[0], atoi(argv[1])));
}

char *sas_get_dns_ip(webs_t wp, char *label, int entry, int position)
{
	int which;
	char name[32];
	char word[256], *next;
	char d[32];
	char *g;

	if (wp->gozila_action) {
		sprintf(name, "%s%i_%i", label, entry, position);
		g = GOZILA_GET(wp, name);
		if (g) {
			return g;
		}
	}

	which = entry;
	char *list = nvram_safe_get(label);

	foreach(word, list, next)
	{
		if (which-- == 0) {
			sprintf(d, "%i", get_single_ip(word, position));
			return d;
		}
	}

	return "0";
}

/*
 * Example: wan_dns = 168.95.1.1 210.66.161.125 168.95.192.1 <%
 * get_dns_ip("wan_dns", "1", "2"); %> produces "161" <%
 * get_dns_ip("wan_dns", "2", "3"); %> produces "1" 
 */
EJ_VISIBLE void ej_sas_get_dns_ip(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s",
		  sas_get_dns_ip(wp, argv[0], atoi(argv[1]), atoi(argv[2])));
}

EJ_VISIBLE void ej_sas_show_wireless_single(webs_t wp, char *prefix)
{
	char wl_mode[16];
	char wl_macaddr[18];
	char wl_ssid[16];
	char power[16];
#ifdef HAVE_REGISTER
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif

#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

	int argc = 1;
	char *argv[] = { "3" };
	char *stage_visible_css = "display: none;";
	char frequencies[128];

	if (ej_sas_stage_is_visible(wp, argc, argv) == 0) {
		stage_visible_css = "";
	}

	sprintf(wl_mode, "%s_mode", prefix);
	sprintf(wl_macaddr, "%s_hwaddr", prefix);
	sprintf(wl_ssid, "%s_ssid", prefix);

	// check the frequency capabilities;
	if (has_2ghz(prefix) && has_5ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies, " [2.4 GHz/5 GHz/802.11ac]");
	} else if (has_5ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies, " [5 GHz/802.11ac]");
	} else if (has_5ghz(prefix) && has_2ghz(prefix)) {
		sprintf(frequencies, " [2.4 GHz/5 GHz]");
	} else if (has_5ghz(prefix)) {
		sprintf(frequencies, " [5 GHz]");
	} else if (has_2ghz(prefix) && has_ac(prefix)) {
		sprintf(frequencies, " [2.4 GHz TurboQAM]");
	} else if (has_2ghz(prefix)) {
		sprintf(frequencies, " [2.4 GHz]");
	} else {
		frequencies[0] = 0;
	}

	// wireless mode
	websWrite(
		wp,
		"<h2 style=\"%s\"><script type=\"text/javascript\">Capture(wl_basic.h2_v24)</script> %s%s</h2>\n",
		stage_visible_css, prefix, frequencies);
	websWrite(wp, "<fieldset style=\"%s\">\n", stage_visible_css);
	websWrite(
		wp,
		"<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s - SSID [",
		IFMAP(prefix));
	tf_webWriteESCNV(wp, wl_ssid); // fix
	websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(wl_macaddr));

	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" onchange=\"refresh(this.form);\">\n",
		wl_mode);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	if (!cpeonly) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");\n",
			nvram_selmatch(wp, wl_mode, "ap") ?
				"selected=\\\"selected\\\"" :
				"");
	}
#if (!defined(HAVE_RT61) && !defined(HAVE_DIR860)) || defined(HAVE_MT76)
	websWrite(
		wp,
		"document.write(\"<option value=\\\"sta\\\" %s >\" + wl_basic.client + \"</option>\");\n",
		nvram_selmatch(wp, wl_mode, "sta") ?
			"selected=\\\"selected\\\"" :
			"");
#endif
#ifndef HAVE_RT2880
#ifdef HAVE_RELAYD
	websWrite(
		wp,
		"document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientRelayd + \"</option>\");\n",
#else
	websWrite(
		wp,
		"document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientBridge + \"</option>\");\n",
#endif
		nvram_selmatch(wp, wl_mode, "wet") ?
			"selected=\\\"selected\\\"" :
			"");
#endif
	if (!cpeonly)
		websWrite(
			wp,
			"document.write(\"<option value=\\\"infra\\\" %s >\" + wl_basic.adhoc + \"</option>\");\n",
			nvram_selmatch(wp, wl_mode, "infra") ?
				"selected=\\\"selected\\\"" :
				"");
#ifndef HAVE_MADWIFI
	if (!cpeonly) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"apsta\\\" %s >\" + wl_basic.repeater + \"</option>\");\n",
			nvram_selmatch(wp, wl_mode, "apsta") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"apstawet\\\" %s >\" + wl_basic.repeaterbridge + \"</option>\");\n",
			nvram_selmatch(wp, wl_mode, "apstawet") ?
				"selected=\\\"selected\\\"" :
				"");
	}
#else
	websWrite(
		wp,
		"document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");\n",
		nvram_selmatch(wp, wl_mode, "wdssta") ?
			"selected=\\\"selected\\\"" :
			"");
	if (!cpeonly && has_wdsap(prefix)) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");\n",
			nvram_selmatch(wp, wl_mode, "wdsap") ?
				"selected=\\\"selected\\\"" :
				"");
	}
#endif
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

// RELAYD OPTIONAL SETTINGS
#ifdef HAVE_RELAYD
	if (nvram_selmatch(wp, wl_mode, "wet")) {
		char wl_relayd[32];
		int ip[4] = { 0, 0, 0, 0 };
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.clientRelaydDefaultGwMode)</script></div>");
		sprintf(wl_relayd, "%s_relayd_gw_auto", prefix);
		nvram_default_get(wl_relayd, "1");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_relayd_gw_auto\" onclick=\"show_layer_ext(this, '%s_relayd_gw_ipaddr', false)\" %s /><script type=\"text/javascript\">Capture(share.auto)</script>&nbsp;(DHCP)&nbsp;\n",
			prefix, prefix,
			nvram_selmatch(wp, wl_relayd, "1") ? "checked" : "");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_relayd_gw_auto\" onclick=\"show_layer_ext(this, '%s_relayd_gw_ipaddr', true)\" %s/><script type=\"text/javascript\">Capture(share.manual)</script>\n",
			prefix, prefix,
			nvram_selmatch(wp, wl_relayd, "0") ? "checked" : "");
		websWrite(wp, "</div>\n");

		sprintf(wl_relayd, "%s_relayd_gw_ipaddr", prefix);
		sscanf(nvram_safe_get(wl_relayd), "%d.%d.%d.%d", &ip[0], &ip[1],
		       &ip[2], &ip[3]);
		sprintf(wl_relayd, "%s_relayd_gw_auto", prefix);
		websWrite(
			wp,
			"<div id=\"%s_relayd_gw_ipaddr\" class=\"setting\"%s>\n"
			"<input type=\"hidden\" name=\"%s_relayd_gw_ipaddr\" value=\"4\">\n"
			"<div class=\"label\"><script type=\"text/javascript\">Capture(share.gateway)</script></div>\n"
			"<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_0\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_1\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_2\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">.<input size=\"3\" maxlength=\"3\" name=\"%s_relayd_gw_ipaddr_3\" value=\"%d\" onblur=\"valid_range(this,1,254,'IP')\" class=\"num\">\n"
			"</div>\n",
			prefix,
			nvram_selmatch(wp, wl_relayd, "1") ?
				" style=\"display: none; visibility: hidden;\"" :
				"",
			prefix, prefix, ip[0], prefix, ip[1], prefix, ip[2],
			prefix, ip[3]);
	}
#endif

	// writeless net mode
	sas_show_netmode(wp, prefix);
#ifdef HAVE_MADWIFI

	// char wl_xchanmode[16];
	char wl_outdoor[16];
	char wl_diversity[16];
	char wl_rxantenna[16];
	char wl_txantenna[16];
	char wl_width[16];
	char wl_preamble[16];
	char wl_xr[16];
	char wl_comp[32];
	char wl_ff[16];
	char wmm[32];
	char wl_isolate[32];
	char wl_intmit[32];
	char wl_noise_immunity[32];
	char wl_ofdm_weak_det[32];
	char wl_protmode[32];
	char wl_doth[32];
	char wl_csma[32];

	sprintf(wl_csma, "%s_csma", prefix);
	sprintf(wl_doth, "%s_doth", prefix);
	sprintf(wl_protmode, "%s_protmode", prefix);
	sprintf(wl_outdoor, "%s_outdoor", prefix);
	sprintf(wl_diversity, "%s_diversity", prefix);
	sprintf(wl_rxantenna, "%s_rxantenna", prefix);
	sprintf(wl_txantenna, "%s_txantenna", prefix);
	sprintf(wl_width, "%s_channelbw", prefix);
	//    sprintf( wl_comp, "%s_compression", prefix );
	sprintf(wl_ff, "%s_ff", prefix);
	sprintf(wl_preamble, "%s_preamble", prefix);
	sprintf(wl_xr, "%s_xr", prefix);

	sprintf(wl_intmit, "%s_intmit", prefix);
	sprintf(wl_noise_immunity, "%s_noise_immunity", prefix);
	sprintf(wl_ofdm_weak_det, "%s_ofdm_weak_det", prefix);

#if defined(HAVE_ATH9K)
	if (!is_mac80211(prefix))
#endif
	{
		showAutoOption(wp, "wl_basic.intmit", wl_intmit, 0);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.noise_immunity)</script></div>\n<select name=\"%s\">\n",
			wl_noise_immunity);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"0\\\" %s >0</option>\");\n",
			sas_nvram_default_match(wp, wl_noise_immunity, "0",
						"4") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"1\\\" %s >1</option>\");\n",
			sas_nvram_default_match(wp, wl_noise_immunity, "1",
						"4") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"2\\\" %s >2</option>\");\n",
			sas_nvram_default_match(wp, wl_noise_immunity, "2",
						"4") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"3\\\" %s >3</option>\");\n",
			sas_nvram_default_match(wp, wl_noise_immunity, "3",
						"4") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"4\\\" %s >4</option>\");\n",
			sas_nvram_default_match(wp, wl_noise_immunity, "4",
						"4") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(wp, "//]]>\n</script>\n</select>\n</div>\n");

		showRadio(wp, "wl_basic.ofdm_weak_det", wl_ofdm_weak_det);
	}

	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div><select name=\"%s\"  onchange=\"refresh(this.form);\">\n",
		wl_width);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");

#if defined(HAVE_ATH9K)
	fprintf(stderr, "[MADWIFI MIMO] %s\n", prefix);
	/* limit channel options by mode */
	if (is_mac80211(prefix)) {
		if ((nvram_selnmatch(wp, "n-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "ng-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "n2-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "mixed", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "n5-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "na-only", "%s_net_mode", prefix))) {
#if defined(HAVE_ATH9K)
			if (!is_mac80211(prefix) || can_ht40(prefix))
#endif
				websWrite(
					wp,
					"document.write(\"<option value=\\\"2040\\\" %s >\" + share.dynamicturbo + \"</option>\");\n",
					nvram_selmatch(wp, wl_width, "2040") ?
						"selected=\\\"selected\\\"" :
						"");
		}
	}
	if (!is_mac80211(prefix) ||
	    (is_mac80211(prefix) &&
	     (nvram_selnmatch(wp, "n-only", "%s_net_mode", prefix) ||
	      nvram_selnmatch(wp, "ng-only", "%s_net_mode", prefix) ||
	      nvram_selnmatch(wp, "n2-only", "%s_net_mode", prefix) ||
	      nvram_selnmatch(wp, "n5-only", "%s_net_mode", prefix) ||
	      nvram_selnmatch(wp, "mixed", "%s_net_mode", prefix) ||
	      nvram_selnmatch(wp, "na-only", "%s_net_mode", prefix))))
#endif
	{
#if defined(HAVE_ATH9K)
		if (!is_mac80211(prefix) || can_ht40(prefix))
#endif
			websWrite(
				wp,
				"document.write(\"<option value=\\\"40\\\" %s >\" + share.turbo + \"</option>\");\n",
				nvram_selmatch(wp, wl_width, "40") ?
					"selected=\\\"selected\\\"" :
					"");
	}
	websWrite(
		wp,
		"document.write(\"<option value=\\\"20\\\" %s >\" + share.full + \"</option>\");\n",
		nvram_selmatch(wp, wl_width, "20") ?
			"selected=\\\"selected\\\"" :
			"");
#if defined(HAVE_MADWIFI) || defined(HAVE_ATH9K)
	{
		websWrite(
			wp,
			"document.write(\"<option value=\\\"10\\\" %s >\" + share.half + \"</option>\");\n",
			nvram_selmatch(wp, wl_width, "10") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"5\\\" %s >\" + share.quarter + \"</option>\");\n",
			nvram_selmatch(wp, wl_width, "5") ?
				"selected=\\\"selected\\\"" :
				"");
		if (registered_has_subquarter()) {
			/* will be enabled once it is tested and the spectrum analyse is done */
			websWrite(
				wp,
				"document.write(\"<option value=\\\"2\\\" %s >\" + share.subquarter + \"</option>\");\n",
				nvram_selmatch(wp, wl_width, "2") ?
					"selected=\\\"selected\\\"" :
					"");
		}
	}
#endif
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
#if defined(HAVE_NS2) || defined(HAVE_NS5) || defined(HAVE_LC2) || \
	defined(HAVE_LC5) || defined(HAVE_NS3)

	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label24)</script></div><select name=\"%s\" >\n",
		wl_txantenna);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.vertical + \"</option>\");\n",
		nvram_selmatch(wp, wl_txantenna, "0") ?
			"selected=\\\"selected\\\"" :
			"");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.horizontal + \"</option>\");\n",
		nvram_selmatch(wp, wl_txantenna, "1") ?
			"selected=\\\"selected\\\"" :
			"");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"3\\\" %s >\" + wl_basic.adaptive + \"</option>\");\n",
		nvram_selmatch(wp, wl_txantenna, "3") ?
			"selected=\\\"selected\\\"" :
			"");
#if defined(HAVE_NS5) || defined(HAVE_NS2) || defined(HAVE_NS3)
	websWrite(
		wp,
		"document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.external + \"</option>\");\n",
		nvram_selmatch(wp, wl_txantenna, "2") ?
			"selected=\\\"selected\\\"" :
			"");
#endif
	websWrite(wp, "//]]>\n</script>\n");

	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
#endif

#endif

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.ssid)</script></div><input name=\"%s\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.ssid)\" value=\"%s\" /></div>\n",
		wl_ssid, nvram_selget(wp, wl_ssid));

#ifdef HAVE_RT2880
	if (nvram_selmatch(wp, wl_mode, "ap") ||
	    nvram_selmatch(wp, wl_mode, "wdsap") ||
	    nvram_selmatch(wp, wl_mode, "infra") ||
	    nvram_selmatch(wp, wl_mode, "apsta") ||
	    nvram_selmatch(wp, wl_mode, "apstawet"))
#else
	if (nvram_selmatch(wp, wl_mode, "ap") ||
	    nvram_selmatch(wp, wl_mode, "wdsap") ||
	    nvram_selmatch(wp, wl_mode, "infra"))
#endif
	{

		if (has_mimo(prefix) &&
		    (nvram_selnmatch(wp, "n-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "ng-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "mixed", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "n2-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "n5-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "ac-only", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "acn-mixed", "%s_net_mode", prefix) ||
		     nvram_selnmatch(wp, "na-only", "%s_net_mode", prefix))) {
			sas_show_channel(wp, prefix, prefix, 1);

			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(
				wp,
				"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div>\n");
			websWrite(wp, "<select name=\"%s_nbw\">\n", prefix);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");\n//]]>\n</script>\n",
				nvram_selnmatch(wp, "0", "%s_nbw", prefix) ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp,
				  "<option value=\"20\" %s>20 MHz</option>\n",
				  nvram_selnmatch(wp, "20", "%s_nbw", prefix) ?
					  "selected=\"selected\"" :
					  "");
			websWrite(wp,
				  "<option value=\"40\" %s>40 MHz</option>\n",
				  nvram_selnmatch(wp, "40", "%s_nbw", prefix) ?
					  "selected=\"selected\"" :
					  "");
			if (has_ac(prefix) && has_5ghz(prefix)) {
				websWrite(
					wp,
					"<option value=\"80\" %s>80 MHz</option>\n",
					nvram_nmatch("80", "%s_nbw", prefix) ?
						"selected=\"selected\"" :
						"");
			}
			websWrite(wp, "</select>\n");
			websWrite(wp, "</div>\n");

			if (nvram_selnmatch(wp, "40", "%s_nbw", prefix)) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(
					wp,
					"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n",
					  prefix);
				websWrite(
					wp,
					"<option value=\"upper\" %s>upper</option>\n",
					nvram_selnmatch(wp, "upper",
							"%s_nctrlsb", prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(
					wp,
					"<option value=\"lower\" %s>lower</option>\n",
					nvram_selnmatch(wp, "lower",
							"%s_nctrlsb", prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(wp, "</select>\n");

				websWrite(wp, "</div>\n");
			}
			if (nvram_selnmatch(wp, "80", "%s_nbw",
					    prefix)) { // 802.11ac
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(
					wp,
					"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n",
					  prefix);
				websWrite(
					wp,
					"<option value=\"ll\" %s>lower lower</option>\n",
					nvram_nmatch("ll", "%s_nctrlsb",
						     prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(
					wp,
					"<option value=\"lu\" %s>lower upper</option>\n",
					nvram_nmatch("lu", "%s_nctrlsb",
						     prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(
					wp,
					"<option value=\"ul\" %s>upper lower</option>\n",
					nvram_nmatch("ul", "%s_nctrlsb",
						     prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(
					wp,
					"<option value=\"uu\" %s>upper upper</option>\n",
					nvram_nmatch("uu", "%s_nctrlsb",
						     prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(wp, "</select>\n");
				websWrite(wp, "</div>\n");
			}
		} else {
			sas_show_channel(wp, prefix, prefix, 0);
#if defined(HAVE_ATH9K)
			if (is_mac80211(prefix) &&
			    (nvram_selmatch(wp, wl_width, "40") ||
			     nvram_selmatch(wp, wl_width, "2040"))) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(
					wp,
					"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
				websWrite(wp, "<select name=\"%s_nctrlsb\" >\n",
					  prefix);
				websWrite(
					wp,
					"<option value=\"upper\" %s>upper</option>\n",
					nvram_selnmatch(wp, "upper",
							"%s_nctrlsb", prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(
					wp,
					"<option value=\"lower\" %s>lower</option>\n",
					nvram_selnmatch(wp, "lower",
							"%s_nctrlsb", prefix) ?
						"selected=\"selected\"" :
						"");
				websWrite(wp, "</select>\n");

				websWrite(wp, "</div>\n");
			}
#endif
		}

		char wl_closed[16];
		sprintf(wl_closed, "%s_closed", prefix);

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			wl_closed,
			nvram_selmatch(wp, wl_closed, "0") ?
				"checked=\"checked\"" :
				"");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			wl_closed,
			nvram_selmatch(wp, wl_closed, "1") ?
				"checked=\"checked\"" :
				"");
		websWrite(wp, "</div>\n");
	}
#ifdef HAVE_BCMMODERN
	if (has_ac(prefix) && has_2ghz(prefix)) {
		char wl_turboqam[32];

		sprintf(wl_turboqam, "%s_turbo_qam", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.turboqam)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			wl_turboqam,
			nvram_selmatch(wp, wl_turboqam, "1") ?
				"checked=\"checked\"" :
				"");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
			wl_turboqam,
			nvram_selmatch(wp, wl_turboqam, "0") ?
				"checked=\"checked\"" :
				"");
		websWrite(wp, "</div>\n");
	}
#endif

	websWrite(wp, "</fieldset>\n");

	websWrite(wp, "<br style=\"%s\" />", stage_visible_css);
}

void sas_show_netmode(webs_t wp, char *prefix)
{
	char wl_net_mode[16];

	sprintf(wl_net_mode, "%s_net_mode", prefix);

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label2)</script></div><select name=\"%s\" onchange=\"refresh(this.form);\">\n",
		wl_net_mode);
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"disabled\\\" %s>\" + share.disabled + \"</option>\");\n",
		nvram_selmatch(wp, wl_net_mode, "disabled") ?
			"selected=\\\"selected\\\"" :
			"");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"mixed\\\" %s>\" + wl_basic.mixed + \"</option>\");\n",
		nvram_selmatch(wp, wl_net_mode, "mixed") ?
			"selected=\\\"selected\\\"" :
			"");
	if (has_mimo(prefix) && has_2ghz(prefix)) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "bg-mixed") ?
				"selected=\\\"selected\\\"" :
				"");
	}
#ifdef HAVE_WHRAG108
	if (!strcmp(prefix, "wlan1"))
#endif
#ifdef HAVE_TW6600
		if (!strcmp(prefix, "wlan1"))
#endif
			if (has_2ghz(prefix)) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"b-only\\\" %s>\" + wl_basic.b + \"</option>\");\n",
					nvram_selmatch(wp, wl_net_mode,
						       "b-only") ?
						"selected=\\\"selected\\\"" :
						"");
			}
#ifdef HAVE_MADWIFI
	if (has_2ghz(prefix)) {
#ifdef HAVE_WHRAG108
		if (!strcmp(prefix, "wlan1"))
#endif
#ifdef HAVE_TW6600
			if (!strcmp(prefix, "wlan1"))
#endif
				websWrite(
					wp,
					"document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");\n",
					nvram_selmatch(wp, wl_net_mode,
						       "g-only") ?
						"selected=\\\"selected\\\"" :
						"");
#ifdef HAVE_WHRAG108
		if (!strcmp(prefix, "wlan1"))
#endif
#ifdef HAVE_TW6600
			if (!strcmp(prefix, "wlan1"))
#endif
#if !defined(HAVE_LS5) || defined(HAVE_EOC5610)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");\n",
					nvram_selmatch(wp, wl_net_mode,
						       "bg-mixed") ?
						"selected=\\\"selected\\\"" :
						"");
#endif
	}
#else
#ifdef HAVE_WHRAG108
	if (!strcmp(prefix, "wlan1"))
#endif
#if !defined(HAVE_LS5) || defined(HAVE_EOC5610)
		if (has_2ghz(prefix)) {
			websWrite(
				wp,
				"document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "g-only") ?
					"selected=\\\"selected\\\"" :
					"");
		}
	if (has_mimo(prefix) && has_2ghz(prefix)) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"ng-only\\\" %s>\" + wl_basic.ng + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "ng-only") ?
				"selected=\\\"selected\\\"" :
				"");
	}
#endif
#endif
	if (has_mimo(prefix) && has_2ghz(prefix)) {
		if (has_5ghz(prefix)) {
			websWrite(
				wp,
				"document.write(\"<option value=\\\"n2-only\\\" %s>\" + wl_basic.n2 + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "n2-only") ?
					"selected=\\\"selected\\\"" :
					"");
		} else {
			websWrite(
				wp,
				"document.write(\"<option value=\\\"n-only\\\" %s>\" + wl_basic.n + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "n-only") ?
					"selected=\\\"selected\\\"" :
					"");
		}
	}
#if !defined(HAVE_FONERA) && !defined(HAVE_LS2) && !defined(HAVE_MERAKI)
#ifndef HAVE_MADWIFI
	if (has_5ghz(prefix)) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "a-only") ?
				"selected=\\\"selected\\\"" :
				"");
	}
	if (has_mimo(prefix) && has_5ghz(prefix)) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"na-only\\\" %s>\" + wl_basic.na + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "na-only") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"n5-only\\\" %s>\" + wl_basic.n5 + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "n5-only") ?
				"selected=\\\"selected\\\"" :
				"");
	}

	if (has_ac(prefix) && has_5ghz(prefix)) {
		websWrite(
			wp,
			"document.write(\"<option value=\\\"acn-mixed\\\" %s>\" + wl_basic.acn + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "acn-mixed") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"ac-only\\\" %s>\" + wl_basic.ac + \"</option>\");\n",
			nvram_selmatch(wp, wl_net_mode, "ac-only") ?
				"selected=\\\"selected\\\"" :
				"");
	}
#else
#if HAVE_WHRAG108
	if (!strcmp(prefix, "wlan0"))
#endif
#ifdef HAVE_TW6600
		if (!strcmp(prefix, "wlan0"))
#endif
			if (has_5ghz(prefix)) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");\n",
					nvram_selmatch(wp, wl_net_mode,
						       "a-only") ?
						"selected=\\\"selected\\\"" :
						"");
			}
#endif

#endif
#if defined(HAVE_ATH9K)
	if (is_mac80211(prefix)) {
		if (has_2ghz(prefix)) {
			websWrite(
				wp,
				"document.write(\"<option value=\\\"ng-only\\\" %s>\" + wl_basic.ng + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "ng-only") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"n2-only\\\" %s>\" + wl_basic.n2 + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "n2-only") ?
					"selected=\\\"selected\\\"" :
					"");
		}
		if (has_5ghz(prefix)) {
			websWrite(
				wp,
				"document.write(\"<option value=\\\"na-only\\\" %s>\" + wl_basic.na + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "na-only") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"n5-only\\\" %s>\" + wl_basic.n5 + \"</option>\");\n",
				nvram_selmatch(wp, wl_net_mode, "n5-only") ?
					"selected=\\\"selected\\\"" :
					"");
		}
	}
#endif
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");

#ifdef HAVE_RT2880
	if (nvram_selnmatch(wp, "n-only", "%s_net_mode", prefix)) {
		char wl_greenfield[32];

		sprintf(wl_greenfield, "%s_greenfield", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label7)</script></div><select name=\"%s\" >\n",
			wl_greenfield);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"0\\\" %s>\" + wl_basic.mixed + \"</option>\");\n",
			sas_nvram_default_match(wp, wl_greenfield, "0", "0") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"1\\\" %s>\" + wl_basic.greenfield + \"</option>\");\n",
			sas_nvram_default_match(wp, wl_greenfield, "1", "0") ?
				"selected=\\\"selected\\\"" :
				"");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
#endif
}

/**
 * displays the wireless channels
 */
void sas_show_channel(webs_t wp, char *dev, char *prefix, int type)
{
	char wl_mode[16];
	sprintf(wl_mode, "%s_mode", prefix);
	int instance = 0;
	char wl_nbw[16];

	char wl_net_mode[16];
	sprintf(wl_net_mode, "%s_net_mode", prefix);

	if (nvram_selmatch(wp, wl_net_mode, "disabled")) {
		return;
	}
	if (nvram_selmatch(wp, wl_mode, "ap") ||
	    nvram_selmatch(wp, wl_mode, "wdsap") ||
	    nvram_selmatch(wp, wl_mode, "infra")) {
		char wl_channel[16];
		sprintf(wl_channel, "%s_channel", prefix);

		char wl_wchannel[16];
		sprintf(wl_wchannel, "%s_wchannel", prefix);

		sas_nvram_default_get(wp, wl_wchannel, "0");
		sprintf(wl_nbw, "%s_nbw", prefix);

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label4)</script></div>\n<select name=\"%s\" onfocus=\"check_action(this,0)\"><script type=\"text/javascript\">\n//<![CDATA[\n",
			wl_channel);
#ifdef HAVE_MADWIFI
		struct wifi_channels *chan;
		char cn[32];
		char fr[32];
		int gotchannels = 0;
		int channelbw = 20;

#if defined(HAVE_ATH9K)
		if (is_mac80211(prefix)) {
#ifdef HAVE_ATH9K
			if (is_mac80211(prefix)) {
				// temp must be replaced with the actual selected country
				char regdomain[16];
				char *country;
				sprintf(regdomain, "%s_regdomain", prefix);
				country = nvram_default_get(regdomain,
							    "UNITED_STATES");
				// temp end
				if (nvram_nmatch("80", "%s_channelbw", prefix))
					channelbw = 80;
				if (nvram_nmatch("160", "%s_channelbw", prefix))
					channelbw = 160;
				if (nvram_nmatch("40", "%s_channelbw", prefix))
					channelbw = 40;
				chan = mac80211_get_channels_simple(
					prefix, getIsoName(country), channelbw,
					0xff);
				/* if (chan == NULL)
				   chan =
				   list_channels_ath9k(dev, "DE", 40,
				   0xff); */
				gotchannels = 1;
			}
#endif
		}
#endif
		if (!gotchannels) {
			chan = list_channels(prefix);
			if (chan == NULL)
				chan = list_channels(dev);
		}
		if (chan) {
			// int cnt = getchannelcount ();
			websWrite(
				wp,
				"document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n",
				nvram_selmatch(wp, wl_channel, "0") ?
					"selected=\\\"selected\\\"" :
					"");
			int i = 0;
			int offset = get_freqoffset(prefix);
			while (chan[i].freq != -1) {
				cprintf("%d\n", chan[i].channel);
				cprintf("%d\n", chan[i].freq);

				sprintf(cn, "%d", chan[i].channel);
				sprintf(fr, "%d", chan[i].freq);
				if (channelbw > 20 && !chan[i].lll &&
				    !chan[i].llu && !chan[i].lul &&
				    !chan[i].luu && !chan[i].ull &&
				    !chan[i].ulu && !chan[i].uul &&
				    !chan[i].uuu) {
					i++;
					continue; // do not show channels where bandwidth is not available
				}
				int freq = chan[i].freq;
				if (freq != -1) {
					websWrite(
						wp,
						"document.write(\"<option value=\\\"%s\\\" %s>%s - %d MHz</option>\");\n",
						fr,
						nvram_selmatch(wp, wl_channel,
							       fr) ?
							"selected=\\\"selected\\\"" :
							"",
						cn, (freq + offset));
				}
				// free (chan[i].freq);
				i++;
			}
		}
		if (gotchannels)
			debug_free(chan);

#else

		if (!strcmp(prefix, "wl1"))
			instance = 1;
		else if (!strcmp(prefix, "wl2"))
			instance = 2;

		unsigned int chanlist[128];
		char *ifn = get_wl_instance_name(instance);
		int chancount = getchannels(chanlist, ifn);
		int net_is_a = 0;

		if (chanlist[0] > 25)
			net_is_a = 1;

		int i, j;

		// supported 5GHz channels for IEEE 802.11n 40MHz
		int na_upper[16] = { 40,  48,  56,  64, 104, 112, 120, 128,
				     136, 153, 161, 0,	0,   0,	  0,   0 };
		int na_lower[16] = { 36,  44,  52,  60, 100, 108, 116, 124,
				     132, 149, 157, 0,	0,   0,	  0,   0 };

		websWrite(
			wp,
			"document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n",
			nvram_selnmatch(wp, "0", "%s_channel", prefix) ?
				"selected=\\\"selected\\\"" :
				"");
		for (i = 0; i < chancount; i++) {
			float ofs;

			if (chanlist[i] < 25)
				ofs = 2.407f;
			else
				ofs = 5.000f;
			ofs += (float)(chanlist[i] * 0.005f);
			if (ofs == 2.477f)
				ofs = 2.484f; // fix: ch 14 is 2.484, not 2.477 GHz

			char channelstring[32];
			int showit = 1;

			if (nvram_selmatch(wp, wl_net_mode, "a-only") ||
			    nvram_selmatch(wp, wl_net_mode, "na-only") ||
			    nvram_selmatch(wp, wl_net_mode, "n5-only") ||
			    nvram_selmatch(wp, wl_net_mode, "acn-mixed") ||
			    nvram_selmatch(wp, wl_net_mode, "ac-only") ||
			    (net_is_a &&
			     nvram_selmatch(wp, wl_net_mode, "mixed"))) {
				if (chanlist[i] < 25)
					showit = 0;
			} else {
				if (chanlist[i] > 25)
					showit = 0;
			}

			if ((nvram_selmatch(wp, wl_net_mode, "na-only") ||
			     nvram_selmatch(wp, wl_net_mode, "ac-only") ||
			     nvram_selmatch(wp, wl_net_mode, "acn-mixed") ||
			     (net_is_a &&
			      nvram_selmatch(wp, wl_net_mode, "mixed")) ||
			     nvram_selmatch(wp, wl_net_mode, "n5-only")) &&
			    nvram_selmatch(wp, wl_nbw, "40")) {
				showit = 0;
				j = 0;
				if (nvram_selnmatch(wp, "upper", "%s_nctrlsb",
						    prefix)) {
					while (na_upper[j]) {
						if (chanlist[i] ==
						    na_upper[j]) {
							showit = 1;
							break;
						}
						j++;
					}
				} else if (nvram_selnmatch(wp, "lower",
							   "%s_nctrlsb",
							   prefix)) {
					while (na_lower[j]) {
						if (chanlist[i] ==
						    na_lower[j]) {
							showit = 1;
							break;
						}
						j++;
					}
				}
			}

			if ((nvram_selmatch(wp, wl_net_mode, "n-only") ||
			     nvram_selmatch(wp, wl_net_mode, "n2-only") ||
			     nvram_selmatch(wp, wl_net_mode, "ng-only") ||
			     (!net_is_a &&
			      nvram_selmatch(wp, wl_net_mode, "mixed"))) &&
			    nvram_selmatch(wp, wl_nbw, "40")) {
				showit = 0;
				if (nvram_selnmatch(wp, "upper", "%s_nctrlsb",
						    prefix)) {
					if (chanlist[i] >= 5 &&
					    chanlist[i] <= 13) {
						showit = 1;
					}
				} else if (nvram_selnmatch(wp, "lower",
							   "%s_nctrlsb",
							   prefix)) {
					if (chanlist[i] <= 9) {
						showit = 1;
					}
				}
			}

			sprintf(channelstring, "%d", chanlist[i]);
			if (showit) {
				websWrite(
					wp,
					"document.write(\"<option value=\\\"%d\\\" %s>%d - %0.3f GHz</option>\");\n",
					chanlist[i],
					nvram_selnmatch(wp, channelstring,
							"%s_channel", prefix) ?
						"selected=\\\"selected\\\"" :
						"",
					chanlist[i], ofs);
			}
		}
#endif
		websWrite(wp, "//]]>\n</script></select></div>\n");
	}
}

EJ_VISIBLE void ej_sas_show_security(webs_t wp, int argc, char_t **argv)
{
	int i = 0;
	int c = getdevicecount();

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, WIFINAME "%d", i);
		sas_show_security_single(wp, argc, argv, buf);
	}
	return;
}

void sas_show_security_single(webs_t wp, int argc, char_t **argv, char *prefix)
{
	char *next;
	char var[80];
	char ssid[80];
	char mac[16];

	char *stage_visible_css = "display: none;";
	if (ej_sas_stage_is_visible(wp, argc, argv) == 0) {
		stage_visible_css = "";
	}

	sprintf(mac, "%s_hwaddr", prefix);
	char *vifs = sas_nvram_nget(wp, "%s_vifs", prefix);

	if (vifs == NULL)
		return;
	sprintf(ssid, "%s_ssid", prefix);
	/*websWrite(wp,
	   "<h2 style=\"%s\"><script type=\"text/javascript\">Capture(wpa.h2)</script> %s</h2>\n",
	   stage_visible_css, prefix); */
	websWrite(wp, "<fieldset style=\"%s\">\n", stage_visible_css);
	// cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
	websWrite(
		wp,
		"<legend><script type=\"text/javascript\">Capture(wpa.h2)</script></legend>");
	/*        "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s SSID [",
	   IFMAP(prefix));
	   tf_webWriteESCNV(wp, ssid);
	   // contains html tag
	   websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(mac)); */
	sas_show_security_prefix(wp, argc, argv, prefix, 1);
	websWrite(wp, "</fieldset>\n<br style=\"%s\"/>\n", stage_visible_css);
	/*foreach(var, vifs, next) {
	   sprintf(ssid, "%s_ssid", var);
	   websWrite(wp, "<fieldset>\n");
	   // cprintf("getting %s %s\n", ssid,nvram_safe_get(ssid));
	   websWrite(wp,
	   "<legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [",
	   IFMAP(var));
	   tf_webWriteESCNV(wp, ssid);     // fix for broken html page if ssid
	   // contains html tag
	   websWrite(wp, "]</legend>\n");
	   rep(var, '.', 'X');
	   sas_show_security_prefix(wp, argc, argv, var, 0);
	   websWrite(wp, "</fieldset>\n<br />\n");
	   } */
}

void sas_show_security_prefix(webs_t wp, int argc, char_t **argv, char *prefix,
			      int primary)
{
	static char var[80];
	static char sta[80];
	static char spf[80];

	// char p2[80];
	cprintf("show security prefix\n");
	sprintf(var, "%s_security_mode", prefix);
	sprintf(spf, "disabled");
	// strcpy(p2,prefix);
	// rep(p2,'X','.');
	// websWrite (wp, "<input type=\"hidden\"
	// name=\"%s_security_mode\"/>\n",p2);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.secmode)</script></div>\n");
	websWrite(
		wp,
		"<select name=\"%s_security_mode\" onchange=\"SelMode('%s', '%s_security_mode',this.form.%s_security_mode.selectedIndex,this.form)\">\n",
		prefix, prefix, prefix, prefix);
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");\n//]]>\n</script>\n",
		selmatch(wp, var, "disabled", "selected=\\\"selected\\\""));
	websWrite(wp, "<option value=\"psk\" %s>WPA Personal</option>\n",
		  selmatch(wp, var, "psk", "selected=\"selected\""));
	if (!strcmp(nvram_selget(wp, var), "psk")) {
		sprintf(spf, "psk");
	}
	sprintf(sta, "%s_mode", prefix);
	if (!primary || nvram_selmatch(wp, sta, "ap") ||
	    nvram_selmatch(wp, sta, "wdsap")) {
		websWrite(wp,
			  "<option value=\"wpa\" %s>WPA Enterprise</option>\n",
			  selmatch(wp, var, "wpa", "selected=\"selected\""));
		if (!strcmp(nvram_selget(wp, var), "wpa")) {
			sprintf(spf, "wpa");
		}
	}
	websWrite(wp, "<option value=\"psk2\" %s>WPA2 Personal</option>\n",
		  selmatch(wp, var, "psk2", "selected=\"selected\""));
	if (!strcmp(nvram_selget(wp, var), "psk2")) {
		sprintf(spf, "psk2");
	}
	if (!primary || nvram_selmatch(wp, sta, "ap") ||
	    nvram_selmatch(wp, sta, "wdsap")) {
		websWrite(
			wp,
			"<option value=\"wpa2\" %s>WPA2 Enterprise</option>\n",
			selmatch(wp, var, "wpa2", "selected=\"selected\""));
		if (!strcmp(nvram_selget(wp, var), "wpa2")) {
			sprintf(spf, "wpa2");
		}
	}
#ifdef HAVE_RT2880
	if (!primary || nvram_selmatch(wp, sta, "ap"))
#endif
		websWrite(
			wp,
			"<option value=\"psk psk2\" %s>WPA2 Personal Mixed</option>\n",
			selmatch(wp, var, "psk psk2", "selected=\"selected\""));
#ifdef HAVE_RT2880
	if (!primary || nvram_selmatch(wp, sta, "ap"))
#endif
		if (!strcmp(nvram_selget(wp, var), "psk psk2")) {
			sprintf(spf, "psk psk2");
		}

	if (!primary || nvram_selmatch(wp, sta, "ap") ||
	    nvram_selmatch(wp, sta, "wdsap")) {
		websWrite(
			wp,
			"<option value=\"wpa wpa2\" %s>WPA2 Enterprise Mixed</option>\n",
			selmatch(wp, var, "wpa wpa2", "selected=\"selected\""));
		if (!strcmp(nvram_selget(wp, var), "wpa wpa2")) {
			sprintf(spf, "wpa wpa2");
		}

		websWrite(wp, "<option value=\"radius\" %s>RADIUS</option>\n",
			  selmatch(wp, var, "radius", "selected=\"selected\""));
		if (!strcmp(nvram_selget(wp, var), "radius")) {
			sprintf(spf, "radius");
		}
	}

	websWrite(wp, "<option value=\"wep\" %s>WEP</option>\n",
		  selmatch(wp, var, "wep", "selected=\"selected\""));
	if (!strcmp(nvram_selget(wp, var), "wep")) {
		sprintf(spf, "wep");
	}
#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
#if !defined(HAVE_RT2880) || defined(HAVE_MT76)
	if (nvram_selmatch(wp, sta, "sta") ||
	    nvram_selmatch(wp, sta, "wdssta") || nvram_match(sta, "apsta") ||
	    nvram_match(sta, "wet")) {
		websWrite(wp, "<option value=\"8021X\" %s>802.1x</option>\n",
			  selmatch(wp, var, "8021X", "selected=\"selected\""));
		if (!strcmp(nvram_selget(wp, var), "8021X")) {
			sprintf(spf, "8021X");
		}
	}
#else
#ifndef HAVE_RT61
	if (nvram_selmatch(wp, sta, "sta") || nvram_match(wp, "wet")) {
		websWrite(wp, "<option value=\"8021X\" %s>802.1x</option>\n",
			  selmatch(wp, var, "8021X", "selected=\"selected\""));
		if (!strcmp(nvram_selget(wp, var), "8021X")) {
			sprintf(spf, "8021X");
		}
	}
#endif
#endif
#endif
#endif

	websWrite(wp, "</select></div>\n");
	rep(prefix, 'X', '.');
	cprintf("ej show wpa\n");
	sas_show_wpa_setting(wp, argc, argv, prefix, spf);
}

void sas_show_wparadius(webs_t wp, char *prefix)
{
	char var[80];

	websWrite(wp, "<div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.algorithms)</script></div>\n");
	websWrite(wp, "<select name=\"%s_crypto\">\n", prefix);
	sprintf(var, "%s_crypto", prefix);
	websWrite(wp, "<option value=\"aes\" %s>AES</option>\n",
		  selmatch(wp, var, "aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n",
		  selmatch(wp, var, "tkip+aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip\" %s>TKIP</option>\n",
		  selmatch(wp, var, "tkip", "selected=\"selected\""));
	websWrite(wp, "</select></div>\n");
#ifdef HAVE_MADWIFI
	sas_show_radius(wp, prefix, 0, 1);
#else
	sas_show_radius(wp, prefix, 0, 0);
#endif
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.rekey)</script></div>\n");
	sprintf(var, "%s_wpa_gtk_rekey", prefix);
	websWrite(
		wp,
		"<input name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"10\" onblur=\"valid_range(this,0,99999,wpa.rekey)\" value=\"%s\" />",
		prefix, sas_nvram_default_get(wp, var, "3600"));
	websWrite(
		wp,
		"&nbsp;<script type=\"text/javascript\">Capture(share.seconds)</script></div>\n");
	websWrite(wp, "</div>\n");
}

#ifdef HAVE_WPA_SUPPLICANT

static void sas_init_80211x_layers(webs_t wp, char *prefix)
{
	char var[80];
	sprintf(var, "%s_security_mode", prefix);
	fprintf(stderr, "[init] %s %s\n", var, nvram_selget(wp, var));
	if (!strcmp(nvram_selget(wp, var), "8021X")) {
		if (sas_nvram_prefix_match(wp, "8021xtype", prefix, "tls")) {
			websWrite(wp, "enable_idtls(\"%s\");\n", prefix);
		} else if (sas_nvram_prefix_match(wp, "8021xtype", prefix,
						  "leap")) {
			websWrite(wp, "enable_idleap(\"%s\");\n", prefix);
		} else if (sas_nvram_prefix_match(wp, "8021xtype", prefix,
						  "ttls")) {
			websWrite(wp, "enable_idttls(\"%s\");\n", prefix);
		} else {
			//if (sas_nvram_prefix_match(wp, "8021xtype", prefix, "peap")) {
			websWrite(wp, "enable_idpeap(\"%s\");\n", prefix);
		}
	}
}

EJ_VISIBLE void ej_sas_init_80211x_layers(webs_t wp, int argc, char_t **argv)
{
#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	int i = 0;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wl%d", i);
		sas_init_80211x_layers(wp, buf);
	}
	return;
#else
	int c = getdevicecount();
	int i = 0;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wlan%d", i);
		if (nvram_selnmatch(wp, "8021X", "%s_security_mode", buf))
			sas_init_80211x_layers(wp, buf);
	}
	return;
#endif
}

void sas_show_80211X(webs_t wp, char *prefix)
{
	/*
	 * fields
	 * _8021xtype
	 * _8021xuser
	 * _8021xpasswd
	 * _8021xca
	 * _8021xpem
	 * _8021xprv
	 * _8021xaddopt
	 */
	char type[32];
	sas_nvram_default_get(wp, type, "ttls");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.xsuptype)</script></div>\n");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"peap\" onclick=\"enable_idpeap('%s')\" %s />Peap&nbsp;\n",
		prefix, prefix,
		sas_nvram_prefix_match(wp, "8021xtype", prefix, "peap") ?
			"checked=\"checked\"" :
			"");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"leap\" onclick=\"enable_idleap('%s')\" %s />Leap&nbsp;\n",
		prefix, prefix,
		sas_nvram_prefix_match(wp, "8021xtype", prefix, "leap") ?
			"checked=\"checked\"" :
			"");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"tls\" onclick=\"enable_idtls('%s')\" %s />TLS&nbsp;\n",
		prefix, prefix,
		sas_nvram_prefix_match(wp, "8021xtype", prefix, "tls") ?
			"checked=\"checked\"" :
			"");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" name=\"%s_8021xtype\" value=\"ttls\" onclick=\"enable_idttls('%s')\" %s />TTLS&nbsp;\n",
		prefix, prefix,
		sas_nvram_prefix_match(wp, "8021xtype", prefix, "ttls") ?
			"checked=\"checked\"" :
			"");
	websWrite(wp, "</div>\n");

	// ttls authentication
	websWrite(wp, "<div id=\"idttls%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_ttls8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "ttls8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_ttls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "ttls8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_ttls8021xpasswd\" type=\"password\" autocomplete=\"off\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "ttls8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_ttls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "ttls8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.servercertif)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_ttls8021xca\" name=\"%s_ttls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_ttls8021xca = fix_cr( '", prefix);
	char namebuf[64];
	sprintf(namebuf, "%s_ttls8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_ttls8021xca\").value = %s_ttls8021xca;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_ttls8021xaddopt\" name=\"%s_ttls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_ttls8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_ttls8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_ttls8021xaddopt\").value = %s_ttls8021xaddopt;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");

	// peap authentication
	websWrite(wp, "<div id=\"idpeap%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_peap8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "peap8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_peap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "peap8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_peap8021xpasswd\" type=\"password\" autocomplete=\"off\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "peap8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_peap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "peap8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.servercertif)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_peap8021xca\" name=\"%s_peap8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);

	websWrite(wp, "var %s_peap8021xca = fix_cr( '", prefix);
	sprintf(namebuf, "%s_peap8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_peap8021xca\").value = %s_peap8021xca;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_peap8021xaddopt\" name=\"%s_peap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_peap8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_peap8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_peap8021xaddopt\").value = %s_peap8021xaddopt;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");

	// leap authentication
	websWrite(wp, "<div id=\"idleap%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_leap8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "leap8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_leap8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "leap8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_leap8021xpasswd\" type=\"password\" autocomplete=\"off\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "leap8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_leap8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "leap8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_leap8021xaddopt\" name=\"%s_leap8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_leap8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_leap8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_leap8021xaddopt\").value = %s_leap8021xaddopt;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");

	// tls authentication
	websWrite(wp, "<div id=\"idtls%s\">\n", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.user)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_tls8021xuser\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, nvram_prefix_get("tls8021xuser", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.anon)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_tls8021xanon\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "tls8021xanon", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.passwd)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_tls8021xpasswd\" type=\"password\" autocomplete=\"off\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "tls8021xpasswd", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.phase2)</script></div>\n");
	websWrite(
		wp,
		"<input name=\"%s_tls8021xphase2\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
		prefix, sas_nvram_prefix_get(wp, "tls8021xphase2", prefix));

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.servercertif)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xca\" name=\"%s_tls8021xca\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xca = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xca", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_tls8021xca\").value = %s_tls8021xca;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.clientcertif)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xpem\" name=\"%s_tls8021xpem\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xpem = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xpem", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_tls8021xpem\").value = %s_tls8021xpem;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(share.privatekey)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"6\" id=\"%s_tls8021xprv\" name=\"%s_tls8021xprv\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xprv = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xprv", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_tls8021xprv\").value = %s_tls8021xprv;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(sec80211x.options)</script></div>\n");
	websWrite(
		wp,
		"<textarea cols=\"60\" rows=\"3\" id=\"%s_tls8021xaddopt\" name=\"%s_tls8021xaddopt\"></textarea>\n<script type=\"text/javascript\">\n//<![CDATA[\n ",
		prefix, prefix);
	websWrite(wp, "var %s_tls8021xaddopt = fix_cr( '", prefix);
	sprintf(namebuf, "%s_tls8021xaddopt", prefix);
	tf_webWriteESCNV(wp, namebuf);
	websWrite(wp, "' );\n");
	websWrite(
		wp,
		"document.getElementById(\"%s_tls8021xaddopt\").value = %s_tls8021xaddopt;\n",
		prefix, prefix);
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</div>\n");

	websWrite(wp, "</div>\n");
	websWrite(wp, "<script>\n//<![CDATA[\n ");
	char peap[32];
	sprintf(peap, "%s_8021xtype", prefix);
	websWrite(
		wp,
		"show_layer_ext(document.wpa.%s_8021xtype, 'idpeap%s', %s);\n",
		prefix, prefix,
		nvram_selmatch(wp, peap, "peap") ? "true" : "false");
	websWrite(wp,
		  "show_layer_ext(document.wpa.%s_8021xtype, 'idtls%s', %s);\n",
		  prefix, prefix,
		  nvram_selmatch(wp, peap, "tls") ? "true" : "false");
	websWrite(
		wp,
		"show_layer_ext(document.wpa.%s_8021xtype, 'idleap%s', %s);\n",
		prefix, prefix,
		nvram_selmatch(wp, peap, "leap") ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");
}
#endif

void sas_show_wpa_setting(webs_t wp, int argc, char_t **argv, char *prefix,
			  char *security_prefix)
{
	char *type, *security_mode;
	char var[80];
	int s_free = 0;

	fprintf(stderr, "[security prefix] %s\n", security_prefix);
	sprintf(var, "%s_security_mode", prefix);
	cprintf("show wpa setting\n");
	type = argv[0];
	rep(var, '.', 'X');
	security_mode = GOZILA_GET(wp, var);
	if (security_mode == NULL)
		//security_mode = nvram_safe_get(var);
		security_mode = nvram_selget(wp, var);
	if (strcmp(security_mode, security_prefix)) {
		security_mode = security_prefix;
	}
	rep(var, 'X', '.');
	cprintf("security mode %s = %s\n", security_mode, var);
	fprintf(stderr, "security mode %s = %s\n", security_mode, var);
	if (!strcmp(security_mode, "psk") || !strcmp(security_mode, "psk2") ||
	    !strcmp(security_mode, "psk psk2"))
		sas_show_preshared(wp, prefix);
#if UI_STYLE != CISCO
	else if (!strcmp(security_mode, "disabled"))
		sas_show_preshared(wp, prefix);
#endif
	else if (!strcmp(security_mode, "radius"))
		sas_show_radius(wp, prefix, 1, 0);
	else if (!strcmp(security_mode, "wpa") ||
		 !strcmp(security_mode, "wpa2") ||
		 !strcmp(security_mode, "wpa wpa2"))
		sas_show_wparadius(wp, prefix);
	else if (!strcmp(security_mode, "wep"))
		sas_show_wep(wp, prefix);
#ifdef HAVE_WPA_SUPPLICANT
#ifndef HAVE_MICRO
	else if (!strcmp(security_mode, "8021X"))
		sas_show_80211X(wp, prefix);
#endif
#endif
	return;
}

char *sas_get_wep_value(webs_t wp, char *temp, char *type, char *_bit,
			char *prefix)
{
	int cnt;
	char *wordlist;
	char wl_wep[] = "wlX.XX_wep_XXXXXX";

	// check for httpd post values
	if (wp->gozila_action && !wp->p->generate_key) {
		char label[32];
		sprintf(label, "%s_%s", prefix, type);
		return nvram_selget(wp, label);
	}

	if (wp->p->generate_key) {
		snprintf(wl_wep, sizeof(wl_wep), "%s_wep_gen", prefix);
	} else {
		snprintf(wl_wep, sizeof(wl_wep), "%s_wep_buf", prefix);
	}

	wordlist = nvram_safe_get(wl_wep);

	if (!strcmp(wordlist, ""))
		return "";

	cnt = count_occurences(wordlist, ':');

	if (!strcmp(type, "passphrase")) {
		if (wordlist[0] == ':')
			return "";
		substring(0, pos_nthoccurence(wordlist, ':', cnt - 4), wordlist,
			  temp, sizeof(temp));
		return temp;
	} else if (!strcmp(type, "key1")) {
		substring(pos_nthoccurence(wordlist, ':', cnt - 4) + 1,
			  pos_nthoccurence(wordlist, ':', cnt - 3), wordlist,
			  temp, sizeof(temp));
		return temp;
	} else if (!strcmp(type, "key2")) {
		substring(pos_nthoccurence(wordlist, ':', cnt - 3) + 1,
			  pos_nthoccurence(wordlist, ':', cnt - 2), wordlist,
			  temp, sizeof(temp));
		return temp;
	} else if (!strcmp(type, "key3")) {
		substring(pos_nthoccurence(wordlist, ':', cnt - 2) + 1,
			  pos_nthoccurence(wordlist, ':', cnt - 1), wordlist,
			  temp, sizeof(temp));
		return temp;
	} else if (!strcmp(type, "key4")) {
		substring(pos_nthoccurence(wordlist, ':', cnt - 1) + 1,
			  pos_nthoccurence(wordlist, ':', cnt), wordlist, temp,
			  sizeof(temp));
		return temp;
	} else if (!strcmp(type, "tx")) {
		substring(pos_nthoccurence(wordlist, ':', cnt) + 1,
			  strlen(wordlist), wordlist, temp, sizeof(temp));
		return temp;
	}

	return "";
}

void sas_show_wep(webs_t wp, char *prefix)
{
	char var[80];
	char *bit;

	cprintf("show wep\n");
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880)
	char wl_authmode[16];

	sprintf(wl_authmode, "%s_authmode", prefix);
	sas_nvram_default_get(wp, wl_authmode, "open");
	if (nvram_invmatch(wl_authmode, "auto")) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"open\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.openn)</script></input>&nbsp;\n",
			wl_authmode,
			nvram_selmatch(wp, wl_authmode, "open") ?
				"checked=\"checked\"" :
				"");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"shared\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.share_key)</script></input>\n",
			wl_authmode,
			nvram_selmatch(wp, wl_authmode, "shared") ?
				"checked=\"checked\"" :
				"");
		websWrite(wp, "</div>\n");
	}
#endif
	websWrite(
		wp,
		"<div><div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wep.defkey)</script></div>");
	websWrite(wp, "<input type=\"hidden\" name=\"%s_WEP_key\" />", prefix);
	websWrite(
		wp,
		"<input type=\"hidden\" name=\"%s_wep\" value=\"restricted\" />",
		prefix);
	sprintf(var, "%s_key", prefix);
	sas_nvram_default_get(wp, var, "1");
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_key\" %s />1&nbsp;\n",
		prefix, selmatch(wp, var, "1", "checked=\"checked\""));
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"2\" name=\"%s_key\" %s />2&nbsp;\n",
		prefix, selmatch(wp, var, "2", "checked=\"checked\""));
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"3\" name=\"%s_key\" %s />3&nbsp;\n",
		prefix, selmatch(wp, var, "3", "checked=\"checked\""));
	websWrite(
		wp,
		"<input class=\"spaceradio\" type=\"radio\" value=\"4\" name=\"%s_key\" %s />4&nbsp;\n",
		prefix, selmatch(wp, var, "4", "checked=\"checked\""));
	websWrite(wp, "</div>");
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script></div>");

	sprintf(var, "%s_wep_bit", prefix);
	//bit = nvram_safe_get(var);
	bit = nvram_selget(wp, var);

	cprintf("bit %s\n", bit);

	websWrite(
		wp,
		"<select name=\"%s_wep_bit\" size=\"1\" onchange=keyMode(this.form)>",
		prefix);
	websWrite(wp, "<option value=\"64\" %s >64 bits 10 hex digits</option>",
		  selmatch(wp, var, "64", "selected=\"selected\""));
	websWrite(wp,
		  "<option value=\"128\" %s >128 bits 26 hex digits</option>",
		  selmatch(wp, var, "128", "selected=\"selected\""));
	websWrite(
		wp,
		"</select>\n</div>\n<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wep.passphrase)</script></div>\n");
	websWrite(
		wp,
		"<input name=%s_passphrase maxlength=\"16\" size=\"20\" value=\"",
		prefix);

	char p_temp[128];
	char temp[256];

	sprintf(p_temp, "%s",
		sas_get_wep_value(wp, temp, "passphrase", bit, prefix));
	nvram_set("passphrase_temp", p_temp);
	tf_webWriteESCNV(wp, "passphrase_temp");
	nvram_unset("passphrase_temp");

	websWrite(wp, "\" />");
	websWrite(
		wp,
		"<input type=\"hidden\" value=\"Null\" name=\"generateButton\" />\n");
	websWrite(
		wp,
		"<input class=\"button\" type=\"button\" value=\"Generate\" onclick=generateKey(this.form,\"%s\") name=wepGenerate />\n</div>",
		prefix);

	char *mlen = "10";
	char *mlen2 = "12";

	if (!strcmp(bit, "128")) {
		mlen = "26";
		mlen2 = "30";
	}
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 1</div>\n");
	websWrite(
		wp,
		"<input name=%s_key1 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
		prefix, mlen2, mlen,
		sas_get_wep_value(wp, temp, "key1", bit, prefix));
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 2</div>\n");
	websWrite(
		wp,
		"<input name=%s_key2 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
		prefix, mlen2, mlen,
		sas_get_wep_value(wp, temp, "key2", bit, prefix));
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 3</div>\n");
	websWrite(
		wp,
		"<input name=%s_key3 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
		prefix, mlen2, mlen,
		sas_get_wep_value(wp, temp, "key3", bit, prefix));
	websWrite(
		wp,
		"<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(share.key)</script> 4</div>\n");
	websWrite(
		wp,
		"<input name=%s_key4 size=\"%s\" maxlength=\"%s\" value=\"%s\" /></div>\n",
		prefix, mlen2, mlen,
		sas_get_wep_value(wp, temp, "key4", bit, prefix));
	websWrite(wp, "</div>\n");
}

void sas_show_preshared(webs_t wp, char *prefix)
{
	char var[80];

	cprintf("show preshared");
	sprintf(var, "%s_crypto", prefix);
	websWrite(wp, "<div><div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.algorithms)</script></div>\n");
	websWrite(wp, "<select name=\"%s_crypto\">\n", prefix);
	websWrite(wp, "<option value=\"aes\" %s>AES</option>\n",
		  selmatch(wp, var, "aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n",
		  selmatch(wp, var, "tkip+aes", "selected=\"selected\""));
	websWrite(wp, "<option value=\"tkip\" %s>TKIP</option>\n",
		  selmatch(wp, var, "tkip", "selected=\"selected\""));
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.shared_key)</script></div>\n");

	sprintf(var, "%s_wpa_psk", prefix);
	websWrite(
		wp,
		"<input type=\"password\" autocomplete=\"off\" id=\"%s_wpa_psk\" name=\"%s_wpa_psk\" onblur=\"valid_psk_length(this)\" maxlength=\"64\" size=\"32\" value=\"",
		prefix, prefix);
	//tf_webWriteESCNV(wp, var);
	websWrite(wp, "%s", nvram_selget(wp, var));
	websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
	websWrite(
		wp,
		"<input type=\"checkbox\" name=\"%s_wl_unmask\" value=\"0\" onclick=\"setElementMask('%s_wpa_psk', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
		prefix, prefix);
	websWrite(wp, "</div>\n");
	/*	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.rekey)</script></div>\n");
	
	sprintf(var, "%s_wpa_gtk_rekey", prefix);
	websWrite(wp,
		"<input class=\"num\" name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"5\" onblur=\"valid_range(this,0,99999,wpa.rekey)\" value=\"%s\" />\n",
		prefix, nvram_default_get(var, "3600"));
	websWrite(wp,
		"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 3600, \" + share.range + \": 1 - 99999)\");\n//]]>\n</script></span>\n");
	websWrite(wp, "</div>\n");*/
	websWrite(wp, "</div>\n");
}

void sas_show_radius(webs_t wp, char *prefix, int showmacformat, int backup)
{
	char var[80];

	cprintf("show radius\n");
	if (showmacformat) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label2)</script></div>\n");
		websWrite(wp, "<select name=\"%s_radmactype\">\n", prefix);
		websWrite(
			wp, "<option value=\"0\" %s >aabbcc-ddeeff</option>\n",
			sas_nvram_prefix_match(wp, "radmactype", prefix, "0") ?
				"selected" :
				"");
		websWrite(wp, "<option value=\"1\" %s >aabbccddeeff</option>\n",
			  sas_nvram_prefix_match(wp, "radmactype", prefix,
						 "1") ?
				  "selected" :
				  "");
		websWrite(
			wp,
			"<option value=\"2\" %s >aa:bb:cc:dd:ee:ff</option>\n",
			sas_nvram_prefix_match(wp, "radmactype", prefix, "2") ?
				"selected" :
				"");
		websWrite(
			wp,
			"<option value=\"3\" %s >aa-bb-cc-dd-ee-ff</option>\n",
			sas_nvram_prefix_match(wp, "radmactype", prefix, "3") ?
				"selected" :
				"");
		websWrite(wp, "</select>\n");
		websWrite(wp, "</div>\n");
	}
	//char *rad = sas_nvram_nget(wp, "%s_radius_ipaddr", prefix);
	//char *rad = printf("%s_radius_ipaddr", prefix);
	char rad[32];
	sprintf(rad, "%s_radius_ipaddr", prefix);

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label3)</script></div>\n");
	websWrite(
		wp,
		"<input type=\"hidden\" name=\"%s_radius_ipaddr\" value=\"%s\" />\n",
		prefix, rad);
	websWrite(
		wp,
		"<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%s\" />.",
		prefix, sas_get_single_ip(wp, rad, 0));
	websWrite(
		wp,
		"<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%s\" />.",
		prefix, sas_get_single_ip(wp, rad, 1));
	websWrite(
		wp,
		"<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%s\" />.",
		prefix, sas_get_single_ip(wp, rad, 2));
	websWrite(
		wp,
		"<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label3)\" class=\"num\" value=\"%s\" />\n",
		prefix, sas_get_single_ip(wp, rad, 3));
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label4)</script></div>\n");
	sprintf(var, "%s_radius_port", prefix);
	websWrite(
		wp,
		"<input name=\"%s_radius_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label4)\" value=\"%s\" />\n",
		prefix, sas_nvram_default_get(wp, var, "1812"));
	websWrite(
		wp,
		"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1812)\");\n//]]>\n</script></span>\n</div>\n");
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label7)</script></div>\n");
	sprintf(var, "%s_radius_key", prefix);
	websWrite(
		wp,
		"<input type=\"password\" autocomplete=\"off\" id=\"%s_radius_key\" name=\"%s_radius_key\" maxlength=\"79\" size=\"32\" value=\"",
		prefix, prefix);

	//tf_webWriteESCNV(wp, var);
	websWrite(wp, "%s", nvram_selget(wp, var));
	websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
	websWrite(
		wp,
		"<input type=\"checkbox\" name=\"%s_radius_unmask\" value=\"0\" onclick=\"setElementMask('%s_radius_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
		prefix, prefix);

	if (backup) {
		//rad = sas_nvram_nget(wp, "%s_radius2_ipaddr", prefix);
		sprintf(rad, "%s_radius2_ipaddr", prefix);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label23)</script></div>\n");
		websWrite(
			wp,
			"<input type=\"hidden\" name=\"%s_radius2_ipaddr\" value=\"4\" />\n",
			prefix);
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label23)\" class=\"num\" value=\"%s\" />.",
			prefix, sas_get_single_ip(wp, rad, 0));
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label23)\" class=\"num\" value=\"%s\" />.",
			prefix, sas_get_single_ip(wp, rad, 1));
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label23)\" class=\"num\" value=\"%s\" />.",
			prefix, sas_get_single_ip(wp, rad, 2));
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_radius2_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label23)\" class=\"num\" value=\"%s\" />\n",
			prefix, sas_get_single_ip(wp, rad, 3));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label24)</script></div>\n");
		sprintf(var, "%s_radius2_port", prefix);
		websWrite(
			wp,
			"<input name=\"%s_radius2_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label24)\" value=\"%s\" />\n",
			prefix, sas_nvram_default_get(wp, var, "1812"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1812)\");\n//]]>\n</script></span>\n</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label27)</script></div>\n");
		sprintf(var, "%s_radius2_key", prefix);
		websWrite(
			wp,
			"<input type=\"password\" autocomplete=\"off\" id=\"%s_radius2_key\" name=\"%s_radius2_key\" maxlength=\"79\" size=\"32\" value=\"",
			prefix, prefix);

		//tf_webWriteESCNV(wp, var);
		websWrite(wp, "%s", nvram_selget(wp, var));
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s_radius2_unmask\" value=\"0\" onclick=\"setElementMask('%s_radius2_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			prefix, prefix);
	}
#ifdef HAVE_MADWIFI
	if (!showmacformat) {
		char acct[32];
		char vvar[32];

		strcpy(vvar, prefix);
		rep(vvar, '.', 'X');
		sprintf(acct, "%s_acct", prefix); //var);
		websWrite(
			wp,
			"<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label18)</script></div>\n");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idacct', true);\" name=\"%s_acct\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>\n",
			vvar, prefix,
			sas_nvram_default_match(wp, acct, "1", "0") ?
				"checked=\"checked\"" :
				"");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idacct', false);\" name=\"%s_acct\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			vvar, prefix,
			sas_nvram_default_match(wp, acct, "0", "0") ?
				"checked=\"checked\"" :
				"");
		websWrite(wp, "</div>\n");
		//char *rad = sas_nvram_nget(wp, "%s_acct_ipaddr", prefix);
		sprintf(rad, "%s_acct_ipaddr", prefix);

		websWrite(wp, "<div id=\"%s_idacct\">\n", vvar);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label13)</script></div>\n");
		websWrite(
			wp,
			"<input type=\"hidden\" name=\"%s_acct_ipaddr\" value=\"4\" />\n",
			prefix);
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label13)\" class=\"num\" value=\"%s\" />.",
			prefix, sas_get_single_ip(wp, rad, 0));
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label13)\" class=\"num\" value=\"%s\" />.",
			prefix, sas_get_single_ip(wp, rad, 1));
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label13)\" class=\"num\" value=\"%s\" />.",
			prefix, sas_get_single_ip(wp, rad, 2));
		websWrite(
			wp,
			"<input size=\"3\" maxlength=\"3\" name=\"%s_acct_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label13)\" class=\"num\" value=\"%s\" />\n",
			prefix, sas_get_single_ip(wp, rad, 3));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label14)</script></div>\n");
		sprintf(var, "%s_acct_port", prefix);
		websWrite(
			wp,
			"<input name=\"%s_acct_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label14)\" value=\"%s\" />\n",
			prefix, sas_nvram_default_get(wp, var, "1813"));
		websWrite(
			wp,
			"<span class=\"default\"><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"(\" + share.deflt + \": 1813)\");\n//]]>\n</script></span>\n</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label17)</script></div>\n");
		sprintf(var, "%s_acct_key", prefix);
		websWrite(
			wp,
			"<input type=\"password\" autocomplete=\"off\" id=\"%s_acct_key\" name=\"%s_acct_key\" maxlength=\"79\" size=\"32\" value=\"",
			prefix, prefix);
		//tf_webWriteESCNV(wp, var);
		websWrite(wp, "%s", nvram_selget(wp, var));
		websWrite(wp, "\" />&nbsp;&nbsp;&nbsp;\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"%s_acct_unmask\" value=\"0\" onclick=\"setElementMask('%s_acct_key', this.checked)\" >&nbsp;<script type=\"text/javascript\">Capture(share.unmask)</script></input>\n",
			prefix, prefix);
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(
			wp,
			"show_layer_ext(document.getElementsByName(\"%s_acct\"), \"%s_idacct\", %s);\n",
			prefix, vvar,
			nvram_selmatch(wp, acct, "1") ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
	}
#endif
}

EJ_VISIBLE void ej_sas_show_dhcpd_settings(webs_t wp, int argc, char_t **argv)
{
	char *stage_visible_css = "display: none;";

	if (ej_sas_stage_is_visible(wp, argc, argv) == 0) {
		stage_visible_css = "";
	}

	if (getWET()) // dhcpd settings disabled in client bridge mode, so we wont display it
		return;
	websWrite(
		wp,
		"<fieldset style=\"%s\"><legend><script type=\"text/javascript\">Capture(idx.dhcp_legend)</script></legend>\n",
		stage_visible_css);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(
		wp,
		"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_type)</script></div>\n");
	websWrite(
		wp,
		"<select class=\"num\" size=\"1\" name=\"dhcpfwd_enable\" onchange=SelDHCPFWD(this.form.dhcpfwd_enable.selectedIndex,this.form)>\n");
	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"0\\\" %s >\" + idx.dhcp_srv + \"</option>\");\n",
		nvram_selmatch(wp, "dhcpfwd_enable", "0") ?
			"selected=\\\"selected\\\"" :
			"");
	websWrite(
		wp,
		"document.write(\"<option value=\\\"1\\\" %s >\" + idx.dhcp_fwd + \"</option>\");\n",
		nvram_selmatch(wp, "dhcpfwd_enable", "1") ?
			"selected=\\\"selected\\\"" :
			"");
	websWrite(wp, "//]]>\n</script>\n");
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	if (nvram_selmatch(wp, "dhcpfwd_enable", "1")) {
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div>\n");
		char *ipfwd = nvram_selget(wp, "dhcpfwd_ip");

		websWrite(
			wp,
			"<input type=\"hidden\" name=\"dhcpfwd_ip\" value=\"4\" /><input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_0\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_1\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_2\" size=\"3\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_3\" size=\"3\" onblur=\"valid_range(this,0,254,idx.dhcp_srv)\" value=\"%d\"\" /></div>\n",
			sas_get_single_ip(wp, ipfwd, 0),
			sas_get_single_ip(wp, ipfwd, 1),
			sas_get_single_ip(wp, ipfwd, 2),
			sas_get_single_ip(wp, ipfwd, 3));

	} else {
		char buf[20];
		prefix_ip_get("lan_ipaddr", buf, 1);
		websWrite(wp, "<div class=\"setting\">\n");
		// char *nv = nvram_safe_get ("wan_wins");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div><input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"dhcp\" onclick=\"SelDHCP('dhcp',this.form)\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			nvram_selmatch(wp, "lan_proto", "dhcp") ?
				"checked=\"checked\"" :
				"");
		websWrite(
			wp,
			"<input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"static\" onclick=\"SelDHCP('static',this.form)\" %s /><script type=\"text/javascript\">Capture(share.disable)</script></div><input type=\"hidden\" name=\"dhcp_check\" /><div class=\"setting\">\n",
			nvram_selmatch(wp, "lan_proto", "static") ?
				"checked=\"checked\"" :
				"");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_start)</script></div>%s",
			buf);
		websWrite(
			wp,
			"<input class=\"num\" name=\"dhcp_start\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,254,idx.dhcp_start)\" value=\"%s\" %s />",
			nvram_selget(wp, "dhcp_start"),
			nvram_selmatch(wp, "lan_proto", "static") ?
				"disabled class=\"off\"" :
				"");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(
			wp,
			"<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_maxusers)</script></div><input class=\"num\" name=\"dhcp_num\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,999,idx.dhcp_maxusers)\" value=\"%s\" %s/></div>\n",
			nvram_selget(wp, "dhcp_num"),
			nvram_selmatch(wp, "lan_proto", "static") ?
				"disabled class=\"off\"" :
				"");
	}

	websWrite(wp, "</fieldset><br style=\"%s\"/>\n", stage_visible_css);
	return;
}

EJ_VISIBLE void ej_show_sas(webs_t wp, int argc, char_t **argv)
{
#ifndef HAVE_IAS
	websWrite(
		wp,
		"<h2><script type=\"text/javascript\">Capture(sas.title);</script></h2>\n");
	websWrite(wp, "<fieldset>\n");
	websWrite(
		wp,
		"	<legend><script type=\"text/javascript\">Capture(sas.title);</script></legend>\n");
	websWrite(
		wp,
		"	<input type=\"button\" name=\"start_sas\" value=\"Start\" class=\"button\" onclick=\"document.location='SetupAssistant.asp'\">\n");
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<br />\n");
#endif
}

#endif
