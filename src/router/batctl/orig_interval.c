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

#include "main.h"
#include "sys.h"

static struct orig_interval_data {
	uint32_t orig_interval;
} orig_interval;

static int parse_orig_interval(struct state *state, int argc, char *argv[])
{
	struct settings_data *settings = state->cmd->arg;
	struct orig_interval_data *data = settings->data;
	char *endptr;

	if (argc != 2) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
		return -EINVAL;
	}

	data->orig_interval = strtoul(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0') {
		fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
		return -EINVAL;
	}

	return 0;
}

static int print_orig_interval(struct nl_msg *msg, void *arg)
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

	if (!attrs[BATADV_ATTR_ORIG_INTERVAL])
		return NL_OK;

	printf("%u\n", nla_get_u32(attrs[BATADV_ATTR_ORIG_INTERVAL]));

	*result = 0;
	return NL_STOP;
}

static int get_orig_interval(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, print_orig_interval);
}

static int set_attrs_orig_interval(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct orig_interval_data *data = settings->data;

	nla_put_u32(msg, BATADV_ATTR_ORIG_INTERVAL, data->orig_interval);

	return 0;
}

static int set_orig_interval(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_orig_interval, NULL);
}

static struct settings_data batctl_settings_orig_interval = {
	.data = &orig_interval,
	.parse = parse_orig_interval,
	.netlink_get = get_orig_interval,
	.netlink_set = set_orig_interval,
};

COMMAND_NAMED(SUBCOMMAND_MIF, orig_interval, "it", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_orig_interval,
	      "[interval]        \tdisplay or modify orig_interval setting");
