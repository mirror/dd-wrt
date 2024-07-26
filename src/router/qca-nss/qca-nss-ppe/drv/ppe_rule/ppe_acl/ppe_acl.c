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

#include <linux/vmalloc.h>
#include <linux/netdevice.h>
#include <ppe_drv_public.h>
#include <ppe_drv_acl.h>
#if !defined(NSS_PPE_LOWMEM_PROFILE_16M)
#include "../ppe_policer/ppe_policer.h"
#endif
#include "ppe_acl.h"

/*
 * Global ACL context
 */
struct ppe_acl_base ppe_acl_gbl = {0};

/*
 * ppe_acl_alloc()
 *      Allocate memory for an ACL rule.
 */
static inline struct ppe_acl *ppe_acl_alloc(void)
{
	/*
	 * Allocate memory for a new rule
	 *
	 * kzalloc with GFP_ATOMIC is used assuming ACL rules can be created
	 * in softirq context as well while processing an SKB in the system.
	 */
	return kzalloc(sizeof(struct ppe_acl), GFP_ATOMIC);
}

/*
 * ppe_acl_free()
 *      Free v4 connections.
 */
static inline void ppe_acl_free(struct ppe_acl *acl)
{
	kfree(acl);
}

/*
 * ppe_acl_rule_return_gen_id()
 *	Return a rule ID to general pool.
 */
static void ppe_acl_rule_return_gen_id(ppe_acl_rule_id_t gen_id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	if ((gen_id < PPE_ACL_GEN_RULE_ID_BASE) || (gen_id > PPE_ACL_RULE_ID_MAX)) {
		ppe_acl_warn("%p: Invalid gneral rule ID: %d", acl_g, gen_id);
		return;
	}

	/*
	 * Clear the in_use state for rule ID.
	 */
	acl_g->rule_id_tbl[gen_id - PPE_ACL_GEN_RULE_ID_BASE].in_use = false;
	ppe_acl_trace("%p: Return the rule ID to free pool: %d", acl_g, gen_id);
}

/*
 * ppe_acl_rule_get_gen_id()
 *	Find a free rule ID from general pool.
 */
static ppe_acl_rule_id_t ppe_acl_rule_get_gen_id(void)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	uint16_t i;

	/*
	 * Get the first available acl rule ID.
	 */
	for (i = 0; i < PPE_ACL_GEN_RULE_ID_MAX; i++) {
		if (!acl_g->rule_id_tbl[i].in_use) {
			acl_g->rule_id_tbl[i].in_use = true;
			return acl_g->rule_id_tbl[i].rule_id;
		}
	}

	ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_rule_table_full);
	ppe_acl_trace("%p: cannot allocate a free rule ID", acl_g);
	return -1;
}

/*
 * ppe_acl_rule_find_by_id()
 *	Find a rule corresponding to a rule ID.
 */
static struct ppe_acl *ppe_acl_rule_find_by_id(ppe_acl_rule_id_t id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *acl;

	if (id >= PPE_ACL_RULE_ID_MAX) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_id_invalid);
		ppe_acl_warn("%p: Invalid rule ID: %d", acl_g, id);
		return NULL;
	}

	/*
	 * Get the first available acl rule ID.
	 */
	if (!list_empty(&acl_g->active_rules)) {
		list_for_each_entry(acl, &acl_g->active_rules, list) {
			if (acl->rule_id == id) {
				ppe_acl_info("%p: ACL rule: %p found for ID: %d",
					acl_g, acl, id);
				return acl;
			}
		}
	}

	ppe_acl_stats_inc(&acl_g->stats.cmn.rule_not_found);
	ppe_acl_warn("%p: No valid ACL rule for ID: %d", acl_g, id);
	return NULL;
}

/*
 * ppe_acl_rule_free()
 *	Free the acl rule.
 */
static void ppe_acl_rule_free(struct kref *kref)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *acl = container_of(kref, struct ppe_acl, ref_cnt);

	/*
	 * Delete the rule node from active list.
	 */
	list_del(&acl->list);

	/*
	 * Return the service code if allocated.
	 */
	if (acl->sc) {
		ppe_drv_acl_sc_return(acl->sc);
		acl->sc = 0;
	}

	/*
	 * Destroy the rule in PPE driver.
	 */
	if (acl->ctx) {
		ppe_drv_acl_destroy(acl->ctx);
		acl->ctx = NULL;
	}

	/*
	 * TODO: Collect the latest stats from hardware?
	 * clear the stats in hardware?
	 */
	ppe_acl_info("%p: ACL rule freed: %u", acl, acl->rule_id);

	if (acl->rule_id >= PPE_ACL_GEN_RULE_ID_BASE) {
		ppe_acl_rule_return_gen_id(acl->rule_id);
	}

	/*
	 * free the acl rule memory.
	 */
	ppe_acl_free(acl);

	/*
	 * Update stats
	 */
	ppe_acl_stats_inc(&acl_g->stats.cmn.acl_free_req);
}

/*
 * ppe_acl_rule_process_skb()
 *	Process the packet based on ACL rule.
 */
bool ppe_acl_rule_process_skb(void *appdata, struct sk_buff *skb, struct ppe_drv_acl_metadata *acl_info)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl_rule_hw_ind_map *acl_hw_map = NULL;
	struct ppe_acl *acl = NULL;
	uint16_t hw_index = acl_info->acl_hw_index;
	ppe_acl_rule_process_callback_t cb;
	void *app_data;

	if (hw_index < 0 || hw_index >= PPE_ACL_HW_INDEX_MAX) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_hw_index_invalid);
		ppe_acl_warn("%p: Invalid Hardware index: %d", acl_g, hw_index);
		return false;
	}

	spin_lock_bh(&acl_g->lock);

	/*
	 * Get the callback registered for this ACL ID and invoke the same.
	 */
	acl_hw_map = &acl_g->acl_hw_map[hw_index];
	acl = acl_hw_map->acl_rule;
	if (!acl) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_warn("%p: No ACL rule found for hw index %d", acl_g, hw_index);
		return false;
	}

	cb = acl->cb;
	app_data = acl->app_data;
	if (!cb) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_warn("%p: No callback registered for hw index %d", acl_g, hw_index);
		return false;
	}

	spin_unlock_bh(&acl_g->lock);

	return cb(app_data, (void *)skb);
}

/*
 * ppe_acl_rule_flow_del_cb()
 *	 Flow delete callback for n-tuple match.
 */
bool ppe_acl_rule_flow_del_cb(void *app_data, struct ppe_drv_acl_flow_bind *info)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *acl;

	/*
	 * Find the matching rule corresponding to the rule ID.
	 */
	spin_lock_bh(&acl_g->lock);
	acl = ppe_acl_rule_find_by_id(info->id);
	if (!acl) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_flow_del_invalid_id);
		ppe_acl_warn("%p: failed to find the rule for ID: %d", acl_g, info->id);
		spin_unlock_bh(&acl_g->lock);
		return false;
	}

	/*
	 * Dereference: during flow_del_cb()
	 * Reference: during flow_add_cb()
	 */
	if (kref_put(&acl->ref_cnt, ppe_acl_rule_free)) {
		ppe_acl_trace("%p: reference goes down to 0 for acl: %p ID: %d\n",
				acl_g, acl, info->id);
	}

	ppe_acl_info("%p: rule_id: %u ref dec: %u", acl_g, info->id, kref_read(&acl->ref_cnt));
	spin_unlock_bh(&acl_g->lock);
	return true;
}

/*
 * ppe_acl_rule_flow_add_cb()
 *	 Return a service code corresponding to rule ID for n-tuple match.
 */
bool ppe_acl_rule_flow_add_cb(void *app_data, struct ppe_drv_acl_flow_bind *info)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *acl;

	/*
	 * Find the matching rule corresponding to the rule ID.
	 */
	spin_lock_bh(&acl_g->lock);
	acl = ppe_acl_rule_find_by_id(info->id);
	if (!acl) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_flow_add_invalid_id);
		ppe_acl_warn("%p: failed to find the rule for ID: %d", acl_g, info->id);
		spin_unlock_bh(&acl_g->lock);
		return false;
	}

	if (!acl->sc) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_flow_add_invalid_sc);
		ppe_acl_warn("%p: ACL rule doesn't have a valid service code rule_id: %d", acl_g, info->id);
		spin_unlock_bh(&acl_g->lock);
		return false;
	}

	info->sc = acl->sc;
	ppe_acl_info("%p: flow add for rule_id: %u sc: %u", acl_g, info->id, info->sc);

	/*
	 * Reference: during flow_add_cb()
	 * Dereference: during flow_del_cb()
	 */
	kref_get(&acl->ref_cnt);

	ppe_acl_info("%p: rule_id: %u ref inc: %u", acl_g, info->id, kref_read(&acl->ref_cnt));
	spin_unlock_bh(&acl_g->lock);
	return true;
}

/*
 * ppe_acl_rule_destroy()
 *	Destroy ACL rule in PPE.
 */
ppe_acl_ret_t ppe_acl_rule_destroy(ppe_acl_rule_id_t id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *acl;

	/*
	 * Stats
	 */
	spin_lock_bh(&acl_g->lock);
	ppe_acl_stats_inc(&acl_g->stats.cmn.acl_destroy_req);

	/*
	 * Find the matching rule corresponding to the rule ID.
	 */
	acl = ppe_acl_rule_find_by_id(id);
	if (!acl) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_destroy_fail_invalid_id);
		ppe_acl_warn("%p: failed to find the rule for ID: %d", acl_g, id);
		spin_unlock_bh(&acl_g->lock);
		return PPE_ACL_RET_DESTROY_FAIL_INVALID_ID;
	}

	if (kref_put(&acl->ref_cnt, ppe_acl_rule_free)) {
		ppe_acl_trace("%p: reference goes down to 0 for acl: %p ID: %d\n",
				acl_g, acl, id);
	}

	ppe_acl_info("%p: rule_id: %u ref dec: %u", acl_g, id, kref_read(&acl->ref_cnt));
	spin_unlock_bh(&acl_g->lock);
	return PPE_ACL_RET_SUCCESS;
}
EXPORT_SYMBOL(ppe_acl_rule_destroy);

/*
 * ppe_acl_rule_to_slice_type
 *	Map ACL rule to PPE slice type.
 */
