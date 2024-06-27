/*
 * detect.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>
#include <glob.h>

#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <cy_conf.h>
#include <bcmdevs.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
#include <broadcom.h>

#ifdef HAVE_IPV6
#include <ifaddrs.h>
#endif
#ifndef IP_ALEN
#define IP_ALEN 4
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect((x), 0)
#endif

#define SIOCGMIIREG 0x8948 /* Read MII PHY register.  */
#define SIOCSMIIREG 0x8949 /* Write MII PHY register.  */

struct mii_ioctl_data {
	unsigned short phy_id;
	unsigned short reg_num;
	unsigned short val_in;
	unsigned short val_out;
};

#define TRX_MAGIC_F7D3301 0x20100322 /* Belkin Share Max; router's birthday ? */
#define TRX_MAGIC_F7D3302 0x20090928 /* Belkin Share; router's birthday ? */
#define TRX_MAGIC_F7D4302 0x20091006 /* Belkin Play; router's birthday ? */

#ifdef HAVE_FONERA
static void getBoardMAC(char *mac)
{
	// 102
	int i;
	char op[32];
	unsigned char data[256];
	FILE *in;

	sprintf(op, "/dev/mtdblock/%d", getMTD("board_config"));
	in = fopen(op, "rb");
	if (in == NULL)
		return;
	fread(data, 256, 1, in);
	fclose(in);
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", data[102] & 0xff, data[103] & 0xff, data[104] & 0xff, data[105] & 0xff,
		data[106] & 0xff, data[107] & 0xff);
}
#endif

void setRouter(char *name)
{
	if (name)
		nvram_set(NVROUTER, name);
#ifdef HAVE_POWERNOC_WORT54G
	nvram_set(NVROUTER_ALT, "WORT54G");
#elif HAVE_POWERNOC_WOAP54G
	nvram_set(NVROUTER_ALT, "WOAP54G");
#elif HAVE_ERC
	char *pic = NULL;
	pic = nvram_safe_get("ree_pic");

	if (!strncmp(pic, "1", 1))
		nvram_set(NVROUTER_ALT, "ServiceGate v1.0");
	else
		nvram_set(NVROUTER_ALT, "ServiceGate Lite v2.0");
#elif HAVE_OMNI
	nvram_set(NVROUTER_ALT, "Omni Wifi Router");
#elif HAVE_ALFA_BRANDING
	nvram_set(NVROUTER_ALT, "WLAN base-station");
#elif HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
	nvram_set(NVROUTER_ALT, "default");
#else
	nvram_set(NVROUTER_ALT, "MAKSAT");
#endif
#elif HAVE_TMK
	nvram_set(NVROUTER_ALT, "KMT-WAS");
#elif HAVE_BKM
	nvram_set(NVROUTER_ALT, "BKM-HSDL");
#elif HAVE_TRIMAX
	nvram_set(NVROUTER_ALT, "M2M Dynamics");
#elif HAVE_WIKINGS
#ifdef HAVE_SUB3
	nvram_set(NVROUTER_ALT, "ExcelMin");
#elif HAVE_SUB6
	nvram_set(NVROUTER_ALT, "ExcelMed");
#else
	nvram_set(NVROUTER_ALT, "Excellent");
#endif
#elif HAVE_SANSFIL
	nvram_set(NVROUTER_ALT, "SANSFIL");
#elif HAVE_ONNET
#ifdef HAVE_MMS344
#ifdef HAVE_WILLY
	nvram_set(NVROUTER_ALT, "9342-AN4g");
#elif HAVE_CPE880
	nvram_set(NVROUTER_ALT, "AP-9344");
#else
	nvram_set(NVROUTER_ALT, "DBDC344");
#endif
#elif HAVE_XD3200
	nvram_set(NVROUTER_ALT, "9563-AC");
#elif HAVE_XD9531
	nvram_set(NVROUTER_ALT, "9531");
#endif
#elif HAVE_RAYTRONIK
#ifdef HAVE_AC722
	nvram_set("NVROUTER_ALT", "RN-150M");
#else
#endif
#elif HAVE_ESPOD
#ifdef HAVE_SUB3
	nvram_set(NVROUTER_ALT, "ESPOD ES-3680");
#elif HAVE_SUB6
	nvram_set(NVROUTER_ALT, "ESPOD ES-3680");
#else
	nvram_set(NVROUTER_ALT, "ESPOD ES-3680");
#endif
#elif HAVE_CARLSONWIRELESS
#ifdef HAVE_LAGUNA
	nvram_set(NVROUTER_ALT, "LH-ST (Rev.2)");
#else
	nvram_set(NVROUTER_ALT, "LH-135/270 ST");
#endif
#elif HAVE_IPR
	nvram_set(NVROUTER_ALT, "IPR-DATKS-501");
#elif defined(HAVE_ANTAIRA) && defined(HAVE_FMS2111)
	nvram_set(NVROUTER_ALT, "AMS2111");
#elif HAVE_ANTAIRA
#ifdef HAVE_ANTAIRA_MINI
	nvram_set(NVROUTER_ALT, "Industrial Access Point");
#else
	nvram_set(NVROUTER_ALT, "Industrial Router");
#endif /*HAVE_ANTAIRA_MINI */
#elif HAVE_NDTRADE
	nvram_set(NVROUTER_ALT, "KT412H-8000");
#endif
	cprintf("router is %s\n", getRouter());
}

char *getRouter()
{
	char *n = nvram_safe_get(NVROUTER);

	return *n ? n : "Unknown Model";
}

