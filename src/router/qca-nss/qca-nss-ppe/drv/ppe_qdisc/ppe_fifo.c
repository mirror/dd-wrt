/*
 * Copyright (c) 2014, 2016-2017, 2020-2021, The Linux Foundation. All rights reserved.
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
#include "ppe_fifo.h"

/*
 * ppefifo instance structure
 */
struct ppe_fifo_sched_data {
	struct ppe_qdisc pq;	/* Common base class for all PPE qdiscs */
	u32 limit;		/* Queue length in packets */
				/* TODO: Support for queue length in bytes */
	u8 set_default;		/* Flag to set qdisc as default qdisc for enqueue */
	bool is_bfifo;		/* Flag to identify bfifo or pfifo */
};

/*
 * ppefifo policy strutcure
 */
static struct nla_policy ppe_fifo_policy[TCA_PPEFIFO_MAX + 1] = {
	[TCA_PPEFIFO_PARMS] = { .len = sizeof(struct tc_ppefifo_qopt) },
};

/*
 * ppe_fifo_enqueue()
 *	Enqueue API for ppefifo qdisc
 */
static int ppe_fifo_enqueue(struct sk_buff *skb, struct Qdisc *sch,
				struct sk_buff **to_free)
{
	return ppe_qdisc_enqueue(skb, sch, to_free);
}

/*
 * ppe_fifo_dequeue()
 *	Dequeue API for ppefifo qdisc
 */
static struct sk_buff *ppe_fifo_dequeue(struct Qdisc *sch)
{
	return ppe_qdisc_dequeue(sch);
}

/*
 * ppe_fifo_reset()
 *	Reset the ppefifo qdisc
 */
static void ppe_fifo_reset(struct Qdisc *sch)
{
	ppe_qdisc_info("%x: ppefifo resetting", sch->handle);
	ppe_qdisc_reset(sch);
}

/*
 * ppe_fifo_destroy()
 *	Destroy the ppefifo qdisc
 */
static void ppe_fifo_destroy(struct Qdisc *sch)
{
	struct ppe_qdisc *pq = (struct ppe_qdisc *)qdisc_priv(sch);

	ppe_qdisc_destroy(pq);
	ppe_qdisc_info("%px: ppefifo destroyed", sch);
}

/*
 * ppe_fifo_params_validate_and_save()
 *	Function to validate and save ppefifo parameters
 */
static int ppe_fifo_params_validate_and_save(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
	struct nlattr *tb[TCA_PPEFIFO_MAX + 1];
	struct tc_ppefifo_qopt *qopt;
	struct ppe_fifo_sched_data *q = qdisc_priv(sch);
	bool is_bfifo = (sch->ops == &ppe_bfifo_qdisc_ops);

	if (!opt) {
		return -EINVAL;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_fifo_policy, tb, TCA_PPEFIFO_MAX, TCA_PPEFIFO_PARMS, extack);
	if (!qopt) {
		ppe_qdisc_warning("%x: ppefifo invalid input", sch->handle);
		return -EINVAL;
	}

	if (!qopt->limit) {
		qopt->limit = qdisc_dev(sch)->tx_queue_len ? : 1;

		if (is_bfifo) {
			qopt->limit *= psched_mtu(qdisc_dev(sch));
		}
	}

	/*
	 * Required for basic stats display
	 */
	sch->limit = qopt->limit;
	q->limit = qopt->limit;
	q->set_default = qopt->set_default;
	q->is_bfifo = is_bfifo;

	ppe_qdisc_info("%x: ppefifo limit:%u set_default:%u", sch->handle, qopt->limit, qopt->set_default);
	return 0;
}

/*
 * ppe_fifo_change()
 *	Function call to configure the ppefifo parameters
 */
static int ppe_fifo_change(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
	struct ppe_fifo_sched_data *q = qdisc_priv(sch);
	struct ppe_qdisc *pq = &q->pq;
	struct ppe_drv_qos_res *res = &pq->res;
	struct ppe_qdisc prev_pq;

	if (ppe_fifo_params_validate_and_save(sch, opt, extack) < 0) {
		ppe_qdisc_warning("%px: ppefifo (qos_tag:%u) params validate and save failed", pq, pq->qos_tag);
		return -EINVAL;
	}

	/*
	 * Save previous configuration for reset purpose.
	 */
	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CONFIGURED)) {
		prev_pq = q->pq;
	}

	/*
	 * In case of bfifo, change the queue limit to number of blocks
	 * as PPE HW has memory in blocks of 256 bytes.
	 */
	if (q->is_bfifo) {
		res->q.qlimit = q->limit / PPE_DRV_QOS_MEM_BLOCK_SIZE;
	} else {
		res->q.qlimit = q->limit;
	}

	res->q.color_en = false;
	res->q.red_en = false;

	/*
	 * Set this qdisc to be the default qdisc for enqueuing packets.
	 */
	if (q->set_default) {
		ppe_qdisc_set_default(pq);
	}

	if (ppe_qdisc_configure(pq, &prev_pq) < 0) {
		ppe_qdisc_warning("%px: ppefifo (qos_tag:%u) configuration failed", pq, pq->qos_tag);
		return -EINVAL;
	}

	ppe_qdisc_info("%px: ppefifo (qos_tag:%u) set as default", pq, pq->qos_tag);
	return 0;
}

