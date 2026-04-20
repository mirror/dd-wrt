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

#include <linux/etherdevice.h>
#include <linux/if_vlan.h>
#include <fal/fal_ip.h>
#include "ppe_drv.h"

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_nexthop_dump()
 *	Prints out a given entry in nexthop table.
 */
void ppe_drv_nexthop_dump(struct ppe_drv_nexthop *nh)
{
	sw_error_t err;
	fal_ip_nexthop_t fal_nh = {0};

	err = fal_ip_nexthop_get(PPE_DRV_SWITCH_ID, nh->index, &fal_nh);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get nexthop configuration", nh);
		return;
	}

	ppe_drv_trace("%p: type: %d", nh, fal_nh.type);
	ppe_drv_trace("%p: vsi: %d", nh, fal_nh.vsi);
	ppe_drv_trace("%p: port: %d", nh, fal_nh.port);
	ppe_drv_trace("%p: ip_to_me_en: %d", nh, fal_nh.ip_to_me_en);
	ppe_drv_trace("%p: l3_if: %d", nh, fal_nh.if_index);
	ppe_drv_trace("%p: pub_ip_index: %d", nh, fal_nh.pub_ip_index);
	ppe_drv_trace("%p: stag_fmt: %d", nh, fal_nh.stag_fmt);
	ppe_drv_trace("%p: svid: %d", nh, fal_nh.svid);
	ppe_drv_trace("%p: ctag_fmt: %d", nh, fal_nh.ctag_fmt);
	ppe_drv_trace("%p: cvid: %d", nh, fal_nh.cvid);
	ppe_drv_trace("%p: mac_addr: %pM", nh, fal_nh.mac_addr.uc);
	ppe_drv_trace("%p: dnat_ip: %pI4", nh, &fal_nh.dnat_ip);
}
#else
/*
 * ppe_drv_nexthop_dump()
 *	Prints out a given entry in nexthop table.
 */
void ppe_drv_nexthop_dump(struct ppe_drv_nexthop *nh)
{
}
#endif

/*
 * ppe_drv_nexthop_vlan_match()
 */
static inline bool ppe_drv_nexthop_vlan_match(struct ppe_drv_nexthop *nh, uint16_t in_vlan, uint16_t out_vlan)
{
	return (nh->inner_vlan == in_vlan) && (nh->outer_vlan == out_vlan);
}

/*
 * ppe_drv_nexthop_v6_l3_if_get()
 *	Return egress l3_if associated with a flow.
 */
static inline struct ppe_drv_l3_if *ppe_drv_nexthop_v6_l3_if_get(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv_iface *iface_vsi;
	struct ppe_drv_iface *iface_l3;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;

	/*
	 * If top interface have l3_if use it.
	 */
	iface_l3 = ppe_drv_v6_conn_flow_eg_l3_if_get(pcf);
	l3_if = iface_l3 ? ppe_drv_iface_l3_if_get(iface_l3) : NULL;
	if (l3_if ) {
		return l3_if;
	}

	/*
	 * For VLAN and bridge, use l3_if associated with egress VSI.
	 */
	iface_vsi = ppe_drv_v6_conn_flow_eg_vsi_if_get(pcf);
	vsi = iface_vsi ? ppe_drv_iface_vsi_get(iface_vsi) : NULL;
	if (vsi && vsi->l3_if) {
		return vsi->l3_if;
	}

	/*
	 * For stand alone port, use port's l3_if
	 */
	pp = ppe_drv_v6_conn_flow_tx_port_get(pcf);
	if (pp) {
		l3_if = ppe_drv_port_find_port_l3_if(pp);
	}

	/*
	 * For port with active vlans use l3_if associated with
	 * untag VLAN rule. This l3_if can be used only for flows
	 * without VLAN tags.
	 */
	if (!ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf)
		&& pp && pp->ingress_untag_vlan) {
		l3_if = pp->active_l3_if;
	}

	return l3_if;
}

/*
 * ppe_drv_nexthop_v6_match()
 *	Iterate through list of active nexthop to find a match.
 */
