/*
 * qos.c
 *
 * Copyright (C) 2019 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <bcmnvram.h>
#include <utils.h>
#include <shutils.h>

#ifdef HAVE_SVQOS

#define TYPE_HTB 0x0
#define TYPE_HFSC 0x1

#define IFTYPE_WAN 0x0
#define IFTYPE_IMQ_WAN 0x1
#define IFTYPE_IMQ_LAN 0x2

#define MAXIMUM_PERCENT 80

#define SERVICEBASE_PERCENT 20 // base if maximum is used
#define PREMIUM_PERCENT 60
#define EXPRESS_PERCENT 30
#define DEFAULT_PERCENT 10
#define BULK_BW 128 // fixed minimum bw guaranteed for bulk class

/* NF Mark/Mask
 *
 * since multiple services needs a NF packet mark,
 * we need to use masks to split the 32bit value into several pieces
 *
 *                                             31       23       15       7      0
 * port_forwards         1 bit(s) offset 31  > 10000000 00000000 00000000 00000000
 * hotspot				 8 bit(s) offset 23  > 01111111 10000000 00000000 00000000
 * quality of service   13 bit(s) offset 10  > 00000000 01111111 11111100 00000000
 *
 * the remaining 11 bits are currently not in use
 */

struct NF_MASKS {
	char *service_name; // name of the service
	int bits_used; // bits used by this service
	int bit_offset; // position of the fist bit
};

static struct NF_MASKS service_masks[] = {
	{ "FORWARD", 1, 31 },
	{ "HOTSPOT", 8, 23 },
	{ "QOS", 13, 10 },
};

char *get_NFServiceMark(char *buffer, size_t len, char *service, uint32 mark)
{
	*buffer = 0;

#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	// no mask support possible in kernel 2.4
	snprintf(buffer, len, "0x%x", mark);
	return buffer;
#else
	int x, offset, bitpos;
	uint32 nfmark = 0, nfmask = 0;

	for (x = 0; x < sizeof(service_masks) / sizeof(struct NF_MASKS); x++) {
		if (strcmp(service, service_masks[x].service_name) == 0) {
			if (mark >= (1 << service_masks[x].bits_used))
				return "0xffffffff/0xffffffff";

			offset = service_masks[x].bit_offset;
			bitpos = offset + service_masks[x].bits_used - 1;

			nfmark = (mark << offset);

			for (; bitpos >= offset; bitpos--)
				nfmask |= (1 << bitpos);

			snprintf(buffer, len, "0x%x/0x%x", nfmark, nfmask);
			return buffer;
		}
	}
	return "0xffffffff/0xffffffff";
#endif
}

char *qos_nfmark(char *buffer, size_t len, uint32 x)
{
	return get_NFServiceMark(buffer, len, "QOS", x);
}

static char *get_wshaper_dev(char *buf)
{
	strcpy(buf, "br0");
	if (nvram_match("wshaper_dev", "WAN"))
		return safe_get_wan_face(buf);
	return buf;
}

int get_mtu_val(void)
{
	char buf[32];
	char ifname[16 + 1];
	if (nvram_match("wshaper_dev", "WAN") && !strcmp(get_wshaper_dev(ifname), "ppp0"))
		return nvram_geti("wan_mtu");
	else if (nvram_match("wshaper_dev", "WAN")) {
		if (nvram_matchi("wan_mtu", 1500))
			return atoi(getMTU(get_wshaper_dev(ifname)));
		else
			return nvram_geti("wan_mtu");
	} else
		return atoi(getBridgeMTU(get_wshaper_dev(ifname), buf));
}

