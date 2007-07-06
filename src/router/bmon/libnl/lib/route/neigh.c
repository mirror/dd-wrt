/*
 * lib/route/neigh.c	Neighbours
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
 * @defgroup neigh Neighbours
 * @brief
 *
 * The neighbour table establishes bindings between protocol addresses and
 * link layer addresses for hosts sharing the same physical link. This
 * module allows you to access and manipulate the content of these tables.
 *
 * @par Neighbour States
 * @code
 * NUD_INCOMPLETE
 * NUD_REACHABLE
 * NUD_STALE
 * NUD_DELAY
 * NUD_PROBE
 * NUD_FAILED
 * NUD_NOARP
 * NUD_PERMANENT
 * @endcode
 *
 * @par Neighbour Flags
 * @code
 * NTF_PROXY
 * NTF_ROUTER
 * @endcode
 *
 * @par Neighbour Identification
 * A neighbour is uniquely identified by the attributes listed below, whenever
 * you refer to an existing neighbour all of the attributes must be set.
 * Neighbours from caches automatically have all required attributes set.
 *   - interface index (rtnl_neigh_set_ifindex())
 *   - destination address (rtnl_neigh_set_dst())
 *
 * @par Changeable Attributes
 * \anchor neigh_changeable
 *  - state (rtnl_neigh_set_state())
 *  - link layer address (rtnl_neigh_set_lladdr())
 *
 * @par Required Caches for Dumping
 * In order to dump neighbour attributes you must provide the following
 * caches via nl_cache_provide()
 *  - link cache holding all links
 *
 * @par TODO
 *   - Document proxy settings
 *   - Document states and their influence
 *
 * @par 1) Retrieving information about configured neighbours
 * @code
 * // The first step is to retrieve a list of all available neighbour within
 * // the kernel and put them into a cache.
 * struct nl_cache *cache = rtnl_neigh_alloc_cache(handle);
 *
 * // Neighbours can then be looked up by the interface and destination
 * // address:
 * struct rtnl_neigh *neigh = rtnl_neigh_get(cache, ifindex, dst_addr);
 * 
 * // After successful usage, the object must be given back to the cache
 * rtnl_neigh_put(neigh);
 * @endcode
 *
 * @par 2) Adding new neighbours
 * @code
 * // Allocate an empty neighbour handle to be filled out with the attributes
 * // of the new neighbour.
 * struct rtnl_neigh *neigh = rtnl_neigh_alloc();
 *
 * // Fill out the attributes of the new neighbour
 * rtnl_neigh_set_ifindex(neigh, ifindex);
 * rtnl_neigh_set_dst(neigh, dst_addr);
 * rtnl_neigh_set_state(neigh, rtnl_neigh_str2state("permanent"));
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_neigh_build_add_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_neigh_add(nl_handle, neigh, NLM_F_REPLACE);
 *
 * // Free the memory
 * rtnl_neigh_put(neigh);
 * @endcode
 *
 * @par 3) Deleting an existing neighbour
 * @code
 * // Allocate an empty neighbour object to be filled out with the attributes
 * // matching the neighbour to be deleted. Alternatively a fully equipped
 * // neighbour object out of a cache can be used instead.
 * struct rtnl_neigh *neigh = rtnl_neigh_alloc();
 *
 * // Neighbours are uniquely identified by their interface index and
 * // destination address, you may fill out other attributes but they
 * // will have no influence.
 * rtnl_neigh_set_ifindex(neigh, ifindex);
 * rtnl_neigh_set_dst(neigh, dst_addr);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_neigh_build_delete_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_neigh_delete(handle, neigh, 0);
 *
 * // Free the memory
 * rtnl_neigh_put(neigh);
 * @endcode
 *
 * @par 4) Changing neighbour attributes
 * @code
 * // Allocate an empty neighbour object to be filled out with the attributes
 * // matching the neighbour to be changed and the new parameters. Alternatively
 * // a fully equipped modified neighbour object out of a cache can be used.
 * struct rtnl_neigh *neigh = rtnl_neigh_alloc();
 *
 * // Identify the neighbour to be changed by its interface index and
 * // destination address
 * rtnl_neigh_set_ifindex(neigh, ifindex);
 * rtnl_neigh_set_dst(neigh, dst_addr);
 *
 * // The link layer address may be modified, if so it is wise to change
 * // its state to "permanent" in order to avoid having it overwritten.
 * rtnl_neigh_set_lladdr(neigh, lladdr);
 *
 * // Secondly the state can be modified allowing normal neighbours to be
 * // converted into permanent entries or to manually confirm a neighbour.
 * rtnl_neigh_set_state(neigh, state);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_neigh_build_change_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_neigh_change(handle, neigh, 0);
 *
 * // Free the memory
 * rtnl_neigh_put(neigh);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/link.h>

/** @cond SKIP */
#define NEIGH_ATTR_FLAGS        0x01
#define NEIGH_ATTR_STATE        0x02
#define NEIGH_ATTR_LLADDR       0x04
#define NEIGH_ATTR_DST          0x08
#define NEIGH_ATTR_CACHEINFO    0x10
#define NEIGH_ATTR_IFINDEX      0x20
#define NEIGH_ATTR_FAMILY       0x40
#define NEIGH_ATTR_TYPE         0x80
#define NEIGH_ATTR_PROBES       0x100

