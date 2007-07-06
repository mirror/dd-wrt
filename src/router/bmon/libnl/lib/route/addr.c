/*
 * lib/route/addr.c		Addresses
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 *                         Baruch Even <baruch@ev-en.org>,
 *                         Mediatrix Telecom, inc. <ericb@mediatrix.com>
 */

/**
 * @ingroup rtnl
 * @defgroup rtaddr Addresses
 * @brief
 *
 * @par 1) Address Addition
 * @code
 * // Allocate an empty address object to be filled out with the attributes
 * // of the new address.
 * struct rtnl_addr *addr = rtnl_addr_alloc();
 *
 * // Fill out the mandatory attributes of the new address. Setting the
 * // local address will automatically set the address family and the
 * // prefix length to the correct values.
 * rtnl_addr_set_ifindex(addr, ifindex);
 * rtnl_addr_set_local(addr, local_addr);
 *
 * // The label of the address can be specified, currently only supported
 * // by IPv4 and DECnet.
 * rtnl_addr_set_label(addr, "mylabel");
 *
 * // The peer address can be specified if necessary, in either case a peer
 * // address will be sent to the kernel in order to fullfil the interface
 * // requirements. If none is set, it will equal the local address.
 * // Note: Real peer addresses are only supported by IPv4 for now.
 * rtnl_addr_set_peer(addr, peer_addr);
 *
 * // In case you want to have the address have a scope other than global
 * // it may be overwritten using rtnl_addr_set_scope(). The scope currently
 * // cannot be set for IPv6 addresses.
 * rtnl_addr_set_scope(addr, rtnl_str2scope("site"));
 *
 * // Broadcast and anycast address may be specified using the relevant
 * // functions, the address family will be verified if one of the other
 * // addresses has been set already. Currently only works for IPv4.
 * rtnl_addr_set_broadcast(addr, broadcast_addr);
 * rtnl_addr_set_anycast(addr, anycast_addr);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_addr_build_add_request() to be
 * // sent out using nl_send_auto_complete().
 * rtnl_addr_add(handle, addr, 0);
 *
 * // Free the memory
 * rtnl_addr_put(addr);
 * @endcode
 *
 * @par 2) Address Deletion
 * @code
 * // Allocate an empty address object to be filled out with the attributes
 * // matching the address to be deleted. Alternatively a fully equipped
 * // address object out of a cache can be used instead.
 * struct rtnl_addr *addr = rtnl_addr_alloc();
 *
 * // The only mandatory parameter besides the address family is the interface
 * // index the address is on, i.e. leaving out all other parameters will
 * // result in all addresses of the specified address family interface tuple
 * // to be deleted.
 * rtnl_addr_set_ifindex(addr, ifindex);
 *
 * // Specyfing the address family manually is only required if neither the
 * // local nor peer address have been specified.
 * rtnl_addr_set_family(addr, AF_INET);
 *
 * // Specyfing the local address is optional but the best choice to delete
 * // specific addresses.
 * rtnl_addr_set_local(addr, local_addr);
 *
 * // The label of the address can be specified, currently only supported
 * // by IPv4 and DECnet.
 * rtnl_addr_set_label(addr, "mylabel");
 *
 * // The peer address can be specified if necessary, in either case a peer
 * // address will be sent to the kernel in order to fullfil the interface
 * // requirements. If none is set, it will equal the local address.
 * // Note: Real peer addresses are only supported by IPv4 for now.
 * rtnl_addr_set_peer(addr, peer_addr);
 *
 * // Build the netlink message and send it to the kernel, the operation will
 * // block until the operation has been completed. Alternatively the required
 * // netlink message can be built using rtnl_addr_build_delete_request()
 * // to be sent out using nl_send_auto_complete().
 * rtnl_addr_delete(handle, addr, 0);
 *
 * // Free the memory
 * rtnl_addr_put(addr);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/addr.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define ADDR_ATTR_FAMILY	0x0001
#define ADDR_ATTR_PREFIXLEN	0x0002
#define ADDR_ATTR_FLAGS		0x0004
#define ADDR_ATTR_SCOPE		0x0008
#define ADDR_ATTR_IFINDEX	0x0010
#define ADDR_ATTR_LABEL		0x0020
#define ADDR_ATTR_CACHEINFO	0x0040
#define ADDR_ATTR_PEER		0x0080
#define ADDR_ATTR_LOCAL		0x0100
#define ADDR_ATTR_BROADCAST	0x0200
#define ADDR_ATTR_ANYCAST	0x0400
#define ADDR_ATTR_MULTICAST	0x0800

static struct nl_cache_ops rtnl_addr_ops;
/** @endcond */

static void addr_free_data(struct nl_object *obj)
{
	struct rtnl_addr *addr = nl_object_priv(obj);

	if (!addr)
		return;

	nl_addr_put(addr->a_peer);
	nl_addr_put(addr->a_local);
	nl_addr_put(addr->a_bcast);
	nl_addr_put(addr->a_anycast);
	nl_addr_put(addr->a_multicast);
}