struct namemaps {
	char *from;
	char *to;
};
static struct namemaps NM[] = { { "applejuice", "apple" }, { "bearshare", "gnu" }, { "bittorrent", "bit" },
				{ "directconnect", "dc" }, { "edonkey", "edk" },   { "gnutella", "gnu" },
				{ "soulseek", "soul" } };

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
void add_client_dev_srvfilter(char *name, char *type, char *data, int level, int base, char *chain)
{
	int idx = level / 10;
	char buffer[32];
	if (idx == 10)
		idx = 0;

	if (name && (!strcmp(name, "windows-telemetry") || !strcmp(name, "ubnt-telemetry") || !strcmp(name, "ad-telemetry")))
		return;
	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "udp", "-m", "udp", "--dport", data, "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "udp", "-m", "udp", "--sport", data, "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", chain, "-p", "udp", "-m", "udp", "--dport", data, "-j", "MARK",
			"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", chain, "-p", "udp", "-m", "udp", "--sport", data, "-j", "MARK",
			"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "tcp", "--dport", data, "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "tcp", "--sport", data, "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "tcp", "--dport", data, "-j", "MARK",
			"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "tcp", "--sport", data, "-j", "MARK",
			"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark",
			qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
#else
	if (strstr(type, "dpi")) {
		insmod("xt_ndpi");
		eval("iptables", "-t", "mangle", "-A", chain, "-m", "ndpi", "--proto", name, "-j", "MARK", "--set-mark",
		     qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", chain, "-m", "ndpi", "--proto", name, "-j", "MARK", "--set-mark",
			qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
	if (strstr(type, "risk")) {
		insmod("xt_ndpi");
		int risk = get_risk_by_name(name);
		char *dep = get_dep_by_name(name);
		if (dep && risk) {
			char lvl[32];
			sprintf(lvl, "%d", risk);
			eval("iptables", "-t", "mangle", "-A", chain, "-m", "ndpi", "--proto", dep, "--risk", lvl, "-j", "MARK",
			     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			evalip6("ip6tables", "-t", "mangle", "-A", chain, "-m", "ndpi", "--proto", dep, "--risk", lvl, "-j", "MARK",
				"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		}
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = name;
		int i;
		for (i = 0; i < sizeof(NM) / sizeof(struct namemaps); i++) {
			if (!strcasecmp(NM[i].from, name))
				proto = NM[i].to;
		}
		if (proto) {
			insmod("ipt_ipp2p");
			char ipp2p[32];
			snprintf(ipp2p, 32, "--%s", proto);

			eval("iptables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), base + idx));
			evalip6("ip6tables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "ipp2p", ipp2p, "-j", "MARK",
				"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced
#ifdef HAVE_MICRO
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt", "-j", "MARK",
				     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
#else
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "length", "--length", "0:550", "-m", "layer7",
				     "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
#endif
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK",
				     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK",
				     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

				evalip6("ip6tables", "-t", "mangle", "-A", chain, "-m", "length", "--length", "0:550", "-m",
					"layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
					qos_nfmark(buffer, sizeof(buffer), base + idx));
				evalip6("ip6tables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK",
					"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				evalip6("ip6tables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK",
					"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			}
		}
	}
}

void add_client_mac_srvfilter(char *name, char *type, char *data, int level, int base, char *client)
{
	int idx = level / 10;
	char buffer[32];

	if (idx == 10)
		idx = 0;
	if (name && (!strcmp(name, "windows-telemetry") || !strcmp(name, "ubnt-telemetry")))
		return;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-m", "mac",
		     "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-m", "mac",
		     "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-m", "mac",
			"--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-m", "mac",
			"--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-m", "mac",
		     "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-m", "mac",
		     "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-m", "mac",
			"--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-m", "mac",
			"--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}

#ifndef HAVE_OPENDPI
	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "layer7",
		     "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "layer7",
			"--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
#else
	if (strstr(type, "dpi")) {
		insmod("xt_ndpi");
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "ndpi", "--proto",
		     name, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "ndpi",
			"--proto", name, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
	if (strstr(type, "risk")) {
		insmod("xt_ndpi");
		int risk = get_risk_by_name(name);
		char *dep = get_dep_by_name(name);
		if (dep && risk) {
			char lvl[32];
			sprintf(lvl, "%d", risk);
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "ndpi",
			     "--proto", dep, "--risk", lvl, "-j", "MARK", "--set-mark",
			     qos_nfmark(buffer, sizeof(buffer), base + idx));
			evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "ndpi",
				"--proto", dep, "--risk", lvl, "-j", "MARK", "--set-mark",
				qos_nfmark(buffer, sizeof(buffer), base + idx));
		}
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = name;
		int i;
		for (i = 0; i < sizeof(NM) / sizeof(struct namemaps); i++) {
			if (!strcasecmp(NM[i].from, name))
				proto = NM[i].to;
		}
		if (proto) {
			insmod("ipt_ipp2p");
			char ipp2p[32];
			snprintf(ipp2p, 32, "--%s", proto);

			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "mac", "--mac-source", client, "-m",
			     "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "mac", "--mac-source", client,
				"-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced
#ifdef HAVE_MICRO
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
				     "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
#else
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
				     "length", "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
#endif
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
				     "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
				     "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));

				evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
					"length", "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK",
					"--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
					"layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark",
					qos_nfmark(buffer, sizeof(buffer), base + idx));
				evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m",
					"layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark",
					qos_nfmark(buffer, sizeof(buffer), base + idx));
			}
		}
	}
}

void add_client_ip_srvfilter(char *name, char *type, char *data, int level, int base, char *client)
{
	int idx = level / 10;
	char buffer[32];
	if (idx == 10)
		idx = 0;

	if (name && (!strcmp(name, "windows-telemetry") || !strcmp(name, "ubnt-telemetry")))
		return;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--dport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--sport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--dport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--sport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--dport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--sport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--dport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--sport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-s", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-d", client, "-j",
		     "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}

#ifndef HAVE_OPENDPI
	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
#else
	if (strstr(type, "dpi")) {
		insmod("xt_ndpi");

		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "ndpi", "--proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "ndpi", "--proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "ndpi", "--proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "ndpi", "--proto", name, "-j", "MARK",
		     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
	}
	if (strstr(type, "risk")) {
		insmod("xt_ndpi");
		int risk = get_risk_by_name(name);
		char *dep = get_dep_by_name(name);
		if (dep && risk) {
			char lvl[32];
			sprintf(lvl, "%d", risk);

			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "ndpi", "--proto", dep, "--risk",
			     lvl, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "ndpi", "--proto", dep, "--risk",
			     lvl, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "ndpi", "--proto", dep, "--risk",
			     lvl, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "ndpi", "--proto", dep, "--risk",
			     lvl, "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
		}
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = name;
		int i;
		for (i = 0; i < sizeof(NM) / sizeof(struct namemaps); i++) {
			if (!strcasecmp(NM[i].from, name))
				proto = NM[i].to;
		}
		if (proto) {
			insmod("ipt_ipp2p");
			char ipp2p[32];
			snprintf(ipp2p, 32, "--%s", proto);

			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "ipp2p", ipp2p, "-j", "MARK",
			     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "ipp2p", ipp2p, "-j", "MARK",
			     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "ipp2p", ipp2p, "-j", "MARK",
			     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "ipp2p", ipp2p, "-j", "MARK",
			     "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced
#ifdef HAVE_MICRO

				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto",
				     "bt", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto",
				     "bt", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto", "bt",
				     "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto", "bt",
				     "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));

#else
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "--length", "0:550", "-m",
				     "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "--length", "0:550", "-m",
				     "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "--length", "0:550", "-m",
				     "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "--length", "0:550", "-m",
				     "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark",
				     qos_nfmark(buffer, sizeof(buffer), base + idx));
#endif
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto",
				     "bt1", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto",
				     "bt1", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto",
				     "bt1", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto",
				     "bt1", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto",
				     "bt2", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto",
				     "bt2", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto",
				     "bt2", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto",
				     "bt2", "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base + idx));
			}
		}
	}
}

#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN)
char *get_tcfmark(char *tcfmark, uint32 mark, int seg)
{
	char nfmark[24] = { 0 };
	char buffer[32];
	char *ntoken = NULL;
	*tcfmark = 0;

	strcpy(nfmark, qos_nfmark(buffer, sizeof(buffer), mark));
	ntoken = strtok(nfmark, "/");
	strcpy(tcfmark, ntoken);
	if (seg == 1)
		return tcfmark;
	ntoken = strtok(NULL, "/");
	if (seg == 2) {
		strcpy(tcfmark, ntoken);
		return tcfmark;
	}
	strcat(tcfmark, " ");
	strcat(tcfmark, ntoken);
	return tcfmark;
}
#endif
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
static void add_tc_class(char *dev, int pref, int pref6, int handle, int classid)
{
	char h[32];
	char c[32];
	char p[32];
	char p6[32];
	sprintf(h, "0x%02x", handle);
	sprintf(c, "1:", classid);
	sprintf(p, "%d", pref);
	sprintf(p6, "%d", pref6);
	eval("tc", "filter", "add", "dev", dev, "protocol", "ip", "pref", p, "handle", h, "fw", "classid", c);
	evalip6("tc", "filter", "add", "dev", dev, "protocol", "ipv6", "pref", p6, "handle", h, "fw", "classid", c);
}
#else
static void add_tc_mark(char *dev, int pref, int pref6, char *mark, char *mark2, int flow)
{
	char p[32];
	char p6[32];
	sprintf(p, "%d", pref);
	sprintf(p6, "%d", pref6);
	char f[32];
	sprintf(f, "1:%d", flow);
	eval("tc", "filter", "add", "dev", dev, "protocol", "ip", "pref", p, "parent", "1:", "u32", "match", "mark", mark, mark2,
	     "flowid", f);
	evalip6("tc", "filter", "add", "dev", dev, "protocol", "ipv6", "pref", p6, "parent", "1:", "u32", "match", "mark", mark,
		mark2, "flowid", f);
}
#endif

static const char *math(char *buf, size_t len, long long val, const char *ext)
{
	snprintf(buf, len, "%lld%s", val, ext);
	return buf;
}

static void add_codel(const char *dev, int handle, const char *aqd, int rtt, int noecn)
{
	char p[32];
	char h[32];
	char r[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	sprintf(r, "%dms", rtt);
	char *ECN = NULL;
	if (noecn == 1)
		ECN = "noecn";
	if (noecn == 0)
		ECN = "ecn";

	if (rtt != -1)
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, "target", r, ECN);
	else
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, ECN);
}

static void add_sfq(const char *dev, int handle, int mtu)
{
	char qmtu[32];
	sprintf(qmtu, "%d", mtu + 14);
	char p[32];
	char h[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, "sfq", "quantum", qmtu, "perturb", "10");
}

static void add_fq_codel(const char *dev, int handle, const char *aqd)
{
	char p[32];
	char h[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd);
}

static void add_cake(int type, const char *dev, int handle, const char *aqd, int rtt)
{
	char p[32];
	char h[32];
	char r[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	sprintf(r, "%dms", rtt);
	char *rttarg1 = NULL;
	char *rttarg2 = NULL;
	if (rtt != -1) {
		rttarg1 = "rtt";
		rttarg2 = r;
	}

	switch (type) {
	case IFTYPE_WAN:
	case IFTYPE_IMQ_WAN:
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, "unlimited", "ethernet", "besteffort",
		     "noatm", "raw", "internet", type == IFTYPE_WAN ? "dual-srchost" : "dual-dsthost", "ack-filter", "nat", rttarg1,
		     rttarg2);
		break;
	case IFTYPE_IMQ_LAN:
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, "unlimited", "ethernet", "besteffort",
		     "noatm", "raw", "lan", "triple-isolate", "no-ack-filter", "nat", rttarg1, rttarg2);
		break;
	}
}

