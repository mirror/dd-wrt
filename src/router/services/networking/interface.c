/*
 * interface.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#include <ctype.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <proto/ethernet.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <rc.h>
#include <cy_conf.h>
#include <cymac.h>
#include <services.h>
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)
int ifconfig(char *name, int flags, char *addr, char *netmask)
{
	// char *down="down";
	// if (flags == IFUP)
	// down = "up";
	cprintf("ifconfig %s = %s/%s\n", name, addr, netmask);
	if (!ifexists(name)) {
		cprintf("interface %s does not exists, ignoring\n", name);
		return -1;
	}
	// if (addr==NULL)
	// addr="0.0.0.0";
	// int ret;
	// if (netmask==NULL)
	// {
	// ret = eval("ifconfig",name,addr,down);
	// }else
	// {
	// ret = eval("ifconfig",name,addr,"netmask",netmask,down);
	// }
	int s;
	struct ifreq ifr;
	struct in_addr in_addr, in_netmask, in_broadaddr;

	cprintf("ifconfig(): name=[%s] flags=[%s] addr=[%s] netmask=[%s]\n", name, flags == IFUP ? "IFUP" : "0", addr, netmask);

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		goto err;
	cprintf("ifconfig(): socket opened\n");

	strncpy(ifr.ifr_name, name, IFNAMSIZ);
	cprintf("ifconfig(): set interface name\n");
	if (flags) {
		ifr.ifr_flags = flags;
		if (ioctl(s, SIOCSIFFLAGS, &ifr) < 0)
			goto err;
	}
	cprintf("ifconfig(): interface flags configured\n");
	if (addr) {
		inet_aton(addr, &in_addr);
		sin_addr(&ifr.ifr_addr).s_addr = in_addr.s_addr;
		ifr.ifr_addr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFADDR, &ifr) < 0)
			goto err;
	}
	cprintf("ifconfig() ip configured\n");

	if (addr && netmask) {
		inet_aton(netmask, &in_netmask);
		sin_addr(&ifr.ifr_netmask).s_addr = in_netmask.s_addr;
		ifr.ifr_netmask.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFNETMASK, &ifr) < 0)
			goto err;

		in_broadaddr.s_addr = (in_addr.s_addr & in_netmask.s_addr) | ~in_netmask.s_addr;
		sin_addr(&ifr.ifr_broadaddr).s_addr = in_broadaddr.s_addr;
		ifr.ifr_broadaddr.sa_family = AF_INET;
		if (ioctl(s, SIOCSIFBRDADDR, &ifr) < 0)
			goto err;
	}
	cprintf("ifconfig() mask configured\n");

	close(s);
	cprintf("ifconfig() done()\n");
	return 0;

err:
	cprintf("ifconfig() done with error\n");
	close(s);
#ifndef HAVE_SILENCE
	perror(name);
#endif
	return errno;
	// return ret;
}

static char *getPhyDev()
{
	if (f_exists("/proc/switch/eth0/enable"))
		return "eth0";

	if (f_exists("/proc/switch/eth1/enable"))
		return "eth1";

	if (f_exists("/proc/switch/eth2/enable"))
		return "eth2";

	return "eth0";
}

#define MAX_VLAN_GROUPS	16
#define MAX_DEV_IFINDEX	16

/*
 * configure vlan interface(s) based on nvram settings 
 */
