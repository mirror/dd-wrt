/*
 * smartdns.c
 *
 * Copyright (C) 2020 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_SMARTDNS
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlutils.h>
#include <services.h>

void start_smartdns(void)
{
	if (!nvram_matchi("smartdns", 1))
		return;
	FILE *fp = fopen("/tmp/smartdns.conf", "wb");
	int port = 6053;
	int ipv6 = nvram_matchi("ipv6_enable", 1);
	int tcp_server = nvram_matchi("smartdns_tcp", 1);
	char *hostname = nvram_safe_get("wan_hostname");
	if (*hostname)
		fprintf(fp, "server-name %s\n", hostname);
	if (ipv6)
		fprintf(fp, "bind [::]:%d\n", port);
	else
		fprintf(fp, "bind :%d\n", port);
	if (tcp_server) {
		if (ipv6)
			fprintf(fp, "bind-tcp [::]:%d\n", port);
		else
			fprintf(fp, "bind-tcp :%d\n", port);
	}

	if (nvram_matchi("smartdns_dualstack_ip_selection", 1))
		fprintf(fp, "dualstack-ip-selection yes\n");
	if (nvram_matchi("smartdns_prefetch_domain", 1))
		fprintf(fp, "prefetch-domain yes\n");
	if (nvram_matchi("smartdns_serve_expired", 1))
		fprintf(fp, "serve-expired yes\n");
	fprintf(fp, "log-size 64K\n");
	fprintf(fp, "log-num 1\n");
	fprintf(fp, "log-level error\n");
	fprintf(fp, "log-file /tmp/smartdns.log\n");
	struct dns_lists *dns_list = NULL;
	dns_list = get_dns_list();

	if (dns_list && dns_list->num_servers > 0) {
		int i;
		for (i = 0; i < dns_list->num_servers; i++)
			fprintf(fp, "server %s\n", dns_list->dns_server[i]);
	}
	if (nvram_matchi("ipv6_enable", 1)) {
		char *a1 = nvram_safe_get("ipv6_dns1");
		char *a2 = nvram_safe_get("ipv6_dns2");
		if (*a1)
			fprintf(fp, "server %s\n", a1);
		if (*a2)
			fprintf(fp, "server %s\n", a2);

		char *next, *wordlist = nvram_safe_get("ipv6_get_dns");
		char word[64];
		foreach(word, wordlist, next) {
			fprintf(fp, "server %s\n", word);
		}
	}

	free_dns_list(dns_list);

	fclose(fp);
	eval("smartdns", "-c", "/tmp/smartdns.conf");
	dd_loginfo("smartdns", "daemon successfully started\n");
}

void stop_smartdns(void)
{
	stop_process("smartdns", "daemon");
}
#endif
