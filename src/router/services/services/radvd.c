/*
 * radvd.c
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <sys/ioctl.h>		/* AhMan March 18 2005 */
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h>		/* AhMan March 18 2005 */
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

#ifdef HAVE_RADVD

static int write_ipv6_dns_servers(FILE * f, const char *prefix, char *dns, const char *suffix, int once)
{
	char p[INET6_ADDRSTRLEN + 1], *next = NULL;
	struct in6_addr addr;
	int cnt = 0;

	foreach(p, dns, next) {
		// verify that this is a valid IPv6 address
		if (inet_pton(AF_INET6, p, &addr) == 1) {
			fprintf(f, "%s%s%s", (once && cnt) ? "" : prefix, p, suffix);
			++cnt;
		}
		if (cnt == 3)
			break;
	}

	return cnt;
}

void start_radvd(void)
{
	int c = 0, manual = 0;
	int mtu = 1500;;
	char *buf, *prefix;
	const char *ip;
	char *p = NULL;
	int do_mtu = 0, do_6to4 = 0, do_6rd = 0;
	FILE *fp;

	if (!nvram_matchi("ipv6_enable", 1) || !nvram_matchi("radvd_enable", 1))
		return;

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

		ip = getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, GIF_LINKLOCAL) ? : "";

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
			" };\n", nvram_safe_get("lan_ifname"), manual ? "on" : "off", mtu, prefix, manual ? "off" : "on", do_6to4 ? "  Base6to4Interface " : "", do_6to4 ? get_wan_face() : "", do_6to4 ? ";\n" : "");

		char ipv6_dns_str[1024] = "";
		char nvname[sizeof("ipv6_dnsXXX")];
		char *next = ipv6_dns_str;
		int i, cnt;

		ipv6_dns_str[0] = '\0';
		for (i = 0; i < 2; i++) {
			snprintf(nvname, sizeof(nvname), "ipv6_dns%d", i + 1);
			p = nvram_safe_get(nvname);
			if (!*p)
				continue;

			next += sprintf(next, *ipv6_dns_str ? " %s" : "%s", p);
		}

		if (!strcmp(nvram_safe_get("ipv6_typ"), "ipv6pd"))
			p = nvram_safe_get("ipv6_get_dns");
		else
			p = ipv6_dns_str;
		if (nvram_matchi("dnsmasq_enable", 1)) {
			fprintf(fp, " RDNSS %s ", getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, GIF_LINKLOCAL));
			fprintf(fp, "{};\n");
		} else {
			cnt = write_ipv6_dns_servers(fp, " RDNSS ", (char *)((p && *p) ? p : ip), " ", 1);
			if (cnt)
				fprintf(fp, "{};\n");
		}
		fprintf(fp, "};\n");
		fclose(fp);
	}

	eval("radvd", "-C", "/tmp/radvd.conf");
	dd_loginfo("radvd", "RADVD daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_radvd(void)
{

	stop_process("radvd", "daemon");
	unlink("/var/run/radvd.pid");

	cprintf("done\n");
	return;
}
#endif
