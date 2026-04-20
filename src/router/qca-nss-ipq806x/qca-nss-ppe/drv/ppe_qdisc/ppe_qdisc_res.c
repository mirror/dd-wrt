/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
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
 * ppe_qdisc_res_l1_free()
 *	Frees Level 1 scheduler resources.
 */
static int ppe_qdisc_res_l1_free(struct ppe_qdisc *pq)
{
	uint32_t offset;

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_L1_SCHEDULER_VALID)) {
		return 0;
	}

	/*
	 * Free Level 1 DRR resource
	 */
	offset = pq->res.scheduler.l1c_drrid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_CDRR);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_L1_CDRR) != 0) {
		ppe_qdisc_warning("Qdisc:%px used res:%d not found for port:%d, type:%d ",
				pq->qdisc, pq->res.scheduler.l1c_drrid, pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_CDRR);
		return -EINVAL;
	}

	offset = pq->res.scheduler.l1e_drrid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_EDRR);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_L1_EDRR) != 0) {
		ppe_qdisc_warning("Qdisc:%px used res:%d not found for port:%d, type:%d ",
				pq->qdisc, pq->res.scheduler.l1e_drrid, pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_EDRR);
		return -EINVAL;
	}

	/*
	 * Free Level 0 SP resource
	 */
	offset = pq->res.l0spid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_SP);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_L0_SP) != 0) {
		ppe_qdisc_warning("Qdisc:%px used res:%d not found for port:%d, type:%d ",
				pq->qdisc, pq->res.l0spid, pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_SP);
		return -EINVAL;
	}

	/*
	 * Reset Res id values in qdisc
	 */
	pq->res.l0spid = 0;
	pq->res.scheduler.l1c_drrid = 0;
	pq->res.scheduler.l1e_drrid = 0;
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_L1_SCHEDULER_VALID);

	ppe_qdisc_info("Qdisc:%px level1 queue scheduler configuration successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_l1_alloc()
 *	Allocates Level 1 scheduler resources
 */
static int ppe_qdisc_res_l1_alloc(struct ppe_qdisc *pq)
{
	struct ppe_qdisc_port_res *l1c_drr = NULL;
	struct ppe_qdisc_port_res *l1e_drr = NULL;
	struct ppe_qdisc_port_res *l0sp = NULL;

	/*
	 * Get Level 0 SP resource
	 */
	l0sp = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_SP);
	if (!l0sp) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ",
				pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_SP);
		goto fail;
	}

	/*
	 * Get Level 1 DRR resource
	 */
	l1c_drr = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_CDRR);
	if (!l1c_drr) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ",
				pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_CDRR);
		goto fail;
	}

	l1e_drr = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_EDRR);
	if (!l1e_drr) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ",
				pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_EDRR);
		goto fail;
	}

	/*
	 * Set Res id values in qdisc
	 */
	pq->res.q.ucast_qid = 0;
	pq->res.q.mcast_qid = 0;
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID);

	pq->res.scheduler.l0c_drrid = 0;
	pq->res.scheduler.l0e_drrid = 0;

	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_L1_SCHEDULER_VALID);
	pq->res.l0spid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_SP) + l0sp->offset;
	pq->res.scheduler.l1c_drrid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_CDRR) + l1c_drr->offset;
	pq->res.scheduler.l1e_drrid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L1_EDRR) + l1e_drr->offset;

	ppe_qdisc_info("Qdisc:%px level1 scheduler resource allocation successful", pq->qdisc);
	return 0;

fail:
	if (l0sp) {
		ppe_qdisc_port_res_free(pq->port_id, l0sp->offset, PPE_DRV_QOS_RES_TYPE_L0_SP);
	}

	if (l1e_drr) {
		ppe_qdisc_port_res_free(pq->port_id, l1e_drr->offset, PPE_DRV_QOS_RES_TYPE_L1_EDRR);
	}

	if (l1c_drr) {
		ppe_qdisc_port_res_free(pq->port_id, l1c_drr->offset, PPE_DRV_QOS_RES_TYPE_L1_CDRR);
	}
	return -EINVAL;
}

