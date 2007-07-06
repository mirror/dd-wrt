/*
 * lib/route/neightbl.c         neighbour tables
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
 * @defgroup neightbl Neighbour Tables
 * @brief
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/neightbl.h>
#include <netlink/route/link.h>

/** @cond SKIP */
#define NEIGHTBL_ATTR_FAMILY       0x001
#define NEIGHTBL_ATTR_STATS        0x002
#define NEIGHTBL_ATTR_NAME	  0x004
#define NEIGHTBL_ATTR_THRESH1	  0x008
#define NEIGHTBL_ATTR_THRESH2	  0x010
#define NEIGHTBL_ATTR_THRESH3	  0x020
#define NEIGHTBL_ATTR_CONFIG	  0x040
#define NEIGHTBL_ATTR_PARMS	  0x080
#define NEIGHTBL_ATTR_GC_INTERVAL  0x100

#define NEIGHTBLPARM_ATTR_IFINDEX	0x0001
#define NEIGHTBLPARM_ATTR_REFCNT		0x0002
#define NEIGHTBLPARM_ATTR_QUEUE_LEN	0x0004
#define NEIGHTBLPARM_ATTR_APP_PROBES	0x0008
#define NEIGHTBLPARM_ATTR_UCAST_PROBES	0x0010
#define NEIGHTBLPARM_ATTR_MCAST_PROBES	0x0020
#define NEIGHTBLPARM_ATTR_PROXY_QLEN	0x0040
#define NEIGHTBLPARM_ATTR_REACHABLE_TIME	0x0080
#define NEIGHTBLPARM_ATTR_BASE_REACHABLE_TIME 0x0100
#define NEIGHTBLPARM_ATTR_RETRANS_TIME	0x0200
#define NEIGHTBLPARM_ATTR_GC_STALETIME	0x0400
#define NEIGHTBLPARM_ATTR_DELAY_PROBE_TIME 0x0800
#define NEIGHTBLPARM_ATTR_ANYCAST_DELAY	0x1000
#define NEIGHTBLPARM_ATTR_PROXY_DELAY	0x2000
#define NEIGHTBLPARM_ATTR_LOCKTIME	0x4000

static struct nl_cache_ops rtnl_neightbl_ops;
/** @endcond */

static int neightbl_filter(struct nl_object *obj, struct nl_object *filter)
{
	struct rtnl_neightbl *o = (struct rtnl_neightbl *) obj;
	struct rtnl_neightbl *f = (struct rtnl_neightbl *) filter;
	struct rtnl_neightbl_parms *op = &o->nt_parms;
	struct rtnl_neightbl_parms *fp = &f->nt_parms;

#define REQ(F) (f->nt_mask & NEIGHTBL_ATTR_##F)
#define AVAIL(F) (o->nt_mask & NEIGHTBL_ATTR_##F)
#define _O(F, EXPR) (REQ(F) && (!AVAIL(F) || (EXPR)))
#define _C(F, N) (REQ(F) && (!AVAIL(F) || (o->N != f->N)))
	
	if (_C(FAMILY,		nt_family)				||
	    _O(NAME,		strcmp(o->nt_name, f->nt_name))		||
	    _C(THRESH1, 	nt_gc_thresh1)   			||
	    _C(THRESH2, 	nt_gc_thresh2)   			||
	    _C(THRESH3, 	nt_gc_thresh3)				||
	    _C(GC_INTERVAL,	nt_gc_interval))
		return 0;
#undef REQ
#undef AVAIL
#undef _O
#undef _C

	if (!(f->nt_mask & NEIGHTBL_ATTR_PARMS) &&
	    !(o->nt_mask & NEIGHTBL_ATTR_PARMS))
		return 1;

#define REQ(F) (fp->ntp_mask & NEIGHTBLPARM_ATTR_##F)
#define AVAIL(F) (op->ntp_mask & NEIGHTBLPARM_ATTR_##F)
#define _C(F, N) (REQ(F) && (!AVAIL(F) || (op->N != fp->N)))
	if (_C(IFINDEX,			ntp_ifindex)			||
	    _C(QUEUE_LEN,		ntp_queue_len)			||
	    _C(APP_PROBES,		ntp_app_probes)			||
	    _C(UCAST_PROBES,		ntp_ucast_probes)		||
	    _C(MCAST_PROBES,		ntp_mcast_probes)		||
	    _C(PROXY_QLEN,		ntp_proxy_qlen)			||
	    _C(LOCKTIME,		ntp_locktime)			||
	    _C(RETRANS_TIME,		ntp_retrans_time)		||
	    _C(BASE_REACHABLE_TIME,	ntp_base_reachable_time)	||
	    _C(GC_STALETIME,		ntp_gc_stale_time)		||
	    _C(DELAY_PROBE_TIME,	ntp_probe_delay)		||
	    _C(ANYCAST_DELAY,		ntp_anycast_delay)		||
	    _C(PROXY_DELAY,		ntp_proxy_delay))
		return 0;
#undef REQ
#undef AVAIL
#undef _C

	return 1;
}

