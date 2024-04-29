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
#include "nss_wrr.h"

struct nss_wrr_class_data {
	struct nss_qdisc nq;		/* Base class used by nss_qdisc */
	struct Qdisc_class_common cl_common;	/* Common class structure */
	u32 quantum;				/* Quantum allocation for DRR */
	struct Qdisc *qdisc;			/* Pointer to child qdisc */
};

struct nss_wrr_sched_data {
	struct nss_qdisc nq;		/* Base class used by nss_qdisc */
	struct nss_wrr_class_data root;	/* Root class */
	struct Qdisc_class_hash clhash;	/* Class hash */
};

static struct nla_policy nss_wrr_policy[TCA_NSSWRR_MAX + 1] = {
	[TCA_NSSWRR_CLASS_PARMS] = { .len = sizeof(struct tc_nsswrr_class_qopt) },
	[TCA_NSSWRR_QDISC_PARMS] = { .len = sizeof(struct tc_nsswrr_qopt) },
};

static inline struct nss_wrr_class_data *nss_wrr_find_class(u32 classid,
							struct Qdisc *sch)
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;
	clc = qdisc_class_find(&q->clhash, classid);
	if (clc == NULL) {
		nss_qdisc_info("Cannot find class with classid %u in qdisc %px hash table %px\n", classid, sch, &q->clhash);
		return NULL;
	}
	return container_of(clc, struct nss_wrr_class_data, cl_common);
}

static void nss_wrr_destroy_class(struct Qdisc *sch, struct nss_wrr_class_data *cl)
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct nss_if_msg nim;

	nss_qdisc_info("Destroying nss_wrr class %px from qdisc %px\n", cl, sch);

	/*
	 * Note, this function gets called even for NSSWRR and not just for NSSWRR_GROUP.
	 * If this is wrr qdisc then we should not call nss_qdisc_destroy or stop polling
	 * for stats. These two actions will happen inside nss_wrr_destroy(), which is called
	 * only for the root qdisc.
	 */
	if (cl == &q->root) {
		nss_qdisc_info("We do not destroy nss_wrr class %px here since this is "
				"the qdisc %px\n", cl, sch);
		return;
	}

	/*
	 * We always have to detach our child qdisc in NSS, before destroying it.
	 */
	if (cl->qdisc != &noop_qdisc) {
		struct nss_qdisc *nq_child = qdisc_priv(cl->qdisc);
		nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		if (nss_qdisc_node_detach(&cl->nq, nq_child, &nim,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			nss_qdisc_error("Failed to detach child %x from class %x\n",
					cl->qdisc->handle, q->nq.qos_tag);
			return;
		}
	}

	/*
	 * And now we destroy the child.
	 */
	 nss_qdisc_put(cl->qdisc);

	/*
	 * Stop the stats polling timer and free class
	 */
	nss_qdisc_stop_basic_stats_polling(&cl->nq);

	/*
	 * Destroy the shaper in NSS
	 */
	nss_qdisc_destroy(&cl->nq);

	/*
	 * Free class
	 */
	kfree(cl);
}

/*
 * nss_wrr_class_params_validate_and_save()
 *	Validates and saves the class configuration parameters.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_wrr_class_params_validate_and_save(struct Qdisc *sch, struct nlattr **tca,
					uint32_t *quantum)
#else
static int nss_wrr_class_params_validate_and_save(struct Qdisc *sch, struct nlattr **tca,
					uint32_t *quantum, struct netlink_ext_ack *extack)
#endif
{
	struct nlattr *tb[TCA_NSSWRR_MAX + 1];
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct tc_nsswrr_class_qopt *qopt;
	struct net_device *dev = qdisc_dev(sch);
	bool is_wrr = (sch->ops == &nss_wrr_qdisc_ops);
#if defined(NSS_QDISC_PPE_SUPPORT)
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
#endif

	nss_qdisc_trace("validating parameters for nsswrr class of qdisc:%x\n", sch->handle);

	if (!opt) {
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_wrr_policy, tb, TCA_NSSWRR_MAX, TCA_NSSWRR_CLASS_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_wrr_policy, tb, TCA_NSSWRR_MAX, TCA_NSSWRR_CLASS_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	sch_tree_lock(sch);

	/*
	 * If the value of quantum is not provided default it based on the type
	 * of operation (i.e. wrr or wfq)
	 */
	*quantum = qopt->quantum;

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (is_wrr) {
		if ((qopt->quantum >= NSS_PPE_DRR_WEIGHT_MAX)) {
			nss_qdisc_warning("quantum %u of nss_wrr class of qdisc %x should be a less than 1024\n",
					qopt->quantum, sch->handle);
			sch_tree_unlock(sch);
			return -EINVAL;
		}
	} else if (q->nq.mode == NSS_QDISC_MODE_PPE) {
		if ((qopt->quantum % NSS_PPE_DRR_WEIGHT_MAX) != 0) {
			nss_qdisc_warning("nss_wfq (accel_mode %d) requires quantum to be a multiple of 1024\n",
					nss_qdisc_accel_mode_get(&q->nq));
			sch_tree_unlock(sch);
			return -EINVAL;
		}

		if (qopt->quantum > (NSS_PPE_DRR_WEIGHT_MAX * NSS_PPE_DRR_WEIGHT_MAX)) {
			nss_qdisc_warning("nss_wfq (accel_mode %d) requires quantum not exceeding 1024*1024\n",
					nss_qdisc_accel_mode_get(&q->nq));
			sch_tree_unlock(sch);
			return -EINVAL;
		}
	}
