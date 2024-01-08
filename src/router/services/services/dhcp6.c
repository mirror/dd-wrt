/*
 * dhcp6.c
 *
 * Copyright (C) 2014 Richard Schneidt
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
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/mount.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <net/route.h>
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
#include <time.h>

#ifdef HAVE_IPV6
#define evalip6(cmd, args...)                          \
	{                                              \
		if (nvram_match("ipv6_enable", "1")) { \
			eval_va(cmd, ##args, NULL);    \
		}                                      \
	}
#else
#define evalip6(...)
#endif

void start_dhcp6c(void)
{
	char wan_if_buffer[33];
	FILE *fp, *fpc;
	char *buf;
	int prefix_len;
	char ea[ETHER_ADDR_LEN];
	unsigned long iaid = 0;
	struct {
		uint16 type;
		uint16 hwtype;
	} __attribute__((__packed__)) duid;
	uint16 duid_len = 0;

	if (!nvram_matchi("ipv6_enable", 1))
		return;
	if (!nvram_match("ipv6_typ", "ipv6pd"))
		return;
	nvram_unset("ipv6_prefix");
	nvram_unset("ipv6_rtr_addr");
	nvram_unset("ipv6_get_dns");

	char mac[18];
	getLANMac(mac);
	if (!*mac)
		strcpy(mac, nvram_safe_get("et0macaddr_safe"));

	if (ether_atoe(mac, ea)) {
		/* Generate IAID from the last 7 digits of WAN MAC */
		iaid = ((unsigned long)(ea[3] & 0x0f) << 16) | ((unsigned long)(ea[4]) << 8) | ((unsigned long)(ea[5]));

		/* Generate DUID-LL */
		duid_len = sizeof(duid) + ETHER_ADDR_LEN;
		duid.type = htons(3); /* DUID-LL */
		duid.hwtype = htons(1); /* Ethernet */
	}

	unlink("/var/dhcp6c_duid");
	if ((duid_len != 0) && (fp = fopen("/var/dhcp6c_duid", "w")) != NULL) {
		fwrite(&duid_len, sizeof(duid_len), 1, fp);
		fwrite(&duid, sizeof(duid), 1, fp);
		fwrite(&ea, ETHER_ADDR_LEN, 1, fp);
		fclose(fp);
	}

	if (nvram_matchi("dhcp6c_custom", 1)) {
		if (nvram_exists("dhcp6c_conf"))
			writenvram("dhcp6c_conf", "/tmp/dhcp6c.conf");
	} else {
		prefix_len = 64 - (atoi(nvram_safe_get("ipv6_pf_len")) ?: 64);
		if (prefix_len < 0)
			prefix_len = 0;

		if ((fpc = fopen("/etc/dhcp6c.conf", "w"))) {
			fprintf(fpc,
				"interface %s {\n" //
				" send ia-pd 0;\n" //
				" send rapid-commit;\n" //
				" request domain-name-servers;\n" //
				" script \"/sbin/dhcp6c-state\";\n",
				safe_get_wan_face(wan_if_buffer));

			/*#define DH6OPT_USER_CLASS 15
#define DH6OPT_VENDOR_CLASS 16
#define DH6OPT_CLIENTID	1
#define DH6OPT_ORO 6
#define DH6OPT_AUTH 11
*/
			char *vendorclass = nvram_safe_get("dhcp6c_vendorclass");
			char *userclass = nvram_safe_get("dhcp6c_userclass");
			char *auth = nvram_safe_get("dhcp6c_authentication");
			char *clientid = nvram_safe_get("dhcp6c_clientid");

			if (nvram_match("wan_proto", "dhcp_auth")) {
				fprintf(fpc, "send raw-option 6 00:0b:00:11:00:17:00:18;\n");
				if (*auth) {
					fprintf(fpc, "send raw-option 11 ");
					int i;
					for (i = 0; i < strlen(auth); i += 2) {
						if (i)
							fprintf(fpc, ":");
						fprintf(fpc, "%c%c", auth[i], auth[i + 1]);
					}
					fprintf(fpc, ";\n");
				}

				if (*clientid) {
					fprintf(fpc, "send raw-option 1 ");
					int i;
					for (i = 0; i < strlen(clientid); i += 2) {
						if (i)
							fprintf(fpc, ":");
						fprintf(fpc, "%c%c", clientid[i], clientid[i + 1]);
					}
					fprintf(fpc, ";\n");
				}
				if (*vendorclass) {
					fprintf(fpc, "send raw-option 16 00:00:04:0e:%02X:%02X", strlen(vendorclass) >> 8,
						strlen(vendorclass) & 0xff); // 00:00:04:0e enterprise id for sagecom
					int i;
					for (i = 0; i < strlen(vendorclass); i++)
						fprintf(fpc, ":%02X", vendorclass[i]);
					fprintf(fpc, ";\n");
				}
				if (*userclass) {
					fprintf(fpc, "send raw-option 15 %02X:%02X", strlen(userclass) >> 8,
						strlen(userclass) & 0xff); // must convert to hex
					int i;
					for (i = 0; i < strlen(userclass); i++)
						fprintf(fpc, ":%02X", userclass[i]);
					fprintf(fpc, ";\n");
				}
			}

			fprintf(fpc,
				"};\n"
				"id-assoc pd 0 {\n" //
				" prefix-interface %s {\n" //
				"  sla-id 0;\n" //
				"  sla-len %d;\n" //
				" };\n" //
				"};\n" //
				"id-assoc na 0 { };\n",
				nvram_safe_get("lan_ifname"), prefix_len);
			fclose(fpc);
		}
	}
	char *wan_ifname = safe_get_wan_face(wan_if_buffer);
