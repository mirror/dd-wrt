/*
 * aoss.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#if defined(HAVE_AOSS) || defined(HAVE_WPS)
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void setupHostAP_ath9k(char *maininterface, int isfirst, int vapid, int aoss);

void stop_aoss(void);

void start_aoss(void)
{
	int ret;
#ifdef HAVE_WZRHPAG300NH
	if (nvram_match("ath0_net_mode", "disabled")
	    && nvram_match("ath1_net_mode", "disabled")) {
		led_control(LED_SES, LED_OFF);
		stop_aoss();
		return;
	}
#else
	if (nvram_match("ath0_net_mode", "disabled")) {
		led_control(LED_SES, LED_OFF);
		stop_aoss();
		return;
	}
#endif
	if (nvram_match("aoss_enable", "0")) {
		stop_aoss();
#ifdef HAVE_WPS
		unlink("/tmp/.wpsdone");
		if (nvram_match("wps_enabled", "1")) {
			if (!nvram_match("ath0_net_mode", "disabled")) {
				eval("hostapd_cli", "-i", "ath0", "wps_pbc");
#ifdef HAVE_IDEXX
				eval("/usr/bin/check_wps");
#endif
			}
#if defined(HAVE_WZRHPAG300NH)
			if (!nvram_match("ath1_net_mode", "disabled")) {
				led_control(LED_SES, LED_FLASH);
				eval("hostapd_cli", "-i", "ath1", "wps_pbc");
#ifdef HAVE_IDEXX
				eval("/usr/bin/check_wps");
#endif
			}
#endif
		}
#endif

		return;
	}
#ifdef HAVE_AOSS
	if (pidof("aoss") > 0)
		return;
	led_control(LED_SES, LED_FLASH);	// when pressed, blink white
	system("killall ledtool");
	nvram_set("aoss_success", "0");
	led_control(LED_SES, LED_OFF);
	system("ledtool 180 2");
	char *vifbak = nvram_safe_get("ath0_vifs");
	char copy[256];
	strcpy(copy, vifbak);

#ifdef HAVE_WZRHPAG300NH
	char *vifbak2 = nvram_safe_get("ath1_vifs");
	char copy2[256];
	strcpy(copy2, vifbak2);
#endif
#ifdef HAVE_ATH9K
	if (!is_ath9k("ath0"))
#endif
	{
		eval("startservice", "deconfigurewifi", "-f");
	}
	nvram_unset("ath0_vifs");
#ifdef HAVE_WZRHPAG300NH
	nvram_unset("ath1_vifs");
#endif
#ifdef HAVE_ATH9K
	if (!is_ath9k("ath0"))
#endif
	{
		eval("startservice", "configurewifi", "-f");
	}
	nvram_set("ath0_vifs", copy);
#ifdef HAVE_WZRHPAG300NH
	nvram_set("ath1_vifs", copy2);
#endif
	nvram_commit();
	int hasaoss = 0;
#ifdef HAVE_WZRHPAG300NH
#ifdef HAVE_ATH9K
	if ((nvram_match("ath0_mode", "ap")
	     || nvram_match("ath0_mode", "wdsap"))
	    && !nvram_match("ath0_net_mode", "disabled")) {
		hasaoss = 1;
		deconfigure_single_ath9k(0);
		configure_single_ath9k(0);
		hasaoss = 1;
		char *next;
		char var[80];
		char *vifs = nvram_safe_get("ath0_vifs");
		int counter = 1;
		foreach(var, vifs, next) {
			counter++;
		}
		setupHostAP_ath9k("ath0", 0, counter, 1);
		FILE *fp = fopen("/var/run/ath0_hostapd.pid", "rb");
		if (fp)		// file not found means that hostapd usually doesnt run
		{
			int pid;
			fscanf(fp, "%d", &pid);
			fclose(fp);
			kill(pid, SIGTERM);
			sleep(2);
		}
		eval("hostapd", "-B", "-P", "/var/run/ath0_hostapd.pid", "/tmp/ath0_hostap.conf");
	}
	if ((nvram_match("ath1_mode", "ap")
	     || nvram_match("ath1_mode", "wdsap"))
	    && !nvram_match("ath1_net_mode", "disabled")) {
		hasaoss = 1;
		deconfigure_single_ath9k(1);
		configure_single_ath9k(1);
		hasaoss = 1;
		char *next;
		char var[80];
		char *vifs = nvram_safe_get("ath1_vifs");
		int counter = 1;
		foreach(var, vifs, next) {
			counter++;
		}
		setupHostAP_ath9k("ath1", 0, counter, 1);
		FILE *fp = fopen("/var/run/ath1_hostapd.pid", "rb");
		if (fp)		// file not found means that hostapd usually doesnt run
		{
			int pid;
			fscanf(fp, "%d", &pid);
			fclose(fp);
			kill(pid, SIGTERM);
			sleep(2);
		}
		eval("hostapd", "-B", "-P", "/var/run/ath1_hostapd.pid", "/tmp/ath1_hostap.conf");

	}
#else

	if ((nvram_match("ath1_mode", "ap")
	     || nvram_match("ath1_mode", "wdsap"))
	    && !nvram_match("ath1_net_mode", "disabled")) {
		hasaoss = 1;
		eval("80211n_wlanconfig", "aossa", "create", "wlandev", "wifi1", "wlanmode", "ap");
		eval("iwconfig", "aossa", "essid", "ESSID-AOSS-1");
		eval("iwpriv", "aossa", "authmode", "4");
		eval("iwconfig", "aossa", "key", "[1]", "4D454C434F");
		eval("iwconfig", "aossa", "key", "[1]");
		eval("ifconfig", "aossa", "0.0.0.0", "up");
	}
	if ((nvram_match("ath0_mode", "ap")
	     || nvram_match("ath0_mode", "wdsap"))
	    && !nvram_match("ath0_net_mode", "disabled")) {
		hasaoss = 1;
		eval("80211n_wlanconfig", "aossg create", "wlandev", "wifi0", "wlanmode", "ap");
		eval("iwconfig", "aossg", "essid", "ESSID-AOSS");
		eval("iwpriv", "aossg", "authmode", "4");
		eval("iwconfig", "aossg", "key", "[1]", "4D454C434F");
		eval("iwconfig", "aossg", "key", "[1]");
		eval("ifconfig", "aossg", "0.0.0.0", "up");
	}
#endif
	if (hasaoss) {
		//create aoss bridge
		eval("brctl", "addbr", "aoss");
		eval("ifconfig", "aoss", "0.0.0.0", "up");
		if (!nvram_match("ath1_net_mode", "disabled")) {
			eval("brctl", "addif", "aoss", "aossa");
		}
		if (!nvram_match("ath0_net_mode", "disabled")) {
			eval("brctl", "addif", "aoss", "aossg");
		}
	}
#else
	if (nvram_match("ath0_mode", "ap") || nvram_match("ath0_mode", "wdsap")) {
#ifdef HAVE_ATH9K
		if (is_ath9k("ath0")) {
			deconfigure_single_ath9k(0);
			configure_single_ath9k(0);
			hasaoss = 1;
			char *next;
			char var[80];
			char *vifs = nvram_safe_get("ath0_vifs");
			int counter = 1;
			foreach(var, vifs, next) {
				counter++;
			}
			setupHostAP_ath9k("ath0", 0, counter, 1);
			FILE *fp = fopen("/var/run/ath0_hostapd.pid", "rb");
			if (fp)	// file not found means that hostapd usually doesnt run
			{
				int pid;
				fscanf(fp, "%d", &pid);
				fclose(fp);
				kill(pid, SIGTERM);
				sleep(2);
			}
			eval("hostapd", "-B", "-P", "/var/run/ath0_hostapd.pid", "/tmp/ath0_hostap.conf");
		} else
#endif

		{
			hasaoss = 1;
			eval("80211n_wlanconfig", "aoss", "create", "wlandev", "wifi0", "wlanmode", "ap");
			eval("iwconfig", "aoss", "essid", "ESSID-AOSS");
			eval("iwpriv", "aoss", "authmode", "4");
			eval("iwconfig", "aoss", "key", "[1]", "4D454C434F");
			eval("iwconfig", "aoss", "key", "[1]");
			eval("ifconfig", "aoss", "0.0.0.0", "up");
		}
	}
#endif
	if (hasaoss) {
		eval("iptables", "-I", "OUTPUT", "-o", "aoss", "-j", "ACCEPT");
		eval("iptables", "-I", "INPUT", "-i", "aoss", "-j", "ACCEPT");
		ret = eval("aoss", "-i", "aoss", "-m", "ap");
		dd_syslog(LOG_INFO, "aoss : aoss daemon successfully started\n");
	} else
		dd_syslog(LOG_INFO, "aoss : aoss daemon not started (operation mode is not AP or WDSAP)\n");

#endif
	cprintf("done\n");
	return;
}

void stop_aoss(void)
{
#ifdef HAVE_AOSS
	stop_process("aoss", "buffalo aoss daemon");
	eval("iptables", "-D", "OUTPUT", "-o", "aoss", "-j", "ACCEPT");
	eval("iptables", "-D", "INPUT", "-i", "aoss", "-j", "ACCEPT");
#endif
	return;
}

#endif