/*
 * ppe_qdisc_res_l1_scheduler_set()
 *	Configures Level 1 scheduler in PPE.
 */
static int ppe_qdisc_res_l1_scheduler_set(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;

	/*
	 * Allocate resources if we have not already done so.
	 */
	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_L1_SCHEDULER_VALID) && ppe_qdisc_res_l1_alloc(pq) != 0) {
		ppe_qdisc_warning("Qdisc:%px level1 queue scheduler configuration failed", pq->qdisc);
		return -EINVAL;
	}

	/*
	 * Disable all queues and set Level 1 PPE configuration
	 * We need to disable and flush the queues before
	 * changing scheduler's sp_id/drr_id/priority.
	 */
	ppe_qdisc_port_disable_all_queue(pq->port_id);

	/*
	 * Set Level 1 configuration
	 */
	if (ppe_drv_qos_l1_scheduler_set(res, pq->port_id) != PPE_DRV_RET_SUCCESS) {
		ppe_qdisc_warning("Qdisc:%px level1 queue scheduler configuration failed", pq->qdisc);
		ppe_qdisc_port_enable_all_queue(pq->port_id);
		ppe_qdisc_res_l1_free(pq);
		return -EINVAL;
	}

	ppe_qdisc_port_enable_all_queue(pq->port_id);

	ppe_qdisc_info("Qdisc:%px level1 queue scheduler configuration successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_l0_free()
 *	Frees level0 scheduler resources.
 */
static int ppe_qdisc_res_l0_free(struct ppe_qdisc *pq)
{
	uint32_t offset;

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_L0_SCHEDULER_VALID)) {
		return 0;
	}

	/*
	 * Free Level 0 DRR resource
	 */
	offset = pq->res.scheduler.l0c_drrid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_CDRR);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_L0_CDRR) != 0) {
		ppe_qdisc_warning("Qdisc:%px used res:%d not found for port:%d, type:%d ",
				pq->qdisc, pq->res.scheduler.l0c_drrid, pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_CDRR);
		return -EINVAL;
	}

	offset = pq->res.scheduler.l0e_drrid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_EDRR);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_L0_EDRR) != 0) {
		ppe_qdisc_warning("Qdisc:%px used res:%d not found for port:%d, type:%d ",
				pq->qdisc, pq->res.scheduler.l0e_drrid, pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_EDRR);
		return -EINVAL;
	}

	/*
	 * Free Level 0 queue resource
	 */
	offset = pq->res.q.ucast_qid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE) != 0) {
		ppe_qdisc_warning("Qdisc:%px  used res:%d not found for port:%d, type:%d ",
				pq->qdisc, pq->res.q.ucast_qid, pq->port_id, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE);
		return -EINVAL;
	}

	/*
	 * Reset Res id values in qdisc
	 */
	pq->res.q.ucast_qid = 0;
	pq->res.q.mcast_qid = 0;
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID);
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_MCAST_QUEUE_VALID);

	pq->res.scheduler.l0c_drrid = 0;
	pq->res.scheduler.l0e_drrid = 0;
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_L0_SCHEDULER_VALID);

	ppe_qdisc_info("Qdisc:%px level0 scheduler resource de-allocation successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_l0_scheduler_reset()
 *	Resets Level 0 scheduler configuration in PPE.
 */
static int ppe_qdisc_res_l0_scheduler_reset(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;

	ppe_qdisc_port_disable_all_queue(pq->port_id);
	if (ppe_drv_qos_l0_scheduler_reset(res, pq->port_id) != PPE_DRV_RET_SUCCESS) {
		ppe_qdisc_warning("Qdisc:%px level0 queue scheduler configuration failed", pq->qdisc);
		ppe_qdisc_port_enable_all_queue(pq->port_id);
		return -EINVAL;
	}

	if (ppe_qdisc_res_l0_free(pq) != 0) {
		ppe_qdisc_warning("Qdisc:%px level0 scheduler resources de-allocation failed", pq->qdisc);
		ppe_qdisc_port_enable_all_queue(pq->port_id);
		return -EINVAL;
	}

	ppe_qdisc_port_enable_all_queue(pq->port_id);
	ppe_qdisc_info("Qdisc:%px level0 queue scheduler configuration successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_l0_alloc()
 *	Allocates level 0 resources.
 */
static int ppe_qdisc_res_l0_alloc(struct ppe_qdisc *pq)
{
	struct ppe_qdisc_port_res *l0c_drr = NULL;
	struct ppe_qdisc_port_res *l0e_drr = NULL;
	struct ppe_qdisc_port_res *q = NULL;

	/*
	 * Get Level 0 DRR Resource
	 */
	l0c_drr = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_CDRR);
	if (!l0c_drr) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ",
				pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_CDRR);
		goto fail;
	}

	l0e_drr = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_EDRR);
	if (!l0e_drr) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ",
				pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_EDRR);
		goto fail;
	}

	/*
	 * Get Level 0 queue Resource
	 */
	q = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE);
	if (!q) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ",
				pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE);
		goto fail;
	}

	/*
	 * Set res id values in qdisc
	 */
	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID);
	pq->res.q.ucast_qid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE) + q->offset;
	if (pq->type > PPE_QDISC_NODE_SCH_MAX) {
		pq->int_pri = q->offset;
		ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_INT_PRI_VALID);
		ppe_qdisc_info("%px:qdisc level0 scheduler resource int-pri:%d set", pq->qdisc, pq->int_pri);
	}

	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_L0_SCHEDULER_VALID);
	pq->res.scheduler.l0c_drrid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_CDRR) + l0c_drr->offset;
	pq->res.scheduler.l0e_drrid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_L0_EDRR) + l0e_drr->offset;
	pq->res.q.mcast_qid = 0;

	/*
	 * If a qdisc or class is attached at queue level,
	 * we need to set the L0 SP Id from the parent qdisc
	 * to make connection between L0 and L1 resources.
	 */
	if (pq->level == PPE_DRV_QOS_QUEUE_LEVEL) {
		pq->res.l0spid = pq->parent->res.l0spid;
	}

	ppe_qdisc_info("Qdisc:%px level0 scheduler resource allocation successful", pq->qdisc);
	return 0;

