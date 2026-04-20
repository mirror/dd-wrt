/*
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
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/netdevice.h>
#include <linux/version.h>
#include <ppe_drv_port.h>
#include <ppe_drv_tun_cmn_ctx.h>
#include <ppe_drv_tun_public.h>
#include <asm/cmpxchg.h>
#include "ppe_tun.h"

struct ppe_tun_priv *ptp;

/*
 * ppe_tun_allow_accel()
 *	Check if particular tunnel type acceleration supported.
 */
static bool ppe_tun_allow_accel(enum ppe_drv_tun_cmn_ctx_type type)
{
	switch (type) {
	case PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP:
		if (ptp->tun_accel.ppe_tun_gretap_accel) {
			return true;
		}
		ppe_tun_warn("%p: PPE gretap acceleration is not enabled", ptp);
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN:
		if (ptp->tun_accel.ppe_tun_vxlan_accel) {
			return true;
		}
		ppe_tun_warn("%p: PPE vxlan acceleration is not enabled", ptp);
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6:
		if (ptp->tun_accel.ppe_tun_ipip6_accel) {
			return true;
		}
		ppe_tun_warn("%p: PPE ipip6 acceleration is not enabled", ptp);
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_MAPT:
		if (ptp->tun_accel.ppe_tun_mapt_accel) {
			return true;
		}
		ppe_tun_warn("%p: PPE mapt acceleration is not enabled", ptp);
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2:
		if (ptp->tun_accel.ppe_tun_l2tp_accel) {
			return true;
		}
		ppe_tun_warn("%p: PPE l2tp acceleration is not enabled", ptp);
		break;

	default:
		break;
	}

	ppe_tun_warn("%p: Accel type %u is invalid", ptp, type);
	return false;
}

/*
 * ppe_tun_set_tun_data()
 *	Set tunnel information
 */
bool ppe_tun_set_tun_data(struct ppe_tun *tun, ppe_tun_data *tun_data)
{
	ppe_tun_data *tunnel_data = tun->tun_data;

	if (!tunnel_data) {
		ppe_tun_trace("%p: tunnel info not allocated for tunnel", tun);
		return false;
	}

	if (tun->type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2) {
		tunnel_data->l2tp_info.session_id = tun_data->l2tp_info.session_id;
		tunnel_data->l2tp_info.tunnel_id = tun_data->l2tp_info.tunnel_id;
		return true;
	}

	ppe_tun_warn("%p: invalid tunnel type %d to set tunnel info", ptp, tun->type);
	return false;
}

/*
 * ppe_tun_set_tun_data()
 *	allocate tunnel data memory
 */
ppe_tun_data *ppe_tun_alloc_tun_data(void)
{
	ppe_tun_data *tun_data;

	tun_data = kzalloc(sizeof(ppe_tun_data), GFP_ATOMIC);
	if (!tun_data) {
		ppe_tun_trace("%p: ppe tun data allocation failed\n", ptp);
	}

	return tun_data;
}

/*
 * ppe_tun_ctx_free()
 *	Free ppe tunnel allocated instance
 */
static void ppe_tun_ctx_free(struct kref *kref)
{
	struct ppe_tun *tun = container_of(kref, struct ppe_tun, ref);
	ppe_vp_num_t vp_num;

	spin_lock_bh(&ptp->lock);
	ptp->tun[tun->idx] = NULL;
	spin_unlock_bh(&ptp->lock);

	if (tun->tun_data) {
		kfree(tun->tun_data);
	}

	vp_num = tun->vp_num;
	ppe_vp_free(vp_num);

	kfree(tun);
	atomic_inc(&ptp->total_free);
	atomic_dec(&ptp->free_pending);
}

/*
 * ppe_tun_deref()
 *	Taken reference of tunnel instance
 */
static bool ppe_tun_deref(struct ppe_tun *tun)
{
	int32_t idx = tun->idx;

	if (kref_put(&tun->ref, ppe_tun_ctx_free)) {
		ppe_tun_trace("%p: reference count is 0 for tun at index: %u", tun, idx);
		return true;
	}

	ppe_tun_trace("%p: tun_idx: %u ref dec:%u", tun, idx, kref_read(&tun->ref));
	return false;
}

/*
 * ppe_tun_ref()
 *	Taken reference of tunnel instance
 */
static struct ppe_tun *ppe_tun_ref(struct ppe_tun *tun)
{
	kref_get(&tun->ref);

	ppe_tun_trace("%p: tun %u ref inc:%u", tun, tun->idx, kref_read(&tun->ref));
	return tun;
}

/*
 * ppe_tun_get_tun_by_netdev_and_ref()
 *	Get PPE tunnel by netdev and get ref
 */
static struct ppe_tun *ppe_tun_get_tun_by_netdev_and_ref(struct net_device *dev)
{
	struct ppe_tun *tun;
	int i = 0;

	spin_lock_bh(&ptp->lock);
	for (i = 0; i < PPE_TUN_MAX; i++) {
		tun = ptp->tun[i];
		if (tun && (tun->dev == dev)) {
			ppe_tun_ref(tun);
			spin_unlock_bh(&ptp->lock);
			return tun;
		}
	}
	spin_unlock_bh(&ptp->lock);

	return NULL;
}

/*
 * ppe_tun_stats()
 *	Update the netdevice stats
 */
