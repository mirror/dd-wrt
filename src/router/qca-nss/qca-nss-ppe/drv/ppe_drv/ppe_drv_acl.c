/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#if (PPE_DRV_DEBUG_LEVEL == 3)
/*
 * ppe_drv_acl_dump()
 *	Dump ACL rule tables.
 */
static void ppe_drv_acl_dump(struct ppe_drv_acl_ctx *ctx)
{
}
#else
static void ppe_drv_acl_dump(struct ppe_drv_acl_ctx *ctx)
{
}
#endif

/*
 * ppe_drv_acl_ctx_alloc()
 *	Allocate memory for an ACL rule.
 */
static inline struct ppe_drv_acl_ctx *ppe_drv_acl_ctx_alloc(void)
{
	/*
	 * Allocate memory for a new rule
	 *
	 * kzalloc with GFP_ATOMIC is used assuming ACL rules can be created
	 * in softirq context as well while processing an SKB in the system.
	 */
	return kzalloc(sizeof(struct ppe_drv_acl_ctx), GFP_ATOMIC);
}

/*
 * ppe_drv_acl_ctx_free()
 *	Free ACL context.
 */
static inline void ppe_drv_acl_ctx_free(struct ppe_drv_acl_ctx *ctx)
{
	kfree(ctx);
}

/*
 * ppe_drv_acl_get_total_slices()
 *	Return total number of slices required in hardware.
 */
static inline uint8_t ppe_drv_acl_get_total_slices(uint8_t req_slices)
{
	switch(req_slices) {
	case 1:
		return 1;
	case 2:
		return 2;
	case 3:
	case 4:
		return 4;
	case 5:
	case 6:
	case 7:
	case 8:
		return 8;
	}

	ppe_drv_warn("Incorrect number of req_slices: %d", req_slices);
	return 0;
}


/*
 * ppe_drv_acl_list_id_get()
 *	Get a free list id for a specific IPO type.
 */
static int16_t ppe_drv_acl_list_id_get(ppe_drv_acl_ipo_t type)
{
	int16_t id;
	int16_t list_id_start, list_id_end;
	struct ppe_drv_acl *acl = ppe_drv_gbl.acl;

	if (type == PPE_DRV_ACL_IPO) {
		list_id_start = PPE_DRV_ACL_LIST_ID_IPO_START;
		list_id_end = PPE_DRV_ACL_LIST_ID_IPO_END;
	} else {
		list_id_start = PPE_DRV_ACL_LIST_ID_PRE_IPO_START;
		list_id_end = PPE_DRV_ACL_LIST_ID_PRE_IPO_END;
	}

	for (id = list_id_start; id <= list_id_end; id++) {
		if (acl->list_id[id].list_id_state == PPE_DRV_ACL_LIST_ID_FREE) {
			acl->list_id[id].list_id_state = PPE_DRV_ACL_LIST_ID_USED;
			return id;
		}
	}

	return -1;
}

/*
 * ppe_drv_acl_list_id_return()
 *	Return a list id to free pool.
 */
static void ppe_drv_acl_list_id_return(int16_t id)
{
	struct ppe_drv_acl *acl = ppe_drv_gbl.acl;

	acl->list_id[id].ctx = NULL;
	acl->list_id[id].list_id_state = PPE_DRV_ACL_LIST_ID_FREE;
}

/*
 * ppe_drv_acl_destroy()
 *	Destroy an ACL rule.
 */
void ppe_drv_acl_destroy(struct ppe_drv_acl_ctx *ctx)
{
	/*
	 * TODO: based on SSDK optimization.
	 *
	 * Clear slices from start slice to number of slices used for this ACL rule.
	 *
	 * Update slide binding.
	 *
	 * Update the used slice bitmap in the row.
	 */
	sw_error_t error;
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	if (ctx->rule_valid) {
		error = fal_acl_rule_delete(PPE_DRV_SWITCH_ID, ctx->list_id, PPE_DRV_ACL_RULE_ID, PPE_DRV_ACL_RULE_NR);
		if (error != SW_OK) {
			ppe_drv_stats_inc(&p->stats.acl_stats.rule_delete_fail);
			ppe_drv_warn("%p: Rule delete failed for list_id: %d error: %d\n", ctx, ctx->list_id, error);
		}
	}

	error = fal_acl_list_destroy(PPE_DRV_SWITCH_ID, ctx->list_id);
	if (error != SW_OK) {
		ppe_drv_stats_inc(&p->stats.acl_stats.list_delete_fail);
		ppe_drv_warn("%p: ACL list destroy failed for list_id: %d, error %d\n", ctx, ctx->list_id, error);
	}

	ppe_drv_info("%p: ACL rule destroy for list_id: %d", ctx, ctx->list_id);

	ppe_drv_stats_sub(&p->stats.acl_stats.req_slices, ctx->req_slices);
	ppe_drv_stats_sub(&p->stats.acl_stats.total_slices, ctx->total_slices);
	ppe_drv_stats_dec(&p->stats.acl_stats.active_rules);
	ppe_drv_acl_list_id_return(ctx->list_id);
	ppe_drv_acl_ctx_free(ctx);
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_destroy);

/*
 * ppe_drv_acl_rule_fill()
 *	Fill rule related information.
 */
