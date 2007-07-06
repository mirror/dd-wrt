#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/rule.h>
#include <netlink/route/link.h>

static struct nl_cache rc = RTNL_INIT_RULE_CACHE();
static struct nl_cache lc = RTNL_INIT_LINK_CACHE();

static int argin = 0;
static int g_argc;
static char **g_argv;

static void
get_filter(struct rtnl_rule *r)
{
	while (g_argc > argin) {
		if (!strcasecmp(g_argv[argin], "tos")) {
			if (g_argc > ++argin)
				rtnl_rule_set_dsfield(r, strtoul(g_argv[argin++], NULL, 0));
		} else if (!strcasecmp(g_argv[argin], "table")) {
			if (g_argc > ++argin)
				rtnl_rule_set_table(r, strtoul(g_argv[argin++], NULL, 0));
		} else {
			fprintf(stderr, "What is '%s'?\n", g_argv[argin]);
			exit(1);
		}
	}

	return;

err:
	fprintf(stderr, "%s\n", nl_geterror());
	exit(1);
}

int
main(int argc, char *argv[])
{
	struct nl_handle h = NL_INIT_HANDLE();
	struct rtnl_rule filter = RTNL_INIT_RULE();
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_BRIEF,
	};

	g_argc = argc;
	g_argv = argv;

	if (argc < 2)
		return -1;

	nl_use_default_verbose_handlers(&h);

	if (nl_connect(&h, NETLINK_ROUTE) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	if (nl_cache_update(&h, &rc) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	if (nl_cache_update(&h, &lc) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	nl_cache_provide(&lc);
	nl_cache_provide(&rc);

	argin = 2;
	if (!strcasecmp(argv[1], "list")) {
		if (!strcasecmp(argv[1], "brief")) {
			argin++;
			params.dp_type = NL_DUMP_BRIEF;
		} else if (!strcasecmp(argv[1], "full")) {
			argin++;
			params.dp_type = NL_DUMP_FULL;
		} else if (!strcasecmp(argv[1], "stats")) {
			argin++;
			params.dp_type = NL_DUMP_STATS;
		}

		get_filter(&filter);
		nl_cache_dump_filter(&rc, stdout, &params, (struct nl_common *) &filter);
	}
	nl_close(&h);

	return 0;
}
