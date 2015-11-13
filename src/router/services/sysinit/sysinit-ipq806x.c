/*
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
	insmod("usb-common");
	insmod("dwc3");
	insmod("dwc3-qcom");
	insmod("gsp");
	insmod("slhc");
	
	insmod("regmap-core");
	insmod("regmap-i2c");
	insmod("regmap-spi");
	insmod("leds-tlc59116");
	insmod("leds-gpio");


	insmod("tmp421");
	insmod("mii");
	insmod("/lib/modules/3.18.23/stmmac.ko"); //for debugging purposes compiled as module
	/*
	 * network drivers 
	 */
	insmod("/lib/ath9k/compat.ko");
	insmod("/lib/ath9k/cfg80211.ko");
	insmod("/lib/ath9k/mac80211.ko");
	insmod("/lib/ath9k/ath.ko");
	insmod("/lib/ath9k/ath9k_hw.ko");
	insmod("/lib/ath9k/ath9k_common.ko");
	insmod("/lib/ath9k/ath9k.ko");
	insmod("/lib/ath9k/ath10k_core.ko");
	insmod("/lib/ath9k/ath10k_pci.ko");
	//insmod("qdpc-host.ko");
	
	system("swconfig dev switch0 set reset 1");
	system("swconfig dev switch0 set enable_vlan 1");
	if (nvram_match("wan_proto", "disabled") && nvram_match("fullswitch", "1")) {
		system("swconfig dev switch0 vlan 1 set ports \"6 1 2 3 4\"");
	} else {
		system("swconfig dev switch0 vlan 1 set ports \"6t 1 2 3 4\"");
		system("swconfig dev switch0 vlan 2 set ports \"5t 0\"");
	}
	system("swconfig dev switch0 set apply");
	eval("ifconfig", "eth1", "up");
	eval("ifconfig", "eth0", "up");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

	int s;
	struct ifreq ifr;
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		char macaddr[32];

		strcpy(macaddr, ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
		close(s);
	}
	
	set_gpio(9, 1);	//wps
	
	

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");
	//nvram_set("wl1_ifname", "ath1");

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
