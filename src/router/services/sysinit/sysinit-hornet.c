/*
 * sysinit-hornet.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include "devices/ethtools.c"

void start_sysinit(void)
{
	time_t tm = 0;

	if (!nvram_match("disable_watchdog", "1"))
		eval("watchdog");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	/*
	 * network drivers 
	 */
	fprintf(stderr, "load ATH Ethernet Driver\n");
	system("insmod ag71xx || insmod ag7240_mod");
	insmod("ledtrig-netdev");
#ifdef HAVE_WR741V4
	FILE *fp = fopen("/dev/mtdblock/0", "rb");
	char mac[32];
	if (fp) {
		unsigned char buf2[256];
		fseek(fp, 0x1fc00, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		MAC_SUB(mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		MAC_ADD(mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		eval("ifconfig", "eth1", "hw", "ether", mac);
#ifndef HAVE_ATH9K
		MAC_SUB(mac);
#endif

	}
#else
	FILE *fp = fopen("/dev/mtdblock/5", "rb");
	char mac[32], mac2[32];
	if (fp) {
		unsigned char buf2[256];
		fseek(fp, 0, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		if (!memcmp(&copy[0], &copy[6], 3)) {
			sprintf(mac2, "%02x:%02x:%02x:%02x:%02x:%02x", copy[6], copy[7], copy[8], copy[9], copy[10], copy[11]);
			if (copy[5] < copy[11]) {
				fprintf(stderr, "Using first mac for eth0 (%s)\n", mac);
				eval("ifconfig", "eth0", "hw", "ether", mac);
				fprintf(stderr, "Using second mac for eth1 (%s)\n", mac2);
				eval("ifconfig", "eth1", "hw", "ether", mac2);
			}
			else {
				fprintf(stderr, "Using second mac for eth0 (%s)\n", mac2);
				eval("ifconfig", "eth0", "hw", "ether", mac2);
				fprintf(stderr, "Using first mac for eth1 (%s)\n", mac);
				eval("ifconfig", "eth1", "hw", "ether", mac);
			}
		}
		else {
			fprintf(stderr, "configure eth0 to %s\n", mac);
			eval("ifconfig", "eth0", "hw", "ether", mac);
			MAC_ADD(mac);
			fprintf(stderr, "configure eth1 to %s\n", mac);
			eval("ifconfig", "eth1", "hw", "ether", mac);
		}
#ifndef HAVE_ATH9K
		MAC_SUB(mac);
#endif

	}
#endif
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");

#ifdef HAVE_WR741V4
#ifdef HAVE_SWCONFIG
	system("swconfig dev eth1 set reset 1");
	system("swconfig dev eth1 set enable_vlan 0");
	system("swconfig dev eth1 vlan 1 set ports \"0 1 2 3 4\"");
	system("swconfig dev eth1 set apply");
#endif
#ifndef HAVE_WR703
	setEthLED(13, "eth0");
	setSwitchLED(14, 0x4);
	setSwitchLED(15, 0x8);
	setSwitchLED(16, 0x10);
	setSwitchLED(17, 0x02);
#endif
#ifdef HAVE_MR3020
	setEthLED(17, "eth1");
#endif
#endif
#ifdef HAVE_CARAMBOLA
	system("swconfig dev switch0 set reset 1");
	system("swconfig dev switch0 set enable_vlan 1");
	system("swconfig dev switch0 vlan 1 set ports \"0t 1\"");
	system("swconfig dev switch0 vlan 2 set ports \"0t 2\"");
	system("swconfig dev switch0 set apply");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth1", "1");
	eval("vconfig", "add", "eth1", "2");
#endif
#ifdef HAVE_HORNET
#ifdef HAVE_ONNET
	setEthLED(13,"eth0");
	setEthLED(17,"eth1");
#else
	setEthLED(17,"eth0");
	setEthLED(13,"eth1");
#endif
#endif
	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr_safe", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}

	detect_wireless_devices();

	led_control(LED_POWER, LED_ON);
	led_control(LED_SES, LED_OFF);
	led_control(LED_SES2, LED_OFF);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN0, LED_OFF);
	led_control(LED_WLAN1, LED_OFF);
	led_control(LED_CONNECTED, LED_OFF);
	setWirelessLed(0, 0);

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");

	return;
	cprintf("done\n");
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
	return "eth0";
}
