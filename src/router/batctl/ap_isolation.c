// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli <a@unstable.cc>
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

static struct simple_boolean_data ap_isolation;

static int print_ap_isolation(struct nl_msg *msg, void *arg)
{
	return sys_simple_print_boolean(msg, arg,
					BATADV_ATTR_AP_ISOLATION_ENABLED);
}

static int get_attrs_ap_isolation(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;

	if (state->selector == SP_VLAN)
		nla_put_u16(msg, BATADV_ATTR_VLANID, state->vid);

	return 0;
}

static int get_ap_isolation(struct state *state)
{
	enum batadv_nl_commands nl_cmd = BATADV_CMD_GET_MESH;

	if (state->selector == SP_VLAN)
		nl_cmd = BATADV_CMD_GET_VLAN;

	return sys_simple_nlquery(state, nl_cmd, get_attrs_ap_isolation,
				  print_ap_isolation);
}

static int set_attrs_ap_isolation(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct simple_boolean_data *data = settings->data;

	nla_put_u8(msg, BATADV_ATTR_AP_ISOLATION_ENABLED, data->val);

	if (state->selector == SP_VLAN)
		nla_put_u16(msg, BATADV_ATTR_VLANID, state->vid);

	return 0;
}

static int set_ap_isolation(struct state *state)
{
	enum batadv_nl_commands nl_cmd = BATADV_CMD_SET_MESH;

	if (state->selector == SP_VLAN)
		nl_cmd = BATADV_CMD_SET_VLAN;

	return sys_simple_nlquery(state, nl_cmd, set_attrs_ap_isolation, NULL);
}

static struct settings_data batctl_settings_ap_isolation = {
	.data = &ap_isolation,
	.parse = parse_simple_boolean,
	.netlink_get = get_ap_isolation,
	.netlink_set = set_ap_isolation,
};

COMMAND_NAMED(SUBCOMMAND_MIF, ap_isolation, "ap", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_ap_isolation,
	      "[0|1]             \tdisplay or modify ap_isolation setting");

COMMAND_NAMED(SUBCOMMAND_VID, ap_isolation, "ap", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_ap_isolation,
	      "[0|1]             \tdisplay or modify ap_isolation setting for vlan device or id");
