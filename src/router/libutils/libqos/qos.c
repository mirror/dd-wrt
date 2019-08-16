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

//	eval("iptables", "-t", "mangle", "-D", "FILTER_OUT", "-j", "VPN_DSCP");
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

//	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "VPN_DSCP");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "CONNMARK", "--save");
	eval("iptables", "-t", "mangle", "-A", "FILTER_OUT", "-j", "RETURN");
}

void deinit_qos(const char *wandev, const char *imq_wan, const char *imq_lan)
{
	eval("tc", "qdisc", "del", "dev", wandev, "root");
	eval("tc", "qdisc", "del", "dev", imq_wan, "root");
	eval("tc", "qdisc", "del", "dev", imq_lan, "root");
	eval("ip", "link", "set", imq_wan, "down");
	eval("ip", "link", "set", imq_lan, "down");
}

static const char *math(char *buf, int val, const char *ext)
{
	sprintf(buf, "%d%s", val, ext);
	return buf;
}

static void add_hfsc_class(const char *dev, const char *parent, const char *classid, int rate, int limit)
{
	char buf[32];
	eval("tc", "class", "add", "dev", dev, "parent", parent, "classid", classid, "hfsc", "sc", "rate", math(buf, rate, "kbit"), "ul", "rate", math(buf, limit, "kbit"));
}

static void add_htb_class(const char *dev, const char *parent, const char *classid, int rate, int limit, int mtu, int p)
{
	char buf[32];
	char qmtu[32];
	char prio[32];
	sprintf(qmtu, "%d", mtu + 14);
	sprintf(prio, "%d", p);
	if (p != -1)
		eval("tc", "class", "add", "dev", dev, "parent", parent, "classid", classid, "htb", "rate", math(buf, rate, "kbit"), "ceil", math(buf, limit, "kbit"), "prio", prio, "quantum", qmtu);
	else
		eval("tc", "class", "add", "dev", dev, "parent", parent, "classid", classid, "htb", "rate", math(buf, rate, "kbit"), "ceil", math(buf, limit, "kbit"), "quantum", qmtu);
}

static void init_htb_class(const char *dev, int rate, int mtu)
{
	add_htb_class(dev, "1:", "1:1", rate, rate, mtu, -1);
	add_htb_class(dev, "1:1", "1:2", 75 * rate / 100, rate, mtu, -1);
	add_htb_class(dev, "1:1", "1:3", 50 * rate / 100, rate, mtu, -1);
	add_htb_class(dev, "1:1", "1:4", 25 * rate / 100, rate, mtu, -1);
	add_htb_class(dev, "1:1", "1:5", 15 * rate / 100, rate, mtu, -1);
	add_htb_class(dev, "1:1", "1:6", 5 * rate / 100, rate, mtu, -1);
	add_htb_class(dev, "1:2", "1:100", 75 * rate / 100, rate, mtu, 0);
	add_htb_class(dev, "1:3", "1:10", 50 * rate / 100, rate, mtu, 1);
	add_htb_class(dev, "1:4", "1:20", 25 * rate / 100, rate, mtu, 2);
	add_htb_class(dev, "1:5", "1:30", 15 * rate / 100, rate, mtu, 5);
	add_htb_class(dev, "1:6", "1:40", 5 * rate / 100, rate, mtu, 7);
}

