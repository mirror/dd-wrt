/*
 * link.c              rtnetlink link layer
 *
 * Copyright (c) 2003-2005 Thomas Graf <tgraf@suug.ch>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

/**
 * @ingroup rtnl
 * @defgroup link Links (Interfaces)
 * Module to access and modify links (interfaces).
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
 * @par Example 1: Retrieving a link cache and dumping all links:
 * @code
 *   struct nl_dump_params p = { .dp_type = NL_DUMP_BRIEF };
 *   struct nl_cache *lc = rtnl_link_build_cache(&nl_handle);
 *   nl_cache_dump(lc, stdout, &p);
 * @endcode
 *
 * @par Example 2: Translating link names and interface indices:
 * @code
 *   int ifindex = rtnl_link_name2i(lc, "eth0");
 *   struct rtnl_link *link = rtnl_link_get(lc, ifindex);
 * @endcode
 *
 * @par Example 3: Dumping links based on a filter:
 * @code
 *   struct nl_dump_params p = { .dp_type = NL_DUMP_FULL };
 *   struct rtnl_link filter = RTNL_INIT_LINK();
 *   rtnl_link_set_mtu(&filter, 1500);
 *   rtnl_link_set_txqlen(&filter, 0);
 *   nl_cache_dump_filter(lc, stdout, &p, (struct nl_common *) &filter);
 * @endcode
 *
 * @par Example 4: Changing weight and MTU attributes
 * @code
 *   struct rtnl_link change_req = RTNL_INIT_LINK();
 *   rtnl_link_set_weight(&change_req, 300);
 *   rtnl_link_set_mtu(&change_req, 1360);
 *   rtnl_link_change(&nl_handle, link, &change_req);
 * @endcode
 *
 * @par Example 5: Shutting an interface down
 * @code
 *   struct rtnl_link change_req = RTNL_INIT_LINK();
 *   rtnl_link_unset_flags(&change_req, rtnl_link_str2flags("up"));
 *   rtnl_link_change(&nl_handle, link, &change_req);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link.h>
#include <netlink/route/rtattr.h>
#include <inttypes.h>

static int link_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			   void *arg)
{
	struct rtnl_link link = RTNL_INIT_LINK();
	struct ifinfomsg *ifi;
	struct rtattr *tb[IFLA_MAX+1];
	struct nl_parser_param *pp = arg;
	size_t len;
	int err;

	if (n->nlmsg_type != RTM_NEWLINK)
		return P_IGNORE;

	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifinfomsg));
	if (len < 0)
		return nl_error(EINVAL, "netlink message too short to be a link message");

	ifi = NLMSG_DATA(n);
	err = nl_parse_rtattr(tb, IFLA_MAX, IFLA_RTA(ifi), len);
	if (err < 0)
		return err;

	if (tb[IFLA_IFNAME] == NULL)
		return nl_error(EINVAL, "Missing link name TLV");

	if (RTA_PAYLOAD(tb[IFLA_IFNAME]) >= (sizeof(link.l_name) - 1))
		return nl_error(EINVAL, "link name TLV length exceeds " \
			"local link name limit");

	memcpy(link.l_name, RTA_DATA(tb[IFLA_IFNAME]),
		RTA_PAYLOAD(tb[IFLA_IFNAME]));
	link.l_name[sizeof(link.l_name) - 1] = '\0';

	link.l_family = ifi->ifi_family;
	link.l_arptype = ifi->ifi_type;
	link.l_index = ifi->ifi_index;
	link.l_flags = ifi->ifi_flags;
	link.l_change = ifi->ifi_change;
	link.l_mask = (LINK_HAS_IFNAME | LINK_HAS_FAMILY | LINK_HAS_ARPTYPE |
		       LINK_HAS_IFINDEX | LINK_HAS_FLAGS | LINK_HAS_CHANGE);

	if (tb[IFLA_STATS]) {
		struct rtnl_link_stats *st;

		if (RTA_PAYLOAD(tb[IFLA_STATS]) < sizeof(*st))
			return nl_error(EINVAL, "link statistics TLV is too short");

		st = (struct rtnl_link_stats *) RTA_DATA(tb[IFLA_STATS]);

		link.l_stats.ls_rx.packets          = st->rx_packets;
		link.l_stats.ls_rx.bytes            = st->rx_bytes;
		link.l_stats.ls_rx.errors           = st->rx_errors;
		link.l_stats.ls_rx.dropped          = st->rx_dropped;
		link.l_stats.ls_rx.compressed       = st->rx_compressed;
		link.l_stats.ls_rx.multicast        = st->multicast;
		link.l_stats.ls_tx.packets          = st->tx_packets;
		link.l_stats.ls_tx.bytes            = st->tx_bytes;
		link.l_stats.ls_tx.errors           = st->tx_errors;
		link.l_stats.ls_tx.dropped          = st->tx_dropped;
		link.l_stats.ls_tx.compressed       = st->tx_compressed;

		link.l_stats.ls_rx_length_errors    = st->rx_length_errors;
		link.l_stats.ls_rx_over_errors      = st->rx_over_errors;
		link.l_stats.ls_rx_crc_errors       = st->rx_crc_errors;
		link.l_stats.ls_rx_frame_errors     = st->rx_frame_errors;
		link.l_stats.ls_rx_fifo_errors      = st->rx_fifo_errors;
		link.l_stats.ls_rx_missed_errors    = st->rx_missed_errors;

		link.l_stats.ls_tx_aborted_errors   = st->tx_aborted_errors;
		link.l_stats.ls_tx_carrier_errors   = st->tx_carrier_errors;
		link.l_stats.ls_tx_fifo_errors      = st->tx_fifo_errors;
		link.l_stats.ls_tx_heartbeat_errors = st->tx_heartbeat_errors;
		link.l_stats.ls_tx_window_errors    = st->tx_window_errors;

		link.l_mask |= LINK_HAS_STATS;
	}

	if (tb[IFLA_TXQLEN]) {
		if ((err = NL_COPY_DATA(link.l_txqlen, tb[IFLA_TXQLEN])) < 0)
			return err;
		link.l_mask |= LINK_HAS_TXQLEN;
	}

	if (tb[IFLA_MTU]) {
		if ((err = NL_COPY_DATA(link.l_mtu, tb[IFLA_MTU])) < 0)
			return err;
		link.l_mask |= LINK_HAS_MTU;
	}

	if (tb[IFLA_ADDRESS]) {
		nl_copy_addr(&link.l_addr, tb[IFLA_ADDRESS]);
		link.l_addr.a_family = nl_addr_guess_family(&link.l_addr);
		link.l_mask |= LINK_HAS_ADDR;
	}

	if (tb[IFLA_BROADCAST]) {
		nl_copy_addr(&link.l_bcast, tb[IFLA_BROADCAST]);
		link.l_bcast.a_family = nl_addr_guess_family(&link.l_bcast);
		link.l_mask |= LINK_HAS_BRD;
	}

	if (tb[IFLA_LINK]) {
		if ((err = NL_COPY_DATA(link.l_link, tb[IFLA_LINK])) < 0)
			return err;
		link.l_mask |= LINK_HAS_LINK;
	}

	if (tb[IFLA_WEIGHT]) {
		if ((err = NL_COPY_DATA(link.l_weight, tb[IFLA_WEIGHT])) < 0)
			return err;
		link.l_mask |= LINK_HAS_WEIGHT;
	}

	if (tb[IFLA_QDISC]) {
		if (RTA_PAYLOAD(tb[IFLA_QDISC]) > (sizeof(link.l_qdisc) - 1))
			return nl_error(EINVAL, "Qdisc TLV exceeds local " \
			    "qdisc name length limit");

		memcpy(link.l_qdisc, RTA_DATA(tb[IFLA_QDISC]),
			RTA_PAYLOAD(tb[IFLA_QDISC]));

		link.l_qdisc[sizeof(link.l_qdisc) - 1] = '\0';
		link.l_mask |= LINK_HAS_QDISC;
	}

	if (tb[IFLA_MAP]) {
		struct rtnl_link_ifmap *map;

		if (RTA_PAYLOAD(tb[IFLA_MAP]) < sizeof(*map)) {
			return nl_error(EINVAL, "Interface ifmap TLV is too " \
			    "short");
		}

		map = (struct rtnl_link_ifmap *) RTA_DATA(tb[IFLA_MAP]);
		link.l_map.lim_mem_start = map->mem_start;
		link.l_map.lim_mem_end   = map->mem_end;
		link.l_map.lim_base_addr = map->base_addr;
		link.l_map.lim_irq       = map->irq;
		link.l_map.lim_dma       = map->dma;
		link.l_map.lim_port      = map->port;
		link.l_mask |= LINK_HAS_MAP;
	}

	if (tb[IFLA_MASTER]) {
		if ((err = NL_COPY_DATA(link.l_master, tb[IFLA_MASTER])) < 0)
			return err;
		link.l_mask |= LINK_HAS_MASTER;
	}

	err = pp->pp_cb((struct nl_common *) &link, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int link_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETLINK, AF_UNSPEC, NLM_F_DUMP);
}

static int link_dump_brief(struct nl_cache *c, struct nl_common *a, FILE *fd,
			   struct nl_dump_params *params)
{
	char llproto[32], flags[128];
	char addr[INET6_ADDRSTRLEN+5];
	struct rtnl_link *l = (struct rtnl_link *) a;

	dp_new_line(fd, params, 0);

	fprintf(fd, "%s ", l->l_name);

	if (l->l_mask & LINK_HAS_LINK) {
		struct rtnl_link *ll = rtnl_link_get(c, l->l_link);
		fprintf(fd, "@%s", ll ? ll->l_name : "NONE");
	}

	fprintf(fd, "%s %s ",
		nl_llproto2str_r(l->l_arptype, llproto, sizeof(llproto)),
		nl_addr2str_r(&(l->l_addr), addr, sizeof(addr)));

	fprintf(fd, "mtu %u ", l->l_mtu);

	if (l->l_mask & LINK_HAS_MASTER) {
		struct rtnl_link *ll = rtnl_link_get(c, l->l_master);
		fprintf(fd, "master %s ", ll ? ll->l_name : "inv");
	}

	rtnl_link_flags2str_r(l->l_flags & ~IFF_RUNNING, flags, sizeof(flags));
	if (flags[0])
		fprintf(fd, "<%s>", flags);

	fprintf(fd, "\n");

	return 1;
}

static int link_dump_full(struct nl_cache *c, struct nl_common *a, FILE *fd,
			  struct nl_dump_params *params)
{
	struct rtnl_link *l = (struct rtnl_link *) a;

	int line = link_dump_brief(c, a, fd, params);
	dp_new_line(fd, params, line++);

	fprintf(fd, "    txqlen %u weight %u ",
		l->l_txqlen, l->l_weight);

	if (l->l_mask & LINK_HAS_QDISC)
		fprintf(fd, "qdisc %s ", l->l_qdisc);

	if (l->l_mask & LINK_HAS_MAP && l->l_map.lim_irq)
		fprintf(fd, "irq %u ", l->l_map.lim_irq);

	if (l->l_mask & LINK_HAS_IFINDEX)
		fprintf(fd, "index %u ", l->l_index);

	if (l->l_mask & LINK_HAS_BRD) {
		char bcast[INET6_ADDRSTRLEN+5];
		fprintf(fd, "brd %s",
			nl_addr2str_r(&l->l_bcast, bcast, sizeof(bcast)));
	}

	fprintf(fd, "\n");

	return line;
}

static int link_dump_with_stats(struct nl_cache *c, struct nl_common *a,
				FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_link *l = (struct rtnl_link *) a;
	char *unit, fmt[64];
	float res;
	
	int line = link_dump_full(c, a, fd, params);

	dp_new_line(fd, params, line++);
	fprintf(fd, "    Stats:    bytes    packets     errors    dropped " \
		"  fifo-err compressed\n");

	res = nl_cancel_down_bytes(l->l_stats.ls_rx.bytes, &unit);

	strcpy(fmt, "    RX  %X.2f %s %10llu %10llu %10llu %10llu %10llu\n");
	fmt[9] = *unit == 'B' ? '9' : '7';
	
	dp_new_line(fd, params, line++);
	fprintf(fd, fmt,
		res, unit, l->l_stats.ls_rx.packets, l->l_stats.ls_rx.errors,
		l->l_stats.ls_rx.dropped, l->l_stats.ls_rx_fifo_errors,
		l->l_stats.ls_rx.compressed);

	res = nl_cancel_down_bytes(l->l_stats.ls_tx.bytes, &unit);

	strcpy(fmt, "    TX  %X.2f %s %10llu %10llu %10llu %10llu %10llu\n");
	fmt[9] = *unit == 'B' ? '9' : '7';
	
	dp_new_line(fd, params, line++);
	fprintf(fd, fmt,
		res, unit,
		l->l_stats.ls_tx.packets, l->l_stats.ls_tx.errors,
		l->l_stats.ls_tx.dropped, l->l_stats.ls_tx_fifo_errors,
		l->l_stats.ls_tx.compressed);

	dp_new_line(fd, params, line++);
	fprintf(fd, "    Errors:  length       over        crc      frame " \
		"    missed  multicast\n");

	dp_new_line(fd, params, line++);
	fprintf(fd, "    RX   %10" PRIu64 " %10" PRIu64 " %10" PRIu64 
		    " %10" PRIu64 " %10" PRIu64 " %10" PRIu64 "\n",
		l->l_stats.ls_rx_length_errors, l->l_stats.ls_rx_over_errors,
		l->l_stats.ls_rx_crc_errors, l->l_stats.ls_rx_frame_errors,
		l->l_stats.ls_rx_missed_errors, l->l_stats.ls_rx.multicast);

	dp_new_line(fd, params, line++);
	fprintf(fd, "    Errors: aborted    carrier  heartbeat     window " \
		" collision\n");
	
	dp_new_line(fd, params, line++);
	fprintf(fd, "    TX   %10" PRIu64 " %10" PRIu64 " %10" PRIu64 
		    " %10" PRIu64 " %10" PRIu64 "\n",
		l->l_stats.ls_tx_aborted_errors,
		l->l_stats.ls_tx_carrier_errors,
		l->l_stats.ls_tx_heartbeat_errors,
		l->l_stats.ls_tx_window_errors,
		l->l_stats.ls_tx_collisions);

	return line;
}

static int link_dump_events(struct nl_cache *c, struct nl_common *a,
			    FILE *fd, struct nl_dump_params *params)
{
	struct rtnl_link *l = (struct rtnl_link *) a;
	int line = 0;

	if (l->l_change == ~0U) {
		dp_new_line(fd, params, line++);
		fprintf(fd, "link %s register/unregister.\n", l->l_name);
		return line;
	}

	if (l->l_change & IFF_UP && l->l_change & IFF_RUNNING) {
		dp_new_line(fd, params, line++);
		fprintf(fd, "link %s changed state to %s.\n",
			l->l_name, l->l_flags & IFF_UP ? "up" : "down");
	}

	if (l->l_change & IFF_SLAVE) {
	}

	if (l->l_change & IFF_PROMISC) {
		dp_new_line(fd, params, line++);
		fprintf(fd, "link %s %s promiscuous mode.\n",
		    l->l_name, l->l_flags & IFF_PROMISC ? "entered" : "left");
	}

	return line;
}

static int link_filter(struct nl_common *obj, struct nl_common *filter)
{
	struct rtnl_link *o = (struct rtnl_link *) obj;
	struct rtnl_link *f = (struct rtnl_link *) filter;

	if (obj->ce_type != RTNL_LINK || filter->ce_type != RTNL_LINK)
		return 0;

#define REQ(F) (f->l_mask & LINK_HAS_##F)
#define AVAIL(F) (o->l_mask & LINK_HAS_##F)
	if ((REQ(MTU)     &&
	      (!AVAIL(MTU)     || o->l_mtu != f->l_mtu))		  ||
	    (REQ(LINK)    &&
	      (!AVAIL(LINK)    || o->l_link != f->l_link))		  ||
	    (REQ(TXQLEN)  &&
	      (!AVAIL(TXQLEN)  || o->l_txqlen != f->l_txqlen))		  ||
	    (REQ(WEIGHT)  &&
	      (!AVAIL(WEIGHT)  || o->l_weight != f->l_weight))		  ||
	    (REQ(MASTER)  &&
	      (!AVAIL(MASTER)  || o->l_master != f->l_master))		  ||
	    (REQ(IFINDEX) &&
	      (!AVAIL(IFINDEX) || o->l_index != f->l_index))		  ||
	    (REQ(QDISC)   &&
	      (!AVAIL(QDISC)   || strcmp(o->l_qdisc, f->l_qdisc)))	  ||
	    (REQ(IFNAME)  &&
	      (!AVAIL(IFNAME)  || strcmp(o->l_name, f->l_name)))	  ||
	    (REQ(ADDR)    &&
	      (!AVAIL(ADDR)    || nl_addrcmp(&o->l_addr, &f->l_addr)))	  ||
	    (REQ(BRD)     &&
	      (!AVAIL(BRD)     || nl_addrcmp(&o->l_bcast, &f->l_bcast)))  ||
	    (REQ(FAMILY)  &&
	      (!AVAIL(FAMILY)  || o->l_family != f->l_family))		  ||
	    (REQ(FLAGS)   &&
	      (!AVAIL(FLAGS)   || f->l_flags ^ (o->l_flags & f->l_flag_mask))))
		return 0;
#undef REQ
#undef AVAIL

	return 1;
}

/**
 * @name General API
 * @{
 */