bool ppe_tun_stats(struct net_device *dev, ppe_vp_hw_stats_t *stats)
{
	struct ppe_tun *tun;
	bool ret;

	tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	if (!tun) {
		ppe_tun_trace("%p: ppe tun stats update failed: tunnel not found", tun);
		return false;
	}

	if (!tun->stats_excp) {
		ppe_tun_trace("%p: tunnel stats callback not present", tun);
		ret = false;
		goto done;
	}

	ret = tun->stats_excp(dev, stats, tun->tun_data);

done:
	ppe_tun_deref(tun);
	return ret;
}

/*
 * ppe_tun_get_tun_by_vp_num_and_ref()
 *	Get tunnel by vp_num
 */
static struct ppe_tun *ppe_tun_get_tun_by_vp_num_and_ref(ppe_vp_num_t vp_num)
{
	struct ppe_tun *tun;
	int i = 0;

	spin_lock_bh(&ptp->lock);
	for (i = 0; i < PPE_TUN_MAX; i++) {
		tun = ptp->tun[i];
		if (tun != NULL) {
			if (tun->vp_num == vp_num) {
				ppe_tun_ref(tun);
				spin_unlock_bh(&ptp->lock);
				return tun;
			}
		}
	}
	spin_unlock_bh(&ptp->lock);

	ppe_tun_warn("%p: No VP instance for vp_num %u", ptp, vp_num);
	return NULL;
}

/*
 * ppe_tun_deactivate_with_conn_entry()
 *	Deactivate the ppe tunnel
 */
static bool ppe_tun_deactivate_with_conn_entry(uint8_t vp_num, void *vdestroy_rule)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_vp_num_and_ref(vp_num);
	if (!tun) {
		return false;
	}

	if (!ppe_drv_tun_deactivate(tun->vp_num, vdestroy_rule)) {
		ppe_tun_warn("%p: tunnel %p could not be deactivated", ptp, tun);
		ppe_tun_deref(tun);
		return false;
	}

	ppe_tun_deref(tun);

	/*
	 * deref the ref taken during tunnel activation.
	 */
	ppe_tun_deref(tun);

	ppe_tun_info("%p: VP %u tunnel deactivated", tun, vp_num);
	return true;
}

/*
 * ppe_tun_activate_with_conn_entry()
 *	Activate the ppe tunnel
 */
static bool ppe_tun_activate_with_conn_entry(uint8_t vp_num, void *create_rule)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_vp_num_and_ref(vp_num);
	if (!tun) {
		return false;
	}

	if (!ppe_drv_tun_activate(tun->vp_num, create_rule)) {
		ppe_tun_warn("%p: tunnel could not be activated", tun);
		ppe_tun_deref(tun);
		return false;
	}

	tun->phys_dev = ppe_drv_port_get_vp_phys_dev(tun->dev);

	/*
	 * We should not deref tun here. we should hold ref on tun till deactivate.
	 */
	ppe_tun_info("%p: VP %u tunnel activated", tun, vp_num);
	return true;
}

/*
 * ppe_tun_exception_dest_cb
 *	Callback handler for destination exception packets.
 */
static bool ppe_tun_exception_dest_cb(struct ppe_vp_cb_info *info, void *cb_data)
{
	struct sk_buff *skb = info->skb;
	struct net_device *dev = skb->dev;
	ppe_tun_exception_method_t cb;
	struct ppe_tun *tun;

	tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	if (!tun) {
		ppe_tun_warn("%p: tunnel not found for dev %s", ptp, dev->name);
		goto free_skb;
	}

	atomic64_inc(&tun->exception_packet);
	atomic64_add(skb->len, &tun->exception_bytes);

	cb = tun->dest_excp;
	if (!cb) {
		ppe_tun_warn("%p: No registered callback for destination exception %s", tun, dev->name);
		ppe_tun_deref(tun);
		goto free_skb;
	}

	cb(info, tun->tun_data);
	ppe_tun_deref(tun);

	return true;

free_skb:
	dev_kfree_skb_any(skb);
	return false;
}

/*
 * ppe_tun_exception_src_cb
 *	Callback handler for src exception packets.
 */
static bool ppe_tun_exception_src_cb(struct ppe_vp_cb_info *info, void *cb_data)
{
	struct sk_buff *skb = info->skb;
	struct net_device *dev = skb->dev;
	ppe_tun_exception_method_t cb;
	struct ppe_tun *tun;

	tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	if (!tun) {
		ppe_tun_warn("%p: tunnel not found for dev %s", ptp, dev->name);
		goto free_skb;
	}

	atomic64_inc(&tun->exception_packet);
	atomic64_add(skb->len, &tun->exception_bytes);

	cb = tun->src_excp;
	if (!cb) {
		ppe_tun_deref(tun);
		ppe_tun_warn("%p: No registered callback for source exception %s", tun, dev->name);
		goto free_skb;
	}

	/*
	 * Map-T is a special case where during exception,
	 * packet comes with tunnel dev instead of physical dev.
	 */
	if (tun->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		dev = tun->phys_dev;
		if (unlikely(!dev)) {
			ppe_tun_deref(tun);
			goto free_skb;
		}

		skb->dev = dev;
		skb->skb_iif = dev->ifindex;
	}

	cb(info, tun->tun_data);
	ppe_tun_deref(tun);

	return true;

free_skb:
	dev_kfree_skb_any(skb);
	return false;
}

/*
 * ppe_tun_mtu_get()
 *	Get a tunnel mtu
 */
