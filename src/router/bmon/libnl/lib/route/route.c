/*
 * route.c             rtnetlink routing layer
 *
 * Copyright (c) 2003-2004 Thomas Graf <tgraf@suug.ch>
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
 * @defgroup route Routing
 * @{
 */


#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/helpers.h>
#include <netlink/data.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>
#include <netlink/route/rtattr.h>

static void route_free_data(struct nl_common *c)
{
	struct rtnl_route *r = (struct rtnl_route *) c;

	if (r == NULL)
		return;

	nl_free_data(&r->rt_session);
	nl_free_data(&r->rt_protoinfo);

	while (r->rt_nexthops) {
		struct rtnl_nexthop *nh = r->rt_nexthops;
		r->rt_nexthops = nh->rtnh_next;
		free(nh);
	}
}


static int route_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n, void *arg)
{
	struct nl_parser_param *pp = arg;
	struct rtnl_route route = RTNL_INIT_ROUTE();
	struct rtattr *tb[RTA_MAX + 1];
	struct rtmsg *r = NLMSG_DATA(n);
	size_t len;
	int err;

	if (n->nlmsg_type != RTM_NEWROUTE)
		return P_IGNORE;

	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(*r));

	if (len < 0)
		return nl_error(EINVAL, "netlink message too short to be a " \
			"routing message");

	err = nl_parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
	if (err < 0)
		return err;

	route.rt_family = r->rtm_family;
	route.rt_dst_len = r->rtm_dst_len;
	route.rt_src_len = r->rtm_src_len;
	route.rt_tos = r->rtm_tos;
	route.rt_table = r->rtm_table;
	route.rt_protocol = r->rtm_protocol;
	route.rt_scope = r->rtm_scope;
	route.rt_type = r->rtm_type;
	route.rt_flags = r->rtm_flags;

	route.rt_mask = (ROUTE_HAS_FAMILY|ROUTE_HAS_DST_LEN|ROUTE_HAS_SRC_LEN|
		ROUTE_HAS_TOS|ROUTE_HAS_TABLE|ROUTE_HAS_PROTOCOL|ROUTE_HAS_SCOPE|
		ROUTE_HAS_TYPE|ROUTE_HAS_FLAGS);

	if (tb[RTA_DST]) {
		nl_copy_addr(&route.rt_dst, tb[RTA_DST]);
		route.rt_dst.a_prefix = route.rt_dst_len;
		route.rt_dst.a_family = route.rt_family;
		route.rt_mask |= ROUTE_HAS_DST;
	}

	if (tb[RTA_SRC]) {
		nl_copy_addr(&route.rt_src, tb[RTA_SRC]);
		route.rt_src.a_prefix = route.rt_src_len;
		route.rt_src.a_family = route.rt_family;
		route.rt_mask |= ROUTE_HAS_SRC;
	}

	if (tb[RTA_IIF]) {
		err = NL_COPY_DATA(route.rt_iif, tb[RTA_IIF]);
		if (err < 0)
			return err;
		route.rt_mask |= ROUTE_HAS_IIF;
	}

	if (tb[RTA_OIF]) {
		err = NL_COPY_DATA(route.rt_oif, tb[RTA_OIF]);
		if (err < 0)
			return err;
		route.rt_mask |= ROUTE_HAS_OIF;
	}

	if (tb[RTA_GATEWAY]) {
		nl_copy_addr(&route.rt_gateway, tb[RTA_GATEWAY]);
		route.rt_gateway.a_family = route.rt_family;
		route.rt_mask |= ROUTE_HAS_GATEWAY;
	}

	if (tb[RTA_PRIORITY]) {
		err = NL_COPY_DATA(route.rt_prio, tb[RTA_PRIORITY]);
		if (err < 0)
			return err;
		route.rt_mask |= ROUTE_HAS_PRIO;
	}

	if (tb[RTA_PREFSRC]) {
		nl_copy_addr(&route.rt_pref_src, tb[RTA_PREFSRC]);
		route.rt_pref_src.a_family = route.rt_family;
		route.rt_mask |= ROUTE_HAS_PREF_SRC;
	}

	if (tb[RTA_METRICS]) {
		int i;
		struct rtattr *mtb[RTAX_MAX + 1];

		err = nl_parse_nested(mtb, RTAX_MAX, tb[RTA_METRICS]);
		if (err < 0)
			return err;

		for (i = 1; i <= RTAX_MAX; i++) {
			if (mtb[i]) {
				uint32_t m = *(uint32_t *) RTA_DATA(mtb[i]);
				route.rt_metrics[i] = m;
				route.rt_metrics_mask |= (1<<(i-1));
			}
		}
		
		route.rt_mask |= ROUTE_HAS_METRICS;
	}

	if (tb[RTA_MULTIPATH]) {
		struct rtnl_nexthop *nh;
		struct rtnexthop *rtnh = RTA_DATA(tb[RTA_MULTIPATH]);
		size_t tlen = RTA_PAYLOAD(tb[RTA_MULTIPATH]);

		for (;;) {
			if (tlen < sizeof(*rtnh) || tlen < rtnh->rtnh_len)
				break;

			nh = calloc(1, sizeof(*nh));
			if (nh == NULL) {
				err = -ENOMEM;
				goto err_out;
			}

			nh->rtnh_flags = rtnh->rtnh_flags;
			nh->rtnh_hops = rtnh->rtnh_hops;
			nh->rtnh_ifindex = rtnh->rtnh_ifindex;
			nh->rtnh_mask = (NEXTHOP_HAS_FLAGS | NEXTHOP_HAS_HOPS
					| NEXTHOP_HAS_IFINDEX);
				
			if (rtnh->rtnh_len > sizeof(*rtnh)) {
				struct rtattr *ntb[RTA_MAX + 1];
				nl_parse_rtattr(ntb, RTA_MAX, RTNH_DATA(rtnh),
					rtnh->rtnh_len - sizeof(*rtnh));

				if (ntb[RTA_GATEWAY]) {
					nl_copy_addr(&nh->rtnh_gateway, ntb[RTA_GATEWAY]);
					nh->rtnh_gateway.a_family = route.rt_family;
					nh->rtnh_mask = NEXTHOP_HAS_GATEWAY;
				}
			}

			nh->rtnh_next = route.rt_nexthops;
			route.rt_nexthops = nh;

			len -= RTNH_ALIGN(rtnh->rtnh_len);
			rtnh = RTNH_NEXT(rtnh);
		}

		route.rt_mask |= ROUTE_HAS_MULTIPATH;
	}

	/* Not sure if there are any users not using this for fwmark,
	 * allocating for now */
	if (tb[RTA_PROTOINFO]) {
		err = nl_alloc_data_from_rtattr(&route.rt_protoinfo, tb[RTA_PROTOINFO]);
		if (err < 0)
			goto err_out;
		route.rt_mask |= ROUTE_HAS_PROTOINFO;
	}

	if (tb[RTA_FLOW]) {
		err = NL_COPY_DATA(route.rt_realm, tb[RTA_FLOW]);
		if (err < 0)
			goto err_out;
		route.rt_mask |= ROUTE_HAS_REALM;
	}

	if (tb[RTA_CACHEINFO]) {
		struct rta_cacheinfo *ci;
		
		if (RTA_PAYLOAD(tb[RTA_CACHEINFO]) < sizeof(struct rta_cacheinfo))
			return nl_error(EINVAL, "routing cacheinfo TLV is too short");

		ci = (struct rta_cacheinfo *) RTA_DATA(tb[RTA_CACHEINFO]);
		route.rt_cacheinfo.rtci_clntref  = ci->rta_clntref;
		route.rt_cacheinfo.rtci_last_use = ci->rta_lastuse;
		route.rt_cacheinfo.rtci_expires  = ci->rta_expires;
		route.rt_cacheinfo.rtci_error    = ci->rta_error;
		route.rt_cacheinfo.rtci_used     = ci->rta_used;
		route.rt_cacheinfo.rtci_id       = ci->rta_id;
		route.rt_cacheinfo.rtci_ts       = ci->rta_ts;
		route.rt_cacheinfo.rtci_tsage    = ci->rta_tsage;

		route.rt_mask |= ROUTE_HAS_CACHEINFO;
	}

	if (tb[RTA_SESSION]) {
		err = nl_alloc_data_from_rtattr(&route.rt_session, tb[RTA_SESSION]);
		if (err < 0)
			goto err_out;
		route.rt_mask |= ROUTE_HAS_SESSION;
	}

	err = pp->pp_cb((struct nl_common *) &route, pp);
	if (err < 0)
		goto err_out;

	return P_ACCEPT;

