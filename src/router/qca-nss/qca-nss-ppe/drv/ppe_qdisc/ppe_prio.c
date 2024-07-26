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
 * ppeprio qdisc instance structure
 */
struct ppe_prio_sched_data {
	struct ppe_qdisc pq;	/* Common base class for all nss qdiscs */
	int bands;		/* Number of priority bands to use */
	struct Qdisc *queues[TCA_PPEPRIO_MAX_BANDS];
				/* Array of child qdisc holder */
};

/*
 * ppeprio policy structure
 */
static struct nla_policy ppe_prio_policy[TCA_PPEPRIO_MAX + 1] = {
	[TCA_PPEPRIO_PARMS] = { .len = sizeof(struct tc_ppeprio_qopt) },
};

/*
 * ppe_prio_enqueue()
 *	Epqueues a skb to ppeprio qdisc.
 */
static int ppe_prio_enqueue(struct sk_buff *skb, struct Qdisc *sch,
			struct sk_buff **to_free)
{
	return ppe_qdisc_enqueue(skb, sch, to_free);
}

/*
 * ppe_prio_dequeue()
 *	Dequeues a skb to ppeprio qdisc.
 */
static struct sk_buff *ppe_prio_dequeue(struct Qdisc *sch)
{
	return ppe_qdisc_dequeue(sch);
}

/*
 * ppe_prio_peek()
 *	Peeks the first packet in queue for this qdisc.
 */
static struct sk_buff *ppe_prio_peek(struct Qdisc *sch)
{
	return ppe_qdisc_peek(sch);
}

/*
 * ppe_prio_reset()
 *	Reset the ppeprio qdisc
 */
static void ppe_prio_reset(struct Qdisc *sch)
{
	return ppe_qdisc_reset(sch);
}

/*
 * ppe_prio_destroy()
 *	Destroy the ppeprio qdisc
 */
static void ppe_prio_destroy(struct Qdisc *sch)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	int i;

	ppe_qdisc_info("Destroying prio");

	/*
	 * Destroy all attached child nodes before destroying prio
	 */
	for (i = 0; i < q->bands; i++) {

		/*
		 * It is very important to check for noop qdisc since PRIO
		 * qdisc does not exist in PPE.
		 */
		if (q->queues[i] != &noop_qdisc) {
			struct ppe_qdisc *pq_child = qdisc_priv(q->queues[i]);

			ppe_qdisc_node_detach(&q->pq, pq_child);
		}

		/*
		 * We can now destroy it
		 */
		qdisc_put(q->queues[i]);
	}

	ppe_qdisc_destroy(&q->pq);
}

/*
 * ppe_prio_change()
 *	Function call to configure the ppeprio parameters
 */
static int ppe_prio_change(struct Qdisc *sch, struct nlattr *opt,
			struct netlink_ext_ack *extack)
{
	struct nlattr *tb[TCA_PPEPRIO_MAX + 1];
	struct ppe_prio_sched_data *q;
	struct tc_ppeprio_qopt *qopt;

	q = qdisc_priv(sch);

	/*
	 * Since ppeprio can be created with no arguments, opt might be NULL
	 * (depending on the kernel version). This is still a valid create
	 * request.
	 */
	if (!opt) {

		/*
		 * If no parameter is passed, set it to the default value.
		 */
		sch_tree_lock(sch);
		q->bands = TCA_PPEPRIO_MAX_BANDS;
		sch_tree_unlock(sch);
		return 0;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_prio_policy, tb, TCA_PPEPRIO_MAX, TCA_PPEPRIO_PARMS, extack);
	if (!qopt) {
		return -EINVAL;
	}

	if (qopt->bands > ppe_qdisc_get_max_prio_bands(&q->pq)) {
		ppe_qdisc_warning("ppeprio requires max bands to be %d",
			ppe_qdisc_get_max_prio_bands(&q->pq));
		return -EINVAL;
	}