static struct nla_policy neightbl_policy[NDTA_MAX+1] = {
	[NDTA_NAME]		= { .type = NLA_STRING,
				    .maxlen = NTBLNAMSIZ },
	[NDTA_THRESH1]		= { .type = NLA_U32 },
	[NDTA_THRESH2]		= { .type = NLA_U32 },
	[NDTA_THRESH3]		= { .type = NLA_U32 },
	[NDTA_GC_INTERVAL]	= { .type = NLA_U32 },
	[NDTA_CONFIG]		= { .minlen = sizeof(struct ndt_config) },
	[NDTA_STATS]		= { .minlen = sizeof(struct ndt_stats) },
	[NDTA_PARMS]		= { .type = NLA_NESTED },
};

static int neightbl_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	struct rtnl_neightbl *ntbl;
	struct nlattr *tb[NDTA_MAX + 1];
	struct nl_parser_param *pp = arg;
	struct rtgenmsg *rtmsg;
	int err;

	ntbl = rtnl_neightbl_alloc();
	if (!ntbl) {
		err = nl_errno(ENOMEM);
		goto errout;
	}

	ntbl->ce_msgtype = n->nlmsg_type;
	rtmsg = nlmsg_data(n);
	
	err = nlmsg_parse(n, sizeof(*rtmsg), tb, NDTA_MAX, neightbl_policy);
	if (err < 0)
		goto errout;

	ntbl->nt_family = rtmsg->rtgen_family;

	if (tb[NDTA_NAME] == NULL) {
		err = nl_error(EINVAL, "NDTA_NAME is missing");
		goto errout;
	}

	nla_strlcpy(ntbl->nt_name, tb[NDTA_NAME], NTBLNAMSIZ);
	ntbl->nt_mask |= NEIGHTBL_ATTR_NAME;

	if (tb[NDTA_THRESH1]) {
		ntbl->nt_gc_thresh1 = nla_get_u32(tb[NDTA_THRESH1]);
		ntbl->nt_mask |= NEIGHTBL_ATTR_THRESH1;
	}

	if (tb[NDTA_THRESH2]) {
		ntbl->nt_gc_thresh2 = nla_get_u32(tb[NDTA_THRESH2]);
		ntbl->nt_mask |= NEIGHTBL_ATTR_THRESH2;
	}

	if (tb[NDTA_THRESH3]) {
		ntbl->nt_gc_thresh3 = nla_get_u32(tb[NDTA_THRESH3]);
		ntbl->nt_mask |= NEIGHTBL_ATTR_THRESH3;
	}

	if (tb[NDTA_GC_INTERVAL]) {
		ntbl->nt_gc_interval = nla_get_u32(tb[NDTA_GC_INTERVAL]);
		ntbl->nt_mask |= NEIGHTBL_ATTR_GC_INTERVAL;
	}

	if (tb[NDTA_CONFIG]) {
		nla_memcpy(&ntbl->nt_config, tb[NDTA_CONFIG],
			   sizeof(ntbl->nt_config));
		ntbl->nt_mask |= NEIGHTBL_ATTR_CONFIG;
	}

	if (tb[NDTA_STATS]) {
		nla_memcpy(&ntbl->nt_stats, tb[NDTA_STATS],
			   sizeof(ntbl->nt_stats));
		ntbl->nt_mask |= NEIGHTBL_ATTR_STATS;
	}

	if (tb[NDTA_PARMS]) {
		struct nlattr *tbp[NDTPA_MAX + 1];
		struct rtnl_neightbl_parms *p = &ntbl->nt_parms;

		err = nla_parse_nested(tbp, NDTPA_MAX, tb[NDTA_PARMS], NULL);
		if (err < 0)
			goto errout;

#define COPY_ENTRY(name, var) \
		if (tbp[NDTPA_ ##name]) { \
			p->ntp_ ##var = nla_get_u32(tbp[NDTPA_ ##name]); \
			p->ntp_mask |= NEIGHTBLPARM_ATTR_ ##name; \
		}

		COPY_ENTRY(IFINDEX, ifindex);
		COPY_ENTRY(REFCNT, refcnt);
		COPY_ENTRY(QUEUE_LEN, queue_len);
		COPY_ENTRY(APP_PROBES, app_probes);
		COPY_ENTRY(UCAST_PROBES, ucast_probes);
		COPY_ENTRY(MCAST_PROBES, mcast_probes);
		COPY_ENTRY(PROXY_QLEN, proxy_qlen);
		COPY_ENTRY(PROXY_DELAY, proxy_delay);
		COPY_ENTRY(ANYCAST_DELAY, anycast_delay);
		COPY_ENTRY(LOCKTIME, locktime);
		COPY_ENTRY(REACHABLE_TIME, reachable_time);
		COPY_ENTRY(BASE_REACHABLE_TIME, base_reachable_time);
		COPY_ENTRY(RETRANS_TIME, retrans_time);
		COPY_ENTRY(GC_STALETIME, gc_stale_time);
		COPY_ENTRY(DELAY_PROBE_TIME, probe_delay);
#undef COPY_ENTRY

		ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
	}

	err = pp->pp_cb((struct nl_object *) ntbl, pp);
	if (err < 0)
		goto errout;

	return P_ACCEPT;
errout:
	rtnl_neightbl_put(ntbl);
	return err;
}

