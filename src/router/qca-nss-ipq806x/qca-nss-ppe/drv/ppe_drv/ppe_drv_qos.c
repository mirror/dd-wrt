/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/types.h>

#include <fal/fal_qm.h>
#include <fal/fal_qos.h>
#include <fal/fal_shaper.h>
#include <fal/fal_bm.h>

#include "ppe_drv.h"

/*
 * Token number is assigned the max value so as to
 * avoid the packet loss at the start of shaper process.
 */
#define PPE_DRV_QOS_TOKEN_MAX		0x3fffffff

/*
 * Frame mode.
 */
enum ppe_drv_qos_frame_mode {
	PPE_DRV_QOS_FRAME_MODE_IPG_PREAMBLE_FRAME_CRC,	/* IPG + Preamble + Frame + CRC */
	PPE_DRV_QOS_FRAME_MODE_FRAME_CRC,	/* Frame + CRC */
	PPE_DRV_QOS_FRAME_MODE_L3_EXCLUDE_CRC,	/* after Ethernet type exclude CRC*/
	PPE_DRV_QOS_FRAME_MODE_MAX		/* Maximum frame mode */
};
typedef enum ppe_drv_qos_frame_mode ppe_drv_qos_frame_mode_t;

/*
 * ppe_drv_qos_int_pri_callback_unregister()
 *	API to unregister INT-PRI fetch callback.
 */
void ppe_drv_qos_int_pri_callback_unregister()
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	p->int_pri_get_cb = NULL;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_qos_int_pri_callback_unregister);

/*
 * ppe_drv_qos_int_pri_callback_register()
 *	API to register INT-PRI fetch callback.
 */
void ppe_drv_qos_int_pri_callback_register(ppe_drv_qos_int_pri_callback_t cb)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * TODO: Return error in case cb is NULL
	 */
	spin_lock_bh(&p->lock);
	p->int_pri_get_cb = cb;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_qos_int_pri_callback_register);

/*
 * ppe_drv_qos_int_pri_get()
 *	Returns the INT-PRI value for a class ID.
  */
int ppe_drv_qos_int_pri_get(struct net_device *dev, uint32_t classid)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint8_t int_pri = 0;

	spin_lock_bh(&p->lock);
	if (p->int_pri_get_cb) {
		int_pri = p->int_pri_get_cb(dev, classid);
	}
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:get int_pri:%d successful for dev:%px classid:%u", p, int_pri, dev, classid);
	return int_pri;
}
EXPORT_SYMBOL(ppe_drv_qos_int_pri_get);

/*
 * ppe_drv_qos_queue_stats_get()
 *	API to fetch queue statistics from PPE HW.
 */
void ppe_drv_qos_queue_stats_get(uint32_t qid, bool is_red, struct ppe_drv_qos_q_stat *stats)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_queue_stats_t info;

	spin_lock_bh(&p->lock);
	fal_queue_counter_get(0, qid, &info);
	spin_unlock_bh(&p->lock);

	stats->tx_pkts = info.tx_packets;
	stats->tx_bytes = info.tx_bytes;

	stats->drop_pkts = info.drop_packets[PPE_DRV_QOS_QUEUE_COLOR_GREEN + 3];
	stats->drop_bytes = info.drop_bytes[PPE_DRV_QOS_QUEUE_COLOR_GREEN + 3];

	if (is_red) {
		stats->drop_pkts += info.drop_packets[PPE_DRV_QOS_QUEUE_COLOR_GREEN];
		stats->drop_bytes += info.drop_bytes[PPE_DRV_QOS_QUEUE_COLOR_GREEN];
	}

	ppe_drv_info("%px:get queue stats successful for qid:%u", p, qid);
}
EXPORT_SYMBOL(ppe_drv_qos_queue_stats_get);

/*
 * ppe_drv_qos_queue_stats_reset()
 *	API to reset queue statistics in PPE HW.
 */
