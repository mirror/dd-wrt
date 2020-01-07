#include <netlink/netlink.h>
#include <netlink/route/link.h>
#include <netlink/route/link/geneve.h>

#include <linux/netlink.h>

#define IPv6 1

int main(int argc, char *argv[])
{
	struct rtnl_link *link;
	struct nl_addr *addr;
	struct nl_sock *sk;
	int err;

	sk = nl_socket_alloc();
	if ((err = nl_connect(sk, NETLINK_ROUTE)) < 0) {
		nl_perror(err, "Unable to connect socket");
		return err;
	}

	link = rtnl_link_geneve_alloc();

	rtnl_link_set_name(link, "gnv123");

	if ((err = rtnl_link_geneve_set_id(link, 123)) < 0) {
		nl_perror(err, "Unable to set GENEVE ID");
		return err;
	}

#if IPv6
	if ((err = nl_addr_parse("2001:0db8:0:f101::1/64", AF_INET6, &addr)) < 0) {
		nl_perror(err,  "Unable to parse IPv6 address");
		return err;
	}
	if ((err = rtnl_link_geneve_set_label(link, 123)) < 0) {
		nl_perror(err, "Unable to set label");
		return err;
	}

	if ((err = rtnl_link_geneve_set_udp_zero_csum6_tx(link, 1)) < 0) {
		nl_perror(err, "Unable to set skip transmitted UDP checksum");
		return err;
	}

	if ((err = rtnl_link_geneve_set_udp_zero_csum6_rx(link, 1)) < 0) {
		nl_perror(err, "Unable to set skip received UDP checksum");
		return err;
	}
#else
	if ((err = nl_addr_parse("10.4.4.4", AF_INET, &addr)) < 0) {
		nl_perror(err, "Unable to parse IP address");
		return err;
	}
#endif

	if ((err = rtnl_link_geneve_set_remote(link, addr)) < 0) {
		nl_perror(err, "Unable to set remote address");
		return err;
	}
	nl_addr_put(addr);

	if ((err = rtnl_link_geneve_set_ttl(link, 1)) < 0) {
		nl_perror(err, "Unable to set TTL");
		return err;
	}

	if ((err = rtnl_link_geneve_set_tos(link, 0)) < 0) {
		nl_perror(err, "Unable to set ToS");
		return err;
	}

	if ((err = rtnl_link_geneve_set_port(link, 5060)) < 0) {
		nl_perror(err, "Unable to set port");
		return err;
	}

	if ((err = rtnl_link_add(sk, link, NLM_F_CREATE)) < 0) {
		nl_perror(err, "Unable to add link");
		return err;
	}

	rtnl_link_put(link);
	nl_close(sk);

	return 0;
}
