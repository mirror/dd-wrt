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

#include <linux/ip.h>
#include <net/vxlan.h>
#include <linux/in.h>
#include <linux/version.h>
#include <nat46/nat46-core.h>
#include <nat46/nat46-netdev.h>

#include <fal/fal_ip.h>
#include <fal_tunnel.h>
#include <fal_mapt.h>
#include <fal_port_ctrl.h>
#include <fal_vport.h>
#include <fal_vxlan.h>
#include <fal/fal_pppoe.h>
#include <fal/fal_tunnel_program.h>

#include <ppe_drv/ppe_drv.h>
#include <ppe_drv_tun_public.h>
#include "ppe_drv_tun.h"
#include "ppe_drv_tun_v4.h"
#include "ppe_drv_tun_v6.h"

/*
 * ppe_drv_tun_check_support()
 *     check if protocol is tunnel
 */
bool ppe_drv_tun_check_support(uint8_t protocol)
{
	switch (protocol) {
	case IPPROTO_IPIP:
	case IPPROTO_GRE:
		return true;
	default:
		return false;
	}
}

/*
 * ppe_drv_tun_ref
 *	Take reference on tunnel context
 */
static struct ppe_drv_tun *ppe_drv_tun_ref(struct ppe_drv_tun *ptun)
{

	kref_get(&ptun->ref);

	ppe_drv_assert(kref_read(&ptun->ref), "%p: ref count rollover for tun_idx:%u", ptun, ptun->tun_idx);
	ppe_drv_trace("%p: tun_idx: %u ref inc:%u", ptun, ptun->tun_idx, kref_read(&ptun->ref));
	return ptun;
}

/*
 * ppe_drv_tun_free
 *	Free tunnel instance
 */
static void ppe_drv_tun_free(struct kref *kref)
{
	struct ppe_drv_tun *ptun = container_of(kref, struct ppe_drv_tun, ref);
	struct ppe_drv *p = &ppe_drv_gbl;

	if (ptun->pp) {
		ppe_drv_port_tun_set(ptun->pp, NULL);
	}

	/*
	 * Reset encap header control settings if configured
	 */
	if (ptun->encap_hdr_bitmap) {
		ppe_drv_tun_encap_hdr_ctrl_reset(ptun->encap_hdr_bitmap);
	}

	/*
	 * Release all the tables reserved for this tunnel context
	 */
	if (ptun->ptec) {
		ppe_drv_tun_encap_deref(ptun->ptec);
	}

	if (ptun->ptdc) {
		ppe_drv_tun_decap_deref(ptun->ptdc);
	}

	if (ptun->ptecxr) {
		ppe_drv_tun_encap_xlate_rule_deref(ptun->ptecxr);
	}

	if (p->tun_gbl.tun_l2tp.l2tp_encap_rule && (!(kref_read(&p->tun_gbl.tun_l2tp.l2tp_encap_rule->ref)))) {
		p->tun_gbl.tun_l2tp.l2tp_encap_rule = NULL;
	}

	if (ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]) {
		ppe_drv_tun_decap_xlate_rule_deref(ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]);
	}

	if (ptun->ptdcxr[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]) {
		ppe_drv_tun_decap_xlate_rule_deref(ptun->ptdcxr[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]);
	}

	if (ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]) {
		ppe_drv_tun_decap_map_entry_deref(ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]);
	}

	if (ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]) {
		ppe_drv_tun_decap_map_entry_deref(ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]);
	}

	kfree(ptun);
}

/*
 * ppe_drv_tun_deref
 *	Release reference to tunnel context
 */
static bool ppe_drv_tun_deref(struct ppe_drv_tun *ptun)
{
	uint8_t tun_idx __maybe_unused = ptun->tun_idx;

	ppe_drv_assert(kref_read(&ptun->ref), "%p: ref count under run for tun", ptun);

	if (kref_put(&ptun->ref, ppe_drv_tun_free)) {
		ppe_drv_trace("reference count is 0 for tun: %p at index: %u", ptun, tun_idx);
		return true;
	}

	ppe_drv_trace("%p: tun_idx: %u ref dec:%u", ptun, tun_idx, kref_read(&ptun->ref));
	return false;
}

/*
 * ppe_drv_tun_deactivate_mapt
 *	Deactivate PPE MAPT
 */
static bool ppe_drv_tun_deactivate_mapt(struct ppe_drv_tun *tun)
{
	sw_error_t err;
	struct ppe_drv_tun_decap *ptdc = tun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY];

	err = fal_mapt_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, A_FALSE);
	if (err != SW_OK) {
		ppe_drv_warn("%p: decap entry %d disable failed", ptdc, ptdc->tl_index);
		return false;
	}

	ptdc = tun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY];
	err = fal_mapt_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, A_FALSE);
	if (err != SW_OK) {
		ppe_drv_warn("%p: decap entry %d disable failed", ptdc, ptdc->tl_index);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_activate_mapt
 *	Activate PPE MAPT
 */
static bool ppe_drv_tun_activate_mapt(struct ppe_drv_tun *ptun, struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr)
{
	/*
	 * Program Source IPv6 address in LPM table and
	 * set it as local IP address.
	 */
	uint16_t rule_id;
	bool status;
	sw_error_t err;
	struct ppe_drv_tun_decap *ptdc;
	struct ppe_drv_tun_cmn_ctx *th = &ptun->th;
	uint32_t tl_l3_if_idx = ppe_drv_tun_l3_if_get_index(ptun->pt_l3_if);

	rule_id = ppe_drv_tun_decap_xlate_rule_get_index(ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]);
	status = ppe_drv_tun_decap_map_configure(ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY], &th->l3.daddr[0],
			th->tun.mapt.remote.ipv6_prefix_len, l2_hdr, true, ptun->vp_num, rule_id, false);
	if (!status) {
		ppe_drv_warn("%p: Failed to configure decap map table for rule index %d", ptun, rule_id);
		return status;
	}

	ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]->tl_l3_if_idx = tl_l3_if_idx;
	ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]->tl_l3_if_idx = tl_l3_if_idx;

	rule_id = ppe_drv_tun_decap_xlate_rule_get_index(ptun->ptdcxr[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]);
	status = ppe_drv_tun_decap_map_configure(ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY], &th->l3.saddr[0],
			th->tun.mapt.local.ipv6_prefix_len, NULL, false, 0, rule_id, true);
	if (!status) {
		ppe_drv_warn("%p: decap map configure failed for rule_id %d", ptun, rule_id);
		return status;
	}

	ptdc = ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY];
	err = fal_mapt_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, A_TRUE);
	if (err != SW_OK) {
		ppe_drv_warn("%p: decap entry %d enable failed", ptdc, ptdc->tl_index);
		return false;
	}

	ptdc = ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY];
	err = fal_mapt_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, A_TRUE);
	if (err != SW_OK) {
		ppe_drv_warn("%p: decap entry %d enable failed", ptdc, ptdc->tl_index);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_mapt_alloc
 *	Create PPE MAP-T context
 */
static struct ppe_drv_tun *ppe_drv_tun_mapt_alloc(struct ppe_drv *p, struct ppe_drv_port *pp,
						  struct ppe_drv_tun_cmn_ctx *th, void *add_cb, void *del_cb)
{
	struct ppe_drv_tun *ptun;
	bool is_dmr = true;
	bool is_src_ipv6 = true;
	uint8_t vp_num;
	uint8_t rule_id;
	bool status;

	ptun = kzalloc(sizeof(struct ppe_drv_tun), GFP_ATOMIC);
	if (!ptun) {
		ppe_drv_warn("%p: Couldn't allocate tun", p);
		return NULL;
	}

	spin_lock_bh(&p->lock);
	kref_init(&ptun->ref);

	ptun->ptec = ppe_drv_tun_encap_alloc(p);
	if (!ptun->ptec) {
		ppe_drv_warn("%p: couldn't get free EG tunnel control instance", p);
		goto err_exit;
	}

	ptun->ptec->port = pp;
	ptun->ptecxr = ppe_drv_tun_encap_xlate_rule_alloc(p);
	if (!ptun->ptecxr) {
		ppe_drv_warn("%p: couldn't get free EG EDIT RULE instance", p);
		goto err_exit;
	}

	/*
	 * Remote entry should be the first table to be configured
	 */
	ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY] = ppe_drv_tun_decap_map_alloc(p);
	if (!ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]) {
		ppe_drv_warn("%p: couldn't get free TL MAP LPM table instance for remote", p);
		goto err_exit;
	}

	ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY] = ppe_drv_tun_decap_map_alloc(p);
	if (!ptun->ptdcm[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]) {
		ppe_drv_warn("%p: couldn't get free TL MAP LPM table instance for local", p);
		goto err_exit;
	}

	ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY] = ppe_drv_tun_decap_xlate_rule_alloc(p);
	if (!ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]) {
		ppe_drv_warn("%p: couldn't get free TL MAP LPM table instance for remote", p);
		goto err_exit;
	}

	ptun->ptdcxr[PPE_DRV_TUN_DECAP_LOCAL_ENTRY] = ppe_drv_tun_decap_xlate_rule_alloc(p);
	if (!ptun->ptdcxr[PPE_DRV_TUN_DECAP_LOCAL_ENTRY]) {
		ppe_drv_warn("%p: couldn't get free TL MAP LPM table instance for local", p);
		goto err_exit;
	}

	vp_num = ppe_drv_port_num_get(pp);
	ptun->vp_num = vp_num;
	memcpy(&ptun->th, th, sizeof(*th));
	ptun->pp = pp;
	ptun->add_cb = add_cb;
	ptun->del_cb = del_cb;

	/*
	 * Following tables need to be programmed:
	 *	Inbound tables:
	 *		1. TL_MAP_LPM_TBL - remote & local
	 *		2. TL_MAP_LPM_ACT - remote & local
	 *		3. TL_MAP_RULE_TBL - remote & local
	 *	Outbound tables:
	 *		1. EG_HEADER_DATA
	 *		2. EG_XLAT_TUN_CTRL
	 *		3. EG_EDIT_RULE - remote
	 */

	/*
	 * Configure remote entry parameters
	 */
	status = ppe_drv_tun_decap_xlate_rule_configure(ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY],
							&th->tun.mapt.remote, is_src_ipv6, is_dmr);
	if (!status) {
		ppe_drv_warn("%p: decap xlate rule configure failed for remote entry", ptun);
		goto err_exit;
	}

	/*
	 * Configure local entry parameters
	 */
	is_src_ipv6 = false;
	status = ppe_drv_tun_decap_xlate_rule_configure(ptun->ptdcxr[PPE_DRV_TUN_DECAP_LOCAL_ENTRY],
							&th->tun.mapt.local,
							is_src_ipv6, is_dmr);
	if (!status) {
		ppe_drv_warn("%p: decap xlate rule configure failed for local entry", ptun);
		goto err_exit;
	}

	/*
	 * Disable dmac check bit in l3_if configuration
	 */
	ppe_drv_l3_if_dmac_check_set(ptun->pp->port_l3_if, false);

	rule_id = ppe_drv_tun_decap_xlate_rule_get_index(ptun->ptdcxr[PPE_DRV_TUN_DECAP_REMOTE_ENTRY]);
	ppe_drv_tun_decap_map_set_rule_id(ptun->ptdcm[PPE_DRV_TUN_DECAP_REMOTE_ENTRY], rule_id);

	rule_id = ppe_drv_tun_encap_xlate_rule_get_index(ptun->ptecxr);
	ppe_drv_tun_encap_set_rule_id(ptun->ptec, rule_id);

	ppe_drv_port_tun_set(pp, ptun);
	spin_unlock_bh(&p->lock);
	ppe_drv_trace("%p: tun context of type %u created, VP: %d", ptun, th->type, vp_num);

	return ptun;