static int neightbl_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETNEIGHTBL, AF_UNSPEC, NLM_F_DUMP);
}


static int neightbl_dump_brief(struct nl_object *arg, struct nl_dump_params *p)
{
	int line = 1;
	struct rtnl_neightbl *ntbl = (struct rtnl_neightbl *) arg;

	dp_dump(p, "%s", ntbl->nt_name);

	if (ntbl->nt_parms.ntp_mask & NEIGHTBLPARM_ATTR_IFINDEX) {
		struct nl_cache *link_cache;
		
		link_cache = nl_cache_mngt_require("route/link");

		if (link_cache) {
			char buf[32];
			dp_dump(p, "<%s> ",
				rtnl_link_i2name(link_cache,
						 ntbl->nt_parms.ntp_ifindex,
						 buf, sizeof(buf)));
		} else
			dp_dump(p, "<%u> ", ntbl->nt_parms.ntp_ifindex);
	} else
		dp_dump(p, " ");

	if (ntbl->nt_mask & NEIGHTBL_ATTR_CONFIG)
		dp_dump(p, "entries %u ", ntbl->nt_config.ndtc_entries);

	if (ntbl->nt_mask & NEIGHTBL_ATTR_PARMS) {
		char rt[32], rt2[32];
		struct rtnl_neightbl_parms *pa = &ntbl->nt_parms;

		dp_dump(p, "reachable-time %s retransmit-time %s",
			nl_msec2str(pa->ntp_reachable_time, rt, sizeof(rt)),
			nl_msec2str(pa->ntp_retrans_time, rt2, sizeof(rt2)));
	}

	dp_dump(p, "\n");

	return line;
}