	sch_tree_lock(sch);
	q->bands = qopt->bands;
	sch_tree_unlock(sch);
	ppe_qdisc_info("%x ppeprio bands = %u", sch->handle, qopt->bands);

	return 0;
}

/*
 * ppe_prio_init()
 *	Initializes the ppeprio qdisc
 */
static int ppe_prio_init(struct Qdisc *sch, struct nlattr *opt,
			struct netlink_ext_ack *extack)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPEPRIO_MAX + 1];
	struct tc_ppeprio_qopt *qopt;
	int i;

	for (i = 0; i < TCA_PPEPRIO_MAX_BANDS; i++) {
		q->queues[i] = &noop_qdisc;
	}

	if (!opt) {
		return -EINVAL;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_prio_policy, tb, TCA_PPEPRIO_MAX, TCA_PPEPRIO_PARMS, extack);
	if (!qopt) {
		return -EINVAL;
	}

	if (ppe_qdisc_init(sch, &q->pq, PPE_QDISC_NODE_TYPE_PRIO, 0, extack) < 0) {
		return -EINVAL;
	}

	ppe_qdisc_info("%x ppeprio initialized parent %x",
		sch->handle, sch->parent);

	if (ppe_prio_change(sch, opt, extack) < 0) {
		ppe_qdisc_destroy(&q->pq);
		return -EINVAL;
	}

	return 0;
}

/*
 * ppe_prio_dump()
 *	Dump the parameters of ppeprio
 */
static int ppe_prio_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts = NULL;
	struct tc_ppeprio_qopt qopt;

	ppe_qdisc_info("%x ppeprio dumping", sch->handle);
	qopt.bands = q->bands;

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_PPEPRIO_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_prio_graft()
 *	Replaces existing child qdisc with the new qdisc that is passed.
 */
static int ppe_prio_graft(struct Qdisc *sch, unsigned long arg,
			struct Qdisc *new, struct Qdisc **old,
			struct netlink_ext_ack *extack)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	struct ppe_qdisc *pq_old, *pq_new;
	uint32_t band = (uint32_t)(arg - 1);

	ppe_qdisc_info("%x ppeprio grafting band %u, available bands %u", sch->handle, band, q->bands);

	if (!new)
		new = &noop_qdisc;

	if (band > q->bands)
		return -EINVAL;

	sch_tree_lock(sch);
	*old = q->queues[band];
	sch_tree_unlock(sch);

	ppe_qdisc_info("%x ppeprio grafting old: %px with new: %px", sch->handle, *old, new);
	if (*old != &noop_qdisc) {
		ppe_qdisc_info("%x ppeprio detaching old", sch->handle);
		pq_old = qdisc_priv(*old);
		ppe_qdisc_node_detach(&q->pq, pq_old);
	}

	if (new != &noop_qdisc) {
		if (!(new->flags & TCQ_F_NSS)) {
			ppe_qdisc_warning("non-ppe qdisc %px used along with ppe qdisc", new);
			return -EINVAL;
		}

		pq_new = qdisc_priv(new);
		pq_new->res.scheduler.priority = band;
		ppe_qdisc_info("%x ppeprio attaching new child with qos tag: %x, priority: %u",
			q->pq.qos_tag, pq_new->qos_tag, band);
		if (ppe_qdisc_node_attach(&q->pq, pq_new) < 0) {
			return -EINVAL;
		}
	}

	ppe_qdisc_replace(sch, new, &q->queues[band]);
	ppe_qdisc_info("%x ppeprio grafted", sch->handle);

	return 0;
}

/*
 * ppe_prio_leaf_class()
 *	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *ppe_prio_leaf(struct Qdisc *sch, unsigned long arg)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	uint32_t band = (uint32_t)(arg - 1);

	ppe_qdisc_info("%x ppeprio returns leaf", sch->handle);

	if (band > q->bands)
		return NULL;

	return q->queues[band];
}

/*
 * ppe_prio_search()
 *	Returns the band if provided the classid.
 */
