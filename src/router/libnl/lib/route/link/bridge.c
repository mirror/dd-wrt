/*
 * lib/route/link/bridge.c	AF_BRIDGE link support
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2010-2013 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup link
 * @defgroup bridge Bridging
 *
 * @details
 * @{
 */

#include <netlink-private/netlink.h>
#include <netlink/netlink.h>
#include <netlink/attr.h>
#include <netlink/route/rtnl.h>
#include <netlink/route/link/bridge.h>
#include <netlink-private/route/link/api.h>
#include <linux/if_bridge.h>

#define VLAN_VID_MASK           0x0fff /* VLAN Identifier */

/** @cond SKIP */
#define BRIDGE_ATTR_PORT_STATE		(1 << 0)
#define BRIDGE_ATTR_PRIORITY		(1 << 1)
#define BRIDGE_ATTR_COST		(1 << 2)
#define BRIDGE_ATTR_FLAGS		(1 << 3)
#define BRIDGE_ATTR_PORT_VLAN           (1 << 4)
#define BRIDGE_ATTR_HWMODE		(1 << 5)
#define BRIDGE_ATTR_SELF		(1 << 6)

#define PRIV_FLAG_NEW_ATTRS		(1 << 0)

struct bridge_data
{
	uint8_t			b_port_state;
	uint8_t			b_priv_flags; /* internal flags */
	uint16_t		b_hwmode;
	uint16_t		b_priority;
	uint16_t		b_self; /* here for comparison reasons */
	uint32_t		b_cost;
	uint32_t		b_flags;
	uint32_t		b_flags_mask;
	uint32_t                ce_mask; /* HACK to support attr macros */
	struct rtnl_link_bridge_vlan vlan_info;
};

static void set_bit(unsigned nr, uint32_t *addr)
{
	if (nr < RTNL_LINK_BRIDGE_VLAN_BITMAP_MAX)
		addr[nr / 32] |= (((uint32_t) 1) << (nr % 32));
}

static int find_next_bit(int i, uint32_t x)
{
	int j;

	if (i >= 32)
		return -1;

	/* find first bit */
	if (i < 0)
		return __builtin_ffs(x);

	/* mask off prior finds to get next */
	j = __builtin_ffs(x >> i);
	return j ? j + i : 0;
}

static struct rtnl_link_af_ops bridge_ops;

#define IS_BRIDGE_LINK_ASSERT(link) \
	if (!rtnl_link_is_bridge(link)) { \
		APPBUG("A function was expecting a link object of type bridge."); \
		return -NLE_OPNOTSUPP; \
	}

static inline struct bridge_data *bridge_data(struct rtnl_link *link)
{
	return rtnl_link_af_data(link, &bridge_ops);
}

static void *bridge_alloc(struct rtnl_link *link)
{
	return calloc(1, sizeof(struct bridge_data));
}

static void *bridge_clone(struct rtnl_link *link, void *data)
{
	struct bridge_data *bd;

	if ((bd = bridge_alloc(link)))
		memcpy(bd, data, sizeof(*bd));

	return bd;
}

static void bridge_free(struct rtnl_link *link, void *data)
{
	free(data);
}

static struct nla_policy br_attrs_policy[IFLA_BRPORT_MAX+1] = {
	[IFLA_BRPORT_STATE]		= { .type = NLA_U8 },
	[IFLA_BRPORT_PRIORITY]		= { .type = NLA_U16 },
	[IFLA_BRPORT_COST]		= { .type = NLA_U32 },
	[IFLA_BRPORT_MODE]		= { .type = NLA_U8 },
	[IFLA_BRPORT_GUARD]		= { .type = NLA_U8 },
	[IFLA_BRPORT_PROTECT]		= { .type = NLA_U8 },
	[IFLA_BRPORT_FAST_LEAVE]	= { .type = NLA_U8 },
	[IFLA_BRPORT_LEARNING]		= { .type = NLA_U8 },
	[IFLA_BRPORT_LEARNING_SYNC]	= { .type = NLA_U8 },
	[IFLA_BRPORT_UNICAST_FLOOD]	= { .type = NLA_U8 },
};

