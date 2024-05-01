/*
 **************************************************************************
 * Copyright (c) 2014-2018, 2020 The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/of.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <net/pkt_sched.h>
#include <net/pkt_cls.h>
#include <net/inet_ecn.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/if_bridge.h>
#include <linux/list.h>
#include <linux/version.h>
#include <br_private.h>
#include <nss_api_if.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#include <linux/netlink.h>
#endif

#if defined(NSS_QDISC_PPE_SUPPORT)
#include "nss_ppe.h"
#endif

#define NSS_QDISC_DEBUG_LEVEL_ERROR 1
#define NSS_QDISC_DEBUG_LEVEL_WARN 2
#define NSS_QDISC_DEBUG_LEVEL_INFO 3
#define NSS_QDISC_DEBUG_LEVEL_TRACE 4

/*
 * Debug message for module init and exit
 */
#define nss_qdisc_info_always(s, ...) printk(KERN_INFO"%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

/*
 * Error and warn message will be enabled by default in Makefile
 */
#if (NSS_QDISC_DEBUG_LEVEL < NSS_QDISC_DEBUG_LEVEL_ERROR)
#define nss_qdisc_assert(s, ...)
#define nss_qdisc_error(s, ...)
#else
#define nss_qdisc_assert(c, s, ...) { if (!(c)) { pr_emerg("ASSERT: %s:%d:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__); BUG(); } }
#define nss_qdisc_error(s, ...) pr_err("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_QDISC_DEBUG_LEVEL < NSS_QDISC_DEBUG_LEVEL_WARN)
#define nss_qdisc_warning(s, ...)
#else
#define nss_qdisc_warning(s, ...) pr_warn("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if defined(CONFIG_DYNAMIC_DEBUG)
/*
 * Compile messages for dynamic enable/disable
 */
#define nss_qdisc_info(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)
#define nss_qdisc_trace(s, ...) pr_debug("%s[%d]:" s, __func__, __LINE__, ##__VA_ARGS__)

#else
/*
 * Statically compile messages at different levels
 */
#if (NSS_QDISC_DEBUG_LEVEL < NSS_QDISC_DEBUG_LEVEL_INFO)
#define nss_qdisc_info(s, ...)
#else
#define nss_qdisc_info(s, ...) pr_notice("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif

#if (NSS_QDISC_DEBUG_LEVEL < NSS_QDISC_DEBUG_LEVEL_TRACE)
#define nss_qdisc_trace(s, ...)
#else
#define nss_qdisc_trace(s, ...) pr_info("%s[%d]:" s, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif
#endif

/*
 * State values
 */
#define NSS_QDISC_STATE_IDLE 0
#define NSS_QDISC_STATE_READY 1

#define NSS_QDISC_STATE_INIT_FAILED -1
#define NSS_QDISC_STATE_ASSIGN_SHAPER_SEND_FAIL -2
#define NSS_QDISC_STATE_SHAPER_ASSIGN_FAILED -3
#define NSS_QDISC_STATE_NODE_ALLOC_SEND_FAIL -4
#define NSS_QDISC_STATE_NODE_ALLOC_FAIL -5
#define NSS_QDISC_STATE_ROOT_SET_SEND_FAIL -6
#define NSS_QDISC_STATE_ROOT_SET_FAIL -7
#define NSS_QDISC_STATE_DEFAULT_SET_SEND_FAIL -8
#define NSS_QDISC_STATE_DEFAULT_SET_FAIL -9
#define NSS_QDISC_STATE_CHILD_ALLOC_SEND_FAIL -10
#define NSS_QDISC_STATE_NODE_ALLOC_FAIL_CHILD -11
#define NSS_QDISC_STATE_FAILED_RESPONSE -12
#define NSS_QDISC_STATE_UNASSIGN_SHAPER_SEND_FAIL -13
#define NSS_QDISC_STATE_UNASSIGN_SHAPER_FAIL -14
#define NSS_QDISC_STATE_NODE_FREE_FAIL -15
#define NSS_QDISC_STATE_NODE_FREE_SEND_FAIL -16

#define NSS_QDISC_BRIDGE_PORT_MAX 100