static inline struct ppe_drv_nexthop *ppe_drv_nexthop_v6_match(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv_nexthop *nh;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint32_t inner_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint32_t outer_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);

	/*
	 * Vlan flow.
	 */
	switch (vlan_cnt) {
	case 2:
		outer_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		inner_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 1:
		inner_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
	}

	list_for_each_entry(nh, &p->nh_active, list) {
		/*
		 * Egress L3_IF
		 */
		if (ppe_drv_nexthop_v6_l3_if_get(pcf) != nh->l3_if) {
			continue;
		}

		/*
		 * DMAC
		 */
		if (!ether_addr_equal(nh->mac_addr, ppe_drv_v6_conn_flow_xmit_dest_mac_addr_get(pcf))) {
			continue;
		}

		/*
		 * VLAN tags
		 */
		if (!ppe_drv_nexthop_vlan_match(nh, inner_vlan, outer_vlan)) {
			continue;
		}

		/*
		 * Egress Port
		 */
		if (nh->dest_port != ppe_drv_v6_conn_flow_tx_port_get(pcf)) {
			continue;
		}

		ppe_drv_trace("%p: found a matching nexthop entry for flow: %p", nh, pcf);
		return nh;
	}

	return NULL;
}

/*
 * ppe_drv_nexthop_v4_l3_if_get()
 *	Return egress l3_if associated with a flow.
 */
static inline struct ppe_drv_l3_if *ppe_drv_nexthop_v4_l3_if_get(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv_iface *iface_vsi;
	struct ppe_drv_iface *iface_l3;
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;

	/*
	 * If top interface have l3_if use it.
	 */
	iface_l3 = ppe_drv_v4_conn_flow_eg_l3_if_get(pcf);
	l3_if = iface_l3 ? ppe_drv_iface_l3_if_get(iface_l3) : NULL;
	if (l3_if ) {
		return l3_if;
	}

	/*
	 * For VLAN and bridge, use l3_if associated with egress VSI.
	 */
	iface_vsi = ppe_drv_v4_conn_flow_eg_vsi_if_get(pcf);
	vsi = iface_vsi ? ppe_drv_iface_vsi_get(iface_vsi) : NULL;
	if (vsi && vsi->l3_if) {
		return vsi->l3_if;
	}

	/*
	 * For stand alone port, use port's l3_if
	 */
	pp = ppe_drv_v4_conn_flow_tx_port_get(pcf);
	if (pp) {
		l3_if = ppe_drv_port_find_port_l3_if(pp);
	}

	/*
	 * For port with active vlans use l3_if associated with
	 * untag VLAN rule. This l3_if can be used only for flows
	 * without VLAN tags.
	 */
	if (!ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf)
		&& pp && pp->ingress_untag_vlan) {
		l3_if = pp->active_l3_if;
	}

	return l3_if;
}

/*
 * ppe_drv_nexthop_v4_match()
 *	Iterate through list of active nexthop to find a match.
 */
static inline struct ppe_drv_nexthop *ppe_drv_nexthop_v4_match(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv_nexthop *nh;
	struct ppe_drv *p = &ppe_drv_gbl;
	uint32_t inner_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint32_t outer_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);
	uint32_t xlate_src_ip = ppe_drv_v4_conn_flow_xlate_src_ip_get(pcf);
	uint32_t xlate_dest_ip = ppe_drv_v4_conn_flow_xlate_dest_ip_get(pcf);
	bool flow_snat_en = ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_XLATE_SRC);
	bool flow_dnat_en = ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLOW_FLAG_XLATE_DEST);

	/*
	 * Vlan flow.
	 */
	switch (vlan_cnt) {
	case 2:
		outer_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		inner_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 1:
		inner_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
	}

	list_for_each_entry(nh, &p->nh_active, list) {
		/*
		 * Egress L3_IF
		 */
		if (ppe_drv_nexthop_v4_l3_if_get(pcf) != nh->l3_if) {
			continue;
		}

		/*
		 * DMAC
		 */
		if (!ether_addr_equal(nh->mac_addr, ppe_drv_v4_conn_flow_xmit_dest_mac_addr_get(pcf))) {
			continue;
		}

		/*
		 * VLAN tags
		 */
		if (!ppe_drv_nexthop_vlan_match(nh, inner_vlan, outer_vlan)) {
			continue;
		}

		/*
		 * Egress Port
		 */
		if (nh->dest_port != ppe_drv_v4_conn_flow_tx_port_get(pcf)) {
			continue;
		}

		/*
		 * SNAT IP
		 */
		if ((!flow_snat_en && nh->snat_en) || (flow_snat_en && !nh->snat_en)
			|| (nh->snat_en && (nh->pub_ip->ip_addr != xlate_src_ip))) {
			continue;
		}

		/*
		 * DNAT IP
		 */
		if ((!flow_dnat_en && nh->dnat_en) || (flow_dnat_en && !nh->dnat_en)
			|| (nh->dnat_en && (nh->dnat_ip != xlate_dest_ip))) {
			continue;
		}

		ppe_drv_trace("%p: found a matching nexthop entry for flow: %p", nh, pcf);
		return nh;
	}

	return NULL;
}

