/*

 * utils.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#define unlikely(x)     __builtin_expect((x),0)
#endif

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */

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
	int offset = 0;;
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
//              case ROUTER_NETGEAR_R6400:
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
		mtd = getMTD("board_data");
		break;
	}

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
		nvram_set("ath0_security_mode", "psk2");
		nvram_set("ath0_crypto", "aes");
		nvram_set("ath0_wpa_psk", pass);
		nvram_set("ath0_akm", "psk2");
		nvram_set("ath1_security_mode", "psk2");
		nvram_set("ath1_crypto", "aes");
		nvram_set("ath1_wpa_psk", pass);
		nvram_set("ath1_akm", "psk2");
#endif
	}

}

int count_processes(char *pidName)
{
	FILE *fp;
	char line[254];
	char safename[64];

	sprintf(safename, " %s ", pidName);
	char zombie[64];

	sprintf(zombie, "Z   [%s]", pidName);	// do not count zombies
	int i = 0;

	cprintf("Search for %s\n", pidName);
	if ((fp = popen("ps", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			int len = strlen(line);
			if (len > 254)
				len = 254;
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
	return (30 + (((month & 9) == 8)
		      || ((month & 9) == 1)) - (month == 2) - (!(((year % 4) == 0)
								 && (((year % 100) != 0)
								     || ((year % 400) == 0)))
							       && (month == 2)));
}

#ifdef HAVE_IPV6
const char *getifaddr(char *ifname, int family, int linklocal)
{
	static char buf[INET6_ADDRSTRLEN];
	void *addr = NULL;
	struct ifaddrs *ifap, *ifa;

	if (getifaddrs(&ifap) != 0) {
		dprintf("getifaddrs failed: %s\n", strerror(errno));
		return NULL;
	}

	for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
		if ((ifa->ifa_addr == NULL) || (strncmp(ifa->ifa_name, ifname, IFNAMSIZ) != 0) || (ifa->ifa_addr->sa_family != family))
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

		if ((addr) && inet_ntop(ifa->ifa_addr->sa_family, addr, buf, sizeof(buf)) != NULL) {
			freeifaddrs(ifap);
			return buf;
		}
	}

	freeifaddrs(ifap);
	return NULL;
}
#endif
#ifdef HAVE_VLANTAGGING
char *getBridge(char *ifname, char *word)
{
	char *next, *wordlist;

	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next) {
		char *port = word;
		char *tag = strsep(&port, ">");
		char *prio = port;

		strsep(&prio, ">");
		if (!tag || !port)
			break;
		if (!strcmp(port, ifname))
			return tag;
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
	foreach(word, wordlist, next) {
		char *stp = word;
		char *bridge = strsep(&stp, ">");
		char *mtu = stp;
		char *prio = strsep(&mtu, ">");

		if (prio)
			strsep(&mtu, ">");

		if (!bridge || !stp)
			break;
		if (!strcmp(bridge, ifname)) {
			if (!prio || !mtu)
				return "1500";
			else
				return mtu;
		}
	}
	return "1500";
}

char *getMTU(char *ifname)
{
	if (!ifname)
		return "1500";
	char *mtu = nvram_nget("%s_mtu", ifname);
	if (!mtu || strlen(mtu) == 0)
		return "1500";
	return mtu;
}

char *getTXQ(char *ifname)
{
	if (!ifname)
		return "1000";
	char *txq = nvram_nget("%s_txq", ifname);
	if (!txq || strlen(txq) == 0) {
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

int check_vlan_support(void)
{
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_MAGICBOX)  || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
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

		snprintf(tmp, sizeof(tmp), "/var/run/%s.pid", buf);
		file_to_buf(tmp, tmp1, sizeof(tmp1));
		pid = atoi(tmp1);
	}
	return pid;
}

int check_wan_link(int num)
{
	int wan_link = 0;

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
	     || (nvram_match("wan_proto", "3g") && !nvram_match("3gdata", "hso")
		 && !nvram_match("3gdata", "qmi") && !nvram_match("3gdata", "mbim")
		 && !nvram_match("3gdata", "sierradirectip"))
#endif
	     || nvram_match("wan_proto", "heartbeat"))
	    ) {
		FILE *fp;
		char filename[80];
		char *name;

		if (num == 0)
			strcpy(filename, "/tmp/ppp/link");
		if ((fp = fopen(filename, "r"))) {
			int pid = -1;

			fclose(fp);
			if (nvram_match("wan_proto", "heartbeat")) {
				char buf[20];

				file_to_buf("/tmp/ppp/link", buf, sizeof(buf));
				pid = atoi(buf);
			} else
				pid = get_ppp_pid(filename);

			name = find_name_by_proc(pid);
			if (!strncmp(name, "pppoecd", 7) ||	// for PPPoE
			    !strncmp(name, "pppd", 4) ||	// for PPTP
			    !strncmp(name, "bpalogin", 8))	// for HeartBeat
				wan_link = 1;	// connect
			else {
				printf("The %s had been died, remove %s\n", nvram_safe_get("wan_proto"), filename);
				wan_link = 0;	// For some reason, the pppoed had been died, 
				// by link file still exist.
				unlink(filename);
			}
		}
	}
#if defined(HAVE_LIBMBIM)
	else if (nvram_match("wan_proto", "3g") && nvram_match("3gdata", "mbim")) {
		FILE *fp = fopen("/tmp/mbimstatus", "rb");
		int value = 0;
		if (fp) {
			fscanf(fp, "%d", &value);
			fclose(fp);
		}
		if (value)
			return 1;
	}
#endif
#if defined(HAVE_LIBQMI) || defined(HAVE_UQMI)
	else if (nvram_match("wan_proto", "3g") && nvram_match("3gdata", "sierradirectip")) {
		FILE *fp = fopen("/tmp/sierradipstatus", "rb");
		int value = 0;
		if (fp) {
			fscanf(fp, "%d", &value);
			fclose(fp);
		}
		if (value) {
#if defined(HAVE_TMK) || defined(HAVE_BKM)
#if 0
			char *gpio3g;
			gpio3g = nvram_get("gpio3g");
			if (gpio3g != NULL)
				set_gpio(atoi(gpio3g), 1);
#endif
#endif
			return 1;
		}
#if defined(HAVE_TMK) || defined(HAVE_BKM)
		char *gpio3g;
		gpio3g = nvram_get("gpio3g");
		if (gpio3g != NULL)
			set_gpio(atoi(gpio3g), 0);
		gpio3g = nvram_get("gpiowancable");
		if (gpio3g != NULL)
			set_gpio(atoi(gpio3g), 0);
#endif
		return 0;
	} else if (nvram_match("wan_proto", "3g") && nvram_match("3gdata", "qmi")) {
		FILE *fp = fopen("/tmp/qmistatus", "rb");
		int value = 0;
		if (fp) {
			fscanf(fp, "%d", &value);
			fclose(fp);
		}
#ifdef HAVE_UQMI
		if (value) {
#if defined(HAVE_TMK) || defined(HAVE_BKM)
#if 0
			char *gpio3g;
			gpio3g = nvram_get("gpio3g");
			if (gpio3g != NULL)
				set_gpio(atoi(gpio3g), 1);
#endif
#endif
			return 1;
		}
	} else if (nvram_match("wan_proto", "3g") && nvram_match("3gdata", "mbim")) {
		FILE *fp = fopen("/tmp/mbimstatus", "rb");
		int value = 0;
		if (fp) {
			fscanf(fp, "%d", &value);
			fclose(fp);
		}
		if (value) {
			return 1;
		}
#if defined(HAVE_TMK) || defined(HAVE_BKM)
		char *gpio3g;
		gpio3g = nvram_get("gpio3g");
		if (gpio3g != NULL)
			set_gpio(atoi(gpio3g), 0);
		gpio3g = nvram_get("gpiowancable");
		if (gpio3g != NULL)
			set_gpio(atoi(gpio3g), 0);
#endif
		return 0;
#else
		if (value)
			return 0;
		return 1;
#endif
	}
#endif
	else
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
					snprintf(ifr.ifr_name, IFNAMSIZ, "iph0");
					ioctl(sock, SIOCGIFFLAGS, &ifr);
					if ((ifr.ifr_flags & (IFF_RUNNING | IFF_UP))
					    == (IFF_RUNNING | IFF_UP))
						wan_link = 1;
					close(sock);
					break;
				}
			}
			fclose(fp);
			if (nvram_match("wan_ipaddr", "0.0.0.0")
			    || nvram_match("wan_ipaddr", ""))
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

#ifdef HAVE_WZR450HP2
struct ENV_EXTDATA {
	unsigned char wanmac[6];	// 0x0
	unsigned char lanmac[6];	// 0x6
	unsigned char wpspin[8];	// 0xc
	unsigned char passphrase[25];	// 0x14, length is max <=24 followed by 0 termination
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
	FILE *fp = fopen("/dev/mtdblock5", "rb");	// board data
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

#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS) || defined(HAVE_WMBR_G300NH) || defined(HAVE_WZRG450) || defined(HAVE_DIR810L) || defined(HAVE_MVEBU) || defined(HAVE_IPQ806X) || defined(HAVE_ALPINE)
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
#elif HAVE_MVEBU
#define UOFFSET 0x0
#elif HAVE_IPQ806X
#define UOFFSET 0x0
#else
#define UOFFSET 0x3E000
#endif
//      static char res[64];
	static char res[256];
	bzero(res, sizeof(res));
	//fprintf(stderr,"[u-boot env]%s\n",name);
#if defined(HAVE_WMBR_G300NH) || defined(HAVE_DIR810L)
	FILE *fp = fopen("/dev/mtdblock/1", "rb");
#elif HAVE_MVEBU
	FILE *fp = fopen("/dev/mtdblock/1", "rb");
#elif HAVE_IPQ806X
	int brand = getRouterBrand();
	FILE *fp;
	if (brand == ROUTER_LINKSYS_EA8500) {
		fp = fopen("/dev/mtdblock/10", "rb");
	} else {
		fp = fopen("/dev/mtdblock/3", "rb");
	}
#else
	FILE *fp = fopen("/dev/mtdblock/0", "rb");
#endif
	char newname[64];
	snprintf(newname, 64, "%s=", name);
	fseek(fp, UOFFSET, SEEK_SET);
	char *mem = safe_malloc(0x2000);
	fread(mem, 0x2000, 1, fp);
	fclose(fp);
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

	if (strlen(wan_ipaddr) == 0)
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

static int diag_led_4702(int type, int act)
{

#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_FONERA) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else
	if (act == START_LED) {
		switch (type) {
		case DMZ:
			writeprocsys("diag", "1");
			break;
		}
	} else {
		switch (type) {
		case DMZ:
			writeprocsys("diag", "0");
			break;
		}
	}
	return 0;
#endif
}

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)