#if (LINUX_VERSION_CODE <= KERNEL_VERSION(3,8,0))
#define nss_qdisc_hlist_for_each_entry(tpos, pos, head, member) hlist_for_each_entry(tpos, pos, head, member)
#define nss_qdisc_hlist_for_each_entry_safe(tpos, pos, n, head, member) hlist_for_each_entry_safe(tpos, pos, n, head, member)
#define nss_qdisc_get_dev_master(dev) (dev->master)
#define nss_qdisc_get_dev(ptr) (struct net_device *)ptr
#else
#define nss_qdisc_hlist_for_each_entry(tpos, pos, head, member) hlist_for_each_entry(tpos, head, member)
#define nss_qdisc_hlist_for_each_entry_safe(tpos, pos, n, head, member) hlist_for_each_entry_safe(tpos, n, head, member)
#define nss_qdisc_get_dev_master(dev) netdev_master_upper_dev_get(dev)
#define nss_qdisc_get_dev(ptr) netdev_notifier_info_to_dev(ptr)
#endif

/*
 * Mode of Qdisc/class
 * These are defined as Magic numbers to avoid false positives
 */
#define NSS_QDISC_MODE_NSS 0x6243
#define NSS_QDISC_MODE_PPE 0x6245

struct nss_qdisc;

typedef void (*nss_qdisc_stats_callback_t)(struct nss_qdisc *nq, struct nss_shaper_node_stats_response *response);
typedef void (*nss_qdisc_configure_callback_t)(struct nss_qdisc *nq, struct nss_shaper_configure *response);

struct nss_qdisc {
	struct Qdisc *qdisc;			/* Handy pointer back to containing qdisc */
	struct nss_qdisc *parent;		/* Pointer to parent nss qdisc */
	void *nss_shaping_ctx;			/* NSS context for general operations */
	int32_t nss_interface_number;		/* NSS Interface number we are shaping on */
	nss_shaper_node_type_t type;		/* Type of shaper node */
	bool is_class;				/* True if this represents a class and not a qdisc */
	bool is_root;				/* True if root qdisc on a net device */
	bool is_bridge;				/* True when qdisc is a bridge */
	bool is_virtual;			/* True when the device is represented as a virtual in
						 * the NSS e.g. perhaps operating on a wifi interface
						 * or bridge.
						 */
	bool needs_ppe_loopback;		/* True when qdisc is on bridge or igs */
	bool destroy_virtual_interface;		/* Set if the interface is first registered in NSS by
						 * us. This means it needs to be un-regisreted when the
						 * module goes down.
						 */
	atomic_t state;				/* < 0: Signal that qdisc has 'failed'. 0
						 * indicates 'pending' setup.  > 0 is READY.
						 * NOTE: volatile AND atomic - this is polled
						 * AND is used for syncronisation.
						 */
	uint32_t shaper_id;			/* Used when is_root. Child qdiscs use this
						 * information to know what shaper under
						 * which to create shaper nodes
						 */
	uint32_t qos_tag;			/* QoS tag of this node */
	volatile int32_t pending_final_state;	/* Used to let the callback cycle know what
						 * state to set the qdisc in on successful
						 * completion.
						 */
	struct nss_virt_if_handle *virt_if_ctx;	/* Context provided by the NSS driver for
						 * new interfaces that are registered.
						 */
	void *bounce_context;			/* Context for bounce registration. Bounce
						 * enables packets to be sent to NSS for
						 * shaping purposes, and is returned to
						 * Linux for transmit.
						 */
	spinlock_t bounce_protection_lock;	/* Lock to protect the enqueue and dequeue
						 * operation on skb lists triggeret by bounce
						 * callbacks.
						 */
	nss_qdisc_stats_callback_t stats_cb;
						/* Stats update callback function for qdisc specific
						 * stats update.
						 */
	nss_qdisc_configure_callback_t config_cb;
						/* Shaper configure callback for reading shaper specific
						 * responses (e.g. memory size).
						 */
	struct gnet_stats_basic_sync bstats;	/* Basic class statistics */
	struct gnet_stats_queue qstats;		/* Qstats for use by classes */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	atomic_t refcnt;			/* Reference count for class use */
#else
	refcount_t refcnt;			/* Reference count for class use */
#endif
	struct timer_list stats_get_timer;	/* Timer used to poll for stats */
	atomic_t pending_stat_requests;		/* Number of pending stats responses */
	wait_queue_head_t wait_queue;		/* Wait queue used to wait on responses from the NSS */
	spinlock_t lock;			/* Lock to protect the nss qdisc structure */
	uint16_t mode;				/* Mode of Qdisc/class */

#if defined(NSS_QDISC_PPE_SUPPORT)
	struct nss_ppe_qdisc npq;		/* PPE Qdisc */
	bool ppe_init_failed;			/* Flag is set if PPE initialization fails */
	bool hybrid_configured;			/* Flag is set only in root qdisc when first NSS Qdisc
						 * is attached to PPE qdisc in the tree.
						 */
#endif
	struct tcf_proto __rcu *filter_list;	/* Filter list */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	struct tcf_block *block;
#endif
};

