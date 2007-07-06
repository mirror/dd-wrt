/*
 * lib/route/route.c	Routes
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
 * @defgroup route Routing
 * @brief
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>
#include <netlink/data.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>

/** @cond SKIP */
#define ROUTE_ATTR_FAMILY    0x000001
#define ROUTE_ATTR_DST_LEN   0x000002
#define ROUTE_ATTR_SRC_LEN   0x000004
#define ROUTE_ATTR_TOS       0x000008
#define ROUTE_ATTR_TABLE     0x000010
#define ROUTE_ATTR_PROTOCOL  0x000020
#define ROUTE_ATTR_SCOPE     0x000040
#define ROUTE_ATTR_TYPE      0x000080
#define ROUTE_ATTR_FLAGS     0x000100
#define ROUTE_ATTR_DST       0x000200
#define ROUTE_ATTR_SRC       0x000400
#define ROUTE_ATTR_IIF       0x000800
#define ROUTE_ATTR_OIF       0x001000
#define ROUTE_ATTR_GATEWAY   0x002000
#define ROUTE_ATTR_PRIO      0x004000
#define ROUTE_ATTR_PREF_SRC  0x008000
#define ROUTE_ATTR_METRICS   0x010000
#define ROUTE_ATTR_MULTIPATH 0x020000
#define ROUTE_ATTR_REALMS    0x040000
#define ROUTE_ATTR_CACHEINFO 0x080000
#define ROUTE_ATTR_MP_ALGO   0x100000

#define NEXTHOP_HAS_FLAGS   0x000001
#define NEXTHOP_HAS_WEIGHT  0x000002
#define NEXTHOP_HAS_IFINDEX 0x000004
#define NEXTHOP_HAS_GATEWAY 0x000008

static struct nl_cache_ops rtnl_route_ops;
/** @endcond */

static void route_constructor(struct nl_object *c)
{
	struct rtnl_route *r = (struct rtnl_route *) c;

	nl_init_list_head(&r->rt_nexthops);
}

static void route_free_data(struct nl_object *c)
{
	struct rtnl_route *r = (struct rtnl_route *) c;
	struct rtnl_nexthop *nh, *tmp;

	if (r == NULL)
		return;

	nl_addr_put(r->rt_dst);
	nl_addr_put(r->rt_src);
	nl_addr_put(r->rt_gateway);
	nl_addr_put(r->rt_pref_src);

	nl_list_for_each_entry_safe(nh, tmp, &r->rt_nexthops, rtnh_list) {
		rtnl_route_remove_nexthop(nh);
		rtnl_route_nh_free(nh);
	}
}

static struct nla_policy route_policy[RTA_MAX+1] = {
	[RTA_IIF]	= { .type = NLA_STRING,
			    .maxlen = IFNAMSIZ, },
	[RTA_OIF]	= { .type = NLA_U32 },
	[RTA_PRIORITY]	= { .type = NLA_U32 },
	[RTA_FLOW]	= { .type = NLA_U32 },
	[RTA_MP_ALGO]	= { .type = NLA_U32 },
	[RTA_CACHEINFO]	= { .minlen = sizeof(struct rta_cacheinfo) },
	[RTA_METRICS]	= { .type = NLA_NESTED },
	[RTA_MULTIPATH]	= { .type = NLA_NESTED },
};

static void copy_rtmsg_into_route(struct rtmsg *rtmsg, struct rtnl_route *route)
{
	route->rt_family	= rtmsg->rtm_family;
	route->rt_dst_len	= rtmsg->rtm_dst_len;
	route->rt_src_len	= rtmsg->rtm_src_len;
	route->rt_tos		= rtmsg->rtm_tos;
	route->rt_table		= rtmsg->rtm_table;
	route->rt_type		= rtmsg->rtm_type;
	route->rt_scope		= rtmsg->rtm_scope;
	route->rt_protocol	= rtmsg->rtm_protocol;
	route->rt_flags		= rtmsg->rtm_flags;

	route->rt_mask = (ROUTE_ATTR_FAMILY | ROUTE_ATTR_DST_LEN | 
			  ROUTE_ATTR_SRC_LEN | ROUTE_ATTR_TABLE |
			  ROUTE_ATTR_PROTOCOL| ROUTE_ATTR_SCOPE |
			  ROUTE_ATTR_TYPE | ROUTE_ATTR_FLAGS);

	if (route->rt_tos)
		route->rt_mask |= ROUTE_ATTR_TOS;
}

static void copy_cacheinfo_into_route(struct rta_cacheinfo *ci,
				      struct rtnl_route *route)
{
	route->rt_cacheinfo.rtci_clntref  = ci->rta_clntref;
	route->rt_cacheinfo.rtci_last_use = ci->rta_lastuse;
	route->rt_cacheinfo.rtci_expires  = ci->rta_expires;
	route->rt_cacheinfo.rtci_error    = ci->rta_error;
	route->rt_cacheinfo.rtci_used     = ci->rta_used;
	route->rt_cacheinfo.rtci_id       = ci->rta_id;
	route->rt_cacheinfo.rtci_ts       = ci->rta_ts;
	route->rt_cacheinfo.rtci_tsage    = ci->rta_tsage;

	route->rt_mask |= ROUTE_ATTR_CACHEINFO;
}

