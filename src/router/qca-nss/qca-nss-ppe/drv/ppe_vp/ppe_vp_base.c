/*
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "ppe_vp_base.h"
#include "ppe_vp_rx.h"

struct ppe_vp_base vp_base;
int vp_enable __read_mostly = 0;

/*
 * ppe_vp_base_feature_enable()
 *	Enable General VP feature
 */
static void ppe_vp_base_feature_enable(struct ppe_vp_base *pvb)
{
	pvb->flags |= PPE_VP_BASE_FLAG_ENABLE_FEATURE;
}

/*
 * ppe_vp_base_feature_disable()
 *	Disable General VP feature
 */
static void ppe_vp_base_feature_disable(struct ppe_vp_base *pvb)
{
	pvb->flags &= ~PPE_VP_BASE_FLAG_ENABLE_FEATURE;
}

/*
 * ppe_vp_enable_handler()
 *	Toggle VP feature.
 *
 * Only disabled allocation of new VPs. Old VPs are not destroyed.
 */
static int ppe_vp_enable_handler(struct ctl_table *ctl, int write, void __user *buffer, size_t *lenp, loff_t *ppos)
{
	int ret;
	struct ppe_vp_base *pvb = &vp_base;

	ret = proc_dointvec(ctl, write, buffer, lenp, ppos);
	if (ret) {
		return ret;
	}

	if (write) {
		if (vp_enable) {
			ppe_vp_base_feature_enable(pvb);
			ppe_vp_info("%px: VP feature enabled\n", pvb);
			return ret;
		}

		ppe_vp_base_feature_disable(pvb);
		ppe_vp_info("%px: VP feature disabled\n", pvb);
	}

	return ret;
}

/*
 * nss_ppe_vp_feature
 *	Sysctl command.
 */
static struct ctl_table nss_ppe_vp_feature[] = {
	{
		.procname		= "vp_enable",
		.data			= &vp_enable,
		.maxlen			= sizeof(int),
		.mode			= 0644,
		.proc_handler		= &ppe_vp_enable_handler,
	},
	{ }
};

/*
 * ppe_vp_base_get_vp_by_port_num()
 *	Get PPE-VP base object by VP port number.
 *
 * Note: RCU lock is held while calling this API.
 */
struct ppe_vp *ppe_vp_base_get_vp_by_port_num(uint8_t port_num)
{
	struct ppe_vp_base *pvb = &vp_base;
	struct ppe_vp_table *pvt = &pvb->vp_table;
	uint8_t vp_idx = PPE_VP_BASE_PORT_TO_IDX(port_num);
	struct ppe_vp *vp = NULL;

	ppe_vp_assert(((port_num >= PPE_DRV_VIRTUAL_START) && (port_num < PPE_DRV_VIRTUAL_END)), "%px: Invalid port number: %d", pvb, port_num);

	if ((port_num < PPE_DRV_VIRTUAL_START) || (port_num >= PPE_DRV_VIRTUAL_END)) {
		ppe_vp_warn("%px: invalid VP number %d", pvb, port_num);
		return NULL;
	}

	vp = rcu_dereference(pvt->vp_allocator[vp_idx]);
	return vp;
}

/*
 * ppe_vp_base_get_vp_by_idx()
 *	Get PPE-VP base object by allocator index.
 *
 * Note: RCU lock is held while calling this API.
 */
struct ppe_vp *ppe_vp_base_get_vp_by_idx(int16_t vp_idx)
{
	struct ppe_vp_base *pvb = &vp_base;
	struct ppe_vp_table *pvt = &pvb->vp_table;
	struct ppe_vp *vp = NULL;

	ppe_vp_assert(((vp_idx >= 0) && (vp_idx < PPE_DRV_VIRTUAL_MAX)), "%px: Invalid index %d for VP table", pvb, vp_idx);

	if ((vp_idx < 0) || (vp_idx >= PPE_DRV_VIRTUAL_MAX)) {
		ppe_vp_warn("%px: invalid VP index %d", pvb, vp_idx);
		return NULL;
	}

