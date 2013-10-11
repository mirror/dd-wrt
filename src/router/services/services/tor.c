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
#include <bcmnvram.h>
#include <shutils.h>
#include "snmp.h"
#include <signal.h>
#include <utils.h>
#include <services.h>

void stop_tor(void)
{
	stop_process("tor", "Stopping TOR");
}

void start_tor(void)
{
	int ret;
	pid_t pid;
	char *tor_argv[] = { "tor", "--defaults-torrc", "/tmp/torrc",
		NULL
	};

	stop_tor();

	if (nvram_match("tor_enable", "0"))
		return;
	
	mkdir("/tmp/tor", 0700);
	FILE *fp = fopen("/tmp/torrc", "wb");
	fprintf(fp, "Log notice syslog\n");
	fprintf(tp, "DataDirectory /tmp/tor\n");
	if (nvram_match("tor_relayonly", "1"))
		fprintf(fp, "SocksPort 0\n");
	else {
		fprintf(fp, "SocksPort 9050\n");
		fprintf(fp, "SocksPort %s:9050\n", nvram_safe_get("lan_ipaddr"));
	}
	fprintf(fp, "RunAsDaemon 1\n");
//      fprintf(fp,"ControlPort 9051\n");
	if (nvram_match("tor_relay", "1")) ;
	fprintf(fp, "ORPort 9001\n");
	if (nvram_match("tor_dir", "1"))
		fprintf(fp, "DirPort 9030\n");
	if (nvram_match("tor_bridge", "1"))
		fprintf(fp, "BridgeRelay 1\n");
	if (nvram_match("tor_transparent", "1")) {
		fprintf(fp, "VirtualAddrNetwork 10.192.0.0/10\n");
		fprintf(fp, "AutomapHostsOnResolve 1\n");
		fprintf(fp, "TransPort 9040\n");
		fprintf(fp, "DNSPort 53\n");
		fprintf(fp, "TransListenAddress %s\n", nvram_safe_get("lan_ipaddr"));
		fprintf(fp, "DNSListenAddress %s\n", nvram_safe_get("lan_ipaddr"));
		sysprintf("iptables -t nat -A PREROUTING -i br0 -p udp --dport 53 -j REDIRECT --to-ports 53");
		sysprintf("iptables -t nat -A PREROUTING -i br0 -p tcp --syn -j REDIRECT --to-ports 9040");

	}

	fclose(fp);
	ret = _evalpid(tor_argv, NULL, 0, &pid);

}

#endif				/* HAVE_WOL */