static struct nla_policy addr_policy[IFA_MAX+1] = {
	[IFA_LABEL]	= { .type = NLA_STRING,
			    .maxlen = IFNAMSIZ },
	[IFA_CACHEINFO]	= { .minlen = sizeof(struct ifa_cacheinfo) },
};

static int addr_msg_parser(struct sockaddr_nl *who, struct nlmsghdr *nlh,
			   void *arg)
{
	struct rtnl_addr *addr;
	struct nl_parser_param *pp = arg;
	struct ifaddrmsg *ifa;
	struct nlattr *tb[IFA_MAX+1];
	int err = -ENOMEM, peer_prefix = 0;

	addr = rtnl_addr_alloc();
	if (!addr) {
		err = nl_errno(ENOMEM);
		goto errout;
	}
	addr->ce_msgtype = nlh->nlmsg_type;

	err = nlmsg_parse(nlh, sizeof(*ifa), tb, IFA_MAX, addr_policy);
	if (err < 0)
		goto errout_free;

	ifa = nlmsg_data(nlh);
	addr->a_family = ifa->ifa_family;
	addr->a_prefixlen = ifa->ifa_prefixlen;
	addr->a_flags = ifa->ifa_flags;
	addr->a_scope = ifa->ifa_scope;
	addr->a_ifindex = ifa->ifa_index;

	addr->a_mask = (ADDR_ATTR_FAMILY | ADDR_ATTR_PREFIXLEN |
			ADDR_ATTR_FLAGS | ADDR_ATTR_SCOPE | ADDR_ATTR_IFINDEX);

	if (tb[IFA_LABEL]) {
		nla_strlcpy(addr->a_label, tb[IFA_LABEL], IFNAMSIZ);
		addr->a_mask |= ADDR_ATTR_LABEL;
	}

	if (tb[IFA_CACHEINFO]) {
		struct ifa_cacheinfo *ca;
		
		ca = nla_data(tb[IFA_CACHEINFO]);
		addr->a_cacheinfo.aci_prefered = ca->ifa_prefered;
		addr->a_cacheinfo.aci_valid = ca->ifa_valid;
		addr->a_cacheinfo.aci_cstamp = ca->cstamp;
		addr->a_cacheinfo.aci_tstamp = ca->tstamp;
		addr->a_mask |= ADDR_ATTR_CACHEINFO;
	}

	if (tb[IFA_LOCAL]) {
		addr->a_local = nla_get_addr(tb[IFA_LOCAL], addr->a_family);
		if (!addr->a_local)
			goto errout_free;
		addr->a_mask |= ADDR_ATTR_LOCAL;
	}

	if (tb[IFA_ADDRESS]) {
		struct nl_addr *a;

		a = nla_get_addr(tb[IFA_ADDRESS], addr->a_family);
		if (!a)
			goto errout_free;

		/* IPv6 sends the local address as IFA_ADDRESS with
		 * no IFA_LOCAL, IPv4 sends both IFA_LOCAL and IFA_ADDRESS
		 * with IFA_ADDRESS being the peer address if they differ */
		if (!tb[IFA_LOCAL] || !nl_addr_cmp(a, addr->a_local)) {
			nl_addr_put(addr->a_local);
			addr->a_local = a;
			addr->a_mask |= ADDR_ATTR_LOCAL;
		} else {
			addr->a_peer = a;
			addr->a_mask |= ADDR_ATTR_PEER;
			peer_prefix = 1;
		}
	}

	nl_addr_set_prefixlen(peer_prefix ? addr->a_peer : addr->a_local,
			      addr->a_prefixlen);

	if (tb[IFA_BROADCAST]) {
		addr->a_bcast = nla_get_addr(tb[IFA_BROADCAST], addr->a_family);
		if (!addr->a_bcast)
			goto errout_free;

		addr->a_mask |= ADDR_ATTR_BROADCAST;
	}

	if (tb[IFA_ANYCAST]) {
		addr->a_anycast = nla_get_addr(tb[IFA_ANYCAST], addr->a_family);
		if (!addr->a_anycast)
			goto errout_free;

		addr->a_mask |= ADDR_ATTR_ANYCAST;
	}

	if (tb[IFA_MULTICAST]) {
		addr->a_multicast = nla_get_addr(tb[IFA_MULTICAST],
						 addr->a_family);
		if (!addr->a_multicast)
			goto errout_free;

		addr->a_mask |= ADDR_ATTR_MULTICAST;
	}

	err = pp->pp_cb((struct nl_object *) addr, pp);
	if (err < 0)
		goto errout_free;

	return P_ACCEPT;

errout_free:
	rtnl_addr_free(addr);
errout:
	return err;
}

static int addr_request_update(struct nl_cache *cache, struct nl_handle *handle)
{
	return nl_rtgen_request(handle, RTM_GETADDR, AF_UNSPEC, NLM_F_DUMP);
}