fail:
	if (q) {
		ppe_qdisc_port_res_free(pq->port_id, q->offset, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE);
	}

	if (l0c_drr) {
		ppe_qdisc_port_res_free(pq->port_id, l0c_drr->offset, PPE_DRV_QOS_RES_TYPE_L0_CDRR);
	}

	if (l0e_drr) {
		ppe_qdisc_port_res_free(pq->port_id, l0e_drr->offset, PPE_DRV_QOS_RES_TYPE_L0_EDRR);
	}
	return -EINVAL;
}

/*
 * ppe_qdisc_res_l0_scheduler_set()
 *	Configures a level 0 scheduler in PPE.
 */
static int ppe_qdisc_res_l0_scheduler_set(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;

	/*
	 * Allocate resources only if scheduler is not valid
	 */
	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_L0_SCHEDULER_VALID)) {
		/*
		 * Allocate Level 0 resources
		 */
		if (ppe_qdisc_res_l0_alloc(pq) != 0) {
			ppe_qdisc_warning("Qdisc:%px level0 queue scheduler configuration failed", pq->qdisc);
			return -EINVAL;
		}
	}

	/*
	 * Disable all queues and set Level 0 PPE configuration
	 * We need to disable and flush the queues before
	 * changing scheduler's sp_id/drr_id/priority.
	 */
	ppe_qdisc_port_disable_all_queue(pq->port_id);

	/*
	 * Set Level 0 configuration
	 */
	if (ppe_drv_qos_l0_scheduler_set(res, pq->port_id) != PPE_DRV_RET_SUCCESS) {
		ppe_qdisc_warning("Qdisc:%px level0 queue scheduler configuration failed", pq->qdisc);
		ppe_qdisc_port_enable_all_queue(pq->port_id);
		ppe_qdisc_res_l0_free(pq);
		return -EINVAL;
	}

	ppe_qdisc_port_enable_all_queue(pq->port_id);

	ppe_qdisc_info("Qdisc:%px level0 queue scheduler configuration successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_mcast_queue_reset()
 *	Deconfigures and deallocates a multicast queue in PPE.
 */
static int ppe_qdisc_res_mcast_queue_reset(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;
	uint32_t offset;

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_MCAST_QUEUE_VALID)) {
		return 0;
	}

	ppe_drv_qos_queue_disable(pq->port_id, res->q.mcast_qid);

	/*
	 * In case, multicast queue is attached to a classful qdisc at
	 * its maximum depth, then reset the multicast queue ID in parent qdisc
	 * and reset queue shaper.
	 */
	if (pq->level == PPE_DRV_QOS_SUB_QUEUE_LEVEL) {
		pq->parent->res.q.mcast_qid = 0;
		if (ppe_drv_qos_mcast_queue_shaper_set(res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px mcast queue shaper configuration failed", pq->qdisc);
			return -EINVAL;
		}
	}

	/*
	 * Free Level 0 multicast queue resource
	 */
	offset = res->q.mcast_qid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE);
	if (ppe_qdisc_port_res_free(pq->port_id, offset, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE) != 0) {
		ppe_qdisc_warning("Qdisc:%px used res:%d not found for port:%d, type:%d ",
				pq->qdisc, res->q.mcast_qid, pq->port_id, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE);
		return -EINVAL;
	}

	res->q.mcast_qid = 0;
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_MCAST_QUEUE_VALID);

	ppe_qdisc_info("Qdisc:%px multicast queue reset successful for port:%d",
			pq->qdisc, pq->port_id);
	return 0;
}