static int C_led_4702(int i)
{
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE)  || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else
	FILE *fp;
	char string[10];
	int flg;

	bzero(string, 10);
	/*
	 * get diag before set 
	 */
	if ((fp = fopen("/proc/sys/diag", "r"))) {
		fgets(string, sizeof(string), fp);
		fclose(fp);
	} else
		perror("/proc/sys/diag");

	if (i)
		flg = atoi(string) | 0x10;
	else
		flg = atoi(string) & 0xef;

	bzero(string, 10);
	sprintf(string, "%d", flg);
	writeprocsys("diag", string);

	return 0;
#endif
}
#endif
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

static int diag_led_4704(int type, int act)
{
#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI)|| defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC) || defined(HAVE_ALPINE)
	return 0;
#else
	unsigned int control, in, outen, out;

#ifdef BCM94712AGR
	/*
	 * The router will crash, if we load the code into broadcom demo board. 
	 */
	return 1;
#endif
	static char hw_error = 0;
	// int brand;
	control = read_gpio("/dev/gpio/control");
	in = read_gpio("/dev/gpio/in");
	out = read_gpio("/dev/gpio/out");
	outen = read_gpio("/dev/gpio/outen");

	write_gpio("/dev/gpio/outen", (outen & 0x7c) | 0x83);
	switch (type) {
	case DIAG:		// GPIO 1
		if (hw_error) {
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x00);
			return 1;
		}

		if (act == STOP_LED) {	// stop blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x83);
			// cprintf("tallest:=====( DIAG STOP_LED !!)=====\n");
		} else if (act == START_LED) {	// start blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x81);
			// cprintf("tallest:=====( DIAG START_LED !!)=====\n");
		} else if (act == MALFUNCTION_LED) {	// start blinking
			write_gpio("/dev/gpio/out", (out & 0x7c) | 0x00);
			hw_error = 1;
			// cprintf("tallest:=====( DIAG MALFUNCTION_LED !!)=====\n");
		}
		break;

	}
	return 1;
#endif
}

static int diag_led_4712(int type, int act)
{

#if defined(HAVE_IPQ806X) || defined(HAVE_MVEBU) || defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA)|| defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC) | defined(HAVE_ALPINE)
	return 0;
#else
	unsigned int control, in, outen, out, ctr_mask, out_mask;

#ifdef BCM94712AGR
	/*
	 * The router will crash, if we load the code into broadcom demo board. 
	 */
	return 1;
#endif
	control = read_gpio("/dev/gpio/control");
	in = read_gpio("/dev/gpio/in");
	out = read_gpio("/dev/gpio/out");
	outen = read_gpio("/dev/gpio/outen");

	ctr_mask = ~(1 << type);
	out_mask = (1 << type);

	write_gpio("/dev/gpio/control", control & ctr_mask);
	write_gpio("/dev/gpio/outen", outen | out_mask);

	if (act == STOP_LED) {	// stop blinking
		// cprintf("%s: Stop GPIO %d\n", __FUNCTION__, type);
		write_gpio("/dev/gpio/out", out | out_mask);
	} else if (act == START_LED) {	// start blinking
		// cprintf("%s: Start GPIO %d\n", __FUNCTION__, type);
		write_gpio("/dev/gpio/out", out & ctr_mask);
	}

	return 1;
#endif
}

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
static int C_led_4712(int i)
{
	if (i == 1)
		return diag_led(DIAG, START_LED);
	else
		return diag_led(DIAG, STOP_LED);
}

int C_led(int i)
{
	int brand = getRouterBrand();

	if (brand == ROUTER_WRT54G1X || brand == ROUTER_LINKSYS_WRT55AG)
		return C_led_4702(i);
	else if (brand == ROUTER_WRT54G)
		return C_led_4712(i);
	else
		return 0;
}

int diag_led(int type, int act)
{
	int brand = getRouterBrand();

	if (brand == ROUTER_WRT54G || brand == ROUTER_WRT54G3G || brand == ROUTER_WRT300NV11)
		return diag_led_4712(type, act);
	else if (brand == ROUTER_WRT54G1X || brand == ROUTER_LINKSYS_WRT55AG)
		return diag_led_4702(type, act);
	else if ((brand == ROUTER_WRTSL54GS || brand == ROUTER_WRT310N || brand == ROUTER_WRT350N || brand == ROUTER_BUFFALO_WZRG144NH) && type == DIAG)
		return diag_led_4704(type, act);
	else {
		if (type == DMZ) {
			if (act == START_LED)
				return led_control(LED_DMZ, LED_ON);
			if (act == STOP_LED)
				return led_control(LED_DMZ, LED_OFF);
			return 1;
		}
	}
	return 0;
}
#endif
// note - broadcast addr returned in ipaddr
void get_broadcast(char *ipaddr, char *netmask)
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

	sprintf(ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
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
	char ch[8];
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

	int c = getIfListB(iflist, ifprefix, 0, 1);

	free(iflist);
	return c;
}

static void skipline(FILE * in)
{
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
	return getIfListB(buffer, ifprefix, 0, 0);
}

static int ifcompare(const void *a, const void *b)
{
	return strcmp(*(const char **)a, *(const char **)b);
}

