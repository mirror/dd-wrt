/*
 * smartdns.c
 *
 * Copyright (C) 2020 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
	char path[64];
	if (!nvram_matchi("smartdns", 1))
		return;
	FILE *fp = fopen("/tmp/smartdns.conf", "wb");
	int port = 6053;
	if (!nvram_matchi("dns_dnsmasq", 1))
		port = 53;
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
	//fprintf(fp, "log-size 64K\n");
	fprintf(fp, "log-size 32K\n");
	fprintf(fp, "log-num 1\n");
	fprintf(fp, "log-level warn\n");
	fprintf(fp, "log-file /tmp/smartdns.log\n");
	if (!nvram_matchi("dns_dnsmasq", 1) && !nvram_matchi("dhcpd_usenvram", 1)) {
		int usejffs = 0;
		if (jffs_mounted() && nvram_matchi("dhcpd_usejffs", 1)) {
			FILE *fpcheck;
			if (!(fpcheck = fopen("/jffs/dnsmasq.leases", "a"))) {
				usejffs = 0;
			} else {
				fclose(fpcheck);
				usejffs = 1;
			}
		}
		if (usejffs)
			fprintf(fp, "dnsmasq-lease-file /jffs/dnsmasq.leases\n");
		else
			fprintf(fp, "dnsmasq-lease-file /tmp/dnsmasq.leases\n");
	}
//egc: do we really need these certificates and if so are these the right ones?
#ifdef HAVE_HTTPS
	fprintf(fp, "ca-file /etc/ssl/ca-bundle.crt\n");
	fprintf(fp, "ca-path /etc/ssl\n");
#endif
#ifdef HAVE_TOR
	if (nvram_match("tor_enable", "1"))
		fprintf(fp, "server %s:5353\n", nvram_safe_get("lan_ipaddr"));
#endif
	struct dns_lists *dns_list = NULL;
	if (nvram_matchi("recursive_dns", 1)) {
		fprintf(fp, "server 127.0.0.1:7053\n");
	} else if (nvram_matchi("smartdns_use_dns", 0)) {
		dns_list = get_dns_list(1);
		if (dns_list && dns_list->num_servers > 0) {
			int i;
			for (i = 0; i < dns_list->num_servers; i++)
				fprintf(fp, "server %s\n", dns_list->dns_server[i].ip);
		}
		if (dns_list)
			free_dns_list(dns_list);
	}
	fwritenvram("smartdns_options", fp);
	fclose(fp);

	log_eval("smartdns", "-c", getdefaultconfig("smartdns", path, sizeof(path), "smartdns.conf"));
}

void stop_smartdns(void)
{
	stop_process("smartdns", "daemon");
}
#endif