/*
 * ppe_qdisc_res_shaper_reset()
 *	Resets a configured shaper in PPE.
 */
static int ppe_qdisc_res_shaper_reset(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_SHAPER_VALID)) {
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_PORT_LEVEL) {
		if (ppe_drv_qos_port_shaper_reset(res, pq->port_id) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px Reset Port shaper failed", pq->qdisc);
			return -EINVAL;
		}
		ppe_qdisc_info("Qdisc:%px reset shaper successful", pq->qdisc);
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_FLOW_LEVEL) {
		if (ppe_drv_qos_flow_shaper_reset(res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px Reset Flow shaper failed", pq->qdisc);
			return -EINVAL;
		}
		ppe_qdisc_info("Qdisc:%px reset shaper successful", pq->qdisc);
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_QUEUE_LEVEL) {
		if (ppe_drv_qos_queue_shaper_reset(res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px Reset Queue shaper failed", pq->qdisc);
			return -EINVAL;
		}
	}

	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_SHAPER_VALID);

	ppe_qdisc_info("Qdisc:%px reset shaper successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_scheduler_reset()
 *	Resets a configured scheduler in PPE.
 */
static int ppe_qdisc_res_scheduler_reset(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_L0_SCHEDULER_VALID|PPE_QDISC_FLAG_L1_SCHEDULER_VALID)) {
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_PORT_LEVEL) {
		if (!ppe_qdisc_type_is_queue(pq)) {
			ppe_qdisc_info("Qdisc:%px reset scheduler successful", pq->qdisc);
			return 0;
		}

		if (ppe_qdisc_res_l0_scheduler_reset(pq) != 0) {
			ppe_qdisc_warning("Qdisc:%px Level0 queue scheduler reset failed", pq->qdisc);
			return -EINVAL;
		}

		if (ppe_qdisc_res_l1_free(pq) != 0) {
			ppe_qdisc_warning("Qdisc:%px Level1 queue scheduler reset failed", pq->qdisc);
			return -EINVAL;
		}

		ppe_qdisc_info("Qdisc:%px reset scheduler successful", pq->qdisc);
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_FLOW_LEVEL) {

		/*
		 * We have allocated L0 resources if this qdisc is of type queue or
		 * this is an hybrid (NSS) qdisc that is attached to PPE qdisc.
		 */
		if (ppe_qdisc_type_is_queue(pq)) {
			if (ppe_qdisc_res_l0_scheduler_reset(pq) < 0) {
				ppe_qdisc_warning("Qdisc:%px Level0 configuration for attach of new qdisc failed", pq->qdisc);
				return -EINVAL;
			}
		}

		if (ppe_qdisc_res_l1_free(pq) != 0) {
			ppe_qdisc_warning("Qdisc:%px Level1 queue scheduler reset failed", pq->qdisc);
			return -EINVAL;
		}
	} else if (pq->level == PPE_DRV_QOS_QUEUE_LEVEL) {
		if (ppe_qdisc_res_l0_scheduler_reset(pq) != 0) {
			ppe_qdisc_warning("Qdisc:%px Level0 queue scheduler reset failed", pq->qdisc);
			return -EINVAL;
		}
	} else {

		/*
		 * When a classful qdisc say HTB is configured with max levels of hierarchy,
		 * and then if a qdisc say FIFO is attached at the last level, we will have all
		 * the resources allocated and we just need to enable/disable the queue.
		 */
		if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID)) {
			ppe_drv_qos_queue_disable(pq->port_id, res->q.ucast_qid);
		}
	}

	ppe_qdisc_info("Qdisc:%px reset scheduler successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_mcast_queue_set()
 *	Allocates and configures a multicast queue in PPE.
 */
int ppe_qdisc_res_mcast_queue_set(struct ppe_qdisc *pq)
{
	struct ppe_qdisc_port_res *mcast_q = NULL;
	struct ppe_drv_qos_res *res = &pq->res;

	/*
	 * Currently multicast queue configuration is based on default node
	 * configuration. This is done as we are supporting only one multicast
	 * queue. So we enable that multicast queue for the default case.
	 *
	 * If multicast queue configurtion is not enabled,
	 * it could be possible that queue's multicast configuration
	 * has changed from enabled to disabled.
	 * So, we need to reset it.
	 */
	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_DEFAULT)) {
		ppe_qdisc_res_mcast_queue_reset(pq);
		return 0;
	}

	/*
	 * If multicast queue configuartion is enabled,
	 * and we have a valid mcast queue, then nothing to do.
	 */
	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_MCAST_QUEUE_VALID)) {
		return 0;
	}

	/*
	 * Get Level 0 multicast queue Resource
	 */
	mcast_q = ppe_qdisc_port_res_alloc(pq->port_id, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE);
	if (!mcast_q) {
		ppe_qdisc_warning("Qdisc:%px free res not found for port:%d, type:%d ", pq->qdisc, pq->port_id, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE);
		return -EINVAL;
	}
	res->q.mcast_qid = ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE) + mcast_q->offset;
	ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_MCAST_QUEUE_VALID);

	if (ppe_drv_qos_mcast_queue_set(res, pq->port_id) != PPE_DRV_RET_SUCCESS) {
		ppe_qdisc_warning("Qdisc:%px level0 mcast queue scheduler configuration failed", pq->qdisc);
		goto fail;
	}

	/*
	 * In case, multicast queue is attached at NSS_PPE_SUB_QUEUE_LEVEL level,
	 * set queue shaper for multicast queue.
	 */
	if (pq->level == PPE_DRV_QOS_SUB_QUEUE_LEVEL) {
		if (ppe_drv_qos_mcast_queue_shaper_set(res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px mcast queue shaper configuration failed", pq->qdisc);
			goto fail;
		}
	}

	ppe_drv_qos_queue_enable(res->q.mcast_qid);

	ppe_qdisc_info("Qdisc:%px multicast queue configuration successful for port:%d", pq->qdisc, pq->port_id);
	return 0;

fail:
	ppe_qdisc_port_res_free(pq->port_id, mcast_q->offset, PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE);
	res->q.mcast_qid = 0;
	ppe_qdisc_flags_clear(pq, PPE_QDISC_FLAG_MCAST_QUEUE_VALID);
	return -EINVAL;
}