void start_config_vlan(void)
{
	int s;
	struct ifreq ifr;
	int i, j;
	char ea[ETHER_ADDR_LEN];
	char *phy = getPhyDev();

	// configure ports
	writevaproc("1", "/proc/switch/%s/reset", phy);
	writevaproc("1", "/proc/switch/%s/enable_vlan", phy);
	for (i = 0; i < 16; i++) {
		char vlanb[16];

		sprintf(vlanb, "vlan%dports", i);
		if (nvram_get(vlanb) == NULL || nvram_match(vlanb, ""))
			continue;
		writevaproc(nvram_safe_get(vlanb), "/proc/switch/%s/vlan/%d/ports", phy, i);
	}

	/*
	 * set vlan i/f name to style "vlan<ID>" 
	 */

	eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");

	/*
	 * create vlan interfaces 
	 */
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

	for (i = 0; i < MAX_VLAN_GROUPS; i++) {
		char vlan_id[16];
		char *hwname, *hwaddr;

		if (!(hwname = nvram_nget("vlan%dhwname", i))) {
			continue;
		}
		if (!(hwaddr = nvram_nget("%smacaddr", hwname))) {
			continue;
		}
		if (strlen(hwname) == 0 || strlen(hwaddr) == 0) {
			continue;
		}
		ether_atoe(hwaddr, ea);
		for (j = 1; j <= MAX_DEV_IFINDEX; j++) {
			ifr.ifr_ifindex = j;
			if (ioctl(s, SIOCGIFNAME, &ifr)) {
				continue;
			}
			if (ioctl(s, SIOCGIFHWADDR, &ifr)) {
				continue;
			}
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER) {
				continue;
			}
			if (!bcmp(ifr.ifr_hwaddr.sa_data, ea, ETHER_ADDR_LEN)) {
				break;
			}
		}
		if (j > MAX_DEV_IFINDEX) {
			continue;
		}
		if (ioctl(s, SIOCGIFFLAGS, &ifr))
			continue;
		if (!(ifr.ifr_flags & IFF_UP))
			eval("ifconfig", ifr.ifr_name, "0.0.0.0", "up");
		snprintf(vlan_id, sizeof(vlan_id), "%d", i);
		eval("vconfig", "add", ifr.ifr_name, vlan_id);
	}

	close(s);

	return;
}

/*
 * begin lonewolf mods 
 */

