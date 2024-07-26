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

#include "ppe_drv.h"
#include "ppe_drv_stats.h"

/*
 * ppe_drv_iface_free()
 *	FREE the interface entry in PPE.
 */
static void ppe_drv_iface_free(struct kref *kref)
{
	struct ppe_drv_iface *iface = container_of(kref, struct ppe_drv_iface, ref);

	ppe_drv_assert(!iface->port, "%p: Interface still associated with the port", iface);
	ppe_drv_assert(!iface->vsi, "%p: Interface still associated with the vsi", iface);
	ppe_drv_assert(!iface->l3, "%p: Interface still associated with the l3_if", iface);

	/*
	 * The cleanup_cb is registered when
	 * the references held by other components(like ECM).
	 * When the reference count of the ppe_iface reaches 0 and
	 * callback is registered, we perform the deinitialization/cleanup of
	 * the port before proceeding to clean up the ppe_iface.
	 * This is a WAR to handle async free call for ppe-vp.
	 */
	if (iface->cleanup_cb) {
		iface->cleanup_cb(iface);
		iface->cleanup_cb = NULL;
	}

	iface->flags &= ~PPE_DRV_IFACE_FLAG_VALID;

	iface->port = NULL;
	iface->vsi = NULL;
	iface->l3 = NULL;
	iface->type = PPE_DRV_IFACE_TYPE_INVALID;
	iface->dev = NULL;
	iface->parent = NULL;
}

/*
 * ppe_drv_iface_ref()
 *	Reference PPE interface
 */
struct ppe_drv_iface *ppe_drv_iface_ref(struct ppe_drv_iface *iface)
{
	kref_get(&iface->ref);
	return iface;
}

/*
 * ppe_drv_iface_deref_internal()
 *	Let go of reference on iface.
 */
bool ppe_drv_iface_deref_internal(struct ppe_drv_iface *iface)
{
	if (kref_put(&iface->ref, ppe_drv_iface_free)) {
		ppe_drv_trace("reference goes down to 0 for iface: %p\n", iface);
		return true;
	}

	return false;
}

/*
 * ppe_drv_iface_deref()
 *	Let go of reference on iface.
 */
bool ppe_drv_iface_deref(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	bool ret;

	spin_lock_bh(&p->lock);
	ret = ppe_drv_iface_deref_internal(iface);
	spin_unlock_bh(&p->lock);

	return ret;
}
EXPORT_SYMBOL(ppe_drv_iface_deref);

/*
 * ppe_drv_iface_get_by_idx()
 *	Get PPE interface by netdev
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_idx(ppe_drv_iface_t idx)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;

	if (idx < 0 || idx >= p->iface_num) {
		return NULL;
	}

	iface = &p->iface[idx];
	if (kref_read(&iface->ref) && ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) == PPE_DRV_IFACE_FLAG_VALID)) {
		return iface;
	}

	return NULL;
}
EXPORT_SYMBOL(ppe_drv_iface_get_by_idx);

/*
 * ppe_drv_iface_is_physical()
 *	Check if PPE interface is a physical interface
 */
bool ppe_drv_iface_is_physical(struct ppe_drv_iface *iface)
{
	return iface->type == PPE_DRV_IFACE_TYPE_PHYSICAL;
}
EXPORT_SYMBOL(ppe_drv_iface_is_physical);

/*
 * ppe_drv_dev_get_by_iface_idx
 *	Get Netdev by PPE interface index.
 */
struct net_device *ppe_drv_dev_get_by_iface_idx(ppe_drv_iface_t index)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	struct net_device *dev = NULL;

	spin_lock_bh(&p->lock);
	if (index < 0 || index >= p->iface_num) {
		ppe_drv_warn("Index out of bounds %d\n", index);
		goto error;
	}

	iface = &p->iface[index];
	if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
		ppe_drv_warn("Invalid iface index %d\n", index);
		goto error;
	}

	dev = iface->dev;
error:
	spin_unlock_bh(&p->lock);
	return dev;
}
EXPORT_SYMBOL(ppe_drv_dev_get_by_iface_idx);

/*
 * ppe_drv_iface_idx_get_by_dev()
 *	Get PPE interface by netdev
 */
