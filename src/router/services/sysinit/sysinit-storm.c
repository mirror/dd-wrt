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

#include <ddnvram.h>
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

	eval("mknod", "/dev/gpio", "c", "127", "0");

	/*
	 * Setup console 
	 */

	klogctl(8, NULL, nvram_geti("console_loglevel"));

	int brand = getRouterBrand();

	/*
	 * network drivers 
	 */
	insmod("gemini_negmac");
#ifdef HAVE_WBD222
	insmod("libata");
	insmod("pata_gemini");
#endif
	eval("ifconfig", "eth0", "up");
#ifdef HAVE_WBD222
	eval("ifconfig", "eth1", "up");
#endif
	struct ifreq ifr;
	int s;

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
	//disable led's
	set_gpio(1, 0);
	set_gpio(2, 0);
	set_gpio(3, 0);
	set_gpio(5, 0);

	eval("hwclock", "-s", "-u");

	return;
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

void sys_overclocking(void)
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
void start_arch_defaults(void)
{
}
