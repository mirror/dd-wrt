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

#include "fal_mapt.h"
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_decap_xlate_rule_dump
 *	Dump DRV_TUN_MAP_DCAP instance
 */
void ppe_drv_tun_decap_xlate_rule_dump(struct ppe_drv_tun_decap_xlate_rule *ptdxrule)
{
	sw_error_t err;
	fal_mapt_decap_edit_rule_entry_t m_decap_edit_rule = {0};

	err = fal_mapt_decap_rule_entry_get(PPE_DRV_SWITCH_ID, ptdxrule->index, &m_decap_edit_rule);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to get rule at index %d", ptdxrule, ptdxrule->index);
		return;
	}

	ppe_drv_trace("%p: [%x], SRC1[%X]", ptdxrule, ptdxrule->index, m_decap_edit_rule.ip4_addr);

	ppe_drv_trace("%p: [%x], v6_addr_src[%d], addr_suffix_en[%d] addr_suffix[%d], suffix_width[%d], suffix_pos[%d]",
			ptdxrule, ptdxrule->index,
			m_decap_edit_rule.ip6_addr_src,
			m_decap_edit_rule.ip6_suffix_sel.enable,
			m_decap_edit_rule.ip6_suffix_sel.src_start,
			m_decap_edit_rule.ip6_suffix_sel.src_width,
			m_decap_edit_rule.ip6_suffix_sel.dest_pos);

	ppe_drv_trace("%p: psid_en[%d], psid_start[%d], psid_width[%d] psid_pos[%d]",
			ptdxrule,
			m_decap_edit_rule.ip6_proto_sel.enable,
			m_decap_edit_rule.ip6_proto_sel.src_start,
			m_decap_edit_rule.ip6_proto_sel.src_width,
			m_decap_edit_rule.ip6_proto_sel.dest_pos);

	ppe_drv_trace("%p: proto_src[%d], proto_psid[%d], proto_psid_width[%d], proto_valid[%d], proto_check_en[%d]",
			ptdxrule,
			m_decap_edit_rule.proto_src,
			m_decap_edit_rule.proto_sel.src_start,
			m_decap_edit_rule.proto_sel.src_width,
			m_decap_edit_rule.proto_sel.enable,
			m_decap_edit_rule.check_proto_enable);
}

/*
 * ppe_drv_tun_decap_xlate_rule_free
 *	Deconfigure MAP rule in DRV_TUN_MAP_DCAP
 */
static void ppe_drv_tun_decap_xlate_rule_free(struct kref *kref)
{
	sw_error_t err;
	fal_mapt_decap_edit_rule_entry_t m_decap_edit_rule = {0};
	struct ppe_drv_tun_decap_xlate_rule *ptdxrule = container_of(kref, struct ppe_drv_tun_decap_xlate_rule, ref);

	err = fal_mapt_decap_rule_entry_set(PPE_DRV_SWITCH_ID, ptdxrule->index, &m_decap_edit_rule);
	if (err != SW_OK) {
		ppe_drv_trace("Failed to deconfigure decap entry at rule index %u", ptdxrule->index);
		return;
	}

	ppe_drv_trace("%p: xlate decap entry deconfigured for index %u", ptdxrule, ptdxrule->index);
}

/*
 * ppe_drv_tun_decap_xlate_rule_deref_internal
 *	Taken reference of TL MAP LPM action interface instance
 */
bool ppe_drv_tun_decap_xlate_rule_deref(struct ppe_drv_tun_decap_xlate_rule *ptdxrule)
{
	uint8_t index __maybe_unused = ptdxrule->index;

	ppe_drv_assert(kref_read(&ptdxrule->ref), "%p: ref count under run for tun_decap_xlate_rule", ptdxrule);

	/*
	 * Deconfigure xlate rule entry if ref count becomes zero
	 */
	if (kref_put(&ptdxrule->ref, ppe_drv_tun_decap_xlate_rule_free)) {
		ppe_drv_trace("%p: tun_decap_xlate_rule freed  at index: %d", ptdxrule, index);
		return true;
	}

	ppe_drv_trace("%p: tun_decap_xlate_rule ref count %d", ptdxrule, kref_read(&ptdxrule->ref));
	return true;
}

/*
 * ppe_drv_tun_decap_xlate_rule_ref
 *	Take reference of TL MAP LPM action interface instance
 */
void ppe_drv_tun_decap_xlate_rule_ref(struct ppe_drv_tun_decap_xlate_rule *ptdxrule)
{
	kref_get(&ptdxrule->ref);

	ppe_drv_assert(kref_read(&ptdxrule->ref), "%p: ref count rollover for tun_decap_xlate_rule at index:%d", ptdxrule, ptdxrule->index);
	ppe_drv_trace("%p: tun_decap_xlate_rule %u ref inc:%u", ptdxrule, ptdxrule->index, kref_read(&ptdxrule->ref));
}