err_exit:
	ppe_drv_tun_deref(ptun);
	spin_unlock_bh(&p->lock);
	return NULL;
}

/*
 *  ppe_drv_tun_mapt_port_tun_get
 *	return ptun associated with the port for mapt
 */
struct ppe_drv_tun *ppe_drv_tun_mapt_port_tun_get(struct ppe_drv_port *tx_port, struct ppe_drv_port *rx_port)
{
	struct ppe_drv_tun *ptun;

	ptun = ppe_drv_port_tun_get(tx_port) ? ppe_drv_port_tun_get(tx_port) : ppe_drv_port_tun_get(rx_port);

	return (ptun && (ptun->th.type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT)) ? ptun : NULL;
}

/*
 * ppe_drv_tun_port_encap_disable
 * 	Disable encapsulation on tunnel virtual port
 */
bool ppe_drv_tun_port_encap_disable(struct ppe_drv_port *pp)
{
	sw_error_t err;
	fal_vport_state_t vp_state = {0};
	uint32_t v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, pp->port);

	err = fal_vport_state_check_get(PPE_DRV_SWITCH_ID, v_port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get vp port state for port vp %d", pp, pp->port);
		return false;
	}

	/*
	 * Ensure that VP Check enable is true
	 */
	ppe_drv_assert(vp_state.check_en == A_TRUE, "%p: VP state check is not enabled on port %d",
						pp, pp->port);

	vp_state.eg_data_valid = A_FALSE;
	vp_state.vp_active = A_FALSE;
	err = fal_vport_state_check_set(PPE_DRV_SWITCH_ID, v_port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to reset vp state port vp %d", pp, pp->port);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_port_deconfigure
 * 	Deconfigure tunnel specific configuration in port
 */
bool ppe_drv_tun_port_reset_physical_port(struct ppe_drv_port *pp)
{
	sw_error_t err;
	uint32_t v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, pp->port);

	err = fal_vport_physical_port_id_set(PPE_DRV_SWITCH_ID, v_port, PPE_DRV_PORT_CPU);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to reset physical port for vp %d", pp, pp->port);
		return false;
	}

	return true;
}

/*
 * ppe_drv_base_port_get
 * 	Get base port associated with the xmit port
 */
uint8_t ppe_drv_tun_xmit_port_get(uint8_t xmit_port)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	while (xmit_port >= PPE_DRV_PHYSICAL_MAX) {
		xmit_port = p->port[xmit_port].xmit_port;
	}

	return xmit_port;
}

/*
 * ppe_drv_tun_port_configure
 * 	Configure L2 VP port table.
 */
bool ppe_drv_tun_port_configure(struct ppe_drv_tun *ptun, uint16_t xmit_port)
{
	sw_error_t err;
	fal_vport_state_t vp_state = {0};
	struct ppe_drv_port *pp = ptun->pp;
	uint32_t v_port = FAL_PORT_ID(FAL_PORT_TYPE_VPORT, pp->port);
	uint16_t extra_hdr_len = 0;
	uint8_t dp_queue_id;
	struct ppe_drv_port *dp = NULL; /* Destination port */
	uint16_t phy_port;

	/*
	 * TODO: Update extra header length setting in port MTU config
	 * based on PPPOE header
	 */
	extra_hdr_len = 0;
	if (!ppe_drv_port_mtu_cfg_update(pp, extra_hdr_len)) {
		ppe_drv_warn("%p: failed to set mtu extra header len for port %d", ptun, pp->port);
		return false;
	}

	/*
	 * Get destination port
	 */
	dp = ppe_drv_port_from_port_num(xmit_port);
	if (!dp) {
		ppe_drv_warn("%p: Couldn't get destination port for iface index %u", ptun, xmit_port);
		return false;
	}

	/*
	 * Set phyiscal port based on xmit_port value
	 */
	phy_port = ppe_drv_tun_xmit_port_get(xmit_port);

	/*
	 * Map destination port queue to tunnel port
	 */
	err = fal_vport_physical_port_id_set(PPE_DRV_SWITCH_ID, v_port, phy_port);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to set physical port:%d for vp port:%d", pp,
					xmit_port, pp->port);
		return false;
	}

	if (ppe_drv_tun_dp_port_ds(dp)) {
		/*
		 * Set flag to denote tunnel end point is a WIFI vp with DS enabled
		 * to configure the correct port profile
		 */
		ppe_drv_port_flags_set(ptun->pp, PPE_DRV_PORT_FLAG_TUN_ENDPOINT_DS);

		/*
		 * Map tunnel port queue to the destination port queue
		 * Needs to be done after mapping the phy port to tun port and
		 * for destination ports being a vp as phy ports are already initializaed with
		 * correct profile and queues initially.
		 */
		dp_queue_id = ppe_drv_port_ucast_queue_get(dp);
		if (!ppe_drv_port_ucast_queue_set(ptun->pp, dp_queue_id)) {
			ppe_drv_warn("%p: Failed to set queue %d for port", ptun, dp_queue_id);
			return false;
		}
	}

	err = fal_vport_state_check_get(PPE_DRV_SWITCH_ID, v_port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get vp port state for port vp %d", pp, pp->port);
		return false;
	}

	/*
	 * Ensure that VP Check enable is true
	 */
	ppe_drv_assert(vp_state.check_en == A_TRUE, "%p: VP state check is not enabled on port %d",
						pp, pp->port);

	/*
	 * Set eg_data_valid and context enable
	 */
	vp_state.eg_data_valid = A_TRUE;
	vp_state.vp_active = A_TRUE;
	err = fal_vport_state_check_set(PPE_DRV_SWITCH_ID, v_port, &vp_state);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to set L2 vp port state for port %d", pp, pp->port);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_pppoe_tl_l3_if_get
 * Get the tl_l3_if associated with pppoe instance
 */