static struct nl_cache_ops rtnl_neigh_ops;
/** @endcond */

static void neigh_free_data(struct nl_object *c)
{
	struct rtnl_neigh *neigh = nl_object_priv(c);

	if (!neigh)
		return;

	nl_addr_put(neigh->n_lladdr);
	nl_addr_put(neigh->n_dst);
}

static int neigh_filter(struct nl_object *obj, struct nl_object *filter)
{
	struct rtnl_neigh *o = (struct rtnl_neigh *) obj;
	struct rtnl_neigh *f = (struct rtnl_neigh *) filter;

#define REQ(F) (f->n_mask & NEIGH_ATTR_##F)
#define AVAIL(F) (o->n_mask & NEIGH_ATTR_##F)
#define _O(F, EXPR) (REQ(F) && (!AVAIL(F) || (EXPR)))
#define _C(F, N) (REQ(F) && (!AVAIL(F) || (o->N != f->N)))
	if (_C(IFINDEX,	n_ifindex)					||
	    _C(FAMILY,	n_family)					||
	    _C(TYPE,	n_type)						||
	    _O(LLADDR,	nl_addr_cmp(o->n_lladdr, f->n_lladdr))		||
	    _O(DST,	nl_addr_cmp(o->n_dst, f->n_dst))		||
	    _O(STATE,	f->n_state ^ (o->n_state & f->n_state_mask))	||
	    _O(FLAGS,	f->n_flags ^ (o->n_flags & f->n_flag_mask)))
		return 0;
#undef REQ
#undef AVAIL
#undef _O
#undef _C

	return 1;
}

static struct nla_policy neigh_policy[NDA_MAX+1] = {
	[NDA_CACHEINFO]	= { .minlen = sizeof(struct nda_cacheinfo) },
	[NDA_PROBES]	= { .type = NLA_U32 },
};

