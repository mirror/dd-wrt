/*
 * sysinit-rb532.c
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

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	unlink("/etc/nvram/.lock");
	cprintf("sysinit() proc\n");
	/*
	 * /proc 
	 */
	mount("proc", "/proc", "proc", MS_MGC_VAL, NULL);
	cprintf("sysinit() tmp\n");

	/*
	 * /tmp 
	 */
	mount("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
	// fix for linux kernel 2.6
	mount("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);

	// load ext2
	// eval("insmod","jbd");
	insmod("ext2");
#ifndef KERNEL_24
	if (mount("/dev/cf/card0/part3", "/usr/local", "ext2", MS_MGC_VAL | MS_SYNCHRONOUS, NULL))
#else
	if (mount("/dev/discs/disc0/part3", "/usr/local", "ext2", MS_MGC_VAL | MS_SYNCHRONOUS, NULL))
#endif
	{
		// not created yet, create ext2 partition
		eval("/sbin/mkfs.ext2", "-F", "-b", "1024", "/dev/cf/card0/part3");
		// mount ext2
		mount("/dev/cf/card0/part3", "/usr/local", "ext2", MS_MGC_VAL | MS_SYNCHRONOUS, NULL);
		eval("/bin/tar", "-xvvjf", "/etc/local.tar.bz2", "-C", "/");
	}
	eval("mkdir", "-p", "/usr/local/nvram");
	unlink("/tmp/nvram/.lock");
	eval("cp", "/etc/nvram/nvram.db", "/tmp/nvram");

	eval("mount", "/usr/local", "-o", "remount,ro");

	// eval ("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
	cprintf("sysinit() var\n");

	/*
	 * /var 
	 */
	mkdir("/tmp/var", 0777);
	mkdir("/var/lock", 0777);
	mkdir("/var/log", 0777);
	mkdir("/var/run", 0777);
	mkdir("/var/tmp", 0777);
	cprintf("sysinit() setup console\n");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	/*
	 * insmod("md5"); insmod("aes"); insmod("blowfish"); insmod("deflate");
	 * insmod("des"); insmod("michael_mic"); insmod("cast5");
	 * insmod("crypto_null"); 
	 */
	detect_wireless_devices(RADIO_ALL);

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_seti("use_crypto", 0);
	nvram_set("wl0_ifname", "wlan0");

	cprintf("done\n");
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
