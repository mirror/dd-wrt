/*
 * devinit.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>

void start_devinit(void)
{
	unlink("/etc/nvram/.lock");
	cprintf("sysinit() proc\n");

	/*
	 * /proc 
	 */
	mount("proc", "/proc", "proc", MS_MGC_VAL, NULL);
	mount("sysfs", "/sys", "sysfs", MS_MGC_VAL, NULL);
	mount("debugfs", "/sys/kernel/debug", "debugfs", MS_MGC_VAL, NULL);
	cprintf("sysinit() tmp\n");
	/*
	 * /tmp 
	 */
	mount("ramfs", "/tmp", "ramfs", MS_MGC_VAL, NULL);
#ifdef HAVE_HOTPLUG2
	// shell-skript. otherwise we loose our console
	system("echo >/proc/sys/kernel/hotplug");
	system("mount -t tmpfs -o size=512K none /dev");

	mknod("/dev/console", S_IFCHR | 0644, makedev(5, 1));
	mknod("/dev/null", S_IFCHR | 0644, makedev(1, 3));
	mkdir("/dev/pts", 0700);
#else
	// fix for linux kernel 2.6
	mknod("/dev/ppp", S_IFCHR | 0644, makedev(108, 0));
#endif
// fix me udevtrigger does not create that (yet) not registered?
	mknod("/dev/nvram", S_IFCHR | 0644, makedev(229, 0));
	mknod("/dev/watchdog", S_IFCHR | 0644, makedev(10, 130));

	mount("devpts", "/dev/pts", "devpts", MS_MGC_VAL, NULL);
	mount("devpts", "/proc/bus/usb", "usbfs", MS_MGC_VAL, NULL);
	mkdir("/tmp/www", 0700);

	unlink("/tmp/nvram/.lock");
	mkdir("/tmp/nvram", 0700);

	/*
	 * /var 
	 */
	mkdir("/tmp/var", 0777);
	mkdir("/tmp/services", 0777);
	mkdir("/tmp/eap_identities", 0777);
	mkdir("/var/lock", 0777);
	mkdir("/var/lock/subsys", 0777);
	mkdir("/var/log", 0777);
	mkdir("/var/run", 0777);
	mkdir("/var/tmp", 0777);
#ifdef HAVE_HOTPLUG2
	fprintf(stderr, "starting hotplug\n");
	system("/etc/hotplug2.startup");
#endif
#ifdef HAVE_OPENRISC
	mkdir("/dev/misc", 0700);
	mknod("/dev/misc/gpio", S_IFCHR | 0644, makedev(125, 0));
#endif
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) && !defined(HAVE_WDR4900)
	fprintf(stderr, "waiting for hotplug\n");
	char dev[64];
	char *disc = getdisc();

	if (disc == NULL) {
		fprintf(stderr, "no valid dd-wrt partition found, calling shell\n");
		eval("/bin/sh");
	}
	// sprintf (dev, "/dev/discs/disc%d/part1", index);
	// mount (dev, "/boot", "ext2", MS_MGC_VAL, NULL);
	if (strlen(disc) == 7)	//mmcblk0
		sprintf(dev, "/dev/%sp3", disc);
	else
		sprintf(dev, "/dev/%s3", disc);
	insmod("jbd2");
	insmod("mbcache");
	insmod("crc16");
	insmod("ext4");
	if (mount(dev, "/usr/local", "ext4", MS_MGC_VAL, NULL)) {
		eval("mkfs.ext4", "-F", "-b", " 1024", dev);
		mount(dev, "/usr/local", "ext4", MS_MGC_VAL, NULL);
	}
	mkdir("/usr/local", 0700);
	mkdir("/usr/local/nvram", 0700);
#endif
#ifdef HAVE_MSTP
	fprintf(stderr, "start MSTP Daemon\n");
	eval("/sbin/mstpd");
#endif
#if defined(HAVE_ATH10K)
	eval("rm", "-f", "/tmp/ath10k-board.bin");
	eval("ln", "-s", "/lib/ath10k/board.bin", "/tmp/ath10k-board.bin");
#if !defined(HAVE_X86) && !defined(HAVE_VENTANA) && !defined(HAVE_LAGUNA) && !defined(HAVE_LIMA) && !defined(HAVE_RAMBUTAN) && !defined(HAVE_NEWPORT)
	eval("ln", "-s", "/lib/ath10k/board_9984.bin", "/tmp/board1.bin");
#endif
#endif

#ifndef HAVE_OPENRISC
#if !defined(HAVE_VENTANA) || defined(HAVE_NEWPORT)
#ifndef HAVE_RAMBUTAN
#ifndef HAVE_WDR4900
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600)
	system("mount --bind /usr/local /jffs");
#elif HAVE_IPQ806X
	eval("mount", "-t", "ubifs", "-o", "sync", "ubi0:rootfs_data", "/jffs");
#else
#endif
#endif
#endif
#endif
#endif
	char *aqd = nvram_safe_get("svqos_aqd");
#ifdef HAVE_CODEL
	if (!strcmp(aqd, "codel")) {
		insmod("sch_codel");
		writeprocsysnet("core/default_qdisc", "codel");
	} else
#endif

#ifdef HAVE_FQ_CODEL
	if (!strcmp(aqd, "fq_codel")) {
		insmod("sch_fq_codel");
		writeprocsysnet("core/default_qdisc", "fq_codel");
	} else
#endif
#ifdef HAVE_FQ_CODEL_FAST
	if (!strcmp(aqd, "fq_codel_fast")) {
		insmod("sch_fq_codel_fast");
		writeprocsysnet("core/default_qdisc", "fq_codel_fast");
	} else
#endif
#ifdef HAVE_PIE
	if (!strcmp(aqd, "pie")) {
		insmod("sch_pie");
		writeprocsysnet("core/default_qdisc", "pie");
	} else
#endif
#ifdef HAVE_CAKE
	if (!strcmp(aqd, "cake")) {
		insmod("sch_cake");
		writeprocsysnet("core/default_qdisc", "cake");
	} else
#endif
	{
		insmod("sch_sfq");
		writeprocsysnet("core/default_qdisc", "sfq");
	}

	if (nvram_match("tcp_congestion_control", "bbr")) {
		writeprocsysnet("core/default_qdisc", "fq_codel");
	}
#ifdef HAVE_IRQBALANCE
	mkdir("/var/run/irqbalance", 0777);
	eval("irqbalance", "-t", "10");
#endif
	fprintf(stderr, "done\n");
}
