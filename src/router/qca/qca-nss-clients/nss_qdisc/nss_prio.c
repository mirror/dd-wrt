/*
 **************************************************************************
 * Copyright (c) 2014-2017 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

#include "nss_qdisc.h"

/*
 * nssprio qdisc instance structure
 */
struct nss_prio_sched_data {
	struct nss_qdisc nq;	/* Common base class for all nss qdiscs */
	int bands;		/* Number of priority bands to use */
	struct Qdisc *queues[TCA_NSSPRIO_MAX_BANDS];
				/* Array of child qdisc holder */
};

/*
 * nssprio policy structure
 */
static struct nla_policy nss_prio_policy[TCA_NSSPRIO_MAX + 1] = {
	[TCA_NSSPRIO_PARMS] = { .len = sizeof(struct tc_nssprio_qopt) },
};

/*
 * nss_prio_enqueue()
 *	Enqueues a skb to nssprio qdisc.
 */
static int nss_prio_enqueue(struct sk_buff *skb, struct Qdisc *sch, struct sk_buff **to_free)
{
	return nss_qdisc_enqueue(skb, sch);
}

/*
 * nss_prio_dequeue()
 *	Dequeues a skb to nssprio qdisc.
 */
static struct sk_buff *nss_prio_dequeue(struct Qdisc *sch)
{
	return nss_qdisc_dequeue(sch);
}

/*
 * nss_prio_peek()
 *	Peeks the first packet in queue for this qdisc.
 */
static struct sk_buff *nss_prio_peek(struct Qdisc *sch)
{
	return nss_qdisc_peek(sch);
}

/*
 * nss_prio_reset()
 *	Reset the nssprio qdisc
 */
static void nss_prio_reset(struct Qdisc *sch)
{
	return nss_qdisc_reset(sch);
}

/*
 * nss_prio_destroy()
 *	Destroy the nssprio qdisc
 */
static void nss_prio_destroy(struct Qdisc *sch)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	struct nss_if_msg nim;
	int i;

	nss_qdisc_info("Destroying prio");

	/*
	 * Destroy all attached child nodes before destroying prio
	 */
	for (i = 0; i < q->bands; i++) {

		/*
		 * We always detach the shaper in NSS before destroying it.
		 * It is very important to check for noop qdisc since those dont
		 * exist in the NSS.
		 */
		if (q->queues[i] != &noop_qdisc) {
			struct nss_qdisc *nq_child = qdisc_priv(q->queues[i]);
			nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
			nim.msg.shaper_configure.config.msg.shaper_node_config.snc.prio_detach.priority = i;
			if (nss_qdisc_node_detach(&q->nq, nq_child, &nim,
					NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
				nss_qdisc_error("Failed to detach child in band %d from prio %x\n",
							i, q->nq.qos_tag);
				return;
			}
		}

		/*
		 * We can now destroy it
		 */
		qdisc_put(q->queues[i]);
	}

	/*
	 * Stop the polling of basic stats
	 */
	nss_qdisc_stop_basic_stats_polling(&q->nq);

	/*
	 * Destroy the qdisc in NSS
	 */
	nss_qdisc_destroy(&q->nq);
}

/*
 * nss_prio_get_max_bands()
 *	Function call to get max bamds supported
 */
static int nss_prio_get_max_bands(struct Qdisc *sch)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	q = qdisc_priv(sch);

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (q->nq.mode == NSS_QDISC_MODE_PPE) {
		return nss_ppe_get_max_prio_bands(&q->nq);
	}
#endif

	if (q->nq.mode == NSS_QDISC_MODE_NSS) {
		return TCA_NSSPRIO_MAX_BANDS;
	}

	return 0;
}

/*
 * nss_prio_change()
 *	Function call to configure the nssprio parameters
 */
