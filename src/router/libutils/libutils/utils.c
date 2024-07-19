/*
 * utils.c
 *
 * Copyright (C) 2007 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <linux/if_vlan.h>
#include <cy_conf.h>
#include <bcmdevs.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
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

void setWifiPass(void)
{
	//In case of a reset use burned in wifi pass
	int mtd, type;
	int offset = 0;
	;
	char mtdpath[64] = { 0 };
	char cmd[64] = { 0 };
	char line[256] = { 0 };
	char var[32] = { 0 };
	char pass[32] = { 0 };
	FILE *fp;

	int brand = getRouterBrand();

	switch (brand) {
	case ROUTER_NETGEAR_R7500V2:
		mtd = getMTD("art");
		offset = 0x0073;
		break;
	case ROUTER_NETGEAR_R7800:
		mtd = getMTD("art");
		offset = 0x007B;
		break;
	case ROUTER_NETGEAR_R9000:
		mtd = getMTD("ART");
		offset = 0x007B;
		break;
	case ROUTER_NETGEAR_AC1450:
	case ROUTER_NETGEAR_EX6200:
	case ROUTER_NETGEAR_R6250:
	case ROUTER_NETGEAR_R6300:
	case ROUTER_NETGEAR_R6300V2:
	case ROUTER_NETGEAR_R6400:
	case ROUTER_NETGEAR_R6400V2:
	case ROUTER_NETGEAR_R6700V3:
	case ROUTER_NETGEAR_R7000:
	case ROUTER_NETGEAR_R7000P:
	case ROUTER_NETGEAR_R8000:
	case ROUTER_NETGEAR_R8500:
		mtd = getMTD("board_data");
		offset = 0x801A;
		break;
	case ROUTER_LINKSYS_EA8500:
		mtd = getMTD("devinfo");
		strcpy(var, "default_passphrase");
		break;
	default:
		return;
	}
	if (mtd == -1)
		return;
	sprintf(mtdpath, "/dev/mtdblock/%d", mtd);

	if (offset != 0) {
		fp = fopen(mtdpath, "rb");

		if (fp) {
			fseek(fp, offset, SEEK_SET);
			fread(pass, 16, 1, fp);
			fclose(fp);
		}
	} else {
		sprintf(cmd, "strings /dev/mtdblock/%d | grep %s", mtd, var);
		fp = popen(cmd, "r");

		if (fp != NULL) {
			while (fgets(line, sizeof(line) - 1, fp) != NULL) {
				if (strstr(line, var)) {
					char *tok, *val;
					tok = strtok(line, "=");
					val = strtok(NULL, "=");
					strcpy(pass, val);
				}
			}
			pclose(fp);
		}
	}

	if (strlen(pass) > 1) {
		fprintf(stderr, "Restoring Factory WIFI Pass: %s\n", pass);
#ifdef HAVE_BCMMODERN
		nvram_set("wl0_security_mode", "psk2");
		nvram_set("wl0_crypto", "aes");
		nvram_set("wl0_wpa_psk", pass);
		nvram_set("wl0_akm", "psk2");
		nvram_set("wl1_security_mode", "psk2");
		nvram_set("wl1_crypto", "aes");
		nvram_set("wl1_wpa_psk", pass);
		nvram_set("wl1_akm", "psk2");
#ifdef HAVE_DHDAP
		nvram_set("wl2_security_mode", "psk2");
		nvram_set("wl2_crypto", "aes");
		nvram_set("wl2_wpa_psk", pass);
		nvram_set("wl2_akm", "psk2");
#endif
#else
		nvram_set("wlan0_security_mode", "psk2");
		nvram_set("wlan0_ccmp", "1");
		nvram_set("wlan0_wpa_psk", pass);
		nvram_set("wlan0_akm", "psk2");
		nvram_set("wlan1_security_mode", "psk2");
		nvram_set("wlan1_ccmp", "1");
		nvram_set("wlan1_wpa_psk", pass);
		nvram_set("wlan1_akm", "psk2");
#endif
	}
}

int count_processes(char *pidName)
{
	FILE *fp;
	char line[255];
	char safename[64];

	sprintf(safename, " %s ", pidName);
	char zombie[64];

	sprintf(zombie, "Z   [%s]", pidName); // do not count zombies
	int i = 0;

	cprintf("Search for %s\n", pidName);
	if ((fp = popen("ps", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			int len = strlen(line);
			if (len > sizeof(line) - 1)
				len = sizeof(line) - 1;
			line[len - 1] = ' ';
			line[len] = 0;
			if (strstr(line, safename) && !strstr(line, zombie)) {
				i++;
			}
		}
		pclose(fp);
	}
	cprintf("Search done... %d\n", i);

	return i;
}

/*
 * This function returns the number of days for the given month in the given
 * year 
 */
unsigned int daysformonth(unsigned int month, unsigned int year)
{
	return (30 + (((month & 9) == 8) || ((month & 9) == 1)) - (month == 2) -
		(!(((year % 4) == 0) && (((year % 100) != 0) || ((year % 400) == 0))) && (month == 2)));
}

#ifdef HAVE_IPV6
const char *getifaddr(char *buf, char *ifname, int family, int linklocal)
{
	void *addr = NULL;
	struct ifaddrs *ifap, *ifa;
	if (!ifname || !*ifname)
		return NULL;
	if (!buf)
		return NULL;

	if (getifaddrs(&ifap) != 0) {
		dprintf("getifaddrs failed: %s\n", strerror(errno));
		return NULL;
	}

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if ((ifa->ifa_addr == NULL) || (strncmp(ifa->ifa_name, ifname, IFNAMSIZ) != 0) ||
		    (ifa->ifa_addr->sa_family != family))
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET6) {
			struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)(ifa->ifa_addr);
			if (IN6_IS_ADDR_LINKLOCAL(&s6->sin6_addr) ^ linklocal)
				continue;
			addr = (void *)&(s6->sin6_addr);
		} else {
			struct sockaddr_in *s = (struct sockaddr_in *)(ifa->ifa_addr);
			addr = (void *)&(s->sin_addr);
		}

		if ((addr) && inet_ntop(ifa->ifa_addr->sa_family, addr, buf, INET6_ADDRSTRLEN) != NULL) {
			freeifaddrs(ifap);
			return buf;
		}
	}

	freeifaddrs(ifap);
	return NULL;
}

const char *getifaddr_any(char *buf, char *ifname, int family)
{
	char *ip = getifaddr(buf, ifname, family, 0);
	if (!ip)
		ip = getifaddr(buf, ifname, family, GIF_LINKLOCAL);
	return ip;
}
#endif
#ifdef HAVE_VLANTAGGING
char *getBridge(char *ifname, char *word)
{
	char *next, *wordlist;
	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		GETENTRYBYIDX(port, word, 1);
		if (!bridge)
			break;
		if (!strcmp(port, ifname)) {
			strcpy(word, bridge);
			return word;
		}
	}
	return nvram_safe_get("lan_ifname");
}
#else
char *getBridge(char *ifname, char *word)
{
	return nvram_safe_get("lan_ifname");
}
#endif

char *getBridgeMTU(const char *ifname, char *word)
{
	char *next, *wordlist;

	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		GETENTRYBYIDX(mtu, word, 3);
		if (!bridge)
			break;
		if (!strcmp(bridge, ifname)) {
			if (!mtu)
				return "1500";
			else {
				strcpy(word, mtu);
				return word;
			}
		}
	}
	return "1500";
}

char *getMTU(char *ifname)
{
	if (!ifname)
		return "1500";
	char *mtu = nvram_nget("%s_mtu", ifname);
	if (!mtu || *mtu == 0)
		return "1500";
	return mtu;
}

char *getTXQ(char *ifname)
{
	if (!ifname)
		return "1000";
	char *txq = nvram_nget("%s_txq", ifname);
	if (!txq || *txq == 0) {
		int s;
		struct ifreq ifr;
		bzero(&ifr, sizeof(ifr));
		if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
			return "0";
		strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
		ioctl(s, SIOCGIFTXQLEN, &ifr);
		close(s);
		static char rtxq[32];
		sprintf(rtxq, "%d", ifr.ifr_qlen);
		// get default len from interface
		return rtxq;
	}
	return txq;
}

