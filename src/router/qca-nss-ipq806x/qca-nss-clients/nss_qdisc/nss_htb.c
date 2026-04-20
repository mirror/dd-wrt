/*
 **************************************************************************
 * Copyright (c) 2014-2017, 2019-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 *
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

#define NSS_HTB_MAX_PRIORITY 4

/*
 * nss_htb class parameters
 */
struct nss_htb_param {
	u32 rate;		/* Allowed bandwidth for this class */
	u32 burst;		/* Allowed burst for this class */
	u32 crate;		/* Ceil bandwidth for this class */
	u32 cburst;		/* Ceil burst for this class */
	u32 quantum;		/* Quantum allocation for DRR */
	u32 priority;		/* Priority value of this class */
	u32 overhead;		/* Overhead in bytes to be added for each packet */
};

/*
 * nss_htb class instance structure
 */
struct nss_htb_class_data {
	struct nss_qdisc nq;			/* Base class used by nss_qdisc */
	struct Qdisc_class_common sch_common;	/* Common class structure for scheduler use */
	struct nss_htb_class_data *parent;	/* Pointer to our parent class */
	struct Qdisc *qdisc;			/* Child qdisc, used by leaf classes */
	int children;				/* Count of number of attached child classes */
	bool is_leaf;				/* True if leaf class */
	struct nss_htb_param param;		/* Parameters for this class */
};

/*
 * nss_htb qdisc instance structure
 */
struct nss_htb_sched_data {
	struct nss_qdisc nq;		/* Base class used by nss_qdisc */
	u16 r2q;			/* The rate to quantum conversion ratio */
	struct nss_htb_class_data root;	/* Root class */
	struct Qdisc_class_hash clhash;	/* Class hash */
};

/*
 * nss_htb_policy structure
 */
static struct nla_policy nss_htb_policy[TCA_NSSHTB_MAX + 1] = {
	[TCA_NSSHTB_CLASS_PARMS] = { .len = sizeof(struct tc_nsshtb_class_qopt) },
	[TCA_NSSHTB_QDISC_PARMS] = { .len = sizeof(struct tc_nsshtb_qopt) },
};

/*
 * nss_htb_find_class()
 *	Returns a pointer to class if classid matches with a class under this qdisc.
 */
static inline struct nss_htb_class_data *nss_htb_find_class(u32 classid, struct Qdisc *sch)
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;
	clc = qdisc_class_find(&q->clhash, classid);
	if (clc == NULL) {
		nss_qdisc_info("cannot find class with classid %x in qdisc %x hash\n",
					classid, sch->handle);
		return NULL;
	}
	return container_of(clc, struct nss_htb_class_data, sch_common);
}