static int nss_prio_change(struct Qdisc *sch, struct nlattr *opt, struct netlink_ext_ack *extack)
{
	struct nss_prio_sched_data *q;
	struct tc_nssprio_qopt *qopt;

	q = qdisc_priv(sch);

	/*
	 * Since nssprio can be created with no arguments, opt might be NULL
	 * (depending on the kernel version). This is still a valid create
	 * request.
	 */
	if (!opt) {

		/*
		 * If no parameter is passed, set it to the default value.
		 */
		sch_tree_lock(sch);
		q->bands = TCA_NSSPRIO_MAX_BANDS;
		sch_tree_unlock(sch);
		return 0;
	}

	qopt = nss_qdisc_qopt_get(opt, nss_prio_policy, TCA_NSSPRIO_MAX, TCA_NSSPRIO_PARMS);
	if (!qopt) {
		return -EINVAL;
	}

	if (qopt->bands > nss_prio_get_max_bands(sch)) {
		nss_qdisc_warning("nssprio (accel_mode %d) requires max bands to be %d\n",
				nss_qdisc_accel_mode_get(&q->nq), nss_prio_get_max_bands(sch));
		return -EINVAL;
	}

	sch_tree_lock(sch);
	q->bands = qopt->bands;
	sch_tree_unlock(sch);
	nss_qdisc_info("Bands = %u\n", qopt->bands);

	/*
	 * We do not pass on this information to NSS since
	 * it not required for operation of prio. This parameter
	 * is needed only for the proxy operation.
	 */

	return 0;
}

/*
 * nss_prio_init()
 *	Initializes the nssprio qdisc
 */
static int nss_prio_init(struct Qdisc *sch, struct nlattr *opt, struct netlink_ext_ack *extack)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	struct tc_nssprio_qopt *qopt;
	int i;
	unsigned int accel_mode;

	for (i = 0; i < TCA_NSSPRIO_MAX_BANDS; i++) {
		q->queues[i] = &noop_qdisc;
	}

	if (!opt) {
		accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else {
		qopt = nss_qdisc_qopt_get(opt, nss_prio_policy, TCA_NSSPRIO_MAX, TCA_NSSPRIO_PARMS);
		if (!qopt) {
			return -EINVAL;
		}
		accel_mode = qopt->accel_mode;
	}

	if (nss_qdisc_init(sch, extack, &q->nq, NSS_SHAPER_NODE_TYPE_PRIO, 0, accel_mode) < 0) {
		return -EINVAL;
	}

	nss_qdisc_info("Nssprio initialized - handle %x parent %x\n",
			sch->handle, sch->parent);

	if (nss_prio_change(sch, opt, NULL) < 0) {
		nss_qdisc_destroy(&q->nq);
		return -EINVAL;
	}

	/*
	 * Start the stats polling timer
	 */
	nss_qdisc_start_basic_stats_polling(&q->nq);
	return 0;
}

/*
 * nss_prio_dump()
 *	Dump the parameters of nssprio
 */
static int nss_prio_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts = NULL;
	struct tc_nssprio_qopt qopt;

	nss_qdisc_info("Nssprio dumping");
	qopt.bands = q->bands;
	qopt.accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	opts = nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_NSSPRIO_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * nss_prio_graft()
 *	Replaces existing child qdisc with the new qdisc that is passed.
 */