static void ppe_acl_rule_to_slice_type(struct ppe_acl_rule_match_one *r, ppe_acl_rule_match_type_t type, bool slice_type[])
{
	switch (type) {
	case PPE_ACL_RULE_MATCH_TYPE_SMAC:
		slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC] = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_DMAC:
		slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_MAC] = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_SVID:
	case PPE_ACL_RULE_MATCH_TYPE_CVID:
	case PPE_ACL_RULE_MATCH_TYPE_SPCP:
	case PPE_ACL_RULE_MATCH_TYPE_CPCP:
		slice_type[PPE_DRV_ACL_SLICE_TYPE_VLAN] = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS:
	case PPE_ACL_RULE_MATCH_TYPE_ETHER_TYPE:
		slice_type[PPE_DRV_ACL_SLICE_TYPE_L2_MISC] = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_L3_1ST_FRAG:
	case PPE_ACL_RULE_MATCH_TYPE_IP_LEN:
	case PPE_ACL_RULE_MATCH_TYPE_TTL_HOPLIMIT:
	case PPE_ACL_RULE_MATCH_TYPE_DSCP_TC:
	case PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR:
	case PPE_ACL_RULE_MATCH_TYPE_TCP_FLAG:
		slice_type[PPE_DRV_ACL_SLICE_TYPE_IP_MISC] = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_SIP:
		if (r->rule.sip.ip_type == PPE_ACL_IP_TYPE_V4) {
			slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4] = true;
		} else if (r->rule.sip.ip_type == PPE_ACL_IP_TYPE_V6) {
			slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0] = true;
			slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1] = true;
			slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2] = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_DIP:
		if (r->rule.dip.ip_type == PPE_ACL_IP_TYPE_V4) {
			slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4] = true;
		} else if (r->rule.dip.ip_type == PPE_ACL_IP_TYPE_V6) {
			slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0] = true;
			slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1] = true;
			slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2] = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_SPORT:
			/*
			 * If this is an independent sport rule without IP, the IP can be ignored
			 */
			if (!slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4] && !slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0]) {
				slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4] = true;
			}
		break;

	case PPE_ACL_RULE_MATCH_TYPE_DPORT:
			/*
			 * If this is an independent dport rule without IP, the IP can be ignored
			 */
			if (!slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4] && !slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0]) {
				slice_type[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4] = true;
			}
		break;


	case PPE_ACL_RULE_MATCH_TYPE_UDF:
		if (r->rule.udf.udf_a_valid || r->rule.udf.udf_b_valid || r->rule.udf.udf_c_valid) {
			slice_type[PPE_DRV_ACL_SLICE_TYPE_UDF_012] = true;
		}

		if (r->rule.udf.udf_d_valid) {
			slice_type[PPE_DRV_ACL_SLICE_TYPE_UDF_123] = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_DEFAULT:
		slice_type[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC] = true;
		break;
	default:
		ppe_acl_warn("%p: invalid rule type: %d", r, type);
	}
}

/*
 * ppe_acl_rule_info_fill()
 *	Fill the ppe-drv rule structure based on user information.
 */
