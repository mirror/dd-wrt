/*
 * sysinit-jjap005.c
 *
 * Copyright (C) 2013 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
	system("insmod ag71xx || insmod ag7240_mod");
	FILE *fp = fopen("/dev/mtdblock/6", "rb");
	if (fp) {
		// fseek(fp, 0xFF0000, SEEK_SET);
		fseek(fp, DDMACOFFSET, SEEK_SET);
		unsigned char buf[20];
		fread(&buf[0], 6, 1, fp);
		char mac[20];
		int i;
		unsigned int copy[20];
		for (i = 0; i < 12; i++)
			copy[i] = buf[i] & 0xff;
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure ETH0 to %s\n", mac);
		nvram_set("et0macaddr_safe", mac);
		nvram_set("et0macaddr", mac);
		set_hwaddr("eth0", mac);
		fread(&buf[6], 6, 1, fp);
		for (i = 0; i < 12; i++)
			copy[i] = buf[i] & 0xff;
		sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", copy[6], copy[7], copy[8], copy[9], copy[10], copy[11]);
		fprintf(stderr, "configure ETH1 to %s\n", mac);
		set_hwaddr("eth1", mac);

		fclose(fp);
	}
	// eval("ifconfig", "eth0", "up");
	// eval("ifconfig", "eth1", "up");
	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}

	detect_wireless_devices(RADIO_ALL);

#if defined(HAVE_ONNET) || defined(HAVE_RAYTRONIK)
	runStartup(".onnet");
#endif

#ifdef HAVE_ALFANX
	eval("/sbin/wlanled", "-L", "generic_12:-94", "-L", "generic_8:-80", "-L", "generic_6:-73", "-L", "generic_7:-65");
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
