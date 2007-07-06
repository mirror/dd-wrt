/*
 * netlink/route/rule.h		libnl rule module
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

#ifndef NETLINK_RULE_H_
#define NETLINK_RULE_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>

/**
 * @name Available Attributes Flags
 * @ingroup rule
 * @anchor rule_avail_attrs
 * Flags used in rtnl_rule::r_mask to indicate which attributes are
 * present in a rule.
 *
 * The corresponding flag is automatically set if a attribute is set
 * via a \c rtnl_rule_set_* function or if the rule was created by
 * the internal message parser, e.g. if it comes from a cache.
 *
 * @par Example: Check if rule has a broadcast address and weight attribute set
 * @code
 * if (rule->r_mask & (LINK_HAS_BRD | LINK_HAS_QDISC))
 *         // action goes here
 * @endcode
 * @{
 */
#define RULE_HAS_FAMILY  0x0001		/**< Has family */
#define RULE_HAS_PRIO    0x0002		/**< Has priority */
#define RULE_HAS_FWMARK  0x0004		/**< Has firewall mark */
#define RULE_HAS_IIF     0x0008		/**< Has incomming interface */
#define RULE_HAS_REALMS  0x0010		/**< Has realms */
#define RULE_HAS_SRC     0x0020		/**< Has source address */
#define RULE_HAS_DST     0x0040		/**< Has destination address */
#define RULE_HAS_DSFIELD 0x0080		/**< Has ds field */
#define RULE_HAS_TABLE   0x0100		/**< Has table */
#define RULE_HAS_TYPE    0x0200		/**< Has type */
#define RULE_HAS_SRC_LEN 0x0400		/**< Has source address length */
#define RULE_HAS_DST_LEN 0x0800		/**< Has destination address length */
#define RULE_HAS_SCOPE   0x1000		/**< Has scope */
#define RULE_HAS_PROTO   0x2000		/**< Has protocol */
/** @} */

/**
 * Routing Rule
 * @ingroup rule
 */
struct rtnl_rule
{
	/** Common header required by cache */
	NLHDR_COMMON

	uint64_t	r_fwmark;
	uint32_t	r_prio;
	uint32_t	r_realms;
	uint32_t	r_table;
	uint8_t		r_dsfield;
	uint8_t		r_type;
	uint8_t		r_proto;
	uint8_t		r_scope;
	uint8_t		r_family;
	uint8_t		r_src_len;
	uint8_t		r_dst_len;
	char		r_iif[IFNAMSIZ+1];

	struct nl_addr  r_src;
	struct nl_addr  r_dst;

	/**
	 * Available attribtues (RULE_HAS_*)
	 * @see \ref rule_avail_attrs "Available Attributes Flags"
	 */
	uint32_t	r_mask;	
};

extern struct nl_cache_ops rtnl_rule_ops;

/**
 * Initialize a rule cache structure.
 * @ingroup rule
 *
 * @code
 * struct nl_cache cache = RTNL_INIT_RULE_CACHE();
 * @endcode
 */
#define RTNL_INIT_RULE_CACHE() {               \
    .c_type = RTNL_RULE,                       \
    .c_type_size = sizeof(struct rtnl_rule),   \
    .c_ops = &rtnl_rule_ops,                   \
}

/**
 * Initialize a rule strcuture.
 * @ingroup rule
 *
 * @code
 * struct rtnl_rule r = RTNL_INIT_RULE();
 * @endcode
 */
#define RTNL_INIT_RULE() {                     \
    .ce_type = RTNL_RULE,                      \
    .ce_size = sizeof(struct rtnl_rule),       \
}

/**
 * Mask specying the size of each realm part
 * @ingroup rule
 */
#define RTNL_REALM_MASK (0xFFFF)

/**
 * Extract from realm from a realms field
 * @ingroup rule
 */
#define RTNL_REALM_FROM(realm) ((realm) >> 16)

/**
 * Extract to realm from a realms field
 * @ingroup rule
 */
#define RTNL_REALM_TO(realm) ((realm) & RTNL_REALM_MASK)

/**
 * Build a realms field
 * @ingroup rule
 */
#define RTNL_MAKE_REALM(from, to) \
	((RTNL_REALM_TO(from) << 16) & RTNL_REALM_TO(to))

extern struct nl_cache * rtnl_rule_build_cache(struct nl_handle *);
extern void rtnl_rule_dump(struct rtnl_rule *, FILE *, struct nl_dump_params *);

extern struct nl_msg * rtnl_rule_build_add_request(struct rtnl_rule *);
extern int rtnl_rule_add(struct nl_handle *, struct rtnl_rule *);
extern struct nl_msg * rtnl_rule_build_delete_request(struct rtnl_rule *);
extern int rtnl_rule_delete(struct nl_handle *, struct rtnl_rule *);

extern char * rtnl_realms2str_r(uint32_t, char *, size_t);
extern char * rtnl_realms2str(uint32_t);

extern void rtnl_rule_set_family(struct rtnl_rule *, int);
extern void rtnl_rule_set_prio(struct rtnl_rule *, int);
extern void rtnl_rule_set_fwmark(struct rtnl_rule *, uint64_t);
extern void rtnl_rule_set_table(struct rtnl_rule *, int);
extern void rtnl_rule_set_dsfield(struct rtnl_rule *, int);
extern void rtnl_rule_set_src_len(struct rtnl_rule *, int);
extern void rtnl_rule_set_dst_len(struct rtnl_rule *, int);
extern int  rtnl_rule_set_src(struct rtnl_rule *, struct nl_addr *);
extern int  rtnl_rule_set_src_str(struct rtnl_rule *, const char *);
extern int  rtnl_rule_set_dst(struct rtnl_rule *, struct nl_addr *);
extern int  rtnl_rule_set_dst_str(struct rtnl_rule *, const char *);

#endif
