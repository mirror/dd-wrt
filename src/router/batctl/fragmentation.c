// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
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

static struct simple_boolean_data fragmentation;

static int print_fragmentation(struct nl_msg *msg, void *arg)
{
	return sys_simple_print_boolean(msg, arg,
					BATADV_ATTR_FRAGMENTATION_ENABLED);
}

static int get_fragmentation(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_fragmentation);
}

static int set_attrs_fragmentation(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct simple_boolean_data *data = settings->data;

	nla_put_u8(msg, BATADV_ATTR_FRAGMENTATION_ENABLED, data->val);

	return 0;
}

static int set_fragmentation(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_fragmentation, NULL);
}

static struct settings_data batctl_settings_fragmentation = {
	.data = &fragmentation,
	.parse = parse_simple_boolean,
	.netlink_get = get_fragmentation,
	.netlink_set = set_fragmentation,
};

COMMAND_NAMED(SUBCOMMAND_MIF, fragmentation, "f", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_fragmentation,
	      "[0|1]             \tdisplay or modify fragmentation setting");
