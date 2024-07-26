/*
 * Copyright (c) 2014-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "ppe_htb.h"
#include "ppe_fifo.h"
#include "ppe_prio.h"
#include "ppe_red.h"
#include "ppe_tbl.h"
#include "ppe_wrr.h"
#include "ppe_wfq.h"
#include "ppe_qdisc_stats.h"

/*
 * Max number of PRIO bands supported based on level.
 */
#define PPE_QDISC_PORT_LEVEL_PRIO_BANDS_MAX	2
#define PPE_QDISC_FLOW_LEVEL_PRIO_BANDS_MAX	4

/*
 * Error codes
 */
#define PPE_QDISC_PARENT_NOT_EXISTING -1

/*
 * ppe_qdisc_def_node_int_pri_get()
 *      Returns the INT-PRI value for the default qdisc.
 */
static uint8_t ppe_qdisc_def_node_int_pri_get(struct net_device *dev)
{
	struct ppe_qdisc *pq_root, *pq_def = NULL;
	pq_root = qdisc_priv(dev->qdisc);

	/*
	 * Return a valid int_pri in case of failure, say 0 as this will
	 * eventually be set in the Tx Desc. This may either cause the traffic
	 * to go through wrong queue or get disrupted if the queue 0 is not
	 * enabled/connected to the hierarchy.
	 *
	 * TODO: Consider an approach that this function never fails
	 * and we return an int_pri in such a way that traffic on the
	 * port is not disrupted.
	 */
	if (!pq_root) {
		ppe_qdisc_warning("%px root node not found for device", dev);
		return 0;
	}

	pq_def = pq_root->def;
	if ((!pq_def) || (!ppe_qdisc_flags_check(pq_def, PPE_QDISC_FLAG_INT_PRI_VALID))) {
		ppe_qdisc_warning("%px no default node or valid int-pri for device", dev);
		return 0;
	}

	ppe_qdisc_info("%px returning default node pq_def %px and pq_def->int_pri %u", dev, pq_def, pq_def->int_pri);
	return (uint8_t)pq_def->int_pri;
}

/*
 * ppe_qdisc_set_parent()
 *	Sets the parent of given qdisc.
 */
static int ppe_qdisc_set_parent(struct ppe_qdisc *pq, uint32_t parent)
{
	struct net_device *dev = qdisc_dev(pq->qdisc);
	struct ppe_qdisc *parent_pq = NULL;
	struct Qdisc *parent_qdisc = NULL;
	unsigned long parent_class = 0;

	/*
	 * If Qdisc is root, bail.
	 */
	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_ROOT)) {
		ppe_qdisc_trace("PPE Qdisc %px is a root, no parent existing", pq->qdisc);
		return 0;
	}

	if (parent != TC_H_ROOT) {
		parent_qdisc = qdisc_lookup(dev, TC_H_MAJ(parent));
		if (parent_qdisc) {
			parent_pq = qdisc_priv(parent_qdisc);
		}
	} else if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CLASS)) {
		parent_pq = qdisc_priv(pq->qdisc);
	}

	/*
	 * Parent does not exist or pattern is of type Queue (FIFO/RED)
	 */
	if ((!parent_pq) || (parent_pq->type > PPE_QDISC_NODE_SCH_MAX)) {
		ppe_qdisc_warning("PPE qdisc/class %px cannot be attached to non-existing class %x", pq->qdisc, parent);
		return PPE_QDISC_PARENT_NOT_EXISTING;
	}

	/*
	 * Set the parent if current Qdisc is attached to a class.
	 */
	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CLASS)) {
		pq->parent = parent_pq;

		/*
		 * If parent is a class.
		 *
		 * Though PRIO is classless but PRIO bands are treated as classes by Qdisc
		 * infrastructure i.e TC_H_MIN(parent) is true for PRIO band.
		 * But, we donot allocate any resources for PRIO bands separately so in
		 * case of any qdisc attached to PRIO band, the parent is set as PRIO qdisc itself.
		 * And the below class check is applicable only for the classful qdiscs.
		 */
		if ((parent_pq->type != PPE_QDISC_NODE_TYPE_PRIO) && (TC_H_MIN(parent))) {
			if (!parent_qdisc) {
				ppe_qdisc_warning("PPE qdisc/class %px cannot be attached to non-existing class %x", pq->qdisc, parent);
				return PPE_QDISC_PARENT_NOT_EXISTING;
			}

			parent_class = parent_qdisc->ops->cl_ops->find(parent_qdisc, parent);
			if (!parent_class) {
				pq->parent = NULL;
				ppe_qdisc_warning("PPE qdisc/class %px cannot be attached to non-existing class %x", pq->qdisc, parent);
				return PPE_QDISC_PARENT_NOT_EXISTING;

			}

			pq->parent = (struct ppe_qdisc *)parent_class;
		}
	}

	return 0;
}

