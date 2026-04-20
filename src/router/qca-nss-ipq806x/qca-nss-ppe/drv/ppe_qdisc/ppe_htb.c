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

/*
 * TODO: Based on level, this needs be updated
 */
#define PPE_HTB_MAX_PRIORITY 4

/*
 * ppe_htb_param
 *	PPE HTB class parameters
 */
struct ppe_htb_param {
	u32 rate;		/* Allowed bandwidth for this class */
	u32 burst;		/* Allowed burst for this class */
	u32 crate;		/* Ceil bandwidth for this class */
	u32 cburst;		/* Ceil burst for this class */
	u32 quantum;		/* Quantum allocation for DRR */
	u32 priority;		/* Priority value of this class */
	u32 overhead;		/* Overhead in bytes to be added for each packet */
};

/*
 * ppe_htb_class_data
 *	PPE HTB class instance structure
 */
struct ppe_htb_class_data {
	struct ppe_qdisc pq;			/* Base class used by ppe_qdisc */
	struct Qdisc_class_common sch_common;	/* Common class structure for scheduler use */
	struct ppe_htb_class_data *parent;	/* Pointer to our parent class */
	struct Qdisc *qdisc;			/* Child qdisc, used by leaf classes */
	int children;				/* Count of number of attached child classes */
	bool is_leaf;				/* True if leaf class */
	struct ppe_htb_param param;		/* Parameters for this class */
};

/*
 * ppe_htb_sched_data
 *	PPE HTB qdisc instance structure
 */
struct ppe_htb_sched_data {
	struct ppe_qdisc pq;		/* Base class used by ppe_qdisc */
	u16 r2q;			/* The rate to quantum conversion ratio */
	struct ppe_htb_class_data root;	/* Root class */
	struct Qdisc_class_hash clhash;	/* Class hash */
};

/*
 * nla_policy
 *	PPE HTB policy structure
 */
static struct nla_policy ppe_htb_policy[TCA_PPEHTB_MAX + 1] = {
	[TCA_PPEHTB_CLASS_PARMS] = { .len = sizeof(struct tc_ppehtb_class_qopt) },
	[TCA_PPEHTB_QDISC_PARMS] = { .len = sizeof(struct tc_ppehtb_qopt) },
};

/*
 * ppe_htb_find_class()
 *	Returns a pointer to class if classid matches with a class under this qdisc.
 */
static inline struct ppe_htb_class_data *ppe_htb_find_class(u32 classid, struct Qdisc *sch)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;
	clc = qdisc_class_find(&q->clhash, classid);
	if (clc) {
		return container_of(clc, struct ppe_htb_class_data, sch_common);
	}

	ppe_qdisc_info("%x: ppehtb cannot find class with classid %x in qdisc hash",
		sch->handle, classid);
	return NULL;
}

/*
 * ppe_htb_class_params_validate_and_save()
 *	Validates and saves the qdisc configuration parameters.
 */
static int ppe_htb_class_params_validate_and_save(struct Qdisc *sch, struct nlattr **tca,
			struct ppe_htb_param *param, struct netlink_ext_ack *extack)
{
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct nlattr *tb[TCA_PPEHTB_MAX + 1];
	struct tc_ppehtb_class_qopt *qopt;
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct net_device *dev = qdisc_dev(sch);
	unsigned int mtu = psched_mtu(dev);

	ppe_qdisc_trace("%x: ppehtb validating parameters for class", sch->handle);

	if (!opt) {
		ppe_qdisc_warning("%x: ppehtb passing null opt for configuring class", sch->handle);
		return -EINVAL;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_htb_policy, tb, TCA_PPEHTB_MAX, TCA_PPEHTB_CLASS_PARMS, extack);
	if (!qopt) {
		return -EINVAL;
	}

	sch_tree_lock(sch);
	if (qopt->rate && !qopt->burst) {
		sch_tree_unlock(sch);
		ppe_qdisc_warning("%x: ppehtb burst needed if rate is non zero", sch->handle);
		return -EINVAL;
	}

	if (!qopt->crate || !qopt->cburst) {
		sch_tree_unlock(sch);
		ppe_qdisc_warning("%x: ppehtb crate and cburst need to be non zero", sch->handle);
		return -EINVAL;
	}

