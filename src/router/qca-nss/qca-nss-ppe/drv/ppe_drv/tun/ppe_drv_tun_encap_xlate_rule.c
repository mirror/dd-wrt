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

#include <fal_tunnel.h>
#include <linux/if_vlan.h>
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_encap_xlate_rule_dump
 *	Dump contents of encap xlate rule instance
 */
static void ppe_drv_tun_encap_xlate_rule_dump(struct ppe_drv_tun_encap_xlate_rule *ptecxr)
{
	sw_error_t err;
	fal_tunnel_encap_rule_t tmerl = {0};

	err = fal_tunnel_encap_rule_entry_get(PPE_DRV_SWITCH_ID, (uint32_t)(ptecxr->rule_index), &tmerl);
	if (err != SW_OK) {
		return;
	}

	ppe_drv_trace("%p: [%u] SRC1[%d], SRC2[%d], VALID2_0[%d], START2_0[%d], WIDTH2_0[%d], POS2_0[%d], VALID2_1[%d], START2_1[%d] ",
			ptecxr, ptecxr->rule_index,
			tmerl.src1_start,
			tmerl.src2_sel,
			tmerl.src2_entry[0].enable,
			tmerl.src2_entry[0].src_start,
			tmerl.src2_entry[0].src_width,
			tmerl.src2_entry[0].dest_pos,
			tmerl.src2_entry[1].enable,
			tmerl.src2_entry[1].src_start);

	ppe_drv_trace("%p: WIDTH2_1[%d], POS2_1[%d], SRC3[%d], VALID3_0[%d], START3_0[%d], WIDTH3_0[%d], POS3_0[%d] ",
			ptecxr,
			tmerl.src2_entry[1].src_width,
			tmerl.src2_entry[1].dest_pos,
			tmerl.src3_sel,
			tmerl.src3_entry[0].enable,
			tmerl.src3_entry[0].src_start,
			tmerl.src3_entry[0].src_width,
			tmerl.src3_entry[0].dest_pos);

	ppe_drv_trace("%p: VALID3_1[%d], START3_1[%d], WIDTH3_1[%d], POS3_1[%d]", ptecxr,
			tmerl.src3_entry[1].enable,
			tmerl.src3_entry[1].src_start,
			tmerl.src3_entry[1].src_width,
			tmerl.src3_entry[1].dest_pos);
}

/*
 * ppe_drv_tun_encap_xlate_rule_free
 *	Deconfigure TL_TBL index allocated for tunnel decap
 */
static void ppe_drv_tun_encap_xlate_rule_free(struct kref *kref)
{
	sw_error_t err;
	fal_tunnel_encap_rule_t tmerl = {0};
	struct ppe_drv_tun_encap_xlate_rule *ptecxr = container_of(kref,struct ppe_drv_tun_encap_xlate_rule, ref);

	err = fal_tunnel_encap_rule_entry_set(PPE_DRV_SWITCH_ID, ptecxr->rule_index, &tmerl);
	if (err != SW_OK) {
		return;
	}

	ppe_drv_trace("%p: encap_xlate_rule_rule reset at entry index: %u", ptecxr, ptecxr->rule_index);
	return;
}

/*
 * ppe_drv_tun_encap_xlate_rule_deref
 *	Taken reference of map edit rule instance
 */