static void check_flag(struct rtnl_link *link, struct nlattr *attrs[],
		       int type, int flag)
{
	if (attrs[type] && nla_get_u8(attrs[type]))
		rtnl_link_bridge_set_flags(link, flag);
}

static int bridge_parse_protinfo(struct rtnl_link *link, struct nlattr *attr,
				 void *data)
{
	struct bridge_data *bd = data;
	struct nlattr *br_attrs[IFLA_BRPORT_MAX+1];
	int err;

	/* Backwards compatibility */
	if (!nla_is_nested(attr)) {
		if (nla_len(attr) < 1)
			return -NLE_RANGE;

		bd->b_port_state = nla_get_u8(attr);
		bd->ce_mask |= BRIDGE_ATTR_PORT_STATE;

		return 0;
	}

	if ((err = nla_parse_nested(br_attrs, IFLA_BRPORT_MAX, attr,
	     br_attrs_policy)) < 0)
		return err;

	bd->b_priv_flags |= PRIV_FLAG_NEW_ATTRS;

	if (br_attrs[IFLA_BRPORT_STATE]) {
		bd->b_port_state = nla_get_u8(br_attrs[IFLA_BRPORT_STATE]);
		bd->ce_mask |= BRIDGE_ATTR_PORT_STATE;
	}

	if (br_attrs[IFLA_BRPORT_PRIORITY]) {
		bd->b_priority = nla_get_u16(br_attrs[IFLA_BRPORT_PRIORITY]);
		bd->ce_mask |= BRIDGE_ATTR_PRIORITY;
	}

	if (br_attrs[IFLA_BRPORT_COST]) {
		bd->b_cost = nla_get_u32(br_attrs[IFLA_BRPORT_COST]);
		bd->ce_mask |= BRIDGE_ATTR_COST;
	}

	check_flag(link, br_attrs, IFLA_BRPORT_MODE, RTNL_BRIDGE_HAIRPIN_MODE);
	check_flag(link, br_attrs, IFLA_BRPORT_GUARD, RTNL_BRIDGE_BPDU_GUARD);
	check_flag(link, br_attrs, IFLA_BRPORT_PROTECT, RTNL_BRIDGE_ROOT_BLOCK);
	check_flag(link, br_attrs, IFLA_BRPORT_FAST_LEAVE, RTNL_BRIDGE_FAST_LEAVE);
	check_flag(link, br_attrs, IFLA_BRPORT_UNICAST_FLOOD,
	           RTNL_BRIDGE_UNICAST_FLOOD);
	check_flag(link, br_attrs, IFLA_BRPORT_LEARNING, RTNL_BRIDGE_LEARNING);
	check_flag(link, br_attrs, IFLA_BRPORT_LEARNING_SYNC,
	           RTNL_BRIDGE_LEARNING_SYNC);

	return 0;
}

