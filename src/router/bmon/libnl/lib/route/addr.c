/*
 * addr.c              rtnetlink addr layer
 *
 * Copyright (c) 2003-2005
 *    Thomas Graf <tgraf@suug.ch>,
 *    Baruch Even <baruch@ev-en.org>,
 *    Mediatrix Telecom, inc. <ericb@mediatrix.com>
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
 * @defgroup rtaddr Addresses
 * Module to access and modify addresses.
 *
 * @par Address Identification
 * A neighbour is uniquely identified by the attributes listed below, whenever
 * you refer to an existing neighbour all of the attributes must be set.
 * Neighbours from caches automatically have all required attributes set.
 *   - interface index (rtnl_addr_set_ifindex())
 *   - destination address (rtnl_neigh_set_dst())
 *
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/addr.h>
#include <netlink/route/route.h>
#include <netlink/route/rtattr.h>
#include <netlink/route/link.h>
#include <netlink/helpers.h>

static int addr_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *n,
			   void *arg)
{
	struct rtnl_addr addr = RTNL_INIT_ADDR();
	struct nl_parser_param *pp = arg;
	struct ifaddrmsg *ifa;
	struct rtattr *tb[IFA_MAX+1];
	size_t len;
	int err, peer_prefix = 0;

	if (n->nlmsg_type != RTM_NEWADDR)
		return P_IGNORE;

	len = n->nlmsg_len - NLMSG_LENGTH(sizeof(struct ifaddrmsg));

	if (len < 0)
		return nl_error(EINVAL, "netlink message too short to be an " \
			"address message");

	ifa = NLMSG_DATA(n);

	err = nl_parse_rtattr(tb, IFA_MAX, IFA_RTA(ifa), len);
	if (err < 0)
		return err;

	addr.a_family = ifa->ifa_family;
	addr.a_prefixlen = ifa->ifa_prefixlen;
	addr.a_flags = ifa->ifa_flags;
	addr.a_scope = ifa->ifa_scope;
	addr.a_ifindex = ifa->ifa_index;

	addr.a_mask = (ADDR_HAS_FAMILY | ADDR_HAS_PREFIXLEN | ADDR_HAS_FLAGS |
			ADDR_HAS_SCOPE | ADDR_HAS_IFINDEX);

	if (tb[IFA_LABEL]) {
		if (RTA_PAYLOAD(tb[IFA_LABEL]) > sizeof(addr.a_label))
			return nl_error(EINVAL, "interface label TLV length " \
					"exceeds local interface name limit");

		memcpy(addr.a_label, RTA_DATA(tb[IFA_LABEL]),
				RTA_PAYLOAD(tb[IFA_LABEL]));
		addr.a_label[sizeof(addr.a_label) - 1] = '\0';
		addr.a_mask |= ADDR_HAS_LABEL;
	}

	if (tb[IFA_CACHEINFO]) {
		struct ifa_cacheinfo *ca;

		if (RTA_PAYLOAD(tb[IFA_CACHEINFO]) < sizeof(*ca))
			return nl_error(EINVAL,
					"addr cacheinfo TLV is too short");

		ca = (struct ifa_cacheinfo *) RTA_DATA(tb[IFA_CACHEINFO]);
		addr.a_cacheinfo.aci_prefered = ca->ifa_prefered;
		addr.a_cacheinfo.aci_valid = ca->ifa_valid;
		addr.a_cacheinfo.aci_cstamp = ca->cstamp;
		addr.a_cacheinfo.aci_tstamp = ca->tstamp;

		addr.a_mask |= ADDR_HAS_CACHEINFO;
	}

	if (tb[IFA_LOCAL]) {
		nl_copy_addr(&addr.a_local, tb[IFA_LOCAL]);
		addr.a_local.a_family = addr.a_family;
		addr.a_local.a_prefix = addr.a_prefixlen;
		addr.a_mask |= ADDR_HAS_LOCAL;
	}

	if (tb[IFA_ADDRESS]) {
		struct nl_addr a;

		nl_copy_addr(&a, tb[IFA_ADDRESS]);

		/* IPv6 sends the local address as IFA_ADDRESS with
		 * no IFA_LOCAL, IPv4 sends both IFA_LOCAL and IFA_ADDRESS
		 * with IFA_ADDRESS being the peer address if they differ */
		if (!tb[IFA_LOCAL] || !nl_addrcmp(&a, &addr.a_local)) {
			memcpy(&addr.a_local, &a, sizeof(a));
			addr.a_local.a_family = addr.a_family;
			addr.a_mask |= ADDR_HAS_LOCAL;
		} else {
			memcpy(&addr.a_peer, &a, sizeof(a));
			addr.a_mask |= ADDR_HAS_PEER;
			addr.a_peer.a_family = addr.a_family;
			peer_prefix = 1;
		}
	}

	if (peer_prefix)
		addr.a_peer.a_prefix = addr.a_prefixlen;
	else
		addr.a_local.a_prefix = addr.a_prefixlen;

	if (tb[IFA_BROADCAST]) {
		nl_copy_addr(&addr.a_bcast, tb[IFA_BROADCAST]);
		addr.a_bcast.a_family = addr.a_family;
		addr.a_mask |= ADDR_HAS_BROADCAST;
	}

	if (tb[IFA_ANYCAST]) {
		nl_copy_addr(&addr.a_anycast, tb[IFA_ANYCAST]);
		addr.a_anycast.a_family = addr.a_family;
		addr.a_mask |= ADDR_HAS_ANYCAST;
	}

	err = pp->pp_cb((struct nl_common *) &addr, pp);
	if (err < 0)
		return err;

	return P_ACCEPT;
}