/*
 * ppe_drv_nexthop_free()
 *	Destroy's nexthop entry.
 */
static void ppe_drv_nexthop_free(struct kref *kref)
{
	struct ppe_drv_nexthop *nh = container_of(kref, struct ppe_drv_nexthop, ref);
	struct ppe_drv *p = &ppe_drv_gbl;
	fal_ip_nexthop_t fal_nh = {0};
	sw_error_t err;

	/*
	 * Remove nexthop from active list.
	 */
	list_del(&nh->list);

	/*
	 * Clear hardware entry
	 */
	err = fal_ip_nexthop_set(PPE_DRV_SWITCH_ID, nh->index, &fal_nh);
	if (err != SW_OK) {
		ppe_drv_warn("%p: nexthop free failed", nh);
		return;
	}

	/*
	 * Release reference to PUB_IP_ADDR entry if
	 * nexthop has SNAT info programmed
	 */
	if (nh->snat_en) {
		ppe_drv_pub_ip_deref(nh->pub_ip);
	}

	/*
	 * Clear SW entry
	 */
	nh->snat_en = 0;
	nh->pub_ip = NULL;
	nh->dnat_en = 0;
	nh->dest_port = NULL;
	nh->dnat_ip = 0;
	nh->inner_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	nh->outer_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	memset(nh->mac_addr, 0x00, ETH_ALEN);

	/*
	 * Place the entry in free list.
	 */
	list_add(&nh->list, &p->nh_free);
}

/*
 * ppe_drv_nexthop_ref()
 *	Reference PPE nexthop interface
 */
struct ppe_drv_nexthop *ppe_drv_nexthop_ref(struct ppe_drv_nexthop *nh)
{
	kref_get(&nh->ref);
	return nh;
}

/*
 * ppe_drv_nexthop_deref()
 *	Let go of reference on nexthop.
 */
bool ppe_drv_nexthop_deref(struct ppe_drv_nexthop *nh)
{
	if (kref_put(&nh->ref, ppe_drv_nexthop_free)) {
		ppe_drv_trace("reference goes down to 0 for nh: %p\n", nh);
		return true;
	}

	return false;
}

/*
 * ppe_drv_nexthop_v6_get_and_ref()
 *	Allocate nexthop entry if it does not exist and returns nexthop instance pointer.
 */