err_out:

	route_free_data((struct nl_common *) &route);
	return err;
}

static int route_request_update(struct nl_cache *c, struct nl_handle *h)
{
	struct rtmsg rmsg = { .rtm_family = AF_UNSPEC };
	return nl_request_with_data(h, RTM_GETROUTE, NLM_F_DUMP,
				    (unsigned char *) &rmsg, sizeof(rmsg));
}

static int route_dump_brief(struct nl_cache *c, struct nl_common *a, FILE *fd,
			    struct nl_dump_params *params)
{
	struct rtnl_route *r = (struct rtnl_route *) a;

	dp_new_line(fd, params, 0);

	if (r->rt_mask & ROUTE_HAS_DST) {
		char dst[INET6_ADDRSTRLEN+5];
		fprintf(fd, "%s ", nl_addr2str_r(&r->rt_dst, dst, sizeof(dst)));
	} else if (r->rt_dst_len)
		fprintf(fd, "0/%u ", r->rt_dst_len);
	else
		fprintf(fd, "default ");

	if (r->rt_mask & ROUTE_HAS_GATEWAY) {
		char via[INET6_ADDRSTRLEN+5];
		fprintf(fd, "via %s ", nl_addr2str_r(&r->rt_gateway, via, sizeof(via)));
	}

