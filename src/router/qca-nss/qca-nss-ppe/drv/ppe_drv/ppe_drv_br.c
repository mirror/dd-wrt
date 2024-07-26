/*
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

#include "ppe_drv.h"
#include <fal/fal_fdb.h>
#include <fal/fal_stp.h>
#include <fal/fal_vsi.h>

/*
 * ppe_drv_br_fdb_del_bymac
 *	Delete FDB entry by mac address.
 */
ppe_drv_ret_t ppe_drv_br_fdb_del_bymac(struct ppe_drv_iface *br_iface, uint8_t *mac_addr)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_fdb_entry_t entry = {0};
	struct ppe_drv_vsi *vsi;
	sw_error_t err;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(br_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: VSI not assinged to bridge\n", br_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	memcpy(&entry.addr, mac_addr, ETH_ALEN);
	entry.fid = vsi->index;
	err = fal_fdb_entry_del_bymac(PPE_DRV_SWITCH_ID, &entry);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: failed to delete fdb entry for mac: %pM\n", br_iface, mac_addr);
		return PPE_DRV_RET_DEL_MAC_FDB_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%p: fdb entry delete for mac: %pM\n", br_iface, mac_addr);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_fdb_del_bymac);

/*
 * ppe_drv_br_fdb_add
 *	Add fdb entry
 */
ppe_drv_ret_t ppe_drv_br_fdb_add(struct ppe_drv_iface *br_iface,
		uint8_t *mac_addr, bool is_static, uint32_t port_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_fdb_entry_t entry = {0};
	struct ppe_drv_vsi *vsi;
	sw_error_t err;

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(br_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: VSI not assinged to bridge\n", br_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	memcpy(&entry.addr, mac_addr, ETH_ALEN);
	entry.fid = vsi->index;
	entry.port.id = port_id;
	entry.static_en = is_static;

	err = fal_fdb_entry_add(PPE_DRV_SWITCH_ID, &entry);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: failed to add fdb entry for mac: %pM\n", br_iface,
				mac_addr);
		return PPE_DRV_RET_ADD_FDB_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("%p: fdb entry added for mac: %pM\n", br_iface, mac_addr);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_fdb_add);

/*
 * ppe_drv_br_flush_fdb()
 *	Flush all dynamic or dynamic and static per port
 *	Flush all dynamic or dynamic and static(for all port)
 */
ppe_drv_ret_t ppe_drv_br_flush_fdb(struct ppe_drv_iface *port_iface,
		bool only_dynamic, bool del_by_port)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;
	sw_error_t err;
	uint32_t del_fdb_flag;

	if (only_dynamic) {
		del_fdb_flag = 0;
	} else {
		del_fdb_flag = FAL_FDB_DEL_STATIC;
	}

	spin_lock_bh(&p->lock);
	if (del_by_port) {
		port = ppe_drv_iface_port_get(port_iface);
		if (!port) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%p: unable to get port from iface\n", port_iface);
			return PPE_DRV_RET_PORT_NOT_FOUND;
		}

		err = fal_fdb_entry_del_byport(PPE_DRV_SWITCH_ID, port->port,
				del_fdb_flag);
		if (err != SW_OK) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("Failed to flush all %s fdb for port %u\n",
					(only_dynamic ? "dynamic" : ""), port->port);

			return PPE_DRV_RET_FLUSH_FDB_BY_PORT_FAIL;
		}

		spin_unlock_bh(&p->lock);
		ppe_drv_info("ALL %s FDBs are deleted for port %u\n",
					(only_dynamic ? "dynamic" : ""), port->port);

	} else {
		err = fal_fdb_entry_flush(PPE_DRV_SWITCH_ID,
				del_fdb_flag);
		if (err != SW_OK) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("Failed to flush all %s fdb \n",
					(only_dynamic ? "dynamic" : ""));

			return PPE_DRV_RET_FLUSH_FDB_FAIL;
		}

		spin_unlock_bh(&p->lock);
		ppe_drv_info("ALL %s FDBs are deleted\n",
					(only_dynamic ? "dynamic" : ""));
	}

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_flush_fdb);

/*
 * ppe_drv_br_set_ageing_time()
 *	set fdb ageing time
 */
