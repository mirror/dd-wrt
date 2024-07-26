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

#include <fal/fal_ip.h>
#include <fal/fal_init.h>
#include <fal/fal_api.h>
#include <fal/fal_vsi.h>
#include <ref/ref_vsi.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_vsi_dump()
 *	Dumps VSI table configuration
 */
static void ppe_drv_vsi_dump(struct ppe_drv_vsi *vsi)
{
	fal_intf_id_t vsi_cfg = {0};
	fal_mc_mode_cfg_t mc_cfg = {0};
	fal_vsi_member_t mem_cfg = {0};
	fal_vsi_newaddr_lrn_t new_addr_cfg = {0};
	fal_vsi_stamove_t stamove_cfg = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	sw_error_t err;

	err = fal_ip_vsi_intf_get(PPE_DRV_SWITCH_ID, vsi->index, &vsi_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: VSI interface query failed for VSI index: %u", p, vsi->index);
		return;
	}

	ppe_drv_trace("%p: l3_if index: %u l3_if_valid: %u\n", p, vsi_cfg.l3_if_index, vsi_cfg.l3_if_valid);

	err = fal_ip_vsi_mc_mode_get(PPE_DRV_SWITCH_ID, vsi->index, &mc_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Multicast mode query failed for vsi: %u", p, vsi->index);
		return;
	}

	ppe_drv_trace("%p: ipv4_mc_en : %u ipv4_mc_mode: %u ipv6_mc_en : %u ipv6_mc_mode: %u\n", p,
			mc_cfg.l2_ipv4_mc_en, mc_cfg.l2_ipv4_mc_mode, mc_cfg.l2_ipv6_mc_en, mc_cfg.l2_ipv6_mc_mode);

	err = fal_vsi_member_get(PPE_DRV_SWITCH_ID, vsi->index, &mem_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: VSI membership query failed for vsi: %u", p, vsi->index);
		return;
	}

	ppe_drv_trace("%p: member_ports: %u uuc_ports: %u umc_ports: %u bc_ports: %u\n", p,
			mem_cfg.member_ports, mem_cfg.uuc_ports, mem_cfg.umc_ports, mem_cfg.bc_ports);

	err = fal_vsi_newaddr_lrn_get(PPE_DRV_SWITCH_ID, vsi->index, &new_addr_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: New address learning query failed for vsi: %u", p, vsi->index);
		return;
	}

	ppe_drv_trace("%p: new_addr_learn_enable: %u new_addr_learn_action: %u \n", p,
			new_addr_cfg.lrn_en, new_addr_cfg.action);

	err = fal_vsi_stamove_get(PPE_DRV_SWITCH_ID, vsi->index, &stamove_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: STA move learning query failed for vsi: %u", p, vsi->index);
		return;
	}

	ppe_drv_trace("%p: stamove_learn_enable: %u stamove_learn_action: %u \n", p,
			stamove_cfg.stamove_en, stamove_cfg.action);
}
#else
static void ppe_drv_vsi_dump(struct ppe_drv_vsi *vsi)
{
}
#endif

/*
 * ppe_drv_vsi_fdb_learning_disable()
 *	Disable FDB learning
 */
void ppe_drv_vsi_fdb_learning_disable(struct ppe_drv_vsi *vsi)
{
	vsi->is_fdb_learn_enabled = false;
}

/*
 * ppe_drv_vsi_fdb_learning_enable()
 *	Enable FDB learning
 */
void ppe_drv_vsi_fdb_learning_enable(struct ppe_drv_vsi *vsi)
{
	vsi->is_fdb_learn_enabled = true;
}

/*
 * ppe_drv_vsi_clear_vlan()
 *	Clear vlan info from a vsi entry
 */
static inline void ppe_drv_vsi_clear_vlan(struct ppe_drv_vsi *vsi)
{
	vsi->vlan.inner_vlan = PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED;
	vsi->vlan.outer_vlan = PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED;
}

/*
 * ppe_drv_vsi_l3_if_detach()
 *	Detach L3 interface from vsi entry in PPE.
 */
