/*
 * src/nl-qdisc-delete.c     Delete Qdiscs
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#include "utils.h"

static void print_usage(void)
{
	printf("Usage: nl-qdisc-delete <ifindex> <parent> <handle>\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct rtnl_qdisc *qdisc;
	uint32_t handle, parent;
	int err = 1;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 3 || !strcmp(argv[1], "-h"))
		print_usage();

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		goto errout;

	qdisc = rtnl_qdisc_alloc();
	if (!qdisc)
		goto errout_free_handle;

	rtnl_qdisc_set_ifindex(qdisc, strtoul(argv[1], NULL, 0));

	if (rtnl_tc_str2handle(argv[2], &parent) < 0) {
		fprintf(stderr, "%s\n", nl_geterror());
		goto errout_free_qdisc;
	}

	if (argc > 3) {
		if (rtnl_tc_str2handle(argv[3], &handle) < 0) {
			fprintf(stderr, "%s\n", nl_geterror());
			goto errout_free_qdisc;
		}

		rtnl_qdisc_set_handle(qdisc, handle);
	}

	rtnl_qdisc_set_parent(qdisc, parent);

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout_free_qdisc;

	if (rtnl_qdisc_delete(nlh, qdisc) < 0) {
		fprintf(stderr, "Unable to delete Qdisc: %s\n", nl_geterror());
		goto errout_close;
	}

	err = 0;
errout_close:
	nl_close(nlh);
errout_free_qdisc:
	rtnl_qdisc_put(qdisc);
errout_free_handle:
	nl_handle_destroy(nlh);
errout:
	return err;
}