struct ppe_drv_nexthop *ppe_drv_nexthop_v6_get_and_ref(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint32_t in_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint32_t out_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);
	uint32_t match_src_ip[4];
	uint32_t match_dest_ip[4];
	uint8_t vtag = PPE_DRV_VLAN_UNTAGGED;
	struct ppe_drv_iface *iface_vsi, *iface_tx;
	fal_ip_nexthop_t fal_nh = {0};
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_nexthop *nh;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;
	struct ppe_drv_port *pp_rx;
	sw_error_t err;
	bool is_vlan_as_vp;

	ppe_drv_v6_conn_flow_match_src_ip_get(pcf, match_src_ip);
	ppe_drv_v6_conn_flow_match_dest_ip_get(pcf, match_dest_ip);

	/*
	 * Both single and double VLANs are handled by EG_VLAN_XLT_* tables, we just need to
	 * provide a unique VSI per vlan combination for handling VLAN on egress frame.
	 */
	switch (vlan_cnt) {
	case 2:
		out_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		in_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		vtag = PPE_DRV_VLAN_TAGGED;
		break;
	case 1:
		in_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		vtag = PPE_DRV_VLAN_TAGGED;
		break;
	case 0:
		break;
	default:
		ppe_drv_warn("%p: ppe doesn't support more than 2 VLANs: %u", pcf, vlan_cnt);
		return NULL;
	}

	nh = ppe_drv_nexthop_v6_match(pcf);
	if (nh) {
		/*
		 * Matching nexthop, take ref and return
		 */
		ppe_drv_trace("%p: matching nexthop entry found with index(%x)", nh, nh->index);
		ppe_drv_nexthop_ref(nh);
		return nh;
	}

	nh = list_first_entry_or_null(&p->nh_free, struct ppe_drv_nexthop, list);
	if (!nh) {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_nh_full);
		ppe_drv_warn("%p: nexthop full - cannot accelerate flow", pcf);
		return NULL;
	}

	/*
	 * Delete the entry from free list and add it to the active list.
	 * Take a reference on the nexthop entry.
	 */
	list_del(&nh->list);
	kref_init(&nh->ref);
	list_add(&nh->list, &p->nh_active);

	/*
	 * Save vlan info in nexthop
	 */
	nh->inner_vlan = in_vlan;
	nh->outer_vlan = out_vlan;

	pp = ppe_drv_v6_conn_flow_tx_port_get(pcf);
	if (!pp) {
		/*
		 * Release next hop entry.
		 */
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: egress port invalid", nh);
		return NULL;
	}

	pp_rx = ppe_drv_v6_conn_flow_rx_port_get(pcf);
	if (!pp_rx) {
		/*
		 * Release next hop entry.
		 */
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: inress port invalid", nh);
		return NULL;
	}

	ppe_drv_info("%p: ppe_port: %d", nh, pp->port);

	/*
	 * VLAN as VP port does not have vsi assosiated with it.
	 * So skip vsi check if the port is VLAN as VP.
	 */
	iface_tx = ppe_drv_v6_conn_flow_eg_port_if_get(pcf);
	is_vlan_as_vp = is_vlan_dev(iface_tx->dev) && (iface_tx->type == PPE_DRV_IFACE_TYPE_VIRTUAL);

	/*
	 * Get vsi for vlan flows.
	 */
	iface_vsi = ppe_drv_v6_conn_flow_eg_vsi_if_get(pcf);
	vsi = iface_vsi ? ppe_drv_iface_vsi_get(iface_vsi) : NULL;
	if ((vtag == PPE_DRV_VLAN_TAGGED) && !vsi && !is_vlan_as_vp) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: vlan-vsi not configured on interface: %u in_vlan: %u, out_vlan: %u",
				p, pp->port, in_vlan, out_vlan);
		return NULL;
	}

	l3_if = ppe_drv_nexthop_v6_l3_if_get(pcf);
	if (!l3_if) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: No egress L3 interface setup for port: %u", pcf, pp->port);
		return NULL;
	}

	fal_nh.if_index = l3_if->l3_if_index;
	fal_nh.pub_ip_index = nh->pub_ip ? nh->pub_ip->index : 0;
	memcpy(&fal_nh.mac_addr, ppe_drv_v6_conn_flow_xmit_dest_mac_addr_get(pcf), sizeof(fal_nh.mac_addr));

	/*
	 * Note: for non-vlan flows program the final egress port.
	 */
	if (vtag != PPE_DRV_VLAN_TAGGED) {
		fal_nh.type = FAL_NEXTHOP_VP;
		fal_nh.port = pp->port;
	} else {
		if (vsi && vsi->is_fdb_learn_enabled && pp->is_fdb_learn_enabled && pp_rx->is_fdb_learn_enabled
		   && !(ppe_drv_v6_conn_flow_flags_check(pcf, PPE_DRV_V6_CONN_FLAG_BRIDGE_VLAN_NETDEV))) {
			fal_nh.type = FAL_NEXTHOP_L3;
			fal_nh.vsi = vsi->index;
		} else {
			/*
			 * FDB learning is disabled, program vlan-ID directly in NEXTHOP_TBL.
			 */
			fal_nh.type = FAL_NEXTHOP_VP;
			fal_nh.port = pp->port;

			ppe_drv_trace("%p: fdb learning disable for dev %s out_vlan %d in_vlan %d\n", pcf,
				      pp->dev->name, out_vlan, in_vlan);

			if (out_vlan != PPE_DRV_VLAN_NOT_CONFIGURED) {
				ppe_drv_trace("%p: fdb learning disable configuring STAG:%u", pcf, out_vlan);
				fal_nh.stag_fmt = 1;
				fal_nh.svid = out_vlan;
			}

			if (in_vlan !=PPE_DRV_VLAN_NOT_CONFIGURED) {
				ppe_drv_trace("%p: fdb learning disable configuring CTAG:%u", pcf, in_vlan);
				fal_nh.ctag_fmt = 1;
				fal_nh.cvid = in_vlan;
			}
		}
	}

	err = fal_ip_nexthop_set(PPE_DRV_SWITCH_ID, nh->index, &fal_nh);
	if (err != SW_OK) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: nexthop configuration failed for flow: %p", nh, pcf);
		return NULL;
	}

	/*
	 * Update shadow copy.
	 */
	nh->l3_if = l3_if;
	nh->dest_port = pp;
	nh->dnat_ip = 0;
	nh->dnat_en = 0;
	memcpy(nh->mac_addr, ppe_drv_v6_conn_flow_xmit_dest_mac_addr_get(pcf), ETH_ALEN);

	ppe_drv_nexthop_dump(nh);

	return nh;
}