ppe_drv_iface_t ppe_drv_iface_idx_get_by_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	int i, index = -1;

	spin_lock_bh(&p->lock);
	for (i = 0; i < p->iface_num; i++) {
		iface = &p->iface[i];
		if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
			continue;
		}

		if (iface->dev == dev) {
			index = iface->index;
			break;
		}
	}

	spin_unlock_bh(&p->lock);
	return index;
}
EXPORT_SYMBOL(ppe_drv_iface_idx_get_by_dev);

/*
 * ppe_drv_iface_get_by_dev()
 *	Get PPE interface by device
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_dev(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	int i, index = -1;

	spin_lock_bh(&p->lock);
	for (i = 0; i < p->iface_num; i++) {
		iface = &p->iface[i];
		if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
			continue;
		}

		if (iface->dev == dev) {
			index = iface->index;
			break;
		}
	}

	if (index < 0) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: No valid PPE interface found for the given dev", dev);
		return NULL;
	}

	iface = ppe_drv_iface_get_by_idx(index);
	spin_unlock_bh(&p->lock);
	return iface;
}
EXPORT_SYMBOL(ppe_drv_iface_get_by_dev);

/*
 * ppe_drv_iface_get_by_dev_internal()
 *	Get PPE interface by device
 */
struct ppe_drv_iface *ppe_drv_iface_get_by_dev_internal(struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface;
	int i, index = -1;

	for (i = 0; i < p->iface_num; i++) {
		iface = &p->iface[i];
		if (!kref_read(&iface->ref) || ((iface->flags & PPE_DRV_IFACE_FLAG_VALID) != PPE_DRV_IFACE_FLAG_VALID)) {
			continue;
		}

		if (iface->dev == dev) {
			index = iface->index;
			break;
		}
	}

	if (index < 0) {
		return NULL;
	}

	iface = ppe_drv_iface_get_by_idx(index);
	return iface;
}

/*
 * ppe_drv_iface_base_clear()
 *	Get base_if of a given PPE interface
 */
void ppe_drv_iface_base_clear(struct ppe_drv_iface *iface)
{
	if (iface->base_if) {
		ppe_drv_iface_deref_internal(iface->base_if);
		iface->base_if = NULL;
	}
}

/*
 * ppe_drv_iface_base_get()
 *	Get base_if of a given PPE interface
 */
struct ppe_drv_iface *ppe_drv_iface_base_get(struct ppe_drv_iface *iface)
{
	return iface->base_if;
}

/*
 * ppe_drv_iface_base_set()
 *	Set base_if of a given PPE interface
 */
void ppe_drv_iface_base_set(struct ppe_drv_iface *iface, struct ppe_drv_iface *base_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (iface->base_if) {
		ppe_drv_warn("%p: base_if already set for iface(%p)\n", p, iface);
		return;
	}

	if ((iface->type == PPE_DRV_IFACE_TYPE_BRIDGE) || (iface->type == PPE_DRV_IFACE_TYPE_LAG)) {
		ppe_drv_warn("%p: Base_if cannot be set for iface(%p)\n", p, iface);
		return;
	}

	iface->base_if = ppe_drv_iface_ref(base_if);
}

/*
 * ppe_drv_iface_parent_clear()
 *	Clear parent of a given PPE interface
 */
void ppe_drv_iface_parent_clear(struct ppe_drv_iface *iface)
{
	if (iface->parent) {
		ppe_drv_iface_deref_internal(iface->parent);
		iface->parent = NULL;
	}
}

/*
 * ppe_drv_iface_parent_get()
 *	Get parent of a given PPE interface
 */
struct ppe_drv_iface *ppe_drv_iface_parent_get(struct ppe_drv_iface *iface)
{
	return iface->parent;
}

/*
 * ppe_drv_iface_parent_set()
 *	Set parent of a given PPE interface
 */
bool ppe_drv_iface_parent_set(struct ppe_drv_iface *iface, struct ppe_drv_iface *parent)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (iface->parent) {
		ppe_drv_warn("%p: parent already set for iface(%p)\n", p, iface);
		return false;
	}

	if (parent->type != PPE_DRV_IFACE_TYPE_BRIDGE) {
		ppe_drv_warn("%p: Parent cannot be set for iface(%p)\n", p, iface);
		return false;
	}

	iface->parent = ppe_drv_iface_ref(parent);
	return true;
}