	if (!(qopt->priority < PPE_HTB_MAX_PRIORITY)) {
		sch_tree_unlock(sch);
		ppe_qdisc_warning("%x: ppehtb priority %u is greater than max prio %u",
			sch->handle, qopt->priority, PPE_HTB_MAX_PRIORITY);
		return -EINVAL;
	}

	if ((qopt->quantum % PPE_DRV_QOS_DRR_WEIGHT_MAX) != 0) {
		sch_tree_unlock(sch);
		ppe_qdisc_warning("%x: ppehtb requires quantum to be a multiple of 1024", sch->handle);
		return -EINVAL;
	}

	if (qopt->quantum > (PPE_DRV_QOS_DRR_WEIGHT_MAX * PPE_DRV_QOS_DRR_WEIGHT_MAX)) {
		sch_tree_unlock(sch);
		ppe_qdisc_warning("%x: ppehtb requires quantum not exceeding 1024*1024", sch->handle);
		sch_tree_unlock(sch);
		return -EINVAL;
	}

	memset(param, 0, sizeof(*param));
	param->rate = qopt->rate;
	param->burst = qopt->burst;
	param->crate = qopt->crate;
	param->cburst = qopt->cburst;
	param->overhead = qopt->overhead;
	param->quantum = qopt->quantum;
	param->priority = qopt->priority;

	/*
	 * If quantum value is not provided, set it to
	 * the interface's MTU value.
	 */
	if (!param->quantum) {
		/*
		 * If quantum was not provided, we have two options.
		 * One, use r2q and rate to figure out the quantum. Else,
		 * use the interface's MTU as the value of quantum.
		 */
		if (q->r2q && param->rate) {
			param->quantum = (param->rate / q->r2q) / 8;
			ppe_qdisc_info("%x: ppehtb quantum not provided on interface %s"
				"Setting quantum to %uB based on r2q %u and rate %uBps",
				sch->handle, dev->name, param->quantum, q->r2q, param->rate / 8);
		} else {
			param->quantum = mtu;
			ppe_qdisc_info("%x: ppehtb quantum value not provided on interface %s"
				"Setting quantum to MTU %uB", sch->handle, dev->name, param->quantum);
		}
	}
	sch_tree_unlock(sch);

	return 0;
}

/*
 * ppe_htb_class_alloc()
 *	Allocates a new class.
 */
static struct ppe_htb_class_data *ppe_htb_class_alloc(struct Qdisc *sch, struct ppe_htb_class_data *parent, u32 classid)
{
	struct ppe_htb_class_data *cl;
	ppe_qdisc_trace("%x: ppehtb creating a new htb class", sch->handle);

	/*
	 * check for valid classid
	 */
	if (!classid || TC_H_MAJ(classid ^ sch->handle) || ppe_htb_find_class(classid, sch)) {
		return NULL;
	}

	ppe_qdisc_trace("%x: ppehtb class %x not found. Allocating a new class.", sch->handle, classid);
	cl = kzalloc(sizeof(struct ppe_htb_class_data), GFP_KERNEL);
	if (!cl) {
		ppe_qdisc_warning("%x: ppehtb class allocation failed for classid %x", sch->handle, classid);
		return NULL;
	}

	ppe_qdisc_trace("%x: ppehtb class %x allocated - addr %px", sch->handle, classid, cl);
	cl->parent = parent;
	cl->sch_common.classid = classid;

	/*
	 * Set this class as leaf. If a new class is attached as
	 * child, it will set this value to false during the attach
	 * process.
	 */
	cl->is_leaf = true;

	/*
	 * We make the child qdisc a noop qdisc, and
	 * set reference count to 1. This is important,
	 * reference count should not be 0.
	 */
	cl->qdisc = &noop_qdisc;
	ppe_qdisc_atomic_set(&cl->pq);

	return cl;
}

/*
 * ppe_htb_ppe_change_class()
 *	Configures a class in ppe qdisc
 */
static void ppe_htb_class_params_fill(struct ppe_qdisc *pq, struct ppe_htb_param *param)
{
	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_SHAPER_VALID);
	pq->res.shaper.rate = param->rate;
	pq->res.shaper.burst = param->burst;
	pq->res.shaper.crate = param->crate;
	pq->res.shaper.cburst = param->cburst;
	pq->res.shaper.overhead = param->overhead;
	pq->res.scheduler.drr_weight = ppe_qdisc_drr_weight_get(param->quantum, PPE_DRV_QOS_DRR_UNIT_BYTE);
	pq->res.scheduler.drr_unit = PPE_DRV_QOS_DRR_UNIT_BYTE;
	pq->res.scheduler.priority = param->priority;
}