static bool ppe_acl_rule_info_fill(struct ppe_acl *acl, struct ppe_acl_rule_match_one *r, ppe_acl_rule_match_type_t type)
{
	struct ppe_drv_acl_rule *info = &acl->info;
	struct ppe_drv_acl_rule_match_one *slice;
	struct ppe_drv_acl_mac *smac, *dmac;
	struct ppe_drv_acl_vlan *vlan;
	struct ppe_drv_acl_l2_misc *l2;
	struct ppe_drv_acl_ipv4 *sip_v4;
	struct ppe_drv_acl_ipv4 *dip_v4;
	struct ppe_drv_acl_ipv6 *dip_v6_0;
	struct ppe_drv_acl_ipv6 *dip_v6_1;
	struct ppe_drv_acl_ipv6 *dip_v6_2;
	struct ppe_drv_acl_ipv6 *sip_v6_0;
	struct ppe_drv_acl_ipv6 *sip_v6_1;
	struct ppe_drv_acl_ipv6 *sip_v6_2;
	struct ppe_drv_acl_ip_misc *ip;
	struct ppe_drv_acl_udf *udf_012;
	struct ppe_drv_acl_udf *udf_123;
	uint32_t sip[4], sip_mask[4];
	uint32_t dip[4], dip_mask[4];
	bool flag_en;

	switch (type) {
	case PPE_ACL_RULE_MATCH_TYPE_SMAC:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC];
		smac = &slice->rule.smac;

		smac->inverse_en = !(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		memcpy(smac->mac, r->rule.smac.mac, ETH_ALEN);
		memset(smac->mac_mask, 0xff, sizeof(smac->mac_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_MAC_MASK) {
			memcpy(smac->mac_mask, r->rule.smac.mac_mask, ETH_ALEN);
		}

		slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_MAC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_DMAC:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_MAC];
		dmac = &slice->rule.dmac;

		dmac->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		memcpy(dmac->mac, r->rule.dmac.mac, ETH_ALEN);
		memset(dmac->mac_mask, 0xff, sizeof(dmac->mac_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_MAC_MASK) {
			memcpy(dmac->mac_mask, r->rule.dmac.mac_mask, ETH_ALEN);
		}

		slice->type = PPE_DRV_ACL_SLICE_TYPE_DST_MAC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_SVID:
		if ((r->rule_flags & PPE_ACL_RULE_FLAG_VID_RANGE)) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC];
			l2 = &slice->rule.l2;

			l2->svid_min = r->rule.svid.vid_min;
			l2->svid_mask_max = r->rule.svid.vid_mask_max;
			l2->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_VID_RANGE);
			l2->inverse_en = !(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);

			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_L2_FLAG_SVID;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_L2_MISC;
			slice->valid = true;
		}

		/*
		 * TODO: clean this up for SVID range alone.
		 */
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_VLAN];
		vlan = &slice->rule.vlan;

		vlan->stag_fmt = (ppe_drv_acl_vtag_fmt_t)r->rule.svid.tag_fmt;
		vlan->svid = r->rule.svid.vid_min;
		memset(&vlan->svid_mask, 0xff, sizeof(vlan->svid_mask));
		if ((r->rule_flags & PPE_ACL_RULE_FLAG_VID_MASK) && !(r->rule_flags & PPE_ACL_RULE_FLAG_VID_RANGE)) {
			vlan->svid_mask = r->rule.svid.vid_mask_max;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (vlan->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all vlan rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		vlan->inverse_en = flag_en;

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_VLAN_FLAG_SVID;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_VLAN;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_CVID:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_VLAN];
		vlan = &slice->rule.vlan;

		vlan->ctag_fmt = (ppe_drv_acl_vtag_fmt_t)r->rule.cvid.tag_fmt;
		vlan->cvid_min = r->rule.cvid.vid_min;
		memset(&vlan->cvid_mask_max, 0xff, sizeof(vlan->cvid_mask_max));
		if ((r->rule_flags & PPE_ACL_RULE_FLAG_VID_MASK) || (r->rule_flags & PPE_ACL_RULE_FLAG_VID_RANGE)) {
			vlan->cvid_mask_max = r->rule.cvid.vid_mask_max;
			vlan->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_VID_RANGE);
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (vlan->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all vlan rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		vlan->inverse_en = flag_en;

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_VLAN_FLAG_CVID;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_VLAN;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_SPCP:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_VLAN];
		vlan = &slice->rule.vlan;

		slice->rule.vlan.spcp = r->rule.spcp.pcp;
		memset(&vlan->spcp_mask, 0xff, sizeof(vlan->spcp_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_PCP_MASK) {
			vlan->spcp_mask = r->rule.spcp.pcp_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (vlan->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all vlan rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		vlan->inverse_en = flag_en;

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_VLAN_FLAG_SPCP;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_VLAN;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_CPCP:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_VLAN];
		vlan = &slice->rule.vlan;

		vlan->cpcp = r->rule.cpcp.pcp;
		memset(&vlan->cpcp_mask, 0xff, sizeof(vlan->cpcp_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_PCP_MASK) {
			vlan->cpcp_mask = r->rule.cpcp.pcp_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (vlan->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all vlan rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		vlan->inverse_en = flag_en;

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_VLAN_FLAG_CPCP;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_VLAN;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_PPPOE_SESS:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC];
		l2 = &slice->rule.l2;

		l2->pppoe_session_id = r->rule.pppoe_sess.pppoe_session_id;
		memset(&l2->pppoe_session_id_mask, 0xff, sizeof(l2->pppoe_session_id_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_PPPOE_MASK) {
			l2->pppoe_session_id_mask = r->rule.pppoe_sess.pppoe_session_id_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (l2->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all l2 misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		l2->inverse_en = flag_en;

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_L2_MISC_FLAG_PPPOE;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_L2_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_ETHER_TYPE:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC];
		l2 = &slice->rule.l2;

		l2->l2_proto = r->rule.ether_type.l2_proto;
		memset(&l2->l2_proto_mask, 0xff, sizeof(l2->l2_proto_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_ETHTYPE_MASK) {
			l2->l2_proto_mask = r->rule.ether_type.l2_proto_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (l2->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all l2 misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_L2_MISC_FLAG_L2PROTO;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_L2_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_L3_1ST_FRAG:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		ip->l3_1st_fragment_flag = 1;
		ip->l3_1st_fragment_flag_mask = ip->l3_1st_fragment_flag;
		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_L31STFRAG;

		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_IP_LEN:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		ip->l3_length = r->rule.l3_len.l3_length_min;
		memset(&ip->l3_length_mask_max, 0xff, sizeof(ip->l3_length_mask_max));
		if ((r->rule_flags & PPE_ACL_RULE_FLAG_IPLEN_MASK) || (r->rule_flags & PPE_ACL_RULE_FLAG_IPLEN_RANGE)) {
			ip->l3_length_mask_max = r->rule.l3_len.l3_length_mask_max;
			ip->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_IPLEN_RANGE);
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_L3LEN;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_TTL_HOPLIMIT:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		ip->hop_limit = r->rule.ttl_hop.hop_limit;
		memset(&ip->hop_limit_mask, 0xff, sizeof(ip->hop_limit_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_TTL_HOPLIMIT_MASK) {
			ip->hop_limit_mask = r->rule.ttl_hop.hop_limit_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_TTLHOP;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_DSCP_TC:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		ip->l3_dscp_tc = r->rule.dscp_tc.l3_dscp_tc;
		memset(&ip->l3_dscp_tc_mask, 0xff, sizeof(ip->l3_dscp_tc_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_DSCP_TC_MASK) {
			ip->l3_dscp_tc_mask = r->rule.dscp_tc.l3_dscp_tc_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_DSCPTC;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_PROTO_NEXTHDR:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		ip->l3_v4proto_v6nexthdr = r->rule.proto_nexthdr.l3_v4proto_v6nexthdr;
		memset(&ip->l3_v4proto_v6nexthdr_mask, 0xff, sizeof(ip->l3_v4proto_v6nexthdr_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_PROTO_NEXTHDR_MASK) {
			ip->l3_v4proto_v6nexthdr_mask = r->rule.proto_nexthdr.l3_v4proto_v6nexthdr_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_L4PROTO;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_IP_GEN:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		if (r->rule_flags & PPE_ACL_RULE_FLAG_L3_FRAG) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_L3FRAG;
			ip->l3_fragment_flag = 1;
			ip->l3_fragment_flag_mask = 0x1;
		}

		if (r->rule_flags & PPE_ACL_RULE_FLAG_ESP_HDR) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_ESP_HDR;
			ip->esp_hdr_flag = 1;
			ip->esp_hdr_flag_mask = 0x1;
		}

		if (r->rule_flags & PPE_ACL_RULE_FLAG_AH_HDR) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_AH_HDR;
			ip->ah_hdr_flag = 1;
			ip->ah_hdr_flag_mask = 0x1;
		}

		if (r->rule_flags & PPE_ACL_RULE_FLAG_MOBILITY_HDR) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_MOB_HDR;
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_L4PROTO;
			ip->mobility_hdr_flag = 1;
			ip->mobility_hdr_flag_mask = 0x1;
		}

		if (r->rule_flags & PPE_ACL_RULE_FLAG_OTHER_EXT_HDR) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_OTHEXT_HDR;
			ip->other_extension_hdr_flag = 1;
			ip->other_extension_hdr_flag_mask = 0x1;
		}

		if (r->rule_flags & PPE_ACL_RULE_FLAG_FRAG_HDR) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_FRAG_HDR;
			ip->frag_hdr_flag = 1;
			ip->frag_hdr_flag_mask = 0x1;
		}

		if (r->rule_flags & PPE_ACL_RULE_FLAG_IPV4_OPTION) {
			slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_L3STATE;
			ip->l3_state_option_flag = 1;
			ip->l3_state_option_flag_mask = 0x1;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;
	case PPE_ACL_RULE_MATCH_TYPE_TCP_FLAG:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];
		ip = &slice->rule.ip;

		ip->tcp_flags = r->rule.tcp_flag.tcp_flags;
		memset(&ip->tcp_flags_mask, 0xff, sizeof(ip->tcp_flags_mask));
		if (r->rule_flags & PPE_ACL_RULE_FLAG_TCP_FLG_MASK) {
			ip->tcp_flags_mask = r->rule.tcp_flag.tcp_flags_mask;
		}

		flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
		if (slice->sub_rule_cnt && (ip->inverse_en != flag_en)) {
			ppe_acl_warn("%p: invalid rule flags, expect all ip misc rules to have "
				       "same inverse logic type:%d flags: 0x%x",
				       r, type, r->rule_flags);
			return false;
		}

		slice->sub_rule_cnt++;
		slice->flags |= PPE_DRV_ACL_IP_MISC_FLAG_TCPFLAG;
		slice->type = PPE_DRV_ACL_SLICE_TYPE_IP_MISC;
		slice->valid = true;
		break;

	case PPE_ACL_RULE_MATCH_TYPE_SIP:
		if (r->rule.sip.ip_type == PPE_ACL_IP_TYPE_V4) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4];
			sip_v4 = &slice->rule.sip_v4;

			sip_v4->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			sip_v4->ip = ntohl(r->rule.sip.ip[0]);
			memset(&sip_v4->ip_mask, 0xff, sizeof(sip_v4->ip_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_SIP_MASK) {
				sip_v4->ip_mask = ntohl(r->rule.sip.ip_mask[0]);
			}

			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_SRC_IPV4_FLAG_IP;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4;
			slice->valid = true;
		} else if (r->rule.sip.ip_type == PPE_ACL_IP_TYPE_V6) {
			/*
			 * IPv6 slice 1
			 */
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0];
			sip_v6_0 = &slice->rule.sip_v6_0;
			sip[0] = ntohl(r->rule.sip.ip[0]);
			sip[1] = ntohl(r->rule.sip.ip[1]);
			sip[2] = ntohl(r->rule.sip.ip[2]);
			sip[3] = ntohl(r->rule.sip.ip[3]);
			sip_mask[0] = ntohl(r->rule.sip.ip_mask[0]);
			sip_mask[1] = ntohl(r->rule.sip.ip_mask[1]);
			sip_mask[2] = ntohl(r->rule.sip.ip_mask[2]);
			sip_mask[3] = ntohl(r->rule.sip.ip_mask[3]);

			sip_v6_0->ip_l32= sip[0];
			sip_v6_0->ip_u16= sip[1] & 0xffff;
			memset(&sip_v6_0->ip_l32_mask, 0xff, sizeof(sip_v6_0->ip_l32_mask));
			memset(&sip_v6_0->ip_u16_mask, 0xff, sizeof(sip_v6_0->ip_u16_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_SIP_MASK) {
				sip_v6_0->ip_l32_mask = sip_mask[0];
				sip_v6_0->ip_u16_mask = sip_mask[1] & 0xffff;
			}

			sip_v6_0->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0;
			slice->valid = true;

			/*
			 * IPv6 slice 1
			 */
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1];
			sip_v6_1 = &slice->rule.sip_v6_1;

			sip_v6_1->ip_l32= (sip[1] >> 16) | (sip[2] << 16);
			sip_v6_1->ip_u16 = sip[2] >> 16;
			memset(&sip_v6_1->ip_l32_mask, 0xff, sizeof(sip_v6_1->ip_l32_mask));
			memset(&sip_v6_1->ip_u16_mask, 0xff, sizeof(sip_v6_1->ip_u16_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_SIP_MASK) {
				sip_v6_1->ip_l32_mask = (sip_mask[1] >> 16) | (sip_mask[2] << 16);
				sip_v6_1->ip_u16_mask = sip_mask[2] >> 16;
			}

			sip_v6_1->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1;
			slice->valid = true;

			/*
			 * IPv6 slice 2
			 */
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2];
			sip_v6_2 = &slice->rule.sip_v6_2;

			sip_v6_2->ip_l32= sip[3];
			memset(&sip_v6_2->ip_l32_mask, 0xff, sizeof(sip_v6_2->ip_l32_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_SIP_MASK) {
				sip_v6_2->ip_l32_mask = sip_mask[3];
			}

			sip_v6_2->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_SRC_IPV6_FLAG_IP;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2;
			slice->valid = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_DIP:
		if (r->rule.dip.ip_type == PPE_ACL_IP_TYPE_V4) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4];
			dip_v4 = &slice->rule.dip_v4;

			dip_v4->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			dip_v4->ip = ntohl(r->rule.dip.ip[0]);
			memset(&dip_v4->ip_mask, 0xff, sizeof(dip_v4->ip_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_DIP_MASK) {
				dip_v4->ip_mask = ntohl(r->rule.dip.ip_mask[0]);
			}

			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_DST_IPV4_FLAG_IP;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_DST_IPV4;
			slice->valid = true;
		} else if (r->rule.dip.ip_type == PPE_ACL_IP_TYPE_V6) {
			/*
			 * IPv6 slice 0
			 */
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0];
			dip_v6_0 = &slice->rule.dip_v6_0;
			dip[0] = ntohl(r->rule.dip.ip[0]);
			dip[1] = ntohl(r->rule.dip.ip[1]);
			dip[2] = ntohl(r->rule.dip.ip[2]);
			dip[3] = ntohl(r->rule.dip.ip[3]);
			dip_mask[0] = ntohl(r->rule.dip.ip_mask[0]);
			dip_mask[1] = ntohl(r->rule.dip.ip_mask[1]);
			dip_mask[2] = ntohl(r->rule.dip.ip_mask[2]);
			dip_mask[3] = ntohl(r->rule.dip.ip_mask[3]);

			dip_v6_0->ip_l32 = dip[0];
			dip_v6_0->ip_u16 = dip[1] & 0xffff;
			memset(&dip_v6_0->ip_l32_mask, 0xff, sizeof(dip_v6_0->ip_l32_mask));
			memset(&dip_v6_0->ip_u16_mask, 0xff, sizeof(dip_v6_0->ip_u16_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_DIP_MASK) {
				dip_v6_0->ip_l32_mask = dip_mask[0];
				dip_v6_0->ip_u16_mask = dip_mask[1] & 0xffff;
			}

			dip_v6_0->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0;
			slice->valid = true;

			/*
			 * IPv6 slice 1
			 */
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1];
			dip_v6_1 = &slice->rule.dip_v6_1;

			dip_v6_1->ip_l32= (dip[1] >> 16) | (dip[2] << 16);
			dip_v6_1->ip_u16 = dip[2] >> 16;
			memset(&dip_v6_1->ip_l32_mask, 0xff, sizeof(dip_v6_1->ip_l32_mask));
			memset(&dip_v6_1->ip_u16_mask, 0xff, sizeof(dip_v6_1->ip_u16_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_DIP_MASK) {
				dip_v6_1->ip_l32_mask = (dip_mask[1] >> 16) | (dip_mask[2] << 16);
				dip_v6_1->ip_u16_mask = dip_mask[2] >> 16;
			}

			dip_v6_1->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1;
			slice->valid = true;

			/*
			 * IPv6 slice 2
			 */
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2];
			dip_v6_2 = &slice->rule.dip_v6_2;

			dip_v6_2->ip_l32= dip[3];
			memset(&dip_v6_2->ip_l32_mask, 0xff, sizeof(dip_v6_2->ip_l32_mask));
			if (r->rule_flags & PPE_ACL_RULE_FLAG_DIP_MASK) {
				dip_v6_2->ip_l32_mask = dip_mask[3];
			}

			dip_v6_2->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_DST_IPV6_FLAG_IP;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2;
			slice->valid = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_SPORT:
		/*
		 * If source IPv4 or IPv6 rule is configured, use it. Otherwise, for rule
		 * based on SPORT alone, we can use SRC_IPV4 rule with noip option.
		 */
		if (info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2].valid) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2];
			sip_v6_2 = &slice->rule.sip_v6_2;

			sip_v6_2->l4_port_min = ntohs(r->rule.sport.l4_port_min);
			memset(&sip_v6_2->l4_port_mask_max, 0xff, sizeof(sip_v6_2->l4_port_mask_max));
			if (r->rule_flags & (PPE_ACL_RULE_FLAG_SPORT_MASK | PPE_ACL_RULE_FLAG_SPORT_RANGE)) {
				sip_v6_2->l4_port_mask_max = ntohs(r->rule.sport.l4_port_max_mask);
				sip_v6_2->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_SPORT_RANGE);
			}

			flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			if (slice->sub_rule_cnt && (sip_v6_2->inverse_en != flag_en)) {
				ppe_acl_warn("%p: invalid rule flags, expect all ipv6 sub rules to have "
						"same inverse logic type:%d flags: 0x%x",
						r, type, r->rule_flags);
				return false;
			}

			sip_v6_2->inverse_en = flag_en;
			slice->flags |= PPE_DRV_ACL_SRC_IPV6_FLAG_L4PORT;
			slice->sub_rule_cnt++;
		} else {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4];
			sip_v4 = &slice->rule.sip_v4;

			sip_v4->l4_port_min = ntohs(r->rule.sport.l4_port_min);
			memset(&sip_v4->l4_port_mask_max, 0xff, sizeof(sip_v4->l4_port_mask_max));
			if (r->rule_flags & (PPE_ACL_RULE_FLAG_SPORT_MASK | PPE_ACL_RULE_FLAG_SPORT_RANGE)) {
				sip_v4->l4_port_mask_max = ntohs(r->rule.sport.l4_port_max_mask);
				sip_v4->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_SPORT_RANGE);
			}

			flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			if (slice->sub_rule_cnt && (sip_v4->inverse_en != flag_en)) {
				ppe_acl_warn("%p: invalid rule flags, expect all ipv6 sub rules to have "
						"same inverse logic type:%d flags: 0x%x",
						r, type, r->rule_flags);
				return false;
			}

			sip_v4->inverse_en = flag_en;
			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_SRC_IPV4_FLAG_L4PORT;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4;
			slice->valid = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_DPORT:
		/*
		 * If destination IPv4 or IPv6 rule is configured, use it. Otherwise, for rule
		 * based on DPORT alone, we can use DST_IPV4 rule with noip option.
		 */
		if (info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2].valid) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2];
			dip_v6_2 = &slice->rule.dip_v6_2;

			dip_v6_2->l4_port_min = ntohs(r->rule.dport.l4_port_min);
			memset(&dip_v6_2->l4_port_mask_max, 0xff, sizeof(dip_v6_2->l4_port_mask_max));
			if (r->rule_flags & (PPE_ACL_RULE_FLAG_DPORT_MASK | PPE_ACL_RULE_FLAG_DPORT_RANGE)) {
				dip_v6_2->l4_port_mask_max = ntohs(r->rule.dport.l4_port_max_mask);
				dip_v6_2->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_DPORT_RANGE);
			}

			flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			if (slice->sub_rule_cnt && (dip_v6_2->inverse_en != flag_en)) {
				ppe_acl_warn("%p: invalid rule flags, expect all ipv6 sub rules to have "
						"same inverse logic type:%d flags: 0x%x",
						r, type, r->rule_flags);
				return false;
			}

			dip_v6_2->inverse_en = flag_en;
			slice->flags |= PPE_DRV_ACL_DST_IPV6_FLAG_L4PORT;
			slice->sub_rule_cnt++;
		} else {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4];
			dip_v4 = &slice->rule.dip_v4;

			dip_v4->l4_port_min = ntohs(r->rule.dport.l4_port_min);
			memset(&dip_v4->l4_port_mask_max, 0xff, sizeof(dip_v4->l4_port_mask_max));
			if (r->rule_flags & (PPE_ACL_RULE_FLAG_DPORT_MASK | PPE_ACL_RULE_FLAG_DPORT_RANGE)) {
				dip_v4->l4_port_mask_max = ntohs(r->rule.dport.l4_port_max_mask);
				dip_v4->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_DPORT_RANGE);
			}

			flag_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
			if (slice->sub_rule_cnt && (dip_v4->inverse_en != flag_en)) {
				ppe_acl_warn("%p: invalid rule flags, expect all ipv6 sub rules to have "
						"same inverse logic type:%d flags: 0x%x",
						r, type, r->rule_flags);
				return false;
			}

			dip_v4->inverse_en = flag_en;

			slice->sub_rule_cnt++;
			slice->flags |= PPE_DRV_ACL_DST_IPV4_FLAG_L4PORT;
			slice->type = PPE_DRV_ACL_SLICE_TYPE_DST_IPV4;
			slice->valid = true;
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_UDF:
		if (r->rule.udf.udf_a_valid || r->rule.udf.udf_b_valid || r->rule.udf.udf_c_valid) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_UDF_012];
			udf_012 = &slice->rule.udf_012;

			if (r->rule.udf.udf_a_valid) {
				udf_012->udf_a_min = r->rule.udf.udf_a_min;
				memset(&udf_012->udf_a_mask_max, 0xff, sizeof(udf_012->udf_a_mask_max));
				if (r->rule_flags & (PPE_ACL_RULE_FLAG_UDFA_MASK | PPE_ACL_RULE_FLAG_UDFA_RANGE)) {
					udf_012->udf_a_mask_max = r->rule.udf.udf_a_mask_max;
					udf_012->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_UDFA_RANGE);
				}

				udf_012->udf_a_valid = true;
				slice->valid = true;
			}

			if (!(r->rule.udf.udf_d_valid || (r->rule_flags & PPE_ACL_RULE_FLAG_UDFB_RANGE))) {
				if (r->rule.udf.udf_b_valid) {
					udf_012->udf_b = r->rule.udf.udf_b_min;
					memset(&udf_012->udf_b_mask, 0xff, sizeof(udf_012->udf_b_mask));
					if (r->rule_flags & PPE_ACL_RULE_FLAG_UDFB_MASK) {
						udf_012->udf_b_mask = r->rule.udf.udf_b_mask_max;
					}

					udf_012->udf_b_valid = true;
					slice->valid = true;
				}

				if (r->rule.udf.udf_c_valid) {
					udf_012->udf_c = r->rule.udf.udf_c;
					memset(&udf_012->udf_c_mask, 0xff, sizeof(udf_012->udf_c_mask));
					if (r->rule_flags & PPE_ACL_RULE_FLAG_UDFC_MASK) {
						udf_012->udf_c_mask = r->rule.udf.udf_c_mask;
					}

					udf_012->udf_c_valid = true;
					slice->valid = true;
				}
			}

			if (slice->valid) {
				udf_012->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
				slice->type = PPE_DRV_ACL_SLICE_TYPE_UDF_012;
			}
		}

		if (r->rule.udf.udf_d_valid || (r->rule_flags & PPE_ACL_RULE_FLAG_UDFB_RANGE)) {
			slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_UDF_123];
			udf_123 = &slice->rule.udf_123;

			if (r->rule.udf.udf_d_valid) {
				udf_123->udf_c = r->rule.udf.udf_d;
				memset(&udf_123->udf_c_mask, 0xff, sizeof(udf_123->udf_c_mask));
				if (r->rule_flags & PPE_ACL_RULE_FLAG_UDFD_MASK) {
					udf_123->udf_c_mask = r->rule.udf.udf_d_mask;
				}

				udf_123->udf_c_valid = true;
				slice->valid = true;
			}

			if (r->rule.udf.udf_b_valid) {
				udf_123->udf_a_min = r->rule.udf.udf_b_min;
				memset(&udf_123->udf_a_mask_max, 0xff, sizeof(udf_123->udf_a_mask_max));
				if (r->rule_flags & (PPE_ACL_RULE_FLAG_UDFB_MASK | PPE_ACL_RULE_FLAG_UDFB_RANGE)) {
					udf_123->udf_a_mask_max = r->rule.udf.udf_b_mask_max;
					udf_123->range_en = !!(r->rule_flags & PPE_ACL_RULE_FLAG_UDFB_RANGE);
				}

				udf_123->udf_a_valid = true;
				slice->valid = true;
			}

			if (r->rule.udf.udf_c_valid) {
				udf_123->udf_b = r->rule.udf.udf_c;
				memset(&udf_123->udf_b_mask, 0xff, sizeof(udf_123->udf_b_mask));
				if (r->rule_flags & PPE_ACL_RULE_FLAG_UDFC_MASK) {
					udf_123->udf_b_mask = r->rule.udf.udf_c_mask;
				}

				udf_123->udf_b_valid = true;
				slice->valid = true;
			}

			if (slice->valid) {
				udf_123->inverse_en = !!(r->rule_flags & PPE_ACL_RULE_GEN_FLAG_INVERSE_EN);
				slice->type = PPE_DRV_ACL_SLICE_TYPE_UDF_123;
			}
		}

		break;

	case PPE_ACL_RULE_MATCH_TYPE_DEFAULT:
		slice = &info->chain[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC];
		memset(slice->rule.smac.mac, 0x0, ETH_ALEN);
		memset(slice->rule.smac.mac_mask, 0x0, ETH_ALEN);
		slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_MAC;
		slice->valid = true;
		break;

	default:
		ppe_acl_warn("%p: invalid rule type: %d", r, type);
		break;
	}

	return true;
}