static int bridge_parse_af_full(struct rtnl_link *link, struct nlattr *attr_full,
                                void *data)
{
	struct bridge_data *bd = data;
	struct bridge_vlan_info *vinfo = NULL;
	uint16_t vid_range_start = 0;
	uint16_t vid_range_flags = -1;

	struct nlattr *attr;
	int remaining;

	nla_for_each_nested(attr, attr_full, remaining) {

		if (nla_type(attr) == IFLA_BRIDGE_MODE) {
			bd->b_hwmode = nla_get_u16(attr);
			bd->ce_mask |= BRIDGE_ATTR_HWMODE;
		} else if (nla_type(attr) != IFLA_BRIDGE_VLAN_INFO)
			continue;

		if (nla_len(attr) != sizeof(struct bridge_vlan_info))
			return -EINVAL;

		vinfo = nla_data(attr);
		if (!vinfo->vid || vinfo->vid >= VLAN_VID_MASK)
			return -EINVAL;


		if (vinfo->flags & BRIDGE_VLAN_INFO_RANGE_BEGIN) {
			vid_range_start = vinfo->vid;
			vid_range_flags = (vinfo->flags ^ BRIDGE_VLAN_INFO_RANGE_BEGIN);
			continue;
		}

		if (vinfo->flags & BRIDGE_VLAN_INFO_RANGE_END) {
			/* sanity check the range flags */
			if (vid_range_flags != (vinfo->flags ^ BRIDGE_VLAN_INFO_RANGE_END)) {
				NL_DBG(1, "VLAN range flags differ; can not handle it.\n");
				return -EINVAL;
			}
		} else {
			vid_range_start = vinfo->vid;
		}

		for (; vid_range_start <= vinfo->vid; vid_range_start++) {
			if (vinfo->flags & BRIDGE_VLAN_INFO_PVID)
				bd->vlan_info.pvid = vinfo->vid;

			if (vinfo->flags & BRIDGE_VLAN_INFO_UNTAGGED)
				set_bit(vid_range_start, bd->vlan_info.untagged_bitmap);

			set_bit(vid_range_start, bd->vlan_info.vlan_bitmap);
			bd->ce_mask |= BRIDGE_ATTR_PORT_VLAN;
		}

		vid_range_flags = -1;
	}

	return 0;
}

