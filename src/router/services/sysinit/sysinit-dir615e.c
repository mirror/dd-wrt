/*
 * sysinit-dir615e.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
//#ifndef HAVE_DIR632
#ifndef HAVE_WR841V8
	eval("ifconfig", "eth0", "hw", "ether", "00:11:22:33:44:55");
	eval("ifconfig", "eth1", "hw", "ether", "00:11:22:33:44:66");
	FILE *in = fopen("/dev/mtdblock/6", "rb");
	char *lanmac = NULL;
	if (in != NULL) {
#ifdef HAVE_DIR632
		fseek(in, 0x40000, SEEK_SET);
#elif HAVE_DIR615I
		fseek(in, 0x10000, SEEK_SET);
#else
		fseek(in, 0x30000, SEEK_SET);
#endif
		unsigned char *config = malloc(65536);
		memset(config, 0, 65536);
		fread(config, 65536, 1, in);
		int len = sizeof("lan_mac=");
		int i;
		int haslan = 0;
		int haswan = 0;
		for (i = 0; i < 65535 - 18; i++) {
			if (!haslan && !strncmp(&config[i], "lan_mac=", 8)) {
				haslan = 1;
				char *mac = &config[i + 8];
				if (mac[0] == '"')
					mac++;
				mac[17] = 0;
				lanmac = malloc(32);
				strcpy(lanmac, mac);
#ifdef HAVE_DIR632
				eval("ifconfig", "eth0", "hw", "ether", mac);
#else
				eval("ifconfig", "eth1", "hw", "ether", mac);
#endif
				nvram_set("et0macaddr_safe", mac);
				nvram_set("et0macaddr", mac);
				if (haswan)
					break;
			}
			if (!haswan && !strncmp(&config[i], "wan_mac=", 8)) {
				haswan = 1;
				char *mac = &config[i + 8];
				if (mac[0] == '"')
					mac++;
				mac[17] = 0;
#ifdef HAVE_DIR632
				eval("ifconfig", "eth1", "hw", "ether", mac);
#else
				eval("ifconfig", "eth0", "hw", "ether", mac);
#endif
				nvram_set("et0macaddr_safe", mac);
				nvram_set("et0macaddr", mac);
				if (haslan)
					break;
			}
		}
		free(config);
		fclose(in);
	}
//#endif
#endif
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
#ifdef HAVE_SWCONFIG
	system("swconfig dev eth1 set reset 1");
	system("swconfig dev eth1 set enable_vlan 0");
	system("swconfig dev eth1 vlan 1 set ports \"0 1 2 3 4\"");
	system("swconfig dev eth1 set apply");

#ifndef HAVE_DIR615I
#ifndef HAVE_DIR632
	setEthLED(17, "eth0");
	setSwitchLED(13, 0x2);
	setSwitchLED(14, 0x4);
	setSwitchLED(15, 0x8);
	setSwitchLED(16, 0x10);
#endif
#endif
#ifdef HAVE_WR841V8
	setEthLED(18, "eth0");
	setSwitchLED(19, 0x4);
	setSwitchLED(20, 0x8);
	setSwitchLED(21, 0x10);
	setSwitchLED(12, 0x02);
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
#ifndef HAVE_ATH9K
	if (lanmac != NULL) {
		fprintf(stderr, "configure wifi0 to %s\n", lanmac);
		eval("ifconfig", "wifi0", "hw", "ether", lanmac);
		free(lanmac);
	}
#endif
#ifdef HAVE_DIR632
	setWirelessLedPhy0(0);
#endif
#ifdef HAVE_DIR615I
	setWirelessLedGeneric(0, 13);
#endif
	led_control(LED_POWER, LED_ON);
	led_control(LED_SES, LED_OFF);
	led_control(LED_SES2, LED_OFF);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN0, LED_OFF);
	led_control(LED_WLAN1, LED_OFF);
	led_control(LED_CONNECTED, LED_OFF);

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
