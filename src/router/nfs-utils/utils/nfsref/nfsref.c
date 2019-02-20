/**
 * @file utils/nfsref/nfsref.c
 * @brief Manage NFS referrals
 */

/*
 * Copyright 2011, 2018 Oracle.  All rights reserved.
 *
 * This file is part of nfs-utils.
 *
 * nfs-utils is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2.0 as
 * published by the Free Software Foundation.
 *
 * nfs-utils is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2.0 for more details.
 *
 * You should have received a copy of the GNU General Public License
 * version 2.0 along with nfs-utils.  If not, see:
 *
 *	http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt
 */

#include <sys/types.h>
#include <sys/capability.h>
#include <sys/prctl.h>
#include <sys/stat.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>

#include <locale.h>
#include <langinfo.h>

#include "junction.h"
#include "xlog.h"
#include "nfsref.h"

/**
 * Short form command line options
 */
static const char nfsref_opts[] = "?dt:";

/**
 * Long form command line options
 */
static const struct option nfsref_longopts[] = {
	{ "debug", 0, NULL, 'd', },
	{ "help", 0, NULL, '?', },
	{ "type", 1, NULL, 't', },
	{ NULL, 0, NULL, 0, },
};

/**
 * Display program synopsis
 *
 * @param progname NUL-terminated C string containing name of program
 */
static void
nfsref_usage(const char *progname)
{
	fprintf(stderr, "Usage: %s [ -t type ] SUBCOMMAND [ ARGUMENTS ]\n\n",
		progname);

	fprintf(stderr, "SUBCOMMAND is one of:\n");
	fprintf(stderr, "\tadd        Add a new junction\n");
	fprintf(stderr, "\tremove     Remove an existing junction\n");
	fprintf(stderr, "\tlookup     Enumerate a junction\n");

	fprintf(stderr, "\nUse \"%s SUBCOMMAND -?\" for details.\n", progname);
}

/**
 * Program entry point
 *
 * @param argc count of command line arguments
 * @param argv array of NUL-terminated C strings containing command line arguments
 * @return program exit status
 */
int
main(int argc, char **argv)
{
	char *progname, *subcommand, *junct_path;
	enum nfsref_type type;
	int arg, exit_status;
	_Bool help;

	(void)setlocale(LC_ALL, "");
	(void)umask(S_IWGRP | S_IWOTH);

	exit_status = EXIT_FAILURE;

	/* Set the basename */
	if ((progname = strrchr(argv[0], '/')) != NULL)
		progname++;
	else
		progname = argv[0];

	xlog_stderr(1);
	xlog_syslog(0);
	xlog_open(progname);

	if (argc < 2) {
		nfsref_usage(progname);
		goto out;
	}

	help = false;
	type = NFSREF_TYPE_UNSPECIFIED;
	while ((arg = getopt_long(argc, argv, nfsref_opts,
			nfsref_longopts, NULL)) != -1) {
		switch (arg) {
		case 'd':
			xlog_config(D_ALL, 1);
			break;
		case 't':
			if (strcmp(optarg, "nfs-basic") == 0)
				type = NFSREF_TYPE_NFS_BASIC;
			else if (strcmp(optarg, "nfs-fedfs") == 0)
				type = NFSREF_TYPE_NFS_FEDFS;
			else {
				xlog(L_ERROR,
					"Unrecognized junction type: %s",
					optarg);
				exit(EXIT_FAILURE);
			}
			break;
		case '?':
			help = true;
		}
	}

	if (argc < optind + 1) {
		nfsref_usage(progname);
		goto out;
	}

	if (!help && geteuid() != 0) {
		xlog(L_ERROR, "Root permission is required");
		goto out;
	}

	subcommand = argv[optind];
	junct_path = argv[optind + 1];

	if (strcasecmp(subcommand, "add") == 0) {
		if (help) {
			exit_status = nfsref_add_help(progname);
			goto out;
		}
		if (argc < optind + 3) {
			xlog(L_ERROR, "Not enough positional parameters");
			nfsref_usage(progname);
			goto out;
		}
		exit_status = nfsref_add(type, junct_path, argv, optind);
		if (exit_status == EXIT_SUCCESS)
			(void)junction_flush_exports_cache();
	} else if (strcasecmp(subcommand, "remove") == 0) {
		if (help) {
			exit_status = nfsref_remove_help(progname);
			goto out;
		}
		exit_status = nfsref_remove(type, junct_path);
		if (exit_status == EXIT_SUCCESS)
			(void)junction_flush_exports_cache();
	} else if (strcasecmp(subcommand, "lookup") == 0) {
		if (help) {
			exit_status = nfsref_lookup_help(progname);
			goto out;
		}
		exit_status = nfsref_lookup(type, junct_path);
	} else {
		xlog(L_ERROR, "Unrecognized subcommand: %s", subcommand);
		nfsref_usage(progname);
	}

out:
	exit(exit_status);
}
