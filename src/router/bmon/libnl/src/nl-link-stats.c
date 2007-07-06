/*
 * src/nl-link-stats.c     Retrieve link statistics
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
"Usage: nl-link-stats <ifindex> [<statistic> ...]\n"
"  ifindex   := { all | number }\n"
"  statistic := { (rx|tx)_packets | (rx|tx)_bytes | (rx|tx)_errors |\n"
"                 (rx|tx)_dropped | (rx|tx)_compressed | (rx|tx)_fifo_err |\n" \
"                 rx_len_err | rx_over_err | rx_crc_err | rx_frame_err |\n"
"                 rx_missed_err | tx_abort_err | tx_carrier_err |\n"
"                 tx_hbeat_err | tx_win_err | tx_collision | multicast }\n");
	exit(1);
}

static char **gargv;
static int gargc;

static void dump_stat(struct rtnl_link *link, int id)
{
	uint64_t st = rtnl_link_get_stat(link, id);
	char buf[62];

	printf("%s.%s %" PRIu64 "\n", rtnl_link_get_name(link),
	       rtnl_link_stat2str(id, buf, sizeof(buf)), st);
}

static void dump_stats(struct nl_object *obj, void *arg)
{
	int i;
	struct rtnl_link *link = (struct rtnl_link *) obj;

	if (!strcasecmp(gargv[0], "all")) {
		for (i = 0; i < RTNL_LINK_STATS_MAX; i++)
			dump_stat(link, i);
	} else {
		for (i = 0; i < gargc; i++) {
			int id = rtnl_link_str2stat(gargv[i]);

			if (id < 0)
				fprintf(stderr, "Warning: Unknown statistic "
					"\"%s\"\n", gargv[i]);
			else
				dump_stat(link, id);
		}
	}
}

int main(int argc, char *argv[])
{
	struct nl_handle *nlh;
	struct nl_cache *link_cache;
	int err = 1;

	if (nltool_init(argc, argv) < 0)
		return -1;

	if (argc < 3 || !strcmp(argv[1], "-h"))
		print_usage();

	nlh = nl_handle_alloc_nondefault(nltool_cbset);
	if (!nlh)
		return -1;

	if (nltool_connect(nlh, NETLINK_ROUTE) < 0)
		goto errout;

	link_cache = nltool_alloc_link_cache(nlh);
	if (!link_cache)
		goto errout_close;

	gargv = &argv[2];
	gargc = argc - 2;

	if (!strcasecmp(argv[1], "all"))
		nl_cache_foreach(link_cache, dump_stats, NULL);
	else {
		int ifindex = strtoul(argv[1], NULL, 0);
		struct rtnl_link *link = rtnl_link_get(link_cache, ifindex);

		if (!link) {
			fprintf(stderr, "Could not find ifindex %d\n", ifindex);
			goto errout_link_cache;
		}

		dump_stats((struct nl_object *) link, NULL);
		rtnl_link_put(link);
	}

	err = 0;
errout_link_cache:
	nl_cache_free(link_cache);
errout_close:
	nl_close(nlh);
errout:
	nl_handle_destroy(nlh);
	return err;
}