static int addr_request_update(struct nl_cache *c, struct nl_handle *h)
{
	return nl_rtgen_request(h, RTM_GETADDR, AF_UNSPEC, NLM_F_DUMP);
}

static int addr_dump_brief(struct nl_cache *c, struct nl_common *nl, FILE *fd,
			   struct nl_dump_params *params)
{
	struct rtnl_addr *a = (struct rtnl_addr *) nl;
	char addr[INET6_ADDRSTRLEN+5], scope[32];
	char flags[128];

	dp_new_line(fd, params, 0);
	if (params && params->dp_print_index)
		fprintf(fd, "%d: ", nl->ce_index);

	if (a->a_mask & ADDR_HAS_LOCAL)
		fprintf(fd, "%s",
			nl_addr2str_r(&(a->a_local), addr, sizeof(addr)));
	else
		fprintf(fd, "none");

	if (a->a_mask & ADDR_HAS_PEER)
		fprintf(fd, " peer %s",
			nl_addr2str_r(&(a->a_peer), addr, sizeof(addr)));

	fprintf(fd, " %s dev %s scope %s",
		nl_af2str(a->a_family),
		rtnl_link_i2name(nl_cache_lookup(RTNL_LINK), a->a_ifindex),
		rtnl_scope2str_r(a->a_scope, scope, sizeof(scope)));

	rtnl_addr_flags2str_r(a->a_flags, flags, sizeof(flags));
	if (flags[0])
		fprintf(fd, " <%s>", flags);


	fprintf(fd, "\n");

	return 1;
}

static int addr_dump_full(struct nl_cache *c, struct nl_common *n, FILE *fd,
			  struct nl_dump_params *params)
{
	struct rtnl_addr *a = (struct rtnl_addr *) n;
	int line = addr_dump_brief(c, n, fd, params);
	char addr[INET6_ADDRSTRLEN+5];

	if (a->a_mask & (ADDR_HAS_LABEL|ADDR_HAS_BROADCAST|ADDR_HAS_ANYCAST)) {
		dp_new_line(fd, params, line++);
		fprintf(fd, "  ");

		if (a->a_mask & ADDR_HAS_LABEL)
			fprintf(fd, " label %s", a->a_label);

		if (a->a_mask & ADDR_HAS_BROADCAST)
			fprintf(fd, " broadcast %s",
				nl_addr2str_r(&(a->a_bcast), addr,
					      sizeof(addr)));

		if (a->a_mask & ADDR_HAS_ANYCAST)
			fprintf(fd, " anycast %s",
				nl_addr2str_r(&(a->a_anycast), addr,
					      sizeof(addr)));

		fprintf(fd, "\n");
	}