static bool ppe_drv_acl_rule_fill(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_rule *info)
{
	fal_acl_rule_t *fal_rule = &ctx->fal_rule;
	enum ppe_drv_acl_slice_type slice_type;
	struct ppe_drv_acl_rule_match_one *slice;
	static fal_ip6_addr_t ip6, ip6_mask;

	for (slice_type = PPE_DRV_ACL_SLICE_TYPE_DST_MAC;
		slice_type < PPE_DRV_ACL_SLICE_TYPE_MAX; slice_type++) {

		if (!info->chain[slice_type].valid) {
			continue;
		}

		switch (slice_type) {
		case PPE_DRV_ACL_SLICE_TYPE_DST_MAC:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_MAC];
			memcpy(&fal_rule->dest_mac_val, slice->rule.dmac.mac, ETH_ALEN);
			memcpy(&fal_rule->dest_mac_mask, slice->rule.dmac.mac_mask, ETH_ALEN);
			FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_DA);
			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_MAC;
			ppe_drv_trace("%p: slice dst mac addr: %pM mask: %pM",
				ctx, slice->rule.dmac.mac, slice->rule.dmac.mac_mask);
			break;

		case PPE_DRV_ACL_SLICE_TYPE_SRC_MAC:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC];
			memcpy(&fal_rule->src_mac_val, slice->rule.smac.mac, ETH_ALEN);
			memcpy(&fal_rule->src_mac_mask, slice->rule.smac.mac_mask, ETH_ALEN);
			FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_SA);
			fal_rule->rule_type = FAL_ACL_RULE_MAC;
			ctx->rule_type_valid = true;
			ppe_drv_trace("%p: slice src mac addr: %pM mask: %pM",
				ctx, slice->rule.smac.mac, slice->rule.smac.mac_mask);
			break;

		case PPE_DRV_ACL_SLICE_TYPE_VLAN:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_VLAN];
			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_VSI_VALID) {
				fal_rule->vsi_valid = slice->rule.vlan.vsi_valid;
				fal_rule->vsi_valid_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_VSI_VALID);
				ppe_drv_trace("%p: slice vlan vsi_valid: %d",
					ctx, slice->rule.vlan.vsi_valid);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_VSI) {
				fal_rule->vsi = slice->rule.vlan.vsi;
				fal_rule->vsi_mask = slice->rule.vlan.vsi_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_VSI);
				ppe_drv_trace("%p: slice vlan vsi: %d, mask: 0x%x",
					ctx, slice->rule.vlan.vsi, slice->rule.vlan.vsi_mask);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_STAG) {
				/*
				 * TODO add this.
				 */
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CTAG) {
				/*
				 * TODO add this.
				 */
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_SVID) {
				fal_rule->stag_vid_val = slice->rule.vlan.svid;
				fal_rule->stag_vid_mask = slice->rule.vlan.svid_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
				ppe_drv_trace("%p: slice vlan svid: %d, mask: 0x%x",
					ctx, slice->rule.vlan.svid, slice->rule.vlan.svid_mask);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CVID) {
				fal_rule->ctag_vid_val = slice->rule.vlan.cvid_min;
				fal_rule->ctag_vid_mask = slice->rule.vlan.cvid_mask_max;
				fal_rule->ctag_vid_op = slice->rule.vlan.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_VID);
				ppe_drv_trace("%p: slice vlan cvid: %d, mask: 0x%x",
					ctx, fal_rule->ctag_vid_val, fal_rule->ctag_vid_mask);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_SPCP) {
				fal_rule->stag_pri_val = slice->rule.vlan.spcp;
				fal_rule->stag_pri_mask = slice->rule.vlan.spcp_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_STAG_PRI);
				ppe_drv_trace("%p: slice vlan spcp: %d, mask: 0x%x",
					ctx, slice->rule.vlan.spcp, slice->rule.vlan.spcp_mask);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CPCP) {
				fal_rule->ctag_pri_val = slice->rule.vlan.cpcp;
				fal_rule->ctag_pri_mask = slice->rule.vlan.cpcp_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_PRI);
				ppe_drv_trace("%p: slice vlan cpcp: %d, mask: 0x%x",
					ctx, slice->rule.vlan.cpcp, slice->rule.vlan.cpcp_mask);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_SDEI) {
				fal_rule->stag_dei_val = slice->rule.vlan.sdei_en;
				fal_rule->stag_dei_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_STAG_DEI);
				ppe_drv_trace("%p: slice vlan sdei: %d", ctx, slice->rule.vlan.sdei_en);
			}

			if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CDEI) {
				fal_rule->ctag_cfi_val = slice->rule.vlan.cdei_en;
				fal_rule->ctag_cfi_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_CTAG_CFI);
				ppe_drv_trace("%p: slice vlan cdei: %d", ctx, slice->rule.vlan.cdei_en);
			}

			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_MAC;
			break;

		case PPE_DRV_ACL_SLICE_TYPE_L2_MISC:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC];
			if (slice->flags & PPE_DRV_ACL_L2_MISC_FLAG_PPPOE) {
				fal_rule->pppoe_sessionid = slice->rule.l2.pppoe_session_id;
				fal_rule->pppoe_sessionid_mask = slice->rule.l2.pppoe_session_id_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
				ppe_drv_trace("%p: slice l2_misc pppoe session: %d, mask: 0x%x",
					ctx, slice->rule.l2.pppoe_session_id, slice->rule.l2.pppoe_session_id_mask);
			}

			if (slice->flags & PPE_DRV_ACL_L2_MISC_FLAG_L2PROTO) {
				fal_rule->ethtype_val = slice->rule.l2.l2_proto;
				fal_rule->ethtype_mask = slice->rule.l2.l2_proto_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_ETHTYPE);
				ppe_drv_trace("%p: slice l2_misc l2 proto: %d, mask: 0x%x",
					ctx, slice->rule.l2.l2_proto, slice->rule.l2.l2_proto_mask);
			}

			if (slice->flags & PPE_DRV_ACL_L2_FLAG_SVID) {
				fal_rule->stag_vid_val = slice->rule.l2.svid_min;
				fal_rule->stag_vid_mask = slice->rule.l2.svid_mask_max;
				fal_rule->stag_vid_op = slice->rule.l2.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MAC_STAG_VID);
				ppe_drv_trace("%p: slice l2_misc svid: %d, mask: 0x%x",
					ctx, slice->rule.l2.svid_min, slice->rule.l2.svid_mask_max);
			}

			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_MAC;
			break;

		case PPE_DRV_ACL_SLICE_TYPE_DST_IPV4:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4];
			if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_PKTTYPE) {
				fal_rule->l3_pkt_type = slice->rule.dip_v4.pkt_type;
				fal_rule->l3_pkt_type_mask = slice->rule.dip_v4.pkt_type_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
				ppe_drv_trace("%p: slice dst ipv4 pkt_type: %d, mask: 0x%x",
					ctx, slice->rule.dip_v4.pkt_type, slice->rule.dip_v4.pkt_type_mask);
			}

			if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_L3FRAG) {
				fal_rule->is_fragement_val = slice->rule.dip_v4.l3_fragment_flag;
				fal_rule->is_fragement_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
				ppe_drv_trace("%p: slice dst ipv4 l3_frag: %d",
					ctx, slice->rule.dip_v4.l3_fragment_flag);
			}

			if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_L4PORT) {
				fal_rule->dest_l4port_val = slice->rule.dip_v4.l4_port_min;
				fal_rule->dest_l4port_mask = slice->rule.dip_v4.l4_port_mask_max;
				fal_rule->dest_l4port_op = slice->rule.dip_v4.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L4_DPORT);
				ppe_drv_trace("%p: slice dst ipv4 l4 port: %d, mask: 0x%x",
					ctx, slice->rule.dip_v4.l4_port_min, slice->rule.dip_v4.l4_port_mask_max);
			}

			if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_IP) {
				fal_rule->dest_ip4_val = slice->rule.dip_v4.ip;
				fal_rule->dest_ip4_mask = slice->rule.dip_v4.ip_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP4_DIP);
				ppe_drv_trace("%p: slice dst ipv4 ip: %pI4, mask: %pI4",
					ctx, &slice->rule.dip_v4.ip, &slice->rule.dip_v4.ip_mask);
			}

			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_IP4;
			break;

		case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4];
			if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_PKTTYPE) {
				fal_rule->l3_pkt_type = slice->rule.sip_v4.pkt_type;
				fal_rule->l3_pkt_type_mask = slice->rule.sip_v4.pkt_type_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
				ppe_drv_trace("%p: slice src ipv4 pkt_type: %d, mask: 0x%x",
					ctx, slice->rule.sip_v4.pkt_type, slice->rule.sip_v4.pkt_type_mask);
			}

			if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_L3FRAG) {
				fal_rule->is_fragement_val = slice->rule.sip_v4.l3_fragment_flag;
				fal_rule->is_fragement_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
				ppe_drv_trace("%p: slice src ipv4 l3_frag: %d",
					ctx, slice->rule.sip_v4.l3_fragment_flag);
			}

			if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_L4PORT) {
				fal_rule->src_l4port_val = slice->rule.sip_v4.l4_port_min;
				fal_rule->src_l4port_mask = slice->rule.sip_v4.l4_port_mask_max;
				fal_rule->src_l4port_op = slice->rule.sip_v4.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L4_SPORT);
				ppe_drv_trace("%p: slice src ipv4 l4 port: %d, mask: 0x%x",
					ctx, slice->rule.sip_v4.l4_port_min, slice->rule.sip_v4.l4_port_mask_max);
			}

			if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_IP) {
				fal_rule->src_ip4_val = slice->rule.sip_v4.ip;
				fal_rule->src_ip4_mask = slice->rule.sip_v4.ip_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP4_SIP);
				ppe_drv_trace("%p: slice src ipv4 ip: %pI4, mask: %pI4",
					ctx, &slice->rule.sip_v4.ip, &slice->rule.sip_v4.ip_mask);
			}

			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_IP4;
			break;

		case PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0:
			memset(&ip6, 0x0, sizeof(fal_ip6_addr_t));

			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0];
			ip6.ul[0] = slice->rule.dip_v6_0.ip_l32;
			ip6.ul[1] = slice->rule.dip_v6_0.ip_u16;
			ip6_mask.ul[0] = slice->rule.dip_v6_0.ip_l32_mask;
			ip6_mask.ul[1] = slice->rule.dip_v6_0.ip_u16_mask;
			break;

		case PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1];
			ip6.ul[1] = (slice->rule.dip_v6_1.ip_l32 << 16) | (ip6.ul[1] & 0xffff);
			ip6.ul[2] = (slice->rule.dip_v6_1.ip_l32 >> 16) | (slice->rule.dip_v6_1.ip_u16 << 16);
			ip6_mask.ul[1] = (slice->rule.dip_v6_1.ip_l32_mask << 16) | (ip6_mask.ul[1] & 0xffff);
			ip6_mask.ul[2] = (slice->rule.dip_v6_1.ip_l32_mask >> 16) | (slice->rule.dip_v6_1.ip_u16_mask << 16);
			break;

		case PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2];
			if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_PKTTYPE) {
				fal_rule->l3_pkt_type = slice->rule.dip_v6_2.pkt_type;
				fal_rule->l3_pkt_type_mask = slice->rule.dip_v6_2.pkt_type_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
			}

			if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_L3FRAG) {
				fal_rule->is_fragement_val = slice->rule.dip_v6_2.l3_fragment_flag;
				fal_rule->is_fragement_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
			}

			if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_L4PORT) {
				fal_rule->dest_l4port_val = slice->rule.dip_v6_2.l4_port_min;
				fal_rule->dest_l4port_mask = slice->rule.dip_v6_2.l4_port_mask_max;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L4_DPORT);
			}

			if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_IP) {
				ip6.ul[3] = slice->rule.dip_v6_2.ip_l32;
				ip6_mask.ul[3] = slice->rule.dip_v6_2.ip_l32_mask;
				memcpy(fal_rule->dest_ip6_val.ul, ip6.ul, sizeof(fal_ip6_addr_t));
				memcpy(fal_rule->dest_ip6_mask.ul, ip6_mask.ul, sizeof(fal_ip6_addr_t));
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP6_DIP);
			}

			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_IP6;
			ppe_drv_trace("%p: slice dst ipv6 ip: %pI6, mask: %pI6",
				ctx, fal_rule->dest_ip6_val.ul, fal_rule->dest_ip6_mask.ul);
			break;

		case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0:
			memset(&ip6, 0x0, sizeof(fal_ip6_addr_t));

			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0];
			ip6.ul[0] = slice->rule.sip_v6_0.ip_l32;
			ip6.ul[1] = slice->rule.sip_v6_0.ip_u16;
			ip6_mask.ul[0] = slice->rule.sip_v6_0.ip_l32_mask;
			ip6_mask.ul[1] = slice->rule.sip_v6_0.ip_u16_mask;
			break;

		case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1];
			ip6.ul[1] = (slice->rule.sip_v6_1.ip_l32 << 16) | (ip6.ul[1] & 0xffff);
			ip6.ul[2] = (slice->rule.sip_v6_1.ip_l32 >> 16) | (slice->rule.sip_v6_1.ip_u16 << 16);
			ip6_mask.ul[1] = (slice->rule.sip_v6_1.ip_l32_mask << 16) | (ip6_mask.ul[1] & 0xffff);
			ip6_mask.ul[2] = (slice->rule.sip_v6_1.ip_l32_mask >> 16) | (slice->rule.sip_v6_1.ip_u16_mask << 16);
			break;

		case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2];
			if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_PKTTYPE) {
				fal_rule->l3_pkt_type = slice->rule.sip_v6_2.pkt_type;
				fal_rule->l3_pkt_type_mask = slice->rule.sip_v6_2.pkt_type_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP_PKT_TYPE);
			}

			if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_L3FRAG) {
				fal_rule->is_fragement_val = slice->rule.sip_v6_2.l3_fragment_flag;
				fal_rule->is_fragement_mask = 1;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
			}

			if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_L4PORT) {
				fal_rule->src_l4port_val = slice->rule.sip_v6_2.l4_port_min;
				fal_rule->src_l4port_mask = slice->rule.sip_v6_2.l4_port_mask_max;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L4_SPORT);
			}

			if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_IP) {
				ip6.ul[3] = slice->rule.sip_v6_2.ip_l32;
				ip6_mask.ul[3] = slice->rule.sip_v6_2.ip_l32_mask;
				memcpy(fal_rule->src_ip6_val.ul, ip6.ul, sizeof(fal_ip6_addr_t));
				memcpy(fal_rule->src_ip6_mask.ul, ip6_mask.ul, sizeof(fal_ip6_addr_t));
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP6_SIP);
			}

			ctx->rule_type_valid = true;
			fal_rule->rule_type = FAL_ACL_RULE_IP6;
			ppe_drv_trace("%p: slice src ipv6 ip: %pI6, mask: %pI6",
				ctx, fal_rule->src_ip6_val.ul, fal_rule->src_ip6_mask.ul);
			break;

		case PPE_DRV_ACL_SLICE_TYPE_IP_MISC:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L3FRAG) {
				fal_rule->is_fragement_val = slice->rule.ip.l3_fragment_flag;
				fal_rule->is_fragement_mask = slice->rule.ip.l3_fragment_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_FRAGMENT);
				ppe_drv_trace("%p: slice ip misc l3 frag: %d, mask: 0x%x",
					ctx, fal_rule->is_fragement_val, fal_rule->is_fragement_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_OTHEXT_HDR) {
				fal_rule->is_other_header_val = slice->rule.ip.other_extension_hdr_flag;
				fal_rule->is_other_header_mask = slice->rule.ip.other_extension_hdr_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_OTHER_EXT_HEADER);
				ppe_drv_trace("%p: slice ip misc l3 frag: %d, mask: 0x%x",
					ctx, fal_rule->is_other_header_val, fal_rule->is_other_header_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_FRAG_HDR) {
				fal_rule->is_fragment_header_val = slice->rule.ip.frag_hdr_flag;
				fal_rule->is_fragment_header_mask = slice->rule.ip.frag_hdr_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_FRAGMENT_HEADER);
				ppe_drv_trace("%p: slice ip misc frag hdr: %d, mask: 0x%x",
					ctx, fal_rule->is_fragment_header_val, fal_rule->is_fragment_header_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_MOB_HDR) {
				fal_rule->is_mobility_header_val = slice->rule.ip.mobility_hdr_flag;
				fal_rule->is_mobility_header_mask = slice->rule.ip.mobility_hdr_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_MOBILITY_HEADER);
				ppe_drv_trace("%p: slice ip misc MOB hdr: %d, mask: 0x%x",
					ctx, fal_rule->is_mobility_header_val, fal_rule->is_mobility_header_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_ESP_HDR) {
				fal_rule->is_esp_header_val = slice->rule.ip.esp_hdr_flag;
				fal_rule->is_esp_header_mask = slice->rule.ip.esp_hdr_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_ESP_HEADER);
				ppe_drv_trace("%p: slice ip misc esp hdr: %d, mask: 0x%x",
					ctx, fal_rule->is_esp_header_val, fal_rule->is_esp_header_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_AH_HDR) {
				fal_rule->is_ah_header_val = slice->rule.ip.ah_hdr_flag;
				fal_rule->is_ah_header_mask = slice->rule.ip.ah_hdr_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_AH_HEADER);
				ppe_drv_trace("%p: slice ip misc AH hdr: %d, mask: 0x%x",
					ctx, fal_rule->is_ah_header_val, fal_rule->is_ah_header_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_TTLHOP) {
				fal_rule->l3_ttl = slice->rule.ip.hop_limit;
				fal_rule->l3_ttl_mask = slice->rule.ip.hop_limit_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_TTL);
				ppe_drv_trace("%p: slice ip misc ttl_hop: %d, mask: 0x%x",
					ctx, fal_rule->l3_ttl, fal_rule->l3_ttl_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L3STATE) {
				fal_rule->is_ipv4_option_val = slice->rule.ip.l3_state_option_flag;
				fal_rule->is_ipv4_option_mask = slice->rule.ip.l3_state_option_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_TTL);
				ppe_drv_trace("%p: slice ip misc option: %d, mask: 0x%x",
					ctx, fal_rule->is_ipv4_option_val, fal_rule->is_ipv4_option_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_TCPFLAG) {
				fal_rule->tcp_flag_val = slice->rule.ip.tcp_flags;
				fal_rule->tcp_flag_mask = slice->rule.ip.tcp_flags_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_TCP_FLAG);
				ppe_drv_trace("%p: slice ip misc tcp flag: %d, mask: 0x%x",
					ctx, fal_rule->tcp_flag_val, fal_rule->tcp_flag_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L31STFRAG) {
				fal_rule->is_first_frag_val = slice->rule.ip.l3_1st_fragment_flag;
				fal_rule->is_first_frag_mask = slice->rule.ip.l3_1st_fragment_flag_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_FIRST_FRAGMENT);
				ppe_drv_trace("%p: slice ip misc first frag: %d, mask: 0x%x",
					ctx, fal_rule->is_first_frag_val, fal_rule->is_first_frag_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_DSCPTC) {
				fal_rule->ip_dscp_val = slice->rule.ip.l3_dscp_tc;
				fal_rule->ip_dscp_mask = slice->rule.ip.l3_dscp_tc_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP_DSCP);
				ppe_drv_trace("%p: slice ip misc dscp_tc: %d, mask: 0x%x",
					ctx, fal_rule->ip_dscp_val, fal_rule->ip_dscp_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L4PROTO) {
				fal_rule->ip_proto_val = slice->rule.ip.l3_v4proto_v6nexthdr;
				fal_rule->ip_proto_mask = slice->rule.ip.l3_v4proto_v6nexthdr_mask;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_IP_PROTO);
				ppe_drv_trace("%p: slice ip misc l4 proto: %d, mask: 0x%x",
					ctx, fal_rule->ip_proto_val, fal_rule->ip_proto_mask);
			}

			if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L3LEN) {
				fal_rule->l3_length = slice->rule.ip.l3_length;
				fal_rule->l3_length_mask = slice->rule.ip.l3_length_mask_max;
				fal_rule->l3_length_op = slice->rule.ip.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
				FAL_FIELD_FLG_SET(fal_rule->field_flg, FAL_ACL_FIELD_L3_LENGTH);
				ppe_drv_trace("%p: slice ip misc l3 len: %d, mask: 0x%x",
					ctx, fal_rule->l3_length, fal_rule->l3_length_mask);
			}

			/*
			 * If rule type is already set due to multiple valid slices,
			 * do not overwrite the rule_type.
			 */
			if (!ctx->rule_type_valid) {
				ctx->rule_type_valid = true;
				fal_rule->rule_type = FAL_ACL_RULE_IP4;
			} else if (fal_rule->rule_type != FAL_ACL_RULE_IP6) {
				fal_rule->rule_type = FAL_ACL_RULE_IP4;
			}

			break;

		case PPE_DRV_ACL_SLICE_TYPE_UDF_012:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_UDF_012];

			if (slice->rule.udf_012.udf_a_valid) {
				fal_rule->udf0_val = slice->rule.udf_012.udf_a_min;
				fal_rule->udf0_mask = slice->rule.udf_012.udf_a_mask_max;
				fal_rule->udf0_op = slice->rule.udf_012.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
			}

			if (slice->rule.udf_012.udf_b_valid) {
				fal_rule->udf1_val = slice->rule.udf_012.udf_b;
				fal_rule->udf1_mask = slice->rule.udf_012.udf_b_mask;
			}

			if (slice->rule.udf_012.udf_c_valid) {
				fal_rule->udf2_val = slice->rule.udf_012.udf_c;
				fal_rule->udf2_mask = slice->rule.udf_012.udf_c_mask;
			}

			/*
			 * If rule type is not set already then set rule_type to UDF.
			 */
			if (!ctx->rule_type_valid) {
				ctx->rule_type_valid = true;
				fal_rule->rule_type = FAL_ACL_RULE_UDF;
			}

			break;

		case PPE_DRV_ACL_SLICE_TYPE_UDF_123:
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_UDF_123];

			if (slice->rule.udf_123.udf_a_valid) {
				fal_rule->udf1_val = slice->rule.udf_123.udf_a_min;
				fal_rule->udf1_mask = slice->rule.udf_123.udf_a_mask_max;
				fal_rule->udf1_op = slice->rule.udf_123.range_en ?
						FAL_ACL_FIELD_RANGE : FAL_ACL_FIELD_MASK;
			}

			if (slice->rule.udf_123.udf_b_valid) {
				fal_rule->udf2_val = slice->rule.udf_123.udf_b;
				fal_rule->udf2_mask = slice->rule.udf_123.udf_b_mask;
			}

			if (slice->rule.udf_123.udf_c_valid) {
				fal_rule->udf3_val = slice->rule.udf_123.udf_c;
				fal_rule->udf3_mask = slice->rule.udf_123.udf_c_mask;
			}

			/*
			 * If rule type is not set already then set rule_type to UDF.
			 */
			if (!ctx->rule_type_valid) {
				ctx->rule_type_valid = true;
				fal_rule->rule_type = FAL_ACL_RULE_UDF;
			}

			break;

		default:
			ppe_drv_warn("%p: invalid slice_type: %d", ctx, slice_type);
			return false;
		}

	}

	/*
	 * Fill the common fields.
	 */
	fal_rule->post_routing = info->cmn.post_routing_en;
	fal_rule->qos_res_prec = info->cmn.qos_res_pre;
	fal_rule->acl_pool = info->cmn.res_chain;

	/*
	 * Update the rule type based on pre-IPO configuration.
	 */
	if (ctx->type == PPE_DRV_ACL_PREIPO) {
		switch (fal_rule->rule_type) {
		case FAL_ACL_RULE_MAC:
			fal_rule->rule_type = FAL_ACL_RULE_TUNNEL_MAC;
			break;
		case FAL_ACL_RULE_IP4:
			fal_rule->rule_type = FAL_ACL_RULE_TUNNEL_IP4;
			break;
		case FAL_ACL_RULE_IP6:
			fal_rule->rule_type = FAL_ACL_RULE_TUNNEL_IP6;
			break;
		case FAL_ACL_RULE_UDF:
			fal_rule->rule_type = FAL_ACL_RULE_TUNNEL_UDF;
			break;
		default:
			ppe_drv_warn("%p: invalid rule_type: %d", ctx, fal_rule->rule_type);
		}
	}

	ppe_drv_trace("%p: post_routing: %d, qos_res_prec: %d, res_chain: %d rule_type: %d",
			ctx, fal_rule->post_routing, fal_rule->qos_res_prec,
			fal_rule->acl_pool, fal_rule->rule_type);

	return true;
}