/*
 * ppe_htb_change_class()
 *	Configures a new class.
 */
static int ppe_htb_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct ppe_htb_class_data *cl = (struct ppe_htb_class_data *)*arg;
	struct ppe_htb_class_data *parent;
	struct ppe_qdisc *pq_parent;
	struct ppe_qdisc *prev_pq = NULL;
	struct ppe_htb_param param;
	bool new_init = false;

	ppe_qdisc_trace("%x: ppehtb configuring htb class %x ", sch->handle, classid);

	if (ppe_htb_class_params_validate_and_save(sch, tca, &param, extack) < 0) {
		ppe_qdisc_warning("%x: ppehtb validation of configuration parameters for htb class %x failed",
				sch->handle, classid);
		return -EINVAL;
	}

	parent = (parentid == TC_H_ROOT) ? NULL : ppe_htb_find_class(parentid, sch);

	/*
	 * The parent could be the htb qdisc, or a class. We set pq_parent pointer
	 * accordingly.
	 */
	if (parent) {
		pq_parent = &parent->pq;
	} else {
		pq_parent = &q->pq;
	}

	if (cl) {
		/*
		 * Save previous configuration for reset purpose.
		 */
		if (ppe_qdisc_flags_check(&cl->pq, PPE_QDISC_FLAG_NODE_CONFIGURED)) {
			prev_pq = &cl->pq;
		}

		ppe_htb_class_params_fill(&cl->pq, &param);

		/*
		 * Change the configuration in SSDK
		 */
		if (ppe_qdisc_configure(&cl->pq, prev_pq) != 0)  {
			ppe_qdisc_warning("%x: ppehtb SSDK scheduler configuration failedfor classid %x", sch->handle, classid);
			return -EINVAL;
		}

		cl->param = param;
		return 0;
	}

	/*
	 * If class with a given classid is not found, we allocate a new one
	 */
	if (!cl) {
		cl = ppe_htb_class_alloc(sch, parent, classid);

		if (!cl) {
			ppe_qdisc_warning("%x: ppehtb class allocation failed for classid %x", sch->handle, classid);
			goto failure;
		}

		*arg = (unsigned long)cl;
		new_init = true;
		ppe_qdisc_trace("%x: ppehtb adding class  %x to qdisc", sch->handle, classid);

		/*
		 * This is where a class gets initialized. Classes do not have a init function
		 * that is registered to Linux. Therefore we initialize the PPEHTB_GROUP shaper
		 * here.
		 */
		cl->pq.parent = pq_parent;
		if (ppe_qdisc_init(sch, &cl->pq, PPE_QDISC_NODE_TYPE_HTB_GROUP, classid, extack) < 0) {
			ppe_qdisc_warning("%x: ppehtb init for htb class %x failed", sch->handle, classid);
			goto failure;
		}

		ppe_htb_class_params_fill(&cl->pq, &param);

		/*
		 * Change the configuration in SSDK
		 */
		if (ppe_qdisc_configure(&cl->pq, prev_pq) != 0)  {
			ppe_qdisc_warning("%x: ppehtb SSDK scheduler configuration failed for htb class %x ", sch->handle, classid);
			ppe_qdisc_destroy(&cl->pq);
			goto failure;
		}

		/*
		 * We can add this class to qdisc hash tree, and increment parent's child count
		 * (if parent exists)
		 */
		sch_tree_lock(sch);
		qdisc_class_hash_insert(&q->clhash, &cl->sch_common);
		if (parent) {
			parent->children++;

			/*
			 * Parent can no longer be leaf. Set flag to false.
			 */
			parent->is_leaf = false;
		}
		sch_tree_unlock(sch);

		/*
		 * Hash grow should not come within the tree lock
		 */
		qdisc_class_hash_grow(sch, &q->clhash);

		ppe_qdisc_trace("%x: ppehtb class %x successfully allocated and initialized", sch->handle, classid);
	}

	cl->param = param;
	ppe_qdisc_info("%x: ppehtb class %x configured successfully", sch->handle, classid);
	return 0;

failure:
	if (cl) {
		kfree(cl);
	}
	return -EINVAL;
}