static unsigned long ppe_prio_search(struct Qdisc *sch, u32 classid)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	unsigned long band = TC_H_MIN(classid);

	ppe_qdisc_info("%x ppeprio classid - %x band %lu available band %u",
			sch->handle, classid, band, q->bands);

	if (band > q->bands)
		return 0;

	return band;
}

/*
 * ppe_prio_walk()
 *	Walks the priority band.
 */
static void ppe_prio_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	int i;

	if (arg->stop)
		return;

	for (i = 0; i < q->bands; i++) {
		if (arg->count < arg->skip) {
			arg->count++;
			continue;
		}
		if (arg->fn(sch, i + 1, arg) < 0) {
			arg->stop = 1;
			break;
		}
		arg->count++;
	}
	ppe_qdisc_info("%x ppeprio walk called", sch->handle);
}

/*
 * ppe_prio_dump_class()
 * 	Dumps all configurable parameters pertaining to this class.
 */
static int ppe_prio_dump_class(struct Qdisc *sch, unsigned long cl,
			struct sk_buff *skb, struct tcmsg *tcm)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);

	tcm->tcm_handle |= TC_H_MIN(cl);
	tcm->tcm_info = q->queues[cl - 1]->handle;

	ppe_qdisc_info("%x ppeprio dumping class", sch->handle);
	return 0;
}

/*
 * ppe_prio_dump_class_stats()
 *	Dumps class statistics.
 */
static int ppe_prio_dump_class_stats(struct Qdisc *sch, unsigned long cl,
			struct gnet_dump *d)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	struct Qdisc *cl_q;

	cl_q = q->queues[cl - 1];
	cl_q->qstats.qlen = cl_q->q.qlen;

	if (ppe_qdisc_gnet_stats_copy_basic(sch, d, &cl_q->bstats) < 0 ||
		ppe_qdisc_gnet_stats_copy_queue(d, &cl_q->qstats) < 0)
		return -1;

	ppe_qdisc_info("%x ppeprio dumping class stats", sch->handle);
	return 0;
}

/*
 * ppe_prio_band_qdisc_get()
 *	Get the qdisc attached to prio band
 */
struct ppe_qdisc *ppe_prio_band_qdisc_get(struct Qdisc *sch, u32 classid)
{
	struct ppe_prio_sched_data *q = qdisc_priv(sch);
	unsigned long band = TC_H_MIN(classid);

	if (!band || (band > TCA_PPEPRIO_MAX_BANDS)) {
		ppe_qdisc_warning("%x ppeprio invalid band", sch->handle);
		return NULL;
	}

	return qdisc_priv(q->queues[band - 1]);
}

/*
 * Registration structure for ppeprio class
 */
const struct Qdisc_class_ops ppe_prio_class_ops = {
	.graft		= ppe_prio_graft,
	.leaf		= ppe_prio_leaf,
	.find		= ppe_prio_search,
	.tcf_block	= ppe_qdisc_tcf_block,
	.bind_tcf	= ppe_qdisc_tcf_bind,
	.unbind_tcf	= ppe_qdisc_tcf_unbind,
	.walk		= ppe_prio_walk,
	.dump		= ppe_prio_dump_class,
	.dump_stats	= ppe_prio_dump_class_stats,
};

/*
 * Registration structure for ppeprio qdisc
 */
struct Qdisc_ops ppe_prio_qdisc_ops __read_mostly = {
	.next		= NULL,
	.id		= "ppeprio",
	.priv_size	= sizeof(struct ppe_prio_sched_data),
	.cl_ops		= &ppe_prio_class_ops,
	.enqueue	= ppe_prio_enqueue,
	.dequeue	= ppe_prio_dequeue,
	.peek		= ppe_prio_peek,
	.init		= ppe_prio_init,
	.reset		= ppe_prio_reset,
	.destroy	= ppe_prio_destroy,
	.change		= ppe_prio_change,
	.dump		= ppe_prio_dump,
	.owner		= THIS_MODULE,
};
