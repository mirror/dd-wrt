#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/pkt_sched.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/qdisc.h>
#include <netlink/route/link.h>
#include <netlink/route/sch/prio.h>

int main(int argc, char *argv[])
{
	struct rtnl_qdisc new_qdisc = RTNL_INIT_QDISC();
	struct nl_handle h = NL_INIT_HANDLE();
	struct nl_cache *lc;
	uint8_t map[] = SCH_PRIO_DEFAULT_PRIOMAP;

	nl_use_default_verbose_handlers(&h);

	if (nl_connect(&h, NETLINK_ROUTE) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	lc = rtnl_link_build_cache(&h);
	if (lc == NULL) {
		fprintf(stderr, "%s", nl_geterror());
		return -1;
	}

	rtnl_qdisc_set_ifindex_name(&new_qdisc, link_cachec, argv[1]);
	rtnl_qdisc_set_parent(&new_qdisc, TC_H_ROOT);
	rtnl_qdisc_set_kind(&new_qdisc, "prio");

	rtnl_sch_prio_set_bands(&new_qdisc, 3);
	rtnl_sch_prio_set_priomap(&new_qdisc, map, sizeof(map));

	if (rtnl_qdisc_add(&h, &new_qdisc, 0) < 0) {
		fprintf(stderr, "%s", nl_geterror());
		return -1;
	}

	nl_close(&h);

	return 0;
}
