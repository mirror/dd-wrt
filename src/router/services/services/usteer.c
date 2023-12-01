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
	if (!nvram_invmatchi("usteer_enable", 0))
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
		 "\"remote_node_timeout\": 10,"	//
		 "\"assoc_steering\": false,"	//
		 "\"min_connect_snr\": 0,"	//
		 "\"min_snr\": 0,"	//
		 "\"min_snr_kick_delay\": 5000,"	//
		 "\"steer_reject_timeout\": 60000,"	//
		 "\"roam_process_timeout\": 5000,"	//
		 "\"roam_scan_snr\": 0,"	//
		 "\"roam_scan_tries\": 3,"	//
		 "\"roam_scan_timeout\": 0,"	//
		 "\"roam_scan_interval\": 10000,"	//
		 "\"roam_trigger_snr\": 0,"	//
		 "\"roam_trigger_interval\": 60000,"	//
		 "\"roam_kick_delay\": 10000,"	//
		 "\"signal_diff_threshold\": 0,"	//
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
		 "\"event_log_types\": ["	//
		 "\"auth_req_deny\","	//
		 "\"assoc_req_deny\","	//
		 "\"load_kick_client\","	//
		 "\"signal_kick\" "	//
		 "] "		//
		 "} ");
	char *cmdline;
	asprintf(&cmdline, "ubus call usteer set_config \"%s\"", config);
	sysprintf("usteerd -i br0 -s -v 1&");
	// wait until usteer started
	eval("ubus", "-t", "10", "wait_for", "usteer");
	system(cmdline);
	free(cmdline);
	free(config);

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
