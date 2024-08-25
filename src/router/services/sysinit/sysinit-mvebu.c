/*
 * sysinit-pb42.c
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

#define SIOCGMIIREG 0x8948 /* Read MII PHY register.  */
#define SIOCSMIIREG 0x8949 /* Write MII PHY register.  */
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

	if (!nvram_matchi("disable_watchdog", 1)) {
		insmod("orion_wdt");
	}
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
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
	insmod("leds-tlc591xx");
	insmod("leds-pca963x");

	insmod("hwmon");
	insmod("thermal_sys");
	insmod("gpio-fan");
	insmod("pwm-fan");
	insmod("armada_thermal");
	insmod("tmp421");

	insmod("rtc-armada38x"); // for WRT1200AC / WRT1900ACv2 only
	insmod("mii");
	// crypto drivers
	insmod("libdes");
	insmod("des_generic");
	insmod("marvell-cesa"); // tested on WRT1900AC v1 so far
	insmod("cryptodev");

	/*
	 * network drivers 
	 */
	insmod("mmc_core");
	insmod("mmc_block");
	insmod("sdhci");
	insmod("sdhci-pltfm");
	insmod("sdhci-pxav3");
	insmod("mvsdio");
	insmod("mwifiex_sdio.ko");
	insmod("bluetooth");
	insmod("btmrvl");
	insmod("btmrvl_sdio");
	int s;
	struct ifreq ifr;
	char *recovery = getUEnv("auto_recovery");
	if (recovery && strcmp(recovery, "yes"))
		eval("ubootenv", "set", "auto_recovery", "yes");

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");
	nvram_set("wl1_ifname", "wlan1");

	writeint("/sys/class/hwmon/hwmon0/pwm1", 0);
	char line[256];
	char *mac;
	if ((fp = fopen("/dev/mtdblock3", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, "hw_mac_addr")) {
				strtok_r(line, "=", &mac);
				//fprintf(stderr, "Found mac: %s\n", mac);
				nvram_set("et0macaddr", mac);
				nvram_set("et0macaddr_safe", mac);
				set_hwaddr("eth0", mac);
				MAC_ADD(mac);
				set_hwaddr("eth1", mac);
			}
		}
		fclose(fp);
	}

	/*     if (brand == ROUTER_WRT_1900AC) {
	   set_smp_affinity(27, 2);
	   set_smp_affinity(42, 2);
	   } else {
	   set_smp_affinity(36, 2);
	   set_smp_affinity(46, 2);
	   } */
	set_gpio(3, 0); //disable sata led as initial value
	set_gpio(4, 0); //disable usb 1 led as initial value
	set_gpio(5, 0); //disable usb 2 led as initial value
	set_gpio(8, 0); //disable usb ss led as initial value

	if (brand == ROUTER_WRT_1900AC) {
		writestr("/sys/class/leds/mamba\\:white\\:esata/trigger", "disk-activity");
	}
	if (brand == ROUTER_WRT_1200AC) {
		writestr("/sys/class/leds/caiman\\:white\\:sata/trigger", "disk-activity");
	}
	if (brand == ROUTER_WRT_1900ACV2) {
		writestr("/sys/class/leds/cobra\\:white\\:sata/trigger", "disk-activity");
	}
	if (brand == ROUTER_WRT_1900ACS) {
		writestr("/sys/class/leds/shelby\\:white\\:sata/trigger", "disk-activity");
	}
	if (brand == ROUTER_WRT_3200ACM) {
		writestr("/sys/class/leds/rango\\:white\\:sata/trigger", "disk-activity");
	}
	if (brand == ROUTER_WRT_32X) {
		writestr("/sys/class/leds/venom\\:blue\\:sata/trigger", "disk-activity");
	}

	nvram_seti("sw_wancpuport", 5);
	nvram_seti("sw_lancpuport", 6);
	nvram_seti("sw_wan", 4);
	nvram_seti("sw_lan1", 3);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 1);
	nvram_seti("sw_lan4", 0);
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);

	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");

	return;
}

int check_cfe_nv(void)
{
	nvram_seti("portprio_support", 0);
	return 0;
}

int check_pmon_nv(void)
{
	return 0;
}

void start_overclocking(void)
{
}

char *enable_dtag_vlan(int enable)
{
	return "eth0";
}

char *set_wan_state(int state)
{
	return NULL;
}

void start_devinit_arch(void)
{
}

void start_resetbc(void)
{
	if (!nvram_match("nobcreset", "1"))
		eval("mtd", "resetbc", "s_env");
}
void load_wifi_drivers(void)
{
	if (!load_mac80211()) {
		insmod("mwlwifi");
		insmod("mwifiex");
		wait_for_wifi();
	}
}