	if (r->rt_mask & ROUTE_HAS_OIF) {
		const char *dev = rtnl_link_i2name(nl_cache_lookup(RTNL_LINK), r->rt_oif);
		fprintf(fd, "dev %s ", dev);
	}

	if (r->rt_table != RT_TABLE_MAIN)
		fprintf(fd, "table %u ", r->rt_table);

	if (r->rt_mask & ROUTE_HAS_FLAGS && r->rt_flags) {
		int flags = r->rt_flags;

		fprintf(fd, "<");
		
#define PRINT_FLAG(f) if (flags & RTNH_F_##f) { \
		flags &= ~RTNH_F_##f; fprintf(fd, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(DEAD);
		PRINT_FLAG(ONLINK);
		PRINT_FLAG(PERVASIVE);
#undef PRINT_FLAG

#define PRINT_FLAG(f) if (flags & RTM_F_##f) { \
		flags &= ~RTM_F_##f; fprintf(fd, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(NOTIFY);
		PRINT_FLAG(CLONED);
		PRINT_FLAG(EQUALIZE);
		PRINT_FLAG(PREFIX);
#undef PRINT_FLAG

		fprintf(fd, ">");
	}

	fprintf(fd, "\n");

	return 1;
}

static int route_dump_full(struct nl_cache *c, struct nl_common *a, FILE *fd,
			   struct nl_dump_params *params)
{
	struct rtnl_route *r = (struct rtnl_route *) a;

	int line = route_dump_brief(c, a, fd, params);

	dp_new_line(fd, params, line++);

	if (r->rt_mask & ROUTE_HAS_PRIO)
		fprintf(fd, "metric %u ", r->rt_prio);

	if (r->rt_mask & ROUTE_HAS_PREF_SRC) {
		char p[INET6_ADDRSTRLEN+5];
		fprintf(fd, "pref %s ", nl_addr2str_r(&r->rt_pref_src, p,
						      sizeof(p)));
	}

	fprintf(fd, "\n");

	return line;
}

static int route_dump_with_stats(struct nl_cache *c, struct nl_common *a,
				 FILE *fd, struct nl_dump_params *params)
{
	return route_dump_full(c, a, fd, params);
}

