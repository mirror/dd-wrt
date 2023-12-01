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
		foreach(var, vaps, next) {
			if (nvram_nmatch("disabled", "%s_mode", var))
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

	asprintf(&config, "{"	//
		 "\"syslog\": true,"	//
		 "\"debug_level\": 1,"	//
		 "\"ipv6\": false,"	//
		 "\"local_mode\": false,"	//
		 "\"sta_block_timeout\": 30000,"	//
		 "\"local_sta_timeout\": 120000,"	//
		 "\"local_sta_update\": 1000,"	//
		 "\"max_neighbor_reports\": 8,"	//
		 "\"max_retry_band\": 5,"	//
		 "\"seen_policy_timeout\": 30000,"	//
		 "\"measurement_report_timeout\": 120000,"	//
		 "\"load_balancing_threshold\": 0,"	//
		 "\"band_steering_threshold\": 5,"	//
		 "\"remote_update_interval\": 1000,"	//
		 "\"remote_node_timeout\": 50,"	//
		 "\"assoc_steering\": false,"	//
		 "\"min_connect_snr\": 0,"	//
		 "\"min_snr\": -82,"	// todo: config
		 "\"min_snr_kick_delay\": 5000,"	//
		 "\"steer_reject_timeout\": 60000,"	//
		 "\"roam_process_timeout\": 5000,"	//
		 "\"roam_scan_snr\": -70,"	// modded
		 "\"roam_scan_tries\": 5,"	// modded
		 "\"roam_scan_timeout\": 0,"	//
		 "\"roam_scan_interval\": 15000,"	//
		 "\"roam_trigger_snr\": -75,"	// modded
		 "\"roam_trigger_interval\": 180000,"	//
		 "\"roam_kick_delay\": 100,"	// modded
		 "\"signal_diff_threshold\": 12,"	// modded
		 "\"initial_connect_delay\": 0,"	//
		 "\"load_kick_enabled\": false,"	//
		 "\"load_kick_threshold\": 75,"	// 
		 "\"load_kick_delay\": 10000,"	//
		 "\"load_kick_min_clients\": 10,"	//
		 "\"load_kick_reason_code\": 5,"	//
		 "\"band_steering_interval\": 120000,"	//
		 "\"band_steering_min_snr\": -60,"	//
		 "\"link_measurement_interval\": 30000,"	//
		 "\"interfaces\": [ "	//
		 "\"br0\" "	//
		 "],"		//
		 "\"ssid_list\": [ "	//
		 "%s"		//
		 "],"		//
		 "\"event_log_types\": ["	//
		 "\"auth_req_deny\","	//
		 "\"assoc_req_deny\","	//
		 "\"load_kick_client\","	//
		 "\"signal_kick\" "	//
		 "] "		//
		 "} ", ssid_list);
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
	sysprintf("usteerd -i br0 -s -v 1&");
	// wait until usteer started
	eval("ubus", "-t", "10", "wait_for", "usteer");
	sleep(2);
	system(cmdline);
	FILE *fp = fopen("/tmp/usteer.json", "wb");
	fprintf(fp, config);
	fclose(fp);
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
	sysprintf("ubusd&");
}

void stop_ubus(void)
{
	stop_process("ubusd", "daemon");
}

#endif