	vp = rcu_dereference(pvt->vp_allocator[vp_idx]);
	return vp;
}

/*
 * ppe_vp_base_free_vp()
 *	Free a VP at a certain index in the VP table.
 */
bool ppe_vp_base_free_vp(uint8_t port_num)
{
	struct ppe_vp_base *pvb = &vp_base;
	struct ppe_vp_table *pvt = &pvb->vp_table;
	uint8_t vp_idx = PPE_VP_BASE_PORT_TO_IDX(port_num);
	struct ppe_vp *vp = NULL;

	ppe_vp_assert(((port_num >= PPE_DRV_VIRTUAL_START) && (port_num < PPE_DRV_VIRTUAL_END)), "%px: Invalid port number: %d", pvb, port_num);

	/*
	 * Using spin lock to write to a RCU object.
	 */
	spin_lock_bh(&pvb->lock);
	vp = rcu_dereference_protected(pvt->vp_allocator[vp_idx], 1);
	if (!vp) {
		ppe_vp_warn("%px: VP already freed, cannot free this VP at idx %u", pvb, vp_idx);
		spin_unlock_bh(&pvb->lock);
		return false;
	}

	rcu_assign_pointer(pvt->vp_allocator[vp_idx], NULL);
	pvt->active_vp--;
	spin_unlock_bh(&pvb->lock);
	synchronize_rcu();

	ppe_vp_info("%px: VP %px at idx %d freed", pvb, vp, vp_idx);
	return true;
}

/*
 * ppe_vp_base_alloc_vp()
 *	Get the first available VP.
 */
struct ppe_vp *ppe_vp_base_alloc_vp(uint8_t port_num)
{
	struct ppe_vp_base *pvb = &vp_base;
	struct ppe_vp_table *pvt = &pvb->vp_table;
	uint8_t vp_idx = PPE_VP_BASE_PORT_TO_IDX(port_num);
	struct ppe_vp *vp = NULL;

	ppe_vp_assert(((port_num >= PPE_DRV_VIRTUAL_START) && (port_num < PPE_DRV_VIRTUAL_END)), "%px: Invalid port number: %d", pvb, port_num);

	if ((port_num < PPE_DRV_VIRTUAL_START) || (port_num >= PPE_DRV_VIRTUAL_END)) {
		ppe_vp_warn("%px: invalid VP number %d", pvb, port_num);
		return NULL;
	}

	/*
	 * Using spin lock to write to a RCU object.
	 */
	spin_lock_bh(&pvb->lock);
	vp = rcu_dereference_protected(pvt->vp_allocator[vp_idx], 1);
	if (vp) {
		ppe_vp_warn("%px: VP %px at idx (port num) %u is already in use", pvb, vp, vp_idx);
		atomic64_inc(&pvb->base_stats.vp_allocation_fails);
		spin_unlock_bh(&pvb->lock);
		return NULL;
	}

	vp = &pvt->vp_pool[vp_idx];
	rcu_assign_pointer(pvt->vp_allocator[vp_idx], vp);
	pvt->active_vp++;
	spin_unlock_bh(&pvb->lock);

	ppe_vp_info("%px: Successfully allocate VP %px at port num %u (idx %u)", pvb, vp, port_num, vp_idx);
	return vp;
}

/*
 * ppe_vp_base_deinit()
 *	De-initialize the Base VP.
 */
static void ppe_vp_base_deinit(void)
{
	struct ppe_vp_base *pvb = &vp_base;
	struct ppe_vp_table *pvt;
	struct ppe_vp *vp;
	ppe_vp_status_t ret;
	int i;

	/*
	 * Destroy the PPE VP
	 */
	pvt = &pvb->vp_table;
	for (i = 0; i < PPE_DRV_VIRTUAL_MAX; i++) {
		RCU_INIT_POINTER(pvt->vp_allocator[i], NULL);
		synchronize_rcu();
		vp = &pvt->vp_pool[i];

		ret = ppe_vp_stats_deinit(vp);
		if (ret != PPE_VP_STATUS_SUCCESS) {
			ppe_vp_warn("%px: Unable to de-initialize VP %px statistics at idx %d", pvb, vp, i);
			return;
		}
	}

	/*
	 * Do not block module unload even if VPs are still active.
	 * Most likely qca-nss-ppe-vp will be unloaded only if all its users are
	 * unloaded
	 */
	if (pvt->active_vp) {
		ppe_vp_warn("%px: %d VPs still active in the system", pvb, pvt->active_vp);
	}

	pvt->active_vp = 0;
	pvb->flags = 0;

	ppe_vp_info("PPE-VP module destroyed successfully");
}

