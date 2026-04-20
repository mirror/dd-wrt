/*
 * Copyright (c) 2014-2017, 2019-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "ppe_wrr.h"

/*
 * ppewrr class instance structure
 */
struct ppe_wrr_class_data {
	struct ppe_qdisc pq;			/* Base class used by ppe_qdisc */
	struct Qdisc_class_common cl_common;	/* Common class structure */
	u32 quantum;				/* Quantum allocation for DRR */
	struct Qdisc *qdisc;			/* Pointer to child qdisc */
};

/*
 * ppewrr qdisc instance structure
 */
struct ppe_wrr_sched_data {
	struct ppe_qdisc pq;			/* Base class used by ppe_qdisc */
	struct ppe_wrr_class_data root;		/* Root class */
	struct Qdisc_class_hash clhash; 	/* Class hash */
};

/*
 * ppe_wrr_policy structure
 */
static struct nla_policy ppe_wrr_policy[TCA_PPEWRR_MAX + 1] = {
	[TCA_PPEWRR_CLASS_PARMS] = { .len = sizeof(struct tc_ppewrr_class_qopt) },
	[TCA_PPEWRR_QDISC_PARMS] = { .len = sizeof(struct tc_ppewrr_qopt) },
};

/*
 * ppe_wrr_find_class()
 * 	Returns a pointer to class if classid matches with a class under this qdisc.
 */
static inline struct ppe_wrr_class_data *ppe_wrr_find_class(u32 classid,
		struct Qdisc *sch)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;

	clc = qdisc_class_find(&q->clhash, classid);
	if (clc == NULL) {
		ppe_qdisc_info("%x Cannot find class with classid %u in qdisc %px hash table %px", sch->handle, classid, sch, &q->clhash);
		return NULL;
	}
	return container_of(clc, struct ppe_wrr_class_data, cl_common);
}

/*
 * ppe_wrr_destroy_class()
 * 	Detaches all child nodes and destroys the class.
 */
static void ppe_wrr_destroy_class(struct Qdisc *sch, struct ppe_wrr_class_data *cl)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);

	ppe_qdisc_info("%x Destroying ppewrr class %px from qdisc %px", sch->handle, cl, sch);

	/*
	 * Note, this function gets called even for PPEWRR and not just for PPEWRR_GROUP.
	 * If this is wrr qdisc then we should not call ppe_qdisc_destroy or stop polling
	 * for stats. These two actions will happen inside ppe_wrr_destroy(), which is called
	 * only for the root qdisc.
	 */
	if (cl == &q->root) {
		ppe_qdisc_info("%x We do not destroy ppewrr class %px here since this is "
				"the qdisc %px", sch->handle, cl, sch);
		return;
	}

	/*
	 * We always have to detach our child qdisc in PPE, before destroying it.
	 */
	if (cl->qdisc != &noop_qdisc) {
		struct ppe_qdisc *pq_child = qdisc_priv(cl->qdisc);

		ppe_qdisc_node_detach(&cl->pq, pq_child);
		ppe_qdisc_warning("%x Detached child %x from class %x", sch->handle,
				cl->qdisc->handle, q->pq.qos_tag);
	}

	/*
	 * And now we destroy the child.
	 * Destroy the shaper in PPE
	 */
	qdisc_put(cl->qdisc);
	ppe_qdisc_destroy(&cl->pq);

	/*
	 * Free class
	 */
	kfree(cl);
}

/*
 * ppe_wrr_class_params_validate_and_save()
 * 	Validates and saves the class configuration parameters.
 */