struct ppe_drv_tun_l3_if *ppe_drv_tun_pppoe_tl_l3_if_get(struct ppe_drv_tun *ptun, struct ppe_drv_pppoe *pppoe)
{
	struct ppe_drv_tun_l3_if *ptun_l3_if;
	struct ppe_drv *p = &ppe_drv_gbl;

	ptun_l3_if = ppe_drv_pppoe_tl_l3_if_get(pppoe);
	if (ptun_l3_if) {
		return ptun_l3_if;
	}

	ptun_l3_if = ppe_drv_tun_l3_if_alloc(p);
	if (!ptun_l3_if) {
		ppe_drv_warn("%p: Failed to allocate tl_l3_if to pppoe %p", ptun, pppoe);
		return NULL;
	}

	ptun->pppoe = pppoe;

	ppe_drv_tun_l3_if_configure(ptun_l3_if);

	ppe_drv_pppoe_tl_l3_if_attach(pppoe, ptun_l3_if);

	return ptun_l3_if;
}


/*
 * ppe_drv_tun_port_tl_l3_if_get
 * Get the tl_l3_if associated with port
 */
struct ppe_drv_tun_l3_if *ppe_drv_tun_port_tl_l3_if_get(struct ppe_drv_tun *ptun, uint16_t xmit_port)
{
	struct ppe_drv_tun_l3_if *ptun_l3_if;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp;

	/*
	 * Get destination port
	 */
	pp = ppe_drv_port_from_port_num(xmit_port);
	if (!pp) {
		ppe_drv_warn("%p: Couldn't get destination port for iface index %u", ptun, xmit_port);
		return NULL;
	}

	/*
	 * check if tl_l3_if attached to port if so take a reference
	 * reference count will be decremented during tun deactivate
	 */
	ptun_l3_if = ppe_drv_port_tl_l3_if_get_n_ref(pp);
	if (ptun_l3_if) {
		return ptun_l3_if;
	}

	/*
	 * Attach tl_l3_if to destination port
	 */
	ptun_l3_if = ppe_drv_tun_l3_if_alloc(p);
	if (!ptun_l3_if) {
		ppe_drv_warn("%p: Failed to allocate tl_l3_if to port %u", ptun, xmit_port);
		return NULL;
	}

	ppe_drv_tun_l3_if_configure(ptun_l3_if);

	ppe_drv_port_tl_l3_if_attach(pp, ptun_l3_if);

	return ptun_l3_if;
}

/*
 * ppe_drv_tun_decap_xmitport_cfg_set
 *	Port DECAP Configuration setup
 */
bool ppe_drv_tun_decap_xmitport_cfg_set(struct ppe_drv_tun *ptun, uint16_t xmit_port,
					struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr, uint16_t tl_l3_if_idx)
{

	fal_tunnel_port_intf_t port_tnl_cfg = {0};
	struct ppe_drv_port *dp = NULL;
	sw_error_t err;

	/*
	 * Get destination port
	 */
	dp = ppe_drv_port_from_port_num(xmit_port);
	if (!dp) {
		ppe_drv_warn("%p: Couldn't get destination port for iface index %u", ptun, xmit_port);
		return false;
	}

	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID) {
		port_tnl_cfg.pppoe_en = (a_bool_t)PPE_DRV_TUN_FIELD_VALID;
	} else {
		port_tnl_cfg.l3_if.l3_if_valid = A_TRUE;
		port_tnl_cfg.l3_if.l3_if_index = tl_l3_if_idx;
	}

	/*
	 * Set MAC address of port on which tunnel is established.
	 */
	memcpy(port_tnl_cfg.mac_addr.uc, &l2_hdr->smac[0], ETH_ALEN);

	err = fal_tunnel_port_intf_set(PPE_DRV_SWITCH_ID, xmit_port, &port_tnl_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to set xmit port %d", ptun, xmit_port);
		return false;
	}

	return true;
}

/*
 *  ppe_drv_tun_detach_mapt_v4_to_v6
 *	delete mapt v4 flow with v6 outer tunnel context
 */
bool ppe_drv_tun_detach_mapt_v4_to_v6(struct ppe_drv_v4_conn *cn)
{
	struct ppe_drv_v4_conn_flow *pcf = &cn->pcf;
	struct ppe_drv_v4_conn_flow *pcr = &cn->pcr;
	struct ppe_drv_tun *ptun;

	ptun = ppe_drv_tun_mapt_port_tun_get(pcf->tx_port, pcf->rx_port);

	if (!ptun) {
		return false;
	}

	ppe_drv_flow_v4_detach_mapt_v6_conn(pcf);
	ppe_drv_flow_v4_detach_mapt_v6_conn(pcr);

	return true;
}

/*
 *  ppe_drv_tun_attach_mapt_v4_to_v6
 *	Attach mapt v4 flow with v6 outer tunnel context
 */
bool ppe_drv_tun_attach_mapt_v4_to_v6(struct ppe_drv_v4_conn *cn)
{
	uint8_t len_adjust;
	struct iphdr ip4 = {0};
	uint16_t sport, dport;
	uint32_t saddr_v6[4], daddr_v6[4];
	struct ppe_drv_v6_5tuple tuple;
	uint8_t tun_vp_port;
	struct net_device *netdev;
	struct ppe_drv_tun *ptun;
	struct ppe_drv_port *pp;
	struct ppe_drv_v6_conn_flow *pcf_v6, *pcr_v6;
	struct ppe_drv_v4_conn_flow *pcr = &cn->pcr;
	struct ppe_drv_v4_conn_flow *pcf = &cn->pcf;
	uint8_t flow_xmit_port = pcf->tx_port->port;
	uint8_t reverse_xmit_port = pcr->tx_port->port;
	bool is_flow_dir = true;

	ptun = ppe_drv_tun_mapt_port_tun_get(pcf->tx_port, pcf->rx_port);

	if (!ptun) {
		return false;
	}

	pp = ptun->pp;
	tun_vp_port = pp->port;
	netdev = pp->dev;

	len_adjust = PPE_DRV_TUN_MAPT_V6_LEN_ADJUST;

	if (tun_vp_port != flow_xmit_port) {
		if (tun_vp_port != reverse_xmit_port) {
			ppe_drv_trace("%p: ingress/egress interface is not MAP-T\n", pp);
			return false;
		}
		is_flow_dir = false;
	}

	if (is_flow_dir) {
		ip4.saddr = htonl(pcf->xlate_src_ip);
		ip4.daddr = htonl(pcf->xlate_dest_ip);
		sport = pcf->xlate_src_ident;
		dport = pcf->xlate_dest_ident;
	} else {
		ip4.saddr = htonl(pcf->xlate_dest_ip);
		ip4.daddr = htonl(pcf->xlate_src_ip);
		sport = pcf->xlate_dest_ident;
		dport = pcf->xlate_src_ident;
	}

	/*
	 * xlate_4_to_6 expects the arguments to be in network byte order(big endian).
	 * Source/Dest ip and port details in pcf are stored in little endian.
	 * So need to convert it before passing it as an argument.
	 */

	if (!(xlate_4_to_6(netdev, &ip4, htons(sport), htons(dport), saddr_v6, daddr_v6))) {
		ppe_drv_trace("%p: Could not find translation pair for v4 address\n", pp);
		return false;
	}

	/*
	 * The source and destination ip address received are in big endian format. Need to convert it
	 * to little endian before fiiling the 5 tuple to match the cn
	 */
	saddr_v6[0] = ntohl(saddr_v6[0]);
	saddr_v6[1] = ntohl(saddr_v6[1]);
	saddr_v6[2] = ntohl(saddr_v6[2]);
	saddr_v6[3] = ntohl(saddr_v6[3]);

	daddr_v6[0] = ntohl(daddr_v6[0]);
	daddr_v6[1] = ntohl(daddr_v6[1]);
	daddr_v6[2] = ntohl(daddr_v6[2]);
	daddr_v6[3] = ntohl(daddr_v6[3]);

	memcpy(tuple.flow_ip, saddr_v6, sizeof(saddr_v6));
	memcpy(tuple.return_ip, daddr_v6, sizeof(daddr_v6));
	tuple.flow_ident = sport;
	tuple.return_ident = dport;
	tuple.protocol = pcf->match_protocol;

	if (!ppe_drv_tun_v6_get_conn_flow(&tuple, &pcf_v6, &pcr_v6)) {
		ppe_drv_trace("%p: could not find flow v6 entry\n", pp);
		return false;
	}

	if (is_flow_dir) {
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcf, pcf_v6, len_adjust);
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcr, pcr_v6, len_adjust);
	} else {
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcr, pcf_v6, len_adjust);
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcf, pcr_v6, len_adjust);
	}

	return true;
}

