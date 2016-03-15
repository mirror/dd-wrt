/*
 * sysinit-ubntm.c
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

#ifdef HAVE_WPE72
	FILE *fp = fopen("/dev/mtdblock/6", "rb");
#else
	FILE *fp = fopen("/dev/mtdblock/5", "rb");
#endif
	char mac[32];
	unsigned int copy[256];
	if (fp) {
		unsigned char buf2[256];
#ifdef HAVE_WPE72
		fseek(fp, 0x1f810, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		if ((!memcmp(buf2, "\xff\xff\xff\xff\xff\xff", 6)
		     || !memcmp(buf2, "\x00\x00\x00\x00\x00\x00", 6)))
			goto out;
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		eval("ifconfig", "eth1", "hw", "ether", mac);
#else
		fread(buf2, 256, 1, fp);
		fclose(fp);
		if ((!memcmp(buf2, "\xff\xff\xff\xff\xff\xff", 6)
		     || !memcmp(buf2, "\x00\x00\x00\x00\x00\x00", 6)))
			goto out;
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		eval("ifconfig", "eth1", "hw", "ether", mac);
#endif
	}
      out:;

	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr", ether_etoa((char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr_safe", ether_etoa((char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}
#ifdef HAVE_SWCONFIG
#ifdef HAVE_DAP3310
	eval("swconfig", "dev", "eth1", "set", "reset", "1");
	eval("swconfig", "dev", "eth1", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth1", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth1", "set", "apply");
	setEthLED(18, "eth0");
	setEthLED(19, "eth1");
#elif HAVE_DAP3410
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 3");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 4");
	eval("swconfig", "dev", "eth0", "set", "apply");
	eval("ifconfig", "eth0", "up");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");
#else
	eval("swconfig", "dev", "eth1", "set", "reset", "1");
	eval("swconfig", "dev", "eth1", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth1", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth1", "set", "apply");
#endif
#endif
	int brand = getRouterBrand();
	if (brand == ROUTER_UBNT_UAPAC) {
		fp = fopen("/dev/mtdblock/5", "rb");
		FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
		if (fp) {
			fseek(fp, 0x5000, SEEK_SET);
			int i;
			for (i = 0; i < 2116; i++)
				putc(getc(fp), out);
			fclose(fp);
			fclose(out);
			eval("rm", "-f", "/tmp/ath10k-board.bin");
			eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
		}
	}

	detect_wireless_devices();

#ifdef HAVE_WPE72
	if (!nvram_match("wlanled", "0"))
		eval("/sbin/wlanled", "-l", "generic_14:-94", "-l", "generic_15:-80", "-l", "generic_16:-73", "-l", "generic_17:-65");
#elif HAVE_DAP3310
	set_gpio(14, 1);
	set_gpio(13, 1);
	set_gpio(20, 1);
	if (!nvram_match("wlanled", "0"))
		eval("/sbin/wlanled", "-L", "generic_14:-94", "-l", "generic_13:-76", "-L", "generic_20:-65");
#elif HAVE_DAP3410
	set_gpio(14, 1);
	set_gpio(15, 1);
	set_gpio(16, 1);
	if (!nvram_match("wlanled", "0"))
		eval("/sbin/wlanled", "-L", "generic_14:-94", "-L", "generic_15:-76", "-L", "generic_16:-65");
#elif HAVE_UBNTXW
	if (brand == ROUTER_UBNT_UAPAC) {
		setWirelessLed(0, 7);
		setWirelessLed(1, 8);
	} else {
		writeproc("/proc/sys/dev/wifi0/softled", "0");
		if (!nvram_match("wlanled", "0"))
			eval("/sbin/wlanled", "-L", "generic_11:-94", "-L", "generic_16:-80", "-l", "generic_13:-73", "-L", "generic_14:-65");
	}
#else
	switch (brand) {
	case ROUTER_BOARD_UNIFI:
		setWirelessLed(0, 0);
		break;
	case ROUTER_UBNT_UAPAC:
		setWirelessLed(0, 7);
		setWirelessLed(1, 8);
		break;
	case ROUTER_BOARD_AIRROUTER:
		break;
	default:
		writeproc("/proc/sys/dev/wifi0/softled", "0");
		if (!nvram_match("wlanled", "0"))
			eval("/sbin/wlanled", "-l", "generic_0:-94", "-l", "generic_1:-80", "-l", "generic_11:-73", "-l", "generic_7:-65");

	}
#endif
	/* ubnt has a hardware fault as it seems, so the power bridge feature can break the hardware which causes endless reboot loops. we keep it disabled here. devices which are already broken will work again then */
	if (nvram_match("ubnt_power", "1"))
		set_gpio(8, 1);

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
