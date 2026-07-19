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

static int writemon(const char *mon, const char *sensor, int val)
{
	char *path;
	asprintf(&path, "%s/%s", mon, sensor);
	if (path) {
		FILE *fp = fopen(path, "wb");
		if (fp) {
			fprintf(fp, "%d", val);
			fclose(fp);
		}
		free(path);
		return 1;
	}
	return 0;
}
static void setpwm(int mon, int val)
{
	static int avg = 255;
	char *path;
	static int lasttarget = -1;

	avg += (val + 1); // round up
	avg /= 2;
	if (lasttarget == avg)
		return;
	lasttarget = avg;
	asprintf(&path, "/sys/class/hwmon/hwmon%d", mon);
	if (path) {
		writemon(path, "pwm1", avg);
		writemon(path, "pwm1_auto_point1_pwm", avg);
		writemon(path, "pwm1_auto_point2_pwm", avg);
		free(path);
	}
}

static void setfantarget(int mon, int val)
{
	static int avg = 255;
	char *path;
	static int lasttarget = -1;
	avg += val;
	avg /= 2;
	if (lasttarget == avg)
		return;
	lasttarget = avg;
	asprintf(&path, "/sys/class/hwmon/hwmon%d", mon);
	if (path) {
		writemon(path, "fan1_target", avg);
		free(path);
	}
}

static int getsensor(int mon)
{
	char *path;
	int val = 0;
	asprintf(&path, "/sys/class/hwmon/hwmon%d/temp1_input", mon);
	if (path) {
		FILE *fp = fopen(path, "rb");
		if (fp) {
			fscanf(fp, "%d", &val);
			fclose(fp);
		}
		free(path);
	}
	return val;
}

static int calcrpm_max(int psu, int maxtemp)
{
	if (psu > 15000 + (maxtemp * 1000))
		psu = 15000 + (maxtemp * 1000); // clip at 65 celsius
	if (psu > maxtemp * 1000) // min temp to turn fan on
		psu -= maxtemp * 1000;
	else
		psu = 0;
	if (psu > 0 && (psu / 59) < 30)
		psu = 30 * 59;
	return (psu / 59) + 1;
}

static int calcrpm(int psu)
{
	return calcrpm_max(psu, 50);
}

