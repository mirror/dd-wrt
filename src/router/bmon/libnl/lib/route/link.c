/*
 * lib/route/link.c	Links (Interfaces)
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup rtnl
 * @defgroup link Links (Interfaces)
 * @brief
 *
 * @par Link Identification
 * A link can be identified by either its interface index or by its
 * name. The kernel favours the interface index but falls back to the
 * interface name if the interface index is lesser-than 0 for kernels
 * >= 2.6.11. Therefore you can request changes without mapping a
 * interface name to the corresponding index first.
 *
 * @par Changeable Attributes
 * @anchor link_changeable
 *  - Link layer address
 *  - Link layer broadcast address
 *  - device mapping (ifmap) (>= 2.6.9)
 *  - MTU (>= 2.6.9)
 *  - Transmission queue length (>= 2.6.9)
 *  - Weight (>= 2.6.9)
 *  - Link name (only via access through interface index) (>= 2.6.9)
 *  - Flags (>= 2.6.9)
 *    - IFF_DEBUG
 *    - IFF_NOTRAILERS
 *    - IFF_NOARP
 *    - IFF_DYNAMIC
 *    - IFF_MULTICAST
 *    - IFF_PORTSEL
 *    - IFF_AUTOMEDIA
 *    - IFF_UP
 *    - IFF_PROMISC
 *    - IFF_ALLMULTI
 *
 * @par Link Flags (linux/if.h)
 * @anchor link_flags
 * @code
 *   IFF_UP            Status of link (up|down)
 *   IFF_BROADCAST     Indicates this link allows broadcasting
 *   IFF_MULTICAST     Indicates this link allows multicasting
 *   IFF_ALLMULTI      Indicates this link is doing multicast routing
 *   IFF_DEBUG         Tell the driver to do debugging (currently unused)
 *   IFF_LOOPBACK      This is the loopback link
 *   IFF_POINTOPOINT   Point-to-point link
 *   IFF_NOARP         Link is unable to perform ARP
 *   IFF_PROMISC       Status of promiscious mode flag
 *   IFF_MASTER        Used by teql
 *   IFF_SLAVE         Used by teql
 *   IFF_PORTSEL       Indicates this link allows port selection
 *   IFF_AUTOMEDIA     Indicates this link selects port automatically
 *   IFF_DYNAMIC       Indicates the address of this link is dynamic
 *   IFF_RUNNING       Link is running and carrier is ok.
 *   IFF_NOTRAILERS    Unused, BSD compat.
 * @endcode
 *
 * @par Notes on IFF_PROMISC and IFF_ALLMULTI flags
 * Although you can query the status of IFF_PROMISC and IFF_ALLMULTI
 * they do not represent the actual state in the kernel but rather
 * whether the flag has been enabled/disabled by userspace. The link
 * may be in promiscious mode even if IFF_PROMISC is not set in a link
 * dump request response because promiscity might be needed by the driver
 * for a period of time.
 *
 * @par 1) Retrieving information about available links
 * @code
 * // The first step is to retrieve a list of all available interfaces within
 * // the kernel and put them into a cache.
 * struct nl_cache *cache = rtnl_link_alloc_cache(nl_handle);
 *
 * // In a second step, a specific link may be looked up by either interface
 * // index or interface name.
 * struct rtnl_link *link = rtnl_link_get_by_name(cache, "lo");
 *
 * // rtnl_link_get_by_name() is the short version for translating the
 * // interface name to an interface index first like this:
 * int ifindex = rtnl_link_name2i(cache, "lo");
 * struct rtnl_link *link = rtnl_link_get(cache, ifindex);
 *
 * // After successful usage, the object must be given back to the cache
 * rtnl_link_put(link);
 * @endcode
 *
 * @par 2) Changing link attributes
 * @code
 * // In order to change any attributes of an existing link, we must allocate
 * // a new link to hold the change requests:
 * struct rtnl_link *request = rtnl_link_alloc();
 *
 * // Now we can go on and specify the attributes we want to change:
 * rtnl_link_set_weight(request, 300);
 * rtnl_link_set_mtu(request, 1360);
 *
 * // We can also shut an interface down administratively
 * rtnl_link_unset_flags(request, rtnl_link_str2flags("up"));
 *
 * // Actually, we should know which link to change, so let's look it up
 * struct rtnl_link *old = rtnl_link_get(cache, "eth0");
 *
 * // Two ways exist to commit this change request, the first one is to
 * // build the required netlink message and send it out in one single
 * // step:
 * rtnl_link_change(nl_handle, old, request);
 *
 * // An alternative way is to build the netlink message and send it
 * // out yourself using nl_send_auto_complete()
 * struct nl_msg *msg = rtnl_link_build_change_request(old, request);
 * nl_send_auto_complete(nl_handle, nlmsg_hdr(msg));
 * nlmsg_free(msg);
 *
 * // Don't forget to give back the link object ;->
 * rtnl_link_put(old);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/utils.h>
#include <netlink/object.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>

/** @cond SKIP */
#define LINK_ATTR_MTU     0x0001
#define LINK_ATTR_LINK    0x0002
#define LINK_ATTR_TXQLEN  0x0004
#define LINK_ATTR_WEIGHT  0x0008
#define LINK_ATTR_MASTER  0x0010
#define LINK_ATTR_QDISC   0x0020
#define LINK_ATTR_MAP     0x0040
#define LINK_ATTR_ADDR    0x0080
#define LINK_ATTR_BRD     0x0100
#define LINK_ATTR_FLAGS   0x0200
#define LINK_ATTR_IFNAME  0x0400
#define LINK_ATTR_IFINDEX 0x0800
#define LINK_ATTR_FAMILY  0x1000
#define LINK_ATTR_ARPTYPE 0x2000
#define LINK_ATTR_STATS   0x4000
#define LINK_ATTR_CHANGE  0x8000

static struct nl_cache_ops rtnl_link_ops;
/** @endcond */

static void link_free_data(struct nl_object *c)
{
	struct rtnl_link *link = nl_object_priv(c);

	if (link) {
		nl_addr_put(link->l_addr);
		nl_addr_put(link->l_bcast);
	}
}

