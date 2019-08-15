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
	{ "FORWARD", 1, 31 },
	{ "HOTSPOT", 8, 23 },
	{ "QOS", 13, 10 },
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

static char
*get_wshaper_dev(void)
{
	if (nvram_match("wshaper_dev", "WAN"))
		return get_wan_face();
	else
		return "br0";
}

int get_mtu_val(void)
{
	char buf[32];
	if (nvram_match("wshaper_dev", "WAN")
	    && !strcmp(get_wshaper_dev(), "ppp0"))
		return nvram_geti("wan_mtu");
	else if (nvram_match("wshaper_dev", "WAN")) {
		if (nvram_matchi("wan_mtu", 1500))
			return atoi(getMTU(get_wshaper_dev()));
		else
			return nvram_geti("wan_mtu");
	} else
		return atoi(getBridgeMTU(get_wshaper_dev(), buf));
}

struct namemaps {
	char *from;
	char *to;
};
static struct namemaps NM[] = {
	{ "applejuice", "apple" },
	{ "bearshare", "gnu" },
	{ "bittorrent", "bit" },
	{ "directconnect", "dc" },
	{ "edonkey", "edk" },
	{ "gnutella", "gnu" },
	{ "soulseek", "soul" }
};

void add_client_dev_srvfilter(char *name, char *type, char *data, int level, int base, char *chain)
{
	int idx = level / 10;

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

void add_client_mac_srvfilter(char *name, char *type, char *data, int level, int base, char *client)
{
	int idx = level / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-m", "mac", "--mac-source", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		char ndpi[32];
		snprintf(ndpi, 32, "--%s", name);
		insmod("xt_ndpi");
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
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

			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "mac", "--mac-source", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
#else
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "length", "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK",
				     "--set-mark", qos_nfmark(base + idx));
#endif
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			}
		}
	}
}

void add_client_ip_srvfilter(char *name, char *type, char *data, int level, int base, char *client)
{
	int idx = level / 10;

	if (idx == 10)
		idx = 0;

	if (strstr(type, "udp") || strstr(type, "both")) {
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "udp", "-m", "udp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "udp", "-m", "udp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "tcp") || strstr(type, "both")) {

		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-p", "tcp", "-m", "tcp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-s", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--dport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-p", "tcp", "-m", "tcp", "--sport", data, "-d", client, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}

	if (strstr(type, "l7")) {
		insmod("ipt_layer7");
		insmod("xt_layer7");
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto", name, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
	}
#ifdef HAVE_OPENDPI
	if (strstr(type, "dpi")) {
		char ndpi[32];
		snprintf(ndpi, 32, "--%s", name);
		insmod("xt_ndpi");

		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
		eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "ndpi", ndpi, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

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

			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
			eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "ipp2p", ipp2p, "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

			if (!strcmp(proto, "bit")) {
				// bittorrent detection enhanced 
#ifdef HAVE_MICRO

				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));

#else
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "--length", "0:550", "-m", "layer7", "--l7proto", "bt", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
#endif
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto", "bt1", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-s", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-d", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-s", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
				eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-d", client, "-m", "layer7", "--l7proto", "bt2", "-j", "MARK", "--set-mark", qos_nfmark(base + idx));
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
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
static void add_tc_class(char *dev, int pref, int handle, int classid)
{
	sysprintf("tc filter add dev %s protocol ip pref %d handle 0x%x fw classid 1:%d", dev, pref, handle, classid);
}
#else
static void add_tc_mark(char *dev, char *mark, int flow)
{
	sysprintf("tc filter add dev %s protocol ip parent 1: u32 match mark %s flowid 1:%d", dev, mark, flow);
}
#endif
static void add_tc_sfq(char *dev, int parent, int handle, int quantum)
{
	sysprintf("tc qdisc add dev %s parent 1:%d handle %d: sfq quantum %d perturb 10", dev, parent, handle, quantum);
}

#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)

