/*

 * qos.c
 *
 * Copyright (C) 2017 Sebastian Gottschall <gottschall@dd-wrt.com>
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
	char *service_name;	// name of the service
	int bits_used;		// bits used by this service
	int bit_offset;		// position of the fist bit 
};

static struct NF_MASKS service_masks[] = {
	{"FORWARD", 1, 31},
	{"HOTSPOT", 8, 23},
	{"QOS", 13, 10},
};

char *get_NFServiceMark(char *service, uint32 mark)
{
	static char buffer[32];
	bzero(&buffer, sizeof(buffer));

#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
// no mask support possible in kernel 2.4
	sprintf(buffer, "0x%x", mark);
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

			sprintf(buffer, "0x%x/0x%x", nfmark, nfmask);
			return buffer;
		}
	}
	return "0xffffffff/0xffffffff";
#endif
}

char *qos_nfmark(uint32 x)
{
	return get_NFServiceMark("QOS", x);
}

char
*get_wshaper_dev(void)
{
	if (nvram_match("wshaper_dev", "WAN"))
		return get_wan_face();
	else
		return "br0";
}

static char *my_getBridgeMTU(const char *ifname, char *word)
{
	char *next, *wordlist;

	wordlist = nvram_safe_get("bridges");
	foreach(word, wordlist, next) {
		char *stp = word;
		char *bridge = strsep(&stp, ">");
		char *mtu = stp;
		char *prio = strsep(&mtu, ">");

		if (prio)
			strsep(&mtu, ">");

		if (!bridge || !stp)
			break;
		if (!strcmp(bridge, ifname)) {
			if (!prio || !mtu)
				return "1500";
			else
				return mtu;
		}
	}
	return "1500";
}

static char *my_getMTU(char *ifname)
{
	if (!ifname)
		return "1500";
	char *mtu = nvram_nget("%s_mtu", ifname);
	if (!mtu || strlen(mtu) == 0)
		return "1500";
	return mtu;
}

char
*get_mtu_val(char *buf)
{
	if (nvram_match("wshaper_dev", "WAN")
	    && !strcmp(get_wshaper_dev(), "ppp0"))
		return nvram_safe_get("wan_mtu");
	else if (nvram_match("wshaper_dev", "WAN")) {
		if (nvram_matchi("wan_mtu", 1500))
			return my_getMTU(get_wshaper_dev());
		else
			return nvram_safe_get("wan_mtu");
	} else
		return my_getBridgeMTU(get_wshaper_dev(), buf);
}

void add_client_dev_srvfilter(char *name, char *type, char *data, char *level, int base, char *chain)
{
	int idx = atoi(level) / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "udp", "-m", "udp", "--dport", data, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "udp", "-m", "udp", "--sport", data, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "tcp", "--dport", data, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "tcp", "--sport", data, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		char ndpi[32];
		snprintf(ndpi, 32, "--%s", name);
		insmod("xt_ndpi");
		eval("iptables", "-t", "mangle", "-A", chain, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = NULL;
		char *realname = name;

		if (!strcasecmp(realname, "applejuice"))
			proto = "apple";
		else if (!strcasecmp(realname, "ares"))
			proto = "ares";
		else if (!strcasecmp(realname, "bearshare"))
			proto = "gnu";
		else if (!strcasecmp(realname, "bittorrent"))
			proto = "bit";
		else if (!strcasecmp(realname, "directconnect"))
			proto = "dc";
		else if (!strcasecmp(realname, "edonkey"))
			proto = "edk";
		else if (!strcasecmp(realname, "gnutella"))
			proto = "gnu";
		else if (!strcasecmp(realname, "kazaa"))
			proto = "kazaa";
		else if (!strcasecmp(realname, "mute"))
			proto = "mute";
		else if (!strcasecmp(realname, "soulseek"))
			proto = "soul";
		else if (!strcasecmp(realname, "waste"))
			proto = "waste";
		else if (!strcasecmp(realname, "winmx"))
			proto = "winmx";
		else if (!strcasecmp(realname, "xdcc"))
			proto = "xdcc";
		if (proto) {
			insmod("ipt_ipp2p");
			char ipp2p[32];
			snprintf(ipp2p, 32, "--%s", proto);

			eval("iptables", "-t", "mangle", "-A", chain, "-p", "tcp", "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
#else
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "length", "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
#endif
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", chain, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			}
		}
	}
}

void add_client_mac_srvfilter(char *name, char *type, char *data, char *level, int base, char *client)
{
	int idx = atoi(level) / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "udp", "-m", "udp", "--dport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "udp", "-m", "udp", "--sport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "tcp", "--dport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "tcp", "--sport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		char ndpi[32];
		snprintf(ndpi, 32, "--%s", name);
		insmod("xt_ndpi");
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-m", "mac", "--mac-source", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = NULL;
		char *realname = name;

		if (!strcasecmp(realname, "applejuice"))
			proto = "apple";
		else if (!strcasecmp(realname, "ares"))
			proto = "ares";
		else if (!strcasecmp(realname, "bearshare"))
			proto = "gnu";
		else if (!strcasecmp(realname, "bittorrent"))
			proto = "bit";
		else if (!strcasecmp(realname, "directconnect"))
			proto = "dc";
		else if (!strcasecmp(realname, "edonkey"))
			proto = "edk";
		else if (!strcasecmp(realname, "gnutella"))
			proto = "gnu";
		else if (!strcasecmp(realname, "kazaa"))
			proto = "kazaa";
		else if (!strcasecmp(realname, "mute"))
			proto = "mute";
		else if (!strcasecmp(realname, "soulseek"))
			proto = "soul";
		else if (!strcasecmp(realname, "waste"))
			proto = "waste";
		else if (!strcasecmp(realname, "winmx"))
			proto = "winmx";
		else if (!strcasecmp(realname, "xdcc"))
			proto = "xdcc";
		if (proto) {
			insmod("ipt_ipp2p");
			char ipp2p[32];
			snprintf(ipp2p, 32, "--%s", proto);

			eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "mac", "--mac-source", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
#else
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-m", "mac", "--mac-source", client, "-m", "length", "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK",
				     "--set-mark", qos_nfmark(base + idx));
#endif
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			}
		}
	}
}

void add_client_ip_srvfilter(char *name, char *type, char *data, char *level, int base, char *client)
{
	int idx = atoi(level) / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "udp", "-m", "udp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "udp", "-m", "udp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "udp", "-m", "udp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "udp", "-m", "udp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "udp", "-m", "udp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "udp", "-m", "udp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "udp", "-m", "udp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "udp", "-m", "udp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {

		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "tcp", "-m", "tcp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "tcp", "-m", "tcp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "tcp", "-m", "tcp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-p", "tcp", "-m", "tcp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "tcp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "tcp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "tcp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-p", "tcp", "-m", "tcp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		char ndpi[32];
		snprintf(ndpi, 32, "--%s", name);
		insmod("xt_ndpi");

		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

	}
#endif

	if (strstr(type, "p2p")) {
		char *proto = NULL;
		char *realname = name;

		if (!strcasecmp(realname, "applejuice"))
			proto = "apple";
		else if (!strcasecmp(realname, "ares"))
			proto = "ares";
		else if (!strcasecmp(realname, "bearshare"))
			proto = "gnu";
		else if (!strcasecmp(realname, "bittorrent"))
			proto = "bit";
		else if (!strcasecmp(realname, "directconnect"))
			proto = "dc";
		else if (!strcasecmp(realname, "edonkey"))
			proto = "edk";
		else if (!strcasecmp(realname, "gnutella"))
			proto = "gnu";
		else if (!strcasecmp(realname, "kazaa"))
			proto = "kazaa";
		else if (!strcasecmp(realname, "mute"))
			proto = "mute";
		else if (!strcasecmp(realname, "soulseek"))
			proto = "soul";
		else if (!strcasecmp(realname, "waste"))
			proto = "waste";
		else if (!strcasecmp(realname, "winmx"))
			proto = "winmx";
		else if (!strcasecmp(realname, "xdcc"))
			proto = "xdcc";
		if (proto) {
			insmod("ipt_ipp2p");
			char ipp2p[32];
			snprintf(ipp2p, 32, "--%s", proto);

			eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO

				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

#else
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
#endif
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-s", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_OUT", "3", "-d", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-s", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-I", "FILTER_IN", "3", "-d", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			}
		}
	}
}

#if !defined(ARCH_broadcom) || defined(HAVE_BCMMODERN)
char *get_tcfmark(uint32 mark)
{
	static char tcfmark[24];
	char nfmark[24];
	char *ntoken = NULL;

	bzero(&tcfmark, sizeof(tcfmark));
	bzero(&nfmark, sizeof(nfmark));

	strcpy(nfmark, qos_nfmark(mark));

	ntoken = strtok(nfmark, "/");
	strcat(tcfmark, ntoken);

	ntoken = strtok(NULL, "/");
	strcat(tcfmark, " ");
	strcat(tcfmark, ntoken);

	return tcfmark;
}
#endif
static void add_tc_class(char *dev, int pref, int handle, int classid)
{
	sysprintf("tc filter add dev %s protocol ip pref %d handle 0x%x fw classid 1:%d", dev, pref, handle, classid);
}

static void add_tc_mark(char *dev, char *mark, int flow)
{
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", dev, mark, flow);
}

#ifdef HAVE_AQOS
void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned int lanrate, unsigned int level)
{
	char *wan_dev = get_wan_face();

	unsigned int uplimit = atoi(nvram_get("wshaper_uplink"));
	unsigned int downlimit = atoi(nvram_get("wshaper_downlink"));
	unsigned int lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;
	char buf[256];
	char target[64] = "";

	int max = 50;
	int sec = 0;
	if (uprate <= 2000) {
		sec = max - (uprate / 50);
		sprintf(target, "target %dms noecn", sec);
	}

	unsigned int quantum = atoi(get_mtu_val(buf)) + 14;

	if (lanrate < 1)
		lanrate = lanlimit;

#else
void add_client_classes(unsigned int base, unsigned int level)
{
	char *wan_dev = get_wan_face();

	unsigned int uplimit = atoi(nvram_get("wshaper_uplink"));
	unsigned int downlimit = atoi(nvram_get("wshaper_downlink"));
	unsigned int lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;
	char buf[256];

	unsigned int quantum = atoi(get_mtu_val(buf)) + 14;

	unsigned int uprate = 0, downrate = 0;
	int lanrate = lanlimit;
#endif

	char *aqd = nvram_safe_get("svqos_aqd");

	switch (level) {
	case 100:
		uprate = uplimit * 75 / 100;
		downrate = downlimit * 75 / 100;
		lanrate = lanlimit * 75 / 100;
		prio = 2;
		parent = 2;
		break;
	case 10:
		uprate = uplimit * 50 / 100;
		downrate = downlimit * 50 / 100;
		lanrate = lanlimit * 50 / 100;
		prio = 3;
		parent = 3;
		break;
	case 20:
		uprate = uplimit * 25 / 100;
		downrate = downlimit * 25 / 100;
		lanrate = lanlimit * 25 / 100;
		prio = 4;
		parent = 4;
		break;
	case 30:
		uprate = uplimit * 15 / 100;
		downrate = downlimit * 15 / 100;
		lanrate = lanlimit * 15 / 100;
		prio = 5;
		parent = 5;
		break;
	case 40:
		uprate = uprate * 5 / 100;
		downrate = downlimit * 5 / 100;
		lanrate = lanlimit * 5 / 100;
		prio = 6;
		parent = 6;
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

	if (nvram_matchi("qos_type", 0)) {
		// HTB
		// internal
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", wan_dev, parent, base, uprate, uplimit, quantum);
		// maximum
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", wan_dev, base, base + 1, uprate * 75 / 100, uplimit, quantum);
		// premium
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", wan_dev, base, base + 2, uprate * 50 / 100, uplimit, quantum, prio);
		// express
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", wan_dev, base, base + 3, uprate * 25 / 100, uplimit, quantum, prio + 1);
		// standard
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", wan_dev, base, base + 4, uprate * 15 / 100, uplimit, quantum, prio + 1);
		// bulk
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 7", wan_dev, base, base + 5, uprate * 5 / 100, uplimit, quantum);

		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", "imq0", parent, base, downrate, downlimit, quantum);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", "imq0", base, base + 1, downrate * 75 / 100, downlimit, quantum);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq0", base, base + 2, downrate * 50 / 100, downlimit, quantum, prio);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq0", base, base + 3, downrate * 25 / 100, downlimit, quantum, prio + 1);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq0", base, base + 4, downrate * 15 / 100, downlimit, quantum, prio + 1);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 7", "imq0", base, base + 5, downrate * 5 / 100, downlimit, quantum);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", "imq1", parent, base, lanrate, lanlimit, quantum);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", "imq1", base, base + 1, lanrate * 75 / 100, lanlimit, quantum);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq1", base, base + 2, lanrate * 50 / 100, lanlimit, quantum, prio);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq1", base, base + 3, lanrate * 25 / 100, lanlimit, quantum, prio + 1);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio %d", "imq1", base, base + 4, lanrate * 15 / 100, lanlimit, quantum, prio + 1);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 7", "imq1", base, base + 5, lanrate * 5 / 100, lanlimit, quantum);
		}

	} else {
		// HFSC
		// internal
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, 1, base, uprate, uplimit);
		// maximum
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 1, uprate * 75 / 100, uplimit);
		// premium
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 2, uprate * 50 / 100, uplimit);
		// express
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 3, uprate * 25 / 100, uplimit);
		// standard
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 4, uprate * 15 / 100, uplimit);
		// bulk
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", wan_dev, base, base + 5, uprate * 5 / 100, uplimit);

		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", 1, base, uprate, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 1, uprate * 75 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 2, uprate * 50 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 3, uprate * 25 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 4, uprate * 15 / 100, downlimit);
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq0", base, base + 5, uprate * 5 / 100, downlimit);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", 1, base, uprate, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 1, uprate * 75 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 2, uprate * 50 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 3, uprate * 25 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 4, uprate * 15 / 100, lanlimit);
			sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", "imq1", base, base + 5, uprate * 5 / 100, lanlimit);
		}

	}
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	// filter rules
	add_tc_class(wan_dev, 1, base, base + 1);
	add_tc_class(wan_dev, 3, base + 1, base + 2);
	add_tc_class(wan_dev, 5, base + 2, base + 3);
	add_tc_class(wan_dev, 8, base + 3, base + 4);
	add_tc_class(wan_dev, 9, base + 4, base + 5);

	add_tc_class("imq1", 1, base, base + 1);
	add_tc_class("imq1", 3, base + 1, base + 2);
	add_tc_class("imq1", 5, base + 2, base + 3);
	add_tc_class("imq1", 8, base + 3, base + 4);
	add_tc_class("imq1", 9, base + 4, base + 5);

	if (nvram_match("wshaper_dev", "LAN")) {

		add_tc_class("imq0", 1, base, base + 1);
		add_tc_class("imq0", 3, base + 1, base + 2);
		add_tc_class("imq0", 5, base + 2, base + 3);
		add_tc_class("imq0", 8, base + 3, base + 4);
		add_tc_class("imq0", 9, base + 4, base + 5);

	}
#else
	add_tc_mark(wan_dev, get_tcfmark(base), base + 1);
	add_tc_mark(wan_dev, get_tcfmark(base + 1), base + 2);
	add_tc_mark(wan_dev, get_tcfmark(base + 2), base + 3);
	add_tc_mark(wan_dev, get_tcfmark(base + 3), base + 4);
	add_tc_mark(wan_dev, get_tcfmark(base + 4), base + 5);

	add_tc_mark("imq0", get_tcfmark(base), base + 1);
	add_tc_mark("imq0", get_tcfmark(base + 1), base + 2);
	add_tc_mark("imq0", get_tcfmark(base + 2), base + 3);
	add_tc_mark("imq0", get_tcfmark(base + 3), base + 4);
	add_tc_mark("imq0", get_tcfmark(base + 4), base + 5);

	if (nvram_match("wshaper_dev", "LAN")) {
		add_tc_mark("imq1", get_tcfmark(base), base + 1);
		add_tc_mark("imq1", get_tcfmark(base + 1), base + 2);
		add_tc_mark("imq1", get_tcfmark(base + 2), base + 3);
		add_tc_mark("imq1", get_tcfmark(base + 3), base + 4);
		add_tc_mark("imq1", get_tcfmark(base + 4), base + 5);
	}
#endif

	// leaf qdiscs
	if (!strcmp(aqd, "sfq")) {
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 1, base + 1, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 2, base + 2, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 3, base + 3, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 4, base + 4, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", wan_dev, base + 5, base + 5, quantum);

		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 1, base + 1, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 2, base + 2, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 3, base + 3, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 4, base + 4, quantum);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq0", base + 5, base + 5, quantum);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 1, base + 1, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 2, base + 2, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 3, base + 3, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 4, base + 4, quantum);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", "imq1", base + 5, base + 5, quantum);
		}
	}
#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)
	if (!strcmp(aqd, "codel")
	    || !strcmp(aqd, "fq_codel")
	    || !strcmp(aqd, "pie")) {
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s %s", wan_dev, base + 1, base + 1, aqd, target);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s %s", wan_dev, base + 2, base + 2, aqd, target);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s %s", wan_dev, base + 3, base + 3, aqd, target);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s %s", wan_dev, base + 4, base + 4, aqd, target);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s %s", wan_dev, base + 5, base + 5, aqd, target);

		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 1, base + 1, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 2, base + 2, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 3, base + 3, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 4, base + 4, aqd);
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq0", base + 5, base + 5, aqd);

		if (nvram_match("wshaper_dev", "LAN")) {
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 1, base + 1, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 2, base + 2, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 3, base + 3, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 4, base + 4, aqd);
			sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", "imq1", base + 5, base + 5, aqd);
		}
	}
#endif
}

#ifdef HAVE_AQOS

void add_usermac(char *mac, int base, char *upstream, char *downstream, char *lanstream)
{
	unsigned int uprate = atoi(upstream);
	unsigned int downrate = atoi(downstream);
	unsigned int lanrate = atoi(lanstream);

	char srvname[32], srvtype[32], srvdata[32], srvlevel[32];
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(0));

	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata, srvlevel) < 4)
			break;

		add_client_mac_srvfilter(srvname, srvtype, srvdata, srvlevel, base, mac);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", mac, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark", qos_nfmark(base));

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");
}

void add_userip(char *ip, int base, char *upstream, char *downstream, char *lanstream)
{
	unsigned int uprate = atoi(upstream);
	unsigned int downrate = atoi(downstream);
	unsigned int lanrate = atoi(lanstream);

//      int ret;
	char srvname[32], srvtype[32], srvdata[32], srvlevel[32];
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(0));

	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "RETURN");

	eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "VPN_DSCP");
	eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %31s ", srvname, srvtype, srvdata, srvlevel) < 4)
			break;

		add_client_ip_srvfilter(srvname, srvtype, srvdata, srvlevel, base, ip);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark", qos_nfmark(base));
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark", qos_nfmark(base));
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark", qos_nfmark(base));
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", ip, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark", qos_nfmark(base));

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");

	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "VPN_DSCP");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "RETURN");
}
#endif
#endif				// HAVE_SVQOS
