/*
 * devinit.c
 *
 * Copyright (C) 2010 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

void start_devinit_arch();

#ifdef HAVE_OPENRISC
static void install_sdcard(void)
{
	mkdir("/usr/local", 0700);
	mkdir("/usr/local/nvram", 0700);
	FILE *fp = fopen("/boot/.installed", "rb");
	if (fp != NULL) // already locally installed?
	{
		fclose(fp);
		return;
	}
	sleep(10); //give some time until sd is up
	fprintf(stderr, "check if secondary device is available\n");
	fp = fopen("/dev/sda", "rb");
	if (fp == NULL) // no cf disc installed or no sd card. doesnt matter, we exit if no secondary device is in
	{
		fclose(fp);
		return;
	}
	fclose(fp);
	fprintf(stderr, "installing firmware to internal SD Card\n");
	mkdir("/tmp/install", 0700);
	int check = mount("/dev/sda", "/tmp/install", "ext4", MS_MGC_VAL, NULL);
	if (check != 0) {
		fprintf(stderr, "device isnt formated, use EXT2\n");
		fp = fopen("/dev/sda", "rb");
		fseeko(fp, 0, SEEK_END);
		off_t size = ftello(fp);
		size -= 65536 * 16;
		size /= 4096;
		char newsize[32];
		sprintf(newsize, "%d", size);
		eval("mkfs.ext4", "-b", "4096", "-N", "65536", "-L", "dd-wrt", "/dev/sda", newsize);
		mount("/dev/sda", "/tmp/install", "ext4", MS_MGC_VAL, NULL);
	}
	fprintf(stderr, "copy files to SD Card\n");
	eval("cp", "-f", "/tmp/install/usr/local/nvram/nvram.bin", "/tmp/install/usr/local/nvram/nvram.bak");
	eval("cp", "-R", "-d", "-f", "/boot", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/bin", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/etc", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/jffs", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/lib", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/mmc", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/mnt", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/opt", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/sbin", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/usr", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/www", "/tmp/install");
	eval("cp", "-R", "-d", "-f", "/var", "/tmp/install");
	eval("mv", "-f", "/tmp/install/usr/local/nvram/nvram.bak", "/tmp/install/usr/local/nvram/nvram.bin");
	mkdir("/tmp/install/dev", 0700);
	mkdir("/tmp/install/sys", 0700);
	mkdir("/tmp/install/proc", 0700);
	mkdir("/tmp/install/tmp", 0700);
	sysprintf("echo \"blank\" > /tmp/install/boot/.installed");
	sysprintf("echo \"mem=59M root=/dev/sda\" > /tmp/install/boot/kparam");
	eval("umount", "/tmp/install");
	eval("sync");
	fprintf(stderr, "signal installation complete\n");
	set_gpio(4, 1);
	sleep(1);
	set_gpio(4, 0);
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
	char s_dev[64];
	char *s_disc = getdisc();

	if (s_disc == NULL) {
		fprintf(stderr, "no valid dd-wrt partition found, calling shell\n");
		eval("/bin/sh");
	}
	// sprintf (dev, "/dev/discs/disc%d/part1", index);
	// mount (dev, "/boot", "ext2", MS_MGC_VAL, NULL);
	if (strlen(s_disc) == 7) //mmcblk0 / nvme0n1
		sprintf(s_dev, "/dev/%sp3", s_disc);
	else
		sprintf(s_dev, "/dev/%s3", s_disc);
	free(s_disc);
	insmod("jbd2");
	insmod("mbcache");
	insmod("crc16");
	insmod("ext4");
	if (mount(s_dev, "/usr/local", "ext4", MS_MGC_VAL | MS_SYNCHRONOUS, NULL)) {
		eval("mkfs.ext4", "-F", "-b", " 1024", s_dev);
		mount(s_dev, "/usr/local", "ext4", MS_MGC_VAL | MS_SYNCHRONOUS, NULL);
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
#if !defined(HAVE_X86) && !defined(HAVE_VENTANA) && !defined(HAVE_LAGUNA) && !defined(HAVE_LIMA) && !defined(HAVE_RAMBUTAN) && \
	!defined(HAVE_NEWPORT) && !defined(HAVE_QCA9888)
	eval("ln", "-s", "/lib/ath10k/board_9984.bin", "/tmp/board1.bin");
#endif
#endif

#ifndef HAVE_OPENRISC
#if !defined(HAVE_VENTANA) || defined(HAVE_NEWPORT)
#ifndef HAVE_RAMBUTAN
#ifndef HAVE_WDR4900
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600)
	system("mount --bind /usr/local /jffs");
	nvram_seti("enable_jffs2", 1);
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
#ifdef _SC_NPROCESSORS_ONLN
	int cpucount = sysconf(_SC_NPROCESSORS_ONLN);
#else
	int cpucount = 1
#endif
#ifdef HAVE_IRQBALANCE
	if (cpucount > 1) {
		/* do not start irqbalance if it doesnt make sense at all, it will just create bogus warnings */
		mkdir("/var/run/irqbalance", 0777);
