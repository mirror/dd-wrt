#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/route.h>
#include <netlink/route/link.h>

static struct nl_cache rc = RTNL_INIT_ROUTE_CACHE();
static struct nl_cache lc = RTNL_INIT_LINK_CACHE();

static int argin = 0;
static int g_argc;
static char **g_argv;

static void
get_filter(struct rtnl_route *r)
{
	while (g_argc > argin) {
		if (!strcasecmp(g_argv[argin], "src")) {
			if (g_argc > ++argin)
				if (rtnl_route_set_pref_src_str(r, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "via")) {
			if (g_argc > ++argin)
				if (rtnl_route_set_gateway_str(r, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "from")) {
			if (g_argc > ++argin)
				if (rtnl_route_set_src_str(r, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "tos")) {
			if (g_argc > ++argin)
				rtnl_route_set_tos(r, strtoul(g_argv[argin++], NULL, 0));
		} else if (!strcasecmp(g_argv[argin], "prio")) {
			if (g_argc > ++argin)
				rtnl_route_set_prio(r, strtoul(g_argv[argin++], NULL, 0));
		} else if (!strcasecmp(g_argv[argin], "scope")) {
			if (g_argc > ++argin)
				rtnl_route_set_prio(r, rtnl_str2scope(g_argv[argin++]));
		} else if (!strcasecmp(g_argv[argin], "dev")) {
			if (g_argc > ++argin)
				if (rtnl_route_set_oif_name(r, &lc, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "table")) {
			if (g_argc > ++argin)
				rtnl_route_set_table(r, strtoul(g_argv[argin++], NULL, 0));
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
	struct rtnl_route filter = RTNL_INIT_ROUTE();
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
	if (!strcasecmp(argv[1], "add")) {
		rtnl_route_set_scope(&filter, RT_SCOPE_UNIVERSE);
		rtnl_route_set_protocol(&filter, RTPROT_BOOT);
		rtnl_route_set_type(&filter, RTN_UNICAST);
		get_filter(&filter);

		
	} else if (!strcasecmp(argv[1], "list")) {
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