// returns a physical interfacelist filtered by ifprefix. if ifprefix is
// NULL, all valid interfaces will be returned
int getIfListB(char *buffer, const char *ifprefix, int bridgesonly, int nosort)
{
	FILE *in = fopen("/proc/net/dev", "rb");
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
				if (!strncmp(ifname, "wifi", 4))
					skip = 1;
				if (!strncmp(ifname, "wlan", 4))
					skip = 1;
				if (!strncmp(ifname, "ifb", 3))
					skip = 1;
				if (!strncmp(ifname, "imq", 3))
					skip = 1;
				if (!strncmp(ifname, "etherip", 7))
					skip = 1;
				if (!strncmp(ifname, "lo", 2))
					skip = 1;
				if (!strncmp(ifname, "teql", 4))
					skip = 1;
				if (!strncmp(ifname, "gre", 3))
					skip = 1;
				if (!strncmp(ifname, "ppp", 3))
					skip = 1;
				if (!strncmp(ifname, "tun", 3))
					skip = 1;
				if (!strncmp(ifname, "tap", 3))
					skip = 1;
			}
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
	if (!nosort) {
		qsort(sort, count, sizeof(char *), ifcompare);
	}
	int i;
	for (i = 0; i < count; i++) {
		strcat(buffer, sort[i]);
		strcat(buffer, " ");
		free(sort[i]);
	}
	if (sort)
		free(sort);
	if (count)
		buffer[strlen(buffer) - 1] = 0;	// fixup last space
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
		} else if (ishexit(*(value + i)))	/* one of 0 1 2 3 4 5 6 7 8 9 
							 * a b c d e f A B C D E F */
			continue;
		else {
			tag = FALSE;
			break;
		}
	}

	if (!tag || i != 17 || count != 5)	/* must have 17's characters and 5's
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
	static char buf[256];
#ifdef HAVE_MVEBU
	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		strcpy(buf, "Marvell Armada 370/XP");
	} else {
		strcpy(buf, "Marvell Armada 385");
	}
	return buf;
#elif HAVE_ALPINE
	strcpy(buf, "Annapurna Labs Alpine");
	return buf;
#elif HAVE_IPQ806X
	strcpy(buf, "QCA IPQ806X");
	return buf;
#elif HAVE_UNIWIP
	strcpy(buf, "FreeScale MPC8314");
	return buf;
#elif HAVE_WDR4900
	strcpy(buf, "FreeScale P1014");
	return buf;
#elif HAVE_RB600
	strcpy(buf, "FreeScale MPC8343");
	return buf;
#elif HAVE_NEWPORT
	strcpy(buf, "Cavium ThunderX CN81XX");
	return buf;
#elif HAVE_VENTANA
	strcpy(buf, "FreeScale i.MX6 Quad/DualLite");
	return buf;
#elif HAVE_NORTHSTAR
	FILE *fp = fopen("/proc/bcm_chipinfo", "rb");
	if (!fp) {
		strcpy(buf, "Broadcom BCM470X");
		return buf;
	}
	int chipid;
	int chiprevision;
	int packageoption;
	fscanf(fp, "%*s %X\n", &chipid);
	fscanf(fp, "%*s %X\n", &chiprevision);
	fscanf(fp, "%*s %X\n", &packageoption);
	fclose(fp);
	if (chipid == 53030 || chipid == 53010 || chipid == 53011 || chipid == 53012 || chipid == 53018 || chipid == 53019) {	// 53030
		if (packageoption == 0)
			strcpy(buf, "Broadcom BCM4709");
		if (packageoption == 1)
			strcpy(buf, "Broadcom BCM4707");
		if (packageoption == 2)
			strcpy(buf, "Broadcom BCM4708");
	} else if (chipid == 53573) {
		strcpy(buf, "Broadcom BCM47189");
	} else
		strcpy(buf, "Broadcom BCM470X");

	return buf;
#else
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
	for (i = 0; i < 256; i++) {
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

#if defined(HAVE_MADWIFI_MIMO) || defined(HAVE_ATH9K)

int isap8x(void)
{
#define CPUSTR "Atheros AR91"
	char *str = cpustring();
	if (str && !strncmp(str, CPUSTR, 12))
		return 1;
	else
		return 0;
#undef CPUSTR

}

#endif

int led_control(int type, int act)
/*
 * type: LED_POWER, LED_DIAG, LED_DMZ, LED_CONNECTED, LED_BRIDGE, LED_VPN,
 * LED_SES, LED_SES2, LED_WLAN0, LED_WLAN1, LED_WLAN2, LED_SEC0, LED_SEC1, USB_POWER, USB_POWER1
 * act: LED_ON, LED_OFF, LED_FLASH 
 * 1st hex number: 1 = inverted, 0 = normal
 */
{
#if (defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_MAGICBOX)  || (defined(HAVE_RB600) && !defined(HAVE_WDR4900)) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_LS5))  && (!defined(HAVE_DIR300) && !defined(HAVE_WRT54G2) && !defined(HAVE_RTG32) && !defined(HAVE_DIR400) && !defined(HAVE_BWRG1000))
	return 0;
#else
	int use_gpio = 0x0ff;
	int gpio_value;
	int enable;
	int disable;

	int power_gpio = 0x0ff;
	int beeper_gpio = 0x0ff;
	int diag_gpio = 0x0ff;
	int diag_gpio_disabled = 0x0ff;
	int dmz_gpio = 0x0ff;
	int connected_gpio = 0x0ff;
	int disconnected_gpio = 0x0ff;
	int bridge_gpio = 0x0ff;
	int vpn_gpio = 0x0ff;
	int ses_gpio = 0x0ff;	// use for SES1 (Linksys), AOSS (Buffalo)
	int ses2_gpio = 0x0ff;
	int wlan_gpio = 0x0ff;	// wlan button led R7000
	int wlan0_gpio = 0x0ff;	// use this only if wlan led is not controlled by hardware!
	int wlan1_gpio = 0x0ff;
	int wlan2_gpio = 0x0ff;
	int usb_gpio = 0x0ff;
	int usb_gpio1 = 0x0ff;
	int sec0_gpio = 0x0ff;	// security leds, wrt600n
	int sec1_gpio = 0x0ff;
	int usb_power = 0x0ff;
	int usb_power1 = 0x0ff;
	int v1func = 0;
	int connblue = nvram_matchi("connblue", 1) ? 1 : 0;

	switch (getRouterBrand())	// gpio definitions here: 0xYZ,
		// Y=0:normal, Y=1:inverted, Z:gpio
		// number (f=disabled)
	{
#ifndef HAVE_BUFFALO
	case ROUTER_BOARD_TECHNAXX3G:
		usb_gpio = 0x109;
		diag_gpio = 0x10c;
		connected_gpio = 0x10b;
		ses_gpio = 0x10c;
		break;
#ifdef HAVE_WPE72
	case ROUTER_BOARD_NS5M:
		diag_gpio = 0x10d;
		break;
#endif
	case ROUTER_BOARD_UNIFI:
		ses_gpio = 0x001;
		sec0_gpio = 0x001;
		break;
	case ROUTER_UBNT_UAPAC:
		ses_gpio = 0x007;
		sec0_gpio = 0x007;
		break;
	case ROUTER_BOARD_AIRROUTER:
		power_gpio = 0x10b;
		diag_gpio = 0x00b;
		connected_gpio = 0x100;
		break;
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		diag_gpio = 0x105;
		ses_gpio = 0x10e;
		sec0_gpio = 0x10e;
		connected_gpio = 0x111;
		disconnected_gpio = 0x112;
		power_gpio = 0x101;
#endif
#ifdef HAVE_SX763
//              diag_gpio = 0x105;
//              ses_gpio = 0x10e;
//              sec0_gpio = 0x10e;
		connected_gpio = 0x1de;
//              disconnected_gpio = 0x112;
//              power_gpio = 0x101;
#endif
		break;
#ifdef HAVE_UNIWIP
	case ROUTER_BOARD_UNIWIP:
		break;
#endif
#ifdef HAVE_WDR4900
	case ROUTER_BOARD_WDR4900:
		diag_gpio = 0x000;
		usb_gpio = 0x001;
		usb_gpio1 = 0x002;
		usb_power = 0x103;
		break;
#endif
#ifdef HAVE_WRT1900AC
	case ROUTER_WRT_1200AC:
	case ROUTER_WRT_1900ACS:

	case ROUTER_WRT_1900ACV2:
		usb_power = 0x032;
	case ROUTER_WRT_1900AC:
		power_gpio = 0x000;
		diag_gpio = 0x100;
		connected_gpio = 0x006;
		disconnected_gpio = 0x007;
//              usb_gpio = 0x004;
//              usb_gpio1 = 0x005;
		ses_gpio = 0x009;
		break;
	case ROUTER_WRT_3200ACM:
//              usb_power = 0x02f;
		power_gpio = 0x000;
		diag_gpio = 0x100;
		connected_gpio = 0x006;
		disconnected_gpio = 0x007;
//              usb_gpio = 0x004;
//              usb_gpio1 = 0x005;
		ses_gpio = 0x009;
		break;
#endif
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WA901
		diag_gpio = 0x102;
		ses_gpio = 0x004;
//              usb_gpio = 0x101;
#elif  HAVE_WR941
		diag_gpio = 0x102;
		ses_gpio = 0x005;
//              usb_gpio = 0x101;
#endif
#ifdef HAVE_MR3020
		connected_gpio = 0x11b;
		diag_gpio = 0x11a;
		usb_power = 0x008;
#elif HAVE_GL150
//              power_gpio = 0x11b;
//              diag_gpio = 0x01b;
//              usb_power = 0x008;
#elif HAVE_WR710
		power_gpio = 0x11b;
		diag_gpio = 0x01b;
#elif HAVE_WA701V2
		diag_gpio = 0x11b;
		ses_gpio = 0x001;
		sec0_gpio = 0x001;
#elif HAVE_WR703
		diag_gpio = 0x11b;
		ses_gpio = 0x001;
		sec0_gpio = 0x001;
		usb_power = 0x008;
#elif HAVE_WR842
		diag_gpio = 0x101;
		ses_gpio = 0x000;
		usb_power = 0x006;

#elif HAVE_WR741V4
		diag_gpio = 0x11b;
		ses_gpio = 0x001;
		sec0_gpio = 0x001;

#elif HAVE_MR3420
		diag_gpio = 0x101;
		connected_gpio = 0x108;
		usb_power = 0x006;
#elif HAVE_WR741
		diag_gpio = 0x101;
		ses_gpio = 0x000;
//              usb_gpio = 0x101;
#endif
#ifdef HAVE_WR1043
		diag_gpio = 0x102;
		ses_gpio = 0x005;
//              usb_gpio = 0x101;
#endif
#ifdef HAVE_WRT160NL
		power_gpio = 0x10e;
		connected_gpio = 0x109;
		ses_gpio = 0x108;
#endif
#ifdef HAVE_TG2521
		ses_gpio = 0x103;
		diag_gpio = 0x103;
		usb_power = 0x105;
#endif
#ifdef HAVE_TEW632BRP
		diag_gpio = 0x101;
		ses_gpio = 0x103;
#endif
#ifdef HAVE_WP543
		diag_gpio = 0x107;
		connected_gpio = 0x106;
#endif
#ifdef HAVE_WP546
		beeper_gpio = 0x001;
		diag_gpio = 0x107;
		connected_gpio = 0x106;
#endif
#ifdef HAVE_DIR825
		power_gpio = 0x102;
		diag_gpio = 0x101;
		connected_gpio = 0x10b;
		disconnected_gpio = 0x106;
		ses_gpio = 0x104;
		usb_gpio = 0x100;
//              wlan0_gpio = 0x0ff; //correct states missing
#endif
#ifdef HAVE_WNDR3700
		power_gpio = 0x102;
		diag_gpio = 0x101;
		connected_gpio = 0x106;
		ses_gpio = 0x104;
#endif
#ifdef HAVE_WZRG300NH
		diag_gpio = 0x101;
		connected_gpio = 0x112;
		ses_gpio = 0x111;
		sec0_gpio = 0x111;
#endif
#ifdef HAVE_DIR632
		power_gpio = 0x001;
		diag_gpio = 0x100;
		connected_gpio = 0x111;
		usb_gpio = 0x10b;
#endif
#ifdef HAVE_WZRG450
		diag_gpio = 0x10e;
		ses_gpio = 0x10d;
		sec0_gpio = 0x10d;
		usb_power = 0x010;
		connected_gpio = 0x12e;	// card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		diag_gpio = 0x110;
		ses_gpio = 0x126;	// card 1, gpio 6
		sec0_gpio = 0x126;
		usb_power = 0x00d;
		connected_gpio = 0x127;	// card 1, gpio 7
#endif
#ifdef HAVE_WZRHPAG300NH
		diag_gpio = 0x101;
		connected_gpio = 0x133;	// card 2 gpio 3
		sec0_gpio = 0x125;
		sec1_gpio = 0x131;
		ses_gpio = 0x125;	// card 1 gpio 5
		ses2_gpio = 0x131;	// card 2 gpio 5
		usb_power = 0x002;
#endif
#ifdef HAVE_DIR615C1
		power_gpio = 0x104;
		wlan0_gpio = 0x10f;
#endif
#ifdef HAVE_DIR615E
		power_gpio = 0x006;
		diag_gpio = 0x001;
		connected_gpio = 0x111;
		disconnected_gpio = 0x007;
		ses_gpio = 0x100;
#endif
#ifdef HAVE_DAP2230
		diag_gpio = 0x00b;
		power_gpio = 0x10b;
#elif HAVE_WR941V6
		disconnected_gpio = 0x00f;
		power_gpio = 0x112;
		diag_gpio = 0x012;

#elif HAVE_WR841V12
		power_gpio = 0x101;
		diag_gpio = 0x001;
		ses_gpio = 0x103;
		sec0_gpio = 0x103;
		connected_gpio = 0x102;
#elif HAVE_WR841V11
		power_gpio = 0x101;
		diag_gpio = 0x001;
		ses_gpio = 0x103;
		sec0_gpio = 0x103;
		connected_gpio = 0x102;
#elif HAVE_WR841V9
		diag_gpio = 0x103;
#elif HAVE_WR842V2
		connected_gpio = 0x10e;
		usb_power = 0x204;
		usb_gpio = 0x10f;
#elif HAVE_WR810N
		diag_gpio = 0x10d;
		usb_power = 0x00b;
#elif HAVE_WR841V8
		diag_gpio = 0x10f;
		connected_gpio = 0x10e;
#elif HAVE_DIR615I
		power_gpio = 0x00e;
		diag_gpio = 0x10f;
		connected_gpio = 0x10c;
		disconnected_gpio = 0x016;
#endif
#ifdef HAVE_WRT400
		power_gpio = 0x001;
		diag_gpio = 0x105;
		ses_gpio = 0x104;
		connected_gpio = 0x007;
#endif
#ifdef HAVE_ALFAAP94
		power_gpio = 0x005;
#endif
		break;
	case ROUTER_ALLNET01:
		connected_gpio = 0x100;
		break;
	case ROUTER_BOARD_WP54G:
		diag_gpio = 0x102;
		connected_gpio = 0x107;
		break;
	case ROUTER_BOARD_NP28G:
		diag_gpio = 0x102;
		connected_gpio = 0x106;
		break;
	case ROUTER_BOARD_GATEWORX_GW2369:
		connected_gpio = 0x102;
		break;
	case ROUTER_BOARD_GW2388:
	case ROUTER_BOARD_GW2380:
#ifdef HAVE_NEWPORT

#elif defined(HAVE_VENTANA)
		power_gpio = 0x166;
		diag_gpio = 0x06F;
		connected_gpio = 0x066;
		disconnected_gpio = 0x067;
#else
		connected_gpio = 0x110;	// 16 is mapped to front led
#endif
		break;
	case ROUTER_BOARD_GATEWORX:
#ifdef HAVE_WG302V1
		diag_gpio = 0x104;
		wlan0_gpio = 0x105;
#elif HAVE_WG302
		diag_gpio = 0x102;
		wlan0_gpio = 0x104;
#else
		if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350"))
			connected_gpio = 0x105;
		else if (nvram_match("DD_BOARD", "Gateworks Cambria GW2358-4"))
			connected_gpio = 0x118;
		else
			connected_gpio = 0x003;
#endif
		break;
	case ROUTER_BOARD_GATEWORX_SWAP:
		connected_gpio = 0x004;
		break;
	case ROUTER_BOARD_STORM:
		connected_gpio = 0x005;
		diag_gpio = 0x003;
		break;
	case ROUTER_LINKSYS_WRH54G:
		diag_gpio = 0x101;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G:
	case ROUTER_WRT54G_V8:
		power_gpio = 0x001;
		dmz_gpio = 0x107;
		connected_gpio = 0x103;	// ses orange
		ses_gpio = 0x102;	// ses white
		ses2_gpio = 0x103;	// ses orange
		break;
	case ROUTER_WRT54G_V81:
		power_gpio = 0x101;
		dmz_gpio = 0x102;
		connected_gpio = 0x104;	// ses orange
		ses_gpio = 0x103;	// ses white
		ses2_gpio = 0x104;	// ses orange
		break;
	case ROUTER_WRT54G1X:
		connected_gpio = 0x103;
		v1func = 1;
		break;
	case ROUTER_WRT350N:
		connected_gpio = 0x103;
		power_gpio = 0x001;
		ses2_gpio = 0x103;	// ses orange
		sec0_gpio = 0x109;
		usb_gpio = 0x10b;
		break;
	case ROUTER_WRT600N:
		power_gpio = 0x102;
		diag_gpio = 0x002;
		usb_gpio = 0x103;
		sec0_gpio = 0x109;
		sec1_gpio = 0x10b;
		break;
	case ROUTER_LINKSYS_WRT55AG:
		connected_gpio = 0x103;
		break;
	case ROUTER_DLINK_DIR330:
		diag_gpio = 0x106;
		connected_gpio = 0x100;
		usb_gpio = 0x104;
		break;
	case ROUTER_ASUS_RTN10PLUS:
//              diag_gpio = 0x10d;
//              connected_gpio = 0x108;
//              power_gpio = 0x109;
		break;
	case ROUTER_BOARD_DIR600B:
		diag_gpio = 0x10d;
		connected_gpio = 0x108;
		power_gpio = 0x109;
		break;
	case ROUTER_BOARD_DIR615D:
#ifdef HAVE_DIR615H
		diag_gpio = 0x007;
		connected_gpio = 0x10d;
		disconnected_gpio = 0x10c;
		ses_gpio = 0x10e;
		power_gpio = 0x009;
#else
		diag_gpio = 0x108;
		connected_gpio = 0x10c;
		disconnected_gpio = 0x10e;
		ses_gpio = 0x10b;
		power_gpio = 0x109;
#endif
		break;
		/*
		   DIR 882 
		   power LED red diag = 8 inv, green 16 inv

		 */
	case ROUTER_BOARD_W502U:
		connected_gpio = 0x10d;
		break;
	case ROUTER_BOARD_OPENRISC:
#ifndef HAVE_ERC
// ERC: diag button is used different / wlan button is handled by a script
		diag_gpio = 0x003;
		ses_gpio = 0x005;
#endif
		break;
	case ROUTER_BOARD_WR5422:
		ses_gpio = 0x10d;
		break;
	case ROUTER_BOARD_F5D8235:
		usb_gpio = 0x117;
		diag_gpio = 0x109;
		disconnected_gpio = 0x106;
		connected_gpio = 0x105;
		ses_gpio = 0x10c;
		break;
#else
	case ROUTER_BOARD_DANUBE:
#ifdef HAVE_WMBR_G300NH
		diag_gpio = 0x105;
		ses_gpio = 0x10e;
		sec0_gpio = 0x10e;
		connected_gpio = 0x111;
		disconnected_gpio = 0x112;
		power_gpio = 0x101;
#endif
		break;
	case ROUTER_BOARD_PB42:
#ifdef HAVE_WZRG300NH
		diag_gpio = 0x101;
		connected_gpio = 0x112;
		ses_gpio = 0x111;
		sec0_gpio = 0x111;
#endif
#ifdef HAVE_WZRHPAG300NH
		diag_gpio = 0x101;
		connected_gpio = 0x133;
		ses_gpio = 0x125;
		ses2_gpio = 0x131;
		sec0_gpio = 0x125;
		sec1_gpio = 0x131;
		usb_power = 0x002;
#endif
#ifdef HAVE_WZRG450
		diag_gpio = 0x10e;
		ses_gpio = 0x10d;
		sec0_gpio = 0x10d;
		usb_power = 0x010;
		connected_gpio = 0x12e;	// card 1, gpio 14
#endif
#ifdef HAVE_WZRG300NH2
		diag_gpio = 0x110;
		ses_gpio = 0x126;
		sec0_gpio = 0x126;
		usb_power = 0x00d;
		connected_gpio = 0x127;
#endif
		break;
#endif
	case ROUTER_BOARD_HAMEA15:
		diag_gpio = 0x111;
		connected_gpio = 0x114;
//              ses_gpio = 0x10e;
		break;
	case ROUTER_BOARD_WCRGN:
		diag_gpio = 0x107;
		connected_gpio = 0x10b;
//              ses_gpio = 0x10e;
		break;
	case ROUTER_DIR882:
		connected_gpio = 0x103;
		disconnected_gpio = 0x104;
		diag_gpio = 0x108;
		power_gpio = 0x110;
		usb_gpio = 0x10c;
		usb_gpio1 = 0x10e;
		break;
	case ROUTER_DIR860LB1:
		power_gpio = 0x10f;
		diag_gpio = 0x10d;
		diag_gpio_disabled = 0x10f;
		disconnected_gpio = 0x10e;
		connected_gpio = 0x110;
		break;
	case ROUTER_DIR810L:
		power_gpio = 0x009;
		diag_gpio = 0x00d;
		diag_gpio_disabled = 0x009;
		connected_gpio = 0x128;
		disconnected_gpio = 0x00c;
		break;
	case ROUTER_WHR300HP2:
		power_gpio = 0x109;
		diag_gpio = 0x107;
		diag_gpio_disabled = 0x109;
		wlan0_gpio = 0x108;
		sec0_gpio = 0x10a;
		ses_gpio = 0x10a;
		connected_gpio = 0x139;
		disconnected_gpio = 0x13b;
		break;
	case ROUTER_BOARD_WHRG300N:
		diag_gpio = 0x107;
		connected_gpio = 0x109;
		ses_gpio = 0x10e;
		break;
#ifdef HAVE_WNR2200
	case ROUTER_BOARD_WHRHPGN:
		power_gpio = 0x122;
		diag_gpio = 0x121;
		connected_gpio = 0x107;
		usb_power = 0x024;	// enable usb port 
		ses_gpio = 0x105;	//correct state missing
		usb_gpio = 0x108;
//              sec0_gpio = 0x104;
		break;
#elif HAVE_WNR2000
	case ROUTER_BOARD_WHRHPGN:
		power_gpio = 0x123;
		diag_gpio = 0x122;
		connected_gpio = 0x100;
//              ses_gpio = 0x104;
//              sec0_gpio = 0x104;
		break;
#elif HAVE_WLAEAG300N
	case ROUTER_BOARD_WHRHPGN:
		power_gpio = 0x110;
		diag_gpio = 0x111;
		connected_gpio = 0x106;
		ses_gpio = 0x10e;
		sec0_gpio = 0x10e;
		break;
#elif HAVE_CARAMBOLA
#ifdef HAVE_ERC
	case ROUTER_BOARD_WHRHPGN:
		vpn_gpio = 0x11B;
		wlan0_gpio = 0x000;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
//              usb_power = 0x01a;
//              usb_gpio = 0x001;
//              ses_gpio = 0x11b;
		break;
#endif
#elif HAVE_HORNET
	case ROUTER_BOARD_WHRHPGN:
		usb_power = 0x01a;
		usb_gpio = 0x001;
		ses_gpio = 0x11b;
		break;
#elif HAVE_RB2011
	case ROUTER_BOARD_WHRHPGN:
//              diag_gpio = 0x10f;
//              connected_gpio = 0x112;
//              disconnected_gpio = 0x113;
//              power_gpio = 0x10e;
//              usb_power = 0x01a;
//              usb_gpio = 0x10b;
//              ses_gpio = 0x11b;
		break;
#elif HAVE_WDR3500
	case ROUTER_BOARD_WHRHPGN:
		usb_gpio = 0x10b;
		usb_power = 0x00f;
		diag_gpio = 0x10e;
		connected_gpio = 0x10f;
		break;
#elif HAVE_WDR4300
	case ROUTER_BOARD_WHRHPGN:
		usb_gpio = 0x10b;
		usb_gpio1 = 0x10c;
		usb_power = 0x015;
		usb_power1 = 0x016;
		diag_gpio = 0x10e;
		connected_gpio = 0x10f;
		break;
#elif HAVE_WNDR3700V4
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x102;
		power_gpio = 0x100;
		connected_gpio = 0x101;
		disconnected_gpio = 0x103;
		usb_power = 0x020;
		usb_gpio = 0x10d;
		ses_gpio = 0x110;
		break;
#elif HAVE_DAP3662
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x10e;	// red
		diag_gpio_disabled = 0x117;	//
		power_gpio = 0x117;	// green
		break;
#elif HAVE_DIR862
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x10e;	// orange
		diag_gpio_disabled = 0x113;	// 
		power_gpio = 0x113;	// green
		connected_gpio = 0x116;	// green
		disconnected_gpio = 0x117;	// orange
		break;
#elif HAVE_CPE880
	case ROUTER_BOARD_WHRHPGN:
		connected_gpio = 0x10c;
		break;
#elif HAVE_MMS344
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x10e;
		break;
#elif HAVE_ARCHERC7V4
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x006;
		connected_gpio = 0x11a;
		disconnected_gpio = 0x119;
		ses_gpio = 0x11f;
		sec0_gpio = 0x11f;

//              usb_power = 0x016;
		usb_gpio = 0x107;

//              usb_power1 = 0x015;
		usb_gpio1 = 0x108;

		break;
#elif HAVE_ARCHERC7
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x010e;
		ses_gpio = 0x10f;
		sec0_gpio = 0x10f;

		usb_power = 0x016;
		usb_gpio = 0x112;

		usb_power1 = 0x015;
		usb_gpio1 = 0x113;

		break;
#elif HAVE_WR1043V2
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x113;
//              connected_gpio = 0x112;
//              disconnected_gpio = 0x113;
//              power_gpio = 0x10e;
		usb_power = 0x015;
		usb_gpio = 0x10f;
		ses_gpio = 0x112;
		sec0_gpio = 0x112;
		break;
#elif HAVE_WZR450HP2
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x114;
//              connected_gpio = 0x112;
//              disconnected_gpio = 0x113;
//              power_gpio = 0x10e;
//              usb_power = 0x01a;
//              usb_gpio = 0x10b;

		connected_gpio = 0x10d;
		power_gpio = 0x113;
		ses_gpio = 0x103;
		sec0_gpio = 0x103;
		break;
#elif HAVE_DHP1565
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x10e;
		diag_gpio_disabled = 0x116;
		connected_gpio = 0x112;
		disconnected_gpio = 0x113;
		power_gpio = 0x116;
		usb_gpio = 0x10b;
		ses_gpio = 0x10f;
		break;
#elif HAVE_E325N
	case ROUTER_BOARD_WHRHPGN:
		connected_gpio = 0x003;
		disconnected_gpio = 0x002;
		break;
#elif defined(HAVE_SR3200) || defined(HAVE_CPE890)
	case ROUTER_BOARD_WHRHPGN:
		power_gpio = 0x101;
		diag_gpio = 0x001;
		break;
#elif HAVE_XD3200
	case ROUTER_BOARD_WHRHPGN:
		break;
#elif HAVE_E380AC
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x003;
		break;
#elif HAVE_WR615N
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x101;
		connected_gpio = 0x102;
		disconnected_gpio = 0x103;
		ses_gpio = 0x10c;
		sec0_gpio = 0x10c;
		break;
#elif HAVE_E355AC
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x002;
		break;
#elif HAVE_WR650AC
	case ROUTER_BOARD_WHRHPGN:
		ses_gpio = 0x114;
		sec0_gpio = 0x114;
		connected_gpio = 0x104;
		diag_gpio = 0x004;
		break;
#elif HAVE_DIR869
	case ROUTER_BOARD_WHRHPGN:
		disconnected_gpio = 0x10f;
		connected_gpio = 0x110;
		diag_gpio = 0x00f;
		break;
#elif HAVE_DIR859
	case ROUTER_BOARD_WHRHPGN:
		power_gpio = 0x10f;
		connected_gpio = 0x110;
		diag_gpio = 0x00f;
		break;
#elif HAVE_JWAP606
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x10b;
		connected_gpio = 0x10d;
		disconnected_gpio = 0x10d;
		power_gpio = 0x10b;
//              usb_power = 0x01a;
//              usb_gpio = 0x10b;
//              ses_gpio = 0x11b;
		break;
#elif HAVE_DIR825C1
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x10f;
		connected_gpio = 0x112;
		disconnected_gpio = 0x113;
		power_gpio = 0x10e;
//              usb_power = 0x01a;
		usb_gpio = 0x10b;
//              ses_gpio = 0x11b;
		break;
#elif HAVE_WDR2543
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x100;
		usb_gpio = 0x108;
		break;
#elif HAVE_WASP
	case ROUTER_BOARD_WHRHPGN:
//              usb_power = 0x01a;
//              usb_gpio = 0x001;
//              ses_gpio = 0x11b;
		break;
#else
	case ROUTER_BOARD_WHRHPGN:
		diag_gpio = 0x101;
		connected_gpio = 0x106;
		ses_gpio = 0x100;
		sec0_gpio = 0x100;
		break;
#endif
	case ROUTER_BUFFALO_WBR54G:
		diag_gpio = 0x107;
		break;
	case ROUTER_BUFFALO_WBR2G54S:
		diag_gpio = 0x001;
		ses_gpio = 0x006;
		break;
	case ROUTER_BUFFALO_WLA2G54C:
		diag_gpio = 0x104;
		ses_gpio = 0x103;
		break;
	case ROUTER_BUFFALO_WLAH_G54:
		diag_gpio = 0x107;
		ses_gpio = 0x106;
		break;
	case ROUTER_BUFFALO_WAPM_HP_AM54G54:
		diag_gpio = 0x107;
		ses_gpio = 0x101;
		break;
	case ROUTER_BOARD_WHRAG108:
		diag_gpio = 0x107;
		bridge_gpio = 0x104;
		ses_gpio = 0x100;
		break;
	case ROUTER_BUFFALO_WHRG54S:
	case ROUTER_BUFFALO_WLI_TX4_G54HP:
		diag_gpio = 0x107;
		if (nvram_match("DD_BOARD", "Buffalo WHR-G125")) {
			connected_gpio = 0x101;
			sec0_gpio = 0x106;
		} else {
			bridge_gpio = 0x101;
			ses_gpio = 0x106;
		}
		break;
	case ROUTER_UBNT_UNIFIAC:
		power_gpio = 0x00e;
		diag_gpio = 0x00f;
		break;
	case ROUTER_D1800H:
		usb_gpio = 0x101;
		usb_power = 0x007;
		power_gpio = 0x002;
		diag_gpio = 0x00d;
		diag_gpio_disabled = 0x002;
		connected_gpio = 0x10f;
		disconnected_gpio = 0x10e;
		break;
	case ROUTER_BUFFALO_WZRRSG54:
		diag_gpio = 0x107;
		vpn_gpio = 0x101;
		ses_gpio = 0x106;
		break;
	case ROUTER_BUFFALO_WZRG300N:
		diag_gpio = 0x107;
		bridge_gpio = 0x101;
		break;
	case ROUTER_BUFFALO_WZRG144NH:
		diag_gpio = 0x103;
		bridge_gpio = 0x101;
		ses_gpio = 0x102;
		break;
	case ROUTER_BUFFALO_WZR900DHP:
	case ROUTER_BUFFALO_WZR600DHP2:
//              usb_power = 0x009;      // USB 2.0 ehci port
		usb_power1 = 0x10a;	// USB 3.0 xhci port
//              wlan0_gpio = 0x028; // wireless orange
//              wlan1_gpio = 0x029; // wireless blue
		connected_gpio = 0x02a;	// connected blue
		sec0_gpio = 0x02b;
		sec1_gpio = 0x02c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		power_gpio = 0x02e;
		diag_gpio = 0x02d;
		diag_gpio_disabled = 0x02e;
		usb_gpio = 0x02f;
		break;

	case ROUTER_BUFFALO_WXR1900DHP:
		usb_power = 0x00d;	// USB 2.0 ehci port
		usb_power1 = 0x00e;	// USB 3.0 xhci port
//              wlan0_gpio = 0x028; // wireless orange
//              wlan1_gpio = 0x029; // wireless blue
		connected_gpio = 0x009;	// connected blue
		disconnected_gpio = 0x00a;	// connected blue
		sec0_gpio = 0x00b;
		sec1_gpio = 0x00b;
		// 0x2b strange led orange
		// 0x2c strange led blue
		power_gpio = 0x006;
		diag_gpio = 0x005;
		diag_gpio_disabled = 0x006;
		break;

	case ROUTER_BUFFALO_WZR1750:
		usb_power = 0x009;	// USB 2.0 ehci port
		usb_power1 = 0x10a;	// USB 3.0 xhci port
//              wlan0_gpio = 0x028; // wireless orange
//              wlan1_gpio = 0x029; // wireless blue
		connected_gpio = 0x02a;	// connected blue
		sec0_gpio = 0x02b;
		sec1_gpio = 0x02c;
		// 0x2b strange led orange
		// 0x2c strange led blue
		power_gpio = 0x02d;
		diag_gpio = 0x02e;
		diag_gpio_disabled = 0x02d;
		usb_gpio = 0x02f;
		break;
#ifndef HAVE_BUFFALO
#ifdef HAVE_DIR300
	case ROUTER_BOARD_FONERA:
		diag_gpio = 0x003;
		bridge_gpio = 0x004;
		ses_gpio = 0x001;
		break;
#endif
#ifdef HAVE_WRT54G2
	case ROUTER_BOARD_FONERA:
		bridge_gpio = 0x004;
		ses_gpio = 0x104;
		diag_gpio = 0x103;
		break;
#endif
#ifdef HAVE_RTG32
	case ROUTER_BOARD_FONERA:
		break;
#endif
#ifdef HAVE_BWRG1000
	case ROUTER_BOARD_LS2:
		diag_gpio = 0x007;
		break;
#endif
#ifdef HAVE_DIR400
	case ROUTER_BOARD_FONERA2200:
		diag_gpio = 0x003;
		bridge_gpio = 0x004;
		ses_gpio = 0x001;
		break;
#endif
#ifdef HAVE_WRK54G
	case ROUTER_BOARD_FONERA:
		diag_gpio = 0x107;
		dmz_gpio = 0x005;
		break;
#endif
	case ROUTER_BOARD_TW6600:
		diag_gpio = 0x107;
		bridge_gpio = 0x104;
		ses_gpio = 0x100;
		break;
	case ROUTER_MOTOROLA:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_RT210W:
		power_gpio = 0x105;
		diag_gpio = 0x005;	// power led blink / off to indicate factory
		// defaults
		connected_gpio = 0x100;
		wlan0_gpio = 0x103;
		break;
	case ROUTER_RT480W:
	case ROUTER_BELKIN_F5D7230_V2000:
	case ROUTER_BELKIN_F5D7231:
		power_gpio = 0x105;
		diag_gpio = 0x005;	// power led blink / off to indicate factory
		// defaults
		connected_gpio = 0x100;
		break;
	case ROUTER_MICROSOFT_MN700:
		power_gpio = 0x006;
		diag_gpio = 0x106;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500GD:
	case ROUTER_ASUS_WL520GUGC:
		diag_gpio = 0x000;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500G_PRE:
	case ROUTER_ASUS_WL700GE:
		power_gpio = 0x101;
		diag_gpio = 0x001;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL550GE:
		power_gpio = 0x102;
		diag_gpio = 0x002;	// power led blink / off to indicate factory
		// defaults
		break;
	case ROUTER_WRT54G3G:
	case ROUTER_WRTSL54GS:
		power_gpio = 0x001;
		dmz_gpio = 0x100;
		connected_gpio = 0x107;	// ses orange
		ses_gpio = 0x105;	// ses white
		ses2_gpio = 0x107;	// ses orange 
		break;
	case ROUTER_MOTOROLA_WE800G:
	case ROUTER_MOTOROLA_V1:
		diag_gpio = 0x103;
		wlan0_gpio = 0x101;
		bridge_gpio = 0x105;
		break;
	case ROUTER_DELL_TRUEMOBILE_2300:
	case ROUTER_DELL_TRUEMOBILE_2300_V2:
		power_gpio = 0x107;
		diag_gpio = 0x007;	// power led blink / off to indicate factory
		// defaults
		wlan0_gpio = 0x106;
		break;
	case ROUTER_NETGEAR_WNR834B:
		power_gpio = 0x104;
		diag_gpio = 0x105;
		wlan0_gpio = 0x106;
		break;
	case ROUTER_SITECOM_WL105B:
		power_gpio = 0x003;
		diag_gpio = 0x103;	// power led blink / off to indicate factory
		// defaults
		wlan0_gpio = 0x104;
		break;
	case ROUTER_WRT300N:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT150N:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		sec0_gpio = 0x105;
		break;
	case ROUTER_WRT300NV11:
		ses_gpio = 0x105;
		sec0_gpio = 0x103;
		// diag_gpio = 0x11; //power led blink / off to indicate fac.def.
		break;
	case ROUTER_WRT310N:
		connected_gpio = 0x103;	//ses orange
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		ses_gpio = 0x109;	// ses blue
		break;
	case ROUTER_WRT310NV2:
		connected_gpio = 0x102;	// ses orange
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate fac.def.
		ses_gpio = 0x104;	// ses blue
		break;
	case ROUTER_WRT160N:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate fac.def. 
		connected_gpio = 0x103;	// ses orange
		ses_gpio = 0x105;	// ses blue
		break;
	case ROUTER_WRT160NV3:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink / off to indicate fac.def. 
		connected_gpio = 0x102;	// ses orange
		ses_gpio = 0x104;	// ses blue
		break;
	case ROUTER_LINKSYS_E800:
	case ROUTER_LINKSYS_E900:
	case ROUTER_LINKSYS_E1500:
	case ROUTER_LINKSYS_E1550:
		power_gpio = 0x106;
		diag_gpio = 0x006;	// power led blink / off to indicate fac.def.
		ses_gpio = 0x108;	// ses blue
		break;
	case ROUTER_LINKSYS_E1000V2:
		power_gpio = 0x106;
		diag_gpio = 0x006;	// power led blink / off to indicate fac.def. 
		connected_gpio = 0x007;	// ses orange
		ses_gpio = 0x008;	// ses blue
		break;
	case ROUTER_LINKSYS_E2500:
		power_gpio = 0x106;
		diag_gpio = 0x006;	// power led blink / off to indicate fac.def.
		break;
	case ROUTER_LINKSYS_E3200:
		power_gpio = 0x103;
		diag_gpio = 0x003;	// power led blink / off to indicate fac.def. 
		break;
	case ROUTER_LINKSYS_E4200:
		power_gpio = 0x105;	// white LED1
		diag_gpio = 0x103;	// power led blink / off to indicate fac.def. 
//              connected_gpio = 0x103; // white LED2
		break;
	case ROUTER_LINKSYS_EA6500:
		diag_gpio = 0x101;	// white led blink / off to indicate fac.def. 
		break;
	case ROUTER_LINKSYS_EA6500V2:
	case ROUTER_LINKSYS_EA6700:
	case ROUTER_LINKSYS_EA6400:
	case ROUTER_LINKSYS_EA6350:
	case ROUTER_LINKSYS_EA6900:
		usb_power = 0x009;	//usb power on/off
		usb_power1 = 0x00a;	//usb power on/off
		diag_gpio = 0x106;	// white led blink / off to indicate fac.def. 
		connected_gpio = 0x008;
		break;
	case ROUTER_LINKSYS_EA8500:
		power_gpio = 0x100;	// power led 
		diag_gpio = 0x000;	// power led orange     
		wlan0_gpio = 0x001;	// radio 0  
		ses_gpio = 0x102;	// wps led
		break;
	case ROUTER_ASUS_WL500G:
		power_gpio = 0x100;
		diag_gpio = 0x000;	// power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_ASUS_WL500W:
		power_gpio = 0x105;
		diag_gpio = 0x005;	// power led blink /off to indicate factory
		// defaults
		break;
	case ROUTER_LINKSYS_WTR54GS:
		diag_gpio = 0x001;
		break;
	case ROUTER_WAP54G_V1:
		diag_gpio = 0x103;
		wlan0_gpio = 0x104;	// LINK led
		break;
	case ROUTER_WAP54G_V3:
		ses_gpio = 0x10c;
		connected_gpio = 0x006;
		break;
	case ROUTER_NETGEAR_WNR834BV2:
		power_gpio = 0x002;
		diag_gpio = 0x003;	// power led amber 
		connected_gpio = 0x007;	// WAN led green 
		break;
	case ROUTER_NETGEAR_WNDR3300:
		power_gpio = 0x005;
		diag_gpio = 0x105;	// power led blink /off to indicate factory defaults
		connected_gpio = 0x007;	// WAN led green 
		break;
	case ROUTER_ASKEY_RT220XD:
		wlan0_gpio = 0x100;
		dmz_gpio = 0x101;	// not soldered 
		break;
	case ROUTER_WRT610N:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power led blink /off to indicate factory defaults
		connected_gpio = 0x103;	// ses amber
		ses_gpio = 0x109;	// ses blue
		usb_gpio = 0x100;
		break;
	case ROUTER_WRT610NV2:
		power_gpio = 0x005;
		diag_gpio = 0x105;	// power led blink
		connected_gpio = 0x100;	// ses amber
		ses_gpio = 0x103;	// ses blue
		usb_gpio = 0x007;
		break;
	case ROUTER_USR_5461:
		usb_gpio = 0x001;
		break;
	case ROUTER_USR_5465:
		//usb_gpio = 0x002; //or 0x001 ??
		break;
	case ROUTER_NETGEAR_WGR614L:
	case ROUTER_NETGEAR_WGR614V9:
		// power_gpio = 0x107;       // don't use - resets router
		diag_gpio = 0x006;
		connected_gpio = 0x104;
		break;
	case ROUTER_NETGEAR_WG602_V4:
		power_gpio = 0x101;	// trick: make lan led green for 100Mbps
		break;
	case ROUTER_BELKIN_F5D7231_V2000:
		connected_gpio = 0x104;
		diag_gpio = 0x001;	// power led blink /off to indicate factory defaults
		break;
	case ROUTER_NETGEAR_WNR3500L:
	case ROUTER_NETGEAR_WNR3500LV2:
		power_gpio = 0x003;	// power led green
		diag_gpio = 0x007;	// power led amber
		ses_gpio = 0x001;	// WPS led green
		connected_gpio = 0x002;	// wan led green
		wlan1_gpio = 0x000;	// radio 1 blue led
		usb_gpio = 0x014;	// usb power
		break;
	case ROUTER_NETGEAR_WNDR3400:
		power_gpio = 0x003;	//power led green
		diag_gpio = 0x007;	// power led amber
		connected_gpio = 0x001;	//wan led green
		usb_gpio = 0x102;	//usb led green
		wlan1_gpio = 0x000;	// radio 1 led blue
		break;
	case ROUTER_NETGEAR_WNDR4000:
		power_gpio = 0x000;	//power led green
		diag_gpio = 0x001;	// power led amber
		connected_gpio = 0x002;	//wan led green
		wlan0_gpio = 0x003;	//radio 0 led green
		wlan1_gpio = 0x004;	// radio 1 led blue
		usb_gpio = 0x005;	//usb led green
		ses_gpio = 0x106;	// WPS led green - inverse
		ses2_gpio = 0x107;	// WLAN led green - inverse
		break;
	case ROUTER_DLINK_DIR860:
		usb_power = 0x00a;
		connected_gpio = 0x104;
		disconnected_gpio = 0x103;
		power_gpio = 0x101;
		diag_gpio = 0x100;
		diag_gpio_disabled = 0x101;
		break;
	case ROUTER_DLINK_DIR868:
	case ROUTER_DLINK_DIR868C:
		usb_power = 0x00a;
		connected_gpio = 0x103;
		disconnected_gpio = 0x101;
		power_gpio = 0x102;
		diag_gpio = 0x100;
		break;

	case ROUTER_DLINK_DIR880:
		connected_gpio = 0x103;
		disconnected_gpio = 0x101;
		power_gpio = 0x102;
		diag_gpio = 0x100;
		diag_gpio_disabled = 0x102;
		usb_gpio = 0x108;
		usb_gpio1 = 0x10f;
//              wlan0_gpio = 0x10d;
//              wlan1_gpio = 0x10e;
		usb_power = 0x009;
		usb_power1 = 0x00a;
		break;
	case ROUTER_DLINK_DIR885:
		usb_power = 0x012;
		usb_gpio = 0x108;
		power_gpio = 0x100;
		diag_gpio = 0x102;
		diag_gpio_disabled = 0x100;
		disconnected_gpio = 0x103;
		connected_gpio = 0x101;
		wlan0_gpio = 0x10d;
		wlan1_gpio = 0x10e;
		break;
	case ROUTER_DLINK_DIR895:
		usb_power = 0x015;
		usb_power1 = 0x012;
		usb_gpio = 0x108;
		usb_gpio1 = 0x10f;
		power_gpio = 0x100;
		diag_gpio = 0x102;
		diag_gpio_disabled = 0x100;
		disconnected_gpio = 0x103;
		connected_gpio = 0x101;
		wlan0_gpio = 0x10d;
		wlan1_gpio = 0x10e;
		break;
	case ROUTER_DLINK_DIR890:
		usb_power = 0x015;
		usb_power1 = 0x012;
		usb_gpio = 0x108;
		usb_gpio1 = 0x10f;
		connected_gpio = 0x101;
		disconnected_gpio = 0x103;
		power_gpio = 0x102;
		diag_gpio = 0x002;
		break;
	case ROUTER_TRENDNET_TEW828:
		usb_gpio = 0x104;
		power_gpio = 0x106;
		diag_gpio = 0x006;
		break;
	case ROUTER_TRENDNET_TEW812:
		// gpio !1 = 2.4 ghz led
		// gpio !2 = 5 ghz led
		// gpio !3 = power somthing
		// gpio !8 = usb led
		// 
		usb_gpio = 0x108;
		diag_gpio = 0x103;
		wlan0_gpio = 0x101;
		wlan1_gpio = 0x102;
		break;
	case ROUTER_ASUS_RTN18U:
		power_gpio = 0x100;
//              usb_power = 0x00d;      //usb power on/off
		if (nvram_match("bl_version", "3.0.0.7")) {
			usb_gpio = 0x10e;
			connected_gpio = 0x103;
			disconnected_gpio = 0x106;
		} else if (nvram_match("bl_version", "1.0.0.0")) {
			usb_gpio = 0x103;
			connected_gpio = 0x106;
			disconnected_gpio = 0x109;
		} else {
			usb_gpio = 0x103;
			usb_gpio1 = 0x10e;
			connected_gpio = 0x106;
			disconnected_gpio = 0x109;
		}
		break;
	case ROUTER_TPLINK_ARCHERC9:
		ses_gpio = 0x002;
		usb_gpio = 0x006;
		usb_gpio1 = 0x007;
		disconnected_gpio = 0x00f;
		connected_gpio = 0x00e;
		power_gpio = 0x112;
		diag_gpio = 0x012;
		usb_power = 0x00c;	// usb 3
		usb_power1 = 0x00d;	// usb 2
		break;
	case ROUTER_TPLINK_ARCHERC3150:
		ses_gpio = 0x002;
//              usb_gpio = 0x006;
//              usb_gpio1 = 0x007;
//              disconnected_gpio = 0x00f;
//              connected_gpio = 0x00e;
//              power_gpio = 0x112;
//              diag_gpio = 0x012;
		usb_power = 0x00c;	// usb 3
		usb_power1 = 0x00d;	// usb 2
		break;
	case ROUTER_ASUS_AC67U:
	case ROUTER_ASUS_AC56U:
		wlan1_gpio = 0x106;
		power_gpio = 0x103;
		usb_power = 0x009;	//usb power on/off
		usb_power1 = 0x00a;	//usb power on/off
		usb_gpio = 0x10e;
		usb_gpio1 = 0x100;
		diag_gpio = 0x003;
		connected_gpio = 0x101;
		disconnected_gpio = 0x102;
		break;
	case ROUTER_ASUS_AC3200:
		usb_power = 0x009;
		power_gpio = 0x103;
		connected_gpio = 0x105;
		diag_gpio = 0x003;
		// wps gpio = 14
		break;
	case ROUTER_ASUS_AC1200:
		usb_power = 0x10a;
		diag_gpio = 0x00a;
		diag_gpio_disabled = 0x10a;
		usb_gpio = 0x10f;
		break;
	case ROUTER_ASUS_AC88U:
	case ROUTER_ASUS_AC3100:
	case ROUTER_ASUS_AC5300:
		usb_power = 0x009;
		usb_gpio = 0x110;
		usb_gpio1 = 0x111;
		power_gpio = 0x103;
		diag_gpio = 0x003;
		connected_gpio = 0x005;
		disconnected_gpio = 0x115;
		ses_gpio = 0x113;
		// komisches symbol gpio 21
		// quantenna reset 8 inv (off / on to reset)    
		break;
	case ROUTER_ASUS_AC87U:
		usb_power = 0x009;
		power_gpio = 0x103;
		connected_gpio = 0x105;
		ses_gpio = 0x101;
		// quantenna reset 8 inv (off / on to reset)    
		break;
	case ROUTER_NETGEAR_EX6200:
		//power_gpio = 0x109;   // connected red
		diag_gpio = 0x101;	// Netgear logo 
		connected_gpio = 0x108;	// connected green
		wlan1_gpio = 0x10b;	// radio led red 2.4G
		wlan0_gpio = 0x10d;	// radio led red 5G
		usb_gpio = 0x105;	// usb led 
		//usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_AC1450:
		power_gpio = 0x102;	// power led green
		//diag_gpio = 0x103;    // power led orange
		diag_gpio = 0x101;	// Netgear logo 
		connected_gpio = 0x10a;	// wan led green - hw controlled
		wlan0_gpio = 0x10b;	// radio led blue
		usb_gpio = 0x108;	// usb led 
		//usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6250:
		power_gpio = 0x102;	// power led green
		//diag_gpio = 0x103;    // power led orange
		diag_gpio = 0x001;	// Netgear logo
		//emblem0_gpio = 0x001; // NETGEAR Emblem       
		//connected_gpio = 0x10f;       // wan led green
		wlan0_gpio = 0x10b;	// radio led blue
		usb_gpio = 0x108;	// usb led green
		//usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6300:
		usb_gpio = 0x108;	//usb led
		usb_power = 0x000;	//usb power on/off
		connected_gpio = 0x10f;	//green led
		power_gpio = 0x102;	//power orange led
		diag_gpio = 0x103;	//power led orange
		//diag_gpio_disabled=0x009;//netgear logo led r
		//emblem0_gpio = 0x101;   // NETGEAR Emblem l     
		//emblem1_gpio = 0x109;   // NETGEAR Emblem r
		wlan0_gpio = 0x10b;	// radio led blue
		break;
	case ROUTER_NETGEAR_R6300V2:
		power_gpio = 0x102;	// power led green
		//diag_gpio = 0x103;    // power led orange
		diag_gpio = 0x101;	// Netgear logo 
		connected_gpio = 0x10a;	// wan led green - hw controlled
		wlan0_gpio = 0x10b;	// radio led blue
		usb_gpio = 0x108;	// usb led 
		//usb_power = 0x000;    // usb enable
		break;
	case ROUTER_NETGEAR_R6400:
		power_gpio = 0x101;	// 
		connected_gpio = 0x107;	//
		usb_power = 0x000;	//
		diag_gpio = 0x102;	// 
		wlan0_gpio = 0x109;	// radio 0 
		wlan1_gpio = 0x108;	// radio 1 
		ses_gpio = 0x10a;	// wps led
		wlan_gpio = 0x10b;	// wifi button led
		usb_gpio = 0x10c;	// usb1 
		usb_gpio1 = 0x10d;	// usb2
		break;
	case ROUTER_NETGEAR_R7000:
		power_gpio = 0x102;	// power led 
		diag_gpio = 0x103;	// power led orange     
		connected_gpio = 0x109;	// wan led
		usb_power = 0x000;	// usb enable
		wlan0_gpio = 0x10d;	// radio 0 
		wlan1_gpio = 0x10c;	// radio 1 
		ses_gpio = 0x10e;	// wps led
		//wlan_gpio = 0x10f;    // wifi button led
		usb_gpio = 0x111;	//usb1 
		usb_gpio1 = 0x112;	//usb2 
		break;
	case ROUTER_NETGEAR_R7000P:
		power_gpio = 0x102;	// power led *
		diag_gpio = 0x103;	// power led orange *    
		connected_gpio = 0x108;	// wan led
		//usb_power = 0x000;    // usb enable
		wlan0_gpio = 0x109;	// radio 0 *
		wlan1_gpio = 0x10a;	// radio 1 *
		ses_gpio = 0x10b;	// wps led * //13 is wifi
		//wlan_gpio = 0x10f;    // wifi button led
		usb_gpio = 0x10e;	//usb1 *
		usb_gpio1 = 0x10f;	//usb2 *
		break;
	case ROUTER_NETGEAR_R7500V2:
	case ROUTER_NETGEAR_R7500:
		power_gpio = 0x000;	// power led 
		diag_gpio = 0x00a;	// power led orange     
		diag_gpio_disabled = 0x000;	// power led orange     
		connected_gpio = 0x007;	// wan led
		usb_power = 0x010;	// usb enable
		usb_power1 = 0x00f;	// usb enable
		wlan0_gpio = 0x001;	// radio 0 
		wlan1_gpio = 0x102;	// radio 1 
		ses_gpio = 0x109;	// wps led
		wlan_gpio = 0x108;	// wifi button led
		usb_gpio = 0x004;	//usb1 
		usb_gpio1 = 0x005;	//usb2 
		break;
	case ROUTER_NETGEAR_R7800:
		power_gpio = 0x000;	// power led 
		diag_gpio = 0x00a;	// power led orange     
		diag_gpio_disabled = 0x000;	// power led orange     
		connected_gpio = 0x007;	// wan led
		usb_power = 0x010;	// usb enable
		usb_power1 = 0x00f;
		wlan0_gpio = 0x009;	// radio 5G 
		wlan1_gpio = 0x008;	// radio 2G
		//ses_gpio = 0x109;     // wps button led used for 2G
		//wlan_gpio = 0x008;    // wifi button led used for 5G
		usb_gpio = 0x004;	//usb1 
		usb_gpio1 = 0x005;	//usb2
		break;
	case ROUTER_ASROCK_G10:
		diag_gpio = 0x009;	// power led orange     
		connected_gpio = 0x008;	// wan led
		disconnected_gpio = 0x007;	// wan led
		break;
	case ROUTER_NETGEAR_R9000:

		power_gpio = 0x016;	// power led 
		diag_gpio = 0x116;	// power led orange     
		diag_gpio_disabled = 0x016;	// power led orange     
		connected_gpio = 0x017;	// wan led
//      usb_power = 0x010;      // usb enable
//      usb_power1 = 0x00f;
		ses_gpio = 0x127;	// wps button led used for 2G
		usb_gpio = 0x024;	//usb1 
		usb_gpio1 = 0x025;	//usb2
		break;
	case ROUTER_TRENDNET_TEW827:
		power_gpio = 0x135;	// power led 
		usb_gpio = 0x107;	// usb led
		break;
	case ROUTER_NETGEAR_R8000:
		power_gpio = 0x102;	// power led 
		diag_gpio = 0x103;	// power led orange     
		connected_gpio = 0x109;	// wan led green
		usb_power = 0x000;	// usb enable
		wlan0_gpio = 0x10d;	// radio 2G 
		wlan1_gpio = 0x10c;	// radio 5G-1 
		wlan2_gpio = 0x110;	// radio 5G-2
		ses_gpio = 0x10e;	// wps led
		wlan_gpio = 0x10f;	// wifi button led
		usb_gpio = 0x111;	//usb1 
		usb_gpio1 = 0x112;	//usb2 
		break;
	case ROUTER_NETGEAR_R8500:
		power_gpio = 0x102;	// power led 
		diag_gpio = 0x10f;	//      
		connected_gpio = 0x109;	// wan led white 1Gb amber 100Mb
		usb_power = 0x000;	// usb enable
		wlan0_gpio = 0x10b;	// radio 5G-1
		wlan1_gpio = 0x10d;	// radio 2G 
		wlan2_gpio = 0x10c;	// radio 5G-2
		ses_gpio = 0x10e;	// wps led
		wlan_gpio = 0x014;	// wifi button led
		usb_gpio = 0x111;	//usb1 
		usb_gpio1 = 0x112;	//usb2 
		break;
	case ROUTER_NETGEAR_WNDR4500:
	case ROUTER_NETGEAR_WNDR4500V2:
		power_gpio = 0x102;	//power led green
		diag_gpio = 0x103;	// power led amber
		connected_gpio = 0x10f;	//wan led green
		wlan0_gpio = 0x109;	//radio 0 led green
		wlan1_gpio = 0x10b;	// radio 1 led blue
		usb_gpio = 0x108;	//usb led green
		usb_gpio1 = 0x10e;	//usb1 led green
		break;
	case ROUTER_ASUS_RTN66:
	case ROUTER_ASUS_AC66U:
		power_gpio = 0x10c;
		diag_gpio = 0x00c;
		usb_gpio = 0x10f;
		break;
	case ROUTER_NETGEAR_WNR2000V2:

		//power_gpio = ??;
		diag_gpio = 0x002;
		ses_gpio = 0x007;	//WPS led
		connected_gpio = 0x006;
		break;
	case ROUTER_WRT320N:
		power_gpio = 0x002;	//power/diag (disabled=blink)
		ses_gpio = 0x103;	// ses blue
		connected_gpio = 0x104;	//ses orange
		break;
	case ROUTER_ASUS_RTN12:
		power_gpio = 0x102;
		diag_gpio = 0x002;	// power blink
		break;
	case ROUTER_BOARD_NEPTUNE:
//              usb_gpio = 0x108;
		// 0x10c //unknown gpio label, use as diag
#ifdef HAVE_RUT500
		diag_gpio = 0x10e;
#else
		diag_gpio = 0x10c;
#endif
		break;
	case ROUTER_ASUS_RTN10U:
		ses_gpio = 0x007;
		usb_gpio = 0x008;
		break;
	case ROUTER_ASUS_RTN12B:
		connected_gpio = 0x105;
		break;
	case ROUTER_ASUS_RTN10PLUSD1:
		ses_gpio = 0x007;
		power_gpio = 0x106;
		diag_gpio = 0x006;
		break;
	case ROUTER_ASUS_RTN10:
	case ROUTER_ASUS_RTN16:
	case ROUTER_NETCORE_NW618:
		power_gpio = 0x101;
		diag_gpio = 0x001;	// power blink
		break;
	case ROUTER_BELKIN_F7D3301:
	case ROUTER_BELKIN_F7D3302:
	case ROUTER_BELKIN_F7D4301:
	case ROUTER_BELKIN_F7D4302:
		power_gpio = 0x10a;	// green
		diag_gpio = 0x10b;	// red
		ses_gpio = 0x10d;	// wps orange
		break;
	case ROUTER_DYNEX_DX_NRUTER:
		power_gpio = 0x001;
		diag_gpio = 0x101;	// power blink
		connected_gpio = 0x100;
		sec0_gpio = 0x103;
		break;
#endif
	}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	if (type == LED_DIAG && v1func == 1) {
		if (act == LED_ON)
			C_led(1);
		else
			C_led(0);
	}
#endif

	switch (type) {
	case LED_POWER:
		use_gpio = power_gpio;
		break;
	case BEEPER:
		use_gpio = beeper_gpio;
		break;
	case USB_POWER:
		use_gpio = usb_power;
		break;
	case USB_POWER1:
		use_gpio = usb_power1;
		break;
	case LED_DIAG:
		if (act == LED_ON)
			led_control(LED_DIAG_DISABLED, LED_OFF);
		else
			led_control(LED_DIAG_DISABLED, LED_ON);
		use_gpio = diag_gpio;
		break;
	case LED_DIAG_DISABLED:
		use_gpio = diag_gpio_disabled;
		break;
	case LED_DMZ:
		use_gpio = dmz_gpio;
		break;
	case LED_CONNECTED:
		if (act == LED_ON)
			led_control(LED_DISCONNECTED, LED_OFF);
		else
			led_control(LED_DISCONNECTED, LED_ON);
		use_gpio = connblue ? ses_gpio : connected_gpio;
		break;
	case LED_DISCONNECTED:
		use_gpio = disconnected_gpio;
		break;
	case LED_BRIDGE:
		use_gpio = bridge_gpio;
		break;
	case LED_VPN:
		use_gpio = vpn_gpio;
		break;
	case LED_SES:
		use_gpio = connblue ? connected_gpio : ses_gpio;
		break;
	case LED_SES2:
		use_gpio = ses2_gpio;
		break;
	case LED_WLAN:
		use_gpio = wlan_gpio;
		break;
	case LED_WLAN0:
		use_gpio = wlan0_gpio;
		break;
	case LED_WLAN1:
		use_gpio = wlan1_gpio;
		break;
	case LED_WLAN2:
		use_gpio = wlan2_gpio;
		break;
	case LED_USB:
		use_gpio = usb_gpio;
		break;
	case LED_USB1:
		use_gpio = usb_gpio1;
		break;
	case LED_SEC0:
		use_gpio = sec0_gpio;
		break;
	case LED_SEC1:
		use_gpio = sec1_gpio;
		break;
	}

	if ((use_gpio & 0x0ff) != 0x0ff) {
		gpio_value = use_gpio & 0x0ff;
		enable = (use_gpio & 0x100) == 0 ? 1 : 0;
		disable = (use_gpio & 0x100) == 0 ? 0 : 1;
		int setin = (use_gpio & 0x200) == 0 ? 0 : 1;
		switch (act) {
		case LED_ON:
			set_gpio(gpio_value, enable);
			if (setin)
				get_gpio(gpio_value);
			break;
		case LED_OFF:
			set_gpio(gpio_value, disable);
			break;
		case LED_FLASH:	// will lit the led for 1 sec.
			set_gpio(gpio_value, enable);
			sleep(1);
			set_gpio(gpio_value, disable);
			break;
		}
	}
	return 1;

#endif
}

