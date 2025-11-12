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
#include <libnftnl/object.h>

static struct nftnl_obj *ct_helper_del_parse(int argc, char *argv[])
{
	struct nftnl_obj *t;
	uint16_t family;

	if (strcmp(argv[1], "ip") == 0)
		family = NFPROTO_IPV4;
	else if (strcmp(argv[1], "ip6") == 0)
		family = NFPROTO_IPV6;
	else if (strcmp(argv[1], "inet") == 0)
		family = NFPROTO_INET;
	else {
		fprintf(stderr, "Unknown family: ip, ip6, inet\n");
		return NULL;
	}

	t = nftnl_obj_alloc();
	if (t == NULL) {
		perror("OOM");
		return NULL;
	}

	nftnl_obj_set_str(t, NFTNL_OBJ_TABLE, argv[2]);
	nftnl_obj_set_str(t, NFTNL_OBJ_NAME, argv[3]);
	nftnl_obj_set_u32(t, NFTNL_OBJ_TYPE, NFT_OBJECT_CT_HELPER);
	nftnl_obj_set_u32(t, NFTNL_OBJ_FAMILY, family);

	return t;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, obj_seq, family;
	struct nftnl_obj *t;
	struct mnl_nlmsg_batch *batch;
	int ret;

	if (argc != 4) {
		fprintf(stderr, "%s <family> <table> <name>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	t = ct_helper_del_parse(argc, argv);
	if (t == NULL)
		exit(EXIT_FAILURE);

	seq = time(NULL);
	batch = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nftnl_batch_begin(mnl_nlmsg_batch_current(batch), seq++);
	mnl_nlmsg_batch_next(batch);

	obj_seq = seq;
	family = nftnl_obj_get_u32(t, NFTNL_OBJ_FAMILY);
	nlh = nftnl_nlmsg_build_hdr(mnl_nlmsg_batch_current(batch),
				    NFT_MSG_DELOBJ, family, NLM_F_ACK,
				    seq++);
	nftnl_obj_nlmsg_build_payload(nlh, t);
	mnl_nlmsg_batch_next(batch);
	nftnl_obj_free(t);

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
		ret = mnl_cb_run(buf, ret, obj_seq, portid, NULL, NULL);
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