/*
 * ppe_drv_acl_action_fill()
 *	Fill action related information.
 */
static bool ppe_drv_acl_action_fill(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_rule *info)
{
	fal_acl_rule_t *fal_rule = &ctx->fal_rule;
	struct ppe_drv_acl_action *action = &info->action;

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_CC) {
		fal_rule->cpu_code = action->cpu_code;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_CPU_CODE);
		ppe_drv_trace("%p: action CC: %d", ctx, fal_rule->cpu_code);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_SYN_TOGGLE) {
		/*
		 * TODO check this.
		 */
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_SC) {
		fal_rule->service_code = action->service_code;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_SERVICE_CODE);
		ppe_drv_trace("%p: action SC: %d", ctx, fal_rule->service_code);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_QID) {
		fal_rule->queue = action->qid;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_QUEUE);
		ppe_drv_trace("%p: action qid: %d", ctx, fal_rule->queue);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_ENQUEUE_PRI) {
		fal_rule->enqueue_pri = action->enqueue_pri;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_ENQUEUE_PRI);
		ppe_drv_trace("%p: action enqueue pri: %d", ctx, fal_rule->enqueue_pri);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_CTAG_DEI) {
		fal_rule->ctag_cfi = action->ctag_dei;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_CFI);
		ppe_drv_trace("%p: action cdei: %d", ctx, fal_rule->ctag_cfi);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_CTAG_PCP) {
		fal_rule->ctag_pri = action->ctag_pcp;
		fal_rule->ctag_fmt = 1;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_PRI);
		ppe_drv_trace("%p: action cpcp: %d", ctx, fal_rule->ctag_pri);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_STAG_DEI) {
		fal_rule->stag_dei = action->stag_dei;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_DEI);
		ppe_drv_trace("%p: action sdei: %d", ctx, fal_rule->stag_dei);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_STAG_PCP) {
		fal_rule->stag_pri = action->stag_pcp;
		fal_rule->stag_fmt = 1;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_PRI);
		ppe_drv_trace("%p: action spcp: %d", ctx, fal_rule->stag_pri);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_DSCP_TC) {
		fal_rule->dscp = action->dscp_tc;
		fal_rule->dscp_mask = 0xff;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_DSCP);
		ppe_drv_trace("%p: action dscp_tc: %d", ctx, fal_rule->dscp);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_CVID) {
		fal_rule->ctag_vid = action->cvid;
		fal_rule->ctag_fmt = 1;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_CTAG_VID);
		ppe_drv_trace("%p: action cvid: %d", ctx, fal_rule->ctag_vid);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_SVID) {
		fal_rule->stag_vid = action->svid;
		fal_rule->stag_fmt = 1;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REMARK_STAG_VID);
		ppe_drv_trace("%p: action svid: %d", ctx, fal_rule->stag_vid);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_DST_INFO) {
		switch (action->dest_type) {
		case PPE_DRV_ACL_DST_TYPE_PORT_BITMAP:
			fal_rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_PORT_BMP, action->dest_info);
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REDPT);
			break;
		case PPE_DRV_ACL_DST_TYPE_NEXTHOP:
			fal_rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_NEXTHOP, action->dest_info);
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REDPT);
			break;
		case PPE_DRV_ACL_DST_TYPE_PORT_ID:
			fal_rule->ports = FAL_ACL_DEST_OFFSET(FAL_ACL_DEST_PORT_ID, action->dest_info);
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_REDPT);
			break;
		}
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_BYPASS_BITMAP) {
		/*
		 * TODO: check this.
		 */
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_POLICER_INDEX) {
		fal_rule->policer_ptr = action->policer_index;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_POLICER_EN);
		ppe_drv_trace("%p: action policer index: %d", ctx, fal_rule->policer_ptr);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_FWD_CMD) {
		switch (action->fwd_cmd) {
		case PPE_DRV_ACL_FWD_CMD_FWD:
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_PERMIT);
			ppe_drv_trace("%p: action cmd fwd", ctx);
			break;
		case PPE_DRV_ACL_FWD_CMD_DROP:
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_DENY);
			ppe_drv_trace("%p: action cmd drop", ctx);
			break;
		case PPE_DRV_ACL_FWD_CMD_COPY:
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_CPYCPU);
			ppe_drv_trace("%p: action cmd copy", ctx);
			break;
		case PPE_DRV_ACL_FWD_CMD_REDIR:
			FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_RDTCPU);
			ppe_drv_trace("%p: action cmd redir", ctx);
			break;
		}

	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_INT_DP) {
		fal_rule->int_dp = action->int_dp;
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_INT_DP);
		ppe_drv_trace("%p: action DP: %d", ctx, fal_rule->int_dp);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_STAG_FMT_TAGGED) {
		/*
		 * TODO check this.
		 */
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_CTAG_FMT_TAGGED) {
		/*
		 * TODO check this.
		 */
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_MIRROR_EN) {
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_MIRROR);
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_METADATA_EN);
		ppe_drv_trace("%p: action mirroring", ctx);
	}

	if (action->flags & PPE_DRV_ACL_ACTION_FLAG_METADATA_EN) {
		FAL_ACTION_FLG_SET(fal_rule->action_flg, FAL_ACL_ACTION_METADATA_EN);
		ppe_drv_trace("%p: ACL metadata mirroring", ctx);
	}

	return true;
}

