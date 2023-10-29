/* SPDX-License-Identifier: LGPL-2.1-only */
/*
 * Copyright (c) 2003-2009 Thomas Graf <tgraf@suug.ch>
 */

#include "nl-default.h"

#include <netlink/cli/utils.h>

int main(int argc, char *argv[])
{
	int err;
	char host[256];
	struct nl_addr *a;

	if (argc < 2) {
		fprintf(stderr, "Usage: nl-util-addr <address>\n");
		return -1;
	}

	a = nl_cli_addr_parse(argv[1], AF_UNSPEC);
	err = nl_addr_resolve(a, host, sizeof(host));
	if (err != 0)
		nl_cli_fatal(err, "Unable to resolve address \"%s\": %s",
		      argv[1], nl_geterror(err));

	printf("%s\n", host);

	return 0;
}
