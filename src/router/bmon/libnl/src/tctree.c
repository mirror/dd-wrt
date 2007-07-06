#include <stdio.h>
#include <string.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <linux/pkt_sched.h>
#include <netlink/route/link.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/class.h>
#include <netlink/route/filter.h>

static struct nl_handle nl_handle = NL_INIT_HANDLE();
static struct nl_cache *qdisc_cache, *class_cache;
static struct nl_dump_params dump_params = { .dp_type = NL_DUMP_FULL };

static void print_qdisc(struct nl_common *, void *);

static void print_filter(struct nl_common *common, void *arg)
{
	dump_params.dp_type = (int) arg;
	rtnl_filter_dump((struct rtnl_filter *) common, stdout, &dump_params);
}

static void print_class(struct nl_common *a, void *arg)
{
	struct rtnl_qdisc *leaf;
	struct rtnl_class *class = (struct rtnl_class *) a;

	dump_params.dp_prefix = (int) arg;
	rtnl_class_dump(class, stdout, &dump_params);

	if ((leaf = rtnl_class_leaf_qdisc(class, qdisc_cache)))
		print_qdisc((struct nl_common *) leaf, arg + 2);

	rtnl_class_foreach_child(class, class_cache, &print_class, arg + 2);
	rtnl_class_foreach_filter_nocache(&nl_handle, class, &print_filter, arg + 2);
}

static void print_qdisc(struct nl_common *common, void *arg)
{
	struct rtnl_qdisc *qdisc = (struct rtnl_qdisc *) common;

	dump_params.dp_prefix = (int) arg;
	rtnl_qdisc_dump(qdisc, stdout, &dump_params);
	rtnl_qdisc_foreach_child(qdisc, class_cache, &print_class, arg + 2);
	rtnl_qdisc_foreach_filter_nocache(&nl_handle, qdisc, &print_filter, arg + 2);
}

static void print_link(struct nl_common *common, void *unused)
{
	struct rtnl_link *link = (struct rtnl_link *) common;
	struct rtnl_qdisc *qdisc;

	dump_params.dp_prefix = 0;
	rtnl_link_dump(link, stdout, &dump_params);

	class_cache = rtnl_class_build_cache(&nl_handle, link->l_index);

	if ((qdisc = rtnl_qdisc_get_root(qdisc_cache, link->l_index)))
		print_qdisc((struct nl_common *) qdisc, (void *) 2);
	else if ((qdisc = rtnl_qdisc_get_by_parent(qdisc_cache,
						   link->l_index, 0)))
		print_qdisc((struct nl_common *) qdisc, (void *) 2);

	if ((qdisc = rtnl_qdisc_get_ingress(qdisc_cache, link->l_index)))
		print_qdisc((struct nl_common *) qdisc, (void *) 2);

	nl_cache_destroy_and_free(class_cache);
}

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;

	if (argc > 1) {
		if (!strcasecmp(argv[1], "brief"))
			dump_params.dp_type = NL_DUMP_BRIEF;
		else if (!strcasecmp(argv[1], "full"))
			dump_params.dp_type = NL_DUMP_FULL;
		else if (!strcasecmp(argv[1], "stats"))
			dump_params.dp_type = NL_DUMP_STATS;
	}

	nl_use_default_verbose_handlers(&nl_handle);

	if (nl_connect(&nl_handle, NETLINK_ROUTE) < 0) {
		fprintf(stderr, "%s\n", nl_geterror());
		return 1;
	}

	link_cache = rtnl_link_build_cache(&nl_handle);
	qdisc_cache = rtnl_qdisc_build_cache(&nl_handle);
	nl_cache_provide(link_cache);

	nl_cache_foreach(link_cache, &print_link, NULL);

	nl_cache_destroy_and_free(qdisc_cache);
	nl_cache_destroy_and_free(link_cache);
	nl_close(&nl_handle);
	return 0;
}
