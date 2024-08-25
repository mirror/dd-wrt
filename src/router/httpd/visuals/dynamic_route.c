/*
 * dynamic_route.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <shutils.h>
#include <broadcom.h>

#define GETFIELD(name)              \
	if (d_##name) {             \
		strcpy(name, word); \
		d_##name = 0;       \
		field++;            \
		continue;           \
	}                           \
	if (!strcmp(word, #name)) { \
		d_##name = 1;       \
		field++;            \
		continue;           \
	}

/*
 * Dump route in <tr><td>IP</td><td>MASK</td><td>GW</td><td>Hop
 * Count</td><td>interface</td></tr> format 
 */
EJ_VISIBLE void ej_dump_route_table(webs_t wp, int argc, char_t **argv)
{
	int count = 0;
	FILE *fp;
	int flgs, ref, use, metric;
	unsigned int dest, gw, netmask;
	char line[256];
	struct in_addr dest_ip;
	struct in_addr gw_ip;
	struct in_addr netmask_ip;
	char sdest[16], sgw[16];
	int debug = 0, blank = 1;

	/*
	 * open route table 
	 */
	if ((fp = popen("ip route show table all", "r")) == NULL) {
		websError(wp, 400, "No route table\n");
		return;
	}

	/*
	 * Read the route cache entries. 
	 */
	// Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT
	//
	// vmnet1 004410AC 00000000 0001 0 0 0 00FFFFFF 40 0 0

	while (fgets(line, sizeof(line), fp) != NULL) {
		char *next;
		char word[128];
		int field = 0;
		char net[128] = { 0 };
		char via[128] = { 0 };
		char dev[32] = { 0 };
		char scope[32] = { 0 };
		char table[32] = { 0 };
		strcpy(table, "default");
		char src[32] = { 0 };
		char metric[32] = { 0 };
		strcpy(metric, "0");
		int d_dev = 0;
		int d_table = 0;
		int d_scope = 0;
		int d_src = 0;
		int d_via = 0;
		int d_metric = 0;
		foreach(word, line, next)
		{
			if (!field && !strcmp(word, "broadcast"))
				goto nextline;
			if (!field && !strcmp(word, "local"))
				goto nextline;
			if (!field && !strcmp(word, "unreachable"))
				goto nextline;
			if (!strcmp(word, "cache"))
				goto nextline;
			if (!strcmp(word, "ff00::/8"))
				goto nextline;
			if (!strcmp(word, "fe80::/64"))
				goto nextline;
			if (!field) {
				strcpy(net, word);
				field++;
				continue;
			}
			GETFIELD(dev);
			GETFIELD(table);
			GETFIELD(src);
			GETFIELD(metric);
			GETFIELD(scope);
			GETFIELD(via);
			field++;
		}

		if (!strcmp(dev, nvram_safe_get("lan_ifname")))
			strcpy(dev, "LAN");
		if (!strcmp(dev, nvram_safe_get("wan_ifname")))
			strcpy(dev, "WAN");
		websWrite(wp, "%c'%s','%s','%s','%s','%s','%s','%s'\n", blank ? ' ' : ',', net, via, table, scope, metric,
			  getNetworkLabel(wp, dev), src);
		blank = 0;
nextline:;
	}
	pclose(fp);
	return;
}

#ifndef HAVE_MICRO
EJ_VISIBLE void ej_dump_pbr_table(webs_t wp, int argc, char_t **argv)
{
	int count = 0;
	FILE *fp;
	int flgs, ref, use, metric;
	unsigned int dest, gw, netmask;
	char line[256];
	struct in_addr dest_ip;
	struct in_addr gw_ip;
	struct in_addr netmask_ip;
	char sdest[16], sgw[16];
	int debug = 0, blank = 1;

	/*
	 * open route table 
	 */
	if ((fp = popen("ip rule show", "r")) == NULL) {
		websError(wp, 400, "No PBR table\n");
		return;
	}

	/*
	 * Read the route cache entries. 
	 */
	// Iface Destination Gateway Flags RefCnt Use Metric Mask MTU Window IRTT
	//
	// vmnet1 004410AC 00000000 0001 0 0 0 00FFFFFF 40 0 0

	while (fgets(line, sizeof(line), fp) != NULL) {
		char *next;
		char word[128];
		int field = 0;
		char priority[16] = { 0 };
		char from[128] = { 0 };
		char to[128] = { 0 };
		char tos[32] = { 0 };
		char fwmark[32] = { 0 };
		char ipproto[32] = { 0 };
		char sport[32] = { 0 };
		char dport[32] = { 0 };
		char iif[32] = { 0 };
		char oif[32] = { 0 };
		char lookup[32] = { 0 };
		char nat[128] = { 0 };
		strcpy(lookup, "default");

		int not = 0;
		int d_from = 0;
		int d_to = 0;
		int d_tos = 0;
		int d_fwmark = 0;
		int d_ipproto = 0;
		int d_sport = 0;
		int d_dport = 0;
		int d_iif = 0;
		int d_oif = 0;
		int d_lookup = 0;
		int d_nat = 0;
		foreach_delim(word, line, next, " \t")
		{
			if (!field) {
				strcpy(priority, word);
				field++;
				continue;
			}
			if (!strcmp(word, "not")) {
				not = 1;
				field++;
				continue;
			}
			GETFIELD(from);
			GETFIELD(to);
			GETFIELD(tos);
			GETFIELD(fwmark);
			GETFIELD(ipproto);
			GETFIELD(sport);
			GETFIELD(dport);
			GETFIELD(iif);
			GETFIELD(oif);
			GETFIELD(lookup);
			GETFIELD(nat);
			field++;
		}
		char *p = strchr(priority, ':');
		*p = 0;
		if (iif[0] && !strcmp(iif, nvram_safe_get("lan_ifname")))
			strcpy(iif, "LAN");
		if (iif[0] && !strcmp(iif, nvram_safe_get("wan_ifname")))
			strcpy(iif, "WAN");
		if (oif[0] && !strcmp(oif, nvram_safe_get("lan_ifname")))
			strcpy(oif, "LAN");
		if (oif[0] && !strcmp(oif, nvram_safe_get("wan_ifname")))
			strcpy(oif, "WAN");
		websWrite(wp, "%c'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s'\n", blank ? ' ' : ',', priority,
			  not ? "!" : "", from, to, tos, fwmark, ipproto, sport, dport, getNetworkLabel(wp, iif),
			  getNetworkLabel(wp, oif), lookup, nat);
		blank = 0;
nextline:;
	}
	pclose(fp);
	return;
}
#endif