int internal_getRouterBrand()
{
#if defined(HAVE_ALLNETWRT) && !defined(HAVE_ECB9750)
	unsigned long boardnum = strtoul(nvram_safe_get("boardnum"), NULL, 0);

	if (boardnum == 8 && nvram_match("boardtype", "0x048e") && nvram_match("boardrev", "0x11")) {
		setRouter("ALLNET EUROWRT 54");
		return ROUTER_ALLNET01;
	}
	eval("event", "3", "1", "15");
	return 0;
#elif defined(HAVE_ALLNETWRT) && defined(HAVE_EOC5610)
	setRouter("Allnet Outdoor A/B/G CPE");
	return ROUTER_BOARD_LS2;
#else
#ifdef HAVE_NP28G
	setRouter("Compex NP28G");
	return ROUTER_BOARD_NP28G;
#elif HAVE_E200
	setRouter("Ubiquiti Edgerouter Pro");
	return ROUTER_UBNT_EROUTERPRO;
#elif HAVE_EROUTER
	setRouter("Ubiquiti Edgerouter Lite");
	return ROUTER_UBNT_EROUTERLITE;
#elif HAVE_WP54G
	setRouter("Compex WP54G");
	return ROUTER_BOARD_WP54G;
#elif HAVE_ADM5120
	if (!nvram_match("DD_BOARD", "OSBRiDGE 5LXi"))
		setRouter("Tonze AP-120");
	return ROUTER_BOARD_ADM5120;
#elif HAVE_RB500
	setRouter("Mikrotik RB500");
	return ROUTER_BOARD_500;
#elif HAVE_GEMTEK
	setRouter("SuperGerry");
	return ROUTER_SUPERGERRY;
#elif HAVE_NORTHSTAR
	unsigned long boardnum = strtoul(nvram_safe_get("boardnum"), NULL, 0);

	if (boardnum == 00 && nvram_match("boardtype", "0xF646") && nvram_match("boardrev", "0x1100") &&
	    nvram_match("melco_id", "RD_BB12068")) {
#ifdef HAVE_BUFFALO
		setRouter("WZR-1750DHP");
#else
		setRouter("Buffalo WZR-1750DHP");
#endif
		return ROUTER_BUFFALO_WZR1750;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") &&
	    nvram_match("melco_id", "RD_BB13049")) {
#ifdef HAVE_BUFFALO
		setRouter("WXR-1900DHP");
#else
		setRouter("Buffalo WXR-1900DHP");
#endif
		return ROUTER_BUFFALO_WXR1900DHP;
	}

	if ((!strncmp(nvram_safe_get("boardnum"), "2013", 4) || !strncmp(nvram_safe_get("boardnum"), "2014", 4)) &&
	    nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110") && nvram_matchi("0:rxchain", 7)) {
#ifdef HAVE_BUFFALO
		setRouter("WZR-900DHP");
#else
		setRouter("Buffalo WZR-900DHP");
#endif
		return ROUTER_BUFFALO_WZR900DHP;
	}

	if ((!strncmp(nvram_safe_get("boardnum"), "2013", 4) || !strncmp(nvram_safe_get("boardnum"), "2014", 4)) &&
	    nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110") && nvram_matchi("0:rxchain", 3)) {
#ifdef HAVE_BUFFALO
		setRouter("WZR-600DHP2");
#else
		setRouter("Buffalo WZR-600DHP2");
#endif
		return ROUTER_BUFFALO_WZR600DHP2;
	}
	if (nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1102") && boardnum == 1 &&
	    nvram_match("reset_gpio", "11")) {
		setRouter("TP-Link Archer C8");

		return ROUTER_TPLINK_ARCHERC8;
	}

	if (nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1102") && boardnum == 1) {
		setRouter("TP-Link Archer C9");

		return ROUTER_TPLINK_ARCHERC9;
	}

	if (nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1112") && boardnum == 1) {
		setRouter("TP-Link Archer C8");

		return ROUTER_TPLINK_ARCHERC9;
	}

	if (nvram_match("boardtype", "0x072F") && nvram_match("boardrev", "0x1101") && (boardnum == 1234 || boardnum == 10)) {
		setRouter("TP-Link Archer C3150");

		return ROUTER_TPLINK_ARCHERC3150;
	}

	if (nvram_match("boardtype", "0x072F") && nvram_match("boardrev", "0x1102") && (boardnum == 1234 || boardnum == 10)) {
		setRouter("TP-Link Archer C3200");

		return ROUTER_TPLINK_ARCHERC3150;
	}

	if (nvram_match("boardtype", "0xD646") && nvram_match("boardrev", "0x1100") && nvram_match("0:devid", "0x43A1")) {
		setRouter("Linksys EA6900");

		return ROUTER_LINKSYS_EA6900;
	}

	if (nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") && nvram_match("boardnum", "01") &&
	    !strncmp(nvram_safe_get("modelNumber"), "EA6400", 6)) {
		setRouter("Linksys EA6400");

		return ROUTER_LINKSYS_EA6400;
	}

	if (nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") && nvram_match("boardnum", "01") &&
	    !strncmp(nvram_safe_get("modelNumber"), "EA6300", 6)) {
		setRouter("Linksys EA6300");

		return ROUTER_LINKSYS_EA6400;
	}

	if (nvram_match("boardtype", "0xF646") && nvram_match("boardrev", "0x1100") && nvram_match("0:devid", "0x4332")) {
		setRouter("Linksys EA6700");

		return ROUTER_LINKSYS_EA6700;
	}

	if (nvram_match("boardtype", "0xE646") && nvram_match("boardrev", "0x1200") && nvram_matchi("boardnum", 20140309)) {
		setRouter("Linksys EA6350");

		return ROUTER_LINKSYS_EA6350;
	}

	if (nvram_match("boardtype", "0xE646") && nvram_match("boardrev", "0x1100") && nvram_matchi("boardnum", 20130125)) {
		setRouter("Linksys EA6200");

		return ROUTER_LINKSYS_EA6350;
	}

	if (nvram_match("boardtype", "0xF646") && nvram_match("boardrev", "0x1100")) {
		setRouter("Linksys EA6500 V2");

		return ROUTER_LINKSYS_EA6500V2;
	}

	if ((nvram_match("boardtype", "0xd72F") || nvram_match("boardtype", "0xA72F")) && nvram_match("boardrev", "0x1100")) {
		setRouter("Linksys EA9500");

		return ROUTER_LINKSYS_EA9500;
	}

	if (nvram_match("productid", "RT-AC56U")) {
		setRouter("Asus RT-AC56U");
		return ROUTER_ASUS_AC56U;
	}

	if (nvram_match("productid", "RT-AC67U")) {
		setRouter("Asus RT-AC67U");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("odmpid", "RT-AC68R")) {
		setRouter("Asus RT-AC68R");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("odmpid", "RT-AC1900P")) {
		setRouter("Asus RT-AC1900P");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("odmpid", "RT-AC68A")) {
		setRouter("Asus RT-AC68A");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("model", "RT-AC1200G+")) {
		setRouter("Asus RT-AC1200G+");
		return ROUTER_ASUS_AC1200;
	}

	if (nvram_match("productid", "RT-AC68U") && nvram_match("boardrev", "0x1100")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("productid", "RT-AC68U") && nvram_match("boardrev", "0x1103") && nvram_match("boardtype", "0x0665")) {
		setRouter("Asus RT-AC68U B1");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("productid", "RT-AC68U") && nvram_match("boardrev", "0x1103")) {
		setRouter("Asus RT-AC68U C1");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("model", "RT-AC68U") && nvram_match("boardrev", "0x1100")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("model", "RT-AC68U") && nvram_match("boardrev", "0x1500")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("model", "RT-AC68U") && nvram_match("boardrev", "0x1103") && nvram_match("boardtype", "0x0665")) {
		setRouter("Asus RT-AC68U B1");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("model", "RT-AC68U") && nvram_match("boardrev", "0x1103")) {
		setRouter("Asus RT-AC68U C1");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("productid", "RT-AC87U") || nvram_match("model", "RT-AC87U")) {
		setRouter("Asus RT-AC87U");
		return ROUTER_ASUS_AC87U;
	}

	if (nvram_match("model", "RT-AC88U")) {
		setRouter("Asus RT-AC88U");
		return ROUTER_ASUS_AC88U;
	}

	if (nvram_match("model", "RT-AC3100")) {
		setRouter("Asus RT-AC3100");
		return ROUTER_ASUS_AC3100;
	}
	if (nvram_match("productid", "RT-AC3100")) {
		setRouter("Asus RT-AC3100");
		return ROUTER_ASUS_AC3100;
	}

	if (nvram_match("odmpid", "RT-AC3100")) {
		setRouter("Asus RT-AC3100");
		return ROUTER_ASUS_AC3100;
	}

	if (nvram_match("model", "RT-AC5300")) {
		setRouter("Asus RT-AC5300");
		return ROUTER_ASUS_AC5300;
	}

	if (nvram_match("productid", "RT-AC3200") || nvram_match("model", "RT-AC3200")) {
		setRouter("Asus RT-AC3200");
		return ROUTER_ASUS_AC3200;
	}

	if (nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1103") && !nvram_match("melco_id", "RD_BB13049")) {
		setRouter("Asus RT-AC87U");
		return ROUTER_ASUS_AC87U;
	}

	if (nvram_match("odmpid", "RT-AC87U")) {
		setRouter("Asus RT-AC87U");
		return ROUTER_ASUS_AC87U;
	}

	if (nvram_match("model", "RT-N18U")) {
		setRouter("Asus RT-N18U");
		return ROUTER_ASUS_RTN18U;
	}

	if (boardnum == 24 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") &&
	    nvram_match("gpio8", "wps_button")) {
		setRouter("Dlink-DIR860L-A1");
		return ROUTER_DLINK_DIR860;
	}
	if (boardnum == 24 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110") &&
	    nvram_match("gpio7", "wps_button") && !nvram_match("gpio6", "wps_led") && nvram_exists("pci/1/1/venid")) {
		setRouter("Dlink-DIR868L");
		return ROUTER_DLINK_DIR868;
	}

	if (boardnum == 24 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1101") &&
	    nvram_match("gpio7", "wps_button") && !nvram_match("gpio6", "wps_led")) {
		setRouter("Dlink-DIR868L rev C");
		return ROUTER_DLINK_DIR868C;
	}

	if (boardnum == 24 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110") &&
	    nvram_match("gpio7", "wps_button") && nvram_match("gpio6", "wps_led")) {
		setRouter("Dlink-DIR880L");
		return ROUTER_DLINK_DIR880;
	}

	if ((boardnum == 24 || nvram_match("boardnum", "N/A")) && nvram_match("boardtype", "0x072F") &&
	    nvram_match("2:devid", "0x43c5") && nvram_match("boardrev", "0x1101") && nvram_match("gpio7", "wps_button")) {
		setRouter("Dlink-DIR895L");
		return ROUTER_DLINK_DIR895;
	}

	if ((boardnum == 24 || nvram_match("boardnum", "N/A")) && nvram_match("boardtype", "0x072F") &&
	    nvram_match("1:devid", "0x43c5") && nvram_match("boardrev", "0x1101") && nvram_match("gpio7", "wps_button")) {
		setRouter("Dlink-DIR885L");
		return ROUTER_DLINK_DIR885;
	}

	if (boardnum == 24 && nvram_match("boardtype", "0x072F") && nvram_match("boardrev", "0x1101") &&
	    nvram_match("gpio7", "wps_button")) {
		setRouter("Dlink-DIR890L");
		return ROUTER_DLINK_DIR890;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") &&
	    nvram_match("gpio15", "wps_button")) {
		setRouter("Asus RT-AC56U");
		return ROUTER_ASUS_AC56U;
	}

	if (boardnum == 1234 && nvram_match("boardtype", "0x072F") && nvram_match("boardrev", "0x1202") &&
	    nvram_match("gpio7", "wps_button")) {
		setRouter("Trendnet TEW828DRU");
		return ROUTER_TRENDNET_TEW828;
	}

	if (boardnum == 1234 && nvram_match("boardtype", "0x0646") && nvram_match("1:devid", "0x43A2") &&
	    nvram_match("boardrev", "0x1100") && nvram_match("gpio7", "wps_button")) {
		setRouter("Trendnet TEW812DRU");
		return ROUTER_TRENDNET_TEW812;
	}

	if (boardnum == 1234 && nvram_match("boardtype", "0xC646") && nvram_match("boardrev", "0x1200") &&
	    nvram_match("gpio7", "wps_button")) {
		setRouter("Trendnet TEW818DRU");
		return ROUTER_TRENDNET_TEW812;
	}

	if (boardnum == 1234 && nvram_match("boardtype", "0xD646") && nvram_match("1:devid", "0x43A2") &&
	    nvram_match("boardrev", "0x1100") && nvram_match("gpio7", "wps_button")) {
		setRouter("Trendnet TEW812DRU");
		return ROUTER_TRENDNET_TEW812;
	}

	if (boardnum == 1234 && nvram_match("boardtype", "0xD646") && nvram_match("1:devid", "0x43A9") &&
	    nvram_match("boardrev", "0x1100") && nvram_match("gpio7", "wps_button")) {
		setRouter("Trendnet TEW811DRU");
		return ROUTER_TRENDNET_TEW812;
	}

	if (boardnum != 24 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1100") &&
	    nvram_match("gpio7", "wps_button")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}

	if (boardnum == 679 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110")) {
		int mtd = getMTD("board_data");
		char devname[32];
		sprintf(devname, "/dev/mtdblock/%d", mtd);
		FILE *model = fopen(devname, "rb");
		if (model) {
#define R6300V2 "U12H240T00_NETGEAR"
#define R6300V2CH "U12H240T70_NETGEAR"
#define AC1450 "U12H240T99_NETGEAR"
#define EX6200 "U12H269T00_NETGEAR"
			char modelstr[32];
			fread(modelstr, 1, strlen(R6300V2), model);
			if (!strncmp(modelstr, R6300V2, strlen(R6300V2)) || !strncmp(modelstr, R6300V2CH, strlen(R6300V2CH))) {
				setRouter("Netgear R6300V2");
				fclose(model);
				return ROUTER_NETGEAR_R6300V2;
			} else if (!strncmp(modelstr, AC1450, strlen(AC1450))) {
				setRouter("Netgear AC1450");
				fclose(model);
				return ROUTER_NETGEAR_AC1450;
			} else if (!strncmp(modelstr, EX6200, strlen(EX6200))) {
				setRouter("Netgear EX6200");
				fclose(model);
				return ROUTER_NETGEAR_EX6200;
			} else {
				setRouter("Netgear R6250");
				fclose(model);
				return ROUTER_NETGEAR_R6250;
			}
		}
	}

	if (boardnum == 32 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1301")) {
		if (nvram_match("board_id", "U12H270T10_NETGEAR")) {
			setRouter("Netgear R6700");
		} else {
			setRouter("Netgear R7000");
		}
		return ROUTER_NETGEAR_R7000;
	}

	if (boardnum == 32 && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1601")) {
		if (nvram_match("board_id", "U12H270T20_NETGEAR")) {
			setRouter("Netgear R7000P");
			return ROUTER_NETGEAR_R7000P;
		} else if (nvram_match("board_id", "U12H332T20_NETGEAR") || nvram_match("board_id", "U12H332T30_NETGEAR")) {
			setRouter("Netgear R6400 v2");
			return ROUTER_NETGEAR_R6400V2;
		} else if (nvram_match("board_id", "U12H332T77_NETGEAR")) {
			setRouter("Netgear R6700 v3");
			return ROUTER_NETGEAR_R6700V3;
		} else {
			setRouter("Netgear R6400 v1");
			return ROUTER_NETGEAR_R6400;
		}
	}

	if (boardnum == 32 && nvram_match("boardtype", "0x0665") && nvram_match("boardrev", "0x1101")) {
		setRouter("Netgear R8000");
		return ROUTER_NETGEAR_R8000;
	}

	if (boardnum == 32 && nvram_match("boardtype", "0x072F") && nvram_match("boardrev", "0x1101")) {
		setRouter("Netgear R8500");
		return ROUTER_NETGEAR_R8500;
	}

	setRouter("Broadcom Northstar");
	return ROUTER_BOARD_NORTHSTAR;
	setRouter("Broadcom Northstar");
	return ROUTER_BOARD_NORTHSTAR;
#elif HAVE_NEWPORT
	char *filename = "/sys/bus/i2c/devices/0-0051/eeprom"; /* bank2=0x100 kernel 3.0 */
	FILE *file = fopen(filename, "rb");
	if (!file) {
		setRouter("Gateworks NewPort GW6XXX");
	} else {
		char gwid[9];
		fseek(file, 0x30, SEEK_SET);
		fread(&gwid[0], 9, 1, file);
		fclose(file);
		if (!strncmp(gwid, "GW6100", 6)) {
			setRouter("Gateworks Newport GW6100");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW6104", 6)) {
			setRouter("Gateworks Newport GW6104");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW6200", 6)) {
			setRouter("Gateworks Newport GW6200");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6204", 6)) {
			setRouter("Gateworks Newport GW6204");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6300", 6)) {
			setRouter("Gateworks Newport GW6300");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6304", 6)) {
			setRouter("Gateworks Newport GW6304");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6400", 6)) {
			setRouter("Gateworks Newport GW6400");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6402", 6)) {
			setRouter("Gateworks Newport GW6402");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6404", 6)) {
			setRouter("Gateworks Newport GW6404");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6500", 6)) {
			setRouter("Gateworks Newport GW6500");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6504", 6)) {
			setRouter("Gateworks Newport GW6504");
			return ROUTER_BOARD_GW6400;
		} else if (!strncmp(gwid, "GW6903", 6)) {
			setRouter("Gateworks Newport GW6903");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW6904", 6)) {
			setRouter("Gateworks Newport GW6904");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW6905", 6)) {
			setRouter("Gateworks Newport GW6905");
			return ROUTER_BOARD_GW2380;
		} else
			setRouter("Gateworks Newport GW6XXX");
	}
	return ROUTER_BOARD_GW2388;
#elif HAVE_VENTANA
	char *filename = "/sys/devices/soc0/soc.0/2100000.aips-bus/21a0000.i2c/i2c-0/0-0051/eeprom"; /* bank2=0x100 kernel 3.0 */
	FILE *file = fopen(filename, "rb");
	if (!file) {
		filename = "/sys/devices/soc0/soc/2100000.aips-bus/21a0000.i2c/i2c-0/0-0051/eeprom"; /* bank2=0x100 kernel 3.18 */
		file = fopen(filename, "rb");
	}
	if (!file) {
		setRouter("Gateworks Ventana GW54XX");
	} else {
		char gwid[9];
		fseek(file, 0x30, SEEK_SET);
		fread(&gwid[0], 9, 1, file);
		fclose(file);
		if (!strncmp(gwid, "GW5400-B", 8)) {
			setRouter("Gateworks Ventana GW5400-B");
		} else if (!strncmp(gwid, "GW5400-C", 8)) {
			setRouter("Gateworks Ventana GW5400-C");
		} else if (!strncmp(gwid, "GW5400-A", 8)) {
			setRouter("Gateworks Ventana GW5400-A");
		} else if (!strncmp(gwid, "GW5400", 6)) {
			setRouter("Gateworks Ventana GW5400-X");
		} else if (!strncmp(gwid, "GW5410-B", 8)) {
			setRouter("Gateworks Ventana GW5410-B");
		} else if (!strncmp(gwid, "GW5410-C", 8)) {
			setRouter("Gateworks Ventana GW5410-C");
		} else if (!strncmp(gwid, "GW5410-E1", 9)) {
			setRouter("Gateworks Ventana GW5410-E1");
		} else if (!strncmp(gwid, "GW5410", 6)) {
			setRouter("Gateworks Ventana GW5410-X");
		} else if (!strncmp(gwid, "GW5100", 6)) {
			setRouter("Gateworks Ventana GW5100");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW5200", 6)) {
			setRouter("Gateworks Ventana GW5200");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW5300", 6)) {
			setRouter("Gateworks Ventana GW5300");
		} else if (!strncmp(gwid, "GW5310", 6)) {
			setRouter("Gateworks Ventana GW5310");
		} else if (!strncmp(gwid, "GW551", 5)) {
			setRouter("Gateworks Ventana GW5510");
		} else if (!strncmp(gwid, "GW552", 5)) {
			setRouter("Gateworks Ventana GW5520");
		} else if (!strncmp(gwid, "GW553", 5)) {
			setRouter("Gateworks Ventana GW5530");
		} else
			setRouter("Gateworks Ventana GW54XX");
	}
	return ROUTER_BOARD_GW2388;
#elif HAVE_LAGUNA
	char *filename = "/sys/devices/platform/cns3xxx-i2c.0/i2c-0/0-0050/eeprom"; /* bank2=0x100 kernel 3.0 */
	FILE *file = fopen(filename, "rb");
	if (!file) {
		filename = "/sys/devices/platform/cns3xxx-i2c.0/i2c-adapter/i2c-0/0-0050/eeprom"; /* bank2=0x100 older kernel */
		file = fopen(filename, "r");
	}
	if (file) {
		fseek(file, 0x130, SEEK_SET);
		char gwid[9];
		fread(&gwid[0], 9, 1, file);
		fclose(file);
		if (!strncmp(gwid, "GW2388", 6)) {
			setRouter("Gateworks Laguna GW2388");
			return ROUTER_BOARD_GW2388;
		} else if (!strncmp(gwid, "GW2389", 6)) {
			setRouter("Gateworks Laguna GW2389");
			return ROUTER_BOARD_GW2388;
		} else if (!strncmp(gwid, "GW2384", 6)) {
			setRouter("Gateworks Laguna GW2384");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2394", 6)) {
			setRouter("Gateworks Laguna GW2394");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2386", 6)) {
			setRouter("Gateworks Laguna GW2386");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2385", 6)) {
			setRouter("Gateworks Laguna GW2385");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2382", 6)) {
			setRouter("Gateworks Laguna GW2382");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2380", 6)) {
			setRouter("Gateworks Laguna GW2380");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2391", 6)) {
			setRouter("Gateworks Laguna GW2391");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2393", 6)) {
			setRouter("Gateworks Laguna GW2393");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2387", 6)) {
			setRouter("Gateworks Laguna GW2387");
			return ROUTER_BOARD_GW2380;
		} else {
			setRouter("Gateworks Laguna GWXXXX");
			return ROUTER_BOARD_GW2388;
		}
	} else {
		setRouter("Gateworks Laguna UNKNOWN");
		return ROUTER_BOARD_GW2388;
	}
#elif HAVE_MI424WR
	setRouter("Actiontec MI424WR");
	return ROUTER_BOARD_GATEWORX_GW2345;
#elif HAVE_USR8200
	setRouter("US Robotics USR8200");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_TONZE
	setRouter("Tonze AP-425");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_NOP8670
	setRouter("Senao NOP-8670");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_WRT300NV2
	setRouter("Linksys WRT300N v2");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_WG302V1
	setRouter("Netgear WG302 v1");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_WG302
	setRouter("Netgear WG302 v2");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_PRONGHORN
	setRouter("ADI Engineering Pronghorn Metro");
	return ROUTER_BOARD_GATEWORX;
#elif HAVE_GATEWORX
	char *filename = "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0051/eeprom"; /* bank2=0x100 
												 */
	FILE *file = fopen(filename, "r");
	if (!file) {
		filename = "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom"; //for 2.6.34.6
		file = fopen(filename, "r");
	}
	if (file) // new detection scheme
	{
		char *gwid;
		char temp[64];
		safe_fread(temp, 1, 40, file);
		gwid = &temp[32];
		gwid[8] = 0;
		fclose(file);
		if (!strncmp(gwid, "GW2347", 6)) {
			setRouter("Gateworks Avila GW2347");
			return ROUTER_BOARD_GATEWORX_SWAP;
		}
		if (!strncmp(gwid, "GW2357", 6)) {
			setRouter("Gateworks Avila GW2357");
			return ROUTER_BOARD_GATEWORX_SWAP;
		}
		if (!strncmp(gwid, "GW2353", 6)) {
			setRouter("Gateworks Avila GW2353");
			return ROUTER_BOARD_GATEWORX;
		}
		if (!strncmp(gwid, "GW2348-2", 8)) {
			setRouter("Gateworks Avila GW2348-2");
			return ROUTER_BOARD_GATEWORX;
		}
		if (!strncmp(gwid, "GW2348-4", 8)) {
			setRouter("Gateworks Avila GW2348-4");
			return ROUTER_BOARD_GATEWORX;
		}
		if (!strncmp(gwid, "GW2348", 6)) {
			setRouter("Gateworks Avila GW2348-4/2");
			return ROUTER_BOARD_GATEWORX;
		}
		if (!strncmp(gwid, "GW2358", 6)) {
			setRouter("Gateworks Cambria GW2358-4");
			return ROUTER_BOARD_GATEWORX;
		}
		if (!strncmp(gwid, "GW2350", 6)) {
			setRouter("Gateworks Cambria GW2350");
			return ROUTER_BOARD_GATEWORX;
		}
		if (!strncmp(gwid, "GW2369", 6)) {
			setRouter("Gateworks Avila GW2369");
			return ROUTER_BOARD_GATEWORX_GW2369;
		}
		if (!strncmp(gwid, "GW2355", 6)) {
			setRouter("Gateworks Avila GW2355");
			return ROUTER_BOARD_GATEWORX_GW2345;
		}
		if (!strncmp(gwid, "GW2345", 6)) {
			setRouter("Gateworks Avila GW2345");
			return ROUTER_BOARD_GATEWORX_GW2345;
		}
	}
old_way:;
	struct mii_ioctl_data *data;
	struct ifreq iwr;
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		setRouter("Gateworks Avila");
		return ROUTER_BOARD_GATEWORX;
	}
	(void)strncpy(iwr.ifr_name, "ixp0", sizeof("ixp0"));
	data = (struct mii_ioctl_data *)&iwr.ifr_data;
	data->phy_id = 1;
