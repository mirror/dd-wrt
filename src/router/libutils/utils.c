/*[18~
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
#ifdef HAVE_ATH9K
#include <glob.h>
#endif

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
#include <cymac.h>
#include <broadcom.h>

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

#define TRX_MAGIC_F7D3301			0x20100322	/* Belkin Share Max; router's birthday ? */
#define TRX_MAGIC_F7D3302			0x20090928	/* Belkin Share; router's birthday ? */
#define TRX_MAGIC_F7D4302			0x20091006	/* Belkin Play; router's birthday ? */

#ifdef HAVE_FONERA
static void inline getBoardMAC(char *mac)
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
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", data[102] & 0xff, data[103] & 0xff, data[104] & 0xff, data[105] & 0xff, data[106] & 0xff, data[107] & 0xff);
}
#endif

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

char *getBridgeMTU(char *ifname)
{
	static char word[256];
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
		memset(&ifr, 0, sizeof(ifr));
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

#ifdef HAVE_SVQOS
char
*get_wshaper_dev(void)
{
	if (nvram_match("wshaper_dev", "WAN"))
		return get_wan_face();
	else
		return "br0";
}

char
*get_mtu_val(void)
{
	if (nvram_match("wshaper_dev", "WAN")
	    && !strcmp(get_wshaper_dev(), "ppp0"))
		return nvram_safe_get("wan_mtu");
	else if (nvram_match("wshaper_dev", "WAN")) {
		if (nvram_match("wan_mtu", "1500"))
			return getMTU(get_wshaper_dev());
		else
			return nvram_safe_get("wan_mtu");
	} else
		return getBridgeMTU(get_wshaper_dev());
}

void add_client_mac_srvfilter(char *name, char *type, char *data, char *level, int base, char *client)
{
	int idx = atoi(level) / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p udp -m udp --dport %s -m mac --mac-source %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p udp -m udp --sport %s -m mac --mac-source %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m tcp --dport %s -m mac --mac-source %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m tcp --sport %s -m mac --mac-source %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto %s -m mac --mac-source %s -j MARK --set-mark %s", name, client, qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		sysprintf("iptables -t mangle -I FILTER_IN 3 -m mac --mac-source %s -m ndpi --%s -j MARK --set-mark %s", client, name, qos_nfmark(base + idx));
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = NULL;
		char *realname = name;

		if (!strcasecmp(realname, "applejuice"))
			proto = "apple";
		else if (!strcasecmp(realname, "ares"))
			proto = "ares";
		else if (!strcasecmp(realname, "bearshare"))
			proto = "gnu";
		else if (!strcasecmp(realname, "bittorrent"))
			proto = "bit";
		else if (!strcasecmp(realname, "directconnect"))
			proto = "dc";
		else if (!strcasecmp(realname, "edonkey"))
			proto = "edk";
		else if (!strcasecmp(realname, "gnutella"))
			proto = "gnu";
		else if (!strcasecmp(realname, "kazaa"))
			proto = "kazaa";
		else if (!strcasecmp(realname, "mute"))
			proto = "mute";
		else if (!strcasecmp(realname, "soulseek"))
			proto = "soul";
		else if (!strcasecmp(realname, "waste"))
			proto = "waste";
		else if (!strcasecmp(realname, "winmx"))
			proto = "winmx";
		else if (!strcasecmp(realname, "xdcc"))
			proto = "xdcc";
		if (proto) {
			insmod("ipt_ipp2p");

			sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m ipp2p --%s -m mac --mac-source %s -j MARK --set-mark %s", proto, client, qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt -m mac --mac-source %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
#else
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m length --length 0:550 -m layer7 --l7proto bt -m mac --mac-source %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
#endif
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt1 -m mac --mac-source %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt2 -m mac --mac-source %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
			}
		}
	}
}

void add_client_ip_srvfilter(char *name, char *type, char *data, char *level, int base, char *client)
{
	int idx = atoi(level) / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p udp -m udp --dport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p udp -m udp --sport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p udp -m udp --dport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p udp -m udp --sport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p udp -m udp --dport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p udp -m udp --sport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p udp -m udp --dport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p udp -m udp --sport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p tcp -m tcp --dport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p tcp -m tcp --sport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p tcp -m tcp --dport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -p tcp -m tcp --sport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m tcp --dport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m tcp --sport %s -s %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m tcp --dport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m tcp --sport %s -d %s -j MARK --set-mark %s", data, client, qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto %s -s %s -j MARK --set-mark %s", name, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto %s -d %s -j MARK --set-mark %s", name, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto %s -s %s -j MARK --set-mark %s", name, client, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto %s -d %s -j MARK --set-mark %s", name, client, qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -s %s -m ndpi --%s -j MARK --set-mark %s", client, name, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_OUT 3 -d %s -m ndpi --%s -j MARK --set-mark %s", client, name, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -s %s -m ndpi --%s -j MARK --set-mark %s", client, name, qos_nfmark(base + idx));
		sysprintf("iptables -t mangle -I FILTER_IN 3 -d %s -m ndpi --%s -j MARK --set-mark %s", client, name, qos_nfmark(base + idx));
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = NULL;
		char *realname = name;

		if (!strcasecmp(realname, "applejuice"))
			proto = "apple";
		else if (!strcasecmp(realname, "ares"))
			proto = "ares";
		else if (!strcasecmp(realname, "bearshare"))
			proto = "gnu";
		else if (!strcasecmp(realname, "bittorrent"))
			proto = "bit";
		else if (!strcasecmp(realname, "directconnect"))
			proto = "dc";
		else if (!strcasecmp(realname, "edonkey"))
			proto = "edk";
		else if (!strcasecmp(realname, "gnutella"))
			proto = "gnu";
		else if (!strcasecmp(realname, "kazaa"))
			proto = "kazaa";
		else if (!strcasecmp(realname, "mute"))
			proto = "mute";
		else if (!strcasecmp(realname, "soulseek"))
			proto = "soul";
		else if (!strcasecmp(realname, "waste"))
			proto = "waste";
		else if (!strcasecmp(realname, "winmx"))
			proto = "winmx";
		else if (!strcasecmp(realname, "xdcc"))
			proto = "xdcc";
		if (proto) {
			insmod("ipt_ipp2p");

			sysprintf("iptables -t mangle -I FILTER_OUT 3 -p tcp -m ipp2p --%s -s %s -j MARK --set-mark %s", proto, client, qos_nfmark(base + idx));
			sysprintf("iptables -t mangle -I FILTER_OUT 3 -p tcp -m ipp2p --%s -d %s -j MARK --set-mark %s", proto, client, qos_nfmark(base + idx));
			sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m ipp2p --%s -s %s -j MARK --set-mark %s", proto, client, qos_nfmark(base + idx));
			sysprintf("iptables -t mangle -I FILTER_IN 3 -p tcp -m ipp2p --%s -d %s -j MARK --set-mark %s", proto, client, qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto bt -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto bt -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
#else
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m length --length 0:550 -m layer7 --l7proto bt -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m length --length 0:550 -m layer7 --l7proto bt -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m length --length 0:550 -m layer7 --l7proto bt -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m length --length 0:550 -m layer7 --l7proto bt -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
#endif
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto bt1 -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto bt1 -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt1 -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt1 -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto bt2 -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_OUT 3 -m layer7 --l7proto bt2 -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt2 -s %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
				sysprintf("iptables -t mangle -I FILTER_IN 3 -m layer7 --l7proto bt2 -d %s -j MARK --set-mark %s", client, qos_nfmark(base + idx));
			}
		}
	}
}

#if !(defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN))
char *get_tcfmark(uint32 mark)
{
	static char tcfmark[24];
	char nfmark[24];
	char *ntoken = NULL;

	memset(&tcfmark, 0, sizeof(tcfmark));
	memset(&nfmark, 0, sizeof(nfmark));

	strcpy(nfmark, qos_nfmark(mark));

	ntoken = strtok(nfmark, "/");
	strcat(tcfmark, ntoken);

	ntoken = strtok(NULL, "/");
	strcat(tcfmark, " ");
	strcat(tcfmark, ntoken);

	return tcfmark;
}
#endif

#ifdef HAVE_AQOS
void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned long lanrate, unsigned int level)
{
	char *wan_dev = get_wan_face();

	unsigned int uplimit = atoi(nvram_get("wshaper_uplink"));
	unsigned int downlimit = atoi(nvram_get("wshaper_downlink"));
	unsigned long lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;

	unsigned int quantum = atoi(get_mtu_val()) + 14;

	if (lanrate < 1)
		lanrate = lanlimit;

#else
void add_client_classes(unsigned int base, unsigned int level)
{
	char *wan_dev = get_wan_face();

	unsigned int uplimit = atoi(nvram_get("wshaper_uplink"));
	unsigned int downlimit = atoi(nvram_get("wshaper_downlink"));
	unsigned long lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;

	unsigned int quantum = atoi(get_mtu_val()) + 14;

	unsigned long uprate = 0, downrate = 0;
	int lanrate = lanlimit;
#endif

	char *aqd = nvram_safe_get("svqos_aqd");

	switch (level) {
	case 100:
		uprate = uplimit * 60 / 100;
		downrate = downlimit * 60 / 100;
		lanrate = lanlimit * 60 / 100;
		prio = 2;
		parent = 2;
		break;
	case 10:
		uprate = uplimit * 25 / 100;
		downrate = downlimit * 25 / 100;
		lanrate = lanlimit * 25 / 100;
		prio = 3;
		parent = 3;
		break;
	case 20:
		uprate = uplimit * 10 / 100;
		downrate = downlimit * 10 / 100;
		lanrate = lanlimit * 10 / 100;
		prio = 4;
		parent = 4;
		break;
	case 30:
		uprate = uplimit * 5 / 100;
		downrate = downlimit * 5 / 100;
		lanrate = lanlimit * 5 / 100;
		prio = 5;
		parent = 5;
		break;
	case 40:
		uprate = uprate * 1 / 100;
		downrate = downlimit * 1 / 100;
		lanrate = lanlimit * 1 / 100;
		prio = 6;
		parent = 6;
		break;
	case 0:
		uplimit = uprate;
		downlimit = downrate;
		lanlimit = lanrate;
		prio = 3;
		parent = 1;
		break;
	}

	if (nvram_match("qos_type", "0")) {	// HTB
		sysprintf	// interior
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", wan_dev, parent, base, uprate, uplimit, quantum);
		sysprintf	// expempt
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", wan_dev, base, base + 1, uprate * 60 / 100, uplimit, quantum);
		sysprintf	// premium
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", wan_dev, base, base + 2, uprate * 25 / 100, uplimit, quantum, prio);
		sysprintf	// express
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", wan_dev, base, base + 3, uprate * 10 / 100, uplimit, quantum, prio + 1);
		sysprintf	// standard
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", wan_dev, base, base + 4, uprate * 5 / 100, uplimit, quantum, prio + 1);
		sysprintf	// bulk
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 7", wan_dev, base, base + 5, 1, uplimit, quantum);

		sysprintf	// interior
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", "imq0", parent, base, downrate, downlimit, quantum);
		sysprintf	// exempt
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", "imq0", base, base + 1, downrate * 60 / 100, downlimit, quantum);
		sysprintf	// premium
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq0", base, base + 2, downrate * 25 / 100, downlimit, quantum, prio);
		sysprintf	// express
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq0", base, base + 3, downrate * 10 / 100, downlimit, quantum, prio + 1);
		sysprintf	// standard
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq0", base, base + 4, downrate * 5 / 100, downlimit, quantum, prio + 1);
		sysprintf	// bulk
		    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 7", "imq0", base, base + 5, 1, downlimit, quantum);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf	// interior
			    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", "imq1", parent, base, lanrate, lanlimit, quantum);
			sysprintf	// exempt
			    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", "imq1", base, base + 1, lanrate * 60 / 100, lanlimit, quantum);
			sysprintf	// premium
			    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq1", base, base + 2, lanrate * 25 / 100, lanlimit, quantum, prio);
			sysprintf	// express
			    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq1", base, base + 3, lanrate * 10 / 100, lanlimit, quantum, prio + 1);
			sysprintf	// standard
			    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq1", base, base + 4, lanrate * 5 / 100, lanlimit, quantum, prio + 1);
			sysprintf	// bulk
			    ("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 7", "imq1", base, base + 5, 1, lanlimit, quantum);
		}
	} else {		// HFSC
		sysprintf	// interior
		    ("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, 1, base, uprate, uplimit);
		sysprintf	// exempt (srv)
		    ("tc class add dev %s parent 1:%d classid 1:%d hfsc rt umax 1500b dmax 30ms rate 100kbit ls rate %d ul rate %d", wan_dev, base, base + 1, uprate, uplimit);
		sysprintf	// premium
		    ("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 2, uprate * 75 / 100, uplimit);
		sysprintf	// express
		    ("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 3, uprate * 15 / 100, uplimit);
		sysprintf	// standard
		    ("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 4, uprate * 10 / 100, uplimit);
		sysprintf	// bulk
		    ("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 5, 1, uplimit);

		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", 1, base, downrate, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc rt umax 1500b dmax 30ms rate 100kbit ls rate %d ul rate %d", "imq0", base, base + 1, downrate, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 2, downrate * 75 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 3, downrate * 15 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 4, downrate * 10 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 5, 1, downlimit);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", 1, base, lanlimit, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc rt umax 1500b dmax 30ms rate 100kbit ls rate %d ul rate %d", "imq1", base, base + 1, lanlimit, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 2, lanlimit * 75 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 3, lanlimit * 15 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 4, lanlimit * 10 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 5, 1, lanlimit);
		}
	}

#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	// filter rules
	sysprintf("tc filter add dev %s protocol ip pref 1 handle 0x%x fw classid 1:%d", wan_dev, base, base + 1);
	sysprintf("tc filter add dev %s protocol ip pref 3 handle 0x%x fw classid 1:%d", wan_dev, base + 1, base + 2);
	sysprintf("tc filter add dev %s protocol ip pref 5 handle 0x%x fw classid 1:%d", wan_dev, base + 2, base + 3);
	sysprintf("tc filter add dev %s protocol ip pref 8 handle 0x%x fw classid 1:%d", wan_dev, base + 3, base + 4);
	sysprintf("tc filter add dev %s protocol ip pref 9 handle 0x%x fw classid 1:%d", wan_dev, base + 4, base + 5);

	sysprintf("tc filter add dev %s protocol ip pref 1 handle 0x%x fw classid 1:%d", "imq0", base, base + 1);
	sysprintf("tc filter add dev %s protocol ip pref 3 handle 0x%x fw classid 1:%d", "imq0", base + 1, base + 2);
	sysprintf("tc filter add dev %s protocol ip pref 5 handle 0x%x fw classid 1:%d", "imq0", base + 2, base + 3);
	sysprintf("tc filter add dev %s protocol ip pref 8 handle 0x%x fw classid 1:%d", "imq0", base + 3, base + 4);
	sysprintf("tc filter add dev %s protocol ip pref 9 handle 0x%x fw classid 1:%d", "imq0", base + 4, base + 5);

	if (nvram_match("wshaper_dev", "LAN")) {
		sysprintf("tc filter add dev %s protocol ip pref 1 handle 0x%x fw classid 1:%d", "imq1", base, base + 1);
		sysprintf("tc filter add dev %s protocol ip pref 3 handle 0x%x fw classid 1:%d", "imq1", base + 1, base + 2);
		sysprintf("tc filter add dev %s protocol ip pref 5 handle 0x%x fw classid 1:%d", "imq1", base + 2, base + 3);
		sysprintf("tc filter add dev %s protocol ip pref 8 handle 0x%x fw classid 1:%d", "imq1", base + 3, base + 4);
		sysprintf("tc filter add dev %s protocol ip pref 9 handle 0x%x‚	 fw classid 1:%d", "imq1", base + 4, base + 5);
	}
#else
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", wan_dev, get_tcfmark(base), base + 1);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", wan_dev, get_tcfmark(base + 1), base + 2);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", wan_dev, get_tcfmark(base + 2), base + 3);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", wan_dev, get_tcfmark(base + 3), base + 4);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", wan_dev, get_tcfmark(base + 4), base + 5);

	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq0", get_tcfmark(base), base + 1);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq0", get_tcfmark(base + 1), base + 2);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq0", get_tcfmark(base + 2), base + 3);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq0", get_tcfmark(base + 3), base + 4);
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq0", get_tcfmark(base + 4), base + 5);

	if (nvram_match("wshaper_dev", "LAN")) {
		sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq1", get_tcfmark(base), base + 1);
		sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq1", get_tcfmark(base + 1), base + 2);
		sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq1", get_tcfmark(base + 2), base + 3);
		sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq1", get_tcfmark(base + 3), base + 4);
		sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", "imq1", get_tcfmark(base + 4), base + 5);
	}