/*
 *  ppe_drv_tun_mapt_translate_v6_to_v4
 *	get translated v6 pcf for a v4 pcf
 */
static bool ppe_drv_tun_mapt_translate_v6_to_v4(bool is_flow_dir, struct net_device *netdev, struct ppe_drv_v6_conn *conn_tun_v6,
						    struct ppe_drv_v4_conn_flow **pcf, struct ppe_drv_v4_conn_flow **pcr)
{
	struct ipv6hdr ip6;
	uint16_t sport, dport;
	uint32_t saddr_v4, daddr_v4;
	uint32_t saddr_v6[4], daddr_v6[4];
	struct ppe_drv_v4_5tuple tuple;
	struct ppe_drv_v4_conn *conn_v4;
	struct ppe_drv_v4_conn_flow *pcf_v4, *pcr_v4;
	struct ppe_drv_v6_conn_flow *pcf_v6 = &conn_tun_v6->pcf;
	struct ppe_drv_flow *flow;

	memset(&ip6, 0, sizeof(struct ipv6hdr));

	if (is_flow_dir) {
		ppe_drv_v6_conn_flow_match_src_ip_get(pcf_v6, saddr_v6);
		ppe_drv_v6_conn_flow_match_dest_ip_get(pcf_v6, daddr_v6);
		sport = pcf_v6->match_src_ident;
		dport = pcf_v6->match_dest_ident;
	} else {
		ppe_drv_v6_conn_flow_match_src_ip_get(pcf_v6, daddr_v6);
		ppe_drv_v6_conn_flow_match_dest_ip_get(pcf_v6, saddr_v6);
		sport = pcf_v6->match_dest_ident;
		dport = pcf_v6->match_src_ident;
	}

	/*
	 * xlate_6_to_4 expects the arguments to be in network byte order(big endian).
	 * Source/Dest ip and protocol details in pcf are stored in little endian.
	 * So need to convert it before passing it as an argument.
	 */
	ip6.saddr.in6_u.u6_addr32[0] = htonl(saddr_v6[0]);
	ip6.saddr.in6_u.u6_addr32[1] = htonl(saddr_v6[1]);
	ip6.saddr.in6_u.u6_addr32[2] = htonl(saddr_v6[2]);
	ip6.saddr.in6_u.u6_addr32[3] = htonl(saddr_v6[3]);

	ip6.daddr.in6_u.u6_addr32[0] = htonl(daddr_v6[0]);
	ip6.daddr.in6_u.u6_addr32[1] = htonl(daddr_v6[1]);
	ip6.daddr.in6_u.u6_addr32[2] = htonl(daddr_v6[2]);
	ip6.daddr.in6_u.u6_addr32[3] = htonl(daddr_v6[3]);

	if (!(xlate_6_to_4(netdev, &ip6, htonl(pcf_v6->match_protocol), &saddr_v4, &daddr_v4))) {
		ppe_drv_trace("%p: Could not find translation pair for v4 address\n", pcf_v6);
		return false;
	}
	tuple.flow_ip = ntohl(saddr_v4);
	tuple.return_ip = ntohl(daddr_v4);
	tuple.flow_ident = sport;
	tuple.return_ident = dport;
	tuple.protocol = pcf_v6->match_protocol;

	flow = ppe_drv_flow_v4_get(&tuple);
	if (!flow) {
		ppe_drv_trace("%p: flow not found for translated v4 address\n", pcf_v6);
		return false;
	}

	pcf_v4 = flow->pcf.v4;
	conn_v4 = ppe_drv_v4_conn_flow_conn_get(pcf_v4);
	pcr_v4 = (pcf_v4 == &conn_v4->pcf) ? &conn_v4->pcr : &conn_v4->pcf;

	*pcf = pcf_v4;
	*pcr = pcr_v4;

	return true;
}

/*
 *  ppe_drv_tun_detach_mapt_v6_to_v4
 *	delete mapt v6 tunnel context in v4 inner flow
 */
static bool ppe_drv_tun_detach_mapt_v6_to_v4(struct ppe_drv_v6_conn *conn_tun_v6)
{
	struct ppe_drv_tun *ptun;
	struct ppe_drv_port *pp;
	uint8_t tun_vp_port;
	struct net_device *netdev;
	struct ppe_drv_v4_conn_flow *pcf_v4 = NULL, *pcr_v4 = NULL;
	struct ppe_drv_v6_conn_flow *pcf = &conn_tun_v6->pcf;
	struct ppe_drv_v6_conn_flow *pcr = &conn_tun_v6->pcr;
	uint8_t flow_xmit_port = pcf->tx_port->port;
	uint8_t reverse_xmit_port = pcr->tx_port->port;
	bool is_flow_dir = true;

	ptun = ppe_drv_tun_mapt_port_tun_get(pcf->tx_port, pcf->rx_port);
	if (!ptun) {
		ppe_drv_trace("invalid tunnel to detach\n");
		return false;
	}

	pp = ptun->pp;
	tun_vp_port = pp->port;
	netdev = pp->dev;

	if (tun_vp_port != flow_xmit_port) {
		if (tun_vp_port != reverse_xmit_port) {
			ppe_drv_trace("%p: ingress/egress interface is not MAP-T\n", ptun);
			return false;
		}
		is_flow_dir = false;
	}
	if (!ppe_drv_tun_mapt_translate_v6_to_v4(is_flow_dir, netdev, conn_tun_v6, &pcf_v4, &pcr_v4)) {
		ppe_drv_trace("%p: cant find pcf v4 corresponding to pcf v6 \n", pp);
		return false;
	}
	ppe_drv_flow_v4_detach_mapt_v6_conn(pcf_v4);
	ppe_drv_flow_v4_detach_mapt_v6_conn(pcr_v4);

	return true;
}

/*
 *  ppe_drv_tun_attach_mapt_v6_to_v4
 *	Attach mapt v6 tunnel context with v4 inner flow
 */
bool ppe_drv_tun_attach_mapt_v6_to_v4(struct ppe_drv_v6_conn *conn_tun_v6)
{
	uint8_t len_adjust;
	struct ppe_drv_tun *ptun;
	struct ppe_drv_port *pp;
	uint8_t tun_vp_port;
	struct net_device *netdev;
	struct ppe_drv_v4_conn_flow *pcf_v4 = NULL, *pcr_v4 = NULL;
	struct ppe_drv_v6_conn_flow *pcf = &conn_tun_v6->pcf;
	struct ppe_drv_v6_conn_flow *pcr = &conn_tun_v6->pcr;
	uint8_t flow_xmit_port = pcf->tx_port->port;
	uint8_t reverse_xmit_port = pcr->tx_port->port;
	bool is_flow_dir = true;

	ptun = ppe_drv_tun_mapt_port_tun_get(pcf->tx_port, pcf->rx_port);

	if (!ptun) {
		return false;
	}

	pp = ptun->pp;
	tun_vp_port = pp->port;
	netdev = pp->dev;

	len_adjust = PPE_DRV_TUN_MAPT_V6_LEN_ADJUST;

	if (tun_vp_port != flow_xmit_port) {
		if (tun_vp_port != reverse_xmit_port) {
			ppe_drv_trace("%p: ingress/egress interface is not MAP-T\n", ptun);
			return false;
		}
		is_flow_dir = false;
	}

	if (!ppe_drv_tun_mapt_translate_v6_to_v4(is_flow_dir, netdev, conn_tun_v6, &pcf_v4, &pcr_v4)) {
		ppe_drv_trace("%p: cant find pcf v4 corresponding to pcf v6 \n", pp);
		return false;
	}

	if (is_flow_dir) {
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcf_v4, pcf, len_adjust);
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcr_v4, pcr, len_adjust);
	} else {
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcr_v4, pcf, len_adjust);
		ppe_drv_flow_v4_attach_mapt_v6_conn(pcf_v4, pcr, len_adjust);
	}

	return true;
}

/*
 *  ppe_drv_tun_v6_conn_tun_del
 *	delete all connections corresponding to tunnel
 */