/*
 * ppe_qdisc_drr_weight_get()
 *	Returns the DRR weight corresponding to quantum.
 */
int ppe_qdisc_drr_weight_get(uint32_t quantum, ppe_drv_qos_drr_unit_t drr_unit)
{
	switch (drr_unit) {
	case PPE_DRV_QOS_DRR_UNIT_BYTE:
		if (quantum < PPE_DRV_QOS_DRR_WEIGHT_MAX) {
			return 1;
		} else {
			return quantum / PPE_DRV_QOS_DRR_WEIGHT_MAX;
		}

	case PPE_DRV_QOS_DRR_UNIT_PACKET:
		return quantum;

	default:
		ppe_qdisc_warning("DRR unit %d not supported", drr_unit);
	}

	return 0;
}

/*
 * ppe_qdisc_get_max_prio_bands()
 *	Returns the number of PRIO bands supported based on qdisc level.
 */
int ppe_qdisc_get_max_prio_bands(struct ppe_qdisc *pq)
{
	switch (pq->level) {
	case PPE_DRV_QOS_PORT_LEVEL:
		return PPE_QDISC_PORT_LEVEL_PRIO_BANDS_MAX;

	case PPE_DRV_QOS_FLOW_LEVEL:
		return PPE_QDISC_FLOW_LEVEL_PRIO_BANDS_MAX;

	default:
		ppe_qdisc_warning("Qdisc:%px not supported at this level", pq->qdisc);
		return 0;
	}
}

/*
 * ppe_qdisc_type_is_queue()
 *	Returns true if qdisc is of type queue.
 */
bool ppe_qdisc_type_is_queue(struct ppe_qdisc *pq)
{
	if (pq->type < PPE_QDISC_NODE_SCH_MAX) {
		return false;
	}

	return true;
}

/*
 * ppe_qdisc_is_depth_valid()
 *	Checks the depth of Qdisc tree.
 */
int ppe_qdisc_is_depth_valid(struct ppe_qdisc *pq)
{
	ppe_drv_qos_level_t valid_level;
	ppe_qdisc_info("Qdisc:%px level:%d", pq->qdisc, pq->level);

	switch (pq->type) {
	case PPE_QDISC_NODE_TYPE_FIFO:
	case PPE_QDISC_NODE_TYPE_RED:
		valid_level = PPE_DRV_QOS_SUB_QUEUE_LEVEL;
		break;

	case PPE_QDISC_NODE_TYPE_HTB:
	case PPE_QDISC_NODE_TYPE_WRR:
	case PPE_QDISC_NODE_TYPE_PRIO:
		valid_level = PPE_DRV_QOS_FLOW_LEVEL;
		break;

	case PPE_QDISC_NODE_TYPE_HTB_GROUP:
	case PPE_QDISC_NODE_TYPE_TBL:
	case PPE_QDISC_NODE_TYPE_WRR_GROUP:
		valid_level = PPE_DRV_QOS_QUEUE_LEVEL;
		break;

	default:
		ppe_qdisc_warning("Qdisc:%px not supported", pq->qdisc);
		return false;
	}

	if (pq->level < valid_level) {
		ppe_qdisc_warning("Qdisc:%px type:%d not supported at level:%d", pq->qdisc, pq->type, pq->level);
		return false;
	}

	return true;
}