static int bridge_fill_af(struct rtnl_link *link, struct nl_msg *msg,
		   void *data)
{
	struct bridge_data *bd = data;

	if ((bd->ce_mask & BRIDGE_ATTR_SELF)||(bd->ce_mask & BRIDGE_ATTR_HWMODE))
		NLA_PUT_U16(msg, IFLA_BRIDGE_FLAGS, BRIDGE_FLAGS_SELF);

	if (bd->ce_mask & BRIDGE_ATTR_HWMODE)
		NLA_PUT_U16(msg, IFLA_BRIDGE_MODE, bd->b_hwmode);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int bridge_fill_pi(struct rtnl_link *link, struct nl_msg *msg,
		   void *data)
{
	struct bridge_data *bd = data;

	if (bd->ce_mask & BRIDGE_ATTR_FLAGS) {
		if (bd->b_flags_mask & RTNL_BRIDGE_BPDU_GUARD) {
			NLA_PUT_U8(msg, IFLA_BRPORT_GUARD,
						bd->b_flags & RTNL_BRIDGE_BPDU_GUARD);
		}
		if (bd->b_flags_mask & RTNL_BRIDGE_HAIRPIN_MODE) {
			NLA_PUT_U8(msg, IFLA_BRPORT_MODE,
			           bd->b_flags & RTNL_BRIDGE_HAIRPIN_MODE);
		}
		if (bd->b_flags_mask & RTNL_BRIDGE_FAST_LEAVE) {
			NLA_PUT_U8(msg, IFLA_BRPORT_FAST_LEAVE,
			           bd->b_flags & RTNL_BRIDGE_FAST_LEAVE);
		}
		if (bd->b_flags_mask & RTNL_BRIDGE_ROOT_BLOCK) {
			NLA_PUT_U8(msg, IFLA_BRPORT_PROTECT,
			           bd->b_flags & RTNL_BRIDGE_ROOT_BLOCK);
		}
		if (bd->b_flags_mask & RTNL_BRIDGE_UNICAST_FLOOD) {
			NLA_PUT_U8(msg, IFLA_BRPORT_UNICAST_FLOOD,
			           bd->b_flags & RTNL_BRIDGE_UNICAST_FLOOD);
		}
		if (bd->b_flags_mask & RTNL_BRIDGE_LEARNING) {
			NLA_PUT_U8(msg, IFLA_BRPORT_LEARNING,
			           bd->b_flags & RTNL_BRIDGE_LEARNING);
		}
		if (bd->b_flags_mask & RTNL_BRIDGE_LEARNING_SYNC) {
			NLA_PUT_U8(msg, IFLA_BRPORT_LEARNING_SYNC,
			           bd->b_flags & RTNL_BRIDGE_LEARNING_SYNC);
		}
	}

	if (bd->ce_mask & BRIDGE_ATTR_COST)
		NLA_PUT_U32(msg, IFLA_BRPORT_COST, bd->b_cost);

	if (bd->ce_mask & BRIDGE_ATTR_PRIORITY)
		NLA_PUT_U16(msg, IFLA_BRPORT_PRIORITY, bd->b_priority);

	if (bd->ce_mask & BRIDGE_ATTR_PORT_STATE)
		NLA_PUT_U8(msg, IFLA_BRPORT_STATE, bd->b_port_state);

	return 0;

nla_put_failure:
	return -NLE_MSGSIZE;
}

static int bridge_override_rtm(struct rtnl_link *link) {
        struct bridge_data *bd;

        if (!rtnl_link_is_bridge(link))
                return 0;

        bd = bridge_data(link);

        if (bd->ce_mask & BRIDGE_ATTR_FLAGS)
                return 1;

        return 0;
}

static int bridge_get_af(struct nl_msg *msg, uint32_t *ext_filter_mask)
{
	*ext_filter_mask |= RTEXT_FILTER_BRVLAN;
	return 0;
}

static void dump_bitmap(struct nl_dump_params *p, const uint32_t *b)
{
	int i = -1, j, k;
	int start = -1, prev = -1;
	int done, found = 0;

	for (k = 0; k < RTNL_LINK_BRIDGE_VLAN_BITMAP_LEN; k++) {
		int base_bit;
		uint32_t a = b[k];

		base_bit = k * 32;
		i = -1;
		done = 0;
		while (!done) {
			j = find_next_bit(i, a);
			if (j > 0) {
				/* first hit of any bit */
				if (start < 0 && prev < 0) {
					start = prev = j - 1 + base_bit;
					goto next;
				}
				/* this bit is a continuation of prior bits */
				if (j - 2 + base_bit == prev) {
					prev++;
					goto next;
				}
			} else
				done = 1;

			if (start >= 0) {
				found++;
				if (done && k < RTNL_LINK_BRIDGE_VLAN_BITMAP_LEN - 1)
					break;

				nl_dump(p, " %d", start);
				if (start != prev)
					nl_dump(p, "-%d", prev);

				if (done)
					break;
			}
			if (j > 0)
				start = prev = j - 1 + base_bit;
next:
			i = j;
		}
	}
	if (!found)
		nl_dump(p, " <none>");

	return;
}

static void rtnl_link_bridge_dump_vlans(struct nl_dump_params *p,
					struct bridge_data *bd)
{
	nl_dump(p, "pvid %u", bd->vlan_info.pvid);

	nl_dump(p, "   all vlans:");
	dump_bitmap(p, bd->vlan_info.vlan_bitmap);

	nl_dump(p, "   untagged vlans:");
	dump_bitmap(p, bd->vlan_info.untagged_bitmap);
}

static void bridge_dump_details(struct rtnl_link *link,
				struct nl_dump_params *p, void *data)
{
	struct bridge_data *bd = data;

	nl_dump_line(p, "    bridge: ");

	if (bd->ce_mask & BRIDGE_ATTR_PORT_STATE)
		nl_dump(p, "port-state %u ", bd->b_port_state);

	if (bd->ce_mask & BRIDGE_ATTR_PRIORITY)
		nl_dump(p, "prio %u ", bd->b_priority);

	if (bd->ce_mask & BRIDGE_ATTR_COST)
		nl_dump(p, "cost %u ", bd->b_cost);

	if (bd->ce_mask & BRIDGE_ATTR_HWMODE) {
		char hbuf[32];

		rtnl_link_bridge_hwmode2str(bd->b_hwmode, hbuf, sizeof(hbuf));
		nl_dump(p, "hwmode %s", hbuf);
	}

	if (bd->ce_mask & BRIDGE_ATTR_PORT_VLAN)
		rtnl_link_bridge_dump_vlans(p, bd);

	if (bd->ce_mask & BRIDGE_ATTR_FLAGS) {
		char buf[256];

		rtnl_link_bridge_flags2str(bd->b_flags & bd->b_flags_mask,
					   buf, sizeof(buf));
		nl_dump(p, "%s", buf);
	}

	nl_dump(p, "\n");
}

static int bridge_compare(struct rtnl_link *_a, struct rtnl_link *_b,
			  int family, uint32_t attrs, int flags)
{
	struct bridge_data *a = bridge_data(_a);
	struct bridge_data *b = bridge_data(_b);
	int diff = 0;

#define BRIDGE_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, BRIDGE_ATTR_##ATTR, a, b, EXPR)
	diff |= BRIDGE_DIFF(PORT_STATE,	a->b_port_state != b->b_port_state);
	diff |= BRIDGE_DIFF(PRIORITY, a->b_priority != b->b_priority);
	diff |= BRIDGE_DIFF(COST, a->b_cost != b->b_cost);
	diff |= BRIDGE_DIFF(PORT_VLAN, memcmp(&a->vlan_info, &b->vlan_info,
					      sizeof(struct rtnl_link_bridge_vlan)));
	diff |= BRIDGE_DIFF(HWMODE, a->b_hwmode != b->b_hwmode);
	diff |= BRIDGE_DIFF(SELF, a->b_self != b->b_self);

	if (flags & LOOSE_COMPARISON)
		diff |= BRIDGE_DIFF(FLAGS,
				  (a->b_flags ^ b->b_flags) & b->b_flags_mask);
	else
		diff |= BRIDGE_DIFF(FLAGS, a->b_flags != b->b_flags);
#undef BRIDGE_DIFF

	return diff;
}
/** @endcond */

/**
 * Allocate link object of type bridge
 *
 * @return Allocated link object or NULL.
 */
struct rtnl_link *rtnl_link_bridge_alloc(void)
{
	struct rtnl_link *link;
	int err;

	if (!(link = rtnl_link_alloc()))
		return NULL;

	if ((err = rtnl_link_set_type(link, "bridge")) < 0) {
		rtnl_link_put(link);
		return NULL;
	}

	return link;
}

/**
 * Create a new kernel bridge device
 * @arg sk              netlink socket
 * @arg name            name of the bridge device or NULL
 *
 * Creates a new bridge device in the kernel. If no name is
 * provided, the kernel will automatically pick a name of the
 * form "type%d" (e.g. bridge0, vlan1, etc.)
 *
 * @return 0 on success or a negative error code
*/
int rtnl_link_bridge_add(struct nl_sock *sk, const char *name)
{
	int err;
	struct rtnl_link *link;

	if (!(link = rtnl_link_bridge_alloc()))
		return -NLE_NOMEM;

	if(name)
		rtnl_link_set_name(link, name);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	rtnl_link_put(link);

	return err;
}

/**
 * Check if a link is a bridge
 * @arg link		Link object
 *
 * @return 1 if the link is a bridge, 0 otherwise.
 */
int rtnl_link_is_bridge(struct rtnl_link *link)
{
	return link->l_family == AF_BRIDGE &&
	       link->l_af_ops == &bridge_ops;
}

/**
 * Check if bridge has extended information
 * @arg link		Link object of type bridge
 *
 * Checks if the bridge object has been constructed based on
 * information that is only available in newer kernels. This
 * affectes the following functions:
 *  - rtnl_link_bridge_get_cost()
 *  - rtnl_link_bridge_get_priority()
 *  - rtnl_link_bridge_get_flags()
 *
 * @return 1 if extended information is available, otherwise 0 is returned.
 */
int rtnl_link_bridge_has_ext_info(struct rtnl_link *link)
{
	struct bridge_data *bd;

	if (!rtnl_link_is_bridge(link))
		return 0;

	bd = bridge_data(link);
	return !!(bd->b_priv_flags & PRIV_FLAG_NEW_ATTRS);
}

/**
 * Set Spanning Tree Protocol (STP) port state
 * @arg link		Link object of type bridge
 * @arg state		New STP port state
 *
 * The value of state must be one of the following:
 *   - BR_STATE_DISABLED
 *   - BR_STATE_LISTENING
 *   - BR_STATE_LEARNING
 *   - BR_STATE_FORWARDING
 *   - BR_STATE_BLOCKING
 *
 * @see rtnl_link_bridge_get_port_state()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 * @retval -NLE_INVAL Invalid state value (0..BR_STATE_BLOCKING)
 */
int rtnl_link_bridge_set_port_state(struct rtnl_link *link, uint8_t state)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	if (state > BR_STATE_BLOCKING)
		return -NLE_INVAL;

	bd->b_port_state = state;
	bd->ce_mask |= BRIDGE_ATTR_PORT_STATE;

	return 0;
}