#define percent(from, val) ((from) * (val) / 100)

static void add_pie(const char *dev, int handle, const char *aqd, int ms5, int noecn)
{
	char p[32];
	char h[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	char *ECN = NULL;
	if (noecn == 1)
		ECN = "noecn";
	if (noecn == 0)
		ECN = "ecn";

	if (ms5)
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, "target", "5ms", ECN);
	else
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, ECN);
}

static void add_htb_class(const char *dev, int parent, int class, int rate, int limit, int mtu, int p)
{
	char buf[32];
	char buf2[32];
	char qmtu[32];
	char prio[32];
	char parentid[32];
	char classid[32];
	sprintf(qmtu, "%d", mtu + 14);
	sprintf(prio, "%d", p);
	sprintf(parentid, "1:%d", parent);
	sprintf(classid, "1:%d", class);
	if (p != -1)
		eval("tc", "class", "add", "dev", dev, "parent", parentid, "classid", classid, "htb", "rate",
		     math(buf, sizeof(buf), rate, "kbit"), "ceil", math(buf2, sizeof(buf2), limit, "kbit"), "prio", prio, "quantum",
		     qmtu);
	else
		eval("tc", "class", "add", "dev", dev, "parent", parentid, "classid", classid, "htb", "rate",
		     math(buf, sizeof(buf), rate, "kbit"), "ceil", math(buf2, sizeof(buf2), limit, "kbit"), "quantum", qmtu);
}