void ppe_drv_qos_queue_stats_reset(uint32_t qid)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	fal_queue_counter_cleanup(0, qid);
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:reset queue stats successful for qid:%u", p, qid);
}
EXPORT_SYMBOL(ppe_drv_qos_queue_stats_reset);

/*
 * ppe_drv_qos_res_queue_disable()
 *	Disables a queue in PPE HW.
 */
void ppe_drv_qos_queue_disable(uint32_t port_id, uint32_t qid)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Disable queue enqueue, dequeue and flush the queue.
	 */
	spin_lock_bh(&p->lock);
	fal_qm_enqueue_ctrl_set(0, qid, A_FALSE);
	fal_scheduler_dequeue_ctrl_set(0, qid, A_FALSE);
	fal_queue_flush(0, port_id, qid);
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:disable level0 queue scheduler successful for qid:%u", p, qid);
}
EXPORT_SYMBOL(ppe_drv_qos_queue_disable);

/*
 * ppe_drv_qos_queue_enable()
 *	Enables a queue in PPE.
 */
void ppe_drv_qos_queue_enable(uint32_t qid)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Enable queue enqueue and dequeue.
	 */
	spin_lock_bh(&p->lock);
	fal_qm_enqueue_ctrl_set(0, qid, A_TRUE);
	fal_scheduler_dequeue_ctrl_set(0, qid, A_TRUE);
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:enable SSDK level0 queue scheduler successful for qid:%u", p, qid);
}
EXPORT_SYMBOL(ppe_drv_qos_queue_enable);

/*
 * ppe_drv_qos_l1_scheduler_set()
 *	Configures Level 1 scheduler in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_l1_scheduler_set(struct ppe_drv_qos_res *res, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_qos_scheduler_cfg_t l1cfg = {0};

	if (res->scheduler.drr_weight >= PPE_DRV_QOS_DRR_WEIGHT_MAX) {
		ppe_drv_warn("%px:drr weight:%u should be less than 1024",
				p, res->scheduler.drr_weight);
		return PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL;
	}

	spin_lock_bh(&p->lock);
	l1cfg.sp_id = port_id;

	l1cfg.c_drr_wt = res->scheduler.drr_weight ? res->scheduler.drr_weight : 1;
	l1cfg.c_drr_unit = res->scheduler.drr_unit;
	l1cfg.e_drr_wt = res->scheduler.drr_weight ? res->scheduler.drr_weight : 1;
	l1cfg.e_drr_unit = res->scheduler.drr_unit;
	l1cfg.c_pri = PPE_DRV_QOS_PRIORITY_MAX - res->scheduler.priority;
	l1cfg.e_pri = PPE_DRV_QOS_PRIORITY_MAX - res->scheduler.priority;
	l1cfg.c_drr_id = res->scheduler.l1c_drrid;
	l1cfg.e_drr_id = res->scheduler.l1e_drrid;
	l1cfg.drr_frame_mode = (fal_qos_drr_frame_mode_t)PPE_DRV_QOS_FRAME_MODE_FRAME_CRC;

	ppe_drv_trace("%px:level1 configuration: port:%u, l0spid:%u, c_drrid:%u, c_pri:%u, c_drr_wt:%u, e_drrid:%u, e_pri:%u, e_drr_wt:%u, l1spid:%u",
			p, port_id, res->l0spid, l1cfg.c_drr_id, l1cfg.c_pri, l1cfg.c_drr_wt, l1cfg.e_drr_id, l1cfg.e_pri, l1cfg.e_drr_wt, l1cfg.sp_id);
	if (fal_queue_scheduler_set(0, res->l0spid, PPE_DRV_QOS_FLOW_LEVEL - 1, port_id, &l1cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:level1 queue scheduler configuration failed for port:%u", p, port_id);
		return PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:level1 queue scheduler configuration successful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_l1_scheduler_set);

/*
 * ppe_drv_qos_l0_scheduler_reset()
 *	Resets Level 0 scheduler configuration in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_l0_scheduler_reset(struct ppe_drv_qos_res *res, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_qos_scheduler_cfg_t l0cfg = {0};

	/*
	 * Reset Level 0 configuration
	 */
	spin_lock_bh(&p->lock);
	l0cfg.sp_id = res->l0spid;
	l0cfg.c_drr_wt = 0;
	l0cfg.e_drr_wt = 0;
	l0cfg.c_drr_id = res->scheduler.l0c_drrid;
	l0cfg.e_drr_id = res->scheduler.l0e_drrid;

	ppe_drv_trace("%px:level0 configuration: port:%u, ucast_qid:%u, c_drrid:%u, c_pri:%u, c_drr_wt:%u, e_drrid:%u, e_pri:%u, e_drr_wt:%u, l0spid:%u",
			p, port_id, res->q.ucast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, res->q.ucast_qid, PPE_DRV_QOS_QUEUE_LEVEL - 1, port_id, &l0cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:level0 queue scheduler configuration failed for port:%u", p, port_id);
		return PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:level0 queue scheduler configuration successful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_l0_scheduler_reset);