/*
 * nss_qdisc bridge update structure
 */
struct nss_qdisc_bridge_update {
	int port_list[NSS_QDISC_BRIDGE_PORT_MAX];
	int port_list_count;
	int unassign_count;
};

/*
 * Task types for bridge scanner.
 */
enum nss_qdisc_bshaper_tasks {
	NSS_QDISC_SCAN_AND_ASSIGN_BSHAPER,
	NSS_QDISC_SCAN_AND_UNASSIGN_BSHAPER,
};

/*
 * Types of messages sent down to NSS interfaces
 */
enum nss_qdisc_interface_msgs {
	NSS_QDISC_IF_SHAPER_ASSIGN,
	NSS_QDISC_IF_SHAPER_UNASSIGN,
	NSS_QDISC_IF_SHAPER_CONFIG,
};

/*
 * Types of mode for hybrid configuration.
 */
enum nss_qdisc_hybrid_mode {
	NSS_QDISC_HYBRID_MODE_DISABLE,
	NSS_QDISC_HYBRID_MODE_ENABLE,
};

/*
 * nss_qdisc_nla_nest_start()
 *	Returns the container attribute
 */
static inline struct nlattr * nss_qdisc_nla_nest_start(struct sk_buff *skb, int attrtype)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	return nla_nest_start(skb, TCA_OPTIONS);
#else
	return nla_nest_start_noflag(skb, TCA_OPTIONS);
#endif
}

/*
 * nss_qdisc_atomic_sub()
 *	Atomically decrements the ref count by 1
 */
static inline void nss_qdisc_atomic_sub(struct nss_qdisc *nq)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	atomic_sub(1, &nq->refcnt);
#else
	atomic_sub(1, &nq->refcnt.refs);
#endif
}

/*
 * nss_qdisc_atomic_sub_return()
 *	Atomically decrements the ref count by 1 and return ref count
 */
static inline int nss_qdisc_atomic_sub_return(struct nss_qdisc *nq)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	return atomic_sub_return(1, &nq->refcnt);
#else
	return atomic_sub_return(1, &nq->refcnt.refs);
#endif
}

/*
 * nss_qdisc_atomic_set()
 *	Atomically sets the ref count by 1
 */
static inline void nss_qdisc_atomic_set(struct nss_qdisc *nq)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	atomic_set(&nq->refcnt, 1);
#else
	refcount_set(&nq->refcnt, 1);
#endif
}

/*
 * nss_qdisc_put()
 *	Destroy the qdisc
 */
static inline void nss_qdisc_put(struct Qdisc *sch)
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 20, 0))
	qdisc_destroy(sch);
#else
	qdisc_put(sch);
#endif
}

/*
 * nss_qdisc_qopt_get()
 *	Extracts qopt from opt.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
extern void *nss_qdisc_qopt_get(struct nlattr *opt, struct nla_policy *policy,
				struct nlattr *tb[], uint32_t tca_max, uint32_t tca_params);
#else
extern void *nss_qdisc_qopt_get(struct nlattr *opt, struct nla_policy *policy,
				struct nlattr *tb[], uint32_t tca_max, uint32_t tca_params, struct netlink_ext_ack *extack);
#endif

/*
 * nss_qdisc_mode_get()
 *	Returns the operating mode of nss_qdisc, 0 = nss-fw, 1 = ppe.
 */
extern uint8_t nss_qdisc_accel_mode_get(struct nss_qdisc *nq);

/*
 * nss_qdisc_peek()
 *	Called to peek at the head of an nss qdisc
 */
extern struct sk_buff *nss_qdisc_peek(struct Qdisc *sch);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
/*
 * nss_qdisc_drop()
 *	Called to drop the packet at the head of queue
 */
extern unsigned int nss_qdisc_drop(struct Qdisc *sch);
#endif

/*
 * nss_qdisc_reset()
 *	Called when a qdisc is reset
 */
extern void nss_qdisc_reset(struct Qdisc *sch);

/*
 * nss_qdisc_enqueue()
 *	Generic enqueue call for enqueuing packets into NSS for shaping
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
extern int nss_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch);
#else
extern int nss_qdisc_enqueue(struct sk_buff *skb, struct Qdisc *sch, struct sk_buff **to_free);
#endif

/*
 * nss_qdisc_dequeue()
 *	Generic dequeue call for dequeuing bounced packets.
 */
extern struct sk_buff *nss_qdisc_dequeue(struct Qdisc *sch);

