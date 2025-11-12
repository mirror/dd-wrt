/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2012-2013 by Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include <stdlib.h>
#include <sys/socket.h>
#include <time.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/common.h>
#include <libnftnl/set.h>

#include <errno.h>
#include "internal.h"

static struct nlmsghdr *__nftnl_nlmsg_build_hdr(char *buf, uint16_t type,
						uint16_t family,
						uint16_t flags, uint32_t seq,
						uint16_t res_id)
{
	struct nlmsghdr *nlh;
	struct nfgenmsg *nfh;

	nlh = mnl_nlmsg_put_header(buf);
	nlh->nlmsg_type = type;
	nlh->nlmsg_flags = NLM_F_REQUEST | flags;
	nlh->nlmsg_seq = seq;

	nfh = mnl_nlmsg_put_extra_header(nlh, sizeof(struct nfgenmsg));
	nfh->nfgen_family = family;
	nfh->version = NFNETLINK_V0;
	nfh->res_id = htons(res_id);

	return nlh;
}

EXPORT_SYMBOL(nftnl_nlmsg_build_hdr);
struct nlmsghdr *nftnl_nlmsg_build_hdr(char *buf, uint16_t type, uint16_t family,
				       uint16_t flags, uint32_t seq)
{
	return __nftnl_nlmsg_build_hdr(buf, (NFNL_SUBSYS_NFTABLES << 8) | type,
				       family, flags, seq, 0);
}

EXPORT_SYMBOL(nftnl_parse_err_alloc);
struct nftnl_parse_err *nftnl_parse_err_alloc(void)
{
	struct nftnl_parse_err *err;

	err = calloc(1, sizeof(struct nftnl_parse_err));
	if (err == NULL)
		return NULL;

	err->error = NFTNL_PARSE_EOPNOTSUPP;

	return err;
}

EXPORT_SYMBOL(nftnl_parse_err_free);
void nftnl_parse_err_free(struct nftnl_parse_err *err)
{
	xfree(err);
}

EXPORT_SYMBOL(nftnl_parse_perror);
int nftnl_parse_perror(const char *msg, struct nftnl_parse_err *err)
{
	switch (err->error) {
	case NFTNL_PARSE_EBADINPUT:
		return fprintf(stderr, "%s: Bad input format in line %d column %d\n",
			       msg, err->line, err->column);
	case NFTNL_PARSE_EMISSINGNODE:
		return fprintf(stderr, "%s: Node \"%s\" not found\n",
			       msg, err->node_name);
	case NFTNL_PARSE_EBADTYPE:
		return fprintf(stderr, "%s: Invalid type in node \"%s\"\n",
			       msg, err->node_name);
	case NFTNL_PARSE_EOPNOTSUPP:
		return fprintf(stderr, "%s: Operation not supported\n", msg);
	default:
		return fprintf(stderr, "%s: Undefined error\n", msg);
	}
}

EXPORT_SYMBOL(nftnl_batch_begin);
struct nlmsghdr *nftnl_batch_begin(char *buf, uint32_t seq)
{
	return __nftnl_nlmsg_build_hdr(buf, NFNL_MSG_BATCH_BEGIN, AF_UNSPEC,
				       0, seq, NFNL_SUBSYS_NFTABLES);
}

EXPORT_SYMBOL(nftnl_batch_end);
struct nlmsghdr *nftnl_batch_end(char *buf, uint32_t seq)
{
	return __nftnl_nlmsg_build_hdr(buf, NFNL_MSG_BATCH_END, AF_UNSPEC,
				       0, seq, NFNL_SUBSYS_NFTABLES);
}

EXPORT_SYMBOL(nftnl_batch_is_supported);
int nftnl_batch_is_supported(void)
{
	struct mnl_socket *nl;
	struct mnl_nlmsg_batch *b;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint32_t seq = time(NULL), req_seq;
	int ret;

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL)
		return -1;

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0)
		return -1;

	b = mnl_nlmsg_batch_start(buf, sizeof(buf));

	nftnl_batch_begin(mnl_nlmsg_batch_current(b), seq++);
	mnl_nlmsg_batch_next(b);

	req_seq = seq;
	nftnl_nlmsg_build_hdr(mnl_nlmsg_batch_current(b), NFT_MSG_NEWSET,
			      AF_INET, NLM_F_ACK, seq++);
	mnl_nlmsg_batch_next(b);

	nftnl_batch_end(mnl_nlmsg_batch_current(b), seq++);
	mnl_nlmsg_batch_next(b);

	ret = mnl_socket_sendto(nl, mnl_nlmsg_batch_head(b),
				mnl_nlmsg_batch_size(b));
	if (ret < 0)
		goto err;

	mnl_nlmsg_batch_stop(b);

	ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, req_seq, mnl_socket_get_portid(nl),
				 NULL, NULL);
		if (ret <= 0)
			break;

		ret = mnl_socket_recvfrom(nl, buf, sizeof(buf));
	}
	mnl_socket_close(nl);

	/* We're sending an incomplete message to see if the kernel supports
	 * set messages in batches. EINVAL means that we sent an incomplete
	 * message with missing attributes. The kernel just ignores messages
	 * that we cannot include in the batch.
	 */
	return (ret == -1 && errno == EINVAL) ? 1 : 0;
err:
	mnl_nlmsg_batch_stop(b);
	return -1;
}