uint32_t ppe_tun_mtu_get(struct net_device *dev)
{
	uint16_t mtu;
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);

	if (!tun) {
		ppe_tun_warn("%p: Tunnel not found for dev %s", tun, dev->name);
		return false;
	}

	if (ppe_vp_mtu_get(tun->vp_num, &mtu) != PPE_VP_STATUS_SUCCESS) {
		ppe_tun_warn("%p: MTU Failed to get for dev %s", tun, dev->name);
		ppe_tun_deref(tun);
		return false;
	}

	ppe_tun_deref(tun);
	return mtu;
}
EXPORT_SYMBOL(ppe_tun_mtu_get);

/*
 * ppe_tun_mtu_set()
 *	Set a tunnel mtu
 */
bool ppe_tun_mtu_set(struct net_device *dev, uint32_t mtu)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	ppe_vp_num_t vp_num;
	bool status = false;

	if (!tun) {
		ppe_tun_warn("%p: Tunnel not found for dev %s", ptp, dev->name);
		return status;
	}

	vp_num = tun->vp_num;

	status = ppe_vp_mtu_set(vp_num, mtu);
	ppe_tun_deref(tun);
	return status;
}
EXPORT_SYMBOL(ppe_tun_mtu_set);

/*
 * ppe_tun_exception_packet_get()
 *	Get the number of exception packets and bytes the netdevice has seen.
 */
bool ppe_tun_exception_packet_get(struct net_device *dev, uint64_t *num_pkt, uint64_t *num_bytes)
{
	struct ppe_tun *tun;
	int i = 0;

	spin_lock_bh(&ptp->lock);
	for (i = 0; i < PPE_TUN_MAX; i++) {
		tun = ptp->tun[i];
		if (tun && (tun->dev == dev)) {
			*num_pkt = atomic64_read(&tun->exception_packet);
			*num_bytes = atomic64_read(&tun->exception_bytes);
			spin_unlock_bh(&ptp->lock);
			return true;
		}
	}
	spin_unlock_bh(&ptp->lock);

	return false;
}
EXPORT_SYMBOL(ppe_tun_exception_packet_get);

/*
 * ppe_tun_get_active_tun_cnt()
 *	Get the active number of struct ppe_tun
 */
uint32_t ppe_tun_get_active_tun_cnt(void)
{
	uint16_t count;

	count = PPE_TUN_MAX - atomic_read(&ptp->total_free);
	return count;
}
EXPORT_SYMBOL(ppe_tun_get_active_tun_cnt);

/*
 * ppe_tun_deactivate()
 *	Deactivate the struct ppe_tun
 */
bool ppe_tun_deactivate(struct net_device *dev)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);

	if (!tun) {
		return false;
	}

	if (!ppe_drv_tun_deactivate(tun->vp_num, NULL)) {
		ppe_tun_deref(tun);
		ppe_tun_warn("%p: failed to deactivate tunnel for dev %s", tun, dev->name);
		return false;
	}

	ppe_tun_deref(tun);

	/*
	 * This additional deref is for the activate reference
	 */
	ppe_tun_deref(tun);

	ppe_tun_info("%p: Tunnel deactivated", tun);
	return true;
}
EXPORT_SYMBOL(ppe_tun_deactivate);

/*
 * ppe_tun_activate()
 *	Activate the struct ppe_tun
 */
bool ppe_tun_activate(struct net_device *dev)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);

	if (!tun) {
		ppe_tun_warn("%p: tunnel not found for dev %s", ptp, dev->name);
		return false;
	}

	if (!ppe_drv_tun_activate(tun->vp_num, NULL)) {
		ppe_tun_warn("%p: tunnel %p activate failed for dev %s", ptp, tun, dev->name);
		ppe_tun_deref(tun);
		return false;
	}

	tun->phys_dev = ppe_drv_port_get_vp_phys_dev(tun->dev);

	ppe_tun_warn("%p: tunnel %p activated for dev %s", ptp, tun, dev->name);

	/*
	 * We should not deref tun here. we should hold ref on tun till deactivate.
	 */
	return true;
}
EXPORT_SYMBOL(ppe_tun_activate);

/*
 * ppe_tun_conf_accel()
 *	Enable / Disable acceleration for a tunnel type
 */
bool ppe_tun_conf_accel(enum ppe_drv_tun_cmn_ctx_type type, bool action)
{
	struct ppe_tun *tun;
	int i;

	/*
	 * Allow setting tunnel accel mode to disabled only if ppe_tun of
	 * netdev is deconfigured.
	 */
	for (i = 0; (action == PPE_TUN_DISABLE) && (i < PPE_TUN_MAX); i++) {
		spin_lock_bh(&ptp->lock);
		tun = ptp->tun[i];
		if (!tun) {
			spin_unlock_bh(&ptp->lock);
			continue;
		}

		spin_unlock_bh(&ptp->lock);

		if (tun->state & PPE_TUN_STATE_CONFIGURED) {
			printk("dev %s should be down before disabling acceleration\n", tun->dev->name);
			return false;
		}

		atomic_inc(&ptp->free_pending);

		/*
		 * Release the reference taken during kref init
		 */
		ppe_tun_deref(tun);
	}

	switch (type) {
	case PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP:
		ptp->tun_accel.ppe_tun_gretap_accel = action;
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN:
		ptp->tun_accel.ppe_tun_vxlan_accel = action;
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6:
		ptp->tun_accel.ppe_tun_ipip6_accel = action;
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_MAPT:
		ptp->tun_accel.ppe_tun_mapt_accel = action;
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2:
		ptp->tun_accel.ppe_tun_l2tp_accel = action;
		break;

	default:
		ppe_tun_info("%p: Tunnel type %u is invalid", ptp, type);
		return false;
	}

	ppe_tun_info("%p: Acceleration %s for tunnel type %u", ptp, (action) ? ("enabled") : ("disabled"), type);
	return true;
}
EXPORT_SYMBOL(ppe_tun_conf_accel);