/*
 * ppe_htb_destroy_class()
 *	Detaches all child nodes and destroys the class.
 */
static void ppe_htb_destroy_class(struct Qdisc *sch, struct ppe_htb_class_data *cl)
{
	struct ppe_qdisc *pq_child;

	ppe_qdisc_trace("%x: ppehtb destroying htb class %x from qdisc", sch->handle, cl->pq.qos_tag);

	/*
	 * We always have to detach the child qdisc, before destroying it.
	 */
	if (cl->qdisc != &noop_qdisc) {
		pq_child = qdisc_priv(cl->qdisc);
		ppe_qdisc_node_detach(&cl->pq, pq_child);
	}

	qdisc_put(cl->qdisc);
	ppe_qdisc_destroy(&cl->pq);
	kfree(cl);
}

/*
 * ppe_htb_delete_class()
 *	Detaches a class from operation, but does not destroy it.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
static int ppe_htb_delete_class(struct Qdisc *sch, unsigned long arg)
#else
static int ppe_htb_delete_class(struct Qdisc *sch, unsigned long arg,
		struct netlink_ext_ack *extack)
#endif
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct ppe_htb_class_data *cl = (struct ppe_htb_class_data *)arg;
	int refcnt;

	if (!cl) {
		ppe_qdisc_info("%x: ppehtb cannot find class", sch->handle);
		return -EINVAL;
	}

	/*
	 * If the class still has child nodes or qdiscs, then we do not
	 * support deleting it.
	 */
	if ((cl->children) || (cl->qdisc != &noop_qdisc)) {
		ppe_qdisc_warning("%x: ppehtb cannot delete htb class with child nodes "
			  "or qdisc attached", cl->pq.qos_tag);
		return -EBUSY;
	}

	ppe_qdisc_info("%x: ppehtb detaching from parent htb qdisc", q->pq.qos_tag);
	ppe_qdisc_node_detach(&q->pq, &cl->pq);

	sch_tree_lock(sch);
	qdisc_class_hash_remove(&q->clhash, &cl->sch_common);

	/*
	 * If we are root class, we dont have to update our parent.
	 * We simply deduct refcnt.
	 * For 5.4 and above kernels, calling ppe_htb_destroy_class
	 * explicitly as there is no put_class which would have called
	 * ppe_htb_destroy_class when refcnt becomes zero.
	 */
	if (!cl->parent) {
		refcnt = ppe_qdisc_atomic_sub_return(&cl->pq);
		sch_tree_unlock(sch);
		ppe_htb_destroy_class(sch, cl);
		return 0;
	}

	/*
	 * We are not root class. Therefore we reduce the children count
	 * for our parent and also update its 'is_leaf' status.
	 */
	cl->parent->children--;
	if (!cl->parent->children) {
		cl->parent->is_leaf = true;
	}

	/*
	 * Decrement refcnt
	 */
	refcnt = ppe_qdisc_atomic_sub_return(&cl->pq);
	sch_tree_unlock(sch);

	/*
	 * For 5.4 and above kernels, calling ppe_htb_destroy_class
	 * explicitly as there is no put_class which would have called
	 * ppe_htb_destroy_class when refcnt becomes zero.
	 */
	ppe_htb_destroy_class(sch, cl);
	return 0;
}

/*
 * ppe_htb_graft_class()
 *	Replaces the qdisc attached to the provided class.
 */
static int ppe_htb_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new, struct Qdisc **old,
				struct netlink_ext_ack *extack)
{
	struct ppe_htb_class_data *cl = (struct ppe_htb_class_data *)arg;
	struct ppe_qdisc *pq_new, *pq_old;

	ppe_qdisc_trace("%x: ppehtb grafting htb class", cl->pq.qos_tag);

	if (!cl) {
		ppe_qdisc_info("%x: ppehtb cannot find class", sch->handle);
		return -EINVAL;
	}

	if (!new) {
		new = &noop_qdisc;
	}

	sch_tree_lock(sch);
	*old = cl->qdisc;
	sch_tree_unlock(sch);

	ppe_qdisc_info("%x: ppehtb grafting old with new: %x", (*old)->handle, new->handle);
	if (*old != &noop_qdisc) {
		ppe_qdisc_trace("%x: ppehtb detaching old", (*old)->handle);
		pq_old = qdisc_priv(*old);
		ppe_qdisc_node_detach(&cl->pq, pq_old);
	}

