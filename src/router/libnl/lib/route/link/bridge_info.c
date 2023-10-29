/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2022 MaxLinear, Inc.
 */

/**
 * @ingroup link
 * @defgroup bridge Bridging
 *
 * @details
 * @{
 */

#include "nl-default.h"

#include <netlink/route/link/bridge_info.h>

#include "nl-route.h"
#include "link-api.h"

#define BRIDGE_ATTR_VLAN_FILTERING (1 << 0)
#define BRIDGE_ATTR_VLAN_PROTOCOL (1 << 1)
#define BRIDGE_ATTR_VLAN_STATS_ENABLED (1 << 2)

struct bridge_info {
	uint32_t ce_mask; /* to support attr macros */
	uint16_t b_vlan_protocol;
	uint8_t b_vlan_filtering;
	uint8_t b_vlan_stats_enabled;
};

static const struct nla_policy bi_attrs_policy[IFLA_BR_MAX + 1] = {
	[IFLA_BR_VLAN_FILTERING] = { .type = NLA_U8 },
	[IFLA_BR_VLAN_PROTOCOL] = { .type = NLA_U16 },
	[IFLA_BR_VLAN_STATS_ENABLED] = { .type = NLA_U8 },
};

static inline struct bridge_info *bridge_info(struct rtnl_link *link)
{
	return link->l_info;
}

static int bridge_info_alloc(struct rtnl_link *link)
{
	struct bridge_info *bi;

	if (link->l_info)
		memset(link->l_info, 0, sizeof(*bi));
	else {
		bi = calloc(1, sizeof(*bi));
		if (!bi)
			return -NLE_NOMEM;

		link->l_info = bi;
	}

	return 0;
}

static int bridge_info_parse(struct rtnl_link *link, struct nlattr *data,
			     struct nlattr *xstats)
{
	struct nlattr *tb[IFLA_BR_MAX + 1];
	struct bridge_info *bi;
	int err;

	NL_DBG(3, "Parsing Bridge link info\n");

	if ((err = nla_parse_nested(tb, IFLA_BR_MAX, data, bi_attrs_policy)) <
	    0)
		return err;

	if ((err = bridge_info_alloc(link)) < 0)
		return err;

	bi = link->l_info;

	if (tb[IFLA_BR_VLAN_FILTERING]) {
		bi->b_vlan_filtering = nla_get_u8(tb[IFLA_BR_VLAN_FILTERING]);
		bi->ce_mask |= BRIDGE_ATTR_VLAN_FILTERING;
	}

	if (tb[IFLA_BR_VLAN_PROTOCOL]) {
		bi->b_vlan_protocol =
			ntohs(nla_get_u16(tb[IFLA_BR_VLAN_PROTOCOL]));
		bi->ce_mask |= BRIDGE_ATTR_VLAN_PROTOCOL;
	}

	if (tb[IFLA_BR_VLAN_STATS_ENABLED]) {
		bi->b_vlan_stats_enabled =
			nla_get_u8(tb[IFLA_BR_VLAN_STATS_ENABLED]);
		bi->ce_mask |= BRIDGE_ATTR_VLAN_STATS_ENABLED;
	}

	return 0;
}

static int bridge_info_put_attrs(struct nl_msg *msg, struct rtnl_link *link)
{
	struct bridge_info *bi = link->l_info;
	struct nlattr *data;

	data = nla_nest_start(msg, IFLA_INFO_DATA);
	if (!data)
		return -NLE_MSGSIZE;

	if (bi->ce_mask & BRIDGE_ATTR_VLAN_FILTERING)
		NLA_PUT_U8(msg, IFLA_BR_VLAN_FILTERING, bi->b_vlan_filtering);

	if (bi->ce_mask & BRIDGE_ATTR_VLAN_PROTOCOL)
		NLA_PUT_U16(msg, IFLA_BR_VLAN_PROTOCOL,
			    htons(bi->b_vlan_protocol));

	if (bi->ce_mask & BRIDGE_ATTR_VLAN_STATS_ENABLED)
		NLA_PUT_U8(msg, IFLA_BR_VLAN_STATS_ENABLED,
			   bi->b_vlan_stats_enabled);

	nla_nest_end(msg, data);
	return 0;

nla_put_failure:
	nla_nest_cancel(msg, data);
	return -NLE_MSGSIZE;
}

static void bridge_info_free(struct rtnl_link *link)
{
	_nl_clear_free(&link->l_info);
}

static struct rtnl_link_info_ops bridge_info_ops = {
	.io_name = "bridge",
	.io_alloc = bridge_info_alloc,
	.io_parse = bridge_info_parse,
	.io_put_attrs = bridge_info_put_attrs,
	.io_free = bridge_info_free,
};

#define IS_BRIDGE_INFO_ASSERT(link)                                                      \
	do {                                                                             \
		if ((link)->l_info_ops != &bridge_info_ops) {                            \
			APPBUG("Link is not a bridge link. Set type \"bridge\" first."); \
		}                                                                        \
	} while (0)

