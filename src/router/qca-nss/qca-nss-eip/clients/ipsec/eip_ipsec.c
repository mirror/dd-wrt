/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: ISC
 */

#include <linux/version.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/if.h>
#include <linux/list.h>
#include <linux/netdevice.h>
#include <linux/completion.h>
#include <net/addrconf.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/debugfs.h>
#include <linux/inetdevice.h>
#include <eip.h>

#include "eip_ipsec_priv.h"

#define EIP_IPSEC_MAX_CORE_MASK ((1 << NR_CPUS) - 1)	/* Max value of core mask */

/*
 * global ipsec instance.
 */
struct eip_ipsec_drv eip_ipsec_drv_g;

/*
 * Module params
 */
uint8_t eip_ipsec_core_id = 0x2;
module_param(eip_ipsec_core_id, byte, S_IRUSR | S_IWUSR | S_IRGRP);
MODULE_PARM_DESC(eip_ipsec_core_id, "CPU id for IPsec processing");

bool disable_v4_offload = false;
module_param(disable_v4_offload, bool, S_IRUSR | S_IWUSR | S_IRGRP);
MODULE_PARM_DESC(disable_v4_offload, "Disable ipv4 acceleration");

int eip_pkt_steer_qlen = EIP_IPSEC_PKT_STEER_FIFO_LEN;
module_param(eip_pkt_steer_qlen, int, S_IRUSR | S_IWUSR | S_IRGRP);
MODULE_PARM_DESC(eip_pkt_steer_qlen, "FIFO length required for packet steer queues");

ulong eip_rps_core_mask = 0x0;	/* Should be enabled as required */
module_param(eip_rps_core_mask, ulong, S_IRUSR | S_IWUSR | S_IRGRP);
MODULE_PARM_DESC(eip_rps_core_mask, "IPsec RPS core mask");

/*
 * eip_ipsec_ppe_init()
 *	Initialize PPE port if enabled.
 */
static int eip_ipsec_ppe_init(void)
{
#if defined(EIP_IPSEC_VP_SUPP)
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	struct ppe_drv_iface *iface;
	ppe_drv_ret_t ret;

	if (!eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
		return 0;
	}

	/*
	 * Allocate and initialize the inline port
	 */
	iface = ppe_drv_iface_alloc(PPE_DRV_IFACE_TYPE_EIP, NULL);
	if (!iface) {
		pr_err("%px: failed to allocate PPE interface for init", drv);
		return -1;
	}

	ret = ppe_drv_eip_init(iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		pr_err("%px: failed to initiallize PPE physical port", drv);
		ppe_drv_iface_deref(iface);
		return -1;
	}

	drv->iface = iface;
	return 0;
#else
	pr_debug("%px: PPE is not enabled", &eip_ipsec_drv_g);
	return 0;
#endif
}

/*
 * eip_ipsec_ppe_deinit()
 *	De-Initialize PPE port if enabled.
 */
static void eip_ipsec_ppe_deinit(void)
{
#if defined(EIP_IPSEC_VP_SUPP)
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	ppe_drv_ret_t ret;

	if (!eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW)) {
		return;
	}

	/*
	 * Should we do BUG_ON()?
	 */
	ret = ppe_drv_eip_deinit(drv->iface);
	if (ret != PPE_DRV_RET_SUCCESS) {
		pr_err("%px: failed to De-initiallize PPE physical port", drv);
	}

	ppe_drv_iface_deref(drv->iface);
	drv->iface = NULL;
#else
	pr_debug("%px: PPE is not enabled", &eip_ipsec_drv_g);
#endif
}

/*
 * eip_ipsec_drv_final()
 *	Mark final deref completion.
 */
void eip_ipsec_drv_final(struct kref *kref)
{
	struct eip_ipsec_drv *drv = container_of(kref, struct eip_ipsec_drv, ref);

	complete(&drv->completion);
}

/*
 * eip_ipsec_init_module()
 *	Module initialization
 */
