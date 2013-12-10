/*
 * sysinit-broadcom.c
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
#include <linux/if_ether.h>
#include <linux/sockios.h>
#include <net/if.h>

#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/sockios.h>

#include <bcmnvram.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <utils.h>
#include <wlutils.h>
#include <cymac.h>
#include <services.h>
#include "devices/wireless.c"

#define sys_restart() eval("event","3","1","1")
#define sys_reboot() eval("kill -15 -1"); eval("sleep 3"); eval("kill -9 -1"); eval("umount -a -r"); eval("sync"); eval("event","3","1","15")

#ifndef BFL_AFTERBURNER
#define	BFL_AFTERBURNER		0x0200
#endif

static void check_brcm_cpu_type(void)
{
	FILE *fcpu;
	char cpu_type[20];
	char type2[30];

	fcpu = fopen("/proc/cpuinfo", "r");

	if (fcpu == NULL)
		cprintf("Open /proc/cpuinfo fail...0\n");
	else {
		char buf[500];

		fgets(buf, 500, fcpu);
		fscanf(fcpu, "%s %s %s %s %s", buf, buf, buf, cpu_type, type2);
		if (!strcmp(type2, "BCM4704")) {
			nvram_set("cpu_type", cpu_type);
			fclose(fcpu);
			return;
		}
		if (!strcmp(type2, "BCM4712")) {
			nvram_set("cpu_type", cpu_type);
			fclose(fcpu);
			return;
		}
		if (!strcmp(type2, "BCM4702")) {
			nvram_set("cpu_type", cpu_type);
			fclose(fcpu);
			return;
		}
		if (!strcmp(type2, "BCM3302")) {
			nvram_set("cpu_type", cpu_type);
			fclose(fcpu);
			return;
		}
		fgets(buf, 500, fcpu);
		fscanf(fcpu, "%s %s %s %s %s", buf, buf, buf, cpu_type, type2);
		// fprintf(stderr, "cpu_type : %s\n", cpu_type); 
		fclose(fcpu);
		if (!strcmp(cpu_type, "BCM4710")
		    || !strcmp(cpu_type, "BCM4702")) {
			cprintf("We got BCM4702 board...\n");
			nvram_set("cpu_type", cpu_type);
		} else if (!strcmp(cpu_type, "BCM3302")
			   || !strcmp(cpu_type, "BCM4712")) {
			cprintf("We got BCM4712 board...\n");
			nvram_set("cpu_type", cpu_type);
		} else {
			cprintf("We got unknown board...\n");
			nvram_set("cpu_type", cpu_type);
		}
	}

}

static void loadWlModule(void)	// set wled params, get boardflags,
					// set afterburner bit, load wl,
					// unset afterburner bit
{

	int brand = getRouterBrand();
	char macbuf[32];
	char eaddr[32];

#ifndef HAVE_BUFFALO
	nvram_set("pa0maxpwr", "251");	// force pa0maxpwr to be 251
#endif

	if (check_hw_type() == BCM4702_CHIP)
		nvram_unset("wl0_abenable");
	else {
		nvram_set("wl0_abenable", "1");
		nvram_set("wl1_abenable", "1");
	}

	switch (brand) {
	case ROUTER_LINKSYS_WRH54G:
		nvram_set("wl0gpio0", "135");
		break;
	case ROUTER_BUFFALO_WZRRSG54:
		nvram_unset("wl0_abenable");
		nvram_unset("wl1_abenable");
		break;
	case ROUTER_ASUS_WL550GE:
		nvram_set("wl0gpio1", "0");
		nvram_set("wl0gpio2", "0");
		break;
	case ROUTER_ASUS_WL500W:
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_MOTOROLA:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_RT480W:
	case ROUTER_USR_5465:
	case ROUTER_ASUS_RTN10:
		nvram_set("wl0gpio0", "136");
		break;
	case ROUTER_ASUS_RTN10U:
		nvram_set("ledbh5", "7");
		break;
	case ROUTER_WAP54G_V3:
		nvram_set("wl0gpio0", "0");
		nvram_set("wl0gpio2", "255");
		nvram_set("wl0gpio3", "255");
		nvram_set("wl0gpio5", "136");
		break;
	case ROUTER_ASUS_WL520GUGC:
		nvram_set("wl0gpio0", "0");
		nvram_set("wl0gpio1", "136");
		nvram_set("wl0gpio2", "0");
		nvram_set("wl0gpio3", "0");
		break;
	case ROUTER_LINKSYS_WTR54GS:
	case ROUTER_WAP54G_V1:
		nvram_set("wl0gpio0", "136");
		nvram_set("wl0gpio1", "0");
		nvram_set("wl0gpio2", "0");
		nvram_set("wl0gpio3", "0");
		break;
	case ROUTER_BUFFALO_WBR54G:
	case ROUTER_BUFFALO_WBR2G54S:
	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
	case ROUTER_WRT300N:
	case ROUTER_WRT300NV11:
	case ROUTER_WRT310N:
	case ROUTER_WRT350N:
	case ROUTER_WRT600N:
	case ROUTER_USR_5461:
		nvram_set("wl0gpio0", "8");
		break;
	case ROUTER_NETGEAR_WG602_V4:
		nvram_set("wl0gpio0", "8");
		nvram_set("wl0gpio1", "0");
		nvram_set("wl0gpio2", "0");
		nvram_set("wl0gpio3", "0");
		break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		nvram_set("wl0gpio2", "136");
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		nvram_set("wl0gpio0", "0");
		nvram_set("wl0gpio5", "136");
		break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL500G_PRE:
		nvram_unset("wl0gpio0");
		break;
	case ROUTER_BELKIN_F5D7230_V2000:
		// case ROUTER_BELKIN_F5D7230_V3000:
	case ROUTER_BELKIN_F5D7231:
		nvram_set("wl0gpio3", "136");
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		nvram_set("wl0gpio5", "8");
		break;
	case ROUTER_BELKIN_F5D7231_V2000:
		nvram_set("wl0gpio0", "2");
		nvram_set("wl0gpio1", "0");
		break;
	case ROUTER_BUFFALO_WLAG54C:
	case ROUTER_ASUS_WL700GE:
		nvram_set("wl0gpio0", "135");
		break;
	case ROUTER_NETGEAR_WNR3500L:
		nvram_set("ledbh0", "7");
		break;
	case ROUTER_WRT320N:
		nvram_set("ledbh0", "136");
		nvram_set("ledbh1", "11");
		break;
	case ROUTER_NETGEAR_WNR2000V2:
		nvram_set("ledbh5", "8");
		break;
	}

	int boardflags;

	switch (brand) {
	case ROUTER_WRT150N:
	case ROUTER_WRT150NV11:
	case ROUTER_WRT160N:
	case ROUTER_WRT160NV3:
	case ROUTER_ASUS_RTN16:
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4301:
	case ROUTER_BELKIN_F7D4302:
	case ROUTER_BELKIN_F5D8235V3:
	case ROUTER_WRT300N:
	case ROUTER_WRT300NV11:
	case ROUTER_WRT310N:
	case ROUTER_WRT310NV2:
	case ROUTER_WRT320N:
	case ROUTER_WRT350N:
	case ROUTER_BUFFALO_WZRG144NH:
	case ROUTER_BUFFALO_WZRG300N:
	case ROUTER_NETGEAR_WNR834B:
	case ROUTER_NETGEAR_WNR834BV2:
	case ROUTER_NETGEAR_WNDR3300:
	case ROUTER_NETGEAR_WNDR3400:
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR2000V2:
	case ROUTER_ASUS_WL500W:
	case ROUTER_WRT610NV2:
	case ROUTER_DYNEX_DX_NRUTER:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E1000V2:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
	case ROUTER_LINKSYS_E2500:
	case ROUTER_LINKSYS_E3200:
	case ROUTER_LINKSYS_E4200:
	case ROUTER_NETGEAR_WNDR4000:
	case ROUTER_ASUS_RTN66:
	case ROUTER_NETCORE_NW715P:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
	case ROUTER_ASUS_AC66U:
	case ROUTER_D1800H:
		insmod("wl");	// load module
		break;
	case ROUTER_LINKSYS_EA6500:
		if (!sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr"))
		    || startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
			unsigned char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("pci/2/1/macaddr", mac);
		}
		nvram_set("partialboots","0");
	case ROUTER_LINKSYS_EA2700:
		nvram_set("bootpartition","0");
		insmod("wl");	// load module
		break;
	case ROUTER_WRT600N:
		fprintf(stderr, "fixing wrt600n\n");
		wl_hwaddr("eth0", macbuf);
		ether_etoa((uchar *) macbuf, eaddr);
		nvram_set("wl0_hwaddr", eaddr);
		MAC_SUB(eaddr);
		if (!nvram_match("et0macaddr", eaddr)) {
			nvram_set("et0macaddr", eaddr);
			nvram_commit();
//              eval("/sbin/reboot");
//              exit( 0 );
		}
		eval("/sbin/ifconfig", "eth2", "hw", "ether", eaddr);
		wl_hwaddr("eth1", macbuf);
		ether_etoa((uchar *) macbuf, eaddr);
		nvram_set("wl1_hwaddr", eaddr);
		break;
	case ROUTER_WRT610N:
		wl_hwaddr("eth0", macbuf);
		ether_etoa((uchar *) macbuf, eaddr);
		nvram_set("wl0_hwaddr", eaddr);
		wl_hwaddr("eth1", macbuf);
		ether_etoa((uchar *) macbuf, eaddr);
		nvram_set("wl1_hwaddr", eaddr);
		break;

	default:
		boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
		fprintf(stderr, "boardflags are 0x%04X\n", boardflags);
		if (boardflags == 0)	// we can try anyway
		{
			nvram_set("boardflags", "0x0200");
			insmod("wl");	// load module
			nvram_unset("boardflags");
		} else if (boardflags & BFL_AFTERBURNER)	// ab flag already
			// set
		{
			insmod("wl");	// load module
		} else		// ab flag not set
		{
			char bf[16];

			sprintf(bf, "0x%04X", boardflags);
			boardflags |= BFL_AFTERBURNER;
			fprintf(stderr, "enable Afterburner, boardflags are 0x%04X\n", boardflags);
			char ab[16];

			sprintf(ab, "0x%04X", boardflags);
			char *oldvalue = nvram_get("boardflags");	// use the

			// string for 
			// restoring
			// since the
			// Buffalo
			// WZR-RS-G54 
			// does await 
			// a 0x10 in
			// the
			// bootloader, 
			// otherwise
			// the nvram
			// gets
			// deleted
			nvram_set("boardflags", ab);	// set boardflags with
			// AfterBurner bit on
			insmod("wl");	// load module
			nvram_set("boardflags", oldvalue);	// set back to
			// original
		}

	}
	detect_wireless_devices();
	return;
}

char wanifname[8], wlifname[8];

#define BCM4712_CPUTYPE "0x4712"

static void setup_4712(void)
{
	uint boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);

	if (nvram_match("cpu_type", BCM4712_CPUTYPE) || nvram_match("cpu_type", "BCM3302") || nvram_match("cpu_type", "BCM4712")) {
		if (boardflags & BFL_ENETVLAN) {
			cprintf("setup_4712(): Enable VLAN\n");
			// nvram_set("setup_4712","1");
			strcpy(wanifname, "vlan1");
			strcpy(wlifname, "eth1");
			if (!strcmp(nvram_safe_get("wan_ifname"), "eth1")) {
				// nvram_set("setup_4712","1-1");
				nvram_set("wan_ifname", "vlan1");
				nvram_set("wan_ifnames", "vlan1");
				nvram_set("wan_default", "vlan1");
			}
			if (!strstr(nvram_safe_get("lan_ifnames"), "vlan0")) {
				// nvram_set("setup_4712","1-2");
				nvram_set("lan_ifnames", "vlan0 eth1");
				nvram_set("vlan0hwname", "et0");
				nvram_set("vlan1hwname", "et0");
				nvram_set("wl0_ifname", "eth1");
				// nvram_set ("need_commit","1");
			}
		}		// VLAN enabled
		else {
			// nvram_set("setup_4712","2");
			cprintf("setup_4712(): Disable VLAN, it must be in bridge mode\n");
			nvram_set("lan_ifnames", "eth0 eth1");
			strcpy(wlifname, "eth1");
			nvram_set("wl0_ifname", "eth1");
		}
	} else {		// 4702, 4704
		cprintf("setup_4712(): It's a 4702 or 4704 hardware, VLAN can't be used in these 2 boards\n");
		strcpy(wanifname, "eth1");
		strcpy(wlifname, "eth2");
		nvram_set("wl0_ifname", "eth2");
		if (!strcmp(nvram_safe_get("wan_ifname"), ""))
			nvram_set("lan_ifnames", "eth0 eth1 eth2 wlanb0 wlana0");
		else
			nvram_set("lan_ifnames", "eth0 eth2");
	}
	// nvram_set ("need_commit","1");

}

static int check_nv(char *name, char *value)
{
	int ret = 0;

	if (nvram_match("manual_boot_nv", "1"))
		return 0;

	if (!nvram_get(name)) {
		cprintf("ERR: Cann't find %s !.......................\n", name);
		nvram_set(name, value);
		ret++;
	} else if (nvram_invmatch(name, value)) {
		cprintf("ERR: The %s is %s, not %s !.................\n", name, nvram_safe_get(name), value);
		nvram_set(name, value);
		ret++;
	}

	return ret;
}


static void set_regulation(int card, char *code, char *rev)
{
	char path[32];
	sprintf(path, "pci/%d/1/regrev", card + 1);
	nvram_set(path, rev);
	sprintf(path, "pci/%d/1/ccode", card + 1);
	nvram_set(path, code);
	sprintf(path, "wl%d_country_rev", card);
	nvram_set(path, rev);
	sprintf(path, "wl%d_country_code", card);
	nvram_set(path, code);
	if (!card) {
		nvram_set("wl_country_rev", rev);
		nvram_set("wl_country_code", code);
	}
}

void start_sysinit(void)
{
	char buf[PATH_MAX];
	struct utsname name;
	struct stat tmp_stat;
	time_t tm = 0;

#ifdef HAVE_BCMMODERN
	mknod("/dev/nvram", S_IFCHR | 0644, makedev(229, 0));
	mkdir("/dev/gpio", 0700);
	mknod("/dev/gpio/in", S_IFCHR | 0644, makedev(127, 0));
	mknod("/dev/gpio/out", S_IFCHR | 0644, makedev(127, 1));
	mknod("/dev/gpio/outen", S_IFCHR | 0644, makedev(127, 2));
	mknod("/dev/gpio/control", S_IFCHR | 0644, makedev(127, 3));
	mknod("/dev/ppp", S_IFCHR | 0644, makedev(108, 0));
	mknod("/dev/rtc", S_IFCHR | 0644, makedev(254, 0));
	mknod("/dev/crypto", S_IFCHR | 0644, makedev(10, 70));
	mkdir("/dev/usb", 0700);
#endif
	cprintf("sysinit() setup console\n");
#ifndef HAVE_MICRO
	if (!nvram_match("disable_watchdog", "1"))
		eval("watchdog");	// system watchdog
#endif
#ifdef HAVE_80211AC
	fprintf(stderr, "boardnum %s\n", nvram_safe_get("boardnum"));
	fprintf(stderr, "boardtype %s\n", nvram_safe_get("boardtype"));
	fprintf(stderr, "boardrev %s\n", nvram_safe_get("boardrev"));
	if (nvram_get("bootflags") == NULL) {
		fprintf(stderr, "nvram invalid, erase\n");
//          eval("erase","nvram"); // ignore it for testbed
//          sys_reboot();
	}
#endif
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, atoi(nvram_safe_get("console_loglevel")));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	led_control(LED_DIAG, LED_ON);
	char *rname = getRouter();

	fprintf(stderr, "Booting device: %s\n", rname);

	nvram_unset("port_swap");

	int need_reboot = 0;

	struct nvram_tuple *basic_params = NULL;
	struct nvram_tuple *extra_params = NULL;

	struct nvram_tuple generic1[] = {
		{"lan_ifnames", "eth0 eth2", 0},
		{"wan_ifname", "eth1", 0},
		{"wl0_ifname", "eth2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple generic1_wantolan[] = {
		{"lan_ifnames", "eth2", 0},
		{"wan_ifname", "eth0", 0},
		{"wl0_ifname", "eth2", 0},
		{0, 0, 0}
	};

	struct nvram_tuple vlan_0_1[] = {
		{"lan_ifnames", "vlan0 eth1", 0},
		{"wan_ifname", "vlan1", 0},
		{"wl0_ifname", "eth1", 0},
		{0, 0, 0}
	};

	struct nvram_tuple vlan_1_2[] = {
		{"lan_ifnames", "vlan1 eth1", 0},
		{"wan_ifname", "vlan2", 0},
		{"wl0_ifname", "eth1", 0},
		{0, 0, 0}
	};

	switch (brand) {
	case ROUTER_BUFFALO_WZRRSG54:
		check_brcm_cpu_type();
		setup_4712();
		basic_params = generic1;
		eval("gpio", "init", "0");	// AOSS button
		eval("gpio", "init", "4");	// reset button
		break;

	case ROUTER_MOTOROLA:
		nvram_set("cpu_type", "BCM4712");
		setup_4712();
		break;

	case ROUTER_RT480W:
		setup_4712();
		break;

	case ROUTER_BELKIN_F5D7231_V2000:
		basic_params = vlan_0_1;
		if (nvram_match("vlan1ports", "0 5u")) {
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
		}
		break;

	case ROUTER_BELKIN_F5D7231:
	case ROUTER_USR_5461:
	case ROUTER_NETCORE_NW618:
		basic_params = vlan_0_1;
		if (nvram_match("vlan1ports", "0 5u"))
			nvram_set("vlan1ports", "0 5");
		break;

	case ROUTER_USR_5465:
		if (nvram_match("vlan1ports", "4 5u"))
			nvram_set("vlan1ports", "4 5");
		break;

	case ROUTER_ASUS_RTN10:
		basic_params = vlan_0_1;
		if (nvram_match("vlan1ports", "4 5u")) {
			nvram_set("vlan1ports", "4 5");
			if (sv_valid_hwaddr(nvram_safe_get("macaddr")))	//fix mac
				nvram_set("et0macaddr", nvram_get("macaddr"));
			need_reboot = 1;
		}
		break;

	case ROUTER_RT210W:
	case ROUTER_ASKEY_RT220XD:
		basic_params = generic1;

		if (nvram_get("et0macaddr") == NULL || nvram_get("et0macaddr") == "") {
			nvram_set("et0macaddr", "00:16:E3:00:00:10");	// fix for
			// missing
			// cfe
			// default =
			// dead LAN
			// ports.
			need_reboot = 1;
		}
		if (nvram_get("et1macaddr") == NULL || nvram_get("et1macaddr") == "") {
			nvram_set("et1macaddr", "00:16:E3:00:00:11");
			need_reboot = 1;
		}
		break;

	case ROUTER_BRCM4702_GENERIC:
		basic_params = generic1;

		if (nvram_get("et0macaddr") == NULL || nvram_get("et0macaddr") == "") {
			nvram_set("et0macaddr", "00:11:22:00:00:10");	// fix for
			// missing
			// cfe
			// default =
			// dead LAN
			// ports.
			need_reboot = 1;
		}
		if (nvram_get("et1macaddr") == NULL || nvram_get("et1macaddr") == "") {
			nvram_set("et1macaddr", "00:11:22:00:00:11");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_WL500G:
		basic_params = generic1;

		if (nvram_get("et0macaddr") == NULL || nvram_get("et0macaddr") == "") {
			nvram_set("et0macaddr", "00:0C:6E:00:00:10");	// fix for
			// missing
			// cfe
			// default =
			// dead LAN
			// ports.
			need_reboot = 1;
		}
		if (nvram_get("et1macaddr") == NULL || nvram_get("et1macaddr") == "") {
			nvram_set("et1macaddr", "00:0C:6E:00:00:10");
			need_reboot = 1;
		}
		break;

	case ROUTER_DELL_TRUEMOBILE_2300:
		setup_4712();
		nvram_set("wan_ifname", "eth1");	// fix for WAN problem.
		break;

	case ROUTER_BUFFALO_WBR54G:	// for WLA-G54
		basic_params = generic1;
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled"))	// = 
			// 
			// no 
			// lan
		{
			basic_params = generic1_wantolan;
		}
		break;

	case ROUTER_BUFFALO_WLI2_TX1_G54:
	case ROUTER_BUFFALO_WLAG54C:
	case ROUTER_WAP54G_V1:
	case ROUTER_SITECOM_WL105B:
		nvram_set("lan_ifnames", "eth1 eth2");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("wan_ifname", "eth0");	// WAN to nonexist. iface.
		nvram_set("port_swap", "1");
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled"))	// = 
			// 
			// no 
			// lan
		{
			nvram_set("lan_ifnames", "eth2");
			nvram_set("wan_ifname", "eth1");
		}
		break;

	case ROUTER_SITECOM_WL111:
		basic_params = generic1;
		break;

	case ROUTER_NETGEAR_WNR834BV2:
		if (nvram_match("force_vlan_supp", "enabled")) {
			nvram_set("lan_ifnames", "vlan0 eth2");
			nvram_set("wan_ifname", "eth1");
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");	//dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			basic_params = generic1;
		}

		if (nvram_get("pci/1/1/macaddr") == NULL) {
			nvram_set("pci/1/1/macaddr", nvram_safe_get("et0macaddr"));
			need_reboot = 1;
		}
		//params taken from firmware ver. 2.1.13 multi-region
		struct nvram_tuple wnr834bv2_pci_1_1_params[] = {
			{"pa2gw1a0", "0", 0},
			{"stbcpo", "0", 0},
			{"pa2gw1a1", "0", 0},
			{"ag0", "2", 0},
			{"ag1", "2", 0},
			{"ag2", "2", 0},
			{"ccdpo", "0", 0},
			{"txpid2ga0", "55", 0},
			{"txpid2ga1", "78", 0},
			{"txpt2g", "0x38", 0},
			{"pa2gw0a0", "0", 0},
			{"pa2gw0a1", "0", 0},
			{"boardflags", "0x200", 0},
			{"boardvendor", "0x14e4", 0},
			{"bw40po", "0", 0},
			{"sromrev", "4", 0},
			{"venid", "0x14e4", 0},
			{"boardrev", "0x4b", 0},
			{"itt2ga0", "0", 0},
			{"itt2ga1", "0", 0},
			{"pa2gw3a0", "0", 0},
			{"pa2gw3a1", "0", 0},
			{"maxp2ga0", "0", 0},
			{"maxp2ga1", "0", 0},
			{"boardtype", "0x46d", 0},
			{"boardflags2", "3", 0},
			{"ofdm2gpo", "0", 0},
			{"ledbh0", "0x8", 0},
			{"ledbh1", "-1", 0},
			{"ledbh2", "-1", 0},
			{"ledbh3", "-1", 0},
			{"mcs2gpo0", "0", 0},
			{"mcs2gpo1", "0", 0},
			{"mcs2gpo2", "0", 0},
			{"mcs2gpo3", "0", 0},
			{"mcs2gpo4", "0", 0},
			{"mcs2gpo5", "0", 0},
			{"mcs2gpo6", "0", 0},
			{"mcs2gpo7", "0", 0},
			{"bwduppo", "0", 0},
			{"aa2g", "7", 0},
			{"pa2gw2a0", "0", 0},
			{"pa2gw2a1", "0", 0},
			{"ccode", "ALL", 0},
			{"regrev", "0", 0},
			{"devid", "0x4329", 0},
			{"cck2gpo", "0", 0},
			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wnr834bv2_pci_1_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
			extra_params++;
		}
		break;

#ifdef HAVE_BCMMODERN
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_WRT320N:
	case ROUTER_ASUS_RTN16:
		basic_params = vlan_1_2;
		nvram_set("vlan2hwname", "et0");
		nvram_set("lan_ifnames", "vlan1 eth1");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan1ports", "1 2 3 4 8*")
		    || nvram_match("vlan2ports", "0 8u")) {
			nvram_set("vlan1ports", "4 3 2 1 8*");
			nvram_set("vlan2ports", "0 8");
			need_reboot = 1;
		}
		break;		
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
		if (nvram_get("clkfreq")==NULL) //set it only if it doesnt exist
			nvram_set("clkfreq","600");
		nvram_set("vlan1hwname", "et0");
		nvram_set("vlan2hwname", "et0");
		nvram_set("vlan1ports", "0 1 2 3 8*");
		nvram_set("vlan2ports", "4 8");
		
		/* now it goes evil */
		int mtd = getMTD("board_data");
		char devname[32];
		sprintf(devname, "/dev/mtdblock%d", mtd);
		FILE *fp = fopen(devname, "rb");
		if (!fp) {
			fprintf(stderr, "something wrong here, boarddata cannot be opened\n");
			break;
		}
		
		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
		    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
		    || !sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr"))
		    || startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
			unsigned char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/2/1/macaddr", mac);
			need_reboot = 1;
		}

		int isr6300 = 0;
		int iswndr4500v2 = 0;
		
