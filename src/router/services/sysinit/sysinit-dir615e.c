/*
 * sysinit-dir615e.c
 *
 * Copyright (C) 2009 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include "devices/ethtools.c"

void start_sysinit(void)
{
	time_t tm = 0;

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	/*
	 * network drivers 
	 */
	fprintf(stderr, "load ATH Ethernet Driver\n");
	system("insmod ag71xx || insmod ag7240_mod");
//#ifndef HAVE_DIR632
#ifndef HAVE_WR841V8
	set_hwaddr("eth0", "00:11:22:33:44:55");
	set_hwaddr("eth1", "00:11:22:33:44:66");
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
#ifdef HAVE_DIR632
				set_hwaddr("eth0", mac);
#else
				set_hwaddr("eth1", mac);
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
				set_hwaddr("eth1", mac);
#else
				set_hwaddr("eth0", mac);
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
#ifndef HAVE_WA901V5
	eval("swconfig", "dev", "eth1", "set", "reset", "1");
	eval("swconfig", "dev", "eth1", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth1", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth1", "set", "apply");

	nvram_set("switchphy", "eth1");
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", -1);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "1 18000 19000 20000");
#endif

#ifndef HAVE_DAP2230
#ifndef HAVE_DIR615I
#ifndef HAVE_DIR632
	setEthLED(17, "eth0");
	setSwitchLED(13, 0x2);
	setSwitchLED(14, 0x4);
	setSwitchLED(15, 0x8);
	setSwitchLED(16, 0x10);
#endif
#endif
#ifdef HAVE_WA901V5
	setEthLED(7, "eth0");
#elif HAVE_WR841HPV3
	setEthLED(12, "eth1");
	setEthLED(14, "eth0");
#elif HAVE_WR940V4
	setEthLED(14, "eth1");
	setSwitchLED(4, 0x2);
	setSwitchLED(18, 0x4);
	setSwitchLED(6, 0x8);
	setSwitchLED(8, 0x10);
#elif HAVE_WR941V6
	setEthLED(14, "eth1");
	setSwitchLED(7, 0x2);
	setSwitchLED(6, 0x4);
	setSwitchLED(5, 0x8);
	setSwitchLED(4, 0x10);
#elif HAVE_WR841V9
	setEthLED(4, "eth1");
	setSwitchLED(11, 0x2);
	setSwitchLED(14, 0x4);
	setSwitchLED(15, 0x8);
	setSwitchLED(16, 0x10);
#elif HAVE_WA860RE
	setEthLED(20, "eth0");
#elif HAVE_WA850RE
	setEthLED(20, "eth0");
#elif HAVE_WR841V8
	setEthLED(18, "eth0");
	setSwitchLED(19, 0x4);
	setSwitchLED(20, 0x8);
	setSwitchLED(21, 0x10);
	setSwitchLED(12, 0x02);
#endif
#endif
#endif
	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
	detect_wireless_devices(RADIO_ALL);
#ifndef HAVE_ATH9K
	if (lanmac != NULL) {
		fprintf(stderr, "configure wifi0 to %s\n", lanmac);
		set_hwaddr("wifi0", lanmac);
		free(lanmac);
	}
#endif
#ifndef HAVE_DAP3320
#ifdef HAVE_DAP2230
//      setWirelessLedGeneric(0, 11);
#elif HAVE_WR841HPV3
	setWirelessLedGeneric(15, 8);
#elif HAVE_WA901V5
	setWirelessLedGeneric(0, 8);
#elif HAVE_WR940V4
	setWirelessLedGeneric(0, 7);
#elif HAVE_WR941V6
	setWirelessLedGeneric(0, 8);
#elif HAVE_DIR632
	setWirelessLedPhy0(0);
#elif HAVE_WA860RE
	setWirelessLedGeneric(0, 2);
#elif HAVE_DIR615I
	setWirelessLedGeneric(0, 13);
#endif
#endif
#ifdef HAVE_WA850RE
	if (!nvram_matchi("wlanled", 0))
		eval("/sbin/wlanled", "-L", "generic_0:-94", "-L", "generic_1:-87", "-L", "generic_2:-80", "-L", "generic_3:-73",
		     "-L", "generic_4:-65");
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
}