static int nss_prio_graft(struct Qdisc *sch, unsigned long arg,
				struct Qdisc *new, struct Qdisc **old, struct netlink_ext_ack *extack)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	struct nss_qdisc *nq_new = qdisc_priv(new);
	uint32_t band = (uint32_t)(arg - 1);
	struct nss_if_msg nim_attach;
	struct nss_if_msg nim_detach;

	nss_qdisc_info("Grafting band %u, available bands %u\n", band, q->bands);

	if (new == NULL)
		new = &noop_qdisc;

	if (band > q->bands)
		return -EINVAL;

	sch_tree_lock(sch);
	*old = q->queues[band];
	sch_tree_unlock(sch);

	nss_qdisc_info("Grafting old: %p with new: %p\n", *old, new);
	if (*old != &noop_qdisc) {
		struct nss_qdisc *nq_old = qdisc_priv(*old);
		nss_qdisc_info("Detaching old: %p\n", *old);
		nim_detach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;

		if (q->nq.mode == NSS_QDISC_MODE_NSS) {
			nim_detach.msg.shaper_configure.config.msg.shaper_node_config.snc.prio_detach.priority = band;
		}

		if (nss_qdisc_node_detach(&q->nq, nq_old, &nim_detach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			return -EINVAL;
		}
	}

	if (new != &noop_qdisc) {
		nss_qdisc_info("Attaching new child with qos tag: %x, priority: %u to "
				"qos_tag: %x\n", nq_new->qos_tag, band, q->nq.qos_tag);
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.prio_attach.child_qos_tag = nq_new->qos_tag;

#if defined(NSS_QDISC_PPE_SUPPORT)
		if (q->nq.mode == NSS_QDISC_MODE_PPE) {
			nq_new->npq.scheduler.priority = band;
		}
#endif

		if (q->nq.mode == NSS_QDISC_MODE_NSS) {
			nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.prio_attach.priority = band;
		}

		if (nss_qdisc_node_attach(&q->nq, nq_new, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			return -EINVAL;
		}
	}

	/*
	 * Replaced in NSS, now replace in Linux.
	 */
	nss_qdisc_replace(sch, new, &q->queues[band]);

	nss_qdisc_info("Nssprio grafted");

	return 0;
}

/*
 * nss_prio_leaf_class()
 *	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *nss_prio_leaf(struct Qdisc *sch, unsigned long arg)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	uint32_t band = (uint32_t)(arg - 1);

	nss_qdisc_info("Nssprio returns leaf\n");

	if (band > q->bands)
		return NULL;

	return q->queues[band];
}

/*
 * nss_prio_get()
 *	Returns the band if provided the classid.
 */
static unsigned long nss_prio_get(struct Qdisc *sch, u32 classid)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	unsigned long band = TC_H_MIN(classid);

	nss_qdisc_info("Inside get. Handle - %x Classid - %x Band %lu Available band %u\n", sch->handle, classid, band, q->bands);

	if (band > q->bands)
		return 0;

	return band;
}

/*
 * nss_prio_walk()
 *	Walks the priority band.
 */
static void nss_prio_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
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
	nss_qdisc_info("Nssprio walk called\n");
}

/*
 * nss_prio_dump_class()
 * 	Dumps all configurable parameters pertaining to this class.
 */
static int nss_prio_dump_class(struct Qdisc *sch, unsigned long cl,
			     struct sk_buff *skb, struct tcmsg *tcm)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);

	tcm->tcm_handle |= TC_H_MIN(cl);
	tcm->tcm_info = q->queues[cl - 1]->handle;

	nss_qdisc_info("Nssprio dumping class\n");
	return 0;
}

/*
 * nss_prio_dump_class_stats()
 *	Dumps class statistics.
 */
static int nss_prio_dump_class_stats(struct Qdisc *sch, unsigned long cl,
			     	    struct gnet_dump *d)
{
	struct nss_prio_sched_data *q = qdisc_priv(sch);
	struct Qdisc *cl_q;

	cl_q = q->queues[cl - 1];
	cl_q->qstats.qlen = cl_q->q.qlen;

	if (nss_qdisc_gnet_stats_copy_basic(d, &cl_q->bstats) < 0 ||
			nss_qdisc_gnet_stats_copy_queue(d, &cl_q->qstats) < 0)
		return -1;

	nss_qdisc_info("Nssprio dumping class stats\n");
	return 0;
}

/*
 * Registration structure for nssprio class
 */
const struct Qdisc_class_ops nss_prio_class_ops = {
	.graft		=	nss_prio_graft,
	.leaf		=	nss_prio_leaf,
	.find		=	nss_prio_get,
	.walk		=	nss_prio_walk,
	.dump		=	nss_prio_dump_class,
	.dump_stats	=	nss_prio_dump_class_stats,
};

/*
 * Registration structure for nssprio qdisc
 */
struct Qdisc_ops nss_prio_qdisc_ops __read_mostly = {
	.next		=	NULL,
	.id		=	"nssprio",
	.priv_size	=	sizeof(struct nss_prio_sched_data),
	.cl_ops		=	&nss_prio_class_ops,
	.enqueue	=	nss_prio_enqueue,
	.dequeue	=	nss_prio_dequeue,
	.peek		=	nss_prio_peek,
	.init		=	nss_prio_init,
	.reset		=	nss_prio_reset,
	.destroy	=	nss_prio_destroy,
	.change		=	nss_prio_change,
	.dump		=	nss_prio_dump,
	.owner		=	THIS_MODULE,
};