static int neigh_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	struct rtnl_neigh *neigh;
	struct nlattr *tb[NDA_MAX + 1];
	struct nl_parser_param *pp = arg;
	struct ndmsg *nm;
	int err;

	neigh = rtnl_neigh_alloc();
	if (!neigh) {
		err = nl_errno(ENOMEM);
		goto errout;
	}

	neigh->ce_msgtype = n->nlmsg_type;
	nm = nlmsg_data(n);

	err = nlmsg_parse(n, sizeof(*nm), tb, NDA_MAX, neigh_policy);
	if (err < 0)
		goto errout;

	neigh->n_family  = nm->ndm_family;
	neigh->n_ifindex = nm->ndm_ifindex;
	neigh->n_state   = nm->ndm_state;
	neigh->n_flags   = nm->ndm_flags;
	neigh->n_type    = nm->ndm_type;

	neigh->n_mask |= (NEIGH_ATTR_FAMILY | NEIGH_ATTR_IFINDEX |
			  NEIGH_ATTR_STATE | NEIGH_ATTR_FLAGS |
			  NEIGH_ATTR_TYPE);

	if (tb[NDA_LLADDR]) {
		neigh->n_lladdr = nla_get_addr(tb[NDA_LLADDR], AF_UNSPEC);
		if (!neigh->n_lladdr)
			goto errout;
		nl_addr_set_family(neigh->n_lladdr,
				   nl_addr_guess_family(neigh->n_lladdr));
		neigh->n_mask |= NEIGH_ATTR_LLADDR;
	}

	if (tb[NDA_DST]) {
		neigh->n_dst = nla_get_addr(tb[NDA_DST], neigh->n_family);
		if (!neigh->n_dst)
			goto errout;
		neigh->n_mask |= NEIGH_ATTR_DST;
	}

	if (tb[NDA_CACHEINFO]) {
		struct nda_cacheinfo *ci = nla_data(tb[NDA_CACHEINFO]);

		neigh->n_cacheinfo.nci_confirmed = ci->ndm_confirmed;
		neigh->n_cacheinfo.nci_used = ci->ndm_used;
		neigh->n_cacheinfo.nci_updated = ci->ndm_updated;
		neigh->n_cacheinfo.nci_refcnt = ci->ndm_refcnt;
		
		neigh->n_mask |= NEIGH_ATTR_CACHEINFO;
	}

	if (tb[NDA_PROBES]) {
		neigh->n_probes = nla_get_u32(tb[NDA_PROBES]);
		neigh->n_mask |= NEIGH_ATTR_PROBES;
	}

	err = pp->pp_cb((struct nl_object *) neigh, pp);
	if (err < 0)
		goto errout;

	return P_ACCEPT;

errout:
	rtnl_neigh_put(neigh);
	return err;
}

static int neigh_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETNEIGH, AF_UNSPEC, NLM_F_DUMP);
}


static int neigh_dump_brief(struct nl_object *a, struct nl_dump_params *p)
{
	char dst[INET6_ADDRSTRLEN+5], lladdr[INET6_ADDRSTRLEN+5];
	struct rtnl_neigh *n = (struct rtnl_neigh *) a;
	struct nl_cache *link_cache;
	char state[128], flags[64];

	link_cache = nl_cache_mngt_require("route/link");

	dp_dump(p, "%s ", nl_addr2str(n->n_dst, dst, sizeof(dst)));

	if (link_cache)
		dp_dump(p, "dev %s ",
			rtnl_link_i2name(link_cache, n->n_ifindex,
					 state, sizeof(state)));
	else
		dp_dump(p, "dev %d ", n->n_ifindex);

	if (n->n_mask & NEIGH_ATTR_LLADDR)
		dp_dump(p, "lladdr %s ",
			nl_addr2str(n->n_lladdr, lladdr, sizeof(lladdr)));

	rtnl_neigh_state2str(n->n_state, state, sizeof(state));
	rtnl_neigh_flags2str(n->n_flags, flags, sizeof(flags));

	if (state[0])
		dp_dump(p, "<%s", state);
	if (flags[0])
		dp_dump(p, "%s%s", state[0] ? "," : "<", flags);
	if (state[0] || flags[0])
		dp_dump(p, ">");
	dp_dump(p, "\n");

	return 1;
}

static int neigh_dump_full(struct nl_object *a, struct nl_dump_params *p)
{
	char rtn_type[32];
	struct rtnl_neigh *n = (struct rtnl_neigh *) a;
	int hz = nl_get_hz();

	int line = neigh_dump_brief(a, p);

	dp_dump_line(p, line++, "    refcnt %u type %s confirmed %u used "
				"%u updated %u\n",
		n->n_cacheinfo.nci_refcnt,
		nl_rtntype2str(n->n_type, rtn_type, sizeof(rtn_type)),
		n->n_cacheinfo.nci_confirmed/hz,
		n->n_cacheinfo.nci_used/hz, n->n_cacheinfo.nci_updated/hz);

	return line;
}

static int neigh_dump_stats(struct nl_object *a, struct nl_dump_params *p)
{
	return neigh_dump_full(a, p);
}

