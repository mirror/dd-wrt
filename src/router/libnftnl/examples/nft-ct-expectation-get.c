/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2019 by Stéphane Veyret <sveyret@gmail.com>
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/object.h>

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

static struct nftnl_obj *obj_parse(int argc, char *argv[])
{
	struct nftnl_obj *t;
	uint16_t family;

	t = nftnl_obj_alloc();
	if (t == NULL) {
		perror("OOM");
		return NULL;
	}

	family = parse_family(argv[1], "family");
	nftnl_obj_set_u32(t, NFTNL_OBJ_FAMILY, family);
	nftnl_obj_set_u32(t, NFTNL_OBJ_TYPE, NFT_OBJECT_CT_EXPECT);
	nftnl_obj_set_str(t, NFTNL_OBJ_TABLE, argv[2]);

	if (argc > 3)
		nftnl_obj_set_str(t, NFTNL_OBJ_NAME, argv[3]);

	return t;
}

static int obj_cb(const struct nlmsghdr *nlh, void *data)
{
	uint32_t *type = data;
	struct nftnl_obj *t;
	char buf[4096];

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
	struct nftnl_obj *t;
	int ret;
	uint32_t type = NFTNL_OUTPUT_DEFAULT;
	uint16_t flags = NLM_F_ACK;

	if (argc < 3 || argc > 4) {
		fprintf(stderr, "%s <family> <table> [<name>]\n", argv[0]);
		return EXIT_FAILURE;
	}

	t = obj_parse(argc, argv);
	if (t == NULL)
		exit(EXIT_FAILURE);
	family = nftnl_obj_get_u32(t, NFTNL_OBJ_FAMILY);

	seq = time(NULL);
	if (argc < 4)
		flags = NLM_F_DUMP;
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETOBJ, family, flags, seq);
	nftnl_obj_nlmsg_build_payload(nlh, t);
	nftnl_obj_free(t);

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