int check_switch_support(void)
{
#ifdef HAVE_SWCONFIG
	if (nvram_exists("sw_cpuport"))
		return 1;
	if (nvram_exists("sw_wancpuport"))
		return 1;
#ifdef HAVE_ALPINE
	return 1;
#endif
#endif

#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || \
	defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || \
	defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) ||        \
	defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else

	int brand = getRouterBrand();

	switch (brand) {
#ifndef HAVE_BUFFALO
	case ROUTER_ASUS_WL500GD:
	case ROUTER_WRT54G1X:
		return 1;
		break;
#endif
	case ROUTER_BUFFALO_WLAG54C:
	case ROUTER_BUFFALO_WLA2G54C:
#ifndef HAVE_BUFFALO
	case ROUTER_LINKSYS_WRT55AG:
	case ROUTER_MOTOROLA_V1:
	case ROUTER_MOTOROLA_WE800G:
	case ROUTER_WAP54G_V1:
	case ROUTER_SITECOM_WL105B:
	case ROUTER_SITECOM_WL111:
	case ROUTER_BUFFALO_WLI2_TX1_G54:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
	case ROUTER_BRCM4702_GENERIC:
	case ROUTER_ASUS_WL500G:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_ASKEY_RT220XD:
#endif
		return 0;
		break;
	}

	unsigned long boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
	if (boardflags & BFL_ENETVLAN)
		return 1;
	if (nvram_match("boardtype", "bcm94710dev"))
		return 1;
	if (nvram_match("boardtype", "0x0101"))
		return 1;
	if (boardflags & 0x0100)
		return 1;

	return 0;
#endif
}

int get_ppp_pid(char *file)
{
	char buf[80];
	int pid = -1;

	if (file_to_buf(file, buf, sizeof(buf))) {
		char tmp[80], tmp1[80];
		char *idx = strchr(buf, '\n');
		if (idx)
			*idx = 0;
		idx = strchr(buf, '\r');
		if (idx)
			*idx = 0;
		snprintf(tmp, sizeof(tmp), "/var/run/%s.pid", buf);
		file_to_buf(tmp, tmp1, sizeof(tmp1));
		pid = atoi(tmp1);
	}
	return pid;
}

int check_wan_link(int num)
{
	int wan_link = 0;
	int ppp3g = 1;

#ifdef HAVE_3G
	FILE *fp = NULL;
	int value = 0;
	if (nvram_match("wan_proto", "3g")) {
		if (nvram_match("3gdata", "hso") || nvram_match("3gdata", "qmi") || nvram_match("3gdata", "mbim") ||
		    nvram_match("3gdata", "sierradirectip")) {
			ppp3g = 0;
			if (nvram_match("3gdata", "sierradirectip")) {
				fp = fopen("/tmp/sierradipstatus", "rb");
			}
#ifdef HAVE_UQMI
			if (nvram_match("3gdata", "qmi")) {
				fp = fopen("/tmp/qmistatus", "rb");
			}
#endif
#ifdef HAVE_LIBMBIM
			if (nvram_match("3gdata", "mbim")) {
				fp = fopen("/tmp/mbimstatus", "rb");
			}
#endif
			// Where is hso? was never handled
			if (fp) {
				fscanf(fp, "%d", &value);
				fclose(fp);
			}
			if (value) {
				wan_link = 1;
			} else {
				wan_link = 0;
#if defined(HAVE_TMK) || defined(HAVE_BKM)
				char *gpio3g;
				gpio3g = nvram_safe_get("gpio3g");
				if (*gpio3g)
					set_gpio(atoi(gpio3g), 0);
				gpio3g = nvram_safe_get("gpiowancable");
				if (*gpio3g)
					set_gpio(atoi(gpio3g), 0);
#endif
			}
			return wan_link;
		}
	}
#endif

	if ((nvram_match("wan_proto", "pptp")
#ifdef HAVE_L2TP
	     || nvram_match("wan_proto", "l2tp")
#endif
#ifdef HAVE_PPPOE
	     || nvram_match("wan_proto", "pppoe")
#endif
#ifdef HAVE_PPPOEDUAL
	     || nvram_match("wan_proto", "pppoe_dual")
#endif
#ifdef HAVE_PPPOA
	     || nvram_match("wan_proto", "pppoa")
#endif
#ifdef HAVE_3G
	     || nvram_match("wan_proto", "3g")
#endif
	     || nvram_match("wan_proto", "heartbeat"))) {
		FILE *fp;
		char filename[80];
		char *name;

		if (num == 0)
			strcpy(filename, "/tmp/ppp/link");
		if (!num && (fp = fopen(filename, "r"))) {
			int pid = -1;

			fclose(fp);
			if (nvram_match("wan_proto", "heartbeat")) {
				char buf[20];

				file_to_buf("/tmp/ppp/link", buf, sizeof(buf));
				pid = atoi(buf);
			} else
				pid = get_ppp_pid(filename);

			name = find_name_by_proc(pid);
			if (!strncmp(name, "pppoecd", 7) || // for PPPoE
			    !strncmp(name, "pppd", 4) || // for PPTP
			    !strncmp(name, "bpalogin", 8)) // for HeartBeat
				wan_link = 1; // connect
			else {
				printf("The %s had been died, remove %s\n", nvram_safe_get("wan_proto"), filename);
				wan_link = 0; // For some reason, the pppoed had been died,
				// by link file still exist.
				unlink(filename);
			}
		} else {
			wan_link = 0;
			return wan_link;
		}
	}
#ifdef HAVE_IPETH
	if (nvram_match("wan_proto", "iphone")) {
		FILE *fp;
		if ((fp = fopen("/proc/net/dev", "r"))) {
			char line[256];
			while (fgets(line, sizeof(line), fp) != NULL) {
				if (strstr(line, "iph0")) {
					int sock;
					struct ifreq ifr;
					if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
						break;
					bzero(&ifr, sizeof(struct ifreq));
					strncpy(ifr.ifr_name, "iph0", IFNAMSIZ);
					ioctl(sock, SIOCGIFFLAGS, &ifr);
					if ((ifr.ifr_flags & (IFF_RUNNING | IFF_UP)) == (IFF_RUNNING | IFF_UP))
						wan_link = 1;
					close(sock);
					break;
				}
			}
			fclose(fp);
			if (nvram_match("wan_ipaddr", "0.0.0.0") || nvram_match("wan_ipaddr", ""))
				wan_link = 0;
		}
	} else if (nvram_match("wan_proto", "android")) {
		FILE *fp;
		if ((fp = fopen("/proc/net/dev", "r"))) {
			char line[256];
			while (fgets(line, sizeof(line), fp) != NULL) {
				if (strstr(line, "usb0")) {
					int sock;
					struct ifreq ifr;
					if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
						break;
					bzero(&ifr, sizeof(struct ifreq));
					strncpy(ifr.ifr_name, "usb0", IFNAMSIZ);
					ioctl(sock, SIOCGIFFLAGS, &ifr);
					if ((ifr.ifr_flags & (IFF_RUNNING | IFF_UP)) == (IFF_RUNNING | IFF_UP))
						wan_link = 1;
					close(sock);
					break;
				}
				if (strstr(line, "wwan0")) {
					int sock;
					struct ifreq ifr;
					if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
						break;
					bzero(&ifr, sizeof(struct ifreq));
					strncpy(ifr.ifr_name, "wwan0", IFNAMSIZ);
					ioctl(sock, SIOCGIFFLAGS, &ifr);
					if ((ifr.ifr_flags & (IFF_RUNNING | IFF_UP)) == (IFF_RUNNING | IFF_UP))
						wan_link = 1;
					close(sock);
					break;
				}
			}
			fclose(fp);
			if (nvram_match("wan_ipaddr", "0.0.0.0") || nvram_match("wan_ipaddr", ""))
				wan_link = 0;
		}
	} else
#endif
	{
		if (nvram_invmatch("wan_ipaddr", "0.0.0.0"))
			wan_link = 1;
	}

	return wan_link;
}

int wanactive(char *wanaddr)
{
	return (!nvram_match("wan_proto", "disabled") && strcmp(wanaddr, "0.0.0.0") && check_wan_link(0));
}

#ifdef HAVE_WZR450HP2
struct ENV_EXTDATA {
	unsigned char wanmac[6]; // 0x0
	unsigned char lanmac[6]; // 0x6
	unsigned char wpspin[8]; // 0xc
	unsigned char passphrase[25]; // 0x14, length is max <=24 followed by 0 termination
	unsigned char authmode[3];
	unsigned char crypto[3];
	unsigned char authmode_ex[4];
	unsigned char region[2];
	unsigned char productid[16];
	unsigned char bootversion[8];
	unsigned char hwversion;
	unsigned char customid;
	unsigned char melcoid[11];
	unsigned char builddate[28];
	unsigned char inspection;
};