static int route_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *nlh,
			    void *arg)
{
	struct nl_parser_param *pp = arg;
	struct rtnl_route *route;
	struct nlattr *tb[RTA_MAX + 1];
	int err;

	route = rtnl_route_alloc();
	if (!route) {
		err = nl_errno(ENOMEM);
		goto errout;
	}

	route->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(struct rtmsg), tb, RTA_MAX,
			  route_policy);
	if (err < 0)
		goto errout;

	copy_rtmsg_into_route((struct rtmsg *) nlmsg_data(nlh), route);

	if (tb[RTA_DST]) {
		route->rt_dst = nla_get_addr(tb[RTA_DST], route->rt_family);
		if (!route->rt_dst)
			goto errout_errno;
		nl_addr_set_prefixlen(route->rt_dst, route->rt_dst_len);
		route->rt_mask |= ROUTE_ATTR_DST;
	}

	if (tb[RTA_SRC]) {
		route->rt_src = nla_get_addr(tb[RTA_SRC], route->rt_family);
		if (!route->rt_src)
			goto errout_errno;
		nl_addr_set_prefixlen(route->rt_src, route->rt_src_len);
		route->rt_mask |= ROUTE_ATTR_SRC;
	}

	if (tb[RTA_IIF]) {
		nla_strlcpy(route->rt_iif, tb[RTA_IIF], IFNAMSIZ);
		route->rt_mask |= ROUTE_ATTR_IIF;
	}

	if (tb[RTA_OIF]) {
		route->rt_oif = nla_get_u32(tb[RTA_OIF]);
		route->rt_mask |= ROUTE_ATTR_OIF;
	}

	if (tb[RTA_GATEWAY]) {
		route->rt_gateway = nla_get_addr(tb[RTA_GATEWAY],
						 route->rt_family);
		if (!route->rt_gateway)
			goto errout_errno;
		route->rt_mask |= ROUTE_ATTR_GATEWAY;
	}

	if (tb[RTA_PRIORITY]) {
		route->rt_prio = nla_get_u32(tb[RTA_PRIORITY]);
		route->rt_mask |= ROUTE_ATTR_PRIO;
	}

	if (tb[RTA_PREFSRC]) {
		route->rt_pref_src = nla_get_addr(tb[RTA_PREFSRC],
						  route->rt_family);
		if (!route->rt_pref_src)
			goto errout_errno;
		route->rt_mask |= ROUTE_ATTR_PREF_SRC;
	}

	if (tb[RTA_METRICS]) {
		struct nlattr *mtb[RTAX_MAX + 1];
		int i;

		err = nla_parse_nested(mtb, RTAX_MAX, tb[RTA_METRICS], NULL);
		if (err < 0)
			goto errout;

		for (i = 1; i <= RTAX_MAX; i++) {
			if (mtb[i] && nla_len(mtb[i]) >= sizeof(uint32_t)) {
				uint32_t m = nla_get_u32(mtb[i]);
				route->rt_metrics[i-1] = m;
				route->rt_metrics_mask |= (1 << (i - 1));
			}
		}
		
		route->rt_mask |= ROUTE_ATTR_METRICS;
	}

	if (tb[RTA_MULTIPATH]) {
		struct rtnl_nexthop *nh;
		struct rtnexthop *rtnh = nla_data(tb[RTA_MULTIPATH]);
		size_t tlen = nla_len(tb[RTA_MULTIPATH]);

		while (tlen >= sizeof(*rtnh) && tlen >= rtnh->rtnh_len) {
			nh = rtnl_route_nh_alloc();
			if (!nh)
				goto errout;

			rtnl_route_nh_set_weight(nh, rtnh->rtnh_hops);
			rtnl_route_nh_set_ifindex(nh, rtnh->rtnh_ifindex);
			rtnl_route_nh_set_flags(nh, rtnh->rtnh_flags);

			if (rtnh->rtnh_len > sizeof(*rtnh)) {
				struct nlattr *ntb[RTA_MAX + 1];
				nla_parse(ntb, RTA_MAX, (struct nlattr *)
					  RTNH_DATA(rtnh),
					  rtnh->rtnh_len - sizeof(*rtnh),
					  route_policy);

				if (ntb[RTA_GATEWAY]) {
					nh->rtnh_gateway = nla_get_addr(
							ntb[RTA_GATEWAY],
							route->rt_family);
					nh->rtnh_mask = NEXTHOP_HAS_GATEWAY;
				}
			}

			rtnl_route_add_nexthop(route, nh);
			tlen -= RTNH_ALIGN(rtnh->rtnh_len);
			rtnh = RTNH_NEXT(rtnh);
		}
	}

	if (tb[RTA_FLOW]) {
		route->rt_realms = nla_get_u32(tb[RTA_FLOW]);
		route->rt_mask |= ROUTE_ATTR_REALMS;
	}

	if (tb[RTA_CACHEINFO])
		copy_cacheinfo_into_route(nla_data(tb[RTA_CACHEINFO]), route);

	if (tb[RTA_MP_ALGO]) {
		route->rt_mp_algo = nla_get_u32(tb[RTA_MP_ALGO]);
		route->rt_mask |= ROUTE_ATTR_MP_ALGO;
	}

	err = pp->pp_cb((struct nl_object *) route, pp);
	if (err < 0)
		goto errout;

	return P_ACCEPT;

errout_errno:
	err = nl_get_errno();
errout:
	rtnl_route_put(route);
	return err;

}

static int route_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETROUTE, AF_UNSPEC, NLM_F_DUMP);
}

static int route_dump_brief(struct nl_object *a, struct nl_dump_params *p)
{
	struct rtnl_route *r = (struct rtnl_route *) a;
	struct nl_cache *link_cache;
	char buf[64];

	link_cache = nl_cache_mngt_require("route/link");

	if (r->rt_mask & ROUTE_ATTR_DST)
		dp_dump(p, "%s ", nl_addr2str(r->rt_dst, buf, sizeof(buf)));
	else if (r->rt_dst_len)
		dp_dump(p, "0/%u ", r->rt_dst_len);
	else
		dp_dump(p, "default ");

	if (r->rt_mask & ROUTE_ATTR_OIF) {
		if (link_cache)
			dp_dump(p, "dev %s ",
				rtnl_link_i2name(link_cache, r->rt_oif,
						 buf, sizeof(buf)));
		else
			dp_dump(p, "dev %d ", r->rt_oif);
	}

	if (r->rt_mask & ROUTE_ATTR_GATEWAY)
		dp_dump(p, "via %s ", nl_addr2str(r->rt_gateway, buf,
						  sizeof(buf)));
	else if (r->rt_mask & ROUTE_ATTR_MULTIPATH)
		dp_dump(p, "via nexthops ");

	if (r->rt_mask & ROUTE_ATTR_TABLE)
		dp_dump(p, "table %s ",
			rtnl_route_table2str(r->rt_table, buf, sizeof(buf)));

	if (r->rt_mask & ROUTE_ATTR_SCOPE)
		dp_dump(p, "scope %s ",
			rtnl_scope2str(r->rt_scope, buf, sizeof(buf)));

	if (r->rt_mask & ROUTE_ATTR_FLAGS && r->rt_flags) {
		int flags = r->rt_flags;

		dp_dump(p, "<");
		
#define PRINT_FLAG(f) if (flags & RTNH_F_##f) { \
		flags &= ~RTNH_F_##f; dp_dump(p, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(DEAD);
		PRINT_FLAG(ONLINK);
		PRINT_FLAG(PERVASIVE);
#undef PRINT_FLAG

#define PRINT_FLAG(f) if (flags & RTM_F_##f) { \
		flags &= ~RTM_F_##f; dp_dump(p, #f "%s", flags ? "," : ""); }
		PRINT_FLAG(NOTIFY);
		PRINT_FLAG(CLONED);
		PRINT_FLAG(EQUALIZE);
		PRINT_FLAG(PREFIX);
#undef PRINT_FLAG

		dp_dump(p, ">");
	}

	dp_dump(p, "\n");

	return 1;
}