void ppe_drv_tun_v6_conn_tun_del(struct ppe_drv_tun *ptun)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v6_conn_flow *pcf, *pcr;
	struct ppe_drv_v6_conn *cn, *cn_tmp;
	struct ppe_drv_tun *cn_ptun;
	struct ppe_drv_v6_conn_sync *cns;

	cns = ppe_drv_v6_conn_stats_alloc();
	if (!cns) {
		ppe_drv_warn("%p: memory allocation failed for v6 connection sync", p);
		return;
	}

	list_for_each_entry_safe(cn, cn_tmp,  &p->conn_tun_v6, list) {
		pcf = &cn->pcf;
		pcr = &cn->pcr;

		cn_ptun = ppe_drv_port_tun_get(pcf->tx_port) ? ppe_drv_port_tun_get(pcf->tx_port) : ppe_drv_port_tun_get(pcf->rx_port);

		if (cn_ptun !=  ptun) {
			continue;
		}

		/*
		 * Release references on interfaces.
		 */
		ppe_drv_v6_if_walk_release(pcf);

		/*
		 * Release references on interfaces.
		 */
		ppe_drv_v6_if_walk_release(pcr);

		/*
		 * Detach v6 tun pcf from v4 flow before deleting cn from the list for MAPT
		 */
		if (cn_ptun->th.type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
			if (!ppe_drv_tun_detach_mapt_v6_to_v4(cn)) {
				ppe_drv_warn("%p: MAP-T v6 to v4 detach failed", p);
			}
		}

		list_del(&cn->list);

		/*
		 * Capture remaining stats.
		 */
		ppe_drv_v6_conn_sync_one(cn, cns, PPE_DRV_STATS_SYNC_REASON_FLUSH);

		ppe_drv_v6_conn_free(cn);

		/*
		 * Release the reference taken during activation
		 */
		ppe_drv_tun_deref(cn_ptun);
	}

	ppe_drv_v6_conn_stats_free(cns);

	return;
}

/*
 *  ppe_drv_tun_v4_conn_tun_del
 *	Delete all instances of connections corresponding to tunnel
 */
void ppe_drv_tun_v4_conn_tun_del(struct ppe_drv_tun *ptun)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_v4_conn_flow *pcf, *pcr;
	struct ppe_drv_v4_conn *cn, *tmp_cn;
	struct ppe_drv_tun *cn_ptun;
	struct ppe_drv_v4_conn_sync *cns;

	cns = ppe_drv_v4_conn_stats_alloc();
	if (!cns) {
		ppe_drv_warn("%p: memory allocation failed for v4 connection sync", p);
		return;
	}

	list_for_each_entry_safe(cn, tmp_cn, &p->conn_tun_v4, list) {
		pcf = &cn->pcf;
		pcr = &cn->pcr;

		cn_ptun = ppe_drv_port_tun_get(pcf->tx_port) ? ppe_drv_port_tun_get(pcf->tx_port) : ppe_drv_port_tun_get(pcf->rx_port);

		if (cn_ptun !=  ptun) {
			continue;
		}

		/*
		 * Release references on interfaces.
		 */
		ppe_drv_v4_if_walk_release(pcf);

		/*
		 * Release references on interfaces.
		 */
		ppe_drv_v4_if_walk_release(pcr);

		list_del(&cn->list);

		/*
		 * Capture remaining stats.
		 */
		ppe_drv_v4_conn_sync_one(cn, cns, PPE_DRV_STATS_SYNC_REASON_FLUSH);

		ppe_drv_v4_conn_free(cn);

		/*
		 * Release the reference taken during activation
		 */
		ppe_drv_tun_deref(cn_ptun);
	}

	ppe_drv_v4_conn_stats_free(cns);

	return;
}

/*
 * ppe_drv_tun_deactivate
 *	Deactivate PPE tunnel
 */
bool ppe_drv_tun_deactivate(uint16_t port_num, void *vdestroy_rule)
{
	ppe_drv_ret_t ret = PPE_DRV_RET_SUCCESS;
	struct ppe_drv_tun_cmn_ctx *pth;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp;
	struct ppe_drv_tun *ptun;
	struct ppe_drv_v4_conn_sync *cns_v4 = NULL;
	struct ppe_drv_v6_conn_sync *cns_v6 = NULL;
	struct ppe_drv_v6_conn *cn_v6 = NULL;
	struct ppe_drv_v4_conn *cn_v4 = NULL;
	bool is_ipv6;

	spin_lock_bh(&p->lock);

	pp = ppe_drv_port_from_port_num(port_num);
	if (!pp) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Invalid port number %d", p, port_num);
		return false;
	}

	ptun = ppe_drv_port_tun_get(pp);
	if (!ptun) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel not found for port %d", p, port_num);
		return false;
	}

	pth = &ptun->th;
	is_ipv6 = ppe_drv_tun_cmn_ctx_tun_is_ipv6(pth);

	if (vdestroy_rule && is_ipv6) {
		ret = ppe_drv_v6_tun_del_ce_validate(vdestroy_rule, &cns_v6, &cn_v6);
		if (ret != PPE_DRV_RET_SUCCESS) {
			spin_unlock_bh(&p->lock);
			return false;
		}

		/*
		 * Detach MAPT v6 tun pcf from v4 flow pcf  added to capture MAP-T stats
		 */
		if (ptun->th.type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
			if (!ppe_drv_tun_detach_mapt_v6_to_v4(cn_v6)) {
				ppe_drv_warn("%p: MAP-T v6 from v4 detach failed", p);
			}
		}

		/*
		 * Remove connection entry from the active connection list.
		 */
		list_del(&cn_v6->list);

		/*
		 * Capture remaining stats.
		 */
		ppe_drv_v6_conn_sync_one(cn_v6, cns_v6, PPE_DRV_STATS_SYNC_REASON_DESTROY);
	} else if (vdestroy_rule) {
		ret = ppe_drv_v4_tun_del_ce_validate(vdestroy_rule, &cns_v4, &cn_v4);
		if (ret != PPE_DRV_RET_SUCCESS) {
			spin_unlock_bh(&p->lock);
			return false;
		}

		/*
		 * Remove connection entry from the active connection list.
		 */
		list_del(&cn_v4->list);
		/*
		 * Capture remaining stats.
		 */
		ppe_drv_v4_conn_sync_one(cn_v4, cns_v4, PPE_DRV_STATS_SYNC_REASON_DESTROY);
	}

	/*
	 * If the flow count is not zero decrement the flow count taken and return
	 * Do not deactivate the tunnel since other flows are still in use.
	 */
	if (vdestroy_rule && !atomic_dec_and_test(&ptun->flow_count)) {
		ppe_drv_info("%p: Flow count is %d , skipping deactivation", ptun, atomic_read(&ptun->flow_count));
		goto skip_tunnel_deactivation;
	}

	/*
	 * For MAP-T cases eg edit rule table is used for encapsulation
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		if (!ppe_drv_tun_deactivate_mapt(ptun)) {
			spin_unlock_bh(&p->lock);
			return false;
		}

		goto disable_encap;
	}

	ppe_drv_trace("%p: Deactivating Tunnel %d at index %u", ptun, pth->type, ptun->tun_idx);

	/*
	 * Disable decapsulation
	 */
	if (!ppe_drv_tun_decap_disable(ptun->ptdc)) {
		ppe_drv_assert(false, "%p: Decap reset failed for deactivating Tunnel %d at index %u", ptun,
				pth->type, ptun->tun_idx);
		goto error;
	}

disable_encap:
	/*
	 * Disable encapsulation
	 */
	if (!ppe_drv_tun_port_encap_disable(pp)) {
		ppe_drv_assert(false, "%p: Encap reset failed for deactivating Tunnel %d at index %u", ptun,
						pth->type, ptun->tun_idx);
		goto error;
	}

	/*
	 * Reset phyisical port associated with tunnel port
	 */
	if (!ppe_drv_tun_port_reset_physical_port(pp)) {
		ppe_drv_assert(false, "%p: Physical port reset failed for  deactivating Tunnel %d at index %u",
				ptun, pth->type, ptun->tun_idx);
		goto error;
	}

	/*
	 * Detach reference for tl_l3_if index; reference is taken during activate
	 */
	ppe_drv_tun_l3_if_deref(ptun->pt_l3_if);
	ptun->pt_l3_if = NULL;

	/*
	 * Reset encap entry association with virtual port
	 */
	if (!ppe_drv_tun_encap_tun_idx_configure(ptun->ptec, ptun->vp_num, false)) {
		ppe_drv_assert(false, "%p: Failed resetting encap index for Tunnel %d at index %u", ptun,
						pth->type, ptun->tun_idx);
		goto error;
	}

	/*
	 * Delete FSE entry created for GRETAP tunnel if endpoint is a DS VP
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP &&
			ppe_drv_tun_is_dest_port_wifi(ptun->xmit_port)) {
		if (vdestroy_rule && is_ipv6) {
			if (!ppe_drv_tun_v6_fse_entry_del(&cn_v6->pcf, &cn_v6->pcr)) {
				goto error;
			}
		} else if (vdestroy_rule){
			if (!ppe_drv_tun_v4_fse_entry_del(&cn_v4->pcf, &cn_v4->pcr)) {
				goto error;
			}
		}
	}

skip_tunnel_deactivation:
	/*
	 * Delete all the instances of tunnel stored in cn list
	 * and release acquired references
	 */
	if (!vdestroy_rule && is_ipv6) {
		ppe_drv_tun_v6_conn_tun_del(ptun);
	} else if (!vdestroy_rule){
		ppe_drv_tun_v4_conn_tun_del(ptun);
	} else {
		/*
		 * Release reference if the deactivate
		 * call is from ECM
		 */
		ppe_drv_tun_deref(ptun);
	}

	spin_unlock_bh(&p->lock);

	/*
	 *  Invoke callback and free the cns structure
	 */
	if (cns_v4) {
		ppe_drv_v4_conn_stats_sync_invoke_cb(cns_v4);
		ppe_drv_v4_conn_stats_free(cns_v4);
	} else if (cns_v6) {
		ppe_drv_v6_conn_stats_sync_invoke_cb(cns_v6);
		ppe_drv_v6_conn_stats_free(cns_v6);
	}

	ppe_drv_v6_conn_free(cn_v6);
	ppe_drv_v4_conn_free(cn_v4);

	return true;