/*
 * ppe_tun_deconfigure()
 *	Disable the tunnel
 */
bool ppe_tun_deconfigure(struct net_device *dev)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);

	if (!tun) {
		ppe_tun_warn("%p: could not find tunnel for dev %s", ptp, dev->name);
		return false;
	}

	if (!ppe_drv_tun_deconfigure(tun->vp_num)) {
		ppe_tun_warn("%p: failed to disable tunnel for dev %s", tun, dev->name);
		ppe_tun_deref(tun);
		return false;
	}

	xchg(&tun->src_excp, NULL);
	xchg(&tun->dest_excp, NULL);
	xchg(&tun->stats_excp, NULL);
	xchg(&tun->phys_dev, NULL);

	tun->state &= ~PPE_TUN_STATE_CONFIGURED;
	ppe_tun_info("%p: Tunnel disabled for dev %s", tun, dev->name);
	ppe_tun_deref(tun);
	return true;
}
EXPORT_SYMBOL(ppe_tun_deconfigure);

/*
 * ppe_tun_configure()
 *	Configure a struct ppe_tun
 */
bool ppe_tun_configure(struct net_device *dev, struct ppe_drv_tun_cmn_ctx *tun_hdr, struct ppe_tun_excp *tun_cb)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	ppe_vp_num_t vp_num;
	bool status;

	if (!tun) {
		ppe_tun_warn("%p: tunnel not found for dev %s", ptp, dev->name);
		return false;
	}

	vp_num = tun->vp_num;

	if (tun_cb->tun_data && !ppe_tun_set_tun_data(tun, tun_cb->tun_data)) {
		ppe_tun_trace("%p: tunnel info set failed", tun);
		ppe_tun_deref(tun);
		return false;
	}

	status = ppe_drv_tun_configure(vp_num, tun_hdr, ppe_tun_activate_with_conn_entry,
				       ppe_tun_deactivate_with_conn_entry);
	if (!status) {
		ppe_tun_trace("%p: tunnel driver configuration failed", tun);
		ppe_tun_deref(tun);
		return false;
	}

	tun->src_excp = tun_cb->src_excp_method;
	tun->dest_excp = tun_cb->dest_excp_method;
	tun->stats_excp = tun_cb->stats_update_method;


	tun->state |= PPE_TUN_STATE_CONFIGURED;
	ppe_tun_info("%p: Tunnel is configured at idx:%d", tun, tun->idx);
	ppe_tun_deref(tun);
	return true;
}
EXPORT_SYMBOL(ppe_tun_configure);

/*
 * ppe_tun_free()
 *	Free a struct ppe_tun
 */
bool ppe_tun_free(struct net_device *dev)
{
	struct ppe_tun *tun = ppe_tun_get_tun_by_netdev_and_ref(dev);

	if (!tun) {
		ppe_tun_info("%p: could not find tunnel for dev %s", ptp, dev->name);
		return false;
	}

	atomic_inc(&ptp->free_pending);

	ppe_tun_info("%p: tunnel is freed for dev %s", tun, dev->name);
	ppe_tun_deref(tun);

	/*
	 * This additional deref is for taken during kref_init
	 */
	ppe_tun_deref(tun);
	return true;
}
EXPORT_SYMBOL(ppe_tun_free);

/*
 * ppe_tun_xcpn_mode_get()
 *	Get the exception mode based on tunnel type.
 */
uint8_t ppe_tun_xcpn_mode_get(enum ppe_drv_tun_cmn_ctx_type type)
{
	uint8_t action = PPE_TUN_XCPN_MODE_0;

	switch (type) {
	case PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP:
		action = ptp->xcpn_mode.gretap;
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6:
		action = ptp->xcpn_mode.ipip6;
		break;

	case PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2:
		/*
		 * exception mode is PPE_TUN_XCPN_MODE_1 by default
		 * for L2TP
		 */
		action = ptp->xcpn_mode.l2tp;
		break;

	default:
		ppe_tun_info("Tunnel type %u is invalid or doesn't support xcpn mode.", type);

	}

	return action;
}

/*
 * ppe_tun_alloc()
 *	Allocate a struct ppe_tun.
 */
