/*
 * sysinit-realtek.c
 *
 * Copyright (C) 2012 - 2025 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include "devices/ethtools.c"

void set_envtools(int mtd, char *offset, char *envsize, char *blocksize, int nums)
{
	char m[32];
	sprintf(m, "/dev/mtd%d", mtd);
	FILE *fp = fopen("/tmp/fw_env.config", "wb");
	if (fp) {
		if (nums)
			fprintf(fp, "%s\t%s\t%s\t%s\t%d\n", m, offset, envsize, blocksize, nums);
		else
			fprintf(fp, "%s\t%s\t%s\t%s\n", m, offset, envsize, blocksize);
		fclose(fp);
	}
}

void start_sysinit(void)
{
	time_t tm = 0;
	struct ifreq ifr;
	int s;
	FILE *fp;
	char *bootcmd;

	/*
	 * Setup console 
	 */

	klogctl(8, NULL, nvram_geti("console_loglevel"));
	int mtd;
	char *mac;
	insmod("rtl838x_eth");
	switch (getRouterBrand()) {
	case ROUTER_HP_1920:
		FILE *fp = openMTD("factory");
		if (fp) {
			fseek(fp, 0x68, SEEK_SET);

			char name[32];
			int i;
			char mac[32];
			char bin[6];
			fread(bin, 6, 1, fp);
			fclose(fp);
			ether_etoa(bin, mac);
			MAC_ADD(mac);
			MAC_ADD(mac);
			for (i = 1; i < 51; i++) {
				sprintf(name, "lan%02d", i);
				if (ifexists(name)) {
					set_hwaddr(name, mac);
					MAC_ADD(mac);
				}
			}
		}
		break;
	case ROUTER_ZYXEL_XGS1250:
		mtd = getMTD("u-boot-env");
		if (mtd != -1)
			set_envtools(mtd, "0x0", "0x10000", "0x10000", 0);
		bootcmd = getUEnv("bootcmd");
		if (!bootcmd || !strstr(bootcmd, "rtk network on")) {
			fprintf(stderr, "change bootcmd to fix networking\n");
			eval("fw_setenv", "netretry", "no");
			eval("fw_setenv", "bootnet", "tftpboot 0x84f00000 192.168.1.254:xgs1250.bin;bootm");
			eval("fw_setenv", "bootcmd", "rtk network on;run bootnet; boota");
		}
		mac = getUEnv("ethaddr");
		if (mac) {
			char name[32];
			int i;
			set_hwaddr("eth0", mac);
			for (i = 1; i < 13; i++) {
				sprintf(name, "lan%02d", i);
				set_hwaddr(name, mac);
				MAC_ADD(mac);
			}
		}
		break;
	case ROUTER_EDGECORE_ECS4125:
		mtd = getMTD("u-boot-env");
		if (mtd != -1)
			set_envtools(mtd, "0x0", "0x10000", "0x10000", 0);
		//		bootcmd = getUEnv("bootcmd");
		//		if (!bootcmd || !strstr(bootcmd, "rtk network on")) {
		//			fprintf(stderr, "change bootcmd to fix networking\n");
		//			eval("fw_setenv","bootnet", "tftpboot 0x84f00000 192.168.1.254:xgs1250.bin");
		//			eval("fw_setenv","bootcmd", "rtk network on; run bootnet; boota");
		//		}
		mac = getUEnv("ethaddr");
		if (mac) {
			char name[32];
			int i;
			set_hwaddr("eth0", mac);
			for (i = 1; i < 11; i++) {
				sprintf(name, "lan%02d", i);
				set_hwaddr(name, mac);
				MAC_ADD(mac);
			}
		}
		break;
	case ROUTER_HASIVO_S1100W8XGT:
		mtd = getMTD("u-boot-env");
		if (mtd != -1)
			set_envtools(mtd, "0x0", "0x400", "0x10000", 0);
		eval("fw_setenv", "bootcmd", "rtk network on;boota");
		mac = getUEnv("mac_start");
		if (mac) {
			char name[32];
			int i;
			for (i = 1; i < 9; i++) {
				sprintf(name, "lan%02d", i);
				set_hwaddr(name, mac);
				MAC_ADD(mac);
			}
		}
		break;
	default:
		mtd = getMTD("u-boot-env");
		if (mtd != -1)
			set_envtools(mtd, "0x0", "0x400", "0x10000", 0);
		mac = getUEnv("mac_start");
		if (mac) {
			char name[32];
			int i;
			for (i = 1; i < 53; i++) {
				sprintf(name, "lan%02d", i);
				set_hwaddr(name, mac);
				MAC_ADD(mac);
			}
		}
		break;
	}
	if (nvram_match("DD_BOARD", "D-Link DGS-1210-28P F") || nvram_match("DD_BOARD", "D-Link DGS-1210-28MP F") ||
	    nvram_match("DD_BOARD", "Zyxel XGS1250-12") || nvram_match("DD_BOARD", "Zyxel XGS1250-12 B1")) {
		sysprintf("echo 1 > /sys/class/hwmon/hwmon0/pwm1_enable");
		sysprintf("echo 250 > /sys/class/hwmon/hwmon0/pwm1");
	}

	nvram_set("dsa", "1"); // flag to hide eth0
	insmod("rtl_crypto");
	insmod("cryptodev");
	/*
	 * network drivers 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

	return;
}

void start_resetleds(void)
{
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

char *enable_dtag_vlan(int enable)
{
	return "lan1";
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
void sys_overclocking(void)
{
	char *oclock = nvram_safe_get("overclocking");
	if (*oclock) {
		sysprintf("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
		sysprintf("echo %s000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed", oclock);
	} else {
		sysprintf("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
		sysprintf("echo 750000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed", oclock);
	}
}
