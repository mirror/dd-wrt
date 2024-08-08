/*
 * sysinit-hornet.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
	insmod("ledtrig-netdev");
#if defined(HAVE_WR741V4) && !defined(HAVE_GL150)
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
		MAC_SUB(mac);
		set_hwaddr("eth0", mac);
		MAC_ADD(mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		set_hwaddr("eth1", mac);
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
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		if (copy[0] == copy[6] && copy[1] == copy[7] && copy[2] == copy[8]) {
			sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[6], copy[7], copy[8], copy[9], copy[10], copy[11]);
			if (copy[5] < copy[11]) {
				fprintf(stderr, "Using first mac for eth0 (%s)\n", mac);
				set_hwaddr("eth0", mac);
				fprintf(stderr, "Using second mac for eth1 (%s)\n", mac2);
				set_hwaddr("eth1", mac2);
			} else {
				fprintf(stderr, "Using second mac for eth0 (%s)\n", mac2);
				set_hwaddr("eth0", mac2);
				fprintf(stderr, "Using first mac for eth1 (%s)\n", mac);
				set_hwaddr("eth1", mac);
			}
		} else {
			fprintf(stderr, "configure eth0 to %s\n", mac);
			set_hwaddr("eth0", mac);
			MAC_ADD(mac);
			fprintf(stderr, "configure eth1 to %s\n", mac);
			set_hwaddr("eth1", mac);
		}
#ifndef HAVE_ATH9K
		MAC_SUB(mac);
#endif
	}
#endif
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
#ifdef HAVE_WR741V4
#ifdef HAVE_SWCONFIG
#ifndef HAVE_WR710
	eval("swconfig", "dev", "eth1", "set", "reset", "1");
	eval("swconfig", "dev", "eth1", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth1", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth1", "set", "apply");

	nvram_set("switchphy", "eth1");
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", -1);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);
#endif
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
#ifdef HAVE_WA701V2
	setEthLED(17, "eth1");
#endif
#endif
#ifdef HAVE_ERC
	eval("swconfig", "dev", "eth1", "set", "reset", "1");
	eval("swconfig", "dev", "eth1", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth1", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth1", "set", "apply");

	nvram_set("switchphy", "eth1");
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", -1);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);

	setSwitchLED(13, 0x02);
	setSwitchLED(14, 0x04);
	setSwitchLED(15, 0x08);
	setSwitchLED(16, 0x10);
	set_gpio(17, 1);
	set_gpio(13, 1);
	set_gpio(14, 1);
	set_gpio(15, 1);
	set_gpio(16, 1);
	setEthLED(17, "eth0");
#elif HAVE_GL150
	setEthLED(13, "eth0");
	setEthLED(15, "eth1");
#elif HAVE_CARAMBOLA
	eval("swconfig", "dev", "switch0", "set", "reset", "1");
	eval("swconfig", "dev", "switch0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "switch0", "vlan", "0", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "switch0", "set", "apply");
#ifdef HAVE_FMS2111
	eval("insmod", "i2c-gpio-custom", "bus0=0,23,22");
	eval("insmod", "rtc-pcf8523");
	writestr("/sys/class/i2c-dev/i2c-0/device/new_device", "pcf8523 0x68");
	eval("hwclock", "-s", "-u");

	eval("ledtool", "10", "0");

	setEthLED(21, "eth0");
	setEthLinkLED(14, "eth0");
	setEthLED(20, "eth1");
	setEthLinkLED(13, "eth1");
#endif //HAVE_FMS2111
#endif

#ifndef HAVE_ERC
#ifndef HAVE_GL150
#ifdef HAVE_HORNET
#ifdef HAVE_ONNET
	setEthLED(13, "eth0");
	setEthLED(17, "eth1");
#else
	setEthLED(17, "eth0");
	setEthLED(13, "eth1");
#endif
#endif
#endif
#endif

	detect_wireless_devices(RADIO_ALL);
#ifndef HAVE_ERC
	setWirelessLed(0, 0);
#endif
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

#ifdef HAVE_ERC
	load_drivers(1);
	if (nvram_matchi("radiooff_boot_off", 0)) {
		set_gpio(0, 1);
		set_gpio(1, 1);
	}
#endif
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "1 2 16000");

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
void load_wifi_drivers(void)
{
}