static char *getUEnvExt(char *name)
{
	struct ENV_EXTDATA data;
	FILE *fp = fopen("/dev/mtdblock5", "rb"); // board data
	if (!fp)
		return NULL;

	fread(&data, 1, sizeof(data), fp);
	fclose(fp);
	if (!strcmp(name, "DEF-p_wireless_ath0_11bg-authmode_ex")) {
		if (!memcmp(data.authmode_ex, "WPA2", 4))
			return "wpa2-psk";
		if (!memcmp(data.authmode_ex, "WPA", 3))
			return "wpa-psk";
	}
	if (!strcmp(name, "DEF-p_wireless_ath0_11bg-authmode")) {
		if (!memcmp(data.authmode_ex, "WPA2", 4) && !memcmp(data.authmode, "PSK", 3))
			return "psk2";
		if (!memcmp(data.authmode_ex, "WPA", 3) && !memcmp(data.authmode, "PSK", 3))
			return "psk";
	}
	if (!strcmp(name, "DEF-p_wireless_ath0_11bg-wpapsk")) {
		static char passphrase[25];
		strncpy(passphrase, (char *)&data.passphrase[1], (data.passphrase[0] & 0xff));
		return passphrase;
	}
	if (!strcmp(name, "DEF-p_wireless_ath0_11bg-crypto")) {
		if (!memcmp(data.crypto, "AES", 3))
			return "aes";
	}
	if (!strcmp(name, "region")) {
		static char region[3];
		region[2] = 0;
		memcpy(region, data.region, 2);
		return region;
	}
	if (!strcmp(name, "pincode")) {
		static char pincode[9];
		memcpy(pincode, data.wpspin, 8);
		pincode[8] = 0;
		return pincode;
	}

	return NULL;
}

#endif

#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS) || defined(HAVE_WMBR_G300NH) || defined(HAVE_WZRG450) ||           \
	defined(HAVE_DIR810L) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_ALPINE) || defined(HAVE_VENTANA) || \
	defined(HAVE_IPQ6018) || defined(HAVE_PERU)
void *getUEnv(char *name)
{
#ifdef HAVE_WZRG300NH
#define UOFFSET 0x40000
#elif HAVE_WZR450HP2
#define UOFFSET 0x40000
#elif HAVE_WZRHPAG300NH
#define UOFFSET 0x40000
#elif HAVE_WZRG450
#define UOFFSET 0x40000
#elif HAVE_WMBR_G300NH
#define UOFFSET 0x0
#elif HAVE_DIR810L
#define UOFFSET 0x0
#elif HAVE_VENTANA
#define UOFFSET 0x0
#elif HAVE_MVEBU
#define UOFFSET 0x0
#elif HAVE_IPQ6018
#define UOFFSET 0x0
#elif HAVE_IPQ806X
#define UOFFSET 0x0
#elif HAVE_PERU
#define UOFFSET 0x40000
#else
#define UOFFSET 0x3E000
#endif
	int try = 0;
	//      static char res[64];
	static char res[256];
	bzero(res, sizeof(res));
	//fprintf(stderr,"[u-boot env]%s\n",name);
#if defined(HAVE_WMBR_G300NH) || defined(HAVE_DIR810L)
	FILE *fp = fopen("/dev/mtdblock/1", "rb");
#elif HAVE_MVEBU
	FILE *fp = fopen("/dev/mtdblock/1", "rb");
#elif HAVE_PERU
	FILE *fp = fopen("/dev/mtdblock/0", "rb");
#elif HAVE_VENTANA
	FILE *fp = fopen("/dev/mtdblock/1", "rb");
#elif HAVE_IPQ6018
	int brand = getRouterBrand();
	FILE *fp;
	switch (brand) {
	case ROUTER_LINKSYS_MR7350:
		fp = fopen("/dev/mtdblock/11", "rb");
		break;
	case ROUTER_DYNALINK_DLWRX36:
		fp = fopen("/dev/mtdblock/14", "rb");
		break;
	case ROUTER_ASUS_AX89X:
	case ROUTER_LINKSYS_MR5500:
	case ROUTER_LINKSYS_MX5500:
		fp = fopen("/dev/mtdblock/9", "rb");
		break;
	default:
		fp = fopen("/dev/mtdblock/18", "rb");
		break;
	}
#elif HAVE_IPQ806X
	int brand = getRouterBrand();
	FILE *fp;
	if (brand == ROUTER_LINKSYS_EA8500) {
		fp = fopen("/dev/mtd10", "rb");
	} else if (brand == ROUTER_LINKSYS_EA8300) {
		fp = fopen("/dev/mtd7", "rb");
	} else {
		fp = fopen("/dev/mtd3", "rb");
	}
#else
	FILE *fp = fopen("/dev/mtdblock/0", "rb");
#endif
	char newname[64];
	snprintf(newname, 64, "%s=", name);
	fseek(fp, UOFFSET, SEEK_SET);
	char *mem = safe_malloc(0x2000);
again:;
	fread(mem, 0x2000, 1, fp);
	fclose(fp);
#ifdef HAVE_VENTANA
	if (try == 0 && mem[0] == 0xff) {
		try = 1;
		fp = fopen("/dev/mtdblock/2", "rb");
		goto again;
	}
#endif
	int s = (0x2000 - 1) - strlen(newname);
	int i;
	int l = strlen(newname);
	for (i = 0; i < s; i++) {
		if (!strncmp(mem + i, newname, l)) {
			strncpy(res, mem + i + l, sizeof(res) - 1);
			free(mem);
			return res;
		}
	}
	free(mem);
#ifdef HAVE_WZR450HP2
	char *result = getUEnvExt(name);
	if (result)
		return result;
#endif
	return NULL;
}
#endif

