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

/*
 * nss_bf class instance structure
 */
struct nss_bf_class_data {
	struct nss_qdisc nq;			/* Base class used by nss_qdisc */
	struct Qdisc_class_common cl_common;	/* Common class structure */
	u32 rate;				/* Allowed bandwidth for this class */
	u32 burst;				/* Allowed burst for this class */
	u32 mtu;				/* MTU size of the interface */
	u32 quantum;				/* Quantum allocation for DRR */
	struct Qdisc *qdisc;			/* Pointer to child qdisc */
};

/*
 * nss_bf qdisc instance structure
 */
struct nss_bf_sched_data {
	struct nss_qdisc nq;			/* Base class used by nss_qdisc */
	u16 defcls;				/* default class id */
	struct nss_bf_class_data root;		/* root class */
	struct Qdisc_class_hash clhash;		/* class hash */
};

/*
 * nss_bf_policy structure
 */
static struct nla_policy nss_bf_policy[TCA_NSSBF_MAX + 1] = {
	[TCA_NSSBF_CLASS_PARMS] = { .len = sizeof(struct tc_nssbf_class_qopt) },
	[TCA_NSSBF_QDISC_PARMS] = { .len = sizeof(struct tc_nssbf_qopt) },
};

/*
 * nss_bf_find_class()
 *	Returns a pointer to class if classid matches with a class under this qdisc.
 */
static inline struct nss_bf_class_data *nss_bf_find_class(u32 classid,
							struct Qdisc *sch)
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct Qdisc_class_common *clc;
	clc = qdisc_class_find(&q->clhash, classid);
	if (clc == NULL) {
		nss_qdisc_info("Cannot find class with classid %u in qdisc %px hash table %px\n", classid, sch, &q->clhash);
		return NULL;
	}
	return container_of(clc, struct nss_bf_class_data, cl_common);
}

