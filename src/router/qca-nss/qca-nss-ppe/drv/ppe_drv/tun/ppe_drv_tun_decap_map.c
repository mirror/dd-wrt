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

#include <fal_mapt.h>
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_decap_map_dump
 *	Dump MAP decap entry instance
 */
void ppe_drv_tun_decap_map_entry_dump(struct ppe_drv_tun_decap *ptdcm)
{
	fal_mapt_decap_entry_t fmde = {0};
	sw_error_t err;

	err = fal_mapt_decap_entry_getfirst(PPE_DRV_SWITCH_ID, &fmde);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Unable to get MAP entry at index %d", ptdcm, ptdcm->index);
		return;
	}

	ppe_drv_trace("%p: [%x], rule_index[%x]", ptdcm, ptdcm->index, fmde.edit_rule_id);

	ppe_drv_trace("%p: ipv6_prefix[%x], ipv6_addr [%x]:[%x]:[%x]:[%x]",
			ptdcm,
			fmde.ip6_prefix_len,
			fmde.ip6_addr.ul[0],
			fmde.ip6_addr.ul[1],
			fmde.ip6_addr.ul[2],
			fmde.ip6_addr.ul[3]);

	ppe_drv_trace("%p: verify_bmp[%x] svlan_id[%d] cvlan_id[%d] svlan_fmt[%d] cvlan_fmt[%d]",
			ptdcm,
			fmde.verify_entry.verify_bmp,
			fmde.verify_entry.svlan_id,
			fmde.verify_entry.cvlan_id,
			fmde.verify_entry.svlan_fmt,
			fmde.verify_entry.cvlan_fmt);

	ppe_drv_trace("%p: src_info_en[%d], src_info_type[%d], src_info[%d], tun_l3_if[%d]",
			ptdcm,
			fmde.src_info_enable,
			fmde.src_info_type,
			fmde.src_info,
			fmde.verify_entry.tl_l3_if);
}

void ppe_drv_tun_decap_map_deconfigure(struct kref *kref)
{
	sw_error_t err;
	fal_mapt_decap_entry_t fmde = {0};
	struct ppe_drv_tun_decap *ptdcm = container_of(kref, struct ppe_drv_tun_decap, ref);

	fmde.op_mode = FAL_TUNNEL_OP_MODE_INDEX;
	fmde.entry_index = ptdcm->tl_index;
	err = fal_mapt_decap_entry_del(PPE_DRV_SWITCH_ID, &fmde);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Unable to reset MAP entry at index %d", ptdcm, ptdcm->index);
		return;
	}

	ppe_drv_info("%p: Reset done MAP entry at index %d", ptdcm, ptdcm->index);
}

/*
 * ppe_drv_tun_decap_map_entry_deref
 *	De reference of  MAP tunnel entry instance
 */
bool ppe_drv_tun_decap_map_entry_deref(struct ppe_drv_tun_decap *ptdcm)
{
	uint8_t index __maybe_unused = ptdcm->index;

	ppe_drv_assert(kref_read(&ptdcm->ref), "%p: ref count under run for ppe_drv_tun_decap", ptdcm);

	if (kref_put(&ptdcm->ref, ppe_drv_tun_decap_map_deconfigure)) {
		ppe_drv_trace("%p:  MAP tunnel action interface is free at index: %d", ptdcm, index);
		return true;
	}

	ppe_drv_trace("%p: ppe_drv_tun_decap ref count: %d", ptdcm, kref_read(&ptdcm->ref));
	return false;
}

/*
 * ppe_drv_tun_decap_map_ref
 *	Take reference of  MAP tunnel entry instance
 */
void ppe_drv_tun_decap_map_ref(struct ppe_drv_tun_decap *ptdcm)
{
	kref_get(&ptdcm->ref);

	ppe_drv_assert(kref_read(&ptdcm->ref), "%p: ref count rollover for ppe_drv_tun_decap at index:%d", ptdcm, ptdcm->index);
	ppe_drv_trace("%p: ppe_drv_tun_decap %u ref inc:%u", ptdcm, ptdcm->index, kref_read(&ptdcm->ref));
}

/*
 * ppe_drv_tun_decap_map_get_rule_index
 *	Return MAP tunnel rule id
 */
uint8_t ppe_drv_tun_decap_map_get_rule_id(struct ppe_drv_tun_decap *ptdcm)
{
	return ptdcm->rule_id;
}

/*
 * ppe_drv_tun_decap_map_set_rule_index
 *	set MAP tunnel entry xlate rule id
 */
void ppe_drv_tun_decap_map_set_rule_id(struct ppe_drv_tun_decap *ptdcm, uint16_t rule_idx)
{
	ptdcm->rule_id = rule_idx;
}

/*
 * ppe_drv_tun_decap_map_configure
 *	Configure map decap tunnel entry
 */
