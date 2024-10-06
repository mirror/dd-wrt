/*
 * sysinit-thunderx.c
 *
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <linux/if_ether.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

#include "devices/ethernet.c"
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	char dev[64];

	if (!nvram_matchi("disable_watchdog", 1)) {
		insmod("imx2_wdt");
	}

	/*
	 * Setup console 
	 */
	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	//for extension board
	struct ifreq ifr;
	int s;

	//      fprintf(stderr, "try modules for ethernet adapters\n");
	//      nvram_seti("intel_eth", 0);
	//      insmod("sky2");
	//      if (detect_ethernet_devices())
	//              nvram_seti("intel_eth", 1);
	/*
	 * network drivers 
	 */
	insmod("cptpf"); // crypto driver
	insmod("cptvf"); // crypto driver

	nvram_default_get("use_ath5k", "1");
	detect_wireless_devices(RADIO_ALL);

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
	eval("ifconfig", "eth0", "promisc");
	eval("ifconfig", "eth1", "promisc");
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");
	eval("hwclock", "-s", "-u");
	eval("i2cset", "-f", "-y", "0", "0x20", "0", "0x0");
	eval("i2cset", "-f", "-y", "0", "0x20", "11", "0x10");

	char *board = nvram_safe_get("DD_BOARD");
	if (!strncmp(board, "Gateworks Newport GW64", 22))
		eval("gsp_updater", "-f", "/etc/gsc_640x_v61.txt", "-r", "61");
	if (!strncmp(board, "Gateworks Newport GW63", 22))
		eval("gsp_updater", "-f", "/etc/gsc_630x_v61.txt", "-r", "61");
	if (!strncmp(board, "Gateworks Newport GW62", 22))
		eval("gsp_updater", "-f", "/etc/gsc_620x_v61.txt", "-r", "61");
	if (!strncmp(board, "Gateworks Newport GW61", 22))
		eval("gsp_updater", "-f", "/etc/gsc_610x_v61.txt", "-r", "61");

	if (!strncmp(board, "Gateworks Newport GW6903", 24))
		eval("gsp_updater", "-f", "/etc/gsc_6903_v61.txt", "-r", "61");
	if (!strncmp(board, "Gateworks Newport GW6904", 24))
		eval("gsp_updater", "-f", "/etc/gsc_6904_v61.txt", "-r", "61");
	if (!strncmp(board, "Gateworks Newport GW6905", 24))
		eval("gsp_updater", "-f", "/etc/gsc_6905_v61.txt", "-r", "61");

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

#include "tools/recover.c"
char *set_wan_state(int state)
{
	return NULL;
}

void start_devinit_arch(void)
{
}
void start_wifi_drivers(void)
{
}