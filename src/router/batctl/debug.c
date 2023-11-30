// SPDX-License-Identifier: GPL-2.0
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <mareklindner@neomailbox.ch>
 *
 * License-Filename: LICENSES/preferred/GPL-2.0
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "debug.h"
#include "functions.h"
#include "netlink.h"
#include "sys.h"

static void debug_table_usage(struct state *state)
{
	struct debug_table_data *debug_table = state->cmd->arg;

	fprintf(stderr, "Usage: batctl [options] %s|%s [parameters]\n",
		state->cmd->name, state->cmd->abbr);
	fprintf(stderr, "parameters:\n");
	fprintf(stderr, " \t -h print this help\n");
	fprintf(stderr, " \t -n don't replace mac addresses with bat-host names\n");
	fprintf(stderr, " \t -H don't show the header\n");
	fprintf(stderr, " \t -w [interval] watch mode - refresh the table continuously\n");

	if (debug_table->option_timeout_interval)
		fprintf(stderr, " \t -t timeout interval - don't print originators not seen for x.y seconds \n");

	if (debug_table->option_orig_iface)
		fprintf(stderr, " \t -i [interface] - show multiif originator table for a specific interface\n");

	if (debug_table->option_unicast_only)
		fprintf(stderr, " \t -u print unicast mac addresses only\n");

	if (debug_table->option_multicast_only)
		fprintf(stderr, " \t -m print multicast mac addresses only\n");
}

int handle_debug_table(struct state *state, int argc, char **argv)
{
	struct debug_table_data *debug_table = state->cmd->arg;
	int optchar, read_opt = USE_BAT_HOSTS;
	char *orig_iface = NULL;
	float orig_timeout = 0.0f;
	float watch_interval = 1;
	int err;

	while ((optchar = getopt(argc, argv, "hnw:t:Humi:")) != -1) {
		switch (optchar) {
		case 'h':
			debug_table_usage(state);
			return EXIT_SUCCESS;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			break;
		case 'w':
			read_opt |= CLR_CONT_READ;
			if (optarg[0] == '-') {
				optind--;
				break;
			}

			if (!sscanf(optarg, "%f", &watch_interval)) {
				fprintf(stderr, "Error - provided argument of '-%c' is not a number\n", optchar);
				return EXIT_FAILURE;
			}
			break;
		case 't':
			if (!debug_table->option_timeout_interval) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(state);
				return EXIT_FAILURE;
			}

			read_opt |= NO_OLD_ORIGS;
			if (!sscanf(optarg, "%f", &orig_timeout)) {
				fprintf(stderr, "Error - provided argument of '-%c' is not a number\n", optchar);
				return EXIT_FAILURE;
			}
			break;
		case 'H':
			read_opt |= SKIP_HEADER;
			break;
		case 'u':
			if (!debug_table->option_unicast_only) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(state);
				return EXIT_FAILURE;
			}

			read_opt |= UNICAST_ONLY;
			break;
		case 'm':
			if (!debug_table->option_multicast_only) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(state);
				return EXIT_FAILURE;
			}

			read_opt |= MULTICAST_ONLY;
			break;
		case 'i':
			if (!debug_table->option_orig_iface) {
				fprintf(stderr, "Error - unrecognised option '-%c'\n", optchar);
				debug_table_usage(state);
				return EXIT_FAILURE;
			}

			if (check_mesh_iface_ownership(state, optarg) != EXIT_SUCCESS)
				return EXIT_FAILURE;

			orig_iface = optarg;
			break;
		case '?':
			if (optopt == 't') {
				fprintf(stderr, "Error - option '-t' needs a number as argument\n");
			} else if (optopt == 'i') {
				fprintf(stderr, "Error - option '-i' needs an interface as argument\n");
			} else if (optopt == 'w') {
				read_opt |= CLR_CONT_READ;
				break;
			}
			else
				fprintf(stderr, "Error - unrecognised option: '-%c'\n", optopt);

			return EXIT_FAILURE;
		default:
			debug_table_usage(state);
			return EXIT_FAILURE;
		}
	}

	check_root_or_die("batctl");

	if (read_opt & UNICAST_ONLY && read_opt & MULTICAST_ONLY) {
		fprintf(stderr, "Error - '-u' and '-m' are exclusive options\n");
		debug_table_usage(state);
		return EXIT_FAILURE;
	}

	err = debug_table->netlink_fn(state , orig_iface, read_opt,
				      orig_timeout, watch_interval);
	return err;
}
