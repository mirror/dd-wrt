// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <getopt.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "batman_adv.h"
#include "functions.h"
#include "main.h"
#include "netlink.h"
#include "sys.h"

#define SYS_GW_MODE		"gw_mode"
#define SYS_GW_SEL		"gw_sel_class"
#define SYS_GW_BW		"gw_bandwidth"

static struct gw_data {
	uint8_t bandwidth_down_found:1;
	uint8_t bandwidth_up_found:1;
	uint8_t sel_class_found:1;
	uint8_t mode;
	uint32_t bandwidth_down;
	uint32_t bandwidth_up;
	uint32_t sel_class;
} gw_globals;

static void gw_mode_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] gw_mode [mode] [sel_class|bandwidth]\n");
	fprintf(stderr, "options:\n");
	fprintf(stderr, " \t -h print this help\n");
}

static bool is_throughput_select_class(struct state *state)
{
	char algoname[32];
	int ret;

	ret = get_algoname(state, state->mesh_ifindex, algoname,
			   sizeof(algoname));

	/* no algo name -> assume that it is a pre-B.A.T.M.A.N. V version */
	if (ret < 0)
		return false;

	if (strcmp(algoname, "BATMAN_V") == 0)
		return true;

	return false;
}
static int parse_gw_limit(char *buff)
{
	char *slash_ptr;
	bool ret;

	slash_ptr = strchr(buff, '/');
	if (slash_ptr)
		*slash_ptr = 0;

	ret = parse_throughput(buff, "download gateway speed",
				&gw_globals.bandwidth_down);
	if (!ret)
		return -EINVAL;

	gw_globals.bandwidth_down_found = 1;

	/* we also got some upload info */
	if (slash_ptr) {
		ret = parse_throughput(slash_ptr + 1, "upload gateway speed",
				       &gw_globals.bandwidth_up);
		if (!ret)
			return -EINVAL;

		gw_globals.bandwidth_up_found = 1;
	}

	return 0;
}

static int parse_gw(struct state *state, int argc, char *argv[])
{
	char buff[256];
	char *endptr;
	int ret;

	if (argc != 2 && argc != 3) {
		fprintf(stderr, "Error - incorrect number of arguments (expected 1/2)\n");
		return -EINVAL;
	}

	if (strcmp(argv[1], "client") == 0) {
		gw_globals.mode = BATADV_GW_MODE_CLIENT;
	} else if (strcmp(argv[1], "server") == 0) {
		gw_globals.mode = BATADV_GW_MODE_SERVER;
	} else if (strcmp(argv[1], "off") == 0) {
		gw_globals.mode = BATADV_GW_MODE_OFF;
	} else {
		fprintf(stderr, "Error - the supplied argument is invalid: %s\n", argv[1]);
		fprintf(stderr, "The following values are allowed:\n");
		fprintf(stderr, " * off\n");
		fprintf(stderr, " * client\n");
		fprintf(stderr, " * server\n");

		return -EINVAL;
	}

	if (argc <= 2)
		return 0;

	strncpy(buff, argv[2], sizeof(buff));
	buff[sizeof(buff) - 1] = '\0';

	switch (gw_globals.mode) {
	case  BATADV_GW_MODE_OFF:
		fprintf(stderr, "Error - unexpected argument for mode \"off\": %s\n", argv[2]);
		return -EINVAL;
	case BATADV_GW_MODE_CLIENT:
		if (is_throughput_select_class(state)) {
			if (!parse_throughput(buff, "sel_class",
					      &gw_globals.sel_class))
				return -EINVAL;
		} else {
			gw_globals.sel_class = strtoul(buff, &endptr, 0);
			if (!endptr || *endptr != '\0') {
				fprintf(stderr, "Error - unexpected argument for mode \"client\": %s\n", buff);
				return -EINVAL;
			}
		}

		gw_globals.sel_class_found = 1;
		break;
	case BATADV_GW_MODE_SERVER:
		ret = parse_gw_limit(buff);
		if (ret < 0)
			return ret;
		break;
	}

	return 0;
}

