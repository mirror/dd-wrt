/* SPDX-License-Identifier: LGPL-2.1-only */

#include "nl-default.h"

#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <linux/netlink.h>

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/cli/utils.h>

static int quit = 0;
static int change = 1;
static int print_ts = 0;

static struct nl_dump_params params = {
	.dp_type = NL_DUMP_LINE,
};


static void print_timestamp(FILE *fp)
{
	struct timeval tv;
	char tshort[40];
	struct tm *tm;
	struct tm tm_buf;

	gettimeofday(&tv, NULL);
	tm = localtime_r(&tv.tv_sec, &tm_buf);

	strftime(tshort, sizeof(tshort), "%Y-%m-%dT%H:%M:%S", tm);
	fprintf(fp, "[%s.%06ld] ", tshort, tv.tv_usec);
}

static void change_cb(struct nl_cache *cache, struct nl_object *obj,
		      int action, void *data)
{
	if (print_ts)
		print_timestamp(stdout);

	if (action == NL_ACT_NEW)
		printf("NEW ");
	else if (action == NL_ACT_DEL)
		printf("DEL ");
	else if (action == NL_ACT_CHANGE)
		printf("CHANGE ");

	nl_object_dump(obj, &params);
	fflush(stdout);

	change = 1;
}

static void sigint(int arg)
{
	quit = 1;
}

static void print_usage(FILE* stream, const char *name)
{
	fprintf(stream,
		"Usage: %s [OPTIONS]... <cache name>... \n"
		"\n"
		"OPTIONS\n"
		" -f, --format=TYPE      Output format { brief | details | stats }\n"
		"                        Default: brief\n"
		" -d, --dump             Dump cache content after a change.\n"
		" -i, --interval=TIME    Dump cache content after TIME seconds when there is no\n"
		"                        change; 0 to disable. Default: 1\n"
		" -I, --iter             Iterate over all address families when updating caches.\n"
		" -t, --tshort           Print a short timestamp before change messages.\n"
		" -h, --help             Show this help text.\n"
		, name);
}

int main(int argc, char *argv[])
{
	bool dump_on_change = false, dump_on_timeout = true, iter = false;
	struct nl_cache_mngr *mngr;
	int timeout = 1000, err;

	for (;;) {
		static struct option long_opts[] = {
			{ "format", required_argument, 0, 'f' },
			{ "dump", no_argument, 0, 'd' },
			{ "interval", required_argument, 0, 'i' },
			{ "iter", no_argument, 0, 'I' },
			{ "tshort", no_argument, 0, 't' },
			{ "help", 0, 0, 'h' },
			{ 0, 0, 0, 0 }
		};
		int c;

		c = getopt_long(argc, argv, "hf:di:It", long_opts, NULL);
		if (c == -1)
			break;

		switch (c) {
			char *endptr;
			long interval;

		case 'f':
			params.dp_type = nl_cli_parse_dumptype(optarg);
			break;

		case 'd':
			dump_on_change = true;
			break;

		case 'i':
			errno = 0;
			interval = strtol(optarg, &endptr, 0);
			if (interval < 0 || errno || *endptr) {
				nl_cli_fatal(EINVAL, "Invalid interval \"%s\".\n",
					     optarg);
				exit(1);
			}
			if (!interval) {
				dump_on_timeout = false;
			} else {
				timeout = interval * 1000;
			}

			break;

		case 'I':
			iter = true;
			break;

		case 't':
			print_ts = true;
			break;

		case 'h':
			print_usage(stdout, argv[0]);
			exit(0);

		case '?':
			print_usage(stderr, argv[0]);
			exit(1);
		}
	}

	err = nl_cache_mngr_alloc(NULL, NETLINK_ROUTE, NL_AUTO_PROVIDE, &mngr);
	if (err < 0)
		nl_cli_fatal(err, "Unable to allocate cache manager: %s",
			     nl_geterror(err));

	while (optind < argc) {
		struct nl_cache *cache;

		err = nl_cache_alloc_name(argv[optind], &cache);
		if (err < 0)
			nl_cli_fatal(err, "Couldn't add cache %s: %s\n",
				     argv[optind], nl_geterror(err));

		if (iter)
			nl_cache_set_flags(cache, NL_CACHE_AF_ITER);

		err = nl_cache_mngr_add_cache(mngr, cache, &change_cb, NULL);
		if (err < 0)
			nl_cli_fatal(err, "Unable to add cache %s: %s",
				     argv[optind], nl_geterror(err));

		optind++;
	}

	params.dp_fd = stdout;
	signal(SIGINT, sigint);

	while (!quit) {
		err = nl_cache_mngr_poll(mngr, timeout);
		if (err < 0 && err != -NLE_INTR)
			nl_cli_fatal(err, "Polling failed: %s", nl_geterror(err));

		if (dump_on_timeout || (dump_on_change && change)) {
			nl_cache_mngr_info(mngr, &params);
			fflush(stdout);
			change = 0;
		}
	}

	nl_cache_mngr_free(mngr);

	return 0;
}
