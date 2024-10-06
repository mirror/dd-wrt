/*
 * sysinit-ls2.c
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
#include "devices/wireless.c"

extern void vlan_init(int num);

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	struct ifreq ifr;
	int s;
	char macaddr[32];

	mknod("/dev/mmc", S_IFBLK | 0660, makedev(126, 0));
	mknod("/dev/mmc0", S_IFBLK | 0660, makedev(126, 1));
	mknod("/dev/mmc1", S_IFBLK | 0660, makedev(126, 2));
	mknod("/dev/mmc2", S_IFBLK | 0660, makedev(126, 3));
	mknod("/dev/mmc3", S_IFBLK | 0660, makedev(126, 4));

	eval("/bin/tar", "-xzf", "/dev/mtdblock/3", "-C", "/");
	FILE *in = fopen("/tmp/nvram/nvram.db", "rb");

	if (in != NULL) {
		fclose(in);
		eval("/usr/sbin/convertnvram");
		eval("/sbin/mtd", "erase", "nvram");
		nvram_commit();
	}
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	/*
	 * Modules 
	 */
	/*
	 * network drivers 
	 */
#ifdef HAVE_HOTPLUG2
	insmod("ar231x");
#else
	insmod("ar2313");
#endif
	detect_wireless_devices(RADIO_ALL);

#ifdef HAVE_BWRG1000
	eval("ifconfig", "eth0", "up"); // wan
#ifdef HAVE_SWCONFIG
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 5t");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "4 5t");
	eval("swconfig", "dev", "eth0", "set", "apply");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");
	nvram_seti("sw_cpuport", 5);
	nvram_seti("sw_wan", 4);
	nvram_seti("sw_lan1", 0);
	nvram_seti("sw_lan2", 1);
	nvram_seti("sw_lan3", 2);
	nvram_seti("sw_lan4", 3);
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "1 2 16000");
#else
	vlan_init(0xff); // 4 lan + 1 wan
#endif
#else
#ifdef HAVE_LS2
#if !defined(HAVE_NS2) && !defined(HAVE_BS2) && !defined(HAVE_LC2) && !defined(HAVE_BS2HP) && !defined(HAVE_MS2) && \
	!defined(HAVE_PICO2) && !defined(HAVE_PICO2HP)
	eval("ifconfig", "eth0", "up"); // wan
#ifdef HAVE_SWCONFIG
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 5t");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "4 5t");
	eval("swconfig", "dev", "eth0", "set", "apply");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");
	nvram_seti("sw_cpuport", 5);
	nvram_seti("sw_wan", 4);
	nvram_seti("sw_lan1", 0);
	nvram_seti("sw_lan2", 1);
	nvram_seti("sw_lan3", 2);
	nvram_seti("sw_lan4", 3);
#else
	vlan_init(0xff); // 4 lan + 1 wan
	writeprocsys("dev/wifi0/ledpin", "7");
	writeprocsys("dev/wifi0/softled", "1");
#endif
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
		set_hwaddr("vlan2", macaddr);
	}
#endif
#endif
#endif
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
		nvram_set("lan_hwaddr", macaddr);
	}
	close(s);
#if defined(HAVE_MS2)
	writeprocsys("dev/wifi0/ledpin", "2");
	writeprocsys("dev/wifi0/softled", "1");
#endif
#if defined(HAVE_BWRG1000)
	writeprocsys("dev/wifi0/ledpin", "2");
	writeprocsys("dev/wifi0/softled", "1");
#endif

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

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
void start_wifi_drivers(void)
{
}