bool ppe_tun_alloc(struct net_device *dev, enum ppe_drv_tun_cmn_ctx_type type)
{
	struct ppe_tun *tun;
	struct ppe_vp_ai vpai = {0};
	int32_t idx;
	ppe_vp_num_t vp_num;
	uint8_t action;

	if (!atomic_read(&ptp->total_free)) {
		ppe_tun_info("%p: Max tunnel %u limit reached", ptp, PPE_TUN_MAX);
		return false;
	}

	/*
	 * Check if acceleration is allowed for this tunnel type
	 */
	if (!ppe_tun_allow_accel(type)) {
		return false;
	}

	ppe_tun_info("%p: PPE Tun Allocating dev:%p type:%d", ptp, dev, type);

	tun = kzalloc(sizeof(struct ppe_tun), GFP_ATOMIC);
	if (!tun) {
		atomic_inc(&ptp->alloc_fail);
		ppe_tun_warn("%p: ppe_tun alloc failed for dev %s", ptp, dev->name);
		return false;
	}

	/*
	 * Allocate PPE VP
	 */
	if ((type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP) || (type == PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN)) {
		vpai.type = PPE_VP_TYPE_HW_L2TUN;
	} else if ((type == PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6) || (type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) ||
					(type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2)) {
		vpai.type = PPE_VP_TYPE_HW_L3TUN;
	} else {
		ppe_tun_warn("%p: tunnel type %u is invalid", dev, type);
		atomic_inc(&ptp->alloc_fail);
		kfree(tun);
		return false;
	}

	vpai.dst_cb = &ppe_tun_exception_dest_cb;
	vpai.src_cb = &ppe_tun_exception_src_cb;
	vpai.dst_cb_data = NULL;
	vpai.src_cb_data = NULL;
	vpai.stats_cb = ppe_tun_stats;

	/*
	 * For Tunnel VP the xmit port is determined during outer rule push. hence
	 * setting it as invalid
	 */
	vpai.xmit_port = PPE_DRV_PORT_ID_INVALID;

	vp_num = ppe_vp_alloc(dev, &vpai);
	if (vp_num == -1) {
		ppe_tun_warn("%p: vp alloc failed for dev %s", ptp, dev->name);
		atomic_inc(&ptp->alloc_fail);
		kfree(tun);
		return false;
	}

	/*
	 * Set exception mode.
	 */
	action = ppe_tun_xcpn_mode_get(type);
	if (!ppe_drv_port_xcpn_mode_set(vp_num, action)) {
		ppe_tun_warn("%p: xcpn_mode set failed for dev %s", ptp, dev->name);
		atomic_inc(&ptp->alloc_fail);
		ppe_vp_free(vp_num);
		kfree(tun);
		return false;
	}

	/*
	 * Use the vp_num to index
	 */
	idx = PPE_TUN_VP_NUM_TO_IDX(vp_num);

	spin_lock_bh(&ptp->lock);
	if (ptp->tun[idx] != NULL) {
		spin_unlock_bh(&ptp->lock);
		ppe_tun_assert(false, "%p: Tunnel %p at index %u already allocated", dev, ptp->tun[idx], idx);
		ppe_vp_free(vp_num);
		kfree(tun);
		return false;
	}

	tun->dev = dev;
	tun->type = type;
	tun->vp_num = vp_num;
	tun->idx = idx;

	if (type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2) {
		tun->tun_data = ppe_tun_alloc_tun_data();
		if (!tun->tun_data) {
			spin_unlock_bh(&ptp->lock);
			ppe_vp_free(vp_num);
			kfree(tun);
			return false;
		}
	}

	kref_init(&tun->ref);

	ptp->tun[idx] = tun;
	spin_unlock_bh(&ptp->lock);

	atomic_dec(&ptp->total_free);

	ppe_tun_info("%p: tunnel allocated for dev: %s idx: %u vp: %u", tun, dev->name, tun->idx, tun->vp_num);

	return true;
}
EXPORT_SYMBOL(ppe_tun_alloc);

/*
 * ppe_tun_setup()
 *	Alloc and Activate the tunnel
 */
bool ppe_tun_setup(struct net_device *dev, struct ppe_drv_tun_cmn_ctx *tun_hdr)
{
	struct ppe_tun_excp tun_cb = {0};
	bool status;

	status = ppe_tun_alloc(dev, tun_hdr->type);
	if (!status) {
		ppe_tun_warn("%p: tunnel allocation failed", dev);
		return false;
	}

	status = ppe_tun_configure(dev, tun_hdr, &tun_cb);
	if (!status) {
		ppe_tun_free(dev);
		ppe_tun_warn("%p: tunnel configuration failed", dev);
		return false;
	}

	status = ppe_tun_activate(dev);
	if (!status) {
		ppe_tun_deconfigure(dev);
		ppe_tun_free(dev);
		ppe_tun_warn("%p: tunnel activation failed", dev);
		return false;
	}

	ppe_tun_info("%p: tunnel setup success", dev);
	return true;
}
EXPORT_SYMBOL(ppe_tun_setup);

/*
 * ppe_tun_decap_disable()
 *	Disable tunnel decapsulation
 */
bool ppe_tun_decap_disable(struct net_device *dev)
{
	struct ppe_tun *tun;
	bool ret;

	tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	if (!tun) {
		ppe_tun_assert(0, "%p: Failed disabling decap for %s", tun, dev->name);
		return false;
	}

	ret = ppe_drv_tun_decap_disable_by_port_num(tun->vp_num);
	if (!ret) {
		ppe_tun_warn("%p, Failed to disable the decap for %s", tun, dev->name);
	}

	ppe_tun_trace("%px: Successfully enabled decap for nss_dev %s", tun, dev->name);
	ppe_tun_deref(tun);
	return ret;
}
EXPORT_SYMBOL(ppe_tun_decap_disable);

/*
 * ppe_tun_decap_enable()
 *	Enable tunnel decapsulation
 */
bool ppe_tun_decap_enable(struct net_device *dev)
{
	struct ppe_tun *tun;
	bool ret;

	tun = ppe_tun_get_tun_by_netdev_and_ref(dev);
	if (!tun) {
		ppe_tun_assert(0, "%p: Failed disabling decap for %s", tun, dev->name);
		return false;
	}

	ret = ppe_drv_tun_decap_enable_by_port_num(tun->vp_num);
	if (!ret) {
		ppe_tun_warn("%p, Failed to disable the decap for %s", tun, dev->name);
	}

	ppe_tun_trace("%px: Successfully enabled decap for nss_dev %s", tun, dev->name);
	ppe_tun_deref(tun);
	return ret;
}
EXPORT_SYMBOL(ppe_tun_decap_enable);

