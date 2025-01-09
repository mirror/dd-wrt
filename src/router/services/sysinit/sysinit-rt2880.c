/*
 * sysinit-rt2880.c
 *
 * Copyright (C) 2008 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#define sys_reboot()                     \
	eval("sync");                    \
	eval("/bin/umount", "-a", "-r"); \
	eval("event", "3", "1", "15")

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
	mknod("/dev/gpio", S_IFCHR | 0644, makedev(252, 0));

	/*
	 * Setup console 
	 */

	klogctl(8, NULL, nvram_geti("console_loglevel"));

	/*
	 * load some netfilter stuff 
	 */

	/*
	 * Set a sane date 
	 */
	if (!nvram_matchi("disable_watchdog", 1)) {
		insmod("mt7621_wdt");
	}

	stime(&tm);
	nvram_set("wl0_ifname", "ra0");
	//      insmod("rt2860v2_ap");
	//      insmod("RTPCI_ap");
	//      insmod("rlt_wifi");
	insmod("raeth");
#ifdef HAVE_WHR300HP2
	insmod("rt2880_wdt");
	int brand = getRouterBrand();
	FILE *in;
	if (brand == ROUTER_R6800 || brand == ROUTER_R6850 || brand == ROUTER_R6220)
		in = fopen("/dev/mtdblock/5", "rb");
	else
		in = fopen("/dev/mtdblock/2", "rb");

	unsigned char mac[32];
	if (in != NULL) {
		if (brand == ROUTER_DIR810L)
			fseek(in, 0x28, SEEK_SET);
		else
			fseek(in, 4, SEEK_SET);

		fread(mac, 6, 1, in);
		fclose(in);
		unsigned int copy[6];
		int i;
		for (i = 0; i < 6; i++)
			copy[i] = mac[i] & 0xff;
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure mac address to %s\n", mac);
		if (!strcmp(mac, "ff:ff:ff:ff:ff:ff"))
			set_hwaddr("eth0", "00:11:22:33:44:55");
		else
			set_hwaddr("eth0", mac);
	}
	switch (brand) {
	case ROUTER_DIR860LB1:
		insmod("thermal_sys");
		insmod("hwmon");
		insmod("compat");
		insmod("mac80211");
		insmod("mt76");
		insmod("mt76x02-lib");
		insmod("mt76x2-common");
		insmod("mt76x2e");
		//              insmod("mt7615e");
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "1 2 3 4 6t");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0 6t");
		eval("swconfig", "dev", "eth0", "set", "apply");
		//              set_smp_affinity(11, 2);        // eth
		//              set_smp_affinity(12, 4);        //wifi1
		//              set_smp_affinity(32, 8);        // wifi2
		nvram_seti("sw_cpuport", 6);
		nvram_seti("sw_wan", 0);
		nvram_seti("sw_lan1", 1);
		nvram_seti("sw_lan2", 2);
		nvram_seti("sw_lan3", 3);
		nvram_seti("sw_lan4", 4);
		break;
	case ROUTER_DIR882:
	case ROUTER_R6850:
	case ROUTER_R6220:
		insmod("thermal_sys");
		insmod("hwmon");
		insmod("compat");
		insmod("mac80211");
		if (!nvram_match("no_mt76", "1")) {
			insmod("mt76");
			insmod("mt76-connac-lib");
			insmod("mt7615-common");
			insmod("mt7615e");
			insmod("mt76x02-lib");
			insmod("mt76x2-common");
			insmod("mt76x2e");
			insmod("mt76x0-common");
			insmod("mt76x0e");
			insmod("mt7603e");
		}
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 6t");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "4 6t");
		eval("swconfig", "dev", "eth0", "set", "apply");
		//              set_smp_affinity(20, 2);        // eth
		//              set_smp_affinity(22, 4);        //wifi1
		//              set_smp_affinity(23, 8);        // wifi2
		nvram_seti("sw_cpuport", 6);
		nvram_seti("sw_wan", 4);
		nvram_seti("sw_lan1", 3);
		nvram_seti("sw_lan2", 2);
		nvram_seti("sw_lan3", 1);
		nvram_seti("sw_lan4", 0);
		break;
	case ROUTER_R6800:
		insmod("thermal_sys");
		insmod("hwmon");
		insmod("compat");
		insmod("mac80211");
		if (!nvram_match("no_mt76", "1")) {
			insmod("mt76");
			insmod("mt76-connac-lib");
			insmod("mt7615-common");
			insmod("mt7615e");
			insmod("mt76x02-lib");
			insmod("mt76x2-common");
			insmod("mt76x2e");
			insmod("mt76x0-common");
			insmod("mt76x0e");
			insmod("mt7603e");
		}
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 6t");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "4 6t");
		eval("swconfig", "dev", "eth0", "set", "apply");
		//              set_smp_affinity(20, 2);        // eth
		//              set_smp_affinity(22, 4);        //wifi1
		//              set_smp_affinity(23, 8);        // wifi2
		nvram_seti("sw_cpuport", 6);
		nvram_seti("sw_wan", 4);
		nvram_seti("sw_lan1", 3);
		nvram_seti("sw_lan2", 2);
		nvram_seti("sw_lan3", 1);
		nvram_seti("sw_lan4", 0);

		writestr("/sys/class/leds/white:wan/brightness", "0");
		writestr("/sys/class/leds/white:lan1/brightness", "0");
		writestr("/sys/class/leds/white:lan2/brightness", "0");
		writestr("/sys/class/leds/white:lan3/brightness", "0");
		writestr("/sys/class/leds/white:lan4/brightness", "0");

		writestr("/sys/class/leds/orange:wan/brightness", "0");
		writestr("/sys/class/leds/orange:lan1/brightness", "0");
		writestr("/sys/class/leds/orange:lan2/brightness", "0");
		writestr("/sys/class/leds/orange:lan3/brightness", "0");
		writestr("/sys/class/leds/orange:lan4/brightness", "0");

		writestr("/sys/class/leds/white:wan/trigger", "switch0");
		writestr("/sys/class/leds/white:lan1/trigger", "switch0");
		writestr("/sys/class/leds/white:lan2/trigger", "switch0");
		writestr("/sys/class/leds/white:lan3/trigger", "switch0");
		writestr("/sys/class/leds/white:lan4/trigger", "switch0");
		writestr("/sys/class/leds/white:wan/port_mask", "0x10");
		writestr("/sys/class/leds/white:lan1/port_mask", "0x8");
		writestr("/sys/class/leds/white:lan2/port_mask", "0x4");
		writestr("/sys/class/leds/white:lan3/port_mask", "0x2");
		writestr("/sys/class/leds/white:lan4/port_mask", "0x1");

		writestr("/sys/class/leds/white:wan/mode", "link");
		writestr("/sys/class/leds/white:lan1/mode", "link");
		writestr("/sys/class/leds/white:lan2/mode", "link");
		writestr("/sys/class/leds/white:lan3/mode", "link");
		writestr("/sys/class/leds/white:lan4/mode", "link");

		writestr("/sys/class/leds/white:wan/speed_mask", "0x8");
		writestr("/sys/class/leds/white:lan1/speed_mask", "0x8");
		writestr("/sys/class/leds/white:lan2/speed_mask", "0x8");
		writestr("/sys/class/leds/white:lan3/speed_mask", "0x8");
		writestr("/sys/class/leds/white:lan4/speed_mask", "0x8");

		writestr("/sys/class/leds/orange:wan/trigger", "switch0");
		writestr("/sys/class/leds/orange:lan1/trigger", "switch0");
		writestr("/sys/class/leds/orange:lan2/trigger", "switch0");
		writestr("/sys/class/leds/orange:lan3/trigger", "switch0");
		writestr("/sys/class/leds/orange:lan4/trigger", "switch0");

		writestr("/sys/class/leds/orange:wan/port_mask", "0x10");
		writestr("/sys/class/leds/orange:lan1/port_mask", "0x8");
		writestr("/sys/class/leds/orange:lan2/port_mask", "0x4");
		writestr("/sys/class/leds/orange:lan3/port_mask", "0x2");
		writestr("/sys/class/leds/orange:lan4/port_mask", "0x1");

		writestr("/sys/class/leds/orange:wan/mode", "link");
		writestr("/sys/class/leds/orange:lan1/mode", "link");
		writestr("/sys/class/leds/orange:lan2/mode", "link");
		writestr("/sys/class/leds/orange:lan3/mode", "link");
		writestr("/sys/class/leds/orange:lan4/mode", "link");

		writestr("/sys/class/leds/orange:wan/speed_mask", "0x4");
		writestr("/sys/class/leds/orange:lan1/speed_mask", "0x4");
		writestr("/sys/class/leds/orange:lan2/speed_mask", "0x4");
		writestr("/sys/class/leds/orange:lan3/speed_mask", "0x4");
		writestr("/sys/class/leds/orange:lan4/speed_mask", "0x4");

		writestr("/sys/class/leds/white:wlan2g/trigger", "phy0radio");
		writestr("/sys/class/leds/white:wlan5g/trigger", "phy1radio");
		break;
	case ROUTER_BOARD_E1700:
	case ROUTER_DIR810L:
		insmod("thermal_sys");
		insmod("hwmon");
		insmod("compat");
		insmod("mac80211");
		/* load soc drivers */
		insmod("rt2x00lib");
		insmod("rt2x00mmio");
		insmod("rt2x00soc");
		insmod("rt2x00pci");
		insmod("rt2800lib");
		insmod("rt2800mmio");
		insmod("rt2800soc");
		//              insmod("rt2800pci");
		if (brand == ROUTER_DIR810L) {
			insmod("mt76");
			insmod("mt76x02-lib");
			insmod("mt76x0-common");
			insmod("mt76x0e");
		}
		break;
	default:
		insmod("thermal_sys");
		insmod("hwmon");
		insmod("compat");
		insmod("mac80211");
		/* load soc drivers */
		insmod("rt2x00lib");
		insmod("rt2x00mmio");
		insmod("rt2x00soc");
		insmod("rt2x00pci");
		insmod("rt2800lib");
		insmod("rt2800mmio");
		insmod("rt2800soc");
		insmod("rt2800pci");

		insmod("mt76");
		insmod("mt76x02-lib");
		insmod("mt76x2-common");
		insmod("mt76x2e");
