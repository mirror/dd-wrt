/*
 * sysinit-pb42.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <string.h>
#include <termios.h>
#include <sys/klog.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/ioctl.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <cymac.h>

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	FILE *fp;

	if (!nvram_match("disable_watchdog", "1")) {
		insmod("orion_wdt");
		eval("watchdog");
	}
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");
	int brand = getRouterBrand();

	/* 
	 * 
	 */

	insmod("i2c-core");
	insmod("i2c-dev");
	insmod("i2c-mv64xxx");
	insmod("regmap-core");
	insmod("regmap-i2c");
	insmod("regmap-spi");
	//insmod("input-core");
	//insmod("button-hotplug");
	//insmod("gpio-button-hotplug");
	insmod("leds-tlc59116");
	insmod("leds-pca963x");

	insmod("hwmon");
	insmod("thermal_sys");
	insmod("gpio-fan");
	insmod("pwm-fan");
	insmod("armada_thermal");
	insmod("tmp421");
	insmod("rtc-armada38x");	// for WRT1200AC / WRT1900ACv2 only
	insmod("mii");
	// crypto drivers
	insmod("des_generic");
	insmod("marvell-cesa");	// tested on WRT1900AC v1 so far
	/*
	 * network drivers 
	 */
	insmod("/lib/ath9k/compat.ko");
	insmod("/lib/ath9k/cfg80211.ko");
	insmod("/lib/ath9k/mac80211.ko");
	insmod("/lib/ath9k/mwlwifi.ko");
	int s;
	struct ifreq ifr;
	char *recovery = getUEnv("auto_recovery");
	if (recovery && strcmp(recovery, "yes"))
		eval("ubootenv", "set", "auto_recovery", "yes");
	eval("mtd", "resetbc", "s_env");	// reset boot counter

/*	
	system("swconfig dev switch0 set reset 1");
	system("swconfig dev switch0 set enable_vlan 1");
	if (nvram_match("wan_proto", "disabled") && nvram_match("fullswitch", "1")) {
		system("swconfig dev switch0 vlan 1 set ports \"0 1 2 3 5\"");
	} else {
		system("swconfig dev switch0 vlan 1 set ports \"5t 0 1 2 3\"");
		system("swconfig dev switch0 vlan 2 set ports \"5t 4\"");
	}
	system("swconfig dev switch0 set apply");

	eval("ifconfig", "eth0", "up");
	//eval("ifconfig", "eth1", "up");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

*/
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");
	nvram_set("wl1_ifname", "ath1");

	sysprintf("echo 0 > /sys/class/hwmon/hwmon0/pwm1");
	char line[256];
	char *mac;
	if ((fp = fopen("/dev/mtdblock3", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, "hw_mac_addr")) {
				strtok_r(line, "=", &mac);
				//fprintf(stderr, "Found mac: %s\n", mac);
				nvram_set("et0macaddr", mac);
				nvram_set("et0macaddr_safe", mac);
				if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
					//fprintf(stderr, "Assign mac: %s to eth0\n", mac);
					strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
					ioctl(s, SIOCGIFHWADDR, &ifr);
					ether_atoe(mac, (unsigned char *)ifr.ifr_hwaddr.sa_data);
					ioctl(s, SIOCSIFHWADDR, &ifr);
					close(s);
				}
			}
		}
		fclose(fp);
	}

	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		set_smp_affinity(90, 2);
		set_smp_affinity(27, 2);
	} else {
		set_smp_affinity(65, 2);
		set_smp_affinity(195, 2);
	}
	return;
	cprintf("done\n");
}

int check_cfe_nv(void)
{
	return 0;
}

int check_pmon_nv(void)
{
	return 0;
}

void start_overclocking(void)
{
}

void enable_dtag_vlan(int enable)
{

}
