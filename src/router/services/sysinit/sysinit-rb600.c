
/*
 * sysinit-rb600.c
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
#include <cymac.h>
#include "devices/wireless.c"

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct stat tmp_stat;
	time_t tm = 0;

#ifndef HAVE_WDR4900
	eval("mount", "-o", "remount,rw", "/dev/root");
	eval("mount", "-o", "remount,rw", "/dev/sda1", "/");
	sleep(1);		//give some time for remount
	mkdir("/usr/local", 0700);
	mkdir("/usr/local/nvram", 0700);
#endif
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	//for extension board
#ifndef HAVE_WDR4900
	insmod("fsl_pq_mdio");	//rb800 only as it seems
	insmod("gianfar_driver");	//rb800 only as it seems
	insmod("atl1c");	//rb800 only as it seems
	insmod("via-velocity");
	insmod("via-rhine");
	insmod("tulip");
#endif
#ifdef HAVE_UNIWIP
	insmod("mpc8xxx_wdt");
//	insmod("ocf");
//	insmod("cryptodev");
//	insmod("cryptosoft");
//	insmod("talitos");
	insmod("gpio");
/* gpios 
8 = reset
10 = pci1 
11 = pci2
18 = gps ant check
19 = wireless
20 = gps ant
21 = r232 enable
22 = port reset
*/


#endif
	struct ifreq ifr;
	int s;

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		char macbase[32];
		sprintf(macbase, "%s", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
		MAC_ADD(macbase);
		int i;
		for (i = 3; i < 9; i++) {
			char ifname[32];
			sprintf(ifname, "eth%d", i);
			eval("ifconfig", ifname, "hw", "ether", macbase);
			MAC_ADD(macbase);
		}
	}
#if defined(HAVE_WDR4900) && !defined(HAVE_UNIWIP)
	eval("mkdir", "/tmp/firmware");
	char mtdpath[64];
	int mtd = getMTD("caldata");
	sprintf(mtdpath, "/dev/mtdblock%d", mtd);
	FILE *fp = fopen(mtdpath, "rb");
	if (fp) {
		mtd = getMTD("u-boot");
		sprintf(mtdpath, "/dev/mtdblock%d", mtd);

		FILE *macfp = fopen(mtdpath, "rb");
		unsigned char wmac[6];
		if (macfp) {
			fseek(macfp, 326656, SEEK_SET);
			int i;
			for (i = 0; i < 6; i++)
				wmac[i] = getc(macfp);
			fclose(macfp);
		}

		fseek(fp, 4096, SEEK_SET);
		FILE *out = fopen("/tmp/firmware/pci_wmac0.eeprom", "wb");
		if (out) {
			int i;
			for (i = 0; i < 2048; i++) {
				if (i > 1 && i < 8) {
					putc(wmac[i - 2], out);
					getc(fp);
				} else
					putc(getc(fp), out);
			}
			fclose(out);
		}
		fseek(fp, 20480, SEEK_SET);
		out = fopen("/tmp/firmware/pci_wmac1.eeprom", "wb");
		if (wmac[5] == 0) {
			wmac[5] = 255;
			wmac[4]--;
		} else
			wmac[5]--;

		if (out) {
			int i;
			for (i = 0; i < 2048; i++)
				if (i > 1 && i < 8) {
					putc(wmac[i - 2], out);
					getc(fp);
				} else
					putc(getc(fp), out);
			fclose(out);
		}
		fclose(fp);
	}
#endif
	/*
	 * network drivers 
	 */
	detect_wireless_devices();

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW))) {
		char eabuf[32];

		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
		nvram_set("et0macaddr_safe", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		nvram_set("et0macaddr", ether_etoa((unsigned char *)ifr.ifr_hwaddr.sa_data, eabuf));
		close(s);
	}
#ifndef HAVE_WDR4900
	//recover nvram if available
	char dev[64];
	FILE *in = fopen64("/usr/local/nvram/nvram.bin", "rb");
	if (in == NULL) {
		fprintf(stderr, "recover broken nvram\n");
		sprintf(dev, "/dev/sda");
		in = fopen(dev, "rb");
		fseeko(in, 0, SEEK_END);
		off_t mtdlen = ftello(in);
		fseeko(in, mtdlen - (65536 * 2), SEEK_SET);
		unsigned char *mem = malloc(65536);
		fread(mem, 65536, 1, in);
		fclose(in);
		if (mem[0] == 0x46 && mem[1] == 0x4c && mem[2] == 0x53 && mem[3] == 0x48) {
			fprintf(stderr, "found recovery\n");
			in = fopen("/usr/local/nvram/nvram.bin", "wb");
			if (in != NULL) {
				fwrite(mem, 65536, 1, in);
				fclose(in);
				free(mem);
				eval("sync");
				sleep(5);
				eval("event", "5", "1", "15");
			}
		}
		free(mem);
	} else {
		fclose(in);
	}
#elif !defined(HAVE_UNIWIP)
	system("swconfig dev switch0 set reset 1");
	system("swconfig dev switch0 set enable_vlan 1");
	system("swconfig dev switch0 vlan 1 set ports \"0t 2 3 4 5\"");
	system("swconfig dev switch0 vlan 2 set ports \"0t 1\"");
	system("swconfig dev switch0 set apply");

	eval("ifconfig", "eth0", "up");
	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
	eval("vconfig", "add", "eth0", "1");
	eval("vconfig", "add", "eth0", "2");

	mtd = getMTD("u-boot");
	sprintf(mtdpath, "/dev/mtdblock%d", mtd);

	fp = fopen(mtdpath, "rb");
	char mac[32];
	if (fp) {
		unsigned char buf2[256];
		fseek(fp, 326656, SEEK_SET);
		fread(buf2, 256, 1, fp);
		fclose(fp);
		unsigned int copy[256];
		int i;
		for (i = 0; i < 256; i++)
			copy[i] = buf2[i] & 0xff;
		sprintf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", copy[0], copy[1], copy[2], copy[3], copy[4], copy[5]);
		fprintf(stderr, "configure eth0 to %s\n", mac);
		eval("ifconfig", "eth0", "hw", "ether", mac);
		MAC_SUB(mac);
		MAC_SUB(mac);
		fprintf(stderr, "configure vlan1 to %s\n", mac);
		eval("ifconfig", "vlan1", "hw", "ether", mac);
		MAC_ADD(mac);
		MAC_ADD(mac);
		MAC_ADD(mac);
		fprintf(stderr, "configure vlan2 to %s\n", mac);
		eval("ifconfig", "vlan2", "hw", "ether", mac);
	}
#else
	set_gpio(244,1); //gps
#endif
	if (!nvram_match("disable_watchdog", "1"))
		eval("watchdog");

	/*
	 * Set a sane date 
	 */
	stime(&tm);
	nvram_set("wl0_ifname", "ath0");

	return;
}

int check_cfe_nv(void)
{
	nvram_set("portprio_support", "0");
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