/*
 * nss_qdisc_set_hybrid_mode()
 *	Configuration function that enables/disables hybrid mode
 */
extern int nss_qdisc_set_hybrid_mode(struct nss_qdisc *nq, enum nss_qdisc_hybrid_mode mode, uint32_t offset);

/*
 * nss_qdisc_node_set_default()
 *	Configuration function that sets shaper node as default for packet enqueue
 */
extern int nss_qdisc_set_default(struct nss_qdisc *nq);

/*
 * nss_qdisc_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
extern int nss_qdisc_node_attach(struct nss_qdisc *nq, struct nss_qdisc *nq_child,
					struct nss_if_msg *nim, int32_t attach_type);

/*
 * nss_qdisc_node_detach()
 *	Configuration function that helps detach a child shaper node to a parent.
 */
extern int nss_qdisc_node_detach(struct nss_qdisc *nq, struct nss_qdisc *nq_child,
					struct nss_if_msg *nim, int32_t detach_type);

/*
 * nss_qdisc_configure()
 *	Configuration function that aids in tuning of queuing parameters.
 */
extern int nss_qdisc_configure(struct nss_qdisc *nq,
	struct nss_if_msg *nim, int32_t config_type);


/*
 * nss_qdisc_register_configure_callback()
 *	Register shaper configure callback, which gets invoked on receiving a response.
 */
extern void nss_qdisc_register_configure_callback(struct nss_qdisc *nq, nss_qdisc_configure_callback_t cb);

/*
 * nss_qdisc_register_stats_callback()
 *	Register shaper stats callback, which gets invoked on receiving a stats response.
 */
extern void nss_qdisc_register_stats_callback(struct nss_qdisc *nq, nss_qdisc_stats_callback_t cb);

/*
 * nss_qdisc_destroy()
 *	Destroys a shaper in NSS, and the sequence is based on the position of
 *	this qdisc (child or root) and the interface to which it is attached to.
 */
extern void nss_qdisc_destroy(struct nss_qdisc *nq);

/*
 * nss_qdisc_init()
 *	Initializes a shaper in NSS, based on the position of this qdisc (child or root)
 *	and if its a normal interface or a bridge interface.
 */
extern int nss_qdisc_init(struct Qdisc *sch, struct nss_qdisc *nq, nss_shaper_node_type_t type, uint32_t classid, uint32_t accel_mode,
		void *extack);

/*
 * nss_qdisc_start_basic_stats_polling()
 *	Call to initiate the stats polling timer
 */
extern void nss_qdisc_start_basic_stats_polling(struct nss_qdisc *nq);

/*
 * nss_qdisc_stop_basic_stats_polling()
 *	Call to stop polling of basic stats
 */
extern void nss_qdisc_stop_basic_stats_polling(struct nss_qdisc *nq);

/*
 * nss_qdisc_gnet_stats_copy_basic()
 *  Wrapper around gnet_stats_copy_basic()
 */
extern int nss_qdisc_gnet_stats_copy_basic(struct Qdisc *sch,
				struct gnet_dump *d, struct gnet_stats_basic_sync *b);

/*
 * nss_qdisc_gnet_stats_copy_queue()
 *  Wrapper around gnet_stats_copy_queue()
 */
extern int nss_qdisc_gnet_stats_copy_queue(struct gnet_dump *d,
				struct gnet_stats_queue *q);

/*
 * nss_qdisc_replace()
 *	Used to replace old qdisc with a new one
 */
extern struct Qdisc *nss_qdisc_replace(struct Qdisc *sch, struct Qdisc *new,
					struct Qdisc **pold);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
/*
 * nss_qdisc_tcf_chain()
 *	Return the filter list of qdisc.
 */
extern struct tcf_proto __rcu **nss_qdisc_tcf_chain(struct Qdisc *sch, unsigned long arg);
#else
/*
 * nss_qdisc_tcf_block()
 *	Return the block containing chain of qdisc.
 */
extern struct tcf_block *nss_qdisc_tcf_block(struct Qdisc *sch, unsigned long cl, struct netlink_ext_ack *extack);
#endif

/*
 * nss_qdisc_tcf_bind()
 *	Bind the filter to the qdisc.
 */
extern unsigned long nss_qdisc_tcf_bind(struct Qdisc *sch, unsigned long parent, u32 classid);

/*
 * nss_qdisc_tcf_unbind()
 *	Unbind the filter from the qdisc.
 */
extern void nss_qdisc_tcf_unbind(struct Qdisc *sch, unsigned long arg);
