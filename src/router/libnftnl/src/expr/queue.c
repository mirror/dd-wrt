/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * (C) 2013 by Eric Leblond <eric@regit.org>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/netfilter/nf_tables.h>

#include "internal.h"
#include <libmnl/libmnl.h>
#include <libnftnl/expr.h>
#include <libnftnl/rule.h>

struct nftnl_expr_queue {
	enum nft_registers	sreg_qnum;
	uint16_t		queuenum;
	uint16_t		queues_total;
	uint16_t		flags;
};

static int nftnl_expr_queue_set(struct nftnl_expr *e, uint16_t type,
				    const void *data, uint32_t data_len)
{
	struct nftnl_expr_queue *queue = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_QUEUE_NUM:
		memcpy(&queue->queuenum, data, data_len);
		break;
	case NFTNL_EXPR_QUEUE_TOTAL:
		memcpy(&queue->queues_total, data, data_len);
		break;
	case NFTNL_EXPR_QUEUE_FLAGS:
		memcpy(&queue->flags, data, data_len);
		break;
	case NFTNL_EXPR_QUEUE_SREG_QNUM:
		memcpy(&queue->sreg_qnum, data, data_len);
		break;
	}
	return 0;
}

static const void *
nftnl_expr_queue_get(const struct nftnl_expr *e, uint16_t type,
			 uint32_t *data_len)
{
	struct nftnl_expr_queue *queue = nftnl_expr_data(e);

	switch(type) {
	case NFTNL_EXPR_QUEUE_NUM:
		*data_len = sizeof(queue->queuenum);
		return &queue->queuenum;
	case NFTNL_EXPR_QUEUE_TOTAL:
		*data_len = sizeof(queue->queues_total);
		return &queue->queues_total;
	case NFTNL_EXPR_QUEUE_FLAGS:
		*data_len = sizeof(queue->flags);
		return &queue->flags;
	case NFTNL_EXPR_QUEUE_SREG_QNUM:
		*data_len = sizeof(queue->sreg_qnum);
		return &queue->sreg_qnum;
	}
	return NULL;
}

static int nftnl_expr_queue_cb(const struct nlattr *attr, void *data)
{
	const struct nlattr **tb = data;
	int type = mnl_attr_get_type(attr);

	if (mnl_attr_type_valid(attr, NFTA_QUEUE_MAX) < 0)
		return MNL_CB_OK;

	switch(type) {
	case NFTA_QUEUE_NUM:
	case NFTA_QUEUE_TOTAL:
	case NFTA_QUEUE_FLAGS:
		if (mnl_attr_validate(attr, MNL_TYPE_U16) < 0)
			abi_breakage();
		break;
	case NFTA_QUEUE_SREG_QNUM:
		if (mnl_attr_validate(attr, MNL_TYPE_U32) < 0)
			abi_breakage();
		break;
	}

	tb[type] = attr;
	return MNL_CB_OK;
}

static void
nftnl_expr_queue_build(struct nlmsghdr *nlh, const struct nftnl_expr *e)
{
	struct nftnl_expr_queue *queue = nftnl_expr_data(e);

	if (e->flags & (1 << NFTNL_EXPR_QUEUE_NUM))
		mnl_attr_put_u16(nlh, NFTA_QUEUE_NUM, htons(queue->queuenum));
	if (e->flags & (1 << NFTNL_EXPR_QUEUE_TOTAL))
		mnl_attr_put_u16(nlh, NFTA_QUEUE_TOTAL, htons(queue->queues_total));
	if (e->flags & (1 << NFTNL_EXPR_QUEUE_FLAGS))
		mnl_attr_put_u16(nlh, NFTA_QUEUE_FLAGS, htons(queue->flags));
	if (e->flags & (1 << NFTNL_EXPR_QUEUE_SREG_QNUM))
		mnl_attr_put_u32(nlh, NFTA_QUEUE_SREG_QNUM, htonl(queue->sreg_qnum));
}