/*
 * ppe_qdisc_qopt_get()
 *	Extracts qopt from opt.
 */
void *ppe_qdisc_qopt_get(struct nlattr *opt, struct nla_policy *policy,
		struct nlattr *tb[], uint32_t tca_max,
		uint32_t tca_params, struct netlink_ext_ack *extack)
{
	int err;

	if (!opt) {
		return NULL;
	}

	err = nla_parse_nested_deprecated(tb, tca_max, opt, policy, extack);
	if (err < 0) {
		return NULL;
	}

	if (tb[tca_params] == NULL) {
		return NULL;
	}
	return nla_data(tb[tca_params]);
}

/*
 * ppe_qdisc_set_default()
 *	Set the default node for the Qdisc.
 */
void ppe_qdisc_set_default(struct ppe_qdisc *pq)
{
	struct ppe_qdisc *pq_root = pq;
	struct net_device *dev = qdisc_dev(pq->qdisc);

	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_NODE_DEFAULT);

	/*
	 * Get the root qdisc and set default node
	 */
	if (!ppe_qdisc_flags_check(pq_root, PPE_QDISC_FLAG_NODE_ROOT)) {
		if (dev) {
			pq_root =  qdisc_priv(dev->qdisc);
		}
	}

	pq_root->def = pq;
}

/*
 * ppe_qdisc_node_detach()
 *	Configuration function that helps detach a child shaper node from a parent.
 */
void ppe_qdisc_node_detach(struct ppe_qdisc *pq, struct ppe_qdisc *pq_child)
{
	/*
	 * pq child was set only if pq is leaf qdisc/class,
	 * set its child as NULL
	 */
	spin_lock_bh(&pq->lock);
	if (pq->child) {
		pq->child = NULL;
	}
	spin_unlock_bh(&pq->lock);

	ppe_qdisc_destroy(pq_child);
	ppe_qdisc_info("Qdisc:%px, node:%px detach complete", pq, pq_child);
}

/*
 * ppe_qdisc_node_attach()
 *	Configuration function that helps attach a child shaper node to a parent.
 */
int ppe_qdisc_node_attach(struct ppe_qdisc *pq, struct ppe_qdisc *pq_child)
{
	/*
	 * In case of child attached to PRIO qdisc, scheduler configuration is
	 * required to set the priority of child qdisc in SSDK.
	 */
	if (pq->type == PPE_QDISC_NODE_TYPE_PRIO) {
		if (ppe_qdisc_res_scheduler_set(pq_child) != 0)  {
			ppe_qdisc_warning("Qdisc:%px scheduler configuration failed", pq->qdisc);
			return -EINVAL;
		}
	}

	/*
	 * Set the child qdisc for the leaf class
	 */
	spin_lock_bh(&pq->lock);
	if (pq_child->type > PPE_QDISC_NODE_SCH_MAX) {
		pq->child = pq_child;
	}
	spin_unlock_bh(&pq->lock);

	ppe_qdisc_info("Qdisc:%px, node:%px attach complete", pq, pq_child);
	return 0;
}

/*
 * ppe_qdisc_configure()
 *	Configures the SSDK schedulers and shapers.
 */