static int addr_dump_brief(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	struct nl_cache *link_cache;
	char buf[128];

	link_cache = nl_cache_mngt_require("route/link");

	if (addr->a_mask & ADDR_ATTR_LOCAL)
		dp_dump(p, "%s",
			nl_addr2str(addr->a_local, buf, sizeof(buf)));
	else
		dp_dump(p, "none");

	if (addr->a_mask & ADDR_ATTR_PEER)
		dp_dump(p, " peer %s",
			nl_addr2str(addr->a_peer, buf, sizeof(buf)));

	dp_dump(p, " %s ", nl_af2str(addr->a_family, buf, sizeof(buf)));

	if (link_cache)
		dp_dump(p, "dev %s ",
			rtnl_link_i2name(link_cache, addr->a_ifindex,
					 buf, sizeof(buf)));
	else
		dp_dump(p, "dev %d ", addr->a_ifindex);

	dp_dump(p, "scope %s",
		rtnl_scope2str(addr->a_scope, buf, sizeof(buf)));

	rtnl_addr_flags2str(addr->a_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump(p, " <%s>", buf);

	dp_dump(p, "\n");

	return 1;
}

static int addr_dump_full(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	int line = addr_dump_brief(obj, p);
	char buf[128];

	if (addr->a_mask & (ADDR_ATTR_LABEL | ADDR_ATTR_BROADCAST |
			    ADDR_ATTR_ANYCAST | ADDR_ATTR_MULTICAST)) {
		dp_dump_line(p, line++, "  ");

		if (addr->a_mask & ADDR_ATTR_LABEL)
			dp_dump(p, " label %s", addr->a_label);

		if (addr->a_mask & ADDR_ATTR_BROADCAST)
			dp_dump(p, " broadcast %s",
				nl_addr2str(addr->a_bcast, buf, sizeof(buf)));

		if (addr->a_mask & ADDR_ATTR_ANYCAST)
			dp_dump(p, " anycast %s",
				nl_addr2str(addr->a_anycast, buf,
					      sizeof(buf)));

		if (addr->a_mask & ADDR_ATTR_MULTICAST)
			dp_dump(p, " multicast %s",
				nl_addr2str(addr->a_multicast, buf,
					      sizeof(buf)));

		dp_dump(p, "\n");
	}

	if (addr->a_mask & ADDR_ATTR_CACHEINFO) {
		struct rtnl_addr_cacheinfo *ci = &addr->a_cacheinfo;

		dp_dump_line(p, line++, "   valid-lifetime %s",
			     ci->aci_valid == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_valid * 1000,
					   buf, sizeof(buf)));

		dp_dump(p, " preferred-lifetime %s\n",
			ci->aci_prefered == 0xFFFFFFFFU ? "forever" :
			nl_msec2str(ci->aci_prefered * 1000,
				      buf, sizeof(buf)));

		dp_dump_line(p, line++, "   created boot-time+%s ",
			     nl_msec2str(addr->a_cacheinfo.aci_cstamp * 10,
					   buf, sizeof(buf)));
		    
		dp_dump(p, "last-updated boot-time+%s\n",
			nl_msec2str(addr->a_cacheinfo.aci_tstamp * 10,
				      buf, sizeof(buf)));
	}

	return line;
}

static int addr_dump_stats(struct nl_object *obj, struct nl_dump_params *p)
{
	return addr_dump_full(obj, p);
}

