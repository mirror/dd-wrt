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
#include <cymac.h>
#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>

#ifdef HAVE_X86

static char *getdisc(void)	// works only for squashfs 
{
	int i;
	static char ret[4];
	unsigned char *disks[] = { "sda2", "sdb2", "sdc2", "sdd2", "sde2", "sdf2", "sdg2", "sdh2",
		"sdi2"
	};
	int a;

	for (a = 0; a < 10; a++) {
		for (i = 0; i < 9; i++) {
			char dev[64];

			sprintf(dev, "/dev/%s", disks[i]);
			FILE *in = fopen(dev, "rb");

			if (in == NULL)
				goto skip;
			// exist, skipping
			char buf[4];

			fread(buf, 4, 1, in);
			if (buf[0] == 'h' && buf[1] == 's' && buf[2] == 'q' && buf[3] == 't') {
				fclose(in);
				// filesystem detected
				fprintf(stderr, "file system detected at %s\n", disks[i]);
				strncpy(ret, disks[i], 3);
				return ret;
			}
			fclose(in);
		      skip:;
		}
		sleep(1);
	}
	return NULL;
}

#endif
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
	system("mount -t tmpfs none /dev -o size=512K");

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
	mkdir("/dev/misc",0700);
	mknod("/dev/misc/gpio", S_IFCHR | 0644, makedev(125, 0));
#endif
#ifdef HAVE_X86
	fprintf(stderr, "waiting for hotplug\n");
	char dev[64];
	char *disc = getdisc();

	if (disc == NULL) {
		fprintf(stderr, "no valid dd-wrt partition found, calling shell\n");
		eval("/bin/sh");
	}
	// sprintf (dev, "/dev/discs/disc%d/part1", index);
	// mount (dev, "/boot", "ext2", MS_MGC_VAL, NULL);

	sprintf(dev, "/dev/%s3", disc);
	if (mount(dev, "/usr/local", "ext2", MS_MGC_VAL, NULL)) {
		eval("/sbin/mkfs.ext2", "-F", "-b", "1024", dev);
		mount(dev, "/usr/local", "ext2", MS_MGC_VAL, NULL);
//              eval("/bin/tar", "-xvvjf", "/etc/local.tar.bz2", "-C", "/");
	}
	mkdir("/usr/local", 0700);
	mkdir("/usr/local/nvram", 0700);
#endif
#ifdef HAVE_MSTP
	fprintf(stderr, "start MSTP Daemon\n");
	eval("/sbin/mstpd");
#endif
	fprintf(stderr, "done\n");
}