/*
 * ppe_drv_iface_port_idx_get()
 *	Get port of a given PPE interface
 */
int32_t ppe_drv_iface_port_idx_get(struct ppe_drv_iface *iface)
{
	struct ppe_drv_port *port;
	if ((iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID) != PPE_DRV_IFACE_FLAG_PORT_VALID) {
		return -1;
	}

	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		return -1;
	}

	return port->port;
}
EXPORT_SYMBOL(ppe_drv_iface_port_idx_get);

/*
 * ppe_drv_iface_port_clear()
 *	Clear port of a given PPE interface
 */
void ppe_drv_iface_port_clear(struct ppe_drv_iface *iface)
{
	if (iface->port) {
		iface->port = NULL;
		iface->flags &= ~PPE_DRV_IFACE_FLAG_PORT_VALID;
	}
}

/*
 * ppe_drv_iface_port_get()
 *	Get port of a given PPE interface
 */
struct ppe_drv_port *ppe_drv_iface_port_get(struct ppe_drv_iface *iface)
{
	if ((iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID) != PPE_DRV_IFACE_FLAG_PORT_VALID) {
		return NULL;
	}

	return iface->port;
}

/*
 * ppe_drv_iface_port_set()
 *	Set port of a given PPE interface
 */
bool ppe_drv_iface_port_set(struct ppe_drv_iface *iface, struct ppe_drv_port *port)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	if (iface->port) {
		ppe_drv_warn("%p: port already set for iface(%p)\n", p, iface);
		return false;
	}

	iface->port = port;
	iface->flags |= PPE_DRV_IFACE_FLAG_PORT_VALID;
	return true;
}

/*
 * ppe_drv_iface_vsi_idx_get()
 *	Get vsi index of a given PPE interface
 */
int32_t ppe_drv_iface_vsi_idx_get(struct ppe_drv_iface *iface)
{
	struct ppe_drv_vsi *vsi;
	if ((iface->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) != PPE_DRV_IFACE_FLAG_VSI_VALID) {
		return -1;
	}

	vsi = ppe_drv_iface_vsi_get(iface);
	if (!vsi) {
		return -1;
	}

	return vsi->index;
}

/*
 * ppe_drv_iface_vsi_clear()
 *	Clear VSI of a given PPE interface
 */
void ppe_drv_iface_vsi_clear(struct ppe_drv_iface *iface)
{
	if (iface->vsi) {
		iface->vsi = NULL;
		iface->flags &= ~PPE_DRV_IFACE_FLAG_VSI_VALID;
	}
}

/*
 * ppe_drv_iface_vsi_get()
 *	Get VSI of a given PPE interface
 */
struct ppe_drv_vsi *ppe_drv_iface_vsi_get(struct ppe_drv_iface *iface)
{
	if ((iface->flags & PPE_DRV_IFACE_FLAG_VSI_VALID) != PPE_DRV_IFACE_FLAG_VSI_VALID) {
		return NULL;
	}

	return iface->vsi;
}

/*
 * ppe_drv_iface_vsi_set()
 *	Set VSI of a given PPE interface
 */
bool ppe_drv_iface_vsi_set(struct ppe_drv_iface *iface, struct ppe_drv_vsi *vsi)
{
	if (iface->vsi) {
		ppe_drv_warn("%p: vsi already set for iface\n", iface);
		return false;
	}

	iface->vsi = vsi;
	iface->flags |= PPE_DRV_IFACE_FLAG_VSI_VALID;
	return true;
}

/*
 * ppe_drv_iface_l3_if_idx_get()
 *	Get l3_if index of a given PPE interface
 */
int32_t ppe_drv_iface_l3_if_idx_get(struct ppe_drv_iface *iface)
{
	struct ppe_drv_l3_if *l3_if;
	if ((iface->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) != PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
		return -1;
	}

	l3_if = ppe_drv_iface_l3_if_get(iface);
	if (!l3_if) {
		return -1;
	}

	return l3_if->l3_if_index;
}

