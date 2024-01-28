/*
 * sysinit-wasp.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <linux/mii.h>
#include "devices/wireless.c"
#include "devices/ethtools.c"

#ifdef HAVE_DIR869
struct regiondef {
	char *match;
	char *region;
};
static struct regiondef regions[] = { { "AU", "AUSTRALIA" }, { "NA", "UNITED_STATES" },	 { "US", "UNITED_STATES" },
				      { "CA", "CANADA" },    { "LA", "BRAZIL" },	 { "BR", "BRAZIL" },
				      { "EU", "GERMANY" },   { "GB", "UNITED_KINGDOM" }, { "CN", "CHINA" },
				      { "SG", "SINGAPORE" }, { "KR", "KOREA_REPUBLIC" }, { "FR", "FRANCE" },
				      { "JP", "JAPAN" },     { "IL", "ISRAEL" },	 { "RU", "RUSSIA" },
				      { "TH", "THAILAND" },  { "MY", "MALASIA" },	 { "IN", "INDIA" },
				      { "EG", "EGYPT" },     { "TW", "TAIWAN" },	 { NULL, NULL } };

static void setdlinkcountry(void)
{
	char buf[32];
	char c[32];
	char *set = NULL;
	FILE *fp = popen("cat /dev/mtdblock0|grep countrycode=", "r");
	fread(buf, 1, 27, fp);
	pclose(fp);
	buf[27] = 0;
	bzero(c, sizeof(c));
	strncpy(c, &buf[12], 2);
	if (!*c)
		return;
	int cnt = 0;
	while (regions[cnt].match) {
		if (!strcmp(regions[cnt].match, c)) {
			set = regions[cnt].region;
			break;
		}
		cnt++;
	}
	if (set) {
		if (!nvram_exists("nocountrysel"))
			nvram_seti("nocountrysel", 1);
		nvram_set("wlan0_regdomain", set);
		nvram_set("wlan1_regdomain", set);
	}
}

#endif

void start_sysinit(void)
{
	time_t tm = 0;
	struct ifreq ifr;
	int s;
	char mac[32];
	FILE *fp;

	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	/*
	 * network drivers 
	 */
	fprintf(stderr, "load ATH Ethernet Driver\n");
	system("insmod ag71xx || insmod ag7240_mod");
#ifndef HAVE_WILLY
#ifdef HAVE_WDR3500
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "0", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth0", "set", "apply");
#else
#ifdef HAVE_WDR4300
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 1");

	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 2);
	nvram_seti("sw_lan2", 3);
	nvram_seti("sw_lan3", 4);
	nvram_seti("sw_lan4", 5);
#elif defined(HAVE_E325N)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");

#elif defined(HAVE_XD3200)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
#ifndef HAVE_SR3200
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 3");
#else
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 1 2 3 4");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 5");
	nvram_seti("sw_cpuport", "0");
	nvram_seti("sw_wan", 5);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);
#endif
#elif defined(HAVE_E355AC)
#elif defined(HAVE_WR810N)
#elif defined(HAVE_WR615N)
#elif defined(HAVE_AP120C)
#elif defined(HAVE_E380AC)
#elif defined(HAVE_DW02_412H)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 1");
	nvram_seti("sw_lancpuport", 0);
	nvram_seti("sw_wancpuport", 0);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 2);
	nvram_seti("sw_lan2", 3);
	nvram_seti("sw_lan3", 4);
	nvram_seti("sw_lan4", 5);

#elif defined(HAVE_RAMBUTAN)
#elif defined(HAVE_WR650AC)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "1 6");
	nvram_seti("sw_lancpuport", 0);
	nvram_seti("sw_wancpuport", 6);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 2);
	nvram_seti("sw_lan2", 3);
	nvram_seti("sw_lan3", 4);
	nvram_seti("sw_lan4", 5);

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");

#elif defined(HAVE_JWAP606)
	// nothing
#elif defined(HAVE_DAP3662)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "1 6");
	nvram_seti("sw_lancpuport", 0);
	nvram_seti("sw_wancpuport", 6);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 2);
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");
#elif defined(HAVE_DIR862)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "5 6");
	nvram_seti("sw_lancpuport", 0);
	nvram_seti("sw_wancpuport", 6);
	nvram_seti("sw_wan", 5);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");