ppe_drv_ret_t ppe_drv_br_set_ageing_time(uint32_t ageing_time)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	sw_error_t err;

	/* Check Ageing time max which is 20 bit value */
	if (ageing_time > 0xFFFFF) {
		ageing_time = 0xFFFFF;
	}

	spin_lock_bh(&p->lock);
	err = fal_fdb_aging_time_set(PPE_DRV_SWITCH_ID, &ageing_time);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Failed to set ageing time\n");
		return PPE_DRV_RET_SET_FDB_AGEING_TIME_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_trace("Global Bridge Ageing time set to %u, which is same for all bridge\n",
			ageing_time);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_set_ageing_time);

/*
 * ppe_drv_br_port_set_learning()
 *	set learning based on port
 */
ppe_drv_ret_t ppe_drv_br_port_set_learning(struct ppe_drv_iface *port_iface,
		bool lrn_enable)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *port;
	sw_error_t err;

	spin_lock_bh(&p->lock);
	port = ppe_drv_iface_port_get(port_iface);
	if (!port) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: unable to get port from port_iface\n", port_iface);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	err = fal_fdb_port_learn_set(PPE_DRV_SWITCH_ID, port->port, lrn_enable);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("port %u: fail to set learning\n", port->port);
		return PPE_DRV_RET_NEW_ADDR_LRN_FAIL;
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_info("Learning is %s for port %u\n",
			(lrn_enable ? "enable" : "disable"), port->port);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_port_set_learning);

/**
 * ppe_drv_br_fdb_lrn_ctrl
 *	Configure FDB learn control for a bridge interface in PPE.
 */
ppe_drv_ret_t ppe_drv_br_fdb_lrn_ctrl(struct ppe_drv_iface *br_iface, bool enable)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_vsi_newaddr_lrn_t newaddr_lrn = {0};
	fal_vsi_stamove_t sta_move = {0};
	struct ppe_drv_vsi *vsi;

	ppe_drv_assert(br_iface->type == PPE_DRV_IFACE_TYPE_BRIDGE,
			"%p: br_iface should be bridge, but type: %u", br_iface, br_iface->type);

	ppe_drv_info("%p: setting bridge fdb learning status: %u", br_iface, enable);

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(br_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: VSI not assinged to bridge", br_iface);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	/*
	 * Set station move
	 */
	sta_move.stamove_en = enable;
	sta_move.action = FAL_MAC_FRWRD;
	if (fal_vsi_stamove_set(PPE_DRV_SWITCH_ID, vsi->index, &sta_move) != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: failed to configure station move config: %u", br_iface, enable);
		return PPE_DRV_RET_STA_MOVE_FAIL;
	}

	/*
	 * Set FDB learning in PPE
	 */
	newaddr_lrn.lrn_en = enable;
	newaddr_lrn.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(PPE_DRV_SWITCH_ID, vsi->index, &newaddr_lrn) != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Failed to configure FDB learning %u", br_iface, enable);
		return PPE_DRV_RET_NEW_ADDR_LRN_FAIL;
	}

	/*
	 * Update vsi shadow copy
	 */
	vsi->is_fdb_learn_enabled = enable;

	/*
	 * Flush FDB table for the bridge vsi
	 */
	if (fal_fdb_entry_del_byfid(PPE_DRV_SWITCH_ID, vsi->index, FAL_FDB_DEL_STATIC) != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: failed to flush existing FDB entries", br_iface);
		return PPE_DRV_RET_FDB_FLUSH_VSI_FAIL;
	}

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_fdb_lrn_ctrl);

/**
 * ppe_drv_br_stp_state_set
 *	Set STP state on bridge interface in PPE.
 */
ppe_drv_ret_t ppe_drv_br_stp_state_set(struct ppe_drv_iface *br_iface, struct net_device *member, fal_stp_state_t state)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *member_iface;
	struct ppe_drv_port *pp;

	ppe_drv_assert(br_iface->type == PPE_DRV_IFACE_TYPE_BRIDGE,
			"%p: br_iface should be bridge, but type: %u", br_iface, br_iface->type);

	ppe_drv_info("%p: setting bridge member: %p stp state: %u", br_iface, member, state);

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface || !(member_iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bad member net_device %p", br_iface, member);
		return PPE_DRV_RET_MEM_IF_INVALID_PORT;
	}

	pp = ppe_drv_iface_port_get(member_iface);
	if (!pp) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: slave interface %p does not have a port in PPE",
				br_iface, member);
		return PPE_DRV_RET_PORT_NOT_FOUND;
	}

	/*
	 * Set STP state to forwarding after port leaves bridge
	 * Note: PPE support STP only on physical port.
	 */
	if (PPE_DRV_PHY_PORT_CHK(pp->port)) {
		if (fal_stp_port_state_set(PPE_DRV_SWITCH_ID, PPE_DRV_BR_SPANNING_TREE_ID, pp->port, state) != SW_OK) {
			spin_unlock_bh(&p->lock);
			ppe_drv_warn("%p: failed to set STA state: %u for port: %u", br_iface, state, pp->port);
			return PPE_DRV_RET_STP_STATE_FAIL;
		}
	}

	spin_unlock_bh(&p->lock);

	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_stp_state_set);