/*
 * ppe_acl_action_fill()
 *	Action corresponding to an ACL rule.
 */
static bool ppe_acl_action_fill(struct ppe_acl *acl, struct ppe_acl_rule_action *r_action)
{
	struct ppe_drv_acl_rule *info = &acl->info;
	struct ppe_drv_acl_action *acl_action = &info->action;
	struct ppe_drv_iface *iface;
	struct net_device *dev;
	int32_t port_num;

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_SERVICE_CODE_EN) {
		acl_action->service_code = r_action->service_code;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_SC;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_QID_EN) {
		acl_action->qid = r_action->qid;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_QID;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_ENQUEUE_PRI_CHANGE_EN) {
		acl_action->enqueue_pri = r_action->enqueue_pri;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_ENQUEUE_PRI;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CTAG_DEI_CHANGE_EN) {
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_CTAG_DEI;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CTAG_PCP_CHANGE_EN) {
		acl_action->ctag_pcp = r_action->ctag_pcp;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_CTAG_PCP;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_STAG_DEI_CHANGE_EN) {
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_STAG_DEI;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_STAG_PCP_CHANGE_EN) {
		acl_action->stag_pcp = r_action->stag_pcp;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_STAG_PCP;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_DSCP_TC_CHANGE_EN) {
		acl_action->dscp_tc = r_action->dscp_tc;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_DSCP_TC;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CVID_CHANGE_EN) {
		acl_action->cvid = r_action->cvid;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_CVID;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_SVID_CHANGE_EN) {
		acl_action->svid = r_action->svid;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_SVID;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_DEST_INFO_CHANGE_EN) {
		dev = dev_get_by_name(&init_net, r_action->dst.dev_name);
		if (!dev) {
			ppe_acl_warn("%p: failed to find valid src for dev %s\n",
					r_action, r_action->dst.dev_name);
			return false;
		}

		iface = ppe_drv_iface_get_by_dev(dev);
		if (!iface) {
			ppe_acl_warn("%p: failed to find PPE interface for dev: %p(%s)\n",
					r_action, dev, r_action->dst.dev_name);
			dev_put(dev);
			return false;
		}

		port_num = ppe_drv_iface_port_idx_get(iface);
		if (port_num < 0) {
			ppe_acl_warn("%p: failed to find PPE port for iface: %p\n",
					r_action, iface);
			dev_put(dev);
			return false;
		}

		ppe_acl_info("%p: Destination port number: %d\n", r_action, port_num);

		dev_put(dev);
		acl_action->dest_info = port_num;
		acl_action->dest_type = PPE_DRV_ACL_DST_TYPE_PORT_ID;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_DST_INFO;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_MIRROR_EN) {
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_MIRROR_EN;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_CTAG_FMT_TAGGED) {
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_CTAG_FMT_TAGGED;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_STAG_FMT_TAGGED) {
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_STAG_FMT_TAGGED;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_REDIR_TO_CORE_EN) {
		if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_SERVICE_CODE_EN) {
			ppe_acl_warn("%p: both sc and rdt can't be enabled together: %d\n",
					r_action, r_action->service_code);
			return false;
		}

		switch (r_action->redir_core) {
		case 0:
			acl_action->service_code = (r_action->flags
					& PPE_ACL_RULE_ACTION_FLAG_REDIR_EDIT_EN)
					? PPE_DRV_SC_EDIT_REDIR_CORE0
					: PPE_DRV_SC_NOEDIT_REDIR_CORE0;
			break;
		case 1:
			acl_action->service_code = (r_action->flags
					& PPE_ACL_RULE_ACTION_FLAG_REDIR_EDIT_EN)
					? PPE_DRV_SC_EDIT_REDIR_CORE1
					: PPE_DRV_SC_NOEDIT_REDIR_CORE1;
			break;
		case 2:
			acl_action->service_code = (r_action->flags
					& PPE_ACL_RULE_ACTION_FLAG_REDIR_EDIT_EN)
					? PPE_DRV_SC_EDIT_REDIR_CORE2
					: PPE_DRV_SC_NOEDIT_REDIR_CORE2;
			break;
		case 3:
			acl_action->service_code = (r_action->flags
					& PPE_ACL_RULE_ACTION_FLAG_REDIR_EDIT_EN)
					? PPE_DRV_SC_EDIT_REDIR_CORE3
					: PPE_DRV_SC_NOEDIT_REDIR_CORE3;
			break;
		default:
			ppe_acl_warn("%p: invalid redirect core: %d", r_action, r_action->redir_core);
			return false;
		}

		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_SC;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_POLICER_EN) {
		acl_action->policer_index = ppe_policer_id_to_hwidx(r_action->policer_id);
		if (acl_action->policer_index < 0) {
			ppe_acl_warn("%p: no valid hw policer index for id: %d",
				r_action, r_action->policer_id);
			return false;
		}

		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_POLICER_INDEX;
	}

	if (r_action->flags & PPE_ACL_RULE_ACTION_FLAG_FW_CMD) {
		acl_action->fwd_cmd = (ppe_drv_acl_fwd_cmd_t)r_action->fwd_cmd;
		acl_action->flags |= PPE_DRV_ACL_ACTION_FLAG_FWD_CMD;
	}

	return true;
}

