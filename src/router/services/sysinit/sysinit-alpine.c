/*
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

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <linux/mii.h>
#include "devices/wireless.c"

void set_envtools(int mtd, char *offset, char *envsize, char *blocksize, int nums, char *offset2)
{
	char m[32];
	sprintf(m, "/dev/mtd%d", mtd);
	FILE *fp = fopen("/tmp/fw_env.config", "wb");
	if (fp) {
		if (nums)
			fprintf(fp, "%s\t%s\t%s\t%s\t%d\n", m, offset, envsize, blocksize, nums);
		else
			fprintf(fp, "%s\t%s\t%s\t%s\n", m, offset, envsize, blocksize);
		if (offset2) {
			if (nums)
				fprintf(fp, "%s\t%s\t%s\t%s\t%d\n", m, offset2, envsize, blocksize, nums);
			else
				fprintf(fp, "%s\t%s\t%s\t%s\n", m, offset2, envsize, blocksize);
		}
		fclose(fp);
	}
}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	FILE *fp = NULL;

	/*
	 * Setup console 
	 */

	klogctl(8, NULL, nvram_geti("console_loglevel"));
	insmod("al_eth_drv");
	char mtdpath[64];
	int board = getRouterBrand();

	int mtd = getMTD("ART");
	char *maddr = NULL;
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
	fp = fopen(mtdpath, "rb");
	if (fp != NULL) {
		fseek(fp, 0x1000, SEEK_SET);
		char *smem = malloc(0x8000);
		fread(smem, 0x8000, 1, fp);

		fclose(fp);
		eval("rm", "-f", "/tmp/board1.bin");
		fp = fopen("/tmp/board1.bin", "wb");
		fwrite(smem, 12064, 1, fp);
		fclose(fp);
		fp = fopen("/tmp/board2.bin", "wb");
		fwrite(&smem[0x4000], 12064, 1, fp);
		fclose(fp);
		free(smem);
	}

	char macaddr[32];
	if (get_hwaddr("eth1", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
		get_hwaddr("eth0", macaddr);
		set_hwaddr("eth2", macaddr);
	}
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
	eval("ifconfig", "eth2", "up");

	eval("swconfig", "dev", "switch1", "set", "reset", "1");
	eval("swconfig", "dev", "switch1", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "switch1", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "switch1", "set", "igmp_v3", "1");

	eval("swconfig", "dev", "switch1", "vlan", "1", "set", "ports", "0t 1 2 4 6");
	eval("swconfig", "dev", "switch1", "vlan", "2", "set", "ports", "3 5t");
	eval("swconfig", "dev", "switch1", "set", "apply");

	eval("swconfig", "dev", "switch0", "set", "reset", "1");
	eval("swconfig", "dev", "switch0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "switch0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "switch0", "set", "igmp_v3", "1");

	eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", "0 1 2 3 4 5 6t");
	eval("swconfig", "dev", "switch0", "set", "apply");

	nvram_unset("sw_cpuport"); // this is a dummy. for the r9000 we need to write complete new code
	nvram_seti("sw_wan",
		   -1); // switch 0 (3) note: we cannot allow wan for now
	nvram_seti("sw_lan1", 2); // switch 0
	nvram_seti("sw_lan2", 1); // switch 0
	nvram_seti("sw_lan3", 14); // switch 1
	nvram_seti("sw_lan4", 13); // switch 1
	nvram_seti("sw_lan5", 12); // switch 1
	nvram_seti("sw_lan6", 11); // switch 1

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_geti("port5vlans", 1);
	nvram_default_geti("port6vlans", 1);

	eval("/sbin/vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("/sbin/vconfig", "add", "eth1", "1");
	eval("/sbin/vconfig", "add", "eth2", "2");

	set_named_smp_affinity("eth1", 0, 1);
	set_named_smp_affinity("eth2", 1, 1);
	set_named_smp_affinity("eth0", 2, 1);

	int i;
	writeprocsysnet("core/message_cost", "0");
	switch (board) {
	case ROUTER_NETGEAR_R9000:
		set_gpio(29, 1); //WIFI button led
		set_gpio(30, 1); //10G led
		break;
	default:
		break;
	}

	set_envtools(1, "0x0", "0x4000", "0x20000", 1, "0x20000");

	/*
	   ","*","Set","a","sane","date","
	   "," */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");
	nvram_set("wl1_ifname", "wlan1");
	nvram_set("clkfreq", "1700");
}

void start_resetleds(void)
{
	writestr("/sys/class/leds/ath10k-phy0/trigger", "none");
	writestr("/sys/class/leds/ath10k-phy1/trigger", "none");
	writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
	writestr("/sys/class/leds/ath10k-phy1/trigger", "phy1tpt");
}

void start_postnetwork(void)
{
	//	set_gpio(434 + 17, 0); // reset wifi card gpio pin
	//	set_gpio(469 + 17, 0); // reset wifi card gpio pin
	//	set_gpio(434 + 17, 1); // reset wifi card gpio pin
	//	set_gpio(469 + 17, 1); // reset wifi card gpio pin
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
	char *oclock = nvram_safe_get("overclocking");
	if (*oclock) {
		sysprintf("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
		sysprintf("echo %s000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed", oclock);
	} else {
		sysprintf("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
		sysprintf("echo 1700000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed", oclock);
		//		sysprintf("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
	}
}

char *enable_dtag_vlan(int enable)
{
	return "eth1";
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
	if (!detect_wireless_devices(RADIO_ALL)) {
		wait_for_wifi(3);
		set_named_smp_affinity("ath10k_pci", 2, 1);
		set_named_smp_affinity("ath10k_pci", 3, 2);
		set_named_smp_affinity_mask("wil6210", 6, 1);
		ifconfig("wlan2", IFUP, "0.0.0.0", NULL); // trigger firmware load and init
		//		start_resetleds();
		start_postnetwork();
	}
}
void start_arch_defaults(void)
{
}