#elif defined(HAVE_ARCHERC25)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", -1);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);
#elif defined(HAVE_XD9531)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");
#elif defined(HAVE_CPE880)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 4");
	eval("swconfig", "dev", "eth0", "set", "apply");
#elif defined(HAVE_MMS344)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 3");
#elif defined(HAVE_WR1043V4)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 1 2 3 4");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 5");
	setSwitchLED(15, 0x20); // wan
	setSwitchLED(9, 0x10); // lan1
	setSwitchLED(14, 0x08); // lan2
	setSwitchLED(21, 0x04); // lan3
	setSwitchLED(20, 0x02); // lan4
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", 5);
#if defined(HAVE_WR1043V5)
	nvram_seti("sw_lan1", 4);
	nvram_seti("sw_lan2", 3);
	nvram_seti("sw_lan3", 2);
	nvram_seti("sw_lan4", 1);
#else
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);
#endif
#elif defined(HAVE_ARCHERC7V5)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 1");
	setSwitchLED(21, 0x2); // wan
	setSwitchLED(8, 0x4); // lan1
	setSwitchLED(17, 0x08); // lan2
	setSwitchLED(16, 0x10); // lan3
	setSwitchLED(15, 0x20); // lan4
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 2);
	nvram_seti("sw_lan2", 3);
	nvram_seti("sw_lan3", 4);
	nvram_seti("sw_lan4", 5);
#elif defined(HAVE_ARCHERC7V4)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 1");
	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 2);
	nvram_seti("sw_lan2", 3);
	nvram_seti("sw_lan3", 4);
	nvram_seti("sw_lan4", 5);
#elif defined(HAVE_LIMA)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
#elif defined(HAVE_ARCHERC7)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 2 3 4 5");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "1 6");
	nvram_seti("sw_lancpuport", 0);
	nvram_seti("sw_wancpuport", 6);
	nvram_seti("sw_wan", 1);
	nvram_seti("sw_lan1", 5);
	nvram_seti("sw_lan2", 4);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 2);

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");
#elif defined(HAVE_WZR450HP2) || defined(HAVE_WR1043V2)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "5 6");
	nvram_seti("sw_lancpuport", 0);
	nvram_seti("sw_wancpuport", 6);
	nvram_seti("sw_wan", 5);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);

	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "2 18000 19000 20000");
	nvram_default_get("port6vlans", "1 18000 19000 20000");
#else
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
	eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
	eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
	eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 1 2 3 4");
	eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 5");

	nvram_seti("sw_cpuport", 0);
	nvram_seti("sw_wan", 5);
	nvram_seti("sw_lan1", 1);
	nvram_seti("sw_lan2", 2);
	nvram_seti("sw_lan3", 3);
	nvram_seti("sw_lan4", 4);
#endif
#endif
#endif
	eval("swconfig", "dev", "eth0", "set", "apply");
#if defined(HAVE_DW02_412H)
	fp = fopen("/dev/mtdblock/5", "rb");
	if (fp) {
		unsigned char buf2[256];
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
	}
#elif defined(HAVE_RAMBUTAN)
	fp = fopen("/dev/mtdblock/0", "rb");
	if (fp) {
		fseek(fp, 0x500000, SEEK_SET);
		unsigned char buf2[256];
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[6], copy[7], copy[8], copy[9], copy[10], copy[11]);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		set_hwaddr("eth1", mac);
	}
#elif defined(HAVE_WNDR3700V4) || defined(HAVE_CPE880)
	fp = fopen("/dev/mtdblock/5", "rb");
	if (fp) {
		unsigned char buf2[256];
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
	}
#elif defined(HAVE_XD9531)
	fp = fopen("/dev/mtdblock/5", "rb");
	if (fp) {
		unsigned char buf2[256];
		fread(buf2, 256, 1, fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[6], copy[7], copy[8], copy[9], copy[10], copy[11]);
		fprintf(stderr, "configure eth1 to %s\n", mac);
		set_hwaddr("eth1", mac);
#ifdef HAVE_ATH10K
		FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
		fseek(fp, 0x1000, SEEK_SET);
		for (i = 0; i < 1088; i++)
			putc(getc(fp), out);
		fclose(out);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
#endif
		fclose(fp);
	}