/*
 * ppe_qdisc_res_shaper_set()
 *	Configures a shaper in PPE.
 */
int ppe_qdisc_res_shaper_set(struct ppe_qdisc *pq)
{
	struct ppe_drv_qos_res *res = &pq->res;

	if (!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_SHAPER_VALID)) {
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_PORT_LEVEL) {
		if (ppe_drv_qos_port_shaper_set(res, pq->port_id) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px port shaper configuration failed", pq->qdisc);
			return -EINVAL;
		}
		ppe_qdisc_info("Qdisc:%px set shaper successful", pq->qdisc);
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_FLOW_LEVEL) {
		if (ppe_drv_qos_flow_shaper_set(res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px port shaper configuration failed", pq->qdisc);
			return -EINVAL;
		}
		ppe_qdisc_info("Qdisc:%px set shaper successful", pq->qdisc);
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_QUEUE_LEVEL) {
		if (ppe_drv_qos_queue_shaper_set(res) != PPE_DRV_RET_SUCCESS) {
			ppe_qdisc_warning("Qdisc:%px queue shaper configuration failed", pq->qdisc);
			return -EINVAL;
		}
	}

	ppe_qdisc_info("Qdisc:%px set shaper successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_scheduler_set()
 *	Configures scheduler resources in PPE.
 */
int ppe_qdisc_res_scheduler_set(struct ppe_qdisc *pq)
{
	if (pq->level == PPE_DRV_QOS_PORT_LEVEL) {

		/*
		 * Only L1 SP can be allocated at port level if qdisc is not of type queue.
		 * Since L1 SP is equal to port number, we don't explicitly allocate it.
		 */
		if (!ppe_qdisc_type_is_queue(pq)) {
			ppe_qdisc_info("Qdisc:%px scheduler configuration successful", pq->qdisc);
			return 0;
		}

		/*
		 * Set queue schedulers for queue qdiscs attached at port level.
		 */
		if (ppe_qdisc_res_l1_scheduler_set(pq) < 0) {
			ppe_qdisc_warning("Qdisc:%px Level1 configuration failed", pq->qdisc);
			return -EINVAL;
		}

		if (ppe_qdisc_res_l0_scheduler_set(pq) < 0) {
			ppe_qdisc_res_l1_free(pq);
			ppe_qdisc_warning("Qdisc:%px Level0 configuration failed", pq->qdisc);
			return -EINVAL;
		}

		ppe_qdisc_info("Qdisc:%px scheduler configuration successful", pq->qdisc);
		return 0;
	}

	if (pq->level == PPE_DRV_QOS_FLOW_LEVEL) {
		if (ppe_qdisc_res_l1_scheduler_set(pq) < 0) {
			ppe_qdisc_warning("Qdisc:%px Level1 configuration failed", pq->qdisc);
			return -EINVAL;
		}

		/*
		 * We need to allocate L0 resources as well if qdisc is of type queue
		 */
		if (ppe_qdisc_type_is_queue(pq)) {
			if (ppe_qdisc_res_l0_scheduler_set(pq) < 0) {
				ppe_qdisc_warning("Qdisc:%px Level0 configuration failed", pq->qdisc);
				return -EINVAL;
			}
		}
	} else if (pq->level == PPE_DRV_QOS_QUEUE_LEVEL) {

		/*
		 * This case is invoked when a Qdisc/class is attached/changed
		 * at queue level. If a class is attached at queue level, we
		 * disable the queue until a qdisc is attached. but if it is
		 * invoked at change configuration time, we just set the schedulers.
		 */
		if (ppe_qdisc_res_l0_scheduler_set(pq) < 0) {
			ppe_qdisc_warning("Qdisc:%px Level0 configuration failed", pq->qdisc);
			return -EINVAL;
		}

		if ((!ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_CONFIGURED)) && (!ppe_qdisc_type_is_queue(pq))) {
			if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID)) {
				ppe_drv_qos_queue_disable(pq->port_id, pq->res.q.ucast_qid);
			}
		}
	} else {
		/*
		 * When a classful qdisc say HTB is configured with max levels of hierarchy,
		 * and then if a qdisc say FIFO is attached at the last level, we will have all
		 * the resources allocated and we just need to enable/disable the queue.
		 */
		if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID)) {
			ppe_drv_qos_queue_enable(pq->res.q.ucast_qid);
		}
	}

	ppe_qdisc_info("Qdisc:%px scheduler configuration successful", pq->qdisc);
	return 0;
}

