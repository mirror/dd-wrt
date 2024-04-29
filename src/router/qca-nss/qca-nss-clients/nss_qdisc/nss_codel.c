/*
 **************************************************************************
 * Copyright (c) 2014, 2016-2018, 2020,  The Linux Foundation. All rights reserved.
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
#include "nss_codel.h"

/*
 * Default number of flow queues used (in fq_codel) if user doesn't specify a value.
 */
#define NSS_CODEL_DEFAULT_FQ_COUNT 1024

/*
 * Codel stats structure.
 */
struct nss_codel_stats_sq {
	u32 peak_queue_delay;	/* Peak delay experienced by a dequeued packet */
	u32 peak_drop_delay;	/* Peak delay experienced by a packet that is dropped */
};

/*
 * Codel floq queue stats structure.
 */
struct nss_codel_stats_fq {
        u64 ecn_mark_cnt;	/* Number of packets marked with ECN */
        u64 new_flow_cnt;	/* Total number of new flows seen. */
	u64 drop_overlimit;	/* Number of overlimit drops */
	u32 new_flows_len;	/* Current number of new flows */
	u32 old_flows_len;	/* Current number of old flows */
	u32 maxpacket;		/* Largest packet seen so far */
};

/*
 * Codel instance structure.
 */
struct nss_codel_sched_data {
	struct nss_qdisc nq;	/* Common base class for all nss qdiscs */
	u32 target;		/* Acceptable value of queue delay */
	u32 limit;		/* Length of queue */
	u32 interval;		/* Monitoring interval */
	u32 flows;		/* Number of flow buckets */
	u32 quantum;		/* Weight (in bytes) used for DRR of flow buckets */
	unsigned long flow_queue_mem;
				/* Pointer to flow queue memory */
	dma_addr_t dma_mapped_mem ;
				/* Mapped memory address */
	u32 flow_queue_sz;	/* Size of a flow queue in firmware (bytes) */
	u8 ecn;			/* 0 - disable ECN, 1 - enable ECN */
	u8 set_default;		/* Flag to set qdisc as default qdisc for enqueue */
	struct nss_codel_stats_sq sq_stats;
				/* Contains single queue codel stats */
	struct nss_codel_stats_fq fq_stats;
				/* Contains flow queue codel stats */
};

/*
 * Codel netlink policy structure.
 */
static struct nla_policy nss_codel_policy[TCA_NSSCODEL_MAX + 1] = {
	[TCA_NSSCODEL_PARMS] = { .len = sizeof(struct tc_nsscodel_qopt) },
};

/*
 * nss_codel_enqueue()
 *	Enqueue a packet into nss_codel queue in NSS firmware (bounce).
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static int nss_codel_enqueue(struct sk_buff *skb, struct Qdisc *sch)
#else
static int nss_codel_enqueue(struct sk_buff *skb, struct Qdisc *sch,
				struct sk_buff **to_free)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	return nss_qdisc_enqueue(skb, sch);
#else
	return nss_qdisc_enqueue(skb, sch, to_free);
#endif
}

/*
 * nss_codel_dequeue()
 *	Dequeue a packet from the bounce complete queue (use when bouncing packets).
 */