error:
	spin_unlock_bh(&p->lock);
	ppe_drv_v4_conn_stats_free(cns_v4);
	ppe_drv_v6_conn_stats_free(cns_v6);

	return false;
}
EXPORT_SYMBOL(ppe_drv_tun_deactivate);

/*
 * ppe_drv_tun_vxlan_deconfigure
 *	Disable VxLAN PPE tunnel
 */
void ppe_drv_tun_vxlan_deconfigure(struct ppe_drv *p)
{
	fal_tunnel_udp_entry_t ftue = {0};
	fal_vxlan_type_t type = FAL_VXLAN;
	sw_error_t err;

	ftue.ip_ver = FAL_TUNNEL_IP_VER_V4;
	ftue.udp_type = FAL_TUNNEL_L4_TYPE_UDP;
	ftue.l4_port_type = FAL_TUNNEL_L4_PORT_TYPE_DST;
	ftue.l4_port = IANA_VXLAN_UDP_PORT;
	err = fal_vxlan_entry_del(PPE_DRV_SWITCH_ID, type, &ftue);
	if (err != SW_OK) {
		ppe_drv_warn("%p VXLAN: failed to delete UDP entry for IPV4 %d", p, err);
	}

	ftue.ip_ver = FAL_TUNNEL_IP_VER_V6;
	err = fal_vxlan_entry_del(PPE_DRV_SWITCH_ID, type, &ftue);
	if (err != SW_OK) {
		ppe_drv_warn("%p VXLAN: failed to delete UDP entry for IPV6 %d", p, err);
	}
}

/*
 * ppe_drv_tun_deconfigure
 *	Disable PPE tunnel
 */
bool ppe_drv_tun_deconfigure(uint16_t port_num)
{
	struct ppe_drv_tun_cmn_ctx *pth = NULL;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun *ptun;
	struct ppe_drv_port *pp;

	spin_lock_bh(&p->lock);

	pp = ppe_drv_port_from_port_num(port_num);
	if (!pp) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Invalid port number %d", port_num);
		return false;
	}

	ptun = ppe_drv_port_tun_get(pp);
	if (!ptun) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("Tunnel not found for port %d", port_num);
		return false;
	}

	pth = &ptun->th;
	ppe_drv_trace("%p: Destroying Tunnel %d at index %u", ptun, pth->type, ptun->tun_idx);

	/*
	 * Release reference
	 */
	ppe_drv_tun_deref(ptun);
	spin_unlock_bh(&p->lock);
	return true;
}
EXPORT_SYMBOL(ppe_drv_tun_deconfigure);

/*
 * ppe_drv_tun_decap_disable_by_port_num
 *	Disable tunnel decapsulation
 */
bool ppe_drv_tun_decap_disable_by_port_num(uint16_t port_num)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp;
	struct ppe_drv_tun *ptun;

	pp = ppe_drv_port_from_port_num(port_num);
	if (!pp) {
		ppe_drv_warn("%p: Invalid port number %d", p, port_num);
		return false;
	}

	ptun = pp->port_tun;
	if (!ptun) {
		ppe_drv_warn("%p: Failed to disable decap. Decap entry not present for port num:%d", p, port_num);
		return false;
	}

	return ppe_drv_tun_decap_disable(ptun->ptdc);
}
EXPORT_SYMBOL(ppe_drv_tun_decap_disable_by_port_num);

/*
 * ppe_drv_tun_decap_enable_by_port_num
 *	Enable tunnel decapsulation
 */
bool ppe_drv_tun_decap_enable_by_port_num(uint16_t port_num)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_port *pp;
	struct ppe_drv_tun *ptun;

	pp = ppe_drv_port_from_port_num(port_num);
	if (!pp) {
		ppe_drv_warn("%p: Invalid port number %d", p, port_num);
		return false;
	}

	ptun = pp->port_tun;
	if (!ptun) {
		ppe_drv_warn("%p: Failed to enable decap. Decap entry not present for port num:%d", p, port_num);
		return false;
	}

	return ppe_drv_tun_decap_enable(ptun->ptdc);
}
EXPORT_SYMBOL(ppe_drv_tun_decap_enable_by_port_num);

/*
 * ppe_drv_tun_activate
 *	Activate PPE tunnel
 */