/*
 * ppe_drv_iface_l3_if_clear()
 *	Clear L3_IF of a given PPE interface
 */
void ppe_drv_iface_l3_if_clear(struct ppe_drv_iface *iface)
{
	if (iface->l3) {
		iface->l3 = NULL;
		iface->flags &= ~PPE_DRV_IFACE_FLAG_L3_IF_VALID;
	}
}

/*
 * ppe_drv_iface_l3_if_get()
 *	Get L3_IF of a given PPE interface
 */
struct ppe_drv_l3_if *ppe_drv_iface_l3_if_get(struct ppe_drv_iface *iface)
{
	if ((iface->flags & PPE_DRV_IFACE_FLAG_L3_IF_VALID) != PPE_DRV_IFACE_FLAG_L3_IF_VALID) {
		return NULL;
	}

	return iface->l3;
}

/*
 * ppe_drv_iface_l3_if_set()
 *	Set L3_IF of a given PPE interface
 */
bool ppe_drv_iface_l3_if_set(struct ppe_drv_iface *iface, struct ppe_drv_l3_if *l3_if)
{
	if (iface->l3) {
		ppe_drv_warn("%p: l3_if already set for iface\n", iface);
		return false;
	}

	iface->l3 = l3_if;
	iface->flags |= PPE_DRV_IFACE_FLAG_L3_IF_VALID;
	return true;
}

/*
 * ppe_drv_iface_eip_set
 *	Configure an interface as inline EIP virtual port
 */
ppe_drv_ret_t ppe_drv_iface_eip_set(struct ppe_drv_iface *iface, ppe_drv_eip_service_t type, uint32_t features)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *vp;
	int32_t queue_id;

	spin_lock_bh(&p->lock);
	vp = ppe_drv_iface_port_get(iface);
	if (!vp || !PPE_DRV_VIRTUAL_PORT_CHK(vp->port)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: invalid port for eip configuration type: %u", iface, type);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	switch (type) {
	case PPE_DRV_EIP_SERVICE_IIPSEC:
		/*
		 * Mark virtual port as inline IPsec ports
		 */
		queue_id = ppe_drv_port_ucast_queue_get_by_port(PPE_DRV_PORT_EIP197);
		ppe_drv_port_flags_set(vp, PPE_DRV_PORT_FLAG_IIPSEC);

		/*
		 * Map VP queues to EIP port.
		 */
		if ((queue_id < 0) || !ppe_drv_port_ucast_queue_set(vp, queue_id)) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%p: failed to set queue for the port: %u", iface, vp->port);
			return PPE_DRV_RET_QUEUE_CFG_FAIL;
		}

		break;
	case PPE_DRV_EIP_SERVICE_NONINLINE:
		break;
	default:
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unsupported EIP service type: %u", iface, type);
		return PPE_DRV_RET_INVALID_EIP_SERVICE;
	}

	/*
	 * EIP port is not represented as net-device.
	 * Force VP based MTU instead of physical port based MTU.
	 */
	if (!ppe_drv_port_pp_mtu_cfg(vp, true)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: failed to set mtu config for the port: %u", iface, vp->port);
		return PPE_DRV_RET_MTU_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);

	/*
	 * Configure based on feature set requested.
	 */
	if (features & PPE_DRV_EIP_FEATURE_MTU_DISABLE) {
		ppe_drv_iface_mtu_disable(iface);
	}

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_iface_eip_set);

