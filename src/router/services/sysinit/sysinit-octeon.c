/*
 * sysinit-octeon.c
 *
 * Copyright (C) 2014 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");
	insmod("lzo_compress");
	insmod("lzo_decompress");
	insmod("xxhash");
	insmod("zstd_common");
	insmod("zstd_compress");
	insmod("zstd_decompress");
	insmod("raid6_pq");
	insmod("xor");
	insmod("btrfs");
	FILE *check = fopen("/dev/sda3", "rb");
	char drive[64];
	if (check) {
		fclose(check);
		sprintf(drive, "/dev/sda3");
	} else {
		sprintf(drive, "/dev/mmcblk0p3");
	}
	if (mount(drive, "/jffs", "btrfs", MS_MGC_VAL | MS_NOATIME, "compress=zstd")) {
		eval("mkfs.btrfs", drive, "-f");
		mount(drive, "/jffs", "btrfs", MS_MGC_VAL | MS_NOATIME, "compress=zstd");
	}
	eval("mount", "--bind", "/jffs", "/usr/local");
	nvram_seti("enable_jffs2", 1);

	int brand = getRouterBrand();

	insmod("ocf");
	insmod("cryptodev");
	insmod("cryptocteon");
	insmod("hwmon");
	insmod("tmp421");
	/*
	 * network drivers 
	 */
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
	eval("ifconfig", "eth2", "up");
	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}

	detect_wireless_devices(RADIO_ALL);

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

	eval("hwclock", "-s", "-u");

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