/*
 * ppe_tun_configure_vxlan_dport()
 *	Configure the VXLAN destination port
 */
bool ppe_tun_configure_vxlan_dport(uint16_t dport)
{
	ppe_tun_trace("Configuring the destination port of VXLAN dport: %u", dport);
	return ppe_drv_tun_configure_vxlan_and_dport(dport);
}
EXPORT_SYMBOL(ppe_tun_configure_vxlan_dport);

/*
 * ppe_tun_l2tp_port_set()
 *      Set L2TP source and destination  port
 */
bool ppe_tun_l2tp_port_set(uint16_t sport, uint16_t dport)
{
	ppe_tun_trace("Set L2TP sport: %u, dport: %u", sport, dport);
	return ppe_drv_tun_l2tp_port_set(sport, dport);
}
EXPORT_SYMBOL(ppe_tun_l2tp_port_set);

/*
 * ppe_tun_l2tp_port_get()
 *      get L2TP source and destination port
 */
bool ppe_tun_l2tp_port_get(uint16_t *sport, uint16_t *dport)
{
	return ppe_drv_tun_l2tp_port_get(sport, dport);
}
EXPORT_SYMBOL(ppe_tun_l2tp_port_get);

/*
 * ppe_tun_l2tp_read()
 *	l2tp read handler
 */
static ssize_t ppe_tun_l2tp_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];

	len = snprintf(lbuf, sizeof(lbuf), "l2tp accel %s\n", (ptp->tun_accel.ppe_tun_l2tp_accel) ? ("enabled") : ("disabled"));

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_l2tp_write()
 *	l2tp write handler
 */
static ssize_t ppe_tun_l2tp_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for l2tp configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for l2tp configuration", ptp);
		return status;
	}

	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2, res);

	return len;
}

const struct file_operations ppe_tun_l2tp_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_l2tp_write,
	.read = ppe_tun_l2tp_read,
};

/*
 * ppe_tun_gretap_read()
 *	gretap read handler
 */
static ssize_t ppe_tun_gretap_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];

	len = snprintf(lbuf, sizeof(lbuf), "gretap accel %s\n", (ptp->tun_accel.ppe_tun_gretap_accel) ? ("enabled") : ("disabled"));

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_gretap_write()
 *	gretap write handler
 */
static ssize_t ppe_tun_gretap_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for gretap configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for gretap configuration", ptp);
		return status;
	}

	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP, res);

	return len;
}

const struct file_operations ppe_tun_gretap_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_gretap_write,
	.read = ppe_tun_gretap_read,
};

/*
 * ppe_tun_vxlan_read()
 *	vxlan read handler
 */
static ssize_t ppe_tun_vxlan_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];

	len = snprintf(lbuf, sizeof(lbuf), "vxlan accel %s\n", (ptp->tun_accel.ppe_tun_vxlan_accel) ? ("enabled") : ("disabled"));

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_vxlan_write()
 *	vxlan write handler
 */
static ssize_t ppe_tun_vxlan_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for vxlan configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for vxlan configuration", ptp);
		return status;
	}

	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN, res);

	return len;
}

const struct file_operations ppe_tun_vxlan_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_vxlan_write,
	.read = ppe_tun_vxlan_read,
};

/*
 * ppe_tun_mapt_read()
 *	mapt read handler
 */
static ssize_t ppe_tun_mapt_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];

	len = snprintf(lbuf, sizeof(lbuf), "mapt accel %s\n", (ptp->tun_accel.ppe_tun_mapt_accel) ? ("enabled") : ("disabled"));

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_mapt_write()
 *	mapt write handler
 */
static ssize_t ppe_tun_mapt_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for mapt configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for mapt configuration", ptp);
		return status;
	}

	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_MAPT, res);

	return len;
}

const struct file_operations ppe_tun_mapt_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_mapt_write,
	.read = ppe_tun_mapt_read,
};

/*
 * ppe_tun_ipip6_read()
 *	ipip6 read handler
 */
static ssize_t ppe_tun_ipip6_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];

	len = snprintf(lbuf, sizeof(lbuf), "ipip6 accel %s\n", (ptp->tun_accel.ppe_tun_ipip6_accel) ? ("enabled") : ("disabled"));

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_ipip6_write()
 *	ipip6 write handler
 */
static ssize_t ppe_tun_ipip6_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for ipip6 configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for ipip6 configuration", ptp);
		return status;
	}

	ppe_tun_conf_accel(PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6, res);

	return len;
}

const struct file_operations ppe_tun_ipip6_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_ipip6_write,
	.read = ppe_tun_ipip6_read,
};

/*
 * ppe_tun_stats_show()
 *	Read ppe tunnel statistics.
 */
