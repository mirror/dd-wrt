// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Linus Lüssing <linus.luessing@c0d3.blue>
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

static struct simple_boolean_data multicast_forceflood;

static int print_multicast_forceflood(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX + 1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr;
	int *result = arg;

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (!attrs[BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED])
		return NL_OK;

	printf("%s\n", nla_get_u8(attrs[BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED]) ? "enabled" : "disabled");

	*result = 0;
	return NL_STOP;
}

static int get_multicast_forceflood(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_multicast_forceflood);
}

static int set_attrs_multicast_forceflood(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct simple_boolean_data *data = settings->data;

	nla_put_u8(msg, BATADV_ATTR_MULTICAST_FORCEFLOOD_ENABLED, data->val);

	return 0;
}

static int set_multicast_forceflood(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_multicast_forceflood, NULL);
}

static struct settings_data batctl_settings_multicast_forceflood = {
	.data = &multicast_forceflood,
	.parse = parse_simple_boolean,
	.netlink_get = get_multicast_forceflood,
	.netlink_set = set_multicast_forceflood,
};

COMMAND_NAMED(SUBCOMMAND_MIF, multicast_forceflood, "mff", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK | COMMAND_FLAG_INVERSE,
	      &batctl_settings_multicast_forceflood,
	      "[0|1]             \tdisplay or modify multicast_forceflood setting");