/*
 * ppe_drv_qos_l0_scheduler_set()
 *	Configures a level 0 queue scheduler in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_l0_scheduler_set(struct ppe_drv_qos_res *res, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_qos_scheduler_cfg_t l0cfg = {0};

	if (res->scheduler.drr_weight >= PPE_DRV_QOS_DRR_WEIGHT_MAX) {
		ppe_drv_warn("%px:drr weight:%u should be less than 1024", p, res->scheduler.drr_weight);
		return PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL;
	}

	spin_lock_bh(&p->lock);
	l0cfg.sp_id = res->l0spid;
	l0cfg.c_drr_wt = res->scheduler.drr_weight ? res->scheduler.drr_weight : 1;
	l0cfg.c_drr_unit = res->scheduler.drr_unit;
	l0cfg.e_drr_wt = res->scheduler.drr_weight ? res->scheduler.drr_weight : 1;
	l0cfg.e_drr_unit = res->scheduler.drr_unit;
	l0cfg.c_pri = PPE_DRV_QOS_PRIORITY_MAX - res->scheduler.priority;
	l0cfg.e_pri = PPE_DRV_QOS_PRIORITY_MAX - res->scheduler.priority;
	l0cfg.c_drr_id = res->scheduler.l0c_drrid;
	l0cfg.e_drr_id = res->scheduler.l0e_drrid;
	l0cfg.drr_frame_mode = (fal_qos_drr_frame_mode_t)PPE_DRV_QOS_FRAME_MODE_FRAME_CRC;

	ppe_drv_trace("%px:level0 configuration: port:%u, ucast_qid:%u, c_drrid:%u, c_pri:%u, c_drr_wt:%u, e_drrid:%u, e_pri:%u, e_drr_wt:%u, l0spid:%u",
			p, port_id, res->q.ucast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, res->q.ucast_qid, PPE_DRV_QOS_QUEUE_LEVEL - 1, port_id, &l0cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:level0 queue scheduler configuration failed for port:%u", p, port_id);
		return PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL;
	}

	/*
	 * This function is called at the time of queue qdisc allocation at all
	 * levels and class allocation at PPE_DRV_QOS_QUEUE_LEVEL.
	 * Also invoked at time of change class at PPE_DRV_QOS_QUEUE_LEVEL and
	 * PPE_DRV_QOS_SUB_QUEUE_LEVEL levels. But, mcast will be valid only in
	 * case queue is attached at PPE_DRV_QOS_SUB_QUEUE_LEVEL and change class
	 * is invoked at PPE_DRV_QOS_QUEUE_LEVEL level.
	 * Other scenarios catering to mcast queue are handled in separate mcast APIs.
	 */
	if (res->q.mcast_qid) {
		ppe_drv_trace("%px:level0 configuration: port:%u, mcast_qid:%u, c_drrid:%u, c_pri:%u, c_drr_wt:%u, e_drrid:%u, e_pri:%u, e_drr_wt:%u, l0spid:%u",
				p, port_id, res->q.mcast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
		if (fal_queue_scheduler_set(0, res->q.mcast_qid, PPE_DRV_QOS_QUEUE_LEVEL - 1, port_id, &l0cfg) != 0) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%px:level0 multicast queue scheduler configuration failed for port:%u", p, port_id);
			return PPE_DRV_RET_QOS_SCHEDULER_CFG_FAIL;
		}
	}

	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px:level0 queue scheduler configuration successful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_l0_scheduler_set);

