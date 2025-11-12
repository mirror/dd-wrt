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

static int obj_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_obj *t;
	char buf[4096];
	uint32_t *type = data;

	t = nftnl_obj_alloc();
	if (t == NULL) {
		perror("OOM");
		goto err;
	}

	if (nftnl_obj_nlmsg_parse(nlh, t) < 0) {
		perror("nftnl_obj_nlmsg_parse");
		goto err_free;
	}

	nftnl_obj_snprintf(buf, sizeof(buf), t, *type, 0);
	printf("%s\n", buf);

err_free:
	nftnl_obj_free(t);
err:
	return MNL_CB_OK;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, family;
	struct nftnl_obj *t = NULL;
	int ret;
	uint32_t type = NFTNL_OUTPUT_DEFAULT;

	if (argc < 3 || argc > 5) {
		fprintf(stderr, "%s <family> <table> [<obj>]\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[1], "ip") == 0)
		family = NFPROTO_IPV4;
	else if (strcmp(argv[1], "ip6") == 0)
		family = NFPROTO_IPV6;
	else if (strcmp(argv[1], "inet") == 0)
		family = NFPROTO_INET;
	else if (strcmp(argv[1], "unspec") == 0)
		family = NFPROTO_UNSPEC;
	else {
		fprintf(stderr, "Unknown family: ip, ip6, inet, unspec");
		exit(EXIT_FAILURE);
	}

	if (argc == 3 || argc == 4) {
		t = nftnl_obj_alloc();
		if (t == NULL) {
			perror("OOM");
			exit(EXIT_FAILURE);
		}
	}

	seq = time(NULL);
        nftnl_obj_set_u32(t, NFTNL_OBJ_TYPE, NFT_OBJECT_CT_HELPER);
	if (argc < 4) {
		nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETOBJ, family,
					    NLM_F_DUMP, seq);
		if (argc == 3) {
			nftnl_obj_set_str(t, NFTNL_OBJ_TABLE, argv[2]);
			nftnl_obj_nlmsg_build_payload(nlh, t);
			nftnl_obj_free(t);
		}
	} else {
		nftnl_obj_set_str(t, NFTNL_OBJ_TABLE, argv[2]);
		nftnl_obj_set_str(t, NFTNL_OBJ_NAME, argv[3]);

		nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETOBJ, family,
					    NLM_F_ACK, seq);
		nftnl_obj_nlmsg_build_payload(nlh, t);
		nftnl_obj_free(t);
	}

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
		ret = mnl_cb_run(buf, ret, seq, portid, obj_cb, &type);
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
