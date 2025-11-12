#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/flowtable.h>

static struct nftnl_flowtable *flowtable_del_parse(int argc, char *argv[])
{
	struct nftnl_flowtable *t;

	t = nftnl_flowtable_alloc();
	if (t == NULL) {
		perror("OOM");
		return NULL;
	}

	nftnl_flowtable_set_str(t, NFTNL_FLOWTABLE_TABLE, argv[2]);
	nftnl_flowtable_set_str(t, NFTNL_FLOWTABLE_NAME, argv[3]);

	return t;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	struct mnl_nlmsg_batch *batch;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, flowtable_seq;
	struct nftnl_flowtable *t;
	int ret, family;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <family> <table> <flowtable>\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}

	if (strcmp(argv[1], "ip") == 0)
		family = NFPROTO_IPV4;
	else if (strcmp(argv[1], "ip6") == 0)
		family = NFPROTO_IPV6;
	else if (strcmp(argv[1], "inet") == 0)
		family = NFPROTO_INET;
	else if (strcmp(argv[1], "bridge") == 0)
		family = NFPROTO_BRIDGE;
	else if (strcmp(argv[1], "arp") == 0)
		family = NFPROTO_ARP;
	else {
		fprintf(stderr, "Unknown family: ip, ip6, inet, bridge, arp\n");
		exit(EXIT_FAILURE);
	}

	t = flowtable_del_parse(argc, argv);
	if (t == NULL)
		exit(EXIT_FAILURE);

	seq = time(NULL);
	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	flowtable_seq = seq;
	nlh = nftnl_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
				    NFT_MSG_DELFLOWTABLE, family,
				    NLM_F_ACK, seq++);
	nftnl_flowtable_nlmsg_build_payload(nlh, t);
	nftnl_flowtable_free(t);
	mnl_nlmsg_batch_next(batch);

	nftnl_batch_end(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}
	portid = mnl_socket_get_portid(nl);

	if (mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
			      mnl_nlmsg_batch_size(batch)) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	mnl_nlmsg_batch_stop(batch);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, flowtable_seq, portid, NULL, NULL);
		if (ret <= 0)
			break;
		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	if (ret == -1) {
		perror("error");
		exit(EXIT_FAILURE);
	}
	mnl_socket_close(nl);

	return EXIT_SUCCESS;
}