/*
 * ppe_drv_qos_port_shaper_reset()
 *	Resets a port shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_port_shaper_reset(struct ppe_drv_qos_res *res, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_config_t cfg = {0};

	spin_lock_bh(&p->lock);
	ppe_drv_trace("%px:port shaper reset for port:%u", p, port_id);
	if (fal_port_shaper_set(0, port_id, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:port shaper configuration failed for port:%u",
				p, port_id);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px: port shaper configuration successful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_port_shaper_reset);

/*
 * ppe_drv_qos_port_shaper_set()
 *	Configures a port shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_port_shaper_set(struct ppe_drv_qos_res *res, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_token_number_t token = {0};
	fal_shaper_config_t cfg = {0};

	/*
	 * Set port shaper token number
	 */
	spin_lock_bh(&p->lock);
	token.c_token_number = PPE_DRV_QOS_TOKEN_MAX;
	token.e_token_number = PPE_DRV_QOS_TOKEN_MAX;

	ppe_drv_trace("%px:port token set for port:%u, c_token_number:%x, e_token_number:%x",
		p, port_id, token.c_token_number, token.e_token_number);
	if (fal_port_shaper_token_number_set(0, port_id, &token) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:port shaper token configuration failed for port:%u",
				p, port_id);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	/*
	 * Set port shaper configuration
	 * Note: SSDK API requires burst in bytes/sec
	 * while rate in kbits/sec.
	 */
	cfg.c_shaper_en = 1;
	cfg.cbs = res->shaper.cburst;
	cfg.cir = (res->shaper.crate / 1000) * 8;
	cfg.shaper_frame_mode = (fal_shaper_frame_mode_t)PPE_DRV_QOS_FRAME_MODE_FRAME_CRC;

	ppe_drv_trace("%px:port shaper configuration for port:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u",
			p, port_id, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_port_shaper_set(0, port_id, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:port shaper configuration failed for port:%u",
				p, port_id);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:port shaper configuration successful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_port_shaper_set);