static int neightbl_dump_full(struct nl_object *arg, struct nl_dump_params *p)
{
	char x[32], y[32], z[32];
	struct rtnl_neightbl *ntbl = (struct rtnl_neightbl *) arg;

	int line = neightbl_dump_brief(arg, p);

	if (ntbl->nt_mask & NEIGHTBL_ATTR_CONFIG) {
		dp_new_line(p, line++);
		dp_dump(p, "    key-len %u entry-size %u last-flush %s\n",
			ntbl->nt_config.ndtc_key_len,
			ntbl->nt_config.ndtc_entry_size,
			nl_msec2str(ntbl->nt_config.ndtc_last_flush,
				      x, sizeof(x)));

		dp_new_line(p, line++);
		dp_dump(p, "    gc threshold %u/%u/%u interval %s " \
			    "chain-position %u\n",
			ntbl->nt_gc_thresh1, ntbl->nt_gc_thresh2,
			ntbl->nt_gc_thresh3,
			nl_msec2str(ntbl->nt_gc_interval, x, sizeof(x)),
			ntbl->nt_config.ndtc_hash_chain_gc);

		dp_new_line(p, line++);
		dp_dump(p, "    hash-rand 0x%08X/0x%08X last-rand %s\n",
			ntbl->nt_config.ndtc_hash_rnd,
			ntbl->nt_config.ndtc_hash_mask,
			nl_msec2str(ntbl->nt_config.ndtc_last_rand,
				      x, sizeof(x)));
	}

	if (ntbl->nt_mask & NEIGHTBL_ATTR_PARMS) {
		struct rtnl_neightbl_parms *pa = &ntbl->nt_parms;

		dp_new_line(p, line++);
		dp_dump(p, "    refcnt %u pending-queue-limit %u " \
			    "proxy-delayed-queue-limit %u\n",
			pa->ntp_refcnt,
			pa->ntp_queue_len,
			pa->ntp_proxy_qlen);

		dp_new_line(p, line++);
		dp_dump(p, "    num-userspace-probes %u num-unicast-probes " \
			    "%u num-multicast-probes %u\n",
			pa->ntp_app_probes,
			pa->ntp_ucast_probes,
			pa->ntp_mcast_probes);

		dp_new_line(p, line++);
		dp_dump(p, "    min-age %s base-reachable-time %s " \
			    "stale-check-interval %s\n",
			nl_msec2str(pa->ntp_locktime, x, sizeof(x)),
			nl_msec2str(pa->ntp_base_reachable_time,
				      y, sizeof(y)),
			nl_msec2str(pa->ntp_gc_stale_time, z, sizeof(z)));

		dp_new_line(p, line++);
		dp_dump(p, "    initial-probe-delay %s answer-delay %s " \
			    "proxy-answer-delay %s\n",
			nl_msec2str(pa->ntp_probe_delay, x, sizeof(x)),
			nl_msec2str(pa->ntp_anycast_delay, y, sizeof(y)),
			nl_msec2str(pa->ntp_proxy_delay, z, sizeof(z)));
	}

	return line;
}

static int neightbl_dump_stats(struct nl_object *arg, struct nl_dump_params *p)
{
	struct rtnl_neightbl *ntbl = (struct rtnl_neightbl *) arg;
	int line = neightbl_dump_full(arg, p);

	if (!(ntbl->nt_mask & NEIGHTBL_ATTR_STATS))
		return line;

	dp_new_line(p, line++);
	dp_dump(p, "    lookups %lld hits %lld failed %lld " \
		    "allocations %lld destroys %lld\n",
		ntbl->nt_stats.ndts_lookups,
		ntbl->nt_stats.ndts_hits,
		ntbl->nt_stats.ndts_res_failed,
		ntbl->nt_stats.ndts_allocs,
		ntbl->nt_stats.ndts_destroys);

	dp_new_line(p, line++);
	dp_dump(p, "    hash-grows %lld forced-gc-runs %lld " \
		    "periodic-gc-runs %lld\n",
		ntbl->nt_stats.ndts_hash_grows,
		ntbl->nt_stats.ndts_forced_gc_runs,
		ntbl->nt_stats.ndts_periodic_gc_runs);

	dp_dump(p, "    rcv-unicast-probes %lld rcv-multicast-probes %lld\n",
		ntbl->nt_stats.ndts_rcv_probes_ucast,
		ntbl->nt_stats.ndts_rcv_probes_mcast);

	return line;
}

/**
 * @name Neighbour Table Object Allocation/Freeage
 * @{
 */

/**
 * Allocate a new neighbour table object
 * @return New neighbour table object
 */
struct rtnl_neightbl *rtnl_neightbl_alloc(void)
{
	return (struct rtnl_neightbl *)
			nl_object_alloc_from_ops(&rtnl_neightbl_ops);
}

/**
 * Give back reference on neighbour table object.
 * @arg neightbl	Neighbour  table object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_neightbl_put(struct rtnl_neightbl *neightbl)
{
	nl_object_put((struct nl_object *) neightbl);
}
/**
 * Free neighbour table object.
 * @arg neightbl	Neighbour table object to be freed.
 *
 * @note Always use rtnl_neightbl_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_neightbl_free(struct rtnl_neightbl *neightbl)
{
	nl_object_free((struct nl_object *) neightbl);
}

/** @} */

