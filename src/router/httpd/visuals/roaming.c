/*
 * roaming.c
 *
 * Copyright (C) 2005 - 2023 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#ifdef __UCLIBC__
#include <error.h>
#endif
#include <signal.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <broadcom.h>
#include <wlioctl.h>
#include <wlutils.h>

void show_roaming(webs_t wp, char *var)
{
	char vakm[32];
	sprintf(vakm, "%s_akm", var);
	char *akm = nvram_safe_get(vakm);

	int v_show_preshared = 0;
	int v_show_owe = 0;
	int v_show_wparadius = 0;
	if (strstr(akm, "psk") || strstr(akm, "psk2") || strstr(akm, "psk2-sha256") || strstr(akm, "psk3"))
		v_show_preshared = 1;
	if (strstr(akm, "owe"))
		v_show_owe = 1;
	if (strstr(akm, "wpa") || strstr(akm, "wpa2") || strstr(akm, "wpa2-sha256") || strstr(akm, "wpa3") || strstr(akm, "wpa3-192") || strstr(akm, "wpa3-128"))
		v_show_wparadius = 1;

	char vvar[32];
	strcpy(vvar, var);
	rep(vvar, '.', 'X');
	if (v_show_preshared || v_show_owe || v_show_wparadius) {
		char bssft[64];
		char temp[64];
		sprintf(bssft, "%s_ft", var);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.ft)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_iddomain', true);\" name=\"%s_ft\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(bssft, 1, 0) ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_iddomain', false);\" name=\"%s_ft\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(bssft, 0, 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div id=\"%s_iddomain\">\n", vvar);
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wpa.nas", NULL);
		sprintf(temp, "%s_nas", var);
		websWrite(wp, "<input id=\"%s_nas\" name=\"%s_nas\" maxlength=\"48\" size=\"32\" value=\"%s\" />\n", var, var, nvram_default_get(temp, "ap.example.com"));
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wpa.domain", NULL);
		sprintf(temp, "%s_domain", var);
		websWrite(wp, "<input id=\"%s_domain\" name=\"%s_domain\" maxlength=\"4\" size=\"6\" onblur=\"valid_domain(this)\" value=\"%s\" />\n", var, var, nvram_default_get(temp, "0000"));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<div class=\"setting\">\n");
		show_caption(wp, "label", "wpa.reassociation_deadline", NULL);
		sprintf(temp, "%s_deadline", var);
		websWrite(wp, "<input id=\"%s_domain\" name=\"%s_deadline\" maxlength=\"6\" size=\"6\" onblur=\"valid_range(this,1000,65535,wpa.reassociation_deadline)\" value=\"%s\" />\n", var, var,
			  nvram_default_get(temp, "1000"));
		websWrite(wp, "</div>\n");
		char wnm[64];

		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.ft_protocol)</script></div>\n");

		sprintf(wnm, "%s_ft_over_ds", var);
		showOptions_trans(wp, wnm, "0 1", (char *[]) { "wpa.ft_over_air", "wpa.ft_over_ds" }, nvram_default_get(wnm, "0"));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_ft\"), \"%s_iddomain\", %s);\n", var, vvar, nvram_matchi(bssft, 1) ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");

		char mbo[64];
		sprintf(mbo, "%s_mbo", var);
		nvram_default_get(mbo, has_ax(var) ? "1" : "0");

		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.mbo)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idmbo', true);\" name=\"%s_mbo\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(mbo, 1, 0) ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idmbo', false);\" name=\"%s_mbo\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(mbo, 0, 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");

		sprintf(wnm, "%s_mbo_cell_data_conn_pref", var);
		websWrite(wp, "<div id=\"%s_idmbo\">\n", vvar);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.mbo_cell_data_conn_pref)</script></div>\n");
		showOptions_trans(wp, wnm, "0 1 255", (char *[]) { "share.excluded", "share.not_prefered", "share.prefered" }, nvram_default_get(wnm, "0"));
		websWrite(wp, "</div>\n");

		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_mbo\"), \"%s_idmbo\", %s);\n", var, vvar, nvram_matchi(mbo, 1) ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");

	}

	char s80211v[64];
	sprintf(s80211v, "%s_80211v", var);
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.s80211v)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_id80211v', true);show_layer_ext(this, '%s_id80211v2', true);\" name=\"%s_80211v\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  var, var, var, nvram_default_matchi(s80211v, 1, 0) ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_id80211v', false);show_layer_ext(this, '%s_id80211v2', false);\" name=\"%s_80211v\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  var, var, var, nvram_default_matchi(s80211v, 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	char wnm[64];
	char adv[64];
	websWrite(wp, "<div id=\"%s_id80211v\">\n", var);

	sprintf(adv, "%s_time_advertisement", var);
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.time_advertisement)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_id_time_zone', true);\" name=\"%s_time_advertisement\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  var, var, nvram_default_matchi(adv, 1, 0) ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_id_time_zone', false);\" name=\"%s_time_advertisement\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  var, var, nvram_default_matchi(adv, 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");

	sprintf(wnm, "%s_time_zone", var);
	websWrite(wp, "<div id=\"%s_id_time_zone\">\n", var);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.timeset)</script></div>\n");
	websWrite(wp, "<select name=\"%s_time_zone\">\n", var);
	char *tz = nvram_default_get(wnm, nvram_safe_get("time_zone"));
	int i;
	for (i = 0; (allTimezones[i].tz_name != NULL); i++) {
		websWrite(wp, "<option value=\"%s\" %s>%s</option>\n", allTimezones[i].tz_name, !strcmp(allTimezones[i].tz_name, tz) ? "selected=\"selected\"" : "", allTimezones[i].tz_name);
	}
	websWrite(wp, "</select>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_time_advertisement\"), \"%s_id_time_zone\", %s);\n", var, var, nvram_matchi(adv, 1) ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

	showRadioPrefix(wp, "wl_basic.wnm_sleep_mode", "wnm_sleep_mode", var);
	showRadioPrefix(wp, "wl_basic.wnm_sleep_mode_no_keys", "wnm_sleep_mode_no_keys", var);
	showRadioPrefix(wp, "wl_basic.bss_transition", "bss_transition", var);
	showRadioPrefix(wp, "wl_basic.proxy_arp", "proxy_arp", var);

	websWrite(wp, "</div>\n");

	char s80211k[64];
	sprintf(s80211k, "%s_80211k", var);
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.s80211k)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_id80211k', true);show_layer_ext(this, '%s_id80211k2', true);\" name=\"%s_80211k\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  var, var, var, nvram_default_matchi(s80211k, 1, 0) ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_id80211k', false);show_layer_ext(this, '%s_id80211k2', false);\" name=\"%s_80211k\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  var, var, var, nvram_default_matchi(s80211k, 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div id=\"%s_id80211k\">\n", var);
	showRadioPrefix(wp, "wl_basic.rrm_neighbor_report", "rrm_neighbor_report", var);
	showRadioPrefix(wp, "wl_basic.rrm_beacon_report", "rrm_beacon_report", var);
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div id=\"%s_id80211k2\">\n", var);
	websWrite(wp, "<div id=\"%s_id80211v2\">\n", var);
	showRadioPrefix(wp, "wl_basic.usteer", "usteer", var);
	websWrite(wp, "</div>\n");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211v\"), \"%s_id80211v\", %s);\n", var, var, nvram_matchi(s80211v, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211k\"), \"%s_id80211k\", %s);\n", var, var, nvram_matchi(s80211k, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211v\"), \"%s_id80211v2\", %s);\n", var, var, nvram_matchi(s80211v, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211k\"), \"%s_id80211k2\", %s);\n", var, var, nvram_matchi(s80211k, 1) ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

}
#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

static void ej_show_roaming_single(webs_t wp, int argc, char_t ** argv, char *prefix)
{
	char *next;
	char var[80];
	char ssid[80];
	char mac[18];

	sprintf(mac, "%s_hwaddr", prefix);
	char *vifs = nvram_nget("%s_vifs", prefix);

	if (vifs == NULL)
		return;
	sprintf(ssid, "%s_ssid", prefix);
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(wl_basic.roaming)</script> %s</h2>\n", prefix);
	websWrite(wp, "<fieldset>\n");
	// cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s SSID [", getNetworkLabel(wp, IFMAP(prefix)));
	tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
	// contains html tag
	websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(mac));
	show_roaming(wp, prefix);
	websWrite(wp, "</fieldset>\n<br />\n");
	foreach(var, vifs, next) {
		sprintf(ssid, "%s_ssid", var);
		websWrite(wp, "<fieldset>\n");
		// cprintf("getting %s %s\n", ssid,nvram_safe_get(ssid));
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.vintrface)</script> %s SSID [", getNetworkLabel(wp, IFMAP(var)));
		tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
		// contains html tag
		sprintf(mac, "%s_hwaddr", var);
		if (nvram_exists(mac))
			websWrite(wp, "] HWAddr [%s", nvram_safe_get(mac));

		websWrite(wp, "]</legend>\n");
		show_roaming(wp, var);
		websWrite(wp, "</fieldset>\n<br />\n");
	}
}

EJ_VISIBLE void ej_show_roaming(webs_t wp, int argc, char_t ** argv)
{
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];
		sprintf(buf, WIFINAME "%d", i);
		ej_show_roaming_single(wp, argc, argv, buf);
	}
	return;
}