	if (new != &noop_qdisc) {
		ppe_qdisc_trace("%x: ppehtb attaching new: %x", cl->pq.qos_tag, new->handle);
		if (!(new->flags & TCQ_F_NSS)) {
			ppe_qdisc_warning("non-ppe qdisc %px used along with ppe qdisc", new);
			return -EINVAL;
		}

		pq_new = qdisc_priv(new);
		if (ppe_qdisc_node_attach(&cl->pq, pq_new) < 0) {
			ppe_qdisc_warning("%x: ppehtb class attach of new qdisc: %x failed", cl->pq.qos_tag, new->handle);
			return -EINVAL;
		}
	}

	/*
	 * Replaced in PPE, now replace in Linux.
	 */
	ppe_qdisc_replace(sch, new, &cl->qdisc);

	return 0;
}

/*
 * ppe_htb_leaf_class()
 *	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *ppe_htb_leaf_class(struct Qdisc *sch, unsigned long arg)
{
	struct ppe_htb_class_data *cl = (struct ppe_htb_class_data *)arg;
	ppe_qdisc_trace("%x: ppehtb class is leaf %d", cl->pq.qos_tag, cl->is_leaf);

	/*
	 * Return qdisc pointer if this is level 0 class
	 */
	return cl->is_leaf ? cl->qdisc : NULL;
}

/*
 * ppe_htb_qlen_notify()
 *	We dont maintain a live set of stats in linux, so this function is not implemented.
 */
static void ppe_htb_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
	ppe_qdisc_trace("%x: ppehtb qlen notify called", sch->handle);

	/*
	 * Gets called when qlen of child changes (Useful for deactivating)
	 * Not useful for us here.
	 */
}

/*
 * ppe_htb_search_class()
 *	Fetches the class pointer if provided the classid.
 */
static unsigned long ppe_htb_search_class(struct Qdisc *sch, u32 classid)
{
	struct ppe_htb_class_data *cl = ppe_htb_find_class(classid, sch);

	return (unsigned long)cl;
}

/*
 * ppe_htb_dump_class()
 *	Dumps all configurable parameters pertaining to this class.
 */
static int ppe_htb_dump_class(struct Qdisc *sch, unsigned long arg, struct sk_buff *skb, struct tcmsg *tcm)
{
	struct ppe_htb_class_data *cl = (struct ppe_htb_class_data *)arg;
	struct nlattr *opts;
	struct tc_ppehtb_class_qopt qopt;

	ppe_qdisc_trace("%x: ppehtb dumping class of qdisc %x", cl->pq.qos_tag, sch->handle);

	qopt.burst = cl->param.burst;
	qopt.rate = cl->param.rate;
	qopt.crate = cl->param.crate;
	qopt.cburst = cl->param.cburst;
	qopt.overhead = cl->param.overhead;
	qopt.quantum = cl->param.quantum;
	qopt.priority = cl->param.priority;

	/*
	 * All htb group nodes are root nodes. i.e. they dont
	 * have any mode htb groups attached beneath them.
	 */
	tcm->tcm_parent = TC_H_ROOT;
	tcm->tcm_handle = cl->sch_common.classid;
	tcm->tcm_info = cl->qdisc->handle;

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_PPEHTB_CLASS_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	ppe_qdisc_warning("%x: ppehtb class dump failed", cl->pq.qos_tag);
	return -EMSGSIZE;
}

/*
 * ppe_htb_dump_class_stats()
 *	Dumps class statistics.
 */
static int ppe_htb_dump_class_stats(struct Qdisc *sch, unsigned long arg, struct gnet_dump *d)
{
	struct ppe_qdisc *pq = (struct ppe_qdisc *)arg;

	if (ppe_qdisc_gnet_stats_copy_basic(sch, d, &pq->bstats) < 0 ||
			ppe_qdisc_gnet_stats_copy_queue(d, &pq->qstats) < 0) {
		ppe_qdisc_warning("%x: ppehtb class stats dump failed", pq->qos_tag);
		return -1;
	}

	return 0;
}

/*
 * ppe_htb_walk()
 *	Used to walk the tree.
 */
