/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <linux/netdevice.h>
#include "ppe_drv.h"

/**
 * ppe_drv_pppoe_session_deinit
 *      Uninitialize pppoe session in PPE.
 */
ppe_drv_ret_t ppe_drv_pppoe_session_deinit(struct ppe_drv_iface *pppoe_iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_pppoe *pppoe;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(pppoe_iface->type == PPE_DRV_IFACE_TYPE_PPPOE,
			"%p: pppoe_iface should be pppoe but: %u", pppoe_iface, pppoe_iface->type);

	spin_lock_bh(&p->lock);

	/*
	 * Clear base iface for pppoe interface
	 */
	ppe_drv_iface_base_clear(pppoe_iface);

	/*
	 * Clear l3_if handle from Iface.
	 */
	l3_if = ppe_drv_iface_l3_if_get(pppoe_iface);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: L3_IF not attached to pppoe", pppoe_iface);
		return PPE_DRV_RET_IFACE_L3_IF_FAIL;
	}

	ppe_drv_trace("%p: PPPOE_PPE: clear l3_if: %p from iface",
			pppoe_iface, l3_if);

	pppoe = ppe_drv_l3_if_pppoe_get(l3_if);
	if (!pppoe) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: pppoe not attached to l3_if : %p", pppoe_iface, l3_if);
		return PPE_DRV_RET_L3_IF_PPPOE_FAIL;
	}

	ppe_drv_iface_l3_if_clear(pppoe_iface);

	/*
	 * Clear mac address on L3_IF
	 */
	ppe_drv_l3_if_mac_addr_clear(l3_if);

	/*
	 * Detach the egress L3 interface
	 */
	ppe_drv_trace("%p: PPPOE_PPE: Detach L3 interface pppoe: %p", pppoe_iface, pppoe);
	ppe_drv_pppoe_l3_if_detach(pppoe);

	/*
	 * Clear the PPPoE on the ingress L3 interface
	 */
	ppe_drv_trace("%p: PPPOE_PPE: Clear pppoe on L3 interface ppe_pppoe: %p", pppoe_iface, l3_if);
	ppe_drv_l3_if_pppoe_clear(l3_if);

	/*
	 * Dereference the ingress L3 interface
	 */
	ppe_drv_trace("%p: PPPOE_PPE: Dereference L3 interface: %p", pppoe_iface, l3_if);
	ppe_drv_l3_if_deref(l3_if);

	/*
	 * Dereference the PPPoE object created in PPE
	 */
	ppe_drv_trace("%p: PPPOE_PPE: Dereference PPPoE : %p", pppoe_iface, pppoe);
	ppe_drv_pppoe_deref(pppoe);

	ppe_drv_trace("%p: pppoe session unconfigured", pppoe_iface);

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_pppoe_session_deinit);

/**
 * ppe_drv_pppoe_session_init
 *      Initialize pppoe session in PPE.
 */
ppe_drv_ret_t ppe_drv_pppoe_session_init(struct ppe_drv_iface *pppoe_iface, struct net_device *base_dev,
		uint16_t session_id, uint8_t *server_mac, uint8_t *local_mac)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *base_iface;
	struct ppe_drv_pppoe *pppoe;
	struct ppe_drv_l3_if *l3_if;
	uint16_t mtu;

	ppe_drv_assert(pppoe_iface->type == PPE_DRV_IFACE_TYPE_PPPOE,
			"%p: pppoe_iface should be pppoe but: %u", pppoe_iface, pppoe_iface->type);

	spin_lock_bh(&p->lock);
	base_iface = ppe_drv_iface_get_by_dev_internal(base_dev);
	if (!base_iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: invalid base net_device %p", pppoe_iface, base_dev);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Create the PPPoE session in PPE
	 */
	pppoe = ppe_drv_pppoe_alloc(session_id, server_mac);
	if (!pppoe) {
		ppe_drv_warn("%p: Unable to create pppoe_object: for session :%u mac:%pM",
				pppoe_iface, session_id, server_mac);
		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_PPPOE_ALLOC_FAIL;
	}

	/*
	 * L3 interface of this PPPoE session creation
	 */
	l3_if = ppe_drv_l3_if_alloc(PPE_DRV_L3_IF_TYPE_PPPOE);
	if (!l3_if) {
		ppe_drv_pppoe_deref(pppoe);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Unable to create ppe L3 IF for pppoe session: %u mac: %pM",
				pppoe_iface, session_id, server_mac);
		return PPE_DRV_RET_L3_IF_ALLOC_FAIL;
	}

	/*
	 * MRU and MTU values are set for the interface
	 */
	mtu = pppoe_iface->dev->mtu;
	if (!ppe_drv_l3_if_mtu_mru_set(l3_if, mtu, mtu)) {
		ppe_drv_l3_if_deref(l3_if);
		ppe_drv_pppoe_deref(pppoe);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Unable to set MTU: %d on  L3 IF for pppoe session: %u mac: %pM",
				pppoe_iface, mtu, session_id, server_mac);
		return PPE_DRV_RET_MTU_CFG_FAIL;
	}

	/*
	 * Associate the PPPoE session with its L3 interface
	 */
	if (!ppe_drv_l3_if_pppoe_set(l3_if, pppoe)) {
		ppe_drv_l3_if_mtu_mru_clear(l3_if);
		ppe_drv_l3_if_deref(l3_if);
		ppe_drv_pppoe_deref(pppoe);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Unable to enable pppoe on L3 IF for pppoe session: %u mac: %pM",
				pppoe_iface, session_id, server_mac);
		return PPE_DRV_RET_L3_IF_PPPOE_SET_FAIL;
	}

	/*
	 * xmit mac for the interface
	 */
	if (!ppe_drv_l3_if_mac_addr_set(l3_if, local_mac)) {
		ppe_drv_l3_if_mtu_mru_clear(l3_if);
		ppe_drv_l3_if_deref(l3_if);
		ppe_drv_pppoe_deref(pppoe);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Unable to set MAC address for L3 IF: %p for pppoe session: %u mac: %pM",
				pppoe_iface, l3_if, session_id, server_mac);
		return PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
	}

	/*
	 * PPPoE sessions outbound L3 interface and attach
	 * to egress l3 interface
	 */
	ppe_drv_pppoe_l3_if_attach(pppoe, l3_if);

	/*
	 * Save l3_if in ppe_iface.
	 */
	ppe_drv_iface_l3_if_set(pppoe_iface, l3_if);

	/*
	 * Set base iface for pppoe interface
	 */
	ppe_drv_iface_base_set(pppoe_iface, base_iface);
	spin_unlock_bh(&p->lock);

	ppe_drv_trace("%p: pppoe session configured session_id: %d, server_mac: %pM local_mac: %pM mtu: %d",
			pppoe_iface, session_id, server_mac, local_mac, mtu);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_pppoe_session_init);