bool ppe_drv_tun_activate(uint16_t port_num, void *vcreate_rule)
{
	struct ppe_drv_comm_stats *comm_stats;
	struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr;
	struct ppe_drv_v4_conn *cn_v4 = NULL;
	struct ppe_drv_v6_conn *cn_v6 = NULL;
	struct ppe_drv_pppoe *pppoe = NULL;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_cmn_ctx *pth;
	bool dc_cfg_status = false;
	struct ppe_drv_tun *ptun;
	struct ppe_drv_port *pp;
	uint16_t tl_l3_if_idx;
	fal_port_t port_id;
	uint16_t xmit_port;
	bool is_ipv6;
	bool status;

	comm_stats = &p->stats.comm_stats[PPE_DRV_CONN_TYPE_TUNNEL];
	spin_lock_bh(&p->lock);

	pp = ppe_drv_port_from_port_num(port_num);
	if (!pp) {
		ppe_drv_warn("%p: invalid port number %d", p, port_num);
		goto err_fail;
	}

	ptun = ppe_drv_port_tun_get(pp);
	if (!ptun) {
		ppe_drv_warn("%p: tunnel not found for port %d", p, port_num);
		goto err_fail;
	}

	if (!ptun->ptec) {
		ppe_drv_warn("%p: tun encap is not initialized properly", ptun);
		goto err_fail;
	}

	/*
	 * ptdc is not used for MAP-T
	 */

	pth = &ptun->th;
	if ((pth->type != PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) && (!ptun->ptdc)) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: tun is not initialized properly", ptun);
		return false;
	}

	l2_hdr = &pth->l2;
	is_ipv6 = ppe_drv_tun_cmn_ctx_tun_is_ipv6(pth);

	/*
	 * there are two ways tunnel can be configured.
	 * a. when ECM is front end, vcreate_rule is filled during outer rule push and we need
	 *    to extract the L2 parameter from it.
	 * b. the L2 parameter can be configured from user when ECM is not present.
	 */
	if (vcreate_rule && is_ipv6) {
		cn_v6 = ppe_drv_v6_conn_alloc();
		if (!cn_v6) {
			ppe_drv_stats_inc(&comm_stats->v6_create_fail_mem);
			ppe_drv_warn("%p: failed to allocate connection memory: %p", p, vcreate_rule);
			goto err_fail;
		}

		if (ppe_drv_v6_tun_add_ce_validate(vcreate_rule, cn_v6)) {
			goto err_fail;
		}

		/*
		 * Extract the L2 HDR from ECM rule
		 */
		ppe_drv_tun_v6_parse_l2_hdr(vcreate_rule, cn_v6, l2_hdr);
	} else if (vcreate_rule) {
		cn_v4 = ppe_drv_v4_conn_alloc();
		if (!cn_v4) {
			ppe_drv_stats_inc(&comm_stats->v4_create_fail_mem);
			ppe_drv_warn("%p: failed to allocate connection memory: %p", p, vcreate_rule);
			goto err_fail;
		}

		if (ppe_drv_v4_tun_add_ce_validate(vcreate_rule, cn_v4)) {
			goto err_fail;
		}

		/*
		 * Extract the L2 HDR from ECM Connection entry
		 */
		ppe_drv_tun_v4_parse_l2_hdr(vcreate_rule, cn_v4, l2_hdr);
	}

	/*
	 * In case of MAPT we can expect multiple create connection request for same tunnel.
	 * In this case we dont need to configure the tunnel again. for rest of the tunnel
	 * it is expected to get create conection request only once
	 */
	if (atomic_read(&ptun->flow_count)) {
		goto skip_tunnel_activation;
	}

	port_id = ptun->vp_num;

	/*
	 * 1. Program EG Header Data table
	 * 2. Program EG tunnel control table
	 * 3. Program EG VP table
	 */
	if (!ppe_drv_tun_encap_configure(ptun->ptec, pth, l2_hdr)) {
		ppe_drv_warn("%p: Failed to do encap configure for tun %d of type %d", ptun, ptun->tun_idx,
								pth->type);
		goto err_fail;
	}

	if (!ppe_drv_tun_encap_tun_idx_configure(ptun->ptec, ptun->vp_num, true)) {
		ppe_drv_warn("%p: Failed to do encap tun idx for tun %d of type %d", ptun, ptun->tun_idx,
								pth->type);
		goto err_fail;
	}

	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		/*
		 * Program EG_EDIT RULE table
		 */
		status = ppe_drv_tun_encap_xlate_rule_configure(ptun->ptecxr, &pth->tun.mapt.remote, ppe_drv_tun_encap_get_len(ptun->ptec), l2_hdr->flags, true);
		if (!status) {
			ppe_drv_warn("%p: Failed to configure encap xlate map table", ptun);
			goto err_fail;
		}
	}

	/*
	 * tunnel Counter configuration is already set during port alloc time
	 *
	 * TODO: Need API to enable VSI_TAG Mode on EG_VP_TBL
	 * Check if EG_L3_IF table needs to be programmed
	 *
	 * TODO: Need to add PPPOE specific handling
	 */

	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID) {
		pppoe = ppe_drv_pppoe_find_session(ntohs(l2_hdr->pppoe.ph.sid), l2_hdr->pppoe.server_mac);
		if (!pppoe) {
			ppe_drv_warn("%p: Could not find pppoe session %x mac %pM", ptun, ntohs(l2_hdr->pppoe.ph.sid), l2_hdr->pppoe.server_mac);
			goto err_fail;
		}
	}

	xmit_port = ppe_drv_tun_xmit_port_get(l2_hdr->xmit_port);

	if (xmit_port == PPE_DRV_PORT_CPU) {
		/*
		 * CPU port is not initialized so tl_l3_if get would fail.
		 * Override xmit_port with destination port number in this case.
		 * This condition is valid for cases where xmit_port is wifi/ds vp.
		 */
		xmit_port = l2_hdr->xmit_port;
	}

	/*
	 * Get the tl_l3_if_index;
	 */

	if (pppoe) {
		ptun->pt_l3_if = ppe_drv_tun_pppoe_tl_l3_if_get(ptun, pppoe);
	} else {
		ptun->pt_l3_if = ppe_drv_tun_port_tl_l3_if_get(ptun, xmit_port);
	}

	if (ptun->pt_l3_if == NULL) {
		ppe_drv_warn("%p: Failed to get active tl l3 index for tun %d of type %d",
					ptun, ptun->tun_idx, pth->type);
		goto err_fail;
	}

	tl_l3_if_idx = ppe_drv_tun_l3_if_get_index(ptun->pt_l3_if);

	/*
	 * Set TL_L3_IDX and transmit mac address in TL_PORT_VP_TBL
	 */
	ppe_drv_tun_decap_xmitport_cfg_set(ptun, xmit_port, l2_hdr, tl_l3_if_idx);

	if (pth->type != PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		ppe_drv_tun_decap_set_tl_l3_idx(ptun->ptdc, tl_l3_if_idx);
		dc_cfg_status = ppe_drv_tun_decap_activate(ptun->ptdc, l2_hdr);
	} else {
		dc_cfg_status = ppe_drv_tun_activate_mapt(ptun, l2_hdr);
	}

	if (!dc_cfg_status) {
		ppe_drv_warn("%p: Failed to activate decap entry for tun %d of type %d", ptun, ptun->tun_idx, pth->type);
		goto err_fail;
	}

	/*
	 * Activate tunnel in L2_VP_TBL
	 */
	if (!ppe_drv_tun_port_configure(ptun, xmit_port)) {
		ppe_drv_warn("%p: Failed to configure VP tunnel port for tun %d of type %d", ptun, ptun->tun_idx, pth->type);
		goto err_fail;
	}

	ptun->xmit_port = xmit_port;

	/*
	 * For GRETAP tunnel endpoint on DS port push a 3 tuple
	 * FSE entry. For now only GRETAP is supported and validated
	 * can be extended for other tunnels in future
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP &&
			ppe_drv_tun_is_dest_port_wifi(xmit_port)) {
		if (vcreate_rule && is_ipv6) {
			if (!ppe_drv_tun_v6_fse_entry_add(vcreate_rule, &cn_v6->pcf, &cn_v6->pcr)) {
				goto err_fail;
			}
		} else if (vcreate_rule){
			if (!ppe_drv_tun_v4_fse_entry_add(vcreate_rule, &cn_v4->pcf, &cn_v4->pcr)) {
				goto err_fail;
			}
		}
	}

skip_tunnel_activation:
	/*
	 * Take reference
	 */
	ppe_drv_tun_ref(ptun);

	/*
	 * Increment flow count as connection is added to the list
	 */
	atomic_inc(&ptun->flow_count);

	if (cn_v6) {
		list_add(&cn_v6->list, &p->conn_tun_v6);
		if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
			ppe_drv_v6_conn_flags_set(cn_v6, PPE_DRV_V6_CONN_FLAG_TYPE_MAPT);

			if (!ppe_drv_tun_attach_mapt_v6_to_v4(cn_v6)) {
				ppe_drv_trace("%p: MAP-T v6 to v4 attach failed", ptun);
			}
		}
	} else if (cn_v4) {
		list_add(&cn_v4->list, &p->conn_tun_v4);
	}

	spin_unlock_bh(&p->lock);
	return true;

err_fail:
	spin_unlock_bh(&p->lock);
	kfree(cn_v4);
	kfree(cn_v6);
	return false;
}
EXPORT_SYMBOL(ppe_drv_tun_activate);

/*
 * ppe_drv_tun_configure
 *	Allocate PPE tunnel instance and initialize objects
 */
bool ppe_drv_tun_configure(uint16_t port_num, struct ppe_drv_tun_cmn_ctx *pth, void *add_cb, void *del_cb)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun *ptun = NULL;
	uint16_t decap_hwidx = PPE_DRV_TUN_DECAP_INVALID_IDX;
	uint8_t rule_id;

	struct ppe_drv_port *pp = ppe_drv_port_from_port_num(port_num);
	if (!pp) {
		ppe_drv_warn("%p: Invalid port number %d", p, port_num);
		return false;
	}

	ppe_drv_assert(pth->type < PPE_DRV_TUN_CMN_CTX_TYPE_MAX, "%p: unknown tunnel type %u", p, pth->type);

	/*
	 * Check if the tunnel type is MAP-T, it requires separate tables
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		ptun =  ppe_drv_tun_mapt_alloc(p, pp, pth, add_cb, del_cb);
		if (!ptun) {
			ppe_drv_warn("%p: Couldn't allocate mapt tables", p);
			return false;
		}

		return true;
	}

	/*
	 * Check and try allocating tunnel instance corresponding to tun_idx
	 * on allocation one reference would be taken
	 */
	ptun = kzalloc(sizeof(struct ppe_drv_tun), GFP_ATOMIC);
	if (!ptun) {
		ppe_drv_warn("%p: Couldn't allocate tun memory", p);
		return false;
	}

	kref_init(&ptun->ref);

	memcpy(&ptun->th, pth, sizeof(*pth));
	ptun->pp = pp;
	ptun->add_cb = add_cb;
	ptun->del_cb = del_cb;
	ptun->vp_num = ppe_drv_port_num_get(pp);

	/*
	 * Inbound tunnel packets are processed through decap TL_TBL, get an instance.
	 */
	spin_lock_bh(&p->lock);
	ptun->ptdc = ppe_drv_tun_decap_alloc(p);
	if (!ptun->ptdc) {
		ppe_drv_warn("%p: Failed to get decap instance", ptun);
		goto err_exit;
	}

	/*
	 * Need to get the Hw index first for decap operation before allocating any other entries
	 */
	decap_hwidx = ppe_drv_tun_decap_configure(ptun->ptdc, pp, pth);
	if (decap_hwidx == PPE_DRV_TUN_DECAP_INVALID_IDX) {
		ppe_drv_warn("%p: Failed to allocate decap hw entry idx", ptun);
		goto err_exit;
	}

	/*
	 * Set the TL table index in HW table
	 */
	ppe_drv_tun_decap_set_tl_index(ptun->ptdc, decap_hwidx);

	/*
	 * Request instances of encap tables
	 */
	ptun->ptec = ppe_drv_tun_encap_alloc(p);
	if (!ptun->ptec) {
		ppe_drv_warn("%p: couldn't get encap index", ptun);
		goto err_exit;
	}

	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2) {
		if (p->tun_gbl.tun_l2tp.l2tp_encap_rule == NULL) {
			/*
			 * Alloc encap EG table entry for L2TP
			 * Alloc is called for first instance of l2tp tunnel only.
			 */
			 p->tun_gbl.tun_l2tp.l2tp_encap_rule = ppe_drv_tun_encap_xlate_rule_alloc(p);
			 if (p->tun_gbl.tun_l2tp.l2tp_encap_rule == NULL) {
				ppe_drv_warn("%p: couldn't get encap rule entry index for l2tp", p);
				goto err_exit;
			}
		} else {
			/*
			 * Take ref on encap rule instance if another L2TP tunnel is already active.
			 */
			ppe_drv_tun_encap_xlate_rule_ref(p->tun_gbl.tun_l2tp.l2tp_encap_rule);
		}

		ptun->ptecxr = p->tun_gbl.tun_l2tp.l2tp_encap_rule;
		rule_id = ppe_drv_tun_encap_xlate_rule_get_index(ptun->ptecxr);
		ppe_drv_tun_encap_set_rule_id(ptun->ptec, rule_id);

		/*
		 * encap header control configuration for L2TP
		 * protomap[1] and protomap[3] are used for ipv4 protocol
		 * and ipv6 protocol update in PPP header
		 */
		if (!ppe_drv_tun_encap_hdr_ctrl_l2tp_configure(p, ptun)) {
			ppe_drv_warn("%p L2TP: failed to configure encap header control", p);
			goto err_exit;
		}
	}

	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN) {
		/*
		 * encap header control configuration for VXLAN.
		 * UDP source port value is updated with a random value
		 */
		if (!ppe_drv_tun_encap_hdr_ctrl_vxlan_configure(p, ptun)) {
			ppe_drv_warn("%p VXLAN: failed to configure encap header control", p);
			goto err_exit;
		}
	}

	ptun->ptec->port = pp;

	ppe_drv_port_tun_set(pp, ptun);

	spin_unlock_bh(&p->lock);
	ppe_drv_trace("%p: tun context with tun_idx %u of type %u created", ptun, ptun->tun_idx, pth->type);

	return true;

