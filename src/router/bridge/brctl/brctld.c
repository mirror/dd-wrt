/*
 * $Id: brctld.c,v 1.1 2005/09/28 11:53:38 seg Exp $
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
#include <netinet/in.h>
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <asm/param.h>
#include "libbridge.h"
#include "brctl.h"

char *help_message =
"addbr\t\t\t\t\tadd bridge\n"
"addif\t\t\t<device>\tadd interface to bridge\n"
"bridge\t\t\t<bridge>\tselect bridge to work in\n"
"delbr\t\t\t\t\tdelete bridge\n"
"delif\t\t\t<device>\tdelete interface from bridge\n"
"setageing\t\t<time>\t\tset ageing time\n"
"setbridgeprio\t\t<prio>\t\tset bridge priority\n"
"setfd\t\t\t<time>\t\tset bridge forward delay\n"
"setgcint\t\t<time>\t\tset garbage collection interval\n"
"sethello\t\t<time>\t\tset hello time\n"
"setmaxage\t\t<time>\t\tset max message age\n"
"setpathcost\t\t<port> <cost>\tset path cost\n"
"setportprio\t\t<port> <prio>\tset port priority\n"
"show\t\t\t\t\tshow a list of bridges\n"
"showbr\t\t\t\t\tshow bridge info\n"
"showmacs\t\t\t\tshow a list of mac addrs\n"
"stp\t\t\t<state>\t\t{dis,en}able stp\n"
"quit\t\t\t\t\texit this session\n"
"\n";

void help()
{
	fprintf(stderr, help_message);
}

struct bridge *br = NULL;

int forkaway()
{
	int f;

	f = fork();
	if (f < 0) {
		perror("fork");
		exit(-1);
	}

	return f;
}

void runchild(int sock)
{
	char hostname[128];

	if (forkaway())
		return;

	/* Hack. */
	close(0); dup(sock);
	close(1); dup(sock);
	close(2); dup(sock);

	br_init();
	gethostname(hostname, 128);

	printf("\n\n\n\n");
	printf("brctld\t\tCopyright (C) 2000 Lennert Buytenhek <buytenh@gnu.org>\n");
	printf("======================================================================");
	printf("\n\n\n\n");

	while (1) {
		char arg0[128];
		char arg1[128];
		char cmd[128];
		struct command *cmdptr;
		char line[1024];
		int numcmd;

		printf("<%s> ", hostname);
		fflush(stdout);
		line[1023] = 0;
		fgets(line, 1023, stdin);
		while (strlen(line) > 0 &&
		       (line[strlen(line)-1] == '\r' ||
			line[strlen(line)-1] == '\n'))
			line[strlen(line)-1] = 0;

		numcmd = sscanf(line, "%s %s %s", cmd, arg0, arg1);

		if (!strcmp(cmd, "help")) {
			help();
			continue;
		} else if (!strcmp(cmd, "bridge")) {
			if (numcmd != 2) {
				fprintf(stderr, "invalid number of arguments\n");
				continue;
			}

			br = br_find_bridge(arg0);
			if (br != NULL)
				printf("now using bridge %s\n\n", arg0);
			else
				printf("can't find bridge %s\n\n", arg0);
			continue;
		} else if (!strcmp(cmd, "quit")) {
			break;
		}

		if ((cmdptr = br_command_lookup(cmd)) == NULL) {
			printf("unknown command '%s'\n\n", line);
			continue;
		}

		if (cmdptr->needs_bridge_argument && br == NULL) {
			printf("this command needs a bridge\n\n");
			continue;
		}

		br_refresh();
		cmdptr->func(br, arg0, arg1);
		printf("\n");
	}

	shutdown(sock, 2);
	exit(0);
}

void sigchild(int sig)
{
	int status;

	wait3(&status, WNOHANG, NULL);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in addr;
	int sock;
	int x;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return 1;
	}

	x = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &x, sizeof(x)) < 0) {
		perror("setsockopt");
		return 1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(31338);
	if (bind(sock, &addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

	if (listen(sock, 1) < 0) {
		perror("listen");
		return 1;
	}

	if (forkaway())
		return 0;

	setsid();
	signal(SIGCHLD, sigchild);

	while (1) {
		int len;
		int newsock;

		len = sizeof(addr);
		if ((newsock = accept(sock, &addr, &len)) < 0) {
			perror("accept");
			return 1;
		}

		runchild(newsock);
	}

	return 0;
}