static struct nla_policy link_policy[IFLA_MAX+1] = {
	[IFLA_IFNAME]	= { .type = NLA_STRING,
			    .maxlen = IFNAMSIZ },
	[IFLA_MTU]	= { .type = NLA_U32 },
	[IFLA_TXQLEN]	= { .type = NLA_U32 },
	[IFLA_LINK]	= { .type = NLA_U32 },
	[IFLA_WEIGHT]	= { .type = NLA_U32 },
	[IFLA_MASTER]	= { .type = NLA_U32 },
	[IFLA_QDISC]	= { .type = NLA_STRING,
			    .maxlen = IFQDISCSIZ },
	[IFLA_STATS]	= { .minlen = sizeof(struct rtnl_link_stats) },
	[IFLA_MAP]	= { .minlen = sizeof(struct rtnl_link_ifmap) },
};

static int link_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			   void *arg)
{
	struct rtnl_link *link;
	struct ifinfomsg *ifi;
	struct nlattr *tb[IFLA_MAX+1];
	struct nl_parser_param *pp = arg;
	int err;

	link = rtnl_link_alloc();
	if (link == NULL) {
		err = nl_errno(ENOMEM);
		goto errout;
	}
		
	link->ce_msgtype = n->nlmsg_type;

	err = nlmsg_parse(n, sizeof(*ifi), tb, IFLA_MAX, link_policy);
	if (err < 0)
		goto errout;

	if (tb[IFLA_IFNAME] == NULL) {
		err = nl_error(EINVAL, "Missing link name TLV");
		goto errout;
	}

	nla_strlcpy(link->l_name, tb[IFLA_IFNAME], IFNAMSIZ);

	ifi = nlmsg_data(n);
	link->l_family = ifi->ifi_family;
	link->l_arptype = ifi->ifi_type;
	link->l_index = ifi->ifi_index;
	link->l_flags = ifi->ifi_flags;
	link->l_change = ifi->ifi_change;
	link->l_mask = (LINK_ATTR_IFNAME | LINK_ATTR_FAMILY | LINK_ATTR_ARPTYPE|
		        LINK_ATTR_IFINDEX | LINK_ATTR_FLAGS | LINK_ATTR_CHANGE);

	if (tb[IFLA_STATS]) {
		struct rtnl_link_stats *st = nla_data(tb[IFLA_STATS]);
		
		link->l_stats[RTNL_LINK_RX_PACKETS]	= st->rx_packets;
		link->l_stats[RTNL_LINK_RX_BYTES]	= st->rx_bytes;
		link->l_stats[RTNL_LINK_RX_ERRORS]	= st->rx_errors;
		link->l_stats[RTNL_LINK_RX_DROPPED]	= st->rx_dropped;
		link->l_stats[RTNL_LINK_RX_COMPRESSED]	= st->rx_compressed;
		link->l_stats[RTNL_LINK_RX_FIFO_ERR]	= st->rx_fifo_errors;
		link->l_stats[RTNL_LINK_TX_PACKETS]	= st->tx_packets;
		link->l_stats[RTNL_LINK_TX_BYTES]	= st->tx_bytes;
		link->l_stats[RTNL_LINK_TX_ERRORS]	= st->tx_errors;
		link->l_stats[RTNL_LINK_TX_DROPPED]	= st->tx_dropped;
		link->l_stats[RTNL_LINK_TX_COMPRESSED]	= st->tx_compressed;
		link->l_stats[RTNL_LINK_TX_FIFO_ERR]	= st->tx_fifo_errors;
		link->l_stats[RTNL_LINK_RX_LEN_ERR]	= st->rx_length_errors;
		link->l_stats[RTNL_LINK_RX_OVER_ERR]	= st->rx_over_errors;
		link->l_stats[RTNL_LINK_RX_CRC_ERR]	= st->rx_crc_errors;
		link->l_stats[RTNL_LINK_RX_FRAME_ERR]	= st->rx_frame_errors;
		link->l_stats[RTNL_LINK_RX_MISSED_ERR]	= st->rx_missed_errors;
		link->l_stats[RTNL_LINK_TX_ABORT_ERR]	= st->tx_aborted_errors;
		link->l_stats[RTNL_LINK_TX_CARRIER_ERR]	= st->tx_carrier_errors;
		link->l_stats[RTNL_LINK_TX_HBEAT_ERR]	= st->tx_heartbeat_errors;
		link->l_stats[RTNL_LINK_TX_WIN_ERR]	= st->tx_window_errors;
		link->l_stats[RTNL_LINK_MULTICAST]	= st->multicast;

		link->l_mask |= LINK_ATTR_STATS;
	}

	if (tb[IFLA_TXQLEN]) {
		link->l_txqlen = nla_get_u32(tb[IFLA_TXQLEN]);
		link->l_mask |= LINK_ATTR_TXQLEN;
	}

	if (tb[IFLA_MTU]) {
		link->l_mtu = nla_get_u32(tb[IFLA_MTU]);
		link->l_mask |= LINK_ATTR_MTU;
	}

	if (tb[IFLA_ADDRESS]) {
		link->l_addr = nla_get_addr(tb[IFLA_ADDRESS], AF_UNSPEC);
		if (link->l_addr == NULL)
			goto errout;
		nl_addr_set_family(link->l_addr,
				   nl_addr_guess_family(link->l_addr));
		link->l_mask |= LINK_ATTR_ADDR;
	}

	if (tb[IFLA_BROADCAST]) {
		link->l_bcast = nla_get_addr(tb[IFLA_BROADCAST], AF_UNSPEC);
		if (link->l_bcast == NULL)
			goto errout;
		nl_addr_set_family(link->l_bcast,
				   nl_addr_guess_family(link->l_bcast));
		link->l_mask |= LINK_ATTR_BRD;
	}

	if (tb[IFLA_LINK]) {
		link->l_link = nla_get_u32(tb[IFLA_LINK]);
		link->l_mask |= LINK_ATTR_LINK;
	}

	if (tb[IFLA_WEIGHT]) {
		link->l_weight = nla_get_u32(tb[IFLA_WEIGHT]);
		link->l_mask |= LINK_ATTR_WEIGHT;
	}

	if (tb[IFLA_QDISC]) {
		nla_strlcpy(link->l_qdisc, tb[IFLA_QDISC], IFQDISCSIZ);
		link->l_mask |= LINK_ATTR_QDISC;
	}

	if (tb[IFLA_MAP]) {
		struct rtnl_link_ifmap *map =  nla_data(tb[IFLA_MAP]);
		link->l_map.lm_mem_start = map->mem_start;
		link->l_map.lm_mem_end   = map->mem_end;
		link->l_map.lm_base_addr = map->base_addr;
		link->l_map.lm_irq       = map->irq;
		link->l_map.lm_dma       = map->dma;
		link->l_map.lm_port      = map->port;
		link->l_mask |= LINK_ATTR_MAP;
	}

	if (tb[IFLA_MASTER]) {
		link->l_master = nla_get_u32(tb[IFLA_MASTER]);
		link->l_mask |= LINK_ATTR_MASTER;
	}

	err = pp->pp_cb((struct nl_object *) link, pp);
	if (err < 0)
		goto errout;

	return P_ACCEPT;

errout:
	rtnl_link_put(link);
	return err;
}