	if (a->a_mask & ADDR_HAS_CACHEINFO) {
		char dstr[128];
		struct rtnl_addr_cacheinfo *ci = &a->a_cacheinfo;

		dp_new_line(fd, params, line++);

		fprintf(fd, "   valid_lifetime ");
		if (ci->aci_valid == 0xFFFFFFFFU)
			fprintf(fd, "[forever]");
		else
			fprintf(fd, "[%s]", nl_msec2str_r(ci->aci_valid*1000,
							dstr, sizeof(dstr)));

		fprintf(fd, " preferred_lifetime ");
		if (ci->aci_prefered == 0xFFFFFFFFU)
			fprintf(fd, "[forever]");
		else
			fprintf(fd, "[%s]", nl_msec2str_r(ci->aci_prefered*1000,
							dstr, sizeof(dstr)));

		fprintf(fd, "\n");
		dp_new_line(fd, params, line++);
		fprintf(fd, "   created boot_time+[%s] ",
			nl_msec2str_r(a->a_cacheinfo.aci_cstamp*10,
				      dstr, sizeof(dstr)));
		    
		fprintf(fd, "updated boot_time+[%s]\n",
			nl_msec2str_r(a->a_cacheinfo.aci_tstamp*10,
				      dstr, sizeof(dstr)));
	}

	return line;
}

static int addr_dump_with_stats(struct nl_cache *c, struct nl_common *n,
				FILE *fd, struct nl_dump_params *params)
{
	return addr_dump_full(c, n, fd, params);
}

