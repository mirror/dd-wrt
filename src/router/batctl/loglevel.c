// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */

#include <errno.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "functions.h"
#include "main.h"
#include "sys.h"

#define SYS_LOG_LEVEL		"log_level"

static struct log_level_data {
	uint32_t log_level;
} log_level_globals;

static void log_level_usage(void)
{
	fprintf(stderr, "Usage: batctl [options] loglevel [parameters] [level[ level[ level]]...]\n");
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, "levels:\n");
	fprintf(stderr, " \t none    Debug logging is disabled\n");
	fprintf(stderr, " \t all     Print messages from all below\n");
	fprintf(stderr, " \t batman  Messages related to routing / flooding / broadcasting\n");
	fprintf(stderr, " \t routes  Messages related to route added / changed / deleted\n");
	fprintf(stderr, " \t tt      Messages related to translation table operations\n");
	fprintf(stderr, " \t bla     Messages related to bridge loop avoidance\n");
	fprintf(stderr, " \t dat     Messages related to arp snooping and distributed arp table\n");
	fprintf(stderr, " \t nc      Messages related to network coding\n");
	fprintf(stderr, " \t mcast   Messages related to multicast\n");
	fprintf(stderr, " \t tp      Messages related to throughput meter\n");
}

static int extract_log_level(struct nl_msg *msg, void *arg)
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

	if (!attrs[BATADV_ATTR_LOG_LEVEL])
		return NL_OK;

	log_level_globals.log_level = nla_get_u32(attrs[BATADV_ATTR_LOG_LEVEL]);

	*result = 0;
	return NL_STOP;
}

static int get_log_level(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_GET_MESH,
				  NULL, extract_log_level);
}

static int set_attrs_log_level(struct nl_msg *msg, void *arg __maybe_unused)
{
	nla_put_u32(msg, BATADV_ATTR_LOG_LEVEL, log_level_globals.log_level);

	return 0;
}

static int set_log_level(struct state *state)
{
	return sys_simple_nlquery(state, BATADV_CMD_SET_MESH,
				  set_attrs_log_level, NULL);
}

static int log_level_read_setting(struct state *state)
{
	int res;

	res = get_log_level(state);
	if (res < 0)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

static int log_level_write_setting(struct state *state)
{
	int res;

	res = set_log_level(state);
	if (res < 0)
		return EXIT_FAILURE;
	else
		return EXIT_SUCCESS;
}

static int loglevel(struct state *state, int argc, char **argv)
{
	int optchar, res = EXIT_FAILURE;
	int i;

	log_level_globals.log_level = 0;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			log_level_usage();
			return EXIT_SUCCESS;
		default:
			log_level_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc != 1) {
		check_root_or_die("batctl loglevel");

		for (i = 1; i < argc; i++) {
			if (strcmp(argv[i], "none") == 0) {
				log_level_globals.log_level = 0;
				break;
			} else if (strcmp(argv[i], "all") == 0) {
				log_level_globals.log_level = 255;
				break;
			} else if (strcmp(argv[i], "batman") == 0)
				log_level_globals.log_level |= BIT(0);
			else if (strcmp(argv[i], "routes") == 0)
				log_level_globals.log_level |= BIT(1);
			else if (strcmp(argv[i], "tt") == 0)
				log_level_globals.log_level |= BIT(2);
			else if (strcmp(argv[i], "bla") == 0)
				log_level_globals.log_level |= BIT(3);
			else if (strcmp(argv[i], "dat") == 0)
				log_level_globals.log_level |= BIT(4);
			else if (strcmp(argv[i], "nc") == 0)
				log_level_globals.log_level |= BIT(5);
			else if (strcmp(argv[i], "mcast") == 0)
				log_level_globals.log_level |= BIT(6);
			else if (strcmp(argv[i], "tp") == 0)
				log_level_globals.log_level |= BIT(7);
			else {
				log_level_usage();
				return EXIT_FAILURE;
			}
		}

		return log_level_write_setting(state);
	}

	res = log_level_read_setting(state);
	if (res != EXIT_SUCCESS)
		return res;

	printf("[%c] %s (%s)\n", (!log_level_globals.log_level) ? 'x' : ' ',
	       "all debug output disabled", "none");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(0)) ? 'x' : ' ',
	       "messages related to routing / flooding / broadcasting",
	       "batman");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(1)) ? 'x' : ' ',
	       "messages related to route added / changed / deleted", "routes");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(2)) ? 'x' : ' ',
	       "messages related to translation table operations", "tt");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(3)) ? 'x' : ' ',
	       "messages related to bridge loop avoidance", "bla");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(4)) ? 'x' : ' ',
	       "messages related to arp snooping and distributed arp table", "dat");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(5)) ? 'x' : ' ',
	       "messages related to network coding", "nc");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(6)) ? 'x' : ' ',
	       "messages related to multicast", "mcast");
	printf("[%c] %s (%s)\n", (log_level_globals.log_level & BIT(7)) ? 'x' : ' ',
	       "messages related to throughput meter", "tp");

	return res;
}

COMMAND(SUBCOMMAND_MIF, loglevel, "ll",
	COMMAND_FLAG_MESH_IFACE | COMMAND_FLAG_NETLINK, NULL,
	"[level]           \tdisplay or modify the log level");
