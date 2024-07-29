/*
 * interface.c
 *
 * Copyright (C) 2007-2023 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef __UCLIBC__
#include <error.h>
#endif
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
#include <rc.h>
#include <cy_conf.h>
#include <utils.h>
#include <services.h>

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

#define MAX_VLAN_GROUPS 16
#define MAX_DEV_IFINDEX 16

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
		if (!nvram_exists(vlanb) || nvram_match(vlanb, ""))
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
		if (!*hwname || !*hwaddr) {
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
	int blen = nvram_geti("portvlan_count");
#ifdef HAVE_SWCONFIG

	if (!nvram_exists("sw_cpuport") && !nvram_exists("sw_wancpuport"))
		return;
#ifdef HAVE_R9000
	if (nvram_match("switch_leds", "0")) {
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x50", "0x00000000", "4");
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x54", "0x00000000", "4");
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x58", "0x00000000", "4");
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x5c", "0x00000000", "4");

		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x50", "0x00000000", "4");
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x54", "0x00000000", "4");
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x58", "0x00000000", "4");
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x5c", "0x00000000", "4");

	} else {
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x50", "0xcc35cc35", "4");
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x54", "0xca35ca35", "4");
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x58", "0xc935c935", "4");
		eval("ssdk_sh_id", "0", "debug", "reg", "set", "0x5c", "0x03ffff00", "4");

		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x50", "0xcc35cc35", "4");
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x54", "0xca35ca35", "4");
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x58", "0xc935c935", "4");
		eval("ssdk_sh_id", "1", "debug", "reg", "set", "0x5c", "0x03ffff00", "4");
	}
	if (!nvram_exists("port7vlans") || nvram_matchi("vlans", 0))
		return; // for some reason VLANs are not set up, and
#else
	if (nvram_match("switch_leds", "0")) {
		eval("swconfig", "dev", "switch0", "set", "leds", "0");
	} else {
		eval("swconfig", "dev", "switch0", "set", "leds", "1");
	}
	if (nvram_matchi("vlans", 0))
		return; // for some reason VLANs are not set up, and
#endif
#ifdef HAVE_R9000
	sysprintf(". /usr/sbin/resetswitch.sh");
#else
	eval("swconfig", "dev", "switch0", "set", "reset", "1");
	eval("swconfig", "dev", "switch0", "set", "igmp_v3", "1");
#endif
	int lanports = 5;
	if (nvram_exists("sw_lan6"))
		lanports = 7;
	if (!*nvram_safe_get("sw_lan4"))
		lanports = 4;
	if (!*nvram_safe_get("sw_lan3"))
		lanports = 3;
	if (!*nvram_safe_get("sw_lan2"))
		lanports = 2;
	if (*nvram_safe_get("sw_lancpuport") && *nvram_safe_get("sw_wancpuport"))
		lanports += 2;
	else if (*nvram_safe_get("sw_cpuport"))
		lanports++;
	char tagged[18];
	char snoop[5];
	memset(&tagged[0], 0, sizeof(tagged));
	memset(&snoop[0], 0, sizeof(snoop));
	int vlan_number;
	int i;
	int vlan_enable = 0;
	char **buildports = malloc(sizeof(char **) * (blen + 2));
	for (i = 0; i < blen + 2; i++) {
		buildports[i] = malloc(32);
		memset(buildports[i], 0, 32);
	}
	char *c = nvram_safe_get("portvlanlist");
	int *vlanlist = malloc(sizeof(int) * strlen(c));
	for (i = 0; i < strlen(c); i++)
		vlanlist[i] = i;
	i = 0;
	char portvlan[32];
	char *next;
	foreach(portvlan, c, next)
	{
		vlanlist[i++] = atoi(portvlan);
	}
	int wancpuportidx = -1;
	int lancpuportidx = -1;
	int cpuportidx = -1;
	if (*nvram_safe_get("sw_lancpuport") && *nvram_safe_get("sw_wancpuport")) {
		wancpuportidx = lanports - 2;
		lancpuportidx = lanports - 1;
	} else if (*nvram_safe_get("sw_cpuport"))
		cpuportidx = lanports - 1;

#ifdef HAVE_R9000
	for (i = 0; i < 7; i++) {
#else
	for (i = 0; i < lanports; i++) {
#endif
		char *vlans = nvram_nget("port%dvlans", i);
		char *next;
		char vlan[32];
		int mask = 0;
		foreach(vlan, vlans, next)
		{
			int tmp = atoi(vlan);
			if (tmp == 16000) {
				tagged[i] = 1;
				vlan_enable = 1;
			}
		}
		foreach(vlan, vlans, next)
		{
			int tmp = atoi(vlan);
			if (tmp >= 16000) {
				switch (tmp) {
				case 22000:
					if (i == 0) {
						eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wan"), "set",
						     "igmp_snooping", "1");
					} else if (i == wancpuportidx) {
						eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wancpuport"), "set",
						     "igmp_snooping", "1");
					} else if (i == lancpuportidx) {
						eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_lancpuport"), "set",
						     "igmp_snooping", "1");
					} else if (i == cpuportidx) {
						eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_cpuport"), "set",
						     "igmp_snooping", "1");
					} else {
						eval("swconfig", "dev", "switch0", "port", nvram_nget("sw_lan%d", i), "set",
						     "igmp_snooping", "1");
					}
					break;
				case 23000:
					if (i == 0) {
						eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wan"), "set",
						     "enable_eee", "1");
					} else if (i == wancpuportidx) {
						//                                              eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wancpuport"), "set", "igmp_snooping", "1");
					} else if (i == lancpuportidx) {
						//                                              eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_lancpuport"), "set", "igmp_snooping", "1");
					} else if (i == cpuportidx) {
						//                                              eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_cpuport"), "set", "igmp_snoopin", "1");
					} else {
						eval("swconfig", "dev", "switch0", "port", nvram_nget("sw_lan%d", i), "set",
						     "enable_eee", "1");
					}
					break;
				case 16000:
#if 0
					if (!nvram_match("sw_wan", "-1") && nvram_exists("sw_wancpuport")) {
						char vl[32];
						sprintf(vl, "%d", vlanlist[vlan_number]);
						char pl[32];
						sprintf(pl, "%st %st", nvram_safe_get("sw_wancpuport"), nvram_safe_get("sw_wan"));
						eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", pl);
					}
#endif
					break;
				case 17000: // no auto negotiate
					mask |= 4;
					break;
				case 18000: // no gigabit
					mask |= 16;
					break;
				case 19000: // no full speed
					mask |= 1;
					break;
				case 20000: // no duplex
					mask |= 2;
					break;
				case 21000: // disabled
					mask |= 8;
					break;
				case 24000: // flow control
					mask |= 32;
					break;
				}
			} else {
				vlan_number = tmp;
				char *ports = buildports[vlan_number];
#if 0
				if (i == 0 && nvram_exists("sw_wancpuport")) {	// wan port
					if (!nvram_match("sw_wan", "-1")) {
						snprintf(ports, 31, "W");	// mark port as wan to prevent overwriting of vlan in later code
						char vl[32];
						sprintf(vl, "%d", vlanlist[vlan_number]);
						char pl[32];
						sprintf(pl, "%st %s%s", nvram_safe_get("sw_wancpuport"), nvram_safe_get("sw_wan"), tagged[i] ? "t" : "");
						eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", pl);
					}
				} else
#endif
				{
					if (i == 0) {
						if (*ports)
							snprintf(ports, 31, "%s %s%s", ports, nvram_safe_get("sw_wan"),
								 tagged[i] ? "t" : "");
						else
							snprintf(ports, 31, "%s%s", nvram_safe_get("sw_wan"), tagged[i] ? "t" : "");
					} else if (i == wancpuportidx) {
						if (*ports)
							snprintf(ports, 31, "%s %s%s", ports, nvram_safe_get("sw_wancpuport"),
								 tagged[i] ? "t" : "");
						else
							snprintf(ports, 31, "%s%s", nvram_safe_get("sw_wancpuport"),
								 tagged[i] ? "t" : "");
					} else if (i == lancpuportidx) {
						if (*ports)
							snprintf(ports, 31, "%s %s%s", ports, nvram_safe_get("sw_lancpuport"),
								 tagged[i] ? "t" : "");
						else
							snprintf(ports, 31, "%s%s", nvram_safe_get("sw_lancpuport"),
								 tagged[i] ? "t" : "");
					} else if (i == cpuportidx) {
						if (*ports)
							snprintf(ports, 31, "%s %s%s", ports, nvram_safe_get("sw_cpuport"),
								 tagged[i] ? "t" : "");
						else
							snprintf(ports, 31, "%s%s", nvram_safe_get("sw_cpuport"),
								 tagged[i] ? "t" : "");
					} else {
						if (*ports)
							snprintf(ports, 31, "%s %s%s", ports, nvram_nget("sw_lan%d", i),
								 tagged[i] ? "t" : "");
						else
							snprintf(ports, 31, "%s%s", nvram_nget("sw_lan%d", i),
								 tagged[i] ? "t" : "");
					}
				}
				char buff[32];
				snprintf(buff, 9, "%d", vlanlist[vlan_number]);
				eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
				char *lanphy = "eth0";
				char *wanphy = "eth0";

				if (nvram_exists("sw_wancpuport") && nvram_match("wan_default", "eth0")) {
					lanphy = "eth1";
					wanphy = "eth0";
				}
				if (nvram_exists("sw_wancpuport") && nvram_match("wan_default", "eth1")) {
					lanphy = "eth0";
					wanphy = "eth1";
				}
				if (*nvram_safe_get("switchphy")) {
					lanphy = nvram_safe_get("switchphy");
				}

				/*
				 * user must now manually handle this at networking. we cannot decide what todo with the config
				 * but i keep it now for testing
				 */
				if (tagged[i] && (i == cpuportidx || i == wancpuportidx || i == lancpuportidx)) {
					if (i == wancpuportidx)
						eval("vconfig", "add", wanphy, buff);
					else if (i == lancpuportidx)
						eval("vconfig", "add", lanphy, buff);
					else
						eval("vconfig", "add", lanphy, buff);
					snprintf(buff, 9, "vlan%d", vlanlist[vlan_number]);
					if (strcmp(nvram_safe_get("wan_ifname"), buff)) {
						char hwaddr[32];
						sprintf(hwaddr, "%s_hwaddr", buff);
						if (!nvram_match(hwaddr, ""))
							set_hwaddr(buff, nvram_safe_get(hwaddr));
						if (nvram_nmatch("0", "%s_bridged", buff)) {
							if (*(nvram_nget("%s_ipaddr", buff)))
								eval("ifconfig", buff, nvram_nget("%s_ipaddr", buff), "netmask",
								     nvram_nget("%s_netmask", buff), "up");
							else
								eval("ifconfig", buff, "0.0.0.0", "up");
						} else {
							char tmp[256];
							eval("ifconfig", buff, "0.0.0.0", "up");
							br_add_interface(getBridge(buff, tmp), buff);
						}
					}
				}
			}
		}
		if (i != cpuportidx && i != lancpuportidx && i != wancpuportidx) {
			char linkstr[128] = { 0 };
			if (mask & 4) {
				if (mask & 2)
					sprintf(linkstr, "duplex half");
				else
					sprintf(linkstr, "duplex full");

				if (mask & 16) {
					if (mask & 1)
						sprintf(linkstr, "%s speed 10", linkstr);
					else
						sprintf(linkstr, "%s speed 100", linkstr);

					sprintf(linkstr, "%s autoneg off", linkstr);
					if (mask & 32) {
						sprintf(linkstr, "%s rxflow txflow", linkstr);
					}
				} else {
					sprintf(linkstr, "%s speed 1000", linkstr);
					sprintf(linkstr, "%s autoneg on", linkstr);
				}

			} else
				sprintf(linkstr, "autoneg on", linkstr);
			if (i == 0) {
				eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wan"), "set", "link", linkstr);
			} else {
				eval("swconfig", "dev", "switch0", "port", nvram_nget("sw_lan%d", i), "set", "link", linkstr);
			}
			if (mask & 8) {
				if (i == 0) {
					eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wan"), "set", "disable", "1");
				} else {
					eval("swconfig", "dev", "switch0", "port", nvram_nget("sw_lan%d", i), "set", "disable",
					     "1");
				}
			} else {
				if (i == 0) {
					eval("swconfig", "dev", "switch0", "port", nvram_safe_get("sw_wan"), "set", "disable", "0");
				} else {
					eval("swconfig", "dev", "switch0", "port", nvram_nget("sw_lan%d", i), "set", "disable",
					     "0");
				}
			}
		}
	}