/**
 * Set VLAN filtering flag
 * @arg link		Link object of type bridge
 * @arg vlan_filtering	VLAN_filtering boolean flag to set.
 *
 * @see rtnl_link_bridge_get_vlan_filtering()
 *
 * @return void
 */
void rtnl_link_bridge_set_vlan_filtering(struct rtnl_link *link,
					 uint8_t vlan_filtering)
{
	struct bridge_info *bi = bridge_info(link);

	IS_BRIDGE_INFO_ASSERT(link);

	bi->b_vlan_filtering = vlan_filtering;

	bi->ce_mask |= BRIDGE_ATTR_VLAN_FILTERING;
}

/**
 * Get VLAN filtering flag
 * @arg link		Link object of type bridge
 * @arg vlan_filtering	Output argument.
 *
 * @see rtnl_link_bridge_set_vlan_filtering()
 *
 * @return Zero on success, otherwise a negative error code.
 * @retval -NLE_NOATTR
 * @retval -NLE_INVAL
 */
int rtnl_link_bridge_get_vlan_filtering(struct rtnl_link *link,
					uint8_t *vlan_filtering)
{
	struct bridge_info *bi = bridge_info(link);

	IS_BRIDGE_INFO_ASSERT(link);

	if (!(bi->ce_mask & BRIDGE_ATTR_VLAN_FILTERING))
		return -NLE_NOATTR;

	if (!vlan_filtering)
		return -NLE_INVAL;

	*vlan_filtering = bi->b_vlan_filtering;
	return 0;
}

/**
 * Set VLAN protocol
 * @arg link		Link object of type bridge
 * @arg vlan_protocol	VLAN protocol to set. The protocol
 *   numbers is in host byte order.
 *
 * @see rtnl_link_bridge_get_vlan_protocol()
 *
 * @return void
 */
void rtnl_link_bridge_set_vlan_protocol(struct rtnl_link *link,
					uint16_t vlan_protocol)
{
	struct bridge_info *bi = bridge_info(link);

	IS_BRIDGE_INFO_ASSERT(link);

	bi->b_vlan_protocol = vlan_protocol;

	bi->ce_mask |= BRIDGE_ATTR_VLAN_PROTOCOL;
}

/**
 * Get VLAN protocol
 * @arg link		Link object of type bridge
 * @arg vlan_protocol	Output argument. The protocol number is in host byte order.
 *
 * @see rtnl_link_bridge_set_vlan_protocol()
 *
 * @return Zero on success, otherwise a negative error code.
 * @retval -NLE_NOATTR
 * @retval -NLE_INVAL
 */
int rtnl_link_bridge_get_vlan_protocol(struct rtnl_link *link,
				       uint16_t *vlan_protocol)
{
	struct bridge_info *bi = bridge_info(link);

	IS_BRIDGE_INFO_ASSERT(link);

	if (!(bi->ce_mask & BRIDGE_ATTR_VLAN_PROTOCOL))
		return -NLE_NOATTR;

	if (!vlan_protocol)
		return -NLE_INVAL;

	*vlan_protocol = bi->b_vlan_protocol;

	return 0;
}

/**
 * Set VLAN stats enabled flag
 * @arg link		Link object of type bridge
 * @arg vlan_stats_enabled	VLAN stats enabled flag to set
 *
 * @see rtnl_link_bridge_get_vlan_stats_enabled()
 *
 * @return void
 */
void rtnl_link_bridge_set_vlan_stats_enabled(struct rtnl_link *link,
					     uint8_t vlan_stats_enabled)
{
	struct bridge_info *bi = bridge_info(link);

	IS_BRIDGE_INFO_ASSERT(link);

	bi->b_vlan_stats_enabled = vlan_stats_enabled;

	bi->ce_mask |= BRIDGE_ATTR_VLAN_STATS_ENABLED;
}

/**
 * Get VLAN stats enabled flag
 * @arg link		Link object of type bridge
 * @arg vlan_stats_enabled	Output argument.
 *
 * @see rtnl_link_bridge_set_vlan_stats_enabled()
 *
 * @return Zero on success, otherwise a negative error code.
 * @retval -NLE_NOATTR
 * @retval -NLE_INVAL
 */
int rtnl_link_bridge_get_vlan_stats_enabled(struct rtnl_link *link,
					    uint8_t *vlan_stats_enabled)
{
	struct bridge_info *bi = bridge_info(link);

	IS_BRIDGE_INFO_ASSERT(link);

	if (!(bi->ce_mask & BRIDGE_ATTR_VLAN_STATS_ENABLED))
		return -NLE_NOATTR;

	if (!vlan_stats_enabled)
		return -NLE_INVAL;

	*vlan_stats_enabled = bi->b_vlan_stats_enabled;

	return 0;
}

static void _nl_init bridge_info_init(void)
{
	rtnl_link_register_info(&bridge_info_ops);
}

static void _nl_exit bridge_info_exit(void)
{
	rtnl_link_unregister_info(&bridge_info_ops);
}

/** @} */