/**
 * @name Neighbour Table Cache Management
 * @{
 */

/**
 * Build a neighbour table cache including all neighbour tables currently configured in the kernel.
 * @arg handle		netlink handle
 *
 * Allocates a new neighbour table cache, initializes it properly and
 * updates it to include all neighbour tables currently configured in
 * the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it.
 * @return The new cache or NULL if an error occured.
 */
struct nl_cache * rtnl_neightbl_alloc_cache(struct nl_handle *handle)
{
	struct nl_cache * cache;
	
	cache = nl_cache_alloc_from_ops(&rtnl_neightbl_ops);
	if (cache == NULL)
		return NULL;
	
	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Lookup neighbour table by name and optional interface index
 * @arg cache		neighbour table cache
 * @arg name		name of table
 * @arg ifindex		optional interface index
 *
 * Looks up the neighbour table matching the specified name and
 * optionally the specified ifindex to retrieve device specific
 * parameter sets.
 *
 * @return ptr to neighbour table inside the cache or NULL if no
 *         match was found.
 */
struct rtnl_neightbl *rtnl_neightbl_get(struct nl_cache *cache,
					const char *name, int ifindex)
{
	struct rtnl_neightbl *nt;

	if (cache->c_ops != &rtnl_neightbl_ops)
		return NULL;

	nl_list_for_each_entry(nt, &cache->c_items, ce_list) {
		if (!strcasecmp(nt->nt_name, name) &&
		    ((!ifindex && !nt->nt_parms.ntp_ifindex) ||
		     (ifindex && ifindex == nt->nt_parms.ntp_ifindex))) {
			nl_object_get((struct nl_object *) nt);
			return nt;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Neighbour Table Modifications
 * @{
 */

/**
 * Builds a netlink change request message to change neighbour table attributes
 * @arg old		neighbour table to change
 * @arg tmpl		template with requested changes
 *
 * Builds a new netlink message requesting a change of neighbour table
 * attributes. The netlink message header isn't fully equipped with all
 * relevant fields and must be sent out via nl_send_auto_complete() or
 * supplemented as needed.
 * \a old must point to a neighbour table currently configured in the
 * kernel and \a tmpl must contain the attributes to be changed set via
 * \c rtnl_neightbl_set_* functions.
 *
 * @return New netlink message
 */
struct nl_msg * rtnl_neightbl_build_change_request(struct rtnl_neightbl *old,
						   struct rtnl_neightbl *tmpl)
{
	struct nl_msg *m;
	struct ndtmsg ndt = {
		.ndtm_family = old->nt_family,
	};

	m = nlmsg_build_simple(RTM_SETNEIGHTBL, 0);
	nlmsg_append(m, &ndt, sizeof(ndt), 1);

	nla_put_string(m, NDTA_NAME, old->nt_name);

	if (tmpl->nt_mask & NEIGHTBL_ATTR_THRESH1)
		nla_put_u32(m, NDTA_THRESH1, tmpl->nt_gc_thresh1);

	if (tmpl->nt_mask & NEIGHTBL_ATTR_THRESH2)
		nla_put_u32(m, NDTA_THRESH2, tmpl->nt_gc_thresh2);

	if (tmpl->nt_mask & NEIGHTBL_ATTR_THRESH2)
		nla_put_u32(m, NDTA_THRESH2, tmpl->nt_gc_thresh2);

	if (tmpl->nt_mask & NEIGHTBL_ATTR_GC_INTERVAL)
		nla_put_u64(m, NDTA_GC_INTERVAL,
				      tmpl->nt_gc_interval);

	if (tmpl->nt_mask & NEIGHTBL_ATTR_PARMS) {
		struct rtnl_neightbl_parms *p = &tmpl->nt_parms;
		struct nl_msg *parms = nlmsg_build(NULL);

		if (old->nt_parms.ntp_mask & NEIGHTBLPARM_ATTR_IFINDEX)
			nla_put_u32(parms, NDTPA_IFINDEX,
					      old->nt_parms.ntp_ifindex);


		if (p->ntp_mask & NEIGHTBLPARM_ATTR_QUEUE_LEN)
			nla_put_u32(parms, NDTPA_QUEUE_LEN, p->ntp_queue_len);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_APP_PROBES)
			nla_put_u32(parms, NDTPA_APP_PROBES, p->ntp_app_probes);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_UCAST_PROBES)
			nla_put_u32(parms, NDTPA_UCAST_PROBES,
				    p->ntp_ucast_probes);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_MCAST_PROBES)
			nla_put_u32(parms, NDTPA_MCAST_PROBES,
				    p->ntp_mcast_probes);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_PROXY_QLEN)
			nla_put_u32(parms, NDTPA_PROXY_QLEN,
				    p->ntp_proxy_qlen);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_BASE_REACHABLE_TIME)
			nla_put_u64(parms, NDTPA_BASE_REACHABLE_TIME,
				    p->ntp_base_reachable_time);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_RETRANS_TIME)
			nla_put_u64(parms, NDTPA_RETRANS_TIME,
				    p->ntp_retrans_time);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_GC_STALETIME)
			nla_put_u64(parms, NDTPA_GC_STALETIME,
				    p->ntp_gc_stale_time);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_DELAY_PROBE_TIME)
			nla_put_u64(parms, NDTPA_DELAY_PROBE_TIME,
				    p->ntp_proxy_delay);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_ANYCAST_DELAY)
			nla_put_u64(parms, NDTPA_ANYCAST_DELAY,
				    p->ntp_anycast_delay);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_PROXY_DELAY)
			nla_put_u64(parms, NDTPA_PROXY_DELAY,
					      p->ntp_proxy_delay);

		if (p->ntp_mask & NEIGHTBLPARM_ATTR_LOCKTIME)
			nla_put_u64(parms, NDTPA_LOCKTIME, p->ntp_locktime);

		nla_put_nested(m, NDTA_PARMS, parms);
		nlmsg_free(parms);
	}
	
	return m;
}