static int route_filter(struct nl_common *obj, struct nl_common *filter)
{
	struct rtnl_route *o = (struct rtnl_route *) obj;
	struct rtnl_route *f = (struct rtnl_route *) filter;

	if (obj->ce_type != RTNL_ROUTE || filter->ce_type != RTNL_ROUTE)
		return 0;

#define REQ(F) (f->rt_mask & ROUTE_HAS_##F)
#define AVAIL(F) (o->rt_mask & ROUTE_HAS_##F)
	if ((REQ(FAMILY)   &&
	      (!AVAIL(FAMILY)   || o->rt_family != f->rt_family))	  ||
	    (REQ(DST_LEN)  &&
	      (!AVAIL(DST_LEN)  || o->rt_dst_len != f->rt_dst_len))	  ||
	    (REQ(SRC_LEN)  &&
	      (!AVAIL(SRC_LEN)  || o->rt_src_len != f->rt_src_len))	  ||
	    (REQ(TOS)      &&
	      (!AVAIL(TOS)      || o->rt_tos != f->rt_tos))		  ||
	    (REQ(TABLE)    &&
	      (!AVAIL(TABLE)    || o->rt_table != f->rt_table))		  ||
	    (REQ(PROTOCOL) &&
	      (!AVAIL(PROTOCOL) || o->rt_protocol != f->rt_protocol))	  ||
	    (REQ(SCOPE)    &&
	      (!AVAIL(SCOPE)    || o->rt_scope != f->rt_scope))		  ||
	    (REQ(TYPE)     &&
	      (!AVAIL(TYPE)     || o->rt_type != f->rt_type))		  ||
	    (REQ(DST)      &&
	      (!AVAIL(DST)      || nl_addrcmp(&o->rt_dst, &f->rt_dst)))	  ||
	    (REQ(SRC)      &&
	      (!AVAIL(SRC)      || nl_addrcmp(&o->rt_src, &f->rt_src)))	  ||
	    (REQ(OIF)      &&
	      (!AVAIL(OIF)      || o->rt_oif != f->rt_oif))		  ||
	    (REQ(PRIO)     &&
	      (!AVAIL(PRIO)     || o->rt_prio != f->rt_prio))		  ||
	    (REQ(REALM)    &&
	      (!AVAIL(REALM)    || o->rt_realm != f->rt_realm))		  ||
	    (REQ(IIF)      &&
	      (!AVAIL(IIF)      || strcmp(o->rt_iif, f->rt_iif)))	  ||
	    (REQ(PREF_SRC) &&
	      (!AVAIL(PREF_SRC) || nl_addrcmp(&o->rt_pref_src,
					      &f->rt_pref_src)))	  ||
	    (REQ(GATEWAY)  &&
	      (!AVAIL(GATEWAY)  || nl_addrcmp(&o->rt_gateway,
					      &f->rt_gateway)))		  ||
	    (REQ(FLAGS)    &&
	      (!AVAIL(FLAGS)    || f->rt_flags ^
					(o->rt_flags & f->rt_flag_mask))))
		return 0;

	if (REQ(METRICS)) {
		int i;

		if (!AVAIL(METRICS))
			return 0;

		for (i = 0; i < RTAX_MAX; i++) {
			if (f->rt_metrics_mask & (1 << i)) {
				if (!(o->rt_metrics_mask & (1 << i)) ||
				    f->rt_metrics[i+1] != o->rt_metrics[i+1])
					return 0;
			}
		}
	}
	
#undef REQ
#undef AVAIL

#if 0
#define ROUTE_HAS_MULTIPATH 0x020000
#define ROUTE_HAS_CACHEINFO 0x080000
#define ROUTE_HAS_SESSION   0x100000
#define ROUTE_HAS_PROTOINFO 0x200000

	struct rtnl_nexthop *	rt_nexthops;
	struct rtnl_rtcacheinfo	rt_cacheinfo;
	struct nl_data		rt_session;
	struct nl_data		rt_protoinfo;
#endif

	return 1;

}

/**
 * @name General API
 * @{
 */

/**
 * Build a route cache including all rout currently configured in the kernel
 * @arg handle		netlink handle
 *
 * Allocates a new cache, initializes it properly and updates it to
 * include all routes currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache * rtnl_route_build_cache(struct nl_handle *handle)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	cache->c_type = RTNL_ROUTE;
	cache->c_type_size = sizeof(struct rtnl_route);
	cache->c_ops = &rtnl_route_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Dump route attributes
 * @arg route		route to be dumped
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_route_dump(struct rtnl_route *route, FILE *fd,
		     struct nl_dump_params *params)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (rtnl_route_ops.co_dump[type])
		rtnl_route_ops.co_dump[type](NULL, (struct nl_common *) route,
		    fd, params);
}

/** @} */

/**
 * \name Scope Translations
 * @{
 */

static struct trans_tbl scopes[] = {
	__ADD(255,nowhere)
	__ADD(254,host)
	__ADD(253,link)
	__ADD(200,site)
	__ADD(0,global)
};