/*
 * ppe_acl_rule_src_fill()
 *	Extract source information
 */
static bool ppe_acl_rule_src_fill(struct ppe_acl *acl, struct ppe_acl_rule *rule)
{
	struct ppe_drv_acl_rule *info = &acl->info;
	struct net_device *dev;
	struct ppe_drv_iface *iface;
	int32_t port_num;
	uint8_t sc;

	switch (rule->stype) {
	case PPE_ACL_RULE_SRC_TYPE_DEV:
		dev = dev_get_by_name(&init_net, rule->src.dev_name);
		if (!dev) {
			ppe_acl_warn("%p: failed to find valid src for dev %s\n",
					rule, rule->src.dev_name);
			return false;
		}

		iface = ppe_drv_iface_get_by_dev(dev);
		if (!iface) {
			ppe_acl_warn("%p: failed to find PPE interface for dev: %p(%s)\n",
					rule, dev, rule->src.dev_name);
			dev_put(dev);
			return false;
		}

		port_num = ppe_drv_iface_port_idx_get(iface);
		if (port_num < 0) {
			ppe_acl_warn("%p: failed to find PPE port for iface: %p\n",
					rule, iface);
			dev_put(dev);
			return false;
		}

		info->stype = PPE_DRV_ACL_SRC_TYPE_PORT_NUM;
		info->src = port_num;
		dev_put(dev);
		ppe_acl_info("%p: Binding ACL rule to port number: %d\n", rule, info->src);
		break;

	case PPE_ACL_RULE_SRC_TYPE_SC:
		info->stype = PPE_DRV_ACL_SRC_TYPE_SC;
		info->src = rule->src.sc;
		acl->sc = info->src;

		ppe_acl_info("%p: Binding ACL rule to servcie code number: %d\n", rule, info->src);
		break;

	case PPE_ACL_RULE_SRC_TYPE_FLOW:

		/*
		 * Get a free service code for this ACL rule.
		 */
		sc = ppe_drv_acl_sc_get();
		if (!sc) {
			ppe_acl_warn("%p: not able to find a free service code", rule);
			return false;
		}

		info->stype = PPE_DRV_ACL_SRC_TYPE_SC;
		info->src = sc;
		acl->sc = info->src;

		ppe_acl_info("%p: FLOW + ACL rule servcie code number: %d\n", rule, info->src);
		break;
	}

	return true;
}

/*
 * ppe_acl_rule_cmn_fill()
 *	Set common fields in ACL rule
 */
static bool ppe_acl_rule_cmn_fill(struct ppe_acl *acl, struct ppe_acl_rule *rule)
{
	struct ppe_drv_acl_rule *info = &acl->info;

	/*
	 * Outer header match and post routing enable not supported simultaneously.
	 */
	if ((rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_POST_RT_EN)
		&& (rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_OUTER_HDR_MATCH)) {
		ppe_acl_warn("%p: cmn_flags: 0x%x post routing and outer hdr not supported!",
				acl, rule->cmn.cmn_flags);
		return false;
	}

	info->cmn.post_routing_en = !!(rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_POST_RT_EN);
	acl->pri = !!(rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_PRI_EN)
			? rule->cmn.pri : PPE_ACL_PRI_NOMINAL;
	acl->ipo = !!(rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_OUTER_HDR_MATCH)
			? PPE_DRV_ACL_PREIPO : PPE_DRV_ACL_IPO;
	info->cmn.qos_res_pre = !!(acl->ipo == PPE_DRV_ACL_PREIPO)
			? PPE_DRV_PORT_QOS_RES_PREC_4 : PPE_DRV_PORT_QOS_RES_PREC_5;
	info->cmn.qos_res_pre = !!(rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_FLOW_QOS_OVERRIDE)
			? PPE_DRV_PORT_QOS_RES_PREC_7 : info->cmn.qos_res_pre;
	acl->qos_res_pre = info->cmn.qos_res_pre;

	if (rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_GROUP_EN) {
		if (rule->cmn.group >= PPE_ACL_GROUP_MAX) {
			ppe_acl_warn("%p: Invalid group number: %d", acl, rule->cmn.group);
			return false;
		}

		info->cmn.res_chain = rule->cmn.group;
	}

	if (rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_METADATA_EN) {
		info->action.flags |= PPE_DRV_ACL_ACTION_FLAG_METADATA_EN;
	}

	ppe_acl_info("%p: cmn_flags: 0x%x setting post_routing: %d, pri: %d group: %d\n",
			acl, rule->cmn.cmn_flags, info->cmn.post_routing_en, acl->pri,
			info->cmn.res_chain);
	return true;
}

/*
 * ppe_acl_rule_exist()
 *	Check if ACL rule already exist.
 */