/**
 * ppe_drv_br_wanif_clear()
 *	clear ppe_drv_iface PPE_DRV_IFACE_FLAG_WAN_IF_VALID flag.
 */
void ppe_drv_br_wanif_clear(struct net_device *member)
{
	struct ppe_drv_iface *member_iface;
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Bad member net_device\n", member);
		return;
	}

	member_iface->flags &= ~(PPE_DRV_IFACE_FLAG_WAN_IF_VALID);
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_br_wanif_clear);

/**
 * ppe_drv_br_wanif_set()
 *	Set ppe_drv_iface flag to PPE_DRV_IFACE_FLAG_WAN_IF_VALID.
 */
void ppe_drv_br_wanif_set(struct net_device *member)
{
	struct ppe_drv_iface *member_iface;
	struct ppe_drv *p = &ppe_drv_gbl;

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Bad member net_device", member);
		return;
	}

	member_iface->flags |= PPE_DRV_IFACE_FLAG_WAN_IF_VALID;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_br_wanif_set);

/**
 * ppe_drv_br_leave
 *	Remove a member from bridge interface in PPE.
 */
ppe_drv_ret_t ppe_drv_br_leave(struct ppe_drv_iface *br_iface, struct net_device *member)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *member_iface;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(br_iface->type == PPE_DRV_IFACE_TYPE_BRIDGE,
			"%p: br_iface should be bridge but: %u", br_iface, br_iface->type);

	ppe_drv_info("%p: removing netdevice: %p from bridge", br_iface, member);

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bad member net_device %p", br_iface, member);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Make sure the member interface has joined the bridge earlier.
	 */
	if (ppe_drv_iface_parent_get(member_iface) != br_iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: member is not en-slaved to bridge %p", br_iface, member_iface);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Clear bridge interface as parent from member interface
	 */
	ppe_drv_iface_parent_clear(member_iface);

	/*
	 * No need to update vsi and l3 for interfaces which do not have
	 * equivalent port handle in PPE e.g. vlan.
	 */
	if (!(member_iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_info("%p: member is not a port %p", br_iface, member);
		return PPE_DRV_RET_SUCCESS;
	}

	/*
	 * Attach port to bridge vsi and corresponding l3_if.
	 */
	vsi = ppe_drv_iface_vsi_get(br_iface);
	if (!vsi) {
		ppe_drv_iface_parent_set(member_iface, br_iface);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: VSI not assinged to bridge %p", br_iface, member);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	pp = ppe_drv_iface_port_get(member_iface);
	if (!pp) {
		ppe_drv_iface_parent_set(member_iface, br_iface);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: slave interface %p does not have a port in PPE",
				br_iface, member);
		return PPE_DRV_RET_MEM_IF_INVALID_PORT;
	}

	l3_if = ppe_drv_iface_l3_if_get(br_iface);
	if (!l3_if) {
		ppe_drv_iface_parent_set(member_iface, br_iface);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bridge vsi: %p should have an l3_if", br_iface, vsi);
		return PPE_DRV_RET_IFACE_L3_IF_FAIL;
	}

	/*
	 * Detach vsi and l3_if to port
	 */
	ppe_drv_port_vsi_detach(pp, vsi);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_leave);

/**
 * ppe_drv_br_join
 *	Add a member to bridge interface in PPE.
 */
ppe_drv_ret_t ppe_drv_br_join(struct ppe_drv_iface *br_iface, struct net_device *member)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_iface *member_iface;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(br_iface->type == PPE_DRV_IFACE_TYPE_BRIDGE,
			"%p: br_iface should be bridge but: %u", br_iface, br_iface->type);

	ppe_drv_info("%p: adding netdevice: %p to bridge", br_iface, member);

	/*
	 * Check if member net-device is a known PPE interface.
	 */
	spin_lock_bh(&p->lock);
	member_iface = ppe_drv_iface_get_by_dev_internal(member);
	if (!member_iface) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bad member net_device %p", br_iface, member);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	/*
	 * Set bridge interface as parent to member interface
	 */
	ppe_drv_iface_parent_set(member_iface, br_iface);

	/*
	 * No need to update vsi and l3 for interfaces which do not have
	 * equivalent port handle in PPE e.g. vlan.
	 */
	if (!(member_iface->flags & PPE_DRV_IFACE_FLAG_PORT_VALID)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_info("%p: member net_device %p not a port ", br_iface, member);
		return PPE_DRV_RET_SUCCESS;
	}

	/*
	 * Attach port to bridge vsi and corresponding l3_if.
	 */
	vsi = ppe_drv_iface_vsi_get(br_iface);
	if (!vsi) {
		ppe_drv_iface_parent_clear(member_iface);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: VSI not assinged to bridge %p", br_iface, member);
		return PPE_DRV_RET_VSI_NOT_FOUND;
	}

	pp = ppe_drv_iface_port_get(member_iface);
	if (!pp) {
		ppe_drv_iface_parent_clear(member_iface);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: slave interface %p does not have a port in PPE",
				br_iface, member);
		return PPE_DRV_RET_MEM_IF_INVALID_PORT;
	}

	l3_if = ppe_drv_iface_l3_if_get(br_iface);
	if (!l3_if) {
		ppe_drv_iface_parent_clear(member_iface);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bridge vsi: %p should have an l3_if", br_iface, vsi);
		return PPE_DRV_RET_IFACE_L3_IF_FAIL;
	}

	/*
	 * Attach vsi and l3_if to port
	 */
	ppe_drv_port_vsi_attach(pp, vsi);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_join);

