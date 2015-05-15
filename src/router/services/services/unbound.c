/*
 * unbound.c
 *
 * Copyright (C) 2015 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_UNBOUND
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

static void unbound_config(void)
{
	FILE *fp = fopen("/tmp/unbound.conf", "wb");
	fprintf(fp, "server:\n"	//
		"verbosity: 1\n"	//
		"interface: 0.0.0.0\n"	//
		"interface: ::0\n"	//
		"outgoing-range: 60\n"	//
		"outgoing-num-tcp: 1\n"	//
		"incoming-num-tcp: 1\n"	//
		"msg-buffer-size: 8192\n"	//
		"msg-cache-size: 100k\n"	//
		"msg-cache-slabs: 1\n"	//
		"num-queries-per-thread: 30\n"	//
		"rrset-cache-size: 100k\n"	//
		"rrset-cache-slabs: 1\n"	//
		"infra-cache-slabs: 1\n"	//
		"infra-cache-numhosts: 200\n"	//
		"access-control: 0.0.0.0/0 allow\n"	//
		"access-control: ::0/0 allow\n"	//
		"username: \"\"\n"	//
		"pidfile: \"/var/run/unbound.pid\"\n"	//
		"root-hints: \"/etc/unbound/named.cache\"\n"	//
		"target-fetch-policy: \"2 1 0 0 0 0\"\n"	//
		"harden-short-bufsize: yes\n"	//
		"harden-large-queries: yes\n"	//
		"auto-trust-anchor-file: \"/etc/unbound/root.key\"\n"	//
		"key-cache-size: 100k\n"	//
		"key-cache-slabs: 1\n"	//
		"neg-cache-size: 10k\n");	//

	FILE *in = fopen("/etc/hosts", "rb");
	char ip[32];
	char name[128];
	while (fscanf(in, "%s %s", &ip[0], &name[0]) == 2) {
		fprintf(fp, "local-data: \"%s A %s\"\n", name, ip);
	}
	fclose(in);
	fprintf(fp, "python:\n");
	fprintf(fp, "remote-control:\n");
	fclose(fp);

	int leasenum = atoi(nvram_safe_get("static_leasenum"));

	if (leasenum > 0) {
		char *lease = nvram_safe_get("static_leases");
		char *leasebuf = (char *)malloc(strlen(lease) + 1);
		char *cp = leasebuf;

		strcpy(leasebuf, lease);
		int i;
		for (i = 0; i < leasenum; i++) {
			char *mac = strsep(&leasebuf, "=");
			char *host = strsep(&leasebuf, "=");
			char *ip = strsep(&leasebuf, "=");
			char *time = strsep(&leasebuf, " ");

			if (mac == NULL || host == NULL || ip == NULL)
				continue;

			addHost(host, ip, 1);
		}
		free(cp);
	}

}

void start_unbound(void)
{
	if (nvram_match("recursive_dns", "1")) {
		unbound_config();
		eval("unbound", "-c", "/tmp/unbound.conf");
		dd_syslog(LOG_INFO, "unbound : recursive dns resolver daemon successfully started\n");
	}
	return;
}

void stop_unbound(void)
{
	stop_process("unbound", "unbound daemon");
	return;
}
#endif