/*
 * ppe_drv_iface_mtu_disable()
 *	Disable MTU check for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mtu_disable(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_l3_if_mtu_mru_disable(l3_if)) {
			ppe_drv_warn("%p: L3_IF MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_l3_if_mtu_mru_disable(l3_if)) {
			ppe_drv_warn("%p: L3_IF MTU MRU config failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VP_L2_TUN:
	case PPE_DRV_IFACE_TYPE_VP_L3_TUN:
	{
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_port_mtu_mru_disable(port)) {
			ppe_drv_warn("%p: PORT MTU MRU config failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mtu_disable);

/*
 * ppe_drv_iface_mtu_set()
 *	Set mac address for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mtu_set(struct ppe_drv_iface *iface, uint16_t mtu)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_l3_if_mtu_mru_set(l3_if, mtu, mtu)) {
			ppe_drv_warn("%p: L3_IF MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_l3_if_mtu_mru_set(l3_if, mtu, mtu)) {
			ppe_drv_warn("%p: L3_IF MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL_PO:
	case PPE_DRV_IFACE_TYPE_VP_L2_TUN:
	case PPE_DRV_IFACE_TYPE_VP_L3_TUN:
	{
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (!ppe_drv_port_mtu_mru_set(port, mtu, mtu)) {
			ppe_drv_warn("%p: PORT MTU MRU failed\n", iface);
			status = PPE_DRV_RET_MTU_CFG_FAIL;
			break;
		}

		if (p->disable_port_mtu_check) {
			ppe_drv_info("Disabling MTU for port %d\n", port->port);
			ppe_drv_port_mtu_disable(port);
		}
		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mtu_set);

/*
 * ppe_drv_iface_mac_addr_clear()
 *	Clear mac address for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mac_addr_clear(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			ppe_drv_warn("%p: No VSI associated with iface\n", iface);
			status = PPE_DRV_RET_VSI_NOT_FOUND;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

#ifndef PPE_DRV_FLOW_IG_MAC_WAR
		if (!ppe_drv_l3_if_mac_addr_clear(l3_if)) {
#else
		if (!ppe_drv_l3_if_eg_mac_addr_clear(l3_if)) {
#endif
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, vsi);
			status =  PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

#ifndef PPE_DRV_FLOW_IG_MAC_WAR
		if (!ppe_drv_l3_if_mac_addr_clear(l3_if)) {
#else
		if (!ppe_drv_l3_if_eg_mac_addr_clear(l3_if)) {
#endif
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL_PO:
	case PPE_DRV_IFACE_TYPE_VP_L2_TUN:
	case PPE_DRV_IFACE_TYPE_VP_L3_TUN:
	{
		struct ppe_drv_l3_if *l3_if;
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_PORT_NOT_FOUND;
			break;
		}

		l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_eg_mac_addr_clear(l3_if)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_CLEAR_CFG_FAIL;
			break;
		}

		ppe_drv_port_mac_addr_clear(port);

		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mac_addr_clear);

/*
 * ppe_drv_iface_mac_addr_set()
 *	Set mac address for the given interface
 */