int ppe_qdisc_configure(struct ppe_qdisc *pq, struct ppe_qdisc *prev_pq)
{
	struct ppe_drv_qos_res *rp = &pq->res;
	struct ppe_drv_qos_res *prev_rp = NULL;

	ppe_qdisc_info("Qdisc %px (type %d) configuring", pq->qdisc, pq->type);

	/*
	 * Change the configuration in SSDK
	 */
	if (ppe_qdisc_res_scheduler_set(pq) != 0)  {
		ppe_qdisc_warning("Qdisc:%px scheduler configuration failed", pq->qdisc);
		goto fail;
	}

	if (ppe_qdisc_res_shaper_set(pq) != 0)  {
		ppe_qdisc_warning("Qdisc:%px shaper configuration failed", pq->qdisc);
		goto fail;
	}

	/*
	 * Queue configuration is not needed if Qdisc is not of queue type
	 */
	if (!ppe_qdisc_type_is_queue(pq)) {
		ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_NODE_CONFIGURED);
		ppe_qdisc_info("Qdisc:%px configured successfully", pq->qdisc);
		return 0;
	}

	if (ppe_qdisc_res_mcast_queue_set(pq) < 0) {
		ppe_qdisc_warning("Qdisc:%px multicast queueue configuration failed", pq->qdisc);
		goto fail;
	}

	/*
	 * Program PPE queue parameters
	 */
	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID)) {
		if (ppe_drv_qos_queue_limit_set(&pq->res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px queue configuration failed", pq->qdisc);
			goto fail;
		}
	}

	/*
	 * There is nothing we need to do if the qdisc is not
	 * set as default qdisc.
	 * TODO: Handle set_default functionality
	 */
	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_NODE_CONFIGURED);

	ppe_qdisc_info("Qdisc:%px configuration complete", pq->qdisc);
	return 0;

fail:
	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CONFIGURED)) {
		return -EINVAL;
	}

	if (!prev_pq) {
		return -EINVAL;
	}
	prev_rp = &prev_pq->res;

	/*
	 * Restore to previous configuration if exists.
	 * In case default node and mcast queue valid bits are
	 * toggled and we have allocated/deallocated the
	 * mcast resources during change configuration, we need to restore them
	 * to old configuration by again deallocating/allocating them.
	 */
	if ((PPE_QDISC_FLAG_NODE_DEFAULT & (pq->flags ^ prev_pq->flags))
			&& (PPE_QDISC_FLAG_MCAST_QUEUE_VALID & (pq->flags ^ prev_pq->flags))) {
		ppe_qdisc_flags_set(prev_pq, pq->flags & PPE_QDISC_FLAG_MCAST_QUEUE_VALID);
		prev_rp->q.mcast_qid = rp->q.mcast_qid;
	}

	memcpy(rp, prev_rp, sizeof(struct ppe_drv_qos_res));
	ppe_drv_qos_queue_limit_set(&pq->res);
	ppe_qdisc_res_mcast_queue_set(pq);

	ppe_qdisc_res_shaper_set(pq);
	ppe_qdisc_res_scheduler_set(pq);

	return -EINVAL;
}

/*
 * ppe_qdisc_destroy()
 *	Destroys PPE Qdisc.
 */
void ppe_qdisc_destroy(struct ppe_qdisc *pq)
{
	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_INITIALIZED)) {
		if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_ROOT)) {
			ppe_qdisc_stats_sync_many_exit(pq);
		}
		return;
	}

	/*
	 * Destroy any attached filter over qdisc.
	 */
	if (pq->block) {
		tcf_block_put(pq->block);
	}

	if (pq->type > PPE_QDISC_NODE_SCH_MAX) {
		ppe_qdisc_stats_qdisc_detach(pq);
	}

	ppe_qdisc_res_free(pq);

	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_ROOT)) {
		ppe_qdisc_port_default_conf_set(pq->port_id);
		ppe_qdisc_stats_stop_polling(pq);
	}

	pq->flags = 0;
	ppe_qdisc_info("Qdisc %px (type %d): destroy complete",
			pq->qdisc, pq->type);
}

/*
 * ppe_qdisc_init()
 *	Allocates PPE resources and initializes a schedulers/shaper in PPE.
 *
 * The resource allocation is based on the position (child or root) and level of this Qdisc/class.
 * Currently supported for physical interfaces only.
 */
int ppe_qdisc_init(struct Qdisc *sch, struct ppe_qdisc *pq, ppe_qdisc_node_type_t type, uint32_t classid,
		struct netlink_ext_ack *extack)
{
	int err;
	struct Qdisc *root;
	u32 parent;
	struct net_device *dev;

	/*
	 * Initialize locks
	 */
	spin_lock_init(&pq->lock);

	/*
	 * Add PPE flag to the qdisc
	 * TODO: Should we change it in Linux?
	 */
	sch->flags |= TCQ_F_NSS;