/*
 * ppe_fifo_init()
 *	Init the ppefifo qdisc
 */
static int ppe_fifo_init(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
	struct ppe_qdisc *pq = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPEFIFO_MAX + 1];
	struct tc_ppefifo_qopt *qopt;

	if (!opt) {
		return -EINVAL;
	}

	ppe_qdisc_info("%x: ppefifo initializing - type %d", sch->handle, PPE_QDISC_NODE_TYPE_FIFO);
	ppe_fifo_reset(sch);

	qopt = ppe_qdisc_qopt_get(opt, ppe_fifo_policy, tb, TCA_PPEFIFO_MAX, TCA_PPEFIFO_PARMS, extack);
	if (!qopt) {
		ppe_qdisc_warning("%x: ppefifo invalid input", sch->handle);
		return -EINVAL;
	}

	if (ppe_qdisc_init(sch, pq, PPE_QDISC_NODE_TYPE_FIFO, 0, extack) < 0) {
		ppe_qdisc_warning("%x: ppefifo init failed", sch->handle);
		return -EINVAL;
	}

	ppe_qdisc_info("%x: ppefifo initialized - parent %x", sch->handle, sch->parent);
	if (ppe_fifo_change(sch, opt, extack) < 0) {
		ppe_qdisc_destroy(pq);
		return -EINVAL;
	}

	return 0;
}

/*
 * ppe_fifo_dump()
 *	Dump the parameters of ppefifo to tc
 */
static int ppe_fifo_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct ppe_fifo_sched_data *q;
	struct nlattr *opts = NULL;
	struct tc_ppefifo_qopt opt;

	ppe_qdisc_info("%x: ppefifo dumping", sch->handle);

	q = qdisc_priv(sch);
	if (!q) {
		ppe_qdisc_warning("%x: ppefifo qdisc invalid", sch->handle);
		return -1;
	}

	opt.limit = q->limit;
	opt.set_default = q->set_default;

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (!opts) {
		goto nla_put_failure;
	}

	if (nla_put(skb, TCA_PPEFIFO_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_fifo_peek()
 *	Peeks the first packet in queue for this qdisc
 */
static struct sk_buff *ppe_fifo_peek(struct Qdisc *sch)
{
	ppe_qdisc_info("%x: ppefifo peeking", sch->handle);
	return ppe_qdisc_peek(sch);
}

/*
 * Registration structure for ppepfifo qdisc
 */
struct Qdisc_ops ppe_pfifo_qdisc_ops __read_mostly = {
	.id		=	"ppepfifo",
	.priv_size	=	sizeof(struct ppe_fifo_sched_data),
	.enqueue	=	ppe_fifo_enqueue,
	.dequeue	=	ppe_fifo_dequeue,
	.peek		=	ppe_fifo_peek,
	.init		=	ppe_fifo_init,
	.reset		=	ppe_fifo_reset,
	.destroy	=	ppe_fifo_destroy,
	.change		=	ppe_fifo_change,
	.dump		=	ppe_fifo_dump,
	.owner		=	THIS_MODULE,
};

/*
 * Registration structure for ppebfifo qdisc
 */
struct Qdisc_ops ppe_bfifo_qdisc_ops __read_mostly = {
	.id		=	"ppebfifo",
	.priv_size	=	sizeof(struct ppe_fifo_sched_data),
	.enqueue	=	ppe_fifo_enqueue,
	.dequeue	=	ppe_fifo_dequeue,
	.peek		=	ppe_fifo_peek,
	.init		=	ppe_fifo_init,
	.reset		=	ppe_fifo_reset,
	.destroy	=	ppe_fifo_destroy,
	.change		=	ppe_fifo_change,
	.dump		=	ppe_fifo_dump,
	.owner		=	THIS_MODULE,
};