#ifndef HAVE_MICRO
	if (nvram_match("wan_priority", "1") && isvlan(wan_ifname)) {
		eval("vconfig", "set_egress_map", wan_ifname, "0", "6");
		eval("vconfig", "set_egress_map", wan_ifname, "1", "0");
		insmod("nf_defrag_ipv6 nf_log_ipv6 ip6_tables nf_conntrack_ipv6 ip6table_filter ip6table_mangle xt_DSCP xt_CLASSIFY");
		eval("iptables", "-t", "mangle", "-D", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");

		eval("iptables", "-t", "mangle", "-D", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");

		eval("iptables", "-t", "mangle", "-D", "POSTROUTING", "-m", "--mark", "0x100000", "-j", "CLASSIFY", "--set-class",
		     "0:1");
		eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-m", "--mark", "0x100000", "-j", "CLASSIFY", "--set-class",
		     "0:1");

		evalip6("ip6tables", "-t", "mangle", "-D", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		evalip6("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		evalip6("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");

		evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "CLASSIFY",
			"--set-class", "0:1");
		evalip6("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "CLASSIFY",
			"--set-class", "0:1");

		evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-o", wan_ifname, "-p", "udp", "--dport", "547", "-j",
			"CLASSIFY", "--set-class", "0:0");
		evalip6("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-o", wan_ifname, "-p", "udp", "--dport", "547", "-j",
			"CLASSIFY", "--set-class", "0:0");
	}
#endif
	if (nvram_match("dhcp6c_norelease", "1"))
		log_eval("dhcp6c", "-n", "-c", "/tmp/dhcp6c.conf", "-T", "LL", wan_ifname);
	else
		log_eval("dhcp6c", "-c", "/tmp/dhcp6c.conf", "-T", "LL", wan_ifname);
}

void stop_dhcp6c(void)
{
	stop_process("dhcp6c", "daemon");
}

void start_dhcp6s(void)
{
	FILE *fp;
	pid_t pid;
	const char *p;
	char *buf;
	char ipv6_dns_str[1024] = "";

	char ea[ETHER_ADDR_LEN];
	unsigned long iaid = 0;
	struct {
		uint16 type;
		uint16 hwtype;
	} __attribute__((__packed__)) duid;
	uint16 duid_len = 0;

	if (!nvram_matchi("ipv6_enable", 1))
		return;
	if (!nvram_matchi("dhcp6s_enable", 1))
		return;

	if (ether_atoe(nvram_safe_get("lan_hwaddr"), ea)) {
		/* Generate IAID from the last 7 digits of WAN MAC */
		iaid = ((unsigned long)(ea[3] & 0x0f) << 16) | ((unsigned long)(ea[4]) << 8) | ((unsigned long)(ea[5]));

		/* Generate DUID-LL */
		duid_len = sizeof(duid) + ETHER_ADDR_LEN;
		duid.type = htons(3); /* DUID-LL */
		duid.hwtype = htons(1); /* Ethernet */
	}

	unlink("/var/dhcp6s_duid");
	if ((duid_len != 0) && (fp = fopen("/var/dhcp6s_duid", "w")) != NULL) {
		fwrite(&duid_len, sizeof(duid_len), 1, fp);
		fwrite(&duid, sizeof(duid), 1, fp);
		fwrite(&ea, ETHER_ADDR_LEN, 1, fp);
		fclose(fp);
	}

	if (nvram_matchi("dhcp6s_custom", 1)) {
		if (nvram_exists("dhcp6s_conf"))
			writenvram("dhcp6s_conf", "/tmp/dhcp6s.conf");
	} else {
		if ((fp = fopen("/tmp/dhcp6s.conf", "w")) == NULL)
			return;

		fprintf(fp, "option refreshtime %d;\n", 900); /* 15 minutes for now */
		if (nvram_matchi("dnsmasq_enable", 1)) {
			char buf[INET6_ADDRSTRLEN];
			fprintf(fp, "option domain-name-servers %s", getifaddr_any(buf, nvram_safe_get("lan_ifname"), AF_INET6));
		} else {
			struct dns_lists *list = get_dns_list(1);
			fprintf(fp, "option domain-name-servers");
			if (list) {
				int i;
				for (i = 0; i < list->num_servers; i++)
					fprintf(fp, " %s", list->dns_server[i].ip);
				free_dns_list(list);
			}
		}
		fprintf(fp, ";\n");
		if (nvram_invmatch("ipv6_get_domain", ""))
			fprintf(fp, "option domain-name \"%s\";\n", nvram_safe_get("ipv6_get_domain"));
		if (nvram_matchi("dhcp6s_seq_ips", 1)) {
			fprintf(fp, "\ninterface %s {\n", nvram_safe_get("lan_ifname"));
			fprintf(fp, "\tallow rapid-commit;\n\taddress-pool pool1 30 86400;\n};\n");
			fprintf(fp, "pool pool1 {\n \t range %s1000 to %sffff;\n};\n", nvram_safe_get("ipv6_prefix"),
				nvram_safe_get("ipv6_prefix"));
		} else {
			fprintf(fp, "\ninterface %s {\n", nvram_safe_get("lan_ifname"));
			fprintf(fp, "\tallow rapid-commit;\n};\n");
		}
		if (nvram_invmatch("dhcp6s_hosts", "")) {
			fprintf(fp, "\n%s\n", nvram_safe_get("dhcp6s_hosts"));
		}
		fclose(fp);
	}

	log_eval("dhcp6s", "-c", "/tmp/dhcp6s.conf", nvram_safe_get("lan_ifname"));
}