static int ppe_tun_stats_show(struct seq_file *m, void __attribute__((unused))*ptr)
{
	struct ppe_tun *tun;
	int i = 0;

	seq_puts(m, "\n################ PPE Tunnel Statistics Start ################\n");
	seq_puts(m, "PPE tunnel driver stats\n");
	seq_printf(m, "\tMax tunnels: %u\n", PPE_TUN_MAX);
	seq_printf(m, "\tActive tunnels: %u\n", (PPE_TUN_MAX - atomic_read(&ptp->total_free)));
	seq_printf(m, "\tDelete pending: %u\n", atomic_read(&ptp->free_pending));
	seq_printf(m, "\tAlloc fail: %u\n", atomic_read(&ptp->alloc_fail));
	seq_puts(m, "\n");

	seq_puts(m, "PPE tunnel stats\n");

	spin_lock_bh(&ptp->lock);
	for (i = 0; i < PPE_TUN_MAX; i++) {
		tun = ptp->tun[i];
		if (tun) {
			seq_printf(m, "dev: %s\n", tun->dev->name);
			seq_printf(m, "\t index %d\n", tun->idx);
			seq_printf(m, "\t VP number %d\n", tun->vp_num);
			seq_printf(m, "\t type %u\n", tun->type);
			seq_printf(m, "\t physical dev: %s\n", (tun->phys_dev) ? (tun->phys_dev->name) : (""));
			seq_printf(m, "\t exception packet: %llu\n", atomic64_read(&tun->exception_packet));
			seq_printf(m, "\t exception bytes: %llu\n", atomic64_read(&tun->exception_bytes));
			seq_puts(m, "\n");
		}
	}
	spin_unlock_bh(&ptp->lock);

	seq_puts(m, "\n################ PPE Tunnel Statistics End  ################\n");

	return 0;
}

/*
 * ppe_tun_stats_open()
 */
static int ppe_tun_stats_open(struct inode *inode, struct file *file)
{
	return single_open(file, ppe_tun_stats_show, inode->i_private);
}

/*
 * nss_ppe_tunipip6_stats_ops
 *	File operations for PPE tunnel driver stats
 */
static const struct file_operations ppe_tun_stats_ops = {
	.open = ppe_tun_stats_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = seq_release
};

/*
 * ppe_tun_xcpn_gretap_read()
 *	gretap xcpn read handler
 */
static ssize_t ppe_tun_xcpn_gretap_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];
	uint8_t xcpn_mode = ptp->xcpn_mode.gretap;

	len = snprintf(lbuf, sizeof(lbuf), "Gretap xcpn mode %u \n", xcpn_mode);

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_xcpn_gretap_write()
 *	gretap xcpn write handler
 */
static ssize_t ppe_tun_xcpn_gretap_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for gretap configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for gretap configuration", ptp);
		return status;
	}

	ptp->xcpn_mode.gretap = (uint8_t) res;

	return len;
}

const struct file_operations ppe_tun_gretap_xcpn_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_xcpn_gretap_write,
	.read = ppe_tun_xcpn_gretap_read,
};

/*
 * ppe_tun_xcpn_ipip6_read()
 *	ipip6 xcpn read handler
 */
static ssize_t ppe_tun_xcpn_ipip6_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];
	uint8_t xcpn_mode = ptp->xcpn_mode.ipip6;

	len = snprintf(lbuf, sizeof(lbuf), "IPIP6 xcpn mode %u \n", xcpn_mode);

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_xcpn_ipip6_write()
 *	ipip6 xcpn write handler
 */
static ssize_t ppe_tun_xcpn_ipip6_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for ipip6 configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for ipip6 configuration", ptp);
		return status;
	}

	ptp->xcpn_mode.ipip6 = (uint8_t) res;

	return len;
}

const struct file_operations ppe_tun_ipip6_xcpn_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_xcpn_ipip6_write,
	.read = ppe_tun_xcpn_ipip6_read,
};

/*
 * ppe_tun_xcpn_l2tp_read()
 *	l2tp xcpn read handler
 */
static ssize_t ppe_tun_xcpn_l2tp_read(struct file *f, char *buf, size_t count, loff_t *offset)
{
	int len;
	char lbuf[24];
	uint8_t xcpn_mode = ptp->xcpn_mode.l2tp;

	len = snprintf(lbuf, sizeof(lbuf), "L2TP xcpn mode %u \n", xcpn_mode);

	return simple_read_from_buffer(buf, count, offset, lbuf, len);
}

/*
 * ppe_tun_xcpn_l2tp_write()
 *	l2tp xcpn write handler
 */
static ssize_t ppe_tun_xcpn_l2tp_write(struct file *f, const char *buffer, size_t len, loff_t *offset)
{
	ssize_t size;
	char data[16];
	bool res;
	int status;

	size = simple_write_to_buffer(data, sizeof(data), offset, buffer, len);
	if (size < 0) {
		ppe_tun_warn("%p: Error reading the input for l2tp configuration", ptp);
		return size;
	}

	status = kstrtobool(data, &res);
	if (status) {
		ppe_tun_warn("%p: Error reading the input for l2tp configuration", ptp);
		return status;
	}

	ptp->xcpn_mode.l2tp = (uint8_t) res;

	return len;
}

const struct file_operations ppe_tun_l2tp_xcpn_file_fops = {
	.owner = THIS_MODULE,
	.write = ppe_tun_xcpn_l2tp_write,
	.read = ppe_tun_xcpn_l2tp_read,
};

/*
 * ppe_tun_module_init()
 *	module init for ppe tunnel driver
 */
