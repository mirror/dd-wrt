/*
 * Copyright (c) 2014-2018, 2020-2021 The Linux Foundation. All rights reserved.
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

#ifndef _PPE_QDISC_H_
#define _PPE_QDISC_H_

#include <linux/version.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/pkt_sched.h>
#include <net/pkt_cls.h>
#include <linux/netlink.h>
#include <linux/list.h>

#include <ppe_drv_public.h>

#include "ppe_qdisc_debug.h"
#include "ppe_qdisc_port.h"
#include "ppe_qdisc_res.h"
#include "ppe_qdisc_stats.h"

#define PPE_QDISC_FLAG_NODE_ROOT		0x00000001	/* Qdisc node is root node */
#define PPE_QDISC_FLAG_NODE_CLASS		0x00000002	/* Qdisc node is a class */
#define PPE_QDISC_FLAG_NODE_DEFAULT		0x00000004	/* Qdisc node is a default node */
#define PPE_QDISC_FLAG_NODE_INITIALIZED		0x00000008	/* Qdisc node is initialized */
#define PPE_QDISC_FLAG_NODE_CONFIGURED		0x00000010	/* Qdisc is configured */
#define PPE_QDISC_FLAG_UCAST_QUEUE_VALID	0x00000020	/* Unicast queue is valid for the qdisc */
#define PPE_QDISC_FLAG_MCAST_QUEUE_VALID	0x00000040	/* Multicast queue is valid for the qdisc */
#define PPE_QDISC_FLAG_L0_SCHEDULER_VALID	0x00000080	/* L0 scheduler is valid for the qdisc */
#define PPE_QDISC_FLAG_L1_SCHEDULER_VALID	0x00000100	/* L1 scheduler is valid for the qdisc */
#define PPE_QDISC_FLAG_SHAPER_VALID		0x00000200	/* Shaper is valid for the qdisc */
#define PPE_QDISC_FLAG_INT_PRI_VALID		0x00000400	/* INT-PRI is valid for the qdisc */

#define PPE_QDISC_STATS_SYNC_MANY_PERIOD msecs_to_jiffies(1000)	/* Statistics sync peroid */

/*
 * ppe_qdisc_hlist_for_each_entry - iterate over list of classes
 */
#define ppe_qdisc_hlist_for_each_entry(tpos, pos, head, member) hlist_for_each_entry(tpos, head, member)

/*
 * ppe_qdisc_hlist_for_each_entry_safe - iterate over list of classes safe against removal of list entry
 */
#define ppe_qdisc_hlist_for_each_entry_safe(tpos, pos, n, head, member) hlist_for_each_entry_safe(tpos, n, head, member)

/**
 * ppe_qdisc_node_type
 *	Types of PPE Qdisc nodes.
 */
enum ppe_qdisc_node_type {
	/*
	 * Scheduler types.
	 */
	PPE_QDISC_NODE_TYPE_HTB,
	PPE_QDISC_NODE_TYPE_HTB_GROUP,
	PPE_QDISC_NODE_TYPE_TBL,
	PPE_QDISC_NODE_TYPE_WRR,
	PPE_QDISC_NODE_TYPE_WRR_GROUP,
	PPE_QDISC_NODE_TYPE_PRIO,
	PPE_QDISC_NODE_SCH_MAX = 0xF,

	/*
	 * Queue types.
	 */
	PPE_QDISC_NODE_TYPE_FIFO,
	PPE_QDISC_NODE_TYPE_RED,
	PPE_QDISC_NODE_TYPE_MAX,
};
typedef enum ppe_qdisc_node_type ppe_qdisc_node_type_t;

/*
 * ppe_qdisc_stats_wq
 * 	Qdisc stats sync info object
 */
struct ppe_qdisc_stats_wq {
	struct ppe_qdisc *pq;			/* Pointer to root ppe_qdisc */
	struct list_head stats_list;		/* List of root nodes for stats sync management work */
	struct timer_list stats_get_timer;	/* Timer used to start fresh iteration */
	bool stats_polling_stopped;		/* True when polling is stopped due to qdisc delete */
	struct list_head q_list_head;		/* List head to manage leaf node stats */
};

/*
 * ppe_qdisc
 *      PPE Qdisc structure
 */
