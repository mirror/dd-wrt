/*
 * tor.c
 *
 * Copyright (C) 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#ifdef HAVE_TOR

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <bcmnvram.h>
#include <shutils.h>
#include "snmp.h"
#include <signal.h>
#include <utils.h>
#include <services.h>

char *tor_deps(void)
{
	return "tor_enable lan_ipaddr tor_address tor_id tor_bwrate tor_bwburst tor_relay tor_relayonly tor_dir tor_bridge enable_jffs2 jffs_mounted";
}

char *tor_proc(void)
{
	return "tor";
}

void stop_tor(void)
{
	stop_process("tor", "daemon");
	nvram_delstates(tor_deps());
}

void start_tor(void)
{
	int ret;
	pid_t pid;
	char *tor_argv[] = { "tor", "--defaults-torrc", "/tmp/torrc",
		NULL
	};

	stop_tor();

	if (nvram_matchi("tor_enable", 0))
		return;

	mkdir("/tmp/tor", 0700);
	FILE *fp = fopen("/tmp/torrc", "wb");
	fprintf(fp, "Log notice syslog\n");
#ifdef HAVE_IPV6
	const char *ipv6addr = NULL;
	if (nvram_match("ipv6_typ", "ipv6native"))
		ipv6addr = getifaddr(get_wan_face(), AF_INET6, 0);
	if (nvram_match("ipv6_typ", "ipv6in4"))
		ipv6addr = getifaddr("ip6tun", AF_INET6, 0);
	if (nvram_match("ipv6_typ", "ipv6pd"))
		ipv6addr = getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, 0);
#endif

	if (nvram_matchi("tor_relayonly", 1)) {
		fprintf(fp, "SocksPort 0\n");
	} else {
		fprintf(fp, "SocksPort 9050\n");
		fprintf(fp, "SocksPort %s:9050\n", nvram_safe_get("lan_ipaddr"));
	}
	fprintf(fp, "RunAsDaemon 1\n");
	fprintf(fp, "Address %s\n", nvram_invmatch("tor_address", "") ? nvram_safe_get("tor_address") : get_wan_ipaddr());
	if (nvram_invmatch("tor_id", ""))
		fprintf(fp, "Nickname %s\n", nvram_safe_get("tor_id"));
	if (nvram_invmatch("tor_bwrate", ""))
		fprintf(fp, "RelayBandwidthRate %d\n", nvram_geti("tor_bwrate") * 1024);
	if (nvram_invmatch("tor_bwburst", ""))
		fprintf(fp, "RelayBandwidthBurst %d\n", nvram_geti("tor_bwburst") * 1024);

//      fprintf(fp, "ControlPort 9051\n");
	if (nvram_matchi("tor_relay", 1)) {
		eval("iptables", "-I", "INPUT", "-p", "tcp", "-i", get_wan_face(), "--dport", "9001", "-j", "ACCEPT");
		fprintf(fp, "ORPort 9001\n");
		fprintf(fp, "ExitRelay 1\n");
#ifdef HAVE_IPV6
		if (ipv6addr) {
			fprintf(fp, "ORPort %s:9001\n", ipv6addr);
			fprintf(fp, "IPv6Exit 1\n");
		}
#endif

	}
	if (nvram_matchi("tor_dir", 1)) {
		eval("iptables", "-I", "INPUT", "-p", "tcp", "-i", get_wan_face(), "--dport", "9030", "-j", "ACCEPT");
		fprintf(fp, "DirPort 9030\n");
	}
	if (nvram_matchi("tor_bridge", 1))
		fprintf(fp, "BridgeRelay 1\n");

	fprintf(fp, "VirtualAddrNetworkIPv4 10.192.0.0/10\n");
	fprintf(fp, "AutomapHostsOnResolve 1\n");
	fprintf(fp, "TransPort %s:9040\n", nvram_safe_get("lan_ipaddr"));
	fprintf(fp, "DNSPort %s:5353\n", nvram_safe_get("lan_ipaddr"));
	if (nvram_matchi("tor_transparent", 1)) {
		sysprintf("iptables -t nat -A PREROUTING -i br0 -p udp --dport 53 -j DNAT --to %s:5353", nvram_safe_get("lan_ipaddr"));
		sysprintf("iptables -t nat -A PREROUTING -i br0 -p udp --dport 5353 -j DNAT --to %s:5353", nvram_safe_get("lan_ipaddr"));
		sysprintf("iptables -t nat -A PREROUTING -i br0 -p tcp --syn -j DNAT --to %s:9040", nvram_safe_get("lan_ipaddr"));
	}
#ifdef HAVE_X86
	eval("mkdir", "-p", "/tmp/tor");
	fprintf(fp, "DataDirectory /tmp/tor\n");
#else
	if (nvram_matchi("enable_jffs2", 1)
	    && nvram_matchi("jffs_mounted", 1)
	    && nvram_matchi("sys_enable_jffs2", 1)) {
		eval("rm", "-rf", "/jffs/tor");
		eval("mkdir", "-p", "/jffs/tor");
		fprintf(fp, "DataDirectory /jffs/tor\n");
	} else {
		eval("rm", "-rf", "/tmp/tor");
		fprintf(fp, "DataDirectory /tmp/tor\n");
	}
#endif

	fclose(fp);
	ret = _evalpid(tor_argv, NULL, 0, &pid);
}

#endif				/* HAVE_WOL */
