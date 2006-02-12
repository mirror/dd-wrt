/*
 * $Id: brctl.c,v 1.1 2005/09/28 11:53:38 seg Exp $
 *
 * Copyright (C) 2000 Lennert Buytenhek
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <asm/param.h>
#include "libbridge.h"
#include "brctl.h"

char *help_message =
"commands:\n"
"\taddbr\t\t<bridge>\t\tadd bridge\n"
"\taddif\t\t<bridge> <device>\tadd interface to bridge\n"
"\tdelbr\t\t<bridge>\t\tdelete bridge\n"
"\tdelif\t\t<bridge> <device>\tdelete interface from bridge\n"
"\tshow\t\t\t\t\tshow a list of bridges\n"
"\tshowbr\t\t<bridge>\t\tshow bridge info\n"
"\tshowmacs\t<bridge>\t\tshow a list of mac addrs\n"
"\n"
"\tsetageing\t<bridge> <time>\t\tset ageing time\n"
"\tsetbridgeprio\t<bridge> <prio>\t\tset bridge priority\n"
"\tsetfd\t\t<bridge> <time>\t\tset bridge forward delay\n"
"\tsetgcint\t<bridge> <time>\t\tset garbage collection interval\n"
"\tsethello\t<bridge> <time>\t\tset hello time\n"
"\tsetmaxage\t<bridge> <time>\t\tset max message age\n"
"\tsetpathcost\t<bridge> <port> <cost>\tset path cost\n"
"\tsetportprio\t<bridge> <port> <prio>\tset port priority\n"
"\tstp\t\t<bridge> <state>\t{dis,en}able stp\n";

void help()
{
	fprintf(stderr, help_message);
}

int main(int argc, char *argv[])
{
	int argindex;
	struct bridge *br;
	struct command *cmd;

	br_init();

	if (argc < 2)
		goto help;

	if ((cmd = br_command_lookup(argv[1])) == NULL) {
		fprintf(stderr, "never heard of command [%s]\n", argv[1]);
		goto help;
	}

	argindex = 2;
	br = NULL;
	if (cmd->needs_bridge_argument) {
		if (argindex >= argc) {
			fprintf(stderr, "this option requires a bridge name as argument\n");
			return 1;
		}

		br = br_find_bridge(argv[argindex]);

		if (br == NULL) {
			fprintf(stderr, "bridge %s doesn't exist!\n", argv[argindex]);
			return 1;
		}

		argindex++;
	}

	cmd->func(br, argv[argindex], argv[argindex+1]);

	return 0;

help:
	help();
	return 1;
}
