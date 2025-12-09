/*
 * Copyright (c) 2013-2017 Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 (or any
 * later) as published by the Free Software Foundation.
 *
 * Development of this code funded by Astaro AG (http://www.astaro.com/)
 */

#include <nft.h>
#include <iface.h>
#include <nftversion.h>

#include <libmnl/libmnl.h>
#include <libnftnl/common.h>
#include <libnftnl/ruleset.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/rule.h>
#include <libnftnl/expr.h>
#include <libnftnl/set.h>
#include <libnftnl/object.h>
#include <libnftnl/flowtable.h>
#include <libnftnl/batch.h>
#include <libnftnl/udata.h>

#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_hook.h>
#include <linux/netfilter/nf_tables.h>

#include <mnl.h>
#include <cmd.h>
#include <intervals.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <utils.h>
#include <nftables.h>
#include <linux/netfilter.h>
#include <linux/netfilter_arp.h>

struct basehook {
	struct list_head list;
	const char *module_name;
	const char *hookfn;
	const char *table;
	const char *chain;
	const char *devname;
	const char *objtype;
	int family;
	int chain_family;
	uint32_t num;
	int prio;
};

struct mnl_socket *nft_mnl_socket_open(void)
{
	struct mnl_socket *nf_sock;
	int one = 1;

	nf_sock = mnl_socket_open(NETLINK_NETFILTER);
	if (!nf_sock)
		netlink_init_error();

	if (fcntl(mnl_socket_get_fd(nf_sock), F_SETFL, O_NONBLOCK))
		netlink_init_error();

	mnl_socket_setsockopt(nf_sock, NETLINK_EXT_ACK, &one, sizeof(one));

	return nf_sock;
}

uint32_t mnl_seqnum_inc(unsigned int *seqnum)
{
	return (*seqnum)++;
}

/* The largest nf_tables netlink message is the set element message, which
 * contains the NFTA_SET_ELEM_LIST_ELEMENTS attribute. This attribute is
 * a nest that describes the set elements. Given that the netlink attribute
 * length (nla_len) is 16 bits, the largest message is a bit larger than
 * 64 KBytes.
 */
#define NFT_NLMSG_MAXSIZE (UINT16_MAX + getpagesize())

static int
nft_mnl_recv(struct netlink_ctx *ctx, uint32_t portid,
	     int (*cb)(const struct nlmsghdr *nlh, void *data), void *cb_data)
{
	char buf[NFT_NLMSG_MAXSIZE];
	bool eintr = false;
	int ret;

	ret = mnl_socket_recvfrom(ctx->nft->nf_sock, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, ctx->seqnum, portid, cb, cb_data);
		if (ret == 0)
			break;
		if (ret < 0) {
			if (errno == EAGAIN) {
				ret = 0;
				break;
			}
			if (errno != EINTR)
				break;

			/* process all pending messages before reporting EINTR */
			eintr = true;
		}
		ret = mnl_socket_recvfrom(ctx->nft->nf_sock, buf, sizeof(buf));
	}
	if (eintr) {
		ret = -1;
		errno = EINTR;
	}
	return ret;
}

int
nft_mnl_talk(struct netlink_ctx *ctx, const void *data, unsigned int len,
	     int (*cb)(const struct nlmsghdr *nlh, void *data), void *cb_data)
{
	uint32_t portid = mnl_socket_get_portid(ctx->nft->nf_sock);

	if (ctx->nft->debug_mask & NFT_DEBUG_MNL)
		mnl_nlmsg_fprintf(ctx->nft->output.output_fp, data, len,
				  sizeof(struct nfgenmsg));

	if (mnl_socket_sendto(ctx->nft->nf_sock, data, len) < 0)
		return -1;

	return nft_mnl_recv(ctx, portid, cb, cb_data);
}

/*
 * Rule-set consistency check across several netlink dumps
 */
static uint32_t nft_genid;

static int genid_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nfgenmsg *nfh = mnl_nlmsg_get_payload(nlh);

	nft_genid = ntohs(nfh->res_id);

	return MNL_CB_OK;
}

uint32_t mnl_genid_get(struct netlink_ctx *ctx)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETGEN, AF_UNSPEC, 0, ctx->seqnum);
	/* Skip error checking, old kernels sets res_id field to zero. */
	nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, genid_cb, NULL);

	return nft_genid;
}

static uint16_t nft_genid_u16(uint32_t genid)
{
	return genid & 0xffff;
}

static int check_genid(const struct nlmsghdr *nlh)
{
	struct nfgenmsg *nfh = mnl_nlmsg_get_payload(nlh);

	if (nft_genid_u16(nft_genid) != ntohs(nfh->res_id)) {
		errno = EINTR;
		return -1;
	}
	return 0;
}

/*
 * Batching
 */

/* Selected batch page is 2 Mbytes long to support loading a ruleset of 3.5M
 * rules matching on source and destination address as well as input and output
 * interfaces. This is what legacy iptables supports.
 */
#define BATCH_PAGE_SIZE 2 * 1024 * 1024

struct nftnl_batch *mnl_batch_init(void)
{
	struct nftnl_batch *batch;

	batch = nftnl_batch_alloc(BATCH_PAGE_SIZE, NFT_NLMSG_MAXSIZE);
	if (batch == NULL)
		memory_allocation_error();

	return batch;
}

static void mnl_nft_batch_continue(struct nftnl_batch *batch)
{
	if (nftnl_batch_update(batch) < 0)
		memory_allocation_error();
}

uint32_t mnl_batch_begin(struct nftnl_batch *batch, uint32_t seqnum)
{
	nftnl_batch_begin(nftnl_batch_buffer(batch), seqnum);
	mnl_nft_batch_continue(batch);

	return seqnum;
}

void mnl_batch_end(struct nftnl_batch *batch, uint32_t seqnum)
{
	nftnl_batch_end(nftnl_batch_buffer(batch), seqnum);
	mnl_nft_batch_continue(batch);
}

bool mnl_batch_ready(struct nftnl_batch *batch)
{
	/* Check if the batch only contains the initial and trailing batch
	 * messages. In that case, the batch is empty.
	 */
	return nftnl_batch_buffer_len(batch) !=
	       (NLMSG_HDRLEN + sizeof(struct nfgenmsg)) * 2;
}

void mnl_batch_reset(struct nftnl_batch *batch)
{
	nftnl_batch_free(batch);
}

static void mnl_err_list_node_add(struct list_head *err_list, int error,
				  int seqnum, uint32_t offset,
				  const char *errmsg)
{
	struct mnl_err *err = xmalloc(sizeof(struct mnl_err));

	err->seqnum = seqnum;
	err->offset = offset;
	err->err = error;
	list_add_tail(&err->head, err_list);
}

void mnl_err_list_free(struct mnl_err *err)
{
	list_del(&err->head);
	free(err);
}

static void mnl_set_sndbuffer(struct netlink_ctx *ctx)
{
	struct mnl_socket *nl = ctx->nft->nf_sock;
	struct nftnl_batch *batch = ctx->batch;
	socklen_t len = sizeof(int);
	int sndnlbuffsiz = 0;
	int newbuffsiz;

	getsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_SNDBUF,
		   &sndnlbuffsiz, &len);

	newbuffsiz = nftnl_batch_iovec_len(batch) * BATCH_PAGE_SIZE;
	if (newbuffsiz <= sndnlbuffsiz)
		return;

	/* Rise sender buffer length to avoid hitting -EMSGSIZE */
	setsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_SNDBUF,
		   &newbuffsiz, sizeof(socklen_t));

	/* unpriviledged containers check for CAP_NET_ADMIN on the init_user_ns. */
	if (setsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_SNDBUFFORCE,
		       &newbuffsiz, sizeof(socklen_t)) < 0) {
		if (errno == EPERM)
			ctx->maybe_emsgsize = newbuffsiz;
	}
}

static unsigned int nlsndbufsiz;

static int mnl_set_rcvbuffer(const struct mnl_socket *nl, socklen_t bufsiz)
{
	socklen_t len = sizeof(nlsndbufsiz);
	int ret;

	if (!nlsndbufsiz) {
		getsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_RCVBUF,
			   &nlsndbufsiz, &len);
	}

	if (nlsndbufsiz >= bufsiz)
		return 0;

	ret = setsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_RCVBUFFORCE,
			 &bufsiz, sizeof(socklen_t));
	if (ret < 0) {
		/* If this doesn't work, try to reach the system wide maximum
		 * (or whatever the user requested).
		 */
		ret = setsockopt(mnl_socket_get_fd(nl), SOL_SOCKET, SO_RCVBUF,
				 &bufsiz, sizeof(socklen_t));
	}

	return ret;
}

static void mnl_nft_batch_to_msg(struct netlink_ctx *ctx, struct msghdr *msg,
				 const struct sockaddr_nl *snl,
				 struct iovec *iov, unsigned int iov_len)
{
	msg->msg_name		= (struct sockaddr_nl *)snl;
	msg->msg_namelen	= sizeof(*snl);
	msg->msg_iov		= iov;
	msg->msg_iovlen		= iov_len;

	nftnl_batch_iovec(ctx->batch, iov, iov_len);
}

static ssize_t mnl_nft_socket_sendmsg(struct netlink_ctx *ctx,
				      const struct msghdr *msg)
{
	uint32_t iov_len = msg->msg_iovlen;
	struct iovec *iov = msg->msg_iov;
	unsigned int i;

	if (ctx->nft->debug_mask & NFT_DEBUG_MNL) {
		for (i = 0; i < iov_len; i++) {
			mnl_nlmsg_fprintf(ctx->nft->output.output_fp,
					  iov[i].iov_base, iov[i].iov_len,
					  sizeof(struct nfgenmsg));
		}
	}

	return sendmsg(mnl_socket_get_fd(ctx->nft->nf_sock), msg, 0);
}

static int err_attr_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	uint16_t type;

	if (mnl_attr_type_valid(attr, NLMSGERR_ATTR_MAX) < 0)
		return MNL_CB_ERROR;

	type = mnl_attr_get_type(attr);
	switch (type) {
	case NLMSGERR_ATTR_OFFS:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			return MNL_CB_ERROR;
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int mnl_batch_extack_cb(const struct nlmsghdr *nlh, void *data)
{
	struct netlink_cb_data *cb_data = data;
	struct nlattr *tb[NLMSGERR_ATTR_MAX + 1] = {};
	const struct nlmsgerr *err = mnl_nlmsg_get_payload(nlh);
	unsigned int hlen = sizeof(*err);
	const char *msg = NULL;
	uint32_t off = 0;
	int errval;

	if (nlh->nlmsg_len < mnl_nlmsg_size(sizeof(struct nlmsgerr)))
		return MNL_CB_ERROR;

	if (err->error < 0)
		errval = -err->error;
	else
		errval = err->error;

	if (errval == 0)
		return MNL_CB_STOP;

	if (!(nlh->nlmsg_flags & NLM_F_CAPPED))
		hlen += mnl_nlmsg_get_payload_len(&err->msg);

	if (mnl_attr_parse(nlh, hlen, err_attr_cb, tb) != MNL_CB_OK)
		return MNL_CB_ERROR;

	if (tb[NLMSGERR_ATTR_OFFS])
		off = mnl_attr_get_u32(tb[NLMSGERR_ATTR_OFFS]);

	mnl_err_list_node_add(cb_data->err_list, errval,
			      nlh->nlmsg_seq, off, msg);
	return MNL_CB_ERROR;
}

#define NFT_MNL_ECHO_RCVBUFF_DEFAULT	(MNL_SOCKET_BUFFER_SIZE * 1024U)
#define NFT_MNL_ACK_MAXSIZE		((sizeof(struct nlmsghdr) + \
					  sizeof(struct nfgenmsg) + (1 << 16)) + \
					  MNL_SOCKET_BUFFER_SIZE)