void start_setup_vlans(void)
{
#if defined(HAVE_RB500) || defined(HAVE_XSCALE) || defined(HAVE_LAGUNA) || defined(HAVE_MAGICBOX) || defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_WHRAG108) || defined(HAVE_LS2) || defined(HAVE_CA8) || defined(HAVE_TW6600) || defined(HAVE_PB42) || defined(HAVE_LS5) || defined(HAVE_LSX) || defined(HAVE_DANUBE) || defined(HAVE_STORM) || defined(HAVE_ADM5120) || defined(HAVE_RT2880) || defined(HAVE_SOLO51) || defined(HAVE_OPENRISC) || defined(HAVE_VENTANA)
	return;
#else
	/*
	 * VLAN #16 is just a convieniant way of storing tagging info.  There is
	 * no VLAN #16 
	 */

	if (!nvram_get("port5vlans") || nvram_match("vlans", "0"))
		return;		// for some reason VLANs are not set up, and
	// we don't want to disable everything!

	if (nvram_match("wan_vdsl", "1") && !nvram_match("fromvdsl", "1")) {
		nvram_set("vdsl_state", "0");
		enable_dtag_vlan(1);
		return;
	}

	int i, j, ret = 0, tmp, workaround = 0, found;
	char *vlans, *next, vlan[4], buff[70], buff2[16];
	FILE *fp;
	char portsettings[16][64];
	char tagged[16];
	unsigned char mac[20];;
	struct ifreq ifr;
	int s;
	char *phy = getPhyDev();

	s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	strcpy(mac, nvram_safe_get("et0macaddr"));

	int vlanmap[6] = { 0, 1, 2, 3, 4, 5 };	// 0=wan; 1,2,3,4=lan; 5=internal 

	getPortMapping(vlanmap);

	int ast = 0;
	char *asttemp;
	char *lanifnames = nvram_safe_get("lan_ifnames");

	if (strstr(lanifnames, "vlan1") && !strstr(lanifnames, "vlan0"))
		asttemp = nvram_safe_get("vlan1ports");
	else if (strstr(lanifnames, "vlan2") && !strstr(lanifnames, "vlan0")
		 && !strstr(lanifnames, "vlan1"))
		asttemp = nvram_safe_get("vlan2ports");
	else
		asttemp = nvram_safe_get("vlan0ports");

	if (strstr(asttemp, "5*") || strstr(asttemp, "8*"))
		ast = 1;

	memset(&portsettings[0][0], 0, 16 * 64);
	memset(&tagged[0], 0, 16);
	for (i = 0; i < 6; i++) {
		vlans = nvram_nget("port%dvlans", i);
		int use = vlanmap[i];

		if (vlans) {
			int lastvlan = 0;
			int portmask = 3;
			int mask = 0;

			foreach(vlan, vlans, next) {
				tmp = atoi(vlan);
				if (tmp < 16) {
					lastvlan = tmp;
					if (i == 5) {
						snprintf(buff, 9, "%d", tmp);
						eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
						eval("vconfig", "add", phy, buff);
						snprintf(buff, 9, "vlan%d", tmp);
						if (strcmp(nvram_safe_get("wan_ifname"), buff)) {
							if (strlen(nvram_nget("%s_ipaddr", buff)) > 0)
								eval("ifconfig", buff, nvram_nget("%s_ipaddr", buff), "netmask", nvram_nget("%s_netmask", buff), "up");
							else
								eval("ifconfig", buff, "0.0.0.0", "up");
						}
					}

					sprintf((char *)
						&portsettings[tmp][0], "%s %d", (char *)
						&portsettings[tmp][0], use);
				} else {
					if (tmp == 16)	// vlan tagged
						tagged[use] = 1;
					if (tmp == 17)	// no auto negotiate
						mask |= 4;
					if (tmp == 18)	// no full speed
						mask |= 1;
					if (tmp == 19)	// no full duplex
						mask |= 2;
					if (tmp == 20)	// disabled
						mask |= 8;
					if (tmp == 21)	// no gigabit
						mask |= 16;

				}
			}
			if (mask & 8 && use < 5) {
				writevaproc("0", "/proc/switch/%s/port/%d/enable", phy, use);
			} else {
				writevaproc("1", "/proc/switch/%s/port/%d/enable", phy, use);
			}
			if (use < 5) {
				snprintf(buff, 69, "/proc/switch/%s/port/%d/media", phy, use);
				if ((fp = fopen(buff, "r+"))) {
					if ((mask & 4) == 4) {
						if (!(mask & 16)) {
							if (mask & 2)
								fputs("1000HD", fp);
							else
								fputs("1000FD", fp);

						} else {
							switch (mask & 3) {
							case 0:
								fputs("100FD", fp);
								break;
							case 1:
								fputs("10FD", fp);
								break;
							case 2:
								fputs("100HD", fp);
								break;
							case 3:
								fputs("10HD", fp);
								break;
							}
						}
					} else {
						fprintf(stderr, "set port %d to AUTO\n", use);
						fputs("AUTO", fp);
					}
					fclose(fp);
				}
			}

		}
	}
	// for (i = 0; i < 16; i++)
	// {
	// fprintf(stderr,"echo %s >
	// /proc/switch/eth0/vlan/%d/ports\n",portsettings[i], i);
	// }
	for (i = 0; i < 16; i++) {
		char port[64];

		strcpy(port, &portsettings[i][0]);
		memset(&portsettings[i][0], 0, 64);
		foreach(vlan, port, next) {
			if (atoi(vlan) < 5 && atoi(vlan) >= 0 && tagged[atoi(vlan)])
				sprintf(&portsettings[i][0], "%s %st", &portsettings[i][0], vlan);
			else if ((atoi(vlan) == 5 || atoi(vlan) == 8)
				 && tagged[atoi(vlan)] && !ast)
				sprintf(&portsettings[i][0], "%s %st", &portsettings[i][0], vlan);
			else if ((atoi(vlan) == 5 || atoi(vlan) == 8)
				 && tagged[atoi(vlan)] && ast)
				sprintf(&portsettings[i][0], "%s %s*", &portsettings[i][0], vlan);
			else
				sprintf(&portsettings[i][0], "%s %s", &portsettings[i][0], vlan);
		}
	}
	for (i = 0; i < 16; i++) {
		writevaproc(" ", "/proc/switch/%s/vlan/%d/ports", phy, i);
	}
	for (i = 0; i < 16; i++) {
		fprintf(stderr, "configure vlan ports to %s\n", portsettings[i]);
		writevaproc(portsettings[i], "/proc/switch/%s/vlan/%d/ports", phy, i);
	}
	return;
#endif
}