static int __init ppe_tun_module_init(void)
{
	struct dentry *dir;
	ppe_acl_ret_t ret;
	struct ppe_acl_rule rule = {0};

	ptp = kzalloc(sizeof(struct ppe_tun_priv), GFP_ATOMIC);
	if (!ptp) {
		ppe_tun_warn("memory allocation for ptp failed");
		return -ENOMEM;
	}

	/*
	 * Initialize base lock.
	 */
	spin_lock_init(&ptp->lock);

	ptp->tun_accel.ppe_tun_gretap_accel = true;
	ptp->tun_accel.ppe_tun_vxlan_accel = true;
	ptp->tun_accel.ppe_tun_ipip6_accel = true;
	ptp->tun_accel.ppe_tun_mapt_accel = true;
	ptp->tun_accel.ppe_tun_l2tp_accel = true;

	ptp->xcpn_mode.gretap = PPE_TUN_XCPN_MODE_1;
	ptp->xcpn_mode.ipip6 = PPE_TUN_XCPN_MODE_1;
	ptp->xcpn_mode.l2tp = PPE_TUN_XCPN_MODE_1;


	atomic_set(&ptp->total_free, PPE_TUN_MAX);

	dir = debugfs_lookup("qca-nss-ppe", NULL);
	if (!dir) {
		ppe_tun_warn("%p: Unable to get parent debugfs entry", ptp);
		goto fail;
	}

	ptp->dentry = debugfs_create_dir("ppe_tun", dir);
	if (!ptp->dentry) {
		ppe_tun_warn("%p: Failed to create debugfs entry for ppe-tun", ptp);
		goto fail;
	}

	dir = debugfs_create_file("stats", S_IRUGO, ptp->dentry, NULL, &ppe_tun_stats_ops);
	if (!dir) {
		ppe_tun_warn("%p: Failed to create debugfs entry for stats", ptp);
		goto fail;
	}

	dir = debugfs_create_dir("accel_mode", ptp->dentry);
	if (!dir) {
		ppe_tun_warn("%p: Failed to create debugfs entry for accel_mode", ptp);
		goto fail;
	}

	/*
	 * Acceleration Operation Nodes
	 */
	if (!debugfs_create_file("gretap", 0644, dir, NULL, &ppe_tun_gretap_file_fops)) {
		ppe_tun_warn("Failed to create debugfs entry for gretap");
	}
	if (!debugfs_create_file("vxlan", 0644, dir, NULL, &ppe_tun_vxlan_file_fops)) {
		ppe_tun_warn("Failed to create debugfs entry for vxlan");
	}
	if (!debugfs_create_file("ipip6", 0644, dir, NULL, &ppe_tun_ipip6_file_fops)) {
		ppe_tun_warn("Failed to create debugfs entry for ipip6");
	}
	if (!debugfs_create_file("mapt", 0644, dir, NULL, &ppe_tun_mapt_file_fops)) {
		ppe_tun_warn("Failed to create debugfs entry for mapt");
	}
	if (!debugfs_create_file("l2tp", 0644, dir, NULL, &ppe_tun_l2tp_file_fops)) {
		ppe_tun_warn("Failed to create debugfs entry for l2tp");
	}

	dir = debugfs_create_dir("xcpn_mode", ptp->dentry);
	if (!dir) {
		ppe_tun_warn("%p: Failed to create debugfs entry for xcpn_mode", ptp);
		goto fail;
	}

	/*
	 * Exception Mode Operation Nodes
	 */
	if (!debugfs_create_file("gretap", 0644, dir, NULL, &ppe_tun_gretap_xcpn_file_fops)) {
		ppe_tun_warn("Failed to create debugfs entry for gretap");
	}
	if (!debugfs_create_file("ipip6", 0644, dir, NULL, &ppe_tun_ipip6_xcpn_file_fops)) {
		ppe_tun_warn("failed to create debugfs entry for ipip6");
	}

	if (!debugfs_create_file("l2tp", 0644, dir, NULL, &ppe_tun_l2tp_xcpn_file_fops)) {
		ppe_tun_warn("failed to create debugfs entry for l2tp");
	}

	rule.cmn.cmn_flags = rule.cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_NO_RULEID;
	rule.stype = PPE_ACL_RULE_SRC_TYPE_SC;
	rule.action.fwd_cmd = PPE_ACL_FWD_CMD_REDIR;
	rule.valid_flags = (1 << PPE_ACL_RULE_MATCH_TYPE_DEFAULT);
	rule.action.flags = PPE_ACL_RULE_ACTION_FLAG_FW_CMD;
	rule.src.sc = PPE_DRV_SC_L2_TUNNEL_EXCEPTION;
	ret = ppe_acl_rule_create(&rule);
	if (ret != PPE_ACL_RET_SUCCESS) {
		ppe_tun_warn("Failed to create ACL rule for VXLAN tunnels. error:%d", ret);
		goto fail;
	}

	ptp->ppe_tun_l2_tunnel_rule_id = rule.rule_id;

	ppe_tun_info("ppe tunnel driver initialized");
	return 0;

fail:
	debugfs_remove_recursive(ptp->dentry);

	kfree(ptp);
	ptp = NULL;

	ppe_tun_warn("ppe tunnel driver initialization failed");
	return -EIO;
}
module_init(ppe_tun_module_init);

/*
 * ppe_tun_module_exit()
 *	module exit for ppe tunnel driver
 */
static void __exit ppe_tun_module_exit(void)
{
	debugfs_remove_recursive(ptp->dentry);

	ppe_acl_rule_destroy(ptp->ppe_tun_l2_tunnel_rule_id);

	kfree(ptp);
}
module_exit(ppe_tun_module_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("NSS PPE tunnel driver");
