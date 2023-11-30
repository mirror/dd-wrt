// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Martin Hundebøll <martin@hundeboll.net>
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

static struct simple_boolean_data network_coding;

static int print_network_coding(struct nl_msg *msg, void *arg)
{
	return sys_simple_print_boolean(msg, arg,
					BATADV_ATTR_NETWORK_CODING_ENABLED);
}

static int get_network_coding(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_network_coding);
}

static int set_attrs_network_coding(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct simple_boolean_data *data = settings->data;

	nla_put_u8(msg, BATADV_ATTR_NETWORK_CODING_ENABLED, data->val);

	return 0;
}

static int set_network_coding(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_network_coding, NULL);
}

static struct settings_data batctl_settings_network_coding = {
	.data = &network_coding,
	.parse = parse_simple_boolean,
	.netlink_get = get_network_coding,
	.netlink_set = set_network_coding,
};

COMMAND_NAMED(SUBCOMMAND_MIF, network_coding, "nc", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_network_coding,
	      "[0|1]             \tdisplay or modify network_coding setting");
