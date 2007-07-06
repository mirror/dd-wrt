/*
 * neigh.c              rtnetlink neighbour
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
 * @defgroup neigh Neighbours
 * Module to access and modify the neighbour tables.
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
 *  - state
 *  - link layer address
 *  - flags
 *
 * @par Required Caches for Dumping
 * In order to dump neighbour attributes you must provide the following
 * caches via nl_cache_provide()
 *  - link cache holding all links
 *
 * @par Example 1: Retrieving a neighbour cache and dumping all neighbours:
 * @code
 * struct nl_dump_params p = { .dp_type = NL_DUMP_BRIEF };
 * struct nl_cache *nc = rtnl_neigh_build_cache(&nl_handle);
 * struct nl_cache *lc = rtnl_link_build_cache(&nl_handle);
 * nl_cache_provide(lc);
 * nl_cache_dump(nc, stdout, &p);
 * @endcode
 *
 * @par Example 2: Dumping neighbours based on filter:
 * @code
 * struct nl_dump_params p = { .dp_type = NL_DUMP_FULL };
 * struct rtnl_neigh filter = RTNL_INIT_NEIGH();
 * rtnl_neigh_set_ifindex_name(&filter, "eth3");
 * nl_cache_dump_filter(nc, stdout, &p, (struct nl_common *) &filter);
 * @endcode
 *
 * @par Example 3: Adding a neighbour
 * @code
 * struct rtnl_neigh neigh = RTNL_INIT_NEIGH();
 * rtnl_neigh_set_ifindex_name(&neigh, "eth3");
 * rtnl_neigh_set_dst_str(&neigh, "10.0.0.10");
 * rtnl_neigh_set_state_name(&neigh, "permanent");
 * rtnl_neigh_add(&nl_handle, &neight);
 * @endcode
 *
 * @par Example 4: Deleting a neighbour
 * @code
 * struct rtnl_neigh *neigh;
 * neigh = rtnl_neigh_get(nc, rtnl_link_name2i(lc, "eth3"), "10.20.30.40");
 * rtnl_neigh_delete(&nl_handle, neigh);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/link.h>

static int neigh_filter(struct nl_common *obj, struct nl_common *filter)
{
	struct rtnl_neigh *o = (struct rtnl_neigh *) obj;
	struct rtnl_neigh *f = (struct rtnl_neigh *) filter;

	if (obj->ce_type != RTNL_NEIGH || filter->ce_type != RTNL_NEIGH)
		return 0;

#define REQ(F) (f->n_mask & NEIGH_HAS_##F)
#define AVAIL(F) (o->n_mask & NEIGH_HAS_##F)
	if ((REQ(IFINDEX) &&
	      (!AVAIL(IFINDEX) || o->n_ifindex != f->n_ifindex))	   ||
	    (REQ(FAMILY)  &&
	      (!AVAIL(FAMILY)  || o->n_family != f->n_family))		   ||
	    (REQ(TYPE)    &&
	      (!AVAIL(TYPE)    || o->n_type != f->n_type))		   ||
	    (REQ(LLADDR)  &&
	      (!AVAIL(LLADDR)  || nl_addrcmp(&o->n_lladdr, &f->n_lladdr))) ||
	    (REQ(DST)     &&
	      (!AVAIL(DST)     || nl_addrcmp(&o->n_dst, &f->n_dst)))	   ||
	    (REQ(STATE)   &&
	      (!AVAIL(STATE)   || f->n_state ^
					(o->n_state & f->n_state_mask)))   ||
	    (REQ(FLAGS)   &&
	      (!AVAIL(FLAGS)   || f->n_flags ^
					(o->n_flags & f->n_flag_mask))))
		return 0;
#undef REQ
#undef AVAIL

	return 1;
}

static int neigh_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			    void *arg)
{
	struct rtnl_neigh neigh = RTNL_INIT_NEIGH();
	struct rtattr *tb[NDA_MAX + 1];
	struct nl_parser_param *pp = arg;
	size_t len;
	struct ndmsg *nm;
	int err;

	if (n->nlmsg_type != RTM_NEWNEIGH)
		return P_IGNORE;

	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(struct ndmsg));

	if (len < 0)
		return nl_error(EINVAL, "Message too short to be a " \
		    "neighbour message");

	nm = NLMSG_DATA(n);
	
	err = nl_parse_rtattr(tb, NDA_MAX, NDA_RTA(nm), len);
	if (err < 0)
		return err;

	neigh.n_family  = nm->ndm_family;
	neigh.n_ifindex = nm->ndm_ifindex;
	neigh.n_state   = nm->ndm_state;
	neigh.n_flags   = nm->ndm_flags;
	neigh.n_type    = nm->ndm_type;

	if (tb[NDA_LLADDR]) {
		nl_copy_addr(&neigh.n_lladdr, tb[NDA_LLADDR]);
		neigh.n_mask |= NEIGH_HAS_LLADDR;
	}

	if (tb[NDA_DST]) {
		nl_copy_addr(&neigh.n_dst, tb[NDA_DST]);
		neigh.n_dst.a_family = neigh.n_family;
		neigh.n_mask |= NEIGH_HAS_DST;
	}

	if (tb[NDA_CACHEINFO]) {
		struct nda_cacheinfo *ci;
		
		if (RTA_PAYLOAD(tb[NDA_CACHEINFO]) < sizeof(*ci))
			return nl_error(EINVAL, "cacheinfo TLV is too short");

		ci = (struct nda_cacheinfo *) RTA_DATA(tb[NDA_CACHEINFO]);

		neigh.n_cacheinfo.nci_confirmed = ci->ndm_confirmed;
		neigh.n_cacheinfo.nci_used = ci->ndm_used;
		neigh.n_cacheinfo.nci_updated = ci->ndm_updated;
		neigh.n_cacheinfo.nci_refcnt = ci->ndm_refcnt;
		
		neigh.n_mask |= NEIGH_HAS_CACHEINFO;
	}

	err = pp->pp_cb((struct nl_common *) &neigh, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int neigh_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETNEIGH, AF_UNSPEC, NLM_F_DUMP);
}


static int neigh_dump_brief(struct nl_cache *c, struct nl_common *a, FILE *fd,
			    struct nl_dump_params *params)
{
	char dst[INET6_ADDRSTRLEN+5], lladdr[INET6_ADDRSTRLEN+5];
	struct rtnl_neigh *n = (struct rtnl_neigh *) a;
	char state[128], flags[64];

	dp_new_line(fd, params, 0);
	fprintf(fd, "%s dev %s lladdr %s ",
		nl_addr2str_r(&(n->n_dst), dst, sizeof(dst)),
		rtnl_link_i2name(nl_cache_lookup(RTNL_LINK), n->n_ifindex),
		nl_addr2str_r(&(n->n_lladdr), lladdr, sizeof(lladdr)));

	rtnl_neigh_state2str_r(n->n_state, state, sizeof(state));
	rtnl_neigh_flags2str_r(n->n_flags, flags, sizeof(flags));

	if (state[0])
		fprintf(fd, "<%s", state);
	if (flags[0])
		fprintf(fd, "%s%s", state[0] ? "," : "<", flags);
	if (state[0] || flags[0])
		fprintf(fd, ">");
	fprintf(fd, "\n");

	return 1;
}

static int neigh_dump_full(struct nl_cache *c, struct nl_common *a, FILE *fd,
			   struct nl_dump_params *params)
{
	char rtn_type[32];
	struct rtnl_neigh *n = (struct rtnl_neigh *) a;
	int hz = nl_get_hz();

	int line = neigh_dump_brief(c, a, fd, params);

	dp_new_line(fd, params, line++);
	fprintf(fd, "    refcnt %u type %s confirmed %u used %u updated %u\n",
		n->n_cacheinfo.nci_refcnt,
		nl_rtntype2str_r(n->n_type, rtn_type, sizeof(rtn_type)),
		n->n_cacheinfo.nci_confirmed/hz,
		n->n_cacheinfo.nci_used/hz, n->n_cacheinfo.nci_updated/hz);

	return line;
}

static int neigh_dump_with_stats(struct nl_cache *c, struct nl_common *a,
				  FILE *fd, struct nl_dump_params *params)
{
	return neigh_dump_full(c, a, fd, params);
}

/**
 * @name General API
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
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The new cache or NULL if an error occured.
 */
