/*
 * vlantagging.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/sysinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>
#include <services.h>
#include <libbridge.h>

#ifdef HAVE_VLANTAGGING
#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

static char *EGRESS(char *map, int class, char *prio)
{
	sprintf(map, "%d:%s", class, prio);
}
void start_vlantagging(void)
{
	char word[256];
	char *next, *wordlist;

	wordlist = nvram_safe_get("vlan_tags");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(type, word, 3);
		if (!tag || !port) {
			break;
		}
		if (!prio)
			prio = "0";
		if (!type)
			type = "0"; // 802.1q

		if (!strcmp(type, "1")) {
			// 802.1ad
			char vlan_name[32];
			sprintf(vlan_name, "%s.%s", tag, port);
			char map0[32];
			char map1[32];
			char map2[32];
			char map3[32];
			char map4[32];
			char map5[32];
			char map6[32];
			char map7[32];
			eval("ip", "link", "add", tag, vlan_name, "type", "vlan", "proto", "802.1ad", "id", port, "egress-qos-map",
			     EGRESS(map0, 0, prio), EGRESS(map1, 1, prio), EGRESS(map2, 2, prio), EGRESS(map3, 3, prio),
			     EGRESS(map4, 4, prio), EGRESS(map5, 5, prio), EGRESS(map6, 6, prio), EGRESS(map7, 7, prio));
			char var[64];

			sprintf(var, "%s_bridged", vlan_name);
			if (nvram_default_matchi(var, 1, 1)) {
				eval("ifconfig", vlan_name, "0.0.0.0", "up");
			} else {
				eval("ifconfig", vlan_name, nvram_nget("%s_ipaddr", vlan_name), "netmask",
				     nvram_nget("%s_netmask", vlan_name), "up");
			}
		}
	}

	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(type, word, 3);
		if (!tag || !port) {
			break;
		}
		if (!prio)
			prio = "0";
		if (!type)
			type = "0"; // 802.1q

		if (!strcmp(type, "0")) {
			char vlan_name[32];
			sprintf(vlan_name, "%s.%s", tag, port);
			eval("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
			eval("vconfig", "add", tag, port);
			eval("vconfig", "set_egress_map", vlan_name, "0", prio);
			eval("vconfig", "set_egress_map", vlan_name, "1", prio);
			eval("vconfig", "set_egress_map", vlan_name, "2", prio);
			eval("vconfig", "set_egress_map", vlan_name, "3", prio);
			eval("vconfig", "set_egress_map", vlan_name, "4", prio);
			eval("vconfig", "set_egress_map", vlan_name, "5", prio);
			eval("vconfig", "set_egress_map", vlan_name, "6", prio);
			eval("vconfig", "set_egress_map", vlan_name, "7", prio);
			char var[64];

			sprintf(var, "%s_bridged", vlan_name);
			if (nvram_default_matchi(var, 1, 1)) {
				eval("ifconfig", vlan_name, "0.0.0.0", "up");
			} else {
				eval("ifconfig", vlan_name, nvram_nget("%s_ipaddr", vlan_name), "netmask",
				     nvram_nget("%s_netmask", vlan_name), "up");
			}
		}
	}

	start_set_routes();
}

void stop_vlantagging(void)
{
	char word[256];
	char *next, *wordlist;

	wordlist = nvram_safe_get("vlan_tags");
	eval("vconfig", "set_name_type", "DEV_PLUS_VID_NO_PAD");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(type, word, 3);

		if (!tag || !port)
			break;
		if (!type)
			type = "0";

		char vlan_name[32];
		sprintf(vlan_name, "%s.%s", tag, port);
		if (!strcmp(type, "0") && ifexists(vlan_name)) {
			eval("vconfig", "rem", vlan_name);
		}
	}
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(type, word, 3);

		if (!tag || !port)
			break;
		if (!type)
			type = "0";

		char vlan_name[32];
		sprintf(vlan_name, "%s.%s", tag, port);
		if (!strcmp(type, "1") && ifexists(vlan_name)) {
			eval("ip", "link", "delete", vlan_name);
		}
	}
}

static void set_stp_state(char *bridge, char *stp)
{
	br_set_stp_state(bridge, strcmp(stp, "Off") ? 1 : 0);
#ifdef HAVE_MSTP
	if (strcmp(stp, "Off"))
		eval("mstpctl", "addbridge", bridge);
	else
		eval("mstpctl", "delbridge", bridge);

	if (!strcmp(stp, "STP"))
		eval("mstpctl", "setforcevers", bridge, "stp");
	if (!strcmp(stp, "MSTP"))
		eval("mstpctl", "setforcevers", bridge, "mstp");
	if (!strcmp(stp, "RSTP"))
		eval("mstpctl", "setforcevers", bridge, "rstp");
#endif
}

void apply_bridgeif(char *ifname, char *realport)
{
	char word[256];
	char *next, *wordlist;
#ifdef HAVE_MICRO
	br_init();
#endif
	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(hairpin, word, 3);
		GETENTRYBYIDX(stp, word, 4);
		GETENTRYBYIDX(pathcost, word, 5);

		if (!strcmp(ifname, port)) {
			if (prio)
				br_set_port_prio(tag, realport, atoi(prio));
			if (hairpin)
				br_set_port_hairpin(tag, realport, atoi(hairpin));
			if (stp)
				br_set_port_stp(tag, realport, atoi(stp));
			if (pathcost)
				br_set_path_cost(tag, realport, atoi(pathcost));
		}
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif
}

void start_bridgesif(void)
{
	char stp[256];
	char word[256];
	char *next, *wordlist;
#ifdef HAVE_MICRO
	br_init();
#endif
	set_stp_state("br0", getBridgeSTPType("br0", stp));

	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(hairpin, word, 3);
		GETENTRYBYIDX(stp, word, 4);
		GETENTRYBYIDX(pathcost, word, 5);

		if (strncmp(tag, "EOP", 3)) {
			br_add_interface(tag, port);
			if (prio)
				br_set_port_prio(tag, port, atoi(prio));
			if (hairpin)
				br_set_port_hairpin(tag, port, atoi(hairpin));
			if (stp)
				br_set_port_stp(tag, port, atoi(stp));
			if (pathcost)
				br_set_path_cost(tag, port, atoi(pathcost));
		}
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif
}

void start_bridging(void)
{
	char word[256];
	char *next, *wordlist;
	char hwaddr[32];
#ifdef HAVE_MICRO
	br_init();
#endif
	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(bridge, word, 0);
		GETENTRYBYIDX(stp, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(mtu, word, 3);
		GETENTRYBYIDX(forward_delay, word, 4);
		GETENTRYBYIDX(max_age, word, 5);
		if (!bridge || !stp)
			break;
		if (!forward_delay)
			forward_delay = "15";
		if (!max_age)
			max_age = "20";

		if (prio && mtu && *mtu)
			nvram_nset(mtu, "%s_mtu", bridge);
		br_add_bridge(bridge);
		set_stp_state(bridge, stp);
		br_set_bridge_max_age(bridge, atoi(max_age));
		br_set_bridge_forward_delay(bridge, atoi(forward_delay));
		if (prio)
			br_set_bridge_prio(bridge, atoi(prio));

		sprintf(hwaddr, "%s_hwaddr", bridge);
		if (strcmp(bridge, "br0") && *(nvram_safe_get(hwaddr))) {
			set_hwaddr(bridge, nvram_safe_get(hwaddr));
		} else {
			set_hwaddr(bridge, nvram_safe_get("lan_hwaddr"));
		}
		eval("ifconfig", bridge, "up");
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif

	start_set_routes();
}

extern char *getMTU(char *);

char *getRealBridge(char *ifname, char *word)
{
	char *next, *wordlist;
	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		if (!tag || !port)
			break;
		if (!strcmp(port, ifname)) {
			strcpy(word, tag);
			return word;
		}
	}
	return NULL;
}

void stop_bridgesif(void)
{
	char word[256];
	char *next, *wordlist;
#ifdef HAVE_MICRO
	br_init();
#endif

	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		GETENTRYBYIDX(port, word, 1);
		if (!tag || !port)
			break;
		if (ifexists(port))
			br_del_interface(tag, port);
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif
}

void stop_bridging(void)
{
	static char word[256];
	char *next, *wordlist;
#ifdef HAVE_MICRO
	br_init();
#endif

	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next)
	{
		GETENTRYBYIDX(tag, word, 0);
		if (!tag)
			break;
		if (ifexists(tag)) {
			eval("ifconfig", tag, "down");
			br_del_bridge(tag);
		}
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif
}

#else
char *getRealBridge(char *ifname, char *word)
{
	return NULL;
}

#endif

int getbridge_main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "syntax: getbridge [ifname]\n");
		return -1;
	}
	char tmp[256];
	char *bridge = getBridge(argv[1], tmp);

	fprintf(stdout, "%s\n", bridge);
	return 0;
}

int setportprio_main(int argc, char *argv[])
{
	char word[256];
	char *next, *wordlist;
	if (argc < 3) {
		fprintf(stderr, "syntax: setportprio [bridge] [ifname]\n");
		return -1;
	}
#ifdef HAVE_MICRO
	br_init();
#endif
	wordlist = nvram_safe_get("bridgesif");
	foreach(word, wordlist, next)
	{
		char *tag = argv[1];
		GETENTRYBYIDX(port, word, 1);
		GETENTRYBYIDX(prio, word, 2);
		GETENTRYBYIDX(hairpin, word, 3);
		GETENTRYBYIDX(stp, word, 4);
		GETENTRYBYIDX(pathcost, word, 5);
		if (!port)
			continue;
		if (!strcmp(port, argv[2])) {
			if (prio)
				br_set_port_prio(tag, port, atoi(prio));
			if (hairpin)
				br_set_port_hairpin(tag, port, atoi(hairpin));
			if (stp)
				br_set_port_stp(tag, port, atoi(stp));
			if (pathcost)
				br_set_path_cost(tag, port, atoi(pathcost));
		}
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif
	return 0;
}
