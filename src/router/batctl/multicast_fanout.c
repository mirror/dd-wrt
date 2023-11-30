// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Linus Lüssing <linus.luessing@c0d3.blue>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "main.h"
#include "sys.h"

static struct multicast_fanout_data {
	uint32_t multicast_fanout;
} multicast_fanout;

static int parse_multicast_fanout(struct state *state, int argc, char *argv[])
{
	struct settings_data *settings = state->cmd->arg;
	struct multicast_fanout_data *data = settings->data;
	char *endptr;

	if (argc != 2) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
		return -EINVAL;
	}

	data->multicast_fanout = strtoul(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0') {
		fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
		return -EINVAL;
	}

	return 0;
}

static int print_multicast_fanout(struct nl_msg *msg, void *arg)
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

	if (!attrs[BATADV_ATTR_MULTICAST_FANOUT])
		return NL_OK;

	printf("%u\n", nla_get_u32(attrs[BATADV_ATTR_MULTICAST_FANOUT]));

	*result = 0;
	return NL_STOP;
}

static int get_multicast_fanout(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_multicast_fanout);
}

static int set_attrs_multicast_fanout(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct multicast_fanout_data *data = settings->data;

	nla_put_u32(msg, BATADV_ATTR_MULTICAST_FANOUT, data->multicast_fanout);

	return 0;
}

static int set_multicast_fanout(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_multicast_fanout, NULL);
}

static struct settings_data batctl_settings_multicast_fanout = {
	.data = &multicast_fanout,
	.parse = parse_multicast_fanout,
	.netlink_get = get_multicast_fanout,
	.netlink_set = set_multicast_fanout,
};

COMMAND_NAMED(SUBCOMMAND_MIF, multicast_fanout, "mo", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_multicast_fanout,
	      "[fanout]        \tdisplay or modify multicast_fanout setting");