static int ppe_wrr_class_params_validate_and_save(struct Qdisc *sch, struct nlattr **tca,
		uint32_t *quantum, struct netlink_ext_ack *extack)
{
	struct nlattr *tb[TCA_PPEWRR_MAX + 1];
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct tc_ppewrr_class_qopt *qopt;
	struct net_device *dev = qdisc_dev(sch);
	bool is_wrr = (sch->ops == &ppe_wrr_qdisc_ops);

	ppe_qdisc_trace("Validating parameters for ppewrr class of qdisc:%x", sch->handle);

	if (!opt) {
		return -EINVAL;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_wrr_policy, tb, TCA_PPEWRR_MAX, TCA_PPEWRR_CLASS_PARMS, extack);

	if (!qopt) {
		return -EINVAL;
	}

	sch_tree_lock(sch);

	/*
	 * If the value of quantum is not provided default it based on the type
	 * of operation (i.e. wrr or wfq)
	 */
	*quantum = qopt->quantum;

	if (is_wrr) {
		if (qopt->quantum >= PPE_DRV_QOS_DRR_WEIGHT_MAX) {
			ppe_qdisc_warning("quantum %u of ppewrr class of qdisc %x should be a less than 1024",
					qopt->quantum, sch->handle);
			sch_tree_unlock(sch);
			return -EINVAL;
		}
	} else {
		if ((qopt->quantum % PPE_DRV_QOS_DRR_WEIGHT_MAX) != 0) {
			ppe_qdisc_warning("%x ppe_wfq requires quantum to be a multiple of 1024", sch->handle);
			sch_tree_unlock(sch);
			return -EINVAL;
		}

		if (qopt->quantum > (PPE_DRV_QOS_DRR_WEIGHT_MAX * PPE_DRV_QOS_DRR_WEIGHT_MAX)) {
			ppe_qdisc_warning("%x ppe_wfq requires quantum not exceeding 1024*1024", sch->handle);
			sch_tree_unlock(sch);
			return -EINVAL;
		}
	}
	if (!*quantum) {
		if (is_wrr) {
			*quantum = 1;
			ppe_qdisc_info("%x Quantum value not provided for ppewrr class on interface %s. "
					"Setting quantum to %up", sch->handle, dev->name, *quantum);
		} else {
			*quantum = psched_mtu(dev);
			ppe_qdisc_info("%x Quantum value not provided for ppewfq class on interface %s. "
					"Setting quantum to %ubytes", sch->handle, dev->name, *quantum);
		}
	}

	sch_tree_unlock(sch);

	return 0;
}

/*
 * ppe_wrr_configure_class()
 * 	Configures a class in ppe qdisc
 */
static int ppe_wrr_configure_class(struct Qdisc *sch, struct ppe_wrr_class_data *cl, uint32_t quantum)
{
	struct ppe_qdisc prev_pq;
	bool is_wrr = (sch->ops == &ppe_wrr_qdisc_ops);
	struct ppe_drv_qos_res *res = &cl->pq.res;

	/*
	 * Save the previous scheduler configuration
	 * for handling failure conditions.
	 */
	prev_pq = cl->pq;

	/*
	 * Quantum is specified in bytes for WFQ while it is in packets for WRR.
	 */
	if (is_wrr) {
		res->scheduler.drr_weight = ppe_qdisc_drr_weight_get(quantum, PPE_DRV_QOS_DRR_UNIT_PACKET);
		res->scheduler.drr_unit = PPE_DRV_QOS_DRR_UNIT_PACKET;
	} else {
		res->scheduler.drr_weight = ppe_qdisc_drr_weight_get(quantum, PPE_DRV_QOS_DRR_UNIT_BYTE);
		res->scheduler.drr_unit = PPE_DRV_QOS_DRR_UNIT_BYTE;
	}

	/*
	 * Change the configuration in SSDK
	 */
	if (ppe_qdisc_configure(&cl->pq, &prev_pq) < 0) {
		ppe_qdisc_warning("%x ppewrr SSDK scheduler configuration failed", sch->handle);
		return -EINVAL;
	}

	cl->quantum = quantum;
	return 0;
}

/*
 * ppe_wrr_change_class()
 * 	Configures a new class.
 */
