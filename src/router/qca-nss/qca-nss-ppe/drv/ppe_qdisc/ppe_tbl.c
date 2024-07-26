/*
 * Copyright (c) 2014-2017, 2019-2021, The Linux Foundation. All rights reserved.
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

/*
 * ppetbl private qdisc structure
 */
struct ppe_tbl_sched_data {
	struct ppe_qdisc pq;	/* Common base class for all ppe qdiscs */
	u32 rate;		/* Limiting rate of TBL */
	u32 burst;		/* Maximum allowed burst size */
	u32 mtu;		/* MTU of the interface attached to */
	struct Qdisc *qdisc;	/* Qdisc to which it is attached to */
};

/*
 * ppetbl policy structure
 */
static struct nla_policy ppe_tbl_policy[TCA_PPETBL_MAX + 1] = {
	[TCA_PPETBL_PARMS] = { .len = sizeof(struct tc_ppetbl_qopt) },
};

/*
 * ppe_tbl_enqueue()
 * 	Enqueue API for ppetbl qdisc
 */
static int ppe_tbl_enqueue(struct sk_buff *skb, struct Qdisc *sch,
		struct sk_buff **to_free)
{
	return ppe_qdisc_enqueue(skb, sch, to_free);
}

/*
 * ppe_tbl_dequeue()
 * 	Dequeue API for ppetbl qdisc
 */
static struct sk_buff *ppe_tbl_dequeue(struct Qdisc *sch)
{
	return ppe_qdisc_dequeue(sch);
}

/*
 * ppe_tbl_peek()
 * 	Peeks the first packet in queue for this qdisc
 */
static struct sk_buff *ppe_tbl_peek(struct Qdisc *sch)
{
	return ppe_qdisc_peek(sch);
}

/*
 * ppe_tbl_reset()
 * 	Reset the ppetbl qdisc
 */
static void ppe_tbl_reset(struct Qdisc *sch)
{
	ppe_qdisc_reset(sch);
}

/*
 * ppe_tbl_destroy()
 * 	Destroy the ppetbl qdisc
 */
static void ppe_tbl_destroy(struct Qdisc *sch)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);
	struct ppe_qdisc *pq_child = (struct ppe_qdisc *)qdisc_priv(q->qdisc);

	/*
	 * We must always detach our child node in PPE before destroying it.
	 * Also, we make sure we dont send down the command for noop qdiscs.
	 */
	if (q->qdisc != &noop_qdisc) {
		ppe_qdisc_node_detach(&q->pq, pq_child);
	}

	/*
	 * Now we can destroy our child qdisc
	 */
	qdisc_put(q->qdisc);
	ppe_qdisc_destroy(&q->pq);
}

/*
 * ppe_tbl_change()
 * 	Function call to configure the ppetbl parameters
 */
static int ppe_tbl_change(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPETBL_MAX + 1];
	struct tc_ppetbl_qopt *qopt;
	struct net_device *dev = qdisc_dev(sch);
	struct ppe_qdisc *pq = &q->pq;
	struct ppe_qdisc prev_pq;
	struct ppe_drv_qos_res *res = &pq->res;

	if (!opt) {
		return -EINVAL;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_tbl_policy, tb, TCA_PPETBL_MAX, TCA_PPETBL_PARMS, extack);

	if (!qopt) {
		return -EINVAL;
	}

	/*
	 * Set MTU if it wasn't specified explicitly
	 */
	if (!qopt->mtu) {
		qopt->mtu = psched_mtu(dev);
		ppe_qdisc_info("%x MTU not provided for ppe_tbl. Setting it to %s's default %u bytes", sch->handle, dev->name, qopt->mtu);
	}

	/*
	 * Burst size cannot be less than MTU
	 */
	if (qopt->burst < qopt->mtu) {
		ppe_qdisc_warning("%x Burst size: %u is less than the specified MTU: %u", sch->handle, qopt->burst, qopt->mtu);
		return -EINVAL;
	}

	/*
	 * Rate can be zero. Therefore we dont do a check on it.
	 */
	q->rate = qopt->rate;
	ppe_qdisc_info("%x Rate = %u", sch->handle, qopt->rate);
	q->burst = qopt->burst;
	ppe_qdisc_info("%x Burst = %u", sch->handle, qopt->burst);
	q->mtu = qopt->mtu;
	ppe_qdisc_info("%x MTU = %u", sch->handle, qopt->mtu);

	/*
	 * Save previous configuration for reset purpose
	 */
	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CONFIGURED)) {
		prev_pq = q->pq;
	}

	res->shaper.rate = q->rate;
	res->shaper.burst = q->burst;
	res->shaper.crate = q->rate;
	res->shaper.cburst = q->burst;
	res->shaper.overhead = 0;
	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_SHAPER_VALID);

	if (ppe_qdisc_configure(pq, &prev_pq) < 0) {
		ppe_qdisc_warning("%x ppe_tbl SSDK scheduler configuration failed", sch->handle);
		return -EINVAL;
	}

	return 0;
}

/*
 * ppe_tbl_init()
 * 	Init the ppetbl qdisc
 */
