#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/neighbour.h>
#include <netlink/route/link.h>

static void
print_usage(void)
{
	printf(
		"Usage:\n"                          \
		"  neigh list [{brief|detailed|stats}] [filter]\n" \
		"\n" \
		"  filter := [dev DEV] [dst ADDR] [lladdr ADDR]\n");
	exit(1);
}

static int argin = 0;
static int g_argc;
static char **g_argv;


static void
get_filter(struct rtnl_neigh *n)
{
	while (g_argc > argin) {
		if (!strcasecmp(g_argv[argin], "dev")) {
			if (g_argc > ++argin) {
				if (rtnl_neigh_set_ifindex_name(n, nl_cache_lookup(RTNL_LINK),
				    g_argv[argin]) < 0) {
					fprintf(stderr, "Link %s is unknown\n", g_argv[argin]);
					exit(1);
				}
				argin++;
			}
		} else if (!strcasecmp(g_argv[argin], "dst")) {
			if (g_argc > ++argin) {
				rtnl_neigh_set_dst_str(n, g_argv[argin]);
				argin++;
			}
		} else if (!strcasecmp(g_argv[argin], "lladdr")) {
			if (g_argc > ++argin) {
				rtnl_neigh_set_lladdr_str(n, g_argv[argin]);
				argin++;
			}
		}
	}
}

int
main(int argc, char *argv[])
{
	struct nl_cache *link_cache;
	struct nl_cache *neigh_cache;
	struct nl_handle h = NL_INIT_HANDLE();

	g_argc = argc;
	g_argv = argv;

	if (argc < 2)
		print_usage();

	nl_use_default_verbose_handlers(&h);

	if (nl_connect(&h, NETLINK_ROUTE) < 0) {
		fprintf(stderr, "%s\n", nl_geterror());
		return -1;
	}

	link_cache = rtnl_link_build_cache(&h);
	if (link_cache == NULL) {
		fprintf(stderr, "%s\n", nl_geterror());
		return -1;
	}
	nl_cache_provide(link_cache);

	neigh_cache = rtnl_neigh_build_cache(&h);
	if (neigh_cache == NULL) {
		fprintf(stderr, "%s\n", nl_geterror());
		return -1;
	}

	nl_cache_provide(neigh_cache);

	argin = 2;
	if (!strcasecmp(argv[1], "list")) {
		struct nl_dump_params params = {
			.dp_type = NL_DUMP_BRIEF
		};
		struct rtnl_neigh n = RTNL_INIT_NEIGH();
		if (argc > argin) {
			if (!strcasecmp(argv[argin], "brief")) {
				argin++;
				params.dp_type = NL_DUMP_BRIEF;
			} else if (!strcasecmp(argv[argin], "detailed")) {
				argin++;
				params.dp_type = NL_DUMP_FULL;
			} else if (!strcasecmp(argv[argin], "stats")) {
				argin++;
				params.dp_type = NL_DUMP_STATS;
			}

			get_filter(&n);
		}

		nl_cache_dump_filter(neigh_cache, stdout, &params, (struct nl_common *) &n);
	} else
		print_usage();

	nl_close(&h);

	return 0;
}