static int link_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETLINK, AF_UNSPEC, NLM_F_DUMP);
}

static int link_dump_brief(struct nl_object *obj, struct nl_dump_params *p)
{
	char buf[128];
	struct nl_cache *cache = dp_cache(obj);
	struct rtnl_link *link = (struct rtnl_link *) obj;
	int line = 1;

	dp_dump(p, "%s ", link->l_name);

	if (link->l_mask & LINK_ATTR_LINK) {
		struct rtnl_link *ll = rtnl_link_get(cache, link->l_link);
		dp_dump(p, "@%s", ll ? ll->l_name : "NONE");
		if (ll)
			rtnl_link_put(ll);
	}

	dp_dump(p, "%s ", nl_llproto2str(link->l_arptype, buf, sizeof(buf)));
	dp_dump(p, "%s ", link->l_addr ?  nl_addr2str(link->l_addr, buf,
						     sizeof(buf)) : "none");
	dp_dump(p, "mtu %u ", link->l_mtu);

	if (link->l_mask & LINK_ATTR_MASTER) {
		struct rtnl_link *master = rtnl_link_get(cache, link->l_master);
		dp_dump(p, "master %s ", master ? master->l_name : "inv");
		if (master)
			rtnl_link_put(master);
	}

	rtnl_link_flags2str(link->l_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump(p, "<%s>", buf);

	dp_dump(p, "\n");

	return line;
}

static int link_dump_full(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	char buf[64];
	int line;

	line = link_dump_brief(obj, p);
	dp_new_line(p, line++);

	dp_dump(p, "    txqlen %u weight %u ", link->l_txqlen, link->l_weight);

	if (link->l_mask & LINK_ATTR_QDISC)
		dp_dump(p, "qdisc %s ", link->l_qdisc);

	if (link->l_mask & LINK_ATTR_MAP && link->l_map.lm_irq)
		dp_dump(p, "irq %u ", link->l_map.lm_irq);

	if (link->l_mask & LINK_ATTR_IFINDEX)
		dp_dump(p, "index %u ", link->l_index);

	if (link->l_mask & LINK_ATTR_BRD)
		dp_dump(p, "brd %s", nl_addr2str(link->l_bcast, buf,
						   sizeof(buf)));

	dp_dump(p, "\n");

	return line;
}

static int link_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	char *unit, fmt[64];
	float res;
	int line;
	
	line = link_dump_full(obj, p);

	dp_dump_line(p, line++, "    Stats:    bytes    packets     errors "
				"   dropped   fifo-err compressed\n");

	res = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_RX_BYTES], &unit);

	strcpy(fmt, "    RX  %X.2f %s %10llu %10llu %10llu %10llu %10llu\n");
	fmt[9] = *unit == 'B' ? '9' : '7';
	
	dp_dump_line(p, line++, fmt,
		res, unit,
		link->l_stats[RTNL_LINK_RX_PACKETS],
		link->l_stats[RTNL_LINK_RX_ERRORS],
		link->l_stats[RTNL_LINK_RX_DROPPED],
		link->l_stats[RTNL_LINK_RX_FIFO_ERR],
		link->l_stats[RTNL_LINK_RX_COMPRESSED]);

	res = nl_cancel_down_bytes(link->l_stats[RTNL_LINK_TX_BYTES], &unit);

	strcpy(fmt, "    TX  %X.2f %s %10llu %10llu %10llu %10llu %10llu\n");
	fmt[9] = *unit == 'B' ? '9' : '7';
	
	dp_dump_line(p, line++, fmt,
		res, unit,
		link->l_stats[RTNL_LINK_TX_PACKETS],
		link->l_stats[RTNL_LINK_TX_ERRORS],
		link->l_stats[RTNL_LINK_TX_DROPPED],
		link->l_stats[RTNL_LINK_TX_FIFO_ERR],
		link->l_stats[RTNL_LINK_TX_COMPRESSED]);

	dp_dump_line(p, line++, "    Errors:  length       over        crc "
				"     frame     missed  multicast\n");

	dp_dump_line(p, line++, "    RX   %10" PRIu64 " %10" PRIu64 " %10"
				PRIu64 " %10" PRIu64 " %10" PRIu64 " %10"
				PRIu64 "\n",
		link->l_stats[RTNL_LINK_RX_LEN_ERR],
		link->l_stats[RTNL_LINK_RX_OVER_ERR],
		link->l_stats[RTNL_LINK_RX_CRC_ERR],
		link->l_stats[RTNL_LINK_RX_FRAME_ERR],
		link->l_stats[RTNL_LINK_RX_MISSED_ERR],
		link->l_stats[RTNL_LINK_MULTICAST]);

	dp_dump_line(p, line++, "    Errors: aborted    carrier  heartbeat "
				"    window  collision\n");
	
	dp_dump_line(p, line++, "    TX   %10" PRIu64 " %10" PRIu64 " %10"
				PRIu64 " %10" PRIu64 " %10" PRIu64 "\n",
		link->l_stats[RTNL_LINK_TX_ABORT_ERR],
		link->l_stats[RTNL_LINK_TX_CARRIER_ERR],
		link->l_stats[RTNL_LINK_TX_HBEAT_ERR],
		link->l_stats[RTNL_LINK_TX_WIN_ERR],
		link->l_stats[RTNL_LINK_TX_COLLISIONS]);

	return line;
}