char *get_wan_ipaddr(void)
{
	char *wan_ipaddr;
	char *wan_proto = nvram_safe_get("wan_proto");
	int wan_link = check_wan_link(0);

	if (!strcmp(wan_proto, "pptp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_ipaddr");
	} else if (!strcmp(wan_proto, "pppoe")
#ifdef HAVE_PPPOATM
		   || !strcmp(wan_proto, "pppoa")
#endif
#ifdef HAVE_PPPOEDUAL
		   || !strcmp(wan_proto, "pppoe_dual")
#endif
#ifdef HAVE_3G
		   || !strcmp(wan_proto, "3g")
#endif
	) {
		wan_ipaddr = wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
#ifdef HAVE_L2TP
	} else if (!strcmp(wan_proto, "l2tp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("l2tp_get_ip") : nvram_safe_get("wan_ipaddr");
#endif
	} else {
		wan_ipaddr = nvram_safe_get("wan_ipaddr");
	}

	if (*wan_ipaddr == 0)
		wan_ipaddr = "0.0.0.0";

	return wan_ipaddr;
}

/*
 * Find process name by pid from /proc directory 
 */
char *find_name_by_proc(int pid)
{
	FILE *fp;
	char line[254];
	char filename[80];
	static char name[80];

	snprintf(filename, sizeof(filename), "/proc/%d/status", pid);

	if ((fp = fopen(filename, "r"))) {
		fgets(line, sizeof(line), fp);
		/*
		 * Buffer should contain a string like "Name: binary_name" 
		 */
		sscanf(line, "%*s %s", name);
		fclose(fp);
		return name;
	}

	return "";
}

unsigned int read_gpio(char *device)
{
	FILE *fp;
	unsigned int val;

	if ((fp = fopen(device, "r"))) {
		fread(&val, 4, 1, fp);
		fclose(fp);
		// fprintf(stderr, "----- gpio %s = [%X]\n",device,val);
		return val;
	} else {
		perror(device);
		return 0;
	}
}

unsigned int write_gpio(char *device, unsigned int val)
{
	FILE *fp;

	if ((fp = fopen(device, "w"))) {
		fwrite(&val, 4, 1, fp);
		fclose(fp);
		// fprintf(stderr, "----- set gpio %s = [%X]\n",device,val);
		return 1;
	} else {
		perror(device);
		return 0;
	}
}

// note - broadcast addr returned in ipaddr
void get_broadcast(char *ipaddr, size_t len, char *netmask)
{
	int ip2[4], mask2[4];
	unsigned char ip[4], mask[4];

	if (!ipaddr || !netmask)
		return;

	sscanf(ipaddr, "%d.%d.%d.%d", &ip2[0], &ip2[1], &ip2[2], &ip2[3]);
	sscanf(netmask, "%d.%d.%d.%d", &mask2[0], &mask2[1], &mask2[2], &mask2[3]);
	int i = 0;

	for (i = 0; i < 4; i++) {
		ip[i] = ip2[i];
		mask[i] = mask2[i];
		// ip[i] = (ip[i] & mask[i]) | !mask[i];
		ip[i] = (ip[i] & mask[i]) | (0xff & ~mask[i]);
	}

	snprintf(ipaddr, len, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#ifdef WDS_DEBUG
	fprintf(fp, "get_broadcast return %s\n", value);
#endif
}

int wanChanged(void)
{
	FILE *fp = fopen("/tmp/.wanchange", "rb");
	if (fp) {
		fclose(fp);
		unlink("/tmp/.wanchange");
		return 1;
	}
	return 0;
}

void notifywanChange(void)
{
	FILE *fp = fopen("/tmp/.wanchange", "wb");
	if (fp) {
		fputs("change", fp);
		fclose(fp);
	}
}

void set_ip_forward(char c)
{
	char ch[16];
	sprintf(ch, "%c", c);
	writeprocsysnet("ipv4/ip_forward", ch);
}

int ifexists(const char *ifname)
{
	return getifcount(ifname) > 0 ? 1 : 0;
}

int getifcount(const char *ifprefix)
{
	/*
	 * char devcall[128];
	 * 
	 * sprintf (devcall, "cat /proc/net/dev|grep \"%s\"|wc -l", ifprefix);
	 * FILE *in = popen (devcall, "rb"); if (in == NULL) return 0; int count;
	 * fscanf (in, "%d", &count); pclose (in); return count;
	 */
	char *iflist = calloc(256, 1);

	int c = getIfListB(iflist, ifprefix, 0, 1, 0);

	free(iflist);
	return c;
}

static void skipline(FILE *in)
{
	if (!in)
		return;
	while (1) {
		int c = getc(in);

		if (c == EOF)
			return;
		if (c == 0x0)
			return;
		if (c == 0xa)
			return;
	}
}

/*
 * strips trailing char(s) c from string
 */
void strtrim_right(char *p, int c)
{
	char *end;
	int len;

	len = strlen(p);
	while (*p && len) {
		end = p + len - 1;
		if (c == *end)
			*end = 0;
		else
			break;
		len = strlen(p);
	}
	return;
}

int getIfList(char *buffer, const char *ifprefix)
{
	return getIfListB(buffer, ifprefix, 0, 0, 0);
}

int getIfListNoPorts(char *buffer, const char *ifprefix)
{
	return getIfListB(buffer, ifprefix, 0, 0, 1);
}

static int ifcompare(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}

int getIfByIdx(char *ifname, int index)
{
	if (!if_indextoname(index, ifname))
		return 0;
	return 1;
#if 0
	FILE *in = fopen("/proc/net/dev", "rb");
	if (!in)
		return 0;
	// skip the first 2 lines
	skipline(in);
	skipline(in);
	int idx = 0;
	int ifcount = 0;
	while (1) {
		int c = getc(in);
		if (c == 0 || c == EOF) {
			fclose(in);
			ifname[0] = 0;
			return 0;
		}
		if (c == 0x20)
			continue;
		if (c == ':' || ifcount == 30) {
			ifname[ifcount++] = 0;
			if (idx == index) {
				fclose(in);
				return 1;
			}
			skipline(in);
			ifcount = 0;
			continue;
		}
		ifname[ifcount++] = c;
	}
#endif
}

// returns a physical interfacelist filtered by ifprefix. if ifprefix is
// NULL, all valid interfaces will be returned
int getIfListB(char *buffer, const char *ifprefix, int bridgesonly, int nosort, int noports)
{
	FILE *in = fopen("/proc/net/dev", "rb");
	if (!in)
		return 0;
	char *ignorelist[] = { "wifi", "ifb", "imq", "etherip", "lo",  "teql",	 "gre",
			       "ppp",  "aux", "ctf", "tap",	"sit", "ip6tnl", "miireg" };
	char ifname[32];

	// skip the first 2 lines
	skipline(in);
	skipline(in);
	int ifcount = 0;
	int count = 0;
	char **sort = NULL;
	while (1) {
		int c = getc(in);

		if (c == 0 || c == EOF) {
			fclose(in);
			goto sort;
		}
		if (c == 0x20)
			continue;
		if (c == ':' || ifcount == 30) {
			ifname[ifcount++] = 0;
			int skip = 0;
			if (bridgesonly && !isbridge(ifname))
				skip = 1;
			if (ifprefix) {
				if (strncmp(ifname, ifprefix, strlen(ifprefix))) {
					skip = 1;
				}
			} else {
				int i;
				for (i = 0; i < sizeof(ignorelist) / sizeof(ignorelist[0]); i++) {
					if (!strncmp(ifname, ignorelist[i], strlen(ignorelist[i]))) {
						skip = 1;
						break;
					}
				}
			}
			if (noports && isbridged(ifname))
				skip = 1;
			if (!skip) {
				if (!sort) {
					sort = malloc(sizeof(char *));
				} else {
					sort = realloc(sort, sizeof(char *) * (count + 1));
				}
				sort[count] = malloc(strlen(ifname) + 1);
				strcpy(sort[count], ifname);
				count++;
			}
			skip = 0;
			ifcount = 0;
			bzero(ifname, 32);
			skipline(in);
			continue;
		}
		if (ifcount < 30)
			ifname[ifcount++] = c;
	}
sort:;
	if (!nosort && count &&
	    sort) { // if ifprefix doesnt match to any interface, sort might be NULL here, so check this condition
		qsort(sort, count, sizeof(char *), ifcompare);
	}
	int i;
	if (sort) {
		for (i = 0; i < count; i++) {
			strcat(buffer, sort[i]);
			strcat(buffer, " ");
			free(sort[i]);
		}
		free(sort);
	}

	if (count)
		buffer[strlen(buffer) - 1] = 0; // fixup last space
	return count;
}

/*
 * Example: legal_hwaddr("00:11:22:33:44:aB"); return true;
 * legal_hwaddr("00:11:22:33:44:5"); return false;
 * legal_hwaddr("00:11:22:33:44:HH"); return false; 
 */
int sv_valid_hwaddr(char *value)
{
	unsigned int hwaddr[6];
	int tag = TRUE;
	int i, count;

	/*
	 * Check for bad, multicast, broadcast, or null address 
	 */
	for (i = 0, count = 0; *(value + i); i++) {
		if (*(value + i) == ':') {
			if ((i + 1) % 3 != 0) {
				tag = FALSE;
				break;
			}
			count++;
		} else if (ishexit(*(value + i))) /* one of 0 1 2 3 4 5 6 7 8 9 
							 * a b c d e f A B C D E F */
			continue;
		else {
			tag = FALSE;
			break;
		}
	}

	if (!tag || i != 17 || count != 5) /* must have 17's characters and 5's
						 * ':' */
		tag = FALSE;
	else if (sscanf(value, "%x:%x:%x:%x:%x:%x", &hwaddr[0], &hwaddr[1], &hwaddr[2], &hwaddr[3], &hwaddr[4], &hwaddr[5]) != 6) {
		tag = FALSE;
	} else
		tag = TRUE;
#ifdef WDS_DEBUG
	if (tag == FALSE)
		fprintf(fp, "failed valid_hwaddr\n");
#endif

	return tag;
}

char *cpustring(void)
{
#ifdef HAVE_MVEBU
	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		return "Marvell Armada 370/XP";
	} else {
		return "Marvell Armada 385";
	}
#elif HAVE_ALPINE
	return "Annapurna Labs Alpine";
#elif defined(HAVE_IPQ806X) || defined(HAVE_IPQ6018)
	struct cpumatches {
		char *match;
		char *name;
	};
	const static struct cpumatches cpunames[] = {
		{ "ipq8065", "QCA IPQ8065" }, //
		{ "ipq8064", "QCA IPQ8064" }, //
		{ "ipq8062", "QCA IPQ8062" }, //
		{ "ipq8068", "QCA IPQ8068" }, //
		{ "ipq4019", "QCA IPQ4019" }, //
		{ "ipq4029", "QCA IPQ4029" }, //
		{ "ipq4018", "QCA IPQ4018" }, //
		{ "ipq4028", "QCA IPQ4028" }, //
		{ "ipq6000", "QCA IPQ6000" }, //
		{ "ipq6010", "QCA IPQ6010" }, //
		{ "ipq6018", "QCA IPQ6018" }, //
		{ "ipq6028", "QCA IPQ6028" }, //
		{ "ipq5000", "QCA IPQ5000" }, //
		{ "ipq5010", "QCA IPQ5010" }, //
		{ "ipq5016", "QCA IPQ5016" }, //
		{ "ipq5028", "QCA IPQ5028" }, //
		{ "ipq5018", "QCA IPQ5018" }, //
		{ "ipq8173", "QCA IPQ8173" }, //
		{ "ipq8174", "QCA IPQ8174" }, //
		{ "ipq8176", "QCA IPQ8176" }, //
		{ "ipq8178", "QCA IPQ8178" }, //
		{ "ipq8070", "QCA IPQ8070" }, //
		{ "ipq8072", "QCA IPQ8072" }, //
		{ "ipq8076", "QCA IPQ8076" }, //
		{ "ipq8078", "QCA IPQ8078" }, //
		{ "ipq8074", "QCA IPQ8074" }, //
	};
	FILE *fp = fopen("/sys/firmware/devicetree/base/compatible", "rb");
	if (!fp) {
		return "QCA IPQXXXX";
	}
	char compatible[128];
	fread(compatible, 1, sizeof(compatible), fp);
	fclose(fp);
	int idx = 0;
	while (idx < 128) {
		if (compatible[idx] == 0)
			compatible[idx] = 0x20;
		idx++;
	}
	compatible[127] = 0;
	char *cpu = &compatible[0];
	int i;
	for (i = 0; i < sizeof(cpunames) / sizeof(cpunames[0]); i++) {
		if (strstr(cpu, cpunames[i].match)) {
			return cpunames[i].name;
		}
	}
	return "QCA IPQXXXX";
#elif HAVE_UNIWIP
	return "FreeScale MPC8314";
#elif HAVE_WDR4900
	return "FreeScale P1014";
#elif HAVE_RB600
	return "FreeScale MPC8343";
#elif HAVE_NEWPORT
	return "Cavium ThunderX CN81XX";
#elif HAVE_VENTANA
	return "FreeScale i.MX6 Quad/DualLite";
#elif HAVE_NORTHSTAR
	FILE *fp = fopen("/proc/bcm_chipinfo", "rb");
	if (!fp) {
		return "Broadcom BCM470X";
		return buf;
	}
	int chipid;
	int chiprevision;
	int packageoption;
	fscanf(fp, "%*s %X\n", &chipid);
	fscanf(fp, "%*s %X\n", &chiprevision);
	fscanf(fp, "%*s %X\n", &packageoption);
	fclose(fp);
	if (chipid == 53030 || chipid == 53010 || chipid == 53011 || chipid == 53012 || chipid == 53018 ||
	    chipid == 53019) { // 53030
		if (packageoption == 0)
			return "Broadcom BCM4709";
		if (packageoption == 1)
			return "Broadcom BCM4707";
		if (packageoption == 2)
			return "Broadcom BCM4708";
	} else if (chipid == 53573) {
		return "Broadcom BCM47189";
	} else
		return "Broadcom BCM470X";
#else
	static char buf[256];
	FILE *fcpu = fopen("/proc/cpuinfo", "r");

	if (fcpu == NULL) {
		return NULL;
	}
	int i;

#ifdef HAVE_MAGICBOX
	int cnt = 0;
#endif
#ifdef HAVE_X86
	int cnt = 0;
#endif
	for (i = 0; i < 256; i++) {
		int c = getc(fcpu);

		if (c == EOF) {
			fclose(fcpu);
			return NULL;
		}
		if (c == ':')
#ifdef HAVE_MAGICBOX
			cnt++;
		if (cnt == 2)
			break;
#elif HAVE_X86
			cnt++;
		if (cnt == 5)
			break;
#else
			break;
#endif
	}
	getc(fcpu);
	for (i = 0; i < 255; i++) {
		int c = getc(fcpu);

		if (c == EOF) {
			fclose(fcpu);
			return NULL;
		}
		if (c == 0xa || c == 0xd)
			break;
		buf[i] = c;
	}
	buf[i] = 0;
	fclose(fcpu);
	return buf;
#endif
}

int _domod(char *module, char *loader)
{
	char word[256];
	char check[256];
	char *next, *wordlist;
	int ret = 0;
	char *target;
	wordlist = module;
	foreach(word, wordlist, next)
	{
		target = word;
		if (nvram_match("module_testing", "1")) {
			sprintf(check, "/jffs/modules_debug/%s.ko", word);
			FILE *fp = fopen(check, "rb");
			if (fp) {
				fclose(fp);
				target = check;
			}
			sprintf(check, "/jffs/modules_debug/%s", word);
			fp = fopen(check, "rb");
			if (fp) {
				fclose(fp);
				target = check;
			}
		}
		ret |= _evalpid((char *const[]){ loader, target, NULL }, ">/dev/null", 0, NULL);
	}
	return ret;
}

int insmod(char *module)
{
	return _domod(module, "insmod");
}

int modprobe(char *module)
{
	return _domod(module, "modprobe");
}

void rmmod(char *module)
{
	char word[256];
	char *next, *wordlist;
	wordlist = module;
	foreach(word, wordlist, next)
	{
		_evalpid((char *const[]){ "rmmod", word, NULL }, ">/dev/null", 0, NULL);
	}
}

#include "revision.h"

char *getSoftwareRevision(void)
{
	return "" SVN_REVISION "";
}

#ifdef HAVE_OLED
void initlcd()
{
}

void lcdmessage(char *message)
{
	eval("oled-print", "DD-WRT v24 sp2", "build:" SVN_REVISION, "3G/UMTS Router", message);
}

void lcdmessaged(char *dual, char *message)
{
}

#endif

#if 0

static int fd;

void SetEnvironment()
{
	system("stty ispeed 2400 < /dev/tts/1");
	system("stty raw < /dev/tts/1");
}

int Cmd = 254;			/* EZIO Command */
int cls = 1;			/* Clear screen */
void Cls()
{
	write(fd, &Cmd, 1);
	write(fd, &cls, 1);
}

int init = 0x28;
void Init()
{
	write(fd, &Cmd, 1);
	write(fd, &init, 1);
}

int stopsend = 0x37;
void StopSend()
{
	write(fd, &Cmd, 1);
	write(fd, &init, 1);
}

int home = 2;			/* Home cursor */
void Home()
{
	write(fd, &Cmd, 1);
	write(fd, &home, 1);
}

int readkey = 6;		/* Read key */
void ReadKey()
{
	write(fd, &Cmd, 1);
	write(fd, &readkey, 1);
}

int blank = 8;			/* Blank display */
void Blank()
{
	write(fd, &Cmd, 1);
	write(fd, &blank, 1);
}

int hide = 12;			/* Hide cursor & display blanked characters */
void Hide()
{
	write(fd, &Cmd, 1);
	write(fd, &hide, 1);
}

int turn = 13;			/* Turn On (blinking block cursor) */
void TurnOn()
{
	write(fd, &Cmd, 1);
	write(fd, &turn, 1);
}

int show = 14;			/* Show underline cursor */
void Show()
{
	write(fd, &Cmd, 1);
	write(fd, &show, 1);
}

int movel = 16;			/* Move cursor 1 character left */
void MoveL()
{
	write(fd, &Cmd, 1);
	write(fd, &movel, 1);
}

int mover = 20;			/* Move cursor 1 character right */
void MoveR()
{
	write(fd, &Cmd, 1);
	write(fd, &mover, 1);
}

int scl = 24;			/* Scroll cursor 1 character left */
void ScrollL()
{
	write(fd, &Cmd, 1);
	write(fd, &scl, 1);
}

int scr = 28;			/* Scroll cursor 1 character right */
void ScrollR()
{
	write(fd, &Cmd, 1);
	write(fd, &scr, 1);
}

int setdis = 64;		/* Command */
void SetDis()
{
	write(fd, &Cmd, 1);
	write(fd, &setdis, 1);

}

int a, b;
void ShowMessage(char *str1, char *str2)
{
	char nul[] = "                                       ";

	a = strlen(str1);
	b = 40 - a;
	write(fd, str1, a);
	write(fd, nul, b);
	write(fd, str2, strlen(str2));
}

void initlcd()
{

	fd = open("/dev/tts/1", O_RDWR);

				  /** Open Serial port (COM2) */
	if (fd > 0) {
		close(fd);
		SetEnvironment();	/* Set RAW mode */
		fd = open("/dev/tts/1", O_RDWR);
		Init();		/* Initialize EZIO twice */
		Init();

		Cls();		/* Clear screen */
	}
	close(fd);
}

void lcdmessage(char *message)
{

	fd = open("/dev/tts/1", O_RDWR);
				   /** Open Serial port (COM2) */

	if (fd > 0) {
		Init();		/* Initialize EZIO twice */
		Init();
		SetDis();
		Cls();
		Home();
		ShowMessage("State", message);
		close(fd);
	}
}

void lcdmessaged(char *dual, char *message)
{

	fd = open("/dev/tts/1", O_RDWR);

				  /** Open Serial port (COM2) */

	if (fd > 0) {
		Init();		/* Initialize EZIO twice */
		Init();
		SetDis();
		Cls();		/* Clear screen */
		Home();
		ShowMessage(dual, message);
		close(fd);
	}
}

#endif
static int i64c(int i)
{
	i &= 0x3f;
	if (i == 0)
		return '.';
	if (i == 1)
		return '/';
	if (i < 12)
		return ('0' - 2 + i);
	if (i < 38)
		return ('A' - 12 + i);
	return ('a' - 38 + i);
}

int crypt_make_salt(char *p, int cnt, int x)
{
	x += getpid() + time(NULL);
	do {
		/*
		 * x = (x*1664525 + 1013904223) % 2^32 generator is lame (low-order
		 * bit is not "random", etc...), but for our purposes it is good
		 * enough 
		 */
		x = x * 1664525 + 1013904223;
		/*
		 * BTW, Park and Miller's "minimal standard generator" is x = x*16807 
		 * % ((2^31)-1) It has no problem with visibly alternating lowest bit
		 * but is also weak in cryptographic sense + needs div, which needs
		 * more code (and slower) on many CPUs 
		 */
		*p++ = i64c(x >> 16);
		*p++ = i64c(x >> 22);
	} while (--cnt);
	*p = '\0';
	return x;
}

#include <crypt.h>

char *getUUID(char *buf)
{
	FILE *fp = fopen("/proc/sys/kernel/random/uuid", "rb");
	if (fp) {
		fscanf(fp, "%s", buf);
		fclose(fp);
		return buf;
	}
	return NULL;
}

unsigned char *zencrypt(unsigned char *passwd, unsigned char *passout)
{
	char salt[sizeof("$N$XXXXXXXX")]; /* "$N$XXXXXXXX" or "XX" */

	strcpy(salt, "$1$");
	crypt_make_salt(salt + 3, 4, 0);
#ifndef __UCLIBC__
	struct crypt_data data;
	strcpy(passout, crypt_r((char *)passwd, (char *)salt, &data));
#else
	strcpy(passout, crypt((char *)passwd, (char *)salt));
#endif
	return passout;
}

int has_gateway(void)
{
	if (nvram_match("wk_mode", "gateway"))
		return 1;
	if (nvram_match("wk_mode", "olsr") && nvram_matchi("olsrd_gateway", 1))
		return 1;
	return 0;
}

void set_smp_affinity(int irq, int mask)
{
	if (nvram_match("console_debug", "1"))
		dd_loginfo(__func__, "set smp_affinity mask %x for irq %d\n", mask, irq);
	char s_cpu[32];
	snprintf(s_cpu, sizeof(s_cpu), "%x", mask);
	writevaproc(s_cpu, "/proc/irq/%d/smp_affinity", irq);
}

void set_named_smp_affinity_mask(char *name, int mask, int entry)
{
	FILE *in = fopen("/proc/interrupts", "rb");
	if (!in)
		return;
	char line[256];
	int e = 0;
	char irq[256];
	while (fgets(line, sizeof(line) - 1, in)) {
		strncpy(irq, line, sizeof(irq) - 1);
		int i;
		int cnt = 0;
		if (!strchr(line, ':'))
			continue;
		for (i = 0; i < strlen(line); i++) {
			if (isdigit(line[i]))
				irq[cnt++] = line[i];
			if (line[i] == ':') {
				irq[cnt++] = 0;
				break;
			}
		}
		char match[256];
		memset(match, 0, sizeof(match));
		cnt = sizeof(match) - 1;
		int offset = strlen(line) - 2;
		int start = 0;
		i = offset;
		while (i--) {
			if (line[i] != 0x20)
				start = i;
			else
				break;
		}
		if (!start)
			continue;
		strcpy(match, &line[start]);
		match[strlen(match) - 1] = 0;
		if (!strcmp(match, name))
			e++;
		if (e == entry) {
			goto out;
		}
	}
	fclose(in);
	return;
out:;
	fclose(in);

	set_smp_affinity(atoi(irq), mask);
}

void set_named_smp_affinity(char *name, int core, int entry)
{
	set_named_smp_affinity_mask(name, 1 << core, entry);
}

void set_named_smp_affinity_list(char *name, char *cpulist, int entry)
{
	char var[32];
	char *next;
	int mask = 0;
	foreach(var, cpulist, next)
	{
		mask |= 1 << atoi(var);
	}
	set_named_smp_affinity_mask(name, mask, entry);
}

void getPortMapping(int *vlanmap)
{
	if (nvram_match("vlan1ports", "0 5")) {
		vlanmap[0] = 0;
		vlanmap[5] = 5;
		if (nvram_match("vlan0ports", "4 3 2 1 5*")) {
			vlanmap[1] = 4;
			vlanmap[2] = 3;
			vlanmap[3] = 2;
			vlanmap[4] = 1;
		} else if (nvram_match("vlan0ports", "4 1 2 3 5*")) {
			vlanmap[1] = 4;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		} else // nvram_match ("vlan0ports", "1 2 3 4 5*")
		// nothing to do
		{
		}
	} else if (nvram_match("vlan2ports", "0 5u")) {
		vlanmap[0] = 0;
		vlanmap[5] = 5;
		if (nvram_match("vlan1ports", "4 3 2 1 5*")) {
			vlanmap[1] = 4;
			vlanmap[2] = 3;
			vlanmap[3] = 2;
			vlanmap[4] = 1;
		} else if (nvram_match("vlan1ports", "4 1 2 3 5*")) {
			vlanmap[1] = 4;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		}
		if (nvram_match("vlan1ports", "4 3 2 1 5*") && nvram_matchi("boardnum", 32)) { //R7000/R7000P
			vlanmap[1] = 1;
			vlanmap[2] = 2;
			vlanmap[3] = 3;
			vlanmap[4] = 4;
		}
	} else if (nvram_match("vlan1ports", "4 5")) {
		vlanmap[0] = 4;
		vlanmap[5] = 5;
		if (nvram_match("vlan0ports", "0 1 2 3 5*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		} else // nvram_match ("vlan0ports", "3 2 1 0 5*")
		{
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
	} else if (nvram_match("vlan1ports", "1 5")) { // Linksys WTR54GS
		vlanmap[5] = 5;
		vlanmap[0] = 1;
		vlanmap[1] = 0;
	} else if (nvram_match("vlan2ports", "0 8") || nvram_match("vlan2ports", "0 8u") || nvram_match("vlan2ports", "0 8t") ||
		   nvram_match("vlan2ports", "0 8*")) {
		vlanmap[0] = 0;
		vlanmap[5] = 8;
		if (nvram_match("vlan1ports", "4 3 2 1 8*")) {
			vlanmap[1] = 4;
			vlanmap[2] = 3;
			vlanmap[3] = 2;
			vlanmap[4] = 1;
		}
	} else if (nvram_match("vlan2ports", "4 8") || nvram_match("vlan2ports", "4 8u")) {
		vlanmap[0] = 4;
		vlanmap[5] = 8;
		if (nvram_match("vlan1ports", "0 1 2 3 8*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		} else // "3 2 1 0 8*"
		{
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
		if (nvram_match("vlan1ports", "0 1 2 3 8*") && nvram_matchi("boardnum",
									    4536)) { // WNDR4500/WNDR4500V2/R6300V1
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
		if (nvram_match("vlan1ports", "3 2 1 0 5 7 8*") && nvram_matchi("boardnum", 32)) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		}
	} else if (nvram_match("vlan1ports", "4 8")) {
		vlanmap[0] = 4;
		vlanmap[5] = 8;
		if (nvram_match("vlan2ports", "0 1 2 3 8*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		}
	} else if (nvram_match("vlan2ports", "4 5") || nvram_match("vlan2ports", "4 5u")) {
		vlanmap[0] = 4;
		vlanmap[5] = 5;
		if (nvram_match("vlan1ports", "0 1 2 3 5*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		} else { // nvram_match ("vlan1ports", "3 2 1 0 5*")
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
		if (nvram_match("vlan1ports", "0 1 2 3 5*") && nvram_matchi("boardnum", 679)) { //R6300V2
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}

	} else if (nvram_match("vlan2ports", "0 5u")) {
		vlanmap[0] = 0;
		vlanmap[5] = 5;

		vlanmap[1] = 1;
		vlanmap[2] = 2;
		vlanmap[3] = 3;
		vlanmap[4] = 4;
	}
}

void getSystemMac(char *newmac)
{
	int brand = getRouterBrand();
	switch (brand) {
	case ROUTER_ASUS_AC87U:
	case ROUTER_ASUS_AC88U:
	case ROUTER_ASUS_AC5300:
		strcpy(newmac, nvram_safe_get("et1macaddr"));
		break;
	case ROUTER_NETGEAR_R8000:
	case ROUTER_NETGEAR_R8500:
	case ROUTER_TRENDNET_TEW828:
	case ROUTER_ASUS_AC3100:
		strcpy(newmac, nvram_safe_get("et2macaddr"));
		break;
	case ROUTER_DLINK_DIR885:
		if (nvram_exists("et0macaddr"))
			strcpy(newmac, nvram_safe_get("et0macaddr"));
		else
			strcpy(newmac, nvram_safe_get("et2macaddr"));
		break;
	default:
		strcpy(newmac, nvram_safe_get("et0macaddr"));
		break;
	}
}

int isvlan(char *name)
{
	int socket_fd;
	struct vlan_ioctl_args ifr;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 0;
	strcpy(ifr.device1, name);
	ifr.cmd = GET_VLAN_VID_CMD;
	int err = ioctl(socket_fd, SIOCSIFVLAN, &ifr);
	close(socket_fd);
	return err ? 0 : 1;
}

#define MAX_BRIDGES 1024

#include <linux/if_bridge.h>
int isbridge(char *name)
{
	int i, num;
	char ifname[IFNAMSIZ];
	int ifindices[MAX_BRIDGES];
	int br_socket_fd = -1;
	unsigned long args[3] = { BRCTL_GET_BRIDGES, (unsigned long)ifindices, MAX_BRIDGES };

	if ((br_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return 0;

	num = ioctl(br_socket_fd, SIOCGIFBR, args);
	close(br_socket_fd);
	if (num < 0) {
		return 0;
	}

	for (i = 0; i < num; i++) {
		if (!if_indextoname(ifindices[i], ifname)) {
			return 0;
		}

		if (!strcmp(ifname, name))
			return 1;
	}
	return 0;
}

int isbridged(char *name)
{
	char path[128];
	sprintf(path, "/sys/class/net/%s/brport/learning", name);
	FILE *fp = fopen(path, "rb");
	if (fp) {
		fclose(fp);
		return 1;
	}
	return 0;
}

int has_multicast_to_unicast(char *name)
{
	char fname[64];
	snprintf(fname, sizeof(fname), "/sys/class/net/%s/brport/multicast_to_unicast", name);
	FILE *fp = fopen(fname, "rb");
	if (fp) {
		fclose(fp);
		return 1;
	}
	return 0;
}

int writeint(char *path, int a)
{
	int fd = open(path, O_WRONLY);
	if (fd == -1)
		return -1;
	char strval[32];
	snprintf(strval, sizeof(strval), "%d", a);
	write(fd, strval, strlen(strval));
	close(fd);
	return 0;
}

int writestr(char *path, char *a)
{
	int fd = open(path, O_WRONLY);
	if (fd == -1)
		return -1;
	write(fd, a, strlen(a));
	close(fd);
	return 0;
}

#define PER_MAC_LEN 18 // contain '\0'

static void s_mac_add(char *m, int inc)
{
	int i, j;
	for (i = 5; i >= 3; i--) {
		if (inc > 0) {
			if (m[i] == 0xFF) {
				m[i] = 0x0;
				continue;
			} else {
				m[i] = m[i] + inc;
				break;
			}
		} else {
			if (m[i] == 0x00) {
				m[i] = 0xFF;
				continue;
			} else {
				m[i] = m[i] + inc;
				break;
			}
		}
	}
}

static void s_MAC_ADD(char *mac, int inc)
{
	int i, j;
	unsigned char m[6];
	for (j = 0, i = 0; i < PER_MAC_LEN; i += 3, j++) {
		if (mac[i] >= 'A' && mac[i] <= 'F')
			mac[i] = mac[i] - 55;
		if (mac[i + 1] >= 'A' && mac[i + 1] <= 'F')
			mac[i + 1] = mac[i + 1] - 55;
		if (mac[i] >= 'a' && mac[i] <= 'f')
			mac[i] = mac[i] - 87;
		if (mac[i + 1] >= 'a' && mac[i + 1] <= 'f')
			mac[i + 1] = mac[i + 1] - 87;
		if (mac[i] >= '0' && mac[i] <= '9')
			mac[i] = mac[i] - 48;
		if (mac[i + 1] >= '0' && mac[i + 1] <= '9')
			mac[i + 1] = mac[i + 1] - 48;
		m[j] = mac[i] * 16 + mac[i + 1];
	}
	s_mac_add(m, inc);
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2], m[3], m[4], m[5]);
}

void MAC_ADD(char *mac)
{
	s_MAC_ADD(mac, 1);
}

void MAC_SUB(char *mac)
{
	s_MAC_ADD(mac, -1);
}

void mac_add(char *mac)
{
	s_mac_add(mac, 1);
}

void mac_sub(char *mac)
{
	s_mac_add(mac, -1);
}

int buf_to_file(char *path, char *buf)
{
	FILE *fp;

	if ((fp = fopen(path, "w"))) {
		fprintf(fp, "%s", buf);
		fclose(fp);
		return 1;
	}

	return 0;
}

#ifndef IFNAMSIZ
#include <net/if.h>
#endif

char *safe_get_wan_face(char *localwanface)
{
	if (nvram_match("wan_proto", "disabled")) {
		strlcpy(localwanface, "br0", IFNAMSIZ);
		return localwanface;
	}

	/*
	 * if (nvram_match ("pptpd_client_enable", "1")) { strncpy (localwanface, 
	 * "ppp0", IFNAMSIZ); return localwanface; }
	 */
	if (nvram_match("wan_proto", "pptp")
#ifdef HAVE_L2TP
	    || nvram_match("wan_proto", "l2tp")
#endif
#ifdef HAVE_PPPOATM
	    || nvram_match("wan_proto", "pppoa")
#endif
#ifdef HAVE_PPPOEDUAL
	    || nvram_match("wan_proto", "pppoe_dual")
#endif
	    || nvram_match("wan_proto", "pppoe")) {
		if (nvram_match("pppd_pppifname", ""))
			strlcpy(localwanface, "ppp0", IFNAMSIZ);
		else
			strlcpy(localwanface, nvram_safe_get("pppd_pppifname"), IFNAMSIZ);
	}
#ifdef HAVE_3G
	else if (nvram_match("wan_proto", "3g")) {
		if (nvram_match("3gdata", "mbim")) {
			strlcpy(localwanface, "wwan0", IFNAMSIZ);
		} else if (nvram_match("3gdata", "qmi")) {
			strlcpy(localwanface, "wwan0", IFNAMSIZ);
		} else if (nvram_match("3gdata", "sierradirectip")) {
			strlcpy(localwanface, "wwan0", IFNAMSIZ);
		} else {
			if (nvram_match("pppd_pppifname", ""))
				strlcpy(localwanface, "ppp0", IFNAMSIZ);
			else
				strlcpy(localwanface, nvram_safe_get("pppd_pppifname"), IFNAMSIZ);
		}

	}
#endif
#ifndef HAVE_MADWIFI
	else if (nvram_invmatch("sta_ifname", "")) {
		strlcpy(localwanface, nvram_safe_get("sta_ifname"), IFNAMSIZ);
	}
#else
	else if (nvram_invmatch("sta_ifname", "")) {
		if (nvram_matchi("wifi_bonding", 1))
			strlcpy(localwanface, "bond0", IFNAMSIZ);
		else
			strlcpy(localwanface, nvram_safe_get("sta_ifname"), IFNAMSIZ);
	}
#endif
#ifdef HAVE_IPETH
	else if (nvram_match("wan_proto", "iphone")) {
		strlcpy(localwanface, "iph0", IFNAMSIZ);
	} else if (nvram_match("wan_proto", "android")) {
		if (ifexists("usb0"))
			strlcpy(localwanface, "usb0", IFNAMSIZ);
		if (ifexists("wwan0"))
			strlcpy(localwanface, "wwan0", IFNAMSIZ);

	}
#endif
	else
		strlcpy(localwanface, nvram_safe_get("wan_ifname"), IFNAMSIZ);

	return localwanface;
}

char *getBridgeSTPType(char *br, char *word)
{
	char *next, *wordlist;
	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		GETENTRYBYIDX(stp, word, 1);
		if (bridge && br && strcmp(bridge, br))
			continue;
		strcpy(word, stp);
		return word;
	}
	if (br && !strcmp(br, "br0"))
		return nvram_matchi("lan_stp", 1) ? "STP" : "Off";
	return "Off";
}

int getBridgeSTP(char *br, char *word)
{
	if (br && word && strcmp(getBridgeSTPType(br, word), "Off"))
		return 1;
	else
		return 0;
}

int getBridgeForwardDelay(char *br)
{
	char *next, *wordlist, word[128];
	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		GETENTRYBYIDX(fd, word, 4);
		if (bridge && br && strcmp(bridge, br))
			continue;
		if (!fd)
			fd = "15";
		return atoi(fd);
	}
	return 15;
}

int getBridgeMaxAge(char *br)
{
	char *next, *wordlist, word[128];
	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		GETENTRYBYIDX(age, word, 5);
		if (bridge && br && strcmp(bridge, br))
			continue;
		if (!age)
			age = "20";
		return atoi(age);
	}
	return 20;
}

char *strstrtok(char *str, char del)
{
	int s = strlen(str);
	int i;
	for (i = 0; i < s; i++) {
		if (str[i] == del) {
			str[i] = 0;
			return &str[i + 1];
		}
	}
	return str;
}

#ifdef HAVE_RAID

char *getMountedDrives(void)
{
	FILE *in = fopen("/proc/mounts", "rb");
	if (in == NULL)
		return NULL;
	char line[512];
	char *drives = NULL;
	while (fgets(line, sizeof(line), in)) {
		char *dev = NULL;
		char *mp = NULL;
		char *fstype = NULL;
		dev = strstrtok(line, ' ');
		if (dev)
			mp = strstrtok(dev, ' ');
		if (mp)
			fstype = strstrtok(mp, ' ');
		if (dev) {
			if (!strncmp(line, "/dev/", 5)) {
				int c = 0;
				if (drives)
					c = 1;
				drives = realloc(drives, drives ? strlen(dev) + 2 + strlen(drives) : strlen(dev) + 1);
				if (c)
					strcat(drives, " ");
				else
					drives[0] = 0;
				strcat(drives, dev);
				continue;
			}
		}
#ifdef HAVE_ZFS
next:;
		if (fstype) {
			if (!strcmp(dev, "zfs")) {
				int c = 0;
				if (drives)
					c = 1;
				drives = realloc(drives, drives ? strlen(dev) + 2 + strlen(drives) : strlen(dev + 1));
				if (c)
					strcat(drives, " ");
				else
					drives[0] = 0;
				strcat(drives, dev);
			}
		}
#endif
	}
	return drives;
}

static char *s_getDrives(int type)
{
	char *mounts = NULL;
	if (type)
		mounts = getMountedDrives();
	DIR *dir;
	char **drives = NULL;
	int alloc = 0;
	struct dirent *file;
	int count = 0;
	if (!(dir = opendir("/dev")))
		return NULL;
	while (dir && (file = readdir(dir))) {
		char drv[128];
		sprintf(drv, "/dev/%s", file->d_name);
		if (!strncmp(file->d_name, "sd", 2) || !strncmp(file->d_name, "hd", 2) || !strncmp(file->d_name, "md", 2) ||
		    !strncmp(file->d_name, "mmcblk", 6) || !strncmp(file->d_name, "nvme", 4)) {
			char var[64];
			char *next;
			if (mounts) {
				foreach(var, mounts, next)
				{
					if (!strcmp(drv, var))
						goto next;
				}
			}
#ifdef HAVE_ZFS
			if (type) {
				char *d = &file->d_name[0];
				char stats[512];
				char cmp[32];
				if (d && *d) {
					strcpy(cmp, d);
					if (!strncmp(cmp, "sd", 2))
						cmp[3] = 0;
					else if (!strncmp(cmp, "hd", 2))
						cmp[3] = 0;
					else if (!strncmp(cmp, "mmcblk", 6))
						cmp[7] = 0;
					else if (!strncmp(cmp, "nvme", 4))
						cmp[7] = 0;
				}
				char grep[128];
				sprintf(grep, "zpool status|grep %s", cmp);
				FILE *p = popen(grep, "rb");
				char *result = NULL;
				if (p) {
					result = fgets(stats, sizeof(stats), p);
					pclose(p);
				}
				if (result) {
					if (strstr(result, cmp))
						goto next;
				}
			}
#endif
			int c = 0;
			if (drives)
				c = 1;
			drives = realloc(drives, sizeof(char **) * (count + 1));
			drives[count++] = strdup(drv);
			alloc += strlen(drv) + 1;
		}
next:;
	}
	closedir(dir);
	if (mounts)
		free(mounts);

	qsort(drives, count, sizeof(char *), ifcompare);
	char *result = malloc(alloc + 1);
	memset(result, 0, alloc + 1);
	int i;
	for (i = 0; i < count; i++) {
		if (i) {
			strcat(result, " ");
		}
		strcat(result, drives[i]);
		free(drives[i]);
	}
	free(drives);
	return result;
}

char *getUnmountedDrives(void)
{
	return s_getDrives(1);
}

char *getAllDrives(void)
{
	return s_getDrives(0);
}

#endif

#ifdef HAVE_SYSCTL_EDIT
#include <dirent.h>

static char *sysctl_blacklist[] = { SYSCTL_BLACKLIST };

static void internal_sysctl_apply(char *path, void *priv,
				  void (*callback)(char *path, char *nvname, char *name, char *sysval, void *priv))
{
	DIR *directory;
	struct dirent *entry;
	char buf[256];
	int cnt = 0;
	if ((directory = opendir(path)) != NULL) {
		while ((entry = readdir(directory)) != NULL) {
			if ((!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
				continue;
			if (entry->d_type == DT_DIR) {
				int a;
				for (a = 0; a < sizeof(sysctl_blacklist) / sizeof(char *); a++) {
					if (!strcmp(entry->d_name,
						    sysctl_blacklist[a])) // supress kernel warning
						continue;
				}
				char dir[1024];
				sprintf(dir, "%s/%s", path, entry->d_name);
				internal_sysctl_apply(dir, priv, callback);
				continue;
			}
		}
		closedir(directory);
	}
	if ((directory = opendir(path)) != NULL) {
		while ((entry = readdir(directory)) != NULL) {
			if (entry->d_type == DT_REG) {
				int a;
				for (a = 0; a < sizeof(sysctl_blacklist) / sizeof(char *); a++) {
					if (!strcmp(entry->d_name,
						    sysctl_blacklist[a])) // supress kernel warning
						goto next;
				}

				char fname[128];
				char fval[128] = { 0 };
				sprintf(fname, "%s/%s", path, entry->d_name);
				struct stat sb;
				stat(fname, &sb);
				if (!(sb.st_mode & S_IWUSR))
					continue;
				FILE *in = fopen(fname, "rb");
				if (!in)
					continue;
				fgets(fval, 127, in);
				int i;
				int len = strlen(fval);
				for (i = 0; i < len; i++) {
					if (fval[i] == '\t')
						fval[i] = 0x20;
					if (fval[i] == '\n')
						fval[i] = 0;
				}
				fclose(in);

				char title[64] = { 0 };
				strcpy(title, &path[10]);
				len = strlen(title);
				for (i = 0; i < len; i++)
					if (title[i] == '/')
						title[i] = '.';
				char nvname[128];
				sprintf(nvname, "%s%s%s", title, strlen(title) ? "." : "", entry->d_name);
				callback(path, nvname, entry->d_name, fval, priv);
next:;
			}
		}
		callback(NULL, NULL, NULL, NULL, priv);
		closedir(directory);
	}
}

void sysctl_apply(void *priv, void (*callback)(char *path, char *nvname, char *name, char *sysval, void *priv))
{
	internal_sysctl_apply("/proc/sys", priv, callback);
}
#endif