/*
 * ppe_drv_acl_fill()
 *	Fill ACL rule and action information
 */
static bool ppe_drv_acl_fill(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_rule *info)
{
	if (!ppe_drv_acl_rule_fill(ctx, info)) {
		ppe_drv_warn("%p: ACL rule fill fail: %p", ctx, info);
		return false;
	}

	if (!ppe_drv_acl_action_fill(ctx, info)) {
		ppe_drv_warn("%p: ACL action fill fail: %p", ctx, info);
		return false;
	}

	return true;
}

/*
 * ppe_drv_acl_src_get()
 *	Map the source for ACL binding.
 */
static void ppe_drv_acl_src_get(struct ppe_drv_acl_rule *info, fal_acl_bind_obj_t *bind_obj)
{
	if (info->stype == PPE_DRV_ACL_SRC_TYPE_PORT_BITMAP) {
		*bind_obj = FAL_ACL_BIND_PORTBITMAP;
	} else if (info->stype == PPE_DRV_ACL_SRC_TYPE_PORT_NUM) {
		*bind_obj = FAL_ACL_BIND_PORT;
	} else if (info->stype == PPE_DRV_ACL_SRC_TYPE_SC) {
		*bind_obj = FAL_ACL_BIND_SERVICE_CODE;
	} else if (info->stype == PPE_DRV_ACL_SRC_TYPE_DEST_L3) {
		*bind_obj = FAL_ACL_BIND_L3_IF;
	}
}

