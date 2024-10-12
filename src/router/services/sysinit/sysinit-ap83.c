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
	time_t tm = 0;

	cprintf("sysinit() setup console\n");
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
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
	eval("swconfig", "dev", "rtl8366s", "set", "reset", "1");
	eval("swconfig", "dev", "rtl8366s", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "rtl8366s", "set", "apply");
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
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		set_hwaddr("eth1", mac);
	}
#endif
#ifdef HAVE_WR1043

	FILE *fp = fopen("/dev/mtdblock/0", "rb");
	char mac[32];
	if (fp) {
		eval("swconfig", "dev", "rtl8366rb", "set", "reset", "1");
		eval("swconfig", "dev", "rtl8366rb", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "rtl8366rb", "vlan", "1", "set", "ports", "1 2 3 4 5t");
		eval("swconfig", "dev", "rtl8366rb", "vlan", "2", "set", "ports", "0 5t");
		eval("swconfig", "dev", "rtl8366rb", "set", "apply");
		unsigned char buf2[256];
		fseek(fp, 0x1fc00, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		fprintf(stderr, "configure vlan1 to %s\n", mac);
		set_hwaddr("vlan1", mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure vlan2 to %s\n", mac);
		set_hwaddr("vlan2", mac);
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
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
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
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "0");
		eval("vconfig", "add", "eth0", "1");
		fprintf(stderr, "configure vlan0 to %s\n", mac);
		MAC_SUB(mac);
		set_hwaddr("vlan0", mac);
		MAC_ADD(mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure vlan1 to %s\n", mac);
		set_hwaddr("vlan1", mac);
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

		if (buf2[0] == 0xff) {
			fseek(fp, secondoffset, SEEK_SET);
			fread(buf2, 19, 1, fp);
		}

		fclose(fp);
		fprintf(stderr, "configure eth0 to %s\n", buf2);
		set_hwaddr("eth0", buf2);
		MAC_ADD(buf2);
		fprintf(stderr, "configure eth1 to %s\n", buf2);
		set_hwaddr("eth1", buf2);
	}
#endif
#ifdef HAVE_TG2521
	set_hwaddr("eth0", "00:11:22:33:44:55");
	set_hwaddr("eth1", "00:11:22:33:44:66");
	FILE *fp = fopen("/dev/mtdblock/7", "rb");
	char mac[32];
	if (fp) {
		unsigned char buf2[256];
		fseek(fp, 0x7d08c3, SEEK_SET); // mac location
		fread(buf2, 6, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 6; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		set_hwaddr("eth1", mac);
		MAC_SUB(mac);
	}
#endif
#if defined(HAVE_TEW632BRP) || defined(HAVE_DIR615E)
	set_hwaddr("eth0", "00:11:22:33:44:55");
	set_hwaddr("eth1", "00:11:22:33:44:66");
	FILE *in = fopen("/dev/mtdblock/0", "rb");
	char *lanmac = NULL;
	if (in != NULL) {
		fseek(in, 0x20000, SEEK_SET);
		unsigned char *config = calloc(65536, 1);
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
				set_hwaddr("eth0", mac);
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
				set_hwaddr("eth1", mac);
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
	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
#ifdef HAVE_WRT160NL
	MAC_ADD(buf2);
	fprintf(stderr, "configure wifi0 to %s\n", buf2);
	set_hwaddr("wifi0", buf2);
	led_control(LED_POWER, LED_ON);
#endif
#if defined(HAVE_TEW632BRP) || defined(HAVE_DIR615E)
	if (lanmac != NULL) {
		fprintf(stderr, "configure wifi0 to %s\n", lanmac);
		set_hwaddr("wifi0", lanmac);
		free(lanmac);
	}
#endif
#ifdef HAVE_TG2521
	{
		fprintf(stderr, "configure wifi0 to %s\n", mac);
		set_hwaddr("wifi0", mac);
	}
//      eval("gpio", "disable", "5");   // enable usb port
#endif
#ifdef HAVE_WR1043
	{
		fprintf(stderr, "configure wifi0 to %s\n", mac);
		set_hwaddr("wifi0", mac);
	}
#endif
#ifdef HAVE_WR941
	{
		fprintf(stderr, "configure wifi0 to %s\n", mac);
		set_hwaddr("wifi0", mac);
	}
#endif

#ifdef HAVE_RS
#elif HAVE_WRT160NL
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4 5");
	eval("swconfig", "dev", "eth0", "set", "apply");
#endif

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

	return;
	cprintf("done\n");
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
	if (!detect_wireless_devices(RADIO_ALL)) {
#ifdef HAVE_RS
		setWirelessLed(0, 2);
		setWirelessLed(1, 2);
		setWirelessLed(2, 2);
#elif HAVE_WRT160NL
		setWirelessLed(0, 6);
		writeprocsys("dev/wifi0/ledpin", "6");
		writeprocsys("dev/wifi0/softled", "1");
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
	}
}