static void add_tc_codel(char *dev, int parent, int handle, char *aqd, char *target)
{
	if (target)
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s %s", dev, parent, handle, aqd, target);
	else
		sysprintf("tc qdisc add dev %s parent 1:%d handle %d: %s", dev, parent, handle, aqd);
}
#endif
static void add_tc_htb(char *dev, int parent, int class, int rate, int ceil, int quantum, int prio)
{
	if (prio > -1)
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d prio 0", dev, parent, class, rate, ceil, quantum, prio);
	else
		sysprintf("tc class add dev %s parent 1:%d classid 1:%d htb rate %dkbit ceil %dkbit quantum %d", dev, parent, class, rate, ceil, quantum);
}

static void add_tc_hfsc(char *dev, int parent, int class, int uprate, int uplimit)
{
	sysprintf("tc class add dev %s parent 1:%d classid 1:%d hfsc sc rate %dkbit ul rate %dkbit", dev, parent, class, uprate, uplimit);
}

#ifdef HAVE_AQOS
void add_client_classes(unsigned int base, unsigned int uprate, unsigned int downrate, unsigned int lanrate, unsigned int level)
{
	char *wan_dev = get_wan_face();

	unsigned int uplimit = nvram_geti("wshaper_uplink");
	unsigned int downlimit = nvram_geti("wshaper_downlink");
	unsigned int lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;
	char target[64] = "";

	int max = 50;
	int sec = 0;
	if (uprate <= 2000) {
		sec = max - (uprate / 50);
		sprintf(target, "target %dms noecn", sec);
	}

	unsigned int quantum = get_mtu_val() + 14;

	if (lanrate < 1)
		lanrate = lanlimit;

#else
void add_client_classes(unsigned int base, unsigned int level)
{
	char *wan_dev = get_wan_face();

	unsigned int uplimit = nvram_geti("wshaper_uplink");
	unsigned int downlimit = nvram_geti("wshaper_downlink");
	unsigned int lanlimit = 1000000;
	unsigned int prio;
	unsigned int parent;

	unsigned int quantum = get_mtu_val() + 14;

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
	unsigned int uprates[5] = { uprate * 75 / 100, uprate * 50 / 100, uprate * 25 / 100, uprate * 15 / 100, uprate * 5 / 100 };
	unsigned int downrates[5] = { downrate * 75 / 100, downrate * 50 / 100, downrate * 25 / 100, downrate * 15 / 100, downrate * 5 / 100 };
	unsigned int lanrates[5] = { lanrate * 75 / 100, lanrate * 50 / 100, lanrate * 25 / 100, lanrate * 15 / 100, lanrate * 5 / 100 };
	if (nvram_matchi("qos_type", 0)) {
		char prios[5] = { 0, prio, prio + 1, prio + 1, 7 };

		add_tc_htb(wan_dev, parent, base, uprate, uplimit, quantum, -1);
		add_tc_htb("imq0", parent, base, downrate, downlimit, quantum, -1);
		if (nvram_match("wshaper_dev", "LAN")) {
			add_tc_htb("imq1", parent, base, lanrate, lanlimit, quantum, -1);
		}
		int i;
		for (i = 0; i < 5; i++) {
			add_tc_htb(wan_dev, base, base + 1 + i, uprates[i], uplimit, quantum, prios[i]);
			add_tc_htb("imq0", base, base + 1 + i, downrates[i], downlimit, quantum, prios[i]);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_tc_htb("imq1", base, base + 1 + i, lanrates[i], lanlimit, quantum, prios[i]);
			}
		}

	} else {
		add_tc_hfsc(wan_dev, 1, base, uprate, uplimit);
		add_tc_hfsc("imq0", 1, base, downrate, downlimit);
		add_tc_hfsc("imq1", 1, base, lanrate, lanlimit);
		int i;
		for (i = 0; i < 5; i++) {
			add_tc_hfsc(wan_dev, base, base + 1 + i, uprates[i], uplimit);
			add_tc_hfsc("imq0", base, base + 1 + i, downrates[i], downlimit);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_tc_hfsc("imq1", base, base + 1 + i, lanrates[i], lanlimit);
			}
		}

	}