#endif

	if (!*quantum) {
		if (is_wrr) {
			*quantum = 1;
			nss_qdisc_info("Quantum value not provided for nss_wrr class on interface %s. "
					"Setting quantum to %up\n", dev->name, *quantum);
		} else {
			*quantum = psched_mtu(dev);
			nss_qdisc_info("Quantum value not provided for nss_wfq class on interface %s. "
					"Setting quantum to %ubytes\n", dev->name, *quantum);
		}
	}

	sch_tree_unlock(sch);

	return 0;
}

#if defined(NSS_QDISC_PPE_SUPPORT)
/*
 * nss_wrr_ppe_change_class()
 *	Configures a class in ppe qdisc
 */
static int nss_wrr_ppe_change_class(struct Qdisc *sch, struct nss_wrr_class_data *cl, uint32_t quantum)
{
	struct nss_ppe_qdisc prev_npq;
	bool is_wrr = (sch->ops == &nss_wrr_qdisc_ops);

	/*
	 * Save the previous scheduler configuration
	 * for handling failure conditions.
	 */
	prev_npq = cl->nq.npq;

	/*
	 * Quantum is specified in bytes for WFQ while it is in packets for WRR.
	 */
	if (is_wrr) {
		cl->nq.npq.scheduler.drr_weight = nss_ppe_drr_weight_get(quantum, NSS_PPE_DRR_UNIT_PACKET);
		cl->nq.npq.scheduler.drr_unit = NSS_PPE_DRR_UNIT_PACKET;
	} else {
		cl->nq.npq.scheduler.drr_weight = nss_ppe_drr_weight_get(quantum, NSS_PPE_DRR_UNIT_BYTE);
		cl->nq.npq.scheduler.drr_unit = NSS_PPE_DRR_UNIT_BYTE;
	}

	/*
	 * Change the configuration in SSDK
	 */
	if (nss_ppe_configure(&cl->nq, &prev_npq) != 0)  {
		nss_qdisc_warning("nss_wrr %x SSDK scheduler configuration failed\n", sch->handle);
		return -EINVAL;
	}

	cl->quantum = quantum;
	return 0;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_wrr_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_wrr_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
#endif
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct nss_wrr_class_data *cl = (struct nss_wrr_class_data *)*arg;
	struct nss_if_msg nim_config;
	bool new_init = false;
	uint32_t quantum;
	unsigned int accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	nss_qdisc_info("Changing nss_wrr class %u\n", classid);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_wrr_class_params_validate_and_save(sch, tca, &quantum) < 0) {
#else
	if (nss_wrr_class_params_validate_and_save(sch, tca, &quantum, extack) < 0) {
#endif
		nss_qdisc_warning("validation of configuration parameters for wrr class %x failed\n",
					sch->handle);
		return -EINVAL;
	}

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (cl && (cl->nq.mode == NSS_QDISC_MODE_PPE)) {
		if (nss_wrr_ppe_change_class(sch, cl, quantum) < 0) {
			nss_qdisc_warning("nss_htb %x SSDK scheduler configuration failed\n", sch->handle);
			return -EINVAL;
		}
		return 0;
	}
#endif

	/*
	 * If class with a given classid is not found, we allocate a new one
	 */
	if (!cl) {

		struct nss_if_msg nim_attach;

		/*
		 * The class does not already exist, we are newly initializing it.
		 */
		new_init = true;

		nss_qdisc_info("Nss_wrr class %u not found. Allocating a new class.\n", classid);
		cl = kzalloc(sizeof(struct nss_wrr_class_data), GFP_KERNEL);

		if (!cl) {
			nss_qdisc_error("Class allocation failed for classid %u\n", classid);
			return -EINVAL;
		}

		nss_qdisc_info("NSS_wrr class %u allocated %px\n", classid, cl);
		cl->cl_common.classid = classid;

		/*
		 * We make the child qdisc a noop qdisc, and
		 * set reference count to 1. This is important,
		 * reference count should not be 0.
		 */
		cl->qdisc = &noop_qdisc;
		nss_qdisc_atomic_set(&cl->nq);
		*arg = (unsigned long)cl;

		nss_qdisc_info("Adding classid %u to qdisc %px hash queue %px\n", classid, sch, &q->clhash);

		/*
		 * This is where a class gets initialized. Classes do not have a init function
		 * that is registered to Linux. Therefore we initialize the NSSWRR_GROUP shaper
		 * here.
		 */
		cl->nq.parent = &q->nq;
		if (nss_qdisc_init(sch, &cl->nq, NSS_SHAPER_NODE_TYPE_WRR_GROUP, classid, accel_mode, extack) < 0)
		{
			nss_qdisc_error("Nss init for class %u failed\n", classid);
			return -EINVAL;
		}

#if defined(NSS_QDISC_PPE_SUPPORT)
		if (cl->nq.mode == NSS_QDISC_MODE_PPE) {
			if (nss_wrr_ppe_change_class(sch, cl, quantum) < 0) {
				nss_qdisc_warning("nss_htb %x SSDK scheduler configuration failed\n", sch->handle);
				nss_qdisc_destroy(&cl->nq);
				goto failure;
			}
		}
#endif
		/*
		 * Set qos_tag of parent to which the class needs to be attached to.
		 * Set the child to be this class.
		 * Send node_attach command down to the NSS.
		 */
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;

		if (cl->nq.mode == NSS_QDISC_MODE_NSS) {
			nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_attach.child_qos_tag = cl->nq.qos_tag;
		}

		if (nss_qdisc_node_attach(&q->nq, &cl->nq, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			nss_qdisc_error("Nss attach for class %u failed\n", classid);
			nss_qdisc_destroy(&cl->nq);
			goto failure;
		}

		/*
		 * Add class to hash tree once it is attached in the NSS
		 */
		sch_tree_lock(sch);
		qdisc_class_hash_insert(&q->clhash, &cl->cl_common);
		sch_tree_unlock(sch);

		/*
		 * Hash grow should not come within the tree lock
		 */
		qdisc_class_hash_grow(sch, &q->clhash);

		/*
		 * Start the stats polling timer
		 */
		nss_qdisc_start_basic_stats_polling(&cl->nq);

		nss_qdisc_info("Class %u successfully allocated\n", classid);
	}

	cl->quantum = quantum;

	/*
	 * Fill information that needs to be sent down to the NSS for configuring the
	 * wrr class.
	 */
	if (cl->nq.mode == NSS_QDISC_MODE_NSS) {
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_group_param.quantum = cl->quantum;

		nss_qdisc_info("Quantum = %u\n", cl->quantum);

		/*
		 * Send configure command to the NSS
		 */
		if (nss_qdisc_configure(&cl->nq, &nim_config,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
			nss_qdisc_error("Failed to configure class %x\n", classid);

			/*
			 * We dont have to destroy the class if this was just a
			 * change command.
			 */
			if (!new_init) {
				return -EINVAL;
			}

			/*
			 * Else, we have failed in the NSS and we will have to
			 * destroy the class
			 */
			nss_qdisc_destroy(&cl->nq);
			goto failure;
		}
	}

	nss_qdisc_info("Class %x changed successfully\n", classid);
	return 0;

failure:
	if (cl) {
		kfree(cl);
	}
	return -EINVAL;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
static int nss_wrr_delete_class(struct Qdisc *sch, unsigned long arg, struct netlink_ext_ack *extack)
#else
static int nss_wrr_delete_class(struct Qdisc *sch, unsigned long arg)
#endif
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct nss_wrr_class_data *cl = (struct nss_wrr_class_data *)arg;
	struct nss_if_msg nim;
	int refcnt;

	/*
	 * Since all classes are leaf nodes in our case, we dont have to make
	 * that check.
	 */
	if (cl == &q->root)
		return -EBUSY;

	/*
	 * The message to NSS should be sent to the parent of this class
	 */
	nss_qdisc_info("Detaching nss_wrr class: %px\n", cl);
	nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_detach.child_qos_tag = cl->nq.qos_tag;
	if (nss_qdisc_node_detach(&q->nq, &cl->nq, &nim,
			NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
		return -EINVAL;
	}

	sch_tree_lock(sch);
	qdisc_reset(cl->qdisc);
	qdisc_class_hash_remove(&q->clhash, &cl->cl_common);

	refcnt = nss_qdisc_atomic_sub_return(&cl->nq);

	sch_tree_unlock(sch);
	if (!refcnt) {
		nss_qdisc_error("Reference count should not be zero for class %px\n", cl);
	}

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_wrr_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
								 struct Qdisc **old)
#else
static int nss_wrr_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
								 struct Qdisc **old, struct netlink_ext_ack *extack)
#endif
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct nss_wrr_class_data *cl = (struct nss_wrr_class_data *)arg;
	struct nss_if_msg nim_detach;
	struct nss_if_msg nim_attach;
	struct nss_qdisc *nq_new = qdisc_priv(new);

	nss_qdisc_info("Grafting class %px\n", sch);

	if (cl == &q->root) {
		nss_qdisc_error("Can't graft root class %px\n", cl);
		return -EINVAL;
	}

	if (new == NULL)
		new = &noop_qdisc;

	sch_tree_lock(sch);
	*old = cl->qdisc;
	sch_tree_unlock(sch);

	/*
	 * Since we initially attached a noop qdisc as child (in Linux),
	 * we do not perform a detach in the NSS if its a noop qdisc.
	 */
	nss_qdisc_info("Grafting old: %px with new: %px\n", *old, new);
	if (*old != &noop_qdisc) {
		struct nss_qdisc *nq_child = qdisc_priv(*old);
		nss_qdisc_info("Detaching old: %px\n", *old);
		nim_detach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		if (nss_qdisc_node_detach(&cl->nq, nq_child, &nim_detach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			return -EINVAL;
		}
	}

	/*
	 * If the new qdisc is a noop qdisc, we do not send down an attach command
	 * to the NSS.
	 */
	if (new != &noop_qdisc) {
		nss_qdisc_info("Attaching new: %px\n", new);
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_group_attach.child_qos_tag = nq_new->qos_tag;
		if (nss_qdisc_node_attach(&cl->nq, nq_new, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			return -EINVAL;
		}
	}

	/*
	 * Replaced in NSS, now replace in Linux.
	 */
	nss_qdisc_replace(sch, new, &cl->qdisc);

	nss_qdisc_info("Nsswrr grafted");

	return 0;
}

static struct Qdisc *nss_wrr_leaf_class(struct Qdisc *sch, unsigned long arg)
{
	struct nss_wrr_class_data *cl = (struct nss_wrr_class_data *)arg;
	nss_qdisc_info("nss_wrr class leaf %px\n", cl);

	/*
	 * Since all nss_wrr groups are leaf nodes, we can always
	 * return the attached qdisc.
	 */
	return cl->qdisc;
}

static void nss_wrr_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
	nss_qdisc_info("nss_wrr qlen notify %px\n", sch);
	/*
	 * Gets called when qlen of child changes (Useful for deactivating)
	 * Not useful for us here.
	 */
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
static unsigned long nss_wrr_get_class(struct Qdisc *sch, u32 classid)
{
	struct nss_wrr_class_data *cl = nss_wrr_find_class(classid, sch);

	nss_qdisc_info("Get nss_wrr class %px - class match = %px\n", sch, cl);

	if (cl != NULL) {
		atomic_add(1, &cl->nq.refcnt);
	}

	return (unsigned long)cl;
}

static void nss_wrr_put_class(struct Qdisc *sch, unsigned long arg)
{
	struct nss_wrr_class_data *cl = (struct nss_wrr_class_data *)arg;
	nss_qdisc_info("nss_wrr put class for %px\n", cl);

	/*
	 * We are safe to destroy the qdisc if the reference count
	 * goes down to 0.
	 */
	if (nss_qdisc_atomic_sub_return(&cl->nq) == 0) {
		nss_wrr_destroy_class(sch, cl);
	}
}
#else
static unsigned long nss_wrr_search_class(struct Qdisc *sch, u32 classid)
{
	struct nss_wrr_class_data *cl = nss_wrr_find_class(classid, sch);

	nss_qdisc_info("Get nss_wrr class %px - class match = %px\n", sch, cl);

	if (cl != NULL) {
		atomic_add(1, &cl->nq.refcnt.refs);
	}

	return (unsigned long)cl;
}
#endif

static int nss_wrr_dump_class(struct Qdisc *sch, unsigned long arg, struct sk_buff *skb,
		struct tcmsg *tcm)
{
	struct nss_wrr_class_data *cl = (struct nss_wrr_class_data *)arg;
	struct nlattr *opts;
	struct tc_nsswrr_class_qopt qopt;

	nss_qdisc_info("Dumping class %px of Qdisc %x\n", cl, sch->handle);

	qopt.quantum = cl->quantum;

	/*
	 * All bf group nodes are root nodes. i.e. they dont
	 * have any mode bf groups attached beneath them.
	 */
	tcm->tcm_parent = TC_H_ROOT;
	tcm->tcm_handle = cl->cl_common.classid;
	tcm->tcm_info = cl->qdisc->handle;

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_NSSWRR_CLASS_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}
	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

static int nss_wrr_dump_class_stats(struct Qdisc *sch, unsigned long arg, struct gnet_dump *d)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)arg;

	if (nss_qdisc_gnet_stats_copy_basic(sch, d, &nq->bstats) < 0 ||
			nss_qdisc_gnet_stats_copy_queue(d, &nq->qstats) < 0) {
		return -1;
	}

	return 0;
}

static void nss_wrr_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct nss_wrr_class_data *cl;
	unsigned int i;

	nss_qdisc_info("In nss_wrr walk %px\n", sch);
	if (arg->stop)
		return;

	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i],
				cl_common.hnode) {
			if (arg->count < arg->skip) {
				arg->count++;
				continue;
			}
			if (arg->fn(sch, (unsigned long)cl, arg) < 0) {
				arg->stop = 1;
				return;
			}
			arg->count++;
		}
	}
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_wrr_init_qdisc(struct Qdisc *sch, struct nlattr *opt)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_wrr_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
#endif
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSWRR_MAX + 1];
	int err;
	struct nss_if_msg nim;
	struct tc_nsswrr_qopt *qopt;

	nss_qdisc_info("Init nss_wrr qdisc %px\n", sch);

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0) {
		return err;
	}

	q->root.cl_common.classid = sch->handle;
	q->root.qdisc = &noop_qdisc;

	qdisc_class_hash_insert(&q->clhash, &q->root.cl_common);
	qdisc_class_hash_grow(sch, &q->clhash);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_wrr_policy, tb, TCA_NSSWRR_MAX, TCA_NSSWRR_QDISC_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_wrr_policy, tb, TCA_NSSWRR_MAX, TCA_NSSWRR_QDISC_PARMS, extack);
