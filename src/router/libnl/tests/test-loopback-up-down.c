#include <net/if.h>
#include <netlink/route/link.h>

int main(void)
{
	struct nl_sock *sk;
	struct rtnl_link *link, *change;
	struct nl_cache *cache;
	int err = 0;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	if ((err = rtnl_link_alloc_cache(sk, AF_UNSPEC, &cache)) < 0) {
		nl_perror(err, "Unable to allocate cache");
		goto out;
	}

	if (!(link = rtnl_link_get_by_name(cache, "lo"))) {
		fprintf(stderr, "Interface not found\n");
		err = 1;
		goto out;
	}

	/* exit if the loopback interface is already deactivated */
	err = rtnl_link_get_flags(link);
	if (!(err & IFF_UP)) {
		err = 0;
		goto out;
	}

	change = rtnl_link_alloc();
	rtnl_link_unset_flags(change, IFF_UP);

	if ((err = rtnl_link_change(sk, link, change, 0)) < 0) {
		nl_perror(err, "Unable to deactivate lo");
		goto out;
	}

	rtnl_link_set_flags(change, IFF_UP);
	if ((err = rtnl_link_change(sk, link, change, 0)) < 0) {
		nl_perror(err, "Unable to activate lo");
		goto out;
	}

	err = 0;

out:
	nl_socket_free(sk);
	return err;
}