int mnl_batch_talk(struct netlink_ctx *ctx, struct list_head *err_list,
		   uint32_t num_cmds)
{
	struct mnl_socket *nl = ctx->nft->nf_sock;
	int ret, fd = mnl_socket_get_fd(nl), portid = mnl_socket_get_portid(nl);
	uint32_t iov_len = nftnl_batch_iovec_len(ctx->batch);
	char rcv_buf[NFT_MNL_ACK_MAXSIZE];
	const struct sockaddr_nl snl = {
		.nl_family = AF_NETLINK
	};
	struct timeval tv = {
		.tv_sec		= 0,
		.tv_usec	= 0
	};
	struct iovec iov[iov_len];
	struct msghdr msg = {};
	unsigned int rcvbufsiz;
	fd_set readfds;
	static mnl_cb_t cb_ctl_array[NLMSG_MIN_TYPE] = {
	        [NLMSG_ERROR] = mnl_batch_extack_cb,
	};
	struct netlink_cb_data cb_data = {
		.err_list = err_list,
		.nl_ctx = ctx,
	};

	mnl_set_sndbuffer(ctx);

	mnl_nft_batch_to_msg(ctx, &msg, &snl, iov, iov_len);

	rcvbufsiz = num_cmds * 1024;
	if (nft_output_echo(&ctx->nft->output)) {
		if (rcvbufsiz < NFT_MNL_ECHO_RCVBUFF_DEFAULT)
			rcvbufsiz = NFT_MNL_ECHO_RCVBUFF_DEFAULT;
	}

	mnl_set_rcvbuffer(ctx->nft->nf_sock, rcvbufsiz);

	ret = mnl_nft_socket_sendmsg(ctx, &msg);
	if (ret == -1)
		return -1;

	/* receive and digest all the acknowledgments from the kernel. */
	while (true) {
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		ret = select(fd + 1, &readfds, NULL, NULL, &tv);
		if (ret == -1)
			return -1;

		if (!FD_ISSET(fd, &readfds))
			break;

		ret = mnl_socket_recvfrom(nl, rcv_buf, sizeof(rcv_buf));
		if (ret == -1) {
			/* Too many errors, not all errors are displayed. */
			if (errno == ENOBUFS)
				continue;

			return -1;
		}

		/* Continue on error, make sure we get all acknowledgments */
		ret = mnl_cb_run2(rcv_buf, ret, 0, portid,
				  netlink_echo_callback, &cb_data,
				  cb_ctl_array, MNL_ARRAY_SIZE(cb_ctl_array));
	}
	return 0;
}

struct mnl_nft_rule_build_ctx {
	struct netlink_linearize_ctx	*lctx;
	struct nlmsghdr			*nlh;
	struct cmd			*cmd;
};

static int mnl_nft_expr_build_cb(struct nftnl_expr *nle, void *data)
{
	struct mnl_nft_rule_build_ctx *ctx = data;
	struct nlmsghdr *nlh = ctx->nlh;
	struct cmd *cmd = ctx->cmd;
	struct nft_expr_loc *eloc;
	struct nlattr *nest;

	eloc = nft_expr_loc_find(nle, ctx->lctx);
	if (eloc)
		cmd_add_loc(cmd, nlh, eloc->loc);

	nest = mnl_attr_nest_start(nlh, NFTA_LIST_ELEM);
	nftnl_expr_build_payload(nlh, nle);
	mnl_attr_nest_end(nlh, nest);

	nftnl_rule_del_expr(nle);
	nftnl_expr_free(nle);

	return 0;
}

static void mnl_nft_rule_build_ctx_init(struct mnl_nft_rule_build_ctx *rule_ctx,
					struct nlmsghdr *nlh,
					struct cmd *cmd,
					struct netlink_linearize_ctx *lctx)
{
	memset(rule_ctx, 0, sizeof(*rule_ctx));
	rule_ctx->nlh = nlh;
	rule_ctx->cmd = cmd;
	rule_ctx->lctx = lctx;
}

int mnl_nft_rule_add(struct netlink_ctx *ctx, struct cmd *cmd,
		     unsigned int flags)
{
	struct mnl_nft_rule_build_ctx rule_ctx;
	struct netlink_linearize_ctx lctx;
	struct rule *rule = cmd->rule;
	struct handle *h = &rule->handle;
	struct nftnl_rule *nlr;
	struct nlmsghdr *nlh;
	struct nlattr *nest;

	nlr = nftnl_rule_alloc();
	if (!nlr)
		memory_allocation_error();

	nftnl_rule_set_u32(nlr, NFTNL_RULE_FAMILY, h->family);
	if (h->position.id)
		nftnl_rule_set_u64(nlr, NFTNL_RULE_POSITION, h->position.id);
	if (h->rule_id)
		nftnl_rule_set_u32(nlr, NFTNL_RULE_ID, h->rule_id);
	if (h->position_id)
		nftnl_rule_set_u32(nlr, NFTNL_RULE_POSITION_ID, h->position_id);

	netlink_linearize_init(&lctx, nlr);
	netlink_linearize_rule(ctx, rule, &lctx);
	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWRULE,
				    cmd->handle.family,
				    NLM_F_CREATE | flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &h->table.location);
	mnl_attr_put_strz(nlh, NFTA_RULE_TABLE, h->table.name);
	cmd_add_loc(cmd, nlh, &h->chain.location);

	if (h->chain_id)
		mnl_attr_put_u32(nlh, NFTA_RULE_CHAIN_ID, htonl(h->chain_id));
	else
		mnl_attr_put_strz(nlh, NFTA_RULE_CHAIN, h->chain.name);

	mnl_nft_rule_build_ctx_init(&rule_ctx, nlh, cmd, &lctx);

	nest = mnl_attr_nest_start(nlh, NFTA_RULE_EXPRESSIONS);
	nftnl_expr_foreach(nlr, mnl_nft_expr_build_cb, &rule_ctx);
	mnl_attr_nest_end(nlh, nest);

	nftnl_rule_nlmsg_build_payload(nlh, nlr);
	nftnl_rule_free(nlr);
	netlink_linearize_fini(&lctx);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_rule_replace(struct netlink_ctx *ctx, struct cmd *cmd)
{
	struct mnl_nft_rule_build_ctx rule_ctx;
	struct netlink_linearize_ctx lctx;
	struct rule *rule = cmd->rule;
	struct handle *h = &rule->handle;
	unsigned int flags = 0;
	struct nftnl_rule *nlr;
	struct nlmsghdr *nlh;
	struct nlattr *nest;

	if (nft_output_echo(&ctx->nft->output))
		flags |= NLM_F_ECHO;

	nlr = nftnl_rule_alloc();
	if (!nlr)
		memory_allocation_error();

	nftnl_rule_set_u32(nlr, NFTNL_RULE_FAMILY, h->family);

	netlink_linearize_init(&lctx, nlr);
	netlink_linearize_rule(ctx, rule, &lctx);
	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWRULE,
				    cmd->handle.family,
				    NLM_F_REPLACE | flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &h->table.location);
	mnl_attr_put_strz(nlh, NFTA_RULE_TABLE, h->table.name);
	cmd_add_loc(cmd, nlh, &h->chain.location);
	mnl_attr_put_strz(nlh, NFTA_RULE_CHAIN, h->chain.name);
	cmd_add_loc(cmd, nlh, &h->handle.location);
	mnl_attr_put_u64(nlh, NFTA_RULE_HANDLE, htobe64(h->handle.id));

	mnl_nft_rule_build_ctx_init(&rule_ctx, nlh, cmd, &lctx);

	nest = mnl_attr_nest_start(nlh, NFTA_RULE_EXPRESSIONS);
	nftnl_expr_foreach(nlr, mnl_nft_expr_build_cb, &rule_ctx);
	mnl_attr_nest_end(nlh, nest);

	nftnl_rule_nlmsg_build_payload(nlh, nlr);
	nftnl_rule_free(nlr);
	netlink_linearize_fini(&lctx);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_rule_del(struct netlink_ctx *ctx, struct cmd *cmd)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELRULE;
	struct handle *h = &cmd->handle;
	struct nftnl_rule *nlr;
	struct nlmsghdr *nlh;

	nlr = nftnl_rule_alloc();
	if (!nlr)
		memory_allocation_error();

	nftnl_rule_set_u32(nlr, NFTNL_RULE_FAMILY, h->family);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYRULE;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    msg_type,
				    nftnl_rule_get_u32(nlr, NFTNL_RULE_FAMILY),
				    0, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &h->table.location);
	mnl_attr_put_strz(nlh, NFTA_RULE_TABLE, h->table.name);
	if (h->chain.name) {
		cmd_add_loc(cmd, nlh, &h->chain.location);
		mnl_attr_put_strz(nlh, NFTA_RULE_CHAIN, h->chain.name);
	}
	if (h->handle.id) {
		cmd_add_loc(cmd, nlh, &h->handle.location);
		mnl_attr_put_u64(nlh, NFTA_RULE_HANDLE, htobe64(h->handle.id));
	}

	nftnl_rule_nlmsg_build_payload(nlh, nlr);
	nftnl_rule_free(nlr);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

/*
 * Rule
 */

static int rule_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_rule_list *nlr_list = data;
	struct nftnl_rule *r;

	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	r = nftnl_rule_alloc();
	if (r == NULL)
		memory_allocation_error();

	if (nftnl_rule_nlmsg_parse(nlh, r) < 0)
		goto err_free;

	nftnl_rule_list_add_tail(r, nlr_list);
	return MNL_CB_OK;

err_free:
	nftnl_rule_free(r);
	return MNL_CB_OK;
}

struct nftnl_rule_list *mnl_nft_rule_dump(struct netlink_ctx *ctx, int family,
					  const char *table, const char *chain,
					  uint64_t rule_handle,
					  bool dump, bool reset)
{
	uint16_t nl_flags = dump ? NLM_F_DUMP : NLM_F_ACK;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nftnl_rule_list *nlr_list;
	struct nftnl_rule *nlr = NULL;
	struct nlmsghdr *nlh;
	int msg_type, ret;

	if (reset)
		msg_type = NFT_MSG_GETRULE_RESET;
	else
		msg_type = NFT_MSG_GETRULE;

	if (table) {
		nlr = nftnl_rule_alloc();
		if (!nlr)
			memory_allocation_error();

		nftnl_rule_set_str(nlr, NFTNL_RULE_TABLE, table);
		if (chain)
			nftnl_rule_set_str(nlr, NFTNL_RULE_CHAIN, chain);
		if (rule_handle)
			nftnl_rule_set_u64(nlr, NFTNL_RULE_HANDLE, rule_handle);
	}

	nlr_list = nftnl_rule_list_alloc();
	if (nlr_list == NULL)
		memory_allocation_error();

	nlh = nftnl_nlmsg_build_hdr(buf, msg_type, family,
				    nl_flags, ctx->seqnum);
	if (nlr) {
		nftnl_rule_nlmsg_build_payload(nlh, nlr);
		nftnl_rule_free(nlr);
	}

	ret = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, rule_cb, nlr_list);
	if (ret < 0)
		goto err;

	return nlr_list;
err:
	nftnl_rule_list_free(nlr_list);
	return NULL;
}

/*
 * Chain
 */

struct nft_dev {
	const char		*ifname;
	const struct location	*location;
};

static void nft_dev_add(struct nft_dev *dev_array, const struct expr *expr, int i)
{
	unsigned int ifname_len;
	char ifname[IFNAMSIZ];

	if (expr->etype != EXPR_VALUE)
		BUG("Must be a value, not %s", expr_name(expr));

	ifname_len = div_round_up(expr->len, BITS_PER_BYTE);
	memset(ifname, 0, sizeof(ifname));

	if (ifname_len > sizeof(ifname))
		BUG("Interface length %u exceeds limit", ifname_len);

	mpz_export_data(ifname, expr->value, BYTEORDER_HOST_ENDIAN, ifname_len);

	if (strnlen(ifname, IFNAMSIZ) >= IFNAMSIZ)
		BUG("Interface length %zu exceeds limit, no NUL byte", strnlen(ifname, IFNAMSIZ));

	dev_array[i].ifname = xstrdup(ifname);
	dev_array[i].location = &expr->location;
}

static struct nft_dev *nft_dev_array(const struct expr *dev_expr, int *num_devs)
{
	struct nft_dev *dev_array;
	int i = 0, len = 1;
	struct expr *expr;