static int link_dump_xml(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	struct nl_cache *cache = dp_cache(obj);
	char buf[128];
	int i, line = 0;

	dp_dump_line(p, line++, "<link name=\"%s\" index=\"%u\">\n",
		     link->l_name, link->l_index);
	dp_dump_line(p, line++, "  <family>%s</family>\n",
		     nl_af2str(link->l_family, buf, sizeof(buf)));
	dp_dump_line(p, line++, "  <arptype>%s</arptype>\n",
		     nl_llproto2str(link->l_arptype, buf, sizeof(buf)));
	dp_dump_line(p, line++, "  <address>%s</address>\n",
		     nl_addr2str(link->l_addr, buf, sizeof(buf)));
	dp_dump_line(p, line++, "  <mtu>%u</mtu>\n", link->l_mtu);
	dp_dump_line(p, line++, "  <txqlen>%u</txqlen>\n", link->l_txqlen);
	dp_dump_line(p, line++, "  <weight>%u</weight>\n", link->l_weight);

	rtnl_link_flags2str(link->l_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "  <flags>%s</flags>\n", buf);

	if (link->l_mask & LINK_ATTR_QDISC)
		dp_dump_line(p, line++, "  <qdisc>%s</qdisc>\n", link->l_qdisc);

	if (link->l_mask & LINK_ATTR_LINK) {
		struct rtnl_link *ll = rtnl_link_get(cache, link->l_link);
		dp_dump_line(p, line++, "  <link>%s</link>\n",
			     ll ? ll->l_name : "none");
		if (ll)
			rtnl_link_put(ll);
	}

	if (link->l_mask & LINK_ATTR_MASTER) {
		struct rtnl_link *master = rtnl_link_get(cache, link->l_master);
		dp_dump_line(p, line++, "  <master>%s</master>\n",
			     master ? master->l_name : "none");
		if (master)
			rtnl_link_put(master);
	}

	if (link->l_mask & LINK_ATTR_BRD)
		dp_dump_line(p, line++, "  <broadcast>%s</broadcast>\n",
			     nl_addr2str(link->l_bcast, buf, sizeof(buf)));

	if (link->l_mask & LINK_ATTR_STATS) {
		dp_dump_line(p, line++, "  <stats>\n");
		for (i = 0; i <= RTNL_LINK_STATS_MAX; i++) {
			rtnl_link_stat2str(i, buf, sizeof(buf));
			dp_dump_line(p, line++,
				     "    <%s>%" PRIu64 "</%s>\n",
				     buf, link->l_stats[i], buf);
		}
		dp_dump_line(p, line++, "  </stats>\n");
	}

	dp_dump_line(p, line++, "</link>\n");

#if 0
	uint32_t	l_change;	/**< Change mask */
	struct rtnl_lifmap l_map;	/**< Interface device mapping */
#endif

	return line;
}

static int link_dump_env(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_link *link = (struct rtnl_link *) obj;
	struct nl_cache *cache = dp_cache(obj);
	char buf[128];
	int i, line = 0;

	dp_dump_line(p, line++, "LINK_NAME=%s\n", link->l_name);
	dp_dump_line(p, line++, "LINK_IFINDEX=%u\n", link->l_index);
	dp_dump_line(p, line++, "LINK_FAMILY=%s\n",
		     nl_af2str(link->l_family, buf, sizeof(buf)));
	dp_dump_line(p, line++, "LINK_TYPE=%s\n",
		     nl_llproto2str(link->l_arptype, buf, sizeof(buf)));
	if (link->l_mask & LINK_ATTR_ADDR)
		dp_dump_line(p, line++, "LINK_ADDRESS=%s\n",
			     nl_addr2str(link->l_addr, buf, sizeof(buf)));
	dp_dump_line(p, line++, "LINK_MTU=%u\n", link->l_mtu);
	dp_dump_line(p, line++, "LINK_TXQUEUELEN=%u\n", link->l_txqlen);
	dp_dump_line(p, line++, "LINK_WEIGHT=%u\n", link->l_weight);

	rtnl_link_flags2str(link->l_flags & ~IFF_RUNNING, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "LINK_FLAGS=%s\n", buf);

	if (link->l_mask & LINK_ATTR_QDISC)
		dp_dump_line(p, line++, "LINK_QDISC=%s\n", link->l_qdisc);

	if (link->l_mask & LINK_ATTR_LINK) {
		struct rtnl_link *ll = rtnl_link_get(cache, link->l_link);

		dp_dump_line(p, line++, "LINK_LINK_IFINDEX=%d\n", link->l_link);
		if (ll) {
			dp_dump_line(p, line++, "LINK_LINK_IFNAME=%s\n",
				     ll->l_name);
			rtnl_link_put(ll);
		}
	}

	if (link->l_mask & LINK_ATTR_MASTER) {
		struct rtnl_link *master = rtnl_link_get(cache, link->l_master);
		dp_dump_line(p, line++, "LINK_MASTER=%s\n",
			     master ? master->l_name : "none");
		if (master)
			rtnl_link_put(master);
	}

	if (link->l_mask & LINK_ATTR_BRD)
		dp_dump_line(p, line++, "LINK_BROADCAST=%s\n",
			     nl_addr2str(link->l_bcast, buf, sizeof(buf)));

	if (link->l_mask & LINK_ATTR_STATS) {
		for (i = 0; i <= RTNL_LINK_STATS_MAX; i++) {
			char *c = buf;

			sprintf(buf, "LINK_");
			rtnl_link_stat2str(i, buf + 5, sizeof(buf) - 5);
			while (*c) {
				*c = toupper(*c);
				c++;
			}
			dp_dump_line(p, line++,
				     "%s=%" PRIu64 "\n", buf, link->l_stats[i]);
		}
	}

	return line;
}

