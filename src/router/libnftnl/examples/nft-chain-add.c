/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This software has been sponsored by Sophos Astaro <http://www.sophos.com>
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/chain.h>

static struct nftnl_chain *chain_add_parse(int argc, char *argv[])
{
	struct nftnl_chain *t;
	int hooknum = 0;

	if (argc == 6) {
		/* This is a base chain, set the hook number */
		if (strcmp(argv[4], "NF_INET_LOCAL_IN") == 0)
			hooknum = NF_INET_LOCAL_IN;
		else if (strcmp(argv[4], "NF_INET_LOCAL_OUT") == 0)
			hooknum = NF_INET_LOCAL_OUT;
		else if (strcmp(argv[4], "NF_INET_PRE_ROUTING") == 0)
			hooknum = NF_INET_PRE_ROUTING;
		else if (strcmp(argv[4], "NF_INET_POST_ROUTING") == 0)
			hooknum = NF_INET_POST_ROUTING;
		else if (strcmp(argv[4], "NF_INET_FORWARD") == 0)
			hooknum = NF_INET_FORWARD;
		else {
			fprintf(stderr, "Unknown hook: %s\n", argv[4]);
			return NULL;
		}
	}

	t = nftnl_chain_alloc();
	if (t == NULL) {
		perror("OOM");
		return NULL;
	}
	nftnl_chain_set_str(t, NFTNL_CHAIN_TABLE, argv[2]);
	nftnl_chain_set_str(t, NFTNL_CHAIN_NAME, argv[3]);
	if (argc == 6) {
		nftnl_chain_set_u32(t, NFTNL_CHAIN_HOOKNUM, hooknum);
		nftnl_chain_set_u32(t, NFTNL_CHAIN_PRIO, atoi(argv[5]));
	}

	return t;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, chain_seq;
	int ret, family;
	struct nftnl_chain *t;
	struct mnl_nlmsg_batch *batch;

	if (argc != 4 && argc != 6) {
		fprintf(stderr, "Usage: %s <family> <table> <chain> "
					  "[<hooknum> <prio>]\n",
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

	t = chain_add_parse(argc, argv);
	if (t == NULL)
		exit(EXIT_FAILURE);

	seq = time(NULL);
	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	chain_seq = seq;
	nlh = nftnl_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
				    NFT_MSG_NEWCHAIN, family,
				    NLM_F_CREATE | NLM_F_ACK, seq++);
	nftnl_chain_nlmsg_build_payload(nlh, t);
	nftnl_chain_free(t);
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
		ret = mnl_cb_run(buf, ret, chain_seq, portid, NULL, NULL);
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