	switch (dev_expr->etype) {
	case EXPR_LIST:
		list_for_each_entry(expr, &expr_list(dev_expr)->expressions, list)
			len++;

		dev_array = xmalloc(sizeof(struct nft_dev) * len);

		list_for_each_entry(expr, &expr_list(dev_expr)->expressions, list) {
			nft_dev_add(dev_array, expr, i);
			i++;
		}
		break;
	case EXPR_VALUE:
		len++;
		dev_array = xmalloc(sizeof(struct nft_dev) * len);
		nft_dev_add(dev_array, dev_expr, i);
		i++;
		break;
	default:
		assert(0);
	}

	dev_array[i].ifname = NULL;
	*num_devs = i;

	return dev_array;
}

static void nft_dev_array_free(const struct nft_dev *dev_array)
{
	int i = 0;

	while (dev_array[i].ifname != NULL)
		free_const(dev_array[i++].ifname);

	free_const(dev_array);
}

static bool is_wildcard_str(const char *str)
{
	size_t len = strlen(str);

	if (len < 1 || str[len - 1] != '*')
		return false;
	if (len < 2 || str[len - 2] != '\\')
		return true;
	/* XXX: ignore backslash escaping for now */
	return false;
}

static void mnl_nft_attr_put_ifname(struct nlmsghdr *nlh, const char *ifname)
{
	uint16_t attr = NFTA_DEVICE_NAME;
	char pfx[IFNAMSIZ];

	if (is_wildcard_str(ifname)) {
		snprintf(pfx, IFNAMSIZ, "%s", ifname);
		pfx[strlen(pfx) - 1] = '\0';

		attr = NFTA_DEVICE_PREFIX;
		ifname = pfx;
	}
	mnl_attr_put_strz(nlh, attr, ifname);
}

static void mnl_nft_chain_devs_build(struct nlmsghdr *nlh, struct cmd *cmd)
{
	const struct expr *dev_expr = cmd->chain->dev_expr;
	const struct nft_dev *dev_array;
	struct nlattr *nest_dev;
	int i, num_devs = 0;

	dev_array = nft_dev_array(dev_expr, &num_devs);
	if (num_devs == 1 && !is_wildcard_str(dev_array[0].ifname)) {
		cmd_add_loc(cmd, nlh, dev_array[0].location);
		mnl_attr_put_strz(nlh, NFTA_HOOK_DEV, dev_array[0].ifname);
	} else {
		nest_dev = mnl_attr_nest_start(nlh, NFTA_HOOK_DEVS);
		for (i = 0; i < num_devs; i++) {
			cmd_add_loc(cmd, nlh, dev_array[i].location);
			mnl_nft_attr_put_ifname(nlh, dev_array[i].ifname);
		}
		mnl_attr_nest_end(nlh, nest_dev);
	}
	nft_dev_array_free(dev_array);
}

int mnl_nft_chain_add(struct netlink_ctx *ctx, struct cmd *cmd,
		      unsigned int flags)
{
	struct nftnl_udata_buf *udbuf;
	struct nftnl_chain *nlc;
	struct nlmsghdr *nlh;
	int priority, policy;

	nlc = nftnl_chain_alloc();
	if (nlc == NULL)
		memory_allocation_error();

	nftnl_chain_set_u32(nlc, NFTNL_CHAIN_FAMILY, cmd->handle.family);

	if (cmd->chain) {
		if (cmd->chain->flags & CHAIN_F_HW_OFFLOAD) {
			nftnl_chain_set_u32(nlc, NFTNL_CHAIN_FLAGS,
					    CHAIN_F_HW_OFFLOAD);
		}
		if (cmd->chain->comment) {
			udbuf = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
			if (!udbuf)
				memory_allocation_error();
			if (!nftnl_udata_put_strz(udbuf, NFTNL_UDATA_CHAIN_COMMENT, cmd->chain->comment))
				memory_allocation_error();
			nftnl_chain_set_data(nlc, NFTNL_CHAIN_USERDATA, nftnl_udata_buf_data(udbuf),
					     nftnl_udata_buf_len(udbuf));
			nftnl_udata_buf_free(udbuf);
		}
	}

	nftnl_chain_set_str(nlc, NFTNL_CHAIN_TABLE, cmd->handle.table.name);
	if (cmd->handle.chain.name)
		nftnl_chain_set_str(nlc, NFTNL_CHAIN_NAME, cmd->handle.chain.name);

	netlink_dump_chain(nlc, ctx);

	nftnl_chain_unset(nlc, NFTNL_CHAIN_TABLE);
	nftnl_chain_unset(nlc, NFTNL_CHAIN_NAME);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWCHAIN,
				    cmd->handle.family,
				    NLM_F_CREATE | flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_CHAIN_TABLE, cmd->handle.table.name);
	cmd_add_loc(cmd, nlh, &cmd->handle.chain.location);

	if (!cmd->chain || !(cmd->chain->flags & CHAIN_F_BINDING)) {
		mnl_attr_put_strz(nlh, NFTA_CHAIN_NAME, cmd->handle.chain.name);
	} else {
		if (cmd->handle.chain.name)
			mnl_attr_put_strz(nlh, NFTA_CHAIN_NAME,
					  cmd->handle.chain.name);

		mnl_attr_put_u32(nlh, NFTA_CHAIN_ID, htonl(cmd->handle.chain_id));
		if (cmd->chain->flags)
			nftnl_chain_set_u32(nlc, NFTNL_CHAIN_FLAGS, cmd->chain->flags);
	}

	if (cmd->chain && cmd->chain->policy) {
		mpz_export_data(&policy, cmd->chain->policy->value,
				BYTEORDER_HOST_ENDIAN, sizeof(int));
		cmd_add_loc(cmd, nlh, &cmd->chain->policy->location);
		mnl_attr_put_u32(nlh, NFTA_CHAIN_POLICY, htonl(policy));
	}

	nftnl_chain_unset(nlc, NFTNL_CHAIN_TYPE);

	nftnl_chain_nlmsg_build_payload(nlh, nlc);

	if (cmd->chain && cmd->chain->flags & CHAIN_F_BASECHAIN) {
		struct nlattr *nest = NULL;

		if (cmd->chain->type.str) {
			cmd_add_loc(cmd, nlh, &cmd->chain->type.loc);
			mnl_attr_put_strz(nlh, NFTA_CHAIN_TYPE, cmd->chain->type.str);
		}

		if (cmd->chain->type.str ||
		    (cmd->chain && cmd->chain->dev_expr))
			nest = mnl_attr_nest_start(nlh, NFTA_CHAIN_HOOK);

		if (cmd->chain->type.str) {
			mnl_attr_put_u32(nlh, NFTA_HOOK_HOOKNUM, htonl(cmd->chain->hook.num));
			mpz_export_data(&priority, cmd->chain->priority.expr->value,
					BYTEORDER_HOST_ENDIAN, sizeof(int));
			mnl_attr_put_u32(nlh, NFTA_HOOK_PRIORITY, htonl(priority));
		}

		if (cmd->chain && cmd->chain->dev_expr)
			mnl_nft_chain_devs_build(nlh, cmd);

		if (nest)
			mnl_attr_nest_end(nlh, nest);
	}

	nftnl_chain_free(nlc);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_chain_rename(struct netlink_ctx *ctx, const struct cmd *cmd,
			 const struct chain *chain)
{
	const char *name = cmd->arg;
	struct nftnl_chain *nlc;
	struct nlmsghdr *nlh;

	nlc = nftnl_chain_alloc();
	if (nlc == NULL)
		memory_allocation_error();

	nftnl_chain_set_u32(nlc, NFTNL_CHAIN_FAMILY, cmd->handle.family);
	nftnl_chain_set_str(nlc, NFTNL_CHAIN_TABLE, cmd->handle.table.name);
	nftnl_chain_set_u64(nlc, NFTNL_CHAIN_HANDLE, chain->handle.handle.id);
	nftnl_chain_set_str(nlc, NFTNL_CHAIN_NAME, name);

	netlink_dump_chain(nlc, ctx);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWCHAIN,
				    cmd->handle.family,
				    0, ctx->seqnum);
	nftnl_chain_nlmsg_build_payload(nlh, nlc);
	nftnl_chain_free(nlc);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_chain_del(struct netlink_ctx *ctx, struct cmd *cmd)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELCHAIN;
	struct nftnl_chain *nlc;
	struct nlmsghdr *nlh;

	nlc = nftnl_chain_alloc();
	if (nlc == NULL)
		memory_allocation_error();

	nftnl_chain_set_u32(nlc, NFTNL_CHAIN_FAMILY, cmd->handle.family);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYCHAIN;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    msg_type,
				    cmd->handle.family,
				    0, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_CHAIN_TABLE, cmd->handle.table.name);
	if (cmd->handle.chain.name) {
		cmd_add_loc(cmd, nlh, &cmd->handle.chain.location);
		mnl_attr_put_strz(nlh, NFTA_CHAIN_NAME, cmd->handle.chain.name);
	} else if (cmd->handle.handle.id) {
		cmd_add_loc(cmd, nlh, &cmd->handle.handle.location);
		mnl_attr_put_u64(nlh, NFTA_CHAIN_HANDLE,
				 htobe64(cmd->handle.handle.id));
	}

	if (cmd->op == CMD_DELETE &&
	    cmd->chain && cmd->chain->dev_expr) {
		struct nlattr *nest;

		nest = mnl_attr_nest_start(nlh, NFTA_CHAIN_HOOK);
		mnl_attr_put_u32(nlh, NFTA_HOOK_HOOKNUM,
				 htonl(cmd->chain->hook.num));
		mnl_nft_chain_devs_build(nlh, cmd);
		mnl_attr_nest_end(nlh, nest);
	}

	nftnl_chain_nlmsg_build_payload(nlh, nlc);
	nftnl_chain_free(nlc);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

static int chain_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_chain_list *nlc_list = data;
	struct nftnl_chain *c;

	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	c = nftnl_chain_alloc();
	if (c == NULL)
		memory_allocation_error();

	if (nftnl_chain_nlmsg_parse(nlh, c) < 0)
		goto err_free;

	nftnl_chain_list_add_tail(c, nlc_list);
	return MNL_CB_OK;

err_free:
	nftnl_chain_free(c);
	return MNL_CB_OK;
}

struct nftnl_chain_list *mnl_nft_chain_dump(struct netlink_ctx *ctx,
					    int family, const char *table,
					    const char *chain)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nftnl_chain_list *nlc_list;
	struct nftnl_chain *nlc = NULL;
	struct nlmsghdr *nlh;
	int ret;

	nlc_list = nftnl_chain_list_alloc();
	if (nlc_list == NULL)
		memory_allocation_error();

	if (table && chain) {
		nlc = nftnl_chain_alloc();
		if (!nlc)
			memory_allocation_error();

		nftnl_chain_set_str(nlc, NFTNL_CHAIN_TABLE, table);
		nftnl_chain_set_str(nlc, NFTNL_CHAIN_NAME, chain);
	}

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETCHAIN, family,
				    nlc ? NLM_F_ACK : NLM_F_DUMP, ctx->seqnum);
	if (nlc) {
		nftnl_chain_nlmsg_build_payload(nlh, nlc);
		nftnl_chain_free(nlc);
	}

	ret = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, chain_cb, nlc_list);
	if (ret < 0 && errno != ENOENT)
		goto err;

	return nlc_list;
err:
	nftnl_chain_list_free(nlc_list);
	return NULL;
}

/*
 * Table
 */
