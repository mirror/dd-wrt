/*
 * sysinit-rt2880.c
 *
 * Copyright (C) 2008 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <linux/if_ether.h>
#include <linux/mii.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>
#include <linux/mii.h>

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <cymac.h>

#define sys_reboot() eval("sync"); eval("event","3","1","15")

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct utsname name;
	struct stat tmp_stat;
	time_t tm = 0;

	cprintf("sysinit() proc\n");
	/*
	 * /proc 
	 */
	mount("proc", "/proc", "proc", MS_MGC_VAL, NULL);
	mount("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
	cprintf("sysinit() tmp\n");

	/*
	 * /tmp 
	 */
	mount("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
	// fix for linux kernel 2.6
	mount("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
	eval("mkdir", "/tmp/www");
	eval("mknod", "/dev/nvram", "c", "229", "0");
	eval("mknod", "/dev/ppp", "c", "108", "0");
	eval("mknod", "-m", "0660", "/dev/mmc", "b", "126", "0");
	eval("mknod", "-m", "0660", "/dev/mmc0", "b", "126", "1");
	eval("mknod", "-m", "0660", "/dev/mmc1", "b", "126", "2");
	eval("mknod", "-m", "0660", "/dev/mmc2", "b", "126", "3");
	eval("mknod", "-m", "0660", "/dev/mmc3", "b", "126", "4");

/*    eval( "mkdir", "/dev/mtd" );
    eval( "mknod", "/dev/mtd/0", "c", "90", "0" );
    eval( "mknod", "/dev/mtd/0ro", "c", "90", "1" );
    eval( "mknod", "/dev/mtd/1", "c", "90", "2" );
    eval( "mknod", "/dev/mtd/1ro", "c", "90", "3" );
    eval( "mknod", "/dev/mtd/2", "c", "90", "4" );
    eval( "mknod", "/dev/mtd/2ro", "c", "90", "5" );
    eval( "mknod", "/dev/mtd/3", "c", "90", "6" );
    eval( "mknod", "/dev/mtd/3ro", "c", "90", "7" );
    eval( "mknod", "/dev/mtd/4", "c", "90", "8" );
    eval( "mknod", "/dev/mtd/4ro", "c", "90", "9" );
*/
	eval("mknod", "/dev/gpio", "c", "252", "0");

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
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	/*
	 * Modules 
	 */
	uname(&name);
	/*
	 * load some netfilter stuff 
	 */

//    eval( "watchdog" );
	/*
	 * Set a sane date 
	 */

	stime(&tm);
	nvram_set("wl0_ifname", "ra0");

	insmod("rt2860v2_ap");

	/* switch config */
	if (getRouterBrand() != ROUTER_BOARD_ECB9750)	// lets load
	{
		eval("ifconfig", "eth2", "up");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth2", "1");	//LAN 
		eval("vconfig", "add", "eth2", "2");	//WAN
		sysprintf("switch reg w 14 405555");
		sysprintf("switch reg w 50 2001");
		sysprintf("switch reg w 98 7f3f");
		sysprintf("switch reg w e4 3f");
#ifdef HAVE_ALLNET11N
		sysprintf("switch reg w 40 1002");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1001");
		sysprintf("switch reg w 70 ffff417e");
#else
		sysprintf("switch reg w 40 1001");
		sysprintf("switch reg w 44 1001");
		sysprintf("switch reg w 48 1002");
		sysprintf("switch reg w 70 ffff506f");
#endif
	}

/*

	switch reg w 14 405555
	switch reg w 50 2001
	switch reg w 98 7f3f
	if [ "$CONFIG_ESW_DOUBLE_VLAN_TAG" == "y" ]; then
		switch reg w e4 3f
	fi
	if [ "$1" = "LLLLW" ]; then
		switch reg w 40 1001
		switch reg w 44 1001
		switch reg w 48 1002
		switch reg w 70 ffff506f
	elif [ "$1" = "WLLLL" ]; then
		switch reg w 40 1002
		switch reg w 44 1001
		switch reg w 48 1001
		switch reg w 70 ffff417e
	elif [ "$1" = "GW" ]; then
		switch reg w 40 1001
		switch reg w 44 1001
		switch reg w 48 2001
		switch reg w 70 ffff605f
	fi


*/
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

char *enable_dtag_vlan(int enable)
{
	if (getRouterBrand() != ROUTER_BOARD_ECB9750) {
		if (enable) {
			sysprintf("switch reg w 14 405555");
			sysprintf("switch reg w 50 7001");
			sysprintf("switch reg w 98 7f2f");
			sysprintf("switch reg w e4 2f");
#ifdef HAVE_ALLNET11N
			sysprintf("switch reg w 40 1007");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#else
			sysprintf("switch reg w 40 1001");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1007");
			sysprintf("switch reg w 70 ffff506f");
#endif
			// now we got vlan7, how do we trunk now. lets find out
			return "eth2";
		} else {
			sysprintf("switch reg w 14 405555");
			sysprintf("switch reg w 50 2001");
			sysprintf("switch reg w 98 7f3f");
			sysprintf("switch reg w e4 3f");
#ifdef HAVE_ALLNET11N
			sysprintf("switch reg w 40 1002");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1001");
			sysprintf("switch reg w 70 ffff417e");
#else
			sysprintf("switch reg w 40 1001");
			sysprintf("switch reg w 44 1001");
			sysprintf("switch reg w 48 1002");
			sysprintf("switch reg w 70 ffff506f");
#endif
			eval("vconfig", "set_name_type",
			     "VLAN_PLUS_VID_NO_PAD");
			eval("vconfig", "add", "eth2", "2");	//WAN
			return "eth2";
		}
	} else {
		return "eth2";
	}
}