static int route_dump_full(struct nl_object *a, struct nl_dump_params *p)
{
	struct rtnl_route *r = (struct rtnl_route *) a;
	struct nl_cache *link_cache;
	char buf[128];
	int i, line;

	link_cache = nl_cache_mngt_require("route/link");
	line = route_dump_brief(a, p);

	if (r->rt_mask & ROUTE_ATTR_MULTIPATH) {
		struct rtnl_nexthop *nh;

		dp_dump_line(p, line++, "  ");

		nl_list_for_each_entry(nh, &r->rt_nexthops, rtnh_list) {
			dp_dump(p, "nh ");

			if (nh->rtnh_mask & NEXTHOP_HAS_GATEWAY)
				dp_dump(p, "via %s ",
					nl_addr2str(nh->rtnh_gateway,
						    buf, sizeof(buf)));
			if (link_cache)
				dp_dump(p, "dev %s ",
					rtnl_link_i2name(link_cache,
							 nh->rtnh_ifindex,
							 buf, sizeof(buf)));
			else
				dp_dump(p, "dev %d ", nh->rtnh_ifindex);

			dp_dump(p, "weight %u <%s> ", nh->rtnh_weight,
				rtnl_route_nh_flags2str(nh->rtnh_flags,
							buf, sizeof(buf)));
		}

		dp_dump(p, "\n");
	}

	dp_dump_line(p, line++, "  ");

	if (r->rt_mask & ROUTE_ATTR_PREF_SRC)
		dp_dump(p, "preferred-src %s ",
			nl_addr2str(r->rt_pref_src, buf, sizeof(buf)));

	if (r->rt_mask & ROUTE_ATTR_TYPE)
		dp_dump(p, "type %s ",
			nl_rtntype2str(r->rt_type, buf, sizeof(buf)));

	if (r->rt_mask & ROUTE_ATTR_PRIO)
		dp_dump(p, "metric %#x ", r->rt_prio);

	if (r->rt_mask & ROUTE_ATTR_FAMILY)
		dp_dump(p, "family %s ",
			nl_af2str(r->rt_family, buf, sizeof(buf)));

	if (r->rt_mask & ROUTE_ATTR_PROTOCOL)
		dp_dump(p, "protocol %s ",
			rtnl_route_proto2str(r->rt_protocol, buf, sizeof(buf)));

	dp_dump(p, "\n");

	if ((r->rt_mask & (ROUTE_ATTR_IIF | ROUTE_ATTR_SRC | ROUTE_ATTR_TOS |
			   ROUTE_ATTR_REALMS)) || r->rt_src_len ||
	    ((r->rt_mask & ROUTE_ATTR_CACHEINFO) &&
	     r->rt_cacheinfo.rtci_error)) {
		dp_dump_line(p, line++, "  ");

		if (r->rt_mask & ROUTE_ATTR_IIF)
			dp_dump(p, "iif %s ", r->rt_iif);

		if (r->rt_mask & ROUTE_ATTR_SRC)
			dp_dump(p, "src %s ",
				nl_addr2str(r->rt_src, buf, sizeof(buf)));
		else if (r->rt_src_len)
			dp_dump(p, "src 0/%u ", r->rt_src_len);

		if (r->rt_mask & ROUTE_ATTR_TOS)
			dp_dump(p, "tos %#x ", r->rt_tos);

		if (r->rt_mask & ROUTE_ATTR_REALMS)
			dp_dump(p, "realm %04x:%04x ",
				RTNL_REALM_FROM(r->rt_realms),
				RTNL_REALM_TO(r->rt_realms));

		if ((r->rt_mask & ROUTE_ATTR_CACHEINFO) &&
		    r->rt_cacheinfo.rtci_error)
			dp_dump(p, "error %d (%s) ", r->rt_cacheinfo.rtci_error,
				strerror(-r->rt_cacheinfo.rtci_error));

		dp_dump(p, "\n");
	}

	if (r->rt_mask & ROUTE_ATTR_METRICS) {
		dp_dump_line(p, line++, "  ");
		for (i = 0; i < RTAX_MAX; i++)
			if (r->rt_metrics_mask & (1 << i))
				dp_dump(p, "%s %u ",
					rtnl_route_metric2str(i+1,
							      buf, sizeof(buf)),
					r->rt_metrics[i]);
		dp_dump(p, "\n");
	}

	return line;
}

static int route_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_route *route = (struct rtnl_route *) obj;
	int line;

	line = route_dump_full(obj, p);

	if (route->rt_mask & ROUTE_ATTR_CACHEINFO) {
		struct rtnl_rtcacheinfo *ci = &route->rt_cacheinfo;
		dp_dump_line(p, line++, "  used %u refcnt %u ",
			     ci->rtci_used, ci->rtci_clntref);
		dp_dump_line(p, line++, "last-use %us expires %us\n",
			     ci->rtci_last_use / nl_get_hz(),
			     ci->rtci_expires / nl_get_hz());
	}

	return line;
}

