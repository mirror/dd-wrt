/*
 **************************************************************************
 * Copyright (c) 2014-2017, 2019-2020, The Linux Foundation. All rights reserved.
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

struct nss_tbl_sched_data {
	struct nss_qdisc nq;	/* Common base class for all nss qdiscs */
	u32 rate;		/* Limiting rate of TBL */
	u32 peakrate;		/* Maximum rate to control bursts */
	u32 burst;		/* Maximum allowed burst size */
	u32 mtu;		/* MTU of the interface attached to */
	struct Qdisc *qdisc;	/* Qdisc to which it is attached to */
};

static struct nla_policy nss_tbl_policy[TCA_NSSTBL_MAX + 1] = {
	[TCA_NSSTBL_PARMS] = { .len = sizeof(struct tc_nsstbl_qopt) },
};

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static int nss_tbl_enqueue(struct sk_buff *skb, struct Qdisc *sch)
#else
static int nss_tbl_enqueue(struct sk_buff *skb, struct Qdisc *sch,
				struct sk_buff **to_free)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	return nss_qdisc_enqueue(skb, sch);
#else
	return nss_qdisc_enqueue(skb, sch, to_free);
#endif
}

static struct sk_buff *nss_tbl_dequeue(struct Qdisc *sch)
{
	return nss_qdisc_dequeue(sch);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static unsigned int nss_tbl_drop(struct Qdisc *sch)
{
	return nss_qdisc_drop(sch);
}
#endif

static struct sk_buff *nss_tbl_peek(struct Qdisc *sch)
{
	return nss_qdisc_peek(sch);
}

static void nss_tbl_reset(struct Qdisc *sch)
{
	nss_qdisc_reset(sch);
}

static void nss_tbl_destroy(struct Qdisc *sch)
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	struct nss_qdisc *nq_child = (struct nss_qdisc *)qdisc_priv(q->qdisc);
	struct nss_if_msg nim;

	/*
	 * We must always detach our child node in NSS before destroying it.
	 * Also, we make sure we dont send down the command for noop qdiscs.
	 */
	if (q->qdisc != &noop_qdisc) {
		nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
		if (nss_qdisc_node_detach(&q->nq, nq_child, &nim,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			nss_qdisc_error("Failed to detach child %x from nss_tbl %x\n",
					q->qdisc->handle, q->nq.qos_tag);
			return;
		}
	}

	/*
	 * Now we can destroy our child qdisc
	 */
	 nss_qdisc_put(q->qdisc);

	/*
	 * Stop the polling of basic stats and destroy qdisc.
	 */
	nss_qdisc_stop_basic_stats_polling(&q->nq);
	nss_qdisc_destroy(&q->nq);
}

#if defined(NSS_QDISC_PPE_SUPPORT)
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_tbl_ppe_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int nss_tbl_ppe_change(struct Qdisc *sch, struct nlattr *opt, struct netlink_ext_ack *extack)
#endif
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	struct nss_qdisc *nq = &q->nq;
	struct nss_ppe_qdisc prev_npq;

	/*
	 * Save previous configuration for reset purpose
	 */
	if (nq->npq.is_configured) {
		prev_npq = nq->npq;
	}

	nq->npq.shaper_present = true;
	nq->npq.shaper.rate = q->rate;
	nq->npq.shaper.burst = q->burst;
	nq->npq.shaper.crate = q->rate;
	nq->npq.shaper.cburst = q->burst;
	nq->npq.shaper.overhead = 0;

	if (nss_ppe_configure(nq, &prev_npq) != 0) {
		nss_qdisc_warning("nss_tbl %x SSDK scheduler configuration failed\n", sch->handle);
		goto fail;
	}

	return 0;

fail:
	if (nq->npq.is_configured) {
		nss_qdisc_warning("nss_tbl %x SSDK scheduler configuration failed\n", sch->handle);
		return -EINVAL;
	}