/**
 * Convert a scope ID to a character string.
 * @arg scope		scope id
 *
 * Converts a scope ID o a character string and stores it in a
 * static buffer.
 *
 * \return A static buffer or the type encoded in hexidecimal
 *         form if no match was found.
 * \attention This funnction is NOT thread safe.
 */
char * rtnl_scope2str(int scope)
{
	static char buf[32];
	memset(buf, 0, sizeof(buf));
	return __type2str_r(scope, buf, sizeof(buf), scopes, ARRAY_SIZE(scopes));
}

/**
 * Convert a scope ID to a character string (Reentrant).
 * @arg scope		scope id
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a scope ID to a character string and stores it in
 * the specified destination buffer.
 *
 * \return The destination buffer or the type encoded in hexidecimal
 *         form if no match was found.
 */
char * rtnl_scope2str_r(int scope, char *buf, size_t len)
{
	return __type2str_r(scope, buf, len, scopes, ARRAY_SIZE(scopes));
}

/**
 * Convert a character string to a scope id.
 * @arg name		Name of cscope
 *
 * Converts the provided character string specifying a scope to
 * the corresponding numeric value.
 *
 * \return Scope ID or a negative value if none was found.
 */
int rtnl_str2scope(const char *name)
{
	return __str2type(name, scopes, ARRAY_SIZE(scopes));
}

/** \} */

/**
 * @name Attribute Modifications
 * @{
 */

/**
 * Set the table of a route to the specified value
 * @arg route		route to be changed
 * @arg table		new table value
 */
void rtnl_route_set_table(struct rtnl_route *route, int table)
{
	route->rt_table = table;
	route->rt_mask |= ROUTE_HAS_TABLE;
}

/**
 * Set the cope of a route to the specified value
 * @arg route		route to be changed
 * @arg scope		new scope
 */
void rtnl_route_set_scope(struct rtnl_route *route, int scope)
{
	route->rt_scope = scope;
	route->rt_mask |= ROUTE_HAS_SCOPE;
}

/**
 * Set the TOS of a route to the specified value
 * @arg route		route to be changed
 * @arg tos		new TOS value
 */
void rtnl_route_set_tos(struct rtnl_route *route, int tos)
{
	route->rt_tos = tos;
	route->rt_mask |= ROUTE_HAS_TOS;
}

/**
 * Set the realm of a route to the specified value
 * @arg route		route to be changed
 * @arg realm		new realm
 */
void rtnl_route_set_realm(struct rtnl_route *route, int realm)
{
	route->rt_realm = realm;
	route->rt_mask |= ROUTE_HAS_REALM;
}

/**
 * Set the protocol of a route to the specified value
 * @arg route		route to be changed
 * @arg proto		new protocol
 */
void rtnl_route_set_protocol(struct rtnl_route *route, int proto)
{
	route->rt_protocol = proto;
	route->rt_mask |= ROUTE_HAS_PROTOCOL;
}

/**
 * Set the priority of a route to the specified value
 * @arg route		route to be changed
 * @arg prio		new priority
 */
void rtnl_route_set_prio(struct rtnl_route *route, int prio)
{
	route->rt_prio = prio;
	route->rt_mask |= ROUTE_HAS_PRIO;
}

/**
 * Set the address family of a route to the specified value
 * @arg route		route to be changed
 * @arg family		new address family
 */
void rtnl_route_set_family(struct rtnl_route *route, int family)
{
	route->rt_family = family;
	route->rt_mask |= ROUTE_HAS_FAMILY;
}

/**
 * Set the destination address prefix of a route to the specified value
 * @arg route		route to be changed
 * @arg prefix		new destination address prefix
 * @attention The destination address prefix gets overwritten by calls
 *            to rtnl_route_set_dst() rtnl_route_set_dst_str().
 */
void rtnl_route_set_dst_len(struct rtnl_route *route, int prefix)
{
	route->rt_dst_len = prefix;
	route->rt_mask |= ROUTE_HAS_DST_LEN;
}

/**
 * Set the source address prefix of a route to the specified value
 * @arg route		route to be changed
 * @arg prefix		new source address prefix
 * @attention The source address prefix gets overwritten by calls
 *            to rtnl_route_src_dst() rtnl_route_set_src_str().
 */
void rtnl_route_set_src_len(struct rtnl_route *route, int prefix)
{
	route->rt_dst_len = prefix;
	route->rt_mask |= ROUTE_HAS_SRC_LEN;
}