#ifdef HAVE_R9000
	for (vlan_number = 0; vlan_number < 16; vlan_number++) {
		char *ports = buildports[vlan_number];
		if (strlen(ports)) {
			//                          sw_user_lan_ports_vlan_config "7" "6" "0" "0" "0" "normal_lan"
			//                          sw_user_lan_ports_vlan_config "1" "" "0" "1" "0" "wan"
			sysprintf(". /usr/sbin/config_vlan.sh \"%d\" \"%s\" \"0\" \"0\" \"0\" \"normal_lan\"");
		}
	}

	sysprintf(". /usr/sbin/config_vlan.sh \"2\" \"\" \"0\" \"1\" \"0\"  \"wan\"");
	sysprintf(". /tmp/ssdk_new.sh");
#else
	for (vlan_number = 0; vlan_number < blen; vlan_number++) {
		char *ports = buildports[vlan_number];
		char vl[32];
		sprintf(vl, "%d", vlanlist[vlan_number]);
		if (!strstr(ports, "t"))
			eval("swconfig", "dev", "switch0", "vlan", vl, "set", "port_based", "1");
		else
			eval("swconfig", "dev", "switch0", "vlan", vl, "set", "port_based", "0");

		if (strlen(ports) && ports[0] != 'W') {
			eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", ports);
		} else if (!strlen(ports)) {
			eval("swconfig", "dev", "switch0", "vlan", vl, "set", "ports", "");
		}
	}

	eval("swconfig", "dev", "switch0", "set", "enable_vlan", vlan_enable ? "1" : "0");
	eval("swconfig", "dev", "switch0", "set", "apply");
