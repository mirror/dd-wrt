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

static struct elp_interval_data {
	uint32_t elp_interval;
} elp_interval;

static int parse_elp_interval(struct state *state, int argc, char *argv[])
{
	struct settings_data *settings = state->cmd->arg;
	struct elp_interval_data *data = settings->data;
	char *endptr;

	if (argc != 2) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
		return -EINVAL;
	}

	data->elp_interval = strtoul(argv[1], &endptr, 0);
	if (!endptr || *endptr != '\0') {
		fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
		return -EINVAL;
	}

	return 0;
}

static int print_elp_interval(struct nl_msg *msg, void *arg)
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

	if (!attrs[BATADV_ATTR_ELP_INTERVAL])
		return NL_OK;

	printf("%u\n", nla_get_u32(attrs[BATADV_ATTR_ELP_INTERVAL]));

	*result = 0;
	return NL_STOP;
}

static int get_attrs_elp_interval(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;

	nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX, state->hif);

	return 0;
}

static int get_elp_interval(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_HARDIF,
				  get_attrs_elp_interval, print_elp_interval);
}

static int set_attrs_elp_interval(struct nl_msg *msg, void *arg)
{
	struct state *state = arg;
	struct settings_data *settings = state->cmd->arg;
	struct elp_interval_data *data = settings->data;

	nla_put_u32(msg, BATADV_ATTR_HARD_IFINDEX, state->hif);
	nla_put_u32(msg, BATADV_ATTR_ELP_INTERVAL, data->elp_interval);

	return 0;
}

static int set_elp_interval(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_HARDIF,
				  set_attrs_elp_interval, NULL);
}

static struct settings_data batctl_settings_elp_interval = {
	.data = &elp_interval,
	.parse = parse_elp_interval,
	.netlink_get = get_elp_interval,
	.netlink_set = set_elp_interval,
};

COMMAND_NAMED(SUBCOMMAND_HIF, elp_interval, "et", handle_sys_setting,
	      COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK,
	      &batctl_settings_elp_interval,
	      "[interval]        \tdisplay or modify elp_interval setting");