static int getmaxtemp(int psu, int start, int end)
{
	int i;
	for (i = start; i < end + 1; i++) {
		int input = getsensor(i);
		if (input > psu)
			psu = input;
	}

	return psu;
}
static void check_fan(int brand)
{
	int cpu = 0, target = 0;
	int wifi1 = 0, wifi2 = 0, wifi3_mac = 0, wifi3_phy = 0;
	switch (brand) {
	default:
		break;
	#ifdef HAVE_MVEBU
	case ROUTER_WRT_1900AC:
		setpwm(6, calcrpm_max(getsensor(0), nvram_default_geti("hwmon_temp_max", 51)));
		break;
	#endif
	#ifdef HAVE_REALTEK
	case ROUTER_ZYXEL_XGS1250:
		if (nvram_match("DD_BOARD", "Zyxel XGS1250-12 B1")) {
			setpwm(3, calcrpm_max(getmaxtemp(getsensor(4), 0, 2), nvram_default_geti("hwmon_temp_max", 51)));
		} else {
			setpwm(0, calcrpm_max(getsensor(1), nvram_default_geti("hwmon_temp_max", 51)));
		}
		break;
	case ROUTER_DGS_1210:
		if (nvram_match("DD_BOARD", "D-Link DGS-1210-28P F") || nvram_match("DD_BOARD", "D-Link DGS-1210-28MP F")) {
			setpwm(0, calcrpm_max(getsensor(1), nvram_default_geti("hwmon_temp_max", 51)));
		}
		break;
	case ROUTER_EDGECORE_ECS4125:
		setpwm(8,
		       calcrpm_max(getmaxtemp(getmaxtemp(getsensor(9), 10, 11), 0, 7), nvram_default_geti("hwmon_temp_max", 51)));
		break;
	#endif
	#ifdef HAVE_ALPINE
	case ROUTER_NETGEAR_R9000:
		cpu = getsensor(1);
		cpu *= 1000;
		wifi1 = getsensor(2);
		wifi2 = getsensor(3);
		int dummy;
		if (!nvram_match("wlan2_net_mode", "disabled")) {
			FILE *check = fopen("/sys/kernel/debug/ieee80211/phy2/wil6210/temp", "rb");
			if (check) {
				fclose(check);

				FILE *tempfp = popen(
					"cat /sys/kernel/debug/ieee80211/phy2/wil6210/temp | grep \"T_mac\" |cut -d = -f 2", "rb");
				if (tempfp) {
					fscanf(tempfp, "%d.%d", &wifi3_mac, &dummy);
					pclose(tempfp);
					wifi3_mac *= 1000;
				}
				tempfp =
					popen("cat /sys/kernel/debug/ieee80211/phy2/wil6210/temp | grep \"T_radio\" |cut -d = -f 2",
					      "rb");
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
		target *= 4000;
		target /= 10000;
		setfantarget(0, target);
		break;
	#endif
	}
}
#if 0
/* check signal code, its unused now, we keep it if we need it later again */
static unsigned char zerocount[8][17];
static void check_signal(const char *var, int interface, int vap)
{
	struct mac80211_info *mac80211_info;

	mac80211_info = mac80211_assoclist(var);
	if (mac80211_info && mac80211_info->wci) {
		struct wifi_client_info *wc;
		for (wc = mac80211_info->wci; wc; wc = wc->next) {
			if (wc) {
				char mac[32];
				ether_etoa(wc->etheraddr, mac);
				if (!(wc->signal - wc->noise)) {
					zerocount[interface][vap]++;
					if (zerocount[interface][vap] > 20)
						dd_logerror("ath11k_watchdog", "zero signal issue detected on interface %s (%s)\n",
							    wc->ifname, mac);
					if (zerocount[interface][vap] == 100) {
						dd_logerror("ath11k_watchdog", "20 consecutive signal fails detected on %s (%s)\n",
							    wc->ifname, mac);
						sys_reboot();
					}
				} else {
					if (zerocount[interface][vap]) {
						if (zerocount[interface][vap] > 20)
							dd_logerror("ath11k_watchdog",
								    "signal measurement received. reset failcount %s (%s)\n",
								    wc->ifname, mac);
						int i;
						for (i = 0; i < 17; i++)
							zerocount[interface][i] = 0;
					}
				}
			}
		}
		free_wifi_clients(mac80211_info->wci);
	}
	if (mac80211_info)
		free(mac80211_info);
}
static void check_wifi(void)
{
	int ifcount = getdevicecount();
	int c = 0;
	int vap = 0;
	for (c = 0; c < ifcount; c++) {
		char interface[32];
		sprintf(interface, "wlan%d", c);
		if (nvram_nmatch("disabled", "%s_net_mode", interface))
			continue;
		if (nvram_nmatch("disabled", "%s_mode", interface))
			continue;

		if (is_ath11k(interface)) {
			check_signal(interface, c, 0);
			char vifs[32];
			char var[32];
			const char *next;
			sprintf(vifs, "wlan%d_vifs", c);
			char *vaps = nvram_safe_get(vifs);
			int vap = 1;
			foreach(var, vaps, next) {
				if (nvram_nmatch("disabled", "%s_net_mode", var))
					continue;
				if (nvram_nmatch("disabled", "%s_mode", var))
					continue;
				check_signal(var, c, vap++);
			}
		}
	}
}
#endif
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
		check_fan(brand);
		static int blockcounter = 0;
		sleep(5);
		if (!((blockcounter++) % 60)) // check every 5 minutes
			check_blocklist("watchdog", NULL);
	}
}

int main(int argc, char *argv[])
{
	#if 0
	memset(zerocount, 0, sizeof(zerocount));
	#endif
	dd_daemon();
	watchdog();
	return 0;
}
#endif
