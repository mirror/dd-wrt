/*
 * sysinit-rt2880.c
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
#include <cymac.h>

#define sys_reboot() eval("sync"); eval("event","3","1","15")

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

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	/*
	 * load some netfilter stuff 
	 */

//    eval( "watchdog" );
	/*
	 * Set a sane date 
	 */

	stime(&tm);
	nvram_set("wl0_ifname", "ra0");
	insmod("rt2860v2_ap");
	insmod("raeth");
#ifdef HAVE_WHR300HP2
	insmod("rt2880_wdt");


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
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr,"configure mac address to %s\n",mac);
		if (!strcmp(mac, "ff:ff:ff:ff:ff:ff"))
			eval("ifconfig", "eth0", "hw", "ether", "00:11:22:33:44:55");
		else
			eval("ifconfig", "eth0", "hw", "ether", mac);
	}

/*	system("swconfig dev eth0 set reset 1");
	system("swconfig dev eth0 set enable_vlan 1");
	system("swconfig dev eth0 vlan 1 set ports \"0 1 2 3 6t\"");
	system("swconfig dev eth0 vlan 2 set ports \"4 6t\"");
	system("swconfig dev eth0 set apply");*/
	
	//LAN/WAN ports as security mode
	sysprintf("switch reg w 2004 ff0003");
	sysprintf("switch reg w 2104 ff0003");
	sysprintf("switch reg w 2204 ff0003");
	sysprintf("switch reg w 2304 ff0003");
	sysprintf("switch reg w 2404 ff0003");
	sysprintf("switch reg w 2504 ff0003");
	//LAN/WAN ports as transparent port
	sysprintf("switch reg w 2010 810000c0");
	sysprintf("switch reg w 2110 810000c0");
	sysprintf("switch reg w 2210 810000c0");
	sysprintf("switch reg w 2310 810000c0");	
	sysprintf("switch reg w 2410 810000c0");
	sysprintf("switch reg w 2510 810000c0");
	//set CPU/P7 port as user port
	sysprintf("switch reg w 2610 81000000");
	sysprintf("switch reg w 2710 81000000");

	sysprintf("switch reg w 2604 20ff0003");// #port6, Egress VLAN Tag Attribution=tagged
	sysprintf("switch reg w 2704 20ff0003");// #port7, Egress VLAN Tag Attribution=tagged
	

	sysprintf("switch reg w 2014 10001");
	sysprintf("switch reg w 2114 10001");
	sysprintf("switch reg w 2214 10001");
	sysprintf("switch reg w 2314 10001");
	sysprintf("switch reg w 2414 10002");
	sysprintf("switch reg w 2514 10001");
	//VLAN member port
	sysprintf("switch vlan set 0 1 11110111");
	sysprintf("switch vlan set 1 2 00001011");
	sysprintf("switch clear");
	eval("ifconfig", "eth0", "up");
	
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");	//LAN
	eval("vconfig", "add", "eth0", "2");	//WAN

	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr_safe", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}

#else

#ifdef HAVE_DIR600
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
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		if (!strcmp(mac, "ff:ff:ff:ff:ff:ff"))
			eval("ifconfig", "eth2", "hw", "ether", "00:11:22:33:44:55");
		else
			eval("ifconfig", "eth2", "hw", "ether", mac);
	}
#endif
#ifdef HAVE_HAMEA15
	FILE *in = fopen("/dev/mtdblock/1", "rb");
	if (in != NULL) {
		unsigned char *config = malloc(65536);
		memset(config, 0, 65536);
		fread(config, 65536, 1, in);
		int len = strlen("WAN_MAC_ADDR=");
		int i;
		for (i = 0; i < 65535 - (len + 18); i++) {
			if (!strncmp(&config[i], "WAN_MAC_ADDR=", len)) {
				char *mac = &config[i + len];
				if (mac[0] == '"')
					mac++;
				mac[17] = 0;
				eval("ifconfig", "eth2", "hw", "ether", mac);
				nvram_set("et0macaddr_safe", mac);
				nvram_set("et0macaddr", mac);
				break;
			}
		}
		free(config);
		fclose(in);
	}