bool ppe_drv_tun_decap_map_configure(struct ppe_drv_tun_decap *ptdcm, uint32_t *ipv6_prefix, uint8_t prefix_len,
					struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr, bool src_info_update, uint16_t port_num,
					uint16_t rule_id,uint8_t ip_to_me)
{
	sw_error_t err;
	fal_mapt_decap_entry_t fmde = {0};

	/*
	 * Set values in remote TL MAP ACT entry,
	 * for local entry these values are not required.
	 */
	if (src_info_update) {
		fmde.src_info = port_num;
		fmde.src_info_type = PPE_DRV_TUN_TL_TBL_SRC_INFO_TYPE_VP;
		fmde.src_info_enable = A_TRUE;
	}

	fmde.edit_rule_id =   ptdcm->rule_id;
	fmde.ip6_addr.ul[0] = htonl(ipv6_prefix[0]);
	fmde.ip6_addr.ul[1] = htonl(ipv6_prefix[1]);
	fmde.ip6_addr.ul[2] = htonl(ipv6_prefix[2]);
	fmde.ip6_addr.ul[3] = htonl(ipv6_prefix[3]);
	fmde.ip6_prefix_len = prefix_len;

	/*
	 * If decap entry in HW is configured with local IPv6 address
	 * then set this bit and program the entry.  Other fields
	 * are not required.
	 */
	if (ip_to_me) {
		fmde.dst_is_local = A_TRUE;
		goto done;
	}

	/*
	 * Update SVLAN Parameters
	 */
	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID) {
		/*
		 * Fill the SVLAN (primary VLAN)
		 */
		fmde.verify_entry.verify_bmp |= FAL_TUNNEL_SVLAN_CHECK_EN;
		fmde.verify_entry.svlan_fmt = PPE_DRV_TUN_FIELD_VALID;
		fmde.verify_entry.svlan_id = l2_hdr->vlan[0].tci;

		/*
		 * Fill the CVLAN (Secondary VLAN)
		 */
		fmde.verify_entry.verify_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		fmde.verify_entry.cvlan_fmt = PPE_DRV_TUN_FIELD_VALID;
		fmde.verify_entry.cvlan_id = l2_hdr->vlan[1].tci;

		ppe_drv_trace("%p: TL_TBL SVLAN_ID: %d", ptdcm, fmde.verify_entry.svlan_id);
		ppe_drv_trace("%p: TL_TBL CVLAN_ID: %d", ptdcm, fmde.verify_entry.cvlan_id);
	} else if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID) {
		/*
		 * Fill the CVLAN (Primary VLAN)
		 */
		fmde.verify_entry.verify_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		fmde.verify_entry.cvlan_fmt = PPE_DRV_TUN_FIELD_VALID;
		fmde.verify_entry.cvlan_id = l2_hdr->vlan[0].tci;
		ppe_drv_trace("%p: TL_TBL CVLAN_ID: %d", ptdcm, fmde.verify_entry.cvlan_id);
	}

	fmde.verify_entry.verify_bmp |= FAL_TUNNEL_L3IF_CHECK_EN;
	fmde.verify_entry.tl_l3_if = ptdcm->tl_l3_if_idx;

done:
	err = fal_mapt_decap_entry_add(PPE_DRV_SWITCH_ID, &fmde);
	if (err != SW_OK) {
		/*
		 * This is a re-activate entry call. So, the decap map entry will be already present.
		 */
		if (err == SW_ALREADY_EXIST) {
			ppe_drv_info("%p: Using existing MAPT decap entry %d", ptdcm, ptdcm->tl_index);
			return true;
		}

		ppe_drv_warn("%p: failed to add MAP decap entry at index %d", ptdcm, ptdcm->index);
		return false;
	}

	/*
	 * Store mapt decap index for further use.
	 */
	ptdcm->tl_index = fmde.entry_index;
	return true;
}

/*
 * ppe_drv_tun_decap_map_alloc
 *	Return free map decap entry.
 */
struct ppe_drv_tun_decap *ppe_drv_tun_decap_map_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;

	/*
	 * Return first free instance
	 */
	for (index = 0; index < PPE_DRV_TUN_DECAP_MAP_MAX_ENTRY; index++) {
		struct ppe_drv_tun_decap *ptdcm = &p->decap_map_entries[index];
		if (kref_read(&ptdcm->ref)) {
			continue;
		}

		ppe_drv_trace("%p: Free TL MAP RULE instance found, index: %d", ptdcm, index);
		kref_init(&ptdcm->ref);
		ptdcm->index = index;
		return ptdcm;
	}

	ppe_drv_warn("%p: Free  MAP tunnel action interface instance is not found", p);
	return NULL;
}

/*
 * ppe_drv_tun_decap_map_entries_free
 *	Free memory allocated for  MAP tunnel decap entries.
 */
void ppe_drv_tun_decap_map_entries_free(struct ppe_drv *p)
{
	if (!p->decap_map_entries) {
		return;
	}

	vfree(p->decap_map_entries);
}

/*
 * ppe_drv_tun_decap_map_entries_alloc
 *	Allocate decap entries for map tunnels.
 */
struct ppe_drv_tun_decap *ppe_drv_tun_decap_map_entries_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;
	struct ppe_drv_tun_decap *decap_map_entries;

	ppe_drv_assert(!p->decap_map_entries, "%p: Decap MAP entries already allocated", p);

	decap_map_entries = vzalloc(sizeof(struct ppe_drv_tun_decap) * PPE_DRV_TUN_DECAP_MAP_MAX_ENTRY);
	if (!decap_map_entries) {
		ppe_drv_warn("%p: failed to allocate ppe_drv_tun_decap entries", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_DECAP_MAP_MAX_ENTRY; index++) {
		decap_map_entries[index].index = index;
	}

	return decap_map_entries;
}