struct nl_cache * rtnl_neigh_build_cache(struct nl_handle *handle)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	cache->c_type = RTNL_NEIGH;
	cache->c_type_size = sizeof(struct rtnl_neigh);
	cache->c_ops = &rtnl_neigh_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

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
	int i;
	struct rtnl_neigh *neigh;

	if (cache->c_type != RTNL_NEIGH || NULL == cache->c_cache)
		return NULL;

	for (i = 0; i < cache->c_index; i++) {
		neigh = (struct rtnl_neigh *) NL_CACHE_ELEMENT_AT(cache, i);

		if (neigh && neigh->n_ifindex == ifindex &&
		    !nl_addrcmp(&neigh->n_dst, dst))
			return neigh;
	}

	return NULL;

}

/**
 * Dump neighbour attributes
 * @arg neigh		neighbour to dump
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_neigh_dump(struct rtnl_neigh *neigh, FILE *fd,
		     struct nl_dump_params *params)
{
	int type = params ? params->dp_type : NL_DUMP_FULL;
	struct nl_cache_ops *ops = &rtnl_neigh_ops;

	if (type < 0 || type > NL_DUMP_MAX)
		BUG();

	if (ops->co_dump[type])
		ops->co_dump[type](NULL, (struct nl_common *) neigh,
		    fd, params);
}

/** @} */