#define R6300 "U12H218T00_NETGEAR"
#define WNDR4500V2 "U12H224T00_NETGEAR"

		char modelstr[32];
		fseek(fp, 0, SEEK_SET);
		fread(modelstr, 1, strlen(R6300), fp);
		if (!strncmp(modelstr, R6300, strlen(R6300))) {
			isr6300 = 1;
		}
		if (!strncmp(modelstr, WNDR4500V2, strlen(WNDR4500V2))) {
			iswndr4500v2 = 1;
		}
		fclose(fp);

		struct nvram_tuple wndr4500_pci_1_1_params[] = {

			{"pa2gw1a0", "0x1DFC", 0},
			{"pa2gw1a1", "0x1FF9", 0},
			{"pa2gw1a2", "0x1E58", 0},
			{"ledbh12", "11", 0},
			{"legofdmbw202gpo", "0x88000000", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"legofdmbw20ul2gpo", "0x88000000", 0},
			{"rxchain", "7", 0},
			{"cckbw202gpo", "0x0000", 0},
			{"mcsbw20ul2gpo", "0x88800000", 0},
			{"pa2gw0a0", "0xFE56", 0},
			{"pa2gw0a1", "0xFEB3", 0},
			{"pa2gw0a2", "0xFE6A", 0},
			{"boardflags", "0x80003200", 0},
			{"tempoffset", "0", 0},
			{"boardvendor", "0x14e4", 0},
			{"triso2g", "3", 0},
			{"sromrev", "9", 0},
			{"extpagain2g", "3", 0},
			{"venid", "0x14e4", 0},
			{"maxp2ga0", "0x62", 0},
			{"maxp2ga1", "0x62", 0},
			{"maxp2ga2", "0x62", 0},
			{"boardtype", "0x59b", 0},
			{"boardflags2", "0x4000000", 0},
			{"tssipos2g", "1", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"ledbh2", "11", 0},
			{"ledbh3", "11", 0},
			{"mcs32po", "0xA", 0},
			{"legofdm40duppo", "0x0", 0},
			{"antswctl2g", "0", 0},
			{"txchain", "7", 0},
			{"elna2g", "2", 0},
			{"antswitch", "0", 0},
			{"aa2g", "7", 0},
			{"cckbw20ul2gpo", "0x0000", 0},
			{"leddc", "0xFFFF", 0},
			{"pa2gw2a0", "0xF886", 0},
			{"pa2gw2a1", "0xF8AA", 0},
			{"pa2gw2a2", "0xF8A7", 0},
			{"pdetrange2g", "3", 0},
			{"devid", "0x4332", 0},
			{"tempthresh", "120", 0},
			{"mcsbw402gpo", "0x0x88800000", 0},
//                      {"macaddr", "84:1B:5E:46:25:2E", 0},
			{"mcsbw202gpo", "0x88800000", 0},

			{0, 0, 0}
		};

		struct nvram_tuple wndr4500_pci_2_1_params[] = {

			{"leddc", "0xFFFF", 0},
			{"txchain", "7", 0},
			{"maxp5gla0", "0x60", 0},
			{"elna5g", "1", 0},
			{"maxp5gla1", "0x60", 0},
			{"maxp5gla2", "0x60", 0},
			{"maxp5gha0", "0x72", 0},
			{"maxp5gha1", "0x72", 0},
			{"maxp5gha2", "0x72", 0},
			{"pa5gw0a0", "0xFE6C", 0},
			{"pa5gw0a1", "0xFE72", 0},
			{"pa5gw0a2", "0xFE75", 0},
			{"mcsbw20ul5gmpo", "0x22200000", 0},
			{"extpagain5g", "3", 0},
			{"pa5glw2a0", "0xFFFF", 0},
			{"boardflags", "0x90000200", 0},
			{"pa5glw2a1", "0xFFFF", 0},
			{"pa5glw2a2", "0xFFFF", 0},
			{"triso5g", "3", 0},
			{"tempoffset", "0", 0},
			{"mcsbw205gmpo", "0x22200000", 0},
			{"devid", "0x4333", 0},
			{"aa5g", "7", 0},
			{"pa5ghw2a0", "0xF8C5", 0},
			{"pa5ghw2a1", "0xF8D6", 0},
			{"pa5ghw2a2", "0xF8DA", 0},
//                      {"macaddr", "2C:B0:5D:46:78:01", 0},
			{"mcsbw20ul5glpo", "0x0", 0},
			{"pa5glw1a0", "0xFFFF", 0},
			{"pa5glw1a1", "0xFFFF", 0},
			{"pa5glw1a2", "0xFFFF", 0},
			{"mcsbw205glpo", "0x0", 0},
			{"mcsbw20ul5ghpo", "0x88800000", 0},
			{"legofdmbw205gmpo", "0x22000000", 0},
			{"ledbh12", "11", 0},
			{"mcsbw205ghpo", "0x88800000", 0},
			{"pa5ghw1a0", "0x1DD1", 0},
			{"pa5ghw1a1", "0x1DFF", 0},
			{"parefldovoltage", "35", 0},
			{"pa5ghw1a2", "0x1D76", 0},
			{"pa5gw2a0", "0xF8E9", 0},
			{"mcsbw405gmpo", "0x22200000", 0},
			{"pa5gw2a1", "0xF907", 0},
			{"pa5gw2a2", "0xF8ED", 0},
			{"boardtype", "0x5a9", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"ledbh2", "11", 0},
			{"legofdmbw20ul5gmpo", "0x22000000", 0},
			{"ledbh3", "11", 0},
			{"rxchain", "7", 0},
			{"pdetrange5g", "4", 0},
			{"legofdm40duppo", "0x0", 0},
			{"maxp5ga0", "0x66", 0},
			{"pa5glw0a0", "0xFFFF", 0},
			{"maxp5ga1", "0x66", 0},
			{"pa5glw0a1", "0xFFFF", 0},
			{"maxp5ga2", "0x66", 0},
			{"pa5glw0a2", "0xFFFF", 0},
			{"legofdmbw205glpo", "0x0", 0},
			{"venid", "0x14e4", 0},
			{"boardvendor", "0x14e4", 0},
			{"legofdmbw205ghpo", "0x88000000", 0},
			{"antswitch", "0", 0},
			{"tempthresh", "120", 0},
			{"pa5ghw0a0", "0xFE74", 0},
			{"pa5ghw0a1", "0xFE7F", 0},
			{"sromrev", "9", 0},
			{"pa5ghw0a2", "0xFE72", 0},
			{"antswctl5g", "0", 0},
			{"pa5gw1a0", "0x1D5E", 0},
			{"mcsbw405glpo", "0x0", 0},
			{"pa5gw1a1", "0x1D3D", 0},
			{"pa5gw1a2", "0x1DA8", 0},
			{"legofdmbw20ul5glpo", "0x0", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"mcsbw405ghpo", "0x88800000", 0},
			{"boardflags2", "0x4200000", 0},
			{"legofdmbw20ul5ghpo", "0x88000000", 0},
			{"mcs32po", "0x9", 0},
			{"tssipos5g", "1", 0},

			{0, 0, 0}
		};

		struct nvram_tuple wndr4500v2_pci_1_1_params[] = {

			{"pa2gw1a0", "0x1791", 0},
			{"pa2gw1a1", "0x189B", 0},
			{"pa2gw1a2", "0x173E", 0},
			{"ledbh12", "11", 0},
			{"legofdmbw202gpo", "0xECA64200", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"legofdmbw20ul2gpo", "0xECA64200", 0},
			{"rxchain", "7", 0},
			{"cckbw202gpo", "0x0000", 0},
			{"mcsbw20ul2gpo", "0xECA64200", 0},
			{"pa2gw0a0", "0xFE90", 0},
			{"pa2gw0a1", "0xFE9F", 0},
			{"pa2gw0a2", "0xFE8B", 0},
			{"boardflags", "0x80003200", 0},
			{"tempoffset", "0", 0},
			{"boardvendor", "0x14e4", 0},
			{"triso2g", "3", 0},
			{"sromrev", "9", 0},
			{"extpagain2g", "1", 0},
			{"venid", "0x14e4", 0},
			{"maxp2ga0", "0x5E", 0},
			{"maxp2ga1", "0x5E", 0},
			{"maxp2ga2", "0x5E", 0},
			{"boardtype", "0x59b", 0},
			{"boardflags2", "0x4100000", 0},
			{"tssipos2g", "1", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"ledbh2", "11", 0},
			{"ledbh3", "11", 0},
			{"mcs32po", "0xA", 0},
			{"legofdm40duppo", "0x0", 0},
			{"antswctl2g", "0", 0},
			{"txchain", "7", 0},
			{"elna2g", "2", 0},
			{"antswitch", "0", 0},
			{"aa2g", "7", 0},
			{"cckbw20ul2gpo", "0x0000", 0},
			{"leddc", "0xFFFF", 0},
			{"pa2gw2a0", "0xFA5C", 0},
			{"pa2gw2a1", "0xFA22", 0},
			{"pa2gw2a2", "0xFA7A", 0},
			{"pdetrange2g", "3", 0},
			{"devid", "0x4332", 0},
			{"tempthresh", "120", 0},
			{"mcsbw402gpo", "0xECAAAAAA", 0},
//                      {"macaddr", "00:00:00:00:00:01", 0},
			{"mcsbw202gpo", "0xECA64200", 0},

			{0, 0, 0}
		};

		struct nvram_tuple wndr4500v2_pci_2_1_params[] = {

			{"leddc", "0xFFFF", 0},
			{"txchain", "7", 0},
			{"maxp5gla0", "0x64", 0},
			{"elna5g", "1", 0},
			{"maxp5gla1", "0x64", 0},
			{"maxp5gla2", "0x64", 0},
			{"maxp5gha0", "0x5E", 0},
			{"maxp5gha1", "0x5E", 0},
			{"maxp5gha2", "0x5E", 0},
			{"pa5gw0a0", "0xFEB2", 0},
			{"pa5gw0a1", "0xFE7D", 0},
			{"pa5gw0a2", "0xFE78", 0},
			{"mcsbw20ul5gmpo", "0x42000000", 0},
			{"extpagain5g", "3", 0},
			{"pa5glw2a0", "0xF98F", 0},
			{"boardflags", "0x90000200", 0},
			{"pa5glw2a1", "0xF9C1", 0},
			{"pa5glw2a2", "0xF99D", 0},
			{"triso5g", "3", 0},
			{"tempoffset", "0", 0},
			{"mcsbw205gmpo", "0x42000000", 0},
			{"devid", "0x4333", 0},
			{"aa5g", "7", 0},
			{"pa5ghw2a0", "0xF9DC", 0},
			{"pa5ghw2a1", "0xFA04", 0},
			{"pa5ghw2a2", "0xF9EE", 0},
//                      {"macaddr", "00:00:00:00:00:10", 0},
			{"mcsbw20ul5glpo", "0x42000000", 0},
			{"pa5glw1a0", "0x1A5D", 0},
			{"pa5glw1a1", "0x1962", 0},
			{"pa5glw1a2", "0x19EC", 0},
			{"mcsbw205glpo", "0x20000000", 0},
			{"mcsbw20ul5ghpo", "0xECA64200", 0},
			{"legofdmbw205gmpo", "0x42000000", 0},
			{"ledbh12", "11", 0},
			{"mcsbw205ghpo", "0xECA64200", 0},
			{"pa5ghw1a0", "0x1896", 0},
			{"pa5ghw1a1", "0x1870", 0},
			{"parefldovoltage", "35", 0},
			{"pa5ghw1a2", "0x1883", 0},
			{"pa5gw2a0", "0xF93C", 0},
			{"mcsbw405gmpo", "0x42000000 ", 0},
			{"pa5gw2a1", "0xF99B", 0},
			{"pa5gw2a2", "0xF995", 0},
			{"boardtype", "0x5a9", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"ledbh2", "11", 0},
			{"legofdmbw20ul5gmpo", "0x42000000", 0},
			{"ledbh3", "11", 0},
			{"rxchain", "7", 0},
			{"pdetrange5g", "4", 0},
			{"legofdm40duppo", "0x0", 0},
			{"maxp5ga0", "0x4A", 0},
			{"pa5glw0a0", "0xFE7F", 0},
			{"maxp5ga1", "0x4A", 0},
			{"pa5glw0a1", "0xFE66", 0},
			{"maxp5ga2", "0x4A", 0},
			{"pa5glw0a2", "0xFE6B", 0},
			{"legofdmbw205glpo", "0x20000000", 0},
			{"venid", "0x14e4", 0},
			{"boardvendor", "0x14e4", 0},
			{"legofdmbw205ghpo", "0xECA64200", 0},
			{"antswitch", "0", 0},
			{"tempthresh", "120", 0},
			{"pa5ghw0a0", "0xFE53", 0},
			{"pa5ghw0a1", "0xFE68", 0},
			{"sromrev", "9", 0},
			{"pa5ghw0a2", "0xFE5D", 0},
			{"antswctl5g", "0", 0},
			{"pa5gw1a0", "0x1C6A", 0},
			{"mcsbw405glpo", "0x42000000", 0},
			{"pa5gw1a1", "0x1A47", 0},
			{"pa5gw1a2", "0x1A39", 0},
			{"legofdmbw20ul5glpo", "0x42000000", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"mcsbw405ghpo", "0xECA64200", 0},
			{"boardflags2", "0x4200000", 0},
			{"legofdmbw20ul5ghpo", "0xECA64200", 0},
			{"mcs32po", "0x9", 0},
			{"tssipos5g", "1", 0},

			{0, 0, 0}
		};

		struct nvram_tuple r6300_pci_1_1_params[] = {

			{"pa2gw1a0", "0x1D7C", 0},
			{"pa2gw1a1", "0x1F79", 0},
			{"pa2gw1a2", "0x1D58", 0},
			{"ledbh12", "11", 0},
			{"legofdmbw202gpo", "0x88000000", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"legofdmbw20ul2gpo", "0x88000000", 0},
			{"rxchain", "7", 0},
			{"cckbw202gpo", "0x0000", 0},
			{"mcsbw20ul2gpo", "0x88800000", 0},
			{"pa2gw0a0", "0xFE56", 0},
			{"pa2gw0a1", "0xFEB3", 0},
			{"pa2gw0a2", "0xFE6A", 0},
			{"boardflags", "0x80003200", 0},
			{"tempoffset", "0", 0},
			{"boardvendor", "0x14e4", 0},
			{"triso2g", "3", 0},
			{"sromrev", "9", 0},
			{"extpagain2g", "3", 0},
			{"venid", "0x14e4", 0},
			{"maxp2ga0", "0x62", 0},
			{"maxp2ga1", "0x62", 0},
			{"maxp2ga2", "0x62", 0},
			{"boardtype", "0x59b", 0},
			{"boardflags2", "0x4000000", 0},
			{"tssipos2g", "1", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"ledbh2", "11", 0},
			{"ledbh3", "11", 0},
			{"mcs32po", "0xA", 0},
			{"legofdm40duppo", "0x0", 0},
			{"antswctl2g", "0", 0},
			{"txchain", "7", 0},
			{"elna2g", "2", 0},
			{"antswitch", "0", 0},
			{"aa2g", "7", 0},
			{"cckbw20ul2gpo", "0x0000", 0},
			{"leddc", "0xFFFF", 0},
			{"pa2gw2a0", "0xF8A1", 0},
			{"pa2gw2a1", "0xF8BF", 0},
			{"pa2gw2a2", "0xF8DA", 0},
			{"pdetrange2g", "3", 0},
			{"devid", "0x4332", 0},
			{"tempthresh", "120", 0},
			{"mcsbw402gpo", "0x0x88800000", 0},
//                      {"macaddr", "84:1B:5E:3D:FD:D7", 0},
			{"mcsbw202gpo", "0x88800000", 0},

			{0, 0, 0}
		};

		struct nvram_tuple r6300_pci_2_1_params[] = {

			{"rxgains5ghtrisoa0", "5", 0},
			{"rxgains5ghtrisoa1", "4", 0},
			{"rxgains5ghtrisoa2", "4", 0},
			{"txchain", "7", 0},
			{"mcslr5gmpo", "0", 0},
			{"phycal_tempdelta", "255", 0},
			{"pdgain5g", "4", 0},
			{"maxp5gb3a0", "0x64", 0},
			{"subband5gver", "0x4", 0},
			{"maxp5gb3a1", "0x64", 0},
			{"maxp5gb3a2", "0x64", 0},
			{"boardflags", "0x10000000", 0},
			{"rxgainerr5g", "0xffff,0xffff,0xffff,0xffff", 0},
			{"tworangetssi5g", "0", 0},
			{"rxgains5gtrisoa0", "7", 0},
			{"rxgains5gtrisoa1", "6", 0},
			{"rxgains5gtrisoa2", "5", 0},
			{"tempoffset", "255", 0},
			{"mcsbw205gmpo", "4001780768", 0},
			{"xtalfreq", "40000", 0},
			{"devid", "0x43a0", 0},
			{"tempsense_option", "0x3", 0},
			{"femctrl", "3", 0},
			{"epagain2g", "0", 0},
			{"aa5g", "7", 0},
			{"rxgains2gelnagaina0", "0", 0},
			{"rxgains2gelnagaina1", "0", 0},
			{"rxgains2gelnagaina2", "0", 0},
			{"cckbw20ul2gpo", "0", 0},
			{"papdcap5g", "0", 0},
			{"tssiposslope5g", "1", 0},
			{"tempcorrx", "0x3f", 0},
			{"noiselvl5gua0", "31", 0},
			{"noiselvl5gua1", "31", 0},
			{"noiselvl5gua2", "31", 0},
			{"mcslr5glpo", "0", 0},
			{"mcsbw402gpo", "0", 0},
			{"sar5g", "15", 0},
//                      {"macaddr", "84:1B:5E:3D:FD:D6", 0},
			{"pa5ga0", "0xff39,0x1a55,0xfcc7,0xff38,0x1a7f,0xfcc3,0xff33,0x1a66,0xfcc4,0xff36,0x1a7b,0xfcc2", 0},
			{"rxgains5gmelnagaina0", "2", 0},
			{"pa5ga1", "0xff3a,0x1a0b,0xfcd3,0xff38,0x1a37,0xfccd,0xff37,0x1aa1,0xfcc0,0xff37,0x1a6f,0xfcc4", 0},
			{"rxgains5gmelnagaina1", "2", 0},
			{"pa5ga2", "0xff3a,0x1a28,0xfccd,0xff38,0x1a2a,0xfcce,0xff35,0x1a93,0xfcc1,0xff38,0x1aab,0xfcbe", 0},
			{"rxgains5gmelnagaina2", "3", 0},
			{"mcslr5ghpo", "0", 0},
			{"mcsbw202gpo", "0", 0},
			{"maxp5gb2a0", "0x64", 0},
			{"maxp5gb2a1", "0x64", 0},
			{"rxgains2gtrisoa0", "0", 0},
			{"maxp5gb2a2", "0x64", 0},
			{"rxgains2gtrisoa1", "0", 0},
			{"noiselvl5gma0", "31", 0},
			{"pcieingress_war", "15", 0},
			{"rxgains2gtrisoa2", "0", 0},
			{"noiselvl5gma1", "31", 0},
			{"noiselvl5gma2", "31", 0},
			{"sb40and80lr5gmpo", "0", 0},
			{"rxgains5gelnagaina0", "1", 0},
			{"rxgains5gelnagaina1", "1", 0},
			{"noiselvl2ga0", "31", 0},
			{"rxgains5gelnagaina2", "1", 0},
			{"noiselvl2ga1", "31", 0},
			{"noiselvl2ga2", "31", 0},
			{"agbg0", "71", 0},
			{"mcsbw205glpo", "3999687200", 0},
			{"agbg1", "71", 0},
			{"agbg2", "133", 0},
			{"measpower1", "0x7f", 0},
			{"sb20in80and160lr5gmpo", "0", 0},
			{"measpower2", "0x7f", 0},
			{"temps_period", "15", 0},
			{"mcsbw805gmpo", "4001780768", 0},
			{"dot11agduplrpo", "0", 0},
			{"mcsbw205ghpo", "3429122848", 0},
			{"measpower", "0x7f", 0},
			{"rxgains5ghelnagaina0", "2", 0},
			{"ofdmlrbw202gpo", "0", 0},
			{"rxgains5ghelnagaina1", "2", 0},
			{"rxgains5ghelnagaina2", "3", 0},
			{"gainctrlsph", "0", 0},
			{"sb40and80hr5gmpo", "0", 0},
			{"sb20in80and160hr5gmpo", "0", 0},
			{"mcsbw1605gmpo", "0", 0},
			{"pa2ga0", "0xfe72,0x14c0,0xfac7", 0},
			{"pa2ga1", "0xfe80,0x1472,0xfabc", 0},
			{"pa2ga2", "0xfe82,0x14bf,0xfad9", 0},
			{"epagain5g", "0", 0},
			{"mcsbw405gmpo", "4001780768", 0},
			{"boardtype", "0x621", 0},
			{"cckbw202gpo", "0", 0},
			{"rxchain", "7", 0},
			{"maxp5gb1a0", "0x64", 0},
			{"maxp5gb1a1", "0x64", 0},
			{"maxp5gb1a2", "0x64", 0},
			{"noiselvl5gla0", "31", 0},
			{"noiselvl5gla1", "31", 0},
			{"noiselvl5gla2", "31", 0},
			{"sb40and80lr5glpo", "0", 0},
			{"maxp5ga0", "92,96,96,96", 0},
			{"maxp5ga1", "92,96,96,96", 0},
			{"maxp5ga2", "92,96,96,96", 0},
			{"noiselvl5gha0", "31", 0},
			{"noiselvl5gha1", "31", 0},
			{"noiselvl5gha2", "31", 0},
			{"sb20in80and160lr5glpo", "0", 0},
			{"sb40and80lr5ghpo", "0", 0},
			{"venid", "0x14e4", 0},
			{"mcsbw805glpo", "3999687200", 0},
			{"pdgain2g", "4", 0},
			{"sar", "0x0F12", 0},
			{"boardvendor", "0x14e4", 0},
			{"sb20in80and160lr5ghpo", "0", 0},
			{"tempsense_slope", "0xff", 0},
			{"mcsbw805ghpo", "3429122848", 0},
			{"antswitch", "0", 0},
			{"aga0", "71", 0},
			{"aga1", "133", 0},
			{"rawtempsense", "0x1ff", 0},
			{"aga2", "133", 0},
			{"tempthresh", "255", 0},
			{"rxgainerr2g", "0xffff", 0},
			{"tworangetssi2g", "0", 0},
			{"dot11agduphrpo", "0", 0},
			{"sb40and80hr5glpo", "0", 0},
			{"sromrev", "11", 0},
			{"boardnum", "21059", 0},
			{"sb20in40lrpo", "0", 0},
			{"rxgains2gtrelnabypa0", "0", 0},
			{"rxgains2gtrelnabypa1", "0", 0},
			{"sb20in80and160hr5glpo", "0", 0},
			{"mcsbw1605glpo", "0", 0},
			{"rxgains2gtrelnabypa2", "0", 0},
			{"sb40and80hr5ghpo", "0", 0},
			{"mcsbw405glpo", "3999687200", 0},
			{"dot11agofdmhrbw202gpo", "0", 0},
			{"aa2g", "0", 0},
			{"boardrev", "0x1307", 0},
			{"rxgains5gmtrisoa0", "5", 0},
			{"sb20in80and160hr5ghpo", "0", 0},
			{"mcsbw1605ghpo", "0", 0},
			{"rxgains5gmtrisoa1", "4", 0},
			{"rxgains5gmtrisoa2", "4", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"maxp5gb0a0", "0x60", 0},
			{"rxgains5gmtrelnabypa0", "1", 0},
			{"ag2", "0", 0},
			{"papdcap2g", "0", 0},
			{"maxp5gb0a1", "0x60", 0},
			{"rxgains5gmtrelnabypa1", "1", 0},
			{"maxp5gb0a2", "0x60", 0},
			{"rxgains5gmtrelnabypa2", "1", 0},
			{"mcsbw405ghpo", "3429122848", 0},
			{"tssiposslope2g", "1", 0},
			{"maxp2ga0", "76", 0},
			{"maxp2ga1", "76", 0},
			{"maxp2ga2", "76", 0},
			{"boardflags2", "0x300002", 0},
			{"boardflags3", "0x300030", 0},
			{"rxgains5ghtrelnabypa0", "1", 0},
			{"rxgains5ghtrelnabypa1", "1", 0},
			{"rxgains5ghtrelnabypa2", "1", 0},
			{"sar2g", "18", 0},
			{"sb20in40hrrpo", "0", 0},
			{"temps_hysteresis", "15", 0},
			{"rxgains5gtrelnabypa0", "1", 0},
			{"rxgains5gtrelnabypa1", "1", 0},
			{"rxgains5gtrelnabypa2", "1", 0},

			{0, 0, 0}
		};

		/*
		 * set router's extra parameters 
		 */

		/* Restore defaults */
		if (isr6300) {
			struct nvram_tuple *t;
			t = r6300_pci_1_1_params;
			while (t->name) {
				nvram_nset(t->value, "pci/1/1/%s", t->name);
				t++;
			}
			t = r6300_pci_2_1_params;
			while (t->name) {
				nvram_nset(t->value, "pci/2/1/%s", t->name);
				t++;
			}

		} else if (iswndr4500v2) {
			struct nvram_tuple *t;
			t = wndr4500v2_pci_1_1_params;
			while (t->name) {
				nvram_nset(t->value, "pci/1/1/%s", t->name);
				t++;
			}
			t = wndr4500v2_pci_2_1_params;
			while (t->name) {
				nvram_nset(t->value, "pci/2/1/%s", t->name);
				t++;
			}

		} else {
			struct nvram_tuple *t;
			t = wndr4500_pci_1_1_params;
			while (t->name) {
				nvram_nset(t->value, "pci/1/1/%s", t->name);
				t++;
			}
			t = wndr4500_pci_2_1_params;
			while (t->name) {
				nvram_nset(t->value, "pci/2/1/%s", t->name);
				t++;
			}

		}
		
		if (nvram_match("wl0_country_code", "US"))
			set_regulation(0, "US", "0");
		else if (nvram_match("wl0_country_code", "Q2"))
			set_regulation(0, "US", "0");
		else if (nvram_match("wl0_country_code", "TW"))
			set_regulation(0, "TW", "13");
		else if (nvram_match("wl0_country_code", "CN"))
			set_regulation(0, "CN", "1");
		else
			set_regulation(0, "DE", "0");

		if (nvram_match("wl1_country_code", "Q2"))
			set_regulation(1, "US", "0");
		else if (nvram_match("wl1_country_code", "EU"))
			set_regulation(1, "EU", "13");
		else if (nvram_match("wl1_country_code", "TW"))
			set_regulation(1, "TW", "13");
		else if (nvram_match("wl1_country_code", "CN"))
			set_regulation(1, "CN", "1");
		else
			set_regulation(1, "US", "0");		

		break;
	case ROUTER_NETCORE_NW715P:
		basic_params = vlan_1_2;
		nvram_set("vlan0ports", " ");
		nvram_set("vlan0hwname", " ");
		break;
	case ROUTER_BELKIN_F5D8235V3:
		basic_params = vlan_1_2;
		if (nvram_match("vlan1ports", "0 8")
		    || nvram_match("vlan2ports", "1 2 3 4 8*")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8");
			need_reboot = 1;
		}
		break;

	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D4301:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan1ports", "4 8")
		    || nvram_match("vlan2ports", "0 1 2 3 8*")) {
			nvram_set("vlan1ports", "3 2 1 0 8*");
			nvram_set("vlan2ports", "4 8");
			need_reboot = 1;
		}
		break;

	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4302:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan1ports", "4 5*")
		    || nvram_match("vlan2ports", "0 1 2 3 5*")) {
			nvram_set("vlan1ports", "0 1 2 3 5*");
			nvram_set("vlan2ports", "4 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WNDR3400:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan2ports", "4 5u")
		    || nvram_match("vlan1ports", "0 1 2 3 5*")) {
			nvram_set("vlan1ports", "3 2 1 0 5*");
			nvram_set("vlan2ports", "4 5");
			need_reboot = 1;
		}
		if (startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4")
		    || startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4")) {
			unsigned char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}

		struct nvram_tuple wndr3400_sb_1_params[] = {

			{"sromrev", "8", 0},
			{"ccode", "ALL", 0},
			{"regrev", "0", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"ledbh2", "11", 0},
			{"ledbh3", "11", 0},
			{"ledbh9", "8", 0},
			{"leddc", "0xffff", 0},
			{"txchain", "3", 0},
			{"rxchain", "3", 0},
			{"antswitch", "0", 0},
			{"aa2g", "3", 0},
			{"ag0", "2", 0},
			{"ag1", "2", 0},
			{"itt2ga0", "0x20", 0},
			{"maxp2ga0", "0x48", 0},
			{"pa2gw0a0", "0xFEA5", 0},
			{"pa2gw1a0", "0x17B2", 0},
			{"pa2gw2a0", "0xFA73", 0},
			{"itt2ga1", "0x20", 0},
			{"maxp2ga1", "0x48", 0},
			{"pa2gw0a1", "0xfeba", 0},
			{"pa2gw1a1", "0x173c", 0},
			{"pa2gw2a1", "0xfa9b", 0},
			{"tssipos2g", "1", 0},
			{"extpagain2g", "2", 0},
			{"pdetrange2g", "2", 0},
			{"triso2g", "3", 0},
			{"antswctl2g", "2", 0},
			{"cck2gpo", "0x0000", 0},
			{"ofdm2gpo", "0x66666666", 0},
			{"mcs2gpo0", "0x6666", 0},
			{"mcs2gpo1", "0x6666", 0},
			{"mcs2gpo2", "0x6666", 0},
			{"mcs2gpo3", "0x6666", 0},
			{"mcs2gpo4", "0x6666", 0},
			{"mcs2gpo5", "0x6666", 0},
			{"mcs2gpo6", "0x6666", 0},
			{"mcs2gpo7", "0x6666", 0},
			{"cddpo", "0", 0},
			{"stbcpo", "0", 0},
			{"bw40po", "0", 0},
			{"bwduppo", "0", 0},

			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wndr3400_sb_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "sb/1/%s", extra_params->name);
			extra_params++;
		}

		struct nvram_tuple wndr3400_pci_1_1_params[] = {

			{"sromrev", "8", 0},
			{"ccode", "ALL", 0},
			{"regrev", "0", 0},
			{"ledbh0", "8", 0},
			{"ledbh1", "0x11", 0},
			{"ledbh2", "0x11", 0},
			{"ledbh3", "0x11", 0},
			{"leddc", "0xffff", 0},
			{"txchain", "3", 0},
			{"rxchain", "3", 0},
			{"antswitch", "0", 0},
			{"cddpo", "0", 0},
			{"stbcpo", "0", 0},
			{"bw40po", "0", 0},
			{"bwduppo", "0", 0},
			{"aa5g", "3", 0},
			{"ag0", "2", 0},
			{"ag1", "2", 0},
			{"itt5ga0", "0x3e", 0},
			{"maxp5ga0", "0x4A", 0},
			{"maxp5gha0", "0x4A", 0},
			{"maxp5gla0", "0x4A", 0},
			{"pa5gw0a0", "0xFEF9", 0},
			{"pa5gw1a0", "0x164B", 0},
			{"pa5gw2a0", "0xFADD", 0},
			{"pa5glw0a0", "0xFEF9", 0},
			{"pa5glw1a0", "0x154B", 0},
			{"pa5glw2a0", "0xFAFD", 0},
			{"pa5ghw0a0", "0xfeda", 0},
			{"pa5ghw1a0", "0x1612", 0},
			{"pa5ghw2a0", "0xfabe", 0},
			{"tssipos5g", "1", 0},
			{"extpagain5g", "2", 0},
			{"pdetrange5g", "4", 0},
			{"triso5g", "3", 0},
			{"antswctl2g", "0", 0},
			{"antswctl5g", "0", 0},
			{"itt5ga1", "0x3e", 0},
			{"maxp5ga1", "0x4A", 0},
			{"maxp5gha1", "0x4A", 0},
			{"maxp5gla1", "0x4A", 0},
			{"pa5gw0a1", "0xff31", 0},
			{"pa5gw1a1", "0x1697", 0},
			{"pa5gw2a1", "0xfb08", 0},
			{"pa5glw0a1", "0xFF31", 0},
			{"pa5glw1a1", "0x1517", 0},
			{"pa5glw2a1", "0xFB2F", 0},
			{"pa5ghw0a1", "0xff18", 0},
			{"pa5ghw1a1", "0x1661", 0},
			{"pa5ghw2a1", "0xfafe", 0},
			{"ofdm5gpo0", "0x0000", 0},
			{"ofdm5gpo1", "0x2000", 0},
			{"ofdm5glpo0", "0x0000", 0},
			{"ofdm5glpo1", "0x2000", 0},
			{"ofdm5ghpo0", "0x0000", 0},
			{"ofdm5ghpo1", "0x2000", 0},
			{"mcs5gpo0", "0x4200", 0},
			{"mcs5gpo1", "0x6664", 0},
			{"mcs5gpo2", "0x4200", 0},
			{"mcs5gpo3", "0x6664", 0},
			{"mcs5gpo4", "0x4200", 0},
			{"mcs5gpo5", "0x6664", 0},
			{"mcs5gpo6", "0x4200", 0},
			{"mcs5gpo7", "0x6664", 0},
			{"mcs5glpo0", "0x4200", 0},
			{"mcs5glpo1", "0x6664", 0},
			{"mcs5glpo2", "0x4200", 0},
			{"mcs5glpo3", "0x6664", 0},
			{"mcs5glpo4", "0x4200", 0},
			{"mcs5glpo5", "0x6664", 0},
			{"mcs5glpo6", "0x4200", 0},
			{"mcs5glpo7", "0x6664", 0},
			{"mcs5ghpo0", "0x4200", 0},
			{"mcs5ghpo1", "0x6664", 0},
			{"mcs5ghpo2", "0x4200", 0},
			{"mcs5ghpo3", "0x6664", 0},
			{"mcs5ghpo4", "0x4200", 0},
			{"mcs5ghpo5", "0x6664", 0},
			{"mcs5ghpo6", "0x4200", 0},
			{"mcs5ghpo7", "0x6664", 0},
			{"cdd5ghpo/cdd5glpo/cdd5gpo/cdd2gpo", "0x0", 0},
			{"stbc5ghpo/stbc5glpo/stbc5gpo/stbc2gpo", "0x0", 0},
			{"bw405ghpo/bw405glpo/bw405gpo/bw402gpo", "0x2", 0},
			{"wdup405ghpo/wdup405glpo/wdup405gpo/wdup402gpo", "0x0",
			 0},

			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wndr3400_pci_1_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
			extra_params++;
		}
		break;

	case ROUTER_NETGEAR_WNDR4000:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan2ports", "4 8u")) {
			nvram_set("vlan2ports", "4 8");
			need_reboot = 1;
		}
		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
		    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
		    || !sv_valid_hwaddr(nvram_safe_get("sb/1/macaddr"))
		    || startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4C")) {
			unsigned char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}

		struct nvram_tuple wndr4000_sb_1_params[] = {

			{"aa2g", "3", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"txchain", "3", 0},
			{"rxchain", "3", 0},
			{"antswitch", "0", 0},
			{"itt2ga0", "0x20", 0},
			{"itt2ga1", "0x20", 0},
			{"tssipos2g", "1", 0},
			{"extpagain2g", "3", 0},
			{"pdetrange2g", "5", 0},
			{"triso2g", "4", 0},
			{"antswctl2g", "2", 0},
			{"elna2g", "3", 0},

			{"pa2gw0a0", "0xFEA6", 0},
			{"pa2gw1a0", "0x191D", 0},
			{"pa2gw2a0", "0xFA18", 0},

			{"pa2gw0a1", "0xFE9E", 0},
			{"pa2gw1a1", "0x1809", 0},
			{"pa2gw2a1", "0xFA4B", 0},

			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wndr4000_sb_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "sb/1/%s", extra_params->name);
			extra_params++;
		}

		struct nvram_tuple wndr4000_pci_1_1_params[] = {

			{"boardflags2", "0x04000000", 0},
			{"legofdmbw20ul5ghpo", "0x11000000", 0},
			{"legofdmbw205ghpo", "0x11000000", 0},
			{"legofdm40duppo", "0x2222", 0},
			{"aa2g", "7", 0},
			{"aa5g", "7", 0},
			{"ag0", "0", 0},
			{"ag1", "0", 0},
			{"ag2", "0", 0},
			{"txchain", "7", 0},
			{"rxchain", "7", 0},
			{"antswitch", "0", 0},
			{"tssipos2g", "1", 0},
			{"extpagain2g", "0", 0},
			{"pdetrange2g", "4", 0},
			{"antswctl2g", "0", 0},
			{"tssipos5g", "1", 0},
			{"extpagain5g", "0", 0},
			{"pdetrange5g", "4", 0},
			{"triso5g", "1", 0},
			{"antswctl5g", "0", 0},

			{"pa2gw0a0", "0xFE6D", 0},
			{"pa2gw0a1", "0xFE72", 0},
			{"pa2gw0a2", "0xFE74", 0},
			{"pa2gw1a0", "0x1772", 0},
			{"pa2gw1a1", "0x1792", 0},
			{"pa2gw1a2", "0x1710", 0},
			{"pa2gw2a0", "0xFA34", 0},
			{"pa2gw2a1", "0xFA31", 0},
			{"pa2gw2a2", "0xFA4F", 0},
			{"maxp2ga0", "0x48", 0},
			{"maxp2ga1", "0x48", 0},
			{"maxp2ga2", "0x48", 0},

			{"pa5gw0a0", "0xFE82", 0},
			{"pa5gw0a1", "0xFE85", 0},
			{"pa5gw0a2", "0xFE7F", 0},
			{"pa5gw1a0", "0x1677", 0},
			{"pa5gw1a1", "0x167C", 0},
			{"pa5gw1a2", "0x1620", 0},
			{"pa5gw2a0", "0xFA72", 0},
			{"pa5gw2a1", "0xFA7A", 0},
			{"pa5gw2a2", "0xFA88", 0},
			{"maxp5ga0", "0x4E", 0},
			{"maxp5ga1", "0x4E", 0},
			{"maxp5ga2", "0x4E", 0},

			{"pa5ghw0a0", "0xFE9A", 0},
			{"pa5ghw0a1", "0xFE89", 0},
			{"pa5ghw0a2", "0xFE98", 0},
			{"pa5ghw1a0", "0x15E7", 0},
			{"pa5ghw1a1", "0x155F", 0},
			{"pa5ghw1a2", "0x15CD", 0},
			{"pa5ghw2a0", "0xFAAC", 0},
			{"pa5ghw2a1", "0xFAB0", 0},
			{"pa5ghw2a2", "0xFAB2", 0},
			{"maxp5gha0", "0x40", 0},
			{"maxp5gha1", "0x40", 0},
			{"maxp5gha2", "0x40", 0},

			{"pa5glw0a0", "0xFE97", 0},
			{"pa5glw0a1", "0xFE82", 0},
			{"pa5glw0a2", "0xFE84", 0},
			{"pa5glw1a0", "0x162F", 0},
			{"pa5glw1a1", "0x15ED", 0},
			{"pa5glw1a2", "0x167F", 0},
			{"pa5glw2a0", "0xFA98", 0},
			{"pa5glw2a1", "0xFA99", 0},
			{"pa5glw2a2", "0xFA84", 0},
			{"maxp5gla0", "0x48", 0},
			{"maxp5gla1", "0x48", 0},
			{"maxp5gla2", "0x48", 0},
			
			{"mcs32po", "0x2222", 0},

			{"legofdmbw205gmpo", "0x33221100", 0},
			{"legofdmbw20ul5gmpo", "0x33221100", 0},
			{"mcsbw205glpo", "0x11000000", 0},
			{"mcsbw205gmpo", "0x44221100", 0},
			{"mcsbw20ul5glpo", "0x11000000", 0},
			{"mcsbw20ul5gmpo", "0x44221100", 0},
			{"mcsbw405glpo", "0x33222222", 0},
			{"mcsbw405gmpo", "0x66443322", 0},


			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wndr4000_pci_1_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
			extra_params++;
		}
		break;

	case ROUTER_NETGEAR_WNR2000V2:
		basic_params = vlan_0_1;
		if (nvram_match("vlan0ports", "1 2 3 4 5*")
		    || nvram_match("vlan1ports", "0 5u")) {
			nvram_set("vlan0ports", "4 3 2 1 5*");
			nvram_set("vlan1ports", "0 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_RTN12:
		basic_params = vlan_0_1;
		eval("gpio", "enable", "0");
		if (!nvram_match("ledbh0", "0")
		    || !nvram_match("ledbh1", "0")) {
			nvram_set("ledbh0", "0");
			nvram_set("ledbh1", "0");
			need_reboot = 1;
		}
		if (nvram_match("vlan0ports", "0 1 2 3 5*")
		    || nvram_match("vlan1ports", "4 5u")) {
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_AC66U:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		struct nvram_tuple bcm4360ac_defaults[] = {
			{"pci/2/1/aa2g", "0", 0},
			{"pci/2/1/aa5g", "7", 0},
			{"pci/2/1/aga0", "2", 0},
			{"pci/2/1/aga1", "2", 0},
			{"pci/2/1/aga2", "0xff", 0},
			{"pci/2/1/agbg0", "2", 0},
			{"pci/2/1/agbg1", "2", 0},
			{"pci/2/1/agbg2", "0xff", 0},
			{"pci/2/1/antswitch", "0", 0},
			{"pci/2/1/cckbw202gpo", "0", 0},
			{"pci/2/1/cckbw20ul2gpo", "0", 0},
			{"pci/2/1/dot11agofdmhrbw202gpo", "0", 0},
			{"pci/2/1/femctrl", "3", 0},
			{"pci/2/1/papdcap2g", "0", 0},
			{"pci/2/1/tworangetssi2g", "0", 0},
			{"pci/2/1/pdgain2g", "4", 0},
			{"pci/2/1/epagain2g", "0", 0},
			{"pci/2/1/tssiposslope2g", "1", 0},
			{"pci/2/1/gainctrlsph", "0", 0},
			{"pci/2/1/papdcap5g", "0", 0},
			{"pci/2/1/tworangetssi5g", "0", 0},
			{"pci/2/1/pdgain5g", "4", 0},
			{"pci/2/1/epagain5g", "0", 0},
			{"pci/2/1/tssiposslope5g", "1", 0},
			{"pci/2/1/maxp2ga0", "76", 0},
			{"pci/2/1/maxp2ga1", "76", 0},
			{"pci/2/1/maxp2ga2", "76", 0},
			{"pci/2/1/mcsbw202gpo", "0", 0},
			{"pci/2/1/mcsbw402gpo", "0", 0},
			{"pci/2/1/measpower", "0x7f", 0},
			{"pci/2/1/measpower1", "0x7f", 0},
			{"pci/2/1/measpower2", "0x7f", 0},
			{"pci/2/1/noiselvl2ga0", "31", 0},
			{"pci/2/1/noiselvl2ga1", "31", 0},
			{"pci/2/1/noiselvl2ga2", "31", 0},
			{"pci/2/1/noiselvl5gha0", "31", 0},
			{"pci/2/1/noiselvl5gha1", "31", 0},
			{"pci/2/1/noiselvl5gha2", "31", 0},
			{"pci/2/1/noiselvl5gla0", "31", 0},
			{"pci/2/1/noiselvl5gla1", "31", 0},
			{"pci/2/1/noiselvl5gla2", "31", 0},
			{"pci/2/1/noiselvl5gma0", "31", 0},
			{"pci/2/1/noiselvl5gma1", "31", 0},
			{"pci/2/1/noiselvl5gma2", "31", 0},
			{"pci/2/1/noiselvl5gua0", "31", 0},
			{"pci/2/1/noiselvl5gua1", "31", 0},
			{"pci/2/1/noiselvl5gua2", "31", 0},
			{"pci/2/1/ofdmlrbw202gpo", "0", 0},
			{"pci/2/1/pa2ga0", "0xfe72,0x14c0,0xfac7", 0},
			{"pci/2/1/pa2ga1", "0xfe80,0x1472,0xfabc", 0},
			{"pci/2/1/pa2ga2", "0xfe82,0x14bf,0xfad9", 0},
			{"pci/2/1/pcieingress_war", "15", 0},
			{"pci/2/1/phycal_tempdelta", "255", 0},
			{"pci/2/1/rawtempsense", "0x1ff", 0},
			{"pci/2/1/rxchain", "7", 0},
			{"pci/2/1/rxgainerr2g", "0xffff", 0},
			{"pci/2/1/rxgainerr5g", "0xffff,0xffff,0xffff,0xffff",
			 0},
			{"pci/2/1/rxgains2gelnagaina0", "0", 0},
			{"pci/2/1/rxgains2gelnagaina1", "0", 0},
			{"pci/2/1/rxgains2gelnagaina2", "0", 0},
			{"pci/2/1/rxgains2gtrelnabypa0", "0", 0},
			{"pci/2/1/rxgains2gtrelnabypa1", "0", 0},
			{"pci/2/1/rxgains2gtrelnabypa2", "0", 0},
			{"pci/2/1/rxgains2gtrisoa0", "0", 0},
			{"pci/2/1/rxgains2gtrisoa1", "0", 0},
			{"pci/2/1/rxgains2gtrisoa2", "0", 0},
			{"pci/2/1/sar2g", "18", 0},
			{"pci/2/1/sar5g", "15", 0},
			{"pci/2/1/sromrev", "11", 0},
			{"pci/2/1/subband5gver", "0x4", 0},
			{"pci/2/1/tempcorrx", "0x3f", 0},
			{"pci/2/1/tempoffset", "255", 0},
			{"pci/2/1/temps_hysteresis", "15", 0},
			{"pci/2/1/temps_period", "15", 0},
			{"pci/2/1/tempsense_option", "0x3", 0},
			{"pci/2/1/tempsense_slope", "0xff", 0},
			{"pci/2/1/tempthresh", "255", 0},
			{"pci/2/1/txchain", "7", 0},
			{"pci/2/1/ledbh0", "2", 0},
			{"pci/2/1/ledbh1", "5", 0},
			{"pci/2/1/ledbh2", "4", 0},
			{"pci/2/1/ledbh3", "11", 0},
			{"pci/2/1/ledbh10", "7", 0},

			{0, 0, 0}
		};

		struct nvram_tuple *t;

		/* Restore defaults */
		for (t = bcm4360ac_defaults; t->name; t++) {
			if (!nvram_get(t->name))
				nvram_set(t->name, t->value);
		}

		nvram_set("pci/1/1/maxp2ga0", "0x70");
		nvram_set("pci/1/1/maxp2ga1", "0x70");
		nvram_set("pci/1/1/maxp2ga2", "0x70");

		nvram_set("pci/1/1/cckbw202gpo", "0x5555");
		nvram_set("pci/1/1/cckbw20ul2gpo", "0x5555");
		nvram_set("pci/1/1/legofdmbw202gpo", "0x97555555");
		nvram_set("pci/1/1/legofdmbw20ul2gpo", "0x97555555");
		nvram_set("pci/1/1/mcsbw202gpo", "0xDA755555");
		nvram_set("pci/1/1/mcsbw20ul2gpo", "0xDA755555");
		nvram_set("pci/1/1/mcsbw402gpo", "0xFC965555");

		nvram_set("pci/2/1/maxp5ga0", "104,104,104,104");
		nvram_set("pci/2/1/maxp5ga1", "104,104,104,104");
		nvram_set("pci/2/1/maxp5ga2", "104,104,104,104");

		nvram_set("pci/2/1/mcsbw205glpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw405glpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw805glpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw205gmpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw405gmpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw805gmpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw205ghpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw405ghpo", "0xBB975311");
		nvram_set("pci/2/1/mcsbw805ghpo", "0xBB975311");

		if (nvram_match("regulation_domain", "US"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "Q2"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "EU"))
			set_regulation(0, "DE", "0");
		else if (nvram_match("regulation_domain", "TW"))
			set_regulation(0, "TW", "13");
		else if (nvram_match("regulation_domain", "CN"))
			set_regulation(0, "CN", "1");
		else
			set_regulation(0, "US", "0");

		if (nvram_match("regulation_domain_5G", "US"))
			set_regulation(1, "US", "0");
		else if (nvram_match("regulation_domain_5G", "Q2"))
			set_regulation(1, "US", "0");
		else if (nvram_match("regulation_domain_5G", "EU"))
			set_regulation(1, "EU", "13");
		else if (nvram_match("regulation_domain_5G", "TW"))
			set_regulation(1, "TW", "13");
		else if (nvram_match("regulation_domain_5G", "CN"))
			set_regulation(1, "CN", "1");
		else
			set_regulation(1, "US", "0");

		nvram_set("pci/2/1/ledbh13", "136");
		eval("gpio", "disable", "13");
		break;

	case ROUTER_D1800H:
		if (nvram_get("ledbh0") == NULL || nvram_match("ledbh11", "130")) {
			nvram_set("ledbh0", "11");
			nvram_set("ledbh1", "11");
			nvram_set("ledbh2", "11");
			nvram_set("ledbh11", "136");
			need_reboot = 1;
		}
		nvram_set("pci/2/1/maxp2ga0", "0x70");
		nvram_set("pci/2/1/maxp2ga1", "0x70");
		nvram_set("pci/2/1/maxp2ga2", "0x70");
		nvram_set("pci/2/1/maxp5ga0", "0x6A");
		nvram_set("pci/2/1/maxp5ga1", "0x6A");
		nvram_set("pci/2/1/maxp5ga2", "0x6A");

		nvram_set("pci/2/1/cckbw202gpo", "0x5555");
		nvram_set("pci/2/1/cckbw20ul2gpo", "0x5555");
		nvram_set("pci/2/1/legofdmbw202gpo", "0x97555555");
		nvram_set("pci/2/1/legofdmbw20ul2gpo", "0x97555555");
		nvram_set("pci/2/1/mcsbw202gpo", "0xDA755555");
		nvram_set("pci/2/1/mcsbw20ul2gpo", "0xDA755555");
		nvram_set("pci/2/1/mcsbw402gpo", "0xFC965555");

		nvram_set("pci/2/1/cckbw205gpo", "0x5555");
		nvram_set("pci/2/1/cckbw20ul5gpo", "0x5555");
		nvram_set("pci/2/1/legofdmbw205gpo", "0x97555555");
		nvram_set("pci/2/1/legofdmbw20ul5gpo", "0x97555555");
		nvram_set("pci/2/1/legofdmbw205gmpo", "0x77777777");
		nvram_set("pci/2/1/legofdmbw20ul5gmpo", "0x77777777");
		nvram_set("pci/2/1/legofdmbw205ghpo", "0x77777777");
		nvram_set("pci/2/1/legofdmbw20ul5ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw205ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw20ul5ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw205gpo", "0xDA755555");
		nvram_set("pci/2/1/mcsbw20ul5gpo", "0xDA755555");
		nvram_set("pci/2/1/mcsbw405gpo", "0xFC965555");
		nvram_set("pci/2/1/mcsbw405ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw405ghpo", "0x77777777");
		nvram_set("pci/2/1/mcs32po", "0x7777");
		nvram_set("pci/2/1/legofdm40duppo", "0x0000");

		nvram_set("pci/1/1/maxp5ga0", "104,104,104,104");
		nvram_set("pci/1/1/maxp5ga1", "104,104,104,104");
		nvram_set("pci/1/1/maxp5ga2", "104,104,104,104");

		nvram_set("pci/1/1/mcsbw205glpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw405glpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw805glpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw205gmpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw405gmpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw805gmpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw205ghpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw405ghpo", "0xBB975311");
		nvram_set("pci/1/1/mcsbw805ghpo", "0xBB975311");

		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		break;
	case ROUTER_ASUS_RTN66:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan2ports", "0 8u")) {
			nvram_set("vlan2ports", "0 8");
			need_reboot = 1;
		}
		nvram_unset("maxp2ga0");
		nvram_unset("maxp2ga1");
		nvram_unset("maxp2ga2");
		nvram_unset("maxp5ga0");
		nvram_unset("maxp5ga1");
		nvram_unset("maxp5ga2");
		nvram_unset("maxp5gha0");
		nvram_unset("maxp5gha1");
		nvram_unset("maxp5gha2");
		nvram_set("pci/1/1/maxp2ga0", "0x70");
		nvram_set("pci/1/1/maxp2ga1", "0x70");
		nvram_set("pci/1/1/maxp2ga2", "0x70");
		nvram_set("pci/1/1/cckbw202gpo", "0x5555");
		nvram_set("pci/1/1/cckbw20ul2gpo", "0x5555");
		nvram_set("pci/1/1/legofdmbw202gpo", "0x97555555");
		nvram_set("pci/1/1/legofdmbw20ul2gpo", "0x97555555");
		nvram_set("pci/1/1/mcsbw202gpo", "0xFC955555");
		nvram_set("pci/1/1/mcsbw20ul2gpo", "0xFC955555");
		nvram_set("pci/1/1/mcsbw402gpo", "0xFFFF9999");
		nvram_set("pci/1/1/mcs32po", "0x9999");
		nvram_set("pci/1/1/legofdm40duppo", "0x4444");

		nvram_set("pci/2/1/maxp5ga0", "0x6A");
		nvram_set("pci/2/1/maxp5ga1", "0x6A");
		nvram_set("pci/2/1/maxp5ga2", "0x6A");
		nvram_set("pci/2/1/legofdmbw205gmpo", "0x77777777");
		nvram_set("pci/2/1/legofdmbw20ul5gmpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw205gmpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw20ul5gmpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw405gmpo", "0x77777777");
		nvram_set("pci/2/1/maxp5gha0", "0x6A");
		nvram_set("pci/2/1/maxp5gha1", "0x6A");
		nvram_set("pci/2/1/maxp5gha2", "0x6A");
		nvram_set("pci/2/1/legofdmbw205ghpo", "0x77777777");
		nvram_set("pci/2/1/legofdmbw20ul5ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw205ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw20ul5ghpo", "0x77777777");
		nvram_set("pci/2/1/mcsbw405ghpo", "0x77777777");
		nvram_set("pci/2/1/mcs32po", "0x7777");
		nvram_set("pci/2/1/legofdm40duppo", "0x0000");

		if (nvram_match("regulation_domain", "US"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "Q2"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "EU"))
			set_regulation(0, "DE", "0");
		else if (nvram_match("regulation_domain", "TW"))
			set_regulation(0, "TW", "13");
		else if (nvram_match("regulation_domain", "CN"))
			set_regulation(0, "CN", "1");
		else
			set_regulation(0, "US", "0");

		if (nvram_match("regulation_domain_5G", "US"))
			set_regulation(1, "US", "0");
		else if (nvram_match("regulation_domain_5G", "Q2"))
			set_regulation(1, "US", "0");
		else if (nvram_match("regulation_domain_5G", "EU"))
			set_regulation(1, "DE", "0");
		else if (nvram_match("regulation_domain_5G", "TW"))
			set_regulation(1, "TW", "13");
		else if (nvram_match("regulation_domain_5G", "CN"))
			set_regulation(1, "CN", "1");
		else
			set_regulation(1, "US", "0");

		break;

	case ROUTER_WRT310NV2:
		basic_params = vlan_1_2;
		nvram_set("vlan2hwname", "et0");
		if (nvram_match("vlan1ports", "1 2 3 4 8*"))
			nvram_set("vlan1ports", "4 3 2 1 8*");
		break;

	case ROUTER_WRT160NV3:
		basic_params = vlan_1_2;
		nvram_set("vlan2hwname", "et0");
		//fix lan port numbering on CSE41, CSE51
		if (nvram_match("clkdivsf", "4")
		    && nvram_match("vlan1ports", "1 2 3 4 5*")) {
			nvram_set("vlan1ports", "4 3 2 1 5*");
		}
		break;

	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1000V2:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
		basic_params = vlan_1_2;
		break;

	case ROUTER_LINKSYS_E2500:
	case ROUTER_LINKSYS_E3200:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		break;

	case ROUTER_LINKSYS_E4200:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr"))
		    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
		    || !sv_valid_hwaddr(nvram_safe_get("sb/1/macaddr"))
		    || startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4C")) {
			unsigned char mac[20];

			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}
		struct nvram_tuple e4200_pci_1_1_params[] = {
			{"pa2gw0a0", "0xfe8c", 0},
			{"pa2gw1a0", "0x1b20", 0},
			{"pa2gw2a0", "0xf98c", 0},
			{"pa2gw0a1", "0xfe98", 0},
			{"pa2gw1a1", "0x19ae", 0},
			{"pa2gw2a1", "0xf9ab", 0},

			{"pa5gw0a0", "0xfe52", 0},
			{"pa5gw1a0", "0x163e", 0},
			{"pa5gw2a0", "0xfa59", 0},
			{"pa5gw0a1", "0xfe63", 0},
			{"pa5gw1a1", "0x1584", 0},
			{"pa5gw2a1", "0xfa92", 0},
			{"pa5gw0a2", "0xfe7c", 0},
			{"pa5gw1a2", "0x1720", 0},
			{"pa5gw2a2", "0xfa4a", 0},

			{"pa5ghw0a0", "0xfe6a", 0},
			{"pa5ghw1a0", "0x163c", 0},
			{"pa5ghw2a0", "0xfa69", 0},
			{"pa5ghw0a1", "0xfe67", 0},
			{"pa5ghw1a1", "0x160e", 0},
			{"pa5ghw2a1", "0xfa6a", 0},
			{"pa5ghw0a2", "0xfe76", 0},
			{"pa5ghw1a2", "0x1766", 0},
			{"pa5ghw2a2", "0xfa2c", 0},
/*			
			{"pa5glw0a0", "0", 0},
			{"pa5glw1a0", "0", 0},
			{"pa5glw2a0", "0", 0},
			{"pa5glw0a1", "0", 0},
			{"pa5glw1a1", "0", 0},
			{"pa5glw2a1", "0", 0},
			{"pa5glw0a2", "0", 0},
			{"pa5glw1a2", "0", 0},
			{"pa5glw2a2", "0", 0},
*/

			{"pa05gidx", "5", 0},
			{"pa05glidx", "0", 0},
			{"pa05ghidx", "7", 0},
			{"pa15gidx", "0", 0},
			{"pa15glidx", "0", 0},
			{"pa15ghidx", "3", 0},
			{"pa25gidx", "5", 0},
			{"pa25glidx", "0", 0},
			{"pa25ghidx", "9", 0},

			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = e4200_pci_1_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
			extra_params++;
		}
		break;

#endif

	case ROUTER_NETGEAR_WNDR3300:
		if (nvram_match("force_vlan_supp", "enabled")) {
			nvram_set("lan_ifnames", "vlan0 eth2 eth3");
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");	//dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth2 eth3");	// dual radio
		}
		nvram_set("wan_ifname", "eth1");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("wl1_ifname", "eth3");
		eval("gpio", "disable", "7");

		if (nvram_get("pci/1/1/macaddr") == NULL || nvram_get("pci/1/3/macaddr") == NULL) {
			unsigned char mac[20];

			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/3/macaddr", mac);
			need_reboot = 1;
		}
		//params taken from firmware ver. 1.0.29 multi-region
		struct nvram_tuple wndr3300_pci_1_1_params[] = {
			{"stbcpo", "0", 0},
			{"mcs5gpo0", "0x4200", 0},
			{"pa2gw1a0", "0x14EA", 0},
			{"mcs5gpo1", "0x6664", 0},
			{"pa2gw1a1", "0x14DA", 0},
			{"mcs5gpo2", "0x4200", 0},
			{"maxp5gha0", "0x4A", 0},
			{"mcs5gpo3", "0x6664", 0},
			{"maxp5gha1", "0x4A", 0},
			{"mcs5gpo4", "0", 0},
			{"mcs5gpo5", "0", 0},
			{"mcs5gpo6", "0", 0},
			{"aa5g", "7", 0},
			{"mcs5gpo7", "0", 0},
			{"pa5glw2a0", "0xFBA2", 0},
			{"pa5glw2a1", "0xFBDB", 0},
			{"ag0", "2", 0},
			{"ag1", "2", 0},
			{"ag2", "2", 0},
			{"pa5gw2a0", "0xFBBA", 0},
			{"pa5gw2a1", "0xFC11", 0},
			{"pa5ghw2a0", "0xFBB5", 0},
			{"pa5ghw2a1", "0xFBD2", 0},
			{"ccdpo", "0", 0},
			{"txpid2ga0", "52", 0},
			{"itt5ga0", "0x3C", 0},
			{"rxchain", "3", 0},
			{"txpid2ga1", "51", 0},
			{"itt5ga1", "0x3C", 0},
			{"maxp5ga0", "0x4A", 0},
			{"maxp5ga1", "0x4A", 0},
			{"txpt2g", "0x48", 0},
			{"pa2gw0a0", "0xFEFC", 0},
			{"pa2gw0a1", "0xFF03", 0},
			{"boardflags", "0x0A00", 0},
			{"mcs5glpo0", "0x4200", 0},
			{"pa5glw1a0", "0x120E", 0},
			{"mcs5glpo1", "0x6664", 0},
			{"ofdm5gpo", "0x88888888", 0},
			{"pa5glw1a1", "0x12BD", 0},
			{"mcs5glpo2", "0x4200", 0},
			{"mcs5glpo3", "0x6664", 0},
			{"mcs5glpo4", "0", 0},
			{"mcs5glpo5", "0", 0},
			{"mcs5glpo6", "0", 0},
			{"mcs5glpo7", "0", 0},
			{"boardvendor", "0x14e4", 0},
			{"bw40po", "0", 0},
			{"sromrev", "4", 0},
			{"venid", "0x14e4", 0},
			{"pa5gw1a0", "0x1337", 0},
			{"pa5gw1a1", "0x14A4", 0},
			{"pa5ghw1a0", "0x11C2", 0},
			{"pa5ghw1a1", "0x1275", 0},
			{"boardrev", "0x13", 0},
			{"itt2ga0", "0x3E", 0},
			{"itt2ga1", "0x3E", 0},
			{"pa2gw3a0", "0", 0},
			{"pa2gw3a1", "0", 0},
			{"maxp2ga0", "0x4A", 0},
			{"maxp2ga1", "0x4A", 0},
			{"boardtype", "0x49C", 0},
			{"boardflags2", "0x0014", 0},
			{"ofdm2gpo", "0x66666666", 0},
			{"ledbh0", "11", 0},
			{"ledbh1", "11", 0},
			{"pa5glw0a0", "0xFEFB", 0},
			{"ledbh2", "11", 0},
			{"pa5glw0a1", "0xFF5B", 0},
			{"ledbh3", "11", 0},
			{"ledbh4", "11", 0},
			{"ledbh5", "5", 0},
			{"ledbh6", "7", 0},
			{"ledbh7", "11", 0},
			{"mcs2gpo0", "0x6666", 0},
			{"mcs2gpo1", "0x6666", 0},
			{"mcs2gpo2", "0x6666", 0},
			{"mcs2gpo3", "0x6666", 0},
			{"txpid5gla0", "18", 0},
			{"mcs2gpo4", "0", 0},
			{"txpid5gla1", "14", 0},
			{"mcs2gpo5", "0", 0},
			{"txpt5g", "0x3C", 0},
			{"mcs2gpo6", "0", 0},
			{"mcs2gpo7", "0", 0},
			{"mcs5ghpo0", "0x4200", 0},
			{"mcs5ghpo1", "0x6664", 0},
			{"bwduppo", "0", 0},
			{"mcs5ghpo2", "0x4200", 0},
			{"mcs5ghpo3", "0x6664", 0},
			{"txchain", "3", 0},
			{"mcs5ghpo4", "0", 0},
			{"mcs5ghpo5", "0", 0},
			{"txpid5gha0", "28", 0},
			{"mcs5ghpo6", "0", 0},
			{"ofdm5glpo", "0x88888888", 0},
			{"txpid5gha1", "25", 0},
			{"mcs5ghpo7", "0", 0},
			{"antswitch", "2", 0},
			{"aa2g", "7", 0},
			{"pa5gw0a0", "0xFF3C", 0},
			{"pa5gw0a1", "0xFFEC", 0},
			{"ofdm5ghpo", "0x88888888", 0},
			{"pa5ghw0a0", "0xFEE8", 0},
			{"pa5ghw0a1", "0xFF72", 0},
			{"leddc", "0xFFFF", 0},
			{"pa2gw2a0", "0xFB44", 0},
			{"pa2gw2a1", "0xFB28", 0},
			{"pa5glw3a0", "0", 0},
			{"pa5glw3a1", "0", 0},
			{"ccode", "0", 0},
			{"pa5gw3a0", "0", 0},
			{"regrev", "0", 0},
			{"pa5gw3a1", "0", 0},
			{"devid", "0x4328", 0},
			{"pa5ghw3a0", "0", 0},
			{"pa5ghw3a1", "0", 0},
			{"txpt5gh", "0x3C", 0},
			{"cck2gpo", "0x0000", 0},
			{"txpt5gl", "0x30", 0},
			{"maxp5gla0", "0x4A", 0},
			{"txpid5ga0", "39", 0},
			{"maxp5gla1", "0x4A", 0},
			{"txpid5ga1", "39", 0},
			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wndr3300_pci_1_1_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "pci/1/1/%s", extra_params->name);
			extra_params++;
		}

		struct nvram_tuple wndr3300_pci_1_3_params[] = {
			{"ag0", "0x02", 0},
			{"boardflags", "0xAA48", 0},
			{"ccode", "0", 0},
			{"aa0", "0x03", 0},
			{"devid", "0x4318", 0},
			{"pa0b0", "0x14ed", 0},
			{"pa0b1", "0xfac7", 0},
			{"pa0b2", "0xfe8a", 0},
			{"pa0itssit", "62", 0},
			{"pa0maxpwr", "0x0042", 0},
			{"opo", "0", 0},
			{"wl0gpio0", "11", 0},
			{"wl0gpio1", "11", 0},
			{"wl0gpio2", "11", 0},
			{"wl0gpio3", "7", 0},
			{"sromrev", "2", 0},
			{0, 0, 0}
		};
		/*
		 * set router's extra parameters 
		 */
		extra_params = wndr3300_pci_1_3_params;
		while (extra_params->name) {
			nvram_nset(extra_params->value, "pci/1/3/%s", extra_params->name);
			extra_params++;
		}
		break;

	case ROUTER_MOTOROLA_WE800G:
		nvram_set("lan_ifnames", "eth1 eth2");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("wan_ifname", "eth0");	// WAN to nonexist. iface.
		nvram_set("port_swap", "1");
		eval("gpio", "disable", "7");
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled"))	// = 
			// 
			// no 
			// lan
		{
			nvram_set("lan_ifnames", "eth2");
			nvram_set("wan_ifname", "eth1");
		}
		break;

	case ROUTER_MOTOROLA_V1:
	case ROUTER_BUFFALO_WZRG300N:
	case ROUTER_NETGEAR_WNR834B:
	case ROUTER_WRT300N:
	case ROUTER_ASUS_WL500W:
	case ROUTER_BUFFALO_WLAH_G54:
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
	case ROUTER_MICROSOFT_MN700:
		nvram_set("wan_ifname", "eth1");
		break;

	case ROUTER_WRT150N:
		nvram_set("wan_ifname", "eth1");
		if (nvram_match("force_vlan_supp", "enabled")) {
			nvram_set("lan_ifnames", "vlan0 eth2");
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");	//dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth2");
		}
		break;

	case ROUTER_WRTSL54GS:
	case ROUTER_WRT160N:
		nvram_set("wan_ifname", "eth1");
		if (nvram_match("force_vlan_supp", "enabled")) {
			nvram_set("lan_ifnames", "vlan0 eth2");
			nvram_set("vlan0ports", "0 1 2 3 5*");
			nvram_set("vlan1ports", "4 5");	//dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth2");
		}
		break;

	case ROUTER_WRT54G1X:
		if (check_vlan_support()) {
			nvram_set("lan_ifnames", "vlan0 eth2");
			nvram_set("wan_ifname", "vlan1");
		}
		break;

	case ROUTER_WRT350N:
	case ROUTER_WRT310N:
	case ROUTER_WRT600N:
		nvram_set("wan_ifname", "vlan2");
		break;

	case ROUTER_WRT610N:
		nvram_set("wan_ifname", "vlan2");
		nvram_set("pci/1/1/ledbh0", "11");
		nvram_set("pci/1/1/ledbh1", "135");
		nvram_set("pci/1/2/ledbh0", "11");
		nvram_set("pci/1/2/ledbh2", "135");
		nvram_set("pci/1/1/boardflags2", "0x0400");
		nvram_set("pci/1/2/boardflags2", "0x0602");

		if (!nvram_match("bootnv_ver", "6")) {
			if (startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
			    || startswith(nvram_safe_get("pci/1/2/macaddr"), "00:90:4C")) {
				unsigned char mac[20];
				strcpy(mac, nvram_safe_get("et0macaddr"));
				MAC_ADD(mac);
				MAC_ADD(mac);
				nvram_set("pci/1/1/macaddr", mac);
				MAC_ADD(mac);
				nvram_set("pci/1/2/macaddr", mac);
				need_reboot = 1;
			}
		}
		break;