err_exit:
	ppe_drv_tun_deref(ptun);
	spin_unlock_bh(&p->lock);
	return false;
}
EXPORT_SYMBOL(ppe_drv_tun_configure);

/*
 * ppe_drv_tun_configure_vxlan_and_dport
 *	Configure the VXLAN destination port.
 */
bool ppe_drv_tun_configure_vxlan_and_dport(uint16_t dport)
{
	fal_tunnel_udp_entry_t ftue = {0};
	fal_vxlan_type_t type = FAL_VXLAN;
	sw_error_t err;

	ppe_drv_gbl.vxlan_dport = dport;

	/*
	 * VxLAN Decap port number match for IPV4 tunnel.
	 */
	ftue.ip_ver = FAL_TUNNEL_IP_VER_V4;
	ftue.udp_type = FAL_TUNNEL_L4_TYPE_UDP;
	ftue.l4_port_type = FAL_TUNNEL_L4_PORT_TYPE_DST;
	ftue.l4_port = dport;
	err = fal_vxlan_entry_add(PPE_DRV_SWITCH_ID, type, &ftue);
	if (err != SW_OK) {
		ppe_drv_warn("%p failed to add UDP entry for IPV4. err: %d", &ftue, err);
		return false;
	}

	/*
	 * VxLAN Decap port number match for IPV6 tunnel.
	 */
	ftue.ip_ver = FAL_TUNNEL_IP_VER_V6;
	err = fal_vxlan_entry_add(PPE_DRV_SWITCH_ID, type, &ftue);
	if (err != SW_OK) {
		ppe_drv_warn("%p failed to add UDP entry for IPV6. err: %d", &ftue, err);
		return false;
	}

	return true;
}
EXPORT_SYMBOL(ppe_drv_tun_configure_vxlan_and_dport);

/*
 * ppe_drv_tun_global_init
 *	Initialize tables with values that does not require changes
 */
bool ppe_drv_tun_global_init(struct ppe_drv *p)
{
	sw_error_t err;
	fal_tunnel_decap_key_t ptdkcfg =  {0};
	fal_tunnel_global_cfg_t ptglcfg =  {0};
	fal_mapt_decap_ctrl_t ptmapglcfg = {0};
	fal_tunnel_type_t tunnel_type;

	spin_lock_bh(&p->lock);

	/*
	 * GRETAP IPv4/6 key configuration
	 */
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_SIP_EN);
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_DIP_EN);
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_L4PROTO_EN);
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_TLINFO_EN);
	ptdkcfg.tunnel_info_mask = FAL_TUNNEL_DECAP_TUNNEL_INFO_MASK;

	tunnel_type = FAL_TUNNEL_TYPE_GRE_TAP_OVER_IPV4;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for GREIPV4", p);
		return false;
	}

	tunnel_type = FAL_TUNNEL_TYPE_GRE_TAP_OVER_IPV6;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for GREIPV6", p);
		return false;
	}

	/*
	 * VxLAN IPv4/6 key configuration
	 */
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_DPORT_EN);
	tunnel_type = FAL_TUNNEL_TYPE_VXLAN_OVER_IPV4;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for VxlanIPV4", p);
		return false;
	}

	tunnel_type = FAL_TUNNEL_TYPE_VXLAN_OVER_IPV6;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for VxlanIPV6", p);
		return false;
	}

	/*
	 * VxLAN GPE IPv4/6 key configuration
	 * TODO: Check if additional flags are needed for GPE
	 */
	tunnel_type = FAL_TUNNEL_TYPE_VXLAN_GPE_OVER_IPV4;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for VxlanGPEIPV4", p);
		return false;
	}

	tunnel_type = FAL_TUNNEL_TYPE_VXLAN_GPE_OVER_IPV6;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for VxlanGPEIPV6", p);
		return false;
	}

	/*
	 * IPv4 over IPv6
	 */
	ptdkcfg.key_bmp = 0;
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_SIP_EN);
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_DIP_EN);
	ptdkcfg.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_L4PROTO_EN);

	tunnel_type = FAL_TUNNEL_TYPE_IPV4_OVER_IPV6;
	err = fal_tunnel_decap_key_set(PPE_DRV_SWITCH_ID, tunnel_type, &ptdkcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Decap key set failure for IPIP6", p);
		return false;
	}

	/*
	 * Initialize Tunnel global configuration
	 */
	ptglcfg.deacce_action = FAL_MAC_RDT_TO_CPU;
	ptglcfg.src_if_check_action = FAL_MAC_RDT_TO_CPU;
	ptglcfg.src_if_check_deacce_en = A_TRUE;
	ptglcfg.vlan_check_action = FAL_MAC_RDT_TO_CPU;
	ptglcfg.vlan_check_deacce_en = A_TRUE;
	ptglcfg.udp_csum_zero_action = FAL_MAC_RDT_TO_CPU;
	ptglcfg.udp_csum_zero_deacce_en = A_FALSE;
	ptglcfg.pppoe_multicast_action = FAL_MAC_RDT_TO_CPU;
	ptglcfg.pppoe_multicast_deacce_en = A_TRUE;
	ptglcfg.hash_mode[0] = PPE_DRV_TUN_TL_HASH_MODE_CRC10;
	ptglcfg.hash_mode[1] = PPE_DRV_TUN_TL_HASH_MODE_XOR;
	err = fal_tunnel_global_cfg_set(PPE_DRV_SWITCH_ID, &ptglcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Global configuration failed", p);
		return false;
	}

	/*
	 * Initialize map-t specific tunnel configuration
	 */
	ptmapglcfg.src_check_action = FAL_MAC_RDT_TO_CPU;
	ptmapglcfg.dst_check_action = FAL_MAC_RDT_TO_CPU;
	ptmapglcfg.no_tcp_udp_action = FAL_MAC_RDT_TO_CPU;
	ptmapglcfg.udp_csum_zero_action = FAL_MAC_FRWRD;
	ptmapglcfg.ipv4_df_set = PPE_DRV_TUN_TL_IPV4_DF_MODE_0;
	err = fal_mapt_decap_ctrl_set(PPE_DRV_SWITCH_ID, &ptmapglcfg);
	if (err != SW_OK) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p: Tunnel Map-t common configuration failed", p);
		return false;
	}

	ppe_drv_tun_encap_hdr_ctrl_init(p);

	spin_unlock_bh(&p->lock);
	return true;
}
