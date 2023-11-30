// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Simon Wunderlich <sw@simonwunderlich.de>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include "main.h"

#include <errno.h>
#include <linux/genetlink.h>
#include <netlink/genl/genl.h>

#include "batman_adv.h"
#include "netlink.h"
#include "sys.h"

static struct simple_boolean_data bridge_loop_avoidance;

static int print_bridge_loop_avoidance(struct nl_msg *msg, void *arg)
{
	return sys_simple_print_boolean(msg, arg,
					BATADV_ATTR_BRIDGE_LOOP_AVOIDANCE_ENABLED);
}

static int get_bridge_loop_avoidance(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_bridge_loop_avoidance);
}

static int set_attrs_bridge_loop_avoidance(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct simple_boolean_data *data = settings->data;

	nla_put_u8(msg, BATADV_ATTR_BRIDGE_LOOP_AVOIDANCE_ENABLED, data->val);

	return 0;
}

static int set_bridge_loop_avoidance(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_bridge_loop_avoidance, NULL);
}

static struct settings_data batctl_settings_bridge_loop_avoidance = {
	.data = &bridge_loop_avoidance,
	.parse = parse_simple_boolean,
	.netlink_get = get_bridge_loop_avoidance,
	.netlink_set = set_bridge_loop_avoidance,
};

COMMAND_NAMED(SUBCOMMAND_MIF, bridge_loop_avoidance, "bl", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_bridge_loop_avoidance,
	      "[0|1]             \tdisplay or modify bridge_loop_avoidance setting");
