// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */


#include <unistd.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <net/if.h>
#include <linux/if_link.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <netlink/attr.h>

#include "main.h"
#include "sys.h"
#include "functions.h"
#include "debug.h"

int parse_simple_boolean(struct state *state, int argc, char *argv[])
{
	struct settings_data *settings = state->cmd->arg;
	struct simple_boolean_data *data = settings->data;
	int ret;

	if (argc != 2) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1)\n");
		return -EINVAL;
	}

	ret = parse_bool(argv[1], &data->val);
	if (ret < 0) {
		fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
		fprintf(stderr, "The following values are allowed:\n");
		fprintf(stderr, " * 0\n");
		fprintf(stderr, " * disable\n");
		fprintf(stderr, " * disabled\n");
		fprintf(stderr, " * 1\n");
		fprintf(stderr, " * enable\n");
		fprintf(stderr, " * enabled\n");

		return ret;
	}

	return 0;
}

static int sys_simple_nlerror(struct sockaddr_nl *nla __maybe_unused,
			      struct nlmsgerr *nlerr,	void *arg)
{
	int *result = arg;

	if (nlerr->error != -EOPNOTSUPP)
		fprintf(stderr, "Error received: %s\n",
			strerror(-nlerr->error));

	*result = nlerr->error;

	return NL_STOP;
}

int sys_simple_nlquery(struct state *state, enum batadv_nl_commands nl_cmd,
		       nl_recvmsg_msg_cb_t attribute_cb,
		       nl_recvmsg_msg_cb_t callback)
{
	int result;
	struct nl_msg *msg;
	int ret;

	if (!state->sock)
		return -EOPNOTSUPP;

	if (callback) {
		result = -EOPNOTSUPP;
		nl_cb_set(state->cb, NL_CB_VALID, NL_CB_CUSTOM, callback,
			  &result);
	} else {
		result = 0;
	}

	nl_cb_err(state->cb, NL_CB_CUSTOM, sys_simple_nlerror, &result);

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	genlmsg_put(msg, NL_AUTO_PID, NL_AUTO_SEQ, state->batadv_family, 0, 0,
		    nl_cmd, 1);
	nla_put_u32(msg, BATADV_ATTR_MESH_IFINDEX, state->mesh_ifindex);

	if (attribute_cb) {
		ret = attribute_cb(msg, state);
		if (ret < 0) {
			nlmsg_free(msg);
			return -ENOMEM;
		}
	}

	nl_send_auto_complete(state->sock, msg);
	nlmsg_free(msg);

	if (callback) {
		ret = nl_recvmsgs(state->sock, state->cb);
		if (ret < 0)
			return ret;
	}

	nl_wait_for_ack(state->sock);

	return result;
}

int sys_simple_print_boolean(struct nl_msg *msg, void *arg,
			     enum batadv_nl_attrs attr)
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

	if (!attrs[attr])
		return NL_OK;

	printf("%s\n", nla_get_u8(attrs[attr]) ? "enabled" : "disabled");

	*result = 0;
	return NL_STOP;
}

static void settings_usage(struct state *state)
{
	const char *default_prefixes[] = {
		"",
		NULL,
	};
	const char *meshif_prefixes[] = {
		"meshif <netdev> ",
		NULL,
	};
	const char *vlan_prefixes[] = {
		"vlan <vdev> ",
		"meshif <netdev> vid <vid> ",
		NULL,
	};
	const char *hardif_prefixes[] = {
		"hardif <netdev> ",
		NULL,
	};
	const char *linestart = "Usage:";
	const char **prefixes;
	const char **prefix;

	switch (state->cmd->type) {
	case SUBCOMMAND_MIF:
		prefixes = meshif_prefixes;
		break;
	case SUBCOMMAND_VID:
		prefixes = vlan_prefixes;
		break;
	case SUBCOMMAND_HIF:
		prefixes = hardif_prefixes;
		break;
	default:
		prefixes = default_prefixes;
		break;
	}

	for (prefix = &prefixes[0]; *prefix; prefix++) {
		fprintf(stderr, "%s batctl [options] %s%s|%s [parameters] %s\n",
			linestart, *prefix, state->cmd->name, state->cmd->abbr,
			state->cmd->usage ? state->cmd->usage : "");

		linestart = "      ";
	}

	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
}

static int sys_read_setting(struct state *state)
{
	struct settings_data *settings = state->cmd->arg;
	int res = EXIT_FAILURE;

	if (settings->netlink_get) {
		res = settings->netlink_get(state);
		if (res < 0)
			return EXIT_FAILURE;
		else
			return EXIT_SUCCESS;
	}

	return res;
}

static int sys_write_setting(struct state *state)
{
	struct settings_data *settings = state->cmd->arg;
	int res = EXIT_FAILURE;

	if (settings->netlink_set) {
		res = settings->netlink_set(state);
		if (res < 0)
			return EXIT_FAILURE;
		else
			return EXIT_SUCCESS;
	}

	return res;
}

int handle_sys_setting(struct state *state, int argc, char **argv)
{
	struct settings_data *settings = state->cmd->arg;
	int optchar, res = EXIT_FAILURE;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			settings_usage(state);
			return EXIT_SUCCESS;
		default:
			settings_usage(state);
			return EXIT_FAILURE;
		}
	}

	if (argc == 1)
		return sys_read_setting(state);

	check_root_or_die("batctl");

	if (settings->parse) {
		res = settings->parse(state, argc, argv);
		if (res < 0)
			return EXIT_FAILURE;
	}

	return sys_write_setting(state);
}