void stop_dhcp6s(void)
{
	stop_process("dhcp6s", "daemon");
}

static int nvram_change(const char *name, char *value)
{
	char *oldval = nvram_safe_get(name);
	if (value == NULL && !nvram_exists(name))
		return 0;
	if (value && !strcmp(oldval, value))
		return 0;
	nvram_set(name, value);
	return 1;
}

static int getprefixlen(char *dev)
{
	char addr6[40], devname[21];
	struct sockaddr_in6 sap;
	int plen, scope, dad_status, if_idx;
	char addr6p[8][5];

	FILE *f = fopen("/proc/net/if_inet6", "rb");
	if (f == NULL)
		return 64;

	while (fscanf(f, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n", addr6p[0], addr6p[1], addr6p[2], addr6p[3],
		      addr6p[4], addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope, &dad_status, devname) != EOF) {
		if (!strcmp(devname, dev)) {
			fclose(f);
			return plen;
		}
	}
	fclose(f);
	return 64;
}

int dhcp6c_state_main(int argc, char **argv)
{
	char prefix[INET6_ADDRSTRLEN];
	struct in6_addr addr;
	int i, r;
	int c = 0;
	char buf[INET6_ADDRSTRLEN];
	c |= nvram_change("ipv6_rtr_addr", getifaddr(buf, nvram_safe_get("lan_ifname"), AF_INET6, 0));
	//      c |= nvram_change("ipv6_pf_len", getprefixlen(nvram_safe_get("lan_ifname")));
	// extract prefix from configured IPv6 address
	if (inet_pton(AF_INET6, nvram_safe_get("ipv6_rtr_addr"), &addr) > 0) {
		r = nvram_geti("ipv6_pf_len") ?: 64;
		for (r = 128 - r, i = 15; r > 0; r -= 8) {
			if (r >= 8)
				addr.s6_addr[i--] = 0;
			else
				addr.s6_addr[i--] &= (0xff << r);
		}
		inet_ntop(AF_INET6, &addr, prefix, sizeof(prefix));

		c |= nvram_change("ipv6_prefix", prefix);
		if (c) {
			//                      char loopback[128];
			//                      sprintf(loopback, "%s/%d", prefix, r);
			//                      eval("ip", "-6", "route", "unreachable", loopback, "dev", "lo");
		}
	}

	c |= nvram_change("ipv6_get_dns", getenv("new_domain_name_servers"));
	c |= nvram_change("ipv6_get_domain", getenv("new_domain_name"));
	c |= nvram_change("ipv6_get_sip_name", getenv("new_sip_name"));
	c |= nvram_change("ipv6_get_sip_servers", getenv("new_sip_servers"));

	if (c) {
		dns_to_resolv();
#ifdef HAVE_RADVD
		stop_radvd();
		start_radvd();
#endif
#ifdef HAVE_SMARTDNS
		stop_smartdns();
		start_smartdns();
#endif
		stop_dhcp6s();
		start_dhcp6s();
	}

	return 0;
}