#endif

	// leaf qdiscs
	if (!strcmp(aqd, "sfq")) {
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 1, base + 1, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 2, base + 2, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 3, base + 3, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 4, base + 4, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 5, base + 5, quantum);

		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 1, base + 1, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 2, base + 2, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 3, base + 3, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 4, base + 4, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 5, base + 5, quantum);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 1, base + 1, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 2, base + 2, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 3, base + 3, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 4, base + 4, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 5, base + 5, quantum);
		}
	}
#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)
	if (!strcmp(aqd, "codel")
	    || !strcmp(aqd, "fq_codel")) {
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", wan_dev, base + 1, base + 1, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", wan_dev, base + 2, base + 2, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", wan_dev, base + 3, base + 3, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", wan_dev, base + 4, base + 4, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", wan_dev, base + 5, base + 5, aqd);

		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 1, base + 1, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 2, base + 2, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 3, base + 3, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 4, base + 4, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 5, base + 5, aqd);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 1, base + 1, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 2, base + 2, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 3, base + 3, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 4, base + 4, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 5, base + 5, aqd);
		}
	}
#endif
}

#ifdef HAVE_AQOS
void add_usermac(char *mac, int base, char *upstream, char *downstream, char *lanstream)
{
	unsigned int uprate = atoi(upstream);
	unsigned int downrate = atoi(downstream);
	unsigned long lanrate = atoi(lanstream);

	char srvname[32], srvtype[32], srvdata[32], srvlevel[32];
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(0));

	system2("iptables -t mangle -D FILTER_IN -j CONNMARK --save");
// http://svn.dd-wrt.com/ticket/2737 (default bandwidth level)
//      sysprintf
//          ("iptables -t mangle -D FILTER_IN -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j MARK --set-mark %s", 
//               qos_nfmark(0x64));
	system2("iptables -t mangle -D FILTER_IN -j RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata, srvlevel) < 4)
			break;

		add_client_mac_srvfilter(srvname, srvtype, srvdata, srvlevel, base, mac);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	sysprintf("iptables -t mangle -A FILTER_IN -m mac --mac-source %s -m mark --mark %s -j MARK --set-mark %s", mac, nullmask, qos_nfmark(base));

	system2("iptables -t mangle -A FILTER_IN -j CONNMARK --save");
// http://svn.dd-wrt.com/ticket/2737 (default bandwidth level)
//      sysprintf
//          ("iptables -t mangle -A FILTER_IN -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j MARK --set-mark %s",
//               qos_nfmark(0x64));
	system2("iptables -t mangle -A FILTER_IN -j RETURN");
}

void add_userip(char *ip, int base, char *upstream, char *downstream, char *lanstream)
{
	unsigned int uprate = atoi(upstream);
	unsigned int downrate = atoi(downstream);
	unsigned long lanrate = atoi(lanstream);

//      int ret;
	char srvname[32], srvtype[32], srvdata[32], srvlevel[32];
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(0));

	system2("iptables -t mangle -D FILTER_IN -j CONNMARK --save");
// http://svn.dd-wrt.com/ticket/2737 (default bandwidth level)
//      sysprintf
//          ("iptables -t mangle -D FILTER_IN -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j MARK --set-mark %s",
//               qos_nfmark(0x64));
	system2("iptables -t mangle -D FILTER_IN -j RETURN");

	system2("iptables -t mangle -D FILTER_OUT -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j CLASSIFY --set-class 1:100");
	system2("iptables -t mangle -D FILTER_OUT -m layer7 --l7proto dns -j CLASSIFY --set-class 1:100");
	system2("iptables -t mangle -D FILTER_OUT -j VPN_DSCP");
	system2("iptables -t mangle -D FILTER_OUT -j CONNMARK --save");
	system2("iptables -t mangle -D FILTER_OUT -j RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata, srvlevel) < 4)
			break;

		add_client_ip_srvfilter(srvname, srvtype, srvdata, srvlevel, base, ip);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	sysprintf("iptables -t mangle -A FILTER_OUT -s %s -m mark --mark %s -j MARK --set-mark %s", ip, nullmask, qos_nfmark(base));
	sysprintf("iptables -t mangle -A FILTER_OUT -d %s -m mark --mark %s -j MARK --set-mark %s", ip, nullmask, qos_nfmark(base));
	sysprintf("iptables -t mangle -A FILTER_IN -s %s -m mark --mark %s -j MARK --set-mark %s", ip, nullmask, qos_nfmark(base));
	sysprintf("iptables -t mangle -A FILTER_IN -d %s -m mark --mark %s -j MARK --set-mark %s", ip, nullmask, qos_nfmark(base));

	system2("iptables -t mangle -A FILTER_IN -j CONNMARK --save");
// http://svn.dd-wrt.com/ticket/2737 (default bandwidth level)
//      sysprintf
//          ("iptables -t mangle -A FILTER_IN -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j MARK --set-mark %s",
//               qos_nfmark(0x64));
	system2("iptables -t mangle -A FILTER_IN -j RETURN");

	system2("iptables -t mangle -A FILTER_OUT -p tcp -m length --length 0:64 --tcp-flags ACK ACK -j CLASSIFY --set-class 1:100");
	system2("iptables -t mangle -A FILTER_OUT -m layer7 --l7proto dns -j CLASSIFY --set-class 1:100");
	system2("iptables -t mangle -A FILTER_OUT -j VPN_DSCP");
	system2("iptables -t mangle -A FILTER_OUT -j CONNMARK --save");
	system2("iptables -t mangle -A FILTER_OUT -j RETURN");
}
#endif
#endif				// HAVE_SVQOS

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

int check_action(void)
{
	char buf[80] = "";

	if (file_to_buf(ACTION_FILE, buf, sizeof(buf))) {
		if (!strcmp(buf, "ACT_TFTP_UPGRADE")) {
			fprintf(stderr, "Upgrading from tftp now ...\n");
			return ACT_TFTP_UPGRADE;
		}
#ifdef HAVE_HTTPS
		else if (!strcmp(buf, "ACT_WEBS_UPGRADE")) {
			fprintf(stderr, "Upgrading from web (https) now ...\n");
			return ACT_WEBS_UPGRADE;
		}
#endif
		else if (!strcmp(buf, "ACT_WEB_UPGRADE")) {
			fprintf(stderr, "Upgrading from web (http) now ...\n");
			return ACT_WEB_UPGRADE;
		} else if (!strcmp(buf, "ACT_SW_RESTORE")) {
			fprintf(stderr, "Receiving restore command from web ...\n");
			return ACT_SW_RESTORE;
		} else if (!strcmp(buf, "ACT_HW_RESTORE")) {
			fprintf(stderr, "Receiving restore command from resetbutton ...\n");
			return ACT_HW_RESTORE;
		} else if (!strcmp(buf, "ACT_NVRAM_COMMIT")) {
			fprintf(stderr, "Committing nvram now ...\n");
			return ACT_NVRAM_COMMIT;
		} else if (!strcmp(buf, "ACT_ERASE_NVRAM")) {
			fprintf(stderr, "Erasing nvram now ...\n");
			return ACT_ERASE_NVRAM;
		}
	}
	// fprintf(stderr, "Waiting for upgrading....\n");
	return ACT_IDLE;
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

	if (nvram_match("boardtype", "bcm94710dev")
	    || nvram_match("boardtype", "0x0101") || (boardflags & 0x0100))
		return 1;
	else
		return 0;
#endif
}

void setRouter(char *name)
{
#ifdef HAVE_POWERNOC_WORT54G
	nvram_set(NVROUTER, "WORT54G");
#elif HAVE_POWERNOC_WOAP54G
	nvram_set(NVROUTER, "WOAP54G");
#elif HAVE_ERC
	nvram_set(NVROUTER, "ServiceGate v1.0");
#elif HAVE_OMNI
	nvram_set(NVROUTER, "Omni Wifi Router");
#elif HAVE_ALFA_BRANDING
	nvram_set(NVROUTER, "WLAN base-station");
	if (name)
		nvram_set("DD_BOARD2", name);
#elif HAVE_MAKSAT
	if (name)
		nvram_set("DD_BOARD2", name);
#ifdef HAVE_MAKSAT_BLANK
	nvram_set(NVROUTER, "default");
#else
	nvram_set(NVROUTER, "MAKSAT");
#endif
#elif HAVE_TMK
	if (name)
		nvram_set("DD_BOARD2", name);
	nvram_set(NVROUTER, "KMT-WAS");
#elif HAVE_BKM
	if (name)
		nvram_set("DD_BOARD2", name);
	nvram_set(NVROUTER, "BKM-HSDL");
#elif HAVE_TRIMAX
	if (name)
		nvram_set("DD_BOARD2", name);
	nvram_set(NVROUTER, "M2M Dynamics");
#elif HAVE_WIKINGS
	if (name)
		nvram_set("DD_BOARD2", name);
#ifdef HAVE_SUB3
	nvram_set(NVROUTER, "ExcelMin");
#elif HAVE_SUB6
	nvram_set(NVROUTER, "ExcelMed");
#else
	nvram_set(NVROUTER, "Excellent");
#endif
#elif HAVE_SANSFIL
	nvram_set("DD_BOARD", "SANSFIL");
	nvram_set("DD_BOARD2", name);
#elif HAVE_ESPOD
	if (name)
		nvram_set("DD_BOARD2", name);
#ifdef HAVE_SUB3
	nvram_set(NVROUTER, "ESPOD ES-3680");
#elif HAVE_SUB6
	nvram_set(NVROUTER, "ESPOD ES-3680");
#else
	nvram_set(NVROUTER, "ESPOD ES-3680");
#endif
#elif HAVE_CARLSONWIRELESS
	nvram_set(NVROUTER, "LH-135/270 ST");
#elif HAVE_IPR
	nvram_set(NVROUTER, "IPR-DATKS-501");
#else
	if (name)
		nvram_set(NVROUTER, name);
#endif
	cprintf("router is %s\n", getRouter());
}