#ifdef HAVE_WHR1166D
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 6t");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "5 6t");
		eval("swconfig", "dev", "eth0", "set", "apply");
		nvram_seti("sw_cpuport", 6);
		nvram_seti("sw_wan", 5);
		nvram_seti("sw_lan1", 0);
		nvram_seti("sw_lan2", 1);
		nvram_seti("sw_lan3", 2);
		nvram_seti("sw_lan4", 3);
#if 0
		eval("switch", "reg", "w", "2004", "ff0003");
		eval("switch", "reg", "w", "2104", "ff0003");
		eval("switch", "reg", "w", "2204", "ff0003");
		eval("switch", "reg", "w", "2304", "ff0003");
		eval("switch", "reg", "w", "2404", "ff0003");
		eval("switch", "reg", "w", "2504", "ff0003");
		//LAN/WAN ports as transparent port
		eval("switch", "reg", "w", "2010", "810000c0");
		eval("switch", "reg", "w", "2110", "810000c0");
		eval("switch", "reg", "w", "2210", "810000c0");
		eval("switch", "reg", "w", "2310", "810000c0");
		eval("switch", "reg", "w", "2410", "810000c0");
		eval("switch", "reg", "w", "2510", "810000c0");
		//set CPU/P7 port as user port
		eval("switch", "reg", "w", "2610", "81000000");
		eval("switch", "reg", "w", "2710", "81000000");

		eval("switch", "reg", "w", "2604", "20ff0003");	// #port6, Egress VLAN Tag Attribution=tagged
		eval("switch", "reg", "w", "2704", "20ff0003");	// #port7, Egress VLAN Tag Attribution=tagged

		eval("switch", "reg", "w", "2014", "10001");
		eval("switch", "reg", "w", "2114", "10001");
		eval("switch", "reg", "w", "2214", "10001");
		eval("switch", "reg", "w", "2314", "10001");
		eval("switch", "reg", "w", "2414", "10002");
		eval("switch", "reg", "w", "2514", "10001");
		//VLAN member port
		eval("switch", "vlan", "set", "0", "1", "11110111");
		eval("switch", "vlan", "set", "1", "2", "00001011");
		eval("switch", "clear");
