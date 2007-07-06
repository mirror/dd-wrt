/*
 * netlink/cache.h       Caching Module
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

#ifndef NETLINK_CACHE_H_
#define NETLINK_CACHE_H_

#include <netlink/netlink.h>
#include <netlink/msg.h>

/**
 * Common part for all types being put into a cache.
 * @ingroup cache
 *
 * Every structure that wants to be put into a cache must include
 * this at the very beginning of the structure. It is used internally
 * to maintain the information needed to manage the object.
 *
 * @attention A modification of this data may lead to a corruption
 *            of the cache.
 */
#define NLHDR_COMMON		\
	int	ce_index;	\
	int	ce_size;	\
	int	ce_type;	\
	int	ce_refcnt;

/**
 * Abstract structure to which every type in a cache can be
 * casted to.
 * @ingroup cache
 */
struct nl_common
{
	NLHDR_COMMON
};

/**
 * Cache types
 * @ingroup cache
 */
enum {
	RTNL_NEIGH,		/**< Neighbour */
	RTNL_LINK,		/**< Link */
	RTNL_CLASS,		/**< Traffic control class */
	RTNL_QDISC,		/**< Traffic control Qdisc */
	RTNL_FILTER,		/**< Traffic control filter */
	RTNL_ROUTE,		/**< Route */
	RTNL_ADDR,		/**< Address */
	RTNL_RULE,		/**< Rule */
	__RTNL_MAX,
};
#define RTNL_MAX (__RTNL_MAX - 1)

struct nl_cache_ops;

/**
 * Cache
 * @ingroup cache
 */
struct nl_cache
{
	/** cache type */
	int                     c_type;
	/** cached data */
	void *                  c_cache;
	/** size of cache (# elements) */
	size_t                  c_size;
	/** size of a cache elemtn */
	size_t                  c_type_size;
	/** index pointed to last element */
	int                     c_index;
	/** type specific argument 1 */
	int                     c_iarg1;
	/** type specific argument 2 */
	int                     c_iarg2;
	/** cache operations */
	struct nl_cache_ops *   c_ops;
};

/**
 * Returns cache element at given index
 * @ingroup cache
 * @arg C	cache
 * @arg I	index
 */
#define NL_CACHE_ELEMENT_AT(C, I) \
	((struct nl_common *) (((char *) (C)->c_cache) + (I * (C)->c_type_size)))

struct nl_parser_param
{
	int             (*pp_cb)(struct nl_common *, struct nl_parser_param *);
	void *            pp_arg;
};

/**
 * Dumping types
 * @ingroup cache
 */
enum nl_dump_type {
	NL_DUMP_BRIEF,		/**< Dump object in a brief one-liner */
	NL_DUMP_FULL,		/**< Dump all attributes but no statistics */
	NL_DUMP_STATS,		/**< Dump all attributes including statistics */
	NL_DUMP_XML,		/**< Dump all attribtes in XML format */
	NL_DUMP_EVENTS,		/**< Dump event */
	__NL_DUMP_MAX,
};
#define NL_DUMP_MAX (__NL_DUMP_MAX - 1)

/**
 * Dumping parameters
 * @ingroup cache
 */
struct nl_dump_params
{
	/**
	 * Specifies the type of dump that is requested.
	 */
	enum nl_dump_type	dp_type;

	/**
	 * Specifies the number of whitespaces to be put in front
	 * of every new line (indentation).
	 */
	int			dp_prefix;

	/**
	 * Causes the cache index to be printed for each element.
	 */
	int			dp_print_index;

	/**
	 * A callback invoked for every new line, can be used to
	 * customize the indentation.
	 *
	 * Passed arguments are:
	 *  - dumping parameters
	 *  - line number starting from 0
	 */
	void			(*dp_nl_cb)(struct nl_dump_params *, int);
};

/**
 * Type specific cache operations
 * @ingroup cache
 */
struct nl_cache_ops
{
	/**
	 * Called whenever an update of the cache is required. Must send
	 * a request message to the kernel requesting a complete dump.
	 */
	int   (*co_request_update)(struct nl_cache *, struct nl_handle *);

	/**
	 * Called whenever a object in the cache gets destroyed, must
	 * free the type specific memory allocations
	 */
	void  (*co_free_data)(struct nl_common *);

	/**
	 * Called whenever a message was received that needs to be parsed.
	 * Must parse the message and call the paser callback function
	 * (nl_parser_param) provided via the argument.
	 */
	int   (*co_msg_parser)(struct sockaddr_nl *, struct nlmsghdr *, void *);

	/**
	 * Called whenever a dump of a cache object is requested. Must
	 * dump the specified object to the specified file descriptor
	 */
	int   (*co_dump[NL_DUMP_MAX+1])(struct nl_cache *, struct nl_common *,
					FILE *, struct nl_dump_params *);

	/**
	 * Must compare the two specified objects and return a non-zero
	 * value if they match.
	 */
	int   (*co_filter)(struct nl_common *, struct nl_common *);
};

extern int  nl_cache_update(struct nl_handle *, struct nl_cache *);
extern int  nl_cache_enlarge(struct nl_cache *, size_t factor);
extern int  nl_cache_add(struct nl_cache *, struct nl_common *);
extern int  nl_cache_parse(struct nl_cache *c, struct nl_msg *msg);
extern void nl_cache_destroy(struct nl_cache *);
extern void nl_cache_destroy_and_free(struct nl_cache *);
extern int  nl_cache_delete_element(struct nl_cache *, struct nl_common *);
extern void nl_cache_foreach(struct nl_cache *,
                             void (*cb)(struct nl_common *, void *),
                             void *);
extern void nl_cache_foreach_filter(struct nl_cache *, struct nl_common *,
                                    void (*cb)(struct nl_common *, void *),
                                    void *);

extern void nl_cache_dump(struct nl_cache *, FILE *, struct nl_dump_params *);
extern void nl_cache_dump_filter(struct nl_cache *, FILE *,
				 struct nl_dump_params *, struct nl_common *);

extern struct nl_common * nl_cache_get(struct nl_cache *, int index);
extern int nl_cache_filter(struct nl_cache *, struct nl_common *,
			   struct nl_common *);

extern void nl_cache_provide(struct nl_cache *);
extern struct nl_cache * nl_cache_lookup(int);

extern inline int nl_cache_is_empty(struct nl_cache *cache);

#endif
