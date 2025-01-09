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

#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>

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
	FILE *fp = NULL;

	/*
	 * Setup console 
	 */

	klogctl(8, NULL, nvram_geti("console_loglevel"));

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

	insmod("mii_gpio");
	insmod("gpio-pca953x");
	insmod("qca-ssdk");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x10", "0x002613a0", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0xe0", "0xc74164de", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0xe4", "0x000ea545", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x4", "0x07680000", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x8", "0x07600000", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0xc", "0x80", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x624", "0x007f7f7f", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x7c", "0x4e", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x90", "0x4e", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x94", "0x4e", "4");

	//      #CPU -->","(P0/5)QCA8337A(P4/6)--->(P0/5)QCA8337B
	//      #remove trunking on -0/5
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x700", "0xd000", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x704", "0x00ec0000", "4");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x660", "0x0014017e", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x66c", "0x0014017d", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x678", "0x0014017b", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x684", "0x00140177", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x690", "0x0014016f", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x69c", "0x0014015f", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x6a8", "0x0014013f", "4");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x420", "0x00010001", "4");
	//      #change","p5","vid","-->2
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x448", "0x00020001", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x428", "0x00010001", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x430", "0x00010001", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x440", "0x00010001", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x450", "0x00010001", "4");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x438", "0x00020001", "4");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x424", "0x00002040", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x44c", "0x00002040", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x42c", "0x00001040", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x434", "0x00001040", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x43c", "0x00001040", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x444", "0x00001040", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x454", "0x00001040", "4");

	//      #VLAN1-0t/1/2/4/5t/6,VLAN2-0t/3/5t
	//      #vlan1-0t/1/2/4/6 vlan2-3/5t
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x610", "0x0019dd50", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x614", "0x80010002", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x610", "0x001b77f0", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x614", "0x80020002", "4");

	//      #","do","not","learn","mac","address","on","internal","trunk","5
	eval("ssdk_sh_id", "0", "fdb", "portLearn", "set", "5", "disable");
	eval("ssdk_sh_id", "0", "fdb", "fdb", "entry", "flush", "0");

	//      # For rfc packet lose issue
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x94", "0x7e", "4");
	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x7c", "0x7e", "4");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x700", "0xd000", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x704", "0xec0000", "4");
	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x620", "0x1000f0", "4");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x808", "0x7f004e", "4");
	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x808", "0x7f004e", "4");

	eval("ssdk_sh_id", "0", "debug", "phy", "set", "0x3", "0xd", "0x7");
	eval("ssdk_sh_id", "0", "debug", "phy", "set", "0x3", "0xe", "0x3c");
	eval("ssdk_sh_id", "0", "debug", "phy", "set", "0x3", "0xd", "0x4007");
	eval("ssdk_sh_id", "0", "debug", "phy", "set", "0x3", "0xe", "0x00");
	eval("ssdk_sh_id", "0", "debug", "phy", "set", "0x3", "0x00", "0x1200");

	eval("ssdk_sh_id", "1", "debug", "phy", "set", "0x4", "0xd", "0x7");
	eval("ssdk_sh_id", "1", "debug", "phy", "set", "0x4", "0xe", "0x3c");
	eval("ssdk_sh_id", "1", "debug", "phy", "set", "0x4", "0xd", "0x4007");
	eval("ssdk_sh_id", "1", "debug", "phy", "set", "0x4", "0xe", "0x00");
	eval("ssdk_sh_id", "1", "debug", "phy", "set", "0x4", "0x00", "0x1200");

	eval("ethtool", "-K", "eth1", "gro", "on");
	eval("ssdk_sh_id", "0", "0", "vlan", "entry", "flush");
	eval("ssdk_sh_id", "1", "0", "vlan", "entry", "flush");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "0", "1");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "0", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "0", "tagged");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "4", "1");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "4", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "4", "tagged");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "6", "1");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "6", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "6", "tagged");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "2", "1");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "2", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "2", "untagged");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "1", "1");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "1", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "1", "untagged");
	eval("ssdk_sh_id", "0", "vlan", "entry", "create", "1");
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "1", "0", "tagged");
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "1", "4", "tagged");
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "1", "6",
	     "tagged"); // connected to switch1 port 0
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "1", "2", "untagged");
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "1", "1", "untagged");
	eval("ssdk_sh_id", "1", "portvlan", "defaultCVid", "set", "0", "1");
	eval("ssdk_sh_id", "1", "portvlan", "ingress", "set", "0", "check");
	eval("ssdk_sh_id", "1", "portvlan", "egress", "set", "0", "tagged");
	eval("ssdk_sh_id", "1", "portvlan", "defaultCVid", "set", "5", "1");
	eval("ssdk_sh_id", "1", "portvlan", "ingress", "set", "5", "check");
	eval("ssdk_sh_id", "1", "portvlan", "egress", "set", "5", "tagged");
	eval("ssdk_sh_id", "1", "portvlan", "defaultCVid", "set", "4", "1");
	eval("ssdk_sh_id", "1", "portvlan", "ingress", "set", "4", "check");
	eval("ssdk_sh_id", "1", "portvlan", "egress", "set", "4", "untagged");
	eval("ssdk_sh_id", "1", "portvlan", "defaultCVid", "set", "3", "1");
	eval("ssdk_sh_id", "1", "portvlan", "ingress", "set", "3", "check");
	eval("ssdk_sh_id", "1", "portvlan", "egress", "set", "3", "untagged");
	eval("ssdk_sh_id", "1", "portvlan", "defaultCVid", "set", "2", "1");
	eval("ssdk_sh_id", "1", "portvlan", "ingress", "set", "2", "check");
	eval("ssdk_sh_id", "1", "portvlan", "egress", "set", "2", "untagged");
	eval("ssdk_sh_id", "1", "portvlan", "defaultCVid", "set", "1", "1");
	eval("ssdk_sh_id", "1", "portvlan", "ingress", "set", "1", "check");
	eval("ssdk_sh_id", "1", "portvlan", "egress", "set", "1", "untagged");
	eval("ssdk_sh_id", "1", "vlan", "entry", "create", "1");
	eval("ssdk_sh_id", "1", "vlan", "member", "add", "1", "0",
	     "tagged"); // connected to port6 on switch 0
	eval("ssdk_sh_id", "1", "vlan", "member", "add", "1", "5", "tagged");
	eval("ssdk_sh_id", "1", "vlan", "member", "add", "1", "4", "untagged");
	eval("ssdk_sh_id", "1", "vlan", "member", "add", "1", "3", "untagged");
	eval("ssdk_sh_id", "1", "vlan", "member", "add", "1", "2", "untagged");
	eval("ssdk_sh_id", "1", "vlan", "member", "add", "1", "1", "untagged");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "5", "2");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "5", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "5", "tagged");
	eval("ssdk_sh_id", "0", "portvlan", "defaultCVid", "set", "3", "2");
	eval("ssdk_sh_id", "0", "portvlan", "ingress", "set", "3", "check");
	eval("ssdk_sh_id", "0", "portvlan", "egress", "set", "3", "untagged");
	eval("ssdk_sh_id", "0", "vlan", "entry", "create", "2");
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "2", "5",
	     "tagged"); // wan cpu port
	eval("ssdk_sh_id", "0", "vlan", "member", "add", "2", "3",
	     "untagged"); // wan port
	eval("ssdk_sh_id", "1", "vlan", "entry", "create", "2");

	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x50", "0xcc35cc35", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x54", "0xca35ca35", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x58", "0xc935c935", "4");
	eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x5c", "0x03ffff00", "4");

	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x50", "0xcc35cc35", "4");
	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x54", "0xca35ca35", "4");
	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x58", "0xc935c935", "4");
	eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x5c", "0x03ffff00", "4");

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
	set_gpio(442 + 17, 0); // reset wifi card gpio pin
	set_gpio(477 + 17, 0); // reset wifi card gpio pin
	set_gpio(442 + 17, 1); // reset wifi card gpio pin
	set_gpio(477 + 17, 1); // reset wifi card gpio pin
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
	char *oclock = nvram_safe_get("overclocking");
	if (*oclock) {
		sysprintf("echo userspace > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
		sysprintf("echo %s000 > /sys/devices/system/cpu/cpufreq/policy0/scaling_setspeed", oclock);
	} else {
		sysprintf("echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor");
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
		writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
		writestr("/sys/class/leds/ath10k-phy1/trigger", "phy1tpt");
		set_named_smp_affinity("ath10k_pci", 2, 1);
		set_named_smp_affinity("ath10k_pci", 3, 2);
	}
}