static int ppe_wrr_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct ppe_wrr_class_data *cl = (struct ppe_wrr_class_data *)*arg;
	bool new_init = false;
	uint32_t quantum;

	ppe_qdisc_info("%x Changing ppewrr class %u", sch->handle, classid);

	if (ppe_wrr_class_params_validate_and_save(sch, tca, &quantum, extack) < 0) {
		ppe_qdisc_warning("Validation of configuration parameters for wrr class %x failed",
				sch->handle);
		return -EINVAL;
	}

	if (cl) {
		if (ppe_wrr_configure_class(sch, cl, quantum) < 0) {
			ppe_qdisc_warning("%x ppewrr SSDK scheduler configuration failed", sch->handle);
			return -EINVAL;
		}

		cl->quantum = quantum;
			return 0;
	}

	/*
	 * If class with a given classid is not found, we allocate a new one
	 */
	if (!cl) {

		new_init = true;

		ppe_qdisc_info("%x ppewrr class %u not found. Allocating a new class.", sch->handle, classid);
		cl = kzalloc(sizeof(struct ppe_wrr_class_data), GFP_KERNEL);

		if (!cl) {
			ppe_qdisc_warning("%x Class allocation failed for classid %u", sch->handle, classid);
			return -EINVAL;
		}

		ppe_qdisc_info("%x ppewrr class %u allocated %px", sch->handle, classid, cl);
		cl->cl_common.classid = classid;

		/*
		 * We make the child qdisc a noop qdisc, and
		 * set reference count to 1. This is important,
		 * reference count should not be 0.
		 */
		cl->qdisc = &noop_qdisc;
		ppe_qdisc_atomic_set(&cl->pq);
		*arg = (unsigned long)cl;

		ppe_qdisc_info("%x Adding classid %u to qdisc %px hash queue %px", sch->handle, classid, sch, &q->clhash);

		/*
		 * This is where a class gets initialized. Classes do not have a init function
		 * that is registered to Linux. Therefore we initialize the PPEWRR_GROUP shaper
		 * here.
		 */
		cl->pq.parent = &q->pq;
		if (ppe_qdisc_init(sch, &cl->pq, PPE_QDISC_NODE_TYPE_WRR_GROUP, classid, extack) < 0) {
			ppe_qdisc_warning("%x Ppe init for class %u failed", sch->handle, classid);
			return -EINVAL;
		}

		if (ppe_wrr_configure_class(sch, cl, quantum) < 0) {
			ppe_qdisc_warning("%x ppewrr SSDK scheduler configuration failed", sch->handle);
			ppe_qdisc_destroy(&cl->pq);
			goto failure;
		}

		if (ppe_qdisc_node_attach(&q->pq, &cl->pq) < 0) {
			ppe_qdisc_warning("%x Ppe attach for class %u failed", sch->handle, classid);
			ppe_qdisc_destroy(&cl->pq);
			goto failure;
		}

		/*
		 * Add class to hash tree once it is attached in the PPE
		 */
		sch_tree_lock(sch);
		qdisc_class_hash_insert(&q->clhash, &cl->cl_common);
		sch_tree_unlock(sch);

		/*
		 * Hash grow should not come within the tree lock
		 */
		qdisc_class_hash_grow(sch, &q->clhash);

		ppe_qdisc_info("%x Class %u successfully allocated", sch->handle, classid);
	}

	cl->quantum = quantum;

	/*
	 * Fill information that needs to be sent down to the PPE for configuring the
	 * wrr class.
	 */

	ppe_qdisc_info("%x Class %u changed successfully", sch->handle, classid);
	return 0;

failure:
	if (cl) {
		kfree(cl);
	}
	return -EINVAL;
}