#elif defined(HAVE_MMS344) && !defined(HAVE_DAP3662)
	fp = fopen("/dev/mtdblock/6", "rb");
	if (fp) {
		unsigned char buf2[256];
#ifdef HAVE_WILLY
		fseek(fp, 0x3f810, SEEK_SET);
#else
		fseek(fp, 0x2e010, SEEK_SET);
#endif
		fread(buf2, 256, 1, fp);
		fclose(fp);
		if ((!memcmp(buf2, "\xff\xff\xff\xff\xff\xff", 6) || !memcmp(buf2, "\x00\x00\x00\x00\x00\x00", 6)))
			goto out;
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		set_hwaddr("eth0", mac);
out:;
	}
#endif

#ifdef HAVE_ONNET
	if (nvram_matchi("wlan10k-ct", 1)) {
		fprintf(stderr, "Select QCA988x (CT) Firmware\n");
		eval("mount", "--bind", "/lib/firmware/ath10k/QCA988X-CT/firmware-2-ct-full-community.bin",
		     "/lib/firmware/ath10k/QCA988X/hw2.0/firmware-5.bin");
		nvram_set("wlan10k-ct_bak", "1");
	} else
		nvram_set("wlan10k-ct_bak", "0");
	nvram_async_commit();
#endif
#if defined(HAVE_ONNET) || defined(HAVE_RAYTRONIK)
	runStartup(".onnet");
#endif
#if !defined(HAVE_WR650AC) && !defined(HAVE_E355AC) && !defined(HAVE_E325N) && !defined(HAVE_E380AC) && !defined(HAVE_WR615N) && \
	!defined(HAVE_AP120C) && !defined(HAVE_WILLY) && !defined(HAVE_WR810N)
#ifndef HAVE_JWAP606
	eval("ifconfig", "eth0", "up");
#if (defined(HAVE_CPE880) || defined(HAVE_MMS344) || defined(HAVE_XD3200) || defined(HAVE_ARCHERC7V4) || \
     defined(HAVE_DW02_412H)) &&                                                                         \
	!defined(HAVE_DIR862)
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

	get_hwaddr("eth0", mac);
	set_hwaddr("vlan1", mac);
	MAC_ADD(mac);
	set_hwaddr("vlan2", mac);
#elif defined(HAVE_WZR450HP2) || defined(HAVE_WDR3500) || defined(HAVE_XD9531)
	eval("ifconfig", "eth1", "up");
#elif defined(HAVE_RAMBUTAN)
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
#elif defined(HAVE_LIMA)
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
#else
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

	get_hwaddr("eth0", mac);
	set_hwaddr("vlan1", mac);
	MAC_ADD(mac);
	set_hwaddr("vlan2", mac);
#endif
#endif
#endif
#ifdef HAVE_WR810N
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
#endif
	char macaddr[32];
	if (get_hwaddr("eth0", macaddr)) {
		nvram_set("et0macaddr", macaddr);
		nvram_set("et0macaddr_safe", macaddr);
	}
#if defined(HAVE_E355AC)
	fp = fopen("/dev/mtdblock/0", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x10000 + 0x5000, SEEK_SET);
		int i;
		for (i = 0; i < 2116; i++)
			putc(getc(fp), out);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
		fclose(out);
		eval("rm", "-f", "/tmp/board1.bin");
		out = fopen("/tmp/board1.bin", "wb");
		fseek(fp, 0x10000 + 0x5000, SEEK_SET);
		char *smem = malloc(12064);
		fread(smem, 12064, 1, fp);
		fwrite(smem, 12064, 1, out);
		free(smem);
		fclose(out);
		fclose(fp);
	}
