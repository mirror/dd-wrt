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

	char mbo[64];
	sprintf(mbo, "%s_mbo", var);

	if (v_show_preshared || v_show_owe || v_show_wparadius) {
		char bssft[64];
		char temp[64];
		websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(roaming.ft)</script></legend>");
		sprintf(bssft, "%s_ft", var);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.ft)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_iddomain', true);\" name=\"%s_ft\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(bssft, 1, 0) ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_iddomain', false);\" name=\"%s_ft\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(bssft, 0, 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		char wnm[64];

		websWrite(wp, "<div id=\"%s_iddomain\">\n", vvar);
		{
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "roaming.nas", NULL);
			sprintf(temp, "%s_nas", var);
			websWrite(wp, "<input id=\"%s_nas\" name=\"%s_nas\" maxlength=\"48\" size=\"32\" value=\"%s\" />\n", var, var, nvram_default_get(temp, "ap.example.com"));
			websWrite(wp, "</div>\n");
			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "roaming.domain", NULL);
			sprintf(temp, "%s_domain", var);
			websWrite(wp, "<input id=\"%s_domain\" name=\"%s_domain\" maxlength=\"4\" size=\"6\" onblur=\"valid_domain(this)\" value=\"%s\" />\n", var, var, nvram_default_get(temp, "0000"));
			websWrite(wp, "</div>\n");

			websWrite(wp, "<div class=\"setting\">\n");
			show_caption(wp, "label", "roaming.reassociation_deadline", NULL);
			sprintf(temp, "%s_deadline", var);
			websWrite(wp, "<input id=\"%s_domain\" name=\"%s_deadline\" maxlength=\"6\" size=\"6\" onblur=\"valid_range(this,1000,65535,roaming.reassociation_deadline)\" value=\"%s\" />\n", var, var,
				  nvram_default_get(temp, "1000"));
			websWrite(wp, "</div>\n");

			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.ft_protocol)</script></div>\n");

			sprintf(wnm, "%s_ft_over_ds", var);
			showOptions_trans(wp, wnm, "0 1", (char *[]) {
					  "roaming.ft_over_air", "roaming.ft_over_ds"}, nvram_default_get(wnm, "0"));
			websWrite(wp, "</div>\n");
		}
		websWrite(wp, "</div>\n");
		websWrite(wp, "<script>\n//<![CDATA[\n ");
		websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_ft\"), \"%s_iddomain\", %s);\n", var, vvar, nvram_matchi(bssft, 1) ? "true" : "false");
		websWrite(wp, "//]]>\n</script>\n");
		websWrite(wp, "</fieldset> <br />\n");

	}

	char s80211v[64];
	sprintf(s80211v, "%s_80211v", var);
	websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(roaming.s80211v)</script></legend>");
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.s80211v)</script></div>\n");
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
	{
		sprintf(adv, "%s_time_advertisement", var);
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.time_advertisement)</script></div>\n");
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

		showRadioPrefix(wp, "roaming.wnm_sleep_mode", "wnm_sleep_mode", var);
		showRadioPrefix(wp, "roaming.wnm_sleep_mode_no_keys", "wnm_sleep_mode_no_keys", var);
		showRadioPrefix(wp, "roaming.bss_transition", "bss_transition", var);
		showRadioPrefix(wp, "roaming.proxy_arp", "proxy_arp", var);
	}
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset> </br>\n");
	websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(roaming.s80211k)</script></legend>");

	char s80211k[64];
	sprintf(s80211k, "%s_80211k", var);
	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.s80211k)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_id80211k', true);show_layer_ext(this, '%s_id80211k2', true);\" name=\"%s_80211k\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  var, var, var, nvram_default_matchi(s80211k, 1, 0) ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_id80211k', false);show_layer_ext(this, '%s_id80211k2', false);\" name=\"%s_80211k\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  var, var, var, nvram_default_matchi(s80211k, 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");
	websWrite(wp, "<div id=\"%s_id80211k\">\n", var);
	{
		showRadioPrefix(wp, "roaming.rrm_neighbor_report", "rrm_neighbor_report", var);
		showRadioPrefix(wp, "roaming.rrm_beacon_report", "rrm_beacon_report", var);
	}
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset> <br />\n");

	if (v_show_preshared || v_show_owe || v_show_wparadius) {
		websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(roaming.mbo)</script></legend>");
		websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.mbo)</script></div>\n");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_idmbo', true);\" name=\"%s_mbo\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(mbo, 1, has_ax(var) ? 1 : 0) ? "checked=\"checked\"" : "");
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_idmbo', false);\" name=\"%s_mbo\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
			  vvar, var, nvram_default_matchi(mbo, 0, has_ax(var) ? 1 : 0) ? "checked=\"checked\"" : "");
		websWrite(wp, "</div>\n");
		sprintf(wnm, "%s_mbo_cell_data_conn_pref", var);
		websWrite(wp, "<div id=\"%s_idmbo\">\n", vvar);
		{
			websWrite(wp, "<div class=\"setting\">\n");
			websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.mbo_cell_data_conn_pref)</script></div>\n");
			showOptions_trans(wp, wnm, "0 1 255", (char *[]) {
					  "share.excluded", "share.not_prefered", "share.prefered"}, nvram_default_get(wnm, "0"));
			websWrite(wp, "</div>\n");
		}
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset> <br />\n");
	}

	char usteer[64];
	sprintf(usteer, "%s_usteer", var);

	websWrite(wp, "<div id=\"%s_id80211k2\">\n", var);
	{
		websWrite(wp, "<div id=\"%s_id80211v2\">\n", var);
		{

			websWrite(wp, "<fieldset><legend><script type=\"text/javascript\">Capture(roaming.usteer)</script></legend>");
			showRadioPrefix(wp, "roaming.usteer", "usteer", var);
			websWrite(wp, "</fieldset> <br />\n");
		}
		websWrite(wp, "</div>\n");
	}
	websWrite(wp, "</div>\n");

	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211v\"), \"%s_id80211v\", %s);\n", var, var, nvram_matchi(s80211v, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211k\"), \"%s_id80211k\", %s);\n", var, var, nvram_matchi(s80211k, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211v\"), \"%s_id80211v2\", %s);\n", var, var, nvram_matchi(s80211v, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_80211k\"), \"%s_id80211k2\", %s);\n", var, var, nvram_matchi(s80211k, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_usteer\"), \"%s_idusteer\", %s);\n", var, var, nvram_matchi(usteer, 1) ? "true" : "false");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_mbo\"), \"%s_idmbo\", %s);\n", var, vvar, nvram_matchi(mbo, 1) ? "true" : "false");
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
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(roaming.roaming)</script> %s</h2>\n", prefix);
	websWrite(wp, "<fieldset>\n");
	// cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
	websWrite(wp, "<legend><script type=\"text/javascript\">Capture(share.pintrface)</script> %s SSID [", getNetworkLabel(wp, IFMAP(prefix)));
	tf_webWriteESCNV(wp, ssid);	// fix for broken html page if ssid
	// contains html tag
	websWrite(wp, "] HWAddr [%s]</legend>\n", nvram_safe_get(mac));
	show_roaming(wp, prefix);
	websWrite(wp, "</fieldset>\n<br />\n");
	foreach(var, vifs, next) {
		if (nvram_nmatch("disabled", "%s_net_mode", var))
			continue;
		if (nvram_nmatch("disabled", "%s_mode", var))
			continue;
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
		if (nvram_nmatch("disabled", "%s_net_mode", buf))
			continue;
		if (nvram_nmatch("disabled", "%s_mode", buf))
			continue;
		ej_show_roaming_single(wp, argc, argv, buf);
	}
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(roaming.usteer_options)</script></h2>\n");
	websWrite(wp, "<fieldset>");
	showInputNum(wp, "roaming.debug_level", "usteer_debug_level", 1, 1, 1);
	showRadio(wp, "bmenu.setupipv6", "usteer_ipv6");
	showRadio(wp, "roaming.local_mode", "usteer_local_mode");
	showInputNum(wp, "roaming.sta_block_timeout", "usteer_sta_block_timeout", 6, 6, 30000);
	showInputNum(wp, "roaming.local_sta_timeout", "usteer_local_sta_timeout", 6, 6, 120000);
	showInputNum(wp, "roaming.local_sta_update", "usteer_local_sta_update", 6, 6, 1000);
	showInputNum(wp, "roaming.max_neighbor_reports", "usteer_max_neighbor_reports", 2, 2, 6);
	showInputNum(wp, "roaming.max_retry_band", "usteer_max_retry_band", 2, 2, 6);
	showInputNum(wp, "roaming.seen_policy_timeout", "usteer_seen_policy_timeout", 6, 6, 30000);
	showInputNum(wp, "roaming.measurement_report_timeout", "usteer_measurement_report_timeout", 6, 6, 120000);
	showInputNum(wp, "roaming.load_balancing_threshold", "usteer_load_balancing_threshold", 4, 4, 0);
	showInputNum(wp, "roaming.band_steering_threshold", "usteer_band_steering_threshold", 4, 4, 0);
	showInputNum(wp, "roaming.remote_update_interval", "usteer_remote_update_interval", 6, 6, 1000);
	showInputNum(wp, "roaming.remote_node_timeout", "usteer_remote_node_timeout", 6, 6, 50);
	showRadio(wp, "roaming.assoc_steering", "usteer_assoc_steering");
	showInputNum(wp, "roaming.min_connect_snr", "usteer_min_connect_snr", 4, 4, 0);
	showInputNum(wp, "roaming.min_snr", "usteer_min_snr", 4, 4, 15);
	showInputNum(wp, "roaming.min_snr_kick_delay", "usteer_min_snr_kick_delay", 6, 6, 5000);
	showInputNum(wp, "roaming.steer_reject_timeout", "usteer_steer_reject_timeout", 6, 6, 60000);
	showInputNum(wp, "roaming.roam_process_timeout", "usteer_roam_process_timeout", 6, 6, 5000);
	showInputNum(wp, "roaming.roam_scan_snr", "usteer_roam_scan_snr", 4, 4, 20);
	showInputNum(wp, "roaming.roam_scan_tries", "usteer_roam_scan_tries", 2, 2, 6);
	showInputNum(wp, "roaming.roam_scan_timeout", "usteer_roam_scan_timeout", 6, 6, 60000);
	showInputNum(wp, "roaming.roam_scan_interval", "usteer_roam_scan_interval", 6, 6, 15000);
	showInputNum(wp, "roaming.roam_trigger_snr", "usteer_roam_trigger_snr", 4, 4, 15);
	showInputNum(wp, "roaming.roam_trigger_interval", "usteer_roam_trigger_interval", 6, 6, 180000);
	showInputNum(wp, "roaming.roam_kick_delay", "usteer_roam_kick_delay", 6, 6, 100);
	showInputNum(wp, "roaming.signal_diff_threshold", "usteer_signal_diff_threshold", 4, 4, 12);
	showInputNum(wp, "roaming.initial_connect_delay", "usteer_initial_connect_delay", 6, 6, 0);

	showInputNum(wp, "roaming.band_steering_interval", "usteer_band_steering_interval", 6, 6, 120000);
	showInputNum(wp, "roaming.band_steering_min_snr", "usteer_band_steering_min_snr", 4, 4, 20);
	showInputNum(wp, "roaming.link_measurement_interval", "usteer_link_measurement_interval", 6, 6, 30000);

	websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.load_kick_enabled)</script></div>\n");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, 'id_load_kick', true);\" name=\"usteer_load_kick_enabled\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		  nvram_default_matchi("usteer_load_kick_enabled", 1, 0) ? "checked=\"checked\"" : "");
	websWrite(wp,
		  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, 'id_load_kick', false);\" name=\"usteer_load_kick_enabled\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
		  nvram_default_matchi("usteer_load_kick_enabled", 0, 0) ? "checked=\"checked\"" : "");
	websWrite(wp, "</div>\n");

	websWrite(wp, "<div id=\"id_load_kick\">\n");
	{
		showInputNum(wp, "roaming.load_kick_threshold", "usteer_load_kick_threshold", 4, 4, 75);
		showInputNum(wp, "roaming.load_kick_delay", "usteer_load_kick_delay", 7, 6, 10000);
		showInputNum(wp, "roaming.load_kick_min_clients", "usteer_load_kick_min_clients", 4, 4, 10);
		showInputNum(wp, "roaming.load_kick_reason_code", "usteer_load_kick_reason_code", 2, 2, 5);
	}
	websWrite(wp, "</div>\n");
	websWrite(wp, "</fieldset> <br />\n");

	websWrite(wp, "<script>\n//<![CDATA[\n ");
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"usteer_load_kick_enabled\"), \"id_load_kick\", %s);\n", nvram_matchi("usteer_load_kick_enabled", 1) ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

	return;
}