void ppe_drv_vsi_l3_if_detach(struct ppe_drv_vsi *vsi)
{
	fal_intf_id_t cfg = {0};
	sw_error_t err;

	/*
	 * Detach L3 IF attached to vsi (if any)
	 */
	if (!vsi->l3_if) {
		ppe_drv_warn("%p: vsi: %u is not attached to l3_if", vsi, vsi->index);
		return;
	}

	ppe_drv_trace("%p: detaching l3_if %u from vsi %u", vsi, vsi->l3_if->l3_if_index, vsi->index);

	cfg.l3_if_valid = A_FALSE;
	err = fal_ip_vsi_intf_set(PPE_DRV_SWITCH_ID, vsi->index, &cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in detaching l3_if %u from vsi %u", vsi, vsi->l3_if->l3_if_index, vsi->index);
		return;
	}

	ppe_drv_l3_if_deref(vsi->l3_if);
	vsi->l3_if = NULL;

	ppe_drv_vsi_dump(vsi);
}

/*
 * ppe_drv_vsi_l3_if_attach()
 *	Attach L3 interface with a vsi entry in PPE.
 */
void ppe_drv_vsi_l3_if_attach(struct ppe_drv_vsi *vsi, struct ppe_drv_l3_if *l3_if)
{
	fal_intf_id_t cfg = {0};
	sw_error_t err;

	if (!kref_read(&vsi->ref)) {
		ppe_drv_warn("%p: attaching l3_if to unused vsi %u", vsi, vsi->index);
		return;
	}

	if (vsi->l3_if) {
		ppe_drv_warn("%p: vsi: %u is already attached to l3_if: %p", vsi, vsi->index, vsi->l3_if);
		return;
	}

	/*
	 * Update L3_VSI_TABLE with corresponding L3
	 */
	ppe_drv_trace("%p: attaching l3_if %u to vsi %u", vsi, l3_if->l3_if_index, vsi->index);

	cfg.l3_if_valid =  A_TRUE;
	cfg.l3_if_index = l3_if->l3_if_index;

	err = fal_ip_vsi_intf_set(PPE_DRV_SWITCH_ID, vsi->index, &cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Error in enabling l3_if %u from vsi %u", vsi, l3_if->l3_if_index, vsi->index);
		return;
	}

	vsi->l3_if = ppe_drv_l3_if_ref(l3_if);
	ppe_drv_vsi_dump(vsi);
}

/*
 * ppe_drv_vsi_free()
 *	Destroys the vsi entry in PPE.
 */
static void ppe_drv_vsi_free(struct kref *kref)
{
	struct ppe_drv_vsi *vsi = container_of(kref, struct ppe_drv_vsi, ref);
	fal_vsi_member_t vsi_mem_cfg = {0};
	fal_vsi_newaddr_lrn_t addr_cfg = {0};
	fal_vsi_stamove_t sta_cfg = {0};

	/*
	 * clear vlan and other info corresponding to vsi
	 */
	ppe_drv_vsi_clear_vlan(vsi);
	ppe_drv_vsi_fdb_learning_disable(vsi);
	vsi->type = PPE_DRV_VSI_TYPE_MAX;

	if (fal_vsi_member_set(PPE_DRV_SWITCH_ID, vsi->index, &vsi_mem_cfg) != SW_OK) {
		ppe_drv_warn("%p: failed to reset port bitmap for vsi\n", vsi);
	}

	/*
	 * Disable learning
	 */
	if (fal_vsi_newaddr_lrn_set(PPE_DRV_SWITCH_ID, vsi->index, &addr_cfg) != SW_OK) {
		ppe_drv_warn("%p: failed to clear address_learning for vsi\n", vsi);
	}

	/*
	 * Disable Station Movement
	 */
	if (fal_vsi_stamove_set(PPE_DRV_SWITCH_ID, vsi->index, &sta_cfg) != SW_OK) {
		ppe_drv_warn("%p: failed to set station move for vsi\n", vsi);
	}

	if (ppe_vsi_free(PPE_DRV_SWITCH_ID, vsi->index) != SW_OK) {
		ppe_drv_warn("%p: vsi free failed\n", vsi);
	}

	if (!vsi->l3_if) {
		return;
	}

	/*
	 * Detach L3 IF attached to vsi (if any)
	 */
	ppe_drv_trace("%p: detaching l3_if %u from vsi %u", vsi, vsi->l3_if->l3_if_index, vsi->index);

	/*
	 * Free l3_if ref, which was taken while creating l3_if from vsi_create
	 */
	ppe_drv_l3_if_deref(vsi->l3_if);
	ppe_drv_vsi_l3_if_detach(vsi);
}