#endif
#else
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 6t");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "4 6t");
		eval("swconfig", "dev", "eth0", "set", "apply");
		nvram_seti("sw_cpuport", 6);
		nvram_seti("sw_wan", 4);
		nvram_seti("sw_lan1", 0);
		nvram_seti("sw_lan2", 1);
		nvram_seti("sw_lan3", 2);
		nvram_seti("sw_lan4", 3);
#endif
		break;
	}
	eval("ifconfig", "eth0", "up");

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "1 2 16000");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1"); //LAN
	eval("vconfig", "add", "eth0", "2"); //WAN

	struct ifreq ifr;
	int s;

	char eabuf[32];
	if (get_hwaddr("eth0", eabuf)) {
		nvram_set("et0macaddr_safe", eabuf);
	}
#else

#if defined(HAVE_DIR600) && !defined(HAVE_ALL02310N)
	writeproc("/proc/rt3052/mii/ctrl", "write 0 0 0x3300");
	writeproc("/proc/rt3052/mii/ctrl", "write 1 0 0x3300");
	writeproc("/proc/rt3052/mii/ctrl", "write 2 0 0x3300");
	writeproc("/proc/rt3052/mii/ctrl", "write 3 0 0x3300");
#endif
#if defined(HAVE_RT10N) || defined(HAVE_F5D8235) || defined(HAVE_RT15N) || defined(HAVE_WCRGN) && !defined(HAVE_HAMEA15)
	FILE *in = fopen("/dev/mtdblock/2", "rb");
	unsigned char mac[32];
	if (in != NULL) {
		fseek(in, 4, SEEK_SET);
		fread(mac, 6, 1, in);
		fclose(in);
		unsigned int copy[6];
		int i;
		for (i = 0; i < 6; i++)
			copy[i] = mac[i] & 0xff;
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		if (!strcmp(mac, "ff:ff:ff:ff:ff:ff"))
			set_hwaddr("eth2", "00:11:22:33:44:55");
		else
			set_hwaddr("eth2", mac);
	}