int mnl_nft_table_add(struct netlink_ctx *ctx, struct cmd *cmd,
		      unsigned int flags)
{
	struct nftnl_udata_buf *udbuf;
	struct nftnl_table *nlt;
	struct nlmsghdr *nlh;

	nlt = nftnl_table_alloc();
	if (nlt == NULL)
		memory_allocation_error();

	udbuf = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
	if (!udbuf)
		memory_allocation_error();

	nftnl_table_set_u32(nlt, NFTNL_TABLE_FAMILY, cmd->handle.family);
	if (cmd->table) {
		nftnl_table_set_u32(nlt, NFTNL_TABLE_FLAGS, cmd->table->flags);

		if (cmd->table->comment) {
			if (!nftnl_udata_put_strz(udbuf, NFTNL_UDATA_TABLE_COMMENT, cmd->table->comment))
				memory_allocation_error();
		}
	} else {
		nftnl_table_set_u32(nlt, NFTNL_TABLE_FLAGS, 0);
	}

	if (!nftnl_udata_put(udbuf, NFTNL_UDATA_TABLE_NFTVER,
			     sizeof(nftversion), nftversion) ||
	    !nftnl_udata_put(udbuf, NFTNL_UDATA_TABLE_NFTBLD,
	                     sizeof(nftbuildstamp), nftbuildstamp))
		memory_allocation_error();
	nftnl_table_set_data(nlt, NFTNL_TABLE_USERDATA,
			     nftnl_udata_buf_data(udbuf),
			     nftnl_udata_buf_len(udbuf));
	nftnl_udata_buf_free(udbuf);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWTABLE,
				    cmd->handle.family,
				    flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_TABLE_NAME, cmd->handle.table.name);
	nftnl_table_nlmsg_build_payload(nlh, nlt);
	nftnl_table_free(nlt);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_table_del(struct netlink_ctx *ctx, struct cmd *cmd)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELTABLE;
	struct nftnl_table *nlt;
	struct nlmsghdr *nlh;

	nlt = nftnl_table_alloc();
	if (nlt == NULL)
		memory_allocation_error();

	nftnl_table_set_u32(nlt, NFTNL_TABLE_FAMILY, cmd->handle.family);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYTABLE;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch), msg_type,
			            cmd->handle.family, 0, ctx->seqnum);

	if (cmd->handle.table.name) {
		cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
		mnl_attr_put_strz(nlh, NFTA_TABLE_NAME, cmd->handle.table.name);
	} else if (cmd->handle.handle.id) {
		cmd_add_loc(cmd, nlh, &cmd->handle.handle.location);
		mnl_attr_put_u64(nlh, NFTA_TABLE_HANDLE,
				 htobe64(cmd->handle.handle.id));
	}
	nftnl_table_nlmsg_build_payload(nlh, nlt);
	nftnl_table_free(nlt);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

static int table_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_table_list *nlt_list = data;
	struct nftnl_table *t;

	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	t = nftnl_table_alloc();
	if (t == NULL)
		memory_allocation_error();

	if (nftnl_table_nlmsg_parse(nlh, t) < 0)
		goto err_free;

	nftnl_table_list_add_tail(t, nlt_list);
	return MNL_CB_OK;

err_free:
	nftnl_table_free(t);
	return MNL_CB_OK;
}

struct nftnl_table_list *mnl_nft_table_dump(struct netlink_ctx *ctx,
					    int family, const char *table)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nftnl_table_list *nlt_list;
	struct nftnl_table *nlt = NULL;
	int flags = NLM_F_DUMP;
	struct nlmsghdr *nlh;
	int ret;

	nlt_list = nftnl_table_list_alloc();
	if (nlt_list == NULL)
		return NULL;

	if (table) {
		nlt = nftnl_table_alloc();
		if (!nlt)
			memory_allocation_error();

		if (family != NFPROTO_UNSPEC)
			nftnl_table_set_u32(nlt, NFTNL_TABLE_FAMILY, family);
		if (table)
			nftnl_table_set_str(nlt, NFTNL_TABLE_NAME, table);

		flags = NLM_F_ACK;
	}

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETTABLE, family,
				    flags, ctx->seqnum);
	if (nlt) {
		nftnl_table_nlmsg_build_payload(nlh, nlt);
		nftnl_table_free(nlt);
	}

	ret = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, table_cb, nlt_list);
	if (ret < 0 && errno != ENOENT)
		goto err;

	return nlt_list;
err:
	nftnl_table_list_free(nlt_list);
	return NULL;
}

static void set_key_expression(struct netlink_ctx *ctx,
				struct expr *expr, uint32_t set_flags,
				struct nftnl_udata_buf *udbuf,
				unsigned int type)
{
	struct nftnl_udata *nest1, *nest2;

	if (!expr_ops(expr)->build_udata)
		return;

	nest1 = nftnl_udata_nest_start(udbuf, type);
	nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_TYPEOF_EXPR, expr->etype);
	nest2 = nftnl_udata_nest_start(udbuf, NFTNL_UDATA_SET_TYPEOF_DATA);
	expr_ops(expr)->build_udata(udbuf, expr);
	nftnl_udata_nest_end(udbuf, nest2);
	nftnl_udata_nest_end(udbuf, nest1);
}

/*
 * Set
 */
int mnl_nft_set_add(struct netlink_ctx *ctx, struct cmd *cmd,
		    unsigned int flags)
{
	struct handle *h = &cmd->handle;
	struct nftnl_udata_buf *udbuf;
	struct set *set = cmd->set;
	struct nftnl_set *nls;
	struct nlmsghdr *nlh;
	struct stmt *stmt;
	int num_stmts = 0;

	nls = nftnl_set_alloc();
	if (!nls)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(nls, NFTNL_SET_TABLE, h->table.name);
	nftnl_set_set_str(nls, NFTNL_SET_NAME, h->set.name);
	nftnl_set_set_u32(nls, NFTNL_SET_ID, h->set_id);

	nftnl_set_set_u32(nls, NFTNL_SET_FLAGS, set->flags);
	nftnl_set_set_u32(nls, NFTNL_SET_KEY_TYPE,
			  dtype_map_to_kernel(set->key->dtype));
	nftnl_set_set_u32(nls, NFTNL_SET_KEY_LEN,
			  div_round_up(set->key->len, BITS_PER_BYTE));
	if (set_is_datamap(set->flags)) {
		nftnl_set_set_u32(nls, NFTNL_SET_DATA_TYPE,
				  dtype_map_to_kernel(set->data->dtype));
		nftnl_set_set_u32(nls, NFTNL_SET_DATA_LEN,
				  div_round_up(set->data->len, BITS_PER_BYTE));
	}
	if (set_is_objmap(set->flags))
		nftnl_set_set_u32(nls, NFTNL_SET_OBJ_TYPE, set->objtype);

	if (set->timeout)
		nftnl_set_set_u64(nls, NFTNL_SET_TIMEOUT, set->timeout);
	if (set->gc_int)
		nftnl_set_set_u32(nls, NFTNL_SET_GC_INTERVAL, set->gc_int);

	nftnl_set_set_u32(nls, NFTNL_SET_ID, set->handle.set_id);

	if (!(set->flags & NFT_SET_CONSTANT)) {
		if (set->policy != NFT_SET_POL_PERFORMANCE)
			nftnl_set_set_u32(nls, NFTNL_SET_POLICY, set->policy);

		if (set->desc.size != 0)
			nftnl_set_set_u32(nls, NFTNL_SET_DESC_SIZE,
					  set->desc.size);
	}

	udbuf = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
	if (!udbuf)
		memory_allocation_error();
	if (!nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_KEYBYTEORDER,
				 set->key->byteorder))
		memory_allocation_error();

	if (set_is_datamap(set->flags) &&
	    !nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_DATABYTEORDER,
				 set->data->byteorder))
		memory_allocation_error();

	if (set->automerge &&
	    !nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_MERGE_ELEMENTS,
				 set->automerge))
		memory_allocation_error();

	set_key_expression(ctx, set->key, set->flags, udbuf, NFTNL_UDATA_SET_KEY_TYPEOF);
	if (set->data) {
		set_key_expression(ctx, set->data, set->flags, udbuf, NFTNL_UDATA_SET_DATA_TYPEOF);
		nftnl_udata_put_u32(udbuf, NFTNL_UDATA_SET_DATA_INTERVAL,
				    !!(set->data->flags & EXPR_F_INTERVAL));
	}

	if (set->desc.field_len[0]) {
		nftnl_set_set_data(nls, NFTNL_SET_DESC_CONCAT,
				   set->desc.field_len,
				   set->desc.field_count *
				   sizeof(set->desc.field_len[0]));
	}

	if (set->comment) {
		if (!nftnl_udata_put_strz(udbuf, NFTNL_UDATA_SET_COMMENT, set->comment))
			memory_allocation_error();
	}

	nftnl_set_set_data(nls, NFTNL_SET_USERDATA, nftnl_udata_buf_data(udbuf),
			   nftnl_udata_buf_len(udbuf));
	nftnl_udata_buf_free(udbuf);

	list_for_each_entry(stmt, &set->stmt_list, list)
		num_stmts++;

	if (num_stmts == 1) {
		list_for_each_entry(stmt, &set->stmt_list, list) {
			nftnl_set_set_data(nls, NFTNL_SET_EXPR,
					   netlink_gen_stmt_stateful(stmt), 0);
			break;
		}
	} else if (num_stmts > 1) {
		list_for_each_entry(stmt, &set->stmt_list, list)
			nftnl_set_add_expr(nls, netlink_gen_stmt_stateful(stmt));
	}

	netlink_dump_set(nls, ctx);

	nftnl_set_unset(nls, NFTNL_SET_TABLE);
	nftnl_set_unset(nls, NFTNL_SET_NAME);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWSET,
				    h->family,
				    NLM_F_CREATE | flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &h->table.location);
	mnl_attr_put_strz(nlh, NFTA_SET_TABLE, h->table.name);
	cmd_add_loc(cmd, nlh, &h->set.location);
	mnl_attr_put_strz(nlh, NFTA_SET_NAME, h->set.name);

	nftnl_set_nlmsg_build_payload(nlh, nls);
	nftnl_set_free(nls);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_set_del(struct netlink_ctx *ctx, struct cmd *cmd)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELSET;
	const struct handle *h = &cmd->handle;
	struct nftnl_set *nls;
	struct nlmsghdr *nlh;

	nls = nftnl_set_alloc();
	if (!nls)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYSET;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    msg_type,
				    h->family,
				    0, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_SET_TABLE, cmd->handle.table.name);
	if (h->set.name) {
		cmd_add_loc(cmd, nlh, &cmd->handle.set.location);
		mnl_attr_put_strz(nlh, NFTA_SET_NAME, cmd->handle.set.name);
	} else if (h->handle.id) {
		cmd_add_loc(cmd, nlh, &cmd->handle.handle.location);
		mnl_attr_put_u64(nlh, NFTA_SET_HANDLE,
				 htobe64(cmd->handle.handle.id));
	}

	nftnl_set_nlmsg_build_payload(nlh, nls);
	nftnl_set_free(nls);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

struct set_cb_args {
	struct netlink_ctx *ctx;
	struct nftnl_set_list *list;
};

static int set_cb(const struct nlmsghdr *nlh, void *data)
{
	struct set_cb_args *args = data;
	struct nftnl_set_list *nls_list = args->list;
	struct nftnl_set *s;

	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	s = nftnl_set_alloc();
	if (s == NULL)
		memory_allocation_error();

	if (nftnl_set_nlmsg_parse(nlh, s) < 0)
		goto err_free;

	netlink_dump_set(s, args->ctx);

	nftnl_set_list_add_tail(s, nls_list);
	return MNL_CB_OK;

err_free:
	nftnl_set_free(s);
	return MNL_CB_OK;
}

struct nftnl_set_list *
mnl_nft_set_dump(struct netlink_ctx *ctx, int family,
		 const char *table, const char *set)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nftnl_set_list *nls_list;
	int flags = NLM_F_DUMP;
	struct nlmsghdr *nlh;
	struct nftnl_set *s;
	int ret;
	struct set_cb_args args;

	s = nftnl_set_alloc();
	if (s == NULL)
		memory_allocation_error();

	if (table != NULL)
		nftnl_set_set_str(s, NFTNL_SET_TABLE, table);
	if (set) {
		nftnl_set_set_str(s, NFTNL_SET_NAME, set);
		flags = NLM_F_ACK;
	}

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETSET, family,
				    flags, ctx->seqnum);
	nftnl_set_nlmsg_build_payload(nlh, s);
	nftnl_set_free(s);

	nls_list = nftnl_set_list_alloc();
	if (nls_list == NULL)
		memory_allocation_error();

	args.list = nls_list;
	args.ctx  = ctx;
	ret = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, set_cb, &args);
	if (ret < 0 && errno != ENOENT)
		goto err;

	return nls_list;
