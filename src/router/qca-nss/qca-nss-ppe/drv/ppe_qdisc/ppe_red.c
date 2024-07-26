/*
 * Copyright (c) 2014-2017, 2020-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ppe_qdisc.h"
#include "ppe_red.h"

/*
 * ppered private qdisc structure
 */
struct  ppe_red_sched_data {
	struct ppe_qdisc pq;	/* Common base class for all ppe qdiscs */
	u32 limit;		/* Queue length */
	struct tc_red_alg_parameter rap;/* Parameters for RED alg */
	u8 ecn;			/* Mark ECN or drop pkt */
	u8 set_default;		/* Flag to set qdisc as default qdisc for enqueue */
};

/*
 * ppered policy structure
 */
static struct nla_policy ppe_red_policy[TCA_PPERED_MAX + 1] = {
	[TCA_PPERED_PARMS] = { .len = sizeof(struct tc_ppered_qopt) },
};

/*
 * ppe_red_enqueue()
 *	Enqueue API for ppered qdisc
 */
static int ppe_red_enqueue(struct sk_buff *skb, struct Qdisc *sch,
		struct sk_buff **to_free)
{
	return ppe_qdisc_enqueue(skb, sch, to_free);
}

/*
 * ppe_red_dequeue()
 *	Dequeue API for ppered qdisc
 */
static struct sk_buff *ppe_red_dequeue(struct Qdisc *sch)
{
	return ppe_qdisc_dequeue(sch);
}

/*
 * ppe_red_reset()
 *	Reset the ppered qdisc
 */
static void ppe_red_reset(struct Qdisc *sch)
{
	ppe_qdisc_info("%px ppered resetting", sch);
	ppe_qdisc_reset(sch);
}

/*
 * ppe_red_destroy()
 *	Destroy the ppered qdisc
 */
static void ppe_red_destroy(struct Qdisc *sch)
{
	struct ppe_qdisc *pq = (struct ppe_qdisc *)qdisc_priv(sch);

	ppe_qdisc_destroy(pq);
	ppe_qdisc_info("%px ppered destroyed", sch);
}

/*
 * ppe_red_change()
 *      Function call to configure the ppered parameters
 */
static int ppe_red_change(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	struct ppe_red_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPEFIFO_MAX + 1];
	struct tc_ppered_qopt *qopt;
	struct ppe_qdisc *pq = &q->pq;
	struct ppe_drv_qos_res *res = &pq->res;
	struct ppe_qdisc prev_pq;

	if (!opt) {
		return -EINVAL;
	}

	/*
	 * Save previous configuration for reset purpose.
	 */
	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CONFIGURED)) {
		prev_pq = q->pq;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_red_policy, tb, TCA_PPERED_MAX, TCA_PPERED_PARMS, extack);
	if (!qopt) {
		return -EINVAL;
	}

	ppe_qdisc_info("%px ppered %x ECN:%d", sch, sch->handle, qopt->ecn);

	/*
	 * This is a red setup command
	 */
	if (!qopt->limit || !qopt->rap.exp_weight_factor) {
		ppe_qdisc_warning("%px ppered %x requires RED algorithm parameters", sch, sch->handle);
		return -EINVAL;
	}

	/*
	 * If min/max not specified, calculate it
	 */
	if (!qopt->rap.max) {
		qopt->rap.max = qopt->rap.min ? qopt->rap.min * 3 : qopt->limit / 4;
	}
	if (!qopt->rap.min) {
		qopt->rap.min = qopt->rap.max / 3;
	}

	q->ecn = qopt->ecn;
	q->limit = qopt->limit;
	q->rap.min = qopt->rap.min;
	q->rap.max = qopt->rap.max;
	q->rap.probability = qopt->rap.probability;
	q->rap.exp_weight_factor = qopt->rap.exp_weight_factor;
	q->set_default = qopt->set_default;

	res->q.red_en = true;
	res->q.color_en = false;

	/*
	 * PPE operates in terms of memory blocks, and each block
	 * is PPE_DRV_QOS_MEM_BLOCK_SIZE bytes in size. Therefore we divide the
	 * input parameters which are in bytes by PPE_DRV_QOS__MEM_BLOCK_SIZE to get
	 * the number of memory blocks to assign.
	 */
	res->q.qlimit = q->limit / PPE_DRV_QOS_MEM_BLOCK_SIZE;
	res->q.min_th[PPE_DRV_QOS_QUEUE_COLOR_GREEN] = q->rap.min / PPE_DRV_QOS_MEM_BLOCK_SIZE;
	res->q.max_th[PPE_DRV_QOS_QUEUE_COLOR_GREEN] = q->rap.max / PPE_DRV_QOS_MEM_BLOCK_SIZE;

	if (ppe_qdisc_configure(pq, &prev_pq) < 0) {
		ppe_qdisc_warning("%px ppered %x configuration failed", sch, sch->handle);
		return -EINVAL;
	}

	/*
	 * Set this qdisc to be the default qdisc for enqueuing packets.
	 */
	if (q->set_default) {
		ppe_qdisc_set_default(pq);
		ppe_qdisc_info("%px ppered queue %x (qos_tag:%u) set as default", sch, sch->handle, q->pq.qos_tag);
	}

	return 0;
}

