/*
 * sysinit-wrt400.c
 *
 * Copyright (C) 2006 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	/*
	 * network drivers 
	 */
	fprintf(stderr, "load ag71xx or ag7100_mod Ethernet Driver\n");
	system("insmod ag71xx || insmod ag7100_mod || insmod ag7240_mod");
	char mac1[32];
	char mac2[32];
	char wmac[32];
	FILE *fp = fopen("/dev/mtdblock/7", "rb");
	if (fp) {
		char mactmp[6];
		int copy[6];
		int i;
#ifdef HAVE_WNDR3700
		eval("swconfig", "dev", "rtl8366s", "set", "reset", "1");
		eval("swconfig", "dev", "rtl8366s", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "rtl8366s", "set", "blinkrate", "2");
		eval("swconfig", "dev", "rtl8366s", "port", "1", "set", "led", "9");
		eval("swconfig", "dev", "rtl8366s", "port", "2", "set", "led", "6");
		eval("swconfig", "dev", "rtl8366s", "port", "5", "set", "led", "2");
		eval("swconfig", "dev", "rtl8366s", "set", "apply");

#ifdef HAVE_WNDR3700V2
		fseek(fp, 0xff0000, SEEK_SET);
#else
		fseek(fp, 0x7f0000, SEEK_SET);
#endif
		fread(mactmp, 6, 1, fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fread(mactmp, 6, 1, fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(wmac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
#elif HAVE_WZRHPAG300NH
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");
		eval("swconfig", "dev", "eth0", "set", "apply");

		nvram_seti("sw_cpuport", 0);
		nvram_seti("sw_wan", -1);
		nvram_seti("sw_lan1", 1);
		nvram_seti("sw_lan2", 2);
		nvram_seti("sw_lan3", 3);
		nvram_seti("sw_lan4", 4);
		nvram_default_get("port5vlans", "1 18000 19000 20000");
		fseek(fp, 0x5120C, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		MAC_ADD(mac2);
//              eval("gpio","enable","2");
#elif HAVE_WZRG300NH2
#ifndef HAVE_WZR300HP
		eval("startservice", "bootloader_check");
#endif
		fseek(fp, 0x5120C, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
//              eval("gpio","enable","13");
#ifdef HAVE_SWCONFIG
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 1 3 4 5");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 2");
		eval("swconfig", "dev", "eth0", "set", "apply");
		nvram_seti("sw_cpuport", 0);
		nvram_seti("sw_wan", 2);
		nvram_seti("sw_lan1", 1);
		nvram_seti("sw_lan2", 3);
		nvram_seti("sw_lan3", 4);
		nvram_seti("sw_lan4", 5);

#endif

		fprintf(stderr, "configure eth0 to %s\n", mac2);
		set_hwaddr("eth0", mac2);
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		fprintf(stderr, "configure vlan1 to %s\n", mac2);
		set_hwaddr("vlan1", mac2);
		fprintf(stderr, "configure vlan2 to %s\n", mac2);
		set_hwaddr("vlan2", mac2);
#elif HAVE_WZRG450
		fseek(fp, 0x51002, SEEK_SET); //osprey eeprom mac location
		fread(mactmp, 6, 1, fp);
		if (!memcmp(mactmp, "\xff\xff\xff\xff\xff\xff", 6)) {
			fseek(fp, 0x50000,
			      SEEK_SET); //osprey eeprom mac location
			fread(mactmp, 6, 1, fp);
		}
		fclose(fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
//              mac1[0] |= 0x02; // add private bit
//              mac2[0] |= 0x02;
//              eval("gpio","disable","16");
#ifdef HAVE_SWCONFIG
		system("swconfig dev switch0 set reset 1");
		system("swconfig dev switch0 set enable_vlan 1");
		system("swconfig dev switch0 vlan 1 set ports \"0t 2 3 4 5\"");
		system("swconfig dev switch0 vlan 2 set ports \"0t 1\"");
		system("swconfig dev switch0 set apply");
		nvram_seti("sw_cpuport", 0);
		nvram_seti("sw_wan", 1);
		nvram_seti("sw_lan1", 2);
		nvram_seti("sw_lan2", 3);
		nvram_seti("sw_lan3", 4);
		nvram_seti("sw_lan4", 5);
#endif

		fprintf(stderr, "configure eth0 to %s\n", mac2);
		set_hwaddr("eth0", mac2);
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		fprintf(stderr, "configure vlan1 to %s\n", mac2);
		set_hwaddr("vlan1", mac2);
		fprintf(stderr, "configure vlan2 to %s\n", mac2);
		set_hwaddr("vlan2", mac2);
#else
		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");
		eval("swconfig", "dev", "eth0", "set", "apply");
		fseek(fp, 0x7f120c, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 5; i >= 3; i--)
			if (++mactmp[i] != 0x00)
				break; // dont know what this is
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		MAC_ADD(mac2);
#endif

	} else {
		sprintf(mac1, "00:11:22:33:44:55");
		sprintf(mac2, "00:11:22:33:44:66");
	}
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "1 2 16000");
#ifndef HAVE_WZRG450
	set_hwaddr("eth0", mac1);
	eval("ifconfig", "eth0", "up");
	set_hwaddr("eth1", mac2);
	eval("ifconfig", "eth1", "up");
#else
	set_hwaddr("eth0", mac2);
	eval("ifconfig", "eth0", "up");
#endif
	struct ifreq ifr;
	int s;

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
	detect_wireless_devices(RADIO_ALL);
#ifdef HAVE_WZRHPAG300NH
	//      set_hwaddr("wifi1", wmac);
	setWirelessLedPhy0(1);
	setWirelessLedPhy1(5);

#else
#ifndef HAVE_WNDR3700

#ifdef HAVE_WZRG300NH2
	setWirelessLedPhy0(5);
#else
#ifndef HAVE_WZRG450
	setWirelessLedGeneric(0, 6);
	setWirelessLedGeneric(1, 6);
#endif
#endif
#else
	set_hwaddr("wifi0", mac1);
	set_hwaddr("wifi1", wmac);
	setWirelessLedPhy0(5);
	setWirelessLedPhy1(5);
#endif
#endif

	getRouterBrand(); // restore some default settings

	if (!nvram_exists("wlan0_rxantenna"))
		nvram_seti("wlan0_rxantenna", 3);
	if (!nvram_exists("wlan0_txantenna"))
		nvram_seti("wlan0_txantenna", 3);
	if (!nvram_exists("wlan1_rxantenna"))
		nvram_seti("wlan1_rxantenna", 3);
	if (!nvram_exists("wlan1_txantenna"))
		nvram_seti("wlan1_txantenna", 3);

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