static int neigh_dump_xml(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_neigh *neigh = (struct rtnl_neigh *) obj;
	char buf[128];
	int line = 0;

	dp_dump_line(p, line++, "<neighbour>\n");
	dp_dump_line(p, line++, "  <family>%s</family>\n",
		     nl_af2str(neigh->n_family, buf, sizeof(buf)));

	if (neigh->n_mask & NEIGH_ATTR_LLADDR)
		dp_dump_line(p, line++, "  <lladdr>%s</lladdr>\n",
			     nl_addr2str(neigh->n_lladdr, buf, sizeof(buf)));

	if (neigh->n_mask & NEIGH_ATTR_DST)
		dp_dump_line(p, line++, "  <dst>%s</dst>\n",
			     nl_addr2str(neigh->n_dst, buf, sizeof(buf)));

	if (neigh->n_mask & NEIGH_ATTR_IFINDEX) {
		struct nl_cache *link_cache;
	
		link_cache = nl_cache_mngt_require("route/link");

		if (link_cache)
			dp_dump_line(p, line++, "  <device>%s</device>\n",
				     rtnl_link_i2name(link_cache,
						      neigh->n_ifindex,
						      buf, sizeof(buf)));
		else
			dp_dump_line(p, line++, "  <device>%u</device>\n",
				     neigh->n_ifindex);
	}

	if (neigh->n_mask & NEIGH_ATTR_PROBES)
		dp_dump_line(p, line++, "  <probes>%u</probes>\n",
			     neigh->n_probes);

	if (neigh->n_mask & NEIGH_ATTR_TYPE)
		dp_dump_line(p, line++, "  <type>%s</type>\n",
			     nl_rtntype2str(neigh->n_type, buf, sizeof(buf)));

	rtnl_neigh_flags2str(neigh->n_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "  <flags>%s</flags>\n", buf);

	rtnl_neigh_state2str(neigh->n_state, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "  <state>%s</state>\n", buf);

	dp_dump_line(p, line++, "</neighbour>\n");

#if 0
	struct rtnl_ncacheinfo n_cacheinfo;
#endif

	return line;
}

static int neigh_dump_env(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_neigh *neigh = (struct rtnl_neigh *) obj;
	char buf[128];
	int line = 0;

	dp_dump_line(p, line++, "NEIGH_FAMILY=%s\n",
		     nl_af2str(neigh->n_family, buf, sizeof(buf)));

	if (neigh->n_mask & NEIGH_ATTR_LLADDR)
		dp_dump_line(p, line++, "NEIGHT_LLADDR=%s\n",
			     nl_addr2str(neigh->n_lladdr, buf, sizeof(buf)));

	if (neigh->n_mask & NEIGH_ATTR_DST)
		dp_dump_line(p, line++, "NEIGH_DST=%s\n",
			     nl_addr2str(neigh->n_dst, buf, sizeof(buf)));

	if (neigh->n_mask & NEIGH_ATTR_IFINDEX) {
		struct nl_cache *link_cache;

		dp_dump_line(p, line++, "NEIGH_IFINDEX=%u\n",
			     neigh->n_ifindex);

		link_cache = nl_cache_mngt_require("route/link");
		if (link_cache)
			dp_dump_line(p, line++, "NEIGH_IFNAME=%s\n",
				     rtnl_link_i2name(link_cache,
						      neigh->n_ifindex,
						      buf, sizeof(buf)));
	}

	if (neigh->n_mask & NEIGH_ATTR_PROBES)
		dp_dump_line(p, line++, "NEIGH_PROBES=%u\n",
			     neigh->n_probes);

	if (neigh->n_mask & NEIGH_ATTR_TYPE)
		dp_dump_line(p, line++, "NEIGH_TYPE=%s\n",
			     nl_rtntype2str(neigh->n_type, buf, sizeof(buf)));

	rtnl_neigh_flags2str(neigh->n_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "NEIGH_FLAGS=%s\n", buf);

	rtnl_neigh_state2str(neigh->n_state, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "NEIGH_STATE=%s\n", buf);

	return line;
}

/**
 * @name Neighbour Object Allocation/Freeage
 * @{
 */

/**
 * Allocate a new neighbour object
 * @return New neighbour object
 */