/**
 * Build a link cache including all links currently configured in the kernel
 * @arg handle		netlink handle
 *
 * Allocates a new cache, initializes it properly and updates it to
 * include all links currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_link_build_cache(struct nl_handle *handle)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	cache->c_type = RTNL_LINK;
	cache->c_type_size = sizeof(struct rtnl_link);
	cache->c_ops = &rtnl_link_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Look up link by interface index in the provided cache
 * @arg cache		link cache
 * @arg ifindex		link interface index
 *
 * @return pointer to link inside the cache or NULL if no match was found.
 */
struct rtnl_link * rtnl_link_get(struct nl_cache *cache, int ifindex)
{
	int i;
	struct rtnl_link *l;

	if (cache->c_type != RTNL_LINK || NULL == cache->c_cache)
		return NULL;

	for (i = 0; i < cache->c_index; i++) {
		l = (struct rtnl_link *) NL_CACHE_ELEMENT_AT(cache, i);

		if (l && l->l_index == ifindex)
			return l;
	}

	return NULL;
}

/**
 * Look up link by link name in the provided cache
 * @arg cache		link cache
 * @arg name		link name
 *
 * @return pointer to link inside the cache or NULL if no match was found.
 */
struct rtnl_link * rtnl_link_get_by_name(struct nl_cache *cache,
					 const char *name)
{
	int i;
	struct rtnl_link *l;

	if (cache->c_type != RTNL_LINK || NULL == cache->c_cache)
		return NULL;

	for (i = 0; i < cache->c_index; i++) {
		l = (struct rtnl_link *) NL_CACHE_ELEMENT_AT(cache, i);

		if (l && !strcmp(name, l->l_name))
			return l;
	}

	return NULL;
}