#endif
#ifdef HAVE_HAMEA15
	FILE *in = fopen("/dev/mtdblock/1", "rb");
	if (in != NULL) {
		unsigned char *config = calloc(65536, 1);
		fread(config, 65536, 1, in);
		int len = sizeof("WAN_MAC_ADDR=") - 1;
		int i;
		for (i = 0; i < 65535 - (len + 18); i++) {
			if (!strncmp(&config[i], "WAN_MAC_ADDR=", len)) {
				char *mac = &config[i + len];
				if (mac[0] == '"')
					mac++;
				mac[17] = 0;
				set_hwaddr("eth2", mac);
				nvram_set("et0macaddr_safe", mac);
				nvram_set("et0macaddr", mac);
				break;
			}
		}
		free(config);
		fclose(in);
	}
#endif
#if (defined(HAVE_DIR600) || defined(HAVE_AR670W) || defined(HAVE_EAP9550) || defined(HAVE_AR690W)) && !defined(HAVE_ALL02310N)

	FILE *in = fopen("/dev/mtdblock/1", "rb");
	if (in != NULL) {
		unsigned char *config = calloc(65536, 1);
		fread(config, 65536, 1, in);
#if defined(HAVE_AR670W) || defined(HAVE_AR690W)
		int len = sizeof("lanmac=") - 1;
#else
		int len = sizeof("ethaddr=") - 1;
#endif
		int i;
		for (i = 0; i < 65535 - (18 + len); i++) {
#if defined(HAVE_AR670W) || defined(HAVE_AR690W)
			if (!strncmp(&config[i], "lanmac=", 7))
#else
			if (!strncmp(&config[i], "ethaddr=", 8))
#endif
			{
				char *mac = &config[i + len];
				if (mac[0] == '"')
					mac++;
				mac[17] = 0;
				set_hwaddr("eth2", mac);
				nvram_set("et0macaddr_safe", mac);
				nvram_set("et0macaddr", mac);
				break;
			}
		}
		free(config);
		fclose(in);
	}