err:
	nftnl_set_list_free(nls_list);
	return NULL;
}

static void obj_tunnel_add_opts(struct nftnl_obj *nlo, struct tunnel *tunnel)
{
	struct nftnl_tunnel_opts *opts;
	struct nftnl_tunnel_opt *opt;

	switch (tunnel->type) {
	case TUNNEL_ERSPAN:
		opts = nftnl_tunnel_opts_alloc(NFTNL_TUNNEL_TYPE_ERSPAN);
		if (!opts)
			memory_allocation_error();

		opt = nftnl_tunnel_opt_alloc(NFTNL_TUNNEL_TYPE_ERSPAN);
		if (!opt)
			memory_allocation_error();

		nftnl_tunnel_opt_set(opt, NFTNL_TUNNEL_ERSPAN_VERSION,
				     &tunnel->erspan.version,
				     sizeof(tunnel->erspan.version));
		switch (tunnel->erspan.version) {
		case 1:
			nftnl_tunnel_opt_set(opt, NFTNL_TUNNEL_ERSPAN_V1_INDEX,
					     &tunnel->erspan.v1.index,
					     sizeof(tunnel->erspan.v1.index));
			break;
		case 2:
			nftnl_tunnel_opt_set(opt, NFTNL_TUNNEL_ERSPAN_V2_DIR,
					     &tunnel->erspan.v2.direction,
					     sizeof(tunnel->erspan.v2.direction));
			nftnl_tunnel_opt_set(opt, NFTNL_TUNNEL_ERSPAN_V2_HWID,
					     &tunnel->erspan.v2.hwid,
					     sizeof(tunnel->erspan.v2.hwid));
			break;
		}

		nftnl_tunnel_opts_add(opts, opt);
		nftnl_obj_set_data(nlo, NFTNL_OBJ_TUNNEL_OPTS, &opts, sizeof(struct nftnl_tunnel_opts *));
		break;
	case TUNNEL_VXLAN:
		opts = nftnl_tunnel_opts_alloc(NFTNL_TUNNEL_TYPE_VXLAN);
		if (!opts)
			memory_allocation_error();

		opt = nftnl_tunnel_opt_alloc(NFTNL_TUNNEL_TYPE_VXLAN);
		if (!opt)
			memory_allocation_error();

		nftnl_tunnel_opt_set(opt, NFTNL_TUNNEL_VXLAN_GBP,
				     &tunnel->vxlan.gbp,
				     sizeof(tunnel->vxlan.gbp));

		nftnl_tunnel_opts_add(opts, opt);
		nftnl_obj_set_data(nlo, NFTNL_OBJ_TUNNEL_OPTS, &opts, sizeof(struct nftnl_tunnel_opts *));
		break;
	case TUNNEL_GENEVE:
		struct tunnel_geneve *geneve;

		opts = nftnl_tunnel_opts_alloc(NFTNL_TUNNEL_TYPE_GENEVE);
		if (!opts)
			memory_allocation_error();

		list_for_each_entry(geneve, &tunnel->geneve_opts, list) {
			opt = nftnl_tunnel_opt_alloc(NFTNL_TUNNEL_TYPE_GENEVE);
			if (!opt)
				memory_allocation_error();

			nftnl_tunnel_opt_set(opt,
					     NFTNL_TUNNEL_GENEVE_TYPE,
					     &geneve->type, sizeof(geneve->type));
			nftnl_tunnel_opt_set(opt,
					     NFTNL_TUNNEL_GENEVE_CLASS,
					     &geneve->geneve_class, sizeof(geneve->geneve_class));
			nftnl_tunnel_opt_set(opt,
					     NFTNL_TUNNEL_GENEVE_DATA,
					     &geneve->data, geneve->data_len);
			nftnl_tunnel_opts_add(opts, opt);
		}
		nftnl_obj_set_data(nlo, NFTNL_OBJ_TUNNEL_OPTS, &opts, sizeof(struct nftnl_tunnel_opts *));
		break;
	case TUNNEL_UNSPEC:
		break;
	}
}

int mnl_nft_obj_add(struct netlink_ctx *ctx, struct cmd *cmd,
		    unsigned int flags)
{
	struct obj *obj = cmd->object;
	struct nft_data_linearize nld;
	struct nftnl_udata_buf *udbuf;
	struct nftnl_obj *nlo;
	struct nlmsghdr *nlh;

	nlo = nftnl_obj_alloc();
	if (!nlo)
		memory_allocation_error();

	nftnl_obj_set_u32(nlo, NFTNL_OBJ_FAMILY, cmd->handle.family);
	nftnl_obj_set_u32(nlo, NFTNL_OBJ_TYPE, obj->type);

	if (obj->comment) {
		udbuf = nftnl_udata_buf_alloc(NFT_USERDATA_MAXLEN);
		if (!udbuf)
			memory_allocation_error();
		if (!nftnl_udata_put_strz(udbuf, NFTNL_UDATA_OBJ_COMMENT, obj->comment))
			memory_allocation_error();
		nftnl_obj_set_data(nlo, NFTNL_OBJ_USERDATA, nftnl_udata_buf_data(udbuf),
				     nftnl_udata_buf_len(udbuf));
		nftnl_udata_buf_free(udbuf);
	}

	switch (obj->type) {
	case NFT_OBJECT_COUNTER:
		nftnl_obj_set_u64(nlo, NFTNL_OBJ_CTR_PKTS,
				  obj->counter.packets);
		nftnl_obj_set_u64(nlo, NFTNL_OBJ_CTR_BYTES,
				  obj->counter.bytes);
                break;
	case NFT_OBJECT_QUOTA:
		nftnl_obj_set_u64(nlo, NFTNL_OBJ_QUOTA_BYTES,
				  obj->quota.bytes);
		nftnl_obj_set_u64(nlo, NFTNL_OBJ_QUOTA_CONSUMED,
				  obj->quota.used);
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_QUOTA_FLAGS,
				  obj->quota.flags);
		break;
	case NFT_OBJECT_LIMIT:
		nftnl_obj_set_u64(nlo, NFTNL_OBJ_LIMIT_RATE, obj->limit.rate);
		nftnl_obj_set_u64(nlo, NFTNL_OBJ_LIMIT_UNIT, obj->limit.unit);
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_LIMIT_BURST, obj->limit.burst);
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_LIMIT_TYPE, obj->limit.type);
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_LIMIT_FLAGS, obj->limit.flags);
		break;
	case NFT_OBJECT_CT_HELPER:
		nftnl_obj_set_str(nlo, NFTNL_OBJ_CT_HELPER_NAME,
				  obj->ct_helper.name);
		nftnl_obj_set_u8(nlo, NFTNL_OBJ_CT_HELPER_L4PROTO,
				 obj->ct_helper.l4proto);
		if (obj->ct_helper.l3proto)
			nftnl_obj_set_u16(nlo, NFTNL_OBJ_CT_HELPER_L3PROTO,
					  obj->ct_helper.l3proto);
		break;
	case NFT_OBJECT_CT_TIMEOUT:
		nftnl_obj_set_u8(nlo, NFTNL_OBJ_CT_TIMEOUT_L4PROTO,
				 obj->ct_timeout.l4proto);
		if (obj->ct_timeout.l3proto)
			nftnl_obj_set_u16(nlo, NFTNL_OBJ_CT_TIMEOUT_L3PROTO,
					  obj->ct_timeout.l3proto);
		nftnl_obj_set_data(nlo, NFTNL_OBJ_CT_TIMEOUT_ARRAY,
				   obj->ct_timeout.timeout,
				   sizeof(obj->ct_timeout.timeout));
		break;
	case NFT_OBJECT_CT_EXPECT:
		if (obj->ct_expect.l3proto)
			nftnl_obj_set_u16(nlo, NFTNL_OBJ_CT_EXPECT_L3PROTO,
					  obj->ct_expect.l3proto);
		nftnl_obj_set_u8(nlo, NFTNL_OBJ_CT_EXPECT_L4PROTO,
				 obj->ct_expect.l4proto);
		nftnl_obj_set_u16(nlo, NFTNL_OBJ_CT_EXPECT_DPORT,
				  obj->ct_expect.dport);
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_CT_EXPECT_TIMEOUT,
				  obj->ct_expect.timeout);
		nftnl_obj_set_u8(nlo, NFTNL_OBJ_CT_EXPECT_SIZE,
				 obj->ct_expect.size);
		break;
	case NFT_OBJECT_SECMARK:
		nftnl_obj_set_str(nlo, NFTNL_OBJ_SECMARK_CTX,
				  obj->secmark.ctx);
		break;
	case NFT_OBJECT_SYNPROXY:
		nftnl_obj_set_u16(nlo, NFTNL_OBJ_SYNPROXY_MSS,
				  obj->synproxy.mss);
		nftnl_obj_set_u8(nlo, NFTNL_OBJ_SYNPROXY_WSCALE,
				 obj->synproxy.wscale);
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_SYNPROXY_FLAGS,
				  obj->synproxy.flags);
		break;
	case NFT_OBJECT_TUNNEL:
		nftnl_obj_set_u32(nlo, NFTNL_OBJ_TUNNEL_ID, obj->tunnel.id);
		if (obj->tunnel.sport)
			nftnl_obj_set_u16(nlo, NFTNL_OBJ_TUNNEL_SPORT,
					  obj->tunnel.sport);
		if (obj->tunnel.dport)
			nftnl_obj_set_u16(nlo, NFTNL_OBJ_TUNNEL_DPORT,
					  obj->tunnel.dport);
		if (obj->tunnel.tos)
			nftnl_obj_set_u8(nlo, NFTNL_OBJ_TUNNEL_TOS,
					  obj->tunnel.tos);
		if (obj->tunnel.ttl)
			nftnl_obj_set_u8(nlo, NFTNL_OBJ_TUNNEL_TTL,
					  obj->tunnel.ttl);
		if (obj->tunnel.src) {
			netlink_gen_data(obj->tunnel.src, &nld);
			if (nld.len == sizeof(struct in_addr)) {
				nftnl_obj_set_u32(nlo,
						  NFTNL_OBJ_TUNNEL_IPV4_SRC,
						  nld.value[0]);
			} else {
				nftnl_obj_set_data(nlo,
						   NFTNL_OBJ_TUNNEL_IPV6_SRC,
						   nld.value, nld.len);
			}
		}
		if (obj->tunnel.dst) {
			netlink_gen_data(obj->tunnel.dst, &nld);
			if (nld.len == sizeof(struct in_addr)) {
				nftnl_obj_set_u32(nlo,
						  NFTNL_OBJ_TUNNEL_IPV4_DST,
						  nld.value[0]);
		        } else {
				nftnl_obj_set_data(nlo,
						   NFTNL_OBJ_TUNNEL_IPV6_DST,
						   nld.value, nld.len);
			}
		}
		obj_tunnel_add_opts(nlo, &obj->tunnel);
		break;
	default:
		BUG("Unknown type %d", obj->type);
		break;
	}
	netlink_dump_obj(nlo, ctx);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWOBJ, cmd->handle.family,
				    NLM_F_CREATE | flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_OBJ_TABLE, cmd->handle.table.name);
	cmd_add_loc(cmd, nlh, &cmd->handle.obj.location);
	mnl_attr_put_strz(nlh, NFTA_OBJ_NAME, cmd->handle.obj.name);

	nftnl_obj_nlmsg_build_payload(nlh, nlo);
	nftnl_obj_free(nlo);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_obj_del(struct netlink_ctx *ctx, struct cmd *cmd, int type)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELOBJ;
	struct nftnl_obj *nlo;
	struct nlmsghdr *nlh;

	nlo = nftnl_obj_alloc();
	if (!nlo)
		memory_allocation_error();

	nftnl_obj_set_u32(nlo, NFTNL_OBJ_FAMILY, cmd->handle.family);
	nftnl_obj_set_u32(nlo, NFTNL_OBJ_TYPE, type);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYOBJ;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    msg_type, cmd->handle.family,
				    0, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_OBJ_TABLE, cmd->handle.table.name);

	if (cmd->handle.obj.name) {
		cmd_add_loc(cmd, nlh, &cmd->handle.obj.location);
		mnl_attr_put_strz(nlh, NFTA_OBJ_NAME, cmd->handle.obj.name);
	} else if (cmd->handle.handle.id) {
		cmd_add_loc(cmd, nlh, &cmd->handle.handle.location);
		mnl_attr_put_u64(nlh, NFTA_OBJ_HANDLE,
				 htobe64(cmd->handle.handle.id));
	}

	nftnl_obj_nlmsg_build_payload(nlh, nlo);
	nftnl_obj_free(nlo);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