/*
 * ppe_drv_acl_stats_update()
 *	Update hardware match counters.
 */
void ppe_drv_acl_stats_update(struct ppe_drv_acl_ctx *ctx)
{
	sw_error_t err;
	uint32_t delta_pkts;
	uint32_t delta_bytes;
	fal_entry_counter_t acl_cntrs = {0};

	ppe_drv_trace("%p: updating acl stats", ctx);

	err = fal_acl_counter_get(PPE_DRV_SWITCH_ID, ctx->fal_rule.hw_info.hw_rule_id, &acl_cntrs);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get stats for acl rule id: %u",
				ctx, ctx->fal_rule.hw_info.hw_rule_id);
		return;
	}

	/*
	 * PPE stats are not clear on read, so we need to calculate the delta
	 * between the latest counters and previously read counters.
	 */
	delta_pkts = PPE_DRV_ACL_PKT_CNTR_ROLLOVER(acl_cntrs.matched_pkts - ctx->pre_cntrs.matched_pkts);
	delta_bytes = PPE_DRV_ACL_BYTE_CNTR_ROLLOVER(acl_cntrs.matched_bytes - ctx->pre_cntrs.matched_bytes);

	/*
	 * Update hardware stats packet and byte counters
	 */
	ppe_drv_acl_stats_add(ctx, delta_pkts, delta_bytes);

	/*
	 * Store current stats for next iteration.
	 */
	ctx->pre_cntrs.matched_pkts = acl_cntrs.matched_pkts;
	ctx->pre_cntrs.matched_bytes = acl_cntrs.matched_bytes;

	ppe_drv_trace("%p: updating stats for acl [index:%u] - curr pkt:%u byte:%llu",
			ctx, ctx->fal_rule.hw_info.hw_rule_id,
			acl_cntrs.matched_pkts, acl_cntrs.matched_bytes);
}