#ifdef NSS_PPE_IPQ53XX
/*
 * ppe_drv_nexthop_v6_bridge_flow_get_and_ref()
 *	Allocate nexthop entry for bridge flow if it does not exist and returns
 *	nexthop instance pointer.
 */
struct ppe_drv_nexthop *ppe_drv_nexthop_v6_bridge_flow_get_and_ref(struct ppe_drv_v6_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint32_t in_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint32_t out_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v6_conn_flow_egress_vlan_cnt_get(pcf);
	fal_ip_nexthop_t fal_nh = {0};
	struct ppe_drv_nexthop *nh;
	sw_error_t err;

	/*
	 * Both single and double VLANs are handled by EG_VLAN_XLT_* tables, we just need to
	 * provide a unique VSI per vlan combination for handling VLAN on egress frame.
	 */
	switch (vlan_cnt) {
	case 2:
		out_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		in_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 1:
		in_vlan = ppe_drv_v6_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	default:
		ppe_drv_warn("%p: PPE doesn't support more than 2 VLANs: %u", pcf, vlan_cnt);
		return NULL;
	}

	/*
	 * find matching nexthop entry
	 */
	list_for_each_entry(nh, &p->nh_active, list) {
		/*
		 * VLAN tags
		 */
		if (ppe_drv_nexthop_vlan_match(nh, in_vlan, out_vlan)) {
			/*
			 * Matching nexthop, take ref and return
			 */
			ppe_drv_trace("%p: matching nexthop entry found with index(%x)", nh, nh->index);
			ppe_drv_nexthop_ref(nh);
			return nh;
		}
	}

	/*
	 * Allocate new entry and program vlan-ID directly in NEXTHOP_TBL.
	 */
	nh = list_first_entry_or_null(&p->nh_free, struct ppe_drv_nexthop, list);
	if (!nh) {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_nh_full);
		ppe_drv_warn("%p: nexthop table full - cannot accelerate flow", pcf);
		return NULL;
	}

	/*
	 * Delete the entry from free list and add it to the active list.
	 * Take a reference on the nexthop entry.
	 */
	list_del(&nh->list);
	kref_init(&nh->ref);
	list_add(&nh->list, &p->nh_active);

	if (out_vlan != PPE_DRV_VLAN_NOT_CONFIGURED) {
		ppe_drv_trace("%p: configuring STAG:%u in nexthop table", pcf, out_vlan);
		fal_nh.stag_fmt = 1;
		fal_nh.svid = out_vlan;
	}

	if (in_vlan != PPE_DRV_VLAN_NOT_CONFIGURED) {
		ppe_drv_trace("%p: configuring CTAG:%u in nexthop table", pcf, in_vlan);
		fal_nh.ctag_fmt = 1;
		fal_nh.cvid = in_vlan;
	}

	err = fal_ip_nexthop_set(PPE_DRV_SWITCH_ID, nh->index, &fal_nh);
	if (err != SW_OK) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("nexthop configuration failed for flow: %p", pcf);
		return NULL;
	}

	/*
	 * Save vlan info in nexthop
	 */
	nh->inner_vlan = in_vlan;
	nh->outer_vlan = out_vlan;

	ppe_drv_nexthop_dump(nh);

	return nh;
}

/*
 * ppe_drv_nexthop_v4_bridge_flow_get_and_ref()
 *	Allocate nexthop entry for bridge flow if it does not exist and returns
 *	nexthop instance pointer.
 */