static int obj_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_obj_list *nln_list = data;
	struct nftnl_obj *n;

	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	n = nftnl_obj_alloc();
	if (n == NULL)
		memory_allocation_error();

	if (nftnl_obj_nlmsg_parse(nlh, n) < 0)
		goto err_free;

	nftnl_obj_list_add_tail(n, nln_list);
	return MNL_CB_OK;

err_free:
	nftnl_obj_free(n);
	return MNL_CB_OK;
}


struct nftnl_obj_list *
mnl_nft_obj_dump(struct netlink_ctx *ctx, int family,
		 const char *table, const char *name,  uint32_t type, bool dump,
		 bool reset)
{
	uint16_t nl_flags = dump ? NLM_F_DUMP : NLM_F_ACK;
	struct nftnl_obj_list *nln_list;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nftnl_obj *n;
	int msg_type, ret;

	if (reset)
		msg_type = NFT_MSG_GETOBJ_RESET;
	else
		msg_type = NFT_MSG_GETOBJ;

	n = nftnl_obj_alloc();
	if (n == NULL)
		memory_allocation_error();

	nlh = nftnl_nlmsg_build_hdr(buf, msg_type, family,
				    nl_flags, ctx->seqnum);
	if (table != NULL)
		nftnl_obj_set_str(n, NFTNL_OBJ_TABLE, table);
	if (name != NULL)
		nftnl_obj_set_str(n, NFTNL_OBJ_NAME, name);
	if (type != NFT_OBJECT_UNSPEC)
		nftnl_obj_set_u32(n, NFTNL_OBJ_TYPE, type);
	nftnl_obj_nlmsg_build_payload(nlh, n);
	nftnl_obj_free(n);

	nln_list = nftnl_obj_list_alloc();
	if (nln_list == NULL)
		memory_allocation_error();

	ret = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, obj_cb, nln_list);
	if (ret < 0)
		goto err;

	return nln_list;
err:
	nftnl_obj_list_free(nln_list);
	return NULL;
}

/*
 * Set elements
 */
static int set_elem_cb(const struct nlmsghdr *nlh, void *data)
{
	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	nftnl_set_elems_nlmsg_parse(nlh, data);
	return MNL_CB_OK;
}

static bool mnl_nft_attr_nest_overflow(struct nlmsghdr *nlh,
				       const struct nlattr *from,
				       const struct nlattr *to)
{
	int len = (void *)to + to->nla_len - (void *)from;

	/* The attribute length field is 16 bits long, thus the maximum payload
	 * that an attribute can convey is UINT16_MAX. In case of overflow,
	 * discard the last attribute that did not fit into the nest.
	 */
	if (len > UINT16_MAX) {
		nlh->nlmsg_len -= to->nla_len;
		return true;
	}
	return false;
}

static void netlink_dump_setelem(const struct nftnl_set_elem *nlse,
				 struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;
	char buf[4096];

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	nftnl_set_elem_snprintf(buf, sizeof(buf), nlse, NFTNL_OUTPUT_DEFAULT, 0);
	fprintf(fp, "\t%s", buf);
}

static void netlink_dump_setelem_done(struct netlink_ctx *ctx)
{
	FILE *fp = ctx->nft->output.output_fp;

	if (!(ctx->nft->debug_mask & NFT_DEBUG_NETLINK) || !fp)
		return;

	fprintf(fp, "\n");
}

static struct nftnl_set_elem *
alloc_nftnl_setelem_interval(const struct set *set, const struct expr *init,
			     struct expr *elem, struct expr *next_elem,
			     struct nftnl_set_elem **nlse_high)
{
	struct nftnl_set_elem *nlse[2] = {};
	LIST_HEAD(interval_list);
	struct expr *expr, *next;
	int i = 0;

	if (setelem_to_interval(set, elem, next_elem, &interval_list) < 0)
		memory_allocation_error();

	if (list_empty(&interval_list)) {
		*nlse_high = NULL;
		nlse[i++] = alloc_nftnl_setelem(init, elem);
		return nlse[0];
	}

	list_for_each_entry_safe(expr, next, &interval_list, list) {
		nlse[i++] = alloc_nftnl_setelem(init, expr);
		list_del(&expr->list);
		expr_free(expr);
	}
	*nlse_high = nlse[1];

	return nlse[0];
}

static int mnl_nft_setelem_batch(const struct nftnl_set *nls, struct cmd *cmd,
				 struct nftnl_batch *batch,
				 enum nf_tables_msg_types msg_type,
				 unsigned int flags, uint32_t *seqnum,
				 const struct set *set, const struct expr *init,
				 struct netlink_ctx *ctx)
{
	struct nftnl_set_elem *nlse, *nlse_high = NULL;
	struct expr *expr = NULL, *next;
	struct nlattr *nest1, *nest2;
	struct nlmsghdr *nlh;
	int i = 0;

	if (msg_type == NFT_MSG_NEWSETELEM)
		flags |= NLM_F_CREATE;

	if (init)
		expr = list_first_entry(&expr_set(init)->expressions, struct expr, list);

next:
	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(batch), msg_type,
				    nftnl_set_get_u32(nls, NFTNL_SET_FAMILY),
				    flags, *seqnum);

	if (nftnl_set_is_set(nls, NFTNL_SET_TABLE)) {
                mnl_attr_put_strz(nlh, NFTA_SET_ELEM_LIST_TABLE,
				  nftnl_set_get_str(nls, NFTNL_SET_TABLE));
	}
	if (nftnl_set_is_set(nls, NFTNL_SET_NAME)) {
		mnl_attr_put_strz(nlh, NFTA_SET_ELEM_LIST_SET,
				  nftnl_set_get_str(nls, NFTNL_SET_NAME));
	}
	if (nftnl_set_is_set(nls, NFTNL_SET_ID)) {
		mnl_attr_put_u32(nlh, NFTA_SET_ELEM_LIST_SET_ID,
				 htonl(nftnl_set_get_u32(nls, NFTNL_SET_ID)));
	}

	if (!init || list_empty(&expr_set(init)->expressions))
		return 0;

	assert(expr);
	nest1 = mnl_attr_nest_start(nlh, NFTA_SET_ELEM_LIST_ELEMENTS);
	list_for_each_entry_from(expr, &expr_set(init)->expressions, list) {

		if (set_is_non_concat_range(set)) {
			if (set_is_anonymous(set->flags) &&
			    !list_is_last(&expr->list, &expr_set(init)->expressions))
				next = list_next_entry(expr, list);
			else
				next = NULL;

			if (!nlse_high) {
				nlse = alloc_nftnl_setelem_interval(set, init, expr, next, &nlse_high);
			} else {
				nlse = nlse_high;
				nlse_high = NULL;
			}
		} else {
			nlse = alloc_nftnl_setelem(init, expr);
		}

		cmd_add_loc(cmd, nlh, &expr->location);

		/* remain with this element, range high still needs to be added. */
		if (nlse_high)
			expr = list_prev_entry(expr, list);

		nest2 = mnl_attr_nest_start(nlh, ++i);
		nftnl_set_elem_nlmsg_build_payload(nlh, nlse);
		mnl_attr_nest_end(nlh, nest2);

		netlink_dump_setelem(nlse, ctx);
		nftnl_set_elem_free(nlse);
		if (mnl_nft_attr_nest_overflow(nlh, nest1, nest2)) {
			if (nlse_high) {
				nftnl_set_elem_free(nlse_high);
				nlse_high = NULL;
			}
			mnl_attr_nest_end(nlh, nest1);
			mnl_nft_batch_continue(batch);
			mnl_seqnum_inc(seqnum);
			goto next;
		}
	}
	mnl_attr_nest_end(nlh, nest1);
	mnl_nft_batch_continue(batch);
	netlink_dump_setelem_done(ctx);

	return 0;
}

int mnl_nft_setelem_add(struct netlink_ctx *ctx, struct cmd *cmd,
			const struct set *set, const struct expr *expr,
			unsigned int flags)
{
	const struct handle *h = &set->handle;
	struct nftnl_set *nls;
	int err;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(nls, NFTNL_SET_TABLE, h->table.name);
	nftnl_set_set_str(nls, NFTNL_SET_NAME, h->set.name);
	if (h->set_id)
		nftnl_set_set_u32(nls, NFTNL_SET_ID, h->set_id);
	if (set_is_datamap(set->flags))
		nftnl_set_set_u32(nls, NFTNL_SET_DATA_TYPE,
				  dtype_map_to_kernel(set->data->dtype));

	netlink_dump_set(nls, ctx);

	err = mnl_nft_setelem_batch(nls, cmd, ctx->batch, NFT_MSG_NEWSETELEM,
				    flags, &ctx->seqnum, set, expr, ctx);
	nftnl_set_free(nls);

	return err;
}

int mnl_nft_setelem_flush(struct netlink_ctx *ctx, const struct cmd *cmd)
{
	const struct handle *h = &cmd->handle;
	struct nftnl_set *nls;
	struct nlmsghdr *nlh;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(nls, NFTNL_SET_TABLE, h->table.name);
	nftnl_set_set_str(nls, NFTNL_SET_NAME, h->set.name);
	if (h->handle.id)
		nftnl_set_set_u64(nls, NFTNL_SET_HANDLE, h->handle.id);

	netlink_dump_set(nls, ctx);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_DELSETELEM,
				    h->family,
				    0, ctx->seqnum);
	nftnl_set_elems_nlmsg_build_payload(nlh, nls);
	nftnl_set_free(nls);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_setelem_del(struct netlink_ctx *ctx, struct cmd *cmd,
			const struct handle *h, const struct set *set,
			const struct expr *init)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELSETELEM;
	struct nftnl_set *nls;
	int err;

	nls = nftnl_set_alloc();
	if (nls == NULL)
		memory_allocation_error();

	nftnl_set_set_u32(nls, NFTNL_SET_FAMILY, h->family);
	nftnl_set_set_str(nls, NFTNL_SET_TABLE, h->table.name);
	if (h->set.name)
		nftnl_set_set_str(nls, NFTNL_SET_NAME, h->set.name);
	else if (h->handle.id)
		nftnl_set_set_u64(nls, NFTNL_SET_HANDLE, h->handle.id);

	netlink_dump_set(nls, ctx);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYSETELEM;

	err = mnl_nft_setelem_batch(nls, cmd, ctx->batch, msg_type, 0,
				    &ctx->seqnum, set, init, ctx);
	nftnl_set_free(nls);

	return err;
}

struct nftnl_set *mnl_nft_setelem_get_one(struct netlink_ctx *ctx,
					  struct nftnl_set *nls_in,
					  bool reset)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nftnl_set *nls_out;
	struct nlmsghdr *nlh;
	int msg_type;
	int err;

	if (reset)
		msg_type = NFT_MSG_GETSETELEM_RESET;
	else
		msg_type = NFT_MSG_GETSETELEM;

	nlh = nftnl_nlmsg_build_hdr(buf, msg_type,
				    nftnl_set_get_u32(nls_in, NFTNL_SET_FAMILY),
				    NLM_F_ACK, ctx->seqnum);
	nftnl_set_elems_nlmsg_build_payload(nlh, nls_in);

	nls_out = nftnl_set_alloc();
	if (!nls_out)
		return NULL;

	nftnl_set_set_str(nls_out, NFTNL_SET_TABLE,
			  nftnl_set_get_str(nls_in, NFTNL_SET_TABLE));
	nftnl_set_set_str(nls_out, NFTNL_SET_NAME,
			  nftnl_set_get_str(nls_in, NFTNL_SET_NAME));

	err = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, set_elem_cb, nls_out);
	if (err < 0) {
		nftnl_set_free(nls_out);
		return NULL;
	}

	return nls_out;
}