static int addr_dump_xml(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	struct nl_cache *link_cache;
	char buf[128];
	int line = 0;

	dp_dump_line(p, line++, "<address>\n");
	dp_dump_line(p, line++, "  <family>%s</family>\n",
		     nl_af2str(addr->a_family, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_LOCAL)
		dp_dump_line(p, line++, "  <local>%s</local>\n",
			     nl_addr2str(addr->a_local, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_PEER)
		dp_dump_line(p, line++, "  <peer>%s</peer>\n",
			     nl_addr2str(addr->a_peer, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_BROADCAST)
		dp_dump_line(p, line++, "  <broadcast>%s</broadcast>\n",
			     nl_addr2str(addr->a_bcast, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_ANYCAST)
		dp_dump_line(p, line++, "  <anycast>%s</anycast>\n",
			     nl_addr2str(addr->a_anycast, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_MULTICAST)
		dp_dump_line(p, line++, "  <multicast>%s</multicast>\n",
			     nl_addr2str(addr->a_multicast, buf,
					   sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_PREFIXLEN)
		dp_dump_line(p, line++, "  <prefixlen>%u</prefixlen>\n",
			     addr->a_prefixlen);
	link_cache = nl_cache_mngt_require("route/link");

	if (link_cache)
		dp_dump_line(p, line++, "  <device>%s</device>\n",
			     rtnl_link_i2name(link_cache, addr->a_ifindex,
			     		      buf, sizeof(buf)));
	else
		dp_dump_line(p, line++, "  <device>%u</device>\n",
			     addr->a_ifindex);

	if (addr->a_mask & ADDR_ATTR_SCOPE)
		dp_dump_line(p, line++, "  <scope>%s</scope>\n",
			     rtnl_scope2str(addr->a_scope, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_LABEL)
		dp_dump_line(p, line++, "  <label>%s</label>\n", addr->a_label);

	rtnl_addr_flags2str(addr->a_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "  <flags>%s</flags>\n", buf);

	if (addr->a_mask & ADDR_ATTR_CACHEINFO) {
		struct rtnl_addr_cacheinfo *ci = &addr->a_cacheinfo;

		dp_dump_line(p, line++, "  <cacheinfo>\n");

		dp_dump_line(p, line++, "    <valid>%s</valid>\n",
			     ci->aci_valid == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_valid * 1000,
					   buf, sizeof(buf)));

		dp_dump_line(p, line++, "    <prefered>%s</prefered>\n",
			     ci->aci_prefered == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_prefered * 1000,
					 buf, sizeof(buf)));

		dp_dump_line(p, line++, "    <created>%s</created>\n",
			     nl_msec2str(addr->a_cacheinfo.aci_cstamp * 10,
					 buf, sizeof(buf)));

		dp_dump_line(p, line++, "    <last-update>%s</last-update>\n",
			     nl_msec2str(addr->a_cacheinfo.aci_tstamp * 10,
					 buf, sizeof(buf)));

		dp_dump_line(p, line++, "  </cacheinfo>\n");
	}

	dp_dump_line(p, line++, "</address>\n");

	return line;
}

static int addr_dump_env(struct nl_object *obj, struct nl_dump_params *p)
{
	struct rtnl_addr *addr = (struct rtnl_addr *) obj;
	struct nl_cache *link_cache;
	char buf[128];
	int line = 0;

	dp_dump_line(p, line++, "ADDR_FAMILY=%s\n",
		     nl_af2str(addr->a_family, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_LOCAL)
		dp_dump_line(p, line++, "ADDR_LOCAL=%s\n",
			     nl_addr2str(addr->a_local, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_PEER)
		dp_dump_line(p, line++, "ADDR_PEER=%s\n",
			     nl_addr2str(addr->a_peer, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_BROADCAST)
		dp_dump_line(p, line++, "ADDR_BROADCAST=%s\n",
			     nl_addr2str(addr->a_bcast, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_ANYCAST)
		dp_dump_line(p, line++, "ADDR_ANYCAST=%s\n",
			     nl_addr2str(addr->a_anycast, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_MULTICAST)
		dp_dump_line(p, line++, "ADDR_MULTICAST=%s\n",
			     nl_addr2str(addr->a_multicast, buf,
					   sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_PREFIXLEN)
		dp_dump_line(p, line++, "ADDR_PREFIXLEN=%u\n",
			     addr->a_prefixlen);
	link_cache = nl_cache_mngt_require("route/link");

	dp_dump_line(p, line++, "ADDR_IFINDEX=%u\n", addr->a_ifindex);
	if (link_cache)
		dp_dump_line(p, line++, "ADDR_IFNAME=%s\n",
			     rtnl_link_i2name(link_cache, addr->a_ifindex,
			     		      buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_SCOPE)
		dp_dump_line(p, line++, "ADDR_SCOPE=%s\n",
			     rtnl_scope2str(addr->a_scope, buf, sizeof(buf)));

	if (addr->a_mask & ADDR_ATTR_LABEL)
		dp_dump_line(p, line++, "ADDR_LABEL=%s\n", addr->a_label);

	rtnl_addr_flags2str(addr->a_flags, buf, sizeof(buf));
	if (buf[0])
		dp_dump_line(p, line++, "ADDR_FLAGS=%s\n", buf);

	if (addr->a_mask & ADDR_ATTR_CACHEINFO) {
		struct rtnl_addr_cacheinfo *ci = &addr->a_cacheinfo;

		dp_dump_line(p, line++, "ADDR_CACHEINFO_VALID=%s\n",
			     ci->aci_valid == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_valid * 1000,
					   buf, sizeof(buf)));

		dp_dump_line(p, line++, "ADDR_CACHEINFO_PREFERED=%s\n",
			     ci->aci_prefered == 0xFFFFFFFFU ? "forever" :
			     nl_msec2str(ci->aci_prefered * 1000,
					 buf, sizeof(buf)));

		dp_dump_line(p, line++, "ADDR_CACHEINFO_CREATED=%s\n",
			     nl_msec2str(addr->a_cacheinfo.aci_cstamp * 10,
					 buf, sizeof(buf)));

		dp_dump_line(p, line++, "ADDR_CACHEINFO_LASTUPDATE=%s\n",
			     nl_msec2str(addr->a_cacheinfo.aci_tstamp * 10,
					 buf, sizeof(buf)));
	}

	return line;
}

static int addr_filter(struct nl_object *obj, struct nl_object *filter)
{
	struct rtnl_addr *o = (struct rtnl_addr *) obj;
	struct rtnl_addr *f = (struct rtnl_addr *) filter;

#define REQ(F) (f->a_mask & ADDR_ATTR_##F)
#define AVAIL(F) (o->a_mask & ADDR_ATTR_##F)
#define _O(F, EXPR) (REQ(F) && (!AVAIL(F) || (EXPR)))
#define _C(F, N) (REQ(F) && (!AVAIL(F) || (o->N != f->N)))
	if (_C(IFINDEX,	   a_ifindex)					||
	    _C(FAMILY,	   a_family)					||
	    _C(SCOPE,	   a_scope)					||
	    _O(FLAGS,	   f->a_flags ^ (o->a_flags & f->a_flag_mask))	||
	    _O(LABEL,	   strcmp(o->a_label, f->a_label))		||
	    _O(PEER, 	   nl_addr_cmp(o->a_peer, f->a_peer))		||
	    _O(LOCAL,	   nl_addr_cmp(o->a_local, f->a_local))		||
	    _O(ANYCAST,	   nl_addr_cmp(o->a_anycast, f->a_anycast))	||
	    _O(MULTICAST,  nl_addr_cmp(o->a_multicast, f->a_multicast))	||
	    _O(BROADCAST,  nl_addr_cmp(o->a_bcast, f->a_bcast)))
		return 0;
#undef REQ
#undef AVAIL
#undef _O
#undef _C

	return 1;
}

/**
 * @name Address Object Creation/Deletion
 * @{
 */

/**
 * Allocate and initialize a new address object
 * @note Free the memory after usage using rtnl_addr_put() or rtnl_addr_free().
 * @return Newly allocated address object or NULL if an error occured.
 */
struct rtnl_addr *rtnl_addr_alloc(void)
{
	return (struct rtnl_addr *) nl_object_alloc_from_ops(&rtnl_addr_ops);
}

/**
 * Give back a reference on a address object.
 * @arg addr		Address object to be given back.
 *
 * Decrements the reference counter and frees the object if the
 * last reference has been released.
 */
void rtnl_addr_put(struct rtnl_addr *addr)
{
	nl_object_put((struct nl_object *) addr);
}

/**
 * Free an address object
 * @arg addr		Address object to be freed.
 *
 * @note Always use rtnl_addr_put() unless you're absolutely sure
 *       that no other user may have a reference on this object.
 */
void rtnl_addr_free(struct rtnl_addr *addr)
{
	nl_object_free((struct nl_object *) addr);
}

/** @} */

/**
 * @name Address Cache Management
 * @{
 */

/**
 * Allocate address cache and fill in all configured addresses
 * @arg handle		Netlink handle.
 *
 * Allocates a new address cache, initializes it properly and updates it
 * to include all addresses currently configured in the kernel.
 *
 * @note Free the memory after usage.
 * @return Newly allocated cache or NULL if an error occured.
 */
struct nl_cache *rtnl_addr_alloc_cache(struct nl_handle *handle)
{
	struct nl_cache *cache;
	
	cache = nl_cache_alloc_from_ops(&rtnl_addr_ops);
	if (!cache)
		return NULL;

	if (nl_cache_update(handle, cache) < 0) {
		nl_cache_free(cache);
		return NULL;
	}

	return cache;
}

/** @} */

static struct nl_msg *build_addr_msg(struct rtnl_addr *tmpl, int cmd, int flags)
{
	struct nl_msg *msg;
	struct ifaddrmsg am = {
		.ifa_family = tmpl->a_family,
		.ifa_index = tmpl->a_ifindex,
		.ifa_prefixlen = tmpl->a_prefixlen,
	};

	if (tmpl->a_mask & ADDR_ATTR_SCOPE)
		am.ifa_scope = tmpl->a_scope;
	else {
		/* compatibility hack */
		if (tmpl->a_family == AF_INET &&
		    tmpl->a_mask & ADDR_ATTR_LOCAL &&
		    *((char *) nl_addr_get_binary_addr(tmpl->a_local)) == 127)
			am.ifa_scope = RT_SCOPE_HOST;
		else
			am.ifa_scope = RT_SCOPE_UNIVERSE;
	}

	msg = nlmsg_build_simple(cmd, flags);
	if (!msg)
		goto nla_put_failure;

	if (nlmsg_append(msg, &am, sizeof(am), 1) < 0)
		goto nla_put_failure;

	if (tmpl->a_mask & ADDR_ATTR_LOCAL)
		NLA_PUT_ADDR(msg, IFA_LOCAL, tmpl->a_local);

	if (tmpl->a_mask & ADDR_ATTR_PEER)
		NLA_PUT_ADDR(msg, IFA_ADDRESS, tmpl->a_peer);
	else
		NLA_PUT_ADDR(msg, IFA_ADDRESS, tmpl->a_local);

	if (tmpl->a_mask & ADDR_ATTR_LABEL)
		NLA_PUT_STRING(msg, IFA_LABEL, tmpl->a_label);

	if (tmpl->a_mask & ADDR_ATTR_BROADCAST)
		NLA_PUT_ADDR(msg, IFA_BROADCAST, tmpl->a_bcast);

	if (tmpl->a_mask & ADDR_ATTR_ANYCAST)
		NLA_PUT_ADDR(msg, IFA_ANYCAST, tmpl->a_anycast);

	return msg;

nla_put_failure:
	nlmsg_free(msg);
	return NULL;
}

/**
 * @name Address Addition
 * @{
 */

/**
 * Build netlink request message to request addition of new address
 * @arg addr		Address object representing the new address.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a new netlink message requesting the addition of a new
 * address. The netlink message header isn't fully equipped with
 * all relevant fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * Minimal required attributes:
 *   - interface index (rtnl_addr_set_ifindex())
 *   - local address (rtnl_addr_set_local())
 *
 * The scope will default to universe except for loopback addresses in
 * which case a host scope is used if not specified otherwise.
 *
 * @note Free the memory after usage using nlmsg_free().
 * @return Newly allocated netlink message or NULL if an error occured.
 */
struct nl_msg *rtnl_addr_build_add_request(struct rtnl_addr *addr, int flags)
{
	int required = ADDR_ATTR_IFINDEX | ADDR_ATTR_FAMILY |
		       ADDR_ATTR_PREFIXLEN | ADDR_ATTR_LOCAL;

	if ((addr->a_mask & required) != required) {
		nl_error(EINVAL, "Missing mandatory attributes, required are: "
				 "ifindex, family, prefixlen, local address.");
		return NULL;
	}
	
	return build_addr_msg(addr, RTM_NEWADDR, NLM_F_CREATE | flags);
}

/**
 * Request addition of new address
 * @arg handle		Netlink handle.
 * @arg addr		Address object representing the new address.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a netlink message by calling rtnl_addr_build_add_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @see rtnl_addr_build_add_request()
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_addr_add(struct nl_handle *handle, struct rtnl_addr *addr, int flags)
{
	struct nl_msg *msg;
	int err;

	msg = rtnl_addr_build_add_request(addr, flags);
	if (!msg)
		return nl_get_errno();

	err = nl_send_auto_complete(handle, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name Address Deletion
 * @{
 */

/**
 * Build a netlink request message to request deletion of an address
 * @arg addr		Address object to be deleteted.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a new netlink message requesting a deletion of an address.
 * The netlink message header isn't fully equipped with all relevant
 * fields and must thus be sent out via nl_send_auto_complete()
 * or supplemented as needed.
 *
 * Minimal required attributes:
 *   - interface index (rtnl_addr_set_ifindex())
 *   - address family (rtnl_addr_set_family())
 *
 * Optional attributes:
 *   - local address (rtnl_addr_set_local())
 *   - label (rtnl_addr_set_label(), IPv4/DECnet only)
 *   - peer address (rtnl_addr_set_peer(), IPv4 only)
 *
 * @note Free the memory after usage using nlmsg_free().
 * @return Newly allocated netlink message or NULL if an error occured.
 */
struct nl_msg *rtnl_addr_build_delete_request(struct rtnl_addr *addr, int flags)
{
	int required = ADDR_ATTR_IFINDEX | ADDR_ATTR_FAMILY;

	if ((addr->a_mask & required) != required) {
		nl_error(EINVAL, "Missing mandatory attributes, required are: "
				 "ifindex, family");
		return NULL;
	}
	
	return build_addr_msg(addr, RTM_DELADDR, flags);
}

/**
 * Request deletion of an address
 * @arg handle		Netlink handle.
 * @arg addr		Address object to be deleted.
 * @arg flags		Additional netlink message flags.
 *
 * Builds a netlink message by calling rtnl_addr_build_delete_request(),
 * sends the request to the kernel and waits for the next ACK to be
 * received and thus blocks until the request has been fullfilled.
 *
 * @see rtnl_addr_build_delete_request();
 *
 * @return 0 on sucess or a negative error if an error occured.
 */
int rtnl_addr_delete(struct nl_handle *handle, struct rtnl_addr *addr,
		     int flags)
{
	struct nl_msg *msg;
	int err;

	msg = rtnl_addr_build_delete_request(addr, flags);
	if (!msg)
		return nl_get_errno();

	err = nl_send_auto_complete(handle, msg);
	nlmsg_free(msg);
	if (err < 0)
		return err;

	return nl_wait_for_ack(handle);
}

/** @} */

/**
 * @name Attribute Access
 * @{
 */

/**
 * Set label of address object
 * @arg addr		Address object to be modified.
 * @arg label		New address label.
 * 
 * @note The maximum size of an address label is IFNAMSIZ.
 */
void rtnl_addr_set_label(struct rtnl_addr *addr, const char *label)
{
	strncpy(addr->a_label, label, sizeof(addr->a_label) - 1);
	addr->a_mask |= ADDR_ATTR_LABEL;
}

/**
 * Get label of address object
 * @arg addr		Address object.
 * @return Address label or NULL if not set.
 */
char *rtnl_addr_get_label(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_LABEL)
		return addr->a_label;
	else
		return NULL;
}

/**
 * Set interface index of address object
 * @arg addr		Address object ot be modified.
 * @arg ifindex		New interface index this address is on.
 */
void rtnl_addr_set_ifindex(struct rtnl_addr *addr, int ifindex)
{
	addr->a_ifindex = ifindex;
	addr->a_mask |= ADDR_ATTR_IFINDEX;
}

/**
 * Get interface index of address object
 * @arg addr		Address object.
 * @return Interface index address is on or RTNL_LINK_NOT_FOUND if not set.
 */
int rtnl_addr_get_ifindex(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_IFINDEX)
		return addr->a_ifindex;
	else
		return RTNL_LINK_NOT_FOUND;
}

/**
 * Set address family of address object.
 * @arg addr		Address object to be modified.
 * @arg family		New address family
 *
 * @note The address family is set automatically if one of the addresses
 *       is set and the family hasn't been specified yet. Setting it manually
 *       can be used to enforce family validation while setting addresses.
 */
void rtnl_addr_set_family(struct rtnl_addr *addr, int family)
{
	addr->a_family = family;
	addr->a_mask |= ADDR_ATTR_FAMILY;
}

/**
 * Get address family of address object.
 * @arg addr		Address object.
 * @return Address family or AF_UNSPEC if not set.
 */
int rtnl_addr_get_family(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_FAMILY)
		return addr->a_family;
	else
		return AF_UNSPEC;
}

/**
 * Set prefix length of address object.
 * @arg addr		Address object to be modified.
 * @arg prefix		New prefix length.
 */
void rtnl_addr_set_prefixlen(struct rtnl_addr *addr, int prefix)
{
	addr->a_prefixlen = prefix;
	addr->a_mask |= ADDR_ATTR_PREFIXLEN;
}

/**
 * Get prefix length of address object.
 * @arg addr		Address object.
 * @return Prefix length or a negative number if not set.
 */
int rtnl_addr_get_prefixlen(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_PREFIXLEN)
		return addr->a_prefixlen;
	else
		return -1;
}

/**
 * Set scope of address object.
 * @arg addr		Address object to be modified.
 * @arg scope		New scope.
 */
void rtnl_addr_set_scope(struct rtnl_addr *addr, int scope)
{
	addr->a_scope = scope;
	addr->a_mask |= ADDR_ATTR_SCOPE;
}

/**
 * Get scope of address object.
 * @arg addr		Address object.
 * @return Scope or a negative number if not set.
 */
int rtnl_addr_get_scope(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_SCOPE)
		return addr->a_scope;
	else
		return -1;
}

/**
 * Set flags of address object.
 * @arg addr		Address object to be modified.
 * @arg flags		Additional flags to set.
 *
 * @note Existing flags that have been set will not be overwritten.
 */
void rtnl_addr_set_flags(struct rtnl_addr *addr, unsigned int flags)
{
	addr->a_flag_mask |= flags;
	addr->a_flags |= flags;
	addr->a_mask |= ADDR_ATTR_FLAGS;
}

/**
 * Unset flags of address object.
 * @arg addr		Address object to be modified.
 * @arg flags		Flags to unset.
 */
void rtnl_addr_unset_flags(struct rtnl_addr *addr, unsigned int flags)
{
	addr->a_flag_mask |= flags;
	addr->a_flags &= ~flags;
	addr->a_mask |= ADDR_ATTR_FLAGS;
}

/**
 * Get flags of address object.
 * @arg addr		Address object.
 * @return Flags in form of a bitmask.
 */
unsigned int rtnl_addr_get_flags(struct rtnl_addr *addr)
{
	return addr->a_flags;
}

static inline int __assign_addr(struct rtnl_addr *addr, struct nl_addr **pos,
			        struct nl_addr *new, int flag)
{
	if (addr->a_mask & ADDR_ATTR_FAMILY) {
		if (new->a_family != addr->a_family)
			return nl_error(EINVAL, "Address family mismatch");
	} else
		addr->a_family = new->a_family;

	if (*pos)
		nl_addr_put(*pos);

	*pos = nl_addr_get(new);
	addr->a_mask |= (flag | ADDR_ATTR_FAMILY);

	return 0;
}

/**
 * Set local address of address object.
 * @arg addr		Address object to be modified.
 * @arg local		New local address.
 *
 * Assigns the new local address to the specified address object. The
 * address is validated against the address family if set already via
 * either rtnl_addr_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of the
 * new address is elected to be the new requirement.
 *
 * @note The address may not contain a prefix length if the peer address
 *       has been specified already.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_local(struct rtnl_addr *addr, struct nl_addr *local)
{
	int err;

	err = __assign_addr(addr, &addr->a_local, local, ADDR_ATTR_LOCAL);
	if (err < 0)
		return err;

	if (!(addr->a_mask & ADDR_ATTR_PEER)) {
		addr->a_prefixlen = nl_addr_get_prefixlen(addr->a_local);
		addr->a_mask |= ADDR_ATTR_PREFIXLEN;
	}

	return 0;
}

/**
 * Get local address of address object.
 * @arg addr		Address object.
 * @return Local address or NULL if not set.
 */
struct nl_addr *rtnl_addr_get_local(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_LOCAL)
		return addr->a_local;
	else
		return NULL;
}

/**
 * Set peer address of address object.
 * @arg addr		Address object to be modified.
 * @arg peer		New peer address.
 *
 * Assigns the new peer address to the specified address object. The
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
	return __assign_addr(addr, &addr->a_peer, peer, ADDR_ATTR_PEER);

	addr->a_prefixlen = nl_addr_get_prefixlen(addr->a_peer);
	addr->a_mask |= ADDR_ATTR_PREFIXLEN;

	return 0;
}

/**
 * Get peer address of address object.
 * @arg addr		Adress object.
 * @return Peer address or NULL if not set.
 */
struct nl_addr *rtnl_addr_get_peer(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_PEER)
		return addr->a_peer;
	else
		return NULL;
}

/**
 * Set broadcast address of address object.
 * @arg addr		Address object to be modified.
 * @arg bcast		New broadcast address.
 *
 * Assigns the new broadcast address to the specified address object. The
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
	return __assign_addr(addr, &addr->a_bcast, bcast, ADDR_ATTR_BROADCAST);
}

/**
 * Get broadcast address of address object.
 * @arg addr		Address object.
 * @return Broadcast address or NULL if not set.
 */
struct nl_addr *rtnl_addr_get_broadcast(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_BROADCAST)
		return addr->a_bcast;
	else
		return NULL;
}

/**
 * Set anycast address of address object.
 * @arg addr		Address object to be modified.
 * @arg anycast		New anycast address.
 *
 * Assigns the new anycast address to the specified address object. The
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
	return __assign_addr(addr, &addr->a_anycast, anycast,
			     ADDR_ATTR_ANYCAST);
}

/**
 * Get anycast address of address object.
 * @arg addr		Address object.
 * @return Anycast address or NULL if not set.
 */
struct nl_addr *rtnl_addr_get_anycast(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_ANYCAST)
		return addr->a_anycast;
	else
		return NULL;
}

/**
 * Set multicast address of address object.
 * @arg addr		Address object to be modified.
 * @arg multicast	New multicast address.
 *
 * Assigns the new multicast address to the specified address object. The
 * address is validated against the address family if set already via
 * either rtnl_addr_set_family() or by setting one of the other addresses.
 * The assignment fails if the address families mismatch. In case the
 * address family has not been specified yet, the address family of this
 * new address is elected to be the requirement.
 * 
 * @return 0 on success or a negative error code.
 */
int rtnl_addr_set_multicast(struct rtnl_addr *addr, struct nl_addr *multicast)
{
	return __assign_addr(addr, &addr->a_multicast, multicast,
			     ADDR_ATTR_MULTICAST);
}

/**
 * Get multicast address of address object.
 * @arg addr		Address object.
 * @return Multicast address or NULL if not set.
 */
struct nl_addr *rtnl_addr_get_multicast(struct rtnl_addr *addr)
{
	if (addr->a_mask & ADDR_ATTR_MULTICAST)
		return addr->a_multicast;
	else
		return NULL;
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
 * Convert address flags to character string.
 * @arg flags		Address flags.
 * @arg buf		Destination buffer.
 * @arg size		Size of destination buffer.
 *
 * Converts address flags to a character string separated by commas and
 * stores the resulting character string in the specified destination buffer.
 *
 * @return Formatted flags as character string.
 */
char *rtnl_addr_flags2str(int flags, char *buf, size_t size)
{
	return __flags2str(flags, buf, size, addr_flags,
			   ARRAY_SIZE(addr_flags));
}

/**
 * Convert character string to address flags.
 * @arg name		Name of address flags.
 *
 * Converts the provided character string specifying any number of address
 * flags separated by commas to the corresponding numeric bitmask.
 *
 * @return Address flags in form of a bitmask.
 */
int rtnl_addr_str2flags(const char *name)
{
	return __str2flags(name, addr_flags, ARRAY_SIZE(addr_flags));
}

/** @} */

static struct nl_cache_ops rtnl_addr_ops = {
	.co_name		= "route/addr",
	.co_size		= sizeof(struct rtnl_addr),
	.co_hdrsize		= sizeof(struct ifaddrmsg),
	.co_msgtypes		= {
					{ RTM_NEWADDR, "new" },
					{ RTM_DELADDR, "delete" },
					{ RTM_GETADDR, "get" },
					{ -1, NULL },
				  },
	.co_protocol		= NETLINK_ROUTE,
	.co_request_update      = addr_request_update,
	.co_msg_parser          = addr_msg_parser,
	.co_free_data		= addr_free_data,
	.co_dump[NL_DUMP_BRIEF] = addr_dump_brief,
	.co_dump[NL_DUMP_FULL]  = addr_dump_full,
	.co_dump[NL_DUMP_STATS] = addr_dump_stats,
	.co_dump[NL_DUMP_XML]	= addr_dump_xml,
	.co_dump[NL_DUMP_ENV]	= addr_dump_env,
	.co_filter              = addr_filter,
};

static void __init addr_init(void)
{
	nl_cache_mngt_register(&rtnl_addr_ops);
}

static void __exit neigh_exit(void)
{
	nl_cache_mngt_unregister(&rtnl_addr_ops);
}

/** @} */