/**
 * Get Spanning Tree Protocol (STP) port state
 * @arg link		Link object of type bridge
 *
 * @see rtnl_link_bridge_set_port_state()
 *
 * @return The STP port state or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_get_port_state(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	return bd->b_port_state;
}

/**
 * Set priority
 * @arg link		Link object of type bridge
 * @arg prio		Bridge priority
 *
 * @see rtnl_link_bridge_get_priority()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_set_priority(struct rtnl_link *link, uint16_t prio)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_priority = prio;
	bd->ce_mask |= BRIDGE_ATTR_PRIORITY;

	return 0;
}

/**
 * Get priority
 * @arg link		Link object of type bridge
 *
 * @see rtnl_link_bridge_set_priority()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_get_priority(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	return bd->b_priority;
}

/**
 * Set Spanning Tree Protocol (STP) path cost
 * @arg link		Link object of type bridge
 * @arg cost		New STP path cost value
 *
 * @see rtnl_link_bridge_get_cost()
 *
 * @return The bridge priority or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_set_cost(struct rtnl_link *link, uint32_t cost)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_cost = cost;
	bd->ce_mask |= BRIDGE_ATTR_COST;

	return 0;
}

/**
 * Get Spanning Tree Protocol (STP) path cost
 * @arg link		Link object of type bridge
 * @arg cost		Pointer to store STP cost value
 *
 * @see rtnl_link_bridge_set_cost()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 * @retval -NLE_INVAL `cost` is not a valid pointer
 */