/*
 * nss_htb_class_params_validate_and_save()
 *	Validates and saves the qdisc configuration parameters.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_htb_class_params_validate_and_save(struct Qdisc *sch, struct nlattr **tca,
					struct nss_htb_param *param)
#else
static int nss_htb_class_params_validate_and_save(struct Qdisc *sch, struct nlattr **tca,
					struct nss_htb_param *param, struct netlink_ext_ack *extack)
#endif
{
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct nlattr *tb[TCA_NSSHTB_MAX + 1];
	struct tc_nsshtb_class_qopt *qopt;
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct net_device *dev = qdisc_dev(sch);
	unsigned int mtu = psched_mtu(dev);

	nss_qdisc_trace("validating parameters for nsshtb class of qdisc:%x\n", sch->handle);

	if (!opt) {
		nss_qdisc_error("passing null opt for configuring htb class %x\n", sch->handle);
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_htb_policy, tb, TCA_NSSHTB_MAX, TCA_NSSHTB_CLASS_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_htb_policy, tb, TCA_NSSHTB_MAX, TCA_NSSHTB_CLASS_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	sch_tree_lock(sch);
	if (qopt->rate && !qopt->burst) {
		nss_qdisc_error("burst needed if rate is non zero - class %x\n", sch->handle);
		sch_tree_unlock(sch);
		return -EINVAL;
	}

	if (!qopt->crate || !qopt->cburst) {
		nss_qdisc_error("crate and cburst need to be non zero - class %x\n",
					sch->handle);
		sch_tree_unlock(sch);
		return -EINVAL;
	}

	if (!(qopt->priority < NSS_HTB_MAX_PRIORITY)) {
		nss_qdisc_error("priority %u of htb class %x greater than max prio %u",
					qopt->priority, sch->handle, NSS_HTB_MAX_PRIORITY);
		sch_tree_unlock(sch);
		return -EINVAL;
	}

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (q->nq.mode == NSS_QDISC_MODE_PPE) {
		if ((qopt->quantum % NSS_PPE_DRR_WEIGHT_MAX) != 0) {
			nss_qdisc_warning("nsshtb (accel_mode %d) requires quantum to be a multiple of 1024\n",
					nss_qdisc_accel_mode_get(&q->nq));
			sch_tree_unlock(sch);
			return -EINVAL;
		}

		if (qopt->quantum > (NSS_PPE_DRR_WEIGHT_MAX * NSS_PPE_DRR_WEIGHT_MAX)) {
			nss_qdisc_warning("nsshtb (accel_mode %d) requires quantum not exceeding 1024*1024\n",
					nss_qdisc_accel_mode_get(&q->nq));
			sch_tree_unlock(sch);
			return -EINVAL;
		}
	}
#endif

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
			nss_qdisc_info("quantum not provided for htb class %x on interface %s\n"
					"Setting quantum to %uB based on r2q %u and rate %uBps\n",
					sch->handle, dev->name, param->quantum, q->r2q, param->rate / 8);
		} else {
			param->quantum = mtu;
			nss_qdisc_info("quantum value not provided for htb class %x on interface %s\n"
					"Setting quantum to MTU %uB\n", sch->handle, dev->name, param->quantum);
		}
	}
	sch_tree_unlock(sch);

	return 0;
}

/*
 * nss_htb_class_alloc()
 *	Allocates a new class.
 */
static struct nss_htb_class_data *nss_htb_class_alloc(struct Qdisc *sch, struct nss_htb_class_data *parent, u32 classid)
{
	struct nss_htb_class_data *cl;
	nss_qdisc_trace("creating a new htb class of qdisc:%x\n", sch->handle);

	/*
	 * check for valid classid
	 */
	if (!classid || TC_H_MAJ(classid ^ sch->handle) || nss_htb_find_class(classid, sch)) {
		return NULL;
	}

	nss_qdisc_trace("htb class %x not found. Allocating a new class.\n", classid);
	cl = kzalloc(sizeof(struct nss_htb_class_data), GFP_KERNEL);

	if (!cl) {
		nss_qdisc_error("class allocation failed for classid %x\n", classid);
		return NULL;
	}

	nss_qdisc_trace("htb class %x allocated - addr %px\n", classid, cl);
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
	nss_qdisc_atomic_set(&cl->nq);

	return cl;
}

#if defined(NSS_QDISC_PPE_SUPPORT)
/*
 * nss_htb_ppe_change_class()
 *	Configures a class in ppe qdisc
 */
static int nss_htb_ppe_change_class(struct Qdisc *sch, struct nss_htb_class_data *cl, struct nss_htb_param *param)
{
	struct nss_ppe_qdisc prev_npq;

	/*
	 * Save the previous scheduler configuration
	 * for handling failure conditions.
	 */
	prev_npq = cl->nq.npq;

	cl->nq.npq.shaper_present = true;
	cl->nq.npq.shaper.rate = param->rate;
	cl->nq.npq.shaper.burst = param->burst;
	cl->nq.npq.shaper.crate = param->crate;
	cl->nq.npq.shaper.cburst = param->cburst;
	cl->nq.npq.shaper.overhead = param->overhead;
	cl->nq.npq.scheduler.drr_weight = nss_ppe_drr_weight_get(param->quantum, NSS_PPE_DRR_UNIT_BYTE);
	cl->nq.npq.scheduler.drr_unit = NSS_PPE_DRR_UNIT_BYTE;
	cl->nq.npq.scheduler.priority = param->priority;

	/*
	 * Change the configuration in SSDK
	 */
	if (nss_ppe_configure(&cl->nq, &prev_npq) != 0)  {
		nss_qdisc_warning("nss_htb %x SSDK scheduler configuration failed\n", sch->handle);
		return -EINVAL;
	}

	cl->param = *param;
	return 0;
}
#endif