#if defined(ARCH_broadcom) && !defined(HAVE_BCMMODERN)
	// filter rules
	int i;
	char priorities[5] = { 1, 3, 5, 8, 9 };
	for (i = 0; i < 5; i++) {
		add_tc_class(wan_dev, priorities[i], base + i, base + 1 + i);
		add_tc_class("imq1", priorities[i], base + i, base + 1 + i);

		if (nvram_match("wshaper_dev", "LAN")) {
			add_tc_class("imq0", priorities[i], base + i, base + 1 + i);
		}
	}
#else
	int i;
	for (i = 0; i < 5; i++) {

		add_tc_mark(wan_dev, get_tcfmark(base + i), base + 1 + i);

		add_tc_mark("imq0", get_tcfmark(base + i), base + 1 + i);

		if (nvram_match("wshaper_dev", "LAN")) {
			add_tc_mark("imq1", get_tcfmark(base + i), base + 1 + i);
		}
	}
#endif

	// leaf qdiscs
	if (!strcmp(aqd, "sfq")) {
		int i;
		for (i = 1; i < 6; i++) {
			add_tc_sfq(wan_dev, base + i, base + i, quantum);
			add_tc_sfq("imq0", base + i, base + i, quantum);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_tc_sfq("imq1", base + i, base + i, quantum);
			}
		}

	}
#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)
	if (!strcmp(aqd, "codel")
	    || !strcmp(aqd, "fq_codel")
	    || !strcmp(aqd, "cake")
	    || !strcmp(aqd, "pie")) {

		int i;
		for (i = 1; i < 6; i++) {
			add_tc_codel(wan_dev, base + i, base + i, aqd, target);
			add_tc_codel("imq0", base + i, base + i, aqd, NULL);
			if (nvram_match("wshaper_dev", "LAN")) {
				add_tc_codel("imq1", base + i, base + i, aqd, NULL);
			}
		}
	}
#endif
}

#ifdef HAVE_AQOS

void add_usermac(char *mac, int base, int uprate, int downrate, int lanrate)
{

	char srvname[32], srvtype[32], srvdata[32];
	int srvlevel;
	char *qos_svcs = nvram_safe_get("svqos_svcs");

	char nullmask[24];
	strcpy(nullmask, qos_nfmark(0));

	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-D", "FILTER_IN", "-j", "RETURN");

	add_client_classes(base, uprate, downrate, lanrate, 0);

	do {
		if (sscanf(qos_svcs, "%31s %31s %31s %d ", srvname, srvtype, srvdata, &srvlevel) < 4)
			break;

		add_client_mac_srvfilter(srvname, srvtype, srvdata, srvlevel, base, mac);
	} while ((qos_svcs = strpbrk(++qos_svcs, "|")) && qos_svcs++);

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-m", "mac", "--mac-source", mac, "-m", "mark", "--mark", nullmask, "-j", "MARK", "--set-mark", qos_nfmark(base));

	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_IN", "-j", "RETURN");
}