/**
 * Change neighbour table attributes
 * @arg handle		netlink handle
 * @arg old		neighbour table to be changed
 * @arg tmpl		template with requested changes
 *
 * Builds a new netlink message by calling
 * rtnl_neightbl_build_change_request(), sends the request to the
 * kernel and waits for the next ACK to be received, i.e. blocks
 * until the request has been processed.
 *
 * @return 0 on success or a negative error code
 */
int rtnl_neightbl_change(struct nl_handle *handle, struct rtnl_neightbl *old,
			 struct rtnl_neightbl *tmpl)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_neightbl_build_change_request(old, tmpl);
	err = nl_send_auto_complete(handle, msg);
	if (err < 0)
		return err;

	nlmsg_free(msg);
	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set the address family of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg family		new address family
 */
void rtnl_neightbl_set_family(struct rtnl_neightbl *ntbl, int family)
{
	ntbl->nt_family = family;
	ntbl->nt_mask |= NEIGHTBL_ATTR_FAMILY;
}

/**
 * Set the gc interval of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new interval in milliseconds
 */
void rtnl_neightbl_set_gc_interval(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_gc_interval = ms;
	ntbl->nt_mask |= NEIGHTBL_ATTR_GC_INTERVAL;
}

/**
 * Set the gc threshold 1 of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg thresh		new gc threshold
 */
void rtnl_neightbl_set_gc_tresh1(struct rtnl_neightbl *ntbl, int thresh)
{
	ntbl->nt_gc_thresh1 = thresh;
	ntbl->nt_mask |= NEIGHTBL_ATTR_THRESH1;
}

/**
 * Set the gc threshold 2 of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg thresh		new gc threshold
 */
void rtnl_neightbl_set_gc_tresh2(struct rtnl_neightbl *ntbl, int thresh)
{
	ntbl->nt_gc_thresh2 = thresh;
	ntbl->nt_mask |= NEIGHTBL_ATTR_THRESH2;
}

/**
 * Set the gc threshold 3 of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg thresh		new gc threshold
 */
void rtnl_neightbl_set_gc_tresh3(struct rtnl_neightbl *ntbl, int thresh)
{
	ntbl->nt_gc_thresh3 = thresh;
	ntbl->nt_mask |= NEIGHTBL_ATTR_THRESH3;
}

/**
 * Set the table name of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg name		new table name
 */
void rtnl_neightbl_set_name(struct rtnl_neightbl *ntbl, const char *name)
{
	strncpy(ntbl->nt_name, name, sizeof(ntbl->nt_name) - 1);
	ntbl->nt_mask |= NEIGHTBL_ATTR_NAME;
}

