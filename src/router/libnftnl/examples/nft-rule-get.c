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
#include <libnftnl/rule.h>

static int table_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_rule *t;
	char buf[4096];
	uint32_t *type = data;

	t = nftnl_rule_alloc();
	if (t == NULL) {
		perror("OOM");
		goto err;
	}

	if (nftnl_rule_nlmsg_parse(nlh, t) < 0) {
		perror("nftnl_rule_nlmsg_parse");
		goto err_free;
	}

	nftnl_rule_snprintf(buf, sizeof(buf), t, *type, 0);
	printf("%s\n", buf);

err_free:
	nftnl_rule_free(t);
err:
	return MNL_CB_OK;
}

static struct nftnl_rule *setup_rule(uint8_t family, const char *table,
				     const char *chain, const char *handle)
{
	struct nftnl_rule *r;
	uint64_t handle_num;

	r = nftnl_rule_alloc();
	if (r == NULL)
		return NULL;

	if (table != NULL)
		nftnl_rule_set_str(r, NFTNL_RULE_TABLE, table);
	if (chain != NULL)
		nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, chain);

	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);

	if (handle != NULL) {
		handle_num = atoll(handle);
		nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, handle_num);
	}

	return r;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, type = NFTNL_OUTPUT_DEFAULT;
	const char *table = NULL, *chain = NULL;
	struct nftnl_rule *r;
	int ret, family;

	if (argc < 2 || argc > 5) {
		fprintf(stderr, "Usage: %s <family> [<table> <chain>]\n",
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
	else if (strcmp(argv[1], "unspec") == 0)
		family = NFPROTO_UNSPEC;
	else {
		fprintf(stderr, "Unknown family: ip, ip6, inet, bridge, arp, unspec\n");
		exit(EXIT_FAILURE);
	}

	/* at least [<table> <chain>] specified */
	if (argc >= 4) {
		table = argv[2];
		chain = argv[3];
	}

	seq = time(NULL);
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, family,
				    NLM_F_DUMP, seq);

	r = setup_rule(family, table, chain, NULL);
	if (!r) {
		perror("setup_rule");
		exit(EXIT_FAILURE);
	}
	nftnl_rule_nlmsg_build_payload(nlh, r);

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

	if (mnl_socket_sendto(nl, nlh, nlh->nlmsg_len) < 0) {
		perror("mnl_socket_send");
		exit(EXIT_FAILURE);
	}

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, table_cb, &type);
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
