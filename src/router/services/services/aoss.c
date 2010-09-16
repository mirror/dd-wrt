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
#ifdef HAVE_AOSS
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>

void stop_aoss(void);

void start_aoss(void)
{
	int ret;

	if (nvram_match("aoss_enable", "0")) {
		stop_aoss();
		return;
	}
	if (pidof("aoss") > 0)
		return;
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
	sysprintf("startservice deconfigurewifi");
	nvram_unset("ath0_vifs");
#ifdef HAVE_WZRHPAG300NH
	nvram_unset("ath1_vifs");
#endif
	sysprintf("startservice configurewifi");
	nvram_set("ath0_vifs", copy);
#ifdef HAVE_WZRHPAG300NH
	nvram_set("ath1_vifs", copy2);
#endif
	nvram_commit();
	int hasaoss = 0;
#ifdef HAVE_WZRHPAG300NH
	if (nvram_match("ath0_mode", "ap") || nvram_match("ath0_mode", "wdsap")) {
		hasaoss = 1;
		sysprintf
		    ("80211n_wlanconfig aossa create wlandev wifi1 wlanmode ap");
		sysprintf("iwconfig aossa essid ESSID-AOSS");
		sysprintf("iwpriv aossa authmode 4");
		sysprintf("iwconfig aossa key [1] 4D454C434F");
		sysprintf("iwconfig aossa key [1]");
		sysprintf("ifconfig aossa 0.0.0.0 up");
	}
	if (nvram_match("ath0_mode", "ap") || nvram_match("ath0_mode", "wdsap")) {
		hasaoss = 1;
		sysprintf
		    ("80211n_wlanconfig aossg create wlandev wifi0 wlanmode ap");
		sysprintf("iwconfig aossg essid ESSID-AOSS");
		sysprintf("iwpriv aossg authmode 4");
		sysprintf("iwconfig aossg key [1] 4D454C434F");
		sysprintf("iwconfig aossg key [1]");
		sysprintf("ifconfig aossg 0.0.0.0 up");
	}
	if (hasaoss) {
		//create aoss bridge
		sysprintf("brctl addbr aoss");
		sysprintf("ifconfig aoss 0.0.0.0 up");
		sysprintf("brctl addif aoss aossa");
		sysprintf("brctl addif aoss aossg");
	}
#else
	if (nvram_match("ath0_mode", "ap") || nvram_match("ath0_mode", "wdsap")) {
		hasaoss = 1;
		sysprintf
		    ("80211n_wlanconfig aoss create wlandev wifi0 wlanmode ap");
		sysprintf("iwconfig aoss essid ESSID-AOSS");
		sysprintf("iwpriv aoss authmode 4");
		sysprintf("iwconfig aoss key [1] 4D454C434F");
		sysprintf("iwconfig aoss key [1]");
		sysprintf("ifconfig aoss 0.0.0.0 up");
	}
#endif
	if (hasaoss) {
		sysprintf("iptables -I OUTPUT -o aoss -j ACCEPT");
		sysprintf("iptables -I INPUT -i aoss -j ACCEPT");
		ret = eval("aoss", "-i", "aoss", "-m", "ap");
		dd_syslog(LOG_INFO,
			  "aoss : aoss daemon successfully started\n");
	} else
		dd_syslog(LOG_INFO,
			  "aoss : aoss daemon not started (operation mode is not AP or WDSAP)\n");

	cprintf("done\n");
	return;
}

void stop_aoss(void)
{
	stop_process("aoss", "buffalo aoss daemon");
	sysprintf("iptables -D OUTPUT -o aoss -j ACCEPT");
	sysprintf("iptables -D INPUT -i aoss -j ACCEPT");
	return;
}

#endif
