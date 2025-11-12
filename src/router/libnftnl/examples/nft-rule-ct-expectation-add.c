/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2019 by Stéphane Veyret <sveyret@gmail.com>
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stddef.h>	/* for offsetof */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>

static uint16_t parse_family(char *str, const char *option)
{
	if (strcmp(str, "ip") == 0)
		return NFPROTO_IPV4;
	else if (strcmp(str, "ip6") == 0)
		return NFPROTO_IPV6;
	else if (strcmp(str, "inet") == 0)
		return NFPROTO_INET;
	else if (strcmp(str, "arp") == 0)
		return NFPROTO_INET;
	fprintf(stderr, "Unknown %s: ip, ip6, inet, arp\n", option);
	exit(EXIT_FAILURE);
}

static void add_ct_expect(struct nftnl_rule *r, const char *obj_name)
{
	struct nftnl_expr *e;

	e = nftnl_expr_alloc("objref");
	if (e == NULL) {
		perror("expr objref oom");
		exit(EXIT_FAILURE);
	}
	nftnl_expr_set_str(e, NFTNL_EXPR_OBJREF_IMM_NAME, obj_name);
	nftnl_expr_set_u32(e, NFTNL_EXPR_OBJREF_IMM_TYPE, NFT_OBJECT_CT_EXPECT);

	nftnl_rule_add_expr(r, e);
}

static struct nftnl_rule *setup_rule(uint8_t family, const char *table,
				     const char *chain, const char *handle,
				     const char *obj_name)
{
	struct nftnl_rule *r = NULL;
	uint64_t handle_num;

	r = nftnl_rule_alloc();
	if (r == NULL) {
		perror("OOM");
		exit(EXIT_FAILURE);
	}

	nftnl_rule_set_str(r, NFTNL_RULE_TABLE, table);
	nftnl_rule_set_str(r, NFTNL_RULE_CHAIN, chain);
	nftnl_rule_set_u32(r, NFTNL_RULE_FAMILY, family);

	if (handle != NULL) {
		handle_num = atoll(handle);
		nftnl_rule_set_u64(r, NFTNL_RULE_POSITION, handle_num);
	}

	add_ct_expect(r, obj_name);

	return r;
}

int main(int argc, char *argv[])
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct mnl_nlmsg_batch *batch;
	uint32_t seq = time(NULL);
	struct mnl_socket *nl;
	struct nftnl_rule *r;
	struct nlmsghdr *nlh;
	uint8_t family;
	int ret;

	if (argc < 5 || argc > 6) {
		fprintf(stderr,
			"Usage: %s <family> <table> <chain> [<handle>] <name>\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}
	family = parse_family(argv[1], "family");

	if (argc < 6)
		r = setup_rule(family, argv[2], argv[3], NULL, argv[4]);
	else
		r = setup_rule(family, argv[2], argv[3], argv[4], argv[5]);

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	nlh = nftnl_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
				    NFT_MSG_NEWRULE,
				    nftnl_rule_get_u32(r, NFTNL_RULE_FAMILY),
				    NLM_F_APPEND | NLM_F_CREATE | NLM_F_ACK,
				    seq++);
	nftnl_rule_nlmsg_build_payload(nlh, r);
	nftnl_rule_free(r);
	mnl_nlmsg_batch_next(batch);

	nftnl_batch_end(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	ret = mnl_socket_sendto(nl, mnl_nlmsg_batch_head(batch),
				mnl_nlmsg_batch_size(batch));
	if (ret == -1) {
		perror("mnl_socket_sendto");
		exit(EXIT_FAILURE);
	}

	mnl_nlmsg_batch_stop(batch);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	if (ret == -1) {
		perror("mnl_socket_recvfrom");
		exit(EXIT_FAILURE);
	}

	ret = mnl_cb_run(buf, ret, 0, mnl_socket_get_portid(nl), NULL, NULL);
	if (ret < 0) {
		perror("mnl_cb_run");
		exit(EXIT_FAILURE);
	}

	mnl_socket_close(nl);

	return EXIT_SUCCESS;
}