#endif

	/* switch config */
	if (getRouterBrand() != ROUTER_BOARD_ECB9750 && getRouterBrand() != ROUTER_BOARD_TECHNAXX3G) // lets load
	{
		eval("ifconfig", "eth2", "up");
#ifndef HAVE_EAP9550
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth2", "1"); //LAN

		eval("vconfig", "add", "eth2", "2"); //WAN
#ifdef HAVE_RT10N
		MAC_ADD(mac);
		set_hwaddr("vlan2", mac);
#endif
#endif

#ifdef HAVE_RUT500
		eval("switch", "reg", "w", "14", "7f5555");
		eval("switch", "reg", "w", "40", "1002");
		eval("switch", "reg", "w", "44", "1001");
		eval("switch", "reg", "w", "48", "1");
		eval("switch", "reg", "w", "50", "2001");
		eval("switch", "reg", "w", "54", "0");
		eval("switch", "reg", "w", "58", "0");
		eval("switch", "reg", "w", "5c", "0");
		eval("switch", "reg", "w", "60", "0");
		eval("switch", "reg", "w", "64", "0");
		eval("switch", "reg", "w", "68", "0");
		eval("switch", "reg", "w", "6c", "0");
		eval("switch", "reg", "w", "70", "415e");
		eval("switch", "reg", "w", "74", "0");
		eval("switch", "reg", "w", "78", "0");
		eval("switch", "reg", "w", "7c", "0");
		eval("switch", "reg", "w", "90", "10007f7f");
		eval("switch", "reg", "w", "98", "7f1f");
		eval("switch", "reg", "w", "e4", "3e000000");
		writeproc("/proc/rt3052/mii/ctrl", "write 0 0 0x3300");
		writeproc("/proc/rt3052/mii/ctrl", "write 1 0 0x3300");
		writeproc("/proc/rt3052/mii/ctrl", "write 2 0 0x3300");
		writeproc("/proc/rt3052/mii/ctrl", "write 3 0 0x3300");
		writeproc("/proc/rt3052/mii/ctrl", "write 4 0 0x3300");
#elif defined(HAVE_ALLNET11N) || defined(HAVE_ESR6650) || defined(HAVE_WR5422) || defined(HAVE_RT10N) || defined(HAVE_ACXNR22) || \
	defined(HAVE_W502U) || defined(HAVE_ESR9752) || defined(HAVE_ALL02310N)
		eval("switch", "reg", "w", "14", "405555");
		eval("switch", "reg", "w", "40", "1002");
		eval("switch", "reg", "w", "44", "1001");
		eval("switch", "reg", "w", "48", "1001");
		eval("switch", "reg", "w", "50", "2001");
		eval("switch", "reg", "w", "70", "ffff417e");
		eval("switch", "reg", "w", "90", "7f7f");
		eval("switch", "reg", "w", "98", "7f3f");
		eval("switch", "reg", "w", "e4", "3f");
#ifdef HAVE_ESR9752
		eval("switch", "reg", "w", "c8", "3f502b28");
#endif
#elif HAVE_AR670W
		eval("mii_mgr", "-s", "-p", "29", "-r", "23", "-v", "0x07c2");
		eval("mii_mgr", "-s", "-p", "29", "-r", "22", "-v", "0x8420");

		eval("mii_mgr", "-s", "-p", "29", "-r", "24", "-v", "0x1");
		eval("mii_mgr", "-s", "-p", "29", "-r", "25", "-v", "0x1");
		eval("mii_mgr", "-s", "-p", "29", "-r", "26", "-v", "0x1");
		eval("mii_mgr", "-s", "-p", "29", "-r", "27", "-v", "0x1");
		eval("mii_mgr", "-s", "-p", "29", "-r", "28", "-v", "0x2");
		eval("mii_mgr", "-s", "-p", "30", "-r", "9", "-v", "0x1089");
		eval("mii_mgr", "-s", "-p", "30", "-r", "1", "-v", "0x2f00");
		eval("mii_mgr", "-s", "-p", "30", "-r", "2", "-v", "0x0030");
#elif HAVE_AR690W
#elif HAVE_RT15N
#elif HAVE_BR6574N
#elif HAVE_F5D8235
		eval("switch", "reg", "w", "14", "405555");
		eval("switch", "reg", "w", "e4", "20");
		eval("switch", "reg", "w", "40", "1001");
		eval("switch", "reg", "w", "44", "1001");
		eval("switch", "reg", "w", "48", "1001");
		eval("switch", "reg", "w", "50", "2001");
		eval("switch", "reg", "w", "4c", "1");
		eval("switch", "reg", "w", "70", "ffffffff");
		eval("switch", "reg", "w", "90", "7f7f");
		eval("switch", "reg", "w", "98", "7f40");
#elif HAVE_EAP9550
		eval("switch", "reg", "w", "14", "5555");
		eval("switch", "reg", "w", "40", "1001");
		eval("switch", "reg", "w", "44", "1001");
		eval("switch", "reg", "w", "48", "1001");
		eval("switch", "reg", "w", "4c", "1");
		eval("switch", "reg", "w", "50", "2001");
		eval("switch", "reg", "w", "70", "ffffffff");
		eval("switch", "reg", "w", "90", "7f7f");
		eval("switch", "reg", "w", "98", "7f7f");
		eval("switch", "reg", "w", "e4", "7f");
#else
		eval("switch", "reg", "w", "14", "405555");
		eval("switch", "reg", "w", "50", "2001");
		eval("switch", "reg", "w", "90", "7f7f");
		eval("switch", "reg", "w", "98", "7f3f");
		eval("switch", "reg", "w", "e4", "3f");
		eval("switch", "reg", "w", "40", "1001");
		eval("switch", "reg", "w", "44", "1001");
		eval("switch", "reg", "w", "48", "1002");
		eval("switch", "reg", "w", "70", "ffff506f");
#endif
	}

	{
		char macaddr[32];
		if (get_hwaddr("eth2", macaddr)) {
			nvram_set("et0macaddr_safe", macaddr);
		}
	}
