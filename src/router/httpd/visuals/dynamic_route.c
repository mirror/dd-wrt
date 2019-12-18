/*
 * dynamic_route.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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

/*
 * Dump route in <tr><td>IP</td><td>MASK</td><td>GW</td><td>Hop
 * Count</td><td>interface</td></tr> format 
 */
void ej_dump_route_table(webs_t wp, int argc, char_t ** argv)
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
	if ((fp = fopen("/proc/net/route", "r")) == NULL) {
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
		if (count) {
			int ifl = 0;

			while (line[ifl] != ' ' && line[ifl] != '\t' && line[ifl] != '\0')
				ifl++;
			line[ifl] = 0;	/* interface */
			if (sscanf(line + ifl + 1, "%x%x%X%d%d%d%x", &dest, &gw, &flgs, &ref, &use, &metric, &netmask) != 7) {
				break;
			}
			debug = 0;
			dest_ip.s_addr = dest;
			gw_ip.s_addr = gw;
			netmask_ip.s_addr = netmask;
			char client[32];
			strcpy(sdest, (dest_ip.s_addr == 0 ? "default" : inet_ntop(AF_INET, &dest_ip, client, 16)));	// default
			strcpy(sgw, (gw_ip.s_addr == 0 ? "*" : inet_ntop(AF_INET, &gw_ip, client, 16)));	// *

			/*
			 * not 0x0001 route usable 
			 */
			if (!(flgs & RTF_UP))
				continue;

			/*
			 * filter ppp pseudo interface for DOD 
			 */
			if (!strcmp(sdest, PPP_PSEUDO_IP)
			    && !strcmp(sgw, PPP_PSEUDO_GW))
				debug = 1;

			/*
			 * Don't show loopback device 
			 */
			if (!strcmp(line, "lo"))
				debug = 1;

			/*
			 * Don't show eth1 information for pppoe mode 
			 */
			if (!strcmp(line, nvram_safe_get("wan_ifname"))
			    && (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pppoe_dual")))
				debug = 1;

			/*
			 * Don't show pseudo interface 
			 */
			if (!strncmp(line, "ppp", 3)) {
				if (!f_exists("/tmp/ppp/link"))
					debug = 1;
			}
			char *ifname = line;

			if (!strcmp(ifname, nvram_safe_get("lan_ifname")))
				ifname = "LAN";
			if (!strcmp(ifname, nvram_safe_get("wan_ifname")))
				ifname = "WAN";
			char flags[32];
			int fidx = 0;
			flags[fidx++] = 'U';
			if (flgs & RTF_GATEWAY)
				flags[fidx++] = 'G';
			if (flgs & RTF_HOST)
				flags[fidx++] = 'H';
			if (flgs & RTF_REINSTATE)
				flags[fidx++] = 'R';
			if (flgs & RTF_DYNAMIC)
				flags[fidx++] = 'D';
			if (flgs & RTF_MODIFIED)
				flags[fidx++] = 'M';
			flags[fidx] = 0;

			websWrite(wp, "%s%c'%s','%s','%s','%s','%d','%s'\n", debug ? "//" : "", blank ? ' ' : ',', sdest, inet_ntop(AF_INET, &netmask_ip, client, 16), sgw, flags, metric, getNetworkLabel(wp, ifname));

			if (debug && blank)
				blank = 1;
			else
				blank = 0;

		}

		count++;
	}
	fclose(fp);
	return;
}