/**
 * @name Neighbour Addition/Modification/Deletion
 * @{
 */

static struct nl_msg * build_neigh_msg(struct rtnl_neigh *tmpl, int cmd,
				       int flags)
{
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = cmd,
		.nlmsg_flags = flags,
	};
	struct ndmsg nm = {
		.ndm_ifindex = tmpl->n_ifindex,
		.ndm_family = tmpl->n_dst.a_family,
		.ndm_state = tmpl->n_state,
	};

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &nm, sizeof(nm));
	nl_msg_append_tlv(m, NDA_DST, &tmpl->n_dst.a_addr, tmpl->n_dst.a_len);

	if (tmpl->n_mask & NEIGH_HAS_LLADDR)
		nl_msg_append_tlv(m, NDA_LLADDR, &tmpl->n_lladdr.a_addr,
		    tmpl->n_lladdr.a_len);

	return m;
}

/**
 * Build netlink request message to add a new neighbour
 * @arg tmpl		template with data of new neighbour
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
struct nl_msg * rtnl_neigh_build_add_request(struct rtnl_neigh *tmpl)
{
	return build_neigh_msg(tmpl, RTM_NEWNEIGH, NLM_F_CREATE);
}

/**
 * Add a new neighbour
 * @arg handle		netlink handle
 * @arg tmpl		template with requested changes
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
int rtnl_neigh_add(struct nl_handle *handle, struct rtnl_neigh *tmpl)
{
	int err;
	struct nl_msg *m = rtnl_neigh_build_add_request(tmpl);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to delete a neighbour
 * @arg neigh		neighbour to delete
 *
 * Builds a new netlink message requesting a deletion of a neighbour.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a neigh must point to an existing
 * neighbour.
 *
 * @return The netlink message
 */
struct nl_msg * rtnl_neigh_build_delete_request(struct rtnl_neigh *neigh)
{
	return build_neigh_msg(neigh, RTM_DELNEIGH, 0);
}

/**
 * Delete a neighbour
 * @arg handle		netlink handle
 * @arg neigh		neighbour to delete
 *
 * Builds a netlink message by calling rtnl_neigh_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_neigh_delete(struct nl_handle *handle, struct rtnl_neigh *neigh)
{
	int err;
	struct nl_msg *m = rtnl_neigh_build_delete_request(neigh);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to change neighbour attributes
 * @arg neigh		the neighbour to change
 * @arg tmpl		template with attributes to be changed
 *
 * Builds a new netlink message requesting a change of a neigh
 * attributes. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 * \a neigh must point to a link handle currently configured in the kernel
 * and \a tmpl must contain the attributes to be changed set via
 * \c rtnl_neig_set_* functions.
 *
 * @return The netlink message
 * @note Not all attributes can be changed, see
 *       \ref neigh_changeable "Changeable Attributes" for a list.
 */
struct nl_msg * rtnl_neigh_build_change_request(struct rtnl_neigh *neigh,
						struct rtnl_neigh *tmpl)
{
	struct rtnl_neigh new_neigh;

	memcpy(&new_neigh, neigh, sizeof(new_neigh));
	new_neigh.n_mask = 0;

	if (tmpl->n_mask & NEIGH_HAS_FLAGS) {
		new_neigh.n_flags = tmpl->n_flags;
		new_neigh.n_mask |= NEIGH_HAS_FLAGS;
	}

