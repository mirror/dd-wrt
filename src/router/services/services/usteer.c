/*
 * usteer.c
 *
 * Copyright (C) 2023 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_USTEER
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_usteer(void)
{
	// Only start if enabled

	int c = getdevicecount();
	char dev[32];
	char var[32], *next;
	int i;
	char *ssid_list = NULL;
	for (i = 0; i < c; i++) {
		sprintf(dev, "wlan%d", i);
		char busname[64];

		if (nvram_nmatch("disabled", "%s_net_mode", dev))
			continue;
		if (nvram_nmatch("disabled", "%s_mode", dev))
			continue;
		if (!nvram_nmatch("1", "%s_80211k", dev))
			continue;
		if (!nvram_nmatch("1", "%s_80211v", dev))
			continue;
		if (nvram_nmatch("1", "%s_usteer", dev)) {
			sprintf(busname, "hostapd.%s", dev);
			eval("ubus", "-t", "10", "wait_for", busname);
			if (!ssid_list) {
				asprintf(&ssid_list, "\"%s\"", nvram_nget("%s_ssid", dev));
			} else {
				char *newssid;
				char *tmp = ssid_list;
				asprintf(&newssid, "\"%s\"", nvram_nget("%s_ssid", dev));
				if (!strstr(ssid_list, newssid)) {
					asprintf(&ssid_list, "%s,%s", tmp, newssid);
					free(tmp);
				}
				free(newssid);
			}
		}
		char vifs[32];
		sprintf(vifs, "wlan%d_vifs", i);
		char *vaps = nvram_safe_get(vifs);
		foreach(var, vaps, next)
		{
			if (nvram_nmatch("disabled", "%s_mode", var))
				continue;
			if (!nvram_nmatch("1", "%s_80211k", var))
				continue;
			if (!nvram_nmatch("1", "%s_80211v", var))
				continue;

			if (nvram_nmatch("1", "%s_usteer", var)) {
				sprintf(busname, "hostapd.%s", var);
				eval("ubus", "-t", "10", "wait_for", busname);
				if (!ssid_list) {
					asprintf(&ssid_list, "\"%s\"", nvram_nget("%s_ssid", var));
				} else {
					char *newssid;
					char *tmp = ssid_list;
					asprintf(&newssid, "\"%s\"", nvram_nget("%s_ssid", var));
					if (!strstr(ssid_list, newssid)) {
						asprintf(&ssid_list, "%s,%s", tmp, newssid);
						free(tmp);
					}
					free(newssid);
				}
			}
		}
	}
	if (!ssid_list)
		return;
	char *config;

	asprintf(&config,
		 "{" //
		 "\"syslog\": true," //
		 "\"debug_level\": %d," //
		 "\"ipv6\": %s," //
		 "\"local_mode\": %s," //
		 "\"sta_block_timeout\": %d," //
		 "\"local_sta_timeout\": %d," //
		 "\"local_sta_update\": %d," //
		 "\"max_neighbor_reports\": %d," //
		 "\"max_retry_band\": %d," //
		 "\"seen_policy_timeout\": %d," //
		 "\"measurement_report_timeout\": %d," //
		 "\"load_balancing_threshold\": %d," //
		 "\"band_steering_threshold\": %d," //
		 "\"remote_update_interval\": %d," //
		 "\"remote_node_timeout\": %d," //
		 "\"assoc_steering\": %s," //
		 "\"min_connect_snr\": %d," //
		 "\"min_snr\": %d," // todo: config
		 "\"min_snr_kick_delay\": %d," //
		 "\"steer_reject_timeout\": %d," //
		 "\"roam_process_timeout\": %d," //
		 "\"roam_scan_snr\": %d," // modded
		 "\"roam_scan_tries\": %d," // modded
		 "\"roam_scan_timeout\": %d," //
		 "\"roam_scan_interval\": %d," //
		 "\"roam_trigger_snr\": %d," // modded
		 "\"roam_trigger_interval\": %d," //
		 "\"roam_kick_delay\": %d," // modded
		 "\"signal_diff_threshold\": %d," // modded
		 "\"initial_connect_delay\": %d," //
		 "\"load_kick_enabled\": %s," //
		 "\"load_kick_threshold\": %d," //
		 "\"load_kick_delay\": %d," //
		 "\"load_kick_min_clients\": %d," //
		 "\"load_kick_reason_code\": %d," //
		 "\"band_steering_interval\": %d," //
		 "\"band_steering_min_snr\": %d," //
		 "\"link_measurement_interval\": %d," //
		 "\"budget_5ghz\": %d," //
		 "\"prefer_5ghz\": %s," //
		 "\"interfaces\": [ " //
		 "\"br0\" " //
		 "]," //
		 "\"ssid_list\": [ " //
		 "%s" //
		 "]," //
		 "\"event_log_types\": [" //
		 "\"auth_req_deny\"," //
		 "\"assoc_req_deny\"," //
		 "\"load_kick_client\"," //
		 "\"signal_kick\" " //
		 "] " //
		 "} ",
		 nvram_default_geti("usteer_debug_level", 1), //
		 nvram_default_geti("usteer_ipv6", 0) ? (nvram_match("ipv6_enable", "1") ? "true" : "false") : "false", //
		 nvram_default_geti("usteer_local_mode", 0) ? "true" : "false", //
		 nvram_default_geti("usteer_sta_block_timeout", 30000), //
		 nvram_default_geti("usteer_local_sta_timeout", 120000), //
		 nvram_default_geti("usteer_local_sta_update", 1000), //
		 nvram_default_geti("usteer_max_neighbor_reports", 6), //
		 nvram_default_geti("usteer_max_retry_band", 6), //
		 nvram_default_geti("usteer_seen_policy_timeout", 30000), //
		 nvram_default_geti("usteer_measurement_report_timeout",
				    120000), //
		 nvram_default_geti("usteer_load_balancing_threshold", 0), //
		 nvram_default_geti("usteer_band_steering_threshold", 0), //
		 nvram_default_geti("usteer_remote_update_interval", 1000), //
		 nvram_default_geti("usteer_remote_node_timeout", 50), //
		 nvram_default_geti("usteer_assoc_steering", 0) ? "true" : "false", //
		 nvram_default_geti("usteer_min_connect_snr", 0), //
		 nvram_default_geti("usteer_min_snr", 15), //
		 nvram_default_geti("usteer_min_snr_kick_delay", 5000), //
		 nvram_default_geti("usteer_steer_reject_timeout", 60000), //
		 nvram_default_geti("usteer_roam_process_timeout", 5000), //
		 nvram_default_geti("usteer_roam_scan_snr", 20), //
		 nvram_default_geti("usteer_roam_scan_tries", 6), //
		 nvram_default_geti("usteer_roam_scan_timeout", 60000), //
		 nvram_default_geti("usteer_roam_scan_interval", 15000), //
		 nvram_default_geti("usteer_roam_trigger_snr", 15), //
		 nvram_default_geti("usteer_roam_trigger_interval", 180000), //
		 nvram_default_geti("usteer_roam_kick_delay", 100), //
		 nvram_default_geti("usteer_signal_diff_threshold", 12), //
		 nvram_default_geti("usteer_initial_connect_delay", 0), //
		 nvram_default_geti("usteer_load_kick_enabled", 0) ? "true" : "false", //
		 nvram_default_geti("usteer_load_kick_threshold", 75), //
		 nvram_default_geti("usteer_load_kick_delay", 10000), //
		 nvram_default_geti("usteer_load_kick_min_clients", 10), //
		 nvram_default_geti("usteer_load_kick_reason_code", 5), //
		 nvram_default_geti("usteer_band_steering_interval", 120000), //
		 nvram_default_geti("usteer_band_steering_min_snr", 20), //
		 nvram_default_geti("usteer_link_measurement_interval",
				    30000), //
		 nvram_default_geti("usteer_budget_5ghz", 5), //
		 nvram_default_geti("usteer_prefer_5ghz", 0) ? "true" : "false", //
		 ssid_list);
	char *cmdline;
	int len = strlen(config);
	char *newconfig = malloc(len * 2);
	int cnt = 0;
	for (i = 0; i < len; i++) {
		if (config[i] == '\"') {
			newconfig[cnt++] = '\\';
			newconfig[cnt++] = config[i];
		} else {
			newconfig[cnt++] = config[i];
		}
	}
	newconfig[cnt++] = 0;
	asprintf(&cmdline, "ubus call usteer set_config \"%s\"", newconfig);
	free(newconfig);
	eval("usteerd", "-i", "br0", "-s", "-v", "1");
	// wait until usteer started
	eval("ubus", "-t", "10", "wait_for", "usteer");
	system(cmdline);
	if (nvram_match("usteer_debug", "1")) {
		FILE *fp = fopen("/tmp/usteer.json", "wb");
		fprintf(fp, config);
		fclose(fp);
	}
	free(cmdline);
	free(config);
	free(ssid_list);

	cprintf("done\n");
	return;
}

void stop_usteer(void)
{
	stop_process("usteerd", "daemon");
	return;
}

void start_ubus(void)
{
	eval("ubusd");
}

void stop_ubus(void)
{
	stop_process("ubusd", "daemon");
}

#endif