bool ppe_drv_tun_encap_xlate_rule_deref(struct ppe_drv_tun_encap_xlate_rule *ptecxr)
{
	uint8_t rule_index __maybe_unused = ptecxr->rule_index;

	ppe_drv_assert(kref_read(&ptecxr->ref), "%p: ref count under run for encap_xlate_rule", ptecxr);
	if (kref_put(&ptecxr->ref, ppe_drv_tun_encap_xlate_rule_free)) {
		/*
		 * Deconfigure encap_xlate_rule_RULE entry
		 */
		ppe_drv_trace("reference count is 0 for tun: %p at index: %u", ptecxr, rule_index);
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_encap_xlate_rule_ref
 *	Taken reference of map edit rule instance
 */
struct ppe_drv_tun_encap_xlate_rule *ppe_drv_tun_encap_xlate_rule_ref(struct ppe_drv_tun_encap_xlate_rule *ptecxr)
{
	kref_get(&ptecxr->ref);

	ppe_drv_assert(kref_read(&ptecxr->ref), "%p: ref count rollover for encap_xlate_rule at index:%d", ptecxr, ptecxr->rule_index);
	ppe_drv_trace("%p: encap_xlate_rule %u ref inc:%u", ptecxr, ptecxr->rule_index, kref_read(&ptecxr->ref));

	return ptecxr;
}

/*
 * ppe_drv_tun_encap_xlate_rule_get_index
 *	Get index of encap_xlate_rule table entry
 */
uint8_t ppe_drv_tun_encap_xlate_rule_get_index(struct ppe_drv_tun_encap_xlate_rule *ptecxr)
{
	return ptecxr->rule_index;
}

/*
 * ppe_drv_tun_encap_xlate_rule_configure
 *	Configure encap_xlate_rule_RULE table entry
 */
bool ppe_drv_tun_encap_xlate_rule_configure(struct ppe_drv_tun_encap_xlate_rule *ptecxr,
				struct ppe_drv_tun_cmn_ctx_xlate_rule *rule, int8_t tun_len, uint32_t l2_flags, bool dmr)
{
	fal_tunnel_encap_rule_t mapt_edit_rule = {0};
	uint8_t daddr_offset;
	uint8_t l2_offset;
	sw_error_t err;
	uint8_t src1;

	ppe_drv_assert(kref_read(&ptecxr->ref), "%p: encap_xlate_rule api called without reference", ptecxr);

	/*
	 * DMR configuration
	 */
	if (!dmr) {
		ppe_drv_trace("%p: FMR support is not yet enabled", ptecxr);
		return false;
	}

	/*
	 *	                PL=96   PL=64   PL=56   PL=48   PL=40   PL=32
	 *	SRC1                    Dst IPv6 address location
	 *	SRC2              1       1       1       1       1       1
	 *	VALID2_0          1       1       1       1       1       1
	 *	START2_0          0       0       0       0       0       0
	 *	WIDTH2_0         31      31      23      15       7      31
	 *	POS2_0            0      24      32      40      48      64
	 *	VALID2_1          0       0       1       1       1       0
	 *	START2_1          0       0      24      16       8       0
	 *	WIDTH2_1          0       0       7      15      23       0
	 *	POS2_1            0       0      64      64      64       0
	 */

	/*
	 * src1 is the position of destination IPv6 address in EG_HEADER_DATA.
	 * Position is configured in 16-bits + 1, as per HW requirement.
	 */
	src1 = (tun_len - 16) / 2 + 1;

	l2_offset = sizeof(struct ethhdr);

	if (l2_flags & PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID) {
		l2_offset += PPPOE_SES_HLEN;
	}

	if (l2_flags & PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID) {
		l2_offset += sizeof(struct vlan_hdr) * 2;
	} else if (l2_flags & PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID) {
		l2_offset += sizeof(struct vlan_hdr);
	}

	daddr_offset = offsetof(struct ipv6hdr, daddr);

	mapt_edit_rule.src1_start = l2_offset + daddr_offset;
	mapt_edit_rule.src1_sel = FAL_TUNNEL_RULE_SRC1_FROM_HEADER_DATA;
	mapt_edit_rule.src2_sel = FAL_TUNNEL_RULE_SRC2_PKT_DATA0;
	mapt_edit_rule.src2_entry[0].enable = A_TRUE;

	switch (rule->ipv6_prefix_len) {
	case 96:
		mapt_edit_rule.src2_entry[0].src_width = 32;
		break;

	case 64:
		mapt_edit_rule.src2_entry[0].src_width = 32;
		mapt_edit_rule.src2_entry[0].dest_pos = 24;
		break;

	case 56:
		mapt_edit_rule.src2_entry[0].src_width = 24;
		mapt_edit_rule.src2_entry[0].dest_pos = 32;
		mapt_edit_rule.src2_entry[1].enable = A_TRUE;
		mapt_edit_rule.src2_entry[1].src_start = 24;
		mapt_edit_rule.src2_entry[1].src_width = 8;
		mapt_edit_rule.src2_entry[1].dest_pos = 64;
		break;

	case 48:
		mapt_edit_rule.src2_entry[0].src_width = 16;
		mapt_edit_rule.src2_entry[0].dest_pos = 40;
		mapt_edit_rule.src2_entry[1].enable = A_TRUE;
		mapt_edit_rule.src2_entry[1].src_start = 16;
		mapt_edit_rule.src2_entry[1].src_width = 16;
		mapt_edit_rule.src2_entry[1].dest_pos = 64;
		break;

	case 40:
		mapt_edit_rule.src2_entry[0].src_width = 8;
		mapt_edit_rule.src2_entry[0].dest_pos = 48;
		mapt_edit_rule.src2_entry[1].enable = A_TRUE;
		mapt_edit_rule.src2_entry[1].src_start = 8;
		mapt_edit_rule.src2_entry[1].src_width = 24;
		mapt_edit_rule.src2_entry[1].dest_pos = 64;
		break;

	case 32:
		mapt_edit_rule.src2_entry[0].src_width = 32;
		mapt_edit_rule.src2_entry[0].dest_pos = 64;
		break;

	default:
		ppe_drv_warn("%p: Invalid IPv6 prefix length", ptecxr);
		return false;
	}

	ppe_drv_trace("%p: encap_xlate_rule entry position: %u", ptecxr, ptecxr->rule_index);
	err = fal_tunnel_encap_rule_entry_set(PPE_DRV_SWITCH_ID , ptecxr->rule_index, &mapt_edit_rule);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Failed to configure encap rule at rule index %d", ptecxr, ptecxr->rule_index);
		return false;
	}

	ppe_drv_tun_encap_xlate_rule_dump(ptecxr);
	return true;
}

/*
 * ppe_drv_tun_encap_xlate_rule_alloc
 *	Return free encap rule instance
 */
struct ppe_drv_tun_encap_xlate_rule *ppe_drv_tun_encap_xlate_rule_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;
	struct ppe_drv_tun_encap_xlate_rule *ptecxr;

	/*
	 * Return first free instance
	 */
	for (index = 0; index < PPE_DRV_TUN_ENCAP_XLTE_RULE_MAX_RULES; index++) {
		ptecxr = &p->encap_xlate_rules[index];
		if (kref_read(&ptecxr->ref)) {
			continue;
		}

		kref_init(&ptecxr->ref);
		ppe_drv_trace("%p: Free encap rule instance found, index: %d", ptecxr, index);
		return ptecxr;
	}

	ppe_drv_warn("%p: Free encap rule instance is not found", p);
	return NULL;
}

/*
 * ppe_drv_tun_encap_xlate_rule_entries_free
 *	Free memory allocated.
 */
void ppe_drv_tun_encap_xlate_rule_entries_free(struct ppe_drv_tun_encap_xlate_rule *encap_xlate_rules)
{
	vfree(encap_xlate_rules);
}

/*
 * ppe_drv_tun_encap_xlate_rule_entries_alloc
 *	Allocate and initialize map edit rule entries.
 */
struct ppe_drv_tun_encap_xlate_rule *ppe_drv_tun_encap_xlate_rule_entries_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;
	struct ppe_drv_tun_encap_xlate_rule *encap_xlate_rules;

	ppe_drv_assert(!p->encap_xlate_rules, "%p: Encap xlate rules already allocated", p);

	encap_xlate_rules = vzalloc(sizeof(struct ppe_drv_tun_encap_xlate_rule) * PPE_DRV_TUN_ENCAP_XLTE_RULE_MAX_RULES);
	if (!encap_xlate_rules) {
		ppe_drv_warn("%p: failed to allocate encap_xlate_rules entries", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_ENCAP_XLTE_RULE_MAX_RULES; index++) {
		encap_xlate_rules[index].rule_index = index;
	}

	return encap_xlate_rules;
}