int flush_interfaces(void)
{
	char all_ifnames[256] = { 0 }, *c, *next, buff[32], buff2[32];

#ifdef HAVE_MADWIFI
#ifdef HAVE_GATEWORX
	snprintf(all_ifnames, 255, "%s %s %s", "ixp0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_NORTHSTAR
	snprintf(all_ifnames, 255, "%s %s %s", "vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_LAGUNA
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_VENTANA
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_X86
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_EAP9550
	snprintf(all_ifnames, 255, "%s %s %s", "eth2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RT2880
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_MAGICBOX
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_UNIWIP
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WDR4900
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RB600
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1 eth2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WRT54G2
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RTG32
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DIR300
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_MR3202A
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_LS2
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan0 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_SOLO51
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan0 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_LS5
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_FONERA
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan0 vlan1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WHRAG108
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WNR2000
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WASP
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WDR2543
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WHRHPGN
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DIR615E
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WA901v1
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_CARAMBOLA
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR703
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WA7510
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR741
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_UBNTM
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_PB42
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif CONFIG_JJAP93
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_JJAP005
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_JJAP501
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_AC722
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_HORNET
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_AC622
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RS
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_JA76PF
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_ALFAAP94
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_JWAP003
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WA901
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR941
	snprintf(all_ifnames, 255, "%s %s %s", "vlan0 vlan1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR1043
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WZRG450
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DIR632
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_AP83
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_AP94
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WP546
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_LSX
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DANUBE
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WBD222
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1 eth2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_STORM
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_OPENRISC
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1 eth2 eth3", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_ADM5120
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_TW6600
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RDAT81
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RCAA01
	snprintf(all_ifnames, 255, "%s %s %s", "vlan0 vlan1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_CA8PRO
	snprintf(all_ifnames, 255, "%s %s %s", "vlan0 vlan1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_CA8
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#else
	snprintf(all_ifnames, 255, "%s %s %s", "ixp0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#endif
#elif HAVE_RT2880
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#else
	if (wl_probe("eth2"))
		snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
	else
		snprintf(all_ifnames, 255, "%s %s %s %s", "eth0", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#endif
	// strcpy(all_ifnames, "eth0 ");
	// strcpy(all_ifnames, "eth0 eth1 "); //James, note: eth1 is the wireless 
	// interface on V2/GS's. I think we need a check here.
	// strcat(all_ifnames, nvram_safe_get("lan_ifnames"));
	// strcat(all_ifnames, " ");
	// strcat(all_ifnames, nvram_safe_get("wan_ifnames"));

	c = nvram_safe_get("port5vlans");
	if (c) {
		foreach(buff, c, next) {
			if (atoi(buff) > 15)
				continue;
			snprintf(buff2, sizeof(buff2), " vlan%s", buff);
			strcat(all_ifnames, buff2);
		}
	}

	foreach(buff, all_ifnames, next) {
		if (strcmp(buff, "br0") == 0)
			continue;
		eval("ifconfig", buff, "0.0.0.0", "down");

		// eval ("ifconfig", buff, "down");
		eval("ip", "addr", "flush", "dev", buff);
		eval("ifconfig", buff, "0.0.0.0", "up");

		// eval ("ifconfig", buff, "up");
	}

	return 0;
}