/*
 * ppe_drv_vsi_ref()
 *	Take reference on vsi.
 */
struct ppe_drv_vsi *ppe_drv_vsi_ref(struct ppe_drv_vsi *vsi)
{
	kref_get(&vsi->ref);

	return vsi;
}

/*
 * ppe_drv_vsi_deref()
 *	Let go of reference on vsi.
 */
bool ppe_drv_vsi_deref(struct ppe_drv_vsi *vsi)
{
	if (kref_put(&vsi->ref, ppe_drv_vsi_free)) {
		ppe_drv_trace("%p: reference goes down to 0 for vsi\n", vsi);
		return true;
	}

	return false;
}

/*
 * ppe_drv_vsi_set_vlan()
 *	Set vlan info associated with vsi.
 *
 * PPE support only 2 vlan, caller should make sure not to set more than double vlan.
 */
bool ppe_drv_vsi_set_vlan(struct ppe_drv_vsi *vsi, uint32_t vlan_id, struct ppe_drv_iface *nh_iface)
{
	/*
	 * PPE support vlan acceleration only on physical, bond or pppoe interface.
	 */
	if ((nh_iface->type == PPE_DRV_IFACE_TYPE_PHYSICAL)
		|| (nh_iface->type == PPE_DRV_IFACE_TYPE_LAG)
		|| (nh_iface->type == PPE_DRV_IFACE_TYPE_VIRTUAL)
		|| (nh_iface->flags & PPE_DRV_IFACE_VLAN_OVER_BRIDGE)
		|| (nh_iface->type == PPE_DRV_IFACE_TYPE_VIRTUAL_PO)
		|| (nh_iface->type == PPE_DRV_IFACE_TYPE_VP_L2_TUN)) {

		/*
		 * single vlan
		 */
		vsi->vlan.inner_vlan = vlan_id;
	} else if (nh_iface->type == PPE_DRV_IFACE_TYPE_VLAN) {
		struct ppe_drv_vsi *nh_vsi = ppe_drv_iface_vsi_get(nh_iface);
		if (!nh_vsi) {
			ppe_drv_warn("%p: Invalid vsi associated with given nh_iface\n", vsi);
			return false;
		}

		vsi->vlan.outer_vlan = nh_vsi->vlan.inner_vlan;
		vsi->vlan.inner_vlan = vlan_id;
	} else {
		ppe_drv_warn("%p: Invalid nh_iface type(%d) during vlan setup dev name %s\n", vsi, nh_iface->type,
			     nh_iface->dev->name);
		return false;
	}

	ppe_drv_trace("%p: vsi configuration done for outer vlan_id(%d) inner vlan_id(%d) and interface_type(%d)\n"
		      "dev(%s) flag %d\n", vsi, vsi->vlan.outer_vlan, vsi->vlan.inner_vlan, nh_iface->type,
		      nh_iface->dev->name, nh_iface->flags);
	return true;
}

/*
 * ppe_drv_vsi_match_vlan()
 *	match vlan IDs associated with vsi.
 */
bool ppe_drv_vsi_match_vlan(struct ppe_drv_vsi *vsi, uint32_t inner_vlan, uint32_t outer_vlan)
{
	if (!kref_read(&vsi->ref)) {
		ppe_drv_warn("%p: operating on unused vsi:%u", vsi, vsi->index);
		return false;
	}

	return (vsi->vlan.inner_vlan == inner_vlan) && (vsi->vlan.outer_vlan == outer_vlan);
}

/*
 * ppe_drv_vsi_l3_if_get_and_ref()
 *	Returns pointer to vsi's l3_if and takes a reference
 */
struct ppe_drv_l3_if *ppe_drv_vsi_l3_if_get_and_ref(struct ppe_drv_vsi *vsi)
{
	struct ppe_drv_l3_if *l3_if;

	if (!kref_read(&vsi->ref)) {
		ppe_drv_warn("%p: operating on unused vsi:%u", vsi, vsi->index);
		return NULL;
	}