static void add_hfsc_class(const char *dev, int parent, int class, long long rate, long long limit)
{
	char buf[32];
	char buf2[32];
	char classid[32];
	char parentid[32];
	sprintf(classid, "1:%d", class);
	sprintf(parentid, "1:%d", parent);
	if (limit == -1)
		eval("tc", "class", "add", "dev", dev, "parent", parentid, "classid", classid, "hfsc", "ls", "m2",
		     math(buf, sizeof(buf), rate, "bit"));
	else
		eval("tc", "class", "add", "dev", dev, "parent", parentid, "classid", classid, "hfsc", "ls", "m2",
		     math(buf, sizeof(buf), rate, "bit"), "ul", "m2", math(buf2, sizeof(buf2), limit, "bit"));
}

void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned int lanrate, unsigned int level)
{
	char wan_if_buffer[33];
	char *wan_dev = safe_get_wan_face(wan_if_buffer);

	unsigned int uplimit = nvram_geti("wshaper_uplink");
	unsigned int downlimit = nvram_geti("wshaper_downlink");
	unsigned int lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;
	int rtt = -1;
	int noecn = -1;
	int max = 50;
	int rtt_cake = -1;
	if (!nvram_matchi("qos_type", 0)) {
		rtt = 5;
	}
	if (uprate <= 2000) {
		rtt = max - (uprate / 50);
		rtt_cake = rtt;
		noecn = 1;
	}

	unsigned int mtu = get_mtu_val();

	if (lanrate < 1)
		lanrate = lanlimit;

	char *aqd = nvram_safe_get("svqos_aqd");

	switch (level) {
	case 100:
		uprate = percent(uplimit, MAXIMUM_PERCENT);
		downrate = percent(downlimit, MAXIMUM_PERCENT);
		lanrate = percent(lanlimit, MAXIMUM_PERCENT);
		prio = 2;
		parent = 1;
		break;
	case 10:
		uprate = percent(uplimit, PREMIUM_PERCENT);
		downrate = percent(downlimit, PREMIUM_PERCENT);
		lanrate = percent(lanlimit, PREMIUM_PERCENT);
		prio = 3;
		parent = 10;
		break;
	case 20:
		uprate = percent(uplimit, EXPRESS_PERCENT);
		downrate = percent(downlimit, EXPRESS_PERCENT);
		lanrate = percent(lanlimit, EXPRESS_PERCENT);
		prio = 4;
		parent = 20;
		break;
	case 30:
		uprate = percent(uplimit, DEFAULT_PERCENT);
		downrate = percent(downlimit, DEFAULT_PERCENT);
		lanrate = percent(lanlimit, DEFAULT_PERCENT);
		prio = 5;
		parent = 30;
		break;
	case 40:
		uprate = BULK_BW;
		downrate = BULK_BW;
		lanrate = BULK_BW;
		prio = 6;
		parent = 40;
		break;
	default:
		if (uprate)
			uplimit = uprate;
		if (downrate)
			downlimit = downrate;
		if (lanrate)
			lanlimit = lanrate;
		prio = 3;
		parent = 1;
		break;
	}
	unsigned int percentages[5] = { MAXIMUM_PERCENT, PREMIUM_PERCENT, EXPRESS_PERCENT, DEFAULT_PERCENT, 1 };
	if (nvram_matchi("qos_type", 0)) {
		char prios[5] = { 0, prio, prio + 1, prio + 1, 7 };

		add_htb_class(wan_dev, parent, base, uprate, uplimit, mtu, -1);
		add_htb_class("imq0", parent, base, downrate, downlimit, mtu, -1);
		if (nvram_match("wshaper_dev", "LAN")) {
			add_htb_class("imq1", parent, base, lanrate, lanlimit, mtu, -1);
		}
		int i;
		for (i = 0; i < 5; i++) {
			int up = BULK_BW;
			int down = BULK_BW;
			int lan = BULK_BW;
			if (i < 4) {
				up = percent(uprate, percentages[i]);
				down = percent(downrate, percentages[i]);
				lan = percent(lanrate, percentages[i]);
			}
			add_htb_class(wan_dev, base, base + 1 + i, up, uplimit, mtu, prios[i] + 1);
			add_htb_class("imq0", base, base + 1 + i, down, downlimit, mtu, prios[i] + 1);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_htb_class("imq1", base, base + 1 + i, lan, lanlimit, mtu, prios[i] + 1);
			}
		}

	} else {
		uprate *= 1000;
		downrate *= 1000;
		lanrate *= 1000;
		long long up = -1;
		long long down = -1;
		long long lan = -1;
		if (!level) {
			up = uprate;
			down = downrate;
			lan = lanrate;
		}
		add_hfsc_class(wan_dev, parent, base, BULK_BW * 1000, up);
		add_hfsc_class("imq0", parent, base, BULK_BW * 1000, down);
		if (nvram_match("wshaper_dev", "LAN")) {
			add_hfsc_class("imq1", parent, base, BULK_BW * 1000, lan);
		}
		int i;
		for (i = 0; i < 5; i++) {
			if (!i) {
				up = percent(BULK_BW * 1000, percentages[i]);
				down = percent(BULK_BW * 1000, percentages[i]);
				lan = percent(BULK_BW * 1000, percentages[i]);
			} else {
				up = percent(percent(BULK_BW * 1000, percentages[i]), SERVICEBASE_PERCENT);
				down = percent(percent(BULK_BW * 1000, percentages[i]), SERVICEBASE_PERCENT);
				lan = percent(percent(BULK_BW * 1000, percentages[i]), SERVICEBASE_PERCENT);
			}
			add_hfsc_class(wan_dev, base, base + 1 + i, up, -1);
			add_hfsc_class("imq0", base, base + 1 + i, down, -1);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_hfsc_class("imq1", base, base + 1 + i, lan, -1);
			}
		}
	}
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	// filter rules
	int i;
	char priorities[5] = { 1, 3, 5, 8, 10 };
	char priorities6[5] = { 2, 4, 6, 9, 11 };
	for (i = 0; i < 5; i++) {
		add_tc_class(wan_dev, priorities[i] + 1, priorities6[i] + 1, base + i, base + 1 + i);
		add_tc_class("imq0", priorities[i] + 1, priorities6[i] + 1, base + i, base + 1 + i);

		if (nvram_match("wshaper_dev", "LAN")) {
			add_tc_class("imq1", priorities[i] + 1, priorities6[i] + 1, base + i, base + 1 + i);
		}
	}
