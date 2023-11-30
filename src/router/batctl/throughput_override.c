// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "functions.h"
#include "main.h"
#include "sys.h"

static struct throughput_override_data {
	uint32_t throughput_override;
} throughput_override;

static int parse_throughput_override(struct state *state, int argc, char *argv[])
{
	struct settings_data *settings = state->cmd->arg;
	struct throughput_override_data *data = settings->data;
	bool ret;

	if (argc != 2) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
		return -EINVAL;
	}

	ret = parse_throughput(argv[1], "throughput override",
				&data->throughput_override);
	if (!ret)
		return -EINVAL;

	return 0;
}

static int print_throughput_override(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs[BATADV_ATTR_MAX + 1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr;
	int *result = arg;
	uint32_t mbit;

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	if (!attrs[BATADV_ATTR_THROUGHPUT_OVERRIDE])
		return NL_OK;

	mbit = nla_get_u32(attrs[BATADV_ATTR_THROUGHPUT_OVERRIDE]);
	printf("%u.%u MBit\n", mbit / 10, mbit % 10);

	*result = 0;
	return NL_STOP;
}

static int get_attrs_throughput_override(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;

	nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX, state->hif);

	return 0;
}

static int get_throughput_override(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_HARDIF,
				  get_attrs_throughput_override,
				  print_throughput_override);
}

static int set_attrs_throughput_override(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct throughput_override_data *data = settings->data;

	nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX, state->hif);
	nla_put_u32(msg, BATADV_ATTR_THROUGHPUT_OVERRIDE, data->throughput_override);

	return 0;
}

static int set_throughput_override(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_HARDIF,
				  set_attrs_throughput_override, NULL);
}

static struct settings_data batctl_settings_throughput_override = {
	.data = &throughput_override,
	.parse = parse_throughput_override,
	.netlink_get = get_throughput_override,
	.netlink_set = set_throughput_override,
};

COMMAND_NAMED(SUBCOMMAND_HIF, throughput_override, "to", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_throughput_override,
	      "[mbit]        \tdisplay or modify throughput_override setting");