#if 0
static int link_handle_event(struct nl_object *a, struct rtnl_link_event_cb *cb)
{
	struct rtnl_link *l = (struct rtnl_link *) a;
	struct nl_cache *c = dp_cache(a);
	int nevents = 0;

	if (l->l_change == ~0U) {
		if (l->ce_msgtype == RTM_NEWLINK)
			cb->le_register(l);
		else
			cb->le_unregister(l);

		return 1;
	}

	if (l->l_change & IFF_SLAVE) {
		if (l->l_flags & IFF_SLAVE) {
			struct rtnl_link *m = rtnl_link_get(c, l->l_master);
			cb->le_new_bonding(l, m);
			if (m)
				rtnl_link_put(m);
		} else
			cb->le_cancel_bonding(l);
	}

#if 0
	if (l->l_change & IFF_UP && l->l_change & IFF_RUNNING)
		dp_dump_line(p, line++, "link %s changed state to %s.\n",
			l->l_name, l->l_flags & IFF_UP ? "up" : "down");

	if (l->l_change & IFF_PROMISC) {
		dp_new_line(p, line++);
		dp_dump(p, "link %s %s promiscuous mode.\n",
		    l->l_name, l->l_flags & IFF_PROMISC ? "entered" : "left");
	}

	if (line == 0)
		dp_dump_line(p, line++, "link %s sent unknown event.\n",
			     l->l_name);
#endif

	return nevents;
}
#endif

static int link_filter(struct nl_object *obj, struct nl_object *filter)
{
	struct rtnl_link *o = (struct rtnl_link *) obj;
	struct rtnl_link *f = (struct rtnl_link *) filter;

#define REQ(F) (f->l_mask & LINK_ATTR_##F)
#define AVAIL(F) (o->l_mask & LINK_ATTR_##F)
#define F_CUS(F, EXPR) (REQ(F) && (!AVAIL(F) || (EXPR)))
#define F_INT(F, N) (REQ(F) && (!AVAIL(F) || (o->N != f->N)))
	if (F_INT(MTU,	l_mtu)						||
	    F_INT(LINK,	l_link)						||
	    F_INT(TXQLEN,	l_txqlen)				||
	    F_INT(WEIGHT,	l_weight)				||
	    F_INT(MASTER,	l_master)				||
	    F_INT(IFINDEX,	l_index)				||
	    F_INT(FAMILY,	l_family)				||
	    F_CUS(QDISC,	strcmp(o->l_qdisc, f->l_qdisc))		||
	    F_CUS(IFNAME,	strcmp(o->l_name, f->l_name))		||
	    F_CUS(ADDR,	nl_addr_cmp(o->l_addr, f->l_addr))		||
	    F_CUS(BRD,	nl_addr_cmp(o->l_bcast, f->l_bcast))		||
	    F_CUS(FLAGS,	f->l_flags ^ (o->l_flags & f->l_flag_mask)))
		return 0;
#undef REQ
#undef AVAIL
#undef F_CUS
#undef F_INT

	return 1;
}

/**
 * @name Link Object Allocation/Freeage
 * @{
 */

/**
 * Allocate and initialize new link object.
 * @note Free the memory after usage using rtnl_link_put() or rtnl_link_free().
 * @return Newly allocated link object or NULL if an error occured.
 */
struct rtnl_link *rtnl_link_alloc(void)
{
	return (struct rtnl_link *) nl_object_alloc_from_ops(&rtnl_link_ops);
}

/**
 * Give back reference on link object.
 * @arg link		Link object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_link_put(struct rtnl_link *link)
{
	nl_object_put((struct nl_object *) link);
}
/**
 * Free link object.
 * @arg link		Link object to be freed.
 *
 * @note Always use rtnl_link_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_link_free(struct rtnl_link *link)
{
	nl_object_free((struct nl_object *) link);
}

/** @} */

/**
 * @name Link Cache Management
 * @{
 */


/**
 * Allocate link cache and fill in all configured links.
 * @arg handle		Netlink handle.
 *
 * Allocates a new link cache, initializes it properly and updates it
 * to include all links currently configured in the kernel.
 *
 * @note Free the memory after usage.
 * @return Newly allocated cache or NULL if an error occured.
 */
struct nl_cache *rtnl_link_alloc_cache(struct nl_handle *handle)
{
	struct nl_cache * cache;
	
	cache = nl_cache_alloc_from_ops(&rtnl_link_ops);
	if (cache == NULL)
		return NULL;
	
	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Look up link by interface index in the provided cache
 * @arg cache		link cache
 * @arg ifindex		link interface index
 *
 * The caller owns a reference on the returned object and
 * must give the object back via rtnl_link_put().
 *
 * @return pointer to link inside the cache or NULL if no match was found.
 */
struct rtnl_link *rtnl_link_get(struct nl_cache *cache, int ifindex)
{
	struct rtnl_link *link;

	if (cache->c_ops != &rtnl_link_ops)
		return NULL;

	nl_list_for_each_entry(link, &cache->c_items, ce_list) {
		if (link->l_index == ifindex) {
			nl_object_get((struct nl_object *) link);
			return link;
		}
	}