	/*
	 * PPE qdisc config failed, try to initialize in NSS.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_ppe_fallback_to_nss(nq, opt)) {
#else
	if (nss_ppe_fallback_to_nss(nq, opt, extack)) {
#endif
	nss_qdisc_warning("nss_tbl %x fallback to nss failed\n", sch->handle);
		return -EINVAL;
	}

	return 0;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_tbl_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int nss_tbl_change(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
#endif
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSTBL_MAX + 1];
	struct tc_nsstbl_qopt *qopt;
	struct nss_if_msg nim;
	struct net_device *dev = qdisc_dev(sch);

	if (!opt) {
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_tbl_policy, tb, TCA_NSSTBL_MAX, TCA_NSSTBL_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_tbl_policy, tb, TCA_NSSTBL_MAX, TCA_NSSTBL_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	/*
	 * Set MTU if it wasn't specified explicitely
	 */
	if (!qopt->mtu) {
		qopt->mtu = psched_mtu(dev);
		nss_qdisc_info("MTU not provided for nss_tbl. Setting it to %s's default %u bytes\n", dev->name, qopt->mtu);
	}

	/*
	 * Burst size cannot be less than MTU
	 */
	if (qopt->burst < qopt->mtu) {
		nss_qdisc_error("Burst size: %u is less than the specified MTU: %u\n", qopt->burst, qopt->mtu);
		return -EINVAL;
	}

	/*
	 * Rate can be zero. Therefore we dont do a check on it.
	 */
	q->rate = qopt->rate;
	nss_qdisc_info("Rate = %u", qopt->rate);
	q->burst = qopt->burst;
	nss_qdisc_info("Burst = %u", qopt->burst);
	q->mtu = qopt->mtu;
	nss_qdisc_info("MTU = %u", qopt->mtu);
	q->peakrate = qopt->peakrate;
	nss_qdisc_info("Peak Rate = %u", qopt->peakrate);

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (q->nq.mode == NSS_QDISC_MODE_PPE) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
		if (nss_tbl_ppe_change(sch, opt) < 0) {
#else
		if (nss_tbl_ppe_change(sch, opt, extack) < 0) {
#endif
			nss_qdisc_warning("nss_tbl %x SSDK scheduler config failed\n", sch->handle);
			return -EINVAL;
		}
		return 0;
	}
#endif

	nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_cir.rate = q->rate;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_cir.burst = q->burst;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_cir.max_size = q->mtu;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_cir.short_circuit = false;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_pir.rate = q->peakrate;

	/*
	 * It is important to set these two parameters to be the same as MTU.
	 * This ensures bursts from CIR dont go above the specified peakrate.
	 */
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_pir.burst = q->mtu;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_pir.max_size = q->mtu;

	/*
	 * We can short circuit peakrate limiter if it is not being configured.
	 */
	if (q->peakrate) {
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_pir.short_circuit = false;
	} else {
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_param.lap_pir.short_circuit = true;
	}