int insmod(char *module)
{
	static char word[256];
	char *next, *wordlist;
	int ret = 0;
	wordlist = module;
	foreach(word, wordlist, next) {
		ret |= _evalpid((char *const[]) {
				"insmod", word, NULL}, ">/dev/null", 0, NULL);
	}
	return ret;
}

void rmmod(char *module)
{
	static char word[256];
	char *next, *wordlist;
	wordlist = module;
	foreach(word, wordlist, next) {
		_evalpid((char *const[]) {
			 "rmmod", word, NULL}, ">/dev/null", 0, NULL);
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
	}
	while (--cnt);
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

char *zencrypt(char *passwd, char *passout)
{
	char salt[sizeof("$N$XXXXXXXX")];	/* "$N$XXXXXXXX" or "XX" */

	strcpy(salt, "$1$");
	crypt_make_salt(salt + 3, 4, 0);
	strcpy(passout, crypt((char *)passwd, (char *)salt));
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

int writeproc(char *path, char *value)
{
	int fd;
	fd = open(path, O_WRONLY);
	if (fd == -1) {
		fprintf(stderr, "cannot open %s\n", path);
		return -1;
	}
	write(fd, value, strlen(value));
	close(fd);
	return 0;
}

int writeprocsysnet(char *path, char *value)
{
	char syspath[128];
	snprintf(syspath, sizeof(syspath), "/proc/sys/net/%s", path);
	return writeproc(syspath, value);
}

int writeprocsys(char *path, char *value)
{
	char syspath[128];
	snprintf(syspath, sizeof(syspath), "/proc/sys/%s", path);
	return writeproc(syspath, value);
}

int writevaproc(char *value, char *fmt, ...)
{
	char varbuf[256];
	va_list args;

	va_start(args, (char *)fmt);
	vsnprintf(varbuf, sizeof(varbuf), fmt, args);
	va_end(args);
	return writeproc(varbuf, value);
}

void set_smp_affinity(int irq, int cpu)
{
	char s_cpu[32];
	snprintf(s_cpu, sizeof(s_cpu), "%d", cpu);
	writevaproc(s_cpu, "/proc/irq/%d/smp_affinity", irq);
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
		} else		// nvram_match ("vlan0ports", "1 2 3 4 5*")
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
		if (nvram_match("vlan1ports", "4 3 2 1 5*") && nvram_matchi("boardnum", 32)) {	//R7000/R7000P
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
		} else		// nvram_match ("vlan0ports", "3 2 1 0 5*")
		{
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
	} else if (nvram_match("vlan1ports", "1 5")) {	// Linksys WTR54GS
		vlanmap[5] = 5;
		vlanmap[0] = 1;
		vlanmap[1] = 0;
	} else if (nvram_match("vlan2ports", "0 8") || nvram_match("vlan2ports", "0 8u") || nvram_match("vlan2ports", "0 8t") || nvram_match("vlan2ports", "0 8*")) {
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
		} else		// "3 2 1 0 8*"
		{
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
		if (nvram_match("vlan1ports", "0 1 2 3 8*") && nvram_matchi("boardnum", 4536)) {	// WNDR4500/WNDR4500V2/R6300V1
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
		} else {	// nvram_match ("vlan1ports", "3 2 1 0 5*")
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}
		if (nvram_match("vlan1ports", "0 1 2 3 5*") && nvram_matchi("boardnum", 679)) {	//R6300V2
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

u_int64_t freediskSpace(char *path)
{
	struct statfs sizefs;

	if ((statfs(path, &sizefs) != 0) || (sizefs.f_type == 0x73717368) || (sizefs.f_type == 0x74717368) || (sizefs.f_type == 0x68737174)) {
		bzero(&sizefs, sizeof(sizefs));
	}

	return (u_int64_t)sizefs.f_bsize * (u_int64_t)sizefs.f_bfree;
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
		if (nvram_get("et0macaddr"))
			strcpy(newmac, nvram_safe_get("et0macaddr"));
		else
			strcpy(newmac, nvram_safe_get("et2macaddr"));
		break;
	default:
		strcpy(newmac, nvram_safe_get("et0macaddr"));
		break;
	}

}

#define MAX_BRIDGES	1024

#include <linux/if_bridge.h>
int isbridge(char *name)
{
	int i, num;
	char ifname[IFNAMSIZ];
	int ifindices[MAX_BRIDGES];
	int br_socket_fd = -1;
	unsigned long args[3] = { BRCTL_GET_BRIDGES,
		(unsigned long)ifindices, MAX_BRIDGES
	};

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

#define PER_MAC_LEN	18	// contain '\0'

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

char *get_wan_face(void)
{
	static char localwanface[IFNAMSIZ];
	if (nvram_match("wan_proto", "disabled"))
		return "br0";

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
			strncpy(localwanface, "ppp0", IFNAMSIZ);
		else
			strncpy(localwanface, nvram_safe_get("pppd_pppifname"), IFNAMSIZ);
	}
#ifdef HAVE_3G
	else if (nvram_match("wan_proto", "3g")) {
		if (nvram_match("3gdata", "mbim")) {
			strncpy(localwanface, "wwan0", IFNAMSIZ);
		} else if (nvram_match("3gdata", "qmi")) {
			strncpy(localwanface, "wwan0", IFNAMSIZ);
		} else if (nvram_match("3gdata", "sierradirectip")) {
			strncpy(localwanface, "wwan0", IFNAMSIZ);
			if (nvram_match("pppd_pppifname", ""))
				strncpy(localwanface, "ppp0", IFNAMSIZ);
			else
				strncpy(localwanface, nvram_safe_get("pppd_pppifname"), IFNAMSIZ);
		}

	}
#endif
#ifndef HAVE_MADWIFI
	else if (nvram_invmatch("sta_ifname", "")) {
		strcpy(localwanface, nvram_safe_get("sta_ifname"));
	}
#else
	else if (nvram_invmatch("sta_ifname", "")) {
		if (nvram_matchi("wifi_bonding", 1))
			strcpy(localwanface, "bond0");
		else
			strcpy(localwanface, nvram_safe_get("sta_ifname"));
	}
#endif
#ifdef HAVE_IPETH
	else if (nvram_match("wan_proto", "iphone")) {
		strncpy(localwanface, "iph0", IFNAMSIZ);
	}
#endif
	else
		strncpy(localwanface, nvram_safe_get("wan_ifname"), IFNAMSIZ);

	return localwanface;
}