int rtnl_link_bridge_get_cost(struct rtnl_link *link, uint32_t *cost)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	if (!cost)
		return -NLE_INVAL;

	*cost = bd->b_cost;

	return 0;
}

/**
 * Unset flags
 * @arg link		Link object of type bridge
 * @arg flags		Bridging flags to unset
 *
 * @see rtnl_link_bridge_set_flags()
 * @see rtnl_link_bridge_get_flags()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_unset_flags(struct rtnl_link *link, unsigned int flags)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_flags_mask |= flags;
	bd->b_flags &= ~flags;
	bd->ce_mask |= BRIDGE_ATTR_FLAGS;

	return 0;
}

/**
 * Set flags
 * @arg link		Link object of type bridge
 * @arg flags		Bridging flags to set
 *
 * Valid flags are:
 *   - RTNL_BRIDGE_HAIRPIN_MODE
 *   - RTNL_BRIDGE_BPDU_GUARD
 *   - RTNL_BRIDGE_ROOT_BLOCK
 *   - RTNL_BRIDGE_FAST_LEAVE
 *   - RTNL_BRIDGE_UNICAST_FLOOD
 *   - RTNL_BRIDGE_LEARNING
 *   - RTNL_BRIDGE_LEARNING_SYNC
 *
 * @see rtnl_link_bridge_unset_flags()
 * @see rtnl_link_bridge_get_flags()
 *
 * @return 0 on success or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_set_flags(struct rtnl_link *link, unsigned int flags)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_flags_mask |= flags;
	bd->b_flags |= flags;
	bd->ce_mask |= BRIDGE_ATTR_FLAGS;

	return 0;
}

/**
 * Get flags
 * @arg link		Link object of type bridge
 *
 * @see rtnl_link_bridge_set_flags()
 * @see rtnl_link_bridge_unset_flags()
 *
 * @return Flags or a negative error code.
 * @retval -NLE_OPNOTSUPP Link is not a bridge
 */