#endif
	if (!qopt) {
		nss_qdisc_warning("Failed to parse input");
		return -EINVAL;
	}

	/*
	 * Initialize the NSSWRR shaper in NSS
	 */
	if (nss_qdisc_init(sch, &q->nq, NSS_SHAPER_NODE_TYPE_WRR, 0, qopt->accel_mode, extack) < 0) {
		nss_qdisc_warning("Failed init nss_wrr qdisc");
		return -EINVAL;
	}

	if (q->nq.mode == NSS_QDISC_MODE_NSS) {
		/*
		 * Configure the qdisc to operate in one of the two modes
		 */
		nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
		if (strncmp(sch->ops->id, "nsswrr", 6) == 0) {
			nim.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_param.operation_mode = NSS_SHAPER_WRR_MODE_ROUND_ROBIN;
		} else if (strncmp(sch->ops->id, "nsswfq", 6) == 0) {
			nim.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_param.operation_mode = NSS_SHAPER_WRR_MODE_FAIR_QUEUEING;
		} else {
			nss_qdisc_error("Unknow qdisc association");
			nss_qdisc_destroy(&q->nq);
			return -EINVAL;
		}

		/*
		 * Send configure command to the NSS
		 */
		if (nss_qdisc_configure(&q->nq, &nim, NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
			nss_qdisc_warning("Failed to configure nss_wrr qdisc %x\n", q->nq.qos_tag);
			nss_qdisc_destroy(&q->nq);
			return -EINVAL;
		}
	}

	nss_qdisc_info("Nsswrr initialized - handle %x parent %x\n", sch->handle, sch->parent);

	/*
	 * Start the stats polling timer
	 */
	nss_qdisc_start_basic_stats_polling(&q->nq);

	return 0;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_wrr_change_qdisc(struct Qdisc *sch, struct nlattr *opt)
#else
static int nss_wrr_change_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
#endif
{
	struct nlattr *tb[TCA_NSSWRR_MAX + 1];
	struct nss_wrr_sched_data *q;
	struct tc_nsswrr_qopt *qopt;

	q = qdisc_priv(sch);

	if (opt == NULL) {
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_wrr_policy, tb, TCA_NSSWRR_MAX, TCA_NSSWRR_QDISC_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_wrr_policy, tb, TCA_NSSWRR_MAX, TCA_NSSWRR_QDISC_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	/*
	 * WRR has no qdisc parameters that can be changed.
	 */

	return 0;
}

static void nss_wrr_reset_class(struct nss_wrr_class_data *cl)
{
	nss_qdisc_reset(cl->qdisc);
	nss_qdisc_info("Nsswrr class resetted %px\n", cl->qdisc);
}

static void nss_wrr_reset_qdisc(struct Qdisc *sch)
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct nss_wrr_class_data *cl;
	struct hlist_node *n __maybe_unused;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i], cl_common.hnode)
		nss_wrr_reset_class(cl);
	}

	nss_qdisc_reset(sch);
	nss_qdisc_info("Nsswrr qdisc resetted %px\n", sch);
}

