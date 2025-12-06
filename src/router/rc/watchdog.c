/*
 * watchdog.c
 *
 * Copyright (C) 2006 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <utils.h>
#include <wlutils.h>
#include <errno.h>
#include <ddnvram.h>
#include <shutils.h>

int isregistered_real(void);
int isregistered(void);
#if !defined(HAVE_MICRO) || defined(HAVE_ADM5120) || defined(HAVE_WRK54G)

static void check_fan(int brand)
{
#ifdef HAVE_MVEBU
	if (brand == ROUTER_WRT_1900AC) {
		int cpu;
		FILE *tempfp;
		tempfp = fopen("/sys/class/hwmon/hwmon0/temp1_input", "rb");
		if (tempfp) {
			fscanf(tempfp, "%d", &cpu);
			fclose(tempfp);
			int target = cpu - (nvram_geti("hwmon_temp_max") * 1000);
			if (target < 0)
				target = 0;
			if (target > 10000)
				target = 10000;
			target *= 255;
			target /= 10000;
			sysprintf("/bin/echo %d > /sys/class/hwmon/hwmon6/pwm1", target);
		}
	}
#endif
#ifdef HAVE_REALTEK
	if (nvram_match("DD_BOARD", "Zyxel XGS1250-12") || nvram_match("DD_BOARD", "Zyxel XGS1250-12 B1")) {
		int psu = 0;
		FILE *tempfp;
		tempfp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "rb");
		if (tempfp) {
			fscanf(tempfp, "%d", &psu);
			fclose(tempfp);
			if (psu >= 51000) {
				sysprintf("/bin/echo 250 > /sys/class/hwmon/hwmon0/pwm1");
			} else {
				sysprintf("/bin/echo 0 > /sys/class/hwmon/hwmon0/pwm1");
			}
		}
	}
	if (nvram_match("DD_BOARD", "D-Link DGS-1210-28P F") || nvram_match("DD_BOARD", "D-Link DGS-1210-28MP F")) {
		int psu = 0;
		FILE *tempfp;
		tempfp = fopen("/sys/class/hwmon/hwmon1/temp0_input", "rb");
		if (tempfp) {
			fscanf(tempfp, "%d", &psu);
			fclose(tempfp);
			if (psu >= 51000) {
				sysprintf("/bin/echo 250 > /sys/class/hwmon/hwmon0/pwm1");
			} else {
				sysprintf("/bin/echo 156 > /sys/class/hwmon/hwmon0/pwm1");
			}
		}
	}
#endif
#ifdef HAVE_R9000
	static int lasttarget = 0;
	int cpu = 0, wifi1 = 0, wifi2 = 0, wifi3_mac = 0, wifi3_phy = 0;
	FILE *tempfp;
	tempfp = fopen("/sys/class/hwmon/hwmon1/temp1_input", "rb");
	if (tempfp) {
		fscanf(tempfp, "%d", &cpu);
		fclose(tempfp);
	}
	cpu *= 1000;
	tempfp = fopen("/sys/class/hwmon/hwmon2/temp1_input", "rb");
	if (tempfp) {
		fscanf(tempfp, "%d", &wifi1);
		fclose(tempfp);
	}
	tempfp = fopen("/sys/class/hwmon/hwmon3/temp1_input", "rb");
	if (tempfp) {
		fscanf(tempfp, "%d", &wifi2);
		fclose(tempfp);
	}
	int dummy;
	if (!nvram_match("wlan2_net_mode", "disabled")) {
		FILE *check = fopen("/sys/kernel/debug/ieee80211/phy2/wil6210/temp", "rb");
		if (check) {
			fclose(check);

			tempfp = popen("cat /sys/kernel/debug/ieee80211/phy2/wil6210/temp | grep \"T_mac\" |cut -d = -f 2", "rb");
			if (tempfp) {
				fscanf(tempfp, "%d.%d", &wifi3_mac, &dummy);
				pclose(tempfp);
				wifi3_mac *= 1000;
			}
			tempfp = popen("cat /sys/kernel/debug/ieee80211/phy2/wil6210/temp | grep \"T_radio\" |cut -d = -f 2", "rb");
			if (tempfp) {
				fscanf(tempfp, "%d.%d", &wifi3_phy, &dummy);
				pclose(tempfp);
				wifi3_phy *= 1000;
			}
		}
	}
	if (wifi1 > cpu)
		cpu = wifi1;
	if (wifi2 > cpu)
		cpu = wifi2;
	if (wifi3_mac > cpu)
		cpu = wifi3_mac;
	if (wifi3_phy > cpu)
		cpu = wifi3_phy;

	int target = cpu - (nvram_geti("hwmon_temp_max") * 1000);
	if (target < 0)
		target = 0;
	if (target > 10000)
		target = 10000;
	//	fprintf(stderr, "%d %d %d %d %d target=%d lasttarget %d\n", cpu, wifi1, wifi2, wifi3_mac, wifi3_phy, target, lasttarget);
	target *= 4000;
	target /= 10000;
	if (target != lasttarget) {
		//		fprintf(stderr, "set fan to %d\n", target);
		sysprintf("/bin/echo %d > /sys/class/hwmon/hwmon0/fan1_target", target);
		lasttarget = target;
	}
#endif
}

static void check_wifi(void)
{
#ifdef HAVE_ATH11K
	int ifcount = getdevicecount();
	int c = 0;
	for (c = 0; c < ifcount; c++) {
		char interface[32];
		sprintf(interface, "wlan%d", c);
		if (is_mac80211(interface)) {
			struct mac80211_info *mac80211_info;
			struct wifi_client_info *wc;
			mac80211_info = mac80211_assoclist(interface);
			if (mac80211_info && mac80211_info->wci) {
				for (wc = mac80211_info->wci; wc; wc = wc->next) {
					if (wc) {
						if (is_ath11k(wc->ifname)) {
							if (!(wc->signal - wc->noise)) {
								dd_logerror("ath11k_watchdog",
									    "zero signal issue detected on interface %s\n",
									    wc->ifname);
							}
						}
					}
				}
				free_wifi_clients(mac80211_info->wci);
			}
			if (mac80211_info)
				free(mac80211_info);
		}
	}
#endif
}

static void watchdog(void)
{
	int brand = getRouterBrand();
	int registered = -1;
	int i;
	int radiostate[16];
	int oldstate[16];
	int dropcounter = 0;
	int radioledinitcount = 0;
	memset(radiostate, -1, sizeof(radiostate));
	memset(oldstate, -1, sizeof(oldstate));
#ifndef HAVE_WZRG300NH
	int fd;
	if (!nvram_matchi("disable_watchdog", 1)) {
		fd = open("/dev/misc/watchdog", O_WRONLY);
		if (fd == -1)
			fd = open("/dev/watchdog", O_WRONLY);
	}
#endif

	int cnt = getdevicecount();

	while (1) {
#ifndef HAVE_WZRG300NH
		if (!nvram_matchi("disable_watchdog", 1)) {
			if (fd != -1) {
				write(fd, "\0", 1);
				fsync(fd);
			}
		}
#endif
		if (!nvram_matchi("flash_active", 1)) {
#ifndef HAVE_RT2880
#ifdef HAVE_REGISTER
			if (registered == -1)
				registered = isregistered_real();
			if (!registered)
				isregistered(); //to poll
#endif
			/* 
			 * software wlan led control 
			 */
			if (radioledinitcount < 5) {
				radioledinitcount++;
				memset(oldstate, -1, sizeof(oldstate));
			}