	/*
	 * Take a reference on vsi's l3_if. This will be let go when
	 * ppe_drv_vsi_l3_if_deref() is called.
	 */
	l3_if = vsi->l3_if;
	if (!l3_if) {
		ppe_drv_warn("%p: Invalid L3_IF for vsi_index:%u", vsi, vsi->index);
		return NULL;
	}

	ppe_drv_l3_if_ref(l3_if);

	return l3_if;
}

/*
 * ppe_drv_vsi_l3_if_deref()
 *	Lets go of reference to vsi's l3_if
 */
void ppe_drv_vsi_l3_if_deref(struct ppe_drv_vsi *vsi)
{
	if (!kref_read(&vsi->ref)) {
		ppe_drv_warn("%p: operating on unused vsi:%u", vsi, vsi->index);
		return;
	}

	if (!vsi->l3_if) {
		ppe_drv_warn("%p: Invalid L3_IF for vsi_index:%u", vsi, vsi->index);
		return;
	}

	ppe_drv_l3_if_deref(vsi->l3_if);
}

/*
 * ppe_drv_vsi_mc_enable()
 *	Enable multicast on vsi entry in PPE.
 */
void ppe_drv_vsi_mc_enable(struct ppe_drv_vsi *vsi)
{
	fal_mc_mode_cfg_t cfg = {0};

	/*
	 * Update L3_VSI_TABLE with L2 Multicast enable.
	 * Note: By default Multicast mode is set to 1, which will enable both IGMP V2 & V3 on this VSI.
	 */
	ppe_drv_trace("%p: Enabling mc on vsi %u l3_if %u", vsi, vsi->index, vsi->l3_if->l3_if_index);

	cfg.l2_ipv4_mc_en =  A_TRUE;
	cfg.l2_ipv4_mc_mode = FAL_MC_MODE_SGV;
	cfg.l2_ipv6_mc_en =  A_TRUE;
	cfg.l2_ipv6_mc_mode = FAL_MC_MODE_SGV;

	if (fal_ip_vsi_mc_mode_set(PPE_DRV_SWITCH_ID, vsi->index, &cfg) != SW_OK) {
		ppe_drv_warn("%p: Error in enabling mc on vsi %u l3_if %u", vsi, vsi->index, vsi->l3_if->l3_if_index);
		return;
	}

	ppe_drv_vsi_dump(vsi);
}

/*
 * ppe_drv_vsi_mc_disable()
 *	Disable multicast on vsi entry in PPE.
 */
void ppe_drv_vsi_mc_disable(struct ppe_drv_vsi *vsi)
{
	fal_mc_mode_cfg_t cfg = {0};

	if (!kref_read(&vsi->ref)) {
		ppe_drv_warn("%p: Disabling mc on unused vsi %u", vsi, vsi->index);
		return;
	}

	ppe_drv_trace("%p: Disabling mc on vsi %u l3_if %u", vsi, vsi->index, vsi->l3_if->l3_if_index);

	if (fal_ip_vsi_mc_mode_set(PPE_DRV_SWITCH_ID, vsi->index, &cfg) != SW_OK) {
		ppe_drv_warn("%p: Error in Disabling mc on vsi %u l3_if %u", vsi, vsi->index, vsi->l3_if->l3_if_index);
		return;
	}

	ppe_drv_vsi_dump(vsi);
}

/*
 * ppe_vsi_alloc()
 *	Allocate a vsi entry in PPE.
 */