#endif
#ifdef HAVE_WCRGN
	set_gpio(0, 1);
	set_gpio(10, 1);
#endif

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
#ifdef HAVE_WHR300HP2
	return "eth2";
#endif

	if (getRouterBrand() != ROUTER_BOARD_ECB9750 && getRouterBrand() != ROUTER_BOARD_TECHNAXX3G) {
		if (enable) {
#if !defined(HAVE_AR670W) && !defined(HAVE_BR6574N) && !defined(HAVE_F5D8235)
			eval("switch", "reg", "w", "14", "405555");
			eval("switch", "reg", "w", "50", "7001");
			eval("switch", "reg", "w", "90", "7f7f");
			eval("switch", "reg", "w", "98", "7f2f");
			eval("switch", "reg", "w", "e4", "2f");
#if defined(HAVE_ALLNET11N) || defined(HAVE_ESR6650) || defined(HAVE_WR5422) || defined(HAVE_RT10N) || defined(HAVE_ACXNR22) || \
	defined(HAVE_W502U) || defined(HAVE_ESR9752) || defined(HAVE_ALL02310N)
			eval("switch", "reg", "w", "40", "1007");
			eval("switch", "reg", "w", "44", "1001");
			eval("switch", "reg", "w", "48", "1001");
			eval("switch", "reg", "w", "70", "ffff417e");
#ifdef HAVE_ESR9752
			eval("switch", "reg", "w", "c8", "3f502b28");
#endif
#else
			eval("switch", "reg", "w", "40", "1001");
			eval("switch", "reg", "w", "44", "1001");
			eval("switch", "reg", "w", "48", "1007");
			eval("switch", "reg", "w", "70", "ffff506f");
#endif
#endif
			// now we got vlan7, how do we trunk now. lets find out
			return "eth2";
		} else {
#ifdef HAVE_RUT500
			eval("switch", "reg", "w", "14", "7f5555");
			eval("switch", "reg", "w", "40", "1002");
			eval("switch", "reg", "w", "44", "1001");
			eval("switch", "reg", "w", "48", "1");
			eval("switch", "reg", "w", "50", "2001");
			eval("switch", "reg", "w", "54", "0");
			eval("switch", "reg", "w", "58", "0");
			eval("switch", "reg", "w", "5c", "0");
			eval("switch", "reg", "w", "60", "0");
			eval("switch", "reg", "w", "64", "0");
			eval("switch", "reg", "w", "68", "0");
			eval("switch", "reg", "w", "6c", "0");
			eval("switch", "reg", "w", "70", "415e");
			eval("switch", "reg", "w", "74", "0");
			eval("switch", "reg", "w", "78", "0");
			eval("switch", "reg", "w", "7c", "0");
			eval("switch", "reg", "w", "90", "10007f7f");
			eval("switch", "reg", "w", "98", "7f1f");
			eval("switch", "reg", "w", "e4", "3e000000");
#elif !defined(HAVE_AR670W) && !defined(HAVE_BR6574N) && !defined(HAVE_F5D8235)
			eval("switch", "reg", "w", "14", "405555");
			eval("switch", "reg", "w", "50", "2001");
			eval("switch", "reg", "w", "90", "7f7f");
			eval("switch", "reg", "w", "98", "7f3f");
			eval("switch", "reg", "w", "e4", "3f");
#if defined(HAVE_ALLNET11N) || defined(HAVE_ESR6650) || defined(HAVE_WR5422) || defined(HAVE_RT10N) || defined(HAVE_ACXNR22) || \
	defined(HAVE_W502U) || defined(HAVE_ESR9752) || defined(HAVE_ALL02310N)
			eval("switch", "reg", "w", "40", "1002");
			eval("switch", "reg", "w", "44", "1001");
			eval("switch", "reg", "w", "48", "1001");
			eval("switch", "reg", "w", "70", "ffff417e");
#ifdef HAVE_ESR9752
			eval("switch", "reg", "w", "c8", "3f502b28");
#endif
#elif HAVE_BR6574N
#elif HAVE_AR690W
#elif HAVE_RT15N
#elif HAVE_AR670W
#elif HAVE_F5D8235
#else
			eval("switch", "reg", "w", "40", "1001");
			eval("switch", "reg", "w", "44", "1001");
			eval("switch", "reg", "w", "48", "1002");
			eval("switch", "reg", "w", "70", "ffff506f");
#endif
			eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
			eval("vconfig", "add", "eth2", "2"); //WAN
			return "eth2";
#endif
		}
	} else {
		return "eth2";
	}
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