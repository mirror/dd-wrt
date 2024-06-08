/*
 * sysinit-ipq807x.c
 *
 * Copyright (C) 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include "devices/ethernet.c"
#include "devices/wireless.c"

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

void *get_deviceinfo_mr7350(char *var)
{
	static char res[256];
	bzero(res, sizeof(res));
	FILE *fp = fopen("/dev/mtd13", "rb");
	if (!fp)
		return NULL;
	char newname[64];
	snprintf(newname, 64, "%s=", var);
	char *mem = safe_malloc(0x2000);
	fread(mem, 0x2000, 1, fp);
	fclose(fp);
	int s = (0x2000 - 1) - strlen(newname);
	int i;
	int l = strlen(newname);
	for (i = 0; i < s; i++) {
		if (!strncmp(mem + i, newname, l)) {
			strncpy(res, mem + i + l, 17);
			free(mem);
			return res;
		}
	}
	free(mem);
	return NULL;
}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;
	char dev[64];
	char mtdpath[64];

	if (!nvram_matchi("disable_watchdog", 1)) {
		insmod("imx2_wdt");
	}

	/*
	 * Setup console 
	 */
	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();
	insmod("qca-ssdk");
	insmod("qca-nss-dp");
	eval("modprobe", "ath11k_ahb");
	int mtd = getMTD("art");
	if (mtd == -1)
		mtd = getMTD("ART");
	sprintf(mtdpath, "/dev/mtd%d", mtd);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		fseek(fp, 0x1000, SEEK_SET);
		int i;
		FILE *out = fopen("/tmp/caldata.bin", "wb");
		for (i = 0; i < 0x10000; i++)
			putc(getc(fp), out);
		fclose(out);
		fseek(fp, 0x1000, SEEK_SET);
		out = fopen("/tmp/board.bin", "wb");
		for (i = 0; i < 0x10000; i++)
			putc(getc(fp), out);
		fclose(out);
		fclose(fp);
	}
	if (!nvram_match("nobcreset", "1"))
		eval("mtd", "resetbc", "s_env");
	set_envtools(11, "0x0", "0x40000", "0x20000",2);
	char *maddr = get_deviceinfo_mr7350("hw_mac_addr");
	int newmac[5];
	if (maddr) {
		fprintf(stderr, "sysinit using mac %s\n", maddr);
		sscanf(maddr, "%02x:%02x:%02x:%02x:%02x:%02x", &newmac[0], &newmac[1], &newmac[2], &newmac[3], &newmac[4],
		       &newmac[5]);
	}

	char ethaddr[32];
	sprintf(ethaddr, "%02x:%02x:%02x:%02x:%02x:%02x", newmac[0] & 0xff, newmac[1] & 0xff, newmac[2] & 0xff, newmac[3] & 0xff,
		newmac[4] & 0xff, newmac[5] & 0xff);
	nvram_set("et0macaddr", ethaddr);
	nvram_set("et0macaddr_safe", ethaddr);
	set_hwaddr("eth0", ethaddr);
	set_hwaddr("eth1", ethaddr);
	set_hwaddr("eth2", ethaddr);
	set_hwaddr("eth3", ethaddr);
	set_hwaddr("eth4", ethaddr);
	MAC_ADD(ethaddr);
	nvram_set("wlan0_hwaddr",ethaddr);
	sysprintf("echo %s > /sys/devices/platform/soc@0/c000000.wifi/ieee80211/phy0/macaddress", ethaddr);
	MAC_ADD(ethaddr);
	nvram_set("wlan1_hwaddr",ethaddr);
	sysprintf("echo %s > /sys/devices/platform/soc@0/c000000.wifi/ieee80211/phy1/macaddress", ethaddr);
	writeproc("/proc/irq/61/smp_affinity", "8");
	writeproc("/proc/irq/62/smp_affinity", "4");
	writeproc("/proc/irq/63/smp_affinity", "2");
	writeproc("/proc/irq/64/smp_affinity", "1");

	writeproc("/proc/irq/47/smp_affinity", "4");
	writeproc("/proc/irq/53/smp_affinity", "2");
	writeproc("/proc/irq/56/smp_affinity", "1");

	writeproc("/proc/irq/57/smp_affinity", "2");
	writeproc("/proc/irq/59/smp_affinity", "4");


/*		sysprintf("echo netdev > /sys/devices/platform/leds/leds/rt-ac58u:blue:lan/trigger");
		sysprintf("echo netdev > /sys/devices/platform/leds/leds/rt-ac58u:blue:wan/trigger");
		sysprintf("echo eth0 > /sys/devices/platform/leds/leds/rt-ac58u:blue:lan/device_name");
		sysprintf("echo eth1 > /sys/devices/platform/leds/leds/rt-ac58u:blue:wan/device_name");

		sysprintf("echo \"link tx rx\" > /sys/devices/platform/leds/leds/rt-ac58u:blue:lan/mode");
		sysprintf("echo \"link tx rx\" > /sys/devices/platform/leds/leds/rt-ac58u:blue:wan/mode");
*/
/*
	ucidef_set_led_netdev "lan1-port-activity" "LAN1-PORT-ACTIVITY" "amber:lan1" "lan1" "link tx rx"
	ucidef_set_led_netdev "lan1-port-link" "LAN1-PORT-LINK" "green:lan1" "lan1" "link_10 link_100 link_1000"
	ucidef_set_led_netdev "lan2-port-activity" "LAN2-PORT-ACTIVITY" "amber:lan2" "lan2" "link tx rx"
	ucidef_set_led_netdev "lan2-port-link" "LAN2-PORT-LINK" "green:lan2" "lan2" "link_10 link_100 link_1000"
	ucidef_set_led_netdev "lan3-port-activity" "LAN3-PORT-ACTIVITY" "amber:lan3" "lan3" "link tx rx"
	ucidef_set_led_netdev "lan3-port-link" "LAN3-PORT-LINK" "green:lan3" "lan3" "link_10 link_100 link_1000"
	ucidef_set_led_netdev "lan4-port-activity" "LAN4-PORT-ACTIVITY" "amber:lan4" "lan4" "link tx rx"
	ucidef_set_led_netdev "lan4-port-link" "LAN4-PORT-LINK" "green:lan4" "lan4" "link_10 link_100 link_1000"
	ucidef_set_led_netdev "wan-port-activity" "WAN-PORT-ACTIVITY" "amber:wan" "wan" "link tx rx"
	ucidef_set_led_netdev "wan-port-link" "WAN-PORT-LINK" "green:wan" "wan" "link_10 link_100 link_1000"
*/
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