static bool ppe_acl_rule_exist(struct ppe_acl *acl)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_drv_acl_mac *dmac, *ae_dmac;
	struct ppe_drv_acl_mac *smac, *ae_smac;
	struct ppe_drv_acl_vlan *vlan, *ae_vlan;
	struct ppe_drv_acl_l2_misc *l2, *ae_l2;
	struct ppe_drv_acl_ipv4 *sip_v4, *ae_sip_v4;
	struct ppe_drv_acl_ipv4 *dip_v4, *ae_dip_v4;
	struct ppe_drv_acl_ipv6 *dip_v6_0, *ae_dip_v6_0;
	struct ppe_drv_acl_ipv6 *dip_v6_1, *ae_dip_v6_1;
	struct ppe_drv_acl_ipv6 *dip_v6_2, *ae_dip_v6_2;
	struct ppe_drv_acl_ipv6 *sip_v6_0, *ae_sip_v6_0;
	struct ppe_drv_acl_ipv6 *sip_v6_1, *ae_sip_v6_1;
	struct ppe_drv_acl_ipv6 *sip_v6_2, *ae_sip_v6_2;
	struct ppe_drv_acl_ip_misc *ip, *ae_ip;
	struct ppe_drv_acl_rule_match_one *slice;
	enum ppe_drv_acl_slice_type slice_type;
	struct ppe_acl *ae;

	if (list_empty(&acl_g->active_rules)) {
		return false;
	}

	list_for_each_entry(ae, &acl_g->active_rules, list) {

		/*
		 * Don't allow new rule id with an existing rule having same rule id
		 */
		if (ae->rule_id == acl->rule_id) {
			ppe_acl_info("%p: found a matching rule ID: %d", ae, ae->rule_id);
			return true;
		}

		/*
		 * If any of the rule flags mismatch this is a new rule.
		 * - This also means a subset of rules are treated as new rule with different priority.
		 * - for each slice we also compare mask, which allow LPM matches for the same rule.
		 * - We also check for inverse so that (type == value) and (type != value) both can be set.
		 */
		if (ae->rule_valid_flag != acl->rule_valid_flag) {
			/*
			 * Go to the next rule.
			 */
			continue;
		}

		/*
		 * If source is different this is a new rule.
		 */
		if ((ae->info.stype != acl->info.stype) || ae->info.src != acl->info.src) {
			/*
			 * Go to next rule.
			 */
			continue;
		}

		/*
		 * This rule has all the flags matching with new rule,
		 * let's compare each valid slice along with mask.
		 */
		for (slice_type = PPE_DRV_ACL_SLICE_TYPE_DST_MAC;
			slice_type < PPE_DRV_ACL_SLICE_TYPE_MAX; slice_type++) {

			if (!ae->info.chain[slice_type].valid) {
				continue;
			}

			switch (slice_type) {
			case PPE_DRV_ACL_SLICE_TYPE_DST_MAC:
				ae_dmac = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_MAC].rule.dmac;
				dmac = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_MAC].rule.dmac;
				if (!(memcmp(ae_dmac->mac, dmac->mac, ETH_ALEN)
					&& memcmp(ae_dmac->mac_mask, dmac->mac_mask, ETH_ALEN)
					&& (ae_dmac->inverse_en != dmac->inverse_en))) {

					goto next_entry;
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_SRC_MAC:
				ae_smac = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC].rule.smac;
				smac = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC].rule.smac;
				if (!(memcmp(ae_smac->mac, smac->mac, ETH_ALEN)
					&& memcmp(ae_smac->mac_mask, smac->mac_mask, ETH_ALEN)
					&& (ae_smac->inverse_en != smac->inverse_en))) {

					goto next_entry;
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_VLAN:
				ae_vlan = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_VLAN].rule.vlan;
				vlan = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_VLAN].rule.vlan;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_VLAN];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_VLAN].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_VLAN].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_VSI_VALID) {
					if (ae_vlan->vsi_valid != vlan->vsi_valid) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_VSI) {
					if (!((ae_vlan->vsi == vlan->vsi)
						&& (ae_vlan->vsi_mask == vlan->vsi_mask))) {
						goto next_entry;
					}
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
					if (!((ae_vlan->svid == vlan->svid)
						&& (ae_vlan->svid_mask == vlan->svid_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CVID) {
					if (!((ae_vlan->cvid_min == vlan->cvid_min)
						&& (ae_vlan->cvid_mask_max == vlan->cvid_mask_max))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_SPCP) {
					if (!((ae_vlan->spcp == vlan->spcp)
						&& (ae_vlan->spcp_mask == vlan->spcp_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CPCP) {
					if (!((ae_vlan->cpcp == vlan->cpcp)
						&& (ae_vlan->cpcp_mask == vlan->cpcp_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_SDEI) {
					if (ae_vlan->sdei_en != vlan->sdei_en) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_VLAN_FLAG_CDEI) {
					if (ae_vlan->cdei_en != vlan->cdei_en) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_L2_MISC:
				ae_l2 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC].rule.l2;
				l2 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC].rule.l2;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_L2_MISC].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_L2_MISC_FLAG_PPPOE) {
					if (!((ae_l2->pppoe_session_id == l2->pppoe_session_id)
						&& (ae_l2->pppoe_session_id_mask == l2->pppoe_session_id_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_L2_MISC_FLAG_L2PROTO) {
					if (!((ae_l2->l2_proto == l2->l2_proto)
						&& (ae_l2->l2_proto_mask == l2->l2_proto_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_L2_FLAG_SVID) {
					if (!((ae_l2->svid_min == l2->svid_min)
						&& (ae_l2->svid_mask_max == l2->svid_mask_max))) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_DST_IPV4:
				ae_dip_v4 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4].rule.dip_v4;
				dip_v4 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4].rule.dip_v4;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV4].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_PKTTYPE) {
					if (!((ae_dip_v4->pkt_type == dip_v4->pkt_type)
						&& (ae_dip_v4->pkt_type_mask == dip_v4->pkt_type_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_L3FRAG) {
					if (ae_dip_v4->l3_fragment_flag != dip_v4->l3_fragment_flag) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_L4PORT) {
					if (!((ae_dip_v4->l4_port_min == dip_v4->l4_port_min)
						&& (ae_dip_v4->l4_port_mask_max == dip_v4->l4_port_mask_max))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV4_FLAG_IP) {
					if (!((ae_dip_v4->ip == dip_v4->ip)
						&& (ae_dip_v4->ip_mask == dip_v4->ip_mask))) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4:
				ae_sip_v4 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4].rule.sip_v4;
				sip_v4 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4].rule.sip_v4;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV4].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_PKTTYPE) {
					if (!((ae_sip_v4->pkt_type == sip_v4->pkt_type)
						&& (ae_sip_v4->pkt_type_mask == sip_v4->pkt_type_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_L3FRAG) {
					if (ae_sip_v4->l3_fragment_flag != sip_v4->l3_fragment_flag) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_L4PORT) {
					if (!((ae_sip_v4->l4_port_min == sip_v4->l4_port_min)
						&& (ae_sip_v4->l4_port_mask_max == sip_v4->l4_port_mask_max))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV4_FLAG_IP) {
					if (!((ae_sip_v4->ip == sip_v4->ip)
						&& (ae_sip_v4->ip_mask == sip_v4->ip_mask))) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0:
				ae_dip_v6_0 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0].rule.dip_v6_0;
				dip_v6_0 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_0].rule.dip_v6_0;

				if (!((ae_dip_v6_0->ip_l32 == dip_v6_0->ip_l32)
					&& (ae_dip_v6_0->ip_u16 == dip_v6_0->ip_u16)
					&& (ae_dip_v6_0->ip_l32_mask == dip_v6_0->ip_l32_mask)
					&& (ae_dip_v6_0->ip_u16_mask == dip_v6_0->ip_u16_mask))) {
					goto next_entry;
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1:
				ae_dip_v6_1 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1].rule.dip_v6_1;
				dip_v6_1 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_1].rule.dip_v6_1;

				if (!((ae_dip_v6_1->ip_l32 == dip_v6_1->ip_l32)
					&& (ae_dip_v6_1->ip_u16 == dip_v6_1->ip_u16)
					&& (ae_dip_v6_1->ip_l32_mask == dip_v6_1->ip_l32_mask)
					&& (ae_dip_v6_1->ip_u16_mask == dip_v6_1->ip_u16_mask))) {
					goto next_entry;
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2:
				ae_dip_v6_2 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2].rule.dip_v6_2;
				dip_v6_2 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2].rule.dip_v6_2;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_DST_IPV6_2].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_PKTTYPE) {
					if (!((ae_dip_v6_2->pkt_type == dip_v6_2->pkt_type)
						&& (ae_dip_v6_2->pkt_type_mask == dip_v6_2->pkt_type_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_L3FRAG) {
					if (ae_dip_v6_2->l3_fragment_flag != dip_v6_2->l3_fragment_flag) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_L4PORT) {
					if (!((ae_dip_v6_2->l4_port_min == dip_v6_2->l4_port_min)
						&& (ae_dip_v6_2->l4_port_mask_max == dip_v6_2->l4_port_mask_max))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_DST_IPV6_FLAG_IP) {
					if (!((ae_dip_v6_2->ip_l32 == dip_v6_2->ip_l32)
						&& (ae_dip_v6_2->ip_u16 == dip_v6_2->ip_u16)
						&& (ae_dip_v6_2->ip_l32_mask == dip_v6_2->ip_l32_mask)
						&& (ae_dip_v6_2->ip_u16_mask == dip_v6_2->ip_u16_mask))) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0:
				ae_sip_v6_0 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0].rule.sip_v6_0;
				sip_v6_0 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_0].rule.sip_v6_0;

				if (!((ae_sip_v6_0->ip_l32 == sip_v6_0->ip_l32)
					&& (ae_sip_v6_0->ip_u16 == sip_v6_0->ip_u16)
					&& (ae_sip_v6_0->ip_l32_mask == sip_v6_0->ip_l32_mask)
					&& (ae_sip_v6_0->ip_u16_mask == sip_v6_0->ip_u16_mask))) {
					goto next_entry;
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1:
				ae_sip_v6_1 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1].rule.sip_v6_1;
				sip_v6_1 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_1].rule.sip_v6_1;

				if (!((ae_sip_v6_1->ip_l32 == sip_v6_1->ip_l32)
					&& (ae_sip_v6_1->ip_u16 == sip_v6_1->ip_u16)
					&& (ae_sip_v6_1->ip_l32_mask == sip_v6_1->ip_l32_mask)
					&& (ae_sip_v6_1->ip_u16_mask == sip_v6_1->ip_u16_mask))) {
					goto next_entry;
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2:
				ae_sip_v6_2 = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2].rule.sip_v6_2;
				sip_v6_2 = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2].rule.sip_v6_2;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_IPV6_2].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_PKTTYPE) {
					if (!((ae_sip_v6_2->pkt_type == sip_v6_2->pkt_type)
						&& (ae_sip_v6_2->pkt_type_mask == sip_v6_2->pkt_type_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_L3FRAG) {
					if (ae_sip_v6_2->l3_fragment_flag != sip_v6_2->l3_fragment_flag) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_L4PORT) {
					if (!((ae_sip_v6_2->l4_port_min == sip_v6_2->l4_port_min)
						&& (ae_sip_v6_2->l4_port_mask_max == sip_v6_2->l4_port_mask_max))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_SRC_IPV6_FLAG_IP) {
					if (!((ae_sip_v6_2->ip_l32 == sip_v6_2->ip_l32)
						&& (ae_sip_v6_2->ip_u16 == sip_v6_2->ip_u16)
						&& (ae_sip_v6_2->ip_l32_mask == sip_v6_2->ip_l32_mask)
						&& (ae_sip_v6_2->ip_u16_mask == sip_v6_2->ip_u16_mask))) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_IP_MISC:
				ae_ip = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC].rule.ip;
				ip = &acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC].rule.ip;
				slice = &ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC];

				if (ae->info.chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC].flags !=
					acl->info.chain[PPE_DRV_ACL_SLICE_TYPE_IP_MISC].flags) {
					goto next_entry;
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L3FRAG) {
					if (!((ae_ip->l3_fragment_flag == ip->l3_fragment_flag)
						&& (ae_ip->l3_fragment_flag_mask == ip->l3_fragment_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_OTHEXT_HDR) {
					if (!((ae_ip->other_extension_hdr_flag == ip->other_extension_hdr_flag)
						&& (ae_ip->other_extension_hdr_flag_mask == ip->other_extension_hdr_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_FRAG_HDR) {
					if (!((ae_ip->frag_hdr_flag == ip->frag_hdr_flag)
						&& (ae_ip->frag_hdr_flag_mask == ip->frag_hdr_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_MOB_HDR) {
					if (!((ae_ip->mobility_hdr_flag == ip->mobility_hdr_flag)
						&& (ae_ip->mobility_hdr_flag_mask == ip->mobility_hdr_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_ESP_HDR) {
					if (!((ae_ip->esp_hdr_flag == ip->esp_hdr_flag)
						&& (ae_ip->esp_hdr_flag_mask == ip->esp_hdr_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_AH_HDR) {
					if (!((ae_ip->ah_hdr_flag == ip->ah_hdr_flag)
						&& (ae_ip->ah_hdr_flag_mask == ip->ah_hdr_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_TTLHOP) {
					if (!((ae_ip->hop_limit == ip->hop_limit)
						&& (ae_ip->hop_limit_mask == ip->hop_limit_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L3STATE) {
					/*
					 * TODO check this.
					 */
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_TCPFLAG) {
					if (!((ae_ip->tcp_flags == ip->tcp_flags)
						&& (ae_ip->tcp_flags_mask == ip->tcp_flags_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L31STFRAG) {
					if (!((ae_ip->l3_1st_fragment_flag == ip->l3_1st_fragment_flag)
						&& (ae_ip->l3_1st_fragment_flag_mask == ip->l3_1st_fragment_flag_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_DSCPTC) {
					if (!((ae_ip->l3_dscp_tc == ip->l3_dscp_tc)
						&& (ae_ip->l3_dscp_tc_mask == ip->l3_dscp_tc_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L4PROTO) {
					if (!((ae_ip->l3_v4proto_v6nexthdr == ip->l3_v4proto_v6nexthdr)
						&& (ae_ip->l3_v4proto_v6nexthdr_mask == ip->l3_v4proto_v6nexthdr_mask))) {
						goto next_entry;
					}
				}

				if (slice->flags & PPE_DRV_ACL_IP_MISC_FLAG_L3LEN) {
					if (!((ae_ip->l3_length == ip->l3_length)
						&& (ae_ip->l3_length_mask_max == ip->l3_length_mask_max))) {
						goto next_entry;
					}
				}

				break;

			case PPE_DRV_ACL_SLICE_TYPE_UDF_012:
				/*
				 * TODO add this.
				 */
				break;

			case PPE_DRV_ACL_SLICE_TYPE_UDF_123:
				/*
				 * TODO add this.
				 */
				break;

			default:
				ppe_acl_warn("%p: invalid slice_type: %d", ae, slice_type);
				return false;
			}
		}

		/*
		 * One complete loop, without any problem - means found a matching entry.
		 */
		ppe_acl_info("%p: found a matching rule ID: %d", ae, ae->rule_id);
		return true;
next_entry:
		ppe_acl_info("%p: checking next ae", acl);
	}

	return false;
}

/*
 * ppe_acl_rule_flow_policer_create()
 *	Create ACL rule for flow policer in PPE.
 */
ppe_acl_ret_t ppe_acl_rule_flow_policer_create(struct ppe_acl_rule_flow_policer *rule)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_drv_acl_rule_match_one *slice;
	struct ppe_drv_acl_ctx *ctx = NULL;
	struct ppe_drv_acl_rule info = {0};
	ppe_acl_rule_id_t gen_id = -1;
	struct ppe_acl *acl = NULL;
	ppe_acl_ret_t ret;
	uint8_t sc = 0;

	ppe_acl_info("%p: rule create request: %p", acl_g, rule);

	spin_lock_bh(&acl_g->lock);

	acl = ppe_acl_alloc();
	if (!acl) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_oom);
		ppe_acl_warn("%p: failed to allocate acl memory: %p", acl_g, rule);
		ret = PPE_ACL_RET_CREATE_FAIL_OOM;
		goto fail;
	}

	gen_id = ppe_acl_rule_get_gen_id();
	if (gen_id < 0) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_rule_table_full);
		ppe_acl_warn("%p: cannot allocate a free rule ID", acl_g);
		ret = PPE_ACL_RET_CREATE_FAIL_RULE_CONFIG;
		goto fail;
	}

	/*
	 * Override user rule_id with general ID for this case.
	 */
	rule->rule_id = gen_id;

	/*
	 * Get a free service code for this ACL rule.
	 */
	sc = ppe_drv_acl_sc_get();
	if (!sc) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_policer_sc);
		ppe_acl_warn("%p: not able to find a free service code", rule);
		ret = PPE_ACL_RET_CREATE_FAIL_SC_ALLOC;
		goto fail;
	}

	info.stype = PPE_DRV_ACL_SRC_TYPE_SC;
	info.src = sc;
	acl->sc = sc;

	ppe_acl_info("%p: FLOW + ACL rule servcie code number: %d\n", rule, info.src);

	/*
	 * Allocate empty rule slices in driver.
	 */
	ctx = ppe_drv_acl_alloc(PPE_DRV_ACL_IPO, 1, PPE_ACL_PRI_NOMINAL);
	if (!ctx) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_alloc);
		ppe_acl_warn("%p: couldn't allocate a free ACL rule policer_id: %d",
				acl_g, rule->hw_policer_idx);
		ret = PPE_ACL_RET_CREATE_FAIL_DRV_ALLOC;
		goto fail;
	}

	ppe_acl_info("%p: ppe acl rule context: %p", acl_g, ctx);

	/*
	 * Configure a default rule just for binding FLOW and ACL with service code.
	 */
	slice = &info.chain[PPE_DRV_ACL_SLICE_TYPE_SRC_MAC];
	memset(slice->rule.smac.mac, 0x0, ETH_ALEN);
	memset(slice->rule.smac.mac_mask, 0x0, ETH_ALEN);
	slice->type = PPE_DRV_ACL_SLICE_TYPE_SRC_MAC;
	slice->valid = true;

	/*
	 * Fill policer index as ACL rule action
	 */
	info.action.policer_index = rule->hw_policer_idx;
	info.action.flags |= PPE_DRV_ACL_ACTION_FLAG_POLICER_INDEX;

	if (rule->pkt_noedit) {
		info.action.flags |= PPE_DRV_ACL_ACTION_FLAG_SC;
		info.action.service_code = PPE_DRV_SC_NOEDIT_ACL_POLICER;
	}

	if (ppe_drv_acl_configure(ctx, &info) != PPE_DRV_RET_SUCCESS) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_rule_config);
		ppe_acl_warn("%p: failed to configure ACL rule: %p", acl_g, ctx);
		ret = PPE_ACL_RET_CREATE_FAIL_RULE_CONFIG;
		goto fail;
	}

	ppe_acl_info("%p: using rule_id : %d", acl_g, rule->rule_id);

	/*
	 * Add new rule node to list
	 */
	list_add(&acl->list, &acl_g->active_rules);

	/*
	 * Store the rule id in response.
	 */
	rule->ret = PPE_ACL_RET_SUCCESS;
	rule->sc = sc;

	/*
	 * Update stats
	 */
	ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_req);
	ppe_acl_info("%p: ACL rule created with rule ID: %d", acl_g, rule->rule_id);

	/*
	 * Store book keeping info.
	 * We use a base rule ID to avoid conflicting it with user space rule-id.
	 */
	acl->rule_id = rule->rule_id;
	acl->ctx = ctx;
	kref_init(&acl->ref_cnt);

	spin_unlock_bh(&acl_g->lock);
	return PPE_ACL_RET_SUCCESS;

fail:
	if (ctx) {
		ppe_drv_acl_destroy(ctx);
	}

	if (acl) {
		if (acl->sc) {
			ppe_drv_acl_sc_return(acl->sc);
		}

		ppe_acl_free(acl);
	}

	rule->ret = ret;
	spin_unlock_bh(&acl_g->lock);
	return ret;
}

/*
 * ppe_acl_rule_create()
 *	Create ACL rule in PPE.
 */
ppe_acl_ret_t ppe_acl_rule_create(struct ppe_acl_rule *rule)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	enum ppe_drv_acl_slice_type slice_t;
	struct ppe_drv_acl_ctx *ctx = NULL;
	ppe_acl_rule_match_type_t rule_t;
	ppe_acl_rule_id_t gen_id = -1;
	struct ppe_acl *acl = NULL;
	uint8_t slice_cnt = 0;
	ppe_acl_ret_t ret;

	ppe_acl_info("%p: rule create request: %p", acl_g, rule);


	spin_lock_bh(&acl_g->lock);
	acl = ppe_acl_alloc();
	if (!acl) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_oom);
		ppe_acl_warn("%p: failed to allocate acl memory: %p", acl_g, rule);
		ret = PPE_ACL_RET_CREATE_FAIL_OOM;
		goto fail;
	}

	/*
	 * If rule_id is not specified by user, find a general free ID. If specified, validate it.
	 */
	if (rule->cmn.cmn_flags & PPE_ACL_RULE_CMN_FLAG_NO_RULEID) {
		gen_id = ppe_acl_rule_get_gen_id();
		if (gen_id < 0) {
			ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_rule_table_full);
			ppe_acl_warn("%p: cannot allocate a free rule ID", acl_g);
			ret = PPE_ACL_RET_CREATE_FAIL_RULE_CONFIG;
			goto fail;
		}

		/*
		 * Override user rule_id with general ID for this case.
		 */
		rule->rule_id = gen_id;
	} else {
		if (rule->rule_id >= PPE_ACL_USER_RULE_ID_MAX) {
			ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_invalid_id);
			ppe_acl_warn("%p: Invalid rule ID: %p", acl_g, rule);
			ret = PPE_ACL_RET_CREATE_FAIL_INVALID_ID;
			goto fail;
		}
	}

	/*
	 * Identify the different slice types needed for rules.
	 */
	for (rule_t = 0; rule_t < PPE_ACL_RULE_MATCH_TYPE_MAX; rule_t++) {
		if (!(rule->valid_flags & (1 << rule_t))) {
			continue;
		}

		ppe_acl_rule_to_slice_type(&rule->rules[rule_t], rule_t, &acl->slice_type[0]);
	}

	/*
	 * Figure out the number of ACL slices needed based on the request from user.
	 */
	for (slice_t = 0; slice_t < PPE_DRV_ACL_SLICE_TYPE_MAX; slice_t++) {
		if (!acl->slice_type[slice_t]) {
			continue;
		}

		slice_cnt++;
	}

	ppe_acl_info("%p: number of slice needed: %d", acl_g, slice_cnt);

	/*
	 * Check if the number of rules required are more than the max.
	 */
	if (slice_cnt > PPE_DRV_ACL_RULE_CHAIN_MAX) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_max_slices);
		ppe_acl_warn("%p: Request slice cnt: %d is more than max: %d", acl_g, slice_cnt, PPE_DRV_ACL_RULE_CHAIN_MAX);
		ret = PPE_ACL_RET_CREATE_FAIL_MAX_SLICES;
		goto fail;
	}

	/*
	 * Fill the common info for this ACL rule.
	 */
	if (!ppe_acl_rule_cmn_fill(acl, rule)) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_invalid_cmn);
		ppe_acl_warn("%p: Invalid common fields in rule info %p\n", acl_g, rule);
		ret = PPE_ACL_RET_CREATE_FAIL_INVALID_CMN;
		goto fail;
	}

	/*
	 * Allocate empty rule slices in driver.
	 */
	ctx = ppe_drv_acl_alloc(acl->ipo, slice_cnt, acl->pri);
	if (!ctx) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_alloc);
		ppe_acl_warn("%p: couldn't allocate a free ACL rule slice_cnt: %d", acl_g, slice_cnt);
		ret = PPE_ACL_RET_CREATE_FAIL_DRV_ALLOC;
		goto fail;
	}

	ppe_acl_info("%p: ppe acl rule context: %p", acl_g, ctx);

	/*
	 * Gather the rule info based on user request.
	 */
	acl->rule_valid_flag = rule->valid_flags;
	for (rule_t = 0; rule_t < PPE_ACL_RULE_MATCH_TYPE_MAX; rule_t++) {
		if (!(rule->valid_flags & (1 << rule_t))) {
			continue;
		}

		if (!ppe_acl_rule_info_fill(acl, &rule->rules[rule_t], rule_t)) {
			ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_fill);
			ppe_acl_warn("%p: couldn't map ACL rule to ACL slices rule-num: %d", acl_g, rule_t);
			ret = PPE_ACL_RET_CREATE_FAIL_RULE_PARSE;
			goto fail;
		}
	}

	/*
	 * Fill the source to which this ACL rule need to be binded.
	 */
	if (!ppe_acl_rule_src_fill(acl, rule)) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_invalid_src);
		ppe_acl_warn("%p: failed to find valid src for rule %p\n", acl_g, rule);
		ret = PPE_ACL_RET_CREATE_FAIL_INVALID_SRC;
		goto fail;
	}

	/*
	 * Now that the rule information is extraced, confirm same rule
	 * doesn't exist already.
	 */
	acl->rule_id = rule->rule_id;
	if (ppe_acl_rule_exist(acl)) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_rule_exist);
		ppe_acl_warn("%p: failed to configure ACL rule: %p", acl_g, ctx);
		ret = PPE_ACL_RET_CREATE_FAIL_RULE_CONFIG;
		goto fail;
	}

	/*
	 * Fill ACL rule action
	 */
	if (!ppe_acl_action_fill(acl, &rule->action)) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_action_config);
		ppe_acl_warn("%p: failed to configure ACL action: %p", acl_g, ctx);
		ret = PPE_ACL_RET_CREATE_FAIL_ACTION_CONFIG;
		goto fail;
	}

	if (ppe_drv_acl_configure(ctx, &acl->info) != PPE_DRV_RET_SUCCESS) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_fail_rule_config);
		ppe_acl_warn("%p: failed to configure ACL rule: %p", acl_g, ctx);
		ret = PPE_ACL_RET_CREATE_FAIL_RULE_CONFIG;
		goto fail;
	}

	ppe_acl_info("%p: using free rule_id : %d", acl_g, rule->rule_id);

	/*
	 * Add new rule node to list
	 */
	list_add(&acl->list, &acl_g->active_rules);

	/*
	 * Store the status in response.
	 */
	rule->ret = PPE_ACL_RET_SUCCESS;

	/*
	 * Update stats
	 */
	ppe_acl_stats_inc(&acl_g->stats.cmn.acl_create_req);
	ppe_acl_info("%p: ACL rule created with rule ID: %d", acl_g, rule->rule_id);

	/*
	 * Store book keeping info.
	 */
	acl->ctx = ctx;
	kref_init(&acl->ref_cnt);
	acl->slice_cnt = slice_cnt;
	memcpy(&acl->rule, rule, sizeof(struct ppe_acl_rule));

	spin_unlock_bh(&acl_g->lock);
	return PPE_ACL_RET_SUCCESS;

fail:
	if (ctx) {
		ppe_drv_acl_destroy(ctx);
	}

	if (acl) {
		if (acl->sc) {
			ppe_drv_acl_sc_return(acl->sc);
		}

		ppe_acl_free(acl);
	}

	rule->ret = ret;
	spin_unlock_bh(&acl_g->lock);
	return ret;
}
EXPORT_SYMBOL(ppe_acl_rule_create);

/*
 * ppe_acl_hw_ind_map_entries_free()
 *	Free the ACL hw index map table.
 */
void ppe_acl_hw_ind_map_entries_free(struct ppe_acl_rule_hw_ind_map *acl_hw_map)
{
	vfree(acl_hw_map);
}

/*
 * ppe_acl_hw_ind_map_entries_alloc()
 *	Allocates and initialize the ACL hw index map.
 */
struct ppe_acl_rule_hw_ind_map *ppe_acl_hw_ind_map_entries_alloc(void)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl_rule_hw_ind_map *acl_hw_map = NULL;

	acl_hw_map = vzalloc(sizeof(struct ppe_acl_rule_hw_ind_map) * PPE_DRV_ACL_HW_INDEX_MAX);
	if (!acl_hw_map) {
		ppe_acl_warn("%p: Failed to allocate ACL rule cb table entries", acl_g);
		return NULL;
	}

	return acl_hw_map;
}

/*
 * ppe_acl_rule_ref()
 *	Get a reference to the ACL rule.
 */
bool ppe_acl_rule_ref(ppe_acl_rule_id_t acl_id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *ppe_acl = NULL;

	spin_lock_bh(&acl_g->lock);

	/*
	 * Find the ACL rule for the ACL ID.
	 */
	ppe_acl = ppe_acl_rule_find_by_id(acl_id);
	if (!ppe_acl) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_not_found);
		ppe_acl_warn("No ACL rule present for id %d\n", acl_id);
		return false;
	}

	kref_get(&ppe_acl->ref_cnt);
	spin_unlock_bh(&acl_g->lock);

	return true;
}
EXPORT_SYMBOL(ppe_acl_rule_ref);

/*
 * ppe_acl_rule_deref()
 *	Dereference to the ACL rule.
 */
bool ppe_acl_rule_deref(ppe_acl_rule_id_t acl_id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *ppe_acl = NULL;

	spin_lock_bh(&acl_g->lock);

	/*
	 * Find the ACL rule for the ACL ID.
	 */
	ppe_acl = ppe_acl_rule_find_by_id(acl_id);
	if (!ppe_acl) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_not_found);
		ppe_acl_warn("No ACL rule present for id %d\n", acl_id);
		return false;
	}

	if (kref_put(&ppe_acl->ref_cnt, ppe_acl_rule_free)) {
		ppe_acl_trace("%p: reference goes down to 0 for acl: %p ID: %d\n",
				acl_g, ppe_acl, acl_id);
	}

	spin_unlock_bh(&acl_g->lock);
	return true;
}
EXPORT_SYMBOL(ppe_acl_rule_deref);