#ifdef HAVE_MADWIFI
			for (i = 0; i < cnt; i++) {
				char st[32];
				sprintf(st, "wlan%d", i);
				radiostate[i] = get_radiostate(st);
			}
#else
#ifndef HAVE_QTN
			for (i = 0; i < cnt; i++) {
#else
			for (i = 0; i < 1; i++) {
#endif

				wl_ioctl(get_wl_instance_name(i), WLC_GET_RADIO, &radiostate[i], sizeof(int));
			}
#endif

			for (i = 0; i < cnt; i++) {
				if (radiostate[i] != oldstate[i]) {
#ifdef HAVE_MADWIFI
					if (radiostate[i] == 1)
#else
					if ((radiostate[i] & WL_RADIO_SW_DISABLE) == 0)
#endif
						led_control(LED_WLAN0 + i, LED_ON);
					else {
						led_control(LED_WLAN0 + i, LED_OFF);
#ifndef HAVE_MADWIFI
						if (!i) {
							/* 
					 * Disable wireless will cause diag led blink, so we want to
					 * stop it. 
					 */
							if (brand == ROUTER_WRT54G)
								diag_led(DIAG, STOP_LED);
							/* 
					 * Disable wireless will cause power led off, so we want to
					 * turn it on. 
					 */
							if (brand == ROUTER_WRT54G_V8)
								led_control(LED_POWER, LED_ON);
						}
#endif
					}

					oldstate[i] = radiostate[i];
				}
			}

#endif
		}
		//#ifdef HAVE_USB
		//#ifndef HAVE_3G_ONLY
		//      if ((dropcounter++) % 4 == 0)
		//              writeprocsys("vm/drop_caches", "3");    // flush fs cache
		//#endif
		//#endif
		check_fan(brand);
		check_wifi();
		static blockcounter=0;
		sleep(5);
		if (!((blockcounter++) % 60)) // check every 5 minutes
			check_blocklist("watchdog", NULL);
	}
}

int main(int argc, char *argv[])
{
	dd_daemon();
	watchdog();
	return 0;
}
#endif