/*
 * ppe_wrr_delete_class()
 * 	Detaches a class from operation, but does not destroy it.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
static int ppe_wrr_delete_class(struct Qdisc *sch, unsigned long arg)
#else
static int ppe_wrr_delete_class(struct Qdisc *sch, unsigned long arg,
		struct netlink_ext_ack *extack)
#endif
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct ppe_wrr_class_data *cl = (struct ppe_wrr_class_data *)arg;
	int refcnt;

	/*
	 * If the class is a root class or has a child qdisc attached
	 * we do not support deleting it.
	 */
	if ((cl == &q->root) || (cl->qdisc != &noop_qdisc)) {
		ppe_qdisc_warning("%x Cannot delete wrr class %x as it is the "
				"root class or has a child qdisc attached", sch->handle, cl->pq.qos_tag);
		return -EBUSY;
	}

	/*
	 * The message to PPE should be sent to the parent of this class
	 */
	ppe_qdisc_info("%x Detaching ppewrr class: %px", sch->handle, cl);
	ppe_qdisc_node_detach(&q->pq, &cl->pq);

	sch_tree_lock(sch);
	qdisc_class_hash_remove(&q->clhash, &cl->cl_common);

	refcnt = ppe_qdisc_atomic_sub_return(&cl->pq);

	sch_tree_unlock(sch);

	ppe_wrr_destroy_class(sch, cl);
	return 0;
}

/*
 * ppe_wrr_graft_class()
 * 	Replaces the qdisc attached to the provided class.
 */
static int ppe_wrr_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
		struct Qdisc **old, struct netlink_ext_ack *extack)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct ppe_wrr_class_data *cl = (struct ppe_wrr_class_data *)arg;
	struct ppe_qdisc *pq_new, *pq_child;

	ppe_qdisc_info("%x Grafting class", sch->handle);

	if (cl == &q->root) {
		ppe_qdisc_warning("%x Can't graft root class %px", sch->handle, cl);
		return -EINVAL;
	}

	if (new == NULL)
		new = &noop_qdisc;

	sch_tree_lock(sch);
	*old = cl->qdisc;
	sch_tree_unlock(sch);

	/*
	 * Since we initially attached a noop qdisc as child (in Linux),
	 * we do not perform a detach in the PPE if its a noop qdisc.
	 */
	ppe_qdisc_info("%x Grafting old: %px with new: %px", sch->handle, *old, new);
	if (*old != &noop_qdisc) {
		pq_child = qdisc_priv(*old);

		ppe_qdisc_info("%x Detaching old: %px", sch->handle, *old);
		ppe_qdisc_node_detach(&cl->pq, pq_child);
	}

	/*
	 * If the new qdisc is a noop qdisc, we do not send down an attach command
	 * to the PPE.
	 */
	if (new != &noop_qdisc) {
		ppe_qdisc_info("%x Attaching new: %px", sch->handle, new);
		if (!(new->flags & TCQ_F_NSS)) {
			ppe_qdisc_warning("non-ppe qdisc %px used along with ppe qdisc", new);
			return -EINVAL;
		}
		pq_new = qdisc_priv(new);
		if (ppe_qdisc_node_attach(&cl->pq, pq_new) < 0) {
			return -EINVAL;
		}
	}

	/*
	 * Replaced in PPE, now replace in Linux.
	 */
	ppe_qdisc_replace(sch, new, &cl->qdisc);

	ppe_qdisc_info("%x ppewrr grafted", sch->handle);

	return 0;
}

/*
 * ppe_wrr_leaf_class()
 * 	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *ppe_wrr_leaf_class(struct Qdisc *sch, unsigned long arg)
{
	struct ppe_wrr_class_data *cl = (struct ppe_wrr_class_data *)arg;

	ppe_qdisc_info("%x ppewrr class leaf %px", sch->handle, cl);

	/*
	 * Since all ppewrr groups are leaf nodes, we can always
	 * return the attached qdisc.
	 */
	return cl->qdisc;
}

/*
 * ppe_wrr_qlen_notify()
 * 	We dont maintain a live set of stats in linux, so this function is not implemented.
 */