static void nss_wrr_destroy_qdisc(struct Qdisc *sch)
{
	struct nss_wrr_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct hlist_node *next;
	struct nss_wrr_class_data *cl;
	struct nss_if_msg nim;
	unsigned int i;

	/*
	 * Destroy all the classes before the root qdisc is destroyed.
	 */
	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry_safe(cl, n, next, &q->clhash.hash[i], cl_common.hnode) {

			/*
			 * If this is the root class, we dont have to destroy it. This will be taken
			 * care of by the nss_wrr_destroy() function.
			 */
			if (cl == &q->root) {
				nss_qdisc_info("We do not detach or destroy nss_wrr class %px here since this is "
						"the qdisc %px\n", cl, sch);
				continue;
			}

			/*
			 * Reduce refcnt by 1 before destroying. This is to
			 * ensure that polling of stat stops properly.
			 */
			 nss_qdisc_atomic_sub(&cl->nq);

			/*
			 * Detach class before destroying it. We dont check for noop qdisc here
			 * since we do not attach anu such at init.
			 */
			nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
			nim.msg.shaper_configure.config.msg.shaper_node_config.snc.wrr_detach.child_qos_tag = cl->nq.qos_tag;
			if (nss_qdisc_node_detach(&q->nq, &cl->nq, &nim, NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
				nss_qdisc_error("Node detach failed for qdisc %x class %x\n",
							cl->nq.qos_tag, q->nq.qos_tag);
				return;
			}

			/*
			 * Now we can destroy the class.
			 */
			nss_wrr_destroy_class(sch, cl);
		}
	}
	qdisc_class_hash_destroy(&q->clhash);

	/*
	 * Stop the polling of basic stats
	 */
	nss_qdisc_stop_basic_stats_polling(&q->nq);

	/*
	 * Now we can go ahead and destroy the qdisc.
	 * Note: We dont have to detach ourself from our parent because this
	 *	 will be taken care of by the graft call.
	 */
	nss_qdisc_destroy(&q->nq);
	nss_qdisc_info("Nsswrr destroyed %px\n", sch);
}