static int
nftnl_expr_queue_parse(struct nftnl_expr *e, struct nlattr *attr)
{
	struct nftnl_expr_queue *queue = nftnl_expr_data(e);
	struct nlattr *tb[NFTA_QUEUE_MAX+1] = {};

	if (mnl_attr_parse_nested(attr, nftnl_expr_queue_cb, tb) < 0)
		return -1;

	if (tb[NFTA_QUEUE_NUM]) {
		queue->queuenum = ntohs(mnl_attr_get_u16(tb[NFTA_QUEUE_NUM]));
		e->flags |= (1 << NFTNL_EXPR_QUEUE_NUM);
	}
	if (tb[NFTA_QUEUE_TOTAL]) {
		queue->queues_total = ntohs(mnl_attr_get_u16(tb[NFTA_QUEUE_TOTAL]));
		e->flags |= (1 << NFTNL_EXPR_QUEUE_TOTAL);
	}
	if (tb[NFTA_QUEUE_FLAGS]) {
		queue->flags = ntohs(mnl_attr_get_u16(tb[NFTA_QUEUE_FLAGS]));
		e->flags |= (1 << NFTNL_EXPR_QUEUE_FLAGS);
	}
	if (tb[NFTA_QUEUE_SREG_QNUM]) {
		queue->sreg_qnum = ntohl(mnl_attr_get_u32(tb[NFTA_QUEUE_SREG_QNUM]));
		e->flags |= (1 << NFTNL_EXPR_QUEUE_SREG_QNUM);
	}

	return 0;
}

static int
nftnl_expr_queue_snprintf(char *buf, size_t remain,
			  uint32_t flags, const struct nftnl_expr *e)
{
	struct nftnl_expr_queue *queue = nftnl_expr_data(e);
	uint16_t total_queues;
	int ret, offset = 0;

	if (e->flags & (1 << NFTNL_EXPR_QUEUE_NUM)) {
		total_queues = queue->queuenum + queue->queues_total - 1;

		ret = snprintf(buf + offset, remain, "num %u", queue->queuenum);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);

		if (queue->queues_total && total_queues != queue->queuenum) {
			ret = snprintf(buf + offset, remain, "-%u", total_queues);
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		}

		ret = snprintf(buf + offset, remain, " ");
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_QUEUE_SREG_QNUM)) {
		ret = snprintf(buf + offset, remain, "sreg_qnum %u ",
			       queue->sreg_qnum);
		SNPRINTF_BUFFER_SIZE(ret, remain, offset);
	}

	if (e->flags & (1 << NFTNL_EXPR_QUEUE_FLAGS)) {
		if (queue->flags & (NFT_QUEUE_FLAG_BYPASS)) {
			ret = snprintf(buf + offset, remain, "bypass ");
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		}
		if (queue->flags & (NFT_QUEUE_FLAG_CPU_FANOUT)) {
			ret = snprintf(buf + offset, remain, "fanout ");
			SNPRINTF_BUFFER_SIZE(ret, remain, offset);
		}
	}
	return offset;
}

static struct attr_policy queue_attr_policy[__NFTNL_EXPR_QUEUE_MAX] = {
	[NFTNL_EXPR_QUEUE_NUM]       = { .maxlen = sizeof(uint16_t) },
	[NFTNL_EXPR_QUEUE_TOTAL]     = { .maxlen = sizeof(uint16_t) },
	[NFTNL_EXPR_QUEUE_FLAGS]     = { .maxlen = sizeof(uint16_t) },
	[NFTNL_EXPR_QUEUE_SREG_QNUM] = { .maxlen = sizeof(uint32_t) },
};

struct expr_ops expr_ops_queue = {
	.name		= "queue",
	.alloc_len	= sizeof(struct nftnl_expr_queue),
	.nftnl_max_attr	= __NFTNL_EXPR_QUEUE_MAX - 1,
	.attr_policy	= queue_attr_policy,
	.set		= nftnl_expr_queue_set,
	.get		= nftnl_expr_queue_get,
	.parse		= nftnl_expr_queue_parse,
	.build		= nftnl_expr_queue_build,
	.output		= nftnl_expr_queue_snprintf,
};