static void init_hfsc_class(const char *dev, int rate)
{
	add_hfsc_class(dev, "1:", "1:1", rate, rate);
	add_hfsc_class(dev, "1:1", "1:2", 75 * rate / 100, rate);
	add_hfsc_class(dev, "1:1", "1:3", 50 * rate / 100, rate);
	add_hfsc_class(dev, "1:1", "1:4", 25 * rate / 100, rate);
	add_hfsc_class(dev, "1:1", "1:5", 15 * rate / 100, rate);
	add_hfsc_class(dev, "1:1", "1:6", 5 * rate / 100, rate);
	add_hfsc_class(dev, "1:2", "1:100", 75 * rate / 100, rate);
	add_hfsc_class(dev, "1:3", "1:10", 50 * rate / 100, rate);
	add_hfsc_class(dev, "1:4", "1:20", 25 * rate / 100, rate);
	add_hfsc_class(dev, "1:5", "1:30", 15 * rate / 100, rate);
	add_hfsc_class(dev, "1:6", "1:40", 5 * rate / 100, rate);

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

static void add_fq_codel(const char *dev, int handle, const char *aqd)
{
	char p[32];
	char h[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd);
}

static void add_cake(const char *dev, int handle, const char *aqd, int rtt)
{
	char p[32];
	char h[32];
	char r[32];
	sprintf(p, "1:%d", handle);
	sprintf(h, "%d:", handle);
	sprintf(r, "%dms", rtt);
	if (rtt != -1)
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat", "rtt", r);
	else
		eval("tc", "qdisc", "add", "dev", dev, "parent", p, "handle", h, aqd, "unlimited", "ethernet", "besteffort", "noatm", "raw", "internet", "dual-srchost", "ack-filter", "nat");
}

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

static void init_qdisc(const char *type, const char *dev, const char *wandev, const char *aqd, int mtu, int up, int ms5)
{
	int noecn = -1;
	int rtt = -1;
	int rtt_cake = -1;
	if (!strcmp(type, "hfsc")) {
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
	if (!strcmp(aqd, "codel")) {
		add_codel(dev, 100, aqd, rtt, noecn);
		add_codel(dev, 10, aqd, rtt, noecn);
		add_codel(dev, 20, aqd, rtt, noecn);
		add_codel(dev, 30, aqd, rtt, noecn);
		add_codel(dev, 40, aqd, rtt, noecn);
	}
	if (!strcmp(aqd, "fq_codel")) {
		add_fq_codel(dev, 100, aqd);
		add_fq_codel(dev, 10, aqd);
		add_fq_codel(dev, 20, aqd);
		add_fq_codel(dev, 30, aqd);
		add_fq_codel(dev, 40, aqd);
	}
	if (!strcmp(aqd, "cake")) {
		add_cake(dev, 100, aqd, rtt_cake);
		add_cake(dev, 10, aqd, rtt_cake);
		add_cake(dev, 20, aqd, rtt_cake);
		add_cake(dev, 30, aqd, rtt_cake);
		add_cake(dev, 40, aqd, rtt_cake);
	}
	if (!strcmp(aqd, "pie")) {
		/* for imq_wan and htb only 5ms is enforced. i dont know why. i just took it from the original script */
		if (ms5 || !strcmp(type, "hfsc"))
			noecn = 0;
		add_pie(dev, 100, aqd, ms5, noecn);
		add_pie(dev, 10, aqd, ms5, noecn);
		add_pie(dev, 20, aqd, ms5, noecn);
		add_pie(dev, 30, aqd, ms5, noecn);
		add_pie(dev, 40, aqd, ms5, noecn);
	}

}

static void add_filter(const char *dev, int pref, int handle, int classid)
{

	char p[32];
	char h[32];
	char c[32];
	sprintf(p, "%d", pref);
	sprintf(h, "0x02X", handle);
	sprintf(c, "1:%d", classid);
	eval("tc", "filter", "add", "dev", dev, "protocol", "ip", "pref", p, "handle", h, "fw", "classid", c);

}

static void init_filter(const char *dev)
{
	add_filter(dev, 1, 0x64, 100);
	add_filter(dev, 3, 0x0A, 10);
	add_filter(dev, 5, 0x14, 20);
	add_filter(dev, 8, 0x1E, 30);
	add_filter(dev, 9, 0x28, 40);
}

void init_qos(const char *type, int up, int down, const char *wandev, int mtu, const char *imq_wan, const char *aqd, const char *imq_lan)
{
	deinit_qos(wandev, imq_wan, imq_lan);

	int ll = 1000000;

	if (!strcmp(type, "htb") && strcmp(wandev, "xx")) {
		eval("tc", "qdisc", "add", "dev", wandev, "root", "handle", "1:", "htb", "default", "30");
		init_htb_class(wandev, up, mtu);
		init_qdisc(type, wandev, wandev, aqd, mtu, up, 0);
		init_filter(wandev);
	}
	if (!strcmp(type, "hfsc") && strcmp(wandev, "xx")) {

		eval("tc", "qdisc", "add", "dev", wandev, "root", "handle", "1:", "hfsc", "default", "30");
		init_hfsc_class(wandev, up);
		init_qdisc(type, wandev, wandev, aqd, mtu, up, 0);
		init_filter(wandev);
	}

	if (down != 0) {
		eval("ip", "link", "set", imq_wan, "up");

		if (!strcmp(type, "htb")) {
			eval("tc", "qdisc", "add", "dev", imq_wan, "root", "handle", "1:", "htb", "default", "30");
			init_htb_class(imq_wan, down, mtu);
			init_qdisc(type, imq_wan, wandev, aqd, mtu, up, 1);	// force 5ms for PIE on imq_wan

		}
		if (!strcmp(type, "hfsc")) {
			eval("tc", "qdisc", "add", "dev", imq_wan, "root", "handle", "1:", "hfsc", "default", "30");
			init_hfsc_class(imq_wan, down);
			init_qdisc(type, imq_wan, wandev, aqd, mtu, up, 0);
		}
		init_filter(imq_wan);

	}
	if (strcmp(imq_lan, "0")) {
		eval("ip", "link", "set", imq_lan, "up");

		if (!strcmp(type, "htb")) {
			eval("tc", "qdisc", "add", "dev", imq_lan, "root", "handle", "1:", "htb", "default", "30");
			init_htb_class(imq_lan, ll, mtu);
		}
		if (!strcmp(type, "hfsc")) {
			eval("tc", "qdisc", "add", "dev", imq_lan, "root", "handle", "1:", "hfsc", "default", "30");
			init_hfsc_class(imq_lan, ll);
		}
		init_qdisc(type, imq_lan, wandev, aqd, mtu, up, 0);

		init_filter(imq_lan);

	}

}

#endif				//","HAVE_SVQOS