static void ppe_wrr_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
	/*
	 * Gets called when qlen of child changes (Useful for deactivating)
	 * Not useful for us here.
	 */
	ppe_qdisc_info("%x ppewrr qlen notify", sch->handle);
}

/*
 * ppe_wrr_search_class()
 * 	Fetches the class pointer if provided the classid.
 */
static unsigned long ppe_wrr_search_class(struct Qdisc *sch, u32 classid)
{
	struct ppe_wrr_class_data *cl = ppe_wrr_find_class(classid, sch);

	ppe_qdisc_info("%x Get ppewrr class %px - class match = %px", sch->handle, sch, cl);

	if (cl) {
		atomic_add(1, &cl->pq.refcnt.refs);
	}

	return (unsigned long)cl;
}

/*
 * ppe_wrr_dump_class()
 * 	Dumps all configurable parameters pertaining to this class.
 */
static int ppe_wrr_dump_class(struct Qdisc *sch, unsigned long arg, struct sk_buff *skb,
		struct tcmsg *tcm)
{
	struct ppe_wrr_class_data *cl = (struct ppe_wrr_class_data *)arg;
	struct nlattr *opts;
	struct tc_ppewrr_class_qopt qopt;

	ppe_qdisc_info("Dumping class %px of Qdisc %x", cl, sch->handle);

	qopt.quantum = cl->quantum;

	/*
	 * All bf group nodes are root nodes. i.e. they dont
	 * have any mode bf groups attached beneath them.
	 */
	tcm->tcm_parent = TC_H_ROOT;
	tcm->tcm_handle = cl->cl_common.classid;
	tcm->tcm_info = cl->qdisc->handle;

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (!opts || nla_put(skb, TCA_PPEWRR_CLASS_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}
	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_wrr_dump_class_stats()
 * 	Dumps class statistics.
 */
static int ppe_wrr_dump_class_stats(struct Qdisc *sch, unsigned long arg, struct gnet_dump *d)
{
	struct ppe_qdisc *pq = (struct ppe_qdisc *)arg;

	if (ppe_qdisc_gnet_stats_copy_basic(sch, d, &pq->bstats) < 0 ||
			ppe_qdisc_gnet_stats_copy_queue(d, &pq->qstats) < 0) {
		return -1;
	}

	return 0;
}

/*
 * ppe_wrr_walk()
 * 	Used to walk the tree.
 */
static void ppe_wrr_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct ppe_wrr_class_data *cl;
	unsigned int i;

	ppe_qdisc_info("%x In ppewrr walk", sch->handle);
	if (arg->stop)
		return;

	for (i = 0; i < q->clhash.hashsize; i++) {
		ppe_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i],
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

/*
 * ppe_wrr_change_qdisc()
 * 	Can be used to configure a wrr qdisc.
 */
static int ppe_wrr_change_qdisc(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	/*
	 * WRR has no qdisc parameters that can be changed.
	 */
	ppe_qdisc_warning("%x WRR has no qdisc parameters that can be changed.", sch->handle);

	return 0;
}

/*
 * ppe_wrr_init_qdisc()
 * 	Initializes the wrr qdisc.
 */
static int ppe_wrr_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
		struct netlink_ext_ack *extack)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	int err;

	ppe_qdisc_info("%x Init ppewrr qdisc", sch->handle);

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0) {
		return err;
	}
	q->root.cl_common.classid = sch->handle;
	q->root.qdisc = &noop_qdisc;

	qdisc_class_hash_insert(&q->clhash, &q->root.cl_common);
	qdisc_class_hash_grow(sch, &q->clhash);

	/*
	 * Initialize the PPEWRR shaper in PPE
	 */
	if (ppe_qdisc_init(sch, &q->pq, PPE_QDISC_NODE_TYPE_WRR, 0, extack) < 0) {
		ppe_qdisc_warning("%x Failed init ppewrr qdisc", sch->handle);
		return -EINVAL;
	}

	if (ppe_wrr_change_qdisc(sch, opt, extack) < 0) {
		ppe_qdisc_destroy(&q->pq);
		return -EINVAL;
	}

	ppe_qdisc_info("ppewrr initialized - handle %x parent %x", sch->handle, sch->parent);

	return 0;
}