/*
 * ppe_drv_acl_sc_return()
 *	Return a flow ACL sc.
 */
void ppe_drv_acl_sc_return(uint8_t sc)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;
	uint8_t i;

	spin_lock_bh(&p->lock);
	for (i = 0; i < PPE_DRV_SC_FLOW_ACL_MAX; i++) {
		if (acl->acl_sc[i].sc == sc) {
			acl->acl_sc[i].in_use = false;
			ppe_drv_info("%p: returned the service code: %d", acl, sc);
			spin_unlock_bh(&p->lock);
			return;
		}
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_warn("%p: not able to find a matching flow ACL service code: %d", acl, sc);
}
EXPORT_SYMBOL(ppe_drv_acl_sc_return);

/*
 * ppe_drv_acl_sc_get()
 *	Reserve an unused flow ACL sc.
 */
uint8_t ppe_drv_acl_sc_get()
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;
	uint8_t i;

	spin_lock_bh(&p->lock);
	for (i = 0; i < PPE_DRV_SC_FLOW_ACL_MAX; i++) {
		if (!acl->acl_sc[i].in_use) {
			acl->acl_sc[i].in_use = true;
			ppe_drv_info("%p: found a free service code: %d", acl, acl->acl_sc[i].sc);
			spin_unlock_bh(&p->lock);
			return acl->acl_sc[i].sc;
		}
	}

	spin_unlock_bh(&p->lock);
	ppe_drv_warn("%p: not able to find a free flow ACL service code", acl);
	return 0;
}
EXPORT_SYMBOL(ppe_drv_acl_sc_get);

/*
 * ppe_drv_acl_process_skbuff()
 *	Process skbuff with a valid ACL index.
 */