/**
 * Set the type of a route to the specified value
 * @arg route		route to be changed
 * @arg type		new route type
 */
void rtnl_route_set_type(struct rtnl_route *route, int type)
{
	route->rt_type = type;
	route->rt_mask |= ROUTE_HAS_TYPE;
}

/**
 * Add flags to a route 
 * @arg route		route to be changed
 * @arg flags		flags to set
 */
void rtnl_route_set_flags(struct rtnl_route *route, int flags)
{
	route->rt_flag_mask |= flags;
	route->rt_flags |= flags;
	route->rt_mask |= ROUTE_HAS_FLAGS;
}

/**
 * Remove flags from a route 
 * @arg route		route to be changed
 * @arg flags		flags to unset
 */
void rtnl_route_unset_flags(struct rtnl_route *route, int flags)
{
	route->rt_flag_mask |= flags;
	route->rt_flags &= ~flags;
	route->rt_mask |= ROUTE_HAS_FLAGS;
}

/**
 * Set a metric of a route to the specified value
 * @arg route		route to be changed
 * @arg metric		metric to be changed (see XXX)
 * @arg value		new metric value
 * @return 0 on sucess or a negative error code
 */
int rtnl_route_set_metric(struct rtnl_route *route, int metric, uint32_t value)
{
	if (metric <= RTAX_MAX || metric < 1)
		return nl_error(EINVAL, "Metric out of range (1..%d)",
		    RTAX_MAX);

	route->rt_metrics[metric - 1] = value;
	route->rt_metrics_mask |= (1 << (metric - 1));

	return 0;
}

/**
 * Unset a metric of a route
 * @arg route		route to be changed
 * @arg metric		metric to be unset (see XXX)
 * @return 0 on sucess or a negative error code
 */
int rtnl_route_unset_metric(struct rtnl_route *route, int metric)
{
	if (metric <= RTAX_MAX || metric < 1)
		return nl_error(EINVAL, "Metric out of range (1..%d)",
		    RTAX_MAX);

	route->rt_metrics_mask &= ~(1 << (metric - 1));

	return 0;
}

