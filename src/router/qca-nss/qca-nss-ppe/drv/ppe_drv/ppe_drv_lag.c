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

#include "ppe_drv.h"

/**
 * ppe_drv_lag_leave
 *	Remove a member from LAG interface in PPE.
 */
ppe_drv_ret_t ppe_drv_lag_leave(struct ppe_drv_iface *lag_iface, struct net_device *member)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *member_iface;
	struct ppe_drv_port *pp;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(lag_iface->type == PPE_DRV_IFACE_TYPE_LAG,
			"%p: lag_iface should be LAG but: %u", lag_iface, lag_iface->type);

	ppe_drv_info("%p: removing netdevice: %p from LAG", lag_iface, member);

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface || !(member_iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bad member net_device %p", lag_iface, member);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * detach port to LAG l3_if.
	 */
	l3_if = ppe_drv_iface_l3_if_get(lag_iface);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: L3_IF not assinged to LAG %p", lag_iface, member);
		return PPE_DRV_RET_IFACE_L3_IF_FAIL;
	}

	pp = ppe_drv_iface_port_get(member_iface);
	if (!pp) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: slave interface %p does not have a port in PPE",
				lag_iface, member);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Detach lag's l3_if from port
	 */
	if (pp->active_l3_if_attached)  {
		ppe_drv_port_l3_if_detach(pp, l3_if);
	}

	/*
	 * Attach port's l3_if after detaching lag's l3_if
	 */
	if (!pp->active_l3_if_attached) {
		ppe_drv_port_l3_if_attach(pp, pp->port_l3_if);
	}

	ppe_drv_iface_deref_internal(member_iface);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_lag_leave);

/**
 * ppe_drv_lag_join
 *	Add a member to LAG interface in PPE.
 */
ppe_drv_ret_t ppe_drv_lag_join(struct ppe_drv_iface *lag_iface, struct net_device *member)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *member_iface;
	struct ppe_drv_port *pp;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(lag_iface->type == PPE_DRV_IFACE_TYPE_LAG,
			"%p: lag_iface should be LAG but: %u", lag_iface, lag_iface->type);

	ppe_drv_info("%p: adding netdevice: %p to LAG", lag_iface, member);

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface || !(member_iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bad member net_device %p", lag_iface, member);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Attach port to LAG l3_if.
	 */
	l3_if = ppe_drv_iface_l3_if_get(lag_iface);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: L3_IF not assinged to LAG %p", lag_iface, member);
		return PPE_DRV_RET_IFACE_L3_IF_FAIL;
	}

	pp = ppe_drv_iface_port_get(member_iface);
	if (!pp) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: slave interface %p does not have a port in PPE",
				lag_iface, member);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Detach port's l3_if before attaching lag's l3_if
	 */
	if (pp->active_l3_if_attached) {
		ppe_drv_port_l3_if_detach(pp, pp->active_l3_if);
	}

	/*
	 * Atttach lag's l3_if
	 */
	ppe_drv_port_l3_if_attach(pp, l3_if);
	ppe_drv_iface_ref(member_iface);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_lag_join);

/**
 * ppe_drv_lag_deinit
 *	Uninitialize lag interface in PPE.
 */
ppe_drv_ret_t ppe_drv_lag_deinit(struct ppe_drv_iface *lag_iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(lag_iface->type == PPE_DRV_IFACE_TYPE_LAG,
			"%p: lag_iface should be LAG but: %u", lag_iface, lag_iface->type);

	spin_lock_bh(&p->lock);
	l3_if = ppe_drv_iface_l3_if_get(lag_iface);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: L3_IF not assigned", lag_iface);
		return PPE_DRV_RET_IFACE_L3_IF_FAIL;
	}

	ppe_drv_info("%p: LAG if unassign l3_if: %p", lag_iface, l3_if);

	/*
	 * Release references & clear ppe_iface
	 */
	ppe_drv_l3_if_deref(l3_if);
	ppe_drv_iface_l3_if_clear(lag_iface);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_lag_deinit);

/**
 * ppe_drv_lag_init
 *	Initialize LAG interface in PPE.
 */
ppe_drv_ret_t ppe_drv_lag_init(struct ppe_drv_iface *lag_iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(lag_iface->type == PPE_DRV_IFACE_TYPE_LAG,
			"%p: lag_iface should be LAG but: %u", lag_iface, lag_iface->type);

	spin_lock_bh(&p->lock);
	l3_if = ppe_drv_l3_if_alloc(PPE_DRV_L3_IF_TYPE_PORT);
	if (!l3_if) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: l3_if allocation failed for LAG iface: %p", p, lag_iface);
		return PPE_DRV_RET_L3_IF_ALLOC_FAIL;
	}

	/*
	 * Save l3_if in ppe_iface.
	 */
	ppe_drv_iface_l3_if_set(lag_iface, l3_if);
	ppe_drv_info("%p: LAG if assigned l3_if %p", lag_iface, l3_if);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_lag_init);