static int ppe_tbl_init(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPETBL_MAX + 1];
	struct tc_ppetbl_qopt *qopt;

	if (!opt) {
		return -EINVAL;
	}

	q->qdisc = &noop_qdisc;

	qopt = ppe_qdisc_qopt_get(opt, ppe_tbl_policy, tb, TCA_PPETBL_MAX, TCA_PPETBL_PARMS, extack);

	if (!qopt) {
		return -EINVAL;
	}

	if (ppe_qdisc_init(sch, &q->pq, PPE_QDISC_NODE_TYPE_TBL, 0, extack) < 0) {
		return -EINVAL;
	}

	if (ppe_tbl_change(sch, opt, extack) < 0) {
		ppe_qdisc_info("%x Failed to configure tbl", sch->handle);
		ppe_qdisc_destroy(&q->pq);
		return -EINVAL;
	}

	return 0;
}

/*
 * ppe_tbl_dump()
 * 	Dump the parameters of ppetbl to tc
 */
static int ppe_tbl_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts = NULL;
	struct tc_ppetbl_qopt opt;

	opt.rate = q->rate;
	opt.burst = q->burst;
	opt.mtu = q->mtu;

	ppe_qdisc_info("%x ppetbl dumping", sch->handle);

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (!opts || nla_put(skb, TCA_PPETBL_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_tbl_dump_class()
 * 	Dumps all configurable parameters pertaining to this class.
 */
static int ppe_tbl_dump_class(struct Qdisc *sch, unsigned long cl,
		struct sk_buff *skb, struct tcmsg *tcm)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);

	ppe_qdisc_info("%x ppetbl dumping class", sch->handle);
	tcm->tcm_handle |= TC_H_MIN(1);
	tcm->tcm_info = q->qdisc->handle;

	return 0;
}

/*
 * ppe_tbl_graft_class()
 * 	Replaces the qdisc attached to the provided class.
 */
static int ppe_tbl_graft(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
		struct Qdisc **old, struct netlink_ext_ack *extack)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);
	struct ppe_qdisc *pq_new, *pq_old;

	if (!new)
		new = &noop_qdisc;

	sch_tree_lock(sch);
	*old = q->qdisc;
	sch_tree_unlock(sch);

	ppe_qdisc_info("%x Grafting old: %px with new: %px", sch->handle, *old, new);
	if (*old != &noop_qdisc) {
		pq_old = (struct ppe_qdisc *)qdisc_priv(*old);

		ppe_qdisc_info("%x Detaching old: %px", sch->handle, *old);
		ppe_qdisc_node_detach(&q->pq, pq_old);
	}

	if (new != &noop_qdisc) {
		ppe_qdisc_info("%x Attaching new: %px", sch->handle, new);
		if (!(new->flags & TCQ_F_NSS)) {
			ppe_qdisc_warning("%x non-ppe qdisc %px used along with ppe qdisc", sch->handle, new);
			return -EINVAL;
		}
		pq_new = (struct ppe_qdisc *)qdisc_priv(new);
		ppe_qdisc_node_attach(&q->pq, pq_new);
	}

	/*
	 * Replaced in PPE, now replace in Linux.
	 */
	ppe_qdisc_replace(sch, new, &q->qdisc);

	ppe_qdisc_info("%x ppetbl grafted", sch->handle);

	return 0;
}

/*
 * ppe_tbl_leaf_class()
 * 	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *ppe_tbl_leaf(struct Qdisc *sch, unsigned long arg)
{
	struct ppe_tbl_sched_data *q = qdisc_priv(sch);

	ppe_qdisc_info("%x ppetbl returns leaf", sch->handle);
	return q->qdisc;
}

/*
 * ppe_tbl_search()
 * 	Dummy function registered with find operation of Qdisc
 */
static unsigned long ppe_tbl_search(struct Qdisc *sch, u32 classid)
{
	return 1;
}

/*
 * ppe_tbl_walk()
 * 	Used to walk the tree.
 */
static void ppe_tbl_walk(struct Qdisc *sch, struct qdisc_walker *walker)
{
	ppe_qdisc_info("%x ppetbl walk called", sch->handle);
	if (!walker->stop) {
		if (walker->count >= walker->skip)
			if (walker->fn(sch, 1, walker) < 0) {
				walker->stop = 1;
				return;
			}
		walker->count++;
	}
}

/*
 * Registration structure for ppe_tbl class
 */
const struct Qdisc_class_ops ppe_tbl_class_ops = {
	.graft		=	ppe_tbl_graft,
	.leaf		=	ppe_tbl_leaf,
	.find       	=   	ppe_tbl_search,
	.tcf_block	=	ppe_qdisc_tcf_block,
	.bind_tcf	=	ppe_qdisc_tcf_bind,
	.unbind_tcf	=	ppe_qdisc_tcf_unbind,
	.walk		=	ppe_tbl_walk,
	.dump		=	ppe_tbl_dump_class,
};

/*
 * Registration structure for ppe_tbl qdisc
 */
struct Qdisc_ops ppe_tbl_qdisc_ops __read_mostly = {
	.next		=	NULL,
	.id		=	"ppetbl",
	.priv_size	=	sizeof(struct ppe_tbl_sched_data),
	.cl_ops		=	&ppe_tbl_class_ops,
	.enqueue	=	ppe_tbl_enqueue,
	.dequeue	=	ppe_tbl_dequeue,
	.peek		=	ppe_tbl_peek,
	.init		=	ppe_tbl_init,
	.reset		=	ppe_tbl_reset,
	.destroy	=	ppe_tbl_destroy,
	.change		=	ppe_tbl_change,
	.dump		=	ppe_tbl_dump,
	.owner		=	THIS_MODULE,
};