#else
	int i;
	char priorities[5] = { 1, 3, 5, 8, 10 };
	char priorities6[5] = { 2, 4, 6, 9, 11 };
	for (i = 0; i < 5; i++) {
		char tcfmark[32] = { 0 };
		char tcfmark2[32] = { 0 };
		add_tc_mark(wan_dev, priorities[i] + 1, priorities6[i] + 1, get_tcfmark(tcfmark, base + i, 1),
			    get_tcfmark(tcfmark2, base + i, 2), base + 1 + i);
		add_tc_mark("imq0", priorities[i] + 1, priorities6[i] + 1, get_tcfmark(tcfmark, base + i, 1),
			    get_tcfmark(tcfmark2, base + i, 2), base + 1 + i);
		if (nvram_match("wshaper_dev", "LAN")) {
			add_tc_mark("imq1", priorities[i] + 1, priorities6[i] + 1, get_tcfmark(tcfmark, base + i, 1),
				    get_tcfmark(tcfmark2, base + i, 2), base + 1 + i);
		}
	}
#endif

	// leaf qdiscs
	if (!strcmp(aqd, "sfq")) {
		int i;
		for (i = 1; i < 6; i++) {
			add_sfq(wan_dev, base + i, mtu);
			add_sfq("imq0", base + i, mtu);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_sfq("imq1", base + i, mtu);
			}
		}
	}
#ifdef HAVE_CODEL
	if (!strcmp(aqd, "codel")) {
		int i;
		for (i = 1; i < 6; i++) {
			add_codel(wan_dev, base + i, aqd, rtt, noecn);
			add_codel("imq0", base + i, aqd, -1, -1);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_codel("imq1", base + i, aqd, -1, -1);
			}
		}
	}