ppe_drv_ret_t ppe_drv_iface_mac_addr_set(struct ppe_drv_iface *iface, uint8_t *mac_addr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_ret_t status = PPE_DRV_RET_SUCCESS;

	spin_lock_bh(&p->lock);
	switch (iface->type) {
	case PPE_DRV_IFACE_TYPE_BRIDGE:
	case PPE_DRV_IFACE_TYPE_VLAN:
	{
		struct ppe_drv_vsi *vsi = ppe_drv_iface_vsi_get(iface);
		struct ppe_drv_l3_if *l3_if;
		if (!vsi) {
			ppe_drv_warn("%p: No VSI associated with iface\n", iface);
			status = PPE_DRV_RET_VSI_NOT_FOUND;
			break;
		}

		l3_if = vsi->l3_if;
		if (!l3_if) {
			ppe_drv_warn("%p: No L3_IF associated with vsi(%p)\n", iface, vsi);
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

#ifndef PPE_DRV_FLOW_IG_MAC_WAR
		if (!ppe_drv_l3_if_mac_addr_set(l3_if, mac_addr)) {
#else
		if (!ppe_drv_l3_if_eg_mac_addr_set(l3_if, mac_addr)) {
#endif
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, vsi);
			status =  PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_LAG:
	case PPE_DRV_IFACE_TYPE_PPPOE:
	{
		struct ppe_drv_l3_if *l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

#ifndef PPE_DRV_FLOW_IG_MAC_WAR
		if (!ppe_drv_l3_if_mac_addr_set(l3_if, mac_addr)) {
#else
		if (!ppe_drv_l3_if_eg_mac_addr_set(l3_if, mac_addr)) {
#endif
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
			break;
		}

		break;
	}

	case PPE_DRV_IFACE_TYPE_PHYSICAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL:
	case PPE_DRV_IFACE_TYPE_VIRTUAL_PO:
	case PPE_DRV_IFACE_TYPE_VP_L2_TUN:
	case PPE_DRV_IFACE_TYPE_VP_L3_TUN:
	{
		struct ppe_drv_l3_if *l3_if;
		struct ppe_drv_port *port = ppe_drv_iface_port_get(iface);
		if (!port) {
			status = PPE_DRV_RET_PORT_NOT_FOUND;
			break;
		}

		l3_if = ppe_drv_iface_l3_if_get(iface);
		if (!l3_if) {
			status = PPE_DRV_RET_L3_IF_NOT_FOUND;
			break;
		}

		if (!ppe_drv_l3_if_eg_mac_addr_set(l3_if, mac_addr)) {
			ppe_drv_warn("%p: L3_IF mac_addr failed(%p)\n", iface, l3_if);
			status =  PPE_DRV_RET_MAC_ADDR_SET_CFG_FAIL;
			break;
		}

		ppe_drv_port_mac_addr_set(port, mac_addr);

		break;
	}

	default:
		ppe_drv_warn("%p: invalid inface type(%d)\n", iface, iface->type);
		status = PPE_DRV_RET_IFACE_INVALID;
	}

	spin_unlock_bh(&p->lock);
	return status;
}
EXPORT_SYMBOL(ppe_drv_iface_mac_addr_set);

/*
 * ppe_drv_iface_ucast_queue_get()
 *	Get queue configuration for given PPE interface.
 */
ppe_drv_ret_t ppe_drv_iface_ucast_queue_get(struct ppe_drv_iface *iface, uint8_t *queue_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;

	spin_lock_bh(&p->lock);
	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	*queue_id = ppe_drv_port_ucast_queue_get(port);
	spin_unlock_bh(&p->lock);

	ppe_drv_trace("%p: Unicast Queue get done for given port(%p)", p, port);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_iface_ucast_queue_get);

/*
 * ppe_drv_iface_ucast_queue_set()
 *	Set queue configuration for given PPE interface.
 */
ppe_drv_ret_t ppe_drv_iface_ucast_queue_set(struct ppe_drv_iface *iface, uint8_t queue_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;

	spin_lock_bh(&p->lock);
	port = ppe_drv_iface_port_get(iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: cannot get port for given interface (%p)", p, iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	if (!ppe_drv_port_ucast_queue_set(port, queue_id)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: queue configuration failed for queue_id(%d)", iface, queue_id);
		return PPE_DRV_RET_QUEUE_CFG_FAIL;
	}

	spin_unlock_bh(&p->lock);

	ppe_drv_trace("%p: Unicast Queue (%d)setting done for given port(%p)", p, queue_id, port);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_iface_ucast_queue_set);

/*
 * ppe_drv_iface_alloc()
 *	Allocates a free interface and takes a reference.
 */
struct ppe_drv_iface *ppe_drv_iface_alloc(enum ppe_drv_iface_type type, struct net_device *dev)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *iface = NULL;
	uint16_t i;

	if (type > PPE_DRV_IFACE_TYPE_MAX) {
		ppe_drv_warn("%p: Unsupported iface type(%d)\n", dev, type);
		return NULL;
	}

	/*
	 * Get a free L3 interface entry from pool
	 */
	spin_lock_bh(&p->lock);
	for (i = 0; i < p->iface_num; i++) {
		if (!kref_read(&p->iface[i].ref)) {
			iface = &p->iface[i];
			break;
		}
	}

	if (!iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: cannot alloc iface, table full", p);
		return NULL;
	}

	kref_init(&iface->ref);
	iface->type = type;
	iface->dev = dev;
	iface->parent = NULL;
	iface->flags = PPE_DRV_IFACE_FLAG_VALID;
	iface->base_if = NULL;
	iface->port = NULL;
	iface->vsi = NULL;
	iface->l3 = NULL;
	iface->cleanup_cb = NULL;

	spin_unlock_bh(&p->lock);

	return iface;
}
EXPORT_SYMBOL(ppe_drv_iface_alloc);

/*
 * ppe_drv_iface_check_flow_offload_enabled()
 *	Check whether the given interface indexes are enabled for offload or not
 */
bool ppe_drv_iface_check_flow_offload_enabled(ppe_drv_iface_t rx_if,
						ppe_drv_iface_t tx_if)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *tx_pp = NULL;
	struct ppe_drv_port *rx_pp = NULL;
	struct ppe_drv_iface *if_rx, *if_tx;

	spin_lock_bh(&p->lock);
	if_rx = ppe_drv_iface_get_by_idx(rx_if);
	if (!if_rx) {
		ppe_drv_trace("%p: No PPE interface corresponding to rx_if: %d", p, rx_if);
		goto offload_disabled;
	}

	if_tx = ppe_drv_iface_get_by_idx(tx_if);
	if (!if_tx) {
		ppe_drv_trace("%p: No PPE interface corresponding to tx_if: %d", p, tx_if);
		goto offload_disabled;
	}

	tx_pp = ppe_drv_iface_port_get(if_tx);
	if (!tx_pp) {
		ppe_drv_trace("%p: Invalid TX port", p);
		goto offload_enabled;
	}

	rx_pp = ppe_drv_iface_port_get(if_rx);
	if (!rx_pp) {
		ppe_drv_trace("%p: Invalid RX port", p);
		goto offload_enabled;
	}

	if ((rx_pp->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP) ||
			(rx_pp->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
		if (!ppe_drv_port_check_flow_offload_enabled(tx_pp)) {
			ppe_drv_trace("%p: offload not enabled for %d port\n",
					p, tx_pp->port);
			goto offload_disabled;
		}
	}

	if ((tx_pp->user_type == PPE_DRV_PORT_USER_TYPE_ACTIVE_VP) ||
			(tx_pp->user_type == PPE_DRV_PORT_USER_TYPE_DS)) {
		if (!ppe_drv_port_check_flow_offload_enabled(rx_pp)) {
			ppe_drv_trace("%p: offload not enabled for %d port\n",
					p, rx_pp->port);
			goto offload_disabled;
		}
	}

	if ((if_tx->flags & PPE_DRV_IFACE_FLAG_MHT_SWITCH_VALID) ||
			(if_rx->flags & PPE_DRV_IFACE_FLAG_MHT_SWITCH_VALID)) {
		ppe_drv_trace("%p: Either tx or rx interface has mht switch\n", p);
		goto offload_disabled;
	}

	if (p->eth2eth_offload_if_bitmap) {
		if (!ppe_drv_port_check_flow_offload_enabled(tx_pp)) {
			ppe_drv_trace("%p: offload not enabled for %d port\n",
					p, tx_pp->port);
			goto offload_disabled;
		}

		if (!ppe_drv_port_check_flow_offload_enabled(rx_pp)) {
			ppe_drv_trace("%p: offload not enabled for %d port\n",
					p, rx_pp->port);
			goto offload_disabled;
		}
	}

offload_enabled:
	spin_unlock_bh(&p->lock);
	return true;
offload_disabled:
	spin_unlock_bh(&p->lock);
	return false;
}
EXPORT_SYMBOL(ppe_drv_iface_check_flow_offload_enabled);

/*
 * ppe_drv_iface_get_index
 *	Return PPE interface index
 */
ppe_drv_iface_t ppe_drv_iface_get_index(struct ppe_drv_iface *iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	ppe_drv_iface_t index;

	spin_lock_bh(&p->lock);
	index = iface->index;
	spin_unlock_bh(&p->lock);

	return index;
}

/*
 * ppe_drv_iface_entries_free()
 *	Free iface entries if it was allocated.
 */
void ppe_drv_iface_entries_free(struct ppe_drv_iface *iface)
{
	vfree(iface);
}

/*
 * ppe_drv_iface_entries_alloc()
 *	Allocated and initialize interface entries.
 */
struct ppe_drv_iface *ppe_drv_iface_entries_alloc()
{
	struct ppe_drv_iface *iface;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i;

	iface = vzalloc(sizeof(struct ppe_drv_iface) * p->iface_num);
	if (!iface) {
		ppe_drv_warn("%p: failed to allocate interface entries", p);
		return NULL;
	}

	/*
	 * Initialize interface values
	 */
	for (i = 0; i < p->iface_num; i++) {
		iface[i].index = i;
	}

	return iface;
}