/*
 * ppe_acl_rule_get_acl_hw_index()
 *	Get ACL hardware index for a software rule ID.
 */
uint16_t ppe_acl_rule_get_acl_hw_index(ppe_acl_rule_id_t acl_id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl *ppe_acl = NULL;
	struct ppe_drv_acl_ctx *ctx = NULL;
	struct ppe_drv_acl_hw_info acl_hw_info = {0};
	acl_hw_info.hw_rule_id = PPE_ACL_INVALID_HW_INDEX;

	spin_lock_bh(&acl_g->lock);

	/*
	 * Find the ACL rule for the ACL ID.
	 */
	ppe_acl = ppe_acl_rule_find_by_id(acl_id);
	if (!ppe_acl) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_not_found);
		ppe_acl_warn("No ACL rule present for id %d\n", acl_id);
		return acl_hw_info.hw_rule_id;
	}

	/*
	 * Get the hardware index for the ACL ID.
	 * The same is stored inside the ACL context while creating the rule.
	 */
	ctx = ppe_acl->ctx;
	ppe_drv_acl_hw_info_get(ctx, &acl_hw_info);

	spin_unlock_bh(&acl_g->lock);
	return acl_hw_info.hw_rule_id;
}
EXPORT_SYMBOL(ppe_acl_rule_get_acl_hw_index);