bool ppe_drv_acl_process_skbuff(struct ppe_drv_acl_metadata *acl_info, struct sk_buff *skb)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = NULL;
	ppe_drv_acl_rule_callback_t acl_rule_cb = NULL;
	void *app_data = NULL;

	ppe_drv_assert((acl_info->acl_hw_index >= 0) && (acl_info->acl_hw_index < PPE_DRV_ACL_HW_INDEX_MAX), "%p: invalid ACL hw index %u", p, acl_info->acl_hw_index);

	spin_lock_bh(&p->lock);
	acl = p->acl;
	acl_rule_cb = acl->acl_rule_cb;
	app_data = acl->acl_rule_app_data;
	spin_unlock_bh(&p->lock);

	/*
	 * Send to packet to PPE Rule module to process
	 * based on ACL ID.
	 */
	if (!acl_rule_cb) {
		ppe_drv_warn("%p: NO callback is registered to process ACL rules\n", p);
		return false;
	}

	return acl_rule_cb(app_data, skb, (void*)acl_info);
}
EXPORT_SYMBOL(ppe_drv_acl_process_skbuff);

/*
 * ppe_drv_acl_hw_info_get()
 *	Return hardware rule information for an ACL rule.
 */
void ppe_drv_acl_hw_info_get(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_hw_info *hw_info)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	hw_info->hw_rule_id = ctx->fal_rule.hw_info.hw_rule_id;
	hw_info->hw_list_id = ctx->fal_rule.hw_info.hw_list_id;
	hw_info->hw_num_slices = ctx->fal_rule.hw_info.hw_list_id;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_hw_info_get);

/*
 * ppe_drv_acl_configure()
 *	Configure slices for ACL rule.
 */
ppe_drv_ret_t ppe_drv_acl_configure(struct ppe_drv_acl_ctx *ctx, struct ppe_drv_acl_rule *info)
{
	sw_error_t error;
	fal_acl_bind_obj_t bind_obj;
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	if (!ppe_drv_acl_fill(ctx, info)) {
		ppe_drv_stats_inc(&p->stats.acl_stats.configure_fail);
		ppe_drv_warn("%p: Invalid ACL rule %p\n", ctx, info);
		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_ACL_RULE_INVALID;
	}

	error = fal_acl_rule_add(PPE_DRV_SWITCH_ID, ctx->list_id, PPE_DRV_ACL_RULE_ID, PPE_DRV_ACL_RULE_NR, &ctx->fal_rule);
	if (error != SW_OK) {
		ppe_drv_stats_inc(&p->stats.acl_stats.rule_add_fail);
		ppe_drv_warn("%p: Could not add acl rule, error = %d\n", ctx, error);
		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_ACL_RULE_ADD_FAIL;
	}

	/*
	 * Bind ACL list with source
	 */
	ppe_drv_acl_src_get(info, &bind_obj);
	error = fal_acl_list_bind(PPE_DRV_SWITCH_ID, ctx->list_id, FAL_ACL_DIREC_IN, bind_obj, info->src);
	if (error != SW_OK) {
		ppe_drv_stats_inc(&p->stats.acl_stats.rule_bind_fail);
		ppe_drv_warn("%p: Could not bind ACL list, error = %d, list_id: %d\n", ctx, error, ctx->list_id);

		error = fal_acl_rule_delete(PPE_DRV_SWITCH_ID, ctx->list_id, PPE_DRV_ACL_RULE_ID, PPE_DRV_ACL_RULE_NR);
		if (error != SW_OK) {
			ppe_drv_warn("%p: Rule delete failed error: %d, list_id: %d\n", ctx, error, ctx->list_id);
		}

		spin_unlock_bh(&p->lock);
		return PPE_DRV_RET_ACL_RULE_BIND_FAIL;
	}

	ctx->rule_valid = true;
	ppe_drv_acl_dump(ctx);
	ppe_drv_info("Created ACL rule\n");
	spin_unlock_bh(&p->lock);
	return PPE_DRV_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_drv_acl_configure);

/*
 * ppe_drv_acl_enable_mirror_capture_core
 *	Enable the capture core for mirrored packets.
 */
bool ppe_drv_acl_enable_mirror_capture_core(uint8_t core_id)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;
	ppe_drv_acl_mirror_core_select_cb_t mirror_core_cb;
	void *mirror_core_app_data;

	spin_lock_bh(&p->lock);
	mirror_core_cb = acl->mirror_core_cb;
	mirror_core_app_data = acl->mirror_core_app_data;

	if (!mirror_core_cb) {
		spin_unlock_bh(&p->lock);
		ppe_drv_warn("%p No mirror core select callback is registered \n", acl);
		return false;
	}

	spin_unlock_bh(&p->lock);

	mirror_core_cb(core_id, mirror_core_app_data);
	return true;
}
EXPORT_SYMBOL(ppe_drv_acl_enable_mirror_capture_core);

/*
 * ppe_drv_acl_alloc()
 *	Allocate slices for ACL rules.
 */
struct ppe_drv_acl_ctx *ppe_drv_acl_alloc(ppe_drv_acl_ipo_t type, uint8_t num_slices, uint16_t pri)
{
	/*
	 * TODO: Optimize based on SSDK implementation.
	 *
	 * Find the best slice which fills up the whole if there is any.
	 *
	 * struct ppe_drv_acl_row *row;
	 * uint8_t bm;
	 *
	 * 1. for (row_num = 0; row_num < PPE_DRV_ACL_MAX_ROW; row_num++) {
	 * 	row = &p->acl.rows[row_num];
	 * 	bm = row->slice_bitmap;
	 *
	 * 	if ((bm & 0xaa) ^ ((bm & 0x55) << 1)) {
	 * 		// found a single slice.
	 * 	}
	 *
	 * 	if ((bm & 0xcc) ^ ((bm & 0x33) << 2)) {
	 * 		// found a dobule slice.
	 * 	}
	 *
	 * 	if ((bm && 0xf0) ^ ((bm & 0xf) << 4)) {
	 * 		//found a four slice slot.
	 * 	}
	 *
	 * 2. fill up the dummy slices if any.
	 *
	 * 3. update slice binding depending on number of slices required.
	 *
	 * 4. Update the used slice_bitmap
	 *
	 * 5. update the ACL row pointer in rule context
	 *
	 * 6. Update the first ACL slice pointer in rule context.
	 *
	 */

	sw_error_t error;
	int16_t list_id = -1;
	struct ppe_drv_acl_ctx *ctx = NULL;
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	ctx = ppe_drv_acl_ctx_alloc();
	if (!ctx) {
		ppe_drv_stats_inc(&p->stats.acl_stats.list_id_full);
		ppe_drv_warn("No free list_id for type: %d\n", type);
		return NULL;
	}