/*
 * nss_htb_change_class()
 *	Configures a new class.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_htb_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_htb_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
#endif
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct nss_htb_class_data *cl = (struct nss_htb_class_data *)*arg;
	struct nss_htb_class_data *parent;
	struct nss_qdisc *nq_parent;
	struct nss_htb_param param;
	struct nss_if_msg nim_config;
	struct net_device *dev = qdisc_dev(sch);
	unsigned int mtu = psched_mtu(dev);
	unsigned int accel_mode = nss_qdisc_accel_mode_get(&q->nq);
	bool new_init = false;

	nss_qdisc_trace("configuring htb class %x of qdisc %x\n", classid, sch->handle);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_htb_class_params_validate_and_save(sch, tca, &param) < 0) {
#else
	if (nss_htb_class_params_validate_and_save(sch, tca, &param, extack) < 0) {
#endif
		nss_qdisc_warning("validation of configuration parameters for htb class %x failed\n",
					sch->handle);
		return -EINVAL;
	}

	parent = (parentid == TC_H_ROOT) ? NULL : nss_htb_find_class(parentid, sch);

	/*
	 * The parent could be the htb qdisc, or a class. We set nq_parent pointer
	 * accordingly.
	 */
	if (parent) {
		nq_parent = &parent->nq;
	} else {
		nq_parent = &q->nq;
	}