static int nss_wrr_dump_qdisc(struct Qdisc *sch, struct sk_buff *skb)
{
	struct nss_wrr_sched_data *q;
	struct nlattr *opts = NULL;
	struct tc_nsswrr_qopt opt;

	nss_qdisc_info("Nsswrr Dumping!");

	q = qdisc_priv(sch);
	if (q == NULL) {
		return -1;
	}

	opt.accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL) {
		goto nla_put_failure;
	}
	if (nla_put(skb, TCA_NSSWRR_QDISC_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static int nss_wrr_enqueue(struct sk_buff *skb, struct Qdisc *sch)
#else
static int nss_wrr_enqueue(struct sk_buff *skb, struct Qdisc *sch,
				struct sk_buff **to_free)
#endif
{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	return nss_qdisc_enqueue(skb, sch);
#else
	return nss_qdisc_enqueue(skb, sch, to_free);
#endif
}

static struct sk_buff *nss_wrr_dequeue(struct Qdisc *sch)
{
	return nss_qdisc_dequeue(sch);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static unsigned int nss_wrr_drop(struct Qdisc *sch)
{
	nss_qdisc_info("Nsswrr drop\n");
	return nss_qdisc_drop(sch);
}
#endif

const struct Qdisc_class_ops nss_wrr_class_ops = {
	.change		= nss_wrr_change_class,
	.delete		= nss_wrr_delete_class,
	.graft		= nss_wrr_graft_class,
	.leaf		= nss_wrr_leaf_class,
	.qlen_notify	= nss_wrr_qlen_notify,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.get		= nss_wrr_get_class,
	.put		= nss_wrr_put_class,
#else
	.find       = nss_wrr_search_class,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.tcf_chain	= nss_qdisc_tcf_chain,
#else
	.tcf_block	= nss_qdisc_tcf_block,
#endif
	.bind_tcf	= nss_qdisc_tcf_bind,
	.unbind_tcf	= nss_qdisc_tcf_unbind,
	.dump		= nss_wrr_dump_class,
	.dump_stats	= nss_wrr_dump_class_stats,
	.walk		= nss_wrr_walk
};

struct Qdisc_ops nss_wrr_qdisc_ops __read_mostly = {
	.id		= "nsswrr",
	.init		= nss_wrr_init_qdisc,
	.change		= nss_wrr_change_qdisc,
	.reset		= nss_wrr_reset_qdisc,
	.destroy	= nss_wrr_destroy_qdisc,
	.dump		= nss_wrr_dump_qdisc,
	.enqueue	= nss_wrr_enqueue,
	.dequeue	= nss_wrr_dequeue,
	.peek		= qdisc_peek_dequeued,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		= nss_wrr_drop,
#endif
	.cl_ops		= &nss_wrr_class_ops,
	.priv_size	= sizeof(struct nss_wrr_sched_data),
	.owner		= THIS_MODULE
};

const struct Qdisc_class_ops nss_wfq_class_ops = {
	.change		= nss_wrr_change_class,
	.delete		= nss_wrr_delete_class,
	.graft		= nss_wrr_graft_class,
	.leaf		= nss_wrr_leaf_class,
	.qlen_notify	= nss_wrr_qlen_notify,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.get		= nss_wrr_get_class,
	.put		= nss_wrr_put_class,
#else
	.find       = nss_wrr_search_class,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.tcf_chain	= nss_qdisc_tcf_chain,
#else
	.tcf_block	= nss_qdisc_tcf_block,
#endif
	.bind_tcf	= nss_qdisc_tcf_bind,
	.unbind_tcf	= nss_qdisc_tcf_unbind,
	.dump		= nss_wrr_dump_class,
	.dump_stats	= nss_wrr_dump_class_stats,
	.walk		= nss_wrr_walk
};

struct Qdisc_ops nss_wfq_qdisc_ops __read_mostly = {
	.id		= "nsswfq",
	.init		= nss_wrr_init_qdisc,
	.change		= nss_wrr_change_qdisc,
	.reset		= nss_wrr_reset_qdisc,
	.destroy	= nss_wrr_destroy_qdisc,
	.dump		= nss_wrr_dump_qdisc,
	.enqueue	= nss_wrr_enqueue,
	.dequeue	= nss_wrr_dequeue,
	.peek		= qdisc_peek_dequeued,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		= nss_wrr_drop,
#endif
	.cl_ops		= &nss_wrr_class_ops,
	.priv_size	= sizeof(struct nss_wrr_sched_data),
	.owner		= THIS_MODULE
};