#define IX_ETH_ACC_MII_PHY_ID1_REG 0x2 /* PHY identifier 1 Register */
#define IX_ETH_ACC_MII_PHY_ID2_REG 0x3 /* PHY identifier 2 Register */
	data->reg_num = IX_ETH_ACC_MII_PHY_ID1_REG;
	ioctl(s, SIOCGMIIREG, &iwr);
	data->phy_id = 1;
	data->reg_num = IX_ETH_ACC_MII_PHY_ID1_REG;
	ioctl(s, SIOCGMIIREG, &iwr);
	int reg1 = data->val_out;

	data->phy_id = 1;
	data->reg_num = IX_ETH_ACC_MII_PHY_ID2_REG;
	ioctl(s, SIOCGMIIREG, &iwr);
	int reg2 = data->val_out;

	close(s);
	if (reg1 == 0x2000 && reg2 == 0x5c90) {
		setRouter("Avila GW2347");
		return ROUTER_BOARD_GATEWORX_SWAP;
	} else if (reg1 == 0x13 && reg2 == 0x7a11) {
#if HAVE_ALFA_BRANDING
		setRouter("WLAN base-station");
#else
		setRouter("Gateworks Avila GW2348-4/2");
#endif
		return ROUTER_BOARD_GATEWORX;
	} else if (reg1 == 0x143 && reg2 == 0xbc31) // broadcom phy
	{
		setRouter("ADI Engineering Pronghorn Metro");
		return ROUTER_BOARD_GATEWORX;
	} else if (reg1 == 0x22 && reg2 == 0x1450) // kendin switch
	{
		setRouter("Gateworks Avila GW2345");
		return ROUTER_BOARD_GATEWORX_GW2345;
	} else if (reg1 == 0x0 && reg2 == 0x8201) // realtek
	{
		setRouter("Compex WP188");
		return ROUTER_BOARD_GATEWORX;
	} else {
		setRouter("Unknown");
		return ROUTER_BOARD_GATEWORX;
	}
#elif HAVE_RT2880

#ifdef HAVE_ECB9750
#ifdef HAVE_ALLNETWRT
	setRouter("Allnet 802.11n Router");
#else
	setRouter("Senao ECB-9750");
#endif
	return ROUTER_BOARD_ECB9750;
#elif HAVE_ALLNET11N
	setRouter("Allnet 802.11n Router");
	return ROUTER_BOARD_WHRG300N;
#elif HAVE_TECHNAXX3G
	setRouter("Technaxx 3G Router");
	return ROUTER_BOARD_TECHNAXX3G;
#elif HAVE_AR670W
	setRouter("Airlink 101 AR670W");
	return ROUTER_BOARD_AR670W;
#elif HAVE_AR690W
	setRouter("Airlink 101 AR690W");
	return ROUTER_BOARD_AR690W;
#elif HAVE_RT15N
	setRouter("Asus RT-N15");
	return ROUTER_BOARD_RT15N;
#elif HAVE_BR6574N
	setRouter("Edimax BR-6574N");
	return ROUTER_BOARD_BR6574N;
#elif HAVE_ESR6650
	setRouter("Senao ESR-6650");
	return ROUTER_BOARD_ESR6650;
#elif HAVE_EAP9550
	setRouter("Senao EAP-9550");
	return ROUTER_BOARD_EAP9550;
#elif HAVE_ESR9752
	setRouter("Senao ESR-9752");
	return ROUTER_BOARD_ESR9752;
#elif HAVE_ACXNR22
	setRouter("Aceex NR22");
	return ROUTER_BOARD_ACXNR22;
#elif HAVE_W502U
	setRouter("Alfa AIP-W502U");
	return ROUTER_BOARD_W502U;
#elif HAVE_DIR615H
	setRouter("Dlink-DIR615 rev h");
	return ROUTER_BOARD_DIR615D;
#elif HAVE_ALL02310N
	setRouter("Allnet ALL02310N");
	return ROUTER_BOARD_DIR615D;
#elif HAVE_DIR615
	setRouter("Dlink-DIR615 rev d");
	return ROUTER_BOARD_DIR615D;
#elif HAVE_RT3352
	setRouter("Ralink RT3352 Device");
	return ROUTER_BOARD_RT3352;
#elif HAVE_RUT500
	setRouter("Teltonika RUT500");
	return ROUTER_BOARD_NEPTUNE;
#elif HAVE_NEPTUNE
	setRouter("Neptune-Mini");
	return ROUTER_BOARD_NEPTUNE;
#elif HAVE_TECHNAXX
	setRouter("TECHNAXX Router-150 Wifi-N");
	return ROUTER_BOARD_TECHNAXX;
#elif HAVE_RT10N
	setRouter("Asus RT-N10+");
	return ROUTER_ASUS_RTN10PLUS;
#elif HAVE_DIR600
#ifdef HAVE_DIR300
	setRouter("Dlink-DIR300 rev b");
#else
	setRouter("Dlink-DIR600 rev b");
#endif
	return ROUTER_BOARD_DIR600B;
#elif HAVE_RT13NB1
	setRouter("Asus RT-N13U B1");
	return ROUTER_BOARD_WHRG300N;
#elif HAVE_ASUSRTN13U
	setRouter("Asus RT-N13U");
	return ROUTER_BOARD_WHRG300N;
#elif HAVE_E1700
	setRouter("Linksys E1700 / N300");
	return ROUTER_BOARD_E1700;
#elif HAVE_R6800
	FILE *fp = fopen("/sys/firmware/devicetree/base/model", "rb");
	if (!fp) {
		fprintf(stderr, "error opening device tree\n");
		setRouter("Netgear R6800");
		return ROUTER_R6800;
	}
	char vendorstr[32];
	char modelstr[32];
	fscanf(fp, "%s %s", &vendorstr[0], &modelstr[0]);
	fclose(fp);
	if (!strcmp(modelstr, "R6800")) {
		setRouter("Netgear R6800");
		return ROUTER_R6800;
	}
	if (!strcmp(modelstr, "R6850")) {
		setRouter("Netgear R6850");
		return ROUTER_R6850;
	}
	if (!strcmp(modelstr, "R6260")) {
		setRouter("Netgear R6260");
		return ROUTER_R6850;
	}
	if (!strcmp(modelstr, "R6350")) {
		setRouter("Netgear R6350");
		return ROUTER_R6850;
	}
	if (!strcmp(modelstr, "R6220")) {
		setRouter("Netgear R6220");
		return ROUTER_R6220;
	}
	if (!strcmp(modelstr, "WAC124")) {
		setRouter("Netgear WAC124");
		return ROUTER_R6850;
	}
	if (!strcmp(modelstr, "WNDR3700V5")) {
		setRouter("Netgear WNDR3700V5");
		return ROUTER_R6220;
	}
	//fallback
	setRouter("Netgear R6800");
	return ROUTER_R6800;
#elif HAVE_DIR860
	FILE *fp = fopen("/dev/mtdblock3", "rb");
	if (fp) {
		fseek(fp, 64, SEEK_SET);
		char sign[32];
		fread(&sign, 32, 1, fp);
		if (!memcmp(sign, "DIR-882", 7)) {
			fclose(fp);
			setRouter("Dlink DIR-882 A1");
			return ROUTER_DIR882;
		}
		if (!memcmp(sign, "DIR-878", 7)) {
			fclose(fp);
			setRouter("Dlink DIR-878 A1");
			return ROUTER_DIR882;
		}
		fseek(fp, 32, SEEK_SET);
		fread(&sign, 32, 1, fp);
		fclose(fp);
		if (!memcmp(sign, "DIR_882", 7)) {
			setRouter("Dlink DIR-882 R1");
			return ROUTER_DIR882;
		}
		if (!memcmp(sign, "DIR_878", 7)) {
			setRouter("Dlink DIR-878 R1");
			return ROUTER_DIR882;
		}
	}
	setRouter("Dlink DIR-860L B1");
	return ROUTER_DIR860LB1;
#elif HAVE_DIR810L
	void *getUEnv(char *name);

	char *hw = getUEnv("HW_BOARD_REV");
	if (hw) {
		if (!strcmp(hw, "B1"))
			setRouter("Dlink DIR-810L B1");
		else if (!strcmp(hw, "A1"))
			setRouter("Dlink DIR-810L A1");
		else if (!strcmp(hw, "C1"))
			setRouter("Dlink DIR-810L C1");
		else
			setRouter("Dlink DIR-810L XX");

	} else
		setRouter("Dlink DIR-810L");
	return ROUTER_DIR810L;
#elif HAVE_WHR1166D
	setRouter("Buffalo WHR-1166D");
	return ROUTER_WHR300HP2;
#elif HAVE_WHR300HP2
	FILE *fp = fopen("/sys/bus/pci/devices/0000:01:00.0/device", "rb");
	if (fp) {
		fclose(fp);
		setRouter("Buffalo WHR-600D");
		return ROUTER_WHR300HP2;
	}
	setRouter("Buffalo WHR-300HP2");
	return ROUTER_WHR300HP2;
#elif HAVE_F5D8235
	setRouter("Belkin F5D8235-4 v2");
	return ROUTER_BOARD_F5D8235;
#elif HAVE_HAMEA15
	setRouter("Hame A-15");
	return ROUTER_BOARD_HAMEA15;
#elif HAVE_WCRGN
	setRouter("Buffalo WCR-GN");
	return ROUTER_BOARD_WCRGN;
#elif HAVE_WHRG300N
	setRouter("Buffalo WHR-G300N");
	return ROUTER_BOARD_WHRG300N;
#elif HAVE_WR5422
	setRouter("Repotec RP-WR5422");
	return ROUTER_BOARD_WR5422;
#else
	setRouter("Generic RT2880");
	return ROUTER_BOARD_RT2880;
#endif
#elif HAVE_X86
#ifdef HAVE_CORENET
	setRouter("CORENET X86i");
	return ROUTER_BOARD_X86;
#else
	char name[64];
	FILE *fp = fopen("/sys/devices/virtual/dmi/id/board_vendor", "rb");
	if (!fp)
		fp = fopen("/sys/devices/virtual/dmi/id/sys_vendor", "rb");
	if (!fp)
		fp = fopen("/sys/devices/virtual/dmi/id/chassis_vendor", "rb");
	if (!fp)
		goto generic;
	int len = 0;
	int b;
	while ((b = getc(fp)) != EOF && len < (sizeof(name) - 1)) {
		if (b == 0xa || b == 0)
			break;
		name[len++] = b;
	}
	fclose(fp);
	if (len < 1)
		goto generic;
	if (!strncasecmp(name, "To be filled", 12) || !strncasecmp(name, "System Product Name", 20) ||
	    !strncasecmp(name, "Default string", 14)) {
		name[0] = 0;
		len = 0;
	}
	fp = fopen("/sys/devices/virtual/dmi/id/board_name", "rb");
	if (!fp)
		fp = fopen("/sys/devices/virtual/dmi/id/product_name", "rb");
	if (!fp)
		goto generic;
	if (len)
		name[len++] = 0x20;
	while ((b = getc(fp)) != EOF && len < (sizeof(name) - 1)) {
		if (b == 0xa || b == 0)
			break;
		name[len++] = b;
	}
	fclose(fp);
	name[len] = 0;
	if (!strncasecmp(name, "To be filled", 12) || !strncasecmp(name, "System Product Name", 20) ||
	    !strncasecmp(name, "Default string", 14)) {
#ifdef HAVE_X64
		setRouter("Generic X86_64");
#else
		setRouter("Generic X86");
#endif
	} else
		setRouter(name);
	return ROUTER_BOARD_X86;
generic:;
#ifdef HAVE_X64
	setRouter("Generic X86_64");
#else
	setRouter("Generic X86");
#endif
	return ROUTER_BOARD_X86;
#endif
#elif HAVE_XSCALE
	setRouter("NewMedia Dual A/B/G");
	return ROUTER_BOARD_XSCALE;
#elif HAVE_MAGICBOX
	setRouter("OpenRB PowerPC Board");
	return ROUTER_BOARD_MAGICBOX;
#elif HAVE_UNIWIP
	setRouter("Octagon Systems UNiWiP");
	return ROUTER_BOARD_UNIWIP;
#elif HAVE_WDR4900
	setRouter("TP-Link WDR4900 V1");
	return ROUTER_BOARD_WDR4900;
#elif HAVE_RB1000
	setRouter("Mikrotik RB1000");
	return ROUTER_BOARD_RB600;
#elif HAVE_RB800
	setRouter("Mikrotik RB800");
	return ROUTER_BOARD_RB600;
#elif HAVE_RB600
	setRouter("Mikrotik RB600");
	return ROUTER_BOARD_RB600;
#elif HAVE_GWMF54G2
	setRouter("Planex GW-MF54G2");
	char mac[32];
	getBoardMAC(mac);
	if (!strncmp(mac, "00:19:3B", 8) || !strncmp(mac, "00:02:6F", 8) || !strncmp(mac, "00:15:6D", 8)) {
		fprintf(stderr, "unsupported board\n");
		sys_reboot();
	}
	return ROUTER_BOARD_FONERA;
#elif HAVE_WRT54GV7
	setRouter("Linksys WRT54G v7");
	return ROUTER_BOARD_FONERA;
#elif HAVE_WRK54G
	setRouter("Linksys WRK54G v3");
	return ROUTER_BOARD_FONERA;
#elif HAVE_WGT624
	setRouter("Netgear WGT624 v4");
	return ROUTER_BOARD_FONERA;
#elif HAVE_WPE53G
	setRouter("Compex WPE53G");
	return ROUTER_BOARD_FONERA;
#elif HAVE_NP25G
	setRouter("Compex NP25G");
	return ROUTER_BOARD_FONERA;
#elif HAVE_MR3202A
	setRouter("MR3202A");
	return ROUTER_BOARD_FONERA;
#elif HAVE_DLM101
	setRouter("Doodle Labs DLM-101");
	return ROUTER_BOARD_FONERA;
#elif HAVE_AR430W
	setRouter("Airlink-101 AR430W");
	return ROUTER_BOARD_FONERA;
#elif HAVE_DIR400
	setRouter("D-Link DIR-400");
	return ROUTER_BOARD_FONERA2200;
#elif HAVE_WRT54G2
	setRouter("Linksys WRT54G2 v1.1");
	return ROUTER_BOARD_FONERA;
#elif HAVE_RTG32
	setRouter("Asus RT-G32");
	return ROUTER_BOARD_FONERA;
#elif HAVE_DIR300
	setRouter("D-Link DIR-300");
	return ROUTER_BOARD_FONERA;
#elif HAVE_CNC
	setRouter("WiFi4You Outdoor AP");
	return ROUTER_BOARD_FONERA;
#elif defined(HAVE_CORENET) && defined(HAVE_NS2)
	setRouter("CORENET XNS2");
	return ROUTER_BOARD_LS2;
#elif defined(HAVE_CORENET) && defined(HAVE_LC2)
	setRouter("CORENET XLO2");
	return ROUTER_BOARD_LS2;
#elif defined(HAVE_CORENET) && defined(HAVE_EOC2610)
	setRouter("CORENET XC61");
	return ROUTER_BOARD_FONERA;
#elif defined(HAVE_CORENET) && defined(HAVE_EOC1650)
	setRouter("CORENET XC65");
	return ROUTER_BOARD_FONERA;
#elif defined(HAVE_CORENET) && defined(HAVE_BS2)
	setRouter("CORENET XBU2");
	return ROUTER_BOARD_LS2;
#elif defined(HAVE_CORENET) && defined(HAVE_BS2HP)
	setRouter("CORENET MBU2i");
	return ROUTER_BOARD_LS2;
#elif HAVE_WBD500
	setRouter("Wiligear WBD-500");
	return ROUTER_BOARD_FONERA;
#elif HAVE_EOC1650
	setRouter("Senao EOC-1650");
	return ROUTER_BOARD_FONERA;
#elif HAVE_EOC2611
	setRouter("Senao EOC-2611");
	return ROUTER_BOARD_FONERA;
#elif HAVE_EOC2610
#ifdef HAVE_TRIMAX
	setRouter("M2M-1200");
#else
	setRouter("Senao EOC-2610");
#endif
	return ROUTER_BOARD_FONERA;
#elif HAVE_ECB3500
	setRouter("Senao ECB-3500");
	return ROUTER_BOARD_FONERA;