/**
 * Dump link attributes
 * @arg link		link to be dumped
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_link_dump(struct rtnl_link *link, FILE *fd,
		    struct nl_dump_params *params)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (rtnl_link_ops.co_dump[type])
		rtnl_link_ops.co_dump[type](NULL, (struct nl_common *) link,
		    fd, params);
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
					       struct rtnl_link *tmpl)
{
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = RTM_SETLINK,
	};

	struct ifinfomsg ifi = {
		.ifi_family = old->l_family,
		.ifi_index = old->l_index,
	};

	if (tmpl->l_mask & LINK_HAS_FLAGS) {
		ifi.ifi_flags = old->l_flags & ~tmpl->l_flag_mask;
		ifi.ifi_flags |= tmpl->l_flags;
	}

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &ifi, sizeof(ifi));

	if (tmpl->l_mask & LINK_HAS_ADDR)
		nl_msg_append_tlv(m, IFLA_ADDRESS, &tmpl->l_addr,
		    tmpl->l_addr.a_len);

	if (tmpl->l_mask & LINK_HAS_BRD)
		nl_msg_append_tlv(m, IFLA_BROADCAST, &tmpl->l_bcast,
		    tmpl->l_bcast.a_len);

	if (tmpl->l_mask & LINK_HAS_MTU)
		nl_msg_append_tlv(m, IFLA_MTU, &tmpl->l_mtu,
		    sizeof(tmpl->l_mtu));

	if (tmpl->l_mask & LINK_HAS_TXQLEN)
		nl_msg_append_tlv(m, IFLA_TXQLEN, &tmpl->l_txqlen,
		    sizeof(tmpl->l_txqlen));

	if (tmpl->l_mask & LINK_HAS_WEIGHT)
		nl_msg_append_tlv(m, IFLA_WEIGHT, &tmpl->l_weight,
		    sizeof(tmpl->l_weight));

	if (tmpl->l_mask & LINK_HAS_IFNAME)
		nl_msg_append_tlv(m, IFLA_IFNAME, (void *) tmpl->l_name,
		    strlen(tmpl->l_name)+1);

	return m;
}

/**
 * Change link attributes
 * @arg handle		netlink handle
 * @arg old		link to be changed
 * @arg tmpl		template with requested changes
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
		     struct rtnl_link *tmpl)
{
	int err;
	struct nl_msg *m = rtnl_link_build_change_request(old, tmpl);
	
	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
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
 *
 * @return link name or NULL if no match was found.
 */
