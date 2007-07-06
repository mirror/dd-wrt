/*
 * src/nl-fib-lookup.c		FIB Route Lookup
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
	printf(
	"Usage: nl-fib-lookup [options] <addr>\n"
	"Options:\n"
	"   -t, --table <table>		Table id\n"
	"   -f, --fwmark <int>		Firewall mark\n"
	"   -s, --scope <scope>		Routing scope\n"
	"   -T, --tos <int>		Type of Service\n");
	exit(1);
}

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *result;
	struct flnl_request *request;
	struct nl_addr *addr;
	struct nl_dump_params params = {
		.dp_fd = stdout,
		.dp_type = NL_DUMP_FULL,
	};
	int table = RT_TABLE_UNSPEC, scope = RT_SCOPE_UNIVERSE;
	int tos = 0, err = 1;
	uint64_t fwmark = 0;

	if (nltool_init(argc, argv) < 0)
		return -1;

	while (1) {
		static struct option long_opts[] = {
			{"table", 1, 0, 't'},
			{"fwmark", 1, 0, 'f'},
			{"scope", 1, 0, 's'},
			{"tos", 1, 0, 'T'},
			{"help", 0, 0, 'h'},
			{0, 0, 0, 0},
		};
		int c, idx = 0;

		c = getopt_long(argc, argv, "t:f:s:T:h", long_opts, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 't':
			table = strtoul(optarg, NULL, 0);
			break;
		case 'f':
			fwmark = strtoul(optarg, NULL, 0);
			break;
		case 's':
			scope = strtoul(optarg, NULL, 0);
			break;
		case 'T':
			tos = strtoul(optarg, NULL, 0);
			break;
		default:
			print_usage();
		}
	}

	if (optind >= argc)
		print_usage();

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	addr = nl_addr_parse(argv[optind], AF_INET);
	if (!addr) {
		fprintf(stderr, "Unable to parse address \"%s\": %s\n",
			argv[optind], nl_geterror());
		goto errout;
	}

	result = flnl_result_alloc_cache();
	if (!result)
		goto errout_addr;

	request = flnl_request_alloc();
	if (!request)
		goto errout_result;

	flnl_request_set_table(request, table);
	flnl_request_set_fwmark(request, fwmark);
	flnl_request_set_scope(request, scope);
	flnl_request_set_tos(request, tos);

	err = flnl_request_set_addr(request, addr);
	nl_addr_put(addr);
	if (err < 0)
		goto errout_put;

	if (nltool_connect(nlh, NETLINK_FIB_LOOKUP) < 0)
		goto errout_put;

	err = flnl_lookup(nlh, request, result);
	if (err < 0) {
		fprintf(stderr, "Unable to lookup: %s\n", nl_geterror());
		goto errout_put;
	}

	nl_cache_dump(result, &params);

	err = 0;
errout_put:
	flnl_request_put(request);
errout_result:
	nl_cache_free(result);
errout_addr:
	nl_addr_put(addr);
errout:
	nl_close(nlh);
	nl_handle_destroy(nlh);
	return err;
}