int __init eip_ipsec_init_module(void)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;
	enum eip_svc svc;
	int err = 0;
	int i;

	/*
	 * Check if DMA is loaded but not enabled.
	 */
	if (!eip_is_enabled()) {
		pr_info("IPsec Module can't be loaded as EIP is unavailable\n");
		return -1;
	}

	/*
	 * Assign the EIP service code.
	 */
	svc = eip_is_inline_supported() ? EIP_SVC_HYBRID_IPSEC : EIP_SVC_IPSEC;

	/*
	 * Validate module param.
	 */
	if ((eip_ipsec_core_id >= NR_CPUS) || (eip_rps_core_mask > EIP_IPSEC_MAX_CORE_MASK)) {
		pr_err("Invalid module param - ipsec_core_id: %d, rps_core_mask: %lu\n",
				(int)eip_ipsec_core_id, eip_rps_core_mask);
		return -1;
	}

	/*
	 * Initialize the global object.
	 * Dereference: eip_ipsec_exit_module()
	 */
	spin_lock_init(&drv->lock);
	init_completion(&drv->completion);

	drv->sa_cache = kmem_cache_create("eip_ipsec_sa", sizeof(struct eip_ipsec_sa), 0, 0, NULL);
	if (!drv->sa_cache) {
		pr_err("%px: Failed to allocate kmem cache for SA\n", drv);
		return -1;
	}

	/*
	 * Device mapping table.
	 */
	for (i = 0; i < ARRAY_SIZE(drv->sa_hlist); i++) {
		INIT_HLIST_HEAD(&drv->sa_hlist[i]);
	}

	/*
	 * Initializes the device db.
	 */
	INIT_LIST_HEAD(&drv->dev_head);

	/*
	 * Create DMA context.
	 */
	drv->ctx = eip_ctx_alloc(svc, &drv->dentry);
	if (!drv->ctx) {
		pr_err("%px: Failed to create DMA context\n", drv);
		err = -ENOSYS;
		goto ctx_fail;
	}

	/*
	 * Overide ESP protocol handler.
	 */
	if (!eip_ipsec_proto_esp_init()) {
		pr_err("%px: Failed to initialize ESP protocol\n", drv);
		err = -EPROTONOSUPPORT;
		goto esp_init;
	}

	err = eip_ipsec_ppe_init();
	if (err < 0) {
		pr_err("%px: Failed to initialize xfrm offload, error(%d)\n", drv, err);
		goto fail_ppe;
	}

	/*
	 * Enable transmit steer when ppe-eip inline channel is not available.
	 * With inline channel hardware supports multi-core receive.
	 */
	drv->tx_steer_en = !eip_feature_check(drv->ctx, EIP_OFFLOAD_OUTER_FLOW);

	/*
	 * Enable receive packet steer when it is configured by user.
	 */
	drv->rx_steer_en = eip_rps_core_mask ? true : false;

	/*
	 * XFRM offload initialization.
	 */
	err = eip_ipsec_xfrm_init();
	if (err < 0) {
		pr_err("%px: Failed to initialize xfrm offload, error(%d)\n", drv, err);
		goto fail_xfrm;
	}

	/*
	 * Initialize flow
	 */
	err = eip_ipsec_flow_init();
	if (err < 0) {
		pr_err("%px: Failed to initialize flow module, error(%d)\n", drv, err);
		goto fail_flow;
	}

	kref_init(&drv->ref);
	pr_info("eip ipsec module loaded\n");
	return 0;

fail_flow:
	eip_ipsec_xfrm_exit();
fail_xfrm:
	eip_ipsec_ppe_deinit();
fail_ppe:
	eip_ipsec_proto_esp_deinit();
esp_init:
	eip_ctx_free(drv->ctx);
ctx_fail:
	kmem_cache_destroy(drv->sa_cache);
	return err;
}

/*
 * eip_ipsec_exit_module()
 *	Module exit cleanup
 */
void __exit eip_ipsec_exit_module(void)
{
	struct eip_ipsec_drv *drv = &eip_ipsec_drv_g;

	/*
	 * Deinitialize flow
	 */
	eip_ipsec_flow_deinit();

	/*
	 * Dereference: eip_ipsec_init_module()
	 */
	eip_ipsec_drv_deref(drv);

	/*
	 * Wait for all deref. Drv reference is usally taken by device allocation.
	 */
	wait_for_completion(&drv->completion);

	eip_ipsec_xfrm_exit();
	eip_ipsec_ppe_deinit();

	/*
	 * Restore protocol handler.
	 */
	eip_ipsec_proto_esp_deinit();

	eip_ctx_free(drv->ctx);
	drv->ctx = NULL;
	kmem_cache_destroy(drv->sa_cache);
	pr_info("eip ipsec module unloaded\n");
}

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS EIP IPsec client");

module_init(eip_ipsec_init_module);
module_exit(eip_ipsec_exit_module);
