/*
 * sysinit-wrt54g2v11.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
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
#include <cymac.h>
#include "devices/wireless.c"

extern void vlan_init(int num);

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

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
	if (!nvram_match("disable_watchdog", "1"))
		eval("watchdog");
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");
#ifdef HAVE_RTG32
	insmod("slhc");
	insmod("ppp_generic");
	insmod("ppp_async");
	insmod("ppp_synctty");
	insmod("ppp_mppe_mppc ");
	insmod("pppox");
	insmod("pppoe");
#endif
	/*
	 * network drivers 
	 */
#ifdef HAVE_HOTPLUG2
	insmod("ar231x");
#else
	insmod("ar2313");
#endif
	detect_wireless_devices();

	eval("ifconfig", "eth0", "up");	// wan
	system("swconfig dev eth0 set reset 1");
	system("swconfig dev eth0 set enable_vlan 1");
#ifdef HAVE_RTG32
	system("swconfig dev eth0 vlan 1 set ports \"0t 1 2 3 4\"");
	system("swconfig dev eth0 vlan 2 set ports \"0t 5\"");
#else
	system("swconfig dev eth0 vlan 1 set ports \"0t 2 3 4 5\"");
	system("swconfig dev eth0 vlan 2 set ports \"0t 1\"");
#endif
	system("swconfig dev eth0 set apply");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		char macaddr[32];

		strcpy(macaddr, ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr", macaddr);
		MAC_ADD(macaddr);
		ether_atoe(macaddr, (unsigned char *)ifr.ifr_hwaddr.sa_data);
		strncpy(ifr.ifr_name, "vlan2", IFNAMSIZ);
		ioctl(s, SIOCSIFHWADDR, &ifr);
		close(s);
	}
	set_gpio(1,1);
#ifdef HAVE_RTG32
	writeproc("/proc/sys/dev/wifi0/ledpin", "7");
	writeproc("/proc/sys/dev/wifi0/softled", "1");
#else
	writeproc("/proc/sys/dev/wifi0/ledpin", "0");
	writeproc("/proc/sys/dev/wifi0/softled", "1");
#endif
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");

	return;
}

int check_cfe_nv(void)
{
	nvram_set("portprio_support", "0");
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
	if (enable) {
		system("swconfig dev eth0 set reset 1");
		system("swconfig dev eth0 set vlan 1");
#ifdef HAVE_RTG32
		system("swconfig dev eth0 vlan 1 set ports \"0t 1 2 3 4\"");
		system("swconfig dev eth0 vlan 7 set ports \"0t 5t\"");
		system("swconfig dev eth0 vlan 8 set ports \"0t 5t\"");
#else
		system("swconfig dev eth0 vlan 1 set ports \"0t 2 3 4 5\"");
		system("swconfig dev eth0 vlan 7 set ports \"0t 1t\"");
		system("swconfig dev eth0 vlan 8 set ports \"0t 1t\"");

#endif
		system("swconfig dev eth0 set apply");
	} else {
		system("swconfig dev eth0 set reset 1");
		system("swconfig dev eth0 set vlan 1");
#ifdef HAVE_RTG32
		system("swconfig dev eth0 vlan 1 set ports \"0t 1 2 3 4\"");
		system("swconfig dev eth0 vlan 2 set ports \"0t 5\"");
#else
		system("swconfig dev eth0 vlan 1 set ports \"0t 2 3 4 5\"");
		system("swconfig dev eth0 vlan 2 set ports \"0t 1\"");
#endif
		system("swconfig dev eth0 set apply");

	}
	return "eth0";
}