char *getRouter()
{
	char *n = nvram_get(NVROUTER);

	return n != NULL ? n : "Unknown Model";
}

int internal_getRouterBrand()
{
#if defined(HAVE_ALLNETWRT) && !defined(HAVE_ECB9750)
	unsigned long boardnum = strtoul(nvram_safe_get("boardnum"), NULL, 0);

	if (boardnum == 8 && nvram_match("boardtype", "0x048e")
	    && nvram_match("boardrev", "0x11")) {
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


	if (boardnum == 00 && nvram_match("boardtype", "0xF646")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("melco_id", "RD_BB12068")) {
#ifdef HAVE_BUFFALO
		setRouter("WZR-1750DHP");
#else
		setRouter("Buffalo WZR-1750DHP");
#endif
		return ROUTER_BUFFALO_WZR1750;
	}

	if ((boardnum == 2013012401 || boardnum == 2013083001 || boardnum == 2013032101) && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("0:rxchain", "7")) {
#ifdef HAVE_BUFFALO
		setRouter("WZR-900DHP");
#else
		setRouter("Buffalo WZR-900DHP");
#endif		
		return ROUTER_BUFFALO_WZR900DHP;
	}
	
	if ((boardnum == 2013012401 || boardnum == 2013083001 || boardnum == 2013032101) && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("0:rxchain", "3")) {
#ifdef HAVE_BUFFALO
		setRouter("WZR-600DHP2");
#else
		setRouter("Buffalo WZR-600DHP2");
#endif
		return ROUTER_BUFFALO_WZR600DHP2;
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

	if (nvram_match("productid", "RT-AC68U")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}

	if (nvram_match("model", "RT-AC68U")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}

	if (boardnum == 24 && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1110")
	    && nvram_match("gpio7", "wps_button")) {
		setRouter("Dlink-DIR868L");
		return ROUTER_DLINK_DIR868;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("gpio15", "wps_button")) {
		setRouter("Asus RT-AC56U");
		return ROUTER_ASUS_AC56U;
	}

	if (boardnum != 24 && nvram_match("boardtype", "0x0646")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("gpio7", "wps_button")) {
		setRouter("Asus RT-AC68U");
		return ROUTER_ASUS_AC67U;
	}


	setRouter("Broadcom Northstar");
	return ROUTER_BOARD_NORTHSTAR;
#elif HAVE_VENTANA
	char *filename = "/sys/devices/soc.0/2100000.aips-bus/21a0000.i2c/i2c-0/0-0051/eeprom";	/* bank2=0x100 kernel 3.0 */
	FILE *file = fopen(filename, "rb");
	if (!file) {
	    setRouter("Gateworks Ventana GW54XX");
	}else{
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
	    } else if (!strncmp(gwid, "GW5410-B", 8)) {
		setRouter("Gateworks Ventana GW5410-B");
	    } else if (!strncmp(gwid, "GW5410-C", 8)) {
		setRouter("Gateworks Ventana GW5410-C");
	    } else
		setRouter("Gateworks Ventana GW54XX");
	}
	return ROUTER_BOARD_GW2388;
#elif HAVE_LAGUNA
	char *filename = "/sys/devices/platform/cns3xxx-i2c.0/i2c-0/0-0050/eeprom";	/* bank2=0x100 kernel 3.0 */
	FILE *file = fopen(filename, "rb");
	if (!file) {
		filename = "/sys/devices/platform/cns3xxx-i2c.0/i2c-adapter/i2c-0/0-0050/eeprom";	/* bank2=0x100 older kernel */
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
		} else if (!strncmp(gwid, "GW2382", 6)) {
			setRouter("Gateworks Laguna GW2382");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2380", 6)) {
			setRouter("Gateworks Laguna GW2380");
			return ROUTER_BOARD_GW2380;
		} else if (!strncmp(gwid, "GW2391", 6)) {
			setRouter("Gateworks Laguna GW2391");
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
	char *filename = "/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0051/eeprom";	/* bank2=0x100 
												 */
	FILE *file = fopen(filename, "r");
	if (!file) {
		filename = "/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0051/eeprom";	//for 2.6.34.6
		file = fopen(filename, "r");
	}
	if (file)		// new detection scheme
	{

		char *gwid;
		char temp[64];
		int ret = fread(temp, 40, 1, file);
		if (ret < 1) {
			fclose(file);
			goto old_way;
		}
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
#define IX_ETH_ACC_MII_PHY_ID1_REG  0x2	/* PHY identifier 1 Register */
#define IX_ETH_ACC_MII_PHY_ID2_REG  0x3	/* PHY identifier 2 Register */
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
	fprintf(stderr, "phy id %X:%X\n", reg1, reg2);
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
	} else if (reg1 == 0x143 && reg2 == 0xbc31)	// broadcom phy
	{
		setRouter("ADI Engineering Pronghorn Metro");
		return ROUTER_BOARD_GATEWORX;
	} else if (reg1 == 0x22 && reg2 == 0x1450)	// kendin switch 
	{
		setRouter("Gateworks Avila GW2345");
		return ROUTER_BOARD_GATEWORX_GW2345;
	} else if (reg1 == 0x0 && reg2 == 0x8201)	// realtek 
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
#elif HAVE_DIR615
	setRouter("Dlink-DIR615 rev d");
	return ROUTER_BOARD_DIR615D;
#elif HAVE_RT3352
	setRouter("Ralink RT3352 Device");
	return ROUTER_BOARD_RT3352;
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
#elif HAVE_WHR300HP2
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
	setRouter("Generic X86");
	return ROUTER_BOARD_X86;
#endif
#elif HAVE_XSCALE
	setRouter("NewMedia Dual A/B/G");
	return ROUTER_BOARD_XSCALE;
#elif HAVE_MAGICBOX
	setRouter("OpenRB PowerPC Board");
	return ROUTER_BOARD_MAGICBOX;
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
	if (!strncmp(mac, "00:19:3B", 8) || !strncmp(mac, "00:02:6F", 8)
	    || !strncmp(mac, "00:15:6D", 8)) {
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
	if (!strncmp(mac, "00:19:3B", 8) || !strncmp(mac, "00:02:6F", 8)
	    || !strncmp(mac, "00:15:6D", 8) || !strncmp(mac, "00:C0:CA", 8)) {
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
		if ((data->val_out & 0xfc00) != 0x0c00)	// marvell phy
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
#elif HAVE_MERAKI
	setRouter("Meraki Mini");
	return ROUTER_BOARD_MERAKI;
#elif HAVE_BWRG1000
	setRouter("Bountiful BWRG-1000");
	return ROUTER_BOARD_LS2;
#elif HAVE_WNR2200
	setRouter("Netgear WNR2200");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WNR2000
	setRouter("Netgear WNR2000v3");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WLAEAG300N
#ifdef HAVE_BUFFALO
	setRouter("WLAE-AG300N");
#else
	setRouter("Buffalo WLAE-AG300N");
#endif
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_CARAMBOLA
	setRouter("8Devices Carambola 2");
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_HORNET
	setRouter("Atheros Hornet");
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_RB2011
	setRouter("Mikrotik RB2011");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR2543
	setRouter("TPLINK TL-WR2543");
	nvram_default_get("ath0_rxantenna", "7");
	nvram_default_get("ath0_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR3600
	setRouter("TPLINK TL-WDR3600 v1");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WDR4300
	setRouter("TPLINK TL-WDR4300 v1");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "7");
	nvram_default_get("ath1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR835A1
	setRouter("Dlink DIR835-A1");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "7");
	nvram_default_get("ath1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WNDR4300
	setRouter("Netgear WNDR4300");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "7");
	nvram_default_get("ath1_txantenna", "7");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WNDR3700V4
	setRouter("Netgear WNDR3700 V4");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_DIR825C1
	setRouter("Dlink DIR825-C1");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WASP
	setRouter("Atheros Wasp");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
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
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WHRG300NV2
#ifdef HAVE_BUFFALO
	setRouter("WHR-G300N");
#else
	setRouter("Buffalo WHR-G300N");
#endif
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_WHRHPGN
#ifdef HAVE_BUFFALO
	setRouter("WHR-HP-GN");
#else
	setRouter("Buffalo WHR-HP-GN");
#endif
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	return ROUTER_BOARD_WHRHPGN;
#elif HAVE_JJAP93
	setRouter("JJPLUS AP93");
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	return ROUTER_BOARD_PB42;
#elif HAVE_JJAP005
	setRouter("JJPLUS AP005");
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	return ROUTER_BOARD_PB42;
#elif HAVE_JJAP501
	setRouter("JJPLUS AP501");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_PB42;
#elif HAVE_AC722
	setRouter("ACCTON AC722");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_PB42;
#elif HAVE_AC622
	setRouter("ACCTON AC622");
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	return ROUTER_BOARD_PB42;
#elif HAVE_WPE72
	setRouter("Compex WPE72");
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	return ROUTER_BOARD_NS5M;
#elif HAVE_UBNTM
	typedef struct UBNTDEV {
		char *devicename;	// device name 
		unsigned short devid;	// pci subdevice id
		char *rxchain;	// rx chainmask
		char *txchain;	// tx chainmask
		int dddev;	// dd-wrt device id
		int offset;	// frequency offset
		int poffset;
	};

	/* these values are guessed and need to be validated */
#define M900 (- (2427 - 907))
#define M365 (- (5540 - 3650))
#define M35 (- (5540 - 3540))
#define M10 (- (5540 - 10322))
	struct UBNTDEV dev[] = {
		{"Ubiquiti NanoStation M2", 0xe002, "3", "3", ROUTER_BOARD_NS2M, 0, 10},	//
		{"Ubiquiti NanoStation M2", 0xe012, "3", "3", ROUTER_BOARD_NS2M, 0, 10},	//
		{"Ubiquiti NanoStation M5", 0xe005, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti NanoStation M6", 0xe006, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti NanoStation M5", 0xe805, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti NanoStation M3", 0xe035, "3", "3", ROUTER_BOARD_NS5M, M35, 10},	//
		{"Ubiquiti NanoStation M365", 0xe003, "3", "3", ROUTER_BOARD_NS5M, M365, 10},	//
		{"Ubiquiti NanoStation M900", 0xe2b9, "3", "3", ROUTER_BOARD_NS5M, M900, 10},	//
//              {"Ubiquiti NanoStation M900 GPS", 0xe0b9, "3", "3", ROUTER_BOARD_NS5M, M900},       //
		{"Ubiquiti Rocket M2", 0xe102, "3", "3", ROUTER_BOARD_R2M, 0, 10},	//
		{"Ubiquiti Rocket M2", 0xe112, "3", "3", ROUTER_BOARD_R2M, 0, 10},	//
		{"Ubiquiti Rocket M2", 0xe1b2, "3", "3", ROUTER_BOARD_R2M, 0, 10},	//
		{"Ubiquiti Rocket M2", 0xe1c2, "3", "3", ROUTER_BOARD_R2M, 0, 10},	//
		{"Ubiquiti Rocket M2 Titanium", 0xe1d2, "3", "3", ROUTER_BOARD_R2M, 0, 10},	// Titanium
		{"Ubiquiti Rocket M5", 0xe105, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti Rocket M5", 0xe1b5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti Rocket M5", 0xe8b5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti Rocket M5", 0xe1c5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti Airbeam 5", 0xe1e5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti Rocket M5 Titanium", 0xe1d5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	// Titanium
		{"Ubiquiti Bullet M2 Titanium", 0xe2d2, "3", "3", ROUTER_BOARD_R2M, 0, 10},	// Titanium
		{"Ubiquiti Bullet M5 Titanium", 0xe2d5, "3", "3", ROUTER_BOARD_R2M, 0, 10},	// Titanium
		{"Ubiquiti Rocket M3", 0xe1c3, "3", "3", ROUTER_BOARD_R5M, M35, 10},	//
		{"Ubiquiti Rocket M3", 0xe1e3, "3", "3", ROUTER_BOARD_R5M, M35, 10},	//
		{"Ubiquiti Rocket M5 X3", 0xe3b5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti Rocket M365", 0xe1b3, "3", "3", ROUTER_BOARD_R5M, M365, 10},	//
		{"Ubiquiti Rocket M365", 0xe1d3, "3", "3", ROUTER_BOARD_R5M, M365, 10},	//
		{"Ubiquiti Rocket M900", 0xe1b9, "3", "3", ROUTER_BOARD_R2M, M900, 10},	//
		{"Ubiquiti Rocket M900", 0xe1d9, "3", "3", ROUTER_BOARD_R2M, M900, 10},	//
		{"Ubiquiti Bullet M2", 0xe202, "1", "1", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti Bullet M5", 0xe205, "1", "1", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti Airgrid M2", 0xe212, "1", "1", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti Airgrid M2", 0xe242, "1", "1", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti Airgrid M5", 0xe215, "1", "1", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti Airgrid M5", 0xe245, "1", "1", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti AirRouter", 0xe4a2, "3", "3", ROUTER_BOARD_NS2M, 0, 10},	//
		{"Ubiquiti AirRouter", 0xe4b2, "3", "3", ROUTER_BOARD_NS2M, 0, 10},	//
		{"Ubiquiti Pico M2", 0xe302, "1", "1", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti Pico M5", 0xe305, "1", "1", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti Airwire", 0xe405, "3", "3", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti Airwire", 0xe4a5, "3", "3", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti Loco M5", 0xe0a5, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti Loco M5", 0xe8a5, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti Loco M2", 0xe0a2, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
//              {"Ubiquiti Loco M2", 0xe8a2, "3", "3", ROUTER_BOARD_NS5M, 0},   //
		{"Ubiquiti Loco M900", 0xe009, "3", "3", ROUTER_BOARD_NS5M, M900, 10},	//
		{"Ubiquiti NanoStation M900 Sector", 0xe0b9, "3", "3", ROUTER_BOARD_NS5M, M900, 10},	//
		{"Ubiquiti LiteStation M25", 0xe115, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti LiteStation M5", 0xe2a5, "3", "3", ROUTER_BOARD_NS5M, 0, 10},	//
		{"Ubiquiti PowerAP N", 0xe402, "3", "3", ROUTER_BOARD_NS2M, 0, 10},	//
		{"Ubiquiti Simple AP", 0xe4a2, "3", "3", ROUTER_BOARD_R2M, 0, 10},	//
		{"Ubiquiti PowerBridge M3", 0xe2a3, "3", "3", ROUTER_BOARD_R5M, M35, 10},	//
		{"Ubiquiti PowerBridge M5", 0xe1a5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti PowerBridge M5 X3", 0xe3a5, "3", "3", ROUTER_BOARD_R5M, 0, 10},	//
		{"Ubiquiti PowerBridge M365", 0xe1a3, "3", "3", ROUTER_BOARD_R5M, M365, 10},	//
		{"Ubiquiti Rocket M10", 0xe110, "3", "3", ROUTER_BOARD_R5M, M10, 10},	// 
		{"Ubiquiti NanoBridge M3", 0xe243, "3", "3", ROUTER_BOARD_BS5M, M35, 10},	//
		{"Ubiquiti NanoBridge M365", 0xe233, "3", "3", ROUTER_BOARD_BS5M, M365, 10},	//
		{"Ubiquiti NanoBridge M900", 0xe239, "3", "3", ROUTER_BOARD_BS5M, M900, 10},	//
		{"Ubiquiti NanoBridge M5", 0xe235, "3", "3", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti NanoBridge M5", 0xe2b5, "3", "3", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti NanoBridge M2", 0xe232, "3", "3", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti 3G Station", 0xe6a2, "3", "3", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti 3G Station Professional", 0xe6b2, "3", "3", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti 3G Station Outdoor", 0xe6c2, "3", "3", ROUTER_BOARD_BS2M, 0, 10},	//
		{"Ubiquiti WispStation M5", 0xe345, "3", "3", ROUTER_BOARD_BS5M, 0, 10},	//
		{"Ubiquiti UniFi AP", 0xe502, "3", "3", ROUTER_BOARD_UNIFI, 0, 10},	//
		{NULL, 0, NULL, NULL, 0},	//
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
	FILE *fp = fopen("/dev/mtdblock5", "rb");	//open board config
	int device = 0;
	if (fp) {
		fseek(fp, 0x1006, SEEK_SET);
		unsigned short cal[128];
		fread(&cal[0], 1, 256, fp);
		fclose(fp);
		int calcnt = 0;
		while (((cal[calcnt] & 0xffff) != 0xffff)) {
			unsigned short reg = cal[calcnt++] & 0xffff;
			if (reg == 0x602c || reg == 0x502c) {
				calcnt++;
				device = cal[calcnt++] & 0xffff;
				break;
			} else {
				calcnt += 2;
			}
		}
	}
#endif
	int devcnt = 0;
	while (dev[devcnt].devicename != NULL) {
		if (dev[devcnt].devid == device) {
			nvram_default_get("ath0_rxantenna", dev[devcnt].rxchain);
			nvram_default_get("ath0_txantenna", dev[devcnt].txchain);
			if (dev[devcnt].offset) {
				char foff[32];
				sprintf(foff, "%d", dev[devcnt].offset);
				nvram_set("ath0_offset", foff);
			}
			if (dev[devcnt].poffset) {
				char poff[32];
				sprintf(poff, "%d", dev[devcnt].poffset);
				nvram_set("ath0_poweroffset", poff);
			}
			setRouter(dev[devcnt].devicename);
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
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
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
	nvram_default_get("ath0_rxantenna", "7");
	nvram_default_get("ath0_txantenna", "7");

	void *getUEnv(char *name);

	char *model = getUEnv("product");


#ifdef HAVE_BUFFALO
	if (!strcmp(model,"BHR-4GRV"))
	    setRouter("BHR-4GRV");
	else
	    setRouter("WZR-HP-G450H");
#else
	if (!strcmp(model,"BHR-4GRV"))
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
	nvram_default_get("ath0_rxantenna", "7");
	nvram_default_get("ath0_txantenna", "7");
	return ROUTER_BOARD_PB42;
#elif HAVE_WZRHPAG300NH
#ifdef HAVE_BUFFALO
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
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("Dlink-DIR-632A");
	return ROUTER_BOARD_PB42;
#elif HAVE_WNDR3700V2
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	setRouter("Netgear WNDR3700 v2/WNDR37AV v2/WNDR3800/WNDR38AV");
	return ROUTER_BOARD_PB42;
#elif HAVE_WNDR3700
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	setRouter("Netgear WNDR3700");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR825
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	setRouter("Dlink DIR-825");
	return ROUTER_BOARD_PB42;
#elif HAVE_TEW673GRU
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	setRouter("Trendnet TEW-673GRU");
	return ROUTER_BOARD_PB42;
#elif HAVE_WRT400
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	nvram_default_get("ath1_rxantenna", "3");
	nvram_default_get("ath1_txantenna", "3");
	setRouter("Linksys WRT400N");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615C1
	setRouter("D-Link DIR-615-C1");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR601A1
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("D-Link DIR-601-A1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841V8
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v8");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615I
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("D-Link DIR-615-I1");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615E1
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("D-Link DIR-615-E1");
	return ROUTER_BOARD_PB42;
#elif HAVE_DIR615E
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
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
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v5");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR840v1
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WR840N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_MR3220
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-MR3220");
	return ROUTER_BOARD_PB42;
#elif HAVE_MR3420
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-MR3420");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR841v7
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WR841ND v7");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR842
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WR842ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR740v1
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WR740N");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA801v1
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WA801ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA901v1
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
	setRouter("TP-Link TL-WA901ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR941v4
	setRouter("TP-Link TL-WR941ND v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR743
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WR743ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_MR3020
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-MR3020");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR703
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WR703N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR740V4
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WR740N v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR741V4
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WR741ND v4");
	return ROUTER_BOARD_PB42;
#elif HAVE_WA7510
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WA7510N v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR741
	nvram_default_get("ath0_rxantenna", "1");
	nvram_default_get("ath0_txantenna", "1");
	setRouter("TP-Link TL-WR741ND v1");
	return ROUTER_BOARD_PB42;
#elif HAVE_WR1043
	nvram_default_get("ath0_rxantenna", "7");
	nvram_default_get("ath0_txantenna", "7");
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
	nvram_default_get("ath0_rxantenna", "3");
	nvram_default_get("ath0_txantenna", "3");
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

	if (boardnum == 45 && nvram_match("boardtype", "0x0472")
	    && nvram_match("boardrev", "0x23") && nvram_match("parkid", "1")) {
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
	if (boardnum == 45 && nvram_match("boardtype", "0x04EC")
	    && nvram_match("boardrev", "0x1402")) {
		setRouter("Asus RT-N10");
		return ROUTER_ASUS_RTN10;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x0550")
	    && nvram_match("boardrev", "0x1102")) {
		setRouter("Asus RT-N10U");
		return ROUTER_ASUS_RTN10U;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x058e")
	    && nvram_match("boardrev", "0x1153")) {
		setRouter("Asus RT-N10+ rev D1");
		return ROUTER_ASUS_RTN10PLUSD1;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x0550")
	    && nvram_match("boardrev", "0x1442")) {
		setRouter("Asus RT-N53");
		return ROUTER_ASUS_RTN53;
	}

	if (boardnum == 0 && nvram_match("boardtype", "0xF5B2")
	    && nvram_match("boardrev", "0x1100")
	    && !nvram_match("pci/2/1/sb20in80and160hr5ghpo", "0")) {
		setRouter("Asus RT-N66U");
		return ROUTER_ASUS_RTN66;
	}

	if (nvram_match("boardnum", "1") && nvram_match("boardtype", "0x054d")
	    && nvram_match("boardrev", "0x1109")) {
		setRouter("NetCore NW715P");
		return ROUTER_NETCORE_NW715P;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x04CD")
	    && nvram_match("boardrev", "0x1201")) {
		setRouter("Asus RT-N12");
		return ROUTER_ASUS_RTN12;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x054D")
	    && nvram_match("boardrev", "0x1101")) {
		char *hwrev = nvram_safe_get("hardware_version");
		if (!strncmp(hwrev, "RTN12C1", 7))
			setRouter("Asus RT-N12C1");
		else
			setRouter("Asus RT-N12B");
		return ROUTER_ASUS_RTN12B;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x04cf")
	    && nvram_match("boardrev", "0x1218")) {
		setRouter("Asus RT-N16");
		return ROUTER_ASUS_RTN16;
	}

	if (nvram_match("boardtype", "0xa4cf")
	    && nvram_match("boardrev", "0x1100")) {
		setRouter("Belkin F5D8235-4 v3");
		return ROUTER_BELKIN_F5D8235V3;
	}

	if (nvram_match("boardtype", "0xd4cf")
	    && nvram_match("boardrev", "0x1204")) {
		setRouter("Belkin F7D4301 / F7D8301 v1");
		return ROUTER_BELKIN_F7D4301;
	}

	if (nvram_match("boardtype", "0xa4cf")
	    && nvram_match("boardrev", "0x1102")) {
		FILE *mtd1 = fopen("/dev/mtdblock/1", "rb");
		unsigned long trxhd;
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
	if (nvram_match("boardnum", "00") && nvram_match("boardtype", "0x0101")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WBR2-G54 / WBR2-G54S");
		return ROUTER_BUFFALO_WBR2G54S;
	}

	if (boardnum == 2 && nvram_match("boardtype", "0x0101")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WLA2-G54C / WLI3-TX1-G54");
		return ROUTER_BUFFALO_WLA2G54C;
	}
	if (boardnum == 0 && melco_id == 29090 && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WLAH-G54");
		return ROUTER_BUFFALO_WLAH_G54;

	}
	if (boardnum == 0 && melco_id == 31070 && nvram_match("boardflags", "0x2288")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Buffalo WAPM-HP-AM54G54");
		return ROUTER_BUFFALO_WAPM_HP_AM54G54;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x11")
	    && nvram_match("boardtype", "0x048e") && melco_id == 32093) {
		setRouter("Buffalo WHR-G125");
		return ROUTER_BUFFALO_WHRG54S;
	}

	if (nvram_match("boardnum", "0x5347") && nvram_match("boardrev", "0x11")
	    && nvram_match("boardtype", "0x048e")) {
		setRouter("Huawei B970b");
		return ROUTER_HUAWEI_B970B;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x10")
	    && nvram_match("boardtype", "0x048e") && melco_id == 32139) {
		setRouter("Buffalo WCA-G");
		return ROUTER_BUFFALO_WCAG;	//vlan1 is lan, vlan0 is unused, implementation not done. will me made after return to germany
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x11")
	    && nvram_match("boardtype", "0x048e") && melco_id == 32064) {
		setRouter("Buffalo WHR-HP-G125");
		return ROUTER_BUFFALO_WHRG54S;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x13")
	    && nvram_match("boardtype", "0x467")) {
		if (nvram_match("boardflags", "0x1658")
		    || nvram_match("boardflags", "0x2658")
		    || nvram_match("boardflags", "0x3658")) {
			setRouter("Buffalo WLI-TX4-G54HP");
			return ROUTER_BUFFALO_WLI_TX4_G54HP;
		}
		if (!nvram_match("buffalo_hp", "1")
		    && nvram_match("boardflags", "0x2758")) {
			setRouter("Buffalo WHR-G54S");
			return ROUTER_BUFFALO_WHRG54S;
		}
		if (nvram_match("buffalo_hp", "1")
		    || nvram_match("boardflags", "0x1758")) {
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

	if (nvram_match("boardnum", "00") && nvram_match("boardrev", "0x10")
	    && nvram_match("boardtype", "0x470")) {
		setRouter("Buffalo WHR-AM54G54");
		return ROUTER_BUFFALO_WHRAM54G54;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x042f")) {

		if (nvram_match("product_name", "WZR-RS-G54")
		    || melco_id == 30083) {
			setRouter("Buffalo WZR-RS-G54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WZR-HP-G54")
		    || melco_id == 30026) {
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
		if (nvram_match("product_name", "WHR3-AG54")
		    || nvram_match("product_name", "WHR3-B11")
		    || melco_id == 29130) {
			setRouter("Buffalo WHR3-AG54");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WVR-G54-NF")
		    || melco_id == 28100) {
			setRouter("Buffalo WVR-G54-NF");
			return ROUTER_BUFFALO_WZRRSG54;
		}
		if (nvram_match("product_name", "WZR-G108") || melco_id == 31095 || melco_id == 30153) {
			setRouter("Buffalo WZR-G108");
			return ROUTER_BRCM4702_GENERIC;
		}
		if (melco_id > 0)	// e.g. 29115
		{
			setRouter("Buffalo WZR series");
			return ROUTER_BUFFALO_WZRRSG54;
		}
	}
#ifndef HAVE_BUFFALO
	if (boardnum == 42 && nvram_match("boardtype", "0x042f")
	    && nvram_match("boardrev", "0x10"))
		// nvram_match ("boardflags","0x0018"))
	{
		setRouter("Linksys WRTSL54GS");
		return ROUTER_WRTSL54GS;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x0101")
	    && nvram_match("boardrev", "0x10")
	    && nvram_match("boot_ver", "v3.6")) {
		setRouter("Linksys WRT54G3G");
		return ROUTER_WRT54G3G;
	}

	if (nvram_match("boardtype", "0x042f")
	    && nvram_match("boardrev", "0x10")) {
		char *hwver = nvram_safe_get("hardware_version");

		if (boardnum == 45 || startswith(hwver, "WL500gp")
		    || startswith(hwver, "WL500gH")) {
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

	if (nvram_match("boardtype", "bcm94710r4")
	    && nvram_match("boardnum", "")) {
		setRouter("Askey board RT2100W-D65)");
		return ROUTER_BRCM4702_GENERIC;
	}

	if (boardnum == 0 && nvram_match("boardtype", "0x0100")
	    && nvram_match("boardrev", "0x10")) {
		if (startswith(et0, "00:11:50") || startswith(et0, "00:30:BD") || startswith(et0, "00:30:bd")) {
			setRouter("Askey board RT2205(6)D-D56");
		} else {
			setRouter("Belkin board F5D8230");
		}
		return ROUTER_ASKEY_RT220XD;
	}

	if (nvram_match("boardtype", "0x0101")) {
		if (startswith(et0, "00:11:50") || startswith(et0, "00:30:BD") || startswith(et0, "00:30:bd")) {
			if (nvram_match("Belkin_ver", "2000")) {
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
	if (boardnum == 1 && nvram_match("boardtype", "0x456")
	    && nvram_match("test_led_gpio", "2")) {
		setRouter("Belkin F5D7230-4 v3000");
		return ROUTER_BELKIN_F5D7230_V3000;
	}

	if (nvram_match("boardtype", "0x456")
	    && nvram_match("hw_model", "F5D7231-4")) {
		setRouter("Belkin F5D7231-4 v1212UK");
		return ROUTER_BELKIN_F5D7231;
	}

	if (boardnum == 8 && nvram_match("boardtype", "0x0467"))	// fccid:
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
	if (boardnum == 2 && nvram_match("boardtype", "bcm94710dev") && melco_id == 29016)	// Buffalo 
		// WLI2-TX1-G54)
	{
		setRouter("Buffalo WLI2-TX1-G54");
		return ROUTER_BUFFALO_WLI2_TX1_G54;
	}
#ifndef HAVE_BUFFALO

	char *gemtek = nvram_safe_get("GemtekPmonVer");
	unsigned long gemteknum = strtoul(gemtek, NULL, 0);

	if (boardnum == 2 && (gemteknum == 10 || gemteknum == 11) &&
	    (startswith(et0, "00:0C:E5") ||
	     startswith(et0, "00:0c:e5") || startswith(et0, "00:11:22") || startswith(et0, "00:0C:10") || startswith(et0, "00:0c:10") || startswith(et0, "00:0C:11") || startswith(et0, "00:0c:11"))) {
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

	if (gemteknum == 9)	// Must be Motorola wr850g v1 or we800g v1 or 
		// Linksys wrt55ag v1
	{
		if (startswith(et0, "00:0C:E5") ||
		    startswith(et0, "00:0c:e5") ||
		    startswith(et0, "00:0C:10") ||
		    startswith(et0, "00:0c:10") || startswith(et0, "00:0C:11") || startswith(et0, "00:0c:11") || startswith(et0, "00:11:22") || startswith(et0, "00:0C:90") || startswith(et0, "00:0c:90")) {
			if (!strlen(nvram_safe_get("phyid_num"))) {
				insmod("switch-core");	// get phy type
				insmod("switch-robo");
				rmmod("switch-robo");
				rmmod("switch-core");
				nvram_set("boardnum", "2");
				nvram_set("boardtype", "bcm94710dev");
			}
			if (nvram_match("phyid_num", "0x00000000")) {
				setRouter("Motorola WE800G v1");
				return ROUTER_MOTOROLA_WE800G;
			} else	// phyid_num == 0xffffffff
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
	if (boardnum == 0 && nvram_match("boardtype", "0x478")
	    && nvram_match("cardbus", "0") && nvram_match("boardrev", "0x10")
	    && nvram_match("boardflags", "0x110") && melco_id == 32027) {
		setRouter("Buffalo WZR-G144NH");
		return ROUTER_BUFFALO_WZRG144NH;
	}

	if (boardnum == 20060330 && nvram_match("boardtype", "0x0472")) {
		setRouter("Buffalo WZR-G300N");
		return ROUTER_BUFFALO_WZRG300N;
	}
#ifndef HAVE_BUFFALO

	if (boardnum == 8 && nvram_match("boardtype", "0x0472")
	    && nvram_match("cardbus", "1")) {
		setRouter("Netgear WNR834B");
		return ROUTER_NETGEAR_WNR834B;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x0472")
	    && nvram_match("boardrev", "0x23")) {
		if (nvram_match("cardbus", "1")) {
			setRouter("Netgear WNR834B v2");
			return ROUTER_NETGEAR_WNR834BV2;
		} else {
			setRouter("Netgear WNDR3300");
			return ROUTER_NETGEAR_WNDR3300;
		}
	}

	if (boardnum == 42)	// Get Linksys N models
	{
		if (nvram_match("boot_hw_model", "WRT300N")
		    && nvram_match("boot_hw_ver", "1.1")) {
			setRouter("Linksys WRT300N v1.1");
			return ROUTER_WRT300NV11;
		} else if (nvram_match("boot_hw_model", "WRT150N")
			   && nvram_match("boot_hw_ver", "1")) {
			setRouter("Linksys WRT150N v1");
			return ROUTER_WRT150N;
		} else if (nvram_match("boot_hw_model", "WRT150N")
			   && nvram_match("boot_hw_ver", "1.1")) {
			setRouter("Linksys WRT150N v1.1");
			return ROUTER_WRT150N;
		} else if (nvram_match("boot_hw_model", "WRT150N")
			   && nvram_match("boot_hw_ver", "1.2")) {
			setRouter("Linksys WRT150N v1.2");
			return ROUTER_WRT150N;
		} else if (nvram_match("boot_hw_model", "WRT160N")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys WRT160N");
			return ROUTER_WRT160N;
		} else if (nvram_match("boot_hw_model", "WRT160N")
			   && nvram_match("boot_hw_ver", "3.0")) {
			setRouter("Linksys WRT160N v3");
			return ROUTER_WRT160NV3;
		} else if (nvram_match("boot_hw_model", "M10")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Cisco Valet M10 v1");	// renamed wrt160nv3
			return ROUTER_WRT160NV3;
		} else if (nvram_match("boot_hw_model", "E100")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1000 v1");	// renamed wrt160nv3
			return ROUTER_WRT160NV3;
		} else if (nvram_match("boot_hw_model", "E800")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E800");
			return ROUTER_LINKSYS_E800;
		} else if (nvram_match("boot_hw_model", "E900")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E900");
			return ROUTER_LINKSYS_E900;
		} else if (nvram_match("boot_hw_model", "E1000")
			   && nvram_match("boot_hw_ver", "2.0")) {
			setRouter("Linksys E1000 v2");
			return ROUTER_LINKSYS_E1000V2;
		} else if (nvram_match("boot_hw_model", "E1000")
			   && nvram_match("boot_hw_ver", "2.1")) {
			setRouter("Linksys E1000 v2.1");
			return ROUTER_LINKSYS_E1000V2;
		} else if (nvram_match("boot_hw_model", "E1200")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1200 v1");
			return ROUTER_LINKSYS_E1500;
		} else if (nvram_match("boot_hw_model", "E1200")
			   && nvram_match("boot_hw_ver", "2.0")) {
			setRouter("Linksys E1200 v2");
			return ROUTER_LINKSYS_E900;
		} else if (nvram_match("boot_hw_model", "E1500")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1500");
			return ROUTER_LINKSYS_E1500;
		} else if (nvram_match("boot_hw_model", "E1550")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E1550");
			return ROUTER_LINKSYS_E1550;
		} else if (nvram_match("boot_hw_model", "WRT310N")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys WRT310N");
			return ROUTER_WRT310N;
		} else if (nvram_match("boot_hw_model", "WRT310N")
			   && nvram_match("boot_hw_ver", "2.0")) {
			setRouter("Linksys WRT310N v2");
			return ROUTER_WRT310NV2;
		} else if (nvram_match("boot_hw_model", "M20")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Cisco Valet Plus M20");	// ranamed wrt310nv2
			return ROUTER_WRT310NV2;
		} else if (nvram_match("boot_hw_model", "E2500")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E2500");
			return ROUTER_LINKSYS_E2500;
		} else if (nvram_match("boot_hw_model", "E3200")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E3200");
			return ROUTER_LINKSYS_E3200;
		} else if (nvram_match("boot_hw_model", "E4200")
			   && nvram_match("boot_hw_ver", "1.0")) {
			setRouter("Linksys E4200");
			return ROUTER_LINKSYS_E4200;
		}
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x0472")
	    && nvram_match("cardbus", "1")) {
		setRouter("Linksys WRT300N v1");
		return ROUTER_WRT300N;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x478") && nvram_match("cardbus", "1")) {
		setRouter("Linksys WRT350N");
		return ROUTER_WRT350N;
	}

	if (nvram_match("boardnum", "20070615") && nvram_match("boardtype", "0x478") && nvram_match("cardbus", "0")) {
		if (nvram_match("switch_type", "BCM5395")) {
			setRouter("Linksys WRT600N v1.1");
			return ROUTER_WRT600N;
		} else {
			setRouter("Linksys WRT600N");
			return ROUTER_WRT600N;
		}
	}

	if (nvram_match("boardtype", "0x478")
	    && nvram_match("boot_hw_model", "WRT610N")) {
		setRouter("Linksys WRT610N");
		return ROUTER_WRT610N;
	}
#ifdef HAVE_BCMMODERN
	if (nvram_match("boardtype", "0x04cf")
	    && nvram_match("boot_hw_model", "WRT610N")) {
		setRouter("Linksys WRT610N v2");
		return ROUTER_WRT610NV2;
	}

	if (nvram_match("boardtype", "0x04cf")
	    && nvram_match("boot_hw_model", "E300")) {
		setRouter("Linksys E3000");	// renamed wrt610nv2
		return ROUTER_WRT610NV2;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x052b")
	    && nvram_match("boardrev", "0x1204")) {
		setRouter("Linksys EA2700");	// renamed wrt610nv2
		return ROUTER_LINKSYS_EA2700;
	}
#endif

	if (boardnum == 42 && nvram_match("boardtype", "bcm94710dev")) {
		setRouter("Linksys WRT54G v1.x");
		return ROUTER_WRT54G1X;
	}

	if ((boardnum == 1 || boardnum == 0)
	    && nvram_match("boardtype", "0x0446")) {
		setRouter("U.S.Robotics USR5430");
		return ROUTER_USR_5430;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x456")
	    && nvram_match("test_led_gpio", "0")) {
		setRouter("Netgear WG602 v3");
		return ROUTER_NETGEAR_WG602_V3;
	}

	if (boardnum == 10496 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5461");
		return ROUTER_USR_5461;
	}

	if (boardnum == 10500 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5432");
		return ROUTER_USR_5461;	// should work in the same way
	}

	if (boardnum == 10506 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5451");
		return ROUTER_USR_5461;	// should work in the same way
	}

	if (boardnum == 10512 && nvram_match("boardtype", "0x456")) {
		setRouter("U.S.Robotics USR5441");
		return ROUTER_USR_5461;	// should work in the same way
	}

	if ((boardnum == 35324 || boardnum == 38256)
	    && nvram_match("boardtype", "0x048e")) {
		setRouter("U.S.Robotics USR5465");
		return ROUTER_USR_5465;
	}

	if (boardnum == 35334 && nvram_match("boardtype", "0x048e")) {
		setRouter("U.S.Robotics USR5455");
		return ROUTER_USR_5465;	// should work in the same way
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

	if (boardnum == 44 && (nvram_match("boardtype", "0x0101")
			       || nvram_match("boardtype", "0x0101\r"))) {
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

	if (boardnum == 00 && nvram_match("boardtype", "0xF5B2")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("pci/2/1/sb20in80and160hr5ghpo", "0")) {
		setRouter("Asus RT-AC66U");
		return ROUTER_ASUS_AC66U;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0xF5B2")
	    && nvram_match("boardrev", "0x1100")
	    && nvram_match("pci/2/1/sb20in80and160hr5ghpo", "0")) {
		setRouter("Asus RT-AC66U");
		return ROUTER_ASUS_AC66U;
	}
	
	if (nvram_match("boardnum","${serno}") && nvram_match("boardtype","0xC617") && nvram_match("boardrev","0x1103")) {
		setRouter("Linksys EA6500");
		return ROUTER_LINKSYS_EA6500;
	}

	if (boardnum == 00 && nvram_match("boardtype", "0xf52e")
	    && nvram_match("boardrev", "0x1204")) {
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
			return ROUTER_D1800H;
		}
	}
#ifndef HAVE_BUFFALO
	if (boardnum == 0 && nvram_match("boardtype", "0x048e") &&	// cfe sets boardnum="", strtoul -> 0
	    nvram_match("boardrev", "0x35")) {
		setRouter("D-Link DIR-320");
		// apply some fixes
		if (nvram_get("vlan2ports") != NULL) {
			nvram_unset("vlan2ports");
			nvram_unset("vlan2hwname");
		}
		return ROUTER_DLINK_DIR320;
	}
	if (nvram_match("model_name", "DIR-330") && nvram_match("boardrev", "0x10")) {
		setRouter("D-Link DIR-330");
		nvram_set("wan_ifnames", "eth0");	// quirk
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
	if (boardnum == 42 && nvram_match("boardtype", "0x048e")
	    && nvram_match("boardrev", "0x10")) {
		if (nvram_match("boardflags", "0x20750")) {
			setRouter("Linksys WRT54G2 / GS2");	// router is wrt54g2v1/v1.3/gs2v1
		} else {
			setRouter("Linksys WRT54Gv8 / GSv7");
		}
		return ROUTER_WRT54G_V8;
	}

	if (boardnum == 8 && nvram_match("boardtype", "0x048e")
	    && nvram_match("boardrev", "0x11")) {
		setRouter("ALLNET EUROWRT 54");	//ALLNET01
		return ROUTER_ALLNET01;
	}

	if (boardnum == 01 && nvram_match("boardtype", "0x048e")
	    && nvram_match("boardrev", "0x11")
	    && (nvram_match("boardflags", "0x650")
		|| nvram_match("boardflags", "0x0458"))) {
		setRouter("Netgear WG602 v4");
		return ROUTER_NETGEAR_WG602_V4;
	}

	if (boardnum == 1 && nvram_match("boardtype", "0x048e")
	    && nvram_match("boardrev", "0x35")
	    && nvram_match("parefldovoltage", "0x28")) {
		setRouter("NetCore NW618 / Rosewill RNX-GX4");
		return ROUTER_NETCORE_NW618;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x048E")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Linksys WRH54G");
		return ROUTER_LINKSYS_WRH54G;
	}

	if (nvram_match("boardnum", "00") && nvram_match("boardtype", "0x048E")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Linksys WRT54G v8.1");
		return ROUTER_WRT54G_V81;
	}

	if (boardnum == 45 && nvram_match("boardtype", "0x456")) {
		setRouter("Asus WL-520G");
		return ROUTER_ASUS_WL520G;
	}

	if (nvram_match("boardtype", "0x48E")
	    && nvram_match("boardrev", "0x10")) {
		char *hwver = nvram_safe_get("hardware_version");

		if (boardnum == 45 && startswith(hwver, "WL500GPV2")) {
			setRouter("Asus WL-500G Premium v2");
			return ROUTER_ASUS_WL500G_PRE_V2;
		} else if (boardnum == 45 && startswith(hwver, "WL330GE")) {
			setRouter("Asus WL-330GE");
			return ROUTER_ASUS_330GE;
		} else if (boardnum == 45 || startswith(hwver, "WL500GU")
			   || startswith(hwver, "WL500GC")) {
			setRouter("Asus WL-520GU/GC");
			return ROUTER_ASUS_WL520GUGC;
		}
	}

	if ((boardnum == 83258 || boardnum == 1 || boardnum == 0123)	//or 01 or 001 or 0x01
	    && (nvram_match("boardtype", "0x048e") || nvram_match("boardtype", "0x48E")) && (nvram_match("boardrev", "0x11") || nvram_match("boardrev", "0x10")) && (nvram_match("boardflags", "0x750") || nvram_match("boardflags", "0x0750")) && nvram_match("sdram_init", "0x000A"))	//16 MB ram
	{
		setRouter("Netgear WGR614v8/L/WW");
		return ROUTER_NETGEAR_WGR614L;
	}

	if (boardnum == 3805 && nvram_match("boardtype", "0x48E")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Netgear WGR614v9");
		return ROUTER_NETGEAR_WGR614V9;
	}

	if (boardnum == 56 && nvram_match("boardtype", "0x456")
	    && nvram_match("boardrev", "0x10")) {
		setRouter("Linksys WTR54GS");
		return ROUTER_LINKSYS_WTR54GS;
	}

	if (nvram_match("boardnum", "WAP54GV3_8M_0614")
	    && (nvram_match("boardtype", "0x0467")
		|| nvram_match("boardtype", "0x467"))
	    && nvram_match("WAPver", "3")) {
		setRouter("Linksys WAP54G v3.x");
		return ROUTER_WAP54G_V3;
	}
#ifdef HAVE_BCMMODERN
	if (boardnum == 1 && nvram_match("boardtype", "0xE4CD")
	    && nvram_match("boardrev", "0x1700")) {
		setRouter("Netgear WNR2000 v2");
		return ROUTER_NETGEAR_WNR2000V2;
	}

	if ((boardnum == 1 || boardnum == 3500)
	    && nvram_match("boardtype", "0x04CF")
	    && (nvram_match("boardrev", "0x1213")
		|| nvram_match("boardrev", "02"))) {
		setRouter("Netgear WNR3500 v2/U/L v1");
		return ROUTER_NETGEAR_WNR3500L;
	}

	if (nvram_match("boardnum", "01") && nvram_match("boardtype", "0xb4cf")
	    && nvram_match("boardrev", "0x1100")) {
		setRouter("Netgear WNDR3400");
		return ROUTER_NETGEAR_WNDR3400;
	}

	if (nvram_match("boardnum", "01") && nvram_match("boardtype", "0xF52C")
	    && nvram_match("boardrev", "0x1101")) {

		int mtd = getMTD("board_data");
		char devname[32];
		sprintf(devname, "/dev/mtdblock/%d", mtd);
		FILE *model = fopen(devname, "rb");
		if (model) {
#define WNDR3700V3 "U12H194T00_NETGEAR"
			char modelstr[32];
			fread(modelstr, 1, strlen(WNDR3700V3), model);
			if (!strncmp(modelstr, WNDR3700V3, strlen(WNDR3700V3))) {
				fclose(model);
				setRouter("Netgear WNDR3700v3");
				return ROUTER_NETGEAR_WNDR4000;
			}
			fclose(model);
		}
		setRouter("Netgear WNDR4000");
		return ROUTER_NETGEAR_WNDR4000;
	}

	if (nvram_match("boardnum", "4536")
	    && nvram_match("boardtype", "0xf52e")
	    && nvram_match("boardrev", "0x1102")) {
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
	
	if (nvram_match("boardnum", "679") && nvram_match("boardtype", "0x0646")
            && nvram_match("boardrev", "0x1110")) { 
                setRouter("Netgear R6250");
                return ROUTER_NETGEAR_R6250;
        }	

	if ((boardnum == 42 || boardnum == 66)
	    && nvram_match("boardtype", "0x04EF")
	    && (nvram_match("boardrev", "0x1304")
		|| nvram_match("boardrev", "0x1305"))) {
		setRouter("Linksys WRT320N");
		return ROUTER_WRT320N;
	}

	if (boardnum == 42 && nvram_match("boardtype", "0x04EF")
	    && nvram_match("boardrev", "0x1307")) {
		setRouter("Linksys E2000");	// renamed (and fixed reset button) wrt320n
		return ROUTER_WRT320N;
	}
#endif

	if (boardnum == 94703 && nvram_match("boardtype", "0x04c0")
	    && nvram_match("boardrev", "0x1100")) {
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
#ifdef HAVE_PPPOA
	     || nvram_match("wan_proto", "pppoa")
#endif
#ifdef HAVE_3G
	     || (nvram_match("wan_proto", "3g") && !nvram_match("3gdata", "hso")
		 && !nvram_match("3gdata", "qmi"))
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
#if defined(HAVE_LIBQMI) || defined(HAVE_UQMI)
	else if (nvram_match("wan_proto", "3g") && nvram_match("3gdata", "qmi")) {
		FILE *fp = fopen("/tmp/qmistatus", "rb");
		int value = 0;
		if (fp) {
			fscanf(fp, "%d", &value);
			fclose(fp);
		}
#ifdef HAVE_UQMI
		if (value)
			return 1;
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
					memset(&ifr, 0, sizeof(struct ifreq));
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

#if defined(HAVE_BUFFALO) || defined(HAVE_BUFFALO_BL_DEFAULTS) || defined(HAVE_WMBR_G300NH) || defined(HAVE_WZRG450)
void *getUEnv(char *name)
{
#ifdef HAVE_WZRG300NH
#define UOFFSET 0x40000
#elif HAVE_WZRHPAG300NH
#define UOFFSET 0x40000
#elif HAVE_WZRG450
#define UOFFSET 0x40000
#elif HAVE_WMBR_G300NH
#define UOFFSET 0x0
#else
#define UOFFSET 0x3E000
#endif
//      static char res[64];
	static char res[256];
	memset(res, 0, sizeof(res));
	//fprintf(stderr,"[u-boot env]%s\n",name);
#ifdef HAVE_WMBR_G300NH
	FILE *fp = fopen("/dev/mtdblock/1", "rb");
#else
	FILE *fp = fopen("/dev/mtdblock/0", "rb");
#endif
	fseek(fp, UOFFSET, SEEK_SET);
	char *mem = safe_malloc(0x2000);
	fread(mem, 0x2000, 1, fp);
	fclose(fp);
	int s = (0x2000 - 1) - strlen(name);
	int i;
	int l = strlen(name);
	for (i = 0; i < s; i++) {
		if (!strncmp(mem + i, name, l)) {
			strncpy(res, mem + i + l + 1, sizeof(res) - 1);
			free(mem);
			return res;
		}
	}
	free(mem);
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

int diag_led_4702(int type, int act)
{

#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_FONERA) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else
	if (act == START_LED) {
		switch (type) {
		case DMZ:
			writeproc("/proc/sys/diag", "1");
			break;
		}
	} else {
		switch (type) {
		case DMZ:
			writeproc("/proc/sys/diag", "0");
			break;
		}
	}
	return 0;
#endif
}

int C_led_4702(int i)
{
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE)  || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else
	FILE *fp;
	char string[10];
	int flg;

	memset(string, 0, 10);
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

	memset(string, 0, 10);
	sprintf(string, "%d", flg);
	writeproc("/proc/sys/diag", string);

	return 0;
#endif
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

static char hw_error = 0;
int diag_led_4704(int type, int act)
{
#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_MERAKI)|| defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else
	unsigned int control, in, outen, out;

#ifdef BCM94712AGR
	/*
	 * The router will crash, if we load the code into broadcom demo board. 
	 */
	return 1;
#endif
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

int diag_led_4712(int type, int act)
{
	unsigned int control, in, outen, out, ctr_mask, out_mask;

#if defined(HAVE_GEMTEK) || defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA)|| defined(HAVE_MERAKI) || defined(HAVE_LS2) || defined(HAVE_WHRAG108) || defined(HAVE_X86) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_OPENRISC)
	return 0;
#else

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

int C_led_4712(int i)
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

#ifdef HAVE_MADWIFI
static char *stalist[] = {
	"ath0", "ath1", "ath2", "ath3", "ath4", "ath5", "ath6", "ath8", "ath9"
};

char *getWifi(char *ifname)
{
	if (!strncmp(ifname, "ath0", 4))
		return "wifi0";
	if (!strncmp(ifname, "ath1", 4))
		return "wifi1";
	if (!strncmp(ifname, "ath2", 4))
		return "wifi2";
	if (!strncmp(ifname, "ath3", 4))
		return "wifi3";
	return NULL;
}

char *getWDSSTA(void)
{

	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char mode[32];
		char netmode[32];

		sprintf(mode, "ath%d_mode", i);
		sprintf(netmode, "ath%d_net_mode", i);
		if (nvram_match(mode, "wdssta")
		    && !nvram_match(netmode, "disabled")) {
			return stalist[i];
		}

	}
	return NULL;
}

char *getSTA(void)
{

#ifdef HAVE_WAVESAT
	if (nvram_match("ofdm_mode", "sta"))
		return "ofdm";
#endif
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("sta", "ath%d_mode", i)
		    && !nvram_nmatch("disabled", "ath%d_net_mode", i)) {
			return stalist[i];
		}

	}
	return NULL;
}

char *getWET(void)
{
#ifdef HAVE_WAVESAT
	if (nvram_match("ofdm_mode", "bridge"))
		return "ofdm";
#endif
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("wet", "ath%d_mode", i)
		    && !nvram_nmatch("disabled", "ath%d_net_mode", i)) {
			return stalist[i];
		}

	}
	return NULL;
}

#elif defined(HAVE_RT2880) || defined(HAVE_RT61)

char *getSTA()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("sta", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i))
				return "ra0";
		}

		if (nvram_nmatch("apsta", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i))
				return "apcli0";
		}

	}
	return NULL;
}

char *getWET()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (!nvram_nmatch("disabled", "wl%d_net_mode", i)
		    && nvram_nmatch("wet", "wl%d_mode", i))
			return "ra0";

		if (!nvram_nmatch("disabled", "wl%d_net_mode", i)
		    && nvram_nmatch("apstawet", "wl%d_mode", i))
			return "apcli0";

	}
	return NULL;
}

#else
char *getSTA()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("sta", "wl%d_mode", i)
		    || nvram_nmatch("apsta", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i))
				return get_wl_instance_name(i);
			// else
			// return nvram_nget ("wl%d_ifname", i);
		}

	}
	return NULL;
}

char *getWET()
{
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		if (nvram_nmatch("wet", "wl%d_mode", i)
		    || nvram_nmatch("apstawet", "wl%d_mode", i)) {
			if (!nvram_nmatch("disabled", "wl%d_net_mode", i))
				return get_wl_instance_name(i);
			// else
			// return nvram_nget ("wl%d_ifname", i);

		}

	}
	return NULL;
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
	    || nvram_match("wan_proto", "pppoe")) {
		if (nvram_match("pppd_pppifname", ""))
			strncpy(localwanface, "ppp0", IFNAMSIZ);
		else
			strncpy(localwanface, nvram_safe_get("pppd_pppifname"), IFNAMSIZ);
	}
#ifdef HAVE_3G
	else if (nvram_match("wan_proto", "3g")) {
		if (nvram_match("3gdata", "qmi")) {
			strncpy(localwanface, "wwan0", IFNAMSIZ);
		} else {
			if (nvram_match("pppd_pppifname", ""))
				strncpy(localwanface, "ppp0", IFNAMSIZ);
			else
				strncpy(localwanface, nvram_safe_get("pppd_pppifname"), IFNAMSIZ);
		}

	}
#endif
#ifndef HAVE_MADWIFI
	else if (getSTA()) {
		strcpy(localwanface, getSTA());
	}
#else
	else if (getSTA()) {
		if (nvram_match("wifi_bonding", "1"))
			strcpy(localwanface, "bond0");
		else
			strcpy(localwanface, getSTA());
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

static int _pidof(const char *name, pid_t ** pids)
{
	const char *p;
	char *e;
	DIR *dir;
	struct dirent *de;
	pid_t i;
	int count;
	char buf[256];

	count = 0;
	*pids = NULL;
	if ((p = strchr(name, '/')) != NULL)
		name = p + 1;
	if ((dir = opendir("/proc")) != NULL) {
		while ((de = readdir(dir)) != NULL) {
			i = strtol(de->d_name, &e, 10);
			if (*e != 0)
				continue;
			if (strcmp(name, psname(i, buf, sizeof(buf))) == 0) {
				if ((*pids = realloc(*pids, sizeof(pid_t) * (count + 1))) == NULL) {
					return -1;
				}
				(*pids)[count++] = i;
			}
		}
	}
	closedir(dir);
	return count;
}

int pidof(const char *name)
{
	pid_t *pids;
	pid_t p;

	if (_pidof(name, &pids) > 0) {
		p = *pids;
		free(pids);
		return p;
	}
	return -1;
}

int killall(const char *name, int sig)
{
	pid_t *pids;
	int i;
	int r;

	if ((i = _pidof(name, &pids)) > 0) {
		r = 0;
		do {
			r |= kill(pids[--i], sig);
		}
		while (i > 0);
		free(pids);
		return r;
	}
	return -2;
}

void set_ip_forward(char c)
{
	FILE *fp;
	char ch[8];
	sprintf(ch, "%c", c);
	writeproc("/proc/sys/net/ipv4/ip_forward", ch);
}

int ifexists(const char *ifname)
{
	return getifcount(ifname) > 0 ? 1 : 0;
}

int getdevicecount(void)
{
	int count = 0;
#ifdef HAVE_ATH9K
	count += getath9kdevicecount();
#endif
	count += getifcount("wifi");

	return count;
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
	char *iflist = safe_malloc(256);

	memset(iflist, 0, 256);
	int c = getIfList(iflist, ifprefix);

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

static int gstrcmp(char *str1, char *str2)
{
	int l1 = strlen(str1);
	int l2 = strlen(str2);
	int maxlen = l1 > l2 ? l2 : l1;
	int i;
	for (i = 0; i < maxlen; i++) {
		if (str1[i] > str2[i])
			return 1;
		if (str1[i] < str2[i])
			return -1;
	}
	return 0;
}

// returns a physical interfacelist filtered by ifprefix. if ifprefix is
// NULL, all valid interfaces will be returned
int getIfList(char *buffer, const char *ifprefix)
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

			if (ifprefix) {
				if (strncmp(ifname, ifprefix, strlen(ifprefix))) {
					skip = 1;
				}
			} else {
				if (!strncmp(ifname, "wifi", 4))
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
			memset(ifname, 0, 32);
			skipline(in);
			continue;
		}
		if (ifcount < 30)
			ifname[ifcount++] = c;
	}
      sort:;
	int i;
	int a;
	for (a = 0; a < count; a++) {
		for (i = 0; i < count - 1; i++) {
			if (gstrcmp(sort[i], sort[i + 1]) > 0) {
				char *temp = sort[i + 1];
				sort[i + 1] = sort[i];
				sort[i] = temp;
			}
		}
	}
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
#ifdef HAVE_RB600
	strcpy(buf, "FreeScale MPC8343");
	return buf;
#elif HAVE_VENTANA
	strcpy(buf, "FreeScale i.MX6 Quad/DualLite");
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
 * LED_SES, LED_SES2, LED_WLAN0, LED_WLAN1, LED_SEC0, LED_SEC1, USB_POWER
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
	int wlan0_gpio = 0x0ff;	// use this only if wlan led is not controlled by hardware!
	int wlan1_gpio = 0x0ff;	// use this only if wlan led is not controlled by hardware!
	int usb_gpio = 0x0ff;
	int usb_gpio1 = 0x0ff;
	int sec0_gpio = 0x0ff;	// security leds, wrt600n
	int sec1_gpio = 0x0ff;
	int usb_power = 0x0ff;
	int usb_power1 = 0x0ff;
	int v1func = 0;
	int connblue = nvram_match("connblue", "1") ? 1 : 0;

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
		diag_gpio = 0x001;
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
#ifdef HAVE_WDR4900
	case ROUTER_BOARD_WDR4900:
		diag_gpio = 0x000;
		usb_gpio = 0x001;
		usb_gpio1 = 0x002;
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
#ifdef HAVE_WR841V8
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
		connected_gpio = 0x110;	// 16 is mapped to front led
		break;
	case ROUTER_BOARD_GATEWORX:
#ifdef HAVE_WG302V1
		diag_gpio = 0x104;
		wlan0_gpio = 0x105;
#elif HAVE_WG302
		diag_gpio = 0x102;
		wlan0_gpio = 0x104;
#else
		if (nvram_match("DD_BOARD", "Gateworks Cambria GW2350")
		    || nvram_match("DD_BOARD2", "Gateworks Cambria GW2350"))
			connected_gpio = 0x105;
		else if (nvram_match("DD_BOARD", "Gateworks Cambria GW2358-4")
			 || nvram_match("DD_BOARD2", "Gateworks Cambria GW2358-4"))
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
	case ROUTER_WHR300HP2:
		power_gpio = 0x109;
		diag_gpio = 0x107;
		wlan0_gpio = 0x108;
		sec0_gpio = 0x10a;
		ses_gpio = 0x10a;
		connected_gpio = 0x139;
		disconnected_gpio = 0x13b;
//		diag_gpio = 0x107;
//		connected_gpio = 0x109;
//		ses_gpio = 0x10e;
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
	case ROUTER_BOARD_WHRHPGN:
//		usb_power = 0x01a;
//		usb_gpio = 0x001;
//		ses_gpio = 0x11b;
		break;
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
		usb_power = 0x009;	// USB 2.0 ehci port
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
		power_gpio = 0x003;	//power led green
		diag_gpio = 0x007;	// power led amber
		ses_gpio = 0x001;	// WPS led green
		connected_gpio = 0x002;	//wan led green
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
	case ROUTER_DLINK_DIR868:
		usb_power = 0x00a;
		connected_gpio= 0x103;
		disconnected_gpio = 0x101;
		power_gpio = 0x102;
		diag_gpio = 0x100;
		break;
	case ROUTER_ASUS_AC67U:
	case ROUTER_ASUS_AC56U:
		usb_power = 0x009;	//usb power on/off
		usb_power1 = 0x00a;	//usb power on/off
		power_gpio = 0x103;
		usb_gpio = 0x10e;
		diag_gpio = 0x003;
		connected_gpio = 0x101;
		disconnected_gpio = 0x102;
		break;
	case ROUTER_NETGEAR_R6250:
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
		diag_gpio = 0x10c;
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
	if (type == LED_DIAG && v1func == 1) {
		if (act == LED_ON)
			C_led(1);
		else
			C_led(0);
	}

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
	case LED_WLAN0:
		use_gpio = wlan0_gpio;
		break;
	case LED_WLAN1:
		use_gpio = wlan1_gpio;
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
		switch (act) {
		case LED_ON:
			set_gpio(gpio_value, enable);
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

int file_to_buf(char *path, char *buf, int len)
{
	FILE *fp;

	memset(buf, 0, len);

	if ((fp = fopen(path, "r"))) {
		fgets(buf, len, fp);
		fclose(fp);
		return 1;
	}

	return 0;
}

int ishexit(char c)
{

	if (strchr("01234567890abcdefABCDEF", c) != (char *)0)
		return 1;

	return 0;
}

int getMTD(char *name)
{
	char buf[128];
	int device;

	sprintf(buf, "cat /proc/mtd|grep \"%s\"", name);
	FILE *fp = popen(buf, "rb");

	fscanf(fp, "%s", &buf[0]);
	device = buf[3] - '0';
	pclose(fp);
	return device;
}

int insmod(char *module)
{
	return eval("insmod", module);
}

void rmmod(char *module)
{
	eval("rmmod", module);
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
#define MD5_OUT_BUFSIZE 36

char *zencrypt(char *passwd)
{
	char salt[sizeof("$N$XXXXXXXX")];	/* "$N$XXXXXXXX" or "XX" */
	static char passout[MD5_OUT_BUFSIZE];

	strcpy(salt, "$1$");
	crypt_make_salt(salt + 3, 4, 0);
	strcpy(passout, crypt((unsigned char *)passwd, (unsigned char *)salt));
	return passout;
}

int has_gateway(void)
{
	if (nvram_match("wk_mode", "gateway"))
		return 1;
	if (nvram_match("wk_mode", "olsr") && nvram_match("olsrd_gateway", "1"))
		return 1;
	return 0;
}

#ifdef HAVE_ATH9K
int getath9kdevicecount(void)
{
	glob_t globbuf;
	int globresult;
	int count = 0;
#ifndef HAVE_MADWIFI_MIMO
	if (1) {
#else
	if (nvram_match("mimo_driver", "ath9k")) {
#endif
		globresult = glob("/sys/class/ieee80211/phy*", GLOB_NOSORT, NULL, &globbuf);
		if (globresult == 0)
			count = (int)globbuf.gl_pathc;
		globfree(&globbuf);
	}
	return (count);
}

int get_ath9k_phy_idx(int idx)
{
	// fprintf(stderr,"channel number %d of %d\n", i,achans.ic_nchans);
	return idx - getifcount("wifi");
}

int get_ath9k_phy_ifname(const char *ifname)
{
	int devnum;
	if (!sscanf(ifname, "ath%d", &devnum))
		return (0);
	// fprintf(stderr,"channel number %d of %d\n", i,achans.ic_nchans);
	return get_ath9k_phy_idx(devnum);
}

int is_ath9k(const char *prefix)
{
	glob_t globbuf;
	int count = 0;
	char globstring[1024];
	int globresult;
	int devnum;
	// get legacy interface count
#ifdef HAVE_MADWIFI_MIMO
	if (!nvram_match("mimo_driver", "ath9k"))
		return (0);
#endif
	// correct index if there are legacy cards arround
	devnum = get_ath9k_phy_ifname(prefix);
	sprintf(globstring, "/sys/class/ieee80211/phy%d", devnum);
	globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
	if (globresult == 0)
		count = (int)globbuf.gl_pathc;
	globfree(&globbuf);
	return (count);
}
#endif

double HTTxRate20_800(unsigned int index)
{
	static const double vHTTxRate20_800[24] = { 6.5, 13.0, 19.5, 26.0, 39.0, 52.0, 58.5, 65.0, 13.0, 26.0, 39.0,
		52.0, 78.0, 104.0, 117.0, 130.0,
		19.5, 39.0, 58.5, 78.0, 117.0, 156.0, 175.5, 195.0
	};
	if (index > sizeof(HTTxRate20_800) / sizeof(double) - 1) {
		fprintf(stderr, "utils.c HTTxRate20_800() index overflow\n");
		return 0.0;
	}
	return vHTTxRate20_800[index];
}

double HTTxRate20_400(unsigned int index)
{
	static const double vHTTxRate20_400[24] = { 7.2, 14.4, 21.7, 28.9, 43.3, 57.8, 65.0, 72.2, 14.444, 28.889,
		43.333, 57.778, 86.667, 115.556, 130.000, 144.444,
		21.7, 43.3, 65.0, 86.7, 130.0, 173.3, 195.0, 216.7
	};
	if (index > sizeof(vHTTxRate20_400) / sizeof(double) - 1) {
		fprintf(stderr, "utils.c HTTxRate20_400() index overflow\n");
		return 0.0;
	}
	return vHTTxRate20_400[index];
}

double HTTxRate40_800(unsigned int index)
{
	static const double vHTTxRate40_800[25] = { 13.5, 27.0, 40.5, 54.0, 81.0, 108.0, 121.5, 135.0, 27.0, 54.0,
		81.0, 108.0, 162.0, 216.0, 243.0, 270.0,
		40.5, 81.0, 121.5, 162.0, 243.0, 324.0, 364.5, 405.0, 6.0
	};
	if (index > sizeof(vHTTxRate40_800) / sizeof(double) - 1) {
		fprintf(stderr, "utils.c HTTxRate40_800() index overflow\n");
		return 0.0;
	}
	return vHTTxRate40_800[index];
}

double HTTxRate40_400(unsigned int index)
{
	static const double vHTTxRate40_400[25] = { 15.0, 30.0, 45.0, 60.0, 90.0, 120.0, 135.0, 150.0, 30.0, 60.0,
		90.0, 120.0, 180.0, 240.0, 270.0, 300.0,
		45.0, 90.0, 135.0, 180.0, 270.0, 360.0, 405.0, 450.0, 6.7
	};
	if (index > sizeof(vHTTxRate40_400) / sizeof(double) - 1) {
		fprintf(stderr, "utils.c HTTxRate40_400() index overflow\n");
		return 0.0;
	}
	return vHTTxRate40_400[index];
}

int writeproc(char *path, char *value)
{
	FILE *fp;
	fp = fopen(path, "wb");
	if (fp == NULL) {
		return -1;
	}
	fprintf(fp, value);
	fclose(fp);
	return 0;
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

/* gartarp */

struct arph {
	uint16_t hw_type;

#define ARPHDR_ETHER	1

	uint16_t proto_type;

	char ha_len;
	char pa_len;

#define ARPOP_BROADCAST	1
#define ARPOP_REPLY	2
	uint16_t opcode;
	char source_add[ETH_ALEN];
	char source_ip[IP_ALEN];
	char dest_add[ETH_ALEN];
	char dest_ip[IP_ALEN];

} __attribute__((packed));

#define ARP_HLEN	sizeof(struct arph) + ETH_HLEN
#define BCAST		"\xff\xff\xff\xff\xff\xff"

static inline int get_iface_attr(int sk, char *iface, char *hw, char *paddr)
{
	int ret;
	struct ifreq ifr;

	strcpy(ifr.ifr_name, iface);

	ret = ioctl(sk, SIOCGIFHWADDR, &ifr);
	if (unlikely(ret == -1)) {
		perror("ioctl SIOCGIFHWADDR");
		return ret;
	}
	memcpy(hw, ifr.ifr_hwaddr.sa_data, ETH_ALEN);

	ret = ioctl(sk, SIOCGIFADDR, &ifr);
	if (unlikely(ret == -1)) {
		perror("ioctl SIOCGIFADDR");
		return ret;
	}
	memcpy(paddr, ifr.ifr_addr.sa_data + 2, IP_ALEN);

	return ret;
}

static inline void setup_eth(struct ether_header *eth, char *hw_addr)
{
	memcpy(eth->ether_shost, hw_addr, ETH_ALEN);
	memcpy(eth->ether_dhost, BCAST, ETH_ALEN);
	eth->ether_type = htons(ETH_P_ARP);
}

static inline void setup_garp(struct arph *arp, char *hw_addr, char *paddr)
{
	arp->hw_type = htons(ARPHDR_ETHER);
	arp->proto_type = htons(ETH_P_IP);
	arp->ha_len = ETH_ALEN;
	arp->pa_len = IP_ALEN;

	memcpy(arp->source_add, hw_addr, ETH_ALEN);
	memcpy(arp->source_ip, paddr, IP_ALEN);
}

static inline void setup_garp_broadcast(struct arph *arp, char *paddr)
{
	arp->opcode = htons(ARPOP_BROADCAST);

	memset(arp->dest_add, 0, ETH_ALEN);
	memcpy(arp->dest_ip, paddr, IP_ALEN);
}

static inline void setup_garp_reply(struct arph *arp, char *hw_addr, char *paddr)
{
	arp->opcode = htons(ARPOP_REPLY);

	memcpy(arp->dest_add, hw_addr, ETH_ALEN);
	memcpy(arp->dest_ip, paddr, IP_ALEN);
}

/*
 * send_garp 
 *
 * - sends 20 gartuitous arps 
 * in a 200 millisec interval.
 * One as braadcast and one as reply.
 * 
 * 
 * parameter iface: sending interface name
 *
 * returns false on failure
 *         true on success 
 */
static int send_garp(char *iface)
{
	char pkt[ARP_HLEN];
	char iface_hw[ETH_ALEN];
	char iface_paddr[IP_ALEN];
	struct sockaddr_ll link;
	struct ether_header *eth;
	struct arph *arp;
	int rc;
	int sk;
	int n_garps = 10;

	eth = (struct ether_header *)pkt;
	arp = (struct arph *)(pkt + ETH_HLEN);

	sk = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
	if (unlikely(sk == -1)) {
		perror("socket");
		return sk;
	}

	rc = get_iface_attr(sk, iface, iface_hw, iface_paddr);
	if (unlikely(rc == -1))
		goto out;

	/* set link layer information for driver */
	memset(&link, 0, sizeof(link));
	link.sll_family = AF_PACKET;
	link.sll_ifindex = if_nametoindex(iface);

	setup_eth(eth, iface_hw);
	setup_garp(arp, iface_hw, iface_paddr);

	while (n_garps--) {
		setup_garp_broadcast(arp, iface_paddr);
		rc = sendto(sk, pkt, ARP_HLEN, 0, (struct sockaddr *)&link, sizeof(struct sockaddr_ll)
		    );
		if (unlikely(rc == -1)) {
			perror("sendto");
			goto out;
		}

		setup_garp_reply(arp, iface_hw, iface_paddr);
		rc = sendto(sk, pkt, ARP_HLEN, 0, (struct sockaddr *)&link, sizeof(struct sockaddr_ll)
		    );
		if (unlikely(rc == -1)) {
			perror("sendto");
			goto out;
		}
		usleep(200000);
	}

out:
	close(sk);
	return rc;
}

int gratarp_main(char *iface)
{
	if (iface) {
		usleep(500000);
		send_garp(iface);
	}

	return 0;
}

/* NF Mark/Mask
 *
 * since multiple services needs a NF packet mark,
 * we need to use masks to split the 32bit value into several pieces
 *
 *                                             31       23       15       7      0
 * port_forwards         1 bit(s) offset 31  > 10000000 00000000 00000000 00000000
 * hotspot				 8 bit(s) offset 23  > 01111111 10000000 00000000 00000000
 * quality of service   13 bit(s) offset 10  > 00000000 01111111 11111100 00000000
 *
 * the remaining 11 bits are currently not in use
 */

struct NF_MASKS {
	char *service_name;	// name of the service
	int bits_used;		// bits used by this service
	int bit_offset;		// position of the fist bit 
};

static struct NF_MASKS service_masks[] = {
	{"FORWARD", 1, 31},
	{"HOTSPOT", 8, 23},
	{"QOS", 13, 10},
};

char *get_NFServiceMark(char *service, uint32 mark)
{
	static char buffer[24];
	memset(&buffer, 0, sizeof(buffer));

#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
// no mask support possible in kernel 2.4
	sprintf(buffer, "0x%x", mark);
	return buffer;
#else
	int x, offset, bitpos;
	uint32 nfmark = 0, nfmask = 0;

	for (x = 0; x < sizeof(service_masks) / sizeof(struct NF_MASKS); x++) {
		if (strcmp(service, service_masks[x].service_name) == 0) {
			if (mark >= (1 << service_masks[x].bits_used))
				return "0xffffffff/0xffffffff";

			offset = service_masks[x].bit_offset;
			bitpos = offset + service_masks[x].bits_used - 1;

			nfmark = (mark << offset);

			for (; bitpos >= offset; bitpos--)
				nfmask |= (1 << bitpos);

			sprintf(buffer, "0x%x/0x%x", nfmark, nfmask);
			return buffer;
		}
	}
	return "0xffffffff/0xffffffff";
#endif
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
	} else if (nvram_match("vlan2ports", "0 8")) {
		vlanmap[0] = 0;
		vlanmap[5] = 8;
		if (nvram_match("vlan1ports", "4 3 2 1 8*")) {
			vlanmap[1] = 4;
			vlanmap[2] = 3;
			vlanmap[3] = 2;
			vlanmap[4] = 1;
		}
	} else if (nvram_match("vlan2ports", "4 8")) {
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
	} else if (nvram_match("vlan2ports", "4 8u")) {
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
	} else if (nvram_match("vlan1ports", "4 8")) {
		vlanmap[0] = 4;
		vlanmap[5] = 8;
		if (nvram_match("vlan2ports", "0 1 2 3 8*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		}
	} else if (nvram_match("vlan2ports", "4 5")) {
		vlanmap[0] = 4;
		vlanmap[5] = 5;
		if (nvram_match("vlan1ports", "0 1 2 3 5*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		} else		// nvram_match ("vlan1ports", "3 2 1 0 5*")
		{
			vlanmap[1] = 3;
			vlanmap[2] = 2;
			vlanmap[3] = 1;
			vlanmap[4] = 0;
		}

	} else if (nvram_match("vlan2ports", "4 5u")) {
		vlanmap[0] = 4;
		vlanmap[5] = 5;
		if (nvram_match("vlan1ports", "0 1 2 3 5*")) {
			vlanmap[1] = 0;
			vlanmap[2] = 1;
			vlanmap[3] = 2;
			vlanmap[4] = 3;
		} else		// nvram_match ("vlan1ports", "3 2 1 0 5*")
		{
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