#if defined(NSS_QDISC_PPE_SUPPORT)
	if (cl && (cl->nq.mode == NSS_QDISC_MODE_PPE)) {
		if (nss_htb_ppe_change_class(sch, cl, &param) < 0) {
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
		cl = nss_htb_class_alloc(sch, parent, classid);

		if (!cl) {
			nss_qdisc_warning("class allocation failed for classid %x\n", classid);
			goto failure;
		}

		*arg = (unsigned long)cl;
		new_init = true;
		nss_qdisc_trace("adding class %x to qdisc %x\n", classid, sch->handle);

		/*
		 * This is where a class gets initialized. Classes do not have a init function
		 * that is registered to Linux. Therefore we initialize the NSSHTB_GROUP shaper
		 * here.
		 */
		cl->nq.parent = nq_parent;
		if (nss_qdisc_init(sch, &cl->nq, NSS_SHAPER_NODE_TYPE_HTB_GROUP, classid, accel_mode, extack) < 0)
		{
			nss_qdisc_error("nss_init for htb class %x failed\n", classid);
			goto failure;
		}

#if defined(NSS_QDISC_PPE_SUPPORT)
		if (cl->nq.mode == NSS_QDISC_MODE_PPE) {
			if (nss_htb_ppe_change_class(sch, cl, &param) < 0) {
				nss_qdisc_warning("nss_htb %x SSDK scheduler configuration failed\n", sch->handle);
				nss_qdisc_destroy(&cl->nq);
				goto failure;
			}
		}
#endif

		/*
		 * Set qos_tag of parent to which the class needs to e attached to.
		 * Set the child to be this class.
		 * Send node_attach command down to the NSS
		 */
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = nq_parent->qos_tag;

		if (cl->nq.mode == NSS_QDISC_MODE_NSS) {
			nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_attach.child_qos_tag = cl->nq.qos_tag;
		}

		if (nss_qdisc_node_attach(nq_parent, &cl->nq, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			nss_qdisc_error("nss_attach for class %x failed\n", classid);
			nss_qdisc_destroy(&cl->nq);
			goto failure;
		}

		/*
		 * We have successfully attached ourselves in the NSS. We can therefore
		 * add this class to qdisc hash tree, and increment parent's child count
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

		nss_qdisc_trace("class %x successfully allocated and initialized\n", classid);
	}

	/*
	 * Fill information that needs to be sent down to the NSS for configuring the
	 * htb class.
	 */
	cl->param = param;
	if (cl->nq.mode == NSS_QDISC_MODE_NSS) {
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.quantum = cl->param.quantum;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.priority = cl->param.priority;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.overhead = cl->param.overhead;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_police.rate = cl->param.rate;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_police.burst = cl->param.burst;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_police.max_size = mtu;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_police.short_circuit = false;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_ceil.rate = cl->param.crate;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_ceil.burst = cl->param.cburst;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_ceil.max_size = mtu;
		nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_param.rate_ceil.short_circuit = false;

		nss_qdisc_info("htb class %x - rate = %ubps burst = %ubytes crate = %ubps cburst = %ubytes MTU = %ubytes "
				"quantum = %ubytes priority = %u\n", classid, cl->param.rate, cl->param.burst, cl->param.crate,
				cl->param.cburst, mtu, cl->param.quantum, cl->param.priority);

		/*
		 * Send configure command to the NSS
		 */
		if (nss_qdisc_configure(&cl->nq, &nim_config,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
			nss_qdisc_error("failed to send configure message for htb class %x\n", classid);

			if (!new_init) {
				return -EINVAL;
			}

			nss_qdisc_destroy(&cl->nq);
			goto failure;
		}
	}

	nss_qdisc_info("htb class %x configured successfully\n", classid);
	return 0;

failure:
	if (cl) {
		kfree(cl);
	}
	return -EINVAL;
}

/*
 * nss_htb_destroy_class()
 *	Detaches all child nodes and destroys the class.
 */
static void nss_htb_destroy_class(struct Qdisc *sch, struct nss_htb_class_data *cl)
{
	struct nss_htb_sched_data *q __maybe_unused = qdisc_priv(sch);
	struct nss_if_msg nim;
	struct nss_qdisc *nq_child;

	nss_qdisc_trace("destroying htb class %x from qdisc %x\n", cl->nq.qos_tag, sch->handle);

	/*
	 * We always have to detach the child qdisc, before destroying it.
	 */
	if (cl->qdisc != &noop_qdisc) {
		nq_child = qdisc_priv(cl->qdisc);
		nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;

		if (cl->nq.mode == NSS_QDISC_MODE_NSS) {
			nim.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_detach.child_qos_tag = nq_child->qos_tag;
		}

		if (nss_qdisc_node_detach(&cl->nq, nq_child, &nim,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			nss_qdisc_error("failed to detach child %x from class %x\n",
					cl->qdisc->handle, q->nq.qos_tag);
			return;
		}
	}

	/*
	 * And now we destroy the child.
	 */
	 nss_qdisc_put(cl->qdisc);

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
 * nss_htb_delete_class()
 *	Detaches a class from operation, but does not destroy it.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
static int nss_htb_delete_class(struct Qdisc *sch, unsigned long arg, struct netlink_ext_ack *extack)
#else
static int nss_htb_delete_class(struct Qdisc *sch, unsigned long arg)
#endif
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct nss_htb_class_data *cl = (struct nss_htb_class_data *)arg;
	struct nss_if_msg nim;
	int refcnt;

	/*
	 * If the class still has child nodes or qdiscs, then we do not
	 * support deleting it.
	 */
	if ((cl->children) || (cl->qdisc != &noop_qdisc)) {
		nss_qdisc_warning("Cannot delete htb class %x with child nodes "
				  "or qdisc attached\n", cl->nq.qos_tag);
		return -EBUSY;
	}

	/*
	 * Check if we are root class or not (parent pointer is NULL for a root class)
	 */
	if (cl->parent) {

		/*
		 * The htb class to be detached has a parent class (i.e. not the root class),
		 * so we need to send a detach msg to its parent class.
		 */
		nss_qdisc_info("detaching from parent htb class %x ", cl->parent->nq.qos_tag);
		nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->parent->nq.qos_tag;
		nim.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_detach.child_qos_tag = cl->nq.qos_tag;
		if (nss_qdisc_node_detach(&q->nq, &cl->nq, &nim, NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			return -EINVAL;
		}
	} else {
		/*
		 * The message to NSS should be sent to the parent of this class
		 */
		nss_qdisc_info("detaching from parent htb qdisc %x", q->nq.qos_tag);
		nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
		if (nss_qdisc_node_detach(&q->nq, &cl->nq, &nim, NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			return -EINVAL;
		}
	}

	sch_tree_lock(sch);
	qdisc_class_hash_remove(&q->clhash, &cl->sch_common);

	/*
	 * If we are root class, we dont have to update our parent.
	 * We simply deduct refcnt and return.
	 * For 5.4 and above kernels, calling nss_htb_destroy_class
	 * explicitly as there is no put_class which would have called
	 * nss_htb_destroy_class when refcnt becomes zero.
	 */
	if (!cl->parent) {
		refcnt = nss_qdisc_atomic_sub_return(&cl->nq);
		sch_tree_unlock(sch);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
		nss_htb_destroy_class(sch, cl);
#endif
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
	 * Decrement refcnt and return
	 */
	refcnt = nss_qdisc_atomic_sub_return(&cl->nq);
	sch_tree_unlock(sch);

	/*
	 * For 5.4 and above kernels, calling nss_htb_destroy_class
	 * explicitly as there is no put_class which would have called
	 * nss_htb_destroy_class when refcnt becomes zero.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	nss_htb_destroy_class(sch, cl);
#endif
	return 0;
}

/*
 * nss_htb_graft_class()
 *	Replaces the qdisc attached to the provided class.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_htb_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new, struct Qdisc **old)
#else
static int nss_htb_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new, struct Qdisc **old,
				struct netlink_ext_ack *extack)
#endif
{
	struct nss_htb_class_data *cl = (struct nss_htb_class_data *)arg;
	struct nss_if_msg nim_detach;
	struct nss_if_msg nim_attach;
	struct nss_qdisc *nq_new = qdisc_priv(new);
	struct nss_qdisc *nq_old;

	nss_qdisc_trace("grafting htb class %x\n", cl->nq.qos_tag);

	if (new == NULL) {
		new = &noop_qdisc;
	}

	sch_tree_lock(sch);
	*old = cl->qdisc;
	sch_tree_unlock(sch);

	/*
	 * Since we initially attached a noop qdisc as child (in Linux),
	 * we do not perform a detach in the NSS if its a noop qdisc.
	 */
	nss_qdisc_info("grafting old: %x with new: %x\n", (*old)->handle, new->handle);
	if (*old != &noop_qdisc) {
		nss_qdisc_trace("detaching old: %x\n", (*old)->handle);
		nq_old = qdisc_priv(*old);
		nim_detach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		nim_detach.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_detach.child_qos_tag = nq_old->qos_tag;
		if (nss_qdisc_node_detach(&cl->nq, nq_old, &nim_detach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
			nss_qdisc_warning("detach of old qdisc %x failed\n", (*old)->handle);
			return -EINVAL;
		}
	}

	/*
	 * If the new qdisc is a noop qdisc, we do not send down an attach command
	 * to the NSS.
	 */
	if (new != &noop_qdisc) {
		nss_qdisc_trace("attaching new: %x\n", new->handle);
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.htb_group_attach.child_qos_tag = nq_new->qos_tag;
		if (nss_qdisc_node_attach(&cl->nq, nq_new, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			nss_qdisc_error("attach of new qdisc %x failed\n", new->handle);
			return -EINVAL;
		}
	}

	/*
	 * Replaced in NSS, now replace in Linux.
	 */
	nss_qdisc_replace(sch, new, &cl->qdisc);

	return 0;
}

/*
 * nss_htb_leaf_class()
 *	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *nss_htb_leaf_class(struct Qdisc *sch, unsigned long arg)
{
	struct nss_htb_class_data *cl = (struct nss_htb_class_data *)arg;
	nss_qdisc_trace("htb class %x is leaf %d\n", cl->nq.qos_tag, cl->is_leaf);

	/*
	 * Return qdisc pointer if this is level 0 class
	 */
	return cl->is_leaf ? cl->qdisc : NULL;
}

/*
 * nss_htb_qlen_notify()
 *	We dont maintain a live set of stats in linux, so this function is not implemented.
 */
static void nss_htb_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
	nss_qdisc_trace("qlen notify called for htb qdisc %x\n", sch->handle);

	/*
	 * Gets called when qlen of child changes (Useful for deactivating)
	 * Not useful for us here.
	 */
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
/*
 * nss_htb_get_class()
 *	Fetches the class pointer if provided the classid.
 */
static unsigned long nss_htb_get_class(struct Qdisc *sch, u32 classid)
{
	struct nss_htb_class_data *cl = nss_htb_find_class(classid, sch);

	if (cl != NULL) {
		nss_qdisc_trace("fetched htb class %x from qdisc %x\n",
				cl->nq.qos_tag, sch->handle);
		atomic_add(1, &cl->nq.refcnt);
	}

	return (unsigned long)cl;
}

/*
 * nss_htb_put_class()
 *	Reduces reference count for this class.
 */
static void nss_htb_put_class(struct Qdisc *sch, unsigned long arg)
{
	struct nss_htb_class_data *cl = (struct nss_htb_class_data *)arg;
	nss_qdisc_trace("executing put on htb class %x in qdisc %x\n",
			cl->nq.qos_tag, sch->handle);

	/*
	 * We are safe to destroy the qdisc if the reference count
	 * goes down to 0.
	 */
	if (nss_qdisc_atomic_sub_return(&cl->nq) == 0) {
		nss_htb_destroy_class(sch, cl);
	}
}
#else
/*
 * nss_htb_search_class()
 *	Fetches the class pointer if provided the classid.
 */
static unsigned long nss_htb_search_class(struct Qdisc *sch, u32 classid)
{
	struct nss_htb_class_data *cl = nss_htb_find_class(classid, sch);

	return (unsigned long)cl;
}
#endif

/*
 * nss_htb_dump_class()
 *	Dumps all configurable parameters pertaining to this class.
 */
static int nss_htb_dump_class(struct Qdisc *sch, unsigned long arg, struct sk_buff *skb, struct tcmsg *tcm)
{
	struct nss_htb_class_data *cl = (struct nss_htb_class_data *)arg;
	struct nlattr *opts;
	struct tc_nsshtb_class_qopt qopt;

	nss_qdisc_trace("dumping htb class %x of qdisc %x\n", cl->nq.qos_tag, sch->handle);

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

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_NSSHTB_CLASS_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	nss_qdisc_error("htb class %x dumo failed\n", cl->nq.qos_tag);
	return -EMSGSIZE;
}

/*
 * nss_htb_dump_class_stats()
 *	Dumps class statistics.
 */
static int nss_htb_dump_class_stats(struct Qdisc *sch, unsigned long arg, struct gnet_dump *d)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)arg;

	if (nss_qdisc_gnet_stats_copy_basic(sch, d, &nq->bstats) < 0 ||
			nss_qdisc_gnet_stats_copy_queue(d, &nq->qstats) < 0) {
		nss_qdisc_error("htb class %x stats dump failed\n", nq->qos_tag);
		return -1;
	}

	return 0;
}

/*
 * nss_htb_walk()
 *	Used to walk the tree.
 */
static void nss_htb_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct nss_htb_class_data *cl;
	unsigned int i;

	nss_qdisc_trace("walking htb qdisc %x\n", sch->handle);

	if (arg->stop)
		return;

	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i],
				sch_common.hnode) {
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
 * nss_htb_change_qdisc()
 *	Can be used to configure a htb qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_htb_change_qdisc(struct Qdisc *sch, struct nlattr *opt)
#else
static int nss_htb_change_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
#endif
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSHTB_MAX + 1];
	struct tc_nsshtb_qopt *qopt;

	/*
	 * Since nsshtb can be created with no arguments, opt might be NULL
	 * (depending on the kernel version). This is still a valid create
	 * request.
	 */
	if (opt == NULL) {

		/*
		 * If no parameter is passed, set it to 0 and continue
		 * creating the qdisc.
		 */
		sch_tree_lock(sch);
		q->r2q = 0;
		sch_tree_unlock(sch);
		return 0;
	}

	/*
	 * If it is not NULL, parse to get qopt.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_htb_policy, tb, TCA_NSSHTB_MAX, TCA_NSSHTB_QDISC_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_htb_policy, tb, TCA_NSSHTB_MAX, TCA_NSSHTB_QDISC_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	nss_qdisc_info("Setting r2q:%u", qopt->r2q);

	sch_tree_lock(sch);
	q->r2q = qopt->r2q;
	sch_tree_unlock(sch);

	/*
	 * The r2q parameter is not needed in the firmware. So we do not
	 * send down a configuration message.
	 */

	return 0;
}

/*
 * nss_htb_reset_class()
 *	Resets child qdisc of class to be reset.
 */
static void nss_htb_reset_class(struct nss_htb_class_data *cl)
{
	if (cl->qdisc == &noop_qdisc) {
		nss_qdisc_trace("Class %x has no child qdisc to reset\n", cl->nq.qos_tag);
		return;
	}

	nss_qdisc_reset(cl->qdisc);
	nss_qdisc_trace("htb class %x reset\n", cl->nq.qos_tag);
}

/*
 * nss_htb_reset_qdisc()
 *	Resets htb qdisc and its classes.
 */
static void nss_htb_reset_qdisc(struct Qdisc *sch)
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct nss_htb_class_data *cl;
	struct hlist_node *n __maybe_unused;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i], sch_common.hnode)
			nss_htb_reset_class(cl);
	}

	nss_qdisc_reset(sch);
	nss_qdisc_trace("nss htb qdisc %x reset\n", sch->handle);
}