/**
 * ppe_drv_br_deinit
 *	Uninitialize bridge interface in PPE.
 */
ppe_drv_ret_t ppe_drv_br_deinit(struct ppe_drv_iface *br_iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;

	ppe_drv_assert(br_iface->type == PPE_DRV_IFACE_TYPE_BRIDGE,
			"%p: br_iface should be bridge but: %u", br_iface, br_iface->type);

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_iface_vsi_get(br_iface);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: VSI not assigned", br_iface);
		return PPE_DRV_RET_FAILURE_INVALID_PARAM;
	}

	ppe_drv_info("%p: bridge if unassign vsi: %p", br_iface, vsi);

	/*
	 * Flush FDB table for the bridge vsi
	 */
	if (fal_fdb_entry_del_byfid(PPE_DRV_SWITCH_ID, vsi->index, FAL_FDB_DEL_STATIC) != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: failed to flush existing FDB entries", br_iface);
		return PPE_DRV_RET_FDB_FLUSH_VSI_FAIL;
	}

	/*
	 * Release references & clear ppe_iface
	 */
	ppe_drv_vsi_l3_if_deref(vsi);
	ppe_drv_iface_l3_if_clear(br_iface);
	ppe_drv_vsi_deref(vsi);
	ppe_drv_iface_vsi_clear(br_iface);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_deinit);

/**
 * ppe_drv_br_init
 *	Initialize bridge interface in PPE.
 */
ppe_drv_ret_t ppe_drv_br_init(struct ppe_drv_iface *br_iface)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_l3_if *l3_if;

	ppe_drv_assert(br_iface->type == PPE_DRV_IFACE_TYPE_BRIDGE,
			"%p: br_iface should be bridge but: %u", br_iface, br_iface->type);

	spin_lock_bh(&p->lock);
	vsi = ppe_drv_vsi_alloc(PPE_DRV_VSI_TYPE_BRIDGE);
	if (!vsi) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: vsi allocation failed for bridge br_iface: %p", p, br_iface);
		return PPE_DRV_RET_FAILURE_NO_RESOURCE;
	}

	/*
	 * Get corresponding l3_if
	 */
	l3_if = ppe_drv_vsi_l3_if_get_and_ref(vsi);
	if (!l3_if) {
		ppe_drv_vsi_deref(vsi);
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: bridge vsi: %p should have an l3_if", br_iface, vsi);
		return PPE_DRV_RET_L3_IF_ALLOC_FAIL;
	}

	/*
	 * Save vsi and l3_if in ppe_iface.
	 */
	ppe_drv_iface_vsi_set(br_iface, vsi);
	ppe_drv_iface_l3_if_set(br_iface, l3_if);
	ppe_drv_info("%p: bridge if assigned vsi %p", br_iface, vsi);

	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_br_init);