static void ppe_htb_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct ppe_htb_class_data *cl;
	unsigned int i;

	ppe_qdisc_trace("%x: ppehtb walking qdisc", sch->handle);

	if (arg->stop)
		return;

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry(cl, &q->clhash.hash[i], sch_common.hnode) {
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
 * ppe_htb_change_qdisc()
 *	Can be used to configure a htb qdisc.
 */
static int ppe_htb_change_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPEHTB_MAX + 1];
	struct tc_ppehtb_qopt *qopt;

	/*
	 * Since ppehtb can be created with no arguments, opt might be NULL
	 * (depending on the kernel version). This is still a valid create
	 * request.
	 */
	if (!opt) {

		/*
		 * If no parameter is passed, set it to 0 and continue
		 * creating the qdisc.
		 */
		sch_tree_lock(sch);
		q->r2q = 0;
		sch_tree_unlock(sch);
		return 0;
	}

	qopt = ppe_qdisc_qopt_get(opt, ppe_htb_policy, tb, TCA_PPEHTB_MAX, TCA_PPEHTB_QDISC_PARMS, extack);
	if (!qopt) {
		return -EINVAL;
	}

	ppe_qdisc_info("%x: ppehtb setting r2q:%u", sch->handle, qopt->r2q);

	sch_tree_lock(sch);
	q->r2q = qopt->r2q;
	sch_tree_unlock(sch);

	/*
	 * The r2q parameter is not needed in PPE HW. So we do not
	 * send down a configuration message.
	 */
	return 0;
}

/*
 * ppe_htb_reset_class()
 *	Resets child qdisc of class to be reset.
 */
static void ppe_htb_reset_class(struct ppe_htb_class_data *cl)
{
	if (cl->qdisc == &noop_qdisc) {
		ppe_qdisc_trace("%x: ppehtb Class has no child qdisc to reset", cl->pq.qos_tag);
		return;
	}

	ppe_qdisc_reset(cl->qdisc);
	ppe_qdisc_trace("%x: ppehtb class reset", cl->pq.qos_tag);
}

/*
 * ppe_htb_reset_qdisc()
 *	Resets htb qdisc and its classes.
 */
static void ppe_htb_reset_qdisc(struct Qdisc *sch)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct ppe_htb_class_data *cl;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		hlist_for_each_entry(cl, &q->clhash.hash[i], sch_common.hnode)
			ppe_htb_reset_class(cl);
	}

	ppe_qdisc_reset(sch);
	ppe_qdisc_trace("%x: ppehtb reset", sch->handle);
}

/*
 * ppe_htb_destroy_qdisc()
 *	Call to destroy a htb qdisc and its associated classes.
 */
static void ppe_htb_destroy_qdisc(struct Qdisc *sch)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct hlist_node *next;
	struct ppe_htb_class_data *cl;
	unsigned int i = 0;

	/*
	 * Destroy all child classes before the parent is destroyed.
	 */
	while (q->clhash.hashelems) {
		hlist_for_each_entry_safe(cl, next, &q->clhash.hash[i], sch_common.hnode) {

			if (cl->children) {
				continue;
			}

			/*
			 * Reduce refcnt by 1 before destroying. This is to
			 * ensure that polling of stat stops properly.
			 */
			 ppe_qdisc_atomic_sub(&cl->pq);

			/*
			 * We are not root class. Therefore we reduce the children count
			 * for our parent.
			 */
			sch_tree_lock(sch);
			if (cl->parent) {
				cl->parent->children--;
			}
			qdisc_class_hash_remove(&q->clhash, &cl->sch_common);
			sch_tree_unlock(sch);

			/*
			 * Now we can destroy the class.
			 */
			ppe_htb_destroy_class(sch, cl);
		}
		i++;

		/*
		 * In the first iteration, all the leaf nodes will be removed.
		 * Now the intermediate nodes (one above the leaf nodes) are
		 * leaf nodes. So, to delete the entire tree level wise,
		 * wrap around the index.
		 */
		if (i == q->clhash.hashsize) {
			i = 0;
		}
	}
	qdisc_class_hash_destroy(&q->clhash);

	/*
	 * Now we can go ahead and destroy the qdisc.
	 * Note: We dont have to detach ourself from our parent because this
	 *	 will be taken care of by the graft call.
	 */
	ppe_qdisc_destroy(&q->pq);
	ppe_qdisc_info("%x: ppehtb destroyed", sch->handle);
}

