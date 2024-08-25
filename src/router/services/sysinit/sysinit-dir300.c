/*
 * sysinit-dir300.c
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
#include <sys/socket.h>
#include <sys/sysmacros.h>
#include <sys/ioctl.h>
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

#include "devices/wireless.c"

extern void vlan_init(int num);

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

#ifndef HAVE_DIR400
	int mtd = getMTD("board_config");
	char mtdpath[64];

	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
	FILE *fp = fopen(mtdpath, "rb");

	if (fp) {
		fseek(fp, 0x1000, SEEK_SET);
		unsigned int test;

		fread(&test, 4, 1, fp);
		fprintf(stderr, "test pattern is %X\n", test);
		if (test != 0xffffffff) {
			fprintf(stderr,
				"radio config fixup is required to clean bad stuff out of memory, otherwise the radio config cannot be detected\n");
			fseek(fp, 0, SEEK_SET);
			char *block = (char *)malloc(65536);

			fread(block, 65536, 1, fp);
			fclose(fp);
			int i;

			for (i = 0x1000; i < 65536; i++)
				block[i] = 0xff;
			fp = fopen("/tmp/radio", "wb");
			fwrite(block, 65536, 1, fp);
			eval("mtd", "-f", "write", "/tmp/radio",
			     "board_config"); // writes
			//
			// back
			// new
			// config
			// and
			// reboots
			eval("event", "5", "1", "15");
		}
		fclose(fp);
	}
#else
	if (!nvram_matchi("dir400preconfig", 1)) {
		nvram_seti("dir400preconfig", 1);
		nvram_commit();
		int mtd = getMTD("fullflash");
		char mtdpath[64];
		char mac[18];

		sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
		FILE *fp = fopen(mtdpath, "rb");
		int s = searchfor(fp, "lan_mac=", 512);

		if (s == -1) {
			fprintf(stderr, "no mac found in config\n");
			fclose(fp);
		} else {
			mac[17] = 0;
			fread(mac, 17, 1, fp);
			fclose(fp);
			mtd = getMTD("board_config");
			sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
			fp = fopen(mtdpath, "rb");
			fseek(fp, 0, SEEK_SET);
			char *block = (char *)malloc(65536);

			fread(block, 65536, 1, fp);
			fclose(fp);
			unsigned char in_addr[6];
			int changed = 0;

			ether_atoe(mac, &in_addr[0]);
			if (memcmp(block + 96, &in_addr[0], 6)) {
				changed++;
				memcpy(block + 96, &in_addr[0], 6); // wlan mac
			}
			in_addr[5]++;
			if (memcmp(block + 102, &in_addr[0], 6)) {
				changed++;
				memcpy(block + 102, &in_addr[0], 6); // eth0 mac
			}
			in_addr[5]++;
			if (memcmp(block + 108, &in_addr[0], 6)) {
				changed++;
				memcpy(block + 108, &in_addr[0], 6); // eth1 mac
			}
			in_addr[5]++;
			if (memcmp(block + 118, &in_addr[0], 6)) {
				changed++;
				memcpy(block + 118, &in_addr[0],
				       6); // wlan1 mac
			}
			if (changed) {
				fprintf(stderr, "radio config needs to be adjusted, system will reboot after flashing\n");
				fp = fopen("/tmp/radio", "wb");
				fwrite(block, 65536, 1, fp);
				fclose(fp);
				eval("mtd", "-f", "write", "/tmp/radio",
				     "board_config"); // writes
				//
				// back
				// new
				// config
				// and
				// reboots
				eval("event", "5", "1", "15");
			} else {
				fprintf(stderr, "no change required, radio config remains unchanged\n");
			}
			free(block);
		}
	}
#endif

	/*
	 * network drivers 
	 */
#ifdef HAVE_HOTPLUG2
	insmod("ar231x");
#else
	insmod("ar2313");
#endif
	detect_wireless_devices(RADIO_ALL);
	// eval ("ifconfig", "wifi0", "up");
	eval("ifconfig", "eth0", "up"); // wan
	writeprocsys("dev/wifi0/ledpin", "2");
	writeprocsys("dev/wifi0/softled", "1");
	if (getRouterBrand() == ROUTER_BOARD_FONERA2200) {
		//              eval("ifconfig", "eth0", "up", "promisc");      // required for vlan config
		eval("/sbin/vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("/sbin/vconfig", "add", "eth0", "0");
		eval("/sbin/vconfig", "add", "eth0", "1");
		struct ifreq ifr;
		int s;

		char macaddr[32];
		if (get_hwaddr("eth0", macaddr)) {
			nvram_set("et0macaddr", macaddr);
			nvram_set("et0macaddr_safe", macaddr);
			set_hwaddr("vlan1", macaddr);
		}
	} else {
#ifdef HAVE_SWCONFIG

		eval("swconfig", "dev", "eth0", "set", "reset", "1");
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 5t");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "4 5t");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");

		nvram_seti("sw_cpuport", 5);
		nvram_seti("sw_wan", 4);
		nvram_seti("sw_lan1", 0);
		nvram_seti("sw_lan2", 1);
		nvram_seti("sw_lan3", 2);
		nvram_seti("sw_lan4", 3);
//      set network.eth0_1.ports="0 1 2 3 5t"
#else
		vlan_init(0xff); // 4 lan + 1 wan
#endif
		nvram_default_geti("port0vlans", 2);
		nvram_default_geti("port1vlans", 1);
		nvram_default_geti("port2vlans", 1);
		nvram_default_geti("port3vlans", 1);
		nvram_default_geti("port4vlans", 1);
		nvram_default_get("port5vlans", "1 2 16000");
		struct ifreq ifr;
		int s;

		char macaddr[32];
		if (get_hwaddr("eth0", macaddr)) {
			nvram_set("et0macaddr", macaddr);
			nvram_set("et0macaddr_safe", macaddr);
			nvram_set("lan_hwaddr", macaddr);
			set_hwaddr("vlan2", macaddr);
		}
	}

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

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

void start_fixboard(void)
{
	int mtd = getMTD("board_config");
	char mtdpath[64];

	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);
	fprintf(stderr, "board config path = %s\n", mtdpath);
	FILE *fp = fopen(mtdpath, "rb");

	if (fp) {
		fseek(fp, 0x1000, SEEK_SET);
		unsigned int test;

		fread(&test, 4, 1, fp);
		fprintf(stderr, "test pattern is %X\n", test);
		if (test != 0xffffffff) {
			fprintf(stderr,
				"radio config fixup is required to clean bad stuff out of memory, otherwise the radio config cannot be detected\n");
			fseek(fp, 0, SEEK_SET);
			char *block = (char *)malloc(65536);

			fread(block, 65536, 1, fp);
			fclose(fp);
			int i;

			for (i = 0x1000; i < 65536; i++)
				block[i] = 0xff;
			fp = fopen("/tmp/radio", "wb");
			fwrite(block, 65536, 1, fp);
			eval("mtd", "-f", "write", "/tmp/radio",
			     "board_config"); // writes
			//
			// back
			// new
			// config
			// and
			// reboots
			eval("event", "5", "1", "15");
		}
		fclose(fp);
	}
}

char *set_wan_state(int state)
{
	return NULL;
}

void start_devinit_arch(void)
{
}
void load_wifi_drivers(void)
{
}