#endif
	for (i = 0; i < blen + 2; i++)
		free(buildports[i]);
	free(buildports);
#else
	/*
	 * VLAN #16 is just a convieniant way of storing tagging info.  There is
	 * no VLAN #16 
	 */

	if (!nvram_exists("port5vlans") || nvram_matchi("vlans", 0))
		return; // for some reason VLANs are not set up, and
	// we don't want to disable everything!

	if (nvram_matchi("wan_vdsl", 1) && !nvram_matchi("fromvdsl", 1)) {
		nvram_seti("vdsl_state", 0);
		enable_dtag_vlan(1);
		return;
	}

	int i, j, ret = 0, tmp, workaround = 0, found;
	char *vlans, *next, vlan[32], buff[70], buff2[16];
	FILE *fp;
	char **portsettings = malloc(sizeof(char **) * (blen + 2));
	for (i = 0; i < blen + 2; i++) {
		portsettings[i] = malloc(64);
		memset(portsettings[i], 0, 32);
	}
	char tagged[18];
	unsigned char mac[20];
	;
	struct ifreq ifr;
	char *phy = getPhyDev();

	char *c = nvram_safe_get("portvlanlist");
	int *vlanlist = malloc(sizeof(int) * strlen(c));
	for (i = 0; i < strlen(c); i++)
		vlanlist[i] = i;
	i = 0;
	char *portvlan[32];
	foreach(portvlan, c, next)
	{
		vlanlist[i++] = atoi(portvlan);
	}
	strcpy(mac, nvram_safe_get("et0macaddr"));

	int vlanmap[6] = { 0, 1, 2, 3, 4, 5 }; // 0=wan; 1,2,3,4=lan; 5=internal

	getPortMapping(vlanmap);

	int ast = 0;
	char *asttemp;
	char *lanifnames = nvram_safe_get("lan_ifnames");

	if (strstr(lanifnames, "vlan1") && !strstr(lanifnames, "vlan0"))
		asttemp = nvram_safe_get("vlan1ports");
	else if (strstr(lanifnames, "vlan2") && !strstr(lanifnames, "vlan0") && !strstr(lanifnames, "vlan1"))
		asttemp = nvram_safe_get("vlan2ports");
	else
		asttemp = nvram_safe_get("vlan0ports");

	if (strstr(asttemp, "5*"))
		ast = 5;
	if (strstr(asttemp, "8*"))
		ast = 8;
	if (strstr(asttemp, "7u"))
		ast = 7;

	bzero(&tagged[0], sizeof(tagged));
	for (i = 0; i < 6; i++) {
		vlans = nvram_nget("port%dvlans", i);
		int use = vlanmap[i];

		if (vlans) {
			int lastvlan = 0;
			int portmask = 3;
			int mask = 0;
			foreach(vlan, vlans, next)
			{
				tmp = atoi(vlan);
				if (tmp == 16000)
					tagged[i] = 1;
			}

			foreach(vlan, vlans, next)
			{
				tmp = atoi(vlan);
				if (tmp < 16000) {
					lastvlan = tmp;
					if (i == 5) {
						snprintf(buff, 9, "%d", vlanlist[tmp]);
						eval("vconfig", "set_name_type", "VLAN_PLUS_VID_NO_PAD");
						eval("vconfig", "add", phy, buff);
						snprintf(buff, 9, "vlan%d", vlanlist[tmp]);

						if (strcmp(nvram_safe_get("wan_ifname"), buff)) {
							char hwaddr[32];
							sprintf(hwaddr, "%s_hwaddr", buff);
							if (!nvram_match(hwaddr, ""))
								set_hwaddr(buff, nvram_safe_get(hwaddr));
							if (nvram_nmatch("0", "%s_bridged", buff)) {
								if (*(nvram_nget("%s_ipaddr", buff)))
									eval("ifconfig", buff, nvram_nget("%s_ipaddr", buff),
									     "netmask", nvram_nget("%s_netmask", buff), "up");
								else
									eval("ifconfig", buff, "0.0.0.0", "up");
							} else {
								char tmp[256];
								eval("ifconfig", buff, "0.0.0.0", "up");
								br_add_interface(getBridge(buff, tmp), buff);
							}
						}
					}

					sprintf((char *)portsettings[tmp], "%s %d%s", (char *)portsettings[tmp], use,
						tagged[i] ? "t" : "");
				} else {
					if (tmp == 17000) // no auto negotiate
						mask |= 4;
					if (tmp == 18000) // gigabit
						mask |= 16;
					if (tmp == 19000) // fullspeed
						mask |= 1;
					if (tmp == 20000) // duplex
						mask |= 2;
					if (tmp == 21000) // enabled
						mask |= 8;
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

	for (i = 0; i < blen; i++) {
		char port[64];

		strcpy(port, portsettings[i]);
		bzero(portsettings[i], 64);
		char strvid[32];
		foreach(vlan, port, next)
		{
			int vlan_number = vlan[0] - '0';
			char strvid[32];
			sprintf(strvid, "%d*", vlan_number);
			if (vlan_number < 5 && vlan_number >= 0) {
				strspcattach(portsettings[i], vlan);
			} else if ((vlan_number == 5 || vlan_number == 8 || vlan_number == 7) && !ast) {
				strspcattach(portsettings[i], vlan);
			} else if ((vlan_number == 5 || vlan_number == 8 || vlan_number == 7) && ast) {
				strspcattach(portsettings[i], strvid);
			} else {
				strspcattach(portsettings[i], vlan);
			}
		}
	}
	for (i = 0; i < blen; i++) {
		writevaproc(" ", "/proc/switch/%s/vlan/%d/ports", phy, vlanlist[i]);
	}
	for (i = 0; i < blen; i++) {
		fprintf(stderr, "configure vlan ports to %s\n", portsettings[i]);
		writevaproc(portsettings[i], "/proc/switch/%s/vlan/%d/ports", phy, vlanlist[i]);
	}
	for (i = 0; i < blen + 2; i++)
		free(portsettings[i]);
	free(portsettings);
#endif
	free(vlanlist);
}

int flush_interfaces(void)
{
	char all_ifnames[256] = { 0 }, *c, *next, buff[128], buff2[128];

#ifdef HAVE_MADWIFI
#ifdef HAVE_GATEWORX
	snprintf(all_ifnames, 255, "%s %s %s", "ixp0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_NORTHSTAR
	snprintf(all_ifnames, 255, "%s %s %s", "vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_EROUTER
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_LAGUNA
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_IPQ6018
	int brand = getRouterBrand();
	if (brand == ROUTER_LINKSYS_MR7350 || brand == ROUTER_DYNALINK_DLWRX36)
		snprintf(all_ifnames, 255, "%s %s %s", "eth4", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
	else if (brand == ROUTER_LINKSYS_MR5500)
		snprintf(all_ifnames, 255, "%s %s %s", "eth5", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
	else if (brand == ROUTER_LINKSYS_MX5500)
		snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
	else if (brand == ROUTER_ASUS_AX89X)
		snprintf(all_ifnames, 255, "%s %s %s", "eth2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
	else
		snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_VENTANA
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_NEWPORT
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_X86
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR810N
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_EAP9550
	snprintf(all_ifnames, 255, "%s %s %s", "eth2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RT2880
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_MAGICBOX
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_UNIWIP
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_MVEBU
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_R9000
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_IPQ806X
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
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
#elif HAVE_WR650AC
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DIR862
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WILLY
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_MMS344
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_CPE880
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_ARCHERC7V4
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WZR450HP2
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WDR3500
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_JWAP606
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_LIMA
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DW02_412H
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_RAMBUTAN
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
#elif HAVE_WR710
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR703
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WA7510
	snprintf(all_ifnames, 255, "%s %s %s", "eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_WR741
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DAP3310
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_DAP3410
	snprintf(all_ifnames, 255, "%s %s %s", "vlan1 vlan2", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_UBNTM
	snprintf(all_ifnames, 255, "%s %s %s", "eth0 eth1", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_PB42
	snprintf(all_ifnames, 255, "%s %s %s", "eth0", nvram_safe_get("lan_ifnames"), nvram_safe_get("wan_ifnames"));
#elif HAVE_JJAP93
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
		snprintf(all_ifnames, 255, "%s %s %s %s", "eth0", "eth1", nvram_safe_get("lan_ifnames"),
			 nvram_safe_get("wan_ifnames"));
#endif
	// strcpy(all_ifnames, "eth0 ");
	// strcpy(all_ifnames, "eth0 eth1 "); //James, note: eth1 is the wireless
	// interface on V2/GS's. I think we need a check here.
	// strcat(all_ifnames, nvram_safe_get("lan_ifnames"));
	// strcat(all_ifnames, " ");
	// strcat(all_ifnames, nvram_safe_get("wan_ifnames"));

	c = nvram_safe_get("port5vlans");
	if (c) {
		foreach(buff, c, next)
		{
			if (atoi(buff) > 15)
				continue;
			snprintf(buff2, sizeof(buff2), " vlan%s", buff);
			strcat(all_ifnames, buff2);
		}
	}
	c = nvram_safe_get("port6vlans");
	if (c) {
		foreach(buff, c, next)
		{
			if (atoi(buff) > 15)
				continue;
			snprintf(buff2, sizeof(buff2), " vlan%s", buff);
			strcat(all_ifnames, buff2);
		}
	}

	foreach(buff, all_ifnames, next)
	{
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