static int addr_filter(struct nl_common *obj, struct nl_common *filter)
{
	struct rtnl_addr *o = (struct rtnl_addr *) obj;
	struct rtnl_addr *f = (struct rtnl_addr *) filter;

	if (obj->ce_type != RTNL_ADDR || filter->ce_type != RTNL_ADDR)
		return 0;

#define REQ(F) (f->a_mask & ADDR_HAS_##F)
#define AVAIL(F) (o->a_mask & ADDR_HAS_##F)
	if ((REQ(IFINDEX)   &&
	      (!AVAIL(IFINDEX)   || o->a_ifindex != f->a_ifindex))	||
	    (REQ(FAMILY)    &&
	      (!AVAIL(FAMILY)    || o->a_family != f->a_family))	||
	    (REQ(SCOPE)     &&
	      (!AVAIL(SCOPE)     || o->a_scope != f->a_scope))		||
	    (REQ(FLAGS)     &&
	      (!AVAIL(FLAGS)     || o->a_flags != f->a_flags))		||
	    (REQ(FLAGS)     &&
	      (!AVAIL(FLAGS)     || f->a_flags ^
					(o->a_flags & f->a_flag_mask))) ||
	    (REQ(LABEL)     &&
	      (!AVAIL(LABEL)     || strcmp(o->a_label, f->a_label)))	||
	    (REQ(PEER)      &&
	      (!AVAIL(PEER)      || nl_addrcmp(&o->a_peer,
					       &f->a_peer)))		||
	    (REQ(LOCAL)     &&
	      (!AVAIL(LOCAL)     || nl_addrcmp(&o->a_local,
					       &f->a_local)))		||
	    (REQ(ANYCAST)   &&
	      (!AVAIL(ANYCAST)   || nl_addrcmp(&o->a_anycast,
					       &f->a_anycast)))		||
	    (REQ(BROADCAST) &&
	      (!AVAIL(BROADCAST) || nl_addrcmp(&o->a_bcast, &f->a_bcast))))
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
 * Build a address cache including all addresses currently configured in the kernel.
 * @arg handle		netlink handle
 *
 * Allocates a new address cache, initializes it properly and updates it
 * to include all addresses currently configured in the kernel.
 *
 * @note The caller is responsible for destroying and freeing the
 *       cache after using it. (nl_cache_destroy_and_free())
 * @return The new cache or NULL if an error occured.
 */
struct nl_cache * rtnl_addr_build_cache(struct nl_handle *handle)
{
	struct nl_cache * cache = calloc(1, sizeof(*cache));

	if (cache == NULL)
		return NULL;

	cache->c_type = RTNL_ADDR;
	cache->c_type_size = sizeof(struct rtnl_addr);
	cache->c_ops = &rtnl_addr_ops;

	if (nl_cache_update(handle, cache) < 0) {
		free(cache);
		return NULL;
	}

	return cache;
}

/**
 * Dump address attributes
 * @arg addr		address to dump
 * @arg fd		file descriptor
 * @arg params		dumping parameters
 */
void rtnl_addr_dump(struct rtnl_addr *addr, FILE *fd,
		    struct nl_dump_params *params)
{
	dump_from_ops((struct nl_common *) addr, fd, params, &rtnl_addr_ops);
}


/** @} */

/**
 * @name Address Addition/Deletion
 * @{
 */

static struct nl_msg *build_addr_msg(struct rtnl_addr *tmpl, int cmd, int flags)
{
	struct nl_msg *m;
	struct nlmsghdr n = {
		.nlmsg_type = cmd,
		.nlmsg_flags = flags,
	};
	struct ifaddrmsg am = { 0 };

	if (tmpl->a_mask & ADDR_HAS_FAMILY)
		am.ifa_family = tmpl->a_family;

	if (tmpl->a_mask & ADDR_HAS_IFINDEX)
		am.ifa_index = tmpl->a_ifindex;

	if (tmpl->a_mask & ADDR_HAS_PREFIXLEN)
		am.ifa_prefixlen = tmpl->a_prefixlen;

	if (tmpl->a_mask & ADDR_HAS_SCOPE)
		am.ifa_scope = tmpl->a_scope;
	else {
		/* compatibility hack */
		if (tmpl->a_family == AF_INET &&
		    tmpl->a_mask & ADDR_HAS_LOCAL &&
		    tmpl->a_local.a_addr[0] == 127)
			am.ifa_scope = RT_SCOPE_HOST;
	}

	m = nl_msg_build(&n);
	nl_msg_append_raw(m, &am, sizeof(am));

	if (tmpl->a_mask & ADDR_HAS_LOCAL)
		nl_msg_append_tlv(m, IFA_LOCAL, &tmpl->a_local.a_addr,
				  tmpl->a_local.a_len);

	if (tmpl->a_mask & ADDR_HAS_PEER)
		nl_msg_append_tlv(m, IFA_ADDRESS, &tmpl->a_peer.a_addr,
				  tmpl->a_peer.a_len);
	else
		nl_msg_append_tlv(m, IFA_ADDRESS, &tmpl->a_local.a_addr,
				  tmpl->a_local.a_len);

	if (tmpl->a_mask & ADDR_HAS_LABEL)
		nl_msg_append_tlv(m, IFA_LABEL, &tmpl->a_label,
				  strlen(tmpl->a_label)+1);

	if (tmpl->a_mask & ADDR_HAS_BROADCAST)
		nl_msg_append_tlv(m, IFA_BROADCAST, &tmpl->a_bcast.a_addr,
				  tmpl->a_bcast.a_len);

	if (tmpl->a_mask & ADDR_HAS_ANYCAST)
		nl_msg_append_tlv(m, IFA_ANYCAST, &tmpl->a_anycast.a_addr,
				  tmpl->a_anycast.a_len);

	return m;
}

/**
 * Build netlink request message to add a new address
 * @arg tmpl		template with data of new address
 *
 * Builds a new netlink message requesting a addition of a new
 * address. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a tmpl must contain the attributes of the new
 * address set via \c rtnl_addr_set_* functions.
 * 
 * The following attributes must be set in the template:
 *  - Interface index (rtnl_neigh_set_ifindex())
 *  - State (rtnl_neigh_set_state())
 *  - Destination address (rtnl_neigh_set_dst())
 *  - Link layer address (rtnl_neigh_set_lladdr())
 *
 * @return The netlink message
 */
struct nl_msg * rtnl_addr_build_add_request(struct rtnl_addr *tmpl)
{
	return build_addr_msg(tmpl, RTM_NEWADDR, NLM_F_CREATE);
}

/**
 * Add a new address
 * @arg handle		netlink handle
 * @arg tmpl		template with requested changes
 *
 * Builds a netlink message by calling rtnl_addr_build_add_request(),
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
int rtnl_addr_add(struct nl_handle *handle, struct rtnl_addr *tmpl)
{
	int err;
	struct nl_msg *m = rtnl_addr_build_add_request(tmpl);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/**
 * Build a netlink request message to delete an address
 * @arg addr		address to delete
 *
 * Builds a new netlink message requesting a deletion of an address.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed. \a addr must point to an existing
 * address.
 *
 * @return The netlink message
 */
struct nl_msg * rtnl_addr_build_delete_request(struct rtnl_addr *addr)
{
	return build_addr_msg(addr, RTM_DELADDR, 0);
}

/**
 * Delete an address
 * @arg handle		netlink handle
 * @arg addr		address to delete
 *
 * Builds a netlink message by calling rtnl_addr_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_addr_delete(struct nl_handle *handle, struct rtnl_addr *addr)
{
	int err;
	struct nl_msg *m = rtnl_addr_build_delete_request(addr);

	if ((err = nl_send_auto_complete(handle, nl_msg_get(m))) < 0)
		return err;

	nl_msg_free(m);
	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name Attribute Modifications
 * @{
 */

/**
 * Set new address label
 * @arg addr		address to change
 * @arg label		new address label
 */
void rtnl_addr_set_label(struct rtnl_addr *addr, const char *label)
{
	strncpy(addr->a_label, label, sizeof(addr->a_label) - 1);
	addr->a_mask |= ADDR_HAS_LABEL;
}

/**
 * Set interface index of device this address is on
 * @arg addr		address to change
 * @arg ifindex		new interface index
 */
void rtnl_addr_set_ifindex(struct rtnl_addr *addr, int ifindex)
{
	addr->a_ifindex = ifindex;
	addr->a_mask |= ADDR_HAS_IFINDEX;
}

/**
 * Set interface index via interface name
 * @arg addr		address to change
 * @arg cache		link cache holding all links
 * @arg name		name of interface
 * @return 0 on success or a negative error code
 */
int rtnl_addr_set_ifindex_name(struct rtnl_addr *addr,
			       struct nl_cache *cache, const char *name)
{
	int i;

	if (cache->c_type != RTNL_LINK)
		return nl_error(EINVAL, "Must be link cache");

	i = rtnl_link_name2i(cache, name);

	if (RTNL_LINK_NOT_FOUND == i)
		return nl_error(ENOENT, "Link %s is unknown", name);

	rtnl_addr_set_ifindex(addr, i);
	return 0;
}

/**
 * Set the address family of an address to the specified value
 * @arg addr		address to change
 * @arg family		new address family
 */
void rtnl_addr_set_family(struct rtnl_addr *addr, int family)
{
	addr->a_family = family;
	addr->a_mask |= ADDR_HAS_FAMILY;
}

/**
 * Set the prefix length of an address to the specified value
 * @arg addr		address to change
 * @arg prefix		new prefix length
 */
void rtnl_addr_set_prefix(struct rtnl_addr *addr, int prefix)
{
	addr->a_prefixlen = prefix;
	addr->a_mask |= ADDR_HAS_PREFIXLEN;
}

/**
 * Set the scope of an address to the specified value
 * @arg addr		address to change
 * @arg scope		new scope
 */
void rtnl_addr_set_scope(struct rtnl_addr *addr, int scope)
{
	addr->a_scope = scope;
	addr->a_mask |= ADDR_HAS_SCOPE;
}

/**
 * Set the scope of an address to the specified string value
 * @arg addr		address to change
 * @arg scope		new scope as string
 */
int rtnl_addr_set_scope_str(struct rtnl_addr *addr, char *scope)
{
	int iscope = rtnl_str2scope(scope);

	if (iscope < 0)
		return nl_error(ENOENT, "Unknown scope \"%s\"", scope);

	rtnl_addr_set_scope(addr, iscope);
	return 0;
}

/**
 * Add flags to an address
 * @arg addr		address to change
 * @arg flags		flags to set
 */
void rtnl_addr_set_flags(struct rtnl_addr *addr, int flags)
{
	addr->a_flag_mask |= flags;
	addr->a_flags |= flags;
	addr->a_mask |= ADDR_HAS_FLAGS;
}

/**
 * Remove flags from an address
 * @arg addr		address to change
 * @arg flags		flags to unset
 */
void rtnl_addr_unset_flags(struct rtnl_addr *addr, int flags)
{
	addr->a_flag_mask |= flags;
	addr->a_flags &= ~flags;
	addr->a_mask |= ADDR_HAS_FLAGS;
}

/**
 * Set the local address of an address
 * @arg addr		address to change
 * @arg local		new local address
 *
 * Assigns the new local address to the specified address handle. The
 * address is validated against the address family if set already via
 * either rtnl_addr_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 *
 * @note The address may not contain a prefix length if the peer address
 *       has been specified already.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_local(struct rtnl_addr *addr, struct nl_addr *local)
{
	if (addr->a_mask & ADDR_HAS_PEER && local->a_prefix)
		return nl_error(EINVAL, "Local address cannot be a prefix " \
		    "if a peer has been specified.");

	if (addr->a_mask & ADDR_HAS_FAMILY) {
		if (local->a_family != addr->a_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		addr->a_family = local->a_family;
		
	memcpy(&addr->a_local, local, sizeof(addr->a_local));
	if (!(addr->a_mask & ADDR_HAS_PEER)) {
		addr->a_prefixlen = addr->a_local.a_prefix;
		addr->a_mask |= ADDR_HAS_PREFIXLEN;
	}
	addr->a_mask |= (ADDR_HAS_LOCAL | ADDR_HAS_FAMILY);
	return 0;
}

/**
 * Set the local address of an address
 * @arg addr		address to change
 * @arg local		new local address as string
 *
 * Translates the specified local address to a binary format and assigns
 * it as the new local address by calling rtnl_addr_set_local().
 *
 * @see rtnl_addr_set_local()
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_local_str(struct rtnl_addr *addr, const char *local)
{
	int err;
	struct nl_addr a = {0};
	int hint = addr->a_mask & ADDR_HAS_FAMILY ? addr->a_family : AF_UNSPEC;
	
	err = nl_str2addr(local, &a, hint);
	if (err < 0)
		return err;

	return rtnl_addr_set_local(addr, &a);
}

/**
 * Set the peer address of an address
 * @arg addr		address to change
 * @arg peer		new peer address
 *
 * Assigns the new peer address to the specified address handle. The
 * address is validated against the address family if set already via
 * either rtnl_addr_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_peer(struct rtnl_addr *addr, struct nl_addr *peer)
{
	if (addr->a_mask & ADDR_HAS_FAMILY) {
		if (peer->a_family != addr->a_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		addr->a_family = peer->a_family;
		
	memcpy(&addr->a_peer, peer, sizeof(addr->a_peer));
	addr->a_prefixlen = addr->a_peer.a_prefix;
	addr->a_mask |= (ADDR_HAS_PEER | ADDR_HAS_FAMILY | ADDR_HAS_PREFIXLEN);
	return 0;
}

/**
 * Set the peer address of an address
 * @arg addr		address to change
 * @arg peer		new peer address as string
 *
 * Translates the specified peer address to a binary format and assigns
 * it as the new peer address by calling rtnl_addr_set_peer().
 *
 * @see rtnl_addr_set_peer()
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_peer_str(struct rtnl_addr *addr, const char *peer)
{
	int err;
	struct nl_addr a = {0};
	int hint = addr->a_mask & ADDR_HAS_FAMILY ? addr->a_family : AF_UNSPEC;
	
	err = nl_str2addr(peer, &a, hint);
	if (err < 0)
		return err;

	return rtnl_addr_set_peer(addr, &a);
}

/**
 * Set the broadcast address of an address
 * @arg addr		address to change
 * @arg bcast		new broadcast address
 *
 * Assigns the new broadcast address to the specified address handle. The
 * address is validated against the address family if set already via
 * either rtnl_addr_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_broadcast(struct rtnl_addr *addr, struct nl_addr *bcast)
{
	if (addr->a_mask & ADDR_HAS_FAMILY) {
		if (bcast->a_family != addr->a_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		addr->a_family = bcast->a_family;
		
	memcpy(&addr->a_bcast, bcast, sizeof(addr->a_bcast));
	addr->a_mask |= (ADDR_HAS_BROADCAST | ADDR_HAS_FAMILY);
	return 0;
}

/**
 * Set the broadcast address of an address
 * @arg addr		address to change
 * @arg bcast		new broadcast address as string
 *
 * Translates the specified broadcast address to a binary format and
 * assigns it as the new broadcast address by calling
 * rtnl_addr_set_broadcast().
 *
 * @see rtnl_addr_set_broadcast()
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_broadcast_str(struct rtnl_addr *addr, const char *bcast)
{
	int err;
	struct nl_addr a = {0};
	int hint = addr->a_mask & ADDR_HAS_FAMILY ? addr->a_family : AF_UNSPEC;
	
	err = nl_str2addr(bcast, &a, hint);
	if (err < 0)
		return err;

	return rtnl_addr_set_broadcast(addr, &a);
}

/**
 * Set the anycast address of an address
 * @arg addr		address to change
 * @arg anycast		new anycast address
 *
 * Assigns the new anycast address to the specified address handle. The
 * address is validated against the address family if set already via
 * either rtnl_addr_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_anycast(struct rtnl_addr *addr, struct nl_addr *anycast)
{
	if (addr->a_mask & ADDR_HAS_FAMILY) {
		if (anycast->a_family != addr->a_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		addr->a_family = anycast->a_family;
		
	memcpy(&addr->a_anycast, anycast, sizeof(addr->a_anycast));
	addr->a_mask |= (ADDR_HAS_ANYCAST | ADDR_HAS_FAMILY);
	return 0;
}

/**
 * Set the anycast address of an address
 * @arg addr		address to change
 * @arg anycast		new anycast address as string
 *
 * Translates the specified anycast address to a binary format and
 * assigns it as the new anycast address by calling rtnl_addr_set_anycast().
 *
 * @see rtnl_addr_set_anycast()
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_anycast_addr(struct rtnl_addr *addr, const char *anycast)
{
	int err;
	struct nl_addr a = {0};
	int hint = addr->a_mask & ADDR_HAS_FAMILY ? addr->a_family : AF_UNSPEC;
	
	err = nl_str2addr(anycast, &a, hint);
	if (err < 0)
		return err;

	return rtnl_addr_set_anycast(addr, &a);
}

/** @} */

/**
 * @name Address Flags Translations
 * @{
 */

static struct trans_tbl addr_flags[] = {
	__ADD(IFA_F_SECONDARY, secondary)
	__ADD(IFA_F_DEPRECATED, deprecated)
	__ADD(IFA_F_TENTATIVE, tentative)
	__ADD(IFA_F_PERMANENT, permanent)
};

/**
 * Convert address flags to a character string.
 * @arg flags		address flags
 *
 * Converts address flags to a character string separated by
 * commas and stores it in a static buffer.
 *
 * @return A static buffer
 * @attention This funnction is NOT thread safe.
 */
char * rtnl_addr_flags2str(int flags)
{
	static char buf[128];
	memset(buf, 0, sizeof(buf));
	return __flags2str_r(flags, buf, sizeof(buf), addr_flags,
			     ARRAY_SIZE(addr_flags));
}

/**
 * Convert address flags to a character string (Reentrant).
 * @arg flags		address flags
 * @arg buf		destination buffer
 * @arg len		buffer length
 *
 * Converts address flags to a character string separated by
 * commands and stores it in the specified destination buffer.
 *
 * \return The destination buffer
 */
char * rtnl_addr_flags2str_r(int flags, char *buf, size_t len)
{
	return __flags2str_r(flags, buf, len, addr_flags,
			     ARRAY_SIZE(addr_flags));
}

/**
 * Convert a character string to a address flag
 * @arg name		Name of address flag
 *
 * Converts the provided character string specifying an
 * address flag to the corresponding numeric value.
 *
 * \return Address flag or a negative value if none was found.
 */
int rtnl_addr_str2flags(const char *name)
{
	return __str2type(name, addr_flags, ARRAY_SIZE(addr_flags));
}

/** @} */

struct nl_cache_ops rtnl_addr_ops = {
	.co_request_update      = addr_request_update,
	.co_msg_parser          = addr_msg_parser,
	.co_dump[NL_DUMP_BRIEF] = addr_dump_brief,
	.co_dump[NL_DUMP_FULL]  = addr_dump_full,
	.co_dump[NL_DUMP_STATS] = addr_dump_with_stats,
	.co_filter              = addr_filter,
};

/** @} */
