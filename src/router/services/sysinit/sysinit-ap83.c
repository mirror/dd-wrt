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
	time_t tm = 0;

	cprintf("sysinit() setup console\n");
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
#ifdef HAVE_WR1043
	fprintf(stderr, "load RTL Switch Driver\n");
	insmod("rtl8366rb_smi");
//      insmod("swconfig");
	insmod("rtl8366_smi");
	insmod("rtl8366rb");
#endif
	fprintf(stderr, "load ag71xx or ag7100_mod Ethernet Driver\n");
	system("insmod ag71xx || insmod ag7100_mod");
#ifdef HAVE_WZRG300NH
	system("swconfig dev rtl8366s set reset 1");
	system("swconfig dev rtl8366s set enable_vlan 0");
	system("swconfig dev rtl8366s set apply");
	FILE *fp = fopen("/dev/mtdblock/6", "rb");
	if (fp) {
		unsigned char buf2[256];
		fseek(fp, 0x1ff120c, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		char mac[32];
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
			copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		eval("ifconfig", "eth1", "hw", "ether", mac);
	}
#endif
#ifdef HAVE_WR1043

	FILE *fp = fopen("/dev/mtdblock/0", "rb");
	char mac[32];
	if (fp) {
		system("swconfig dev rtl8366rb set reset 1");
		system("swconfig dev rtl8366rb set enable_vlan 1");
		system
		    ("swconfig dev rtl8366rb vlan 1 set ports \"1 2 3 4 5t\"");
		system("swconfig dev rtl8366rb vlan 2 set ports \"0 5t\"");
		system("swconfig dev rtl8366s set apply");
		unsigned char buf2[256];
		fseek(fp, 0x1fc00, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
			copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		fprintf(stderr, "configure vlan1 to %s\n", mac);
		eval("ifconfig", "vlan1", "hw", "ether", mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure vlan2 to %s\n", mac);
		eval("ifconfig", "vlan2", "hw", "ether", mac);
		MAC_SUB(mac);
	}
#endif
#ifdef HAVE_WA901
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
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
			copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		eval("ifconfig", "eth0", "up");
	}
#elif HAVE_WR941

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
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
			copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "0");
		eval("vconfig", "add", "eth0", "1");
		fprintf(stderr, "configure vlan0 to %s\n", mac);
		MAC_SUB(mac);
		eval("ifconfig", "vlan0", "hw", "ether", mac);
		MAC_ADD(mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure vlan1 to %s\n", mac);
		eval("ifconfig", "vlan1", "hw", "ether", mac);
		MAC_SUB(mac);
		MAC_SUB(mac);
	}
#endif
#ifdef HAVE_WRT160NL
	FILE *fp = fopen("/dev/mtdblock/0", "rb");
	unsigned char buf2[256];
	if (fp) {
#ifdef HAVE_E2100
		unsigned int firstoffset = 0x3f29a;
		unsigned int secondoffset = 0x3f288;
#else
		unsigned int firstoffset = 0x3f288;
		unsigned int secondoffset = 0x3f29a;
#endif
		fseek(fp, firstoffset, SEEK_SET);
		fread(buf2, 19, 1, fp);
		if (buf2[0] == 0xff)
			fseek(fp, secondoffset, SEEK_SET);
		fread(buf2, 19, 1, fp);

		fclose(fp);
		fprintf(stderr, "configure eth0 to %s\n", buf2);
		eval("ifconfig", "eth0", "hw", "ether", buf2);
		MAC_ADD(buf2);
		fprintf(stderr, "configure eth1 to %s\n", buf2);
		eval("ifconfig", "eth1", "hw", "ether", buf2);
	}
#endif
#ifdef HAVE_TG2521
	eval("ifconfig", "eth0", "hw", "ether", "00:11:22:33:44:55");
	eval("ifconfig", "eth1", "hw", "ether", "00:11:22:33:44:66");
	FILE *fp = fopen("/dev/mtdblock/7", "rb");
	char mac[32];
	if (fp) {
		unsigned char buf2[256];
		fseek(fp, 0x7d08c3, SEEK_SET);	// mac location
		fread(buf2, 6, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 6; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x",
			copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		eval("ifconfig", "eth1", "hw", "ether", mac);
		MAC_SUB(mac);
	}
#endif
#if defined(HAVE_TEW632BRP) || defined(HAVE_DIR615E)
	eval("ifconfig", "eth0", "hw", "ether", "00:11:22:33:44:55");
	eval("ifconfig", "eth1", "hw", "ether", "00:11:22:33:44:66");
	FILE *in = fopen("/dev/mtdblock/0", "rb");
	char *lanmac = NULL;
	if (in != NULL) {
		fseek(in, 0x20000, SEEK_SET);
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
				eval("ifconfig", "eth0", "hw", "ether", mac);
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
				eval("ifconfig", "eth1", "hw", "ether", mac);
				nvram_set("et0macaddr_safe", mac);
				nvram_set("et0macaddr", mac);
				if (haslan)
					break;
			}
		}
		free(config);
		fclose(in);
	}
#endif
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr",
			  ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data,
				     eabuf));
		nvram_set("et0macaddr_safe",
			  ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data,
				     eabuf));
		close(s);
	}
	detect_wireless_devices();
#ifdef HAVE_WRT160NL
	MAC_ADD(buf2);
	fprintf(stderr, "configure wifi0 to %s\n", buf2);
	eval("ifconfig", "wifi0", "hw", "ether", buf2);
	led_control(LED_POWER, LED_ON);
#endif
#if defined(HAVE_TEW632BRP) || defined(HAVE_DIR615E)
	if (lanmac != NULL) {
		fprintf(stderr, "configure wifi0 to %s\n", lanmac);
		eval("ifconfig", "wifi0", "hw", "ether", lanmac);
		free(lanmac);
	}
#endif
#ifdef HAVE_TG2521
	{
		fprintf(stderr, "configure wifi0 to %s\n", mac);
		eval("ifconfig", "wifi0", "hw", "ether", mac);
	}
//      eval("gpio", "disable", "5");   // enable usb port
#endif
#ifdef HAVE_WR1043
	{
		fprintf(stderr, "configure wifi0 to %s\n", mac);
		eval("ifconfig", "wifi0", "hw", "ether", mac);
	}
#endif
#ifdef HAVE_WR941
	{
		fprintf(stderr, "configure wifi0 to %s\n", mac);
		eval("ifconfig", "wifi0", "hw", "ether", mac);
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

#ifdef HAVE_RS
	setWirelessLed(0, 2);
	setWirelessLed(1, 2);
	setWirelessLed(2, 2);
#elif HAVE_WRT160NL
	setWirelessLed(0, 6);
	writeproc("/proc/sys/dev/wifi0/ledpin", "6");
	writeproc("/proc/sys/dev/wifi0/softled", "1");
	system("swconfig dev eth0 set reset 1");
	system("swconfig dev eth0 set enable_vlan 1");
	system("swconfig dev eth0 vlan 1 set ports \"0 1 2 3 4 5\"");
#elif HAVE_WZRG300NH
	setWirelessLed(0, 6);
#elif HAVE_TEW632BRP
	setWirelessLed(0, 6);
#elif HAVE_WR941
	setWirelessLed(0, 9);
#elif HAVE_WR1043
	setWirelessLed(0, 9);
#else
	setWirelessLed(0, 2);
#endif

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