struct ppe_drv_nexthop *ppe_drv_nexthop_v4_bridge_flow_get_and_ref(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint32_t in_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint32_t out_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);
	fal_ip_nexthop_t fal_nh = {0};
	struct ppe_drv_nexthop *nh;
	sw_error_t err;

	/*
	 * Outer and Inner VLAN identifiers are obtained based on single or double VLAN.
	 */
	switch (vlan_cnt) {
	case 2:
		out_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		in_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	case 1:
		in_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		break;
	default:
		ppe_drv_warn("%p: PPE doesn't support more than 2 VLANs: %u", pcf, vlan_cnt);
		return NULL;
	}

	/*
	 * find matching nexthop entry
	 */
	list_for_each_entry(nh, &p->nh_active, list) {
		/*
		 * VLAN tags
		 */
		if (ppe_drv_nexthop_vlan_match(nh, in_vlan, out_vlan)) {
			/*
			 * Matching nexthop, take ref and return
			 */
			ppe_drv_trace("%p: matching nexthop entry found with index(%x)", nh, nh->index);
			ppe_drv_nexthop_ref(nh);
			return nh;
		}
	}

	/*
	 * Allocate new entry and program vlan-ID directly in NEXTHOP_TBL.
	 */
	nh = list_first_entry_or_null(&p->nh_free, struct ppe_drv_nexthop, list);
	if (!nh) {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_nh_full);
		ppe_drv_warn("%p: nexthop table full - cannot accelerate flow", pcf);
		return NULL;
	}

	/*
	 * Delete the entry from free list and add it to the active list.
	 * Take a reference on the nexthop entry.
	 */
	list_del(&nh->list);
	kref_init(&nh->ref);
	list_add(&nh->list, &p->nh_active);

	if (out_vlan != PPE_DRV_VLAN_NOT_CONFIGURED) {
		ppe_drv_trace("%p: configuring STAG:%u in nexthop table", pcf, out_vlan);
		fal_nh.stag_fmt = 1;
		fal_nh.svid = out_vlan;
	}

	if (in_vlan != PPE_DRV_VLAN_NOT_CONFIGURED) {
		ppe_drv_trace("%p: configuring CTAG:%u in nexthop table", pcf, in_vlan);
		fal_nh.ctag_fmt = 1;
		fal_nh.cvid = in_vlan;
	}

	err = fal_ip_nexthop_set(PPE_DRV_SWITCH_ID, nh->index, &fal_nh);
	if (err != SW_OK) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("nexthop configuration failed for flow: %p", pcf);
		return NULL;
	}

	/*
	 * Save vlan info in nexthop
	 */
	nh->inner_vlan = in_vlan;
	nh->outer_vlan = out_vlan;

	ppe_drv_nexthop_dump(nh);

	return nh;
}
#endif

/*
 * ppe_drv_nexthop_v4_get_and_ref()
 *	Allocate nexthop entry if it does not exist and returns nexthop instance pointer.
 */
