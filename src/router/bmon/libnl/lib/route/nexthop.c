/*
 * lib/route/nexthop.c	Routing Nexthop
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup route
 * @defgroup nexthop Nexthop
 * @brief
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/route.h>

/** @cond SKIP */
#define NEXTHOP_HAS_FLAGS   0x000001
#define NEXTHOP_HAS_WEIGHT  0x000002
#define NEXTHOP_HAS_IFINDEX 0x000004
#define NEXTHOP_HAS_GATEWAY 0x000008
/** @endcond */

/**
 * @name Nexthop Allocation/Freeage
 * @{
 */

/**
 * Allocate a routing nexthop.
 * @return Newly allocated routing nexthop object.
 */
struct rtnl_nexthop *rtnl_route_nh_alloc(void)
{
	struct rtnl_nexthop *nh;

	nh = calloc(1, sizeof(*nh));
	if (!nh) {
		nl_errno(ENOMEM);
		return NULL;
	}

	nl_init_list_head(&nh->rtnh_list);

	return nh;
}

/**
 * Free a routing nexthop.
 * @arg nh		Routing nexthop to be freed.
 */
void rtnl_route_nh_free(struct rtnl_nexthop *nh)
{
	nl_addr_put(nh->rtnh_gateway);
	free(nh);
}

/** @} */
/**
 * @name Attribute: Weight
 */

/**
 * Set weight of routing nexthop.
 * @arg nh		Routing nexthop.
 * @arg weight		New weight value.
 */
void rtnl_route_nh_set_weight(struct rtnl_nexthop *nh, int weight)
{
	nh->rtnh_weight = weight;
	nh->rtnh_mask |= NEXTHOP_HAS_WEIGHT;
}

/**
 * Get weight of routing nexthop.
 * @arg nh		Routing nexthop.
 * @return Weight value or 0 if not available.
 */
int rtnl_route_nh_get_weight(struct rtnl_nexthop *nh)
{
	if (nh->rtnh_mask & NEXTHOP_HAS_WEIGHT)
		return nh->rtnh_weight;
	else
		return 0;
}

/** @} */
/**
 * @name Attribute: Interface Index
 * @{
 */

/**
 * Set interface index for outgoing interface of routing nexthop.
 * @arg nh		Routing nexthop.
 * @arg ifindex		New interface index.
 */
void rtnl_route_nh_set_ifindex(struct rtnl_nexthop *nh, int ifindex)
{
	nh->rtnh_ifindex = ifindex;
	nh->rtnh_mask |= NEXTHOP_HAS_IFINDEX;
}

/**
 * Get interface index of outgoing index of routing nexthop.
 * @arg nh		Routing nexthop.
 * @return Interface index or -1 if not available.
 */
int rtnl_route_nh_get_ifindex(struct rtnl_nexthop *nh)
{
	if (nh->rtnh_mask & NEXTHOP_HAS_IFINDEX)
		return nh->rtnh_ifindex;
	else
		return -1;
}	

/** @} */
/**
 * @name Attribute: Gateway Address
 * @{
 */

/**
 * Set gateway address of routing nexthop.
 * @arg nh		Routing nexthop.
 * @arg addr		New gateway address.
 *
 * An eventual existing gateway address will be freed and a
 * reference is acquried of the new address.
 */
void rtnl_route_nh_set_gateway(struct rtnl_nexthop *nh, struct nl_addr *addr)
{
	struct nl_addr *old = nh->rtnh_gateway;

	nh->rtnh_gateway = nl_addr_get(addr);
	if (old)
		nl_addr_put(old);

	nh->rtnh_mask |= NEXTHOP_HAS_GATEWAY;
}

/**
 * Get gateway address of routing nexthop.
 * @arg nh		Routing nexthop.
 * @return Gateway address or NULL if not available.
 */
struct nl_addr *rtnl_route_nh_get_gateway(struct rtnl_nexthop *nh)
{
	if (nh->rtnh_mask & NEXTHOP_HAS_GATEWAY)
		return nh->rtnh_gateway;
	else
		return NULL;
}

/** @} */
/**
 * @name Attribute: Flags
 * @{
 */

/**
 * Set flags of routing nexthop.
 * @arg nh		Routing nexthop.
 * @arg flags		Flags to be set.
 */
void rtnl_route_nh_set_flags(struct rtnl_nexthop *nh, unsigned int flags)
{
	nh->rtnh_flag_mask |= flags;
	nh->rtnh_flags |= flags;
	nh->rtnh_mask |= NEXTHOP_HAS_FLAGS;
}

/**
 * Unset flags of routing nexthop.
 * @arg nh		Routing nexthop.
 * @arg flags		Flags to be unset.
 */
void rtnl_route_nh_unset_flags(struct rtnl_nexthop *nh, unsigned int flags)
{
	nh->rtnh_flag_mask |= flags;
	nh->rtnh_flags &= ~flags;
	nh->rtnh_mask |= NEXTHOP_HAS_FLAGS;
}

/**
 * Get flags of routing nexthop.
 * @arg nh		Routing nexthop.
 * @return Flags or 0 if not available.
 */
unsigned int rtnl_route_nh_get_flags(struct rtnl_nexthop *nh)
{
	if (nh->rtnh_mask & NEXTHOP_HAS_FLAGS)
		return nh->rtnh_flags;
	else
		return 0;
}

/** @} */
/** @} */
