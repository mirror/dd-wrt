/*
 * Copyright (C) 2009-2011 B.A.T.M.A.N. contributors:
 *
 * Andrew Lunn <andrew@lunn.ch>
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <getopt.h>
#include <string.h>

#include "main.h"
#include "vis.h"
#include "functions.h"
#include "bat-hosts.h"
#include "debug.h"
#include "debugfs.h"

#define TQ_MAX_VALUE 255

typedef void (*print_tq_t) (char *orig, char *from, const long tq);
typedef void (*print_HNA_t) (char *orig, char *from);
typedef void (*print_1st_t) (char *orig);
typedef void (*print_2nd_t) (char *orig, char *from);
typedef void (*print_header_t) (void);
typedef void (*print_footer_t) (void);

struct funcs {
	print_tq_t print_tq;
	print_HNA_t print_HNA;
	print_1st_t print_1st;
	print_2nd_t print_2nd;
	print_header_t print_header;
	print_footer_t print_footer;
};

static bool with_HNA = true;
static bool with_2nd = true;
static bool with_names = true;

static void usage(void)
{
	printf("batctl vis_data dot {-h}{--no-HNA|-H} {--no-2nd|-2} {--numbers|-n}\n");
	printf("or\n");
	printf("batctl vis_data json {-h}{--no-HNA|-H} {--no-2nd|-2} {--numbers|-n}\n");
}

static void dot_print_tq(char *orig, char *from, const long tq)
{
	int int_part = TQ_MAX_VALUE / tq;
	int frac_part = (1000 * TQ_MAX_VALUE / tq) - (int_part * 1000);

	printf("\t\"%s\" -> ",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\"%s\" [label=\"%d.%03d\"]\n",
	       get_name_by_macstr(from, (with_names ? USE_BAT_HOSTS : 0)),
	       int_part, frac_part);
}

static void dot_print_HNA(char *orig, char *from)
{
	printf("\t\"%s\" -> ",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\"%s\" [label=\"HNA\"]\n",
	       get_name_by_macstr(from, (with_names ? USE_BAT_HOSTS : 0)));
}

static void dot_print_1st(char *orig)
{
	printf("\tsubgraph \"cluster_%s\" {\n",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\t\t\"%s\" [peripheries=2]\n",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\t}\n");
}

static void dot_print_2nd(char *orig, char *from)
{
	printf("\tsubgraph \"cluster_%s\" {\n",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\t\t\"%s\" [peripheries=2]\n",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\t\t\"%s\"\n",
	       get_name_by_macstr(from, (with_names ? USE_BAT_HOSTS : 0)));
	printf("\t}\n");
}

static void dot_print_header(void)
{
	printf("digraph {\n");
}

static void dot_print_footer(void)
{
	printf("}\n");
}

const struct funcs dot_funcs = { dot_print_tq,
	dot_print_HNA,
	dot_print_1st,
	dot_print_2nd,
	dot_print_header,
	dot_print_footer
};

static void json_print_tq(char *orig, char *from, const long tq)
{
	int int_part = TQ_MAX_VALUE / tq;
	int frac_part = (1000 * TQ_MAX_VALUE / tq) - (int_part * 1000);

	printf("\t{ router : \"%s\", ",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("neighbor : \"%s\", label : \"%d.%03d\" }\n",
	       get_name_by_macstr(from, (with_names ? USE_BAT_HOSTS : 0)),
	       int_part, frac_part);
}

static void json_print_HNA(char *orig, char *from)
{
	printf("\t{ router : \"%s\", ",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
	printf("gateway : \"%s\", label : \"HNA\" }\n",
	       get_name_by_macstr(from, (with_names ? USE_BAT_HOSTS : 0)));
}

static void json_print_1st(char *orig)
{
	printf("\t{ primary : \"%s\" }\n",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
}

static void json_print_2nd(char *orig, char *from)
{
	printf("\t{ secondary : \"%s\", ",
	       get_name_by_macstr(from, (with_names ? USE_BAT_HOSTS : 0)));

	printf("of : \"%s\" }\n",
	       get_name_by_macstr(orig, (with_names ? USE_BAT_HOSTS : 0)));
}

const struct funcs json_funcs = { json_print_tq,
	json_print_HNA,
	json_print_1st,
	json_print_2nd,
	NULL,
	NULL
};

static FILE *open_vis(char *mesh_iface)
{
	char full_path[MAX_PATH+1];
	char *debugfs_mnt;

	debugfs_mnt = debugfs_mount(NULL);
	if (!debugfs_mnt) {
		printf("Error - can't mount or find debugfs\n");
		return NULL;
	}

	debugfs_make_path(DEBUG_BATIF_PATH_FMT "/" DEBUG_VIS_DATA, mesh_iface, full_path, sizeof(full_path));

	return fopen(full_path, "r");
}

static int format(char *mesh_iface, const struct funcs *funcs)
{
	size_t len = 0;
	char *line = NULL;
	char *orig, *from;
	char *duplet;
	char *line_save_ptr, *component_save_ptr;
	char *duplet_save_ptr;
	char *endptr;
	char *value;
	long tq;
	char *flag;

	FILE *fp = open_vis(mesh_iface);

	if (!fp)
		return EXIT_FAILURE;

	if (funcs->print_header)
		funcs->print_header();

	while (getline(&line, &len, fp) != -1) {
		/* First MAC address is the originator */
		orig = strtok_r(line, ",", &line_save_ptr);

		duplet_save_ptr = line_save_ptr;
		while ((duplet = strtok_r(NULL, ",", &duplet_save_ptr)) != NULL) {
			flag = strtok_r(duplet, " ", &component_save_ptr);
			if (!flag)
				continue;
			if (!strcmp(flag, "TQ")) {
				from = strtok_r(NULL, " ", &component_save_ptr);
				value = strtok_r(NULL, " ", &component_save_ptr);
				tq = strtoul(value, &endptr, 0);
				funcs->print_tq(orig, from, tq);
				continue;
			}
			if (!strcmp(flag, "HNA")) {
				/* We have an HNA record */
				if (!with_HNA)
					continue;
				from = strtok_r(NULL, " ", &component_save_ptr);
				funcs->print_HNA(orig, from);
				continue;
			}
			if (!strcmp(flag, "SEC") && with_2nd) {
				/* We found a secondary interface MAC address. */
				from = strtok_r(NULL, " ", &component_save_ptr);
				funcs->print_2nd(orig, from);
			}
			if (!strcmp(flag, "PRIMARY") && with_2nd) {
				/* We found a primary interface MAC address. */
				funcs->print_1st(orig);
			}
		}
	}

	if (funcs->print_footer)
		funcs->print_footer();

	if (line)
		free(line);
	return EXIT_SUCCESS;
}

int vis_data(char *mesh_iface, int argc, char *argv[])
{
	bool dot = false;
	bool json = false;
	int c;

	if (argc <= 1) {
		usage();
		return EXIT_FAILURE;
	}

	/* Do we know the requested format? */
	if (strcmp(argv[1], "dot") == 0)
		dot = true;
	if (strcmp(argv[1], "json") == 0)
		json = true;

	if (!dot && !json) {
		usage();
		return EXIT_FAILURE;
	}

	/* Move over the output format */
	argc--;
	argv++;

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{"no-HNA", 0, 0, 'H'},
			{"no-2nd", 0, 0, '2'},
			{"numbers", 0, 0, 'n'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv, "hH2n", long_options, &option_index);
		if (c == -1)
			break;

		switch (c) {
		case 'H':
			with_HNA = false;
			break;
		case '2':
			with_2nd = false;
			break;
		case 'n':
			with_names = false;
			break;
		case 'h':
		default:
			usage();
			return -1;
		}
	}

	if (with_names)
		bat_hosts_init(USE_BAT_HOSTS);

	if (dot)
		return format(mesh_iface, &dot_funcs);

	if (json)
		return format(mesh_iface, &json_funcs);

	return EXIT_FAILURE;
}