#elif defined(HAVE_ARCHERC25)
	fp = fopen("/dev/mtdblock/5", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x5000, SEEK_SET);
		int i;

		for (i = 0; i < 6; i++)
			putc(getc(fp), out);
		char eabuf[32];
		char macaddr[32];
		get_hwaddr("eth0", macaddr);
		MAC_ADD(macaddr);
		MAC_ADD(macaddr);
		ether_atoe(macaddr, mac);
		for (i = 0; i < 6; i++)
			putc(mac[i], out);
		fseek(fp, 0x5000 + 12, SEEK_SET);
		for (i = 0; i < 2104; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
	eval("ifconfig", "eth0", "up");
	eval("ifconfig", "eth1", "up");
#elif defined(HAVE_XD3200)
	fp = fopen("/dev/mtdblock/5", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x5000, SEEK_SET);
		int i;
		for (i = 0; i < 2116; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#ifdef HAVE_CPE890
	nvram_default_get("no_ath9k", "1");
#endif
#elif defined(HAVE_E380AC)
	fp = fopen("/dev/mtdblock/0", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x20000 + 0x5000, SEEK_SET);
		int i;
		for (i = 0; i < 2116; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#elif defined(HAVE_WR650AC)
	fp = fopen("/dev/mtdblock/0", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x20000 + 0x5000, SEEK_SET);
		int i;
		for (i = 0; i < 2116; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#elif defined(HAVE_ARCHERC7V5) && !defined(HAVE_ARCHERA7V5)
	fp = fopen("/dev/mtdblock/0", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x55000, SEEK_SET);
		int i;
		for (i = 0; i < 6; i++)
			putc(getc(fp), out);
		char eabuf[32];
		char macaddr[32];
		get_hwaddr("eth0", macaddr);
		MAC_ADD(macaddr);
		MAC_ADD(macaddr);
		ether_atoe(macaddr, mac);

		for (i = 0; i < 6; i++)
			putc(mac[i], out);
		fseek(fp, 0x55000 + 12, SEEK_SET);
		for (i = 0; i < 2104; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#elif defined(HAVE_DAP2680)
	fp = fopen("/dev/mtdblock/5", "rb");
		eval("rm", "-f", "/tmp/board1.bin");
	FILE *out = fopen("/tmp/board1.bin", "wb");
	if (fp) {
		fseek(fp, 20480, SEEK_SET);
		int i;
		for (i = 0; i < 6; i++)
			putc(getc(fp), out);
		char eabuf[32];
		char macaddr[32];
		get_hwaddr("eth0", macaddr);
		MAC_ADD(macaddr);
		MAC_ADD(macaddr);
		ether_atoe(macaddr, mac);

		for (i = 0; i < 6; i++)
			putc(mac[i], out);
		fseek(fp, 20492, SEEK_SET);
		for (i = 0; i < 12052; i++)
			putc(getc(fp), out);
		fclose(fp);
	}
	fclose(out);
#elif defined(HAVE_ARCHERC7) || defined(HAVE_DIR859) || defined(HAVE_DAP3662)
	fp = fopen("/dev/mtdblock/5", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 20480, SEEK_SET);
		int i;
		for (i = 0; i < 6; i++)
			putc(getc(fp), out);
		char eabuf[32];
		char macaddr[32];
		get_hwaddr("eth0", macaddr);
		MAC_ADD(macaddr);
		MAC_ADD(macaddr);
		ether_atoe(macaddr, mac);

		for (i = 0; i < 6; i++)
			putc(mac[i], out);
		fseek(fp, 20492, SEEK_SET);
		for (i = 0; i < 2104; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#elif defined(HAVE_DIR862)
	fp = fopen("/lib/ath10k/board.bin", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		int i;
		for (i = 0; i < 6; i++)
			putc(getc(fp), out);
		memcpy(mac, "\x00\x01\x02\x03\x04\x05", 6);
		get_ether_hwaddr("eth0", mac);
		for (i = 0; i < 6; i++)
			putc(mac[i], out);
		fseek(fp, 12, SEEK_SET);
		for (i = 0; i < 2104; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#elif defined(HAVE_DW02_412H)
	fp = fopen("/dev/mtdblock5", "rb");
	FILE *out = fopen("/tmp/archerc7-board.bin", "wb");
	if (fp) {
		fseek(fp, 0x5000, SEEK_SET);
		int i;
		for (i = 0; i < 6; i++)
			putc(getc(fp), out);
		memcpy(mac, "\x00\x01\x02\x03\x04\x05", 6);
		get_ether_hwaddr("eth0", mac);
		for (i = 0; i < 6; i++)
			putc(mac[i], out);
		fseek(fp, 0x5000 + 12, SEEK_SET);
		for (i = 0; i < 2104; i++)
			putc(getc(fp), out);
		fclose(fp);
		eval("rm", "-f", "/tmp/ath10k-board.bin");
		eval("ln", "-s", "/tmp/archerc7-board.bin", "/tmp/ath10k-board.bin");
	}
	fclose(out);
#endif
	detect_wireless_devices(RADIO_ALL);
#ifdef HAVE_RAMBUTAN
	mount("/dev/ubi0_2", "/jffs", "ubifs", MS_MGC_VAL | MS_NOATIME, NULL);
#endif
#ifdef HAVE_PERU
	insmod("ledtrig-netdev");
	setEthLED(0, "eth1");
	setEthLinkLED(16, "eth1");
	setEthLED(15, "eth0");
	setEthLinkLED(14, "eth0");
	eval("insmod", "i2c-gpio-custom", "bus0=0,3,2");
	eval("insmod", "rtc-pcf8523");
	writestr("/sys/class/i2c-dev/i2c-0/device/new_device", "pcf8523 0x68");
	eval("hwclock", "-s", "-u");

	eval("ledtool", "1", "4"); //buzzer
	eval("ledtool", "5", "0"); //diag ~5sec
#elif HAVE_DW02_412H
//      don't use setWirelessLed since we only have one LED for two distinct radio's
#elif !defined(HAVE_WR810N) && !defined(HAVE_LIMA) && !defined(HAVE_RAMBUTAN)

#ifdef HAVE_WNDR3700V4
	setWirelessLed(0, 11);
	setWirelessLed(1, 14);
#elif HAVE_XD9531
	insmod("ledtrig-netdev");
	setEthLED(16, "eth1");
	setWirelessLed(0, 12);
#elif HAVE_CPE880
	insmod("ledtrig-netdev");
	setEthLED(19, "vlan2");
	setWirelessLed(0, 12);
	writestr("/sys/devices/platform/leds-gpio/leds/generic_17/brightness", "0");
	writestr("/sys/devices/platform/leds-gpio/leds/generic_20/brightness", "0");
	writestr("/sys/devices/platform/leds-gpio/leds/generic_21/brightness", "0");
	writestr("/sys/devices/platform/leds-gpio/leds/generic_22/brightness", "0");

	if (!nvram_matchi("wlanled", 0))
		eval("/sbin/wlanled", "-l", "generic_17:-94", "-l", "generic_20:-80", "-l", "generic_21:-73", "-l",
		     "generic_22:-65");
#elif HAVE_CPE890
	writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
	if (!nvram_matchi("wlanled", 0))
		eval("/sbin/wlanled", "-L", "generic_17:-94", "-L", "generic_16:-80", "-L", "generic_15:-73", "-L",
		     "generic_14:-65");
#elif HAVE_ARCHERC25
	/*
#define ARCHER_C25_74HC_GPIO_BASE		120
#define ARCHER_C25_74HC_GPIO_LED_WAN_AMBER	(ARCHER_C25_74HC_GPIO_BASE + 4)
#define ARCHER_C25_74HC_GPIO_LED_WAN_GREEN	(ARCHER_C25_74HC_GPIO_BASE + 5)
#define ARCHER_C25_74HC_GPIO_LED_WLAN2		(ARCHER_C25_74HC_GPIO_BASE + 6)
#define ARCHER_C25_74HC_GPIO_LED_WLAN5		(ARCHER_C25_74HC_GPIO_BASE + 7)
#define ARCHER_C25_74HC_GPIO_LED_LAN1		(ARCHER_C25_74HC_GPIO_BASE + 0)
#define ARCHER_C25_74HC_GPIO_LED_LAN2		(ARCHER_C25_74HC_GPIO_BASE + 1)
#define ARCHER_C25_74HC_GPIO_LED_LAN3		(ARCHER_C25_74HC_GPIO_BASE + 2)
#define ARCHER_C25_74HC_GPIO_LED_LAN4		(ARCHER_C25_74HC_GPIO_BASE + 3)
*/

	set_gpio(21, 0); // enable output
	set_gpio(19, 1); // reset
	setEthLED(125, "eth1");
	setSwitchLED(120, 0x10); // lan1
	setSwitchLED(121, 0x08); // lan2
	setSwitchLED(122, 0x04); // lan3
	setSwitchLED(123, 0x02); // lan4
	setWirelessLed(0, 126 + 32);
	setWirelessLed(1, 127 + 32);

#elif HAVE_JWAP606
	//      setWirelessLed(0, 14);
	setWirelessLed(1, 14);
#elif HAVE_WR1043V2
#ifndef HAVE_ONNET
	setWirelessLed(0, 12);
#endif
#ifdef HAVE_WDR4900V2
	setWirelessLed(1, 17);
#endif
#ifdef HAVE_WR1043V4
	setWirelessLed(0, 19);
#elif defined(HAVE_ARCHERC7V5)
	setWirelessLed(0, 14);
	setWirelessLed(1, 9);
#elif defined(HAVE_ARCHERC7V4)
	setWirelessLed(0, 24);
	setWirelessLed(1, 9);
#elif defined(HAVE_ARCHERC7)
	setWirelessLed(1, 17);
#endif
#elif HAVE_WZR450HP2
	setWirelessLed(0, 18);
#elif HAVE_WR615N
	setWirelessLed(0, 12);
#elif HAVE_E325N
	setWirelessLed(0, 0);
#elif HAVE_AP120C
	setWirelessLed(0, 0);
#elif HAVE_SR3200
	setWirelessLed(0, 19);
	writestr("/sys/class/leds/ath10k-phy1/trigger", "phy1tpt");
#elif HAVE_E380AC
	setWirelessLed(0, 0);
	setWirelessLed(1, 2);
#elif HAVE_E355AC
	setWirelessLed(0, 0);
	setWirelessLed(1, 3);
#elif HAVE_WR650AC
	setWirelessLed(0, 13);
	setWirelessLed(1, 2);
#elif HAVE_DIR869
	setdlinkcountry();
#elif HAVE_DIR859
	setWirelessLed(0, 19);
//      setWirelessLed(1, 32);
#elif HAVE_DIR825C1
	setWirelessLed(0, 13);
	setWirelessLed(1, 32);
#else
	setWirelessLed(0, 0);
#endif
#endif
	nvram_default_geti("port0vlans", 2);
	nvram_default_geti("port1vlans", 1);
	nvram_default_geti("port2vlans", 1);
	nvram_default_geti("port3vlans", 1);
	nvram_default_geti("port4vlans", 1);
	nvram_default_get("port5vlans", "1 2 16000");
	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "wlan0");

	return;
	cprintf("done\n");
}

void start_resetleds(void)
{
	writestr("/sys/class/leds/ath10k-phy0/trigger", "none");
	writestr("/sys/class/leds/ath10k-phy0/trigger", "phy0tpt");
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
#if defined(HAVE_XD3200)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
#ifndef HAVE_SR3200
	if (!state) {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 2 3");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "rem", "vlan1");
		eval("vconfig", "rem", "vlan2");
		return "eth0";
	} else {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 3");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		return NULL;
	}
#else
	if (!state) {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0 1 2 3 4 5");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "rem", "vlan1");
		eval("vconfig", "rem", "vlan2");
		return "eth0";
	} else {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 1 2 3 4");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 5");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		return NULL;
	}
#endif
#elif defined(HAVE_CPE880)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	if (!state) {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0 4 5");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "rem", "vlan1");
		eval("vconfig", "rem", "vlan2");
		return "eth0";
	} else {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 5");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 4");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		return NULL;
	}
#elif defined(HAVE_E355AC)
#elif defined(HAVE_WR810N)
#elif defined(HAVE_WR615N)
#elif defined(HAVE_AP120C)
#elif defined(HAVE_E380AC)
#elif defined(HAVE_RAMBUTAN)
#elif defined(HAVE_WR650AC)
#elif defined(HAVE_JWAP606)
#elif defined(HAVE_DAP3662)
#elif defined(HAVE_DIR862)
#elif defined(HAVE_ARCHERC25)
#elif defined(HAVE_XD9531)
#elif defined(HAVE_MMS344)
	eval("swconfig", "dev", "eth0", "set", "reset", "1");
	if (!state) {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0 2 3");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "rem", "vlan1");
		eval("vconfig", "rem", "vlan2");
		return "eth0";
	} else {
		eval("swconfig", "dev", "eth0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "eth0", "set", "igmp_snooping", "0");
		eval("swconfig", "dev", "eth0", "set", "igmp_v3", "1");
		eval("swconfig", "dev", "eth0", "vlan", "1", "set", "ports", "0t 2");
		eval("swconfig", "dev", "eth0", "vlan", "2", "set", "ports", "0t 3");
		eval("swconfig", "dev", "eth0", "set", "apply");
		eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
		eval("vconfig", "add", "eth0", "1");
		eval("vconfig", "add", "eth0", "2");
		return NULL;
	}
#endif
	return NULL;
}

void start_devinit_arch(void)
{
}