	return NULL;
}

/**
 * Look up link by link name in the provided cache
 * @arg cache		link cache
 * @arg name		link name
 *
 * The caller owns a reference on the returned object and
 * must give the object back via rtnl_link_put().
 *
 * @return pointer to link inside the cache or NULL if no match was found.
 */
struct rtnl_link *rtnl_link_get_by_name(struct nl_cache *cache,
					 const char *name)
{
	struct rtnl_link *link;

	if (cache->c_ops != &rtnl_link_ops)
		return NULL;

	nl_list_for_each_entry(link, &cache->c_items, ce_list) {
		if (!strcmp(name, link->l_name)) {
			nl_object_get((struct nl_object *) link);
			return link;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Link Modifications
 * @{
 */

/**
 * Builds a netlink change request message to change link attributes
 * @arg old		link to be changed
 * @arg tmpl		template with requested changes
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a change of link attributes.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed.
 * \a old must point to a link currently configured in the kernel
 * and \a tmpl must contain the attributes to be changed set via
 * \c rtnl_link_set_* functions.
 *
 * @return New netlink message
 * @note Not all attributes can be changed, see
 *       \ref link_changeable "Changeable Attributes" for more details.
 */
struct nl_msg * rtnl_link_build_change_request(struct rtnl_link *old,
					       struct rtnl_link *tmpl,
					       int flags)
{
	struct nl_msg *msg;
	struct ifinfomsg ifi = {
		.ifi_family = old->l_family,
		.ifi_index = old->l_index,
	};

	if (tmpl->l_mask & LINK_ATTR_FLAGS) {
		ifi.ifi_flags = old->l_flags & ~tmpl->l_flag_mask;
		ifi.ifi_flags |= tmpl->l_flags;
	}

	msg = nlmsg_build_simple(RTM_SETLINK, flags);
	if (!msg)
		goto nla_put_failure;

	if (nlmsg_append(msg, &ifi, sizeof(ifi), 1) < 0)
		goto nla_put_failure;

	if (tmpl->l_mask & LINK_ATTR_ADDR)
		NLA_PUT_ADDR(msg, IFLA_ADDRESS, tmpl->l_addr);

	if (tmpl->l_mask & LINK_ATTR_BRD)
		NLA_PUT_ADDR(msg, IFLA_BROADCAST, tmpl->l_bcast);

	if (tmpl->l_mask & LINK_ATTR_MTU)
		NLA_PUT_U32(msg, IFLA_MTU, tmpl->l_mtu);

	if (tmpl->l_mask & LINK_ATTR_TXQLEN)
		NLA_PUT_U32(msg, IFLA_TXQLEN, tmpl->l_txqlen);

	if (tmpl->l_mask & LINK_ATTR_WEIGHT)
		NLA_PUT_U32(msg, IFLA_WEIGHT, tmpl->l_weight);

	if (tmpl->l_mask & LINK_ATTR_IFNAME)
		NLA_PUT_STRING(msg, IFLA_IFNAME, tmpl->l_name);

	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

/**
 * Change link attributes
 * @arg handle		netlink handle
 * @arg old		link to be changed
 * @arg tmpl		template with requested changes
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message by calling rtnl_link_build_change_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received, i.e. blocks until the request has been processed.
 *
 * @return 0 on success or a negative error code
 * @note Not all attributes can be changed, see
 *       \ref link_changeable "Changeable Attributes" for more details.
 */
int rtnl_link_change(struct nl_handle *handle, struct rtnl_link *old,
		     struct rtnl_link *tmpl, int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_link_build_change_request(old, tmpl, flags);
	if (!msg)
		return nl_errno(ENOMEM);
	
	err = nl_send_auto_complete(handle, msg);
	if (err < 0)
		return err;

	nlmsg_free(msg);
	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name Name <-> Index Translations
 * @{
 */

/**
 * Translate an interface index to the corresponding link name
 * @arg cache		link cache
 * @arg ifindex		link interface index
 * @arg dst		destination buffer
 * @arg len		length of destination buffer
 *
 * Translates the specified interface index to the corresponding
 * link name and stores the name in the destination buffer.
 *
 * @return link name or NULL if no match was found.
 */
char * rtnl_link_i2name(struct nl_cache *cache, int ifindex, char *dst,
			size_t len)
{
	struct rtnl_link *link = rtnl_link_get(cache, ifindex);

	if (link) {
		strncpy(dst, link->l_name, len - 1);
		rtnl_link_put(link);
		return dst;
	}

	return NULL;
}

/**
 * Translate a link name to the corresponding interface index
 * @arg cache		link cache
 * @arg name		link name
 *
 * @return interface index or RTNL_LINK_NOT_FOUND if no match was found.
 */
int rtnl_link_name2i(struct nl_cache *cache, const char *name)
{
	int ifindex = RTNL_LINK_NOT_FOUND;
	struct rtnl_link *link;
	
	link = rtnl_link_get_by_name(cache, name);
	if (link) {
		ifindex = link->l_index;
		rtnl_link_put(link);
	}

	return ifindex;
}

/** @} */

/**
 * @name Link Flags Translations
 * @{
 */

static struct trans_tbl link_flags[] = {
	__ADD(IFF_LOOPBACK, loopback)
	__ADD(IFF_BROADCAST, broadcast)
	__ADD(IFF_POINTOPOINT, pointopoint)
	__ADD(IFF_MULTICAST, multicast)
	__ADD(IFF_NOARP, noarp)
	__ADD(IFF_ALLMULTI, allmulti)
	__ADD(IFF_PROMISC, promisc)
	__ADD(IFF_MASTER, master)
	__ADD(IFF_SLAVE, slave)
	__ADD(IFF_DEBUG, debug)
	__ADD(IFF_DYNAMIC, dynamic)
	__ADD(IFF_AUTOMEDIA, automedia)
	__ADD(IFF_PORTSEL, portsel)
	__ADD(IFF_NOTRAILERS, notrailers)
	__ADD(IFF_UP, up)
	__ADD(IFF_RUNNING, running)
	__ADD(IFF_LOWER_UP, lowerup)
	__ADD(IFF_DORMANT, dormant)
};

/**
 * Convert link flags to a character string (Reentrant).
 * @arg flags		link flags
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts link flags to a character string separated by
 * commands and stores it in the specified destination buffer.
 *
 * \return The destination buffer
 */
char * rtnl_link_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, link_flags,
			   ARRAY_SIZE(link_flags));
}

/**
 * Convert a character string to a link flag
 * @arg name		Name of link flag
 *
 * Converts the provided character string specifying a link
 * link the corresponding numeric value.
 *
 * \return Link flag or a negative value if none was found.
 */
int rtnl_link_str2flags(const char *name)
{
	return __str2flags(name, link_flags, ARRAY_SIZE(link_flags));
}

/** @} */

/**
 * @name Link Statistics Translations
 * @{
 */

static struct trans_tbl link_stats[] = {
	__ADD(RTNL_LINK_RX_PACKETS, rx_packets)
	__ADD(RTNL_LINK_TX_PACKETS, tx_packets)
	__ADD(RTNL_LINK_RX_BYTES, rx_bytes)
	__ADD(RTNL_LINK_TX_BYTES, tx_bytes)
	__ADD(RTNL_LINK_RX_ERRORS, rx_errors)
	__ADD(RTNL_LINK_TX_ERRORS, tx_errors)
	__ADD(RTNL_LINK_RX_DROPPED, rx_dropped)
	__ADD(RTNL_LINK_TX_DROPPED, tx_dropped)
	__ADD(RTNL_LINK_RX_COMPRESSED, rx_compressed)
	__ADD(RTNL_LINK_TX_COMPRESSED, tx_compressed)
	__ADD(RTNL_LINK_RX_FIFO_ERR, rx_fifo_err)
	__ADD(RTNL_LINK_TX_FIFO_ERR, tx_fifo_err)
	__ADD(RTNL_LINK_RX_LEN_ERR, rx_len_err)
	__ADD(RTNL_LINK_RX_OVER_ERR, rx_over_err)
	__ADD(RTNL_LINK_RX_CRC_ERR, rx_crc_err)
	__ADD(RTNL_LINK_RX_FRAME_ERR, rx_frame_err)
	__ADD(RTNL_LINK_RX_MISSED_ERR, rx_missed_err)
	__ADD(RTNL_LINK_TX_ABORT_ERR, tx_abort_err)
	__ADD(RTNL_LINK_TX_CARRIER_ERR, tx_carrier_err)
	__ADD(RTNL_LINK_TX_HBEAT_ERR, tx_hbeat_err)
	__ADD(RTNL_LINK_TX_WIN_ERR, tx_win_err)
	__ADD(RTNL_LINK_TX_COLLISIONS, tx_collision)
	__ADD(RTNL_LINK_MULTICAST, multicast)
};

/**
 * Convert a link statistic to a character string (Reentrant).
 * @arg st		link statistic
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a link statistic to a character string and stores it in
 * the specified destination buffer.
 *
 * @return The destination buffer or the statistic encoded in
 *         hexidecimal form if no match was found.
 */
char *rtnl_link_stat2str(int st, char *buf, size_t len)
{
	return __type2str(st, buf, len, link_stats, ARRAY_SIZE(link_stats));
}

/**
 * Convert a character string to a link statistic
 * @arg name		name of link statistic
 *
 * Converts the provided character string specifying a link
 * statistic to the corresponding numeric value.
 *
 * @return Link statistic or a negative value if none was found.
 */
int rtnl_link_str2stat(const char *name)
{
	return __str2type(name, link_stats, ARRAY_SIZE(link_stats));
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set QDisc name
 * @arg link		link to change
 * @arg qdisc		qdisc name
 */
void rtnl_link_set_qdisc(struct rtnl_link *link, const char *qdisc)
{
	strncpy(link->l_qdisc, qdisc, sizeof(link->l_qdisc) - 1);
	link->l_mask |= LINK_ATTR_QDISC;
}

/**
 * Get QDisc name
 * @arg link		link handle
 * @return Name of the qdisc or NULL if not set.
 */
char *rtnl_link_get_qdisc(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_QDISC)
		return link->l_qdisc;
	else
		return NULL;
}

/**
 * Set new link name
 * @arg link		link to change
 * @arg name		new link name
 */
void rtnl_link_set_name(struct rtnl_link *link, const char *name)
{
	strncpy(link->l_name, name, sizeof(link->l_name) - 1);
	link->l_mask |= LINK_ATTR_IFNAME;
}

/**
 * Get link name
 * @arg link		link handle
 * @return Name of the link or NULL if not set.
 */
char *rtnl_link_get_name(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_IFNAME)
		return link->l_name;
	else
		return NULL;
}

static inline void __assign_addr(struct rtnl_link *link, struct nl_addr **pos,
				 struct nl_addr *new, int flag)
{
	if (*pos)
		nl_addr_put(*pos);

	nl_addr_get(new);
	*pos = new;

	link->l_mask |= flag;
}

/**
 * Set link layer address
 * @arg link		link to change
 * @arg addr		new link layer address
 */
void rtnl_link_set_addr(struct rtnl_link *link, struct nl_addr *addr)
{
	__assign_addr(link, &link->l_addr, addr, LINK_ATTR_ADDR);
}

/**
 * Get link layer address
 * @arg link		link handle
 * @return link layer address or NULL if not set
 */
struct nl_addr *rtnl_link_get_addr(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_ADDR)
		return link->l_addr;
	else
		return NULL;
}

/**
 * Set link layer broadcast address
 * @arg link		link to change
 * @arg brd		new link layer broadcast address
 *
 * Assigns the new broadcast address to the specified link handle.
 *
 * @note The prefix length of the address will be ignored.
 */
void rtnl_link_set_broadcast(struct rtnl_link *link, struct nl_addr *brd)
{
	__assign_addr(link, &link->l_bcast, brd, LINK_ATTR_BRD);
}

/**
 * Get link layer broadcast address
 * @arg link		link handle
 * @return Link layer broadcast address or NULL if not set
 */
struct nl_addr *rtnl_link_get_broadcast(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_BRD)
		return link->l_bcast;
	else
		return NULL;
}

/**
 * Set flags
 * @arg link		link to change
 * @arg flags		flags to set (see \ref link_flags "Link Flags")
 */
void rtnl_link_set_flags(struct rtnl_link *link, unsigned int flags)
{
	link->l_flag_mask |= flags;
	link->l_flags |= flags;
	link->l_mask |= LINK_ATTR_FLAGS;
}

/**
 * Unset flags
 * @arg link		link to change
 * @arg flags		flags to unset (see \ref link_flags "Link Flags")
 */
void rtnl_link_unset_flags(struct rtnl_link *link, unsigned int flags)
{
	link->l_flag_mask |= flags;
	link->l_flags &= ~flags;
	link->l_mask |= LINK_ATTR_FLAGS;
}

/**
 * Get flags
 * @arg link		link handle
 * @return Link flags
 */
unsigned int rtnl_link_get_flags(struct rtnl_link *link)
{
	return link->l_flags;
}

/**
 * Set link layer address family
 * @arg link		link to change
 * @arg family		new address family
 */
void rtnl_link_set_family(struct rtnl_link *link, int family)
{
	link->l_family = family;
	link->l_mask |= LINK_ATTR_FAMILY;
}

/**
 * Get link layer address family
 * @arg link		link handle
 * @return Link layer address family or AF_UNSPEC if not set.
 */
int rtnl_link_get_family(struct rtnl_link *link)
{
	if (link->l_family & LINK_ATTR_FAMILY)
		return link->l_family;
	else
		return AF_UNSPEC;
}

/**
 * Set link layer type
 * @arg link		link handle
 * @arg arptype		Link layer type.
 */
void rtnl_link_set_type(struct rtnl_link *link, unsigned int arptype)
{
	link->l_arptype = arptype;
}

/**
 * Get link layer type
 * @arg link		link handle
 * @return Link layer type.
 */
unsigned int rtnl_link_get_arptype(struct rtnl_link *link)
{
	return link->l_arptype;
}

/**
 * Set interface index
 * @arg link		link to change
 * @arg ifindex		new interface index
 */
void rtnl_link_set_ifindex(struct rtnl_link *link, int ifindex)
{
	link->l_index = ifindex;
	link->l_mask |= LINK_ATTR_IFINDEX;
}

/**
 * Get interface index
 * @arg link		link handle
 * @return interface index or RTNL_LINK_NOT_FOUND if not set.
 */
int rtnl_link_get_ifindex(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_IFINDEX)
		return link->l_index;
	else
		return RTNL_LINK_NOT_FOUND;
}

/**
 * Set Maximum Transmission Unit
 * @arg link		link to change
 * @arg mtu		new MTU
 */
void rtnl_link_set_mtu(struct rtnl_link *link, unsigned int mtu)
{
	link->l_mtu = mtu;
	link->l_mask |= LINK_ATTR_MTU;
}

/**
 * Get Maximum Transmission Unit
 * @arg link		link handle
 * @return Link MTU or 0 if MTU is not set.
 */
unsigned int rtnl_link_get_mtu(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_MTU)
		return link->l_mtu;
	else
		return 0;
}

/**
 * Set Transmission Queue Length
 * @arg link		link to change
 * @arg txqlen		new TX queue length
 * @note The unit of the transmission queue length depends on the
 *       link type, a common unit is \a packets.
 */
void rtnl_link_set_txqlen(struct rtnl_link *link, unsigned int txqlen)
{
	link->l_txqlen = txqlen;
	link->l_mask |= LINK_ATTR_TXQLEN;
}

/**
 * Get Transmission Queue Length
 * @arg link		link handle
 * @return Transmission Queue Length or UINT_MAX if not set.
 */
unsigned int rtnl_link_get_txqlen(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_TXQLEN)
		return link->l_txqlen;
	else
		return UINT_MAX;
}