	spin_lock_bh(&p->lock);
nxt_list:
	list_id = ppe_drv_acl_list_id_get(type);
	if (list_id < 0) {
		ppe_drv_stats_inc(&p->stats.acl_stats.list_id_full);
		ppe_drv_warn("No free list_id for type: %d\n", type);
		goto fail;
	}

	error = fal_acl_list_creat(PPE_DRV_SWITCH_ID, list_id, pri);
	if (error != SW_OK) {
		/*
		 * If this list id is already used by other sub system, mark it
		 * as reserved and try next one.
		 *
		 * The ones marked as RESERVED are permanently marked as reserved,
		 * this is expected to go away once all the subsystems move to NSS
		 * acl model.
		 *
		 * TODO: print the reserved LIST IDs through debugfs.
		 */
		if (error == SW_ALREADY_EXIST) {
			ppe_drv_warn("%p: list ID: %d already in use!\n", acl, list_id);
			acl->list_id[list_id].list_id_state = PPE_DRV_ACL_LIST_ID_RESERVED;
			goto nxt_list;
		}

		/*
		 * for other errors, flag the failure.
		 */
		ppe_drv_stats_inc(&p->stats.acl_stats.list_create_fail);
		ppe_drv_warn("List creation failed for list_id: %d, pri: %d with error: %d\n",
				list_id, pri, error);
		goto fail;
	}

	/*
	 * return rule context pointer.
	 */
	ctx->type = type;
	ctx->list_id = list_id;
	ctx->req_slices = num_slices;
	ctx->total_slices = ppe_drv_acl_get_total_slices(num_slices);
	acl->list_id[list_id].ctx = ctx;
	spin_unlock_bh(&p->lock);
	ppe_drv_info("%p: acl rule allocated ctx: %p, list_id: %d, num_slices: %d, pri: %d, type: %d",
			acl, ctx, list_id, num_slices, pri, type);

	ppe_drv_stats_add(&p->stats.acl_stats.req_slices, ctx->req_slices);
	ppe_drv_stats_add(&p->stats.acl_stats.total_slices, ctx->total_slices);
	ppe_drv_stats_inc(&p->stats.acl_stats.active_rules);
	return ctx;
fail:
	if (list_id != -1) {
		ppe_drv_acl_list_id_return(list_id);
	}

	if (ctx) {
		ppe_drv_acl_ctx_free(ctx);
	}

	spin_unlock_bh(&p->lock);
	return NULL;
}
EXPORT_SYMBOL(ppe_drv_acl_alloc);

/*
 * ppe_drv_acl_get_hw_stats
 *	Get the hardware stats for an ACL rule.
 */
void ppe_drv_acl_get_hw_stats(struct ppe_drv_acl_ctx *ctx, uint64_t *pkts, uint64_t *bytes)
{
	struct ppe_drv *p = &ppe_drv_gbl;

	spin_lock_bh(&p->lock);
	*pkts = atomic64_read(&ctx->total_pkts);
	*bytes = atomic64_read(&ctx->total_bytes);
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_get_hw_stats);

/*
 * ppe_drv_acl_mirror_core_select_unregister_cb
 *	Callback unregistration for mirror core selection.
 */
void ppe_drv_acl_mirror_core_select_unregister_cb(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	spin_lock_bh(&p->lock);
	acl->mirror_core_cb = NULL;
	acl->mirror_core_app_data = NULL;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_mirror_core_select_unregister_cb);

/*
 * ppe_drv_acl_mirror_core_select_register_cb
 *	Callback registration for mirror core selection.
 */
void ppe_drv_acl_mirror_core_select_register_cb(ppe_drv_acl_mirror_core_select_cb_t cb, void *app_data)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	spin_lock_bh(&p->lock);
	acl->mirror_core_cb = cb;
	acl->mirror_core_app_data = app_data;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_mirror_core_select_register_cb);

/*
 * ppe_drv_acl_rule_unregister_cb
 *	Callback unregistration for ACL rule processing.
 */
void ppe_drv_acl_rule_unregister_cb()
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	spin_lock_bh(&p->lock);
	acl->acl_rule_cb = NULL;
	acl->acl_rule_app_data = NULL;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_rule_unregister_cb);

/*
 * ppe_drv_acl_rule_register_cb
 *	Callback registration for ACL rule processing.
 */
void ppe_drv_acl_rule_register_cb(ppe_drv_acl_rule_callback_t acl_cb, void *app_data)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	spin_lock_bh(&p->lock);
	acl->acl_rule_cb = acl_cb;
	acl->acl_rule_app_data = app_data;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_rule_register_cb);

/*
 * ppe_drv_acl_flow_unregister_cb
 *	Callback unregistration for flow + ACL combination.
 */
void ppe_drv_acl_flow_unregister_cb()
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	spin_lock_bh(&p->lock);
	acl->flow_add_cb = NULL;
	acl->flow_del_cb = NULL;
	acl->flow_app_data = NULL;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_flow_unregister_cb);

/*
 * ppe_drv_acl_flow_register_cb
 *	Callback registration for flow + ACL combination.
 */
void ppe_drv_acl_flow_register_cb(ppe_drv_acl_flow_callback_t add_cb, ppe_drv_acl_flow_callback_t del_cb, void *app_data)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl = p->acl;

	spin_lock_bh(&p->lock);
	acl->flow_add_cb = add_cb;
	acl->flow_del_cb = del_cb;
	acl->flow_app_data = app_data;
	spin_unlock_bh(&p->lock);
}
EXPORT_SYMBOL(ppe_drv_acl_flow_register_cb);

/*
 * ppe_drv_acl_entries_free()
 *	Free acl instance.
 */
void ppe_drv_acl_entries_free(struct ppe_drv_acl *acl)
{
	vfree(acl);
}

/*
 * ppe_drv_acl_entries_alloc()
 *	Allocates ACL entries.
 */
struct ppe_drv_acl *ppe_drv_acl_entries_alloc(void)
{
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_acl *acl;
	uint8_t i;
	int id;

	acl = vzalloc(sizeof(struct ppe_drv_acl));
	if (!acl) {
		ppe_drv_warn("%p: Failed to allocate ACL table entries", p);
		return NULL;
	}

	/*
	 * Initialize list_id.
	 */
	for (id = 0; id < PPE_DRV_ACL_LIST_ID_MAX; id++) {
		acl->list_id[id].list_id_state = PPE_DRV_ACL_LIST_ID_FREE;
	}

	/*
	 * Initialize FLOW ACL service code.
	 */
	for (i = 0; i < PPE_DRV_SC_FLOW_ACL_MAX; i++) {
		acl->acl_sc[i].sc = PPE_DRV_SC_FLOW_ACL_FIRST + i;
		acl->acl_sc[i].in_use = false;
	}

	return acl;
}