/*
 * ppe_htb_init_qdisc()
 *	Initializes the htb qdisc.
 */
static int ppe_htb_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
			struct netlink_ext_ack *extack)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_PPEHTB_MAX + 1];
	struct tc_ppehtb_qopt *qopt;
	int err;
	unsigned int r2q = 0;

	ppe_qdisc_trace("%x: ppehtb initializing", sch->handle);

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0) {
		ppe_qdisc_warning("%x: ppehtb hash init failed", sch->handle);
		return err;
	}

	if (opt) {
		qopt = ppe_qdisc_qopt_get(opt, ppe_htb_policy, tb, TCA_PPEHTB_MAX, TCA_PPEHTB_QDISC_PARMS, extack);
		if (!qopt) {
			return -EINVAL;
		}
		r2q = qopt->r2q;
	}

	ppe_qdisc_info("%x: ppehtb r2q = %u", sch->handle, r2q);

	/*
	 * Initialize the PPEHTB shaper in NSS
	 */
	if (ppe_qdisc_init(sch, &q->pq, PPE_QDISC_NODE_TYPE_HTB, 0, extack) < 0) {
		ppe_qdisc_warning("%x: ppehtb failed to initialize", sch->handle);
		return -EINVAL;
	}

	ppe_qdisc_info("%x: ppehtb initialized with handle", sch->handle);

	/*
	 * Tune HTB parameters
	 */
	if (ppe_htb_change_qdisc(sch, opt, extack) < 0) {
		ppe_qdisc_destroy(&q->pq);
		return -EINVAL;
	}

	return 0;
}

/*
 * ppe_htb_dump_qdisc()
 *	Dumps htb qdisc's configurable parameters.
 */
static int ppe_htb_dump_qdisc(struct Qdisc *sch, struct sk_buff *skb)
{
	struct ppe_htb_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts = NULL;
	struct tc_ppehtb_qopt qopt;

	ppe_qdisc_trace("%x: ppehtb dumping qdisc", sch->handle);

	qopt.r2q = q->r2q;

	ppe_qdisc_info("%x: ppehtb r2q = %u", sch->handle, qopt.r2q);

	opts = ppe_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (!opts || nla_put(skb, TCA_PPEHTB_QDISC_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

 nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * ppe_htb_epqueue()
 *	Enqueues a skb to htb qdisc.
 */
static int ppe_htb_enqueue(struct sk_buff *skb, struct Qdisc *sch,
			struct sk_buff **to_free)
{
	return ppe_qdisc_enqueue(skb, sch, to_free);
}

/*
 * ppe_htb_dequeue()
 *	Dequeues a skb from htb qdisc.
 */
static struct sk_buff *ppe_htb_dequeue(struct Qdisc *sch)
{
	return ppe_qdisc_dequeue(sch);
}

/*
 * Registration structure for htb class
 */
const struct Qdisc_class_ops ppe_htb_class_ops = {
	.change			= ppe_htb_change_class,
	.delete			= ppe_htb_delete_class,
	.graft			= ppe_htb_graft_class,
	.leaf			= ppe_htb_leaf_class,
	.qlen_notify		= ppe_htb_qlen_notify,
	.find			= ppe_htb_search_class,
	.tcf_block		= ppe_qdisc_tcf_block,
	.bind_tcf		= ppe_qdisc_tcf_bind,
	.unbind_tcf		= ppe_qdisc_tcf_unbind,
	.dump			= ppe_htb_dump_class,
	.dump_stats		= ppe_htb_dump_class_stats,
	.walk			= ppe_htb_walk
};

/*
 * Registration structure for htb qdisc
 */
struct Qdisc_ops ppe_htb_qdisc_ops __read_mostly = {
	.id		= "ppehtb",
	.init		= ppe_htb_init_qdisc,
	.change		= ppe_htb_change_qdisc,
	.reset		= ppe_htb_reset_qdisc,
	.destroy	= ppe_htb_destroy_qdisc,
	.dump		= ppe_htb_dump_qdisc,
	.enqueue	= ppe_htb_enqueue,
	.dequeue	= ppe_htb_dequeue,
	.peek		= qdisc_peek_dequeued,
	.cl_ops		= &ppe_htb_class_ops,
	.priv_size	= sizeof(struct ppe_htb_sched_data),
	.owner		= THIS_MODULE
};