	if (tmpl->n_mask & NEIGH_HAS_STATE) {
		new_neigh.n_state = tmpl->n_state;
		new_neigh.n_mask |= NEIGH_HAS_STATE;
	}

	if (tmpl->n_mask & NEIGH_HAS_LLADDR && tmpl->n_lladdr.a_len) {
		memcpy(&new_neigh.n_lladdr, &tmpl->n_lladdr,
		    sizeof(new_neigh.n_lladdr));
		new_neigh.n_mask |= NEIGH_HAS_LLADDR;
	}

	return build_neigh_msg(&new_neigh, RTM_NEWNEIGH, NLM_F_REPLACE);
}

/**
 * Change neighbour attributes
 * @arg handle		netlink handle
 * @arg neigh		neighbour to be changed
 * @arg tmpl		template with requested changes
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
		      struct rtnl_neigh *tmpl)
{
	int err;
	struct nl_msg *m = rtnl_neigh_build_change_request(neigh, tmpl);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
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
 * Convert neighbour states to a character string.
 * @arg state		neighbour state
 *
 * Converts a neighbour state to a character string separated by
 * commas and stores it in a static buffer.
 *
 * @return A static buffer
 * @attention This funnction is NOT thread safe.
 */
char * rtnl_neigh_state2str(int state)
{
	static char buf[128];
	memset(buf, 0, sizeof(buf));
	return __flags2str_r(state, buf, sizeof(buf), neigh_states,
	    ARRAY_SIZE(neigh_states));
}

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
char * rtnl_neigh_state2str_r(int state, char *buf, size_t len)
{
	return __flags2str_r(state, buf, len, neigh_states,
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
 * Convert neighbour flags to a character string
 * @arg flags		neighbour flags
 *
 * Converts neighbour flags to a character string separate by
 * commas and stores it in a static buffer.
 *
 * @return A static buffer
 * @attention This funnction is NOT thread safe.
 */
char * rtnl_neigh_flags2str(int flags)
{
	static char buf[64];
	memset(buf, 0, sizeof(buf));
	return __flags2str_r(flags, buf, sizeof(buf), neigh_flags,
	    ARRAY_SIZE(neigh_flags));
}

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
char * rtnl_neigh_flags2str_r(int flags, char *buf, size_t len)
{
	return __flags2str_r(flags, buf, len, neigh_flags,
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
 * @name Attribute Modifications
 * @{
 */

/**
 * Set a state
 * @arg neigh		neighbour to change
 * @arg state		state to set
 */
void rtnl_neigh_set_state(struct rtnl_neigh *neigh, int state)
{
	neigh->n_state_mask |= state;
	neigh->n_state |= state;
	neigh->n_mask |= NEIGH_HAS_STATE;
}

/**
 * Set a state by its name
 * @arg neigh		neighbour to change
 * @arg name		name to set
 */
void rtnl_neigh_set_state_name(struct rtnl_neigh *neigh, const char *name)
{
	int state = rtnl_neigh_str2state(name);
	if (state >= 0)
		rtnl_neigh_set_state(neigh, state);
}

/**
 * Unset a state
 * @arg neigh		neighbour to change
 * @arg state		state to unset
 */
void rtnl_neigh_unset_state(struct rtnl_neigh *neigh, int state)
{
	neigh->n_state_mask |= state;
	neigh->n_state &= ~state;
	neigh->n_mask |= NEIGH_HAS_STATE;
}

/**
 * Unset a state by its name
 * @arg neigh		neighbour to change
 * @arg name		state name to unset
 */
void rtnl_neigh_unset_state_name(struct rtnl_neigh *neigh, const char *name)
{
	int state = rtnl_neigh_str2state(name);
	if (state >= 0)
		rtnl_neigh_unset_state(neigh, state);
}

/**
 * Set a flag
 * @arg neigh		neighbour to change
 * @arg flag		flag to set
 */
void rtnl_neigh_set_flag(struct rtnl_neigh *neigh, int flag)
{
	neigh->n_flag_mask |= flag;
	neigh->n_flags |= flag;
	neigh->n_mask |= NEIGH_HAS_FLAGS;
}

/**
 * Unset a flag
 * @arg neigh		neighbour to change
 * @arg flag		flag to unset
 */
void rtnl_neigh_unset_flag(struct rtnl_neigh *neigh, int flag)
{
	neigh->n_flag_mask |= flag;
	neigh->n_flags &= ~flag;
	neigh->n_mask |= NEIGH_HAS_FLAGS;
}

/**
 * Set interface index of device this neighbour is on
 * @arg neigh		neighbour to change
 * @arg ifindex		new interface index
 */
void rtnl_neigh_set_ifindex(struct rtnl_neigh *neigh, int ifindex)
{
	neigh->n_ifindex = ifindex;
	neigh->n_mask |= NEIGH_HAS_IFINDEX;
}

/**
 * Set interface index via interface name
 * @arg neigh		neighbour to change
 * @arg cache		link cache holding all links
 * @arg name		name of interface
 * @return 0 on success or a negative error code
 */
int rtnl_neigh_set_ifindex_name(struct rtnl_neigh *neigh,
				struct nl_cache *cache, const char *name)
{
	int i;

	if (cache->c_type != RTNL_LINK)
		return nl_error(EINVAL, "Must be link cache");

	i = rtnl_link_name2i(cache, name);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", name);

	rtnl_neigh_set_ifindex(neigh, i);
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
	memcpy(&neigh->n_lladdr, addr, sizeof(neigh->n_lladdr));
	neigh->n_mask |= NEIGH_HAS_LLADDR;
}

/**
 * Set link layer address of a neighbour
 * @arg neigh		neighbour to change
 * @arg addr		new link layer address as string
 *
 * Translates the specified address to a binary format and assigns it
 * as the new link layer address by calling rtnl_neigh_set_lladdr().
 *
 * @see rtnl_neigh_set_lladdr()
 * @return 0 on success or a negative error code.
 */
int rtnl_neigh_set_lladdr_str(struct rtnl_neigh *neigh, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	
	err = nl_str2addr(addr, &a, AF_UNSPEC);
	if (err < 0)
		return err;

	rtnl_neigh_set_lladdr(neigh, &a);
	return 0;
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
	if (neigh->n_mask & NEIGH_HAS_FAMILY) {
		if (neigh->n_family != addr->a_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		neigh->n_family = addr->a_family;
		
	memcpy(&neigh->n_dst, addr, sizeof(neigh->n_dst));
	neigh->n_mask |= (NEIGH_HAS_DST | NEIGH_HAS_FAMILY);
	return 0;
}

/**
 * Set the destination address of a neighbour
 * @arg neigh		neighbour to change
 * @arg addr		new destination address as string
 *
 * Translates the specified address to a binary format and assigns
 * it as the destination address by calling rtnl_neigh_set_dst().
 *
 * @see rtnl_neigh_set_dst();
 * @return 0 on success or a negative error code.
 */
int rtnl_neigh_set_dst_str(struct rtnl_neigh *neigh, const char *addr)
{
	int err;
	struct nl_addr a = {0};
	int hint = neigh->n_mask & NEIGH_HAS_FAMILY ? neigh->n_family : AF_UNSPEC;
	
	err = nl_str2addr(addr, &a, hint);
	if (err < 0)
		return err;

	return rtnl_neigh_set_dst(neigh, &a);
}

/**
 * Set destination address family
 * @arg neigh		neighbour to change
 * @arg family		new destination address family
 */
void rtnl_neigh_set_family(struct rtnl_neigh *neigh, int family)
{
	neigh->n_family = family;
	neigh->n_mask |= NEIGH_HAS_FAMILY;
}

/**
 * Set RTN type
 * @arg neigh		neighbour to change
 * @arg type		new rtn type
 */
int rtnl_neigh_set_type(struct rtnl_neigh *neigh, const char *type)
{
	int t = nl_str2rtntype(type);

	if (-1 == t)
		return nl_error(EINVAL, "Invalid neighbour type");

	neigh->n_type = t;
	neigh->n_mask = NEIGH_HAS_TYPE;
	return 0;
}

/** @} */

struct nl_cache_ops rtnl_neigh_ops = {
	.co_request_update	= &neigh_request_update,
	.co_msg_parser		= &neigh_msg_parser,
	.co_dump[NL_DUMP_BRIEF]	= &neigh_dump_brief,
	.co_dump[NL_DUMP_FULL]	= &neigh_dump_full,
	.co_dump[NL_DUMP_STATS]	= &neigh_dump_with_stats,
	.co_filter		= &neigh_filter,
};

/** @} */