#endif
#if defined(HAVE_FQ_CODEL) || defined(HAVE_FQ_CODEL_FAST)
	if (!strcmp(aqd, "fq_codel") || !strcmp(aqd, "fq_codel_fast")) {
		int i;
		for (i = 1; i < 6; i++) {
			add_fq_codel(wan_dev, base + i, aqd);
			add_fq_codel("imq0", base + i, aqd);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_fq_codel("imq1", base + i, aqd);
			}
		}
	}
#endif
#ifdef HAVE_CAKE
	if (!strcmp(aqd, "cake")) {
		int i;
		for (i = 1; i < 6; i++) {
			add_cake(IFTYPE_WAN, wan_dev, base + i, aqd, rtt_cake);
			add_cake(IFTYPE_IMQ_WAN, "imq0", base + i, aqd, -1);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_cake(IFTYPE_IMQ_LAN, "imq1", base + i, aqd, -1);
			}
		}
	}
#endif
#ifdef HAVE_PIE
	if (!strcmp(aqd, "pie")) {
		if (!nvram_matchi("qos_type", 0))
			noecn = 0;
		int i;
		for (i = 1; i < 6; i++) {
			add_pie(wan_dev, base + i, aqd, 0, noecn);
			if (nvram_matchi("qos_type", 0))
				add_pie("imq0", base + i, aqd, 1, 0);
			else
				add_pie("imq0", base + i, aqd, 0, noecn);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_pie("imq1", base + i, aqd, 0, noecn);
			}
		}
	}
#endif
}

void add_usermac(char *mac, int base, int uprate, int downrate, int lanrate)
{
	char srvname[32], srvtype[32], srvdata[32];
	int srvlevel;
	char buffer[32];
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(buffer, sizeof(buffer), 0));

	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "RETURN");

	evalip6("ip6tables", "-t", "mangle", "-D", "FILTER_IN", "-j", "CONNMARK", "--save");
	evalip6("ip6tables", "-t", "mangle", "-D", "FILTER_IN", "-j", "RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %d ", srvname, srvtype, srvdata, &srvlevel) < 4)
			break;

		add_client_mac_srvfilter(srvname, srvtype, srvdata, srvlevel, base, mac);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", mac, "-m", "mark", "--mark", nullmask,
	     "-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base));

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");

	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", mac, "-m", "mark", "--mark", nullmask,
		"-j", "MARK", "--set-mark", qos_nfmark(buffer, sizeof(buffer), base));

	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK", "--save");
	evalip6("ip6tables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");
}

void add_userip(char *ip, int base, int uprate, int downrate, int lanrate)
{
	//      int ret;
	char srvname[32], srvtype[32], srvdata[32];
	int srvlevel;
	char buffer[32];
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(buffer, sizeof(buffer), 0));

	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "RETURN");

	//      eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "VPN_DSCP");
	eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %d ", srvname, srvtype, srvdata, &srvlevel) < 4)
			break;

		add_client_ip_srvfilter(srvname, srvtype, srvdata, srvlevel, base, ip);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark",
	     qos_nfmark(buffer, sizeof(buffer), base));
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark",
	     qos_nfmark(buffer, sizeof(buffer), base));
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark",
	     qos_nfmark(buffer, sizeof(buffer), base));
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark",
	     qos_nfmark(buffer, sizeof(buffer), base));

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");

	//      eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "VPN_DSCP");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "RETURN");
}

void deinit_qos(const char *wandev, const char *imq_wan, const char *imq_lan)
{
	eval("tc", "filter", "del", "dev", wandev);
	eval("tc", "qdisc", "del", "dev", wandev, "root");
	eval("tc", "class", "del", "dev", wandev);
	if (imq_wan) {
		eval("ip", "link", "set", imq_wan, "down");
		eval("tc", "filter", "del", "dev", imq_wan);
		eval("tc", "qdisc", "del", "dev", imq_wan, "root");
		eval("tc", "class", "del", "dev", imq_wan);
	}

	if (imq_lan) {
		eval("ip", "link", "set", imq_lan, "down");
		eval("tc", "filter", "del", "dev", imq_lan);
		eval("tc", "qdisc", "del", "dev", imq_lan, "root");
		eval("tc", "class", "del", "dev", imq_lan);
	}
}

static void init_htb_class(const char *dev, int rate, int mtu)
{
	add_htb_class(dev, 0, 1, rate, rate, mtu, -1);
	add_htb_class(dev, 1, 100, percent(rate, MAXIMUM_PERCENT), rate, mtu, 1);
	add_htb_class(dev, 1, 2, percent(rate, SERVICEBASE_PERCENT), rate, mtu, 2);
	add_htb_class(dev, 2, 10, percent(rate, PREMIUM_PERCENT), rate, mtu, 3);
	add_htb_class(dev, 2, 20, percent(rate, EXPRESS_PERCENT), rate, mtu, 4);
	add_htb_class(dev, 2, 30, percent(rate, DEFAULT_PERCENT), rate, mtu, 5);
	add_htb_class(dev, 2, 40, BULK_BW, rate, mtu, 6);
}