	/*
	 * Record our qdisc and type in the private region for handy use
	 */
	pq->qdisc = sch;
	pq->type = type;
	pq->flags = 0;

	/*
	 * If we are a class, then classid is used as the qos tag.
	 * Else the qdisc handle will be used as the qos tag.
	 */
	if (classid) {
		pq->qos_tag = classid;
		ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_NODE_CLASS);
	} else {
		pq->qos_tag = (uint32_t)sch->handle;
	}

	parent = sch->parent;

	/*
	 * If our parent is TC_H_ROOT and we are not a class, then we are the root qdisc.
	 * Note, classes might have its qdisc as root, however we should not set root node to
	 * true for classes. This is the reason why we check for classid.
	 */
	if ((sch->parent == TC_H_ROOT) && (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CLASS))) {
		ppe_qdisc_info("Qdisc %px dev-name %s qdisc_dev(sch)->qdisc %px, qdisc_dev(sch)->qdisc->handle %x, (type %d) is root",
				qdisc_dev(sch), qdisc_dev(sch)->name, qdisc_dev(sch)->qdisc, qdisc_dev(sch)->qdisc->handle, pq->type);
		ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_NODE_ROOT);
		root = sch;

		/*
		 * Memory alloacated here is released in ppe_qdisc_destroy functions
		 */
		ppe_qdisc_stats_sync_many_init(pq);
	} else {
		ppe_qdisc_info("Qdisc %px (type %d) not root", pq->qdisc, pq->type);
		root = qdisc_dev(sch)->qdisc;
	}

	/*
	 * Get the net device as it will tell us if we are on a bridge,
	 * or on a net device that is represented by a virtual NSS interface (e.g. WIFI)
	 * Currently PPE Qdisc is supported on physical interfaces only
	 */
	dev = qdisc_dev(sch);
	if (dev->priv_flags & IFF_EBRIDGE) {
		ppe_qdisc_warning("PPE Qdisc not supported on bridge interfaces %px", pq->qdisc);
		return -1;
	}

	ppe_qdisc_info("Qdisc %px (type %d) init root: %px, qos tag: %x, "
			"parent: %x rootid: %s owner: %px", pq->qdisc, pq->type, root,
			pq->qos_tag, parent, root->ops->id, root->ops->owner);

	/*
	 * The root must be of PPE type.
	 * This is to prevent mixing PPE qdisc with linux qdisc.
	 */
	if ((parent != TC_H_ROOT) && (root->ops->owner != THIS_MODULE)) {
		ppe_qdisc_warning("PPE qdisc %px (type %d) used along with non-ppe qdiscs", pq->qdisc, pq->type);
		return -1;
	}

	RCU_INIT_POINTER(pq->filter_list, NULL);
	pq->block = NULL;
	err = tcf_block_get(&pq->block, &pq->filter_list, sch, extack);
	if (err) {
		ppe_qdisc_warning("%px: Unable to initialize tcf_block\n", &pq->block);
		return -1;
	}

	/*
	 * Set the parent of PPE qdisc.
	 */
	if (!parent || (ppe_qdisc_set_parent(pq, parent) < 0)) {
		ppe_qdisc_warning("PPE qdisc/class %x cannot be attached to non-existing parent %x", pq->qos_tag, parent);
		return -1;
	}
	/*
	 * The device we are operational on MUST be recognized as an PPE interface.
	 * Currently the support is provided only for Physical interfaces.
	 * TODO: Evaluate and support Qdisc on PPE VP and Bridge interfaces.
	 */
	pq->port_id = ppe_qdisc_port_id_get(dev);
	if ((pq->port_id < PPE_DRV_PHYSICAL_START)  || (pq->port_id >= PPE_DRV_PHYSICAL_MAX)) {
		ppe_qdisc_warning("Qdisc %px (type %d) net device is not a "
			"physical interface %s", pq->qdisc, pq->type, dev->name);
		return -1;
	}

	if (ppe_qdisc_res_init(pq) < 0) {
		ppe_qdisc_warning("Qdisc %px (type %d) init failed", pq->qdisc, pq->type);
		memset(&pq->res, 0, sizeof(struct ppe_drv_qos_res));
		return -1;
	}

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CLASS)) {
		ppe_qdisc_stats_qdisc_attach(pq);

		if (sch->parent == TC_H_ROOT) {
			ppe_qdisc_stats_start_polling(pq);
			ppe_drv_qos_port_bm_control_enable(pq->port_id, false);
		}
	}

	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_NODE_INITIALIZED);
	ppe_qdisc_info("%px Qdisc (type %d): initialized", pq->qdisc, pq->type);
	return 0;
}