#elif HAVE_EAP3660
	setRouter("Senao EAP-3660");
	return ROUTER_BOARD_FONERA;
#elif HAVE_MR3201A
	setRouter("Accton MR3201A");
	return ROUTER_BOARD_FONERA;
#elif HAVE_FONERA
	struct mii_ioctl_data *data;
	struct ifreq iwr;
	char mac[32];
	getBoardMAC(mac);
	if (!strncmp(mac, "00:19:3B", 8) || !strncmp(mac, "00:02:6F", 8) || !strncmp(mac, "00:15:6D", 8) ||
	    !strncmp(mac, "00:C0:CA", 8)) {
		fprintf(stderr, "unsupported board\n");
		sys_reboot();
	}
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	if (s < 0) {
		fprintf(stderr, "socket(SOCK_DRAGM)\n");
		setRouter("Fonera 2100/2200");
		return ROUTER_BOARD_FONERA;
	}
	(void)strncpy(iwr.ifr_name, "eth0", sizeof("eth0"));
	data = (struct mii_ioctl_data *)&iwr.ifr_data;
	data->phy_id = 0x10;
	data->reg_num = 0x2;
	ioctl(s, SIOCGMIIREG, &iwr);
	data->phy_id = 0x10;
	data->reg_num = 0x2;
	ioctl(s, SIOCGMIIREG, &iwr);
	if (data->val_out == 0x0141) {
		data->phy_id = 0x10;
		data->reg_num = 0x3;
		ioctl(s, SIOCGMIIREG, &iwr);
		close(s);
		if ((data->val_out & 0xfc00) != 0x0c00) // marvell phy
		{
			setRouter("Fonera 2100/2200");
			return ROUTER_BOARD_FONERA;
		} else {
			setRouter("Fonera 2201");
			return ROUTER_BOARD_FONERA2200;
		}
	} else {
		setRouter("Fonera 2100/2200");
		return ROUTER_BOARD_FONERA;
	}
#elif HAVE_WRT1900AC
	FILE *fp = fopen("/sys/firmware/devicetree/base/model", "rb");
	if (!fp) {
		fprintf(stderr, "error opening device tree\n");
		setRouter("Linksys WRT1900AC");
		return ROUTER_WRT_1900AC;
	}
	char vendorstr[32];
	char modelstr[32];
	fscanf(fp, "%s %s", &vendorstr[0], &modelstr[0]);
	fclose(fp);
	if (!strcmp(modelstr, "WRT1200AC")) {
		setRouter("Linksys WRT1200AC");
		return ROUTER_WRT_1200AC;
	}
	if (!strcmp(modelstr, "WRT1900ACv2")) {
		setRouter("Linksys WRT1900ACv2");
		return ROUTER_WRT_1900ACV2; // similar
	}
	if (!strcmp(modelstr, "WRT1900AC")) {
		setRouter("Linksys WRT1900AC");
		return ROUTER_WRT_1900AC; // similar
	}

	if (!strcmp(modelstr, "WRT1900ACS")) {
		setRouter("Linksys WRT1900ACS");
		return ROUTER_WRT_1900ACS; // similar
	}
	if (!strcmp(modelstr, "WRT3200ACM")) {
		setRouter("Linksys WRT3200ACM");
		return ROUTER_WRT_3200ACM;
	}
	if (!strcmp(modelstr, "WRT32X")) {
		setRouter("Linksys WRT32X");
		return ROUTER_WRT_32X;
	}
	setRouter("Linksys WRTXXXXACM");
	return ROUTER_WRT_3200ACM;
#elif HAVE_R9000
	setRouter("Netgear Nighthawk X10");
	return ROUTER_NETGEAR_R9000;
#elif HAVE_IPQ6018
	FILE *fp = fopen("/sys/firmware/devicetree/base/model", "rb");
	if (!fp) {
		fprintf(stderr, "error opening device tree\n");
		setRouter("Qualcomm IPQ6018");
		return ROUTER_IPQ6018;
	}
	char vendorstr[32];
	char modelstr[32];
	fscanf(fp, "%s %s", &vendorstr[0], &modelstr[0]);
	fclose(fp);
	if (!strcmp(modelstr, "MR7350")) {
		setRouter("Linksys MR7350");
		return ROUTER_LINKSYS_MR7350;
	}

	if (!strcmp(modelstr, "MX4200v1")) {
		setRouter("Linksys MX4200 v1");
		return ROUTER_LINKSYS_MX4200V1;
	}

	if (!strcmp(modelstr, "MX4200v2")) {
		setRouter("Linksys MX4200 v2");
		return ROUTER_LINKSYS_MX4200V2;
	}

	if (!strcmp(modelstr, "MR5500")) {
		setRouter("Linksys MR5500");
		return ROUTER_LINKSYS_MR5500;
	}

	if (!strcmp(modelstr, "DL-WRX36")) {
		setRouter("Dynalink DL-WRX36");
		return ROUTER_DYNALINK_DLWRX36;
	}
	setRouter("Qualcomm IPQ6018");
	return ROUTER_IPQ6018;
#elif HAVE_IPQ806X
	FILE *fp = fopen("/sys/firmware/devicetree/base/model", "rb");
	if (!fp) {
		fprintf(stderr, "error opening device tree\n");
		setRouter("Netgear R7500");
		return ROUTER_NETGEAR_R7500;
	}
	char vendorstr[32];
	char modelstr[32];
	fscanf(fp, "%s %s", &vendorstr[0], &modelstr[0]);
	fclose(fp);
	if (!strcmp(modelstr, "R7800")) {
		setRouter("Netgear R7800");
		return ROUTER_NETGEAR_R7800;
	}

	if (!strncmp(modelstr, "Habanero", 8)) {
		setRouter("8devices Habanero");
		return ROUTER_HABANERO;
	}

	if (!strncmp(modelstr, "RT-AC58U", 8)) {
		setRouter("Asus RT-AC58U");
		return ROUTER_ASUS_AC58U;
	}

	if (!strncmp(modelstr, "EA8300", 6)) {
		setRouter("Linksys EA8300");
		return ROUTER_LINKSYS_EA8300;
	}

	if (!strncmp(modelstr, "MR8300", 6)) {
		setRouter("Linksys MR8300");
		return ROUTER_LINKSYS_EA8300;
	}

	if (!strncmp(modelstr, "MR9000", 6)) {
		setRouter("Linksys MR9000");
		return ROUTER_LINKSYS_EA8300;
	}

	if (!strcmp(modelstr, "XR500")) {
		setRouter("Netgear XR500");
		return ROUTER_NETGEAR_R7800;
	}

	if (!strcmp(modelstr, "R7500v2")) {
		setRouter("Netgear R7500v2");
		return ROUTER_NETGEAR_R7500V2;
	}

	if (!strcmp(modelstr, "R7500")) {
		setRouter("Netgear R7500");
		return ROUTER_NETGEAR_R7500;
	}

	if (!strcmp(modelstr, "EA8500")) {
		setRouter("Linksys EA8500");
		return ROUTER_LINKSYS_EA8500;
	}

	if (!strcmp(modelstr, "TEW-827")) {
		setRouter("Trendnet TEW-827");
		return ROUTER_TRENDNET_TEW827;
	}

	if (!strcmp(modelstr, "G10")) {
		setRouter("ASRock G10");
		return ROUTER_ASROCK_G10;
	}
#elif HAVE_MERAKI
	setRouter("Meraki Mini");
	return ROUTER_BOARD_MERAKI;
#elif HAVE_BWRG1000
	setRouter("Bountiful BWRG-1000");
	return ROUTER_BOARD_LS2;
#elif HAVE_WNR2200
	setRouter("Netgear WNR2200");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WNR2000
	setRouter("Netgear WNR2000v3");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WLAEAG300N
#ifdef HAVE_BUFFALO
	setRouter("WLAE-AG300N");
#else
	setRouter("Buffalo WLAE-AG300N");
#endif
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_CARAMBOLA
#ifdef HAVE_ERC
	setRouter("ERC ServiceGate");
#else
	setRouter("8Devices Carambola 2");