const char * rtnl_link_i2name(struct nl_cache *cache, int ifindex)
{
	struct rtnl_link *l = rtnl_link_get(cache, ifindex);
	return l ? l->l_name : NULL;
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
	struct rtnl_link *l = rtnl_link_get_by_name(cache, name);
	return l ? l->l_index : RTNL_LINK_NOT_FOUND;
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
};

/**
 * Convert link flags to a character string.
 * @arg flags		link flags
 *
 * Converts link flags to a character string separated by
 * commas and stores it in a static buffer.
 *
 * @return A static buffer
 * @attention This funnction is NOT thread safe.
 */
char * rtnl_link_flags2str(int flags)
{
	static char buf[128];
	memset(buf, 0, sizeof(buf));
	return __flags2str_r(flags, buf, sizeof(buf), link_flags, ARRAY_SIZE(link_flags));
}

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
char * rtnl_link_flags2str_r(int flags, char *buf, size_t len)
{
	return __flags2str_r(flags, buf, len, link_flags, ARRAY_SIZE(link_flags));
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
	return __str2type(name, link_flags, ARRAY_SIZE(link_flags));
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set QDisc
 * @arg link		link to change
 * @arg qdisc		qdisc name
 */
void rtnl_link_set_qdisc(struct rtnl_link *link, const char *qdisc)
{
	strncpy(link->l_qdisc, qdisc, sizeof(link->l_qdisc) - 1);
	link->l_mask |= LINK_HAS_QDISC;
}

/**
 * Set new link name
 * @arg link		link to change
 * @arg name		new link name
 */
void rtnl_link_set_name(struct rtnl_link *link, const char *name)
{
	strncpy(link->l_name, name, sizeof(link->l_name) - 1);
	link->l_mask |= LINK_HAS_IFNAME;
}

/**
 * Set link layer address
 * @arg link		link to change
 * @arg addr		new link layer address
 */
void rtnl_link_set_addr(struct rtnl_link *link, struct nl_addr *addr)
{
	memcpy(&link->l_addr, addr, sizeof(link->l_addr));
	link->l_mask |= LINK_HAS_ADDR;
}

/**
 * Set link layer address
 * @arg link		link to change
 * @arg addr		new link layer address as string
 *
 * Translates the specified address to a binary format and assigns
 * it as the new link layer address by calling rtnl_link_set_addr().
 *
 * @see rtnl_link_set_addr()
 * @return 0 on success or a negative error code.
 */
int rtnl_link_set_addr_str(struct rtnl_link *link, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	int hint = link->l_mask & LINK_HAS_FAMILY ? link->l_family : AF_UNSPEC;
	
	err = nl_str2addr(addr, &a, hint);
	if (err < 0)
		return err;

	rtnl_link_set_addr(link, &a);
	return 0;
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
	memcpy(&link->l_bcast, brd, sizeof(link->l_bcast));
	link->l_mask |= (LINK_HAS_BRD);
}

/**
 * Set link layer broadcast address
 * @arg link		link to change
 * @arg brd		new link layer broadcast address a string
 *
 * Translates the specified address to a binary format and assigns it
 * as the new broadcast address by calling rtnl_link_set_broadcast().
 *
 * @see rtnl_link_set_broadcast()
 * @return 0 on success or a negative error code.
 */
int rtnl_link_set_broadcast_str(struct rtnl_link *link, const char *brd)
{
	int err;
	struct nl_addr a = {0};
	int hint = link->l_mask & LINK_HAS_FAMILY ? link->l_family : AF_UNSPEC;
	
	err = nl_str2addr(brd, &a, hint);
	if (err < 0)
		return err;

	rtnl_link_set_broadcast(link, &a);
	return 0;
}

/**
 * Set flags
 * @arg link		link to change
 * @arg flag		flags to set (see \ref link_flags "Link Flags")
 */
void rtnl_link_set_flags(struct rtnl_link *link, int flag)
{
	link->l_flag_mask |= flag;
	link->l_flags |= flag;
	link->l_mask |= LINK_HAS_FLAGS;
}

/**
 * Unset flags
 * @arg link		link to change
 * @arg flag		flags to unset (see \ref link_flags "Link Flags")
 */
void rtnl_link_unset_flags(struct rtnl_link *link, int flag)
{
	link->l_flag_mask |= flag;
	link->l_flags &= ~flag;
	link->l_mask |= LINK_HAS_FLAGS;
}

/**
 * Set link layer address family
 * @arg link		link to change
 * @arg family		new address family
 */
void rtnl_link_set_family(struct rtnl_link *link, int family)
{
	link->l_family = family;
	link->l_mask |= LINK_HAS_FAMILY;
}

/**
 * Set interface index
 * @arg link		link to change
 * @arg ifindex		new interface index
 */
void rtnl_link_set_ifindex(struct rtnl_link *link, int ifindex)
{
	link->l_index = ifindex;
	link->l_mask |= LINK_HAS_IFINDEX;
}

/**
 * Set interface index via a link name
 * @arg link		link to change
 * @arg cache		link cache holding all links
 * @arg name		interace name
 * @return 0 on success or a negative error code.
 */
int rtnl_link_set_ifindex_name(struct rtnl_link *link,
			       struct nl_cache *cache, const char *name)
{
	int i = rtnl_link_name2i(cache, name);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", name);
	rtnl_link_set_ifindex(link, i);
	return 0;
}

/**
 * Set  Maximum Transmission Unit
 * @arg link		link to change
 * @arg mtu		new MTU
 */
void rtnl_link_set_mtu(struct rtnl_link *link, uint32_t mtu)
{
	link->l_mtu = mtu;
	link->l_mask |= LINK_HAS_MTU;
}

/**
 * Set Transmission Queue Length
 * @arg link		link to change
 * @arg txqlen		new TX queue length
 * @note The unit of the transmission queue length depends on the
 *       link type, a common unit is \a packets.
 */
void rtnl_link_set_txqlen(struct rtnl_link *link, uint32_t txqlen)
{
	link->l_txqlen = txqlen;
	link->l_mask |= LINK_HAS_TXQLEN;
}

/**
 * Set Weight
 * @arg link		link to change
 * @arg weight		new weight
 */
void rtnl_link_set_weight(struct rtnl_link *link, uint32_t weight)
{
	link->l_weight = weight;
	link->l_mask |= LINK_HAS_WEIGHT;
}

/**
 * Set parent interface index
 * @arg link		link to change
 * @arg ifindex		new parent's interface index
 */
void rtnl_link_set_link(struct rtnl_link *link, uint32_t ifindex)
{
	link->l_link = ifindex;
	link->l_mask |= LINK_HAS_LINK;
}

/**
 * Set parent interface index via a link name
 * @arg link		link to change
 * @arg cache		link cache holding all links
 * @arg name		link name
 * @return 0 on success or a negative error code.
 */
int rtnl_link_set_link_name(struct rtnl_link *link, struct nl_cache *cache,
			    const char *name)
{
	int i = rtnl_link_name2i(cache, name);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", name);

	rtnl_link_set_link(link, i);
	return 0;
}

/**
 * Set master interface index
 * @arg link		link to change
 * @arg ifindex		new master's interface index
 */
void rtnl_link_set_master(struct rtnl_link *link, uint32_t ifindex)
{
	link->l_master = ifindex;
	link->l_mask |= LINK_HAS_MASTER;
}

/**
 * Set master interface index via a link name
 * @arg link		link to change
 * @arg cache		link cache holding all links
 * @arg name		link name
 * @return 0 on success or a negative error code.
 */
int rtnl_link_set_master_name(struct rtnl_link *link, struct nl_cache *cache,
			      const char *name)
{
	int i = rtnl_link_name2i(cache, name);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", name);

	rtnl_link_set_master(link, i);
	return 0;
}

/** @} */

struct nl_cache_ops rtnl_link_ops = {
	.co_request_update	= link_request_update,
	.co_free_data		= NULL,
	.co_msg_parser		= link_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= link_dump_brief,
	.co_dump[NL_DUMP_FULL]	= link_dump_full,
	.co_dump[NL_DUMP_STATS]	= link_dump_with_stats,
	.co_dump[NL_DUMP_EVENTS] = link_dump_events,
	.co_filter		= link_filter,
};

/** @} */