int rtnl_link_bridge_get_flags(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	return bd->b_flags;
}

/**
 * Set link change type to self
 * @arg link		Link Object of type bridge
 *
 * This will set the bridge change flag to self, meaning that changes to
 * be applied with this link object will be applied directly to the physical
 * device in a bridge instead of the virtual device.
 *
 * @return 0 on success or negative error code
 * @return -NLE_OPNOTSUP Link is not a bridge
 */
int rtnl_link_bridge_set_self(struct rtnl_link *link)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	bd->b_self |= 1;
	bd->ce_mask |= BRIDGE_ATTR_SELF;

	return 0;
}

/**
 * Get hardware mode
 * @arg link            Link object of type bridge
 * @arg hwmode          Output argument.
 *
 * @see rtnl_link_bridge_set_hwmode()
 *
 * @return 0 if hardware mode is present and returned in hwmode
 * @return -NLE_NOATTR if hardware mode is not present
 * @return -NLE_OPNOTSUP Link is not a bridge
 */
int rtnl_link_bridge_get_hwmode(struct rtnl_link *link, uint16_t *hwmode)
{
	struct bridge_data *bd = bridge_data(link);

	IS_BRIDGE_LINK_ASSERT(link);

	if (!(bd->ce_mask & BRIDGE_ATTR_HWMODE))
		return -NLE_NOATTR;

	*hwmode = bd->b_hwmode;
	return 0;
}

/**
 * Set hardware mode
 * @arg link		Link object of type bridge
 * @arg hwmode		Hardware mode to set on link
 *
 * This will set the hardware mode of a link when it supports hardware
 * offloads for bridging.
 * @see rtnl_link_bridge_get_hwmode()
 *
 * Valid modes are:
 *   - RTNL_BRIDGE_HWMODE_VEB
 *   - RTNL_BRIDGE_HWMODE_VEPA
 *
 * When setting hardware mode, the change type will be set to self.
 * @see rtnl_link_bridge_set_self()
 *
 * @return 0 on success or negative error code
 * @return -NLE_OPNOTSUP Link is not a bridge
 * @return -NLE_INVAL when specified hwmode is unsupported.
 */
int rtnl_link_bridge_set_hwmode(struct rtnl_link *link, uint16_t hwmode)
{
	int err;
	struct bridge_data *bd = bridge_data(link);

	if (hwmode > RTNL_BRIDGE_HWMODE_MAX)
		return -NLE_INVAL;

	if ((err = rtnl_link_bridge_set_self(link)) < 0)
		return err;

	bd->b_hwmode = hwmode;
	bd->ce_mask |= BRIDGE_ATTR_HWMODE;

	return 0;
}


static const struct trans_tbl bridge_flags[] = {
	__ADD(RTNL_BRIDGE_HAIRPIN_MODE, hairpin_mode),
	__ADD(RTNL_BRIDGE_BPDU_GUARD, 	bpdu_guard),
	__ADD(RTNL_BRIDGE_ROOT_BLOCK,	root_block),
	__ADD(RTNL_BRIDGE_FAST_LEAVE,	fast_leave),
	__ADD(RTNL_BRIDGE_UNICAST_FLOOD,	flood),
	__ADD(RTNL_BRIDGE_LEARNING,			learning),
	__ADD(RTNL_BRIDGE_LEARNING_SYNC,	learning_sync),
};

/**
 * @name Flag Translation
 * @{
 */

char *rtnl_link_bridge_flags2str(int flags, char *buf, size_t len)
{
	return __flags2str(flags, buf, len, bridge_flags, ARRAY_SIZE(bridge_flags));
}

int rtnl_link_bridge_str2flags(const char *name)
{
	return __str2flags(name, bridge_flags, ARRAY_SIZE(bridge_flags));
}

/** @} */

