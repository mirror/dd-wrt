/*
 * sysinit-x86.c
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
 * System Initialisation for Standard PC and compatible Routers
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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "devices/ethernet.c"
#include "devices/wireless.c"

#define sys_reboot() eval("sync"); eval("/bin/umount","-a","-r"); eval("event","3","1","15")

void start_sysinit(void)
{
	time_t tm = 0;

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
	//recover nvram if available
	in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in == NULL) {
		fprintf(stderr, "recover broken nvram\n");
		sprintf(dev, "/dev/%s", disk);
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

	if (!nvram_matchi("disable_watchdog", 1))
		eval("watchdog");	// system watchdog
#ifdef HAVE_ERC
	if (isregistered_real() && nvram_matchi("ree_resetme", 1)) {
		fprintf(stderr, "Restoring REE default nvram\n");
		eval("nvram", "restore", "/etc/defaults/x86ree.backup");
		eval("reboot");
		eval("event", "5", "1", "15");
	}
#endif

	cprintf("sysinit() setup console\n");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	/*
	 * eval("insmod","md5"); eval("insmod","aes"); eval("insmod","blowfish");
	 * eval("insmod","deflate"); eval("insmod","des");
	 * eval("insmod","michael_mic"); eval("insmod","cast5");
	 * eval("insmod","crypto_null"); 
	 */
	/* load sensors */
	insmod("input-polldev");
	insmod("hwmon-vid");

	insmod("abituguru");
	insmod("abituguru3");
	insmod("ad7314");
	insmod("ad7418");
	insmod("adm1021");
	insmod("adm1025");
	insmod("adm1026");
	insmod("adm1029");
	insmod("adm1031");
	insmod("adm9240");
	insmod("applesmc");
	insmod("asb100");
	insmod("atxp1");
	insmod("coretemp");
	insmod("dme1737");
	insmod("ds1621");
	insmod("f71805f");
	insmod("fam15h_power");
	insmod("gl518sm");
	insmod("gl520sm");
	insmod("it87");
	insmod("k10temp");
	insmod("k8temp");
	insmod("lm63");
	insmod("lm75");
	insmod("lm77");
	insmod("lm78");
	insmod("lm80");
	insmod("lm83");
	insmod("lm85");
	insmod("lm87");
	insmod("lm90");
	insmod("lm92");
	insmod("lm93");
	insmod("max1619");
	insmod("max1668");
	insmod("max6650");
	insmod("pc87360");
	insmod("pc87427");
	insmod("pcf8591");
	insmod("sis5595");
	insmod("smsc47b397");
	insmod("smsc47m1");
	insmod("smsc47m192");
	insmod("thmc50");
	insmod("via686a");
	insmod("vt1211");
	insmod("vt8231");
	insmod("w83627ehf");
	insmod("w83627hf");
	insmod("w83781d");
	insmod("w83791d");
	insmod("w83792d");
	insmod("w83793");
	insmod("w83l785ts");

	detect_ethernet_devices();
	eval("ifconfig", "eth0", "0.0.0.0", "up");
	eval("ifconfig", "eth1", "0.0.0.0", "up");
	eval("ifconfig", "eth2", "0.0.0.0", "up");
	eval("ifconfig", "eth3", "0.0.0.0", "up");

	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
	FILE *fp = fopen("/sys/bus/pci/devices/0000:04:00.0/device", "rb");	//pcengines apu fuckup check
	if (fp) {
		char checkbuf[32];
		fscanf(fp, "%s", &checkbuf[0]);
		fclose(fp);
		if (!strcmp(&checkbuf[0], "0xabcd")) {
			sys_reboot();
		}

	}

	detect_wireless_devices(RADIO_ALL);

	mknod("/dev/rtc", S_IFCHR | 0644, makedev(253, 0));
#ifdef HAVE_CPUTEMP
	// insmod("nsc_gpio");
	// insmod("scx200_gpio");
	// insmod("scx200_i2c");
	// insmod("scx200_acb");
	// insmod("lm77");
#endif

	nvram_set("wl0_ifname", "ath0");
	mknod("/dev/crypto", S_IFCHR | 0644, makedev(10, 70));
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	eval("hwclock", "-s", "-u");
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

void enable_dtag_vlan(int enable)
{

}

#include "tools/recover.c"
