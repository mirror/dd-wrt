/*
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
	if (!nvram_matchi("disable_watchdog", 1))
		eval("watchdog");

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	char mtdpath[64];
	int board = getRouterBrand();

	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
	eval("ifconfig", "eth2", "up");

	int mtd = getMTD("ART");
	char *maddr = NULL;
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
	fp = fopen(mtdpath, "rb");
	if (fp != NULL) {
		fseek(fp, 0x1000, SEEK_SET);
		char *smem = malloc(0x8000);
		fread(smem, 0x8000, 1, fp);

		fclose(fp);

		fp = fopen("/tmp/board1.bin", "wb");
		fwrite(smem, 12064, 1, fp);
		fclose(fp);
		fp = fopen("/tmp/board2.bin", "wb");
		fwrite(&smem[0x4000], 12064, 1, fp);
		fclose(fp);
		free(smem);

	}


	detect_wireless_devices();

	int s;
	struct ifreq ifr;
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		char macaddr[32];

		strcpy(macaddr, ether_etoa((char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
		close(s);
	}
	insmod("mii_gpio");


/*
# Switch-A:
# sw port 0 -> Trunk to CPU(eth1)
# sw port 5 -> Trunk to CPU(eth2)
# sw port 4 -> Trunk to Switch-B sw port 0
# sw port 6 -> Trunk to Switch-B sw port 5
# sw port 3 -> WAN
# sw port 2 -> LAN1
# sw port 1 -> LAN2
# Switch-B:
# sw port 0 -> Trunk to Switch-A sw port 4
# sw port 5 -> Trunk to Switch-A sw port 6
# sw port 4 -> LAN3
# sw port 3 -> LAN4
# sw port 2 -> LAN5
# sw port 1 -> LAN6
# sw port 6 -> No Used
*/


	system("swconfig dev switch0 set reset 1");
	system("swconfig dev switch0 set enable_vlan 0");
	system("swconfig dev switch0 vlan 1 set ports \"0 4 1 2\"");
	system("swconfig dev switch0 vlan 2 set ports \"5 3\"");
	system("swconfig dev switch0 set apply");

	system("swconfig dev switch1 set reset 1");
	system("swconfig dev switch1 set enable_vlan 0");
	system("swconfig dev switch1 vlan 1 set ports \"0 4 3 2 1\"");
	system("swconfig dev switch1 set apply");


//	sw_user_lan_ports_vlan_config "1" "1 2 3 4 5 6" "0" "0" "0" "normal_lan"
//	sw_user_lan_ports_vlan_config "2" ""            "0" "1" "0" "wan"
//	local vlanid="$1"
//	local lan_ports="$2"
//	local is_bridge_vlan="$3"
//	local is_wan_vlan="$4"
//	local is_iptv_vlan="$5"
//	local vlan_mode="$6"

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");
	nvram_set("wl1_ifname", "ath1");
}

int check_cfe_nv(void)
{
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