struct rtnl_neigh *rtnl_neigh_alloc(void)
{
	return (struct rtnl_neigh *) nl_object_alloc_from_ops(&rtnl_neigh_ops);
}

/**
 * Give back reference on neighbour object.
 * @arg neigh		Neighbour  object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_neigh_put(struct rtnl_neigh *neigh)
{
	nl_object_put((struct nl_object *) neigh);
}
/**
 * Free neighbour object.
 * @arg neigh		Neighbour object to be freed.
 *
 * @note Always use rtnl_neigh_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_neigh_free(struct rtnl_neigh *neigh)
{
	nl_object_free((struct nl_object *) neigh);
}

/** @} */

/**
 * @name Neighbour Cache Managament
 * @{
 */

/**
 * Build a neighbour cache including all neighbours currently configured in the kernel.
 * @arg handle		netlink handle
 *
 * Allocates a new neighbour cache, initializes it properly and updates it
 * to include all neighbours currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it.
 * @return The new cache or NULL if an error occured.
 */
struct nl_cache *rtnl_neigh_alloc_cache(struct nl_handle *handle)
{
	struct nl_cache *cache = nl_cache_alloc_from_ops(&rtnl_neigh_ops);

	if (cache == NULL)
		return NULL;

	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	NL_DBG(2, "Returning new cache %p\n", cache);

	return cache;
}

/**
 * Look up a neighbour by interface index and destination address
 * @arg cache		neighbour cache
 * @arg ifindex		interface index the neighbour is on
 * @arg dst		destination address of the neighbour
 * @return neighbour handle or NULL if no match was found.
 */
struct rtnl_neigh * rtnl_neigh_get(struct nl_cache *cache, int ifindex,
				   struct nl_addr *dst)
{
	struct rtnl_neigh *neigh;

	nl_list_for_each_entry(neigh, &cache->c_items, ce_list) {
		if (neigh->n_ifindex == ifindex &&
		    !nl_addr_cmp(neigh->n_dst, dst)) {
			nl_object_get((struct nl_object *) neigh);
			return neigh;
		}
	}

	return NULL;
}

/** @} */

/**
 * @name Neighbour Addition
 * @{
 */