struct ppe_drv_nexthop *ppe_drv_nexthop_v4_get_and_ref(struct ppe_drv_v4_conn_flow *pcf)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	uint32_t in_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint32_t out_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
	uint8_t vlan_cnt = ppe_drv_v4_conn_flow_egress_vlan_cnt_get(pcf);
	uint32_t match_src_ip = ppe_drv_v4_conn_flow_match_src_ip_get(pcf);
	uint32_t xlate_src_ip = ppe_drv_v4_conn_flow_xlate_src_ip_get(pcf);
	uint32_t match_dest_ip = ppe_drv_v4_conn_flow_match_dest_ip_get(pcf);
	uint32_t xlate_dest_ip = ppe_drv_v4_conn_flow_xlate_dest_ip_get(pcf);
	uint8_t vtag = PPE_DRV_VLAN_UNTAGGED;
	struct ppe_drv_iface *iface_vsi, *iface_tx;
	fal_ip_nexthop_t fal_nh = {0};
	struct ppe_drv_l3_if *l3_if;
	struct ppe_drv_nexthop *nh;
	struct ppe_drv_vsi *vsi;
	struct ppe_drv_port *pp;
	struct ppe_drv_port *pp_rx;
	sw_error_t err;
	bool is_vlan_as_vp;

	/*
	 * Both single and double VLANs are handled by EG_VLAN_XLT_* tables, we just need to
	 * provide a unique VSI per vlan combination for handling VLAN on egress frame.
	 */
	switch (vlan_cnt) {
	case 2:
		out_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		in_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 1)->tci & PPE_DRV_VLAN_ID_MASK;
		vtag = PPE_DRV_VLAN_TAGGED;
		break;
	case 1:
		in_vlan = ppe_drv_v4_conn_flow_egress_vlan_get(pcf, 0)->tci & PPE_DRV_VLAN_ID_MASK;
		vtag = PPE_DRV_VLAN_TAGGED;
		break;
	case 0:
		break;
	default:
		ppe_drv_warn("%p: ppe doesn't support more than 2 VLANs: %u", pcf, vlan_cnt);
		return NULL;
	}

	nh = ppe_drv_nexthop_v4_match(pcf);
	if (nh) {
		/*
		 * Matching nexthop, take ref and return
		 */
		ppe_drv_trace("%p: matching nexthop entry found with index(%x)", nh, nh->index);
		ppe_drv_nexthop_ref(nh);
		return nh;
	}

	nh = list_first_entry_or_null(&p->nh_free, struct ppe_drv_nexthop, list);
	if (!nh) {
		ppe_drv_stats_inc(&p->stats.gen_stats.fail_nh_full);
		ppe_drv_warn("%p: nexthop table full - cannot accelerate flow", pcf);
		return NULL;
	}

	/*
	 * Delete the entry from free list and add it to the active list.
	 * Take a reference on the nexthop entry.
	 */
	list_del(&nh->list);
	kref_init(&nh->ref);
	list_add(&nh->list, &p->nh_active);

	/*
	 * Save vlan info in nexthop
	 */
	nh->inner_vlan = in_vlan;
	nh->outer_vlan = out_vlan;

	pp = ppe_drv_v4_conn_flow_tx_port_get(pcf);
	if (!pp) {
		/*
		 * Release next hop entry.
		 */
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: egress port invalid", nh);
		return NULL;
	}

	pp_rx = ppe_drv_v4_conn_flow_rx_port_get(pcf);
	if (!pp_rx) {
		/*
		 * Release next hop entry.
		 */
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: ingress port invalid", nh);
		return NULL;
	}

	ppe_drv_info("%p: ppe_port: %d", nh, pp->port);

	/*
	 * Get public ip address index for SNAT
	 */
	if (match_src_ip != xlate_src_ip) {
		nh->pub_ip = ppe_drv_pub_ip_get_and_ref(xlate_src_ip);
		if (!nh->pub_ip) {
			/*
			 * release next hop entry.
			 */
			ppe_drv_warn("%p: failed to get a pub ip entry", nh);
			ppe_drv_nexthop_deref(nh);
			return NULL;
		}

		nh->snat_en = true;
	}

	/*
	 * VLAN as VP port does not have vsi assosiated with it.
	 * So skip vsi check if the port is VLAN as VP.
	 */
	iface_tx = ppe_drv_v4_conn_flow_eg_port_if_get(pcf);
	is_vlan_as_vp = is_vlan_dev(iface_tx->dev) && (iface_tx->type == PPE_DRV_IFACE_TYPE_VIRTUAL);

	/*
	 * Get vsi for vlan flows.
	 */
	iface_vsi = ppe_drv_v4_conn_flow_eg_vsi_if_get(pcf);
	vsi = iface_vsi ? ppe_drv_iface_vsi_get(iface_vsi) : NULL;
	if ((vtag == PPE_DRV_VLAN_TAGGED) && !vsi && !is_vlan_as_vp) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: vlan-vsi not configured on interface: %u in_vlan: %u, out_vlan: %u",
				p, pp->port, in_vlan, out_vlan);
		return NULL;
	}

	l3_if = ppe_drv_nexthop_v4_l3_if_get(pcf);
	if (!l3_if) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: No egress L3 interface setup for port: %u", pcf, pp->port);
		return NULL;
	}

	fal_nh.if_index = l3_if->l3_if_index;
	fal_nh.pub_ip_index = nh->pub_ip ? nh->pub_ip->index : 0;
	fal_nh.dnat_ip = xlate_dest_ip;
	memcpy(&fal_nh.mac_addr, ppe_drv_v4_conn_flow_xmit_dest_mac_addr_get(pcf), sizeof(fal_nh.mac_addr));

	/*
	 * Note: for non-vlan flows program the final egress port.
	 */
	if (vtag != PPE_DRV_VLAN_TAGGED) {
		fal_nh.type = FAL_NEXTHOP_VP;
		fal_nh.port = pp->port;
	} else {
		if (vsi && vsi->is_fdb_learn_enabled && pp->is_fdb_learn_enabled && pp_rx->is_fdb_learn_enabled &&
		   !(ppe_drv_v4_conn_flow_flags_check(pcf, PPE_DRV_V4_CONN_FLAG_BRIDGE_VLAN_NETDEV))) {
			fal_nh.type = FAL_NEXTHOP_L3;
			fal_nh.vsi = vsi->index;
		} else {
			/*
			 * FDB learning is disabled, program vlan-ID directly in NEXTHOP_TBL.
			 */
			fal_nh.type = FAL_NEXTHOP_VP;
			fal_nh.port = pp->port;

			ppe_drv_trace("%p: fdb learning disable for dev %s out_vlan %d in_vlan %d\n", pcf,
				      pp->dev->name, out_vlan, in_vlan);

			if (out_vlan != PPE_DRV_VLAN_NOT_CONFIGURED) {
				ppe_drv_trace("%p: fdb learning disable configuring STAG:%u", pcf, out_vlan);
				fal_nh.stag_fmt = 1;
				fal_nh.svid = out_vlan;
			}

			if (in_vlan !=PPE_DRV_VLAN_NOT_CONFIGURED) {
				ppe_drv_trace("%p: fdb learning disable configuring CTAG:%u", pcf, in_vlan);
				fal_nh.ctag_fmt = 1;
				fal_nh.cvid = in_vlan;
			}
		}
	}

	err = fal_ip_nexthop_set(PPE_DRV_SWITCH_ID, nh->index, &fal_nh);
	if (err != SW_OK) {
		ppe_drv_nexthop_deref(nh);
		ppe_drv_warn("%p: nexthop configuration failed for flow: %p", nh, pcf);
		return NULL;
	}

	/*
	 * Update shadow copy.
	 */
	nh->l3_if = l3_if;
	nh->dest_port = pp;
	nh->dnat_ip = xlate_dest_ip;
	nh->dnat_en = (match_dest_ip != xlate_dest_ip);
	memcpy(nh->mac_addr, ppe_drv_v4_conn_flow_xmit_dest_mac_addr_get(pcf), ETH_ALEN);

	ppe_drv_nexthop_dump(nh);

	return nh;
}