static int route_dump_xml(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_route *route = (struct rtnl_route *) obj;
	char buf[128];
	int line = 0;
	
	dp_dump_line(p, line++, "<route>\n");
	dp_dump_line(p, line++, "  <family>%s</family>\n",
		     nl_af2str(route->rt_family, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_DST)
		dp_dump_line(p, line++, "  <dst>%s</dst>\n",
			     nl_addr2str(route->rt_dst, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_DST_LEN)
		dp_dump_line(p, line++, "  <dstlen>%u</dstlen>\n",
			     route->rt_dst_len);

	if (route->rt_mask & ROUTE_ATTR_SRC)
		dp_dump_line(p, line++, "  <src>%s</src>\n",
			     nl_addr2str(route->rt_src, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_SRC_LEN)
		dp_dump_line(p, line++, "  <srclen>%u</srclen>\n",
			     route->rt_src_len);

	if (route->rt_mask & ROUTE_ATTR_GATEWAY)
		dp_dump_line(p, line++, "  <gateway>%s</gateway>\n",
			     nl_addr2str(route->rt_gateway, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_PREF_SRC)
		dp_dump_line(p, line++, "  <prefsrc>%s</prefsrc>\n",
			     nl_addr2str(route->rt_pref_src, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_IIF)
		dp_dump_line(p, line++, "  <iif>%s</iif>\n", route->rt_iif);

	if (route->rt_mask & ROUTE_ATTR_REALMS)
		dp_dump_line(p, line++, "  <realms>%u</realms>\n",
			     route->rt_realms);

	if (route->rt_mask & ROUTE_ATTR_TOS)
		dp_dump_line(p, line++, "  <tos>%u</tos>\n", route->rt_tos);

	if (route->rt_mask & ROUTE_ATTR_TABLE)
		dp_dump_line(p, line++, "  <table>%u</table>\n",
			     route->rt_table);

	if (route->rt_mask & ROUTE_ATTR_SCOPE)
		dp_dump_line(p, line++, "  <scope>%s</scope>\n",
			     rtnl_scope2str(route->rt_scope, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_PRIO)
		dp_dump_line(p, line++, "  <metric>%u</metric>\n",
			     route->rt_prio);

	if (route->rt_mask & ROUTE_ATTR_OIF) {
		struct nl_cache *link_cache;
	
		link_cache = nl_cache_mngt_require("route/link");
		if (link_cache)
			dp_dump_line(p, line++, "  <oif>%s</oif>\n",
				     rtnl_link_i2name(link_cache,
						      route->rt_oif,
						      buf, sizeof(buf)));
		else
			dp_dump_line(p, line++, "  <oif>%u</oif>\n",
				     route->rt_oif);
	}

	if (route->rt_mask & ROUTE_ATTR_TYPE)
		dp_dump_line(p, line++, "  <type>%s</type>\n",
			     nl_rtntype2str(route->rt_type, buf, sizeof(buf)));

	dp_dump_line(p, line++, "</route>\n");

#if 0
	uint8_t			rt_protocol;
	uint32_t		rt_flags;
	uint32_t		rt_metrics[RTAX_MAX];
	uint32_t		rt_metrics_mask;
	struct rtnl_nexthop *	rt_nexthops;
	struct rtnl_rtcacheinfo	rt_cacheinfo;
	uint32_t		rt_mp_algo;

#endif

	return line;
}

static int route_dump_env(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_route *route = (struct rtnl_route *) obj;
	char buf[128];
	int line = 0;

	dp_dump_line(p, line++, "ROUTE_FAMILY=%s\n",
		     nl_af2str(route->rt_family, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_DST)
		dp_dump_line(p, line++, "ROUTE_DST=%s\n",
			     nl_addr2str(route->rt_dst, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_DST_LEN)
		dp_dump_line(p, line++, "ROUTE_DSTLEN=%u\n",
			     route->rt_dst_len);

	if (route->rt_mask & ROUTE_ATTR_SRC)
		dp_dump_line(p, line++, "ROUTE_SRC=%s\n",
			     nl_addr2str(route->rt_src, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_SRC_LEN)
		dp_dump_line(p, line++, "ROUTE_SRCLEN=%u\n",
			     route->rt_src_len);

	if (route->rt_mask & ROUTE_ATTR_GATEWAY)
		dp_dump_line(p, line++, "ROUTE_GATEWAY=%s\n",
			     nl_addr2str(route->rt_gateway, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_PREF_SRC)
		dp_dump_line(p, line++, "ROUTE_PREFSRC=%s\n",
			     nl_addr2str(route->rt_pref_src, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_IIF)
		dp_dump_line(p, line++, "ROUTE_IIF=%s\n", route->rt_iif);

	if (route->rt_mask & ROUTE_ATTR_REALMS)
		dp_dump_line(p, line++, "ROUTE_REALM=%u\n",
			     route->rt_realms);

	if (route->rt_mask & ROUTE_ATTR_TOS)
		dp_dump_line(p, line++, "ROUTE_TOS=%u\n", route->rt_tos);

	if (route->rt_mask & ROUTE_ATTR_TABLE)
		dp_dump_line(p, line++, "ROUTE_TABLE=%u\n",
			     route->rt_table);

	if (route->rt_mask & ROUTE_ATTR_SCOPE)
		dp_dump_line(p, line++, "ROUTE_SCOPE=%s\n",
			     rtnl_scope2str(route->rt_scope, buf, sizeof(buf)));

	if (route->rt_mask & ROUTE_ATTR_PRIO)
		dp_dump_line(p, line++, "ROUTE_METRIC=%u\n",
			     route->rt_prio);

	if (route->rt_mask & ROUTE_ATTR_OIF) {
		struct nl_cache *link_cache;

		dp_dump_line(p, line++, "ROUTE_OIF_IFINDEX=%u\n",
			     route->rt_oif);

		link_cache = nl_cache_mngt_require("route/link");
		if (link_cache)
			dp_dump_line(p, line++, "ROUTE_OIF_IFNAME=%s\n",
				     rtnl_link_i2name(link_cache,
						      route->rt_oif,
						      buf, sizeof(buf)));
	}

	if (route->rt_mask & ROUTE_ATTR_TYPE)
		dp_dump_line(p, line++, "ROUTE_TYPE=%s\n",
			     nl_rtntype2str(route->rt_type, buf, sizeof(buf)));

	return line;
}

static int route_filter(struct nl_object *obj, struct nl_object *filter)
{
	struct rtnl_route *o = (struct rtnl_route *) obj;
	struct rtnl_route *f = (struct rtnl_route *) filter;

#define REQ(F) (f->rt_mask & ROUTE_ATTR_##F)
#define AVAIL(F) (o->rt_mask & ROUTE_ATTR_##F)
#define _O(F, EXPR) (REQ(F) && (!AVAIL(F) || (EXPR)))
#define _C(F, N) (REQ(F) && (!AVAIL(F) || (o->N != f->N)))
	if (_C(FAMILY,	  rt_family)					||
	    _C(DST_LEN,	  rt_dst_len)					||
	    _C(SRC_LEN,	  rt_src_len)					||
	    _C(TOS,	  rt_tos)					||
	    _C(TABLE,	  rt_table)					||
	    _C(PROTOCOL,  rt_protocol)					||
	    _C(SCOPE,	  rt_scope)					||
	    _C(TYPE,	  rt_type)					||
	    _C(OIF,	  rt_oif)					||
	    _C(PRIO,	  rt_prio)					||
	    _C(REALMS,	  rt_realms)					||
	    _C(MP_ALGO,	  rt_mp_algo)					||
	    _O(DST,	  nl_addr_cmp(o->rt_dst, f->rt_dst))		||
	    _O(SRC,	  nl_addr_cmp(o->rt_src, f->rt_src))		||
	    _O(IIF,	  strcmp(o->rt_iif, f->rt_iif))			||
	    _O(PREF_SRC,  nl_addr_cmp(o->rt_pref_src, f->rt_pref_src))	||
	    _O(GATEWAY,	  nl_addr_cmp(o->rt_gateway, f->rt_gateway))	||
	    _O(FLAGS,	  f->rt_flags ^ (o->rt_flags & f->rt_flag_mask)))
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

	if (REQ(MULTIPATH)) {
		/* FIXME */
	}
	
#undef REQ
#undef AVAIL
#undef _O
#undef _C

	return 1;

}

/**
 * @name Route Object Allocation/Freeage
 * @{
 */

/**
 * Allocate a new route object
 * @return New route object
 */
struct rtnl_route *rtnl_route_alloc(void)
{
	return (struct rtnl_route *) nl_object_alloc_from_ops(&rtnl_route_ops);
}

/**
 * Free route object.
 * @arg route		Route object to be freed.
 *
 * @note Always use rtnl_route_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_route_free(struct rtnl_route *route)
{
	nl_object_free((struct nl_object *) route);
}

/** @} */

/**
 * @name Route Object Reference Counting
 * @{
 */

void rtnl_route_get(struct rtnl_route *route)
{
	nl_object_get((struct nl_object *) route);
}

void rtnl_route_put(struct rtnl_route *route)
{
	nl_object_put((struct nl_object *) route);
}

/** @} */

/**
 * @name Route Cache Management
 * @{
 */

/**
 * Build a route cache holding all routes currently configured in the kernel
 * @arg handle		netlink handle
 *
 * Allocates a new cache, initializes it properly and updates it to
 * contain all routes currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it.
 * @return The cache or NULL if an error has occured.
 */
struct nl_cache *rtnl_route_alloc_cache(struct nl_handle *handle)
{
	struct nl_cache *cache = nl_cache_alloc_from_ops(&rtnl_route_ops);

	if (!cache)
		return NULL;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

/** @} */

/**
 * @name Route Addition
 * @{
 */

static struct nl_msg *build_route_msg(struct rtnl_route *tmpl, int cmd,
				      int flags)
{
#if 0
	struct nl_msg *msg;
	struct rtmsg rtmsg = {
		.rtm_family = tmpl->rt_family,
	};
	route->rt_dst_len	= rtmsg->rtm_dst_len;
	route->rt_src_len	= rtmsg->rtm_src_len;
	route->rt_tos		= rtmsg->rtm_tos;
	route->rt_table		= rtmsg->rtm_table;
	route->rt_type		= rtmsg->rtm_type;
	route->rt_scope		= rtmsg->rtm_scope;
	route->rt_protocol	= rtmsg->rtm_protocol;
	route->rt_flags		= rtmsg->rtm_flags;

	route->rt_mask = (ROUTE_ATTR_FAMILY | ROUTE_ATTR_DST_LEN | 
			  ROUTE_ATTR_SRC_LEN | ROUTE_ATTR_TABLE |
			  ROUTE_ATTR_PROTOCOL| ROUTE_ATTR_SCOPE |
			  ROUTE_ATTR_TYPE | ROUTE_ATTR_FLAGS);

	msg = nlmsg_build_simple(cmd, flags);
	if (!msg)
		return NULL;

	if (nlmsg_append(msg, &rtmsg, sizeof(rtmsg), 1) < 0)
		goto nla_put_failure;

	NLA_PUT_ADDR(msg, NDA_DST, tmpl->n_dst);

	if (tmpl->n_mask & NEIGH_ATTR_LLADDR)
		NLA_PUT_ADDR(msg, NDA_LLADDR, tmpl->n_lladdr);

	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
#endif

	return NULL;
}

/**
 * Build netlink request message to add a new route
 * @arg tmpl		template with data of new route
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a addition of a new route.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete() or
 * supplemented as needed. \a tmpl must contain the attributes of the
 * new route set via \c rtnl_route_set_* functions.
 * 
 * The following attributes must be set in the template:
 *  - Interface index (rtnl_neigh_set_ifindex())
 *  - State (rtnl_neigh_set_state())
 *  - Destination address (rtnl_neigh_set_dst())
 *  - Link layer address (rtnl_neigh_set_lladdr())
 *
 * @return The netlink message
 */
struct nl_msg *rtnl_route_build_add_request(struct rtnl_route *tmpl, int flags)
{
	return build_route_msg(tmpl, RTM_NEWROUTE, NLM_F_CREATE | flags);
}

/** @} */
/**
 * @name Attribute: Routing Table
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
	route->rt_mask |= ROUTE_ATTR_TABLE;
}

/**
 * Get the table of a route
 * @arg route		route handle
 * @return Table id or -1 if not set
 */
int rtnl_route_get_table(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_TABLE)
		return route->rt_table;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Scope
 * @{
 */

/**
 * Set the scope of a route to the specified value
 * @arg route		route to be changed
 * @arg scope		new scope
 */
void rtnl_route_set_scope(struct rtnl_route *route, int scope)
{
	route->rt_scope = scope;
	route->rt_mask |= ROUTE_ATTR_SCOPE;
}

/**
 * Get the scope of a route
 * @arg route		route handle
 * @return Scope or -1 if not set
 */
int rtnl_route_get_scope(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_SCOPE)
		return route->rt_scope;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Type Of Service
 * @{
 */

/**
 * Set the TOS of a route to the specified value
 * @arg route		route to be changed
 * @arg tos		new TOS value
 */
void rtnl_route_set_tos(struct rtnl_route *route, int tos)
{
	route->rt_tos = tos;
	route->rt_mask |= ROUTE_ATTR_TOS;
}

/**
 * Get the TOS of a route
 * @arg route		route handle
 * @return TOS value or -1 if not set
 */
int rtnl_route_get_tos(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_TOS)
		return route->rt_tos;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Realm
 * @{
 */

/**
 * Set the realms of a route to the specified value
 * @arg route		route to be changed
 * @arg realms		New realms value.
 */
void rtnl_route_set_realms(struct rtnl_route *route, realm_t realms)
{
	route->rt_realms = realms;
	route->rt_mask |= ROUTE_ATTR_REALMS;
}

/**
 * Get realms of route object.
 * @arg route		Route object.
 * @return Realms value or 0 if not set.
 */
realm_t rtnl_route_get_realms(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_REALMS)
		return route->rt_realms;
	else
		return 0;
}

/** @} */
/**
 * @name Attribute: Routing Protocol
 * @{
 */

/**
 * Set the protocol of a route to the specified value
 * @arg route		route to be changed
 * @arg proto		new protocol
 */
void rtnl_route_set_protocol(struct rtnl_route *route, int proto)
{
	route->rt_protocol = proto;
	route->rt_mask |= ROUTE_ATTR_PROTOCOL;
}

/**
 * Get the protocol of a route
 * @arg route		route handle
 * @return Protocol number or -1 if not set
 */
int rtnl_route_get_protocol(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_PROTOCOL)
		return route->rt_protocol;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Priority/Metric
 * @{
 */

/**
 * Set the priority of a route to the specified value
 * @arg route		route to be changed
 * @arg prio		new priority
 */
void rtnl_route_set_prio(struct rtnl_route *route, int prio)
{
	route->rt_prio = prio;
	route->rt_mask |= ROUTE_ATTR_PRIO;
}

/**
 * Get the priority of a route
 * @arg route		route handle
 * @return Priority or -1 if not set
 */
int rtnl_route_get_prio(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_PRIO)
		return route->rt_prio;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Address Family
 * @{
 */

/**
 * Set the address family of a route to the specified value
 * @arg route		route to be changed
 * @arg family		new address family
 */
void rtnl_route_set_family(struct rtnl_route *route, int family)
{
	route->rt_family = family;
	route->rt_mask |= ROUTE_ATTR_FAMILY;
}

/**
 * Get the address family of a route
 * @arg route		route handle
 * @return Address family or AF_UNSPEC if not set
 */
int rtnl_route_get_family(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_FAMILY)
		return route->rt_family;
	else
		return AF_UNSPEC;
}

/** @} */
/**
 * @name Attribute: Destination Address
 * @{
 */

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
	if (route->rt_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;

	if (route->rt_dst)
		nl_addr_put(route->rt_dst);

	nl_addr_get(addr);
	route->rt_dst = addr;
	
	route->rt_mask |= (ROUTE_ATTR_DST|ROUTE_ATTR_FAMILY|ROUTE_ATTR_DST_LEN);

	return 0;
}

/**
 * Get the destination address of a route
 * @arg route		route handle
 * @return Destination address or NULL if not set
 */
struct nl_addr *rtnl_route_get_dst(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_DST)
		return route->rt_dst;
	else
		return NULL;
}

/**
 * Set the destination address prefix length of a route to the specified value
 * @arg route		route to be changed
 * @arg prefix		new destination address prefix
 * @attention The destination address prefix gets overwritten by calls
 *            to rtnl_route_set_dst() rtnl_route_set_dst_str().
 */
void rtnl_route_set_dst_len(struct rtnl_route *route, int prefix)
{
	route->rt_dst_len = prefix;
	route->rt_mask |= ROUTE_ATTR_DST_LEN;
}

/**
 * Get the destination address prefix length of a route
 * @arg route		route handle
 * @return Prefix length or -1 if not set
 */
int rtnl_route_get_dst_len(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_DST_LEN)
		return route->rt_dst_len;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Source Address
 * @{
 */

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
	if (route->rt_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;

	if (route->rt_src)
		nl_addr_put(route->rt_src);

	nl_addr_get(addr);
	route->rt_src = addr;
	route->rt_mask |= (ROUTE_ATTR_SRC|ROUTE_ATTR_FAMILY|ROUTE_ATTR_SRC_LEN);

	return 0;
}

/**
 * Get the source address of a route
 * @arg route		route handle
 * @return Source address or NULL if not set
 */
struct nl_addr *rtnl_route_get_src(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_SRC)
		return route->rt_src;
	else
		return NULL;
}

/**
 * Set the source address prefix length of a route to the specified value
 * @arg route		route to be changed
 * @arg prefix		new source address prefix
 * @attention The source address prefix gets overwritten by calls
 *            to rtnl_route_src_dst() rtnl_route_set_src_str().
 */
void rtnl_route_set_src_len(struct rtnl_route *route, int prefix)
{
	route->rt_dst_len = prefix;
	route->rt_mask |= ROUTE_ATTR_SRC_LEN;
}

/**
 * Get the source address prefix length of a route
 * @arg route		route handle
 * @return Prefix length or -1 if not set
 */
int rtnl_route_get_src_len(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_SRC_LEN)
		return route->rt_src_len;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Gateway Address
 * @{
 */

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
	if (route->rt_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;

	if (route->rt_gateway)
		nl_addr_put(route->rt_gateway);

	nl_addr_get(addr);
	route->rt_gateway = addr;
	route->rt_mask |= (ROUTE_ATTR_GATEWAY | ROUTE_ATTR_FAMILY);

	return 0;
}

/**
 * Get the gateway address of a route
 * @arg route		route handle
 * @return Gateway address or NULL if not set
 */
struct nl_addr *rtnl_route_get_gateway(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_GATEWAY)
		return route->rt_gateway;
	else
		return NULL;
}

/** @} */
/**
 * @name Attribute: Type
 * @{
 */

/**
 * Set the type of a route to the specified value
 * @arg route		route to be changed
 * @arg type		new route type
 */
void rtnl_route_set_type(struct rtnl_route *route, int type)
{
	route->rt_type = type;
	route->rt_mask |= ROUTE_ATTR_TYPE;
}

/**
 * Get the type of a route
 * @arg route		route handle
 * @return Type of route or -1 if not set
 */
int rtnl_route_get_type(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_TYPE)
		return route->rt_type;
	else
		return -1;
}

/** @} */
/**
 * @name Attribute: Flags
 * @{
 */

/**
 * Add flags to a route 
 * @arg route		route to be changed
 * @arg flags		flags to set
 */
void rtnl_route_set_flags(struct rtnl_route *route, unsigned int flags)
{
	route->rt_flag_mask |= flags;
	route->rt_flags |= flags;
	route->rt_mask |= ROUTE_ATTR_FLAGS;
}

/**
 * Remove flags from a route 
 * @arg route		route to be changed
 * @arg flags		flags to unset
 */
void rtnl_route_unset_flags(struct rtnl_route *route, unsigned int flags)
{
	route->rt_flag_mask |= flags;
	route->rt_flags &= ~flags;
	route->rt_mask |= ROUTE_ATTR_FLAGS;
}

/**
 * Get flags of a route
 * @arg route		route handle
 */
unsigned int rtnl_route_get_flags(struct rtnl_route *route)
{
	return route->rt_flags;
}

/** @} */
/**
 * @name Attribute: Routing Metrics
 * @{
 */

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
 * Get a metric for a route
 * @arg route		route handle
 * @arg metric		metric to get
 * @return The value for the specified metric or UINT_MAX if not set
 */
unsigned int rtnl_route_get_metric(struct rtnl_route *route, int metric)
{
	if (metric <= RTAX_MAX || metric < 1)
		return UINT_MAX;

	if (!(route->rt_metrics_mask & (1 << (metric - 1))))
		return UINT_MAX;

	return route->rt_metrics[metric - 1];
}

/** @} */
/**
 * @name Attribute: Preferred Source Address
 * @{
 */

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
	if (route->rt_mask & ROUTE_ATTR_FAMILY) {
		if (addr->a_family != route->rt_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		route->rt_family = addr->a_family;

	if (route->rt_pref_src)
		nl_addr_put(route->rt_pref_src);

	nl_addr_get(addr);
	route->rt_pref_src = addr;
	route->rt_mask |= (ROUTE_ATTR_PREF_SRC | ROUTE_ATTR_FAMILY);

	return 0;
}

/**
 * Get the preferred source address of a route
 * @arg route		route handle
 * @return Preferred source address or NULL if not set
 */
struct nl_addr *rtnl_route_get_pref_src(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_PREF_SRC)
		return route->rt_pref_src;
	else
		return NULL;
}

/** @} */
/**
 * @name Attribute: Outgoing Interface Index
 * @{
 */

/**
 * Set the outgoing interface of a route to the specified value
 * @arg route		route to be changed
 * @arg ifindex		interface index of new outoing interface
 */
void rtnl_route_set_oif(struct rtnl_route *route, int ifindex)
{
	route->rt_oif = ifindex;
	route->rt_mask |= ROUTE_ATTR_OIF;
}

/**
 * Get the outgoing interface index of a route
 * @arg route		route handle
 * @return interface index or RTNL_LINK_NOT_FOUND if not set
 */
int rtnl_route_get_oif(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_OIF)
		return route->rt_oif;
	else
		return RTNL_LINK_NOT_FOUND;
}

/** @} */
/**
 * @name Attribute: Incoming Interface
 * @{
 */

/**
 * Set the incoming interface of a route to the specified value
 * @arg route		route to be changed
 * @arg name		interface name of the new incoming interface
 */
void rtnl_route_set_iif(struct rtnl_route *route, const char *name)
{
	strncpy(route->rt_iif, name, sizeof(route->rt_iif) - 1);
	route->rt_mask |= ROUTE_ATTR_IIF;
}

/**
 * Get the incomming interface name of a route
 * @arg route		route handle
 * @return interface name or NULL if not set
 */
char *rtnl_route_get_iif(struct rtnl_route *route)
{
	if (route->rt_mask & ROUTE_ATTR_IIF)
		return route->rt_iif;
	else
		return NULL;
}

#if 0
	struct rtnl_rtcacheinfo	rt_cacheinfo;
#endif

/** @} */
/**
 * @name Attribute: Nexthop
 * @{
 */

void rtnl_route_add_nexthop(struct rtnl_route *route, struct rtnl_nexthop *nh)
{
	nl_list_add_tail(&nh->rtnh_list, &route->rt_nexthops);
	route->rt_mask |= ROUTE_ATTR_MULTIPATH;
}

void rtnl_route_remove_nexthop(struct rtnl_nexthop *nh)
{
	nl_list_del(&nh->rtnh_list);
}

struct nl_list_head *rtnl_route_get_nexthops(struct rtnl_route *route)
{
	return &route->rt_nexthops;
}

/** @} */
	
/**
 * @name Routing Table Identifier Translations
 * @{
 */

static struct trans_tbl route_tables[] = {
	__ADD(RT_TABLE_UNSPEC, unspec)
	__ADD(RT_TABLE_DEFAULT, default)
	__ADD(RT_TABLE_MAIN, main)
	__ADD(RT_TABLE_LOCAL, local)
};

/**
 * Convert routing table identifier to character string.
 * @arg table		Routing table identifier.
 * @arg buf		Destination buffer
 * @arg size		Size of destination buffer.
 *
 * Converts a routing table identifier to a character string and stores
 * it in the specified destination buffer.
 *
 * @return The destination buffer or the type encoded in hexidecimal
 *         form if the routing table identifier is unknown.
 */
char *rtnl_route_table2str(int table, char *buf, size_t size)
{
	return __type2str(table, buf, size, route_tables,
			  ARRAY_SIZE(route_tables));
}

/**
 * Convert character string to routing table identifier.
 * @arg name		Name of routing table.
 *
 * Converts the provided character string specifying a routing table
 * identifier to the corresponding numeric value.
 *
 * @return Routing table identifier or a negative value if no match was found.
 */
int rtnl_route_str2table(const char *name)
{
	return __str2type(name, route_tables, ARRAY_SIZE(route_tables));
}


/** @} */

/**
 * @name Routing Protocol Translations
 * @{
 */

static struct trans_tbl route_protos[] = {
	__ADD(RTPROT_UNSPEC, unspec)
	__ADD(RTPROT_REDIRECT, redirect)
	__ADD(RTPROT_KERNEL, kernel)
	__ADD(RTPROT_BOOT, boot)
	__ADD(RTPROT_STATIC, static)
};

/**
 * Convert routing protocol identifier to character string.
 * @arg proto		Routing protocol identifier.
 * @arg buf		Destination buffer
 * @arg size		Size of destination buffer.
 *
 * Converts a routing protocol identifier to a character string and stores
 * it in the specified destination buffer.
 *
 * @return The destination buffer or the protocol encoded in hexidecimal
 *         form if the routing protocol is unknown.
 */
char *rtnl_route_proto2str(int proto, char *buf, size_t size)
{
	return __type2str(proto, buf, size, route_protos,
			  ARRAY_SIZE(route_protos));
}

/**
 * Convert character string to routing protocol identifier.
 * @arg name		Name of routing protocol.
 *
 * Converts the provided character string specifying a routing protocl
 * identifier to the corresponding numeric value.
 *
 * @return Routing protocol dentifier or a negative value if no match was found.
 */
int rtnl_route_str2proto(const char *name)
{
	return __str2type(name, route_protos, ARRAY_SIZE(route_protos));
}

/** @} */

/**
 * @name Routing Metrices Translations
 * @{
 */

static struct trans_tbl route_metrices[] = {
	__ADD(RTAX_UNSPEC, unspec)
	__ADD(RTAX_LOCK, lock)
	__ADD(RTAX_MTU, mtu)
	__ADD(RTAX_WINDOW, window)
	__ADD(RTAX_RTT, rtt)
	__ADD(RTAX_RTTVAR, rttvar)
	__ADD(RTAX_SSTHRESH, ssthresh)
	__ADD(RTAX_CWND, cwnd)
	__ADD(RTAX_ADVMSS, advmss)
	__ADD(RTAX_REORDERING, reordering)
	__ADD(RTAX_HOPLIMIT, hoplimit)
	__ADD(RTAX_INITCWND, initcwnd)
	__ADD(RTAX_FEATURES, features)
};

/**
 * Convert routing metric identifier to character string.
 * @arg metric		Routing metric identifier.
 * @arg buf		Destination buffer
 * @arg size		Size of destination buffer.
 *
 * Converts a routing metric identifier to a character string and stores
 * it in the specified destination buffer.
 *
 * @return The destination buffer or the metric encoded in hexidecimal
 *         form if the routing metric identifier is unknown.
 */
char *rtnl_route_metric2str(int metric, char *buf, size_t size)
{
	return __type2str(metric, buf, size, route_metrices,
			  ARRAY_SIZE(route_metrices));
}

/**
 * Convert character string to routing metric identifier.
 * @arg name		Name of routing metric.
 *
 * Converts the provided character string specifying a routing metric
 * identifier to the corresponding numeric value.
 *
 * @return Routing metric dentifier or a negative value if no match was found.
 */
int rtnl_route_str2metric(const char *name)
{
	return __str2type(name, route_metrices, ARRAY_SIZE(route_metrices));
}

/** @} */

/**
 * @name Nexthop Flags Translations
 * @{
 */

static struct trans_tbl nh_flags[] = {
	__ADD(RTNH_F_DEAD, dead)
	__ADD(RTNH_F_PERVASIVE, pervasive)
	__ADD(RTNH_F_ONLINK, onlink)
};

/**
 * Convert nexthop flags to a character string.
 * @arg flags		Nexthop flags.
 * @arg buf		Destination buffer.
 * @arg len		Length of destination buffer.
 *
 * Converts nexthop flags to a character string separated by
 * commas and stores it in the specified destination buffer.
 *
 * \return The destination buffer
 */
char * rtnl_route_nh_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, nh_flags, ARRAY_SIZE(nh_flags));
}

/**
 * Convert a character string to a nexthop flag
 * @arg name		Name of nexthop flag.
 *
 * Converts the provided character string specifying a nexthop
 * flag to the corresponding numeric value.
 *
 * \return Nexthop flag or a negative value if none was found.
 */
int rtnl_route_nh_str2flags(const char *name)
{
	return __str2flags(name, nh_flags, ARRAY_SIZE(nh_flags));
}

/** @} */

static struct nl_cache_ops rtnl_route_ops = {
	.co_name		= "route/route",
	.co_size		= sizeof(struct rtnl_route),
	.co_hdrsize		= sizeof(struct rtmsg),
	.co_msgtypes		= {
					{ RTM_NEWROUTE, "new" },
					{ RTM_DELROUTE, "delete" },
					{ RTM_GETROUTE, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= route_request_update,
	.co_msg_parser		= route_msg_parser,
	.co_constructor		= route_constructor,
	.co_free_data		= route_free_data,
	.co_dump[NL_DUMP_BRIEF]	= route_dump_brief,
	.co_dump[NL_DUMP_FULL]	= route_dump_full,
	.co_dump[NL_DUMP_STATS]	= route_dump_stats,
	.co_dump[NL_DUMP_XML]	= route_dump_xml,
	.co_dump[NL_DUMP_ENV]	= route_dump_env,
	.co_filter		= route_filter,
};

static void __init route_init(void)
{
	nl_cache_mngt_register(&rtnl_route_ops);
}

static void __exit route_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_route_ops);
}

/** @} */