#ifdef HAVE_BCMMODERN
	case ROUTER_WRT610NV2:
		nvram_set("wan_ifname", "vlan2");
		nvram_set("vlan2hwname", "et0");
		nvram_set("pci/1/1/ledbh2", "8");
		nvram_set("sb/1/ledbh1", "8");
		if (nvram_match("vlan1ports", "1 2 3 4 8*"))
			nvram_set("vlan1ports", "4 3 2 1 8*");
		if (startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C")
		    || startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4c")) {
			unsigned char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}
		break;
#endif

	case ROUTER_WRT300NV11:
	case ROUTER_BUFFALO_WZRG144NH:
		nvram_set("wan_ifname", "vlan1");
		break;

	case ROUTER_ASUS_WL500G_PRE:
	case ROUTER_ASUS_WL700GE:
		nvram_set("lan_ifnames", "vlan0 eth2");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("wan_ifname", "vlan1");	// fix for Asus WL500gPremium 
		// 
		// WAN problem.
		if (nvram_match("vlan1ports", "0 5u")) {
			nvram_set("vlan1ports", "0 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL550GE:
		nvram_set("wl0_ifname", "eth1");
		break;

	case ROUTER_BUFFALO_WLA2G54C:
	case ROUTER_WAP54G_V2:
	case ROUTER_VIEWSONIC_WAPBR_100:
	case ROUTER_USR_5430:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_NETGEAR_WG602_V4:
	case ROUTER_ASUS_330GE:
		nvram_set("lan_ifnames", "eth0 eth1");
		nvram_set("wl0_ifname", "eth1");
		nvram_set("wan_ifname", "eth2");	// map WAN port to
		// nonexistant interface
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled"))	// = 
			// 
			// no 
			// lan
		{
			nvram_set("lan_ifnames", "eth1");
			nvram_set("wan_ifname", "eth0");
		}
		break;

	case ROUTER_BELKIN_F5D7230_V3000:
		if (nvram_match("vlan1ports", "4 5u"))
			nvram_set("vlan1ports", "4 5");
		break;

	case ROUTER_DYNEX_DX_NRUTER:
		nvram_set("lan_ifnames", "vlan0 eth2");
		nvram_set("wan_ifname", "vlan1");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("pci/1/1/ledbh0", "136");
		if (nvram_match("vlan1ports", "\"4 5*\"")) {
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_DELL_TRUEMOBILE_2300_V2:	// we must fix cfe defaults
		// with CR added
		nvram_set("vlan0hwname", "et0");
		nvram_set("vlan1hwname", "et0");
		nvram_set("et0mdcport", "0");
		nvram_set("et0phyaddr", "30");
		nvram_set("gpio2", "adm_eecs");
		nvram_set("gpio3", "adm_eesk");
		nvram_set("gpio4", "adm_eedi");
		nvram_set("gpio5", "adm_rc");
		nvram_unset("gpio6");
		break;

	case ROUTER_ASUS_WL520G:
	case ROUTER_ASUS_WL500G_PRE_V2:
	case ROUTER_WRT54G_V81:
		if (nvram_match("vlan1ports", "4 5u"))
			nvram_set("vlan1ports", "4 5");
		break;

	case ROUTER_ASUS_WL520GUGC:
		if (nvram_match("vlan1ports", "0 5u"))
			nvram_set("vlan1ports", "0 5");
		if (!nvram_get("Fix_WL520GUGC_clock")) {
			nvram_set("Fix_WL520GUGC_clock", "1");
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		if (nvram_match("vlan1ports", "0 5u"))
			nvram_set("vlan1ports", "0 5");
		if (nvram_match("sromrev", "2")
		    && nvram_match("boardrev", "0x10")
		    && nvram_match("boardtype", "0x48E")) {
			nvram_set("sromrev", "3");	// This is a fix for WGR614L NA - which has a wrong sromrev
			need_reboot = 1;
		}
		break;

	case ROUTER_ALLNET01:
		nvram_set("wl0_ifname", "eth1");
		if (nvram_match("vlan1ports", "5u"))	//correct bad parameters
		{
			nvram_set("vlan1ports", "4 5");
			nvram_set("vlan0ports", "0 1 2 3 5*");
		}
		break;

	case ROUTER_LINKSYS_WTR54GS:
		eval("gpio", "enable", "3");	// prevent reboot loop on
		// reset
		break;

	case ROUTER_WAP54G_V3:
		eval("gpio", "enable", "0");	// reset gpio 0 for reset
		// button 
		// nvram_set ("vlan0ports", "1 5*");
		// nvram_set ("vlan1ports", "4 5");
		// if (nvram_match ("wan_to_lan", "yes") && nvram_invmatch
		// ("wan_proto", "disabled")) // = no lan
		// {
		// nvram_set ("vlan0ports", "4 5*");
		// nvram_set ("vlan1ports", "1 5");
		// }
		break;

	}

#if 0
	/*
	 * fix il0macaddr to be lanmac+2 
	 */
	if (nvram_get("il0macaddr") == NULL)
		need_reboot = 1;

	unsigned char mac[20];

	if (nvram_match("port_swap", "1"))
		strcpy(mac, nvram_safe_get("et1macaddr"));
	else
		strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	MAC_ADD(mac);
	nvram_set("il0macaddr", mac);
#endif

	/*
	 * set router's basic parameters 
	 */
	while (basic_params && basic_params->name) {
		nvram_set(basic_params->name, basic_params->value);
		basic_params++;
	}

	/*
	 * ifnames 
	 */
	if (!nvram_match("wan_ifname", "")) {
		strcpy(wanifname, nvram_safe_get("wan_ifname"));
		nvram_set("wan_ifname", wanifname);
		nvram_set("wan_ifname2", wanifname);
		nvram_set("wan_ifnames", wanifname);
		nvram_set("wan_default", wanifname);
		nvram_set("pppoe_wan_ifname", wanifname);
		nvram_set("pppoe_ifname", wanifname);
	}

	strcpy(wlifname, nvram_safe_get("wl0_ifname"));

	/*
	 * set wan_ifnames, pppoe_wan_ifname and pppoe_ifname 
	 */

	/*
	 * MAC address sdjustments 
	 */
	switch (brand) {
	case ROUTER_ALLNET01:
	case ROUTER_BELKIN_F5D7231_V2000:

		if (!nvram_match("no_sercom", "1")) {
			//fix mac
			unsigned char mac[6];
			FILE *in = fopen("/dev/mtdblock/0", "rb");

			if (in != NULL)	//special sercom mac address handling
			{
				fseek(in, 0x1ffa0, SEEK_SET);
				fread(mac, 6, 1, in);
				fclose(in);
				char macstr[32];

				sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X", (int)mac[0] & 0xff, (int)mac[1] & 0xff, (int)mac[2] & 0xff, (int)mac[3] & 0xff, (int)mac[4] & 0xff, (int)mac[5] & 0xff);
				nvram_set("et0macaddr", macstr);
				eval("ifconfig", "eth0", "hw", "ether", macstr);
			}
		}
		break;
	}

	/*
	 * Must have stuff 
	 */
	switch (brand) {
	case ROUTER_WRT320N:
		if (!nvram_match("reset_gpio", "5")) {
			nvram_set("reset_gpio", "5");
			need_reboot = 1;
		}
		break;

	case ROUTER_MOTOROLA_V1:
		eval("gpio", "disable", "7");
		break;

	case ROUTER_WRT54G_V8:
		nvram_set("reset_gpio", "7");
		break;

	case ROUTER_ASUS_WL700GE:
		eval("gpio", "enable", "3");	// POWER-enable, turns on power to HDD and switch leds
		break;
	}

	/*
	 * additional boardflags adjustment, etc...
	 */
	switch (brand) {
	case ROUTER_BELKIN_F5D7231:
		if (nvram_match("boardflags", "0x388")
		    || nvram_match("boardflags", "0x0388")) {
			nvram_set("boardflags", "0x0f58");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASKEY_RT220XD:
		if (nvram_match("boardflags", "0x388")
		    || nvram_match("boardflags", "0x0388")) {
			nvram_set("boardflags", "0x0208");
			need_reboot = 1;
		}
		break;

	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		if (!nvram_match("buffalo_hp", "1")
		    && (nvram_match("boardflags", "0x1658")
			|| nvram_match("boardflags", "0x2658"))) {
			nvram_set("buffalo_hp", "1");
#ifndef HAVE_BUFFALO		// if HAVE_BUFFALO not used to be FCC/CE
			// valid
			nvram_set("boardflags", "0x3658");	// enable high gain
			// PA
			need_reboot = 1;
#endif
		}
		break;

	case ROUTER_BUFFALO_WHRG54S:	// for HP only
		if (!nvram_match("buffalo_hp", "1")
		    && nvram_match("boardflags", "0x1758")) {
			nvram_set("buffalo_hp", "1");
#ifndef HAVE_BUFFALO		// if HAVE_BUFFALO not used to be FCC/CE
			// valid
			nvram_set("boardflags", "0x3758");	// enable high gain
			// PA
			need_reboot = 1;
#endif
		}
		break;

	case ROUTER_WRTSL54GS:
		if (nvram_match("force_vlan_supp", "enabled")
		    && nvram_match("boardflags", "0x0018")) {
			nvram_set("boardflags", "0x0118");	//enable lan vlans
			need_reboot = 1;
		} else if (!nvram_match("force_vlan_supp", "enabled")
			   && nvram_match("boardflags", "0x0118")) {
			nvram_set("boardflags", "0x0018");	//disable vlans
			need_reboot = 1;
		}
		break;

	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
		if (nvram_match("force_vlan_supp", "enabled")
		    && nvram_match("boardflags", "0x0010")) {
			nvram_set("boardflags", "0x0110");	//enable lan vlans
			need_reboot = 1;
		} else if (!nvram_match("force_vlan_supp", "enabled")
			   && nvram_match("boardflags", "0x0110")) {
			nvram_set("boardflags", "0x0010");	//disable vlans
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WNR834BV2:
	case ROUTER_NETGEAR_WNDR3300:
		if (nvram_match("force_vlan_supp", "enabled")
		    && nvram_match("boardflags", "0x10")) {
			nvram_set("boardflags", "0x110");	//enable lan vlans
			need_reboot = 1;
		} else if (!nvram_match("force_vlan_supp", "enabled")
			   && nvram_match("boardflags", "0x110")) {
			nvram_set("boardflags", "0x10");	//disable vlans
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WG602_V4:
		if (nvram_match("boardflags", "0x650")) {
			nvram_set("boardflags", "0x0458");
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WNR3500L:	//usb power fix (gpio 12)
		if (nvram_match("boardflags", "0x00001710")) {
			nvram_set("boardflags", "0x00000710");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_WL500G_PRE:
		if (nvram_match("sdram_init", "0x000b"))
			nvram_set("sdram_init", "0x0009");
		break;

	}
	
#ifdef HAVE_80211AC
	if ( nvram_get("et_txq_thresh") == NULL ) {
		nvram_set("et_txq_thresh","1024");
//		nvram_set("et_dispatch_mode","1"); 1=better throughput 0=better ping
	}
#endif	

	if (need_reboot) {
		nvram_commit();
		fprintf(stderr, "Need reboot now .....\n");
		sys_reboot();
	}

	/*
	 * Modules 
	 */
	uname(&name);

	snprintf(buf, sizeof(buf), "/lib/modules/%s", name.release);
	if (stat("/proc/modules", &tmp_stat) == 0 && stat(buf, &tmp_stat) == 0) {
		char module[80], *modules = "", *next;

#ifdef HAVE_ACK
		nvram_set("portprio_support", "0");	// no portprio support in NEWD or BCMMODERN
#else
		nvram_set("portprio_support", "1");	// only switch drivers in VINT support this
#endif

		if (check_vlan_support() && check_hw_type() != BCM5325E_CHIP) {
			switch (brand) {
			case ROUTER_WRT310N:
			case ROUTER_WRT310NV2:
			case ROUTER_WRT320N:
			case ROUTER_WRT350N:
			case ROUTER_WRT600N:
			case ROUTER_WRT610N:
			case ROUTER_WRT610NV2:
			case ROUTER_WRT300NV11:
			case ROUTER_BUFFALO_WZRG144NH:
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_BELKIN_F7D3301:
			case ROUTER_BELKIN_F7D4301:
			case ROUTER_BELKIN_F5D8235V3:
			case ROUTER_LINKSYS_E3200:
			case ROUTER_LINKSYS_E4200:
			case ROUTER_NETGEAR_WNDR4000:
				nvram_set("portprio_support", "0");
#ifdef HAVE_BCMMODERN
				modules = "bcm57xx switch-core switch-robo";
#else
				modules = "bcm57xxlsys switch-core switch-robo";
#endif
				break;
			case ROUTER_ASUS_RTN16:
			case ROUTER_ASUS_RTN66:
			case ROUTER_ASUS_AC66U:
			case ROUTER_LINKSYS_EA2700:
			case ROUTER_LINKSYS_EA6500:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_R6300:
			case ROUTER_D1800H:
				modules = "et switch-core switch-robo";
				break;
			case ROUTER_LINKSYS_WRT55AG:
			case ROUTER_MOTOROLA:
			case ROUTER_BUFFALO_WBR2G54S:
			case ROUTER_DELL_TRUEMOBILE_2300_V2:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "switch-core switch-adm";
				break;

			case ROUTER_WRT54G_V8:
			case ROUTER_WRT54G_V81:
			case ROUTER_LINKSYS_WRH54G:
			case ROUTER_ASUS_WL520G:
			case ROUTER_ASUS_WL520GUGC:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "switch-core switch-robo";
				break;

			case ROUTER_WRT54G1X:
			case ROUTER_WRT54G:
				insmod("switch-core");
				if (insmod("switch-robo"))
					insmod("switch-adm");
				break;

			case ROUTER_RT480W:
			case ROUTER_BUFFALO_WLI2_TX1_G54:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "";
				insmod("switch-core");
				if (insmod("switch-robo"))
					insmod("switch-adm");
				break;

			case ROUTER_WRT54G3G:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "switch-core switch-robo pcmcia_core yenta_socket ds serial_cs usbcore usb-ohci usbserial sierra";
				break;
			case ROUTER_ASUS_RTN10:
			case ROUTER_ASUS_RTN10PLUSD1:
			case ROUTER_ASUS_RTN10U:
			case ROUTER_ASUS_RTN53:
			case ROUTER_NETCORE_NW715P:
			case ROUTER_ASUS_RTN12B:
			case ROUTER_LINKSYS_E1000V2:
			case ROUTER_LINKSYS_E800:
			case ROUTER_LINKSYS_E900:
			case ROUTER_LINKSYS_E1500:
			case ROUTER_LINKSYS_E1550:
			case ROUTER_LINKSYS_E2500:
				nvram_set("portprio_support", "0");
				modules = "";
				break;
			default:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "switch-core switch-robo";
				break;
			}
		} else {
			switch (brand) {
			case ROUTER_WRT310N:
			case ROUTER_WRT310NV2:
			case ROUTER_WRT320N:
			case ROUTER_WRT350N:
			case ROUTER_WRT600N:
			case ROUTER_WRT610N:
			case ROUTER_WRT610NV2:
			case ROUTER_WRT300NV11:
			case ROUTER_BUFFALO_WZRG144NH:
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_LINKSYS_E3200:
			case ROUTER_LINKSYS_E4200:
			case ROUTER_NETGEAR_WNDR4000:
				nvram_set("portprio_support", "0");
#ifdef HAVE_BCMMODERN
				modules = "bcm57xx switch-core switch-robo";
#else
				modules = "bcm57xxlsys switch-core switch-robo";
#endif
				break;
			case ROUTER_ASUS_AC66U:
			case ROUTER_ASUS_RTN16:
			case ROUTER_ASUS_RTN66:
			case ROUTER_LINKSYS_EA2700:
			case ROUTER_LINKSYS_EA6500:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_R6300:
			case ROUTER_D1800H:
				modules = "et switch-core switch-robo";
				break;
			case ROUTER_LINKSYS_WRT55AG:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "switch-core switch-adm";

				break;
			case ROUTER_ASUS_WL500GD:
			case ROUTER_ASUS_WL550GE:
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "switch-core switch-robo";
				break;
			case ROUTER_BUFFALO_WZRRSG54:
				nvram_set("portprio_support", "0");
				modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules")
				    : "";
				break;
			case ROUTER_WRT54G3G:
				if (check_vlan_support())
					modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules") : "switch-core switch-robo pcmcia_core yenta_socket ds";
				else {
					nvram_set("portprio_support", "0");

					modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules") : "pcmcia_core yenta_socket ds";
				}
				break;
			case ROUTER_ASUS_RTN10:
			case ROUTER_ASUS_RTN10PLUSD1:
			case ROUTER_ASUS_RTN10U:
			case ROUTER_ASUS_RTN53:
			case ROUTER_NETCORE_NW715P:
			case ROUTER_ASUS_RTN12B:
			case ROUTER_LINKSYS_E1000V2:
			case ROUTER_LINKSYS_E800:
			case ROUTER_LINKSYS_E900:
			case ROUTER_LINKSYS_E1500:
			case ROUTER_LINKSYS_E1550:
			case ROUTER_LINKSYS_E2500:
				nvram_set("portprio_support", "0");
				modules = "";
				break;

			default:
#ifndef HAVE_BCMMODERN
				if (check_vlan_support())
					modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules") : "switch-core switch-robo";
				else
#endif
				{
					nvram_set("portprio_support", "0");
					modules = nvram_invmatch("ct_modules", "") ? nvram_safe_get("ct_modules") : "switch-core switch-robo";
				}
				break;
			}
		}
//      fprintf( "insmod %s\n", modules );

		foreach(module, modules, next) {
#ifdef HAVE_MACBIND
			if (nvram_match("et0macaddr", MACBRAND))
				insmod(module);
#else

			fprintf(stderr, "loading %s\n", module);
			insmod(module);
			cprintf("done\n");
#endif
		}

		if (check_hw_type() == BCM4702_CHIP)
			insmod("diag");

		loadWlModule();

	}
	/*
	 * Set a sane date 
	 */
	stime(&tm);

	led_control(LED_POWER, LED_ON);
	led_control(LED_DIAG, LED_OFF);
	led_control(LED_SES, LED_OFF);
	led_control(LED_SES2, LED_OFF);
	led_control(LED_BRIDGE, LED_OFF);
	led_control(LED_WLAN0, LED_OFF);
	led_control(LED_WLAN1, LED_OFF);
	led_control(LED_CONNECTED, LED_OFF);

	if (brand == ROUTER_WRT54G3G) {
		eval("cardmgr");
	}

	cprintf("done\n");
	return;

}


int check_cfe_nv(void)
{
	int ret = 0;

	switch (getRouterBrand()) {
	case ROUTER_BUFFALO_WZRRSG54:
		ret += check_nv("lan_hwnames", "et0 wl0");
		ret += check_nv("wan_hwname", "et1");
		ret += check_nv("vlans", "0");
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		ret += check_nv("aa0", "3");

		ret += check_nv("pa0itssit", "62");
		ret += check_nv("pa0b0", "0x1136");
		ret += check_nv("pa0b1", "0xfb93");
		ret += check_nv("pa0b2", "0xfea5");
		ret += check_nv("wl0gpio2", "0");
		ret += check_nv("wl0gpio3", "0");
		ret += check_nv("cctl", "0");
		ret += check_nv("ccode", "0");
		break;
#ifndef HAVE_BUFFALO

	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_WRT54G_V81:
		ret += check_nv("aa0", "3");
		/*
		 * if (check_hw_type () == BCM5352E_CHIP || check_hw_type () ==
		 * BCM5354G_CHIP) ret += check_nv ("ag0", "0x02"); else ret +=
		 * check_nv ("ag0", "255");
		 */
		if (check_hw_type() == BCM5325E_CHIP) {
			/*
			 * Lower the DDR ram drive strength , the value will be
			 * stable for all boards Latency 3 is more stable for all ddr 
			 * 20050420 by honor 
			 */

			ret += check_nv("sdram_init", "0x010b");
			ret += check_nv("sdram_config", "0x0062");

			if (nvram_match("clkfreq", "200")
			    && nvram_match("overclocking", "200")) {
				ret += check_nv("clkfreq", "216");
				nvram_set("overclocking", "216");
			}

			if (ret) {
				nvram_set("sdram_ncdl", "0x0");

			}
			ret += check_nv("pa0itssit", "62");
			ret += check_nv("pa0b0", "0x15eb");
			ret += check_nv("pa0b1", "0xfa82");
			ret += check_nv("pa0b2", "0xfe66");
		} else if (check_hw_type() == BCM5354G_CHIP) {
			ret += check_nv("pa0itssit", "62");
			ret += check_nv("pa0b0", "0x1326");
			ret += check_nv("pa0b1", "0xFB51");
			ret += check_nv("pa0b2", "0xFE87");
			ret += check_nv("reset_gpio", "7");
		} else if (check_hw_type() == BCM4705_BCM5397_EWC_CHIP) {
			// nothing to do
		} else if (check_hw_type() == BCM4704_BCM5325F_CHIP) {
			// nothing to do
		} else {
			ret += check_nv("pa0itssit", "62");
			ret += check_nv("pa0b0", "0x170c");
			ret += check_nv("pa0b1", "0xfa24");
			ret += check_nv("pa0b2", "0xfe70");
		}

		// ret += check_nv("gpio2", "adm_eecs");
		// ret += check_nv("gpio3", "adm_eesk");
		// ret += check_nv("gpio5", "adm_eedi");
		// ret += check_nv("gpio6", "adm_rc");

		ret += check_nv("wl0gpio2", "0");
		ret += check_nv("wl0gpio3", "0");

		ret += check_nv("cctl", "0");
		ret += check_nv("ccode", "0");
		break;
#endif
	}
	if (ret) {
		fprintf(stderr, "Some error found, we want to reboot!.....................\n");
		nvram_commit();
		sys_reboot();
	}

	return ret;
}

int check_pmon_nv(void)
{
	return 0;
}

void start_overclocking(void)
{
#ifdef HAVE_OVERCLOCKING
	cprintf("Overclocking started\n");

	int rev = cpu_plltype();

	if (rev == 0)
		return;		// unsupported

	char *ov = nvram_get("overclocking");

	if (ov == NULL)
		return;
	int clk = atoi(ov);

	if (nvram_get("clkfreq") == NULL)
		return;		// unsupported

	char *pclk = nvram_safe_get("clkfreq");
	char dup[64];

	strcpy(dup, pclk);
	int i;

	for (i = 0; i < strlen(dup); i++)
		if (dup[i] == ',')
			dup[i] = 0;
	int cclk = atoi(dup);

	if (clk == cclk) {
		cprintf("clkfreq %d MHz identical with new setting\n", clk);
		return;		// clock already set
	}

	int set = 1;
	int clk2 = 0;
	int clk2_1 = 0;
	int clk2_2 = 0;
	char clkfr[16];

	switch (clk) {
	case 150:
		clk2 = 75;
		break;
	case 183:
		clk2 = 92;
		break;
	case 187:
		clk2 = 94;
		break;
	case 192:
		clk2 = 96;
		break;
	case 198:
		clk2 = 98;
		break;
	case 200:
		clk2 = 100;
		clk2_1 = 100;
		if (rev == 8)
			clk2_2 = 50;
		else
			clk2_2 = 33;
		break;
	case 216:
		clk2 = 108;
		break;
	case 225:
		clk2 = 113;
		break;
	case 228:
		clk2 = 114;
		break;
	case 233:
		clk2 = 116;
		break;
	case 237:
		clk2 = 119;
		break;
	case 240:
		clk2 = 120;
		clk2_1 = 120;
		clk2_2 = 33;
		break;
	case 250:
		clk2 = 125;
		break;
	case 252:
		clk2 = 126;
		clk2_1 = 126;
		clk2_2 = 33;
		break;
	case 264:
		clk2 = 132;
		clk2_1 = 132;
		clk2_2 = 33;
		break;
	case 280:
		clk2 = 120;
		break;
	case 300:
		if (rev == 10) {
			clk2_1 = 200;
			clk2_2 = 100;
		} else {
			clk2 = 120;
			clk2_1 = 150;
			if (rev == 8)
				clk2_2 = 75;
			else
				clk2_2 = 37;
		}
		break;
	case 330:
		clk2_1 = 132;
		clk2_2 = 33;
		break;
	case 333:
		clk2_1 = 166;
		clk2_2 = 166;
		break;
	case 400:
		clk2_1 = 200;
		clk2_2 = 100;
		break;
	case 480:
		clk2_1 = 240;
		clk2_2 = 120;
		break;
	case 500:
		clk2_1 = 250;
		clk2_2 = 125;
		break;
	case 533:
		clk2_1 = 266;
		clk2_2 = 133;
		break;
	case 600:
		clk2_1 = 300;
		clk2_2 = 150;
		break;
	case 632:
		clk2_1 = 316;
		clk2_2 = 158;
		break;
	case 650:
		clk2_1 = 325;
		clk2_2 = 162;
		break;
	case 662:
		clk2_1 = 331;
		clk2_2 = 165;
		break;
	default:
		set = 0;
		break;
	}

	if (set) {
		cprintf("clock frequency adjusted from %d to %d, reboot needed\n", cclk, clk);
		if (rev == 2 || rev == 8 || rev == 10)
			sprintf(clkfr, "%d,%d,%d", clk, clk2_1, clk2_2);
		else
			sprintf(clkfr, "%d,%d", clk, clk2);
		nvram_set("clkfreq", clkfr);
		nvram_commit();
		fprintf(stderr, "Overclocking done, rebooting...\n");
		sys_reboot();
	}
#endif
}

char *enable_dtag_vlan(int enable)
{
	int donothing = 0;

	nvram_set("fromvdsl", "1");
	if (nvram_match("vdsl_state", "1") && enable)
		donothing = 1;
	if ((nvram_match("vdsl_state", "0")
	     || nvram_match("vdsl_state", "")) && !enable)
		donothing = 1;
	if (enable)
		nvram_set("vdsl_state", "1");
	else
		nvram_set("vdsl_state", "0");

	char *eth = "eth0";
#ifdef HAVE_MADWIFI
	eth = "eth0";
#else

	FILE *in = fopen("/proc/switch/eth1/reset", "rb");	// this

	// condition
	// fails
	// almost.
	// just one
	// router
	// (DLINK
	// DIR-330)
	// requires
	// it

	if (in) {
		eth = "eth1";
		fclose(in);
	} else {
		FILE *in = fopen("/proc/switch/eth2/reset", "rb");
		if (in) {
			eth = "eth2";
			fclose(in);
		} else
			eth = "eth0";
	}
#endif

	char *lan_vlan = nvram_safe_get("lan_ifnames");
	char *wan_vlan = nvram_safe_get("wan_ifname");
	char *vlan_lan_ports = NULL;
	char *vlan_wan_ports = NULL;
	int lan_vlan_num = 0;
	int wan_vlan_num = 1;

	if (startswith(lan_vlan, "vlan0")) {
		lan_vlan_num = 0;
	} else if (startswith(lan_vlan, "vlan1")) {
		lan_vlan_num = 1;
	} else if (startswith(lan_vlan, "vlan2")) {
		lan_vlan_num = 2;
	} else
		return eth;

	if (startswith(wan_vlan, "vlan0")) {
		wan_vlan_num = 0;
	} else if (startswith(wan_vlan, "vlan1")) {
		wan_vlan_num = 1;
	} else if (startswith(wan_vlan, "vlan2")) {
		wan_vlan_num = 2;
	} else
		return eth;

	if (wan_vlan_num == lan_vlan_num)
		return eth;

	vlan_lan_ports = nvram_nget("vlan%dports", lan_vlan_num);
	vlan_wan_ports = nvram_nget("vlan%dports", wan_vlan_num);

	char *vlan7ports = "4t 5";;

	if (!strcmp(vlan_wan_ports, "4 5"))
		vlan7ports = "4t 5";
	else if (!strcmp(vlan_wan_ports, "0 5"))
		vlan7ports = "0t 5";
	else if (!strcmp(vlan_wan_ports, "1 5"))
		vlan7ports = "1t 5";
	else if (!strcmp(vlan_wan_ports, "4 8"))
		vlan7ports = "4t 8";
	else if (!strcmp(vlan_wan_ports, "4 8u"))
		vlan7ports = "4t 8";
	else if (!strcmp(vlan_wan_ports, "0 8"))
		vlan7ports = "0t 8";

	if (!donothing) {
		writevaproc("1", "/proc/switch/%s/reset", eth);
		writevaproc("1", "/proc/switch/%s/enable_vlan", eth);
		if (enable) {
			fprintf(stderr, "enable vlan port mapping %s/%s\n", vlan_lan_ports, vlan7ports);
			if (!nvram_match("dtag_vlan8", "1")
			    || nvram_match("wan_vdsl", "0")) {
				writevaproc(vlan_lan_ports, "/proc/switch/%s/vlan/%d/ports", eth, lan_vlan_num);
				start_setup_vlans();
				writevaproc(" ", "/proc/switch/%s/vlan/%d/ports", eth, wan_vlan_num);
				writevaproc(vlan7ports, "/proc/switch/%s/vlan/7/ports", eth);
			} else {
				writevaproc(vlan_lan_ports, "/proc/switch/%s/vlan/%d/ports", eth, lan_vlan_num);
				start_setup_vlans();
				writevaproc("", "/proc/switch/%s/vlan/%d/ports", eth, wan_vlan_num);
				writevaproc(vlan7ports, "/proc/switch/%s/vlan/7/ports", eth);
				writevaproc(vlan7ports, "/proc/switch/%s/vlan/8/ports", eth);
			}
		} else {
			fprintf(stderr, "disable vlan port mapping %s/%s\n", vlan_lan_ports, vlan_wan_ports);
			writevaproc(" ", "/proc/switch/%s/vlan/7/ports", eth);
			writevaproc(" ", "/proc/switch/%s/vlan/8/ports", eth);
			writevaproc(vlan_lan_ports, "/proc/switch/%s/vlan/%d/ports", eth, lan_vlan_num);
			writevaproc(vlan_wan_ports, "/proc/switch/%s/vlan/%d/ports", eth, wan_vlan_num);
			start_setup_vlans();
		}
	}
	nvram_set("fromvdsl", "0");
	return eth;
}

void start_dtag(void)
{
	enable_dtag_vlan(1);
}