#ifdef HAVE_IPQ6018
//		eval("irqbalance", "-t", "10", "-i", "33", "-i", "34", "-i", "35", "-i", "36", "-i", "47", "-i", "53", "-i", "56", "-i","57","-i", "59","-i", "61", "-i", "62", "-i", "63", "-i", "64");
#else
		eval("irqbalance", "-t", "10");
#endif
	}
#endif
#ifdef HAVE_X86
	char dev[64];
	char *disk = getdisc();

	if (disk == NULL) {
		fprintf(stderr, "no valid dd-wrt partition found, calling shell");
		eval("/bin/sh");
		exit(0);
	}

	FILE *in = fopen("/usr/local/nvram/nvram.db", "rb");

	if (in != NULL) {
		fclose(in);
		mkdir("/tmp/nvram", 0700);
		eval("cp", "/etc/nvram/nvram.db", "/tmp/nvram");
		eval("cp", "/etc/nvram/offsets.db", "/tmp/nvram");
		eval("/usr/sbin/convertnvram");
		nvram_commit();
		unlink("/etc/nvram/nvram.db");
		unlink("/etc/nvram/offsets.db");
	}
	sprintf(dev, "/dev/%s", disk);
	free(disk);
	eval("hdparm", "-W", "0", dev);
	eval("sdparm", "-s", "WCE", "-S", dev);
	eval("sdparm", "-c", "WCE", "-S", dev);

	//recover nvram if available
	in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in == NULL) {
		fprintf(stderr, "recover broken nvram\n");
		int size = nvram_size();
		in = fopen(dev, "rb");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (size + 65536), SEEK_SET);
		unsigned char *mem = malloc(size);
		fread(mem, size, 1, in);
		fclose(in);
		if (mem[0] == 0x46 && mem[1] == 0x4c && mem[2] == 0x53 && mem[3] == 0x48) {
			fprintf(stderr, "found recovery\n");
			in = fopen("/usr/local/nvram/nvram.bin", "wb");
			if (in != NULL) {
				fwrite(mem, size, 1, in);
				fclose(in);
				eval("sync");
				eval("mount", "-o", "remount,ro", "/usr/local");
				eval("mount", "-o", "remount,ro", "/");
				eval("hdparm", "-f", dev);
				eval("hdparm", "-F", dev);
				sleep(5);
				writeproc("/proc/sysrq-trigger", "b");
			}
		}
		free(mem);
	} else {
		fclose(in);
	}

#endif
#if defined(HAVE_RB600) || defined(HAVE_NEWPORT) && !defined(HAVE_WDR4900)
	//recover nvram if available
	char dev[64];
	FILE *in = fopen64("/usr/local/nvram/nvram.bin", "rb");
	if (in == NULL) {
		fprintf(stderr, "recover broken nvram\n");
		sprintf(dev, "/dev/sda");
		int size = nvram_size();
		in = fopen(dev, "rb");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (size + 65536), SEEK_SET);
		unsigned char *mem = malloc(size);
		fread(mem, size, 1, in);
		fclose(in);
		if (mem[0] == 0x46 && mem[1] == 0x4c && mem[2] == 0x53 && mem[3] == 0x48) {
			fprintf(stderr, "found recovery\n");
			in = fopen("/usr/local/nvram/nvram.bin", "wb");
			if (in != NULL) {
				fwrite(mem, size, 1, in);
				fclose(in);
				eval("sync");
				sleep(5);
				eval("event", "5", "1", "15");
			}
		}
		free(mem);
	} else {
		fclose(in);
	}
#endif
#ifdef HAVE_OPENRISC
	install_sdcard();
#endif
	start_devinit_arch();
	fprintf(stderr, "done\n");
}