static int print_gw(struct nl_msg *msg, void *arg)
{
	static const int mandatory[] = {
		BATADV_ATTR_GW_MODE,
	};
	static const int mandatory_client[] = {
		BATADV_ATTR_ALGO_NAME,
		BATADV_ATTR_GW_SEL_CLASS,
	};
	static const int mandatory_server[] = {
		BATADV_ATTR_GW_BANDWIDTH_DOWN,
		BATADV_ATTR_GW_BANDWIDTH_UP,
	};
	struct nlattr *attrs[BATADV_ATTR_MAX + 1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	struct genlmsghdr *ghdr;
	int *result = arg;
	const char *algo;
	uint8_t gw_mode;
	uint32_t val;
	uint32_t down;
	uint32_t up;

	if (!genlmsg_valid_hdr(nlh, 0))
		return NL_OK;

	ghdr = nlmsg_data(nlh);

	if (nla_parse(attrs, BATADV_ATTR_MAX, genlmsg_attrdata(ghdr, 0),
		      genlmsg_len(ghdr), batadv_netlink_policy)) {
		return NL_OK;
	}

	/* ignore entry when attributes are missing */
	if (missing_mandatory_attrs(attrs, mandatory, ARRAY_SIZE(mandatory)))
		return NL_OK;

	gw_mode = nla_get_u8(attrs[BATADV_ATTR_GW_MODE]);
	switch (gw_mode) {
	case BATADV_GW_MODE_OFF:
		printf("off\n");
		break;
	case BATADV_GW_MODE_CLIENT:
		if (missing_mandatory_attrs(attrs, mandatory_client,
		    ARRAY_SIZE(mandatory_client)))
			return NL_OK;

		algo = nla_data(attrs[BATADV_ATTR_ALGO_NAME]);
		val = nla_get_u32(attrs[BATADV_ATTR_GW_SEL_CLASS]);

		if (strcmp(algo, "BATMAN_V") == 0)
			printf("client (selection class: %u.%01u MBit)\n",
			       val / 10, val % 10);
		else
			printf("client (selection class: %u)\n", val);
		break;
	case BATADV_GW_MODE_SERVER:
		if (missing_mandatory_attrs(attrs, mandatory_server,
		    ARRAY_SIZE(mandatory_server)))
			return NL_OK;

		down = nla_get_u32(attrs[BATADV_ATTR_GW_BANDWIDTH_DOWN]);
		up = nla_get_u32(attrs[BATADV_ATTR_GW_BANDWIDTH_UP]);

		printf("server (announced bw: %u.%01u/%u.%01u MBit)\n",
		       down / 10, down % 10, up / 10, up % 10);
		break;
	default:
		printf("unknown\n");
		break;
	}

	*result = 0;
	return NL_STOP;
}

static int get_gw(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH, NULL, print_gw);
}

static int set_attrs_gw(struct nl_msg *msg, void *arg __maybe_unused)
{
	nla_put_u8(msg, BATADV_ATTR_GW_MODE, gw_globals.mode);

	if (gw_globals.bandwidth_down_found)
		nla_put_u32(msg, BATADV_ATTR_GW_BANDWIDTH_DOWN,
			    gw_globals.bandwidth_down);

	if (gw_globals.bandwidth_up_found)
		nla_put_u32(msg, BATADV_ATTR_GW_BANDWIDTH_UP,
			    gw_globals.bandwidth_up);

	if (gw_globals.sel_class_found)
		nla_put_u32(msg, BATADV_ATTR_GW_SEL_CLASS,
			    gw_globals.sel_class);

	return 0;
}

static int set_gw(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH, set_attrs_gw,
				  NULL);
}

static int gw_read_setting(struct state *state)
{
	int res;

	res = get_gw(state);
	if (res < 0)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

static int gw_write_setting(struct state *state)
{
	int res = EXIT_FAILURE;

	res = set_gw(state);
	if (res < 0)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

static int gw_mode(struct state *state, int argc, char **argv)
{
	int optchar, res = EXIT_FAILURE;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			gw_mode_usage();
			return EXIT_SUCCESS;
		default:
			gw_mode_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc == 1)
		return gw_read_setting(state);

	check_root_or_die("batctl gw_mode");

	res = parse_gw(state, argc, argv);
	if (res < 0)
		return EXIT_FAILURE;

	return gw_write_setting(state);
}

COMMAND(SUBCOMMAND_MIF, gw_mode, "gw",
	COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK, NULL,
	"[mode]            \tdisplay or modify the gateway mode");