/*
 * ppe_wrr_reset_class()
 * 	Resets child qdisc of class to be reset.
 */
static void ppe_wrr_reset_class(struct ppe_wrr_class_data *cl)
{
	if (cl->qdisc == &noop_qdisc) {
		ppe_qdisc_trace("Class %x has no child qdisc to reset", cl->pq.qos_tag);
		return;
	}

	ppe_qdisc_reset(cl->qdisc);
	ppe_qdisc_info("ppewrr class resetted %px", cl->qdisc);
}

/*
 * ppe_wrr_reset_qdisc()
 * 	Resets wrr qdisc and its classes.
 */
static void ppe_wrr_reset_qdisc(struct Qdisc *sch)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct ppe_wrr_class_data *cl;
	struct hlist_node *n __maybe_unused;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		ppe_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i], cl_common.hnode)
			ppe_wrr_reset_class(cl);
	}

	ppe_qdisc_reset(sch);
	ppe_qdisc_info("%x ppewrr qdisc resetted", sch->handle);
}

/*
 * ppe_wrr_destroy_qdisc()
 * 	Call to destroy a wrr qdisc and its associated classes.
 */
static void ppe_wrr_destroy_qdisc(struct Qdisc *sch)
{
	struct ppe_wrr_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct hlist_node *next;
	struct ppe_wrr_class_data *cl;
	unsigned int i;

	/*
	 * Destroy all the classes before the root qdisc is destroyed.
	 */
	for (i = 0; i < q->clhash.hashsize; i++) {
		ppe_qdisc_hlist_for_each_entry_safe(cl, n, next, &q->clhash.hash[i], cl_common.hnode) {

			/*
			 * If this is the root class, we dont have to destroy it. This will be taken
			 * care of by the ppe_wrr_destroy() function.
			 */
			if (cl == &q->root) {
				ppe_qdisc_info("%px We do not detach or destroy ppewrr class %px here since this is "
						"the qdisc %x", sch, cl, sch->handle);
				continue;
			}

			/*
			 * Reduce refcnt by 1 before destroying. This is to
			 * ensure that polling of stat stops properly.
			 */
			ppe_qdisc_atomic_sub(&cl->pq);

			/*
			 * Detach class before destroying it. We dont check for noop qdisc here
			 * since we do not attach anu such at init.
			 */
			ppe_qdisc_info("%x Detaching Node for qdisc %x class %x", sch->handle, cl->pq.qos_tag, q->pq.qos_tag);
			ppe_qdisc_node_detach(&q->pq, &cl->pq);

			/*
			 * Now we can destroy the class.
			 */
			ppe_wrr_destroy_class(sch, cl);
		}
	}
	qdisc_class_hash_destroy(&q->clhash);

	/*
	 * Now we can go ahead and destroy the qdisc.
	 * Note: We dont have to detach ourself from our parent because this
	 * will be taken care of by the graft call.
	 */
	ppe_qdisc_destroy(&q->pq);
	ppe_qdisc_info("%x ppewrr destroyed", sch->handle);
}

/*
 * ppe_wrr_dump_qdisc()
 * 	Dumps wrr qdisc's configurable parameters.
 */
