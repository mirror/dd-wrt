/*
 * sysinit-pb42.c
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
	fprintf(stderr, "load ATH Ethernet Driver\n");
	system("insmod ag71xx || insmod ag7100_mod");

	// sleep(1);
	FILE *fp = fopen("/dev/mtdblock/7", "rb");
	unsigned char buf2[256];
	fseek(fp, 0, SEEK_END);
	int totalsize = ftell(fp);
	rewind(fp);
	fseek(fp, totalsize - 0x1000, SEEK_SET);
	fread(buf2, 256, 1, fp);
	fclose(fp);
	int i;
	int offsetmac1 = -1;
	int offsetmac2 = -1;
	unsigned int copy[256];

	for (i = 0; i < 256; i++)
		copy[i] = buf2[i] & 0xff;
	for (i = 0; i < 256 - 12; i++) {
		if (!strncmp(&buf2[i], "ar7100_esa", 10)) {
			offsetmac1 = i + 11;
		}
		if (!strncmp(&buf2[i], "ar7100_esa_2", 12)) {
			offsetmac2 = i + 13;
		}
	}
	if (offsetmac1 != -1) {
		char mac[32];

		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0 + offsetmac1], copy[1 + offsetmac1], copy[2 + offsetmac1],
			copy[3 + offsetmac1], copy[4 + offsetmac1], copy[5 + offsetmac1]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		/*
		 * Right now the routerstation has no on-board mac stored for the secondary interface (LAN)
		 * to solve this i advised UBNT to use "ar7100_esa_2" as future redboot parameter for the secondary mac address
		 * if this parameter exists, we will take it as secondary mac, if not we increase the primary mac by 1
		 * consider that if you feel you have problems with mac conflicts using multiple routerstations, add this secondary parameter to redboot
		 * this should only happen, if you connect the wan port of a routerstation to the lan port of another one and if these routerstation are 
		 * from the same production batch 
		 * 
		 */
		if (offsetmac2 != -1) {
			sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0 + offsetmac2], copy[1 + offsetmac2],
				copy[2 + offsetmac2], copy[3 + offsetmac2], copy[4 + offsetmac2], copy[5 + offsetmac2]);
		} else {
			MAC_ADD(mac);
		}
		fprintf(stderr, "configure eth1 to %s\n", mac);
		set_hwaddr("eth1", mac);
	} else {
		// no mac found, use default
		set_hwaddr("eth0", "00:15:6D:FE:00:00");
		set_hwaddr("eth1", "00:15:6D:FE:00:01");
	}

	fp = fopen("/dev/mtdblock/0", "r");
	if (fp) {
		fseek(fp, 0x1f800, SEEK_SET);
		unsigned int signature;

		fread(&signature, 4, 1, fp);
		if (signature == 0x20021103) {
			fprintf(stderr, "Compex WP543 detected\n");
			eval("ifconfig", "eth0", "0.0.0.0", "down");
			eval("ifconfig", "eth1", "0.0.0.0", "down");
			unsigned char buf[20];

			fseek(fp, 0x1f810, SEEK_SET);
			fread(&buf[0], 6, 1, fp);
			char mac[20];
			int i;

			unsigned int copy[20];

			for (i = 0; i < 12; i++)
				copy[i] = buf[i] & 0xff;

			sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
			fprintf(stderr, "configure ETH0 to %s\n", mac);
			nvram_set("et0macaddr_safe", mac);
			set_hwaddr("eth0", mac);
			fseek(fp, 0x1f818, SEEK_SET);
			fread(&buf[6], 6, 1, fp);
			for (i = 0; i < 12; i++)
				copy[i] = buf[i] & 0xff;
			sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[6], copy[7], copy[8], copy[9], copy[10], copy[11]);
			fprintf(stderr, "configure ETH1 to %s\n", mac);
			set_hwaddr("eth1", mac);
		}
		fclose(fp);
	}
	//#endif
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}

#ifdef HAVE_WP546
	led_control(BEEPER, LED_ON);
	usleep(1000);
	led_control(BEEPER, LED_OFF);
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
void load_wifi_drivers(void)
{
	detect_wireless_devices(RADIO_ALL);

#ifdef HAVE_RS
	setWirelessLed(0, 2);
	setWirelessLed(1, 2);
	setWirelessLed(2, 2);
#elif HAVE_WP546
	setWirelessLed(0, 5);
	setWirelessLed(1, 4);
#elif HAVE_WP543
	setWirelessLed(0, 5);
#else
	setWirelessLed(0, 2);
#endif
}