/*
 * ppe_acl_rule_callback_register()
 *	Register PPE ACL callback for ACL rule module.
 */
bool ppe_acl_rule_callback_register(ppe_acl_rule_id_t acl_id, ppe_acl_rule_process_callback_t cb, void *appdata)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl_rule_hw_ind_map *acl_hw_map = NULL;
	struct ppe_acl *ppe_acl = NULL;
	struct ppe_drv_acl_ctx *ctx = NULL;
	struct ppe_drv_acl_hw_info acl_hw_info = {0};

	acl_hw_info.hw_rule_id = PPE_ACL_INVALID_HW_INDEX;

	if (acl_id < 0 || acl_id >= PPE_ACL_RULE_ID_MAX) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_id_invalid);
		ppe_acl_warn("%p: Invalid rule ID: %d", acl_g, acl_id);
		return false;
	}

	ppe_acl_assert(cb, "%p: cannot register null cb for ACL id %u", p, acl_id);

	spin_lock_bh(&acl_g->lock);

	/*
	 * Get the ACL rule to store the callback info.
	 */
	ppe_acl = ppe_acl_rule_find_by_id(acl_id);
	if (!ppe_acl) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_not_found);
		ppe_acl_warn("%p: ACL rule not found: %d", acl_g, acl_id);
		return false;
	}

	/*
	 * Get the hardware index for the ACL ID.
	 * The same is stored inside the ACL context while creating the rule.
	 */
	ctx = ppe_acl->ctx;
	ppe_drv_acl_hw_info_get(ctx, &acl_hw_info);

	if (!acl_g->acl_hw_map) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_warn("%p: No ACL hw mapping found: %d", acl_g, acl_id);
		return false;
	}

	acl_hw_map = &acl_g->acl_hw_map[acl_hw_info.hw_rule_id];
	ppe_acl_assert(!acl_hw_map->acl_rule, "%p: multiple registration for acl id:%u - "
				"prev cb:%p current cb:%p", acl_g, acl_id, cb, acl_hw_map->acl_rule->cb);

	ppe_acl->cb = cb;
	ppe_acl->app_data = appdata;
	kref_get(&ppe_acl->ref_cnt);

	acl_hw_map->acl_rule = ppe_acl;

	spin_unlock_bh(&acl_g->lock);
	return true;
}
EXPORT_SYMBOL(ppe_acl_rule_callback_register);

/*
 * ppe_acl_rule_callback_unregister()
 *	Unregister the acl rule callback.
 */
void ppe_acl_rule_callback_unregister(ppe_acl_rule_id_t acl_id)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl_rule_hw_ind_map *acl_hw_map = NULL;
	struct ppe_acl *ppe_acl = NULL;
	struct ppe_drv_acl_ctx *ctx = NULL;
	struct ppe_drv_acl_hw_info acl_hw_info = {0};

	acl_hw_info.hw_rule_id = PPE_ACL_INVALID_HW_INDEX;

	if (acl_id < 0 || acl_id >= PPE_ACL_RULE_ID_MAX) {
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_id_invalid);
		ppe_acl_warn("%p: Invalid rule ID: %d", acl_g, acl_id);
		return;
	}

	spin_lock_bh(&acl_g->lock);

	if (!acl_g->acl_hw_map) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_warn("%p: No ACL hw mapping found: %d", acl_g, acl_id);
		return;
	}

	/*
	 * Get the ACL rule to get the hardware context.
	 */
	ppe_acl = ppe_acl_rule_find_by_id(acl_id);
	if (!ppe_acl) {
		spin_unlock_bh(&acl_g->lock);
		ppe_acl_stats_inc(&acl_g->stats.cmn.rule_not_found);
		ppe_acl_warn("%p: ACL rule not found: %d", acl_g, acl_id);
		return;
	}

	/*
	 * Get the hardware index for the ACL ID.
	 * The same is stored inside the ACL context while creating the rule.
	 */
	ctx = ppe_acl->ctx;
	ppe_drv_acl_hw_info_get(ctx, &acl_hw_info);

	acl_hw_map = &acl_g->acl_hw_map[acl_hw_info.hw_rule_id];
	ppe_acl_assert(acl_hw_map->acl_rule, "%p: No callback registered for the ACL ID:%u - ", acl_g, acl_id);

	/*
	 * unregister the callback for the ACL rule.
	 * Unmap the ACL id from the hw index.
	 */
	acl_hw_map->acl_rule->cb = NULL;
	acl_hw_map->acl_rule->app_data = NULL;
	if (kref_put(&acl_hw_map->acl_rule->ref_cnt, ppe_acl_rule_free)) {
			ppe_acl_trace("%p: reference goes down to 0 for acl: %p ID: %d\n",
				acl_g, acl_hw_map->acl_rule, acl_id);
	}

	acl_hw_map->acl_rule = NULL;
	spin_unlock_bh(&acl_g->lock);
}
EXPORT_SYMBOL(ppe_acl_rule_callback_unregister);

/*
 * ppe_acl_deinit()
 *	ACL deinit API
 */
void ppe_acl_deinit(void)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;

	ppe_drv_acl_flow_unregister_cb();
	ppe_drv_acl_rule_unregister_cb();

	ppe_acl_hw_ind_map_entries_free(acl_g->acl_hw_map);

	ppe_acl_dump_exit();
	ppe_acl_stats_debugfs_exit();
}

/*
 * ppe_acl_init()
 *	ACL init API
 */
void ppe_acl_init(struct dentry *d_rule)
{
	struct ppe_acl_base *acl_g = &ppe_acl_gbl;
	struct ppe_acl_gen_rule_id *rule_id;
	int i;

	spin_lock_init(&acl_g->lock);

	/*
	 * Initialize active list
	 */
	INIT_LIST_HEAD(&acl_g->active_rules);

	/*
	 * Rule ID table - used to maitain a free pool of general IDs.
	 */
	acl_g->rule_id_tbl = vzalloc(sizeof(struct ppe_acl_gen_rule_id) * PPE_ACL_GEN_RULE_ID_MAX);
	if (!acl_g->rule_id_tbl) {
		ppe_acl_warn("%p: failed to allocate ACL rule ID table", acl_g);
		return;
	}

	for (i = 0; i < PPE_ACL_GEN_RULE_ID_MAX; i++) {
		rule_id = &acl_g->rule_id_tbl[i];
		rule_id->rule_id = PPE_ACL_GEN_RULE_ID_BASE + i;
		rule_id->in_use = false;
	}

	ppe_drv_acl_flow_register_cb(ppe_acl_rule_flow_add_cb, ppe_acl_rule_flow_del_cb, NULL);
	ppe_drv_acl_rule_register_cb(ppe_acl_rule_process_skb, NULL);

	/*
	 * Allocate memory for ACL callback table.
	 */
	acl_g->acl_hw_map = ppe_acl_hw_ind_map_entries_alloc();

	/*
	 * Create debugfs directory/files.
	 */
	ppe_acl_stats_debugfs_init(d_rule);

	/*
	 * Initialization of ACL dump.
	 */
	ppe_acl_dump_init(d_rule);
}
