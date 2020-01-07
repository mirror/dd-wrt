#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/vrf.h>

#include <linux/netlink.h>

int main(int argc, char *argv[])
{
	struct nl_cache *link_cache;
	struct rtnl_link *link, *link2;
	struct nl_sock *sk;
	uint32_t tb_id;
	int err;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	if (!(link = rtnl_link_vrf_alloc())) {
		fprintf(stderr, "Unable to allocate link");
		return -1;
	}

	rtnl_link_set_name(link, "vrf-red");

	if ((err = rtnl_link_vrf_set_tableid(link, 10)) < 0) {
		nl_perror(err, "Unable to set VRF table id");
		return err;
	}

	if ((err = rtnl_link_add(sk, link, NLM_F_CREATE)) < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	if ((err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &link_cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		return err;
	}

	if (!(link2 = rtnl_link_get_by_name(link_cache, "vrf-red"))) {
		fprintf(stderr, "Unable to lookup vrf-red");
		return -1;
	}

	if ((err = rtnl_link_vrf_get_tableid(link2, &tb_id)) < 0) {
		nl_perror(err, "Unable to get VRF table id");
		return err;
	}

	if (tb_id != 10) {
		fprintf(stderr, "Mismatch with VRF table id\n");
	}

	rtnl_link_put(link);
	nl_close(sk);

	return 0;
}
