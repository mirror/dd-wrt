/* SPDX-License-Identifier: LGPL-2.1-only */

#include "nl-default.h"

#include <netlink/route/link/xfrmi.h>

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;
	struct rtnl_link *link;
	struct nl_sock *sk;
	int err, if_index;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &link_cache);
	if (err < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if_index = rtnl_link_name2i(link_cache, "eth0");
	if (!if_index) {
		fprintf(stderr, "Unable to lookup eth0");
		return -1;
	}

	link = rtnl_link_xfrmi_alloc();
	if (!link) {
		nl_perror(err, "Unable to allocate link");
		return -1;

	}

	rtnl_link_set_name(link, "ipsec0");
	rtnl_link_xfrmi_set_link(link, if_index);
	rtnl_link_xfrmi_set_if_id(link, 16);

	err = rtnl_link_add(sk, link, NLM_F_CREATE);
	if (err < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	rtnl_link_put(link);
	nl_close(sk);
	return 0;
}
