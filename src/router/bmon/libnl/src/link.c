#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/link.h>

static void
print_usage(void)
{
	printf(
	"Usage:\n"                          \
	"  link list [{brief|detailed|stats}] [filter]\n" \
	"  link set DEV change filter\n" \
	"\n" \
	"  filter := [dev DEV] [mtu MTU] [txqlen TXQLEN] [weight WEIGHT] [link LINK]\n" \
	"            [master MASTER] [qdisc QDISC] [addr ADDR] [broadcast BRD]\n" \
	"            [{ up | down }] [{ arp | noarp }] [{ promisc | nopromisc }]\n" \
	"            [{ dynamic | nodynamic }] [{ multicast | nomulticast }]\n" \
	"            [{ trailers | notrailers }] [{ allmulticast | noallmulticast }]\n");
	exit(1);
}

static int argin = 0;
static int g_argc;
static char **g_argv;
static struct nl_cache lc = RTNL_INIT_LINK_CACHE();

static void
get_filter(struct rtnl_link *l)
{
	while (g_argc > argin) {
		if (!strcasecmp(g_argv[argin], "dev")) {
			if (g_argc > ++argin)
				if (rtnl_link_set_ifindex_name(l, &lc, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "mtu")) {
			if (g_argc > ++argin)
				rtnl_link_set_mtu(l, strtoul(g_argv[argin++], NULL, 0));
		} else if (!strcasecmp(g_argv[argin], "txqlen")) {
			if (g_argc > ++argin)
				rtnl_link_set_txqlen(l, strtoul(g_argv[argin++], NULL, 0));
		} else if (!strcasecmp(g_argv[argin], "weight")) {
			if (g_argc > ++argin)
				rtnl_link_set_weight(l, strtoul(g_argv[argin++], NULL, 0));
		} else if (!strcasecmp(g_argv[argin], "link")) {
			if (g_argc > ++argin)
				if (rtnl_link_set_link_name(l, &lc, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "master")) {
			if (g_argc > ++argin)
				if (rtnl_link_set_master_name(l, &lc, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "qdisc")) {
			if (g_argc > ++argin)
				rtnl_link_set_qdisc(l, g_argv[argin++]);
		} else if (!strcasecmp(g_argv[argin], "name")) {
			if (g_argc > ++argin)
				rtnl_link_set_name(l, g_argv[argin++]);
		} else if (!strcasecmp(g_argv[argin], "addr")) {
			if (g_argc > ++argin)
				if (rtnl_link_set_addr_str(l, g_argv[argin++]) < 0)
					goto err;
		} else if (!strcasecmp(g_argv[argin], "broadcast")) {
			if (g_argc > ++argin)
				if (rtnl_link_set_broadcast_str(l, g_argv[argin++]) < 0)
					goto err;
		}
#define MFLAG(STR, FLAG) \
	else if (!strcasecmp(g_argv[argin], STR)) { \
		rtnl_link_set_flags(l, FLAG); argin++; }
#define MNOFLAG(STR, FLAG) \
	else if (!strcasecmp(g_argv[argin], STR)) { \
		rtnl_link_unset_flags(l, FLAG); argin++; }

		MFLAG("up", IFF_UP)
		MNOFLAG("down", IFF_UP)
		MFLAG("noarp", IFF_NOARP)
		MNOFLAG("arp", IFF_NOARP)
		MFLAG("promisc", IFF_PROMISC)
		MNOFLAG("nopromisc", IFF_PROMISC)
		MFLAG("dynamic", IFF_DYNAMIC)
		MNOFLAG("nodynamic", IFF_DYNAMIC)
		MFLAG("multicast", IFF_MULTICAST)
		MNOFLAG("nomulticast", IFF_MULTICAST)
		MFLAG("allmulticast", IFF_ALLMULTI)
		MNOFLAG("noallmulticast", IFF_ALLMULTI)
#undef MFLAG
#undef MNOFLAG
		else {
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
	struct rtnl_link l = RTNL_INIT_LINK();
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_BRIEF
	};

	nl_use_default_verbose_handlers(&h);

	g_argc = argc;
	g_argv = argv;

	if (argc < 2)
		print_usage();

	if (nl_connect(&h, NETLINK_ROUTE) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	if (nl_cache_update(&h, &lc) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	nl_cache_provide(&lc);

	argin = 2;
	if (!strcasecmp(argv[1], "list")) {
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

			get_filter(&l);
		}

		nl_cache_dump_filter(&lc, stdout, &params, (struct nl_common *) &l);

	} else if (!strcasecmp(argv[1], "set")) {
		int i;
		if (argc <= (argin+2))
			print_usage();

		i = rtnl_link_name2i(&lc, argv[argin++]);
		if (RTNL_LINK_NOT_FOUND == i) {
			fprintf(stderr, "Link %s was not found\n", argv[2]);
			return 1;
		}

		if (strcasecmp(argv[argin++], "change"))
			print_usage();

		get_filter(&l);

		if (rtnl_link_change(&h, rtnl_link_get(&lc, i), &l) < 0) {
			fprintf(stderr, "%s\n", nl_geterror());
			return 1;
		}
	} else
		print_usage();

	nl_close(&h);

	return 0;
}