/*
 * ppe_vp_base_init()
 *	API for PPE base infrastructure.
 *
 * This API handles the base infra structure of the VP.
 */
static void ppe_vp_base_init(void)
{
	struct ppe_vp_base *pvb = &vp_base;
	struct ppe_vp_table *pvt;
	struct ppe_vp *vp;
	ppe_vp_status_t ret;
	int i;

	/*
	 * Initialize the PPE VP table.
	 */
	pvt = &pvb->vp_table;
	for (i = 0; i < PPE_DRV_VIRTUAL_MAX; i++) {
		vp = &pvt->vp_pool[i];
		spin_lock_init(&vp->lock);

		ret = ppe_vp_stats_init(vp);
		if (ret != PPE_VP_STATUS_SUCCESS) {
			ppe_vp_warn("%px: Unable to initialize VP %px statistics at idx %d", pvb, vp, i);
			return;
		}
	}

	pvt->active_vp = 0;
	ppe_vp_base_feature_enable(pvb);
	spin_lock_init(&pvb->lock);

	ppe_vp_trace("%px: Allocated %d VPs, active vp %d", pvb, PPE_DRV_VIRTUAL_MAX, pvt->active_vp);
}

/*
 * ppe_vp_base_module_init()
 *	module init for ppe vp driver.
 */
static int __init ppe_vp_base_module_init(void)
{
	struct ppe_vp_base *pvb = &vp_base;

	/*
	 * Allocate the VP base infrastructure.
	 */
	ppe_vp_base_init();

	pvb->edma_vp_dev = nss_dp_vp_init();
	if (!pvb->edma_vp_dev) {
		ppe_vp_warn("%px: Virtual port DP initialization failed", pvb);
		return -EINVAL;
	}

	pvb->vp_hdr = register_sysctl("ppe/ppe_vp", nss_ppe_vp_feature);
	if (!pvb->vp_hdr) {
		ppe_vp_warn("%px: Unable to register sysctl table for PPE virtual port\n", pvb);
		return -EINVAL;
	}

	pvb->dentry = ppe_drv_get_dentry();
	if (!pvb->dentry) {
		ppe_vp_warn("%px: Invalid PPE dentry", pvb);
		return -EINVAL;
	}

	ppe_vp_base_stats_init(pvb);

	/*
	 * Register ppe_vp Rx handlers with nss-dp
	 */
	nss_dp_vp_rx_register_cb(ppe_vp_rx_dp_cb, ppe_vp_rx_dp_list_cb);

	ppe_vp_info("%px: PPE-VP module loaded successfully", pvb);

	return 0;
}
module_init(ppe_vp_base_module_init);

/*
 * ppe_vp_base_module_exit()
 *	module exit for ppe vp driver.
 */
static void __exit ppe_vp_base_module_exit(void)
{
	struct ppe_vp_base *pvb = &vp_base;

	nss_dp_vp_rx_unregister_cb();
	del_timer_sync(&pvb->hw_port_stats_timer);

	if (pvb->vp_hdr) {
		unregister_sysctl_table(pvb->vp_hdr);
	}

	if (!nss_dp_vp_deinit(pvb->edma_vp_dev)) {
		ppe_vp_warn("%px: Virtual port DP de-initialization failed", pvb);
	}
	pvb->edma_vp_dev = NULL;

	ppe_vp_base_deinit();

	ppe_vp_info("%px: PPE-VP module unloaded", pvb);
}
module_exit(ppe_vp_base_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("PPE VP driver");