/*
 * ppe_red_init()
 *	Init the ppered qdisc
 */
static int ppe_red_init(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	struct ppe_qdisc *pq = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPERED_MAX + 1];
	struct tc_ppered_qopt *qopt;

	if (!opt) {
		return -EINVAL;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_red_policy, tb, TCA_PPERED_MAX, TCA_PPERED_PARMS, extack);
	if (!qopt) {
		return -EINVAL;
	}

	ppe_qdisc_info("%px Initializing red - type %d", sch, PPE_QDISC_NODE_TYPE_RED);
	ppe_red_reset(sch);

	if (ppe_qdisc_init(sch, pq, PPE_QDISC_NODE_TYPE_RED, 0, extack) < 0) {
		return -EINVAL;
	}

	ppe_qdisc_info("%px ppered initialized - handle %x parent %x", sch, sch->handle, sch->parent);
	if (ppe_red_change(sch, opt, extack) < 0) {
		ppe_red_destroy(sch);
		return -EINVAL;
	}

	return 0;
}

/*
 * ppe_red_dump()
 *	Dump the parameters of ppered to tc
 */
static int ppe_red_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct ppe_red_sched_data *q;
	struct nlattr *opts = NULL;
	struct tc_ppered_qopt opt;

	ppe_qdisc_info("%px ppered Dumping", sch);
	q = qdisc_priv(sch);
	if (q == NULL) {
		return -1;
	}

	opt.ecn = q->ecn;
	opt.limit = q->limit;
	opt.rap.min = q->rap.min;
	opt.rap.max = q->rap.max;
	opt.rap.exp_weight_factor = q->rap.exp_weight_factor;
	opt.rap.probability = q->rap.probability;

	opt.set_default = q->set_default;

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_PPERED_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_red_peek()
 *	Peeks the first packet in queue for this qdisc
 */
static struct sk_buff *ppe_red_peek(struct Qdisc *sch)
{
	ppe_qdisc_info("%px ppered Peeking", sch);
	return ppe_qdisc_peek(sch);
}

/*
 * Registration structure for ppe_red qdisc
 */
struct Qdisc_ops ppe_red_qdisc_ops __read_mostly = {
	.id		=	"ppered",
	.priv_size	=	sizeof(struct ppe_red_sched_data),
	.enqueue	=	ppe_red_enqueue,
	.dequeue	=	ppe_red_dequeue,
	.peek		=	ppe_red_peek,
	.init		=	ppe_red_init,
	.reset		=	ppe_red_reset,
	.destroy	=	ppe_red_destroy,
	.change		=	ppe_red_change,
	.dump		=	ppe_red_dump,
	.owner		=	THIS_MODULE,
};