static void init_hfsc_class(const char *dev, long long rate)
{
	rate *= 1000;
	add_hfsc_class(dev, 0, 1, rate, rate);
	add_hfsc_class(dev, 1, 100, percent(rate, MAXIMUM_PERCENT), -1);
	add_hfsc_class(dev, 1, 10, percent(percent(rate, PREMIUM_PERCENT), SERVICEBASE_PERCENT), -1);
	add_hfsc_class(dev, 1, 20, percent(percent(rate, EXPRESS_PERCENT), SERVICEBASE_PERCENT), -1);
	add_hfsc_class(dev, 1, 30, percent(percent(rate, DEFAULT_PERCENT), SERVICEBASE_PERCENT), -1);
	add_hfsc_class(dev, 1, 40, BULK_BW * 1000, -1);
}

static void init_qdisc(int type, int wan_type, const char *dev, const char *wandev, const char *aqd, int mtu, int up, int ms5)
{
	int noecn = -1;
	int rtt = -1;
	int rtt_cake = -1;
	if (type == TYPE_HFSC) {
		rtt = 5;
	}

	if (strcmp(wandev, "xx") && up < 2000) {
		rtt = 20;
		rtt_cake = 20;
		noecn = 1;
	}

	if (!strcmp(aqd, "sfq")) {
		add_sfq(dev, 100, mtu);
		add_sfq(dev, 10, mtu);
		add_sfq(dev, 20, mtu);
		add_sfq(dev, 30, mtu);
		add_sfq(dev, 40, mtu);
	}
#ifdef HAVE_CODEL
	if (!strcmp(aqd, "codel")) {
		add_codel(dev, 100, aqd, rtt, noecn);
		add_codel(dev, 10, aqd, rtt, noecn);
		add_codel(dev, 20, aqd, rtt, noecn);
		add_codel(dev, 30, aqd, rtt, noecn);
		add_codel(dev, 40, aqd, rtt, noecn);
	}
#endif
#if defined(HAVE_FQ_CODEL) || defined(HAVE_FQ_CODEL_FAST)
	if (!strcmp(aqd, "fq_codel") || !strcmp(aqd, "fq_codel_fast")) {
		add_fq_codel(dev, 100, aqd);
		add_fq_codel(dev, 10, aqd);
		add_fq_codel(dev, 20, aqd);
		add_fq_codel(dev, 30, aqd);
		add_fq_codel(dev, 40, aqd);
	}
#endif
#ifdef HAVE_CAKE
	if (!strcmp(aqd, "cake")) {
		add_cake(wan_type, dev, 100, aqd, rtt_cake);
		add_cake(wan_type, dev, 10, aqd, rtt_cake);
		add_cake(wan_type, dev, 20, aqd, rtt_cake);
		add_cake(wan_type, dev, 30, aqd, rtt_cake);
		add_cake(wan_type, dev, 40, aqd, rtt_cake);
	}
#endif
#ifdef HAVE_PIE
	if (!strcmp(aqd, "pie")) {
		/* for imq_wan and htb only 5ms is enforced. i dont know why. i just took it from the original script */
		if (ms5 || type == TYPE_HFSC)
			noecn = 0;
		add_pie(dev, 100, aqd, ms5, noecn);
		add_pie(dev, 10, aqd, ms5, noecn);
		add_pie(dev, 20, aqd, ms5, noecn);
		add_pie(dev, 30, aqd, ms5, noecn);
		add_pie(dev, 40, aqd, ms5, noecn);
	}
#endif
}

void init_ackprio(const char *dev)
{
#if 0
	char *qos_pkts = nvram_safe_get("svqos_pkts");
	char pkt_filter[5];
	do {
		if (sscanf(qos_pkts, "%4s ", pkt_filter) < 1)
			break;
		if (!strcmp(pkt_filter, "ACK")) {
			eval("tc", "filter", "add", "dev", dev, "parent", "1:", "prio", "4", "protocol", "ip", "u32",	//
			     "match", "ip", "protocol", "6", "0xff",	//
			     "match", "u8", "0x05", "0x0f", "at", "0",	//
			     "match", "u16", "0x0000", "0xffc0", "at", "2",	//
			     "match", "u8", "0x10", "0xff", "at", "33",	//
			     "flowid", "1:100");
		}
		if (!strcmp(pkt_filter, "SYN")) {
			eval("tc", "filter", "add", "dev", dev, "parent", "1:", "prio", "5", "protocol", "ip", "u32",	//
			     "match", "ip", "protocol", "6", "0xff",	//
			     "match", "u8", "0x05", "0x0f", "at", "0",	//
			     "match", "u16", "0x0000", "0xffc0", "at", "2",	//
			     "match", "u8", "0x02", "0x02", "at", "33",	//
			     "flowid", "1:100");
		}
		if (!strcmp(pkt_filter, "FIN")) {
			eval("tc", "filter", "add", "dev", dev, "parent", "1:", "prio", "6", "protocol", "ip", "u32",	//
			     "match", "ip", "protocol", "6", "0xff",	//
			     "match", "u8", "0x05", "0x0f", "at", "0",	//
			     "match", "u16", "0x0000", "0xffc0", "at", "2",	//
			     "match", "u8", "0x01", "0x01", "at", "33",	//
			     "flowid", "1:100");
		}
		if (!strcmp(pkt_filter, "RST")) {
			eval("tc", "filter", "add", "dev", dev, "parent", "1:", "prio", "7", "protocol", "ip", "u32",	//
			     "match", "ip", "protocol", "6", "0xff",	//
			     "match", "u8", "0x05", "0x0f", "at", "0",	//
			     "match", "u16", "0x0000", "0xffc0", "at", "2",	//
			     "match", "u8", "0x04", "0x04", "at", "33",	//
			     "flowid", "1:100");
		}
		if (!strcmp(pkt_filter, "ICMP")) {
			eval("tc", "filter", "add", "dev", dev, "parent", "1:", "prio", "8", "protocol", "ip", "u32",	//
			     "match", "ip", "protocol", "1", "0xff",	//
			     "flowid", "1:100");
		}
	} while ((qos_pkts = strpbrk(++qos_pkts, "|")) && qos_pkts++);
#endif
}

