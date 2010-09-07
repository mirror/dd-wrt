/*
 * sysinit-wrt400.c
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
	insmod("ag7100_mod");
	char mac1[32];
	char mac2[32];
	char wmac[32];
	FILE *fp = fopen("/dev/mtdblock/7", "rb");
	if (fp) {
		char mactmp[6];
		int copy[6];
		int i;
#ifdef HAVE_WNDR3700
		fseek(fp, 0x7f0000, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
		fread(mactmp, 6, 1, fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(wmac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
#elif HAVE_WZRHPAG300NH
		fseek(fp, 0x5120C, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 5; i >= 3; i--)
			if (++mactmp[i] != 0x00)
				break;	// dont know what this is 
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
		MAC_ADD(mac2);


#else
		fseek(fp, 0x7f120c, SEEK_SET);
		fread(mactmp, 6, 1, fp);
		fclose(fp);
		for (i = 5; i >= 3; i--)
			if (++mactmp[i] != 0x00)
				break;	// dont know what this is 
		for (i = 0; i < 6; i++)
			copy[i] = mactmp[i];
		for (i = 0; i < 6; i++)
			copy[i] &= 0xff;
		sprintf(mac1, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
		sprintf(mac2, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0],
			copy[1], copy[2], copy[3], copy[4], copy[5]);
		MAC_ADD(mac2);
#endif

	} else {
		sprintf(mac1, "00:11:22:33:44:55");
		sprintf(mac2, "00:11:22:33:44:66");
	}
	eval("ifconfig", "eth0", "hw", "ether", mac1);
	eval("ifconfig", "eth1", "hw", "ether", mac2);

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
#ifdef HAVE_WZRHPAG300NH
//	eval("ifconfig", "wifi1", "hw", "ether", wmac);
	system2("echo 5 >/proc/sys/dev/wifi0/ledpin");
	system2("echo 1 >/proc/sys/dev/wifi0/softled");
	system2("echo 1 >/proc/sys/dev/wifi1/ledpin");
	system2("echo 1 >/proc/sys/dev/wifi1/softled");

#else


#ifndef HAVE_WNDR3700
	system2("echo 6 >/proc/sys/dev/wifi0/ledpin");
	system2("echo 1 >/proc/sys/dev/wifi0/softled");
	system2("echo 6 >/proc/sys/dev/wifi1/ledpin");
	system2("echo 1 >/proc/sys/dev/wifi1/softled");
#else
	eval("ifconfig", "wifi1", "hw", "ether", wmac);
	system2("echo 5 >/proc/sys/dev/wifi0/ledpin");
	system2("echo 1 >/proc/sys/dev/wifi0/softled");
	system2("echo 5 >/proc/sys/dev/wifi1/ledpin");
	system2("echo 1 >/proc/sys/dev/wifi1/softled");
#endif
#endif

	led_control(LED_POWER, LED_ON);
	led_control(LED_SES, LED_OFF);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN, LED_OFF);
	led_control(LED_CONNECTED, LED_OFF);

	if (!nvram_get("ath0_rxantenna"))
		nvram_set("ath0_rxantenna", "3");
	if (!nvram_get("ath0_txantenna"))
		nvram_set("ath0_txantenna", "3");
	if (!nvram_get("ath1_rxantenna"))
		nvram_set("ath1_rxantenna", "3");
	if (!nvram_get("ath1_txantenna"))
		nvram_set("ath1_txantenna", "3");

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