/**
 * Set the device of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ifindex		interface index of the device
 */
void rtnl_neightbl_set_dev(struct rtnl_neightbl *ntbl, int ifindex)
{
	ntbl->nt_parms.ntp_ifindex = ifindex;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_IFINDEX;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the queue length for pending requests of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg len		new queue len
 */
void rtnl_neightbl_set_queue_len(struct rtnl_neightbl *ntbl, int len)
{
	ntbl->nt_parms.ntp_queue_len = len;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_QUEUE_LEN;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the queue length for delay proxy arp requests of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg len		new queue len
 */
void rtnl_neightbl_set_proxy_queue_len(struct rtnl_neightbl *ntbl, int len)
{
	ntbl->nt_parms.ntp_proxy_qlen = len;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_PROXY_QLEN;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the number of application probes of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg probes		new probes value
 */
void rtnl_neightbl_set_app_probes(struct rtnl_neightbl *ntbl, int probes)
{
	ntbl->nt_parms.ntp_app_probes = probes;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_APP_PROBES;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the number of unicast probes of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg probes		new probes value
 */
void rtnl_neightbl_set_ucast_probes(struct rtnl_neightbl *ntbl, int probes)
{
	ntbl->nt_parms.ntp_ucast_probes = probes;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_UCAST_PROBES;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the number of multicast probes of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg probes		new probes value
 */
void rtnl_neightbl_set_mcast_probes(struct rtnl_neightbl *ntbl, int probes)
{
	ntbl->nt_parms.ntp_mcast_probes = probes;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_MCAST_PROBES;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the base reachable time of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new base reachable time in milliseconds
 */
void rtnl_neightbl_set_base_reachable_time(struct rtnl_neightbl *ntbl,
					   uint64_t ms)
{
	ntbl->nt_parms.ntp_base_reachable_time = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_BASE_REACHABLE_TIME;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the retransmit time of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new retransmit time
 */
void rtnl_neightbl_set_retrans_time(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_parms.ntp_retrans_time = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_RETRANS_TIME;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the gc stale time of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new gc stale time in milliseconds
 */
void rtnl_neightbl_set_gc_stale_time(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_parms.ntp_gc_stale_time = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_GC_STALETIME;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the first probe delay time of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new first probe delay time in milliseconds
 */
void rtnl_neightbl_set_delay_probe_time(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_parms.ntp_probe_delay = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_DELAY_PROBE_TIME;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the anycast delay of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new anycast delay in milliseconds
 */
void rtnl_neightbl_set_anycast_delay(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_parms.ntp_anycast_delay = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_ANYCAST_DELAY;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the proxy delay of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new proxy delay in milliseconds
 */
void rtnl_neightbl_set_proxy_delay(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_parms.ntp_proxy_delay = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_PROXY_DELAY;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/**
 * Set the locktime of a neighbour table to the specified value
 * @arg ntbl		neighbour table to change
 * @arg ms		new locktime in milliseconds
 */
void rtnl_neightbl_set_locktime(struct rtnl_neightbl *ntbl, uint64_t ms)
{
	ntbl->nt_parms.ntp_locktime = ms;
	ntbl->nt_parms.ntp_mask |= NEIGHTBLPARM_ATTR_LOCKTIME;
	ntbl->nt_mask |= NEIGHTBL_ATTR_PARMS;
}

/** @} */

static struct nl_cache_ops rtnl_neightbl_ops = {
	.co_name		= "route/neightbl",
	.co_size		= sizeof(struct rtnl_neightbl),
	.co_hdrsize		= sizeof(struct rtgenmsg),
	.co_msgtypes		= {
					{ RTM_NEWNEIGHTBL, "new" },
					{ RTM_SETNEIGHTBL, "set" },
					{ RTM_GETNEIGHTBL, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= &neightbl_request_update,
	.co_msg_parser		= &neightbl_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= &neightbl_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &neightbl_dump_full,
	.co_dump[NL_DUMP_STATS]	= &neightbl_dump_stats,
	.co_filter		= &neightbl_filter,
};

static void __init neightbl_init(void)
{
	nl_cache_mngt_register(&rtnl_neightbl_ops);
}

static void __exit neightbl_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_neightbl_ops);
}

/** @} */
