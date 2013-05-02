/*
 * sysinit-wrt300nv2.c
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
 * 
 * System Initialisation for Avila Gateworks and compatible Routers
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

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <cymac.h>
#include "devices/wireless.c"

void start_sysinit(void)
{
	struct stat tmp_stat;
	time_t tm = 0;

	eval("mknod", "/dev/gpio", "c", "127", "0");
	eval("mknod", "/dev/rtc", "c", "254", "0");
	eval("mknod", "/dev/crypto", "c", "10", "70");
	eval("mount", "-o", "remount,rw", "/");

	if (!nvram_match("disable_watchdog", "1"))
		eval("watchdog");	// system watchdog

	cprintf("sysinit() setup console\n");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	nvram_set("intel_eth", "0");

#if 1
	insmod("ixp400th");
	insmod("ixp400");
	system2("cat /usr/lib/firmware/IxNpeMicrocode.dat > /dev/IxNpe");
	insmod("ixp400_eth");
	eval("ifconfig", "ixp0", "0.0.0.0", "up");
	eval("ifconfig", "ixp1", "0.0.0.0", "up");
	insmod("ocf");
	insmod("cryptodev");
	// insmod("ixp4xx", "init_crypto=0");
#else
	// eval ("mknod", "/dev/IxNpe","c","10","184");
	system2("cat /usr/lib/firmware/NPE-B > /dev/misc/ixp4xx_ucode");
	system2("cat /usr/lib/firmware/NPE-C > /dev/misc/ixp4xx_ucode");
#endif

	detect_wireless_devices();
	/*
	 * Configure mac addresses by reading data from eeprom 
	 */
	// char *filename =
	// "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom"; /*
	// bank2=0x100 */
	char *filename = "/dev/mtdblock/0";	/* bank2=0x100 */
	FILE *file = fopen(filename, "r");

	if (file) {
		unsigned char buf[20];

		fseek(file, 0x5ffa0, SEEK_SET);	// point of mac address
		fread(&buf[0], 6, 1, file);
		char mac[20];

		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5]);
		fprintf(stderr, "configure primary mac %s\n", mac);
		eval("ifconfig", "ixp0", "hw", "ether", mac);
		eval("ifconfig", "wifi0", "hw", "ether", mac);
		nvram_set("et0macaddr", mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure secondary mac %s\n", mac);
		eval("ifconfig", "ixp1", "hw", "ether", mac);

		fclose(file);
	}
	eval("ifconfig", "ixp0", "0.0.0.0", "up");
	eval("ifconfig", "ixp1", "0.0.0.0", "up");

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");

	nvram_set("use_crypto", "0");
	cprintf("done\n");
	return;
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

void enable_dtag_vlan(int enable)
{

}
