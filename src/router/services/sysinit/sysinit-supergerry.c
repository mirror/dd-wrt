/*
 * sysinit-supergerry.c
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

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	eval("/bin/tar", "-xzf", "/dev/mtdblock4", "-C", "/");
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	/*
	 * test -e /lib/modules/$KERNELVER/extra/profdrvdd.ko && insmod
	 * /lib/modules/$KERNELVER/extra/profdrvdd.ko test -e
	 * /lib/modules/$KERNELVER/extra/atmapi.ko && insmod
	 * /lib/modules/$KERNELVER/extra/atmapi.ko test -e
	 * /lib/modules/$KERNELVER/extra/atmbonding.ko && insmod
	 * /lib/modules/$KERNELVER/extra/atmbonding.ko test -e
	 * /lib/modules/$KERNELVER/extra/atmbondingeth.ko && insmod
	 * /lib/modules/$KERNELVER/extra/atmbondingeth.ko test -e
	 * /lib/modules/$KERNELVER/extra/adsldd.ko && insmod
	 * /lib/modules/$KERNELVER/extra/adsldd.ko test -e
	 * /lib/modules/$KERNELVER/extra/blaa_dd.ko && insmod
	 * /lib/modules/$KERNELVER/extra/blaa_dd.ko test -e
	 * /lib/modules/$KERNELVER/kernel/net/atm/br2684.ko && insmod
	 * /lib/modules/$KERNELVER/kernel/net/atm/br2684.ko test -e
	 * /lib/modules/$KERNELVER/extra/bcmprocfs.ko && insmod
	 * /lib/modules/$KERNELVER/extra/bcmprocfs.ko test -e
	 * /lib/modules/$KERNELVER/extra/bcm_enet.ko && insmod
	 * /lib/modules/$KERNELVER/extra/bcm_enet.ko test -e
	 * /lib/modules/$KERNELVER/extra/bcm_usb.ko && insmod
	 * /lib/modules/$KERNELVER/extra/bcm_usb.ko test -e
	 * /lib/modules/$KERNELVER/extra/wl.ko && insmod
	 * /lib/modules/$KERNELVER/extra/wl.ko test -e
	 * /lib/modules/$KERNELVER/extra/dspdd.ko && insmod
	 * /lib/modules/$KERNELVER/extra/dspdd.ko test -e
	 * /lib/modules/$KERNELVER/extra/endpointdd.ko && insmod
	 * /lib/modules/$KERNELVER/extra/endpointdd.ko 
	 */
	insmod("atmapi");
	insmod("adsldd");
	insmod("blaa_dd");
	insmod("bcmprocfs");
	insmod("bcm_enet");

	/*
	 * network drivers 
	 */
	detect_wireless_devices(RADIO_ALL);

	/*
	 * Set a sane date 
	 */
	stime(&tm);

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