/*
 * addr.h               libnl address module
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
 *
 * ======================================================================
 *
 * Retrieving a local cache:
 *   struct nl_cache addr_cache = RTNL_INIT_ADDR_CACHE();
 *   nl_cache_update(&nl_handle, &addr_cache);
 *
 * Dumping all addrs:
 *   nl_cache_dump(NL_DUMP_BRIEF, &addr_cache, stdout);
 * ======================================================================
 */

#ifndef NETADDR_ADDR_H_
#define NETADDR_ADDR_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>

/**
 * @name Available Attributes Flags
 * @ingroup rtaddr
 * @anchor addr_avail_attrs
 * Flags used in rtnl_addr::a_mask to indicate which attributes are
 * present in a address.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_addr_set_* function or if the link was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if address has the label and scope attribute set
 * @code
 * if (addr->a_mask & (ADDR_HAS_LOCAL | ADDR_HAS_SCOPE))
 *         // action goes here
 * @endcode
 * @{
 */

#define ADDR_HAS_FAMILY		0x0001	/**< Has family */
#define ADDR_HAS_PREFIXLEN	0x0002	/**< Has prefix length */
#define ADDR_HAS_FLAGS		0x0004	/**< Has flags */
#define ADDR_HAS_SCOPE		0x0008	/**< Has scope */
#define ADDR_HAS_IFINDEX	0x0010	/**< Has interface index */
#define ADDR_HAS_LABEL		0x0020	/**< Has label */
#define ADDR_HAS_CACHEINFO	0x0040	/**< Has cache information */
#define ADDR_HAS_PEER		0x0080	/**< Has peer address */
#define ADDR_HAS_LOCAL		0x0100	/**< Has local address */
#define ADDR_HAS_BROADCAST	0x0200	/**< Has broadcast address */
#define ADDR_HAS_ANYCAST	0x0400	/**< Has anycast address */

/** @} */

/**
 * Address caching information
 * @ingroup rtaddr
 */
struct rtnl_addr_cacheinfo
{
	/**
	 * Preferred lifetime in seconds
	 */
	uint32_t aci_prefered;

	/**
	 * Valid lifetime in seconds
	 */
	uint32_t aci_valid;

	/**
	 * Timestamp of creation in 1/100s seince boottime
	 */
	uint32_t aci_cstamp;

	/**
	 * Timestamp of last update in 1/100s since boottime
	 */
	uint32_t aci_tstamp;
};

/**
 * Address
 * @ingroup rtaddr
 */
struct rtnl_addr
{
	/** Commong header required by cache */
	NLHDR_COMMON

	uint8_t		a_family;	/**< Address family */
	uint8_t		a_prefixlen;	/**< Prefix length */
	uint8_t		a_flags;	/**< Address flags */
	uint8_t		a_scope;	/**< Scope of address */
	uint32_t	a_ifindex;	/**< Device the address if on */

	struct nl_addr	a_peer;		/**< Peer address */
	struct nl_addr	a_local;	/**< Local address */
	struct nl_addr	a_bcast;	/**< Broadcast address */
	struct nl_addr	a_anycast;	/**< Anycast address */

	/** Caching information */
	struct rtnl_addr_cacheinfo a_cacheinfo;
	
	/** Address label (e.g. eth0:1) */
	char a_label[IFNAMSIZ];
	
	/** Available attributes mask */
	uint32_t a_mask;
	uint32_t a_flag_mask;
};

extern struct nl_cache_ops rtnl_addr_ops;

/**
 * Initialize an address cache structure.
 * @ingroup rtaddr
 *
 * @code
 * struct nl_cache cache = RTNL_INIT_ADDR_CACHE();
 * @endcode
 */
#define RTNL_INIT_ADDR_CACHE() {               \
    .c_type = RTNL_ADDR,                       \
    .c_type_size = sizeof(struct rtnl_addr),   \
    .c_ops = &rtnl_addr_ops,                   \
}

/**
 * Initialize an address structure.
 * @ingroup rtaddr
 *
 * @code
 * struct rtnl_addr l = RTNL_INIT_ADDR();
 * @endcode
 */
#define RTNL_INIT_ADDR() {                     \
    .ce_type = RTNL_ADDR,                      \
    .ce_size = sizeof(struct rtnl_addr),       \
}

extern struct nl_msg * rtnl_addr_build_add_request(struct rtnl_addr *);
extern int rtnl_addr_add(struct nl_handle *, struct rtnl_addr *);

extern struct nl_msg * rtnl_addr_build_delete_request(struct rtnl_addr *);
extern int rtnl_addr_delete(struct nl_handle *, struct rtnl_addr *);

extern struct nl_cache * rtnl_addr_build_cache(struct nl_handle *);
extern void rtnl_addr_dump(struct rtnl_addr *, FILE *, struct nl_dump_params *);

extern void rtnl_addr_set_family(struct rtnl_addr *, int);
extern void rtnl_addr_set_label(struct rtnl_addr *, const char *);
extern void rtnl_addr_set_ifindex(struct rtnl_addr *, int);
extern int  rtnl_addr_set_ifindex_name(struct rtnl_addr *, struct nl_cache *,
				      const char *);
extern void rtnl_addr_set_prefix(struct rtnl_addr *, int);
extern void rtnl_addr_set_scope(struct rtnl_addr *, int);
extern int  rtnl_addr_set_scope_str(struct rtnl_addr *, char *);
extern void rtnl_addr_set_flags(struct rtnl_addr *, int);
extern void rtnl_addr_unset_flags(struct rtnl_addr *, int);

extern int rtnl_addr_set_local(struct rtnl_addr *, struct nl_addr *);
extern int rtnl_addr_set_local_str(struct rtnl_addr *, const char *);

extern int rtnl_addr_set_peer(struct rtnl_addr *, struct nl_addr *);
extern int rtnl_addr_set_peer_str(struct rtnl_addr *, const char *);

extern int rtnl_addr_set_broadcast(struct rtnl_addr *, struct nl_addr *);
extern int rtnl_addr_set_broadcast_str(struct rtnl_addr *, const char *);

extern int rtnl_addr_set_anycast(struct rtnl_addr *, struct nl_addr *);
extern int rtnl_addr_set_anycast_addr(struct rtnl_addr *, const char *);

extern char * rtnl_addr_flags2str(int);
extern char * rtnl_addr_flags2str_r(int, char *, size_t);
extern int    rtnl_addr_str2flags(const char *);

#endif