struct ppe_drv_vsi *ppe_drv_vsi_alloc(enum ppe_drv_vsi_type type)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_vsi *vsi = NULL;
	struct ppe_drv_l3_if *l3_if;
	fal_vsi_member_t vsi_mem_cfg = {0};
	fal_vsi_newaddr_lrn_t addr_cfg = {0};
	fal_vsi_stamove_t sta_cfg = {0};
	uint32_t vsi_idx;

	if (type >= PPE_DRV_VSI_TYPE_MAX) {
		ppe_drv_warn("%p: Invalid vsi type %d", p, type);
		return NULL;
	}

	if (ppe_vsi_alloc(PPE_DRV_SWITCH_ID, &vsi_idx) != SW_OK) {
		ppe_drv_warn("%p: vsi allocation failed in ssdk for type %d", p, type);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_vsi_full);
		return NULL;
	}

	if (kref_read(&p->vsi[vsi_idx].ref)) {
		ppe_drv_warn("%p: Trying to use an inuse vsi with index %d", p, vsi_idx);
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_vsi_reuse);
		return NULL;
	}

	vsi = &p->vsi[vsi_idx];
	kref_init(&vsi->ref);

	/*
	 * Pre-allocate a l3_if for vsi
	 */
	l3_if = ppe_drv_l3_if_alloc(PPE_DRV_L3_IF_TYPE_PORT);
	if (!l3_if) {
		ppe_drv_warn("%p: failed to allocate vsi since no l3 is available", p);
		ppe_drv_vsi_deref(vsi);
		return NULL;
	}

	/*
	 * Enable vsi as unicast, multicast and broadcast ports
	 */
	vsi_mem_cfg.member_ports = PPE_DRV_VSI_PORT_BITMAP(1);
	vsi_mem_cfg.uuc_ports = PPE_DRV_VSI_UUC_BITMAP(1);
	vsi_mem_cfg.umc_ports = PPE_DRV_VSI_UMC_BITMAP(1);
	vsi_mem_cfg.bc_ports = PPE_DRV_VSI_BC_BITMAP(1);

	if (fal_vsi_member_set(PPE_DRV_SWITCH_ID, vsi->index, &vsi_mem_cfg) != SW_OK) {
		ppe_drv_l3_if_deref(l3_if);
		ppe_drv_vsi_deref(vsi);
		ppe_drv_warn("%p: failed to configure port bitmap for vsi(%p)", p, vsi);
		return NULL;
	}

	/*
	 * Enable learning
	 */
	addr_cfg.lrn_en = 1;
	addr_cfg.action = FAL_MAC_FRWRD;
	if (fal_vsi_newaddr_lrn_set(PPE_DRV_SWITCH_ID, vsi->index, &addr_cfg) != SW_OK) {
		ppe_drv_l3_if_deref(l3_if);
		ppe_drv_vsi_deref(vsi);
		ppe_drv_warn("%p: failed to set address_learning for vsi(%p)", p, vsi);
		return NULL;
	}

	/*
	 * Enable Station Movement
	 */
	sta_cfg.stamove_en = 1;
	sta_cfg.action = FAL_MAC_RDT_TO_CPU;
	if (fal_vsi_stamove_set(PPE_DRV_SWITCH_ID, vsi->index, &sta_cfg) != SW_OK) {
		ppe_drv_l3_if_deref(l3_if);
		ppe_drv_vsi_deref(vsi);
		ppe_drv_warn("%p: failed to set station move for vsi(%p)", p, vsi);
		return NULL;
	}

	/*
	 * Note: the reference for l3_if was taken during the create process at the
	 * start of this function.
	 */
	ppe_drv_vsi_l3_if_attach(vsi, l3_if);
	vsi->type = type;
	ppe_drv_vsi_fdb_learning_enable(vsi);

	ppe_drv_trace("%p: vsi %u of type %u created", vsi, vsi->index, type);

	ppe_drv_vsi_dump(vsi);

	return vsi;
}

/*
 * ppe_drv_vsi_entries_free()
 *	Free vsi table entries if it was allocated.
 */
void ppe_drv_vsi_entries_free(struct ppe_drv_vsi *vsi)
{
	vfree(vsi);
}

/*
 * ppe_drv_vsi_entries_alloc()
 *	Allocate and initialize vsi entries.
 */
struct ppe_drv_vsi *ppe_drv_vsi_entries_alloc()
{
	struct ppe_drv_vsi *vsi;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint16_t i;

	vsi = vzalloc(sizeof(struct ppe_drv_vsi) * p->vsi_num);
	if (!vsi) {
		ppe_drv_warn("%p: failed to allocate vsi entries", p);
		return NULL;
	}

	for (i = 0; i < p->vsi_num; i++) {
		vsi[i].index = i;
		vsi[i].type = PPE_DRV_VSI_TYPE_MAX;
		vsi[i].vlan.inner_vlan = PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED;
		vsi[i].vlan.outer_vlan = PPE_DRV_VLAN_HDR_VLAN_NOT_CONFIGURED;
	}

	return vsi;
}