/*
 * ppe_qdisc_int_pri_get()
 *      Returns the INT-PRI value for a given classid.
 *
 * Note: Caller should check if Qdisc is PPE Qdisc before invoking it.
 */
uint8_t ppe_qdisc_int_pri_get(struct net_device *dev, uint32_t classid)
{
	const struct Qdisc_class_ops *clops = NULL;
	struct Qdisc *q = NULL;
	struct ppe_qdisc *pq, *pq_child = NULL;
	uint8_t int_pri = 0;

	if (!classid) {
		ppe_qdisc_info("%px:class Id is zero, returning default int_pri for device", dev);
		return ppe_qdisc_def_node_int_pri_get(dev);
	}

	q = qdisc_lookup(dev, TC_H_MAJ(classid));
	if (!q) {
		ppe_qdisc_info("%px:qdisc not found for class:%u, returning default int_pri", dev, classid);
		return ppe_qdisc_def_node_int_pri_get(dev);
	}

	/*
	 * If it is ppefifo or ppered qdisc, these are root qdisc, return their int_pri
	 */
	pq = qdisc_priv(q);
	if (pq->type > PPE_QDISC_NODE_SCH_MAX) {
		if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_INT_PRI_VALID)) {
			ppe_qdisc_info("%px:qdisc %px is root node with int_pri %u", dev, pq, pq->int_pri);
			return pq->int_pri;
		}

		return ppe_qdisc_def_node_int_pri_get(dev);
	}

	/*
	 * If qdisc is prio, we need to get the child qdisc attached to its band
	 */
	if (pq->type == PPE_QDISC_NODE_TYPE_PRIO) {
		pq_child = ppe_prio_band_qdisc_get(q, classid);
		if ((pq_child) && (ppe_qdisc_flags_check(pq_child, PPE_QDISC_FLAG_INT_PRI_VALID))) {
			ppe_qdisc_info("%px:returning leaf node int_pri %u", dev, pq_child->int_pri);
			return pq_child->int_pri;
		}

		return ppe_qdisc_def_node_int_pri_get(dev);
	}

	clops = q->ops->cl_ops;
	if (!clops) {
		ppe_qdisc_warning("%px:classid %u for unsupported qdisc %px, returning default int_pri", dev, classid, q);
		return ppe_qdisc_def_node_int_pri_get(dev);
	}

	pq = (struct ppe_qdisc *)clops->find(q, classid);
	if (!pq) {
		ppe_qdisc_warning("%px: class %u not found for qdisc %px", dev, classid, q);
		return ppe_qdisc_def_node_int_pri_get(dev);
	}

	spin_lock_bh(&pq->lock);
	if ((pq->child) && (ppe_qdisc_flags_check(pq->child, PPE_QDISC_FLAG_INT_PRI_VALID))) {
		int_pri = pq->child->int_pri;
		spin_unlock_bh(&pq->lock);
		ppe_qdisc_info("%px:returning leaf node int_pri %u", dev, int_pri);
		return (uint8_t)int_pri;
	}
	spin_unlock_bh(&pq->lock);

	ppe_qdisc_info("%px:no child qdisc atytached to class:%u, returning default int_pri", dev, classid);
	return ppe_qdisc_def_node_int_pri_get(dev);
}

/*
 * ppe_qdisc_module_init()
 *	Loads and initializes PPE qdisc module.
 */
