/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 * Copyright 2014, ASUSTeK Inc.
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND ASUS GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 */

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include <utils.h>

#include <qtnapi.h>

//#define dbG(a,...)

#define dbg(fmt, args...) fprintf(stderr, fmt, ##args)
#define dbG(fmt, args...) \
	dbg("%s(0x%04x): " fmt, __FUNCTION__, __LINE__, ##args)

static void cleanup(void)
{
	remove("/var/run/qtn_monitor.pid");
	exit(0);
}

static void qtn_monitor_exit(int sig)
{
	if (sig == SIGTERM)
		cleanup();
}

void rpc_parse_nvram_from_httpd(void)
{
	if (!rpc_qtn_ready())
		return;

	// rpc_qcsapi_set_SSID(WIFINAME, value);
	rpc_qcsapi_set_SSID_broadcast(WIFINAME,
				      nvram_default_get("wl1_closed", "0"));
	char *netmode = nvram_safe_get("wl1_net_mode");
	if ((!strcmp(netmode, "mixed") || !strcmp(netmode, "ac-only")))
		rpc_qcsapi_set_vht("1");
	else
		rpc_qcsapi_set_vht("0");

	const char *country = getIsoName(nvram_safe_get("wl_regdomain"));
	char lower[32];
	int i;
	if (*country) {
		for (i = 0; i < strlen(country); i++)
			lower[i] = tolower(country[i]);
		lower[i] = 0;
	} else {
		strcpy(lower, "us");
	}
	rpc_qcsapi_set_region("wifi0", lower);

	// rpc_qcsapi_set_bw(value);
	// rpc_qcsapi_set_channel(value);
	rpc_qcsapi_set_beacon_type(WIFINAME, nvram_safe_get("wl1_akm"));
	rpc_qcsapi_set_WPA_encryption_modes(WIFINAME,
					    nvram_safe_get("wl1_crypto"));
	rpc_qcsapi_set_key_passphrase(WIFINAME, nvram_safe_get("wl1_wpa_psk"));
	rpc_qcsapi_set_dtim(nvram_safe_get("wl1_dtim"));
	rpc_qcsapi_set_beacon_interval(nvram_safe_get("wl1_bcn"));
	rpc_set_radio(1, 0, nvram_default_geti("wl1_radio", 1));
	//rpc_update_macmode(nvram_safe_get("wl1_macmode"));
	//rpc_update_wlmaclist();
	//rpc_update_wdslist();
	//rpc_update_wdslist();
	//rpc_update_wds_psk(nvram_safe_get("wl1_wds_psk"));
	rpc_update_ap_isolate(WIFINAME, nvram_geti("wl1_ap_isolate"));

	char *viflist[4] = { "wl1.1", "wl1.2", "wl1.3" };
	int cnt = 0;
	for (i = 0; i < 3; i++) {
		char *next;
		char var[80];
		char *vifs = nvram_safe_get("wl1_vifs");
		int found = 0;
		foreach(var, vifs, next)
		{
			if (!strcmp(var, viflist[cnt++])) {
				found = 1;
				char mbss[32];
				sprintf(mbss, "%s_bss_enabled", var);
				rpc_update_mbss(mbss, "1");

				char ssid[32];
				sprintf(ssid, "%s_ssid", var);
				rpc_update_mbss(ssid, nvram_safe_get(ssid));

				char closed[32];
				sprintf(ssid, "%s_closed", var);
				rpc_update_mbss(closed, nvram_safe_get(closed));

				char psk[32];
				sprintf(ssid, "%s_wpa_psk", var);
				rpc_update_mbss(psk, nvram_safe_get(psk));

				char rekey[32];
				sprintf(rekey, "%s_wpa_gtk_rekey", var);
				rpc_update_mbss(rekey, nvram_safe_get(rekey));

				char auth[32];
				sprintf(auth, "%s_akm", var);
				rpc_update_mbss(auth, nvram_safe_get(auth));
			}
		}
		if (!found) {
			char mbss[32];
			sprintf(mbss, "%s_bss_enabled", viflist[i]);
			rpc_update_mbss(mbss, "0");
		}
	}

	//      rpc_show_config();
}

static int qtn_monitor_main(int argc, char *argv[])
{
	FILE *fp;
	sigset_t sigs_to_catch;
	int ret, retval = 0;
	time_t start_time = uptime();

	/* write pid */
	if ((fp = fopen("/var/run/qtn_monitor.pid", "w")) != NULL) {
		fprintf(fp, "%d", getpid());
		fclose(fp);
	}

	/* set the signal handler */
	sigemptyset(&sigs_to_catch);
	sigaddset(&sigs_to_catch, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sigs_to_catch, NULL);

	signal(SIGTERM, qtn_monitor_exit);

QTN_RESET:
	ret = rpc_qcsapi_init(1);
	if (ret < 0) {
		dbG("rpc_qcsapi_init error, return: %d\n", ret);
		retval = -1;
		goto ERROR;
	}
#if 0 /* replaced by STATELESS, send configuration from brcm to qtn */
	else if (nvram_get_int("qtn_restore_defaults")) {
		nvram_unset("qtn_restore_defaults");
		rpc_qcsapi_restore_default_config(0);
		dbG("Restaring Qcsapi init...\n");
		sleep(15);
		goto QTN_RESET;
	}
#endif

#if 0
	if (nvram_get_int("sw_mode") == SW_MODE_AP && nvram_get_int("wlc_psta") == 1 && nvram_get_int("wlc_band") == 1) {
		dbG("[sw_mode] start QTN PSTA mode\n");
		start_psta_qtn();
	}
#endif

	ret = qcsapi_init();
	if (ret < 0) {
		dbG("Qcsapi qcsapi_init error, return: %d\n", ret);
		retval = -1;
		goto ERROR;
	} else
		dbG("Qcsapi qcsapi init takes %ld seconds\n",
		    uptime() - start_time);

	eval("ifconfig", "br0:2", "down");
	eval("killall", "tftpd");
	nvram_seti("qtn_ready", 1);

	dbG("[QTN] update router_command.sh from brcm to qtn\n");
	qcsapi_wifi_run_script("set_test_mode", "update_router_command");

#if 1 /* STATELESS */
	//      if(nvram_get_int("sw_mode") == SW_MODE_AP &&
	//              nvram_get_int("wlc_psta") == 1 &&
	//              nvram_get_int("wlc_band") == 1){
	//              dbG("[sw_mode] skip start_psta_qtn, QTN will run scripts automatically\n");
	//              // start_psta_qtn();
	//      }else
	if (nvram_match("wl1_mode", "ap")) {
		//              dbG("[***] rpc_parse_nvram_from_httpd\n");
		rpc_parse_nvram_from_httpd(); /* wifi0 */

		dbG("[sw_mode] skip start_ap_qtn, QTN will run scripts automatically\n");
		// start_ap_qtn();
		qcsapi_mac_addr wl_mac_addr;
		ret = rpc_qcsapi_interface_get_mac_addr(WIFINAME, &wl_mac_addr);
		if (ret < 0)
			dbG("rpc_qcsapi_interface_get_mac_addr, return: %d\n",
			    ret);
		else {
			char tmp[32];
			nvram_set("1:macaddr",
				  ether_etoa((struct ether_addr *)&wl_mac_addr,
					     tmp));
			nvram_set("wl1_hwaddr",
				  ether_etoa((struct ether_addr *)&wl_mac_addr,
					     tmp));
		}

		//              rpc_update_wdslist();

		//              ret = qcsapi_wps_set_ap_pin(WIFINAME, nvram_safe_get("wps_device_pin"));
		//              if (ret < 0)
		//                      dbG("Qcsapi qcsapi_wps_set_ap_pin %s error, return: %d\n", WIFINAME, ret);

		//              ret = qcsapi_wps_registrar_set_pp_devname(WIFINAME, 0, (const char *) get_productid());
		//              if (ret < 0)
		//                      dbG("Qcsapi qcsapi_wps_registrar_set_pp_devname %s error, return: %d\n", WIFINAME, ret);

		ret = rpc_qcsapi_wifi_disable_wps(WIFINAME, 1);
		if (ret < 0)
			dbG("Qcsapi rpc_qcsapi_wifi_disable_wps %s error, return: %d\n",
			    WIFINAME, ret);

		rpc_set_radio(1, 0, nvram_default_geti("wl1_radio", 1));
	}
#endif

	/*	if(nvram_get_int("sw_mode") == SW_MODE_ROUTER ||
		(nvram_get_int("sw_mode") == SW_MODE_AP &&
			nvram_get_int("wlc_psta") == 0)){
		if(nvram_get_int("wl1_chanspec") == 0){
			dbG("[dfs] start nodfs scanning and selection\n");
			start_nodfs_scan_qtn();
		}
	}
	if(nvram_get_int("sw_mode") == SW_MODE_AP &&
		nvram_get_int("wlc_psta") == 1 &&
		nvram_get_int("wlc_band") == 0){
		ret = qcsapi_wifi_reload_in_mode(WIFINAME, qcsapi_station);
		if (ret < 0)
			dbG("qtn reload_in_mode STA fail\n");
	}*/

	dbG("[dbg] qtn_monitor startup\n");
ERROR:
	remove("/var/run/qtn_monitor.pid");

	return retval;
}