int mnl_nft_setelem_get(struct netlink_ctx *ctx, struct nftnl_set *nls,
			bool reset)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	int msg_type;

	if (reset)
		msg_type = NFT_MSG_GETSETELEM_RESET;
	else
		msg_type = NFT_MSG_GETSETELEM;

	nlh = nftnl_nlmsg_build_hdr(buf, msg_type,
				    nftnl_set_get_u32(nls, NFTNL_SET_FAMILY),
				    NLM_F_DUMP, ctx->seqnum);
	nftnl_set_elems_nlmsg_build_payload(nlh, nls);

	return nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, set_elem_cb, nls);
}

static int flowtable_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_flowtable_list *nln_list = data;
	struct nftnl_flowtable *n;

	if (check_genid(nlh) < 0)
		return MNL_CB_ERROR;

	n = nftnl_flowtable_alloc();
	if (n == NULL)
		memory_allocation_error();

	if (nftnl_flowtable_nlmsg_parse(nlh, n) < 0)
		goto err_free;

	nftnl_flowtable_list_add_tail(n, nln_list);
	return MNL_CB_OK;

err_free:
	nftnl_flowtable_free(n);
	return MNL_CB_OK;
}

struct nftnl_flowtable_list *
mnl_nft_flowtable_dump(struct netlink_ctx *ctx, int family,
		       const char *table, const char *ft)
{
	struct nftnl_flowtable_list *nln_list;
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nftnl_flowtable *n;
	int flags = NLM_F_DUMP;
	struct nlmsghdr *nlh;
	int ret;

	n = nftnl_flowtable_alloc();
	if (n == NULL)
		memory_allocation_error();

	if (table != NULL)
		nftnl_flowtable_set_str(n, NFTNL_FLOWTABLE_TABLE, table);
	if (ft) {
		nftnl_flowtable_set_str(n, NFTNL_FLOWTABLE_NAME, ft);
		flags = NLM_F_ACK;
	}
	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETFLOWTABLE, family,
				    flags, ctx->seqnum);
	nftnl_flowtable_nlmsg_build_payload(nlh, n);
	nftnl_flowtable_free(n);

	nln_list = nftnl_flowtable_list_alloc();
	if (nln_list == NULL)
		memory_allocation_error();

	ret = nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, flowtable_cb, nln_list);
	if (ret < 0 && errno != ENOENT)
		goto err;

	return nln_list;
err:
	nftnl_flowtable_list_free(nln_list);
	return NULL;
}

static void mnl_nft_ft_devs_build(struct nlmsghdr *nlh, struct cmd *cmd)
{
	const struct expr *dev_expr = cmd->flowtable->dev_expr;
	const struct nft_dev *dev_array;
	struct nlattr *nest_dev;
	int i, num_devs= 0;

	dev_array = nft_dev_array(dev_expr, &num_devs);
	nest_dev = mnl_attr_nest_start(nlh, NFTA_FLOWTABLE_HOOK_DEVS);
	for (i = 0; i < num_devs; i++) {
		cmd_add_loc(cmd, nlh, dev_array[i].location);
		mnl_nft_attr_put_ifname(nlh, dev_array[i].ifname);
	}

	mnl_attr_nest_end(nlh, nest_dev);
	nft_dev_array_free(dev_array);
}

int mnl_nft_flowtable_add(struct netlink_ctx *ctx, struct cmd *cmd,
			  unsigned int flags)
{
	struct nftnl_flowtable *flo;
	struct nlmsghdr *nlh;
	struct nlattr *nest;
	int priority;

	flo = nftnl_flowtable_alloc();
	if (!flo)
		memory_allocation_error();

	nftnl_flowtable_set_u32(flo, NFTNL_FLOWTABLE_FAMILY,
				cmd->handle.family);

	nftnl_flowtable_set_u32(flo, NFTNL_FLOWTABLE_FLAGS,
				cmd->flowtable->flags);

	netlink_dump_flowtable(flo, ctx);

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    NFT_MSG_NEWFLOWTABLE, cmd->handle.family,
				    NLM_F_CREATE | flags, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_FLOWTABLE_TABLE, cmd->handle.table.name);
	cmd_add_loc(cmd, nlh, &cmd->handle.flowtable.location);
	mnl_attr_put_strz(nlh, NFTA_FLOWTABLE_NAME, cmd->handle.flowtable.name);

	nftnl_flowtable_nlmsg_build_payload(nlh, flo);

	nest = mnl_attr_nest_start(nlh, NFTA_FLOWTABLE_HOOK);

	if (cmd->flowtable && cmd->flowtable->priority.expr) {
		mnl_attr_put_u32(nlh, NFTA_FLOWTABLE_HOOK_NUM, htonl(cmd->flowtable->hook.num));
		mpz_export_data(&priority, cmd->flowtable->priority.expr->value,
				BYTEORDER_HOST_ENDIAN, sizeof(int));
		mnl_attr_put_u32(nlh, NFTA_FLOWTABLE_HOOK_PRIORITY, htonl(priority));
	}

	if (cmd->flowtable->dev_expr)
		mnl_nft_ft_devs_build(nlh, cmd);

	mnl_attr_nest_end(nlh, nest);

	nftnl_flowtable_free(flo);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

int mnl_nft_flowtable_del(struct netlink_ctx *ctx, struct cmd *cmd)
{
	enum nf_tables_msg_types msg_type = NFT_MSG_DELFLOWTABLE;
	struct nftnl_flowtable *flo;
	struct nlmsghdr *nlh;
	struct nlattr *nest;

	flo = nftnl_flowtable_alloc();
	if (!flo)
		memory_allocation_error();

	nftnl_flowtable_set_u32(flo, NFTNL_FLOWTABLE_FAMILY,
				cmd->handle.family);

	if (cmd->op == CMD_DESTROY)
		msg_type = NFT_MSG_DESTROYFLOWTABLE;

	nlh = nftnl_nlmsg_build_hdr(nftnl_batch_buffer(ctx->batch),
				    msg_type, cmd->handle.family,
				    0, ctx->seqnum);

	cmd_add_loc(cmd, nlh, &cmd->handle.table.location);
	mnl_attr_put_strz(nlh, NFTA_FLOWTABLE_TABLE, cmd->handle.table.name);

	if (cmd->handle.flowtable.name) {
		cmd_add_loc(cmd, nlh, &cmd->handle.flowtable.location);
		mnl_attr_put_strz(nlh, NFTA_FLOWTABLE_NAME,
				  cmd->handle.flowtable.name);
	} else if (cmd->handle.handle.id) {
		cmd_add_loc(cmd, nlh, &cmd->handle.handle.location);
		mnl_attr_put_u64(nlh, NFTA_FLOWTABLE_HANDLE,
				 htobe64(cmd->handle.handle.id));
	}

	nftnl_flowtable_nlmsg_build_payload(nlh, flo);

	if (cmd->op == CMD_DELETE &&
	    cmd->flowtable && cmd->flowtable->dev_expr) {
		nest = mnl_attr_nest_start(nlh, NFTA_FLOWTABLE_HOOK);
		mnl_nft_ft_devs_build(nlh, cmd);
		mnl_attr_nest_end(nlh, nest);
	}

	nftnl_flowtable_free(flo);

	mnl_nft_batch_continue(ctx->batch);

	return 0;
}

/*
 * events
 */
#define NFTABLES_NLEVENT_BUFSIZ	(1 << 24)

int mnl_nft_event_listener(struct mnl_socket *nf_sock, unsigned int debug_mask,
			   struct output_ctx *octx,
			   int (*cb)(const struct nlmsghdr *nlh, void *data),
			   void *cb_data)
{
	/* Set netlink socket buffer size to 16 Mbytes to reduce chances of
	 * message loss due to ENOBUFS.
	 */
	unsigned int bufsiz = NFTABLES_NLEVENT_BUFSIZ;
	int fd = mnl_socket_get_fd(nf_sock);
	char buf[NFT_NLMSG_MAXSIZE];
	fd_set readfds;
	int ret;

	ret = mnl_set_rcvbuffer(nf_sock, bufsiz);
	if (ret < 0)
		nft_print(octx, "# Cannot set up netlink receive socket buffer size to %u bytes, falling back to %u bytes\n",
			  NFTABLES_NLEVENT_BUFSIZ, bufsiz);

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);

		ret = select(fd + 1, &readfds, NULL, NULL, NULL);
		if (ret < 0)
			return -1;

		if (FD_ISSET(fd, &readfds)) {
			ret = mnl_socket_recvfrom(nf_sock, buf, sizeof(buf));
			if (ret < 0) {
				if (errno == ENOBUFS) {
					nft_print(octx, "# ERROR: We lost some netlink events!\n");
					continue;
				}
				nft_print(octx, "# ERROR: %s\n",
					  strerror(errno));
				break;
			}
		}

		if (debug_mask & NFT_DEBUG_MNL) {
			mnl_nlmsg_fprintf(octx->output_fp, buf, sizeof(buf),
					  sizeof(struct nfgenmsg));
		}
		ret = mnl_cb_run(buf, ret, 0, 0, cb, cb_data);
		if (ret <= 0)
			break;
	}
	return ret;
}

static struct basehook *basehook_alloc(void)
{
	return xzalloc(sizeof(struct basehook));
}

static void basehook_free(struct basehook *b)
{
	list_del(&b->list);
	free_const(b->module_name);
	free_const(b->hookfn);
	free_const(b->chain);
	free_const(b->table);
	free_const(b->devname);
	free(b);
}

static bool basehook_eq(const struct basehook *prev, const struct basehook *hook)
{
	if (prev->num != hook->num)
		return false;

	if (prev->devname != NULL && hook->devname != NULL)
		return strcmp(prev->devname, hook->devname) == 0;

	if (prev->devname == NULL && hook->devname == NULL)
		return true;

	return false;
}

static void basehook_list_add_tail(struct basehook *b, struct list_head *head)
{
	struct basehook *hook;

	list_for_each_entry(hook, head, list) {
		if (hook->family != b->family)
			continue;
		if (!basehook_eq(hook, b))
			continue;
		if (hook->prio < b->prio)
			continue;

		list_add(&b->list, &hook->list);
		return;
	}

	list_add_tail(&b->list, head);
}