static int __init ppe_qdisc_module_init(void)
{
	int ret;

	ppe_qdisc_port_alloc();

	ret = register_qdisc(&ppe_pfifo_qdisc_ops);
	if (ret != 0)
		goto fail1;
	ppe_qdisc_info("ppepfifo registered");

	ret = register_qdisc(&ppe_bfifo_qdisc_ops);
	if (ret != 0)
		goto fail2;
	ppe_qdisc_info("ppebfifo registered");

	ret = register_qdisc(&ppe_htb_qdisc_ops);
	if (ret != 0)
		goto fail3;
	ppe_qdisc_info("ppehtb registered");

	ret = register_qdisc(&ppe_prio_qdisc_ops);
	if (ret != 0)
		goto fail4;
	ppe_qdisc_info("ppeprio registered");

	ret = register_qdisc(&ppe_red_qdisc_ops);
	if (ret != 0)
		goto fail5;
	ppe_qdisc_info("ppered registered");

	ret = register_qdisc(&ppe_tbl_qdisc_ops);
	if (ret != 0)
		goto fail6;
	ppe_qdisc_info("ppetbl registered");

	ret = register_qdisc(&ppe_wrr_qdisc_ops);
	if (ret != 0)
		goto fail7;
	ppe_qdisc_info("ppewrr registered");

	ret = register_qdisc(&ppe_wfq_qdisc_ops);
	if (ret != 0)
		goto fail8;
	ppe_qdisc_info("ppewfq registered");

	if (!ppe_qdisc_stats_work_queue_init()) {
		ppe_qdisc_warning("Failed to initialized stats workqueue thread");
		goto fail9;
	}

	ppe_drv_qos_int_pri_callback_register(ppe_qdisc_int_pri_get);

	ppe_qdisc_info("ppe qdisc module initialized");
	return 0;

fail9:
	unregister_qdisc(&ppe_wfq_qdisc_ops);
fail8:
	unregister_qdisc(&ppe_wrr_qdisc_ops);
fail7:
	unregister_qdisc(&ppe_tbl_qdisc_ops);
fail6:
	unregister_qdisc(&ppe_red_qdisc_ops);
fail5:
	unregister_qdisc(&ppe_prio_qdisc_ops);
fail4:
	unregister_qdisc(&ppe_htb_qdisc_ops);
fail3:
	unregister_qdisc(&ppe_bfifo_qdisc_ops);
fail2:
	unregister_qdisc(&ppe_pfifo_qdisc_ops);
fail1:
	ppe_qdisc_port_free();
	return ret;
}

/*
 * ppe_qdisc_module_exit()
 *	Unloads PPE Qdisc module.
 */
static void __exit ppe_qdisc_module_exit(void)
{
	ppe_drv_qos_int_pri_callback_unregister();
	ppe_qdisc_stats_work_queue_exit();

	unregister_qdisc(&ppe_pfifo_qdisc_ops);
	ppe_qdisc_info("ppepfifo unregistered");

	unregister_qdisc(&ppe_bfifo_qdisc_ops);
	ppe_qdisc_info("ppebfifo unregistered");

	unregister_qdisc(&ppe_htb_qdisc_ops);
	ppe_qdisc_info("ppehtb unregistered");

	unregister_qdisc(&ppe_prio_qdisc_ops);
	ppe_qdisc_info("ppeprio unregistered");

	unregister_qdisc(&ppe_red_qdisc_ops);
	ppe_qdisc_info("ppered unregistered");

	unregister_qdisc(&ppe_tbl_qdisc_ops);
	ppe_qdisc_info("ppetbl unregistered");

	unregister_qdisc(&ppe_wrr_qdisc_ops);
	ppe_qdisc_info("ppewrr unregistered");

	unregister_qdisc(&ppe_wfq_qdisc_ops);
	ppe_qdisc_info("ppewfq unregistered");

	ppe_qdisc_port_free();
	ppe_qdisc_info("ppe qdisc module exited");
}

module_init(ppe_qdisc_module_init)
module_exit(ppe_qdisc_module_exit)

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("PPE Qdisc client");