static const struct trans_tbl port_states[] = {
	__ADD(BR_STATE_DISABLED, disabled),
	__ADD(BR_STATE_LISTENING, listening),
	__ADD(BR_STATE_LEARNING, learning),
	__ADD(BR_STATE_FORWARDING, forwarding),
	__ADD(BR_STATE_BLOCKING, blocking),
};

/**
 * @name Port State Translation
 * @{
 */

char *rtnl_link_bridge_portstate2str(int st, char *buf, size_t len)
{
	return __type2str(st, buf, len, port_states, ARRAY_SIZE(port_states));
}

int rtnl_link_bridge_str2portstate(const char *name)
{
	return __str2type(name, port_states, ARRAY_SIZE(port_states));
}

/** @} */

static const struct trans_tbl hw_modes[] = {
	__ADD(RTNL_BRIDGE_HWMODE_VEB, veb),
	__ADD(RTNL_BRIDGE_HWMODE_VEPA, vepa),
	__ADD(RTNL_BRIDGE_HWMODE_UNDEF, undef),
};

/**
 * @name Hardware Mode Translation
 * @{
 */

char *rtnl_link_bridge_hwmode2str(uint16_t st, char *buf, size_t len) {
	return __type2str(st, buf, len, hw_modes, ARRAY_SIZE(hw_modes));
}

uint16_t rtnl_link_bridge_str2hwmode(const char *name)
{
	return __str2type(name, hw_modes, ARRAY_SIZE(hw_modes));
}

/** @} */

int rtnl_link_bridge_pvid(struct rtnl_link *link)
{
	struct bridge_data *bd;

	IS_BRIDGE_LINK_ASSERT(link);

	bd = link->l_af_data[AF_BRIDGE];
	if (bd->ce_mask & BRIDGE_ATTR_PORT_VLAN)
		return (int) bd->vlan_info.pvid;

	return -EINVAL;
}

int rtnl_link_bridge_has_vlan(struct rtnl_link *link)
{
	struct bridge_data *bd;
	int i;

	IS_BRIDGE_LINK_ASSERT(link);

	bd = link->l_af_data[AF_BRIDGE];
	if (bd->ce_mask & BRIDGE_ATTR_PORT_VLAN) {
		if (bd->vlan_info.pvid)
			return 1;

		for (i = 0; i < RTNL_LINK_BRIDGE_VLAN_BITMAP_LEN; ++i) {
			if (bd->vlan_info.vlan_bitmap[i] ||
			    bd->vlan_info.untagged_bitmap[i])
				return 1;
		}
	}
	return 0;
}

struct rtnl_link_bridge_vlan *rtnl_link_bridge_get_port_vlan(struct rtnl_link *link)
{
	struct bridge_data *data;

	if (!rtnl_link_is_bridge(link))
		return NULL;

	data = link->l_af_data[AF_BRIDGE];
	if (data && (data->ce_mask & BRIDGE_ATTR_PORT_VLAN))
		return &data->vlan_info;

	return NULL;
}

static struct rtnl_link_af_ops bridge_ops = {
	.ao_family			= AF_BRIDGE,
	.ao_alloc			= &bridge_alloc,
	.ao_clone			= &bridge_clone,
	.ao_free			= &bridge_free,
	.ao_parse_protinfo		= &bridge_parse_protinfo,
	.ao_dump[NL_DUMP_DETAILS]	= &bridge_dump_details,
	.ao_compare			= &bridge_compare,
	.ao_parse_af_full		= &bridge_parse_af_full,
	.ao_get_af			= &bridge_get_af,
	.ao_fill_af			= &bridge_fill_af,
	.ao_fill_pi			= &bridge_fill_pi,
	.ao_fill_pi_flags	= NLA_F_NESTED,
	.ao_override_rtm		= &bridge_override_rtm,
	.ao_fill_af_no_nest	= 1,
};

static void __init bridge_init(void)
{
	rtnl_link_af_register(&bridge_ops);
}

static void __exit bridge_exit(void)
{
	rtnl_link_af_unregister(&bridge_ops);
}

/** @} */