#endif
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_HORNET
	setRouter("Atheros Hornet");
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_RB2011
	setRouter("Mikrotik RB2011");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR2543
	setRouter("TP-Link TL-WR2543");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR941NV6_CN
	setRouter("TP-Link TL-WR941N-cn v6");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR3500
	setRouter("TP-Link TL-WDR3500 v1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR3600
	setRouter("TP-Link TL-WDR3600 v1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR4300
	setRouter("TP-Link TL-WDR4300 v1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR835A1
	setRouter("Dlink DIR835-A1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WNDR4300
	setRouter("Netgear WNDR4300");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WNDR3700V4
	setRouter("Netgear WNDR3700 V4");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_TEW824
	setRouter("Trendnet TEW824DRU");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR866
	setRouter("Dlink DIR866-A1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DAP2680
	setRouter("Dlink DAP2680");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DAP2660
	setRouter("Dlink DAP2660");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DAP2330
	setRouter("Dlink DAP2330");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DAP3662
	setRouter("Dlink DAP3662");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR862
	setRouter("Dlink DIR862-A1");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_CPE880
	setRouter("Yuncore CPE880");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WILLY
	setRouter("Wallystech DR342-NAS");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_MMS344
	setRouter("Compex MMS344");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR4900V2
	setRouter("TP-Link WDR4900 v2");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_ARCHERC5
	setRouter("TP-Link ARCHER-C5 v1");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR1043V5
	setRouter("TP-Link WR1043N v5");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR1043V4
	setRouter("TP-Link WR1043ND v4");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_ARCHERA7V5
	setRouter("TP-Link ARCHER-A7 v5");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_ARCHERC7V5
	setRouter("TP-Link ARCHER-C7 v5");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_ARCHERC7V4
	setRouter("TP-Link ARCHER-C7 v4");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_ARCHERC7
	setRouter("TP-Link ARCHER-C7 v2");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR1043V3
	setRouter("TP-Link WR1043ND V3");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR1043V2
	setRouter("TP-Link WR1043ND V2");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WZR450HP2
#ifdef HAVE_BUFFALO
	setRouter("WZR-450HP2");
#else
	setRouter("Buffalo WZR-450HP2");
#endif
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DHP1565
	setRouter("Dlink DHP1565-A1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_XD9531
	setRouter("Yuncore XD9531");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR615N
	setRouter("Comfast WR615N");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_E325N
	setRouter("Comfast E325N");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_E355AC
	setRouter("Comfast E355AC");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_XD3200
#ifdef HAVE_SR3200
	setRouter("Yuncore SR3200");
#elif HAVE_CPE890
	setRouter("Yuncore CPE890");
#else
	setRouter("Yuncore XD3200");
#endif
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_E380AC
	setRouter("Comfast E380AC");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_AP120C
	setRouter("Alfa AP120C");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WR650AC
	setRouter("Comfast WR650AC");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "7");
	nvram_default_get("wlan1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR869
	setRouter("Dlink DIR869");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR859
	setRouter("Dlink DIR859");
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_JWAP606
	setRouter("JJPlus JWAP606");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR825C1
	setRouter("Dlink DIR825-C1");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_PERU
	setRouter("Antaira Peru");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_LIMA
	setRouter("8devices Lima");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DW02_412H
	setRouter("Dongwong DW02-412H");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_RAMBUTAN
	setRouter("8devices Rambutan");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WASP
	setRouter("Atheros Wasp");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WHRHPG300N
#ifdef HAVE_BUFFALO
#ifdef HAVE_WHR300HP
	setRouter("WHR-300HP");
#else
	setRouter("WHR-HP-G300N");
#endif
#else
	setRouter("Buffalo WHR-HP-G300N");
#endif
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WHRG300NV2
#ifdef HAVE_BUFFALO
	setRouter("WHR-G300N");
#else
	setRouter("Buffalo WHR-G300N");
#endif
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WHRHPGN
#ifdef HAVE_BUFFALO
	setRouter("WHR-HP-GN");
#else
	setRouter("Buffalo WHR-HP-GN");
#endif
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_JJAP93
	setRouter("JJPLUS AP93");
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	return ROUTER_BOARD_PB42;
#elif HAVE_JJAP005
	setRouter("JJPLUS AP005");
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	return ROUTER_BOARD_PB42;
#elif HAVE_JJAP501
	setRouter("JJPLUS AP501");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_PB42;
#elif HAVE_AC722
	setRouter("ACCTON AC722");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_PB42;
#elif HAVE_AC622
	setRouter("ACCTON AC622");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_PB42;
#elif HAVE_WPE72
	setRouter("Compex WPE72");
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	return ROUTER_BOARD_NS5M;
#elif HAVE_DAP3310
	setRouter("DLink DAP3310");
	return ROUTER_BOARD_NS5M;
#elif HAVE_DAP3410
	setRouter("DLink DAP3410");
	return ROUTER_BOARD_NS5M;
#elif HAVE_UBNTM
	typedef struct UBNTDEV {
		int gpiolock; // gpio number for lock
		char *devicename; // device name
		unsigned short devid; // pci subdevice id
		unsigned char rxchain; // rx chainmask
		unsigned char txchain; // tx chainmask
		unsigned char rxchain5; // rx chainmask 5 ghz
		unsigned char txchain5; // tx chainmask 5 ghz
		int dddev; // dd-wrt device id
		int offset; // frequency offset
		int poffset;
	};
	/* these values are guessed and need to be validated */
#define M900 (-(2427 - 907))
#define M365 (-(5540 - 3650))
#define M35 (-(5540 - 3540))
#define M10 (-(5540 - 10322))

#define UNLOCK_UAPV2 16
	struct UBNTDEV dev[] = {
		/* some these AC devices are untested and not yet supported, its just the id definition for detection right now */
		{ -1, "NanoBeam 2AC 13", 0xe4f2, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 4 }, //
		{ -1, "NanoBeam 2AC 13", 0xe4f3, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 4 }, //
		{ -1, "NanoBeam 5AC 16", 0xe9f5, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "NanoBeam 5AC Gen2", 0xe7fc, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "LiteBeam 5AC 23", 0xe8f5, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "LiteBeam 5AC Gen2", 0xe7f9, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "LiteBeam 5AC LR", 0xe7fe, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 2 }, //
		{ -1, "LiteAP AC", 0xe8e5, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 1 }, //
		{ -1, "LiteAP GPS", 0xe7fd, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 2 }, //
		{ -1, "PowerBeam 2AC 400", 0xe3f2, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 4 }, //
		{ -1, "PowerBeam 2AC 400", 0xe3f3, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 4 }, //
		{ -1, "PowerBeam 5AC 300", 0xe6f5, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "PowerBeam 5AC 400", 0xe7f5, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "PowerBeam 5AC Gen2", 0xe3d6, 3, 3, 0, 0, ROUTER_UBNT_POWERBEAMAC_GEN2, 0, 1 }, //
		{ -1, "PowerBeam 5AC X Gen2", 0xe3d9, 3, 3, 0, 0, ROUTER_UBNT_POWERBEAMAC_GEN2, 0, 1 }, //
		{ -1, "ISO Station 5AC", 0xe7f7, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 2 }, //
		{ -1, "ISO Station 5AC", 0xe6f5, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 2 }, //
		{ -1, "NanoStation 5AC loco", 0xe7fa, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 2 }, //
		{ -1, "NanoStation 5AC", 0xe7fb, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 2 }, //
		{ -1, "Bullet AC", 0xe2c5, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "Bullet AC IP67", 0xe2c7, 3, 3, 0, 0, ROUTER_UBNT_NANOAC, 0, 1 }, //
		{ -1, "NanoBeam M2 XW", 0xe2c2, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 10 }, //
		{ -1, "NanoBeam M5 XW", 0xe3e5, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "NanoBeam M2 XW", 0xe812, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 6 }, //
		{ -1, "NanoBeam M5 XW", 0xe815, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "NanoBeam M5 XW", 0xe825, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "NanoBeam M5 XW", 0xe875, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "LiteBeam M5 XW", 0xe865, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 6 }, //
		{ -1, "LiteBeam M5 XW", 0xe895, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "PowerBeam M2 XW", 0xe2c2, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 10 }, //
		{ -1, "PowerBeam M5 XW", 0xe3e5, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 10 }, //
		{ -1, "PowerBeam M5-400 XW", 0xe4e5, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "PowerBeam M5-400 ISO XW", 0xe6e5, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "PowerBeam M5 XW", 0xe5e5, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "PowerBeam M5 XW", 0xe885, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 4 }, //
		{ -1, "ISO Station M5", 0xe7f8, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 2 }, //
		{ -1, "NanoStation M2", 0xe002, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 6 }, //
		{ -1, "NanoStation M2", 0xe012, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 12 }, //
		{ -1, "NanoStation M5", 0xe005, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 5 }, //
		{ -1, "NanoStation M6", 0xe006, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 5 }, //
		{ -1, "NanoStation M5", 0xe805, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 5 }, //
		{ -1, "NanoStation M5 XW", 0xe855, 3, 3, 0, 0, ROUTER_BOARD_NS5MXW, 0, 5 }, //
		{ -1, "NanoStation M2 XW", 0xe866, 3, 3, 0, 0, ROUTER_BOARD_NS5MXW, 0, 6 }, //
		{ -1, "NanoStation M3", 0xe035, 3, 3, 0, 0, ROUTER_BOARD_NS5M, M35, 3 }, //
		{ -1, "NanoStation M365", 0xe003, 3, 3, 0, 0, ROUTER_BOARD_NS5M, M365, 3 }, //
		{ -1, "NanoStation M900", 0xe2b9, 3, 3, 0, 0, ROUTER_BOARD_NS5M, M900, 6 }, //
		//              {-1,"NanoStation M900 GPS", 0xe0b9, 3, 3, ROUTER_BOARD_NS5M, M900},       //
		{ -1, "Rocket M2", 0xe102, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 6 }, //
		{ -1, "Rocket M2", 0xe112, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 10 }, //
		{ -1, "Rocket M2", 0xe1b2, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 10 }, //
		{ -1, "Rocket M2", 0xe1c2, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 10 }, //
		{ -1, "Rocket M2 XW", 0xe868, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 7 }, //
		{ -1, "Rocket M2 Titanium XW", 0xe1d2, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 10 }, // Titanium
		{ -1, "Rocket M5 Titanium XW", 0xe4d5, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 5 }, // Titanium
		{ -1, "Rocket M5", 0xe105, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Rocket M5", 0xe1b5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Rocket M5 XW", 0xe6b5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Rocket M5", 0xe8b5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Rocket M5", 0xe1c5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Rocket M5 Titanium XW", 0xe1d5, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 5 }, // Titanium
		{ -1, "Rocket M6", 0xe1b6, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Rocket M3", 0xe1c3, 3, 3, 0, 0, ROUTER_BOARD_R5M, M35, 3 }, //
		{ -1, "Rocket M3", 0xe1e3, 3, 3, 0, 0, ROUTER_BOARD_R5M, M35, 3 }, //
		{ -1, "Rocket M5 X3 XW", 0xe3b5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 6 }, //
		{ -1, "Rocket M365", 0xe1b3, 3, 3, 0, 0, ROUTER_BOARD_R5M, M365, 3 }, //
		{ -1, "Rocket M365", 0xe1d3, 3, 3, 0, 0, ROUTER_BOARD_R5M, M365, 3 }, //
		{ -1, "Rocket M900", 0xe1b9, 3, 3, 0, 0, ROUTER_BOARD_R2M, M900, 6 }, //
		{ -1, "Rocket M900", 0xe1d9, 3, 3, 0, 0, ROUTER_BOARD_R2M, M900, 6 }, //
		{ -1, "Airbeam 5", 0xe1e5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "Bullet M2", 0xe202, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 12 }, //
		{ -1, "Bullet M2", 0xe2c3, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "Bullet M2", 0xe2c4, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "Bullet M2 XW", 0xe869, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 2 }, //
		{ -1, "Bullet M2", 0xe2d2, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 12 }, // Titanium
		{ -1, "Bullet M2", 0xe2d4, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, // Titanium
		{ -1, "Bullet M5", 0xe205, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "Bullet M5", 0xe2d5, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, // Titanium
		{ -1, "Bullet M2 Titanium", 0xe879, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 2 }, // Titanium
		{ -1, "Airgrid M2", 0xe212, 1, 1, 0, 0, ROUTER_BOARD_BS2M, 0, 1 }, //
		{ -1, "Airgrid M2", 0xe242, 1, 1, 0, 0, ROUTER_BOARD_BS2M, 0, 9 }, //
		{ -1, "Airgrid M2HP", 0xe252, 1, 1, 0, 0, ROUTER_BOARD_BS2M, 0, 9 }, //
		{ -1, "Airgrid M5", 0xe215, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 1 }, //
		{ -1, "Airgrid M5", 0xe245, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "Airgrid M5", 0xe255, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "Airgrid M5 XW", 0xe835, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "AirRouter", 0xe4a2, 1, 1, 0, 0, ROUTER_BOARD_AIRROUTER, 0, 1 }, //
		{ -1, "AirRouter", 0xe4b2, 1, 1, 0, 0, ROUTER_BOARD_AIRROUTER, 0, 9 }, //
		{ -1, "Pico M2", 0xe302, 1, 1, 0, 0, ROUTER_BOARD_BS2M, 0, 12 }, //
		{ -1, "Pico M5", 0xe305, 1, 1, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "Airwire", 0xe405, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 1 }, //
		{ -1, "Airwire", 0xe4a5, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 1 }, //
		{ -1, "Loco M2", 0xe0a2, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 2 }, //
		{ -1, "Loco M2 XW", 0xe867, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 2 }, //
		{ -1, "Loco M5", 0xe0a5, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 1 }, //
		{ -1, "Loco M5", 0xe8a5, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 1 }, //
		{ -1, "Loco M5 XW", 0xe845, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 1 }, //
		{ -1, "Loco M900", 0xe009, 3, 3, 0, 0, ROUTER_BOARD_NS5M, M900, 6 }, //
		{ -1, "NanoStation M900 Sector", 0xe0b9, 3, 3, 0, 0, ROUTER_BOARD_NS5M, M900, 10 }, //
		{ -1, "LiteStation M25", 0xe115, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 3 }, //
		{ -1, "LiteStation M5", 0xe2a5, 3, 3, 0, 0, ROUTER_BOARD_NS5M, 0, 5 }, //
		{ -1, "PowerAP N", 0xe402, 3, 3, 0, 0, ROUTER_BOARD_NS2M, 0, 10 }, //
		{ -1, "Simple AP", 0xe4a2, 3, 3, 0, 0, ROUTER_BOARD_R2M, 0, 10 }, //
		{ -1, "PowerBridge M3", 0xe2a3, 3, 3, 0, 0, ROUTER_BOARD_R5M, M35, 3 }, //
		{ -1, "PowerBridge M5", 0xe1a5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "PowerBridge M10", 0xe110, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 10 }, //
		{ -1, "PowerBridge M5 X3", 0xe3a5, 3, 3, 0, 0, ROUTER_BOARD_R5M, 0, 5 }, //
		{ -1, "PowerBridge M365", 0xe1a3, 3, 3, 0, 0, ROUTER_BOARD_R5M, M365, 3 }, //
		{ -1, "Rocket M10", 0xe110, 3, 3, 0, 0, ROUTER_BOARD_R5M, M10, 10 }, //
		{ -1, "NanoBridge M3", 0xe243, 3, 3, 0, 0, ROUTER_BOARD_BS5M, M35, 3 }, //
		{ -1, "NanoBridge M365", 0xe233, 3, 3, 0, 0, ROUTER_BOARD_BS5M, M365, 3 }, //
		{ -1, "NanoBridge M900", 0xe239, 3, 3, 0, 0, ROUTER_BOARD_BS5M, M900, 6 }, //
		{ -1, "NanoBridge M5", 0xe235, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 1 }, //
		{ -1, "NanoBridge M5", 0xe2b5, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 1 }, //
		{ -1, "NanoBridge M5", 0xe2e5, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 4 }, //
		{ -1, "NanoBridge M2", 0xe232, 3, 3, 0, 0, ROUTER_BOARD_BS2M, 0, 2 }, //
		{ -1, "NanoBridge M2", 0xe2b2, 3, 3, 0, 0, ROUTER_BOARD_BS2M, 0, 10 }, //
		{ -1, "PicoStation M2", 0xe302, 1, 1, 0, 0, ROUTER_BOARD_BS2M, 0, 10 }, //
		{ -1, "PicoStation M5", 0xe305, 1, 1, 0, 0, ROUTER_BOARD_BS2M, 0, 10 }, //
		{ -1, "3G Station", 0xe6a2, 3, 3, 0, 0, ROUTER_BOARD_BS2M, 0, 1 }, //
		{ -1, "3G Station Professional", 0xe6b2, 3, 3, 0, 0, ROUTER_BOARD_BS2M, 0, 1 }, //
		{ -1, "3G Station Outdoor", 0xe6c2, 3, 3, 0, 0, ROUTER_BOARD_BS2M, 0, 6 }, //
		{ -1, "WispStation M5", 0xe345, 3, 3, 0, 0, ROUTER_BOARD_BS5M, 0, 6 }, //
		{ -1, "UniFi UAP", 0xe502, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-PRO", 0xe507, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-LR", 0xe512, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-Mini", 0xe522, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-Outdoor", 0xe532, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-Outdoor 5G", 0xe515, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-AC", 0xe902, 7, 7, 7, 7, ROUTER_BOARD_UNIFI, 0, 0 }, //
		{ -1, "UniFi UAP-Outdoor+", 0xe562, 3, 3, 0, 0, ROUTER_BOARD_UNIFI, 0, 10 }, //
		{ -1, "UniFi UAP-AC v2", 0xe912, 7, 7, 7, 7, ROUTER_BOARD_UNIFI, 0, 0 }, //
		{ UNLOCK_UAPV2, "UniFi UAP v2", 0xe572, 3, 3, 0, 0, ROUTER_BOARD_UNIFI_V2, 0, 10 }, //
		{ UNLOCK_UAPV2, "UniFi UAP-LR v2", 0xe582, 3, 3, 0, 0, ROUTER_BOARD_UNIFI_V2, 0, 10 }, //
		{ -1, "UniFi UAP-AC-Lite", 0xe517, 3, 3, 3, 3, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-LR", 0xe527, 7, 7, 3, 3, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-Pro", 0xe537, 7, 7, 7, 7, ROUTER_UBNT_UAPACPRO, 0, 0 }, //
		{ -1, "UniFi UAP-AC-EDU", 0xe547, 7, 7, 7, 7, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-MESH", 0xe557, 7, 7, 3, 3, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-MESH-Pro", 0xe567, 7, 7, 7, 7, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-InWall", 0xe587, 7, 7, 7, 7, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-InWall-Pro", 0xe592, 3, 3, 0, 0, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-AC-HD", 0xe535, 3, 3, 0, 0, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi MeshXG", 0xe575, 3, 3, 0, 0, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UEMB-AC-AD", 0xe553, 3, 3, 0, 0, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-nanoHD", 0xec20, 3, 3, 0, 0, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, "UniFi UAP-HD-IW", 0xec22, 3, 3, 0, 0, ROUTER_UBNT_UAPAC, 0, 0 }, //
		{ -1, NULL, 0, 0, 0, 0, 0, 0 }, //
	};

#undef M35
#undef M365
#undef M900
#undef M10

#if 0
	FILE *fp = fopen("/sys/bus/pci/devices/0000:00:00.0/subsystem_device", "rb");
	if (fp == NULL)
		return ROUTER_BOARD_PB42;
	int device;
	fscanf(fp, "0x%04X", &device);
	fclose(fp);
#else
	FILE *fp = fopen("/dev/mtdblock5", "rb"); //open board config
	int device = 0;
	if (fp) {
		fseek(fp, 0x1006, SEEK_SET);
		unsigned short cal[132];
		fread(&cal[0], 1, 256, fp);
		int calcnt = 0;
		while (((cal[calcnt] & 0xffff) != 0xffff) && calcnt < 128) {
			unsigned short reg = cal[calcnt++] & 0xffff;
			if (reg == 0x602c || reg == 0x502c) {
				calcnt++;
				device = cal[calcnt++] & 0xffff;
				break;
			} else {
				calcnt += 2;
			}
		}
		if (device == 0) {
			fseek(fp, 12, SEEK_SET);
			unsigned short devid;
			fread(&devid, 2, 1, fp);
			device = devid;
		}

		fclose(fp);
	}
#endif
	int devcnt = 0;
	while (dev[devcnt].devicename != NULL) {
		if (dev[devcnt].devid == device) {
			char rxchain[16];
			char txchain[16];
			sprintf(rxchain, "%d", dev[devcnt].rxchain);
			sprintf(txchain, "%d", dev[devcnt].txchain);
			nvram_default_get("wlan0_rxantenna", rxchain);
			nvram_default_get("wlan0_txantenna", txchain);
			if (dev[devcnt].rxchain5) {
				sprintf(rxchain, "%d", dev[devcnt].rxchain5);
				sprintf(txchain, "%d", dev[devcnt].txchain5);
				nvram_default_get("wlan1_rxantenna", rxchain);
				nvram_default_get("wlan1_txantenna", txchain);
			}
			if (dev[devcnt].offset) {
				char foff[32];
				sprintf(foff, "%d", dev[devcnt].offset);
				nvram_set("wlan0_offset", foff);
			}
			if (dev[devcnt].poffset) {
				char poff[32];
				sprintf(poff, "%d", dev[devcnt].poffset);
				nvram_set("wlan0_poweroffset", poff);
			}
			static char devicename[64];
			sprintf(devicename, "Ubiquiti %s", dev[devcnt].devicename);
			if (dev[devcnt].gpiolock != -1) {
				set_gpio(dev[devcnt].gpiolock, 0);
			}
			setRouter(devicename);
			return dev[devcnt].dddev;
		}
		devcnt++;
	}
	setRouter("Ubiquiti Unknown Model");
	return ROUTER_BOARD_PB42;
#elif HAVE_NS2
	setRouter("Ubiquiti NanoStation 2");
	return ROUTER_BOARD_LS2;
#elif HAVE_EOC5510
	setRouter("Senao EOC-5510");
	return ROUTER_BOARD_LS2;
#elif HAVE_EOC5611
	setRouter("Senao EOC-5611");
	return ROUTER_BOARD_LS2;
#elif HAVE_EOC5610
	setRouter("Senao EOC-5610");
	return ROUTER_BOARD_LS2;
#elif HAVE_NS5
	setRouter("Ubiquiti NanoStation 5");
	return ROUTER_BOARD_LS2;
#elif HAVE_SOLO51
	setRouter("Alfa SoLo48-N");
	return ROUTER_BOARD_LS2;
#elif HAVE_NS3
	setRouter("Ubiquiti NanoStation 3");
	return ROUTER_BOARD_LS2;
#elif HAVE_BS5
	setRouter("Ubiquiti Bullet 5");
	return ROUTER_BOARD_LS2;
#elif HAVE_BS2
	setRouter("Ubiquiti Bullet 2");
	return ROUTER_BOARD_LS2;
#elif HAVE_PICO2
	setRouter("Ubiquiti PicoStation 2");
	return ROUTER_BOARD_LS2;
#elif HAVE_PICO2HP
	setRouter("Ubiquiti PicoStation 2 HP");
	return ROUTER_BOARD_LS2;
#elif HAVE_PICO5
	setRouter("Ubiquiti PicoStation 5");
	return ROUTER_BOARD_LS2;
#elif HAVE_MS2
	setRouter("Ubiquiti MiniStation");
	return ROUTER_BOARD_LS2;
#elif HAVE_BS2HP
	setRouter("Ubiquiti Bullet 2 HP");
	return ROUTER_BOARD_LS2;
#elif HAVE_LC2
	setRouter("Ubiquiti NanoStation 2 Loco");
	return ROUTER_BOARD_LS2;
#elif HAVE_LC5
	setRouter("Ubiquiti NanoStation 5 Loco");
	return ROUTER_BOARD_LS2;
#elif HAVE_PS2
	setRouter("Ubiquiti PowerStation 2");
	return ROUTER_BOARD_LS2;
#elif HAVE_PS5
	setRouter("Ubiquiti PowerStation 5");
	return ROUTER_BOARD_LS2;
#elif HAVE_LS2
	setRouter("Ubiquiti LiteStation 2");
	return ROUTER_BOARD_LS2;
#elif HAVE_LS5
	setRouter("Ubiquiti LiteStation 5");
	return ROUTER_BOARD_LS2;
#elif HAVE_WHRAG108
	setRouter("Buffalo WHR-HP-AG108");
	return ROUTER_BOARD_WHRAG108;
#elif HAVE_PB42
	setRouter("Atheros PB42");
	return ROUTER_BOARD_PB42;
#elif HAVE_RSPRO
	setRouter("Ubiquiti RouterStation Pro");
	return ROUTER_BOARD_PB42;
#elif HAVE_RS
#ifdef HAVE_DDLINK
	setRouter("ddlink1x1");
#else
	setRouter("Ubiquiti RouterStation");
#endif
	return ROUTER_BOARD_PB42;
#elif HAVE_E2100
	setRouter("Linksys E2100L");
	return ROUTER_BOARD_PB42;
#elif HAVE_WRT160NL
	setRouter("Linksys WRT160NL");
	return ROUTER_BOARD_PB42;
#elif HAVE_TG2521
	setRouter("ZCom TG-2521");
	return ROUTER_BOARD_PB42;
#elif HAVE_WZRG300NH2
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
#ifdef HAVE_BUFFALO
#ifdef HAVE_WZR300HP
	setRouter("WZR-300HP");
#else
	setRouter("WZR-HP-G300NH2");
#endif
#else
	setRouter("Buffalo WZR-HP-G300NH2");
#endif
	return ROUTER_BOARD_PB42;
#elif HAVE_WZRG450
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");

	void *getUEnv(char *name);

	char *model = getUEnv("product");

#ifdef HAVE_BUFFALO
	if (!strcmp(model, "BHR-4GRV"))
		setRouter("BHR-4GRV");
	else
		setRouter("WZR-HP-G450H");
#else
	if (!strcmp(model, "BHR-4GRV"))
		setRouter("Buffalo BHR-4GRV");
	else
		setRouter("Buffalo WZR-HP-G450H");
#endif
	return ROUTER_BOARD_PB42;
#elif HAVE_WZRG300NH
#ifdef HAVE_BUFFALO
	setRouter("WZR-HP-G300NH");
#else
	setRouter("Buffalo WZR-HP-G300NH");
#endif
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	return ROUTER_BOARD_PB42;
#elif HAVE_WZRHPAG300NH
#if defined(HAVE_BUFFALO) || defined(HAVE_IDEXX)
#ifdef HAVE_WZR600DHP
	setRouter("WZR-600DHP");
#else
	setRouter("WZR-HP-AG300H");
#endif
#else
	setRouter("Buffalo WZR-HP-AG300H");
#endif
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR632
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("Dlink-DIR-632A");
	return ROUTER_BOARD_PB42;
#elif HAVE_WNDR3700V2
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	setRouter("Netgear WNDR3700 v2/WNDR37AV v2/WNDR3800/WNDR38AV");
	return ROUTER_BOARD_PB42;
#elif HAVE_WNDR3700
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	setRouter("Netgear WNDR3700");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR825
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	setRouter("Dlink DIR-825");
	return ROUTER_BOARD_PB42;
#elif HAVE_TEW673GRU
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	setRouter("Trendnet TEW-673GRU");
	return ROUTER_BOARD_PB42;
#elif HAVE_WRT400
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	nvram_default_get("wlan1_rxantenna", "3");
	nvram_default_get("wlan1_txantenna", "3");
	setRouter("Linksys WRT400N");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615C1
	setRouter("D-Link DIR-615-C1");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR601A1
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("D-Link DIR-601-A1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR842V2
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR842ND v2");
	return ROUTER_BOARD_PB42;
#elif HAVE_DAP3320
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("Dlink DAP-3320");
	return ROUTER_BOARD_PB42;
#elif HAVE_DAP2230
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("Dlink DAP-2230");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA901V4
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	setRouter("TP-Link TL-WA901ND v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA901V5
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	setRouter("TP-Link TL-WA901ND v5");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR940V6
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	setRouter("TP-Link TL-WR940ND v6");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR940V4
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	setRouter("TP-Link TL-WR940ND v4/v5");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR941V6
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	setRouter("TP-Link TL-WR941ND v6");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841HPV3
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841HP v3");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841V12
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v12");
	return ROUTER_BOARD_PB42;
#elif HAVE_ARCHERC25
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	nvram_default_get("wlan1_rxantenna", "1");
	nvram_default_get("wlan1_txantenna", "1");
	setRouter("TP-Link TL-ARCHER C25");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841V11
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v11");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841V10
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v10");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841V9
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v9");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA860RE
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WA860RE v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA850RE
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WA850RE v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA901V3
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WA901ND v3");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR810N
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR810N v1/v2");
	return ROUTER_BOARD_PB42;
#elif HAVE_PERU
	setRouter("Antaira Peru");
	return ROUTER_BOARD_PB42;
#elif HAVE_LIMA
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("8devices Lima");
	return ROUTER_BOARD_PB42;
#elif HAVE_DW02_412H
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("Dongwong DW02-412H");
	return ROUTER_BOARD_PB42;
#elif HAVE_RAMBUTAN
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("8devices Rambutan");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841V8
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v8");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615I
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("D-Link DIR-615-I1");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615E1
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("D-Link DIR-615-E1");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615E
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("D-Link DIR-615-E3/E4");
	return ROUTER_BOARD_PB42;
#elif HAVE_TEW652BRP
	setRouter("Trendnet TEW-652BRP");
	return ROUTER_BOARD_PB42;
#elif HAVE_TEW632BRP
	setRouter("Trendnet TEW-632BRP");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841v3
	setRouter("TP-Link TL-WR841ND v3");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA901
	setRouter("TP-Link TL-WA901ND v2");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR941
	setRouter("TP-Link TL-WR941ND v2/v3");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841v5
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v5");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR840v1
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR840N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_MR3220
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-MR3220");
	return ROUTER_BOARD_PB42;
#elif HAVE_MR3420
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-MR3420");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841v7
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v7");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR842
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WR842ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR740v1
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR740N");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA801v1
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WA801ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA901v1
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	setRouter("TP-Link TL-WA901ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR941v4
	setRouter("TP-Link TL-WR941ND v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR743
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR743ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_MR3020
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-MR3020");
	return ROUTER_BOARD_PB42;
#elif HAVE_GL150
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("GL.iNet-AR150");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR71021
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR710N v2.1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR710V1
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR710N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR710
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR710N v2");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA701V2
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WA701ND v2");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR703
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR703N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR740V4
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR740N v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR743V2
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR743ND v2");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR741V4
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR741ND v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA7510
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WA7510N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR741
	nvram_default_get("wlan0_rxantenna", "1");
	nvram_default_get("wlan0_txantenna", "1");
	setRouter("TP-Link TL-WR741ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR1043
	nvram_default_get("wlan0_rxantenna", "7");
	nvram_default_get("wlan0_txantenna", "7");
	setRouter("TP-Link TL-WR1043ND");
	return ROUTER_BOARD_PB42;
#elif HAVE_AP83
	setRouter("Atheros AP83");
	return ROUTER_BOARD_PB42;
#elif HAVE_WP546
	setRouter("Compex WP546");
	return ROUTER_BOARD_PB42;
#elif HAVE_WP543
	setRouter("Compex WP543");
	return ROUTER_BOARD_PB42;
#elif HAVE_JA76PF
	setRouter("JJPLUS JA76PF");
	return ROUTER_BOARD_PB42;
#elif HAVE_JWAP003
	setRouter("JJPLUS JWAP003");
	return ROUTER_BOARD_PB42;
#elif HAVE_ALFAAP94
	setRouter("Alfa AP94 Board");
	return ROUTER_BOARD_PB42;
#elif HAVE_LSX
	setRouter("Ubiquiti LiteStation-SR71");
	return ROUTER_BOARD_PB42;
#elif HAVE_WMBR_G300NH
	setRouter("Buffalo WBMR-HP-G300H");
	nvram_default_get("wlan0_rxantenna", "3");
	nvram_default_get("wlan0_txantenna", "3");
	return ROUTER_BOARD_DANUBE;
#elif HAVE_VF802
	setRouter("Vodafone Easybox 802");
	return ROUTER_BOARD_DANUBE;
#elif HAVE_VF803
	setRouter("Vodafone Easybox 803");
	return ROUTER_BOARD_DANUBE;
#elif HAVE_SX763
	setRouter("Gigaset SX763");
	return ROUTER_BOARD_DANUBE;
#elif HAVE_DANUBE
	setRouter("Infineon Danube");
	return ROUTER_BOARD_DANUBE;
#elif HAVE_WBD222
	setRouter("Wiligear WBD-222");
	return ROUTER_BOARD_STORM;
#elif HAVE_STORM
	setRouter("Wiligear WBD-111");
	return ROUTER_BOARD_STORM;
#elif HAVE_OPENRISC
	setRouter("Alekto OpenRisc");
	return ROUTER_BOARD_OPENRISC;
#elif HAVE_TW6600
	setRouter("AW-6660");
	return ROUTER_BOARD_TW6600;
#elif HAVE_ALPHA
	setRouter("Alfa Networks AP48");
	return ROUTER_BOARD_CA8;
#elif HAVE_USR5453
	setRouter("US Robotics USR5453");
	return ROUTER_BOARD_CA8;
#elif HAVE_RDAT81
	setRouter("Wistron RDAT-81");
	return ROUTER_BOARD_RDAT81;
#elif HAVE_RCAA01
	setRouter("Airlive WLA-9000AP");
	return ROUTER_BOARD_RCAA01;
#elif HAVE_CA8PRO
	setRouter("Wistron CA8-4 PRO");
	return ROUTER_BOARD_CA8PRO;
#elif HAVE_CA8
#ifdef HAVE_WHA5500CPE
	setRouter("Airlive WHA-5500CPE");
#elif HAVE_AIRMAX5
	setRouter("Airlive AirMax 5");
#else
	setRouter("Airlive WLA-5000AP");
#endif
	return ROUTER_BOARD_CA8;
#else

	unsigned long boardnum = strtoul(nvram_safe_get("boardnum"), NULL, 0);
	unsigned long melco_id = strtoul(nvram_safe_get("melco_id"), NULL, 0);

	if (boardnum == 42 && nvram_match("boardtype", "bcm94710ap")) {
		setRouter("Buffalo WBR-G54 / WLA-G54");
		return ROUTER_BUFFALO_WBR54G;
	}
#ifndef HAVE_BUFFALO
	if (nvram_match("boardnum", "mn700") && nvram_match("boardtype", "bcm94710ap")) {
		setRouter("Microsoft MN-700");
		return ROUTER_MICROSOFT_MN700;
	}

	if (nvram_match("boardnum", "asusX") && nvram_match("boardtype", "bcm94710dev")) {
		setRouter("Asus WL-300g / WL-500g");
		return ROUTER_ASUS_WL500G;
	}

	if (boardnum == 44 && nvram_match("boardtype", "bcm94710ap")) {
		setRouter("Dell TrueMobile 2300");
		return ROUTER_DELL_TRUEMOBILE_2300;
	}
#endif

	if (boardnum == 100 && nvram_match("boardtype", "bcm94710dev")) {
		setRouter("Buffalo WLA-G54C");
		return ROUTER_BUFFALO_WLAG54C;
	}
#ifndef HAVE_BUFFALO
	if (boardnum == 45 && nvram_match("boardtype", "bcm95365r")) {
		setRouter("Asus WL-500g Deluxe");
		return ROUTER_ASUS_WL500GD;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x0472") && nvram_match("boardrev", "0x23") && nvram_matchi("parkid", 1)) {
		setRouter("Asus WL-500W");
		return ROUTER_ASUS_WL500W;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x467")) {
		char *hwver0 = nvram_safe_get("hardware_version");

		if (startswith(hwver0, "WL320G")) {
			setRouter("Asus WL-320gE/gP");
			return ROUTER_ASUS_WL550GE;
		} else {
			setRouter("Asus WL-550gE");
			return ROUTER_ASUS_WL550GE;
		}
	}
#ifdef HAVE_BCMMODERN
	if (boardnum == 45 && nvram_match("boardtype", "0x04EC") && nvram_match("boardrev", "0x1402")) {
		setRouter("Asus RT-N10");
		return ROUTER_ASUS_RTN10;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x0550") && nvram_match("boardrev", "0x1102")) {
		setRouter("Asus RT-N10U");
		return ROUTER_ASUS_RTN10U;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x058e") && nvram_match("boardrev", "0x1153")) {
		setRouter("Asus RT-N10+ rev D1");
		return ROUTER_ASUS_RTN10PLUSD1;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x0550") && nvram_match("boardrev", "0x1442")) {
		setRouter("Asus RT-N53");
		return ROUTER_ASUS_RTN53;
	}

	if (nvram_match("boardtype", "0xF5B2") && nvram_match("boardrev", "0x1100") &&
	    !nvram_matchi("pci/2/1/sb20in80and160hr5ghpo", 0)) {
		setRouter("Asus RT-N66U");
		return ROUTER_ASUS_RTN66;
	}

	if (nvram_matchi("boardnum", 1) && nvram_match("boardtype", "0x054d") && nvram_match("boardrev", "0x1109")) {
		setRouter("NetCore NW715P");
		return ROUTER_NETCORE_NW715P;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x04CD") && nvram_match("boardrev", "0x1201")) {
		setRouter("Asus RT-N12");
		return ROUTER_ASUS_RTN12;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x054D") && nvram_match("boardrev", "0x1101")) {
		char *hwrev = nvram_safe_get("hardware_version");
		if (!strncmp(hwrev, "RTN12D1", 7))
			setRouter("Asus RT-N12D1");
		else if (!strncmp(hwrev, "RTN12C1", 7))
			setRouter("Asus RT-N12C1");
		else
			setRouter("Asus RT-N12B");
		return ROUTER_ASUS_RTN12B;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x04cf") && nvram_match("boardrev", "0x1218")) {
		setRouter("Asus RT-N16");
		return ROUTER_ASUS_RTN16;
	}

	if (nvram_match("boardtype", "0xa4cf") && nvram_match("boardrev", "0x1100")) {
		setRouter("Belkin F5D8235-4 v3");
		return ROUTER_BELKIN_F5D8235V3;
	}

	if (nvram_match("boardtype", "0xd4cf") && nvram_match("boardrev", "0x1204")) {
		setRouter("Belkin F7D4301 / F7D8301 v1");
		return ROUTER_BELKIN_F7D4301;
	}

	if (nvram_match("boardtype", "0xa4cf") && nvram_match("boardrev", "0x1102")) {
		FILE *mtd1 = fopen("/dev/mtdblock/1", "rb");
		unsigned int trxhd;
		if (mtd1) {
			fread(&trxhd, 4, 1, mtd1);
			fclose(mtd1);
			if (trxhd == TRX_MAGIC_F7D3301) {
				setRouter("Belkin F7D3301 / F7D7301 v1");
				return ROUTER_BELKIN_F7D3301;
			}
			if (trxhd == TRX_MAGIC_F7D3302) {
				setRouter("Belkin F7D3302 / F7D7302 v1");
				return ROUTER_BELKIN_F7D3302;
			}
		}
		setRouter("Belkin F7D4302 / F7D8302 v1");
		return ROUTER_BELKIN_F7D4302;
	}
#endif

#endif
	if (nvram_match("boardnum", "00") && nvram_match("boardtype", "0x0101") && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WBR2-G54 / WBR2-G54S");
		return ROUTER_BUFFALO_WBR2G54S;
	}

	if (boardnum == 2 && nvram_match("boardtype", "0x0101") && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WLA2-G54C / WLI3-TX1-G54");
		return ROUTER_BUFFALO_WLA2G54C;
	}
	if (boardnum == 0 && melco_id == 29090 && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WLAH-G54");
		return ROUTER_BUFFALO_WLAH_G54;
	}
	if (boardnum == 0 && melco_id == 31070 && nvram_match("boardflags", "0x2288") && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WAPM-HP-AM54G54");
		return ROUTER_BUFFALO_WAPM_HP_AM54G54;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x11") && nvram_match("boardtype", "0x048e") &&
	    melco_id == 32093) {
		setRouter("Buffalo WHR-G125");
		return ROUTER_BUFFALO_WHRG54S;
	}

	if (nvram_match("boardnum", "0x5347") && nvram_match("boardrev", "0x11") && nvram_match("boardtype", "0x048e")) {
		setRouter("Huawei B970b");
		return ROUTER_HUAWEI_B970B;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x10") && nvram_match("boardtype", "0x048e") &&
	    melco_id == 32139) {
		setRouter("Buffalo WCA-G");
		return ROUTER_BUFFALO_WCAG; //vlan1 is lan, vlan0 is unused, implementation not done. will me made after return to germany
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x11") && nvram_match("boardtype", "0x048e") &&
	    melco_id == 32064) {
		setRouter("Buffalo WHR-HP-G125");
		return ROUTER_BUFFALO_WHRG54S;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x13") && nvram_match("boardtype", "0x467")) {
		if (nvram_match("boardflags", "0x1658") || nvram_match("boardflags", "0x2658") ||
		    nvram_match("boardflags", "0x3658")) {
			setRouter("Buffalo WLI-TX4-G54HP");
			return ROUTER_BUFFALO_WLI_TX4_G54HP;
		}
		if (!nvram_matchi("buffalo_hp", 1) && nvram_match("boardflags", "0x2758")) {
			setRouter("Buffalo WHR-G54S");
			return ROUTER_BUFFALO_WHRG54S;
		}
		if (nvram_matchi("buffalo_hp", 1) || nvram_match("boardflags", "0x1758")) {
#ifndef HAVE_BUFFALO
			setRouter("Buffalo WHR-HP-G54");
#else
#ifdef BUFFALO_JP
			setRouter("Buffalo AS-A100");
#else
			setRouter("Buffalo WHR-HP-G54DD");
#endif
#endif
			return ROUTER_BUFFALO_WHRG54S;
		}
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x10") && nvram_match("boardtype", "0x470")) {
		setRouter("Buffalo WHR-AM54G54");
		return ROUTER_BUFFALO_WHRAM54G54;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x042f")) {
		if (nvram_match("product_name", "WZR-RS-G54") || melco_id == 30083) {
			setRouter("Buffalo WZR-RS-G54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WZR-HP-G54") || melco_id == 30026) {
			setRouter("Buffalo WZR-HP-G54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WZR-G54") || melco_id == 30061) {
			setRouter("Buffalo WZR-G54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("melco_id", "290441dd")) {
			setRouter("Buffalo WHR2-A54G54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WHR3-AG54") || nvram_match("product_name", "WHR3-B11") || melco_id == 29130) {
			setRouter("Buffalo WHR3-AG54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WVR-G54-NF") || melco_id == 28100) {
			setRouter("Buffalo WVR-G54-NF");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WZR-G108") || melco_id == 31095 || melco_id == 30153) {
			setRouter("Buffalo WZR-G108");
			return ROUTER_BRCM4702_GENERIC;
		}
		if (melco_id > 0) // e.g. 29115
		{
			setRouter("Buffalo WZR series");
			return ROUTER_BUFFALO_WZRRSG54;
		}
	}
#ifndef HAVE_BUFFALO
	if (boardnum == 42 && nvram_match("boardtype", "0x042f") && nvram_match("boardrev", "0x10"))
	// nvram_match ("boardflags","0x0018"))
	{
		setRouter("Linksys WRTSL54GS");
		return ROUTER_WRTSL54GS;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x0101") && nvram_match("boardrev", "0x10") &&
	    nvram_match("boot_ver", "v3.6")) {
		setRouter("Linksys WRT54G3G");
		return ROUTER_WRT54G3G;
	}

	if (nvram_match("boardtype", "0x042f") && nvram_match("boardrev", "0x10")) {
		char *hwver = nvram_safe_get("hardware_version");

		if (boardnum == 45 || startswith(hwver, "WL500gp") || startswith(hwver, "WL500gH")) {
			setRouter("Asus WL-500g Premium");
			return ROUTER_ASUS_WL500G_PRE;
		}
		if (boardnum == 44 || startswith(hwver, "WL700g")) {
			setRouter("Asus WL-700gE");
			return ROUTER_ASUS_WL700GE;
		}
	}

	char *et0 = nvram_safe_get("et0macaddr");

	if (boardnum == 100 && nvram_match("boardtype", "bcm94710r4")) {
		if (startswith(et0, "00:11:50")) {
			setRouter("Belkin F5D7130 / F5D7330");
			return ROUTER_RT210W;
		}
		if (startswith(et0, "00:30:BD") || startswith(et0, "00:30:bd")) {
			setRouter("Belkin F5D7230-4 v1000");
			return ROUTER_RT210W;
		}
		if (startswith(et0, "00:01:E3") || startswith(et0, "00:01:e3") || startswith(et0, "00:90:96")) {
			setRouter("Siemens SE505 v1");
			return ROUTER_RT210W;
		} else {
			setRouter("Askey RT210W generic");
			return ROUTER_RT210W;
		}
	}

	if (nvram_match("boardtype", "bcm94710r4") && nvram_match("boardnum", "")) {
		setRouter("Askey board RT2100W-D65)");
		return ROUTER_BRCM4702_GENERIC;
	}

	if (boardnum == 0 && nvram_match("boardtype", "0x0100") && nvram_match("boardrev", "0x10")) {
		if (startswith(et0, "00:11:50") || startswith(et0, "00:30:BD") || startswith(et0, "00:30:bd")) {
			setRouter("Askey board RT2205(6)D-D56");
		} else {
			setRouter("Belkin board F5D8230");
		}
		return ROUTER_ASKEY_RT220XD;
	}

	if (nvram_match("boardtype", "0x0101")) {
		if (startswith(et0, "00:11:50") || startswith(et0, "00:30:BD") || startswith(et0, "00:30:bd")) {
			if (nvram_matchi("Belkin_ver", 2000)) {
				setRouter("Belkin F5D7230-4 v2000");
				return ROUTER_BELKIN_F5D7230_V2000;
			} else {
				setRouter("Belkin F5D7230-4 v1444");
				return ROUTER_RT480W;
			}
		}
		if (startswith(et0, "00:01:E3") || startswith(et0, "00:01:e3") || startswith(et0, "00:90:96")) {
			setRouter("Siemens SE505 v2");
			return ROUTER_RT480W;
		}
	}
	if (boardnum == 1 && nvram_match("boardtype", "0x456") && nvram_matchi("test_led_gpio", 2)) {
		setRouter("Belkin F5D7230-4 v3000");
		return ROUTER_BELKIN_F5D7230_V3000;
	}

	if (nvram_match("boardtype", "0x456") && nvram_match("hw_model", "F5D7231-4")) {
		setRouter("Belkin F5D7231-4 v1212UK");
		return ROUTER_BELKIN_F5D7231;
	}

	if (boardnum == 8 && nvram_match("boardtype", "0x0467")) // fccid:
	// K7SF5D7231B
	{
		setRouter("Belkin F5D7231-4 v2000");
		return ROUTER_BELKIN_F5D7231_V2000;
	}

	if (nvram_match("boardtype", "0x467")) {
		if (startswith(et0, "00:11:50") || startswith(et0, "00:30:BD") || startswith(et0, "00:30:bd")) {
			setRouter("Belkin F5D7231-4 v2000");
			return ROUTER_BELKIN_F5D7231;
		}
	}
#endif
	if (boardnum == 2 && nvram_match("boardtype", "bcm94710dev") && melco_id == 29016) // Buffalo
	// WLI2-TX1-G54)
	{
		setRouter("Buffalo WLI2-TX1-G54");
		return ROUTER_BUFFALO_WLI2_TX1_G54;
	}
#ifndef HAVE_BUFFALO

	char *gemtek = nvram_safe_get("GemtekPmonVer");
	unsigned long gemteknum = strtoul(gemtek, NULL, 0);

	if (boardnum == 2 && (gemteknum == 10 || gemteknum == 11) &&
	    (startswith(et0, "00:0C:E5") || startswith(et0, "00:0c:e5") || startswith(et0, "00:11:22") ||
	     startswith(et0, "00:0C:10") || startswith(et0, "00:0c:10") || startswith(et0, "00:0C:11") ||
	     startswith(et0, "00:0c:11"))) {
		setRouter("Motorola WE800G v1");
		return ROUTER_MOTOROLA_WE800G;
	}

	if (boardnum == 2 && (startswith(gemtek, "RC") || gemteknum == 1 || gemteknum == 10)) {
		setRouter("Linksys WAP54G v1.x");
		return ROUTER_WAP54G_V1;
	}

	if (boardnum == 2 && gemteknum == 1) {
		setRouter("Sitecom WL-105(b)");
		return ROUTER_SITECOM_WL105B;
	}

	if (boardnum == 2 && gemteknum == 7 && nvram_match("boardtype", "bcm94710dev")) {
		setRouter("Sitecom WL-111");
		return ROUTER_SITECOM_WL111;
	}

	if (gemteknum == 9) // Must be Motorola wr850g v1 or we800g v1 or
	// Linksys wrt55ag v1
	{
		if (startswith(et0, "00:0C:E5") || startswith(et0, "00:0c:e5") || startswith(et0, "00:0C:10") ||
		    startswith(et0, "00:0c:10") || startswith(et0, "00:0C:11") || startswith(et0, "00:0c:11") ||
		    startswith(et0, "00:11:22") || startswith(et0, "00:0C:90") || startswith(et0, "00:0c:90")) {
			if (!*(nvram_safe_get("phyid_num"))) {
				insmod("switch-core"); // get phy type
				insmod("switch-robo");
				rmmod("switch-robo");
				rmmod("switch-core");
				nvram_seti("boardnum", 2);
				nvram_set("boardtype", "bcm94710dev");
			}
			if (nvram_match("phyid_num", "0x00000000")) {
				setRouter("Motorola WE800G v1");
				return ROUTER_MOTOROLA_WE800G;
			} else // phyid_num == 0xffffffff
			{
				setRouter("Motorola WR850G v1");
				return ROUTER_MOTOROLA_V1;
			}
		} else {
			setRouter("Linksys WRT55AG v1");
			return ROUTER_LINKSYS_WRT55AG;
		}
	}
#endif
	if (boardnum == 0 && nvram_match("boardtype", "0x478") && nvram_matchi("cardbus", 0) && nvram_match("boardrev", "0x10") &&
	    nvram_match("boardflags", "0x110") && melco_id == 32027) {
		setRouter("Buffalo WZR-G144NH");
		return ROUTER_BUFFALO_WZRG144NH;
	}

	if (boardnum == 20060330 && nvram_match("boardtype", "0x0472")) {
		setRouter("Buffalo WZR-G300N");
		return ROUTER_BUFFALO_WZRG300N;
	}
#ifndef HAVE_BUFFALO

	if (boardnum == 8 && nvram_match("boardtype", "0x0472") && nvram_matchi("cardbus", 1)) {
		setRouter("Netgear WNR834B");
		return ROUTER_NETGEAR_WNR834B;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x0472") && nvram_match("boardrev", "0x23")) {
		if (nvram_matchi("cardbus", 1)) {
			setRouter("Netgear WNR834B v2");
			return ROUTER_NETGEAR_WNR834BV2;
		} else {
			setRouter("Netgear WNDR3300");
			return ROUTER_NETGEAR_WNDR3300;
		}
	}

	if (boardnum == 42) // Get Linksys N models
	{
		if (nvram_match("boot_hw_model", "WRT300N") && nvram_match("boot_hw_ver", "1.1")) {
			setRouter("Linksys WRT300N v1.1");
			return ROUTER_WRT300NV11;
		} else if (nvram_match("boot_hw_model", "WRT150N") && nvram_matchi("boot_hw_ver", 1)) {
			setRouter("Linksys WRT150N v1");
			return ROUTER_WRT150N;
		} else if (nvram_match("boot_hw_model", "WRT150N") && nvram_match("boot_hw_ver", "1.1")) {
			setRouter("Linksys WRT150N v1.1");
			return ROUTER_WRT150N;
		} else if (nvram_match("boot_hw_model", "WRT150N") && nvram_match("boot_hw_ver", "1.2")) {
			setRouter("Linksys WRT150N v1.2");
			return ROUTER_WRT150N;
		} else if (nvram_match("boot_hw_model", "WRT160N") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys WRT160N");
			return ROUTER_WRT160N;
		} else if (nvram_match("boot_hw_model", "WRT160N") && nvram_match("boot_hw_ver", "3.0")) {
			setRouter("Linksys WRT160N v3");
			return ROUTER_WRT160NV3;
		} else if (nvram_match("boot_hw_model", "M10") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Cisco Valet M10 v1"); // renamed wrt160nv3
			return ROUTER_WRT160NV3;
		} else if (nvram_match("boot_hw_model", "E100") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1000 v1"); // renamed wrt160nv3
			return ROUTER_WRT160NV3;
		} else if (nvram_match("boot_hw_model", "E800") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E800");
			return ROUTER_LINKSYS_E800;
		} else if (nvram_match("boot_hw_model", "E900") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E900");
			return ROUTER_LINKSYS_E900;
		} else if (nvram_match("boot_hw_model", "E1000") && nvram_match("boot_hw_ver", "2.0")) {
			setRouter("Linksys E1000 v2");
			return ROUTER_LINKSYS_E1000V2;
		} else if (nvram_match("boot_hw_model", "E1000") && nvram_match("boot_hw_ver", "2.1")) {
			setRouter("Linksys E1000 v2.1");
			return ROUTER_LINKSYS_E1000V2;
		} else if (nvram_match("boot_hw_model", "E1200") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1200 v1");
			return ROUTER_LINKSYS_E1500;
		} else if (nvram_match("boot_hw_model", "E1200") && nvram_match("boot_hw_ver", "2.0")) {
			setRouter("Linksys E1200 v2");
			return ROUTER_LINKSYS_E900;
		} else if (nvram_match("boot_hw_model", "E1500") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1500");
			return ROUTER_LINKSYS_E1500;
		} else if (nvram_match("boot_hw_model", "E1550") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1550");
			return ROUTER_LINKSYS_E1550;
		} else if (nvram_match("boot_hw_model", "WRT310N") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys WRT310N");
			return ROUTER_WRT310N;
		} else if (nvram_match("boot_hw_model", "WRT310N") && nvram_match("boot_hw_ver", "2.0")) {
			setRouter("Linksys WRT310N v2");
			return ROUTER_WRT310NV2;
		} else if (nvram_match("boot_hw_model", "M20") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Cisco Valet Plus M20"); // ranamed wrt310nv2
			return ROUTER_WRT310NV2;
		} else if (nvram_match("boot_hw_model", "E2500") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E2500");
			return ROUTER_LINKSYS_E2500;
		} else if (nvram_match("boot_hw_model", "E3200") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E3200");
			return ROUTER_LINKSYS_E3200;
		} else if (nvram_match("boot_hw_model", "E4200") && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E4200");
			return ROUTER_LINKSYS_E4200;
		}
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x0472") && nvram_matchi("cardbus", 1)) {
		setRouter("Linksys WRT300N v1");
		return ROUTER_WRT300N;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x478") && nvram_matchi("cardbus", 1)) {
		setRouter("Linksys WRT350N");
		return ROUTER_WRT350N;
	}

	if (nvram_matchi("boardnum", 20070615) && nvram_match("boardtype", "0x478") && nvram_matchi("cardbus", 0)) {
		if (nvram_match("switch_type", "BCM5395")) {
			setRouter("Linksys WRT600N v1.1");
			return ROUTER_WRT600N;
		} else {
			setRouter("Linksys WRT600N");
			return ROUTER_WRT600N;
		}
	}

	if (nvram_match("boardtype", "0x478") && nvram_match("boot_hw_model", "WRT610N")) {
		setRouter("Linksys WRT610N");
		return ROUTER_WRT610N;
	}
#ifdef HAVE_BCMMODERN
	if (nvram_match("boardtype", "0x04cf") && nvram_match("boot_hw_model", "WRT610N")) {
		setRouter("Linksys WRT610N v2");
		return ROUTER_WRT610NV2;
	}

	if (nvram_match("boardtype", "0x04cf") && nvram_match("boot_hw_model", "E300")) {
		setRouter("Linksys E3000"); // renamed wrt610nv2
		return ROUTER_WRT610NV2;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x052b") && nvram_match("boardrev", "0x1204")) {
		setRouter("Linksys EA2700"); // renamed wrt610nv2
		return ROUTER_LINKSYS_EA2700;
	}
#endif

	if (boardnum == 42 && nvram_match("boardtype", "bcm94710dev")) {
		setRouter("Linksys WRT54G v1.x");
		return ROUTER_WRT54G1X;
	}

	if ((boardnum == 1 || boardnum == 0) && nvram_match("boardtype", "0x0446")) {
		setRouter("U.S.Robotics USR5430");
		return ROUTER_USR_5430;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x456") && nvram_matchi("test_led_gpio", 0)) {
		setRouter("Netgear WG602 v3");
		return ROUTER_NETGEAR_WG602_V3;
	}

	if (boardnum == 10496 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5461");
		return ROUTER_USR_5461;
	}

	if (boardnum == 10500 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5432");
		return ROUTER_USR_5461; // should work in the same way
	}

	if (boardnum == 10506 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5451");
		return ROUTER_USR_5461; // should work in the same way
	}

	if (boardnum == 10512 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5441");
		return ROUTER_USR_5461; // should work in the same way
	}

	if ((boardnum == 35324 || boardnum == 38256) && nvram_match("boardtype", "0x048e")) {
		setRouter("U.S.Robotics USR5465");
		return ROUTER_USR_5465;
	}

	if (boardnum == 35334 && nvram_match("boardtype", "0x048e")) {
		setRouter("U.S.Robotics USR5455");
		return ROUTER_USR_5465; // should work in the same way
	}

	if (boardnum == 1024 && nvram_match("boardtype", "0x0446")) {
		char *cfe = nvram_safe_get("cfe_version");

		if (strstr(cfe, "WRE54G")) {
			setRouter("Linksys WRE54G v1");
			return ROUTER_WAP54G_V2;
		} else if (strstr(cfe, "iewsonic")) {
			setRouter("Viewsonic WAPBR-100");
			return ROUTER_VIEWSONIC_WAPBR_100;
		} else {
			setRouter("Linksys WAP54G v2");
			return ROUTER_WAP54G_V2;
		}
	}

	if (nvram_invmatch("CFEver", "")) {
		char *cfe = nvram_safe_get("CFEver");

		if (!strncmp(cfe, "MotoWR", 6)) {
			setRouter("Motorola WR850G v2/v3");
			return ROUTER_MOTOROLA;
		}
	}

	if (boardnum == 44 && (nvram_match("boardtype", "0x0101") || nvram_match("boardtype", "0x0101\r"))) {
		char *cfe = nvram_safe_get("CFEver");

		if (!strncmp(cfe, "GW_WR110G", 9)) {
			setRouter("Sparklan WX-6615GT");
			return ROUTER_DELL_TRUEMOBILE_2300_V2;
		} else {
			setRouter("Dell TrueMobile 2300 v2");
			return ROUTER_DELL_TRUEMOBILE_2300_V2;
		}
	}
#endif
	if (nvram_match("boardtype", "bcm94710ap")) {
		setRouter("Buffalo WBR-B11");
		return ROUTER_BUFFALO_WBR54G;
	}

	if (nvram_match("productid", "RT-AC66U")) {
		setRouter("Asus RT-AC66U");
		return ROUTER_ASUS_AC66U;
	}

	if (nvram_match("boardtype", "0xF5B2") && nvram_match("boardrev", "0x1100") &&
	    nvram_matchi("pci/2/1/sb20in80and160hr5ghpo", 0)) {
		setRouter("Asus RT-AC66U");
		return ROUTER_ASUS_AC66U;
	}

	if (nvram_match("boardnum", "${serno}") && nvram_match("boardtype", "0xC617") && nvram_match("boardrev", "0x1103")) {
		setRouter("Linksys EA6500");
		return ROUTER_LINKSYS_EA6500;
	}

	if (nvram_match("boardtype", "0x0617") && nvram_match("boardrev", "0x1103")) {
		setRouter("Ubiquiti UnifiAP AC");
#ifdef HAVE_REGISTER
		return ROUTER_UBNT_UNIFIAC;
#else
		sys_reboot();
		exit(-1);
#endif
	}

	if (boardnum == 24 && nvram_match("boardtype", "0x0617") && nvram_match("boardrev", "0x1102") &&
	    nvram_match("gpio7", "usbport1")) {
		setRouter("Dlink-DIR865L");
		return ROUTER_DLINK_DIR865;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0xf52e") && nvram_match("boardrev", "0x1204")) {
		if (nvram_match("product", "WLI-H4-D1300")) {
#ifdef HAVE_BUFFALO
			setRouter("WLI-H4-D1300");
#else
			setRouter("Buffalo WLI-H4-D1300");
#endif
		} else {
#ifdef HAVE_BUFFALO
			setRouter("WZR-D1800H");
#else
			setRouter("Buffalo WZR-D1800H");
#endif
		}
		return ROUTER_D1800H;
	}
#ifndef HAVE_BUFFALO
	if (boardnum == 0 &&
	    nvram_match("boardtype",
			"0x048e") && // cfe sets boardnum="", strtoul -> 0
	    nvram_match("boardrev", "0x35")) {
		setRouter("D-Link DIR-320");
		// apply some fixes
		if (!nvram_exists("vlan2ports")) {
			nvram_unset("vlan2ports");
			nvram_unset("vlan2hwname");
		}
		return ROUTER_DLINK_DIR320;
	}
	if (nvram_match("model_name", "DIR-330") && nvram_match("boardrev", "0x10")) {
		setRouter("D-Link DIR-330");
		nvram_set("wan_ifnames", "eth0"); // quirk
		nvram_set("wan_ifname", "eth0");
		if (nvram_match("et0macaddr", "00:90:4c:4e:00:0c")) {
			FILE *in = fopen("/dev/mtdblock/1", "rb");

			fseek(in, 0x7a0022, SEEK_SET);
			char mac[32];

			fread(mac, 32, 1, in);
			fclose(in);
			mac[17] = 0;
			if (sv_valid_hwaddr(mac)) {
				nvram_set("et0macaddr", mac);
				fprintf(stderr, "restore D-Link MAC\n");
				nvram_commit();
				sys_reboot();
			}
		}
		/*
		 * if (nvram_get("vlan2ports")!=NULL) { nvram_unset("vlan2ports");
		 * nvram_unset("vlan2hwname"); }
		 */
		return ROUTER_DLINK_DIR330;
	}
	if (boardnum == 42 && nvram_match("boardtype", "0x048e") && nvram_match("boardrev", "0x10")) {
		if (nvram_match("boardflags", "0x20750")) {
			setRouter("Linksys WRT54G2 / GS2"); // router is wrt54g2v1/v1.3/gs2v1
		} else {
			setRouter("Linksys WRT54Gv8 / GSv7");
		}
		return ROUTER_WRT54G_V8;
	}

	if (boardnum == 8 && nvram_match("boardtype", "0x048e") && nvram_match("boardrev", "0x11")) {
		setRouter("ALLNET EUROWRT 54"); //ALLNET01
		return ROUTER_ALLNET01;
	}

	if (boardnum == 01 && nvram_match("boardtype", "0x048e") && nvram_match("boardrev", "0x11") &&
	    (nvram_match("boardflags", "0x650") || nvram_match("boardflags", "0x0458"))) {
		setRouter("Netgear WG602 v4");
		return ROUTER_NETGEAR_WG602_V4;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x048e") && nvram_match("boardrev", "0x35") &&
	    nvram_match("parefldovoltage", "0x28")) {
		setRouter("NetCore NW618 / Rosewill RNX-GX4");
		return ROUTER_NETCORE_NW618;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x048E") && nvram_match("boardrev", "0x10")) {
		setRouter("Linksys WRH54G");
		return ROUTER_LINKSYS_WRH54G;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardtype", "0x048E") && nvram_match("boardrev", "0x10")) {
		setRouter("Linksys WRT54G v8.1");
		return ROUTER_WRT54G_V81;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x456")) {
		setRouter("Asus WL-520G");
		return ROUTER_ASUS_WL520G;
	}

	if (nvram_match("boardtype", "0x48E") && nvram_match("boardrev", "0x10")) {
		char *hwver = nvram_safe_get("hardware_version");

		if (boardnum == 45 && startswith(hwver, "WL500GPV2")) {
			setRouter("Asus WL-500G Premium v2");
			return ROUTER_ASUS_WL500G_PRE_V2;
		} else if (boardnum == 45 && startswith(hwver, "WL330GE")) {
			setRouter("Asus WL-330GE");
			return ROUTER_ASUS_330GE;
		} else if (boardnum == 45 || startswith(hwver, "WL500GU") || startswith(hwver, "WL500GC")) {
			setRouter("Asus WL-520GU/GC");
			return ROUTER_ASUS_WL520GUGC;
		}
	}

	if ((boardnum == 83258 || boardnum == 1 || boardnum == 0123) //or 01 or 001 or 0x01
	    && (nvram_match("boardtype", "0x048e") || nvram_match("boardtype", "0x48E")) &&
	    (nvram_match("boardrev", "0x11") || nvram_match("boardrev", "0x10")) &&
	    (nvram_match("boardflags", "0x750") || nvram_match("boardflags", "0x0750")) &&
	    nvram_match("sdram_init", "0x000A")) //16 MB ram
	{
		setRouter("Netgear WGR614v8/L/WW");
		return ROUTER_NETGEAR_WGR614L;
	}

	if (boardnum == 3805 && nvram_match("boardtype", "0x48E") && nvram_match("boardrev", "0x10")) {
		setRouter("Netgear WGR614v9");
		return ROUTER_NETGEAR_WGR614V9;
	}

	if (boardnum == 56 && nvram_match("boardtype", "0x456") && nvram_match("boardrev", "0x10")) {
		setRouter("Linksys WTR54GS");
		return ROUTER_LINKSYS_WTR54GS;
	}

	if (nvram_match("boardnum", "WAP54GV3_8M_0614") &&
	    (nvram_match("boardtype", "0x0467") || nvram_match("boardtype", "0x467")) && nvram_matchi("WAPver", 3)) {
		setRouter("Linksys WAP54G v3.x");
		return ROUTER_WAP54G_V3;
	}
#ifdef HAVE_BCMMODERN
	if (boardnum == 1 && nvram_match("boardtype", "0xE4CD") && nvram_match("boardrev", "0x1700")) {
		setRouter("Netgear WNR2000 v2");
		return ROUTER_NETGEAR_WNR2000V2;
	}

	if ((boardnum == 1 || boardnum == 3500) && nvram_match("boardtype", "0x04CF") &&
	    (nvram_match("boardrev", "0x1213") || nvram_matchi("boardrev", 02))) {
		setRouter("Netgear WNR3500 v2/U/L v1");
		return ROUTER_NETGEAR_WNR3500L;
	}

	if (nvram_match("boardnum", "3500L") && nvram_match("boardtype", "0x052b")) {
		setRouter("Netgear WNR3500L v2");
		return ROUTER_NETGEAR_WNR3500LV2;
	}

	if (nvram_match("boardnum", "01") && nvram_match("boardtype", "0xb4cf") && nvram_match("boardrev", "0x1100")) {
		setRouter("Netgear WNDR3400");
		return ROUTER_NETGEAR_WNDR3400;
	}

	if (nvram_match("boardnum", "01") && nvram_match("boardtype", "0xF52C") && nvram_match("boardrev", "0x1101")) {
		int mtd = getMTD("board_data");
		char devname[32];
		sprintf(devname, "/dev/mtdblock/%d", mtd);
		FILE *model = fopen(devname, "rb");
		if (model) {
#define WNDR3700V3 "U12H194T00_NETGEAR"
#define R6200 "U12H192T00_NETGEAR"
			char modelstr[32];
			fread(modelstr, 1, strlen(WNDR3700V3), model);
			fclose(model);
			if (!strncmp(modelstr, WNDR3700V3, strlen(WNDR3700V3))) {
				setRouter("Netgear WNDR3700v3");
				return ROUTER_NETGEAR_WNDR4000;
			}
			if (!strncmp(modelstr, R6200, strlen(R6200))) {
				setRouter("Netgear R6200");
				return ROUTER_NETGEAR_R6200;
			}
		}
		setRouter("Netgear WNDR4000");
		return ROUTER_NETGEAR_WNDR4000;
	}

	if (nvram_matchi("boardnum", 4536) && nvram_match("boardtype", "0xf52e") && nvram_match("boardrev", "0x1102")) {
		int mtd = getMTD("board_data");
		char devname[32];
		sprintf(devname, "/dev/mtdblock/%d", mtd);
		FILE *model = fopen(devname, "rb");
		if (model) {
#define R6300 "U12H218T00_NETGEAR"
#define WNDR4500V2 "U12H224T00_NETGEAR"
			char modelstr[32];
			fread(modelstr, 1, strlen(R6300), model);
			if (!strncmp(modelstr, R6300, strlen(R6300))) {
				fclose(model);
				setRouter("Netgear R6300");
				return ROUTER_NETGEAR_R6300;
			}
			fread(modelstr, 1, strlen(R6300), model);
			if (!strncmp(modelstr, WNDR4500V2, strlen(WNDR4500V2))) {
				fclose(model);
				setRouter("Netgear WNDR4500V2");
				return ROUTER_NETGEAR_WNDR4500V2;
			}
			fclose(model);
		}
		setRouter("Netgear WNDR4500");
		return ROUTER_NETGEAR_WNDR4500;
	}

	if (nvram_matchi("boardnum", 679) && nvram_match("boardtype", "0x0646") && nvram_match("boardrev", "0x1110")) {
		setRouter("Netgear R6250");
		return ROUTER_NETGEAR_R6250;
	}

	if ((boardnum == 42 || boardnum == 66) && nvram_match("boardtype", "0x04EF") &&
	    (nvram_match("boardrev", "0x1304") || nvram_match("boardrev", "0x1305"))) {
		setRouter("Linksys WRT320N");
		return ROUTER_WRT320N;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x04EF") && nvram_match("boardrev", "0x1307")) {
		setRouter("Linksys E2000"); // renamed (and fixed reset button) wrt320n
		return ROUTER_WRT320N;
	}
#endif

	if (boardnum == 94703 && nvram_match("boardtype", "0x04c0") && nvram_match("boardrev", "0x1100")) {
		setRouter("Dynex DX-NRUTER");
		return ROUTER_DYNEX_DX_NRUTER;
	}

	setRouter("Linksys WRT54G/GL/GS");
	return ROUTER_WRT54G;
#else
	eval("event", "3", "1", "15");
	return 0;
#endif
#endif
#endif
}

static int router_type = -1;
int getRouterBrand()
{
	if (router_type == -1)
		router_type = internal_getRouterBrand();
	return router_type;
}