struct ppe_qdisc {
	struct Qdisc *qdisc;		/* Handy pointer back to containing qdisc */
	struct ppe_qdisc *parent;	/* Pointer to parent PPE qdisc */
	struct ppe_qdisc *child;	/* Pointer to child PPE qdisc */
	struct ppe_qdisc *def;		/* Pointer to the default qdisc */
	int32_t port_id;		/* PPE Port number we are shaping on */
	uint32_t qos_tag;		/* QoS tag of this node */
	ppe_qdisc_node_type_t type;	/* Type of Qdisc node */
	uint32_t flags;			/* Qdisc falgs */
	uint32_t int_pri;		/* INT PRI value */
	ppe_drv_qos_level_t level;	/* Level at which qdisc is configured */
	struct ppe_drv_qos_res res;	/* PPE Qdisc scheduler and shaper resources */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	struct gnet_stats_basic_packed bstats;	/* Basic class statistics */
#else
	struct gnet_stats_basic_sync bstats;
#endif
	struct gnet_stats_queue qstats;	/* Qstats for use by classes */
	refcount_t refcnt;		/* Reference count for class use */
	spinlock_t lock;		/* Lock to protect the nss qdisc structure */
	struct list_head q_list_element;	/*List element in the list of leaf nodes */
	struct ppe_qdisc_stats_wq *stats_wq;	/* Stats info and stats work object */
	struct tcf_proto __rcu *filter_list;	/* Filter list */
	struct tcf_block *block;	/* TC filter block */
};

/*
 * ppe_qdisc_int_pri_get()
 *	Returns the INT-PRI value for a given classid.
 */
uint8_t ppe_qdisc_int_pri_get(struct net_device *dev, uint32_t classid);

/*
 * ppe_qdisc_nla_nest_start()
 *	Returns the container attribute
 */
static inline struct nlattr *ppe_qdisc_nla_nest_start(struct sk_buff *skb, int attrtype)
{
	return nla_nest_start_noflag(skb, TCA_OPTIONS);
}

/*
 * ppe_qdisc_atomic_sub()
 *	Atomically decrements the ref count by 1
 */
static inline void ppe_qdisc_atomic_sub(struct ppe_qdisc *pq)
{
	atomic_sub(1, &pq->refcnt.refs);
}

/*
 * ppe_qdisc_atomic_sub_return()
 *	Atomically decrements the ref count by 1 and return ref count
 */
static inline int ppe_qdisc_atomic_sub_return(struct ppe_qdisc *pq)
{
	return atomic_sub_return(1, &pq->refcnt.refs);
}

/*
 * ppe_qdisc_atomic_set()
 *	Atomically sets the ref count by 1
 */
static inline void ppe_qdisc_atomic_set(struct ppe_qdisc *pq)
{
	refcount_set(&pq->refcnt, 1);
}

/*
 * ppe_qdisc_tcf_block()
 *	Return the block containing chain of qdisc.
 */
static inline struct tcf_block *ppe_qdisc_tcf_block(struct Qdisc *sch, unsigned long cl, struct netlink_ext_ack *extack)
{
	struct ppe_qdisc *pq = qdisc_priv(sch);

	/*
	 * Currently, support is available only for tc filter iterations
	 * at root qdisc.
	 */
	if (pq->flags & PPE_QDISC_FLAG_NODE_ROOT) {
		return pq->block;
	}

	return NULL;
}

/*
 * ppe_qdisc_tcf_bind()
 *	Bind the filter to the qdisc.
 *
 * This is an empty callback, because, currently, tc filter iteration support
 * is not present at class of a qdisc.
 */
static inline unsigned long ppe_qdisc_tcf_bind(struct Qdisc *sch, unsigned long parent, u32 classid)
{
	return (unsigned long)NULL;
}

/*
 * ppe_qdisc_tcf_unbind()
 *	Unbind the filter from the qdisc.
 *
 * This is an empty callback, because, currently, tc filter iteration support
 * is not present at class of a qdisc.
 */
static inline void ppe_qdisc_tcf_unbind(struct Qdisc *sch, unsigned long arg)
{
	return;
}

/*
 * ppe_qdisc_peek()
 *	Called to peek at the head of an PPE qdisc
 */
static inline struct sk_buff *ppe_qdisc_peek(struct Qdisc *sch)
{
	return qdisc_peek_head(sch);
}

/*
 * ppe_qdisc_reset()
 *	Called when a qdisc is reset
 */
static inline void ppe_qdisc_reset(struct Qdisc *sch)
{
	qdisc_reset_queue(sch);
}

/*
 * ppe_qdisc_dequeue()
 *	Generic dequeue call for dequeuing packets.
 */
static inline struct sk_buff *ppe_qdisc_dequeue(struct Qdisc *sch)
{
	return __qdisc_dequeue_head(&sch->q);
}

/*
 * ppe_qdisc_enqueue()
 *	Generic enqueue call for enqueuing packets into PPE for shaping
 */