/*
 * ppe_drv_qos_flow_shaper_reset()
 *	Resets a flow shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_flow_shaper_reset(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_config_t cfg = {0};

	/*
	 * Reset flow shaper configuration
	 */
	spin_lock_bh(&p->lock);
	ppe_drv_trace("%px:flow shaper reset for l0spid:%u", p, res->l0spid);
	if (fal_flow_shaper_set(0, res->l0spid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:flow shaper configuration failed for l0spid:%u",
			p, res->l0spid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:flow shaper configuration successful for l0spid:%u", p, res->l0spid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_flow_shaper_reset);

/*
 * ppe_drv_qos_flow_shaper_set()
 *	Configures a flow shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_flow_shaper_set(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_token_number_t token = {0};
	fal_shaper_config_t cfg = {0};

	/*
	 * Set flow shaper token number
	 */
	spin_lock_bh(&p->lock);
	token.c_token_number = PPE_DRV_QOS_TOKEN_MAX;
	token.e_token_number = PPE_DRV_QOS_TOKEN_MAX;

	ppe_drv_trace("%px:flow token set for l0spid:%u, c_token_number:%x, e_token_number:%x",
		p, res->l0spid, token.c_token_number, token.e_token_number);
	if (fal_flow_shaper_token_number_set(0, res->l0spid, &token) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:flow shaper token configuration failed for l0spid:%u",
			p, res->l0spid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	/*
	 * Set flow shaper token configuration
	 * Note: SSDK API requires burst in bytes/sec
	 * while rate in kbits/sec.
	 */
	cfg.c_shaper_en = 1;
	cfg.cbs = res->shaper.burst;
	cfg.cir = (res->shaper.rate / 1000) * 8;
	cfg.e_shaper_en = 1;
	cfg.ebs = res->shaper.cburst;
	cfg.eir = ((res->shaper.crate / 1000) * 8) - cfg.cir;
	cfg.shaper_frame_mode = (fal_shaper_frame_mode_t)PPE_DRV_QOS_FRAME_MODE_FRAME_CRC;

	ppe_drv_trace("%px:flow shaper configuration: l0spid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u",
		p, res->l0spid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_flow_shaper_set(0, res->l0spid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:flow shaper configuration failed for l0spid:%u",
			p, res->l0spid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:flow shaper configuration successful for l0spid:%u", p, res->l0spid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_flow_shaper_set);

/*
 * ppe_drv_qos_mcast_queue_shaper_reset()
 *	Resets a multicast queue shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_mcast_queue_shaper_reset(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_config_t cfg = {0};

	spin_lock_bh(&p->lock);
	ppe_drv_trace("%px:multicast queue shaper reset for mcast_qid:%u", p, res->q.mcast_qid);
	if (fal_queue_shaper_set(0, res->q.mcast_qid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:multicast queue shaper configuration failed for mcast_qid:%u",
			p, res->q.mcast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:multicast queue reset successful for mcast_qid:%u", p, res->q.mcast_qid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_mcast_queue_shaper_reset);

/*
 * ppe_drv_qos_mcast_queue_shaper_set()
 *	Configures a multicast queue shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_mcast_queue_shaper_set(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_token_number_t token = {0};
	fal_shaper_config_t cfg = {0};

	spin_lock_bh(&p->lock);
	token.c_token_number = PPE_DRV_QOS_TOKEN_MAX;
	token.e_token_number = PPE_DRV_QOS_TOKEN_MAX;

	ppe_drv_trace("%px:multicast queue token set for mcast_qid:%u, c_token_number:%x, e_token_number:%x",
		p, res->q.mcast_qid, token.c_token_number, token.e_token_number);
	if (fal_queue_shaper_token_number_set(0, res->q.mcast_qid, &token) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:multicast queue shaper token configuration failed for mcast_qid:%u", p, res->q.mcast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	if (fal_queue_shaper_get(0, res->q.ucast_qid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue shaper configuration fetch failed for ucast_qid:%u", p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	ppe_drv_trace("%px:multicast queue shaper configuration for mcast_qid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u",
		p, res->q.mcast_qid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_queue_shaper_set(0, res->q.mcast_qid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:multicast queue shaper configuration failed for mcast_qid:%u", p, res->q.mcast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:queue shaper configuration successful for mcast_qid:%u", p, res->q.mcast_qid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_mcast_queue_shaper_set);

/*
 * ppe_drv_qos_queue_shaper_reset()
 *	Resets a queue shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_queue_shaper_reset(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_config_t cfg = {0};

	/*
	 * Reset queue shaper configuration
	 */
	spin_lock_bh(&p->lock);
	ppe_drv_trace("%px:queue shaper reset for ucast_qid:%u",
			p, res->q.ucast_qid);
	if (fal_queue_shaper_set(0, res->q.ucast_qid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue shaper configuration failed for ucast_qid:%u",
			p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:queue shaper configuration successful for ucast_qid:%u", p, res->q.ucast_qid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_queue_shaper_reset);

/*
 * ppe_drv_qos_queue_shaper_set()
 *	Configures a queue shaper in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_queue_shaper_set(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_shaper_token_number_t token = {0};
	fal_shaper_config_t cfg = {0};

	/*
	 * Set queue shaper token number
	 */
	spin_lock_bh(&p->lock);
	token.c_token_number = PPE_DRV_QOS_TOKEN_MAX;
	token.e_token_number = PPE_DRV_QOS_TOKEN_MAX;

	ppe_drv_trace("%px:queue token set for ucast_qid:%u, c_token_number:%x, e_token_number:%x",
		p, res->q.ucast_qid, token.c_token_number, token.e_token_number);
	if (fal_queue_shaper_token_number_set(0, res->q.ucast_qid, &token) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue shaper token configuration failed for ucast_qid:%u", p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	/*
	 * Set queue shaper confguration
	 * Note: SSDK API requires burst in bytes/sec
	 * while rate in kbits/sec.
	 */
	cfg.c_shaper_en = 1;
	cfg.cbs = res->shaper.burst;
	cfg.cir = (res->shaper.rate / 1000) * 8;
	cfg.e_shaper_en = 1;
	cfg.ebs = res->shaper.cburst;
	cfg.eir = ((res->shaper.crate / 1000) * 8) - cfg.cir;
	cfg.shaper_frame_mode = (fal_shaper_frame_mode_t)PPE_DRV_QOS_FRAME_MODE_FRAME_CRC;

	ppe_drv_trace("%px:queue shaper configuration for ucast_qid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u",
		p, res->q.ucast_qid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
	if (fal_queue_shaper_set(0, res->q.ucast_qid, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue shaper configuration failed for ucast_qid:%u", p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
	}

	/*
	 * This function is called at the time of class allocation and change at
	 * PPE_DRV_QOS_QUEUE_LEVEL. But, mcast will be valid only in case queue
	 * is attached at PPE_DRV_QOS_SUB_QUEUE_LEVEL and change class is invoked
	 * at PPE_DRV_QOS_QUEUE_LEVEL level.
	 * Other scenarios catering to mcast queue are handled in separate mcast APIs.
	 */
	if (res->q.mcast_qid) {
		ppe_drv_trace("%px:multicast queue shaper configuration for mcast_qid:%u, couple_en:%u, meter_unit:%u, c_shaper_en:%u, cbs:%u, cir:%u, ebs:%u, eir:%u, shaper_frame_mode:%u",
			p, res->q.mcast_qid, cfg.couple_en, cfg.meter_unit, cfg.c_shaper_en, cfg.cbs, cfg.cir, cfg.ebs, cfg.eir, cfg.shaper_frame_mode);
		if (fal_queue_shaper_set(0, res->q.mcast_qid, &cfg) != 0) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%px:multicast queue shaper configuration failed for mcast_qid:%u", p, res->q.mcast_qid);
			return PPE_DRV_RET_QOS_SHAPER_CFG_FAIL;
		}
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:queue shaper configuration successful for mcast_qid:%u", p, res->q.mcast_qid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_queue_shaper_set);

/*
 * ppe_drv_qos_mcast_queue_set()
 *	Configures a multicast queue in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_mcast_queue_set(struct ppe_drv_qos_res *res, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_qos_scheduler_cfg_t l0cfg;

	spin_lock_bh(&p->lock);
	ppe_drv_trace("%px:level0 configuration get for port:%u, ucast_qid:%u", p, port_id, res->q.ucast_qid);
	if (fal_queue_scheduler_get(0, res->q.ucast_qid, PPE_DRV_QOS_QUEUE_LEVEL - 1, &port_id, &l0cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:level0 unicast queue scheduler configuration fetch failed for port:%u, ucast_qid:%u", p, port_id, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
	}

	ppe_drv_trace("%px:level0 configuration: port:%u, mcast_qid:%u, c_drrid:%u, c_pri:%u, c_drr_wt:%u, e_drrid:%u, e_pri:%u, e_drr_wt:%u, l0spid:%u",
			p, port_id, res->q.mcast_qid, l0cfg.c_drr_id, l0cfg.c_pri, l0cfg.c_drr_wt, l0cfg.e_drr_id, l0cfg.e_pri, l0cfg.e_drr_wt, l0cfg.sp_id);
	if (fal_queue_scheduler_set(0, res->q.mcast_qid, PPE_DRV_QOS_QUEUE_LEVEL - 1, port_id, &l0cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:level0 multicast queue scheduler configuration failed for port:%u, ucast_qid:%u", p, port_id, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:multicast queue configuration successful for port:%u, ucast_qid:%u", p, port_id, res->q.ucast_qid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_mcast_queue_set);

/*
 * ppe_drv_qos_queue_limit_set()
 *	Sets queue size in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_queue_limit_set(struct ppe_drv_qos_res *res)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_ac_obj_t obj = {0};
	fal_ac_static_threshold_t cfg = {0};
	fal_ac_ctrl_t ctrl_cfg = {0};

	/*
	 * Unicast queue configuration.
	 */
	spin_lock_bh(&p->lock);
	obj.obj_id = res->q.ucast_qid;

	ppe_drv_trace("%px:queue buffer set for ucast_qid:%u, qlimit:%u", p, res->q.ucast_qid, res->q.qlimit);
	if (fal_ac_prealloc_buffer_set(0, &obj, res->q.qlimit) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue buffer allocation failed for ucast_qid:%u", p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
	}

	/*
	 * Enable force drop for PPE qdisc.
	 * When set to 1, the flow control will be overriden
	 * for that queue and packets drop gets enabled.
	 */
	ctrl_cfg.ac_en = 1;
	ctrl_cfg.ac_fc_en = 1;

	ppe_drv_trace("%px:queue flow control set for ucast_qid:%u, enable:%u", p, res->q.ucast_qid, ctrl_cfg.ac_fc_en);
	if (fal_ac_ctrl_set(0, &obj, &ctrl_cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue flow control set failed for ucast_qid:%u", p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
	}

	cfg.wred_enable = res->q.red_en;
	cfg.green_max = res->q.max_th[PPE_DRV_QOS_QUEUE_COLOR_GREEN];
	cfg.green_min_off = res->q.min_th[PPE_DRV_QOS_QUEUE_COLOR_GREEN];
	ppe_drv_trace("%px:queue ac threshold set for ucast_qid:%u, wred_enable:%u, green_max:%u, green_min_off:%u",
		p, res->q.ucast_qid, cfg.wred_enable, cfg.green_max, cfg.green_min_off);
	if (fal_ac_static_threshold_set(0, &obj, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:queue static threshold failed for ucast_qid:%u", p, res->q.ucast_qid);
		return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
	}

	/*
	 * Multicast queue configuration.
	 */
	if (res->q.mcast_qid) {
		obj.obj_id = res->q.mcast_qid;

		ppe_drv_trace("%px:multicast queue buffer set for mcast_qid:%u, qlimit:%u", p, res->q.mcast_qid, res->q.qlimit);
		if (fal_ac_prealloc_buffer_set(0, &obj, res->q.qlimit) != 0) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%px:multicast queue buffer allocation failed for mcast_qid:%u", p, res->q.mcast_qid);
			return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
		}

		ppe_drv_trace("%px:multicast queue ac threshold set for mcast_qid:%u, wred_enable:%u, green_max:%u, green_min_off:%u",
			p, res->q.mcast_qid, cfg.wred_enable, cfg.green_max, cfg.green_min_off);
		if (fal_ac_static_threshold_set(0, &obj, &cfg) != 0) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%px:multicast queue static threshold failed for mcast_qid:%u", p, res->q.mcast_qid);
			return PPE_DRV_RET_QOS_QUEUE_CFG_FAIL;
		}
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%px:queue configuration successful for ucast_qid:%u", p, res->q.ucast_qid);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_queue_limit_set);

/*
 *ppe_drv_qos_port_bm_control_enable()
 *	Enable or disable port buffer management for flow control
 */
void ppe_drv_qos_port_bm_control_enable(uint32_t port_id, bool set)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	fal_port_bm_ctrl_set(PPE_DRV_SWITCH_ID, port_id, set);
	spin_unlock_bh(&p->lock);

	ppe_drv_info("%px: port buffer flow control is successfully set to %d for port %u", p, set, port_id);

}
EXPORT_SYMBOL(ppe_drv_qos_port_bm_control_enable);

/*
 * ppe_drv_qos_default_conf_set()
 *	Sets default queue scheduler in PPE.
 */
ppe_drv_ret_t ppe_drv_qos_default_conf_set(uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Invoke SSDK API to reset the default configuration for a given port.
	 */
	spin_lock_bh(&p->lock);
	if (fal_port_scheduler_cfg_reset(0, port_id) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:reset default queue configuration failed for port:%u", p, port_id);
		return PPE_DRV_RET_QOS_PORT_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_qos_port_bm_control_enable(port_id, true);
	ppe_drv_info("%px:queue configuration successful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_default_conf_set);

/*
 * ppe_drv_qos_port_res_get()
 *	Gets boot time QoS resource allocation information for a given port.
 */
ppe_drv_ret_t ppe_drv_qos_port_res_get(uint32_t port_id, struct ppe_drv_qos_port *port)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_portscheduler_resource_t cfg = {0};

	/*
	 * Get boot time QoS resource information for a given port
	 */
	spin_lock_bh(&p->lock);
	if (fal_port_scheduler_resource_get(0, port_id, &cfg) != 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%px:port QoS resources info failed for port:%u", p, port_id);
		return PPE_DRV_RET_FAILURE_NO_RESOURCE;
	}

	spin_unlock_bh(&p->lock);
	port->max[PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE] = cfg.ucastq_num;
	port->base[PPE_DRV_QOS_RES_TYPE_UCAST_QUEUE] = cfg.ucastq_start;

	/*
	 * Even though we reserve more mcast queues in the device tree, we only use 1.
	 */
	port->max[PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE] = PPE_DRV_QOS_MCAST_QUEUE_MAX;
	port->base[PPE_DRV_QOS_RES_TYPE_MCAST_QUEUE] = cfg.mcastq_start;

	port->max[PPE_DRV_QOS_RES_TYPE_L0_CDRR] = cfg.l0cdrr_num;
	port->base[PPE_DRV_QOS_RES_TYPE_L0_CDRR] = cfg.l0cdrr_start;

	port->max[PPE_DRV_QOS_RES_TYPE_L0_EDRR] = cfg.l0edrr_num;
	port->base[PPE_DRV_QOS_RES_TYPE_L0_EDRR] = cfg.l0edrr_start;

	port->max[PPE_DRV_QOS_RES_TYPE_L0_SP] = cfg.l0sp_num;
	port->base[PPE_DRV_QOS_RES_TYPE_L0_SP] = cfg.l0sp_start;

	port->max[PPE_DRV_QOS_RES_TYPE_L1_CDRR] = cfg.l1cdrr_num;
	port->base[PPE_DRV_QOS_RES_TYPE_L1_CDRR] = cfg.l1cdrr_start;

	port->max[PPE_DRV_QOS_RES_TYPE_L1_EDRR] = cfg.l1edrr_num;
	port->base[PPE_DRV_QOS_RES_TYPE_L1_EDRR] = cfg.l1edrr_start;

	ppe_drv_info("%px:port QoS resources infosuccessful for port:%u", p, port_id);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_qos_port_res_get);