/*
 * ppe_qdisc_res_free()
 *	Frees all the scheduler and shaper resources.
 */
void ppe_qdisc_res_free(struct ppe_qdisc *pq)
{
	ppe_qdisc_res_mcast_queue_reset(pq);
	ppe_qdisc_res_shaper_reset(pq);
	ppe_qdisc_res_scheduler_reset(pq);

	ppe_qdisc_info("PPE Qdisc:%px resource freed", pq->qdisc);
}

/*
 * ppe_qdisc_res_init()
 *	Allocates and initializes PPE Qdisc resources.
 */
int ppe_qdisc_res_init(struct ppe_qdisc *pq)
{
	struct net_device *dev = qdisc_dev(pq->qdisc);
	bool alloc_scheduler = true;

	memset(&pq->res, 0, sizeof(struct ppe_drv_qos_res));
	pq->res.q.qlimit = dev->tx_queue_len ? : 1;
	pq->res.scheduler.priority = PPE_DRV_QOS_PRIORITY_MAX;

	if (ppe_qdisc_flags_check(pq, PPE_QDISC_FLAG_NODE_ROOT)) {

		/*
		 * We do not allocate any schedulers for HTB Qdisc.
		 * This is because all other scheduler qdisc (such as TBL, WRR, etc) have some logic
	 	 * in qdisc that needs resource allocation in PPE. HTB qdisc on the other hand does
	 	 * nothing useful and thus we don't allocate any resource.
		 */
		if (pq->type == PPE_QDISC_NODE_TYPE_HTB) {
			alloc_scheduler = false;
		}

		pq->level = PPE_DRV_QOS_MAX_LEVEL - 1;
		goto res_alloc;
	}

	/*
	 * Currently we only allow one ppehtb qdisc and
	 * that should be at the root.
	 */
	if (pq->type == PPE_QDISC_NODE_TYPE_HTB) {
		ppe_qdisc_warning("PPE HTB qdisc:%px needs to be root", pq->qdisc);
		return -1;
	}

	if (!pq->parent) {
		ppe_qdisc_warning("Qdisc:%px is not root and has no parent", pq->qdisc);
		return -1;
	}

	/*
	 * First HTB class has a same level as HTB qdisc.
	 * While all other qdiscs and classes wil be at one level less than the parent level.
	 */
	ppe_qdisc_info("Qdisc parent = %px, handle=%x", pq->parent, pq->parent->qos_tag);
	if ((pq->parent->type == PPE_QDISC_NODE_TYPE_HTB)) {
		pq->level = pq->parent->level;
	} else {
		pq->level = pq->parent->level - 1;
	}

	if (!ppe_qdisc_is_depth_valid(pq)) {
		ppe_qdisc_warning("Qdisc:%px tree is too deep", pq->qdisc);
		return -1;
	}


res_alloc:
	if (alloc_scheduler) {
		if (ppe_qdisc_res_scheduler_set(pq) < 0) {
			ppe_qdisc_warning("Qdisc:%px Scheduler configuration failed", pq->qdisc);
			return -1;
		}
	}

	if (pq->level == PPE_DRV_QOS_SUB_QUEUE_LEVEL) {
		pq->res.q = pq->parent->res.q;
		ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_UCAST_QUEUE_VALID);
		pq->int_pri = pq->parent->res.q.ucast_qid - ppe_qdisc_port_res_base_get(pq->port_id, PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE);
		ppe_qdisc_flags_set(pq, PPE_QDISC_FLAG_INT_PRI_VALID);
		ppe_qdisc_info("Qdisc:%px level0 scheduler resource int-pri:%d set\n", pq->qdisc, pq->int_pri);
	}

	ppe_qdisc_info("PPE Qdisc:%px initialization successful", pq->qdisc);
	return 0;
}