void add_userip(char *ip, int base, int uprate, int downrate, int lanrate)
{

//      int ret;
	char srvname[32], srvtype[32], srvdata[32];
	int srvlevel;
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
		if (sscanf(qos_svcs, "%31s %31s %31s %d ", srvname, srvtype, srvdata, &srvlevel) < 4)
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

void deinit_qos(char *wandev, char *imq_wan, char *imq_lan)
{
	eval("tc", "qdisc", "del", "dev", wandev, "root");
	eval("tc", "qdisc", "del", "dev", imq_wan, "root");
	eval("tc", "qdisc", "del", "dev", imq_lan, "root");
	eval("ip", "link", "set", imq_wan, "down");
	eval("ip", "link", "set", imq_lan, "down");
}

static char *math(char *buf, int val, char *ext)
{
	sprintf(buf, "%d%s", val, ext);
	return buf;
}

void init_qos(char *type, int up, int down, char *wandev, int mtu, char *imq_wan, char *aqd, char *imq_lan)
{
	deinit_qos(wandev, imq_wan, imq_lan);
	char qmtu[32];
	sprintf(qmtu, "%d", mtu + 14);
	char buf[32];

	char *TGT = NULL;
	char *MS = NULL;
	char *ECN = NULL;
	int ll = 1000000;
	if (!strcmp(type, "hfsc")) {
		TGT = "target";
		MS = "5ms";
	}

	if (up < 2000) {
		TGT = "target";
		MS = "20ms";
		ECN = "noecn";
	}

	if (!strcmp(type, "htb")) {
		eval("tc", "qdisc", "add", "dev", wandev, "root", "handle", "1:", "htb", "default", "30");
		eval("tc", "class", "add", "dev", wandev, "parent", "1:", "classid", "1:1", "htb", "rate", math(buf, up, "kbit"), "ceil", math(buf, up, "kbit"), "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:2", "htb", "rate", math(buf, 75 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:3", "htb", "rate", math(buf, 50 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:4", "htb", "rate", math(buf, 25 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:5", "htb", "rate", math(buf, 15 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:6", "htb", "rate", math(buf, 5 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:2", "classid", "1:100", "htb", "rate", math(buf, 75 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "prio", "0", "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:3", "classid", "1:10", "htb", "rate", math(buf, 50 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "prio", "1", "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:4", "classid", "1:20", "htb", "rate", math(buf, 25 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "prio", "2", "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:5", "classid", "1:30", "htb", "rate", math(buf, 15 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "prio", "5", "quantum", qmtu);
		eval("tc", "class", "add", "dev", wandev, "parent", "1:6", "classid", "1:40", "htb", "rate", math(buf, 5 * up / 100, "kbit"), "ceil", math(buf, up, "kbit"), "prio", "7", "quantum", qmtu);
	}
	if (!strcmp(type, "hfsc")) {

		eval("tc", "qdisc", "add", "dev", wandev, "root", "handle", "1:", "hfsc", "default", "30");
		eval("tc", "class", "add", "dev", wandev, "parent", "1:", "classid", "1:1", "hfsc", "sc", "rate", math(buf, up, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:2", "hfsc", "sc", "rate", math(buf, 75 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:3", "hfsc", "sc", "rate", math(buf, 50 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:4", "hfsc", "sc", "rate", math(buf, 25 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:5", "hfsc", "sc", "rate", math(buf, 15 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:1", "classid", "1:6", "hfsc", "sc", "rate", math(buf, 5 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:2", "classid", "1:100", "hfsc", "sc", "rate", math(buf, 75 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:3", "classid", "1:10", "hfsc", "sc", "rate", math(buf, 50 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:4", "classid", "1:20", "hfsc", "sc", "rate", math(buf, 25 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:5", "classid", "1:30", "hfsc", "sc", "rate", math(buf, 15 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
		eval("tc", "class", "add", "dev", wandev, "parent", "1:6", "classid", "1:40", "hfsc", "sc", "rate", math(buf, 5 * up / 100, "kbit"), "ul", "rate", math(buf, up, "kbit"));
	}

	if (!strcmp(aqd, "sfq")) {

		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:100", "handle", "100:", "sfq", "quantum", qmtu, "perturb", "10");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:10", "handle", "10:", "sfq", "quantum", qmtu, "perturb", "10");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:20", "handle", "20:", "sfq", "quantum", qmtu, "perturb", "10");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:30", "handle", "30:", "sfq", "quantum", qmtu, "perturb", "10");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:40", "handle", "40:", "sfq", "quantum", qmtu, "perturb", "10");
	}
	if (!strcmp(aqd, "codel")) {

		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:100", "handle", "100:", aqd, TGT, MS, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:10", "handle", "10:", aqd, TGT, MS, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:20", "handle", "20:", aqd, TGT, MS, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:30", "handle", "30:", aqd, TGT, MS, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:40", "handle", "40:", aqd, TGT, MS, ECN);
	}
	if (!strcmp(aqd, "fq_codel")) {

		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:100", "handle", "100:", aqd);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:10", "handle", "10:", aqd);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:20", "handle", "20:", aqd);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:30", "handle", "30:", aqd);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:40", "handle", "40:", aqd);
	}
	if (!strcmp(aqd, "cake")) {

		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:100", "handle", "100:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:10", "handle", "10:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:20", "handle", "20:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:30", "handle", "30:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat");
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:40", "handle", "40:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat");
	}
	if (!strcmp(aqd, "pie")) {

		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:100", "handle", "100:", aqd, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:10", "handle", "10:", aqd, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:20", "handle", "20:", aqd, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:30", "handle", "30:", aqd, ECN);
		eval("tc", "qdisc", "add", "dev", wandev, "parent", "1:40", "handle", "40:", aqd, ECN);
	}

	eval("tc", "filter", "add", "dev", wandev, "protocol", "ip", "pref", "1", "handle", "0x64", "fw", "classid", "1:100");
	eval("tc", "filter", "add", "dev", wandev, "protocol", "ip", "pref", "3", "handle", "0x0A", "fw", "classid", "1:10");
	eval("tc", "filter", "add", "dev", wandev, "protocol", "ip", "pref", "5", "handle", "0x14", "fw", "classid", "1:20");
	eval("tc", "filter", "add", "dev", wandev, "protocol", "ip", "pref", "8", "handle", "0x1E", "fw", "classid", "1:30");
	eval("tc", "filter", "add", "dev", wandev, "protocol", "ip", "pref", "9", "handle", "0x28", "fw", "classid", "1:40");

	if (down != 0) {

		eval("ip", "link", "set", imq_wan, "up");

		if (!strcmp(type, "htb")) {
			eval("tc", "qdisc", "add", "dev", imq_wan, "root", "handle", "1:", "htb", "default", "30");
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:", "classid", "1:1", "htb", "rate", math(buf, down, "kbit"), "ceil", math(buf, down, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:2", "htb", "rate", math(buf, 75 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:3", "htb", "rate", math(buf, 50 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:4", "htb", "rate", math(buf, 25 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:5", "htb", "rate", math(buf, 15 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:6", "htb", "rate", math(buf, 5 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:2", "classid", "1:100", "htb", "rate", math(buf, 75 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "prio", "0", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:3", "classid", "1:10", "htb", "rate", math(buf, 50 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "prio", "1", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:4", "classid", "1:20", "htb", "rate", math(buf, 25 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "prio", "2", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:5", "classid", "1:30", "htb", "rate", math(buf, 15 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "prio", "5", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:6", "classid", "1:40", "htb", "rate", math(buf, 5 * down / 100, "kbit"), "ceil", math(buf, down, "kbit"), "prio", "7", "quantum", qmtu);
		}
		if (!strcmp(type, "hfsc")) {
			eval("tc", "qdisc", "add", "dev", imq_wan, "root", "handle", "1:", "hfsc", "default", "30");
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:", "classid", "1:1", "hfsc", "sc", "rate", math(buf, down, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:2", "hfsc", "sc", "rate", math(buf, 75 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:3", "hfsc", "sc", "rate", math(buf, 50 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:4", "hfsc", "sc", "rate", math(buf, 25 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:5", "hfsc", "sc", "rate", math(buf, 15 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:1", "classid", "1:6", "hfsc", "sc", "rate", math(buf, 5 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:2", "classid", "1:100", "hfsc", "sc", "rate", math(buf, 75 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:3", "classid", "1:10", "hfsc", "sc", "rate", math(buf, 50 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:4", "classid", "1:20", "hfsc", "sc", "rate", math(buf, 25 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:5", "classid", "1:30", "hfsc", "sc", "rate", math(buf, 15 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
			eval("tc", "class", "add", "dev", imq_wan, "parent", "1:6", "classid", "1:40", "hfsc", "sc", "rate", math(buf, 5 * down / 100, "kbit"), "ul", "rate", math(buf, down, "kbit"));
		}
		if (!strcmp(aqd, "sfq")) {

			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:100", "handle", "100:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:10", "handle", "10:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:20", "handle", "20:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:30", "handle", "30:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:40", "handle", "40:", "sfq", "quantum", qmtu, "perturb", "10");
		}
		if (!strcmp(aqd, "codel")) {

			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:100", "handle", "100:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:10", "handle", "10:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:20", "handle", "20:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:30", "handle", "30:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:40", "handle", "40:", aqd, TGT, ECN);
		}
		if (!strcmp(aqd, "fq_codel")) {

			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:100", "handle", "100:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:10", "handle", "10:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:20", "handle", "20:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:30", "handle", "30:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:40", "handle", "40:", aqd);
		}
		if (!strcmp(aqd, "cake")) {

			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:100", "handle", "100:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-dsthost", "ack-filter", "nat");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:10", "handle", "10:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-dsthost", "ack-filter", "nat");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:20", "handle", "20:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-dsthost", "ack-filter", "nat");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:30", "handle", "30:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-dsthost", "ack-filter", "nat");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:40", "handle", "40:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-dsthost", "ack-filter", "nat");
		}
		if (!strcmp(aqd, "pie")) {

			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:100", "handle", "100:", aqd, "target", "5 ms", "ecn");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:10", "handle", "10:", aqd, "target", "5 ms", "ecn");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:20", "handle", "20:", aqd, "target", "5 ms", "ecn");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:30", "handle", "30:", aqd, "target", "5 ms", "ecn");
			eval("tc", "qdisc", "add", "dev", imq_wan, "parent", "1:40", "handle", "40:", aqd, "target", "5 ms", "ecn");
		}

		eval("tc", "filter", "add", "dev", imq_wan, "protocol", "ip", "pref", "1", "handle", "0x64", "fw", "classid", "1:100");
		eval("tc", "filter", "add", "dev", imq_wan, "protocol", "ip", "pref", "3", "handle", "0x0A", "fw", "classid", "1:10");
		eval("tc", "filter", "add", "dev", imq_wan, "protocol", "ip", "pref", "5", "handle", "0x14", "fw", "classid", "1:20");
		eval("tc", "filter", "add", "dev", imq_wan, "protocol", "ip", "pref", "8", "handle", "0x1E", "fw", "classid", "1:30");
		eval("tc", "filter", "add", "dev", imq_wan, "protocol", "ip", "pref", "9", "handle", "0x28", "fw", "classid", "1:40");
	}
	if (strcmp(imq_lan, "0")) {
		eval("ip", "link", "set", imq_lan, "up");

		if (!strcmp(type, "htb")) {
			eval("tc", "qdisc", "add", "dev", imq_lan, "root", "handle", "1:", "htb", "default", "30");
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:", "classid", "1:1", "htb", "rate", math(buf, ll, "kbit"), "prio", "0", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:2", "htb", "rate", math(buf, 75 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:3", "htb", "rate", math(buf, 50 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:4", "htb", "rate", math(buf, 25 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:5", "htb", "rate", math(buf, 15 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:6", "htb", "rate", math(buf, 5 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:2", "classid", "1:100", "htb", "rate", math(buf, 75 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "prio", "0", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:3", "classid", "1:10", "htb", "rate", math(buf, 50 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "prio", "1", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:4", "classid", "1:20", "htb", "rate", math(buf, 25 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "prio", "2", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:5", "classid", "1:30", "htb", "rate", math(buf, 15 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "prio", "5", "quantum", qmtu);
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:6", "classid", "1:40", "htb", "rate", math(buf, 5 * ll / 100, "kbit"), "ceil", math(buf, ll, "kbit"), "prio", "7", "quantum", qmtu);
		}
		if (!strcmp(type, "hfsc")) {
			eval("tc", "qdisc", "add", "dev", imq_lan, "root", "handle", "1:", "hfsc", "default", "30");
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:", "classid", "1:1", "hfsc", "sc", "rate", math(buf, ll, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:2", "hfsc", "sc", "rate", math(buf, 75 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:3", "hfsc", "sc", "rate", math(buf, 50 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:4", "hfsc", "sc", "rate", math(buf, 25 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:5", "hfsc", "sc", "rate", math(buf, 15 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:1", "classid", "1:6", "hfsc", "sc", "rate", math(buf, 5 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:2", "classid", "1:100", "hfsc", "sc", "rate", math(buf, 75 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:3", "classid", "1:10", "hfsc", "sc", "rate", math(buf, 50 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:4", "classid", "1:20", "hfsc", "sc", "rate", math(buf, 25 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:5", "classid", "1:30", "hfsc", "sc", "rate", math(buf, 15 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
			eval("tc", "class", "add", "dev", imq_lan, "parent", "1:6", "classid", "1:40", "hfsc", "sc", "rate", math(buf, 5 * ll / 100, "kbit"), "ul", "rate", math(buf, ll, "kbit"));
		}
		if (!strcmp(aqd, "sfq")) {

			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:100", "handle", "100:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:10", "handle", "10:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:20", "handle", "20:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:30", "handle", "30:", "sfq", "quantum", qmtu, "perturb", "10");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:40", "handle", "40:", "sfq", "quantum", qmtu, "perturb", "10");
		}
		if (!strcmp(aqd, "codel")) {

			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:100", "handle", "100:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:10", "handle", "10:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:20", "handle", "20:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:30", "handle", "30:", aqd, TGT, ECN);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:40", "handle", "40:", aqd, TGT, ECN);
		}
		if (!strcmp(aqd, "fq_codel")) {

			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:100", "handle", "100:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:10", "handle", "10:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:20", "handle", "20:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:30", "handle", "30:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:40", "handle", "40:", aqd);
		}
		if (!strcmp(aqd, "cake")) {

			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:100", "handle", "100:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "lan", "triple-isolate", "no-ack-filter", "nonat");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:10", "handle", "10:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "lan", "triple-isolate", "no-ack-filter", "nonat");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:20", "handle", "20:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "lan", "triple-isolate", "no-ack-filter", "nonat");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:30", "handle", "30:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "lan", "triple-isolate", "no-ack-filter", "nonat");
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:40", "handle", "40:", aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "lan", "triple-isolate", "no-ack-filter", "nonat");
		}
		if (!strcmp(aqd, "pie")) {

			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:100", "handle", "100:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:10", "handle", "10:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:20", "handle", "20:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:30", "handle", "30:", aqd);
			eval("tc", "qdisc", "add", "dev", imq_lan, "parent", "1:40", "handle", "40:", aqd);
		}

		eval("tc", "filter", "add", "dev", imq_lan, "protocol", "ip", "pref", "1", "handle", "0x64", "fw", "classid", "1:100");
		eval("tc", "filter", "add", "dev", imq_lan, "protocol", "ip", "pref", "3", "handle", "0x0A", "fw", "classid", "1:10");
		eval("tc", "filter", "add", "dev", imq_lan, "protocol", "ip", "pref", "5", "handle", "0x14", "fw", "classid", "1:20");
		eval("tc", "filter", "add", "dev", imq_lan, "protocol", "ip", "pref", "8", "handle", "0x1E", "fw", "classid", "1:30");
		eval("tc", "filter", "add", "dev", imq_lan, "protocol", "ip", "pref", "9", "handle", "0x28", "fw", "classid", "1:40");
	}

}

#endif				//","HAVE_SVQOS
