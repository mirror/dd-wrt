/*
 * radvd.c
 *
 * Copyright (C) 2009 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h> /* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>
#include <syslog.h>
#include <services.h>

#if defined(HAVE_RADVD) && defined(HAVE_IPV6)

void start_radvd(void)
{
	char wan_if_buffer[33];
	int c = 0, manual = 0;
	int mtu = 1500;
	;
	char *buf, *prefix;
	const char *ip;
	char *p = NULL;
	int do_mtu = 0, do_6to4 = 0, do_6rd = 0;
	FILE *fp;

	if (!nvram_matchi("ipv6_enable", 1) || !nvram_matchi("radvd_enable", 1)) {
		stop_radvd();
		return;
	}

	if (nvram_matchi("dhcp6s_enable", 1) && (nvram_matchi("dhcp6s_seq_ips", 1) || nvram_invmatch("dhcp6s_hosts", "")))
		manual = 1;

	if (nvram_invmatch("ipv6_mtu", "")) {
		do_mtu = 1;
		mtu = nvram_geti("ipv6_mtu");
	} else {
		mtu = nvram_geti("wan_mtu") - 20;
	}

	if ((fp = fopen("/proc/sys/net/ipv6/conf/all/forwarding", "r+"))) {
		fputc('1', fp);
		fclose(fp);
	}

	if (nvram_matchi("radvd_custom", 1)) {
		buf = nvram_safe_get("radvd_conf");
		if (buf != NULL)
			writenvram("radvd_conf", "/tmp/radvd.conf");
	} else {
		if (nvram_match("ipv6_typ", "ipv6native")) {
			if (do_mtu) {
				mtu = nvram_geti("ipv6_mtu");
			} else {
				mtu = nvram_geti("wan_mtu");
			}
		}

		if (nvram_match("ipv6_typ", "ipv6to4")) {
			do_6to4 = 1;
		}

		if (!strcmp(nvram_safe_get("ipv6_typ"), "ipv6rd")) {
			do_6rd = 1;
		}

		prefix = do_6to4 ? "0:0:0:1::" : nvram_safe_get("ipv6_prefix");

		if (!(*prefix) || (strlen(prefix) <= 0))
			prefix = "::";
		if ((fp = fopen("/tmp/radvd.conf", "w")) == NULL)
			return;

		char buf[INET6_ADDRSTRLEN];
		ip = getifaddr_any(buf, nvram_safe_get("lan_ifname"), AF_INET6) ?: "";

		fprintf(fp,
			"interface %s\n"
			"{\n"
			" IgnoreIfMissing on;\n"
			" AdvSendAdvert on;\n"
			" MinRtrAdvInterval 3;\n"
			" MaxRtrAdvInterval 10;\n"
			" AdvHomeAgentFlag off;\n"
			" AdvManagedFlag %s;\n"
			" AdvOtherConfigFlag on;\n"
			" AdvLinkMTU %d;\n"
			" prefix %s/64 \n"
			" {\n"
			"  AdvOnLink on;\n"
			"  AdvAutonomous %s;\n"
			"  AdvValidLifetime 30;\n"
			"  AdvPreferredLifetime 20;\n"
			"%s%s%s"
			" };\n",
			nvram_safe_get("lan_ifname"), manual ? "on" : "off", mtu, prefix, manual ? "off" : "on",
			do_6to4 ? "  Base6to4Interface " : "", do_6to4 ? safe_get_wan_face(wan_if_buffer) : "",
			do_6to4 ? ";\n" : "");

		int i;

		struct dns_lists *dns_list = get_dns_list(2);

		if (dns_list && dns_list->num_servers) {
			fprintf(fp, " RDNSS");
			for (i = 0; i < dns_list->num_servers; i++) {
				fprintf(fp, " %s", dns_list->dns_server[i].ip);
			}
			fprintf(fp, "{};\n");
		}
		if (dns_list)
			free_dns_list(dns_list);

		fprintf(fp, "};\n");
		fclose(fp);
	}
	if (reload_process("radvd")) {
		log_eval("radvd", "-C", "/tmp/radvd.conf");
	}
	cprintf("done\n");
	return;
}

void restart_radvd(void)
{
	start_radvd();
}

void stop_radvd(void)
{
	stop_process("radvd", "daemon");
	unlink("/var/run/radvd.pid");

	cprintf("done\n");
	return;
}
#endif
