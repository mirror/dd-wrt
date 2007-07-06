/*
 * nl-list-sockets.c	Pretty-print /proc/net/netlink
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

#define PROC_NETLINK "/proc/net/netlink"

static void print_usage(void)
{
	fprintf(stderr, "Usage: nl-list-sockets [<file>]\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	FILE *fd;
	char buf[2048], p[64];

	if (argc > 1 && !strcasecmp(argv[1], "-h"))
		print_usage();

	fd = fopen(PROC_NETLINK, "r");
	if (fd == NULL) {
		perror("fopen");
		return -1;
	}

	printf("Address    Family           PID    Groups   rmem   wmem   " \
	       "CB         refcnt\n");

	while (fgets(buf, sizeof(buf), fd)) {
		unsigned long sk, cb;
		int ret, proto, pid, rmem, wmem, refcnt;
		uint32_t groups;
		
		ret = sscanf(buf, "%lx %d %d %08x %d %d %lx %d\n",
			     &sk, &proto, &pid, &groups, &rmem, &wmem,
			     &cb, &refcnt);
		if (ret != 8)
			continue;
		
		printf("0x%08lx %-16s %-6d %08x %-6d %-6d 0x%08lx %d\n",
			sk, nl_nlfamily2str(proto, p, sizeof(p)), pid,
			groups, rmem, wmem, cb, refcnt);
	}

	fclose(fd);

	return 0;
}