/*
 * nss_htb_destroy_qdisc()
 *	Call to destroy a htb qdisc and its associated classes.
 */
static void nss_htb_destroy_qdisc(struct Qdisc *sch)
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct hlist_node *next;
	struct nss_htb_class_data *cl;
	unsigned int i = 0;

	/*
	 * Destroy all the child classes before the parent is destroyed.
	 */
	while (q->clhash.hashelems) {
		nss_qdisc_hlist_for_each_entry_safe(cl, n, next, &q->clhash.hash[i], sch_common.hnode) {

			if (cl->children) {
				continue;
			}

			/*
			 * Reduce refcnt by 1 before destroying. This is to
			 * ensure that polling of stat stops properly.
			 */
			 nss_qdisc_atomic_sub(&cl->nq);

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
			nss_htb_destroy_class(sch, cl);
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
	nss_qdisc_destroy(&q->nq);
	nss_qdisc_info("htb qdisc %x destroyed\n", sch->handle);
}

/*
 * nss_htb_init_qdisc()
 *	Initializes the htb qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_htb_init_qdisc(struct Qdisc *sch, struct nlattr *opt)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_htb_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
#endif
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSHTB_MAX + 1];
	struct tc_nsshtb_qopt *qopt;
	int err;
	unsigned int accel_mode;
	unsigned int r2q = 0;

	nss_qdisc_trace("initializing htb qdisc %x\n", sch->handle);

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0) {
		nss_qdisc_error("hash init failed for htb qdisc %x", sch->handle);
		return err;
	}

	if (!opt) {
		accel_mode = TCA_NSS_ACCEL_MODE_PPE;
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
		qopt = nss_qdisc_qopt_get(opt, nss_htb_policy, tb, TCA_NSSHTB_MAX, TCA_NSSHTB_QDISC_PARMS);
#else
		qopt = nss_qdisc_qopt_get(opt, nss_htb_policy, tb, TCA_NSSHTB_MAX, TCA_NSSHTB_QDISC_PARMS, extack);
#endif
		if (!qopt) {
			return -EINVAL;
		}
		accel_mode = qopt->accel_mode;
		r2q = qopt->r2q;
	}

	nss_qdisc_info("r2q = %u accel_mode = %u\n", r2q, accel_mode);

	/*
	 * Initialize the NSSHTB shaper in NSS
	 */
	if (nss_qdisc_init(sch, &q->nq, NSS_SHAPER_NODE_TYPE_HTB, 0, accel_mode, extack) < 0) {
		nss_qdisc_error("failed to initialize htb qdisc %x in nss", sch->handle);
		return -EINVAL;
	}

	nss_qdisc_info("htb qdisc initialized with handle %x\n", sch->handle);

	/*
	 * Tune HTB parameters
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_htb_change_qdisc(sch, opt) < 0) {
#else
	if (nss_htb_change_qdisc(sch, opt, extack) < 0) {
#endif
		nss_qdisc_destroy(&q->nq);
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_htb_dump_qdisc()
 *	Dumps htb qdisc's configurable parameters.
 */
static int nss_htb_dump_qdisc(struct Qdisc *sch, struct sk_buff *skb)
{
	struct nss_htb_sched_data *q = qdisc_priv(sch);
	struct nlattr *opts = NULL;
	struct tc_nsshtb_qopt qopt;

	nss_qdisc_trace("dumping htb qdisc %x\n", sch->handle);

	qopt.r2q = q->r2q;
	qopt.accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	nss_qdisc_info("r2q = %u accel_mode = %u", qopt.r2q, qopt.accel_mode);

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (!opts || nla_put(skb, TCA_NSSHTB_QDISC_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

 nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * nss_htb_enqueue()
 *	Enqueues a skb to htb qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static int nss_htb_enqueue(struct sk_buff *skb, struct Qdisc *sch)
#else
static int nss_htb_enqueue(struct sk_buff *skb, struct Qdisc *sch,
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
 * nss_htb_dequeue()
 *	Dequeues a skb from htb qdisc.
 */
static struct sk_buff *nss_htb_dequeue(struct Qdisc *sch)
{
	return nss_qdisc_dequeue(sch);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
/*
 * nss_htb_drop()
 *	Drops a single skb from linux queue, if not empty.
 *
 * Does not drop packets that are queued in the NSS.
 */
static unsigned int nss_htb_drop(struct Qdisc *sch)
{
	nss_qdisc_trace("drop called on htb qdisc %x\n", sch->handle);
	return nss_qdisc_drop(sch);
}
#endif

/*
 * Registration structure for htb class
 */
const struct Qdisc_class_ops nss_htb_class_ops = {
	.change		= nss_htb_change_class,
	.delete		= nss_htb_delete_class,
	.graft		= nss_htb_graft_class,
	.leaf		= nss_htb_leaf_class,
	.qlen_notify	= nss_htb_qlen_notify,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.get		= nss_htb_get_class,
	.put		= nss_htb_put_class,
#else
	.find		=	nss_htb_search_class,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.tcf_chain	= nss_qdisc_tcf_chain,
#else
	.tcf_block	= nss_qdisc_tcf_block,
#endif
	.bind_tcf	= nss_qdisc_tcf_bind,
	.unbind_tcf	= nss_qdisc_tcf_unbind,
	.dump		= nss_htb_dump_class,
	.dump_stats	= nss_htb_dump_class_stats,
	.walk		= nss_htb_walk
};

/*
 * Registration structure for htb qdisc
 */
struct Qdisc_ops nss_htb_qdisc_ops __read_mostly = {
	.id		= "nsshtb",
	.init		= nss_htb_init_qdisc,
	.change		= nss_htb_change_qdisc,
	.reset		= nss_htb_reset_qdisc,
	.destroy	= nss_htb_destroy_qdisc,
	.dump		= nss_htb_dump_qdisc,
	.enqueue	= nss_htb_enqueue,
	.dequeue	= nss_htb_dequeue,
	.peek		= qdisc_peek_dequeued,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		= nss_htb_drop,
#endif
	.cl_ops		= &nss_htb_class_ops,
	.priv_size	= sizeof(struct nss_htb_sched_data),
	.owner		= THIS_MODULE
};