static struct nl_msg * build_neigh_msg(struct rtnl_neigh *tmpl, int cmd,
				       int flags)
{
	struct nl_msg *msg;
	struct ndmsg nhdr = {
		.ndm_ifindex = tmpl->n_ifindex,
		.ndm_family = nl_addr_get_family(tmpl->n_dst),
		.ndm_state = NUD_PERMANENT,
	};

	if (tmpl->n_mask & NEIGH_ATTR_STATE)
		nhdr.ndm_state = tmpl->n_state;

	msg = nlmsg_build_simple(cmd, flags);
	if (!msg)
		return NULL;

	if (nlmsg_append(msg, &nhdr, sizeof(nhdr), 1) < 0)
		goto nla_put_failure;

	NLA_PUT_ADDR(msg, NDA_DST, tmpl->n_dst);

	if (tmpl->n_mask & NEIGH_ATTR_LLADDR)
		NLA_PUT_ADDR(msg, NDA_LLADDR, tmpl->n_lladdr);

	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

/**
 * Build netlink request message to add a new neighbour
 * @arg tmpl		template with data of new neighbour
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a addition of a new
 * neighbour. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a tmpl must contain the attributes of the new
 * neighbour set via \c rtnl_neigh_set_* functions.
 * 
 * The following attributes must be set in the template:
 *  - Interface index (rtnl_neigh_set_ifindex())
 *  - State (rtnl_neigh_set_state())
 *  - Destination address (rtnl_neigh_set_dst())
 *  - Link layer address (rtnl_neigh_set_lladdr())
 *
 * @return The netlink message
 */
struct nl_msg * rtnl_neigh_build_add_request(struct rtnl_neigh *tmpl, int flags)
{
	return build_neigh_msg(tmpl, RTM_NEWNEIGH, NLM_F_CREATE | flags);
}

/**
 * Add a new neighbour
 * @arg handle		netlink handle
 * @arg tmpl		template with requested changes
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_neigh_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * The following attributes must be set in the template:
 *  - Interface index (rtnl_neigh_set_ifindex())
 *  - State (rtnl_neigh_set_state())
 *  - Destination address (rtnl_neigh_set_dst())
 *  - Link layer address (rtnl_neigh_set_lladdr())
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_neigh_add(struct nl_handle *handle, struct rtnl_neigh *tmpl, int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_neigh_build_add_request(tmpl, flags);
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
 * @name Neighbour Deletion
 * @{
 */

/**
 * Build a netlink request message to delete a neighbour
 * @arg neigh		neighbour to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a deletion of a neighbour.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a neigh must point to an existing
 * neighbour.
 *
 * @return The netlink message
 */
struct nl_msg *rtnl_neigh_build_delete_request(struct rtnl_neigh *neigh,
					       int flags)
{
	return build_neigh_msg(neigh, RTM_DELNEIGH, flags);
}

/**
 * Delete a neighbour
 * @arg handle		netlink handle
 * @arg neigh		neighbour to delete
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_neigh_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_neigh_delete(struct nl_handle *handle, struct rtnl_neigh *neigh,
		      int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_neigh_build_delete_request(neigh, flags);
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
 * @name Neighbour Modification
 * @{
 */

/**
 * Build a netlink request message to change neighbour attributes
 * @arg neigh		the neighbour to change
 * @arg flags		additional netlink message flags
 *
 * Builds a new netlink message requesting a change of a neigh
 * attributes. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * @return The netlink message
 * @note Not all attributes can be changed, see
 *       \ref neigh_changeable "Changeable Attributes" for a list.
 */
struct nl_msg *rtnl_neigh_build_change_request(struct rtnl_neigh *neigh,
					       int flags)
{
	return build_neigh_msg(neigh, RTM_NEWNEIGH, NLM_F_REPLACE | flags);
}

/**
 * Change neighbour attributes
 * @arg handle		netlink handle
 * @arg neigh		neighbour to be changed
 * @arg flags		additional netlink message flags
 *
 * Builds a netlink message by calling rtnl_neigh_build_change_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 * @note Not all attributes can be changed, see
 *       \ref neigh_changeable "Changeable Attributes" for a list.
 */
int rtnl_neigh_change(struct nl_handle *handle, struct rtnl_neigh *neigh,
		      int flags)
{
	int err;
	struct nl_msg *msg;
	
	msg = rtnl_neigh_build_change_request(neigh, flags);
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
 * @name Neighbour States Translations
 * @{
 */

static struct trans_tbl neigh_states[] = {
	__ADD(NUD_INCOMPLETE, incomplete)
	__ADD(NUD_REACHABLE, reachable)
	__ADD(NUD_STALE, stale)
	__ADD(NUD_DELAY, delay)
	__ADD(NUD_PROBE, probe)
	__ADD(NUD_FAILED, failed)
	__ADD(NUD_NOARP, norarp)
	__ADD(NUD_PERMANENT, permanent)
};

/**
 * Convert neighbour states to a character string (Reentrant).
 * @arg state		neighbour state
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts a neighbour state to a character string separated by
 * commands and stores it in the specified destination buffer.
 *
 * @return The destination buffer
 */
char * rtnl_neigh_state2str(int state, char *buf, size_t len)
{
	return __flags2str(state, buf, len, neigh_states,
	    ARRAY_SIZE(neigh_states));
}

/**
 * Convert a character string to a neighbour state
 * @arg name		Name of cscope
 *
 * Converts the provided character string specifying a neighbour
 * state the corresponding numeric value.
 *
 * @return Neighbour state or a negative value if none was found.
 */
int rtnl_neigh_str2state(const char *name)
{
	return __str2type(name, neigh_states, ARRAY_SIZE(neigh_states));
}

/** @} */

/**
 * @name Neighbour Flags Translations
 * @{
 */

static struct trans_tbl neigh_flags[] = {
	__ADD(NTF_PROXY, proxy)
	__ADD(NTF_ROUTER, router)
};

/**
 * Convert neighbour flags to a character string (Reentrant).
 * @arg flags		neighbour flags
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts neighbour flags to a character string separated by
 * commands and stores it in the specified destination buffer.
 *
 * @return The destination buffer or a empty string if no flags are set.
 */
char * rtnl_neigh_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, neigh_flags,
	    ARRAY_SIZE(neigh_flags));
}

/**
 * Convert a character string to a neighbour flag
 * @arg name		name of the flag
 *
 * Converts the provided character string specifying a neighbour
 * flag the corresponding numeric value.
 *
 * @return Neighbour flag or a negative value if none was found.
 */
int rtnl_neigh_str2flag(const char *name)
{
	return __str2type(name, neigh_flags, ARRAY_SIZE(neigh_flags));
}

/** @} */

/**
 * @name Attribute Modification
 * @{
 */

/**
 * Set a neighbour state
 * @arg neigh		neighbour to change
 * @arg state		state to set
 */
void rtnl_neigh_set_state(struct rtnl_neigh *neigh, int state)
{
	neigh->n_state_mask |= state;
	neigh->n_state |= state;
	neigh->n_mask |= NEIGH_ATTR_STATE;
}

/**
 * Get neighbour states
 * @arg neigh		neighbour handle
 * @return Neighbour state or -1 if not set
 */
int rtnl_neigh_get_state(struct rtnl_neigh *neigh)
{
	if (neigh->n_mask & NEIGH_ATTR_STATE)
		return neigh->n_state;
	else
		return -1;
}

/**
 * Unset a neigbour state
 * @arg neigh		neighbour to change
 * @arg state		state to unset
 */
void rtnl_neigh_unset_state(struct rtnl_neigh *neigh, int state)
{
	neigh->n_state_mask |= state;
	neigh->n_state &= ~state;
	neigh->n_mask |= NEIGH_ATTR_STATE;
}

/**
 * Set neighbour flags
 * @arg neigh		neighbour to change
 * @arg flags		flag to set
 */
void rtnl_neigh_set_flags(struct rtnl_neigh *neigh, unsigned int flags)
{
	neigh->n_flag_mask |= flags;
	neigh->n_flags |= flags;
	neigh->n_mask |= NEIGH_ATTR_FLAGS;
}

/**
 * Get neighbour flags
 * @arg neigh		neighbour handle
 * @return Neighbour flags
 */
unsigned int rtnl_neigh_get_flags(struct rtnl_neigh *neigh)
{
	return neigh->n_flags;
}

/**
 * Unset neighbour flags
 * @arg neigh		neighbour to change
 * @arg flags		flag to unset
 */
void rtnl_neigh_unset_flags(struct rtnl_neigh *neigh, unsigned int flags)
{
	neigh->n_flag_mask |= flags;
	neigh->n_flags &= ~flags;
	neigh->n_mask |= NEIGH_ATTR_FLAGS;
}

/**
 * Set the interface index of device this neighbour is on
 * @arg neigh		neighbour to change
 * @arg ifindex		new interface index
 */
void rtnl_neigh_set_ifindex(struct rtnl_neigh *neigh, int ifindex)
{
	neigh->n_ifindex = ifindex;
	neigh->n_mask |= NEIGH_ATTR_IFINDEX;
}

/**
 * Get the interface index of the device this neighbour is on
 * @arg neigh		neighbour handle
 * @return Interface index or RTNL_LINK_NOT_FOUND if not set
 */
int rtnl_neigh_get_ifindex(struct rtnl_neigh *neigh)
{
	if (neigh->n_mask & NEIGH_ATTR_IFINDEX)
		return neigh->n_ifindex;
	else
		return RTNL_LINK_NOT_FOUND;
}

static inline int __assign_addr(struct rtnl_neigh *neigh, struct nl_addr **pos,
			        struct nl_addr *new, int flag, int nocheck)
{
	if (!nocheck) {
		if (neigh->n_mask & NEIGH_ATTR_FAMILY) {
			if (new->a_family != neigh->n_family)
				return nl_error(EINVAL,
						"Address family mismatch");
		} else {
			neigh->n_family = new->a_family;
			neigh->n_mask |= NEIGH_ATTR_FAMILY;
		}
	}

	if (*pos)
		nl_addr_put(*pos);

	nl_addr_get(new);
	*pos = new;

	neigh->n_mask |= flag;

	return 0;
}

/**
 * Set link layer address of a neighbour
 * @arg neigh		neighbour to change
 * @arg addr		new link layer address
 *
 * Assigns the new link layer address to the specified neighbour handle.
 *
 * @note The prefix length of the address will be ignored.
 */
void rtnl_neigh_set_lladdr(struct rtnl_neigh *neigh, struct nl_addr *addr)
{
	__assign_addr(neigh, &neigh->n_lladdr, addr, NEIGH_ATTR_LLADDR, 1);
}

/**
 * Get link layer address of a neighbour
 * @arg neigh		neighbour handle
 * @return Link layer address or NULL if not set
 */
struct nl_addr *rtnl_neigh_get_lladdr(struct rtnl_neigh *neigh)
{
	if (neigh->n_mask & NEIGH_ATTR_LLADDR)
		return neigh->n_lladdr;
	else
		return NULL;
}

/**
 * Set destination address of a neighbour
 * @arg neigh		neighbour to change
 * @arg addr		new destination address
 *
 * Assigns the new destination address to the specified neighbour handle.
 * The address is validated against the address family if set already via
 * rtnl_neigh_set_family(). The assignment fails if the address families
 * mismatch. In case the address family has not been specified yet, the
 * address family of this new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_neigh_set_dst(struct rtnl_neigh *neigh, struct nl_addr *addr)
{
	return __assign_addr(neigh, &neigh->n_dst, addr,
			     NEIGH_ATTR_DST, 0);
}

/**
 * Get the destination address of a neighbour
 * @arg neigh		neighbour handle
 * @return Destination address or NULL if not set
 */
struct nl_addr *rtnl_neigh_get_dst(struct rtnl_neigh *neigh)
{
	if (neigh->n_mask & NEIGH_ATTR_DST)
		return neigh->n_dst;
	else
		return NULL;
}

/**
 * Set destination address family
 * @arg neigh		neighbour to change
 * @arg family		new destination address family
 */
void rtnl_neigh_set_family(struct rtnl_neigh *neigh, int family)
{
	neigh->n_family = family;
	neigh->n_mask |= NEIGH_ATTR_FAMILY;
}

/**
 * Set RTN type of a neighbour
 * @arg neigh		neighbour to change
 * @arg type		new rtn type
 */
void rtnl_neigh_set_type(struct rtnl_neigh *neigh, int type)
{
	neigh->n_type = type;
	neigh->n_mask = NEIGH_ATTR_TYPE;
}

/**
 * Get RTN type of a neighbour
 * @arg neigh		neighbour handle
 * @return Type or -1 if not set
 */
int rtnl_neigh_get_type(struct rtnl_neigh *neigh)
{
	if (neigh->n_mask & NEIGH_ATTR_TYPE)
		return neigh->n_type;
	else
		return -1;
}

/** @} */

static struct nl_cache_ops rtnl_neigh_ops = {
	.co_name		= "route/neigh",
	.co_size		= sizeof(struct rtnl_neigh),
	.co_hdrsize		= sizeof(struct ndmsg),
	.co_msgtypes		= {
					{ RTM_NEWNEIGH, "new" },
					{ RTM_DELNEIGH, "delete" },
					{ RTM_GETNEIGH, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update	= neigh_request_update,
	.co_msg_parser		= neigh_msg_parser,
	.co_free_data		= neigh_free_data,
	.co_dump[NL_DUMP_BRIEF]	= neigh_dump_brief,
	.co_dump[NL_DUMP_FULL]	= neigh_dump_full,
	.co_dump[NL_DUMP_STATS]	= neigh_dump_stats,
	.co_dump[NL_DUMP_XML]	= neigh_dump_xml,
	.co_dump[NL_DUMP_ENV]	= neigh_dump_env,
	.co_filter		= neigh_filter,
};

static void __init neigh_init(void)
{
	nl_cache_mngt_register(&rtnl_neigh_ops);
}

static void __exit neigh_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_neigh_ops);
}

/** @} */