static int dump_nf_attr_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFNLA_HOOK_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFNLA_HOOK_HOOKNUM:
	case NFNLA_HOOK_PRIORITY:
                if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
                        return MNL_CB_ERROR;
		break;
	case NFNLA_HOOK_DEV:
                if (mnl_attr_validate(attr, MNL_TYPE_STRING) < 0)
                        return MNL_CB_ERROR;
		break;
	case NFNLA_HOOK_MODULE_NAME:
	case NFNLA_HOOK_FUNCTION_NAME:
                if (mnl_attr_validate(attr, MNL_TYPE_NUL_STRING) < 0)
                        return MNL_CB_ERROR;
		break;
	case NFNLA_HOOK_CHAIN_INFO:
                if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
                        return MNL_CB_ERROR;
		break;
	default:
		return MNL_CB_OK;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int dump_nf_chain_info_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFNLA_HOOK_INFO_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFNLA_HOOK_INFO_DESC:
                if (mnl_attr_validate(attr, MNL_TYPE_NESTED) < 0)
                        return MNL_CB_ERROR;
		break;
	case NFNLA_HOOK_INFO_TYPE:
                if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
                        return MNL_CB_ERROR;
		break;
	default:
		return MNL_CB_OK;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int dump_nf_attr_chain_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFNLA_CHAIN_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFNLA_CHAIN_TABLE:
	case NFNLA_CHAIN_NAME:
                if (mnl_attr_validate(attr, MNL_TYPE_NUL_STRING) < 0)
                        return MNL_CB_ERROR;
		break;
	case NFNLA_CHAIN_FAMILY:
                if (mnl_attr_validate(attr, MNL_TYPE_U8) < 0)
                        return MNL_CB_ERROR;
		break;
	default:
		return MNL_CB_OK;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static int dump_nf_attr_bpf_cb(const struct nlattr *attr, void *data)
{
	int type = mnl_attr_get_type(attr);
	const struct nlattr **tb = data;

	if (mnl_attr_type_valid(attr, NFNLA_HOOK_BPF_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFNLA_HOOK_BPF_ID:
                if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
                        return MNL_CB_ERROR;
		break;
	default:
		return MNL_CB_OK;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

struct dump_nf_hook_data {
	struct list_head *hook_list;
	const char *devname;
	int family;
};

static int dump_nf_hooks(const struct nlmsghdr *nlh, void *_data)
{
	const struct nfgenmsg *nfg = mnl_nlmsg_get_payload(nlh);
	struct nlattr *tb[NFNLA_HOOK_MAX + 1] = {};
	struct dump_nf_hook_data *data = _data;
	struct basehook *hook;

	/* NB: Don't check the nft generation ID, this is not
	 * an nftables subsystem.
	 */
	if (mnl_attr_parse(nlh, sizeof(*nfg), dump_nf_attr_cb, tb) < 0)
		return -1;

	if (!tb[NFNLA_HOOK_PRIORITY])
		netlink_abi_error();

	hook = basehook_alloc();
	hook->prio = ntohl(mnl_attr_get_u32(tb[NFNLA_HOOK_PRIORITY]));
	hook->devname = data->devname ? xstrdup(data->devname) : NULL;

	if (tb[NFNLA_HOOK_FUNCTION_NAME])
		hook->hookfn = xstrdup(mnl_attr_get_str(tb[NFNLA_HOOK_FUNCTION_NAME]));

	if (tb[NFNLA_HOOK_MODULE_NAME])
		hook->module_name = xstrdup(mnl_attr_get_str(tb[NFNLA_HOOK_MODULE_NAME]));

	if (tb[NFNLA_HOOK_CHAIN_INFO]) {
		struct nlattr *nested[NFNLA_HOOK_INFO_MAX + 1] = {};
		uint32_t type;

		if (mnl_attr_parse_nested(tb[NFNLA_HOOK_CHAIN_INFO],
					  dump_nf_chain_info_cb, nested) < 0) {
			basehook_free(hook);
			return -1;
		}

		type = ntohl(mnl_attr_get_u32(nested[NFNLA_HOOK_INFO_TYPE]));
		if (type == NFNL_HOOK_TYPE_NFTABLES ||
		    type == NFNL_HOOK_TYPE_NFT_FLOWTABLE) {
			struct nlattr *info[NFNLA_CHAIN_MAX + 1] = {};
			const char *tablename, *chainname;

			if (mnl_attr_parse_nested(nested[NFNLA_HOOK_INFO_DESC],
						  dump_nf_attr_chain_cb,
						  info) < 0) {
				basehook_free(hook);
				return -1;
			}

			tablename = mnl_attr_get_str(info[NFNLA_CHAIN_TABLE]);
			chainname = mnl_attr_get_str(info[NFNLA_CHAIN_NAME]);
			if (tablename && chainname) {
				hook->table = xstrdup(tablename);
				hook->chain = xstrdup(chainname);
			}
			hook->chain_family = mnl_attr_get_u8(info[NFNLA_CHAIN_FAMILY]);
			if (type == NFNL_HOOK_TYPE_NFT_FLOWTABLE)
				hook->objtype = "flowtable";
			else
				hook->objtype = "chain";
		} else if (type == NFNL_HOOK_TYPE_BPF) {
			struct nlattr *info[NFNLA_HOOK_BPF_MAX + 1] = {};

			if (mnl_attr_parse_nested(nested[NFNLA_HOOK_INFO_DESC],
						  dump_nf_attr_bpf_cb, info) < 0) {
				basehook_free(hook);
				return -1;
			}

			if (info[NFNLA_HOOK_BPF_ID]) {
				char tmpbuf[16];

				snprintf(tmpbuf, sizeof(tmpbuf), "id %u",
					 ntohl(mnl_attr_get_u32(info[NFNLA_HOOK_BPF_ID])));

				hook->chain = xstrdup(tmpbuf);
			}
		}
	}
	if (tb[NFNLA_HOOK_HOOKNUM])
		hook->num = ntohl(mnl_attr_get_u32(tb[NFNLA_HOOK_HOOKNUM]));

	hook->family = nfg->nfgen_family;

	basehook_list_add_tail(hook, data->hook_list);

	return MNL_CB_OK;
}

static struct nlmsghdr *nf_hook_dump_request(char *buf, uint8_t family, uint32_t seq)
{
	struct nlmsghdr *nlh = mnl_nlmsg_put_header(buf);
	struct nfgenmsg *nfg;

	nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	nlh->nlmsg_type = NFNL_SUBSYS_HOOK << 8;
	nlh->nlmsg_seq = seq;

	nfg = mnl_nlmsg_put_extra_header(nlh, sizeof(*nfg));
	nfg->nfgen_family = family;
	nfg->version = NFNETLINK_V0;

	return nlh;
}

static int __mnl_nft_dump_nf_hooks(struct netlink_ctx *ctx, uint8_t query_family,
				   uint8_t family, uint8_t hooknum,
				   const char *devname,
				   struct list_head *hook_list)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct dump_nf_hook_data data = {
		.hook_list	= hook_list,
		.devname	= devname,
		.family		= query_family,
	};
	struct nlmsghdr *nlh;

	nlh = nf_hook_dump_request(buf, family, ctx->seqnum);
	if (devname)
		mnl_attr_put_strz(nlh, NFNLA_HOOK_DEV, devname);

	mnl_attr_put_u32(nlh, NFNLA_HOOK_HOOKNUM, htonl(hooknum));

	return nft_mnl_talk(ctx, nlh, nlh->nlmsg_len, dump_nf_hooks, &data);
}

static void print_hooks(struct netlink_ctx *ctx, int family, struct list_head *hook_list)
{
	struct basehook *hook, *tmp, *prev = NULL;
	bool same, family_in_use = false;
	int prio;
	FILE *fp;

	fp = ctx->nft->output.output_fp;

	list_for_each_entry_safe(hook, tmp, hook_list, list) {
		if (hook->family == family) {
			family_in_use = true;
			break;
		}
	}

	if (!family_in_use)
		return;

	fprintf(fp, "family %s {\n", family2str(family));

	list_for_each_entry_safe(hook, tmp, hook_list, list) {
		if (hook->family != family)
			continue;

		if (prev) {
			if (basehook_eq(prev, hook)) {
				fprintf(fp, "\n");
				same = true;
			} else {
				same = false;
				fprintf(fp, "\n\t}\n");
			}
		} else {
			same = false;
		}
		prev = hook;

		if (!same) {
			if (hook->devname)
				fprintf(fp, "\thook %s device %s {\n",
					hooknum2str(family, hook->num), hook->devname);
			else
				fprintf(fp, "\thook %s {\n",
					hooknum2str(family, hook->num));
		}

		prio = hook->prio;
		if (prio < 0)
			fprintf(fp, "\t\t%011d", prio); /* outputs a '-' sign */
		else if (prio == 0)
			fprintf(fp, "\t\t %010u", prio);
		else
			fprintf(fp, "\t\t+%010u", prio);

		if (hook->table && hook->chain)
			fprintf(fp, " %s %s %s %s",
				hook->objtype, family2str(hook->chain_family),
				hook->table, hook->chain);
		else if (hook->hookfn && hook->chain)
			fprintf(fp, " %s %s", hook->hookfn, hook->chain);
		else if (hook->hookfn) {
			fprintf(fp, " %s", hook->hookfn);
		}
		if (hook->module_name)
			fprintf(fp, " [%s]", hook->module_name);
	}

	fprintf(fp, "\n\t}\n");
	fprintf(fp, "}\n");
}

static int mnl_nft_dump_nf(struct netlink_ctx *ctx, int family,
			   const char *devname, struct list_head *hook_list)
{
	int i, err;

	for (i = 0; i <= NF_INET_POST_ROUTING; i++) {
		int tmp;

		tmp = __mnl_nft_dump_nf_hooks(ctx, family, family, i, devname, hook_list);
		if (tmp == 0)
			err = 0;
	}

	return err;
}

static int mnl_nft_dump_nf_arp(struct netlink_ctx *ctx, int family,
			       const char *devname, struct list_head *hook_list)
{
	int err1, err2;

	err1 = __mnl_nft_dump_nf_hooks(ctx, family, family, NF_ARP_IN, devname, hook_list);
	err2 = __mnl_nft_dump_nf_hooks(ctx, family, family, NF_ARP_OUT, devname, hook_list);

	return err1 ? err2 : err1;
}

static int mnl_nft_dump_nf_netdev(struct netlink_ctx *ctx, int family,
				  const char *devname, struct list_head *hook_list)
{
	int err1, err2;

	err1 = __mnl_nft_dump_nf_hooks(ctx, family, NFPROTO_NETDEV, NF_NETDEV_INGRESS, devname, hook_list);
	err2 = __mnl_nft_dump_nf_hooks(ctx, family, NFPROTO_NETDEV, NF_NETDEV_EGRESS, devname, hook_list);

	return err1 ? err2 : err1;
}

static void release_hook_list(struct list_head *hook_list)
{
	struct basehook *hook, *next;

	list_for_each_entry_safe(hook, next, hook_list, list)
		basehook_free(hook);
}

static void warn_if_device(struct nft_ctx *nft, const char *devname)
{
	if (devname)
		nft_print(&nft->output, "# device keyword (%s) unexpected for this family\n", devname);
}

int mnl_nft_dump_nf_hooks(struct netlink_ctx *ctx, int family, const char *devname)
{
	LIST_HEAD(hook_list);
	int ret = -1, tmp;

	errno = 0;

	switch (family) {
	case NFPROTO_UNSPEC:
		ret = mnl_nft_dump_nf_hooks(ctx, NFPROTO_ARP, NULL);
		tmp = mnl_nft_dump_nf_hooks(ctx, NFPROTO_INET, NULL);
		if (tmp == 0)
			ret = 0;
		tmp = mnl_nft_dump_nf_hooks(ctx, NFPROTO_BRIDGE, NULL);
		if (tmp == 0)
			ret = 0;

		tmp = mnl_nft_dump_nf_hooks(ctx, NFPROTO_NETDEV, devname);
		if (tmp == 0)
			ret = 0;

		return ret;
	case NFPROTO_INET:
		ret = 0;
		if (devname)
			ret = __mnl_nft_dump_nf_hooks(ctx, family, NFPROTO_NETDEV,
						      NF_NETDEV_INGRESS, devname, &hook_list);
		tmp = mnl_nft_dump_nf_hooks(ctx, NFPROTO_IPV4, NULL);
		if (tmp == 0)
			ret = 0;
		tmp = mnl_nft_dump_nf_hooks(ctx, NFPROTO_IPV6, NULL);
		if (tmp == 0)
			ret = 0;

		break;
	case NFPROTO_IPV4:
	case NFPROTO_IPV6:
	case NFPROTO_BRIDGE:
		warn_if_device(ctx->nft, devname);
		ret = mnl_nft_dump_nf(ctx, family, devname, &hook_list);
		break;
	case NFPROTO_ARP:
		warn_if_device(ctx->nft, devname);
		ret = mnl_nft_dump_nf_arp(ctx, family, devname, &hook_list);
		break;
	case NFPROTO_NETDEV:
		if (devname) {
			ret = mnl_nft_dump_nf_netdev(ctx, family, devname, &hook_list);
		} else {
			const struct iface *iface;

			iface = iface_cache_get_next_entry(NULL);
			ret = 0;

			while (iface) {
				tmp = mnl_nft_dump_nf_netdev(ctx, family, iface->name, &hook_list);
				if (tmp == 0)
					ret = 0;

				iface = iface_cache_get_next_entry(iface);
			}
		}

		break;
	}

	print_hooks(ctx, family, &hook_list);
	release_hook_list(&hook_list);

	return ret;
}
