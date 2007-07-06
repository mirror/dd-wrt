/*
 * netlink/route/neighbour.h	libnl neighbour module
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

#ifndef NETLINK_NEIGHBOUR_H_
#define NETLINK_NEIGHBOUR_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>
#include <netlink/route/rtattr.h>

/**
 * @name Available Attributes Flags
 * @ingroup neigh
 * @anchor neigh_avail_attrs
 * Flags used in rtnl_neigh::n_mask to indicate which attributes are
 * present in a neighbour.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_neigh_set_* function or if the neighbour was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if neighbour has a interface index and link layer address attribute set
 * @code
 * if (neigh->n_mask & (NEIGH_HAS_IFINDEX | NEIGH_HAS_LLADDR))
 *         // action goes here
 * @endcode
 * @{
 */
#define NEIGH_HAS_FLAGS        0x01	/**< Has flags */
#define NEIGH_HAS_STATE        0x02	/**< Has state */
#define NEIGH_HAS_LLADDR       0x04	/**< Has link layer adddress */
#define NEIGH_HAS_DST          0x08	/**< Has destination addreess */
#define NEIGH_HAS_CACHEINFO    0x10	/**< Has caching information */
#define NEIGH_HAS_IFINDEX      0x20	/**< Has interface index */
#define NEIGH_HAS_FAMILY       0x40	/**< Has destination address family */
#define NEIGH_HAS_TYPE         0x80	/**< Has rtn type */

/** @} */

/**
 * Neighbour caching information
 * @ingroup neigh
 */
struct rtnl_ncacheinfo
{
	uint32_t nci_confirmed;	/**< Time since neighbour validty was last confirmed */
	uint32_t nci_used;	/**< Time since neighbour entry was last ued */
	uint32_t nci_updated;	/**< Time since last update */
	uint32_t nci_refcnt;	/**< Reference counter */
};

/**
 * Neighbour
 * @ingroup neigh
 */
struct rtnl_neigh
{
	/** Common header required by cache */
	NLHDR_COMMON
	uint32_t	n_family;	/**< Address family of destination address */
	uint32_t	n_ifindex;	/**< Interface index of device this neighbour is on */
	uint16_t	n_state;	/**< Neighbour state */
	uint8_t		n_flags;	/**< Neighbour flags */
	uint8_t		n_type;		/**< Address type of neighbour */
	struct nl_addr	n_lladdr;	/**< Link layer address */
	struct nl_addr	n_dst;		/**< Destination address */
	uint32_t	n_probes;	/**< Number of probes */

	/**
	 * Caching information
	 */
	struct rtnl_ncacheinfo n_cacheinfo;

	/**
	 * Mask of available attributes
	 */
	uint32_t                n_mask;

	uint32_t                n_state_mask;
	uint32_t                n_flag_mask;
};

extern struct nl_cache_ops rtnl_neigh_ops;

/**
 * Initialize a neighbour cache
 * @ingroup neigh
 * @code
 * struct nl_cache cache = RTNL_INIT_NEIGH_CACHE();
 * @endcode
 */
#define RTNL_INIT_NEIGH_CACHE() {              \
    .c_type = RTNL_NEIGH,                      \
    .c_type_size = sizeof(struct rtnl_neigh),  \
    .c_ops = &rtnl_neigh_ops,                  \
}

/**
 * Initialize a neighbour
 * @ingroup neigh
 * @code
 * struct rtnl_neigh n = RTNL_INIT_NEIGH();
 * @endcode
 */
#define RTNL_INIT_NEIGH() {                     \
    .ce_type = RTNL_NEIGH,                      \
    .ce_size = sizeof(struct rtnl_neigh),       \
}

extern struct nl_cache * rtnl_neigh_build_cache(struct nl_handle *);
extern struct rtnl_neigh * rtnl_neigh_get(struct nl_cache *, int,
					  struct nl_addr *);

extern void rtnl_neigh_dump(struct rtnl_neigh *, FILE *,
			    struct nl_dump_params *);

extern char *	rtnl_neigh_state2str(int);
extern char *	rtnl_neigh_state2str_r(int, char *, size_t);
extern int	rtnl_neigh_str2state(const char *);

extern char *	rtnl_neigh_flags2str(int);
extern char *	rtnl_neigh_flags2str_r(int, char *, size_t);
extern int	rtnl_neigh_str2flag(const char *);

extern int  rtnl_neigh_add(struct nl_handle *, struct rtnl_neigh *);
extern struct nl_msg * rtnl_neigh_build_add_request(struct rtnl_neigh *);
extern int  rtnl_neigh_change(struct nl_handle *, struct rtnl_neigh *,
                              struct rtnl_neigh *);
extern struct nl_msg * rtnl_neigh_build_change_request(struct rtnl_neigh *,
                                                       struct rtnl_neigh *);
extern int  rtnl_neigh_delete(struct nl_handle *, struct rtnl_neigh *);
extern struct nl_msg * rtnl_neigh_build_delete_request(struct rtnl_neigh *);

extern void rtnl_neigh_set_state(struct rtnl_neigh *n, int state);
extern void rtnl_neigh_set_state_name(struct rtnl_neigh *, const char *);
extern void rtnl_neigh_unset_state(struct rtnl_neigh *n, int state);
extern void rtnl_neigh_unset_state_name(struct rtnl_neigh *, const char *);
extern void rtnl_neigh_set_flag(struct rtnl_neigh *n, int flag);
extern void rtnl_neigh_unset_flag(struct rtnl_neigh *n, int flag);
extern void rtnl_neigh_set_ifindex(struct rtnl_neigh *n, int ifindex);
extern int  rtnl_neigh_set_ifindex_name(struct rtnl_neigh *n,
                                        struct nl_cache *c, const char *dev);
extern void rtnl_neigh_set_lladdr(struct rtnl_neigh *n, struct nl_addr *addr);
extern int  rtnl_neigh_set_lladdr_str(struct rtnl_neigh *n, const char *addr);
extern int  rtnl_neigh_set_dst(struct rtnl_neigh *n, struct nl_addr *addr);
extern int  rtnl_neigh_set_dst_str(struct rtnl_neigh *n, const char *addr);
extern int  rtnl_neigh_set_type(struct rtnl_neigh *n, const char *type);

#endif