/*
 * nss_bf_change_class()
 *	Configures a new class.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_bf_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_bf_change_class(struct Qdisc *sch, u32 classid, u32 parentid,
		  struct nlattr **tca, unsigned long *arg, struct netlink_ext_ack *extack)
{
#endif
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nss_bf_class_data *cl = (struct nss_bf_class_data *)*arg;
	struct nlattr *opt = tca[TCA_OPTIONS];
	struct nlattr *tb[TCA_NSSBF_MAX + 1];
	struct tc_nssbf_class_qopt *qopt;
	struct nss_if_msg nim_config;
	struct net_device *dev = qdisc_dev(sch);
	unsigned int accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	nss_qdisc_info("Changing bf class %u\n", classid);
	if (!opt) {
		return -EINVAL;
	}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_bf_policy, tb, TCA_NSSBF_MAX, TCA_NSSBF_CLASS_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_bf_policy, tb, TCA_NSSBF_MAX, TCA_NSSBF_CLASS_PARMS, extack);
#endif

	if (!qopt) {
		return -EINVAL;
	}

	/*
	 * If class with a given classid is not found, we allocate a new one
	 */
	if (!cl) {
		struct nss_if_msg nim_attach;
		nss_qdisc_info("Bf class %u not found. Allocating a new class.\n", classid);
		cl = kzalloc(sizeof(struct nss_bf_class_data), GFP_KERNEL);

		if (!cl) {
			nss_qdisc_error("Class allocation failed for classid %u\n", classid);
			return -EINVAL;
		}

		nss_qdisc_info("Bf class %u allocated %px\n", classid, cl);
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
		 * that is registered to Linux. Therefore we initialize the NSSBF_GROUP shaper
		 * here.
		 */
		if (nss_qdisc_init(sch, &cl->nq, NSS_SHAPER_NODE_TYPE_BF_GROUP, classid, accel_mode, extack) < 0)
		{
			nss_qdisc_error("Nss init for class %u failed\n", classid);
			kfree(cl);
			return -EINVAL;
		}

		/*
		 * Set qos_tag of parent to which the class needs to be attached to.
		 */
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;

		/*
		 * Set the child to be this class.
		 */
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_attach.child_qos_tag = cl->nq.qos_tag;

		/*
		 * Send node_attach command down to the NSS
		 */
		if (nss_qdisc_node_attach(&q->nq, &cl->nq, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			nss_qdisc_error("Nss attach for class %u failed\n", classid);
			nss_qdisc_destroy(&cl->nq);
			kfree(cl);
			return -EINVAL;
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

		nss_qdisc_info("Class %u successfully allocated\n", classid);
	}

	sch_tree_lock(sch);
	cl->rate = qopt->rate;
	cl->burst = qopt->burst;

	/*
	 * If MTU and quantum values are not provided, set them to
	 * the interface's MTU value.
	 */
	if (!qopt->mtu) {
		cl->mtu = psched_mtu(dev);
		nss_qdisc_info("MTU not provided for bf class on interface %s. "
				"Setting MTU to %u bytes\n", dev->name, cl->mtu);
	} else {
		cl->mtu = qopt->mtu;
	}

	if (!qopt->quantum) {
		cl->quantum = psched_mtu(dev);
		nss_qdisc_info("Quantum value not provided for bf class on interface %s. "
				"Setting quantum to %u\n", dev->name, cl->quantum);
	} else {
		cl->quantum = qopt->quantum;
	}

	sch_tree_unlock(sch);

	/*
	 * Fill information that needs to be sent down to the NSS for configuring the
	 * bf class.
	 */
	nim_config.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
	nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_group_param.quantum = cl->quantum;
	nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_group_param.lap.rate = cl->rate;
	nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_group_param.lap.burst = cl->burst;
	nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_group_param.lap.max_size = cl->mtu;
	nim_config.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_group_param.lap.short_circuit = false;

	nss_qdisc_info("Rate = %u Burst = %u MTU = %u Quantum = %u\n", cl->rate, cl->burst, cl->mtu, cl->quantum);

	/*
	 * Send configure command to the NSS
	 */
	if (nss_qdisc_configure(&cl->nq, &nim_config,
			NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_CHANGE_PARAM) < 0) {
		nss_qdisc_error("Failed to configure class %u\n", classid);
		return -EINVAL;
	}

	nss_qdisc_info("Class %u changed successfully\n", classid);
	return 0;
}

/*
 * nss_bf_destroy_class()
 *	Detaches all child nodes and destroys the class.
 */
static void nss_bf_destroy_class(struct Qdisc *sch, struct nss_bf_class_data *cl)
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nss_if_msg nim;

	nss_qdisc_info("Destroying bf class %px from qdisc %px\n", cl, sch);

	/*
	 * Note, this function gets called even for NSSBF and not just for NSSBF_GROUP.
	 * If this is BF qdisc then we should not call nss_qdisc_destroy or stop polling
	 * for stats. These two actions will happen inside nss_bf_destroy(), which is called
	 * only for the root qdisc.
	 */
	if (cl == &q->root) {
		nss_qdisc_info("We do not destroy bf class %px here since this is "
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
	 * Destroy the shaper in NSS
	 */
	nss_qdisc_destroy(&cl->nq);

	/*
	 * Free class
	 */
	kfree(cl);
}

/*
 * nss_bf_delete_class()
 *	Detaches a class from operation, but does not destroy it.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 12, 0))
static int nss_bf_delete_class(struct Qdisc *sch, unsigned long arg, struct netlink_ext_ack *extack)
#else
static int nss_bf_delete_class(struct Qdisc *sch, unsigned long arg)
#endif
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nss_bf_class_data *cl = (struct nss_bf_class_data *)arg;
	struct nss_if_msg nim;
	int refcnt;
	struct nss_qdisc *nq_child = (struct nss_qdisc *)qdisc_priv(cl->qdisc);

	/*
	 * If the class is the root class or has qdiscs attached, we do not
	 * support deleting it.
	 */
	if ((cl == &q->root) || (cl->qdisc != &noop_qdisc)) {
		nss_qdisc_warning("Cannot delete bf class %x as it is the root "
				  "class or has child qdisc attached\n", cl->nq.qos_tag);
		return -EBUSY;
	}

	/*
	 * The message to NSS should be sent to the parent of this class
	 */
	nss_qdisc_info("Detaching bf class: %px\n", cl);
	nim.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = q->nq.qos_tag;
	nim.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_detach.child_qos_tag = cl->nq.qos_tag;
	if (nss_qdisc_node_detach(&q->nq, nq_child, &nim,
			NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
		return -EINVAL;
	}

	sch_tree_lock(sch);
	qdisc_class_hash_remove(&q->clhash, &cl->cl_common);
	refcnt = nss_qdisc_atomic_sub_return(&cl->nq);
	sch_tree_unlock(sch);

	/*
	 * For 5.4 and above kernels, calling nss_htb_destroy_class
	 * explicitly as there is no put_class which would have called
	 * nss_bf_destroy_class when refcnt becomes zero.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0))
	nss_bf_destroy_class(sch, cl);
#else
	if (!refcnt) {
		nss_qdisc_error("Reference count should not be zero for class %px\n", cl);
	}
#endif
	return 0;
}

/*
 * nss_bf_graft_class()
 *	Replaces the qdisc attached to the provided class.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_bf_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
								 struct Qdisc **old)
#else
static int nss_bf_graft_class(struct Qdisc *sch, unsigned long arg, struct Qdisc *new,
								 struct Qdisc **old, struct netlink_ext_ack *extack)
#endif
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nss_bf_class_data *cl = (struct nss_bf_class_data *)arg;
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
		struct nss_qdisc *nq_old = (struct nss_qdisc *)qdisc_priv(*old);
		nss_qdisc_info("Detaching old: %px\n", *old);
		nim_detach.msg.shaper_configure.config.msg.shaper_node_config.qos_tag = cl->nq.qos_tag;
		if (nss_qdisc_node_detach(&cl->nq, nq_old, &nim_detach,
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
		nim_attach.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_group_attach.child_qos_tag = nq_new->qos_tag;
		if (nss_qdisc_node_attach(&cl->nq, nq_new, &nim_attach,
				NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_ATTACH) < 0) {
			return -EINVAL;
		}
	}

	/*
	 * Replaced in NSS, now replace in Linux.
	 */
	nss_qdisc_replace(sch, new, &cl->qdisc);

	nss_qdisc_info("Nssbf grafted");

	return 0;
}

/*
 * nss_bf_leaf_class()
 *	Returns pointer to qdisc if leaf class.
 */
static struct Qdisc *nss_bf_leaf_class(struct Qdisc *sch, unsigned long arg)
{
	struct nss_bf_class_data *cl = (struct nss_bf_class_data *)arg;
	nss_qdisc_info("bf class leaf %px\n", cl);

	/*
	 * Since all nss_bf groups are leaf nodes, we can always
	 * return the attached qdisc.
	 */
	return cl->qdisc;
}

/*
 * nss_bf_qlen_notify()
 *	We dont maintain a live set of stats in linux, so this function is not implemented.
 */
static void nss_bf_qlen_notify(struct Qdisc *sch, unsigned long arg)
{
	nss_qdisc_info("bf qlen notify %px\n", sch);
	/*
	 * Gets called when qlen of child changes (Useful for deactivating)
	 * Not useful for us here.
	 */
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
/*
 * nss_bf_get_class()
 *	Fetches the class pointer if provided the classid.
 */
static unsigned long nss_bf_get_class(struct Qdisc *sch, u32 classid)
{
	struct nss_bf_class_data *cl = nss_bf_find_class(classid, sch);

	nss_qdisc_info("Get bf class %px - class match = %px\n", sch, cl);

	if (cl != NULL)
		atomic_add(1, &cl->nq.refcnt);

	return (unsigned long)cl;
}

/*
 * nss_bf_put_class()
 *	Reduces reference count for this class.
 */
static void nss_bf_put_class(struct Qdisc *sch, unsigned long arg)
{
	struct nss_bf_class_data *cl = (struct nss_bf_class_data *)arg;
	nss_qdisc_info("bf put class for %px\n", cl);

	/*
	 * We are safe to destroy the qdisc if the reference count
	 * goes down to 0.
	 */
	if (nss_qdisc_atomic_sub_return(&cl->nq) == 0) {
		nss_bf_destroy_class(sch, cl);
	}
}
#else
/*
 * nss_bf_search_class()
 *	Fetches the class pointer if provided the classid.
 */
static unsigned long nss_bf_search_class(struct Qdisc *sch, u32 classid)
{
	struct nss_bf_class_data *cl = nss_bf_find_class(classid, sch);

	nss_qdisc_info("Get bf class %px - class match = %px\n", sch, cl);

	return (unsigned long)cl;
}
#endif

/*
 * nss_bf_dump_class()
 *	Dumps all configurable parameters pertaining to this class.
 */
static int nss_bf_dump_class(struct Qdisc *sch, unsigned long arg, struct sk_buff *skb,
		struct tcmsg *tcm)
{
	struct nss_bf_class_data *cl = (struct nss_bf_class_data *)arg;
	struct nlattr *opts;
	struct tc_nssbf_class_qopt qopt;

	nss_qdisc_info("Dumping class %px of Qdisc %px\n", cl, sch);

	qopt.burst = cl->burst;
	qopt.rate = cl->rate;
	qopt.mtu = cl->mtu;
	qopt.quantum = cl->quantum;

	/*
	 * All bf group nodes are root nodes. i.e. they dont
	 * have any mode bf groups attached beneath them.
	 */
	tcm->tcm_parent = TC_H_ROOT;
	tcm->tcm_handle = cl->cl_common.classid;
	tcm->tcm_info = cl->qdisc->handle;

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (opts == NULL || nla_put(skb, TCA_NSSBF_CLASS_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * nss_bf_dump_class_stats()
 *	Dumps class statistics.
 */
static int nss_bf_dump_class_stats(struct Qdisc *sch, unsigned long arg, struct gnet_dump *d)
{
	struct nss_qdisc *nq = (struct nss_qdisc *)arg;

	if (nss_qdisc_gnet_stats_copy_basic(sch, d, &nq->bstats) < 0 ||
			nss_qdisc_gnet_stats_copy_queue(d, &nq->qstats) < 0) {
		return -1;
	}

	return 0;
}

/*
 * nss_bf_walk()
 *	Used to walk the tree.
 */
static void nss_bf_walk(struct Qdisc *sch, struct qdisc_walker *arg)
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct nss_bf_class_data *cl;
	unsigned int i;

	nss_qdisc_info("In bf walk %px\n", sch);
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

/*
 * nss_bf_change_qdisc()
 *	Can be used to configure a nssbf qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_bf_change_qdisc(struct Qdisc *sch, struct nlattr *opt)
#else
static int nss_bf_change_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
#endif
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSBF_MAX + 1];
	struct tc_nssbf_qopt *qopt;

	/*
	 * Since nssbf can be created with no arguments, opt might be NULL
	 * (depending on the kernel version). This is still a valid create
	 * request.
	 */
	if (opt == NULL) {

		/*
		 * If no parameter is passed, set it to 0 and continue
		 * creating the qdisc.
		 */
		sch_tree_lock(sch);
		q->defcls = 0;
		sch_tree_unlock(sch);
		return 0;
	}

	/*
	 * If it is not NULL, parse to get qopt.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	qopt = nss_qdisc_qopt_get(opt, nss_bf_policy, tb, TCA_NSSBF_MAX, TCA_NSSBF_QDISC_PARMS);
#else
	qopt = nss_qdisc_qopt_get(opt, nss_bf_policy, tb, TCA_NSSBF_MAX, TCA_NSSBF_QDISC_PARMS, extack);
#endif
	if (!qopt) {
		return -EINVAL;
	}

	sch_tree_lock(sch);
	q->defcls = qopt->defcls;
	sch_tree_unlock(sch);

	/*
	 * This information is unused in the NSS. So we do not send
	 * a configuration message down.
	 */

	return 0;
}

/*
 * nss_bf_reset_class()
 *	Resets child qdisc of class to be reset.
 */
static void nss_bf_reset_class(struct nss_bf_class_data *cl)
{
	if (cl->qdisc == &noop_qdisc) {
		nss_qdisc_trace("Class %x has no child qdisc to reset\n", cl->nq.qos_tag);
		return;
	}

	nss_qdisc_reset(cl->qdisc);
	nss_qdisc_info("Nssbf class resetted %px\n", cl->qdisc);
}

/*
 * nss_bf_reset_qdisc()
 *	Resets nssbf qdisc and its classes.
 */
static void nss_bf_reset_qdisc(struct Qdisc *sch)
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nss_bf_class_data *cl;
	struct hlist_node *n __maybe_unused;
	unsigned int i;

	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry(cl, n, &q->clhash.hash[i], cl_common.hnode)
			nss_bf_reset_class(cl);
	}

	nss_qdisc_reset(sch);
	nss_qdisc_info("Nssbf qdisc resetted %px\n", sch);
}

/*
 * nss_bf_destroy_qdisc()
 *	Call to destroy a nssbf qdisc and its associated classes.
 */
static void nss_bf_destroy_qdisc(struct Qdisc *sch)
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct hlist_node *n __maybe_unused;
	struct hlist_node *next;
	struct nss_bf_class_data *cl;
	struct nss_if_msg nim;
	unsigned int i;

	/*
	 * Destroy all the classes before the root qdisc is destroyed.
	 */
	for (i = 0; i < q->clhash.hashsize; i++) {
		nss_qdisc_hlist_for_each_entry_safe(cl, n, next, &q->clhash.hash[i], cl_common.hnode) {

			/*
			 * If this is the root class, we dont have to destroy it. This will be taken
			 * care of by the nss_bf_destroy() function.
			 */
			if (cl == &q->root) {
				nss_qdisc_info("We do not detach or destroy bf class %px here since this is "
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
			nim.msg.shaper_configure.config.msg.shaper_node_config.snc.bf_detach.child_qos_tag = cl->nq.qos_tag;
			if (nss_qdisc_node_detach(&q->nq, &cl->nq, &nim,
					NSS_SHAPER_CONFIG_TYPE_SHAPER_NODE_DETACH) < 0) {
				nss_qdisc_error("Node detach failed for qdisc %x class %x\n",
							cl->nq.qos_tag, q->nq.qos_tag);
				return;
			}

			/*
			 * Now we can destroy the class.
			 */
			nss_bf_destroy_class(sch, cl);
		}
	}
	qdisc_class_hash_destroy(&q->clhash);

	/*
	 * Now we can go ahead and destroy the qdisc.
	 * Note: We dont have to detach ourself from our parent because this
	 *	 will be taken care of by the graft call.
	 */
	nss_qdisc_destroy(&q->nq);
	nss_qdisc_info("Nssbf destroyed %px\n", sch);
}

/*
 * nss_bf_init_qdisc()
 *	Initializes the nssbf qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
static int nss_bf_init_qdisc(struct Qdisc *sch, struct nlattr *opt)
{
	struct netlink_ext_ack *extack = NULL;
#else
static int nss_bf_init_qdisc(struct Qdisc *sch, struct nlattr *opt,
				struct netlink_ext_ack *extack)
{
#endif
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct nlattr *tb[TCA_NSSBF_MAX + 1];
	struct tc_nssbf_qopt *qopt;
	int err;
	unsigned int accel_mode;

	nss_qdisc_info("Init bf qdisc %px\n", sch);

	err = qdisc_class_hash_init(&q->clhash);
	if (err < 0) {
		return err;
	}

	q->root.cl_common.classid = sch->handle;
	q->root.qdisc = &noop_qdisc;

	qdisc_class_hash_insert(&q->clhash, &q->root.cl_common);
	qdisc_class_hash_grow(sch, &q->clhash);

	/*
	 * opt is NULL when no parameter is passed by user in TC.
	 */
	if (!opt) {
		accel_mode = TCA_NSS_ACCEL_MODE_NSS_FW;
	} else {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
		qopt = nss_qdisc_qopt_get(opt, nss_bf_policy, tb, TCA_NSSBF_MAX, TCA_NSSBF_QDISC_PARMS);
#else
		qopt = nss_qdisc_qopt_get(opt, nss_bf_policy, tb, TCA_NSSBF_MAX, TCA_NSSBF_QDISC_PARMS, extack);
#endif
		if (!qopt) {
			return -EINVAL;
		}
		accel_mode = qopt->accel_mode;
	}

	/*
	 * Initialize the NSSBF shaper in NSS
	 */
	if (nss_qdisc_init(sch, &q->nq, NSS_SHAPER_NODE_TYPE_BF, 0, accel_mode, extack) < 0) {
		return -EINVAL;
	}

	nss_qdisc_info("Nssbf initialized - handle %x parent %x\n", sch->handle, sch->parent);

	/*
	 * Tune nss_bf parameters.
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 16, 0))
	if (nss_bf_change_qdisc(sch, opt) < 0) {
#else
	if (nss_bf_change_qdisc(sch, opt, extack) < 0) {
#endif
		nss_qdisc_destroy(&q->nq);
		return -EINVAL;
	}

	return 0;
}

/*
 * nss_bf_dump_qdisc()
 *	Dumps nssbf qdisc's configurable parameters.
 */
static int nss_bf_dump_qdisc(struct Qdisc *sch, struct sk_buff *skb)
{
	struct nss_bf_sched_data *q = qdisc_priv(sch);
	struct tc_nssbf_qopt qopt;
	struct nlattr *opts = NULL;

	nss_qdisc_info("In bf dump qdisc\n");
	qopt.defcls = q->defcls;
	qopt.accel_mode = nss_qdisc_accel_mode_get(&q->nq);

	opts = nss_qdisc_nla_nest_start(skb, TCA_OPTIONS);
	if (!opts || nla_put(skb, TCA_NSSBF_QDISC_PARMS, sizeof(qopt), &qopt)) {
		goto nla_put_failure;
	}

	return nla_nest_end(skb, opts);

nla_put_failure:
	nla_nest_cancel(skb, opts);
	return -EMSGSIZE;
}

/*
 * nss_bf_enqueue()
 *	Enqueues a skb to nssbf qdisc.
 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
static int nss_bf_enqueue(struct sk_buff *skb, struct Qdisc *sch)
#else
static int nss_bf_enqueue(struct sk_buff *skb, struct Qdisc *sch,
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
 * nss_bf_dequeue()
 *	Dequeues a skb to nssbf qdisc.
 */
static struct sk_buff *nss_bf_dequeue(struct Qdisc *sch)
{
	return nss_qdisc_dequeue(sch);
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
/*
 * nss_bf_drop()
 *	Drops a single skb from linux queue, if not empty.
 *
 * Does not drop packets that are queued in the NSS.
 */
static unsigned int nss_bf_drop(struct Qdisc *sch)
{
	printk("In bf drop\n");
	return nss_qdisc_drop(sch);
}
#endif

/*
 * Registration structure for nssbf class
 */
const struct Qdisc_class_ops nss_bf_class_ops = {
	.change		= nss_bf_change_class,
	.delete		= nss_bf_delete_class,
	.graft		= nss_bf_graft_class,
	.leaf		= nss_bf_leaf_class,
	.qlen_notify	= nss_bf_qlen_notify,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 14, 0))
	.get		= nss_bf_get_class,
	.put		= nss_bf_put_class,
#else
	.find		= nss_bf_search_class,
#endif
#if (LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 0))
	.tcf_chain	= nss_qdisc_tcf_chain,
#else
	.tcf_block      = nss_qdisc_tcf_block,
#endif
	.bind_tcf	= nss_qdisc_tcf_bind,
	.unbind_tcf	= nss_qdisc_tcf_unbind,
	.dump		= nss_bf_dump_class,
	.dump_stats	= nss_bf_dump_class_stats,
	.walk		= nss_bf_walk
};

/*
 * Registration structure for nssbf qdisc
 */
struct Qdisc_ops nss_bf_qdisc_ops __read_mostly = {
	.id		= "nssbf",
	.init		= nss_bf_init_qdisc,
	.change		= nss_bf_change_qdisc,
	.reset		= nss_bf_reset_qdisc,
	.destroy	= nss_bf_destroy_qdisc,
	.dump		= nss_bf_dump_qdisc,
	.enqueue	= nss_bf_enqueue,
	.dequeue	= nss_bf_dequeue,
	.peek		= qdisc_peek_dequeued,
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 8, 0))
	.drop		= nss_bf_drop,
#endif
	.cl_ops		= &nss_bf_class_ops,
	.priv_size	= sizeof(struct nss_bf_sched_data),
	.owner		= THIS_MODULE
};