/*
 * ppe_drv_nexthop_entries_free()
 *	Free nexthop table entries if it was allocated.
 */
void ppe_drv_nexthop_entries_free(struct ppe_drv_nexthop *nexthop)
{
	struct ppe_drv *p __maybe_unused = &ppe_drv_gbl;

	ppe_drv_assert(list_empty(&p->nh_active), "%p: there should not be any active nexthop entries ", p);
	ppe_drv_assert(!list_empty(&p->nh_free), "%p: there should be free nexthop entries ", p);

	/*
	 * TODO: do we need to delete entries from free list?
	 */

	/*
	 * Release memory
	 */
	vfree(nexthop);
}

/*
 * ppe_drv_nexthop_entries_alloc()
 *	Allocate and initialize nexthop entries and nexthop free list.
 */
struct ppe_drv_nexthop *ppe_drv_nexthop_entries_alloc()
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_nexthop *nexthop;
	uint16_t i;

	nexthop = vzalloc(sizeof(struct ppe_drv_nexthop) * p->nexthop_num);
	if (!nexthop) {
		ppe_drv_warn("%p: failed to allocate nexthop entries", p);
		return NULL;
	}

	/*
	 * Initialize active and free list
	 */
	INIT_LIST_HEAD(&p->nh_active);
	INIT_LIST_HEAD(&p->nh_free);

	/*
	 * Assign all nexthop entries to free list to start with.
	 */
	for (i = 0; i < p->nexthop_num; i++) {
		struct ppe_drv_nexthop *nh;
		nh = &nexthop[i];
		nh->index = i;
		nh->inner_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
		nh->outer_vlan = PPE_DRV_VLAN_NOT_CONFIGURED;
		INIT_LIST_HEAD(&nh->list);
		list_add(&nh->list, &p->nh_free);
	}

	return nexthop;
}