/*
 * ppe_tl_map_lpm_rule_tbl_get_rule_id
 *	Return MAP rule id
 */
uint8_t ppe_drv_tun_decap_xlate_rule_get_index(struct ppe_drv_tun_decap_xlate_rule *ptdxrule)
{
	return ptdxrule->index;
}

/*
 * ppe_drv_tun_decap_xlate_rule_configure
 *	Configure MAP rule in DRV_TUN_MAP_DCAP
 */
bool ppe_drv_tun_decap_xlate_rule_configure(struct ppe_drv_tun_decap_xlate_rule *ptdxrule, struct ppe_drv_tun_cmn_ctx_xlate_rule *rule, bool src_ipv6, bool dmr)
{
	uint8_t start2_suffix;
	uint8_t start2_psid;
	uint8_t psid_len;
	uint8_t start3_psid;
	sw_error_t err;
	fal_mapt_decap_edit_rule_entry_t m_decap_edit_rule = {0};
	uint8_t p = (PPE_DRV_TUN_DCAP_XLTE_RULE_IPV4_ADDR_LEN - rule->ipv4_prefix_len);

	/*
	 * DMR configuration
	 */
	if (!dmr) {
		ppe_drv_trace("%p: FMR support is not yet enabled", ptdxrule);
		return false;
	}

	/*
	 *	                PL=96   PL=64   PL=56   PL=48   PL=40   PL=32
	 *	SRC2              1       1       1       1       1       1
	 *	START2_SUFFIX     0      24      32      40      64      64
	 *	WIDTH2_SUFFIX    31      31      23      15      23      31
	 *	POS2_SUFFIX       0       0       0       0       8       0
	 *	SRC2_VALID        1       1       1       1       1       1
	 *	START2_PSID       0       0      64      64      48       0
	 *	WIDTH2_PSID       0       0       7      15       7       0
	 *	SRC3              0       0       1       1       0       0
	 *	START3_PSID       0       0       8       0       0       0
	 *	WIDTH3_PSID       0       0       0       0       0       0
	 *	SRC3_VALID        0       0       0       0       0       0
	 *	CHECK_EN          1       1       0       0       0       1
	 */
	if (src_ipv6) {
		m_decap_edit_rule.ip6_addr_src = FAL_TUNNEL_MAPT_FROM_SRC;
		m_decap_edit_rule.ip6_proto_sel.enable = A_TRUE;
		m_decap_edit_rule.check_proto_enable =  A_TRUE;

		switch (rule->ipv6_prefix_len) {
		case 96:
			m_decap_edit_rule.ip6_suffix_sel.src_width = 32;
			break;
		case 64:
			m_decap_edit_rule.ip6_suffix_sel.src_start = 24;
			m_decap_edit_rule.ip6_suffix_sel.src_width = 32;
			break;
		case 56:
			m_decap_edit_rule.ip6_suffix_sel.src_start = 32;
			m_decap_edit_rule.ip6_suffix_sel.src_width = 24;
			m_decap_edit_rule.ip6_proto_sel.src_start = 64;
			m_decap_edit_rule.ip6_proto_sel.src_width = 8;
			m_decap_edit_rule.proto_sel.src_start = 8;
			m_decap_edit_rule.proto_src = FAL_TUNNEL_MAPT_FROM_SRC;
			m_decap_edit_rule.check_proto_enable = A_FALSE;
			break;
		case 48:
			m_decap_edit_rule.ip6_suffix_sel.src_start = 40;
			m_decap_edit_rule.ip6_suffix_sel.src_width = 16;
			m_decap_edit_rule.ip6_proto_sel.src_start = 64;
			m_decap_edit_rule.ip6_proto_sel.src_width = 16;
			m_decap_edit_rule.proto_src = FAL_TUNNEL_MAPT_FROM_SRC;
			m_decap_edit_rule.check_proto_enable = A_FALSE;
			break;
		case 40:
			m_decap_edit_rule.ip6_suffix_sel.src_start = 64;
			m_decap_edit_rule.ip6_suffix_sel.src_width = 24;
			m_decap_edit_rule.ip6_proto_sel.src_start = 48;
			m_decap_edit_rule.ip6_proto_sel.src_width = 8;
			m_decap_edit_rule.ip6_suffix_sel.dest_pos = 8;
			m_decap_edit_rule.check_proto_enable = A_FALSE;
			break;
		case 32:
			m_decap_edit_rule.ip6_suffix_sel.src_start = 64;
			m_decap_edit_rule.ip6_suffix_sel.src_width = 32;
			break;
		default:
			ppe_drv_warn("%p: Invalid IPv6 prefix length", ptdxrule);
			return false;
		}

		err = fal_mapt_decap_rule_entry_set(PPE_DRV_SWITCH_ID, ptdxrule->index,
							&m_decap_edit_rule);
		if (err != SW_OK) {
			ppe_drv_warn("%p, Failed to set map decap entry for rule %d",  ptdxrule, ptdxrule->index);
			return false;
		}

		return true;
	}

	/*
	 * r = length of the IPv4 prefix given by the BMR
	 * o = length of the EA bit field as given by the BMR
	 * p = length of the IPv4 suffix contained in the EA bit field.
	 *   = 32 - r
	 * n = Rule IPv6 prefix length
	 * q = length of PSID
	 *   = o - p
	 * a = left bits of port
	 *   = 16 - q
	 * START2_SUFFIX=128-n-p, WIDTH2_SUFFIX=p-1, POS2_SUFFIX=0, START2_PSID=128-n-o,
	 * WIDTH2_PSID=q-1, START3_PSID=16-a-q, WIDTH3_PSID=q-1
	 */
	start2_suffix = (PPE_DRV_TUN_DCAP_XLTE_RULE_IPV6_HDR_ADDR_SIZE - rule->ipv6_prefix_len - p);
	start2_psid = (PPE_DRV_TUN_DCAP_XLTE_RULE_IPV6_HDR_ADDR_SIZE - rule->ipv6_prefix_len - rule->ea_len);
	psid_len = (rule->ea_len - p);
	start3_psid = (PPE_DRV_TUN_DCAP_XLTE_L4_PORT_LEN - rule->psid_offset - psid_len);

	m_decap_edit_rule.ip4_addr = rule->ipv4_prefix;
	m_decap_edit_rule.ip6_addr_src = FAL_TUNNEL_MAPT_FROM_DST;
	m_decap_edit_rule.ip6_suffix_sel.src_start = start2_suffix;
	m_decap_edit_rule.ip6_suffix_sel.src_width = p;
	m_decap_edit_rule.ip6_suffix_sel.enable = A_TRUE;
	m_decap_edit_rule.ip6_proto_sel.src_start = start2_psid;
	m_decap_edit_rule.ip6_proto_sel.src_width = psid_len;
	m_decap_edit_rule.ip6_proto_sel.enable = A_TRUE;
	m_decap_edit_rule.proto_sel.enable = A_TRUE;
	m_decap_edit_rule.proto_sel.src_start = start3_psid;
	m_decap_edit_rule.proto_sel.src_width = psid_len;
	m_decap_edit_rule.check_proto_enable = A_TRUE;
	m_decap_edit_rule.proto_src = FAL_TUNNEL_MAPT_FROM_DST;

	err = fal_mapt_decap_rule_entry_set(PPE_DRV_SWITCH_ID, ptdxrule->index, &m_decap_edit_rule);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure rule entry at index %d", ptdxrule, ptdxrule->index);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_decap_xlate_rule_alloc
 *	Return free TL MAP LPM action interface instance or requested at given index
 */
struct ppe_drv_tun_decap_xlate_rule *ppe_drv_tun_decap_xlate_rule_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;

	/*
	 * Return first free instance
	 */
	for (index = 0; index < PPE_DRV_TUN_DCAP_XLTE_RULE_MAX_ENTRIES; index++) {
		struct ppe_drv_tun_decap_xlate_rule *ptdxrule = &p->decap_xlate_rules[index];
		if (kref_read(&ptdxrule->ref)) {
			continue;
		}

		kref_init(&ptdxrule->ref);
		ppe_drv_trace("%p: Free MAP RULE instance found, index: %d", ptdxrule, index);
		return ptdxrule;
	}

	ppe_drv_warn("%p: Free MAP RULE instance is not found", p);
	return NULL;
}

/*
 * ppe_drv_tun_decap_xlate_rule_entries_free
 *	Free memory allocated for tunnel TL xlate rules.
 */
void ppe_drv_tun_decap_xlate_rule_entries_free(struct ppe_drv_tun_decap_xlate_rule *decap_xlate_rules)
{
	vfree(decap_xlate_rules);
}

/*
 * ppe_drv_tun_decap_xlate_rule_entries_alloc
 *	Allocate and initialize xlate rule entries.
 */
struct ppe_drv_tun_decap_xlate_rule *ppe_drv_tun_decap_xlate_rule_entries_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;
	struct ppe_drv_tun_decap_xlate_rule *decap_xlate_rules;

	ppe_drv_assert(!p->decap_xlate_rules, "%p: Decap xlate rules already allocated", p);

	decap_xlate_rules = vzalloc(sizeof(struct ppe_drv_tun_decap_xlate_rule) * PPE_DRV_TUN_DCAP_XLTE_RULE_MAX_ENTRIES);
	if (!decap_xlate_rules) {
		ppe_drv_warn("%p: decap xlate entries allocation failed", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_DCAP_XLTE_RULE_MAX_ENTRIES; index++) {
		decap_xlate_rules[index].index = index;
	}

	return decap_xlate_rules;
}