#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
static void add_filter(const char *dev, int pref, int pref6, int handle, int classid)
{
	char p[32];
	char p6[32];
	char h[32];
	char c[32];
	sprintf(p, "%d", pref);
	sprintf(p6, "%d", pref6);
	sprintf(h, "0x%02X", handle);
	sprintf(c, "1:%d", classid);
	eval("tc", "filter", "add", "dev", dev, "protocol", "ip", "pref", p, "handle", h, "fw", "classid", c);
	evalip6("tc", "filter", "add", "dev", dev, "protocol", "ipv6", "pref", p6, "handle", h, "fw", "classid", c);
}

static void init_filter(const char *dev)
{
	//      add_filter(dev, 0 + 1, 1000, 1000);
	add_filter(dev, 1 + 1, 2 + 1, 100, 100);
	add_filter(dev, 3 + 1, 4 + 1, 10, 10);
	add_filter(dev, 5 + 1, 6 + 1, 20, 20);
	add_filter(dev, 8 + 1, 9 + 1, 30, 30);
	add_filter(dev, 10 + 1, 11 + 1, 40, 40);
}
#else
static inline void init_filter(const char *dev)
{
}

#endif

void init_qos(const char *strtype, int up, int down, const char *wandev, int mtu, const char *imq_wan, const char *aqd,
	      const char *imq_lan)
{
	int type;
	deinit_qos(wandev, imq_wan, imq_lan);

	if (!strcmp(strtype, "htb"))
		type = TYPE_HTB;
	else
		type = TYPE_HFSC;

	int lanlimit = 1000000;

	if (strcmp(wandev, "xx")) {
		if (type == TYPE_HTB) {
			eval("tc", "qdisc", "add", "dev", wandev, "root", "handle", "1:", "htb", "default", "30");
			init_htb_class(wandev, up, mtu);
			init_qdisc(type, IFTYPE_WAN, wandev, wandev, aqd, mtu, up, 0);
		} else {
			eval("tc", "qdisc", "add", "dev", wandev, "root", "handle", "1:", "hfsc", "default", "30");
			init_hfsc_class(wandev, up);
			init_qdisc(type, IFTYPE_WAN, wandev, wandev, aqd, mtu, up, 0);
		}
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
		init_ackprio(wandev);
#endif
		init_filter(wandev);
	}

	if (down != 0) {
		eval("ip", "link", "set", imq_wan, "up");

		if (type == TYPE_HTB) {
			eval("tc", "qdisc", "add", "dev", imq_wan, "root", "handle", "1:", "htb", "default", "30");
			init_htb_class(imq_wan, down, mtu);
			init_qdisc(type, IFTYPE_IMQ_WAN, imq_wan, wandev, aqd, mtu, up, 1); // force 5ms for PIE on imq_wan

		} else {
			eval("tc", "qdisc", "add", "dev", imq_wan, "root", "handle", "1:", "hfsc", "default", "30");
			init_hfsc_class(imq_wan, down);
			init_qdisc(type, IFTYPE_IMQ_WAN, imq_wan, wandev, aqd, mtu, up, 0);
		}
		init_filter(imq_wan);
	}
	if (imq_lan) {
		eval("ip", "link", "set", imq_lan, "up");

		if (type == TYPE_HTB) {
			eval("tc", "qdisc", "add", "dev", imq_lan, "root", "handle", "1:", "htb", "default", "30");
			init_htb_class(imq_lan, lanlimit, mtu);
		} else {
			eval("tc", "qdisc", "add", "dev", imq_lan, "root", "handle", "1:", "hfsc", "default", "30");
			init_hfsc_class(imq_lan, lanlimit);
		}
		init_qdisc(type, IFTYPE_IMQ_LAN, imq_lan, wandev, aqd, mtu, up, 0);
		init_filter(imq_lan);
	}
}

#endif //","HAVE_SVQOS