static struct sk_buff *nss_codel_dequeue(struct Qdisc *sch)
{
	return nss_qdisc_dequeue(sch);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
/*
 * nss_codel_drop()
 *	Drops a packet from the bounce complete queue.
 *
 * Note: this does not drop packets from the NSS queues.
 */
static unsigned int nss_codel_drop(struct Qdisc *sch)
{
	return nss_qdisc_drop(sch);
}
#endif

/*
 * nss_codel_reset()
 *	Resets nss_codel qdisc.
 */
static void nss_codel_reset(struct Qdisc *sch)
{
	nss_qdisc_info("nss_codel resetting!");
	nss_qdisc_reset(sch);
}

/*
 * nss_codel_flow_queues_free()
 *	Free memory allocated for the flow queues (used in fq_codel mode).
 */
static void nss_codel_flow_queues_free(struct nss_codel_sched_data *q)
{
	if (!q->flow_queue_mem) {
		return;
	}

	/*
	 * If DMA mapping was successful, unmap it.
	 */
	if (!dma_mapping_error(nss_shaper_get_dev(), q->dma_mapped_mem)) {
		nss_qdisc_trace("Unmapping flow queue memory\n");
		dma_unmap_single(nss_shaper_get_dev(), q->dma_mapped_mem, q->flow_queue_sz * q->flows, DMA_FROM_DEVICE);
	}

	free_pages(q->flow_queue_mem, get_order(q->flow_queue_sz * q->flows));
	q->flow_queue_mem = 0;
	q->dma_mapped_mem = (dma_addr_t)0;

	nss_qdisc_trace("Flow queues freed\n");
}

/*
 * nss_codel_destroy()
 *	Destroys the nss_codel qdisc.
 */
static void nss_codel_destroy(struct Qdisc *sch)
{
	struct nss_codel_sched_data *q = qdisc_priv(sch);

	/*
	 * Stop the polling of basic stats
	 */
	nss_qdisc_stop_basic_stats_polling(&q->nq);
	nss_qdisc_destroy(&q->nq);
	nss_codel_flow_queues_free(q);
	nss_qdisc_info("nss_codel destroyed");
}

/*
 * nss_codel_stats_callback()
 *	Invoked by the nss_qdisc infrastructure on receiving stats update from firmware.
 */
static void nss_codel_stats_callback(struct nss_qdisc *nq, struct nss_shaper_node_stats_response *response)
{
	struct nss_codel_sched_data *q = (struct nss_codel_sched_data *)nq;

	if (!q->flows) {
		q->sq_stats.peak_queue_delay = response->per_sn_stats.codel.sq.packet_latency_peak_msec_dequeued;
		q->sq_stats.peak_drop_delay = response->per_sn_stats.codel.sq.packet_latency_peak_msec_dropped;
		return;
	}

	q->fq_stats.new_flows_len = response->per_sn_stats.codel.fq.new_flows_len;
	q->fq_stats.old_flows_len = response->per_sn_stats.codel.fq.old_flows_len;
	q->fq_stats.maxpacket = response->per_sn_stats.codel.fq.maxpacket;
	q->fq_stats.drop_overlimit += response->sn_stats.delta.enqueued_packets_dropped;
	q->fq_stats.ecn_mark_cnt += response->per_sn_stats.codel.fq.delta.ecn_mark_cnt;
	q->fq_stats.new_flow_cnt += response->per_sn_stats.codel.fq.delta.new_flow_cnt;
}

/*
 * nss_codel_configure_callback()
 *	Invoked by the nss_qdisc infrastructure on receiving a response for a configure msg.
 */
static void nss_codel_configure_callback(struct nss_qdisc *nq, struct nss_shaper_configure *nsc)
{
	struct nss_codel_sched_data *q;

	/*
	 * Currently only care about firmware response for the memory size
	 * get message. The rest we ignore.
	 */
	nss_qdisc_trace("Callback for request %d\n", nsc->request_type);
	if (nsc->request_type != NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_MEM_REQ) {
		return;
	}

	if (nsc->response_type != NSS_SHAPER_RESPONSE_TYPE_SUCCESS) {
		return;
	}

	q = (struct nss_codel_sched_data *)nq;
	q->flow_queue_sz = nsc->msg.shaper_node_config.snc.codel_mem_req.mem_req;
	nss_qdisc_info("Flow queue size %u bytes\n", q->flow_queue_sz);
}

/*
 * nss_codel_mem_sz_get()
 *	Sends a message to nss_codel queue (in FW) requesting per flow-queue mem size.
 *
 * The response for this message will invoke the nss_codel_configure_callback() API.
 */
static int nss_codel_mem_sz_get(struct Qdisc *sch, struct tc_nsscodel_qopt *qopt)
{
	struct nss_if_msg nim;
	struct nss_qdisc *nq = qdisc_priv(sch);

	/*
	 * No memory allocation needed for codel when operating in single
	 * queue mode. This is needed only for flow-queue mode.
	 */
	if (!qopt->flows) {
		return 0;
	}

	nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = nq->qos_tag;
	if (nss_qdisc_configure(nq, &nim,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_MEM_REQ) < 0) {
		nss_qdisc_warning("Memory size get failed\n");
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_codel_change()
 *	Used to configure the nss_codel queue in NSS firmware.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_codel_change(struct Qdisc *sch, struct nlattr *opt)
#else
static int nss_codel_change(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
#endif
{
	struct nss_codel_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSCODEL_MAX + 1];
	struct tc_nsscodel_qopt *qopt;
	struct nss_if_msg nim;
	struct net_device *dev = qdisc_dev(sch);
	uint32_t order, sz = 0;
	bool is_fq_codel = (sch->ops == &nss_fq_codel_qdisc_ops);
	struct nss_shaper_node_config *config;
	bool free_flow_queue = true;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_codel_policy, tb, TCA_NSSCODEL_MAX, TCA_NSSCODEL_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_codel_policy, tb, TCA_NSSCODEL_MAX, TCA_NSSCODEL_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	if (!qopt->target || !qopt->interval) {
		nss_qdisc_error("nss_codel requires a non-zero value for target "
				"and interval\n");
		return -EINVAL;
	}

	/*
	 * If flow queues have already been allocated, then ensure the user does not
	 * attempt to change the number of flows in a subsequent change command.
	 */
	if (q->flow_queue_mem) {
		if (q->flows != qopt->flows) {
			nss_qdisc_warning("Change of flow queue count not allowed");
			return -EINVAL;
		}

		/*
		 * Since the flow queue is already allocated, we should not free
		 * it if this update fails. The expectation is that the queue
		 * should continue to operate in its current configuration.
		 */
		free_flow_queue = false;
	}

	if (!qopt->limit) {
		qopt->limit = dev->tx_queue_len ? : 1;
	}

	q->target = qopt->target;
	q->limit = qopt->limit;
	q->interval = qopt->interval;
	q->quantum = qopt->quantum;
	q->flows = qopt->flows;
	q->ecn = qopt->ecn;
	q->set_default = qopt->set_default;

	/*
	 * Required for basic stats display
	 */
	sch->limit = qopt->limit;

	nss_qdisc_info("Target:%u Limit:%u Interval:%u set_default = %u\n",
		q->target, q->limit, q->interval, qopt->set_default);

	/*
	 * Allocate memory for the flow queues if it is nssfq_codel and memory
	 * has not been allocated previously (i.e. init time).
	 */
	if (is_fq_codel && !q->flow_queue_mem) {
		/*
		 * Use the default flow queue count if value is not speficied by user.
		 */
		if (!q->flows) {
			q->flows = NSS_CODEL_DEFAULT_FQ_COUNT;
		}

		sz = q->flow_queue_sz * q->flows;
		order = get_order(sz);

		nss_qdisc_trace("Allocating %u bytes (%u pages) for flow qeueue\n", sz, 1 << order);

		nss_qdisc_info("Flows:%u Quantum:%u Ecn:%u\n",
			q->flows, q->quantum, q->ecn);

		q->flow_queue_mem = __get_free_pages(GFP_KERNEL | __GFP_NOWARN | __GFP_ZERO, order);
		if (!q->flow_queue_mem) {
			nss_qdisc_warning("Failed to allocated flow queue memory\n");
			return -ENOMEM;
		}

		q->dma_mapped_mem = dma_map_single(nss_shaper_get_dev(), (void *)q->flow_queue_mem, sz, DMA_TO_DEVICE);
		if(unlikely(dma_mapping_error(nss_shaper_get_dev(), q->dma_mapped_mem))) {
			nss_qdisc_warning("DMA map failed for virtual address = %lu\n", q->flow_queue_mem);
			nss_codel_flow_queues_free(q);
			return -ENOMEM;
		}
		nss_qdisc_trace("Successfully allocated memory for flow queue\n");
	}

	/*
	 * Target and interval time needs to be provided in milliseconds
	 * (tc provides us the time in mircoseconds and therefore we divide by 1000)
	 */
	config = &nim.msg.shaper_configure.config.msg.shaper_node_config;
	config->snc.codel_param.cap.interval = q->interval / 1000;
	config->snc.codel_param.cap.target = q->target / 1000;
	config->qos_tag = q->nq.qos_tag;
	config->snc.codel_param.qlen_max = q->limit;
	config->snc.codel_param.cap.mtu = psched_mtu(dev);
	config->snc.codel_param.quantum = q->quantum;
	config->snc.codel_param.flows = q->flows;
	config->snc.codel_param.flows_mem = (uint32_t)q->dma_mapped_mem;
	config->snc.codel_param.flows_mem_sz = sz;
	config->snc.codel_param.ecn = q->ecn;
	nss_qdisc_info("MTU size of interface %s is %u bytes\n",
			dev->name, config->snc.codel_param.cap.mtu);

	if (nss_qdisc_configure(&q->nq, &nim,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
		goto fail;
	}

	/*
	 * There is nothing we need to do if the qdisc is not
	 * set as default qdisc.
	 */
	if (!q->set_default) {
		return 0;
	}

	/*
	 * Set this qdisc to be the default qdisc for enqueuing packets.
	 */
	if (nss_qdisc_set_default(&q->nq) < 0) {
		goto fail;
	}

	return 0;

fail:
	if (free_flow_queue) {
		nss_codel_flow_queues_free(q);
	}

	return -EINVAL;
}

/*
 * nss_codel_init()
 *	Initializes the nss_codel qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_codel_init(struct Qdisc *sch, struct nlattr *opt)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_codel_init(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
#endif
	struct nss_qdisc *nq = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSCODEL_MAX + 1];
	struct tc_nsscodel_qopt *qopt;

	if (!opt) {
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_codel_policy, tb, TCA_NSSCODEL_MAX, TCA_NSSCODEL_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_codel_policy, tb, TCA_NSSCODEL_MAX, TCA_NSSCODEL_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	if (qopt->accel_mode != TCA_NSS_ACCEL_MODE_NSS_FW) {
		nss_qdisc_warning("NSS codel supports only offload mode %d", TCA_NSS_ACCEL_MODE_NSS_FW);
		return -EINVAL;
	}

	nss_codel_reset(sch);
	nss_qdisc_register_configure_callback(nq, nss_codel_configure_callback);
	nss_qdisc_register_stats_callback(nq, nss_codel_stats_callback);

	if (nss_qdisc_init(sch, nq, NSS_SHAPER_NODE_TYPE_CODEL, 0, qopt->accel_mode, extack) < 0)
	{
		return -EINVAL;
	}

	if (nss_codel_mem_sz_get(sch, qopt) < 0) {
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_codel_change(sch, opt) < 0) {
#else
	if (nss_codel_change(sch, opt, extack) < 0) {
#endif
		nss_qdisc_destroy(nq);
		return -EINVAL;
	}

	/*
	 * Start the stats polling timer
	 */
	nss_qdisc_start_basic_stats_polling(nq);

	return 0;
}

/*
 * nss_codel_dump()
 *	Dumps out the nss_codel configuration parameters.
 */
static int nss_codel_dump(struct Qdisc *sch, struct sk_buff *skb)
{
	struct nss_codel_sched_data *q;
	struct nlattr *opts = NULL;
	struct tc_nsscodel_qopt opt;

	nss_qdisc_info("NssCodel Dumping!");

	q = qdisc_priv(sch);
	if (q == NULL) {
		return -1;
	}

	opt.target = q->target;
	opt.limit = q->limit;
	opt.interval = q->interval;
	opt.set_default = q->set_default;
	opt.accel_mode = nss_qdisc_accel_mode_get(&q->nq);
	opt.quantum = q->quantum;
	opt.flows = q->flows;
	opt.ecn = q->ecn;

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL) {
		goto nla_put_failure;
	}

	if (nla_put(skb, TCA_NSSCODEL_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * nss_codel_dump_stats()
 *	Dumps out the nss_codel statistics.
 */
static int nss_codel_dump_stats(struct Qdisc *sch, struct gnet_dump *d)
{
	struct nss_codel_sched_data *q = qdisc_priv(sch);
	struct tc_nssfq_codel_xstats fqcst;
	struct tc_nsscodel_xstats cst;

	if (!q->flows) {
		cst.peak_queue_delay = q->sq_stats.peak_queue_delay;
		cst.peak_drop_delay = q->sq_stats.peak_drop_delay;
		return gnet_stats_copy_app(d, &cst, sizeof(cst));
	}

	fqcst.new_flow_count = q->fq_stats.new_flow_cnt;
	fqcst.new_flows_len = q->fq_stats.new_flows_len;
	fqcst.old_flows_len = q->fq_stats.old_flows_len;
	fqcst.ecn_mark = q->fq_stats.ecn_mark_cnt;
	fqcst.drop_overlimit = q->fq_stats.drop_overlimit;
	fqcst.maxpacket = q->fq_stats.maxpacket;
	return gnet_stats_copy_app(d, &fqcst, sizeof(fqcst));
}

/*
 * nss_codel_peek()
 *	Peeks into the first sk_buff in the bounce complete queue.
 */
static struct sk_buff *nss_codel_peek(struct Qdisc *sch)
{
	nss_qdisc_info("Nsscodel Peeking");
	return nss_qdisc_peek(sch);
}

/*
 * nss_codel ops parameters.
 */
struct Qdisc_ops nss_codel_qdisc_ops __read_mostly = {
	.id		=	"nsscodel",
	.priv_size	=	sizeof(struct nss_codel_sched_data),
	.enqueue	=	nss_codel_enqueue,
	.dequeue	=	nss_codel_dequeue,
	.peek		=	nss_codel_peek,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		=	nss_codel_drop,
#endif
	.init		=	nss_codel_init,
	.reset		=	nss_codel_reset,
	.destroy	=	nss_codel_destroy,
	.change		=	nss_codel_change,
	.dump		=	nss_codel_dump,
	.dump_stats	=	nss_codel_dump_stats,
	.owner		=	THIS_MODULE,
};

/*
 * nss_fq_codel ops parameters.
 */
struct Qdisc_ops nss_fq_codel_qdisc_ops __read_mostly = {
	.id		=	"nssfq_codel",
	.priv_size	=	sizeof(struct nss_codel_sched_data),
	.enqueue	=	nss_codel_enqueue,
	.dequeue	=	nss_codel_dequeue,
	.peek		=	nss_codel_peek,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		=	nss_codel_drop,
#endif
	.init		=	nss_codel_init,
	.reset		=	nss_codel_reset,
	.destroy	=	nss_codel_destroy,
	.change		=	nss_codel_change,
	.dump		=	nss_codel_dump,
	.dump_stats	=	nss_codel_dump_stats,
	.owner		=	THIS_MODULE,
};
