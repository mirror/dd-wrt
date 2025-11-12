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

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables_compat.h>

#include <libmnl/libmnl.h>

static int data_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_COMPAT_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_COMPAT_NAME:
		if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	case NFTA_COMPAT_REV:
	case NFTA_COMPAT_TYPE:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0) {
			perror("mnl_attr_validate");
			return MNL_CB_ERROR;
		}
		break;
	}
	tb[type] = attr;
	return MNL_CB_OK;
}

static int cb(const struct nlmsghdr *nlh, void *data)
{
	struct nlattr *tb[NFTA_COMPAT_MAX+1] = {};
	struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);

	if (mnl_attr_parse(nlh, sizeof(*nfg), data_attr_cb, tb) < 0)
		return MNL_CB_ERROR;

	if (tb[NFTA_COMPAT_NAME])
		printf("name=%s ", mnl_attr_get_str(tb[NFTA_COMPAT_NAME]));
	if (tb[NFTA_COMPAT_REV])
		printf("rev=%d ", ntohl(mnl_attr_get_u32(tb[NFTA_COMPAT_REV])));
	if (tb[NFTA_COMPAT_TYPE])
		printf("type=%d ", ntohl(mnl_attr_get_u32(tb[NFTA_COMPAT_REV])));

	printf("\n");

	return MNL_CB_OK;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t portid, seq, rev, type;
	int ret;

	if (argc != 4) {
		fprintf(stderr, "Usage: %s <extension_name> <type> <rev>\n",
			argv[0]);
		return EXIT_FAILURE;
	}

	if (strcmp(argv[2], "target") == 0)
		type = 1;
	else if (strcmp(argv[2], "match") == 0)
		type = 0;
	else {
		fprintf(stderr, "type should be `target' or `match'\n");
		return EXIT_FAILURE;
	}
	rev = atoi(argv[3]);

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = (NFNL_SUBSYS_NFT_COMPAT << 8) | NFNL_MSG_COMPAT_GET;
	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_ACK;
	nlh->nlmsg_seq = seq = time(NULL);

	struct nfgenmsg *nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = AF_INET;
	nfg->version = NFNETLINK_V0;
	nfg->res_id = 0;

	mnl_attr_put_strz(nlh, NFTA_COMPAT_NAME, argv[1]);
	mnl_attr_put_u32(nlh, NFTA_COMPAT_REV, htonl(rev));
	mnl_attr_put_u32(nlh, NFTA_COMPAT_TYPE, htonl(type));

	printf("requesting `%s' rev=%d type=%d\n", argv[1], rev, type);

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
		ret = mnl_cb_run(buf, ret, seq, portid, cb, NULL);
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
