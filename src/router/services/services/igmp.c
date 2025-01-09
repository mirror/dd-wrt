/*
 * igmp.c
 *
 * Copyright (C) 2007 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_MULTICAST
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_igmprt(void);
void start_igmprt(void)
{
	int ret = 0;
	char wan_if_buffer[33];
	char name[80], *next, *svbuf;
	char *argv[] = { "igmprt", "/tmp/igmpproxy.conf", NULL };

	int ifcount = 0;
	if (nvram_match("wan_proto", "disabled") || !*(safe_get_wan_face(wan_if_buffer))) // todo: add upstream
		return;

	FILE *fp = fopen("/tmp/igmpproxy.conf", "wb");
	int fromvlan = 0;
	fromvlan |= (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("wan_vdsl", 1));
#ifdef HAVE_PPTP
	fromvlan |= (nvram_match("wan_proto", "pptp") && nvram_exists("tvnicfrom"));
#endif
#ifdef HAVE_L2TP
	fromvlan |= (nvram_match("wan_proto", "l2tp") && nvram_exists("tvnicfrom"));
#endif
#ifdef HAVE_PPPOEDUAL
	fromvlan |= (nvram_match("wan_proto", "pppoe_dual") && nvram_exists("tvnicfrom"));
#endif
	fromvlan |= (nvram_match("wan_proto", "dhcp_auth") && nvram_exists("tvnicfrom"));

	if ((nvram_matchi("dtag_bng", 1) && nvram_matchi("wan_vdsl", 1) && nvram_match("wan_proto", "pppoe")) || !fromvlan) {
		fprintf(fp, "quickleave\nphyint %s upstream  ratelimit 0  threshold 1\n", safe_get_wan_face(wan_if_buffer));
	} else {
		fprintf(fp, "quickleave\nphyint %s upstream  ratelimit 0  threshold 1 altnet 0.0.0.0/0\n",
			nvram_safe_get("tvnicfrom"));
		fprintf(fp, "phyint %s disabled\n", safe_get_wan_face(wan_if_buffer));
	}
	if (nvram_matchi("block_multicast", 0)) {
		fprintf(fp, "phyint %s downstream  ratelimit 0  threshold 1\n", nvram_safe_get("lan_ifname"));
		ifcount++;
	} else {
		fprintf(fp,
			"phyint %s disabled\n"
			"phyint %s:0 disabled\n",
			nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ifname"));
	}
	char ifnames[256];

	getIfLists(ifnames, 256);
	foreach(name, ifnames, next)
	{
		if (strcmp(safe_get_wan_face(wan_if_buffer), name) && strcmp(nvram_safe_get("lan_ifname"), name) &&
		    strcmp(nvram_safe_get("tvnicfrom"), name)) {
			if ((nvram_nmatch("0", "%s_bridged", name) || isbridge(name)) && nvram_nmatch("1", "%s_multicast", name)) {
				fprintf(fp, "phyint %s downstream  ratelimit 0  threshold 1\n", name);
				ifcount++;
			} else
				fprintf(fp, "phyint %s disabled\n", name);
		}
	}
	fprintf(fp, "phyint lo disabled\n");
	fclose(fp);
	if (ifcount) {
		if (pidof("igmprt") < 1) {
			_log_evalpid(argv, NULL, 0, NULL);
		}
	}

	return;
}

void stop_igmprt(void)
{
	stop_process("igmprt", "multicast daemon");
	return;
}
#endif