/**
 * Set Weight
 * @arg link		link to change
 * @arg weight		new weight
 */
void rtnl_link_set_weight(struct rtnl_link *link, unsigned int weight)
{
	link->l_weight = weight;
	link->l_mask |= LINK_ATTR_WEIGHT;
}

/**
 * Get Weight
 * @arg link		link handle
 * @return Link weight or UINT_MAX if not set.
 */
unsigned int rtnl_link_get_weight(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_WEIGHT)
		return link->l_weight;
	else
		return UINT_MAX;
}

/**
 * Set parent interface index
 * @arg link		link to change
 * @arg ifindex		new parent's interface index
 */
void rtnl_link_set_link(struct rtnl_link *link, int ifindex)
{
	link->l_link = ifindex;
	link->l_mask |= LINK_ATTR_LINK;
}

/**
 * Get parent interface index
 * @arg link		link handle
 * @return Parent interface index or RTNL_LINK_NOT_FOUND if not set
 */
int rtnl_link_get_link(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_LINK)
		return link->l_link;
	else
		return RTNL_LINK_NOT_FOUND;
}

/**
 * Set master interface index
 * @arg link		link to change
 * @arg ifindex		new master's interface index
 */
void rtnl_link_set_master(struct rtnl_link *link, int ifindex)
{
	link->l_master = ifindex;
	link->l_mask |= LINK_ATTR_MASTER;
}