	if (nss_qdisc_configure(&q->nq, &nim, NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
		return -EINVAL;
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_tbl_init(struct Qdisc *sch, struct nlattr *opt)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_tbl_init(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
#endif
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSTBL_MAX + 1];
	struct tc_nsstbl_qopt *qopt;

	if (!opt) {
		return -EINVAL;
	}

	q->qdisc = &noop_qdisc;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_tbl_policy, tb, TCA_NSSTBL_MAX, TCA_NSSTBL_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_tbl_policy, tb, TCA_NSSTBL_MAX, TCA_NSSTBL_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	if (nss_qdisc_init(sch, &q->nq, NSS_SHAPER_NODE_TYPE_TBL, 0, qopt->accel_mode, extack) < 0)
	{
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_tbl_change(sch, opt) < 0) {
#else
	if (nss_tbl_change(sch, opt, extack) < 0) {
#endif
		nss_qdisc_info("Failed to configure tbl\n");
		nss_qdisc_destroy(&q->nq);
		return -EINVAL;
	}

	/*
	 * Start the stats polling timer
	 */
	nss_qdisc_start_basic_stats_polling(&q->nq);

	return 0;
}

static int nss_tbl_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts = NULL;
	struct tc_nsstbl_qopt opt;

	opt.rate = q->rate;
	opt.peakrate = q->peakrate;
	opt.burst = q->burst;
	opt.mtu = q->mtu;
	opt.accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	nss_qdisc_info("Nsstbl dumping");

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_NSSTBL_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

static int nss_tbl_dump_class(struct Qdisc *sch, unsigned long cl,
			     struct sk_buff *skb, struct tcmsg *tcm)
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	nss_qdisc_info("Nsstbl dumping class");

	tcm->tcm_handle |= TC_H_MIN(1);
	tcm->tcm_info = q->qdisc->handle;

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_tbl_graft(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
			struct Qdisc **old)
#else
static int nss_tbl_graft(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
			struct Qdisc **old, struct netlink_ext_ack *extack)
#endif
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	struct nss_qdisc *nq_new = (struct nss_qdisc *)qdisc_priv(new);
	struct nss_if_msg nim_attach;
	struct nss_if_msg nim_detach;

	if (new == NULL)
		new = &noop_qdisc;

	sch_tree_lock(sch);
	*old = q->qdisc;
	sch_tree_unlock(sch);

	nss_qdisc_info("Grafting old: %px with new: %px\n", *old, new);
	if (*old != &noop_qdisc) {
		struct nss_qdisc *nq_old = (struct nss_qdisc *)qdisc_priv(*old);
		nss_qdisc_info("Detaching old: %px\n", *old);
		nim_detach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
		if (nss_qdisc_node_detach(&q->nq, nq_old, &nim_detach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			return -EINVAL;
		}
	}

	if (new != &noop_qdisc) {
		nss_qdisc_info("Attaching new: %px\n", new);
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.tbl_attach.child_qos_tag = nq_new->qos_tag;
		if (nss_qdisc_node_attach(&q->nq, nq_new, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			return -EINVAL;
		}
	}

	/*
	 * Replaced in NSS, now replace in Linux.
	 */
	nss_qdisc_replace(sch, new, &q->qdisc);

	nss_qdisc_info("Nsstbl grafted");

	return 0;
}

static struct Qdisc *nss_tbl_leaf(struct Qdisc *sch, unsigned long arg)
{
	struct nss_tbl_sched_data *q = qdisc_priv(sch);
	nss_qdisc_info("Nsstbl returns leaf");
	return q->qdisc;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static unsigned long nss_tbl_get(struct Qdisc *sch, u32 classid)
{
	return 1;
}

static void nss_tbl_put(struct Qdisc *sch, unsigned long arg)
{
}
#else
static unsigned long nss_tbl_search(struct Qdisc *sch, u32 classid)
{
	return 1;
}
#endif

static void nss_tbl_walk(struct Qdisc *sch, struct qdisc_walker *walker)
{
	nss_qdisc_info("Nsstbl walk called");
	if (!walker->stop) {
		if (walker->count >= walker->skip)
			if (walker->fn(sch, 1, walker) < 0) {
				walker->stop = 1;
				return;
			}
		walker->count++;
	}
}

const struct Qdisc_class_ops nss_tbl_class_ops = {
	.graft		=	nss_tbl_graft,
	.leaf		=	nss_tbl_leaf,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.get		=	nss_tbl_get,
	.put		=	nss_tbl_put,
#else
	.find       =   nss_tbl_search,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.tcf_chain	=	nss_qdisc_tcf_chain,
#else
	.tcf_block	=	nss_qdisc_tcf_block,
#endif
	.bind_tcf	=	nss_qdisc_tcf_bind,
	.unbind_tcf	=	nss_qdisc_tcf_unbind,
	.walk		=	nss_tbl_walk,
	.dump		=	nss_tbl_dump_class,
};

struct Qdisc_ops nss_tbl_qdisc_ops __read_mostly = {
	.next		=	NULL,
	.id		=	"nsstbl",
	.priv_size	=	sizeof(struct nss_tbl_sched_data),
	.cl_ops		=	&nss_tbl_class_ops,
	.enqueue	=	nss_tbl_enqueue,
	.dequeue	=	nss_tbl_dequeue,
	.peek		=	nss_tbl_peek,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		=	nss_tbl_drop,
#endif
	.init		=	nss_tbl_init,
	.reset		=	nss_tbl_reset,
	.destroy	=	nss_tbl_destroy,
	.change		=	nss_tbl_change,
	.dump		=	nss_tbl_dump,
	.owner		=	THIS_MODULE,
};
