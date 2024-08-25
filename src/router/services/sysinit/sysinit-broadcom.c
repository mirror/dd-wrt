/*
 * sysinit-broadcom.c
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

#include <services.h>
#include "devices/wireless.c"

#define sys_restart() eval("event", "3", "1", "1")
#define sys_reboot()                \
	eval("kill", "-15", "-1");  \
	eval("sleep", "3");         \
	eval("umount", "-a", "-r"); \
	eval("sync");               \
	eval("event", "3", "1", "15")

#ifndef BFL_AFTERBURNER
#define BFL_AFTERBURNER 0x0200
#endif

static char *brcm_to_swconfig(char *vlan, char *buf)
{
	strcpy(buf, vlan);
	int i;
	int len = strlen(buf);
	for (i = 0; i < len; i++) {
		if (buf[i] == '*')
			buf[i] = 't';
		if (buf[i] == 'u') {
			buf[i] = buf[i + 1];
			len--;
		}
	}
	return buf;
}

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
		if (!strcmp(cpu_type, "BCM4710") || !strcmp(cpu_type, "BCM4702")) {
			cprintf("We got BCM4702 board...\n");
			nvram_set("cpu_type", cpu_type);
		} else if (!strcmp(cpu_type, "BCM3302") || !strcmp(cpu_type, "BCM4712")) {
			cprintf("We got BCM4712 board...\n");
			nvram_set("cpu_type", cpu_type);
		} else {
			cprintf("We got unknown board...\n");
			nvram_set("cpu_type", cpu_type);
		}
	}
}

static void loadWlModule(void) // set wled params, get boardflags,
	// set afterburner bit, load wl,
	// unset afterburner bit
{
	int brand = getRouterBrand();
	char macbuf[32];
	char eaddr[32];

#ifndef HAVE_BUFFALO
	nvram_seti("pa0maxpwr", 251); // force pa0maxpwr to be 251
#endif

	if (check_hw_type() == BCM4702_CHIP)
		nvram_unset("wl0_abenable");
	else {
		nvram_seti("wl0_abenable", 1);
		nvram_seti("wl1_abenable", 1);
	}

	switch (brand) {
	case ROUTER_LINKSYS_WRH54G:
		nvram_seti("wl0gpio0", 135);
		break;
	case ROUTER_BUFFALO_WZRRSG54:
		nvram_unset("wl0_abenable");
		nvram_unset("wl1_abenable");
		break;
	case ROUTER_ASUS_WL550GE:
		nvram_seti("wl0gpio1", 0);
		nvram_seti("wl0gpio2", 0);
		break;
	case ROUTER_ASUS_WL500W:
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
	case ROUTER_MOTOROLA:
	case ROUTER_NETGEAR_WG602_V3:
	case ROUTER_RT480W:
	case ROUTER_USR_5465:
	case ROUTER_ASUS_RTN10:
		nvram_seti("wl0gpio0", 136);
		break;
	case ROUTER_ASUS_RTN10U:
		nvram_seti("ledbh5", 7);
		break;
	case ROUTER_WAP54G_V3:
		nvram_seti("wl0gpio0", 0);
		nvram_seti("wl0gpio2", 255);
		nvram_seti("wl0gpio3", 255);
		nvram_seti("wl0gpio5", 136);
		break;
	case ROUTER_ASUS_WL520GUGC:
		nvram_seti("wl0gpio0", 0);
		nvram_seti("wl0gpio1", 136);
		nvram_seti("wl0gpio2", 0);
		nvram_seti("wl0gpio3", 0);
		break;
	case ROUTER_LINKSYS_WTR54GS:
	case ROUTER_WAP54G_V1:
		nvram_seti("wl0gpio0", 136);
		nvram_seti("wl0gpio1", 0);
		nvram_seti("wl0gpio2", 0);
		nvram_seti("wl0gpio3", 0);
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
		nvram_seti("wl0gpio0", 8);
		break;
	case ROUTER_NETGEAR_WG602_V4:
		nvram_seti("wl0gpio0", 8);
		nvram_seti("wl0gpio1", 0);
		nvram_seti("wl0gpio2", 0);
		nvram_seti("wl0gpio3", 0);
		break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		nvram_seti("wl0gpio2", 136);
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		nvram_seti("wl0gpio0", 0);
		nvram_seti("wl0gpio5", 136);
		break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL500G_PRE:
		nvram_unset("wl0gpio0");
		break;
	case ROUTER_BELKIN_F5D7230_V2000:
		// case ROUTER_BELKIN_F5D7230_V3000:
	case ROUTER_BELKIN_F5D7231:
		nvram_seti("wl0gpio3", 136);
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		nvram_seti("wl0gpio5", 8);
		break;
	case ROUTER_BELKIN_F5D7231_V2000:
		nvram_seti("wl0gpio0", 2);
		nvram_seti("wl0gpio1", 0);
		break;
	case ROUTER_BUFFALO_WLAG54C:
	case ROUTER_ASUS_WL700GE:
		nvram_seti("wl0gpio0", 135);
		break;
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR3500LV2:
		nvram_seti("ledbh0", 7);
		break;
	case ROUTER_WRT320N:
		nvram_seti("ledbh0", 136);
		nvram_seti("ledbh1", 11);
		break;
	case ROUTER_NETGEAR_WNR2000V2:
		nvram_seti("ledbh5", 8);
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
	case ROUTER_NETGEAR_WNR3500LV2:
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
	case ROUTER_NETGEAR_R6200:
	case ROUTER_ASUS_RTN66:
	case ROUTER_NETCORE_NW715P:
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
	case ROUTER_ASUS_AC66U:
	case ROUTER_D1800H:
	case ROUTER_UBNT_UNIFIAC:
	case ROUTER_DLINK_DIR865:
		insmod("wl"); // load module
		break;
	case ROUTER_LINKSYS_EA6500:
		if (!sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr")) ||
		    startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
			char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("pci/2/1/macaddr", mac);
		}
		nvram_seti("partialboots", 0);
		nvram_async_commit();
	case ROUTER_LINKSYS_EA2700:
		nvram_seti("bootpartition", 0);
		nvram_seti("partialboots", 0);
		nvram_async_commit();
		insmod("wl"); // load module
		break;
	case ROUTER_WRT600N:
		fprintf(stderr, "fixing wrt600n\n");
		wl_hwaddr("eth0", macbuf);
		ether_etoa((char *)macbuf, eaddr);
		nvram_set("wl0_hwaddr", eaddr);
		MAC_SUB(eaddr);
		if (!nvram_match("et0macaddr", eaddr)) {
			nvram_set("et0macaddr", eaddr);
			nvram_commit();
			//              eval("/sbin/reboot");
			//              exit( 0 );
		}
		set_hwaddr("eth2", eaddr);
		wl_hwaddr("eth1", macbuf);
		ether_etoa((char *)macbuf, eaddr);
		nvram_set("wl1_hwaddr", eaddr);
		break;
	case ROUTER_WRT610N:
		wl_hwaddr("eth0", macbuf);
		ether_etoa((char *)macbuf, eaddr);
		nvram_set("wl0_hwaddr", eaddr);
		wl_hwaddr("eth1", macbuf);
		ether_etoa((char *)macbuf, eaddr);
		nvram_set("wl1_hwaddr", eaddr);
		break;

	default:
		boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
		fprintf(stderr, "boardflags are 0x%04X\n", boardflags);
		if (boardflags == 0) // we can try anyway
		{
			nvram_set("boardflags", "0x0200");
			insmod("wl"); // load module
			nvram_unset("boardflags");
		} else if (boardflags & BFL_AFTERBURNER) // ab flag already
		// set
		{
			insmod("wl"); // load module
		} else // ab flag not set
		{
			char bf[16];

			sprintf(bf, "0x%04X", boardflags);
			boardflags |= BFL_AFTERBURNER;
			fprintf(stderr, "enable Afterburner, boardflags are 0x%04X\n", boardflags);
			char ab[16];

			sprintf(ab, "0x%04X", boardflags);
			char *oldvalue = nvram_safe_get("boardflags"); // use the

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
			nvram_set("boardflags", ab); // set boardflags with
			// AfterBurner bit on
			insmod("wl"); // load module
			nvram_set("boardflags", oldvalue); // set back to
			// original
		}
	}
	detect_wireless_devices(RADIO_ALL);
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
		} // VLAN enabled
		else {
			// nvram_set("setup_4712","2");
			cprintf("setup_4712(): Disable VLAN, it must be in bridge mode\n");
			nvram_set("lan_ifnames", "eth0 eth1");
			strcpy(wlifname, "eth1");
			nvram_set("wl0_ifname", "eth1");
		}
	} else { // 4702, 4704
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

	if (nvram_matchi("manual_boot_nv", 1))
		return 0;

	if (!nvram_exists(name)) {
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

static void restore_set(const char *prefix, struct nvram_param *set)
{
	struct nvram_param *t;
	for (t = set; t->name; t++) {
		char name[64];
		sprintf(name, "%s/%s", prefix, t->name);
		if (!nvram_get(name))
			nvram_set(name, t->value);
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
	fprintf(stderr, "boardnum %s\n", nvram_safe_get("boardnum"));
	fprintf(stderr, "boardtype %s\n", nvram_safe_get("boardtype"));
	fprintf(stderr, "boardrev %s\n", nvram_safe_get("boardrev"));
#ifdef HAVE_80211AC
	if (!nvram_exists("bootflags")) {
		fprintf(stderr, "nvram invalid, erase\n");
		//          eval("erase","nvram"); // ignore it for testbed
		//          sys_reboot();
	}
#endif
	/*
	 * Setup console 
	 */

	cprintf("sysinit() klogctl\n");
	klogctl(8, NULL, nvram_geti("console_loglevel"));
	cprintf("sysinit() get router\n");

	int brand = getRouterBrand();

	led_control(LED_DIAG, LED_ON);
	char *rname = getRouter();

	fprintf(stderr, "Booting device: %s\n", rname);

	nvram_unset("port_swap");

	int need_reboot = 0;

	struct nvram_param *basic_params = NULL;
	struct nvram_param *extra_params = NULL;

	struct nvram_param generic1[] = {
		{ "lan_ifnames", "eth0 eth2" }, { "wan_ifname", "eth1" }, { "wl0_ifname", "eth2" }, { 0, 0 }
	};

	struct nvram_param generic1_wantolan[] = {
		{ "lan_ifnames", "eth2" }, { "wan_ifname", "eth0" }, { "wl0_ifname", "eth2" }, { 0, 0 }
	};

	struct nvram_param vlan_0_1[] = {
		{ "lan_ifnames", "vlan0 eth1" }, { "wan_ifname", "vlan1" }, { "wl0_ifname", "eth1" }, { 0, 0 }
	};

	struct nvram_param vlan_1_2[] = {
		{ "lan_ifnames", "vlan1 eth1" }, { "wan_ifname", "vlan2" }, { "wl0_ifname", "eth1" }, { 0, 0 }
	};

	switch (brand) {
	case ROUTER_BUFFALO_WZRRSG54:
		check_brcm_cpu_type();
		setup_4712();
		basic_params = generic1;
		eval("gpio", "init", "0"); // AOSS button
		eval("gpio", "init", "4"); // reset button
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
			if (sv_valid_hwaddr(nvram_safe_get("macaddr"))) //fix mac
				nvram_set("et0macaddr", nvram_safe_get("macaddr"));
			need_reboot = 1;
		}
		break;

	case ROUTER_RT210W:
	case ROUTER_ASKEY_RT220XD:
		basic_params = generic1;

		if (!nvram_exists("et0macaddr") || nvram_match("et0macaddr", "")) {
			nvram_set("et0macaddr", "00:16:E3:00:00:10"); // fix for
			// missing
			// cfe
			// default =
			// dead LAN
			// ports.
			need_reboot = 1;
		}
		if (!nvram_exists("et1macaddr") || nvram_match("et1macaddr", "")) {
			nvram_set("et1macaddr", "00:16:E3:00:00:11");
			need_reboot = 1;
		}
		break;

	case ROUTER_BRCM4702_GENERIC:
		basic_params = generic1;

		if (!nvram_exists("et0macaddr") || nvram_match("et0macaddr", "")) {
			nvram_set("et0macaddr", "00:11:22:00:00:10"); // fix for
			// missing
			// cfe
			// default =
			// dead LAN
			// ports.
			need_reboot = 1;
		}
		if (!nvram_exists("et1macaddr") || nvram_match("et1macaddr", "")) {
			nvram_set("et1macaddr", "00:11:22:00:00:11");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_WL500G:
		basic_params = generic1;

		if (!nvram_exists("et0macaddr") || nvram_match("et0macaddr", "")) {
			nvram_set("et0macaddr", "00:0C:6E:00:00:10"); // fix for
			// missing
			// cfe
			// default =
			// dead LAN
			// ports.
			need_reboot = 1;
		}
		if (!nvram_exists("et1macaddr") || nvram_match("et1macaddr", "")) {
			nvram_set("et1macaddr", "00:0C:6E:00:00:10");
			need_reboot = 1;
		}
		break;

	case ROUTER_DELL_TRUEMOBILE_2300:
		setup_4712();
		nvram_set("wan_ifname", "eth1"); // fix for WAN problem.
		break;

	case ROUTER_BUFFALO_WBR54G: // for WLA-G54
		basic_params = generic1;
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled")) // =
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
		nvram_set("wan_ifname", "eth0"); // WAN to nonexist. iface.
		nvram_seti("port_swap", 1);
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled")) // =
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
			nvram_set("vlan1ports", "4 5"); //dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			basic_params = generic1;
		}

		if (!nvram_exists("pci/1/1/macaddr")) {
			nvram_set("pci/1/1/macaddr", nvram_safe_get("et0macaddr"));
			need_reboot = 1;
		}
		//params taken from firmware ver. 2.1.13 multi-region
		struct nvram_param wnr834bv2_pci_1_1_params[] = { { "pa2gw1a0", "0" },
								  { "stbcpo", "0" },
								  { "pa2gw1a1", "0" },
								  { "ag0", "2" },
								  { "ag1", "2" },
								  { "ag2", "2" },
								  { "ccdpo", "0" },
								  { "txpid2ga0", "55" },
								  { "txpid2ga1", "78" },
								  { "txpt2g", "0x38" },
								  { "pa2gw0a0", "0" },
								  { "pa2gw0a1", "0" },
								  { "boardflags", "0x200" },
								  { "boardvendor", "0x14e4" },
								  { "bw40po", "0" },
								  { "sromrev", "4" },
								  { "venid", "0x14e4" },
								  { "boardrev", "0x4b" },
								  { "itt2ga0", "0" },
								  { "itt2ga1", "0" },
								  { "pa2gw3a0", "0" },
								  { "pa2gw3a1", "0" },
								  { "maxp2ga0", "0" },
								  { "maxp2ga1", "0" },
								  { "boardtype", "0x46d" },
								  { "boardflags2", "3" },
								  { "ofdm2gpo", "0" },
								  { "ledbh0", "0x8" },
								  { "ledbh1", "-1" },
								  { "ledbh2", "-1" },
								  { "ledbh3", "-1" },
								  { "mcs2gpo0", "0" },
								  { "mcs2gpo1", "0" },
								  { "mcs2gpo2", "0" },
								  { "mcs2gpo3", "0" },
								  { "mcs2gpo4", "0" },
								  { "mcs2gpo5", "0" },
								  { "mcs2gpo6", "0" },
								  { "mcs2gpo7", "0" },
								  { "bwduppo", "0" },
								  { "aa2g", "7" },
								  { "pa2gw2a0", "0" },
								  { "pa2gw2a1", "0" },
								  { "ccode", "ALL" },
								  { "regrev", "0" },
								  { "devid", "0x4329" },
								  { "cck2gpo", "0" },
								  { 0, 0 } };
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
	case ROUTER_NETGEAR_WNR3500LV2:
	case ROUTER_WRT320N:
	case ROUTER_ASUS_RTN16:
		basic_params = vlan_1_2;
		nvram_set("vlan2hwname", "et0");
		nvram_set("lan_ifnames", "vlan1 eth1");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan1ports", "1 2 3 4 8*") || nvram_match("vlan2ports", "0 8u")) {
			nvram_set("vlan1ports", "4 3 2 1 8*");
			nvram_set("vlan2ports", "0 8");
			need_reboot = 1;
		}
		break;
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
	case ROUTER_NETGEAR_R6300:
		if (!nvram_exists("clkfreq")) //set it only if it doesnt exist
			nvram_seti("clkfreq", 600);
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

		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr")) ||
		    startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C") ||
		    !sv_valid_hwaddr(nvram_safe_get("pci/2/1/macaddr")) ||
		    startswith(nvram_safe_get("pci/2/1/macaddr"), "00:90:4C")) {
			char mac[20];
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

		struct nvram_param wndr4500_pci_1_1_params[] = {

			{ "pa2gw1a0", "0x1DFC" },
			{ "pa2gw1a1", "0x1FF9" },
			{ "pa2gw1a2", "0x1E58" },
			{ "ledbh12", "11" },
			{ "legofdmbw202gpo", "0x88000000" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "legofdmbw20ul2gpo", "0x88000000" },
			{ "rxchain", "7" },
			{ "cckbw202gpo", "0x0000" },
			{ "mcsbw20ul2gpo", "0x88800000" },
			{ "pa2gw0a0", "0xFE56" },
			{ "pa2gw0a1", "0xFEB3" },
			{ "pa2gw0a2", "0xFE6A" },
			{ "boardflags", "0x80003200" },
			{ "tempoffset", "0" },
			{ "boardvendor", "0x14e4" },
			{ "triso2g", "3" },
			{ "sromrev", "9" },
			{ "extpagain2g", "3" },
			{ "venid", "0x14e4" },
			{ "maxp2ga0", "0x62" },
			{ "maxp2ga1", "0x62" },
			{ "maxp2ga2", "0x62" },
			{ "boardtype", "0x59b" },
			{ "boardflags2", "0x4000000" },
			{ "tssipos2g", "1" },
			{ "ledbh0", "11" },
			{ "ledbh1", "11" },
			{ "ledbh2", "11" },
			{ "ledbh3", "11" },
			{ "mcs32po", "0xA" },
			{ "legofdm40duppo", "0x0" },
			{ "antswctl2g", "0" },
			{ "txchain", "7" },
			{ "elna2g", "2" },
			{ "antswitch", "0" },
			{ "aa2g", "7" },
			{ "cckbw20ul2gpo", "0x0000" },
			{ "leddc", "0xFFFF" },
			{ "pa2gw2a0", "0xF886" },
			{ "pa2gw2a1", "0xF8AA" },
			{ "pa2gw2a2", "0xF8A7" },
			{ "pdetrange2g", "3" },
			{ "devid", "0x4332" },
			{ "tempthresh", "120" },
			{ "mcsbw402gpo", "0x0x88800000" },
			//                      {"macaddr", "84:1B:5E:46:25:2E"},
			{ "mcsbw202gpo", "0x88800000" },

			{ 0, 0 }
		};

		struct nvram_param wndr4500_pci_2_1_params[] = {

			{ "leddc", "0xFFFF" },
			{ "txchain", "7" },
			{ "maxp5gla0", "0x60" },
			{ "elna5g", "1" },
			{ "maxp5gla1", "0x60" },
			{ "maxp5gla2", "0x60" },
			{ "maxp5gha0", "0x72" },
			{ "maxp5gha1", "0x72" },
			{ "maxp5gha2", "0x72" },
			{ "pa5gw0a0", "0xFE6C" },
			{ "pa5gw0a1", "0xFE72" },
			{ "pa5gw0a2", "0xFE75" },
			{ "mcsbw20ul5gmpo", "0x22200000" },
			{ "extpagain5g", "3" },
			{ "pa5glw2a0", "0xFFFF" },
			{ "boardflags", "0x90000200" },
			{ "pa5glw2a1", "0xFFFF" },
			{ "pa5glw2a2", "0xFFFF" },
			{ "triso5g", "3" },
			{ "tempoffset", "0" },
			{ "mcsbw205gmpo", "0x22200000" },
			{ "devid", "0x4333" },
			{ "aa5g", "7" },
			{ "pa5ghw2a0", "0xF8C5" },
			{ "pa5ghw2a1", "0xF8D6" },
			{ "pa5ghw2a2", "0xF8DA" },
			//                      {"macaddr", "2C:B0:5D:46:78:01"},
			{ "mcsbw20ul5glpo", "0x0" },
			{ "pa5glw1a0", "0xFFFF" },
			{ "pa5glw1a1", "0xFFFF" },
			{ "pa5glw1a2", "0xFFFF" },
			{ "mcsbw205glpo", "0x0" },
			{ "mcsbw20ul5ghpo", "0x88800000" },
			{ "legofdmbw205gmpo", "0x22000000" },
			{ "ledbh12", "11" },
			{ "mcsbw205ghpo", "0x88800000" },
			{ "pa5ghw1a0", "0x1DD1" },
			{ "pa5ghw1a1", "0x1DFF" },
			{ "parefldovoltage", "35" },
			{ "pa5ghw1a2", "0x1D76" },
			{ "pa5gw2a0", "0xF8E9" },
			{ "mcsbw405gmpo", "0x22200000" },
			{ "pa5gw2a1", "0xF907" },
			{ "pa5gw2a2", "0xF8ED" },
			{ "boardtype", "0x5a9" },
			{ "ledbh0", "11" },
			{ "ledbh1", "11" },
			{ "ledbh2", "11" },
			{ "legofdmbw20ul5gmpo", "0x22000000" },
			{ "ledbh3", "11" },
			{ "rxchain", "7" },
			{ "pdetrange5g", "4" },
			{ "legofdm40duppo", "0x0" },
			{ "maxp5ga0", "0x66" },
			{ "pa5glw0a0", "0xFFFF" },
			{ "maxp5ga1", "0x66" },
			{ "pa5glw0a1", "0xFFFF" },
			{ "maxp5ga2", "0x66" },
			{ "pa5glw0a2", "0xFFFF" },
			{ "legofdmbw205glpo", "0x0" },
			{ "venid", "0x14e4" },
			{ "boardvendor", "0x14e4" },
			{ "legofdmbw205ghpo", "0x88000000" },
			{ "antswitch", "0" },
			{ "tempthresh", "120" },
			{ "pa5ghw0a0", "0xFE74" },
			{ "pa5ghw0a1", "0xFE7F" },
			{ "sromrev", "9" },
			{ "pa5ghw0a2", "0xFE72" },
			{ "antswctl5g", "0" },
			{ "pa5gw1a0", "0x1D5E" },
			{ "mcsbw405glpo", "0x0" },
			{ "pa5gw1a1", "0x1D3D" },
			{ "pa5gw1a2", "0x1DA8" },
			{ "legofdmbw20ul5glpo", "0x0" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "mcsbw405ghpo", "0x88800000" },
			{ "boardflags2", "0x4200000" },
			{ "legofdmbw20ul5ghpo", "0x88000000" },
			{ "mcs32po", "0x9" },
			{ "tssipos5g", "1" },

			{ 0, 0 }
		};

		struct nvram_param wndr4500v2_pci_1_1_params[] = {

			{ "pa2gw1a0", "0x1791" },
			{ "pa2gw1a1", "0x189B" },
			{ "pa2gw1a2", "0x173E" },
			{ "ledbh12", "11" },
			{ "legofdmbw202gpo", "0xECA64200" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "legofdmbw20ul2gpo", "0xECA64200" },
			{ "rxchain", "7" },
			{ "cckbw202gpo", "0x0000" },
			{ "mcsbw20ul2gpo", "0xECA64200" },
			{ "pa2gw0a0", "0xFE90" },
			{ "pa2gw0a1", "0xFE9F" },
			{ "pa2gw0a2", "0xFE8B" },
			{ "boardflags", "0x80003200" },
			{ "tempoffset", "0" },
			{ "boardvendor", "0x14e4" },
			{ "triso2g", "3" },
			{ "sromrev", "9" },
			{ "extpagain2g", "1" },
			{ "venid", "0x14e4" },
			{ "maxp2ga0", "0x5E" },
			{ "maxp2ga1", "0x5E" },
			{ "maxp2ga2", "0x5E" },
			{ "boardtype", "0x59b" },
			{ "boardflags2", "0x4100000" },
			{ "tssipos2g", "1" },
			{ "ledbh0", "11" },
			{ "ledbh1", "11" },
			{ "ledbh2", "11" },
			{ "ledbh3", "11" },
			{ "mcs32po", "0xA" },
			{ "legofdm40duppo", "0x0" },
			{ "antswctl2g", "0" },
			{ "txchain", "7" },
			{ "elna2g", "2" },
			{ "antswitch", "0" },
			{ "aa2g", "7" },
			{ "cckbw20ul2gpo", "0x0000" },
			{ "leddc", "0xFFFF" },
			{ "pa2gw2a0", "0xFA5C" },
			{ "pa2gw2a1", "0xFA22" },
			{ "pa2gw2a2", "0xFA7A" },
			{ "pdetrange2g", "3" },
			{ "devid", "0x4332" },
			{ "tempthresh", "120" },
			{ "mcsbw402gpo", "0xECAAAAAA" },
			//                      {"macaddr", "00:00:00:00:00:01"},
			{ "mcsbw202gpo", "0xECA64200" },

			{ 0, 0 }
		};

		struct nvram_param wndr4500v2_pci_2_1_params[] = {

			{ "leddc", "0xFFFF" },
			{ "txchain", "7" },
			{ "maxp5gla0", "0x64" },
			{ "elna5g", "1" },
			{ "maxp5gla1", "0x64" },
			{ "maxp5gla2", "0x64" },
			{ "maxp5gha0", "0x5E" },
			{ "maxp5gha1", "0x5E" },
			{ "maxp5gha2", "0x5E" },
			{ "pa5gw0a0", "0xFEB2" },
			{ "pa5gw0a1", "0xFE7D" },
			{ "pa5gw0a2", "0xFE78" },
			{ "mcsbw20ul5gmpo", "0x42000000" },
			{ "extpagain5g", "3" },
			{ "pa5glw2a0", "0xF98F" },
			{ "boardflags", "0x90000200" },
			{ "pa5glw2a1", "0xF9C1" },
			{ "pa5glw2a2", "0xF99D" },
			{ "triso5g", "3" },
			{ "tempoffset", "0" },
			{ "mcsbw205gmpo", "0x42000000" },
			{ "devid", "0x4333" },
			{ "aa5g", "7" },
			{ "pa5ghw2a0", "0xF9DC" },
			{ "pa5ghw2a1", "0xFA04" },
			{ "pa5ghw2a2", "0xF9EE" },
			//                      {"macaddr", "00:00:00:00:00:10"},
			{ "mcsbw20ul5glpo", "0x42000000" },
			{ "pa5glw1a0", "0x1A5D" },
			{ "pa5glw1a1", "0x1962" },
			{ "pa5glw1a2", "0x19EC" },
			{ "mcsbw205glpo", "0x20000000" },
			{ "mcsbw20ul5ghpo", "0xECA64200" },
			{ "legofdmbw205gmpo", "0x42000000" },
			{ "ledbh12", "11" },
			{ "mcsbw205ghpo", "0xECA64200" },
			{ "pa5ghw1a0", "0x1896" },
			{ "pa5ghw1a1", "0x1870" },
			{ "parefldovoltage", "35" },
			{ "pa5ghw1a2", "0x1883" },
			{ "pa5gw2a0", "0xF93C" },
			{ "mcsbw405gmpo", "0x42000000 " },
			{ "pa5gw2a1", "0xF99B" },
			{ "pa5gw2a2", "0xF995" },
			{ "boardtype", "0x5a9" },
			{ "ledbh0", "11" },
			{ "ledbh1", "11" },
			{ "ledbh2", "11" },
			{ "legofdmbw20ul5gmpo", "0x42000000" },
			{ "ledbh3", "11" },
			{ "rxchain", "7" },
			{ "pdetrange5g", "4" },
			{ "legofdm40duppo", "0x0" },
			{ "maxp5ga0", "0x4A" },
			{ "pa5glw0a0", "0xFE7F" },
			{ "maxp5ga1", "0x4A" },
			{ "pa5glw0a1", "0xFE66" },
			{ "maxp5ga2", "0x4A" },
			{ "pa5glw0a2", "0xFE6B" },
			{ "legofdmbw205glpo", "0x20000000" },
			{ "venid", "0x14e4" },
			{ "boardvendor", "0x14e4" },
			{ "legofdmbw205ghpo", "0xECA64200" },
			{ "antswitch", "0" },
			{ "tempthresh", "120" },
			{ "pa5ghw0a0", "0xFE53" },
			{ "pa5ghw0a1", "0xFE68" },
			{ "sromrev", "9" },
			{ "pa5ghw0a2", "0xFE5D" },
			{ "antswctl5g", "0" },
			{ "pa5gw1a0", "0x1C6A" },
			{ "mcsbw405glpo", "0x42000000" },
			{ "pa5gw1a1", "0x1A47" },
			{ "pa5gw1a2", "0x1A39" },
			{ "legofdmbw20ul5glpo", "0x42000000" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "mcsbw405ghpo", "0xECA64200" },
			{ "boardflags2", "0x4200000" },
			{ "legofdmbw20ul5ghpo", "0xECA64200" },
			{ "mcs32po", "0x9" },
			{ "tssipos5g", "1" },

			{ 0, 0 }
		};

		struct nvram_param r6300_pci_1_1_params[] = {

			{ "pa2gw1a0", "0x1D7C" },
			{ "pa2gw1a1", "0x1F79" },
			{ "pa2gw1a2", "0x1D58" },
			{ "ledbh12", "11" },
			{ "legofdmbw202gpo", "0x88000000" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "legofdmbw20ul2gpo", "0x88000000" },
			{ "rxchain", "7" },
			{ "cckbw202gpo", "0x0000" },
			{ "mcsbw20ul2gpo", "0x88800000" },
			{ "pa2gw0a0", "0xFE56" },
			{ "pa2gw0a1", "0xFEB3" },
			{ "pa2gw0a2", "0xFE6A" },
			{ "boardflags", "0x80003200" },
			{ "tempoffset", "0" },
			{ "boardvendor", "0x14e4" },
			{ "triso2g", "3" },
			{ "sromrev", "9" },
			{ "extpagain2g", "3" },
			{ "venid", "0x14e4" },
			{ "maxp2ga0", "0x62" },
			{ "maxp2ga1", "0x62" },
			{ "maxp2ga2", "0x62" },
			{ "boardtype", "0x59b" },
			{ "boardflags2", "0x4000000" },
			{ "tssipos2g", "1" },
			{ "ledbh0", "11" },
			{ "ledbh1", "11" },
			{ "ledbh2", "11" },
			{ "ledbh3", "11" },
			{ "mcs32po", "0xA" },
			{ "legofdm40duppo", "0x0" },
			{ "antswctl2g", "0" },
			{ "txchain", "7" },
			{ "elna2g", "2" },
			{ "antswitch", "0" },
			{ "aa2g", "7" },
			{ "cckbw20ul2gpo", "0x0000" },
			{ "leddc", "0xFFFF" },
			{ "pa2gw2a0", "0xF8A1" },
			{ "pa2gw2a1", "0xF8BF" },
			{ "pa2gw2a2", "0xF8DA" },
			{ "pdetrange2g", "3" },
			{ "devid", "0x4332" },
			{ "tempthresh", "120" },
			{ "mcsbw402gpo", "0x0x88800000" },
			//                      {"macaddr", "84:1B:5E:3D:FD:D7"},
			{ "mcsbw202gpo", "0x88800000" },

			{ 0, 0 }
		};

		struct nvram_param r6300_pci_2_1_params[] = {

			{ "rxgains5ghtrisoa0", "5" },
			{ "rxgains5ghtrisoa1", "4" },
			{ "rxgains5ghtrisoa2", "4" },
			{ "txchain", "7" },
			{ "mcslr5gmpo", "0" },
			{ "phycal_tempdelta", "255" },
			{ "pdgain5g", "4" },
			{ "maxp5gb3a0", "0x64" },
			{ "subband5gver", "0x4" },
			{ "maxp5gb3a1", "0x64" },
			{ "maxp5gb3a2", "0x64" },
			{ "boardflags", "0x10000000" },
			{ "rxgainerr5g", "0xffff,0xffff,0xffff,0xffff" },
			{ "tworangetssi5g", "0" },
			{ "rxgains5gtrisoa0", "7" },
			{ "rxgains5gtrisoa1", "6" },
			{ "rxgains5gtrisoa2", "5" },
			{ "tempoffset", "255" },
			{ "mcsbw205gmpo", "4001780768" },
			{ "xtalfreq", "40000" },
			{ "devid", "0x43a0" },
			{ "tempsense_option", "0x3" },
			{ "femctrl", "3" },
			{ "epagain2g", "0" },
			{ "aa5g", "7" },
			{ "rxgains2gelnagaina0", "0" },
			{ "rxgains2gelnagaina1", "0" },
			{ "rxgains2gelnagaina2", "0" },
			{ "cckbw20ul2gpo", "0" },
			{ "papdcap5g", "0" },
			{ "tssiposslope5g", "1" },
			{ "tempcorrx", "0x3f" },
			{ "noiselvl5gua0", "31" },
			{ "noiselvl5gua1", "31" },
			{ "noiselvl5gua2", "31" },
			{ "mcslr5glpo", "0" },
			{ "mcsbw402gpo", "0" },
			{ "sar5g", "15" },
			//                      {"macaddr", "84:1B:5E:3D:FD:D6"},
			{ "pa5ga0", "0xff39,0x1a55,0xfcc7,0xff38,0x1a7f,0xfcc3,0xff33,0x1a66,0xfcc4,0xff36,0x1a7b,0xfcc2" },
			{ "rxgains5gmelnagaina0", "2" },
			{ "pa5ga1", "0xff3a,0x1a0b,0xfcd3,0xff38,0x1a37,0xfccd,0xff37,0x1aa1,0xfcc0,0xff37,0x1a6f,0xfcc4" },
			{ "rxgains5gmelnagaina1", "2" },
			{ "pa5ga2", "0xff3a,0x1a28,0xfccd,0xff38,0x1a2a,0xfcce,0xff35,0x1a93,0xfcc1,0xff38,0x1aab,0xfcbe" },
			{ "rxgains5gmelnagaina2", "3" },
			{ "mcslr5ghpo", "0" },
			{ "mcsbw202gpo", "0" },
			{ "maxp5gb2a0", "0x64" },
			{ "maxp5gb2a1", "0x64" },
			{ "rxgains2gtrisoa0", "0" },
			{ "maxp5gb2a2", "0x64" },
			{ "rxgains2gtrisoa1", "0" },
			{ "noiselvl5gma0", "31" },
			{ "pcieingress_war", "15" },
			{ "rxgains2gtrisoa2", "0" },
			{ "noiselvl5gma1", "31" },
			{ "noiselvl5gma2", "31" },
			{ "sb40and80lr5gmpo", "0" },
			{ "rxgains5gelnagaina0", "1" },
			{ "rxgains5gelnagaina1", "1" },
			{ "noiselvl2ga0", "31" },
			{ "rxgains5gelnagaina2", "1" },
			{ "noiselvl2ga1", "31" },
			{ "noiselvl2ga2", "31" },
			{ "agbg0", "71" },
			{ "mcsbw205glpo", "3999687200" },
			{ "agbg1", "71" },
			{ "agbg2", "133" },
			{ "measpower1", "0x7f" },
			{ "sb20in80and160lr5gmpo", "0" },
			{ "measpower2", "0x7f" },
			{ "temps_period", "15" },
			{ "mcsbw805gmpo", "4001780768" },
			{ "dot11agduplrpo", "0" },
			{ "mcsbw205ghpo", "3429122848" },
			{ "measpower", "0x7f" },
			{ "rxgains5ghelnagaina0", "2" },
			{ "ofdmlrbw202gpo", "0" },
			{ "rxgains5ghelnagaina1", "2" },
			{ "rxgains5ghelnagaina2", "3" },
			{ "gainctrlsph", "0" },
			{ "sb40and80hr5gmpo", "0" },
			{ "sb20in80and160hr5gmpo", "0" },
			{ "mcsbw1605gmpo", "0" },
			{ "pa2ga0", "0xfe72,0x14c0,0xfac7" },
			{ "pa2ga1", "0xfe80,0x1472,0xfabc" },
			{ "pa2ga2", "0xfe82,0x14bf,0xfad9" },
			{ "epagain5g", "0" },
			{ "mcsbw405gmpo", "4001780768" },
			{ "boardtype", "0x621" },
			{ "cckbw202gpo", "0" },
			{ "rxchain", "7" },
			{ "maxp5gb1a0", "0x64" },
			{ "maxp5gb1a1", "0x64" },
			{ "maxp5gb1a2", "0x64" },
			{ "noiselvl5gla0", "31" },
			{ "noiselvl5gla1", "31" },
			{ "noiselvl5gla2", "31" },
			{ "sb40and80lr5glpo", "0" },
			{ "maxp5ga0", "92,96,96,96" },
			{ "maxp5ga1", "92,96,96,96" },
			{ "maxp5ga2", "92,96,96,96" },
			{ "noiselvl5gha0", "31" },
			{ "noiselvl5gha1", "31" },
			{ "noiselvl5gha2", "31" },
			{ "sb20in80and160lr5glpo", "0" },
			{ "sb40and80lr5ghpo", "0" },
			{ "venid", "0x14e4" },
			{ "mcsbw805glpo", "3999687200" },
			{ "pdgain2g", "4" },
			{ "sar", "0x0F12" },
			{ "boardvendor", "0x14e4" },
			{ "sb20in80and160lr5ghpo", "0" },
			{ "tempsense_slope", "0xff" },
			{ "mcsbw805ghpo", "3429122848" },
			{ "antswitch", "0" },
			{ "aga0", "71" },
			{ "aga1", "133" },
			{ "rawtempsense", "0x1ff" },
			{ "aga2", "133" },
			{ "tempthresh", "255" },
			{ "rxgainerr2g", "0xffff" },
			{ "tworangetssi2g", "0" },
			{ "dot11agduphrpo", "0" },
			{ "sb40and80hr5glpo", "0" },
			{ "sromrev", "11" },
			{ "boardnum", "21059" },
			{ "sb20in40lrpo", "0" },
			{ "rxgains2gtrelnabypa0", "0" },
			{ "rxgains2gtrelnabypa1", "0" },
			{ "sb20in80and160hr5glpo", "0" },
			{ "mcsbw1605glpo", "0" },
			{ "rxgains2gtrelnabypa2", "0" },
			{ "sb40and80hr5ghpo", "0" },
			{ "mcsbw405glpo", "3999687200" },
			{ "dot11agofdmhrbw202gpo", "0" },
			{ "aa2g", "0" },
			{ "boardrev", "0x1307" },
			{ "rxgains5gmtrisoa0", "5" },
			{ "sb20in80and160hr5ghpo", "0" },
			{ "mcsbw1605ghpo", "0" },
			{ "rxgains5gmtrisoa1", "4" },
			{ "rxgains5gmtrisoa2", "4" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "maxp5gb0a0", "0x60" },
			{ "rxgains5gmtrelnabypa0", "1" },
			{ "ag2", "0" },
			{ "papdcap2g", "0" },
			{ "maxp5gb0a1", "0x60" },
			{ "rxgains5gmtrelnabypa1", "1" },
			{ "maxp5gb0a2", "0x60" },
			{ "rxgains5gmtrelnabypa2", "1" },
			{ "mcsbw405ghpo", "3429122848" },
			{ "tssiposslope2g", "1" },
			{ "maxp2ga0", "76" },
			{ "maxp2ga1", "76" },
			{ "maxp2ga2", "76" },
			{ "boardflags2", "0x300002" },
			{ "boardflags3", "0x300030" },
			{ "rxgains5ghtrelnabypa0", "1" },
			{ "rxgains5ghtrelnabypa1", "1" },
			{ "rxgains5ghtrelnabypa2", "1" },
			{ "sar2g", "18" },
			{ "sb20in40hrrpo", "0" },
			{ "temps_hysteresis", "15" },
			{ "rxgains5gtrelnabypa0", "1" },
			{ "rxgains5gtrelnabypa1", "1" },
			{ "rxgains5gtrelnabypa2", "1" },

			{ 0, 0 }
		};

		/*
		 * set router's extra parameters 
		 */

		/* Restore defaults */
		if (isr6300) {
			restore_set("pci/1/1", r6300_pci_1_1_params);
			restore_set("pci/2/1", r6300_pci_2_1_params);
		} else if (iswndr4500v2) {
			restore_set("pci/1/1", wndr4500v2_pci_1_1_params);
			restore_set("pci/2/1", wndr4500v2_pci_2_1_params);
		} else {
			restore_set("pci/1/1", wndr4500_pci_1_1_params);
			restore_set("pci/2/1", wndr4500_pci_2_1_params);
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
			set_regulation(0, "EU", "66");

		if (nvram_match("wl1_country_code", "Q2"))
			set_regulation(1, "US", "0");
		else if (nvram_match("wl1_country_code", "EU"))
			set_regulation(1, "EU", "38");
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
		if (nvram_match("vlan1ports", "0 8") || nvram_match("vlan2ports", "1 2 3 4 8*")) {
			nvram_set("vlan1ports", "1 2 3 4 8*");
			nvram_set("vlan2ports", "0 8");
			need_reboot = 1;
		}
		break;

	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D4301:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan1ports", "4 8") || nvram_match("vlan2ports", "0 1 2 3 8*")) {
			nvram_set("vlan1ports", "3 2 1 0 8*");
			nvram_set("vlan2ports", "4 8");
			need_reboot = 1;
		}
		break;

	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4302:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan1ports", "4 5*") || nvram_match("vlan2ports", "0 1 2 3 5*")) {
			nvram_set("vlan1ports", "0 1 2 3 5*");
			nvram_set("vlan2ports", "4 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WNDR3400:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan2ports", "4 5u") || nvram_match("vlan1ports", "0 1 2 3 5*")) {
			nvram_set("vlan1ports", "3 2 1 0 5*");
			nvram_set("vlan2ports", "4 5");
			need_reboot = 1;
		}
		if (startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4") ||
		    startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4")) {
			char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}

		struct nvram_param wndr3400_sb_1_params[] = {

			{ "sromrev", "8" },
			{ "ccode", "ALL" },
			{ "regrev", "0" },
			{ "ledbh0", "11" },
			{ "ledbh1", "11" },
			{ "ledbh2", "11" },
			{ "ledbh3", "11" },
			{ "ledbh9", "8" },
			{ "leddc", "0xffff" },
			{ "txchain", "3" },
			{ "rxchain", "3" },
			{ "antswitch", "0" },
			{ "aa2g", "3" },
			{ "ag0", "2" },
			{ "ag1", "2" },
			{ "itt2ga0", "0x20" },
			{ "maxp2ga0", "0x48" },
			{ "pa2gw0a0", "0xFEA5" },
			{ "pa2gw1a0", "0x17B2" },
			{ "pa2gw2a0", "0xFA73" },
			{ "itt2ga1", "0x20" },
			{ "maxp2ga1", "0x48" },
			{ "pa2gw0a1", "0xfeba" },
			{ "pa2gw1a1", "0x173c" },
			{ "pa2gw2a1", "0xfa9b" },
			{ "tssipos2g", "1" },
			{ "extpagain2g", "2" },
			{ "pdetrange2g", "2" },
			{ "triso2g", "3" },
			{ "antswctl2g", "2" },
			{ "cck2gpo", "0x0000" },
			{ "ofdm2gpo", "0x66666666" },
			{ "mcs2gpo0", "0x6666" },
			{ "mcs2gpo1", "0x6666" },
			{ "mcs2gpo2", "0x6666" },
			{ "mcs2gpo3", "0x6666" },
			{ "mcs2gpo4", "0x6666" },
			{ "mcs2gpo5", "0x6666" },
			{ "mcs2gpo6", "0x6666" },
			{ "mcs2gpo7", "0x6666" },
			{ "cddpo", "0" },
			{ "stbcpo", "0" },
			{ "bw40po", "0" },
			{ "bwduppo", "0" },

			{ 0, 0 }
		};
		restore_set("sb/1", wndr3400_sb_1_params);

		struct nvram_param wndr3400_pci_1_1_params[] = {

			{ "sromrev", "8" },
			{ "ccode", "ALL" },
			{ "regrev", "0" },
			{ "ledbh0", "8" },
			{ "ledbh1", "0x11" },
			{ "ledbh2", "0x11" },
			{ "ledbh3", "0x11" },
			{ "leddc", "0xffff" },
			{ "txchain", "3" },
			{ "rxchain", "3" },
			{ "antswitch", "0" },
			{ "cddpo", "0" },
			{ "stbcpo", "0" },
			{ "bw40po", "0" },
			{ "bwduppo", "0" },
			{ "aa5g", "3" },
			{ "ag0", "2" },
			{ "ag1", "2" },
			{ "itt5ga0", "0x3e" },
			{ "maxp5ga0", "0x4A" },
			{ "maxp5gha0", "0x4A" },
			{ "maxp5gla0", "0x4A" },
			{ "pa5gw0a0", "0xFEF9" },
			{ "pa5gw1a0", "0x164B" },
			{ "pa5gw2a0", "0xFADD" },
			{ "pa5glw0a0", "0xFEF9" },
			{ "pa5glw1a0", "0x154B" },
			{ "pa5glw2a0", "0xFAFD" },
			{ "pa5ghw0a0", "0xfeda" },
			{ "pa5ghw1a0", "0x1612" },
			{ "pa5ghw2a0", "0xfabe" },
			{ "tssipos5g", "1" },
			{ "extpagain5g", "2" },
			{ "pdetrange5g", "4" },
			{ "triso5g", "3" },
			{ "antswctl2g", "0" },
			{ "antswctl5g", "0" },
			{ "itt5ga1", "0x3e" },
			{ "maxp5ga1", "0x4A" },
			{ "maxp5gha1", "0x4A" },
			{ "maxp5gla1", "0x4A" },
			{ "pa5gw0a1", "0xff31" },
			{ "pa5gw1a1", "0x1697" },
			{ "pa5gw2a1", "0xfb08" },
			{ "pa5glw0a1", "0xFF31" },
			{ "pa5glw1a1", "0x1517" },
			{ "pa5glw2a1", "0xFB2F" },
			{ "pa5ghw0a1", "0xff18" },
			{ "pa5ghw1a1", "0x1661" },
			{ "pa5ghw2a1", "0xfafe" },
			{ "ofdm5gpo0", "0x0000" },
			{ "ofdm5gpo1", "0x2000" },
			{ "ofdm5glpo0", "0x0000" },
			{ "ofdm5glpo1", "0x2000" },
			{ "ofdm5ghpo0", "0x0000" },
			{ "ofdm5ghpo1", "0x2000" },
			{ "mcs5gpo0", "0x4200" },
			{ "mcs5gpo1", "0x6664" },
			{ "mcs5gpo2", "0x4200" },
			{ "mcs5gpo3", "0x6664" },
			{ "mcs5gpo4", "0x4200" },
			{ "mcs5gpo5", "0x6664" },
			{ "mcs5gpo6", "0x4200" },
			{ "mcs5gpo7", "0x6664" },
			{ "mcs5glpo0", "0x4200" },
			{ "mcs5glpo1", "0x6664" },
			{ "mcs5glpo2", "0x4200" },
			{ "mcs5glpo3", "0x6664" },
			{ "mcs5glpo4", "0x4200" },
			{ "mcs5glpo5", "0x6664" },
			{ "mcs5glpo6", "0x4200" },
			{ "mcs5glpo7", "0x6664" },
			{ "mcs5ghpo0", "0x4200" },
			{ "mcs5ghpo1", "0x6664" },
			{ "mcs5ghpo2", "0x4200" },
			{ "mcs5ghpo3", "0x6664" },
			{ "mcs5ghpo4", "0x4200" },
			{ "mcs5ghpo5", "0x6664" },
			{ "mcs5ghpo6", "0x4200" },
			{ "mcs5ghpo7", "0x6664" },
			{ "cdd5ghpo/cdd5glpo/cdd5gpo/cdd2gpo", "0x0" },
			{ "stbc5ghpo/stbc5glpo/stbc5gpo/stbc2gpo", "0x0" },
			{ "bw405ghpo/bw405glpo/bw405gpo/bw402gpo", "0x2" },
			{ "wdup405ghpo/wdup405glpo/wdup405gpo/wdup402gpo", "0x0" },
			{ 0, 0 }
		};
		restore_set("pci/1/1", wndr3400_pci_1_1_params);
		break;
	case ROUTER_NETGEAR_R6200:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan2ports", "4 8u")) {
			nvram_set("vlan2ports", "4 8");
			need_reboot = 1;
		}
		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr")) ||
		    startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C") || !sv_valid_hwaddr(nvram_safe_get("sb/1/macaddr")) ||
		    startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4C")) {
			char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}
		struct nvram_param r6200_sb_1_params[] = { { "ofdm2gpo", "0x86666666" },
							   { "elna2g", "3" },
							   { "mcs2gpo0", "0x6666" },
							   { "mcs2gpo1", "0xB966" },
							   { "mcs2gpo2", "0x6666" },
							   { "mcs2gpo3", "0xB966" },
							   { "mcs2gpo4", "0xAAAA" },
							   { "mcs2gpo5", "0xCBAA" },
							   { "mcs2gpo6", "0xAAAA" },
							   { "mcs2gpo7", "0xCBAA" },
							   { "triso2g", "4" },
							   { "sromrev", "8" },
							   { "pa2gw2a0", "0xFA18" },
							   { "pa2gw2a1", "0xFA4B" },
							   { "itt2ga0", "0x20" },
							   { "itt2ga1", "0x20" },
							   { "ag0", "0" },
							   { "ag1", "0" },
							   { "ag2", "0" },
							   { "extpagain2g", "3" },
							   { "stbcpo", "0x0" },
							   { "aa2g", "3" },
							   { "bwduppo", "0x0" },
							   { "txchain", "3" },
							   { "pa2gw1a0", "0x191D" },
							   { "pa2gw1a1", "0x1809" },
							   { "boardflags2", "0x00110402" },
							   { "boardflags", "0x00001310" },
							   { "leddc", "0xFFFF" },
							   { "cck2gpo", "0x0" },
							   { "pa2gw0a0", "0xFEA6" },
							   { "pa2gw0a1", "0xFE9E" },
							   { "devid", "0x4329" },
							   { "bw40po", "0x0" },
							   { "pdetrange2g", "5" },
							   { "tssipos2g", "1" },
							   { "antswctl2g", "2" },
							   { "ledbh0", "11" },
							   { "maxp2ga0", "0x5e" },
							   { "ledbh1", "11" },
							   { "maxp2ga1", "0x5E" },
							   { "ledbh2", "11" },
							   { "ledbh3", "11" },
							   { "cddpo", "0x0" },
							   { "opo", "0x0" },
							   { "rxchain", "3" },
							   { "antswitch", "0" },

							   { 0, 0 } };

		restore_set("sb/1", r6200_sb_1_params);

		struct nvram_param r6200_pci_1_1_params[] = {
			{ "mcsbw805glpo", "0x0" },
			{ "mrrs", "128" },
			{ "sb20in80and160lr5gmpo", "0" },
			{ "aa5g", "3" },
			{ "sar5g", "15" },
			{ "mcsbw805ghpo", "0x99552222" },
			{ "rawtempsense", "0x1ff" },
			{ "mcsbw1605gmpo", "0" },
			{ "tworangetssi5g", "0" },
			{ "rxgains2gtrisoa0", "0" },
			{ "rxgains2gtrisoa1", "0" },
			{ "rxgains2gtrisoa2", "0" },
			{ "sb20in40lrpo", "0x0" },
			{ "sb40and80lr5gmpo", "0" },
			{ "pcieingress_war", "15" },
			{ "_mrrs", "128" },
			{ "noiselvl5gla0", "31" },
			{ "noiselvl5gla1", "31" },
			{ "agbg0", "71" },
			{ "noiselvl5gla2", "31" },
			{ "agbg1", "71" },
			{ "mcsbw405glpo", "0x0" },
			{ "agbg2", "133" },
			{ "epagain2g", "0" },
			{ "gainctrlsph", "0" },
			{ "papdcap5g", "0" },
			{ "noiselvl5gha0", "31" },
			{ "sb20in80and160hr5gmpo", "0" },
			{ "noiselvl5gha1", "31" },
			{ "noiselvl5gha2", "31" },
			{ "mcsbw405ghpo", "0x33330000" },
			{ "tempcorrx", "0x3f" },
			{ "tssiposslope5g", "1" },
			{ "mcslr5gmpo", "0" },
			{ "rxchain", "3" },
			{ "cckbw202gpo", "0" },
			{ "sb40and80hr5gmpo", "0" },
			{ "maxp5ga0", "60,90,90,90" },
			{ "maxp5ga1", "60,90,90,90" },
			{ "maxp5ga2", "60,90,90,90" },
			{ "sb20in80and160lr5glpo", "0x0" },
			{ "pa5ga0", "0xff39,0x1a55,0xfcc7,0xff38,0x1a7f,0xfcc3,0xff33,0x1a66,0xfcc4,0xff36,0x1a7b,0xfcc2" },
			{ "boardflags", "0x10000000" },
			{ "pdgain2g", "4" },
			{ "pa5ga1", "0xff3a,0x1a0b,0xfcd3,0xff38,0x1a37,0xfccd,0xff37,0x1aa1,0xfcc0,0xff37,0x1a6f,0xfcc4" },
			{ "pa5ga2", "0xff3a,0x1a28,0xfccd,0xff38,0x1a2a,0xfcce,0xff35,0x1a93,0xfcc1,0xff38,0x1aab,0xfcbe" },
			{ "tempoffset", "255" },
			{ "mcsbw1605glpo", "0x0" },
			{ "sb20in80and160lr5ghpo", "0x0" },
			{ "boardvendor", "0x14e4" },
			{ "mcsbw1605ghpo", "0x0" },
			{ "sb40and80lr5glpo", "0x0" },
			{ "subband5gver", "0x4" },
			{ "boardnum", "21059" },
			{ "measpower", "0x7f" },
			{ "dot11agduplrpo", "0x0" },
			{ "sromrev", "11" },
			{ "rxgainerr2g", "0xffff" },
			{ "sb40and80lr5ghpo", "0x0" },
			{ "venid", "0x14e4" },
			{ "sb20in80and160hr5glpo", "0x0" },
			{ "sb20in40hrpo", "0x0" },
			{ "ofdmlrbw202gpo", "0" },
			{ "mcsbw205gmpo", "3999678464" },
			{ "boardrev", "0x1303" },
			{ "sb20in40hrrpo", "0" },
			{ "dot11agofdmhrbw202gpo", "0" },
			{ "rxgains2gtrelnabypa0", "0" },
			{ "mcslr5glpo", "0x0" },
			{ "rxgains2gtrelnabypa1", "0" },
			{ "rxgains2gtrelnabypa2", "0" },
			{ "sb20in80and160hr5ghpo", "0x0" },
			{ "tempsense_slope", "0xff" },
			{ "maxp2ga0", "76" },
			{ "maxp2ga1", "76" },
			{ "epagain5g", "0" },
			{ "maxp2ga2", "76" },
			{ "sb40and80hr5glpo", "0x0" },
			{ "mcslr5ghpo", "0x0" },
			{ "boardtype", "0x621" },
			{ "pa2ga0", "0xfe72,0x14c0,0xfac7" },
			{ "pa2ga1", "0xfe80,0x1472,0xfabc" },
			{ "boardflags2", "0x300000" },
			{ "pa2ga2", "0xfe82,0x14bf,0xfad9" },
			{ "boardflags3", "0x300030" },
			{ "sb40and80hr5ghpo", "0x0" },
			{ "aga0", "71" },
			{ "aga1", "133" },
			{ "aga2", "133" },
			{ "measpower1", "0x7f" },
			{ "measpower2", "0x7f" },
			{ "rxgains5gtrelnabypa0", "0" },
			{ "rxgains5gtrelnabypa1", "0" },
			{ "rxgains5gtrelnabypa2", "0" },
			{ "rxgains2gelnagaina0", "0" },
			{ "rxgains2gelnagaina1", "0" },
			{ "txchain", "3" },
			{ "pdgain5g", "4" },
			{ "rxgains2gelnagaina2", "0" },
			{ "aa2g", "0" },
			{ "antswitch", "0" },
			{ "sar2g", "18" },
			{ "noiselvl2ga0", "31" },
			{ "temps_hysteresis", "15" },
			{ "noiselvl2ga1", "31" },
			{ "noiselvl2ga2", "31" },
			{ "mcsbw205glpo", "0x55555555" },
			{ "tworangetssi2g", "0" },
			{ "temps_period", "15" },
			{ "dot11agduphrpo", "0x0" },
			{ "mcsbw805gmpo", "3999678464" },
			{ "cckbw20ul2gpo", "0" },
			{ "mcsbw205ghpo", "0x11110000" },
			{ "phycal_tempdelta", "255" },
			{ "rxgainerr5g", "0xffff,0xffff,0xffff,0xffff" },
			{ "noiselvl5gua0", "31" },
			{ "noiselvl5gua1", "31" },
			{ "noiselvl5gua2", "31" },
			{ "papdcap2g", "0" },
			{ "rxgains5gelnagaina0", "3" },
			{ "rxgains5gelnagaina1", "3" },
			{ "femctrl", "3" },
			{ "rxgains5gelnagaina2", "3" },
			{ "tssiposslope2g", "1" },
			{ "noiselvl5gma0", "31" },
			{ "rxgains5gtrisoa0", "1" },
			{ "noiselvl5gma1", "31" },
			{ "rxgains5gtrisoa1", "1" },
			{ "noiselvl5gma2", "31" },
			{ "rxgains5gtrisoa2", "1" },
			{ "mcsbw405gmpo", "3999678464" },
			{ "tempsense_option", "0x3" },
			{ "devid", "0x43a0" },
			{ "tempthresh", "255" },
			{ "mcsbw402gpo", "0" },
			{ "mcsbw202gpo", "0" },

			{ 0, 0 }
		};
		restore_set("pci/1/1", r6200_pci_1_1_params);
		break;

	case ROUTER_NETGEAR_WNDR4000:
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		if (nvram_match("vlan2ports", "4 8u")) {
			nvram_set("vlan2ports", "4 8");
			need_reboot = 1;
		}
		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr")) ||
		    startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C") || !sv_valid_hwaddr(nvram_safe_get("sb/1/macaddr")) ||
		    startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4C")) {
			char mac[20];
			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}

		struct nvram_param wndr4000_sb_1_params[] = {

			{ "aa2g", "3" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "txchain", "3" },
			{ "rxchain", "3" },
			{ "antswitch", "0" },
			{ "itt2ga0", "0x20" },
			{ "itt2ga1", "0x20" },
			{ "tssipos2g", "1" },
			{ "extpagain2g", "3" },
			{ "pdetrange2g", "5" },
			{ "triso2g", "4" },
			{ "antswctl2g", "2" },
			{ "elna2g", "3" },

			{ "pa2gw0a0", "0xFEA6" },
			{ "pa2gw1a0", "0x191D" },
			{ "pa2gw2a0", "0xFA18" },

			{ "pa2gw0a1", "0xFE9E" },
			{ "pa2gw1a1", "0x1809" },
			{ "pa2gw2a1", "0xFA4B" },

			{ 0, 0 }
		};

		restore_set("sb/1", wndr4000_sb_1_params);

		struct nvram_param wndr4000_pci_1_1_params[] = {

			{ "boardflags2", "0x04000000" },
			{ "legofdmbw20ul5ghpo", "0x11000000" },
			{ "legofdmbw205ghpo", "0x11000000" },
			{ "legofdm40duppo", "0x2222" },
			{ "aa2g", "7" },
			{ "aa5g", "7" },
			{ "ag0", "0" },
			{ "ag1", "0" },
			{ "ag2", "0" },
			{ "txchain", "7" },
			{ "rxchain", "7" },
			{ "antswitch", "0" },
			{ "tssipos2g", "1" },
			{ "extpagain2g", "0" },
			{ "pdetrange2g", "4" },
			{ "antswctl2g", "0" },
			{ "tssipos5g", "1" },
			{ "extpagain5g", "0" },
			{ "pdetrange5g", "4" },
			{ "triso5g", "1" },
			{ "antswctl5g", "0" },

			{ "pa2gw0a0", "0xFE6D" },
			{ "pa2gw0a1", "0xFE72" },
			{ "pa2gw0a2", "0xFE74" },
			{ "pa2gw1a0", "0x1772" },
			{ "pa2gw1a1", "0x1792" },
			{ "pa2gw1a2", "0x1710" },
			{ "pa2gw2a0", "0xFA34" },
			{ "pa2gw2a1", "0xFA31" },
			{ "pa2gw2a2", "0xFA4F" },
			{ "maxp2ga0", "0x48" },
			{ "maxp2ga1", "0x48" },
			{ "maxp2ga2", "0x48" },

			{ "pa5gw0a0", "0xFE82" },
			{ "pa5gw0a1", "0xFE85" },
			{ "pa5gw0a2", "0xFE7F" },
			{ "pa5gw1a0", "0x1677" },
			{ "pa5gw1a1", "0x167C" },
			{ "pa5gw1a2", "0x1620" },
			{ "pa5gw2a0", "0xFA72" },
			{ "pa5gw2a1", "0xFA7A" },
			{ "pa5gw2a2", "0xFA88" },
			{ "maxp5ga0", "0x4E" },
			{ "maxp5ga1", "0x4E" },
			{ "maxp5ga2", "0x4E" },

			{ "pa5ghw0a0", "0xFE9A" },
			{ "pa5ghw0a1", "0xFE89" },
			{ "pa5ghw0a2", "0xFE98" },
			{ "pa5ghw1a0", "0x15E7" },
			{ "pa5ghw1a1", "0x155F" },
			{ "pa5ghw1a2", "0x15CD" },
			{ "pa5ghw2a0", "0xFAAC" },
			{ "pa5ghw2a1", "0xFAB0" },
			{ "pa5ghw2a2", "0xFAB2" },
			{ "maxp5gha0", "0x40" },
			{ "maxp5gha1", "0x40" },
			{ "maxp5gha2", "0x40" },

			{ "pa5glw0a0", "0xFE97" },
			{ "pa5glw0a1", "0xFE82" },
			{ "pa5glw0a2", "0xFE84" },
			{ "pa5glw1a0", "0x162F" },
			{ "pa5glw1a1", "0x15ED" },
			{ "pa5glw1a2", "0x167F" },
			{ "pa5glw2a0", "0xFA98" },
			{ "pa5glw2a1", "0xFA99" },
			{ "pa5glw2a2", "0xFA84" },
			{ "maxp5gla0", "0x48" },
			{ "maxp5gla1", "0x48" },
			{ "maxp5gla2", "0x48" },

			{ "mcs32po", "0x2222" },

			{ "legofdmbw205gmpo", "0x33221100" },
			{ "legofdmbw20ul5gmpo", "0x33221100" },
			{ "mcsbw205glpo", "0x11000000" },
			{ "mcsbw205gmpo", "0x44221100" },
			{ "mcsbw20ul5glpo", "0x11000000" },
			{ "mcsbw20ul5gmpo", "0x44221100" },
			{ "mcsbw405glpo", "0x33222222" },
			{ "mcsbw405gmpo", "0x66443322" },

			{ 0, 0 }
		};
		restore_set("pci/1/1", wndr4000_pci_1_1_params);
		break;

	case ROUTER_NETGEAR_WNR2000V2:
		basic_params = vlan_0_1;
		if (nvram_match("vlan0ports", "1 2 3 4 5*") || nvram_match("vlan1ports", "0 5u")) {
			nvram_set("vlan0ports", "4 3 2 1 5*");
			nvram_set("vlan1ports", "0 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_RTN12:
		basic_params = vlan_0_1;
		set_gpio(0, 1);
		if (!nvram_matchi("ledbh0", 0) || !nvram_matchi("ledbh1", 0)) {
			nvram_seti("ledbh0", 0);
			nvram_seti("ledbh1", 0);
			need_reboot = 1;
		}
		if (nvram_match("vlan0ports", "0 1 2 3 5*") || nvram_match("vlan1ports", "4 5u")) {
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASUS_AC66U: {
		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
		struct nvram_param bcm4360ac_defaults_pci2_1[] = { { "aa5g", "7" },
								   { "aga0", "0" },
								   { "aga1", "0" },
								   { "aga2", "0" },
								   { "agbg0", "133" },
								   { "agbg1", "133" },
								   { "agbg2", "133" },
								   { "antswitch", "0" },
								   { "dot11agofdmhrbw202gpo", "0" },
								   { "femctrl", "3" },
								   { "epagain2g", "0" },
								   { "gainctrlsph", "0" },
								   { "papdcap5g", "0" },
								   { "tworangetssi5g", "0" },
								   { "pdgain5g", "4" },
								   { "epagain5g", "0" },
								   { "tssiposslope5g", "1" },
								   { "measpower", "0x7f" },
								   { "measpower1", "0x7f" },
								   { "measpower2", "0x7f" },
								   { "noiselvl5gha0", "31" },
								   { "noiselvl5gha1", "31" },
								   { "noiselvl5gha2", "31" },
								   { "noiselvl5gla0", "31" },
								   { "noiselvl5gla1", "31" },
								   { "noiselvl5gla2", "31" },
								   { "noiselvl5gma0", "31" },
								   { "noiselvl5gma1", "31" },
								   { "noiselvl5gma2", "31" },
								   { "noiselvl5gua0", "31" },
								   { "noiselvl5gua1", "31" },
								   { "noiselvl5gua2", "31" },
								   { "pcieingress_war", "15" },
								   { "phycal_tempdelta", "255" },
								   { "rawtempsense", "0x1ff" },
								   { "rxchain", "7" },
								   { "rxgainerr5g", "0xffff,0xffff,0xffff,0xffff" },
								   { "sar5g", "15" },
								   { "sromrev", "11" },
								   { "subband5gver", "0x4" },
								   { "tempcorrx", "0x3f" },
								   { "tempoffset", "255" },
								   { "temps_hysteresis", "15" },
								   { "temps_period", "15" },
								   { "tempsense_option", "0x3" },
								   { "tempsense_slope", "0xff" },
								   { "tempthresh", "255" },
								   { "txchain", "7" },
								   { "ledbh0", "2" },
								   { "ledbh1", "5" },
								   { "ledbh2", "4" },
								   { "ledbh3", "11" },
								   { "ledbh10", "7" },
								   { "maxp5ga0", "104,104,104,104" },
								   { "maxp5ga1", "104,104,104,104" },
								   { "maxp5ga2", "104,104,104,104" },
								   { "mcsbw205glpo", "0xBB975311" },
								   { "mcsbw405glpo", "0xBB975311" },
								   { "mcsbw805glpo", "0xBB975311" },
								   { "mcsbw205gmpo", "0xBB975311" },
								   { "mcsbw405gmpo", "0xBB975311" },
								   { "mcsbw805gmpo", "0xBB975311" },
								   { "mcsbw205ghpo", "0xBB975311" },
								   { "mcsbw405ghpo", "0xBB975311" },
								   { "mcsbw805ghpo", "0xBB975311" },
								   { "leddc", "0xffff" },
								   { "xtalfreq", "40000" },

								   { 0, 0 } };
		restore_set("pci/2/1", bcm4360ac_defaults_pci2_1);

		struct nvram_param bcm4331_defaults_pci1_1[] = { { "maxp2ga0", "0x70" },
								 { "maxp2ga1", "0x70" },
								 { "maxp2ga2", "0x70" },
								 { "cckbw202gpo", "0x5555" },
								 { "cckbw20ul2gpo", "0x5555" },
								 { "legofdmbw202gpo", "0x97555555" },
								 { "legofdmbw20ul2gpo", "0x97555555" },
								 { "mcsbw202gpo", "0xDA755555" },
								 { "mcsbw20ul2gpo", "0xDA755555" },
								 { "mcsbw402gpo", "0xFC965555" },
								 { "xtalfreq", "20000" },
								 { 0, 0 } };
		restore_set("pci/1/1", bcm4331_defaults_pci1_1);

		if (nvram_match("regulation_domain", "US"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "Q2"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "EU"))
			set_regulation(0, "EU", "66");
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
			set_regulation(1, "EU", "38");
		else if (nvram_match("regulation_domain_5G", "TW"))
			set_regulation(1, "TW", "13");
		else if (nvram_match("regulation_domain_5G", "CN"))
			set_regulation(1, "CN", "1");
		else
			set_regulation(1, "US", "0");

		nvram_seti("pci/2/1/ledbh13", 136);
		set_gpio(13, 0);
	} break;

	case ROUTER_D1800H: {
		if (!nvram_exists("ledbh0") || nvram_matchi("ledbh11", 130)) {
			nvram_seti("ledbh0", 11);
			nvram_seti("ledbh1", 11);
			nvram_seti("ledbh2", 11);
			nvram_seti("ledbh11", 136);
			need_reboot = 1;
		}
		struct nvram_param bcm4360ac_defaults_pci2_1[] = { { "maxp2ga0", "0x70" },
								   { "maxp2ga1", "0x70" },
								   { "maxp2ga2", "0x70" },
								   { "maxp5ga0", "0x6A" },
								   { "maxp5ga1", "0x6A" },
								   { "maxp5ga2", "0x6A" },
								   { "cckbw202gpo", "0x5555" },
								   { "cckbw20ul2gpo", "0x5555" },
								   { "legofdmbw202gpo", "0x97555555" },
								   { "legofdmbw20ul2gpo", "0x97555555" },
								   { "mcsbw202gpo", "0xDA755555" },
								   { "mcsbw20ul2gpo", "0xDA755555" },
								   { "mcsbw402gpo", "0xFC965555" },
								   { "cckbw205gpo", "0x5555" },
								   { "cckbw20ul5gpo", "0x5555" },
								   { "legofdmbw205gpo", "0x97555555" },
								   { "legofdmbw20ul5gpo", "0x97555555" },
								   { "legofdmbw205gmpo", "0x77777777" },
								   { "legofdmbw20ul5gmpo", "0x77777777" },
								   { "legofdmbw205ghpo", "0x77777777" },
								   { "legofdmbw20ul5ghpo", "0x77777777" },
								   { "mcsbw205ghpo", "0x77777777" },
								   { "mcsbw20ul5ghpo", "0x77777777" },
								   { "mcsbw205gpo", "0xDA755555" },
								   { "mcsbw20ul5gpo", "0xDA755555" },
								   { "mcsbw405gpo", "0xFC965555" },
								   { "mcsbw405ghpo", "0x77777777" },
								   { "mcsbw405ghpo", "0x77777777" },
								   { "mcs32po", "0x7777" },
								   { "legofdm40duppo", "0x0000" },
								   { 0, 0 } };

		restore_set("pci/2/1", bcm4360ac_defaults_pci2_1);

		struct nvram_param bcm4360ac_defaults_pci1_1[] = { { "maxp5ga0", "104,104,104,104" },
								   { "maxp5ga1", "104,104,104,104" },
								   { "maxp5ga2", "104,104,104,104" },
								   { "mcsbw205glpo", "0xBB975311" },
								   { "mcsbw405glpo", "0xBB975311" },
								   { "mcsbw805glpo", "0xBB975311" },
								   { "mcsbw205gmpo", "0xBB975311" },
								   { "mcsbw405gmpo", "0xBB975311" },
								   { "mcsbw805gmpo", "0xBB975311" },
								   { "mcsbw205ghpo", "0xBB975311" },
								   { "mcsbw405ghpo", "0xBB975311" },
								   { "mcsbw805ghpo", "0xBB975311" },
								   { 0, 0 } };
		restore_set("pci/1/1", bcm4360ac_defaults_pci1_1);

		nvram_set("lan_ifnames", "vlan1 eth1 eth2");
		nvram_set("wan_ifname", "vlan2");
	} break;
	case ROUTER_ASUS_RTN66: {
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
		struct nvram_param bcm4360ac_defaults_pci1_1[] = { { "maxp2ga0", "0x70" },
								   { "maxp2ga1", "0x70" },
								   { "maxp2ga2", "0x70" },
								   { "cckbw202gpo", "0x5555" },
								   { "cckbw20ul2gpo", "0x5555" },
								   { "legofdmbw202gpo", "0x97555555" },
								   { "legofdmbw20ul2gpo", "0x97555555" },
								   { "mcsbw202gpo", "0xFC955555" },
								   { "mcsbw20ul2gpo", "0xFC955555" },
								   { "mcsbw402gpo", "0xFFFF9999" },
								   { "mcs32po", "0x9999" },
								   { "legofdm40duppo", "0x4444" },
								   { 0, 0 } };
		restore_set("pci/1/1", bcm4360ac_defaults_pci1_1);

		struct nvram_param bcm4360ac_defaults_pci2_1[] = { { "maxp5ga0", "0x6A" },
								   { "maxp5ga1", "0x6A" },
								   { "maxp5ga2", "0x6A" },
								   { "legofdmbw205gmpo", "0x77777777" },
								   { "legofdmbw20ul5gmpo", "0x77777777" },
								   { "mcsbw205gmpo", "0x77777777" },
								   { "mcsbw20ul5gmpo", "0x77777777" },
								   { "mcsbw405gmpo", "0x77777777" },
								   { "maxp5gha0", "0x6A" },
								   { "maxp5gha1", "0x6A" },
								   { "maxp5gha2", "0x6A" },
								   { "legofdmbw205ghpo", "0x77777777" },
								   { "legofdmbw20ul5ghpo", "0x77777777" },
								   { "mcsbw205ghpo", "0x77777777" },
								   { "mcsbw20ul5ghpo", "0x77777777" },
								   { "mcsbw405ghpo", "0x77777777" },
								   { "mcs32po", "0x7777" },
								   { "legofdm40duppo", "0x0000" },
								   { 0, 0 } };
		restore_set("pci/2/1", bcm4360ac_defaults_pci2_1);

		if (nvram_match("regulation_domain", "US"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "Q2"))
			set_regulation(0, "US", "0");
		else if (nvram_match("regulation_domain", "EU"))
			set_regulation(0, "EU", "66");
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
			set_regulation(1, "EU", "38");
		else if (nvram_match("regulation_domain_5G", "TW"))
			set_regulation(1, "TW", "13");
		else if (nvram_match("regulation_domain_5G", "CN"))
			set_regulation(1, "CN", "1");
		else
			set_regulation(1, "US", "0");
	} break;

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
		if (nvram_matchi("clkdivsf", 4) && nvram_match("vlan1ports", "1 2 3 4 5*")) {
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
		if (!sv_valid_hwaddr(nvram_safe_get("pci/1/1/macaddr")) ||
		    startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C") || !sv_valid_hwaddr(nvram_safe_get("sb/1/macaddr")) ||
		    startswith(nvram_safe_get("sb/1/macaddr"), "00:90:4C")) {
			char mac[20];

			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("sb/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			need_reboot = 1;
		}
		struct nvram_param e4200_pci_1_1_params[] = { { "pa2gw0a0", "0xfe8c" },
							      { "pa2gw1a0", "0x1b20" },
							      { "pa2gw2a0", "0xf98c" },
							      { "pa2gw0a1", "0xfe98" },
							      { "pa2gw1a1", "0x19ae" },
							      { "pa2gw2a1", "0xf9ab" },

							      { "pa5gw0a0", "0xfe52" },
							      { "pa5gw1a0", "0x163e" },
							      { "pa5gw2a0", "0xfa59" },
							      { "pa5gw0a1", "0xfe63" },
							      { "pa5gw1a1", "0x1584" },
							      { "pa5gw2a1", "0xfa92" },
							      { "pa5gw0a2", "0xfe7c" },
							      { "pa5gw1a2", "0x1720" },
							      { "pa5gw2a2", "0xfa4a" },

							      { "pa5ghw0a0", "0xfe6a" },
							      { "pa5ghw1a0", "0x163c" },
							      { "pa5ghw2a0", "0xfa69" },
							      { "pa5ghw0a1", "0xfe67" },
							      { "pa5ghw1a1", "0x160e" },
							      { "pa5ghw2a1", "0xfa6a" },
							      { "pa5ghw0a2", "0xfe76" },
							      { "pa5ghw1a2", "0x1766" },
							      { "pa5ghw2a2", "0xfa2c" },
							      /*			
			{"pa5glw0a0", "0"},
			{"pa5glw1a0", "0"},
			{"pa5glw2a0", "0"},
			{"pa5glw0a1", "0"},
			{"pa5glw1a1", "0"},
			{"pa5glw2a1", "0"},
			{"pa5glw0a2", "0"},
			{"pa5glw1a2", "0"},
			{"pa5glw2a2", "0"},
*/

							      { "pa05gidx", "5" },
							      { "pa05glidx", "0" },
							      { "pa05ghidx", "7" },
							      { "pa15gidx", "0" },
							      { "pa15glidx", "0" },
							      { "pa15ghidx", "3" },
							      { "pa25gidx", "5" },
							      { "pa25glidx", "0" },
							      { "pa25ghidx", "9" },

							      { 0, 0 } };
		restore_set("pci/1/1", e4200_pci_1_1_params);
		break;

#endif

	case ROUTER_NETGEAR_WNDR3300:
		if (nvram_match("force_vlan_supp", "enabled")) {
			nvram_set("lan_ifnames", "vlan0 eth2 eth3");
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5"); //dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			nvram_set("lan_ifnames",
				  "eth0 eth2 eth3"); // dual radio
		}
		nvram_set("wan_ifname", "eth1");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("wl1_ifname", "eth3");
		set_gpio(7, 0);

		if (!nvram_exists("pci/1/1/macaddr") || !nvram_exists("pci/1/3/macaddr")) {
			char mac[20];

			strcpy(mac, nvram_safe_get("et0macaddr"));
			MAC_ADD(mac);
			MAC_ADD(mac);
			nvram_set("pci/1/1/macaddr", mac);
			MAC_ADD(mac);
			nvram_set("pci/1/3/macaddr", mac);
			need_reboot = 1;
		}
		//params taken from firmware ver. 1.0.29 multi-region
		struct nvram_param wndr3300_pci_1_1_params[] = { { "stbcpo", "0" },
								 { "mcs5gpo0", "0x4200" },
								 { "pa2gw1a0", "0x14EA" },
								 { "mcs5gpo1", "0x6664" },
								 { "pa2gw1a1", "0x14DA" },
								 { "mcs5gpo2", "0x4200" },
								 { "maxp5gha0", "0x4A" },
								 { "mcs5gpo3", "0x6664" },
								 { "maxp5gha1", "0x4A" },
								 { "mcs5gpo4", "0" },
								 { "mcs5gpo5", "0" },
								 { "mcs5gpo6", "0" },
								 { "aa5g", "7" },
								 { "mcs5gpo7", "0" },
								 { "pa5glw2a0", "0xFBA2" },
								 { "pa5glw2a1", "0xFBDB" },
								 { "ag0", "2" },
								 { "ag1", "2" },
								 { "ag2", "2" },
								 { "pa5gw2a0", "0xFBBA" },
								 { "pa5gw2a1", "0xFC11" },
								 { "pa5ghw2a0", "0xFBB5" },
								 { "pa5ghw2a1", "0xFBD2" },
								 { "ccdpo", "0" },
								 { "txpid2ga0", "52" },
								 { "itt5ga0", "0x3C" },
								 { "rxchain", "3" },
								 { "txpid2ga1", "51" },
								 { "itt5ga1", "0x3C" },
								 { "maxp5ga0", "0x4A" },
								 { "maxp5ga1", "0x4A" },
								 { "txpt2g", "0x48" },
								 { "pa2gw0a0", "0xFEFC" },
								 { "pa2gw0a1", "0xFF03" },
								 { "boardflags", "0x0A00" },
								 { "mcs5glpo0", "0x4200" },
								 { "pa5glw1a0", "0x120E" },
								 { "mcs5glpo1", "0x6664" },
								 { "ofdm5gpo", "0x88888888" },
								 { "pa5glw1a1", "0x12BD" },
								 { "mcs5glpo2", "0x4200" },
								 { "mcs5glpo3", "0x6664" },
								 { "mcs5glpo4", "0" },
								 { "mcs5glpo5", "0" },
								 { "mcs5glpo6", "0" },
								 { "mcs5glpo7", "0" },
								 { "boardvendor", "0x14e4" },
								 { "bw40po", "0" },
								 { "sromrev", "4" },
								 { "venid", "0x14e4" },
								 { "pa5gw1a0", "0x1337" },
								 { "pa5gw1a1", "0x14A4" },
								 { "pa5ghw1a0", "0x11C2" },
								 { "pa5ghw1a1", "0x1275" },
								 { "boardrev", "0x13" },
								 { "itt2ga0", "0x3E" },
								 { "itt2ga1", "0x3E" },
								 { "pa2gw3a0", "0" },
								 { "pa2gw3a1", "0" },
								 { "maxp2ga0", "0x4A" },
								 { "maxp2ga1", "0x4A" },
								 { "boardtype", "0x49C" },
								 { "boardflags2", "0x0014" },
								 { "ofdm2gpo", "0x66666666" },
								 { "ledbh0", "11" },
								 { "ledbh1", "11" },
								 { "pa5glw0a0", "0xFEFB" },
								 { "ledbh2", "11" },
								 { "pa5glw0a1", "0xFF5B" },
								 { "ledbh3", "11" },
								 { "ledbh4", "11" },
								 { "ledbh5", "5" },
								 { "ledbh6", "7" },
								 { "ledbh7", "11" },
								 { "mcs2gpo0", "0x6666" },
								 { "mcs2gpo1", "0x6666" },
								 { "mcs2gpo2", "0x6666" },
								 { "mcs2gpo3", "0x6666" },
								 { "txpid5gla0", "18" },
								 { "mcs2gpo4", "0" },
								 { "txpid5gla1", "14" },
								 { "mcs2gpo5", "0" },
								 { "txpt5g", "0x3C" },
								 { "mcs2gpo6", "0" },
								 { "mcs2gpo7", "0" },
								 { "mcs5ghpo0", "0x4200" },
								 { "mcs5ghpo1", "0x6664" },
								 { "bwduppo", "0" },
								 { "mcs5ghpo2", "0x4200" },
								 { "mcs5ghpo3", "0x6664" },
								 { "txchain", "3" },
								 { "mcs5ghpo4", "0" },
								 { "mcs5ghpo5", "0" },
								 { "txpid5gha0", "28" },
								 { "mcs5ghpo6", "0" },
								 { "ofdm5glpo", "0x88888888" },
								 { "txpid5gha1", "25" },
								 { "mcs5ghpo7", "0" },
								 { "antswitch", "2" },
								 { "aa2g", "7" },
								 { "pa5gw0a0", "0xFF3C" },
								 { "pa5gw0a1", "0xFFEC" },
								 { "ofdm5ghpo", "0x88888888" },
								 { "pa5ghw0a0", "0xFEE8" },
								 { "pa5ghw0a1", "0xFF72" },
								 { "leddc", "0xFFFF" },
								 { "pa2gw2a0", "0xFB44" },
								 { "pa2gw2a1", "0xFB28" },
								 { "pa5glw3a0", "0" },
								 { "pa5glw3a1", "0" },
								 { "ccode", "0" },
								 { "pa5gw3a0", "0" },
								 { "regrev", "0" },
								 { "pa5gw3a1", "0" },
								 { "devid", "0x4328" },
								 { "pa5ghw3a0", "0" },
								 { "pa5ghw3a1", "0" },
								 { "txpt5gh", "0x3C" },
								 { "cck2gpo", "0x0000" },
								 { "txpt5gl", "0x30" },
								 { "maxp5gla0", "0x4A" },
								 { "txpid5ga0", "39" },
								 { "maxp5gla1", "0x4A" },
								 { "txpid5ga1", "39" },
								 { 0, 0 } };
		restore_set("pci/1/1", wndr3300_pci_1_1_params);

		struct nvram_param wndr3300_pci_1_3_params[] = { { "ag0", "0x02" },
								 { "boardflags", "0xAA48" },
								 { "ccode", "0" },
								 { "aa0", "0x03" },
								 { "devid", "0x4318" },
								 { "pa0b0", "0x14ed" },
								 { "pa0b1", "0xfac7" },
								 { "pa0b2", "0xfe8a" },
								 { "pa0itssit", "62" },
								 { "pa0maxpwr", "0x0042" },
								 { "opo", "0" },
								 { "wl0gpio0", "11" },
								 { "wl0gpio1", "11" },
								 { "wl0gpio2", "11" },
								 { "wl0gpio3", "7" },
								 { "sromrev", "2" },
								 { 0, 0 } };
		restore_set("pci/1/3", wndr3300_pci_1_3_params);
		break;

	case ROUTER_MOTOROLA_WE800G:
		nvram_set("lan_ifnames", "eth1 eth2");
		nvram_set("wl0_ifname", "eth2");
		nvram_set("wan_ifname", "eth0"); // WAN to nonexist. iface.
		nvram_seti("port_swap", 1);
		set_gpio(7, 0);
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled")) // =
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
			nvram_set("vlan1ports", "4 5"); //dummy
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
			nvram_set("vlan1ports", "4 5"); //dummy
			nvram_set("vlan0hwname", "et0");
		} else {
			nvram_set("lan_ifnames", "eth0 eth2");
		}
		break;

	case ROUTER_WRT54G1X:
		if (check_switch_support()) {
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
		nvram_seti("pci/1/1/ledbh0", 11);
		nvram_seti("pci/1/1/ledbh1", 135);
		nvram_seti("pci/1/2/ledbh0", 11);
		nvram_seti("pci/1/2/ledbh2", 135);
		nvram_set("pci/1/1/boardflags2", "0x0400");
		nvram_set("pci/1/2/boardflags2", "0x0602");

		if (!nvram_matchi("bootnv_ver", 6)) {
			if (startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C") ||
			    startswith(nvram_safe_get("pci/1/2/macaddr"), "00:90:4C")) {
				char mac[20];
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
		nvram_seti("pci/1/1/ledbh2", 8);
		nvram_seti("sb/1/ledbh1", 8);
		if (nvram_match("vlan1ports", "1 2 3 4 8*"))
			nvram_set("vlan1ports", "4 3 2 1 8*");
		if (startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4C") ||
		    startswith(nvram_safe_get("pci/1/1/macaddr"), "00:90:4c")) {
			char mac[20];
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
		nvram_set("wan_ifname", "vlan1"); // fix for Asus WL500gPremium
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
		nvram_set("wan_ifname", "eth2"); // map WAN port to
		// nonexistant interface
		if (nvram_match("wan_to_lan", "yes") && nvram_invmatch("wan_proto", "disabled")) // =
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
		nvram_seti("pci/1/1/ledbh0", 136);
		if (nvram_match("vlan1ports", "\"4 5*\"")) {
			nvram_set("vlan0ports", "3 2 1 0 5*");
			nvram_set("vlan1ports", "4 5");
			need_reboot = 1;
		}
		break;

	case ROUTER_DELL_TRUEMOBILE_2300_V2: // we must fix cfe defaults
		// with CR added
		nvram_set("vlan0hwname", "et0");
		nvram_set("vlan1hwname", "et0");
		nvram_seti("et0mdcport", 0);
		nvram_seti("et0phyaddr", 30);
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
		if (!nvram_exists("Fix_WL520GUGC_clock")) {
			nvram_seti("Fix_WL520GUGC_clock", 1);
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		if (nvram_match("vlan1ports", "0 5u"))
			nvram_set("vlan1ports", "0 5");
		if (nvram_matchi("sromrev", 2) && nvram_match("boardrev", "0x10") && nvram_match("boardtype", "0x48E")) {
			nvram_seti("sromrev",
				   3); // This is a fix for WGR614L NA - which has a wrong sromrev
			need_reboot = 1;
		}
		break;

	case ROUTER_ALLNET01:
		nvram_set("wl0_ifname", "eth1");
		if (nvram_match("vlan1ports", "5u")) //correct bad parameters
		{
			nvram_set("vlan1ports", "4 5");
			nvram_set("vlan0ports", "0 1 2 3 5*");
		}
		break;

	case ROUTER_LINKSYS_WTR54GS:
		set_gpio(3, 1); //prevent reboot loop
		// reset
		break;

	case ROUTER_WAP54G_V3:
		set_gpio(0, 1); // reset gpio 0 for reset
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
	if (!nvram_exists("il0macaddr"))
		need_reboot = 1;

	char mac[20];

	if (nvram_matchi("port_swap", 1))
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
		nvram_set("wan_ifnames", wanifname);
		nvram_set("wan_default", wanifname);
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

		if (!nvram_matchi("no_sercom", 1)) {
			//fix mac
			unsigned char mac[6];
			FILE *in = fopen("/dev/mtdblock/0", "rb");

			if (in != NULL) //special sercom mac address handling
			{
				fseek(in, 0x1ffa0, SEEK_SET);
				fread(mac, 6, 1, in);
				fclose(in);
				char macstr[32];

				sprintf(macstr, "%02X:%02X:%02X:%02X:%02X:%02X", (int)mac[0] & 0xff, (int)mac[1] & 0xff,
					(int)mac[2] & 0xff, (int)mac[3] & 0xff, (int)mac[4] & 0xff, (int)mac[5] & 0xff);
				nvram_set("et0macaddr", macstr);
				set_hwaddr("eth0", macstr);
			}
		}
		break;
	}

	/*
	 * Must have stuff 
	 */
	switch (brand) {
	case ROUTER_WRT320N:
		if (!nvram_matchi("reset_gpio", 5)) {
			nvram_seti("reset_gpio", 5);
			need_reboot = 1;
		}
		break;

	case ROUTER_MOTOROLA_V1:
		set_gpio(7, 0);
		break;

	case ROUTER_WRT54G_V8:
		nvram_seti("reset_gpio", 7);
		break;

	case ROUTER_ASUS_WL700GE:
		set_gpio(3,
			 1); // POWER-enable, turns on power to HDD and switch leds
		break;
	}

	/*
	 * additional boardflags adjustment, etc...
	 */
	switch (brand) {
	case ROUTER_BELKIN_F5D7231:
		if (nvram_match("boardflags", "0x388") || nvram_match("boardflags", "0x0388")) {
			nvram_set("boardflags", "0x0f58");
			need_reboot = 1;
		}
		break;

	case ROUTER_ASKEY_RT220XD:
		if (nvram_match("boardflags", "0x388") || nvram_match("boardflags", "0x0388")) {
			nvram_set("boardflags", "0x0208");
			need_reboot = 1;
		}
		break;

	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		if (!nvram_matchi("buffalo_hp", 1) &&
		    (nvram_match("boardflags", "0x1658") || nvram_match("boardflags", "0x2658"))) {
			nvram_seti("buffalo_hp", 1);
#ifndef HAVE_BUFFALO // if HAVE_BUFFALO not used to be FCC/CE \
	// valid
			nvram_set("boardflags", "0x3658"); // enable high gain
			// PA
			need_reboot = 1;
#endif
		}
		break;

	case ROUTER_BUFFALO_WHRG54S: // for HP only
		if (!nvram_matchi("buffalo_hp", 1) && nvram_match("boardflags", "0x1758")) {
			nvram_seti("buffalo_hp", 1);
#ifndef HAVE_BUFFALO // if HAVE_BUFFALO not used to be FCC/CE \
	// valid
			nvram_set("boardflags", "0x3758"); // enable high gain
			// PA
			need_reboot = 1;
#endif
		}
		break;

	case ROUTER_WRTSL54GS:
		if (nvram_match("force_switch_supp", "enabled") && nvram_match("boardflags", "0x0018")) {
			nvram_set("boardflags", "0x0118"); //enable lan vlans
			need_reboot = 1;
		} else if (!nvram_match("force_switch_supp", "enabled") && nvram_match("boardflags", "0x0118")) {
			nvram_set("boardflags", "0x0018"); //disable vlans
			need_reboot = 1;
		}
		break;

	case ROUTER_WRT150N:
	case ROUTER_WRT160N:
		if (nvram_match("force_switch_supp", "enabled") && nvram_match("boardflags", "0x0010")) {
			nvram_set("boardflags", "0x0110"); //enable lan vlans
			need_reboot = 1;
		} else if (!nvram_match("force_switch_supp", "enabled") && nvram_match("boardflags", "0x0110")) {
			nvram_set("boardflags", "0x0010"); //disable vlans
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WNR834BV2:
	case ROUTER_NETGEAR_WNDR3300:
		if (nvram_match("force_switch_supp", "enabled") && nvram_match("boardflags", "0x10")) {
			nvram_set("boardflags", "0x110"); //enable lan vlans
			need_reboot = 1;
		} else if (!nvram_match("force_switch_supp", "enabled") && nvram_match("boardflags", "0x110")) {
			nvram_set("boardflags", "0x10"); //disable vlans
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WG602_V4:
		if (nvram_match("boardflags", "0x650")) {
			nvram_set("boardflags", "0x0458");
			need_reboot = 1;
		}
		break;

	case ROUTER_NETGEAR_WNR3500L: //usb power fix (gpio 12)
	case ROUTER_NETGEAR_WNR3500LV2:
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
	if (!nvram_exists("et_txq_thresh")) {
		nvram_seti("et_txq_thresh", 3300);
		//              nvram_set("et_dispatch_mode","1"); 1=better throughput 0=better ping
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

#ifdef HAVE_SWCONFIG
	nvram_set("has_ctf", "1");
	if (nvram_match("sfe", "2"))
		nvram_set("ctf_disable", "0");
	else
		nvram_set("ctf_disable", "1");
#endif

	snprintf(buf, sizeof(buf), "/lib/modules/%s", name.release);
	if (stat("/proc/modules", &tmp_stat) == 0 && stat(buf, &tmp_stat) == 0) {
		char module[80], *modules = "", *next;

#ifdef HAVE_ACK
		nvram_seti("portprio_support",
			   0); // no portprio support in NEWD or BCMMODERN
#else
		nvram_seti("portprio_support",
			   1); // only switch drivers in VINT support this
#endif
		if (brand == ROUTER_WRT600N)
			insmod("wl");

		if (check_switch_support() && check_hw_type() != BCM5325E_CHIP) {
			switch (brand) {
				//#ifdef HAVE_BCMMODERN
				//                              modules = "bcm57xx";
				//                              break;
				//#endif

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
				nvram_seti("portprio_support", 0);
#ifdef HAVE_BCMMODERN
				modules = "bcm57xx switch-core switch-robo";
#else
				modules = "bcm57xxlsys switch-core switch-robo";
#endif
				break;
			case ROUTER_ASUS_RTN16:
			case ROUTER_ASUS_RTN66:
			case ROUTER_ASUS_AC66U:
			case ROUTER_NETGEAR_WNR3500LV2:
			case ROUTER_LINKSYS_EA2700:
			case ROUTER_LINKSYS_EA6500:
			case ROUTER_NETGEAR_WNDR4500:
			case ROUTER_NETGEAR_WNDR4500V2:
			case ROUTER_NETGEAR_R6300:
			case ROUTER_D1800H:
			case ROUTER_DLINK_DIR865:
			case ROUTER_UBNT_UNIFIAC:
			case ROUTER_NETGEAR_R6200:
				modules = "ctf et switch-core switch-robo";
				break;
			case ROUTER_LINKSYS_WRT55AG:
			case ROUTER_MOTOROLA:
			case ROUTER_BUFFALO_WBR2G54S:
			case ROUTER_DELL_TRUEMOBILE_2300_V2:
				modules = "switch-core switch-adm";
				break;

			case ROUTER_WRT54G_V8:
			case ROUTER_WRT54G_V81:
			case ROUTER_LINKSYS_WRH54G:
			case ROUTER_ASUS_WL520G:
			case ROUTER_ASUS_WL520GUGC:
				modules = "switch-core switch-robo";
				break;

			case ROUTER_WRT54G1X:
			case ROUTER_WRT54G:
				insmod("switch-core");
				if (insmod("switch-robo"))
					insmod("switch-adm");
				break;

			case ROUTER_RT480W:
			case ROUTER_BUFFALO_WLI2_TX1_G54:
				modules = "";
				insmod("switch-core");
				if (insmod("switch-robo"))
					insmod("switch-adm");
				break;

			case ROUTER_WRT54G3G:
				modules =
					"switch-core switch-robo pcmcia_core yenta_socket ds serial_cs usbcore usb-ohci usbserial sierra";
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
				nvram_seti("portprio_support", 0);
				modules = "";
				break;
			case ROUTER_ASUS_AC1200:
				nvram_seti("portprio_support", 0);
				modules = "";
				break;
			default:
				modules = "switch-core switch-robo";
				break;
			}
		} else {
			switch (brand) {
			case ROUTER_WRT610N:
				//#ifdef HAVE_BCMMODERN
				//                              modules = "bcm57xx";
				//                              break;
				//#endif
			case ROUTER_WRT310N:
			case ROUTER_WRT310NV2:
			case ROUTER_WRT320N:
			case ROUTER_WRT350N:
			case ROUTER_WRT600N:
			case ROUTER_WRT610NV2:
			case ROUTER_WRT300NV11:
			case ROUTER_BUFFALO_WZRG144NH:
			case ROUTER_NETGEAR_WNR3500L:
			case ROUTER_LINKSYS_E3200:
			case ROUTER_LINKSYS_E4200:
			case ROUTER_NETGEAR_WNDR4000:
				nvram_seti("portprio_support", 0);
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
			case ROUTER_NETGEAR_WNR3500LV2:
			case ROUTER_NETGEAR_R6300:
			case ROUTER_D1800H:
			case ROUTER_DLINK_DIR865:
			case ROUTER_UBNT_UNIFIAC:
			case ROUTER_NETGEAR_R6200:
				modules = "ctf et switch-core switch-robo";
				break;
			case ROUTER_LINKSYS_WRT55AG:
				modules = "switch-core switch-adm";

				break;
			case ROUTER_ASUS_WL500GD:
			case ROUTER_ASUS_WL550GE:
				modules = "switch-core switch-robo";
				break;
			case ROUTER_BUFFALO_WZRRSG54:
				nvram_seti("portprio_support", 0);
				modules = "";
				break;
			case ROUTER_WRT54G3G:
				if (check_switch_support())
					modules = "switch-core switch-robo pcmcia_core yenta_socket ds";
				else {
					nvram_seti("portprio_support", 0);

					modules = "pcmcia_core yenta_socket ds";
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
				nvram_seti("portprio_support", 0);
				modules = "";
				break;
			case ROUTER_ASUS_AC1200:
				nvram_seti("portprio_support", 0);
				modules = "";
				break;

			default:
#ifndef HAVE_BCMMODERN
				if (check_switch_support())
					modules = "switch-core switch-robo";
				else
#endif
				{
					nvram_seti("portprio_support", 0);
					modules = "switch-core switch-robo";
				}
				break;
			}
		}
		//      fprintf( "insmod %s\n", modules );

		insmod(modules);

		if (check_hw_type() == BCM4702_CHIP)
			insmod("diag");

		loadWlModule();
	}
	/*
	 * Set a sane date 
	 */
	stime(&tm);

	if (brand == ROUTER_WRT54G3G) {
		eval("cardmgr");
	}

	if (brand == ROUTER_UBNT_UNIFIAC) {
		nvram_set("vlan1ports", "0 8*");
		nvram_set("vlan2ports", "1 8");
	}

	char *v1 = nvram_safe_get("vlan0ports");
	char *v2 = nvram_safe_get("vlan1ports");
	int vlan2_supp = 0;
	if (!*v1 || *nvram_safe_get("vlan2ports") || brand == ROUTER_WRT600N) {
		v1 = v2;
		vlan2_supp = 1;
		v2 = nvram_safe_get("vlan2ports");
	}
	if (!vlan2_supp) {
		nvram_default_get("port0vlans", "1");
		nvram_default_get("port1vlans", "0");
		nvram_default_get("port2vlans", "0");
		nvram_default_get("port3vlans", "0");
		nvram_default_get("port4vlans", "0");
		nvram_default_get("port5vlans", "0 1 16000");
	} else {
		nvram_default_get("port0vlans", "2");
		nvram_default_get("port1vlans", "1");
		nvram_default_get("port2vlans", "1");
		nvram_default_get("port3vlans", "1");
		nvram_default_get("port4vlans", "1");
		nvram_default_get("port5vlans", "1 2 16000");
	}

#ifdef HAVE_SWCONFIG
	insmod("b5301x_common");
	insmod("b5301x_srab");

	char vlan2buf[64];
	char vlan1buf[64];
	char *vlan2 = brcm_to_swconfig(v2, vlan2buf);
	char *vlan1 = brcm_to_swconfig(v1, vlan1buf);
	char var[32], *next;
	int port = 0;
	int wanport = 0;
	foreach(var, vlan2, next)
	{
		if (strlen(var) == 1) {
			nvram_set("sw_wan", var);
			wanport = atoi(var);
			port++;
			break;
		}
	}
	char cpuport[32] = { 0 };
	int swap = 0;
	int first = -1;
	int last = -1;
	foreach(var, vlan1, next)
	{
		if (strlen(var) == 1) {
			if (wanport > atoi(var))
				swap++;
			last = atoi(var);
			if (first == -1)
				first = last;
			char *tv = strdup(var);
			nvram_nset(tv, "sw_lan%d", port++);
			free(tv);
		} else {
			strncpy(cpuport, var, 1);
		}
	}
	if (swap != port - 1 || first > last)
		swap = 0;
	if (swap) { // lan ports are in physical reverse order (guessed)
		int i;
		for (i = 1; i < (port / 2) + 1; i++) {
			char s1[32];
			char s2[32];
			sprintf(s1, "sw_lan%d", i);
			sprintf(s2, "sw_lan%d", port - i);
			char *sw1 = strdup(nvram_safe_get(s1));
			char *sw2 = strdup(nvram_safe_get(s2));
			nvram_set(s2, sw1);
			nvram_set(s1, sw2);
			free(sw1);
			free(sw2);
		}
	}
	nvram_set("sw_cpuport", strdup(cpuport));
	eval("swconfig", "dev", "switch0", "set", "enable_vlan", "1");
	if (vlan2_supp) {
		eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", vlan1);
		eval("swconfig", "dev", "switch0", "vlan", "2", "set", "ports", vlan2);
	} else {
		eval("swconfig", "dev", "switch0", "vlan", "0", "set", "ports", vlan1);
		eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", vlan2);
	}
	eval("swconfig", "dev", "switch0", "set", "apply");
#endif

	switch (brand) {
	case ROUTER_WRT600N:
	case ROUTER_WRT610N:
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL550GE:
	case ROUTER_MOTOROLA:
	case ROUTER_RT480W:
	case ROUTER_WRT350N:
	case ROUTER_BUFFALO_WZRG144NH:
	case ROUTER_DELL_TRUEMOBILE_2300_V2:
	case ROUTER_WRT54G1X:
	case ROUTER_UBNT_UNIFIAC:
		start_config_vlan();
		break;
	default:
		if (check_switch_support()) {
			start_config_vlan();
		}
		break;
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

			if (nvram_matchi("clkfreq", 200) && nvram_matchi("overclocking", 200)) {
				ret += check_nv("clkfreq", "216");
				nvram_seti("overclocking", 216);
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
		return; // unsupported

	char *ov = nvram_safe_get("overclocking");

	if (!*ov)
		return;
	int clk = atoi(ov);

	if (!nvram_exists("clkfreq"))
		return; // unsupported

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
		return; // clock already set
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

	nvram_seti("fromvdsl", 1);
	if (nvram_matchi("vdsl_state", 1) && enable)
		donothing = 1;
	if ((nvram_matchi("vdsl_state", 0) || nvram_match("vdsl_state", "")) && !enable)
		donothing = 1;
	if (enable)
		nvram_seti("vdsl_state", 1);
	else
		nvram_seti("vdsl_state", 0);

	char *eth = "eth0";
#ifdef HAVE_MADWIFI
	eth = "eth0";
#else

	FILE *in = fopen("/proc/switch/eth1/reset", "rb"); // this

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

	char *vlan7ports = "4t 5";
	;

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

#ifndef HAVE_SWCONFIG
	if (!donothing) {
		writevaproc("1", "/proc/switch/%s/reset", eth);
		writevaproc("1", "/proc/switch/%s/enable_vlan", eth);
		if (enable) {
			fprintf(stderr, "enable vlan port mapping %s/%s\n", vlan_lan_ports, vlan7ports);
			if (!nvram_matchi("dtag_vlan8", 1) || nvram_matchi("wan_vdsl", 0)) {
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
#else
	if (!donothing) {
		char vlanbuf[64];
		if (enable) {
			fprintf(stderr, "enable vlan port mapping %s/%s\n", vlan_lan_ports, vlan7ports);
			if (!nvram_matchi("dtag_vlan8", 1) || nvram_matchi("wan_vdsl", 0)) {
				char vl[32];
				sprintf(vl, "%d", lan_vlan_num);
				eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports",
				     brcm_to_swconfig(vlan_lan_ports, vlanbuf));
				start_setup_vlans();
				sprintf(vl, "%d", wan_vlan_num);
				eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", "");
				eval("swconfig", "dev", "switch0", "vlan", "7", "set", "ports",
				     brcm_to_swconfig(vlan7ports, vlanbuf));
				eval("swconfig", "dev", "switch0", "set", "apply");
			} else {
				char vl[32];
				sprintf(vl, "%d", lan_vlan_num);
				eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports",
				     brcm_to_swconfig(vlan_lan_ports, vlanbuf));
				start_setup_vlans();
				sprintf(vl, "%d", wan_vlan_num);
				eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", "");
				eval("swconfig", "dev", "switch0", "vlan", "7", "set", "ports",
				     brcm_to_swconfig(vlan7ports, vlanbuf));
				eval("swconfig", "dev", "switch0", "vlan", "8", "set", "ports",
				     brcm_to_swconfig(vlan7ports, vlanbuf));
				eval("swconfig", "dev", "switch0", "set", "apply");
			}
		} else {
			fprintf(stderr, "disable vlan port mapping %s/%s\n", vlan_lan_ports, vlan_wan_ports);
			eval("swconfig", "dev", "switch0", "vlan", "7", "set", "ports", "");
			eval("swconfig", "dev", "switch0", "vlan", "8", "set", "ports", "");
			char vl[32];
			sprintf(vl, "%d", lan_vlan_num);
			eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", brcm_to_swconfig(vlan_lan_ports, vlanbuf));
			sprintf(vl, "%d", wan_vlan_num);
			eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", brcm_to_swconfig(vlan_wan_ports, vlanbuf));
			eval("swconfig", "dev", "switch0", "set", "apply");
			start_setup_vlans();
		}
	}
#endif
	nvram_seti("fromvdsl", 0);
	return eth;
}

void start_dtag(void)
{
	enable_dtag_vlan(1);
}

char *set_wan_state(int state)
{
#ifdef HAVE_SWCONFIG
	char *v1 = nvram_safe_get("vlan0ports");
	char *v2 = nvram_safe_get("vlan1ports");
	int vlan2_supp = 0;
	if (!*v1 || *nvram_safe_get("vlan2ports")) {
		v1 = v2;
		vlan2_supp = 1;
		v2 = nvram_safe_get("vlan2ports");
	}
	char vlan2buf[64];
	char vlan1buf[64];
	char *vlan2 = brcm_to_swconfig(v2, vlan2buf);
	char *vlan1 = brcm_to_swconfig(v1, vlan1buf);

	if (!state) {
		char *p = strchr(vlan1, 't');
		if (!p)
			strcat(vlan1, "t");
		p = strchr(vlan2, 't');
		if (!p)
			strcat(vlan2, "t");
		strspcattach(vlan1, vlan2);
		eval("swconfig", "dev", "switch0", "set", "reset", "1");
		eval("swconfig", "dev", "switch0", "set", "enable_vlan", "1");
		eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", vlan1);
		eval("swconfig", "dev", "switch0", "set", "apply");
		eval("ifconfig", "eth0", "up");
		eval("vconfig", "rem", "vlan0");
		eval("vconfig", "rem", "vlan2");
		return "vlan1";
	} else {
		eval("swconfig", "dev", "switch0", "set", "reset", "1");
		eval("swconfig", "dev", "switch0", "set", "enable_vlan", "1");
		if (vlan2_supp) {
			eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", vlan1);
			eval("swconfig", "dev", "switch0", "vlan", "2", "set", "ports", vlan2);
			eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
			eval("vconfig", "add", "eth0", "1");
			eval("vconfig", "add", "eth0", "2");
		} else {
			eval("swconfig", "dev", "switch0", "vlan", "0", "set", "ports", vlan1);
			eval("swconfig", "dev", "switch0", "vlan", "1", "set", "ports", vlan2);
			eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
			eval("vconfig", "add", "eth0", "0");
			eval("vconfig", "add", "eth0", "1");
		}
		eval("swconfig", "dev", "switch0", "set", "apply");
	}
#endif
	return NULL;
}

void start_devinit_arch(void)
{
}
void load_wifi_drivers(void)
{
}