#endif
#if defined(HAVE_DIR600) || defined(HAVE_AR670W) || defined(HAVE_EAP9550) || defined(HAVE_AR690W)
	FILE *in = fopen("/dev/mtdblock/1", "rb");
	if (in != NULL) {
		unsigned char *config = malloc(65536);
		memset(config, 0, 65536);
		fread(config, 65536, 1, in);
#if defined(HAVE_AR670W) || defined(HAVE_AR690W)
		int len = strlen("lanmac=");
#else
		int len = strlen("ethaddr=");
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
				eval("ifconfig", "eth2", "hw", "ether", mac);
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
	if (getRouterBrand() != ROUTER_BOARD_ECB9750 && getRouterBrand() != ROUTER_BOARD_TECHNAXX3G)	// lets load
	{
		eval("ifconfig", "eth2", "up");
#ifndef HAVE_EAP9550
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth2", "1");	//LAN 

		eval("vconfig", "add", "eth2", "2");	//WAN
#ifdef HAVE_RT10N
		MAC_ADD(mac);
		eval("ifconfig", "vlan2", "hw", "ether", mac);
#endif
#endif

#ifdef HAVE_ALLNET11N
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ESR9752
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
		sysprintf("switch reg w c8 3f502b28");
#elif HAVE_ESR6650
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#elif HAVE_WR5422
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#elif HAVE_RT10N
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ACXNR22
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#elif HAVE_W502U
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#elif HAVE_AR670W
		sysprintf("mii_mgr -s -p 29 -r 23 -v 0x07c2");
		sysprintf("mii_mgr -s -p 29 -r 22 -v 0x8420");

		sysprintf("mii_mgr -s -p 29 -r 24 -v 0x1");
		sysprintf("mii_mgr -s -p 29 -r 25 -v 0x1");
		sysprintf("mii_mgr -s -p 29 -r 26 -v 0x1");
		sysprintf("mii_mgr -s -p 29 -r 27 -v 0x1");
		sysprintf("mii_mgr -s -p 29 -r 28 -v 0x2");
		sysprintf("mii_mgr -s -p 30 -r 9 -v 0x1089");
		sysprintf("mii_mgr -s -p 30 -r 1 -v 0x2f00");
		sysprintf("mii_mgr -s -p 30 -r 2 -v 0x0030");
#elif HAVE_AR690W
#elif HAVE_RT15N
#elif HAVE_BR6574N
#elif HAVE_F5D8235
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f40");
		sysprintf("switch reg w e4 20");
		sysprintf("switch reg w 40 1001");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 4c 1");
		sysprintf("switch reg w 70 ffffffff");
#elif HAVE_EAP9550
		sysprintf("switch reg w 14 5555");
		sysprintf("switch reg w 40 1001");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 4c 1");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 70 ffffffff");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f7f");
		sysprintf("switch reg w e4 7f");
#else
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 90 7f7f");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
		sysprintf("switch reg w 40 1001");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1002");
		sysprintf("switch reg w 70 ffff506f");
#endif
	}

	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth2", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr_safe", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}
#endif
	led_control(LED_POWER, LED_ON);
	led_control(LED_SES, LED_OFF);
	led_control(LED_SES2, LED_OFF);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN0, LED_OFF);
	led_control(LED_WLAN1, LED_OFF);
	led_control(LED_CONNECTED, LED_OFF);
#ifdef HAVE_WCRGN
	sysprintf("gpio enable 0");	// ses fixup
	sysprintf("gpio enable 10");	// reset fixup
#endif

	if (!nvram_match("disable_watchdog", "1")) {
		eval("watchdog");
	}
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
#ifdef HAVE_WHR300HP2
	return "eth2";
#endif

	if (getRouterBrand() != ROUTER_BOARD_ECB9750 && getRouterBrand() != ROUTER_BOARD_TECHNAXX3G) {
		if (enable) {
#if !defined(HAVE_AR670W) && !defined(HAVE_BR6574N) && !defined(HAVE_F5D8235)
			sysprintf("switch reg w 14 405555");
			sysprintf("switch reg w 50 7001");
			sysprintf("switch reg w 90 7f7f");
			sysprintf("switch reg w 98 7f2f");
			sysprintf("switch reg w e4 2f");
#ifdef HAVE_ALLNET11N
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ESR9752
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
			sysprintf("switch reg w c8 3f502b28");
#elif HAVE_ESR6650
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_WR5422
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_RT10N
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ACXNR22
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_W502U
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#else
			sysprintf("switch reg w 40 1001");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1007");
			sysprintf("switch reg w 70 ffff506f");
#endif
#endif
			// now we got vlan7, how do we trunk now. lets find out
			return "eth2";
		} else {
#if !defined(HAVE_AR670W) && !defined(HAVE_BR6574N) && !defined(HAVE_F5D8235)
			sysprintf("switch reg w 14 405555");
			sysprintf("switch reg w 50 2001");
			sysprintf("switch reg w 90 7f7f");
			sysprintf("switch reg w 98 7f3f");
			sysprintf("switch reg w e4 3f");
#ifdef HAVE_ALLNET11N
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ESR9752
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
			sysprintf("switch reg w c8 3f502b28");
#elif HAVE_WR5422
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_RT10N
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ESR6650
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_ACXNR22
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_W502U
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#elif HAVE_BR6574N
#elif HAVE_AR690W
#elif HAVE_RT15N
#elif HAVE_AR670W
#elif HAVE_F5D8235
#else
			sysprintf("switch reg w 40 1001");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1002");
			sysprintf("switch reg w 70 ffff506f");
#endif
			eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
			eval("vconfig", "add", "eth2", "2");	//WAN
			return "eth2";
#endif
		}
	} else {
		return "eth2";
	}
}