/**
 * Get master interface index
 * @arg link		link handle
 * @return Interface index of master or RTNL_LINK_NOT_FOUND if not set
 */
int rtnl_link_get_master(struct rtnl_link *link)
{
	if (link->l_mask & LINK_ATTR_MASTER)
		return link->l_master;
	else
		return RTNL_LINK_NOT_FOUND;
}

/**
 * Get the statistic specified by the id
 * @arg link		link handle
 * @arg id		statistic id
 * @return The current counter of the specified statistic
 */
uint64_t rtnl_link_get_stat(struct rtnl_link *link, int id)
{
	if (id < 0 || id > RTNL_LINK_STATS_MAX)
		return 0;

	return link->l_stats[id];
}

/** @} */

static struct nl_cache_ops rtnl_link_ops = {
	.co_name		= "route/link",
	.co_size		= sizeof(struct rtnl_link),
	.co_hdrsize		= sizeof(struct ifinfomsg),
	.co_msgtypes		= {
					{ RTM_NEWLINK, "new" },
					{ RTM_DELLINK, "delete" },
					{ RTM_GETLINK, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= link_request_update,
	.co_msg_parser		= link_msg_parser,
	.co_free_data		= link_free_data,
	.co_dump[NL_DUMP_BRIEF]	= link_dump_brief,
	.co_dump[NL_DUMP_FULL]	= link_dump_full,
	.co_dump[NL_DUMP_STATS]	= link_dump_stats,
	.co_dump[NL_DUMP_XML]	= link_dump_xml,
	.co_dump[NL_DUMP_ENV]	= link_dump_env,
	.co_filter		= link_filter,
};

static void __init link_init(void)
{
	nl_cache_mngt_register(&rtnl_link_ops);
}

static void __exit link_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_link_ops);
}

/** @} */
