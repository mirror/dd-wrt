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
	{
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
	}
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
	{
		showRadioPrefix(wp, "wl_basic.rrm_neighbor_report", "rrm_neighbor_report", var);
		showRadioPrefix(wp, "wl_basic.rrm_beacon_report", "rrm_beacon_report", var);
	}
	websWrite(wp, "</div>\n");
	char usteer[64];
	sprintf(usteer, "%s_usteer", var);
	char load_kick[64];
	sprintf(load_kick, "%s_load_kick_enabled", var);

	websWrite(wp, "<div id=\"%s_id80211k2\">\n", var);
	{
		websWrite(wp, "<div id=\"%s_id80211v2\">\n", var);
		{
			showRadioPrefix(wp, "wl_basic.usteer", "usteer", var);
			websWrite(wp, "<div id=\"%s_idusteer\">\n", var);
			{
				showInputNumPrefix(wp, "roaming.debug_level", "debug_level", var, 2, 1, 1);
				showRadioPrefix(wp, "bmenu.setupipv6", "ipv6", var);
				showRadioPrefix(wp, "roaming.local_mode", "local_mode", var);
				showInputNumPrefix(wp, "roaming.sta_block_timeout", "sta_block_timeout", var, 7, 6, 30000);
				showInputNumPrefix(wp, "roaming.local_sta_timeout", "local_sta_timeout", var, 7, 6, 120000);
				showInputNumPrefix(wp, "roaming.local_sta_update", "local_sta_update", var, 7, 6, 1000);
				showInputNumPrefix(wp, "roaming.max_neighbor_reports", "max_neighbor_reports", var, 3, 2, 6);
				showInputNumPrefix(wp, "roaming.max_retry_band", "max_retry_band", var, 3, 2, 6);
				showInputNumPrefix(wp, "roaming.seen_policy_timeout", "seen_policy_timeout", var, 7, 6, 30000);
				showInputNumPrefix(wp, "roaming.measurement_report_timeout", "measurement_report_timeout", var, 7, 6, 120000);
				showInputNumPrefix(wp, "roaming.load_balancing_threshold", "load_balancing_threshold", var, 4, 3, 0);
				showInputNumPrefix(wp, "roaming.band_steering_threshold", "band_steering_threshold", var, 4, 3, 0);
				showInputNumPrefix(wp, "roaming.remote_update_interval", "remote_update_interval", var, 7, 6, 1000);
				showInputNumPrefix(wp, "roaming.remote_node_timeout", "remote_node_timeout", var, 7, 6, 50);
				showRadioPrefix(wp, "roaming.assoc_steering", "assoc_steering", var);
				showInputNumPrefix(wp, "roaming.min_connect_snr", "min_connect_snr", var, 4, 3, 0);
				showInputNumPrefix(wp, "roaming.min_snr", "min_snr", var, 4, 3, -82);
				showInputNumPrefix(wp, "roaming.min_snr_kick_delay", "min_snr_kick_delay", var, 7, 6, 5000);
				showInputNumPrefix(wp, "roaming.steer_reject_timeout", "steer_reject_timeout", var, 7, 6, 60000);
				showInputNumPrefix(wp, "roaming.roam_process_timeout", "roam_process_timeout", var, 7, 6, 5000);
				showInputNumPrefix(wp, "roaming.roam_scan_snr", "roam_scan_snr", var, 4, 3, -70);
				showInputNumPrefix(wp, "roaming.roam_scan_tries", "roam_scan_tries", var, 3, 2, 6);
				showInputNumPrefix(wp, "roaming.roam_scan_timeout", "roam_scan_timeout", var, 7, 6, 60000);
				showInputNumPrefix(wp, "roaming.roam_scan_interval", "roam_scan_interval", var, 7, 6, 15000);
				showInputNumPrefix(wp, "roaming.roam_trigger_snr", "roam_trigger_snr", var, 4, 3, -75);
				showInputNumPrefix(wp, "roaming.roam_trigger_interval", "roam_trigger_interval", var, 7, 6, 180000);
				showInputNumPrefix(wp, "roaming.roam_kick_delay", "roam_kick_delay", var, 7, 6, 100);
				showInputNumPrefix(wp, "roaming.signal_diff_threshold", "signal_diff_threshold", var, 4, 3, 12);
				showInputNumPrefix(wp, "roaming.initial_connect_delay", "initial_connect_delay", var, 7, 6, 0);

				showInputNumPrefix(wp, "roaming.band_steering_interval", "band_steering_interval", var, 7, 6, 120000);
				showInputNumPrefix(wp, "roaming.band_steering_min_snr", "band_steering_min_snr", var, 3, 2, -60);
				showInputNumPrefix(wp, "roaming.link_measurement_interval", "link_measurement_interval", var, 7, 6, 30000);

				websWrite(wp, "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(roaming.load_kick_enabled)</script></div>\n");
				websWrite(wp,
					  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" onclick=\"show_layer_ext(this, '%s_id_load_kick', true);\" name=\"%s_load_kick_enabled\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
					  var, var, nvram_default_matchi(load_kick, 1, 0) ? "checked=\"checked\"" : "");
				websWrite(wp,
					  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" onclick=\"show_layer_ext(this, '%s_id_load_kick', false);\" name=\"%s_load_kick_enabled\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>&nbsp;\n",
					  var, var, nvram_default_matchi(load_kick, 0, 0) ? "checked=\"checked\"" : "");
				websWrite(wp, "</div>\n");

				websWrite(wp, "<div id=\"%s_id_load_kick\">\n", var);
				{
					showInputNumPrefix(wp, "roaming.load_kick_threshold", "load_kick_threshold", var, 4, 3, 75);
					showInputNumPrefix(wp, "roaming.load_kick_delay", "load_kick_delay", var, 7, 6, 10000);
					showInputNumPrefix(wp, "roaming.load_kick_min_clients", "load_kick_min_clients", var, 4, 3, 10);
					showInputNumPrefix(wp, "roaming.load_kick_reason_code", "load_kick_reason_code", var, 3, 2, 5);
				}
				websWrite(wp, "</div>\n");
			}
			websWrite(wp, "</div>\n");
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
	websWrite(wp, "show_layer_ext(document.getElementsByName(\"%s_load_kick_enabled\"), \"%s_id_load_kick\", %s);\n", var, var, nvram_matchi(load_kick, 1) ? "true" : "false");
	websWrite(wp, "//]]>\n</script>\n");

}

#if defined(HAVE_RT2880) && !defined(HAVE_MT76)
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

static void ej_show_roaming_single(webs_t wp, int argc, char_t **argv, char *prefix)
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

EJ_VISIBLE void ej_show_roaming(webs_t wp, int argc, char_t **argv)
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