static inline int ppe_qdisc_enqueue(struct sk_buff *skb,
		struct Qdisc *sch,
		struct sk_buff **to_free)
{
	if (!skb->priority) {
		struct tcf_proto *tcf = NULL;
		struct tcf_result res;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
		if (TC_ACT_UNSPEC != tcf_classify(skb, tcf, &res, false)) {
#else
		if (TC_ACT_UNSPEC != tcf_classify(skb, NULL, tcf, &res, false)) {
#endif
			if (!res.class) {
				skb->priority = res.classid;
			}
		}
	}

	/*
	 * Set the SKB int_pri from SKB priority.
	 * This int_pri will be used in EDMA Tx descriptor for PPE Tx queue.
	 */
	skb_set_int_pri(skb, ppe_qdisc_int_pri_get(qdisc_dev(sch), skb->priority));
	__qdisc_enqueue_tail(skb, &sch->q);
	__netif_schedule(sch);
	return NET_XMIT_SUCCESS;
}

/*
 * ppe_qdisc_replace()
 *	Used to replace old qdisc with a new qdisc.
 */
static inline struct Qdisc *ppe_qdisc_replace(struct Qdisc *sch,
		struct Qdisc *new,
		struct Qdisc **pold)
{
	return qdisc_replace(sch, new, pold);
}

/*
 * ppe_qdisc_gnet_stats_copy_basic()
 *  Wrapper around gnet_stats_copy_basic()
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
static inline int ppe_qdisc_gnet_stats_copy_basic(struct Qdisc *sch, struct gnet_dump *d,
		struct gnet_stats_basic_packed *b)
{
	return gnet_stats_copy_basic(qdisc_root_sleeping_running(sch), d, NULL, b);
}
#else
static inline int ppe_qdisc_gnet_stats_copy_basic(struct Qdisc *sch, struct gnet_dump *d,
		struct gnet_stats_basic_sync *b)
{
	return gnet_stats_copy_basic(d, NULL, b, true);
}
#endif

/*
 * ppe_qdisc_gnet_stats_copy_queue()
 *  Wrapper around gnet_stats_copy_queue()
 */
static inline int ppe_qdisc_gnet_stats_copy_queue(struct gnet_dump *d,
		struct gnet_stats_queue *q)
{
	return gnet_stats_copy_queue(d, NULL, q, q->qlen);
}

/*
 * ppe_qdisc_flags_check()
 *	check bit flags in Qdisc.
 */
static inline bool ppe_qdisc_flags_check(struct ppe_qdisc *pq, uint32_t flags)
{
	return (pq->flags & flags);
}

/*
 * ppe_qdisc_flags_clear()
 *	Clear specific bit flags in Qdisc.
 */
static inline void ppe_qdisc_flags_clear(struct ppe_qdisc *pq, uint32_t flags)
{
	pq->flags &= ~flags;
}

/*
 * ppe_qdisc_flags_set()
 *	Set specific bit flags in Qdisc.
 */
static inline void ppe_qdisc_flags_set(struct ppe_qdisc *pq, uint32_t flags)
{
	pq->flags |= flags;
}

/*
 * ppe_qdisc_type_is_queue()
 *	Returns true is qdisc is of type queue.
 */
bool ppe_qdisc_type_is_queue(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_drr_weight_get()
 *	Returns the DRR weight corresponding to quantum.
 */
int ppe_qdisc_drr_weight_get(uint32_t quantum, ppe_drv_qos_drr_unit_t drr_unit);

/*
 * ppe_qdisc_get_max_prio_bands()
 *	Returns the number of PRIO bands supported based on qdisc level.
 */
int ppe_qdisc_get_max_prio_bands(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_is_depth_valid()
 *	Checks the depth of Qdisc tree.
 */
int ppe_qdisc_is_depth_valid(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_qopt_get()
 *	Extracts qopt from opt.
 */
void *ppe_qdisc_qopt_get(struct nlattr *opt, struct nla_policy *policy,
		struct nlattr *tb[], uint32_t tca_max,
		uint32_t tca_params, struct netlink_ext_ack *extack);

/*
 * ppe_qdisc_set_default()
 *	Set the default node for the Qdisc.
 */
void ppe_qdisc_set_default(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_node_detach()
 *	Configuration function that helps detach a child shaper node from a parent.
 */
void ppe_qdisc_node_detach(struct ppe_qdisc *pq, struct ppe_qdisc *pq_child);

/*
 * ppe_qdisc_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
int ppe_qdisc_node_attach(struct ppe_qdisc *pq, struct ppe_qdisc *pq_child);

/*
 * ppe_qdisc_configure()
 *	Configures the SSDK schedulers and shapers.
 */
int ppe_qdisc_configure(struct ppe_qdisc *pq, struct ppe_qdisc *prev_pq);

/*
 * ppe_qdisc_destroy()
 *	Destroys PPE Qdisc.
 */
void ppe_qdisc_destroy(struct ppe_qdisc *pq);

/*
 * ppe_qdisc_init()
 *	Allocates PPE resources and initializes a schedulers/shaper in PPE.
 *
 * The resource allocation is based on the position (child or root) and level of this Qdisc/class.
 * Currently supported for physical interfaces only.
 */
int ppe_qdisc_init(struct Qdisc *sch, struct ppe_qdisc *pq,
		ppe_qdisc_node_type_t type, uint32_t classid,
		struct netlink_ext_ack *extack);

#endif
