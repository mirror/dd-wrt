/*
 * Copyright (C) 2009-2011 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <lindner_marek@yahoo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "main.h"
#include "debug.h"
#include "debugfs.h"
#include "functions.h"

void originators_usage(void)
{
	printf("Usage: batctl [options] originators \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't replace mac addresses with bat-host names\n");
	printf(" \t -w [interval] watch mode - refresh the originator table continuously\n");
	printf(" \t -t timeout interval - don't print originators not seen for x.y seconds \n");
}

void trans_local_usage(void)
{
	printf("Usage: batctl [options] translocal \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't replace mac addresses with bat-host names\n");
	printf(" \t -w [interval] watch mode - refresh the local translation table continuously\n");
}

void trans_global_usage(void)
{
	printf("Usage: batctl [options] transglobal \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't replace mac addresses with bat-host names\n");
	printf(" \t -w [interval] watch mode - refresh the global translation table continuously\n");
}

void softif_neigh_usage(void)
{
	printf("Usage: batctl [options] softif_neigh \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't replace mac addresses with bat-host names\n");
	printf(" \t -w [interval] watch mode - refresh the soft-interface neighbor table continuously\n");
}

void gateways_usage(void)
{
	printf("Usage: batctl [options] gateways \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't replace mac addresses with bat-host names\n");
	printf(" \t -w [interval] watch mode - refresh the gateway server list continuously\n");
}

int handle_debug_table(char *mesh_iface, int argc, char **argv,
		       char *file_path, void table_usage(void))
{
	int optchar, read_opt = USE_BAT_HOSTS;
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;
	float orig_timeout;
	float watch_interval = 1;
	opterr = 0;

	while ((optchar = getopt(argc, argv, "hnw:t:")) != -1) {
		switch (optchar) {
		case 'h':
			table_usage();
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
				printf("Error - provided argument of -w is not a number\n");
				return EXIT_FAILURE;
			}
			break;
		case 't':
			if (table_usage != originators_usage) {
				table_usage();
				return EXIT_FAILURE;
			}

			read_opt |= NO_OLD_ORIGS;
			if (!sscanf(optarg, "%f", &orig_timeout)) {
				printf("Error - provided argument of -t is not a number\n");
				return EXIT_FAILURE;
			}
			break;
		case '?':
			if (optopt == 't')
				printf("Error - argument -t needs a number\n");

			else if (optopt == 'w') {
				read_opt |= CLR_CONT_READ;
				break;
			}
			else
				printf("Error - unrecognised option -%c\n", optopt);

			return EXIT_FAILURE;
		default:
			table_usage();
			return EXIT_FAILURE;
		}
	}

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		printf("Error - can't mount or find debugfs\n");
		return EXIT_FAILURE;
	}

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/", mesh_iface, full_path, sizeof(full_path));
	return read_file(full_path, file_path, read_opt, orig_timeout, watch_interval);
}

static void log_usage(void)
{
	printf("Usage: batctl [options] log \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
	printf(" \t -n don't replace mac addresses with bat-host names\n");
}

int log_print(char *mesh_iface, int argc, char **argv)
{
	int optchar, res, read_opt = USE_BAT_HOSTS | LOG_MODE;
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;

	while ((optchar = getopt(argc, argv, "hn")) != -1) {
		switch (optchar) {
		case 'h':
			log_usage();
			return EXIT_SUCCESS;
		case 'n':
			read_opt &= ~USE_BAT_HOSTS;
			break;
		default:
			log_usage();
			return EXIT_FAILURE;
		}
	}

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		printf("Error - can't mount or find debugfs\n");
		return EXIT_FAILURE;
	}

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/", mesh_iface, full_path, sizeof(full_path));
	res = read_file(full_path, DEBUG_LOG, read_opt, 0, 0);

	if ((res != EXIT_SUCCESS) && (errno == ENOENT))
		printf("To read the debug log you need to compile the module with debugging enabled (see the README)\n");

	return res;
}