static int ppe_wrr_dump_qdisc(struct Qdisc *sch, struct sk_buff *skb)
{
	struct ppe_wrr_sched_data *q;
	struct nlattr *opts = NULL;
	struct tc_ppewrr_qopt opt;

	ppe_qdisc_info("%x ppewrr Dumping", sch->handle);

	q = qdisc_priv(sch);
	if (q == NULL) {
		return -1;
	}

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL) {
		goto nla_put_failure;
	}
	if (nla_put(skb, TCA_PPEWRR_QDISC_PARMS, sizeof(opt), &opt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_wrr_epqueue()
 * 	Enqueues a skb to wrr qdisc.
 */
static int ppe_wrr_enqueue(struct sk_buff *skb, struct Qdisc *sch,
		struct sk_buff **to_free)
{
	return ppe_qdisc_enqueue(skb, sch, to_free);
}

/*
 * ppe_wrr_dequeue()
 * 	Dequeues a skb from wrr qdisc.
 */
static struct sk_buff *ppe_wrr_dequeue(struct Qdisc *sch)
{
	return ppe_qdisc_dequeue(sch);
}

/*
 * Registration structure for wrr class
 */
const struct Qdisc_class_ops ppe_wrr_class_ops = {
	.change         = ppe_wrr_change_class,
	.delete         = ppe_wrr_delete_class,
	.graft          = ppe_wrr_graft_class,
	.leaf           = ppe_wrr_leaf_class,
	.qlen_notify    = ppe_wrr_qlen_notify,
	.find       = ppe_wrr_search_class,
	.tcf_block      = ppe_qdisc_tcf_block,
	.bind_tcf       = ppe_qdisc_tcf_bind,
	.unbind_tcf     = ppe_qdisc_tcf_unbind,
	.dump           = ppe_wrr_dump_class,
	.dump_stats     = ppe_wrr_dump_class_stats,
	.walk           = ppe_wrr_walk
};

/*
 * Registration structure for wrr qdisc
 */
struct Qdisc_ops ppe_wrr_qdisc_ops __read_mostly = {
	.id             = "ppewrr",
	.init           = ppe_wrr_init_qdisc,
	.change         = ppe_wrr_change_qdisc,
	.reset          = ppe_wrr_reset_qdisc,
	.destroy        = ppe_wrr_destroy_qdisc,
	.dump           = ppe_wrr_dump_qdisc,
	.enqueue        = ppe_wrr_enqueue,
	.dequeue        = ppe_wrr_dequeue,
	.peek           = qdisc_peek_dequeued,
	.cl_ops         = &ppe_wrr_class_ops,
	.priv_size      = sizeof(struct ppe_wrr_sched_data),
	.owner          = THIS_MODULE
};

/*
 * Registration structure for wfq class
 */
const struct Qdisc_class_ops ppe_wfq_class_ops = {
	.change         = ppe_wrr_change_class,
	.delete         = ppe_wrr_delete_class,
	.graft          = ppe_wrr_graft_class,
	.leaf           = ppe_wrr_leaf_class,
	.qlen_notify    = ppe_wrr_qlen_notify,
	.find       = ppe_wrr_search_class,
	.tcf_block      = ppe_qdisc_tcf_block,
	.bind_tcf       = ppe_qdisc_tcf_bind,
	.unbind_tcf     = ppe_qdisc_tcf_unbind,
	.dump           = ppe_wrr_dump_class,
	.dump_stats     = ppe_wrr_dump_class_stats,
	.walk           = ppe_wrr_walk
};

/*
 * Registration structure for wfq qdisc
 */
struct Qdisc_ops ppe_wfq_qdisc_ops __read_mostly = {
	.id             = "ppewfq",
	.init           = ppe_wrr_init_qdisc,
	.change         = ppe_wrr_change_qdisc,
	.reset          = ppe_wrr_reset_qdisc,
	.destroy        = ppe_wrr_destroy_qdisc,
	.dump           = ppe_wrr_dump_qdisc,
	.enqueue        = ppe_wrr_enqueue,
	.dequeue        = ppe_wrr_dequeue,
	.peek           = qdisc_peek_dequeued,
	.cl_ops         = &ppe_wrr_class_ops,
	.priv_size      = sizeof(struct ppe_wrr_sched_data),
	.owner          = THIS_MODULE
};