/**
 * Set the destination address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new destination address
 *
 * Assigns the new destination address to the specified \a route,
 * overwrites the destination address length (rtnl_route::rt_dst_len),
 * and sets the route's address family to the new address's family if
 * it is not set already.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_dst(struct rtnl_route *route, struct nl_addr *addr)
{
	if (route->rt_mask & ROUTE_HAS_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;
		
	route->rt_dst_len = addr->a_prefix;
	memcpy(&route->rt_dst, addr, sizeof(route->rt_dst));
	route->rt_mask |= (ROUTE_HAS_DST | ROUTE_HAS_FAMILY | ROUTE_HAS_DST_LEN);

	return 0;
}

/**
 * Set the destination address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new destination address
 *
 * Translates the specified \a addr to a binary format and assigns
 * it as the new destination address. The destination address length
 * (rtnl_route::rt_dst_len) and the address family is automatically
 * set as well.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_dst_str(struct rtnl_route *route, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	int hint = route->rt_mask & ROUTE_HAS_FAMILY ? route->rt_family : AF_UNSPEC;
	
	err = nl_str2addr(addr, &a, hint);
	if (err < 0)
		return err;

	return rtnl_route_set_dst(route, &a);
}

/**
 * Set the source address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new source address
 *
 * Assigns the new source address to the specified \a route,
 * overwrites the source address length (rtnl_route::rt_src_len),
 * and sets the route's address family to the new address's family if
 * it is not set already.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_src(struct rtnl_route *route, struct nl_addr *addr)
{
	if (route->rt_mask & ROUTE_HAS_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;
		
	route->rt_src_len = addr->a_prefix;
	memcpy(&route->rt_src, addr, sizeof(route->rt_src));
	route->rt_mask |= (ROUTE_HAS_SRC | ROUTE_HAS_FAMILY | ROUTE_HAS_SRC_LEN);

	return 0;
}

/**
 * Set the source address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new source address
 *
 * Translates the specified \a addr to a binary format and assigns
 * it as the new source address. The source address length
 * (rtnl_route::rt_src_len) and the address family is automatically
 * set as well.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_src_str(struct rtnl_route *route, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	int hint = route->rt_mask & ROUTE_HAS_FAMILY ? route->rt_family : AF_UNSPEC;
	
	err = nl_str2addr(addr, &a, hint);
	if (err < 0)
		return err;

	return rtnl_route_set_src(route, &a);
}

/**
 * Set the gateway address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new gateway address
 *
 * Assigns the new gateway address to the specified \a route,
 * and sets the route's address family to the new address's family if
 * it is not set already.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_gateway(struct rtnl_route *route, struct nl_addr *addr)
{
	if (route->rt_mask & ROUTE_HAS_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;
		
	memcpy(&route->rt_gateway, addr, sizeof(route->rt_gateway));
	route->rt_mask |= (ROUTE_HAS_GATEWAY | ROUTE_HAS_FAMILY);

	return 0;
}

/**
 * Set the gateway address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new gateway address
 *
 * Translates the specified \a addr to a binary format and assigns
 * it as the new gateway address.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_gateway_str(struct rtnl_route *route, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	int hint = route->rt_mask & ROUTE_HAS_FAMILY ? route->rt_family : AF_UNSPEC;
	
	err = nl_str2addr(addr, &a, hint);
	if (err < 0)
		return err;

	return rtnl_route_set_gateway(route, &a);
}

/**
 * Set the preferred source address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new preferred source address
 *
 * Assigns the new preferred source address to the specified \a route,
 * and sets the route's address family to the new address's family if
 * it is not set already.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_pref_src(struct rtnl_route *route, struct nl_addr *addr)
{
	if (route->rt_mask & ROUTE_HAS_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;
		
	memcpy(&route->rt_pref_src, addr, sizeof(route->rt_pref_src));
	route->rt_mask |= (ROUTE_HAS_PREF_SRC | ROUTE_HAS_FAMILY);

	return 0;
}

/**
 * Set the preferred source address of a route to the specified address
 * @arg route		route to be changed
 * @arg addr		new preferred source address
 *
 * Translates the specified \a addr to a binary format and assigns
 * it as the new preferred source address.
 *
 * If a address family has been specified already via either calling
 * rtnl_route_set_family() or by setting one of the other addresses,
 * the specified \a addr is automatically validated against this family
 * and the assignment fails in case of a mismatch.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_pref_src_str(struct rtnl_route *route, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	int hint = route->rt_mask & ROUTE_HAS_FAMILY ? route->rt_family : AF_UNSPEC;
	
	err = nl_str2addr(addr, &a, hint);
	if (err < 0)
		return err;

	return rtnl_route_set_pref_src(route, &a);
}

/**
 * Set the outgoing interface of a route to the specified value
 * @arg route		route to be changed
 * @arg ifindex		interface index of new outoing interface
 */
void rtnl_route_set_oif(struct rtnl_route *route, int ifindex)
{
	route->rt_oif = ifindex;
	route->rt_mask |= ROUTE_HAS_OIF;
}

/**
 * Set the outgoing interface of a route via a interface name
 * @arg route		route to be changed
 * @arg cache		link cache to look up interface index
 * @arg name		interface name of new outgoing interface
 * @return 0 on success or a negative error code.
 */
int rtnl_route_set_oif_name(struct rtnl_route *route,
			    struct nl_cache *cache, const char *name)
{
	int i = rtnl_link_name2i(cache, name);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", name);
	rtnl_route_set_oif(route, i);
	return 0;
}

/**
 * Set the incoming interface of a route to the specified value
 * @arg route		route to be changed
 * @arg name		interface name of the new incoming interface
 */
void rtnl_route_set_iif(struct rtnl_route *route, const char *name)
{
	strncpy(route->rt_iif, name, sizeof(route->rt_iif) - 1);
	route->rt_mask |= ROUTE_HAS_IIF;
}


#if 0
	struct rtnl_nexthop *	rt_nexthops;
	struct rtnl_rtcacheinfo	rt_cacheinfo;
	struct nl_data		rt_session;
	struct nl_data		rt_protoinfo;
#endif

/** @} */

struct nl_cache_ops rtnl_route_ops = {
	.co_request_update	= route_request_update,
	.co_msg_parser		= route_msg_parser,
	.co_free_data		= route_free_data,
	.co_dump[NL_DUMP_BRIEF]	= route_dump_brief,
	.co_dump[NL_DUMP_FULL]	= route_dump_full,
	.co_dump[NL_DUMP_STATS]	= route_dump_with_stats,
	.co_filter		= route_filter,
};

/** @} */
