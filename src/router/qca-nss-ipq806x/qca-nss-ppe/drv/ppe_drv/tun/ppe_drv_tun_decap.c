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
#include <linux/udp.h>
#include <net/gre.h>
#include <net/vxlan.h>
#include <linux/if_tunnel.h>
#include <linux/version.h>
#include <fal_tunnel.h>
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun.h"
#include <fal_vxlan.h>

/*
 * ppe_drv_tun_decap_deconfigure
 *	Deconfigure TL_TBL index allocated for tunnel decap
 */
static bool ppe_drv_tun_decap_deconfigure(struct ppe_drv_tun_decap *ptdc)
{
	fal_tunnel_decap_entry_t ftde = {0};
	sw_error_t err;
	uint16_t tun_hw_idx = ptdc->tl_index;

	ftde.decap_rule.entry_id = tun_hw_idx;
	err = fal_tunnel_decap_entry_del(PPE_DRV_SWITCH_ID, FAL_TUNNEL_OP_MODE_INDEX, &ftde);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to free decap entry at %d", ptdc, tun_hw_idx);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_decap_free
 *	Deconfigure TL_TBL index allocated for tunnel decap
 */
static void ppe_drv_tun_decap_free(struct kref *kref)
{
	struct ppe_drv_tun_decap *ptdc = container_of(kref, struct ppe_drv_tun_decap, ref);

	/*
	 * Below condition can happen if SW entry is created but HW entry allocation for same failed
	 */
	if (ptdc->tl_index == PPE_DRV_TUN_DECAP_INVALID_IDX) {
		return;
	}

	if (ptdc->pgm_prsr) {
		ppe_drv_tun_prgm_prsr_deref(ptdc->pgm_prsr);
	}

	ppe_drv_tun_decap_deconfigure(ptdc);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(6, 1, 0))
	memset(ptdc, 0, sizeof(*ptdc));
#else
        memset(&(ptdc->ppe_drv_tun_decap_group), 0, sizeof(*ptdc));
#endif
}

/*
 * ppe_drv_tun_decap_deref
 *	Taken reference of tunnel encap instance
 */
bool ppe_drv_tun_decap_deref(struct ppe_drv_tun_decap *ptdc)
{
	uint8_t index __maybe_unused = ptdc->index;

	ppe_drv_assert(kref_read(&ptdc->ref), "%p: ref count under run for ptdc", ptdc);

	if (kref_put(&ptdc->ref, ppe_drv_tun_decap_free)) {
		/*
		 * Deconfigure EG_TUN_CTRL entry
		 */
		ppe_drv_trace("reference count is 0 for tun at index: %u", index);
		return true;
	}

	ppe_drv_trace("%p: tun_idx: %u ref dec:%u", ptdc, index, kref_read(&ptdc->ref));
	return false;
}

/*
 * ppe_drv_tun_decap_ref
 *	Taken reference of tunnel encap instance
 */
struct ppe_drv_tun_decap *ppe_drv_tun_decap_ref(struct ppe_drv_tun_decap *ptdc)
{
	kref_get(&ptdc->ref);

	ppe_drv_assert(kref_read(&ptdc->ref), "%p: ref count rollover for ptdc at index:%d", ptdc, ptdc->index);
	ppe_drv_trace("%p: ptdc %u ref inc:%u", ptdc, ptdc->index, kref_read(&ptdc->ref));
	return ptdc;
}

/*
 * ppe_drv_tun_decap_gre_check_n_set
 *	Validate GRE header parameters and fill TL_TBL entry data.
 */
static bool ppe_drv_tun_decap_gre_check_n_set(struct ppe_drv_tun_decap *ptdc,
				struct ppe_drv_tun_cmn_ctx *pth, fal_tunnel_rule_t *decap_entry)
{
	struct ppe_drv_tun_prgm_prsr *pgm = NULL;

	decap_entry->l4_proto = IPPROTO_GRE;

	if (pth->tun.gre.flags & PPE_DRV_TUN_CMN_CTX_GRE_R_KEY) {
		uint32_t gre_key = pth->tun.gre.remote_key;

		if (pth->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV4) {
			decap_entry->tunnel_type = FAL_TUNNEL_TYPE_GRE_TAP_OVER_IPV4;
		} else {
			decap_entry->tunnel_type = FAL_TUNNEL_TYPE_GRE_TAP_OVER_IPV6;
		}

		decap_entry->tunnel_info = htonl(gre_key);
		decap_entry->key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_TLINFO_EN);
		ppe_drv_trace("%p: GRE remote Key: %d", pth, gre_key);
	} else {
		/*
		 * Configure PPE in Programable parser mode for GRETAP without key acceleration.
		 * Lock is accquired for every succesful program parser entry allocated.
		 */
		pgm = ppe_drv_tun_prgm_prsr_entry_alloc(PPE_DRV_TUN_PROGRAM_MODE_GRE);
		if (!pgm) {
			ppe_drv_warn("%p: Error getting programable parser for GRE\n", pth);
			return false;
		}

		ptdc->pgm_prsr = pgm;

		/*
		 * Configure the program parser instance to match gre tunnel without
		 * key. If the program parser instance is already configured then this function
		 * would simply exit.
		 * If the tunnel configuration fails then release the reference taken on the
		 * program parser instance.
		 */
		if (!ppe_drv_tun_prgm_prsr_gre_configure(pgm)) {
			ppe_drv_tun_prgm_prsr_deref(pgm);
			ptdc->pgm_prsr = NULL;
			ppe_drv_warn("%p: GRE tunnel configuration failed\n", pth);
			return false;
		}

		decap_entry->tunnel_type = PPE_DRV_TUN_GET_TUNNEL_TYPE_FROM_PGM_TYPE(pgm->parser_idx);
		ppe_drv_trace("%p: Configure GRE with Tunnel Parser : %d\n", pth, decap_entry->tunnel_type);
	}

	return true;
}

/*
 * ppe_drv_tun_decap_l2tp_check_n_set
 *	Validate L2TP header parameters and fill TL_TBL entry data.
 */
static bool ppe_drv_tun_decap_l2tp_check_n_set(struct ppe_drv_tun_decap *ptdc,
				struct ppe_drv_tun_cmn_ctx *pth, fal_tunnel_rule_t *decap_entry)
{
	struct ppe_drv_tun_prgm_prsr *pgm;

	/*
	 * Configure PPE in Programable parser mode for L2TP acceleration
	 */
	pgm = ppe_drv_tun_prgm_prsr_entry_alloc(PPE_DRV_TUN_PROGRAM_MODE_L2TP_V2);
	if (!pgm) {
		ppe_drv_warn("%p: Error getting programable parser for L2TP tunnel\n", pth);
		return false;
	}

	ptdc->pgm_prsr = pgm;

	if (!ppe_drv_tun_l2tp_prgm_prsr_configure(pgm)) {
		ppe_drv_warn("%p: L2TP tunnel configuration failed\n", pth);
		ppe_drv_tun_prgm_prsr_deref(pgm);
		ptdc->pgm_prsr = NULL;
		return false;
	}

	decap_entry->tunnel_type = PPE_DRV_TUN_GET_TUNNEL_TYPE_FROM_PGM_TYPE(pgm->parser_idx);
	ppe_drv_trace("%p: Configure L2TP with Tunnel Parser : %d\n", pth, decap_entry->tunnel_type);

	decap_entry->l4_proto = IPPROTO_UDP;
	decap_entry->dport = ntohs((uint16_t)pth->tun.l2tp.sport);
	decap_entry->sport = ntohs((uint16_t)pth->tun.l2tp.dport);
	decap_entry->key_bmp |= (PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_DPORT_EN) |PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_SPORT_EN));

	/*
	 * Configure the tunnel id and session id to be matched against udf offsets
	 */
	decap_entry->udf0 = (uint16_t)pth->tun.l2tp.tunnel_id;
	decap_entry->udf1 = (uint16_t)pth->tun.l2tp.session_id;
	decap_entry->key_bmp |= (PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_UDF0_EN) |PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_UDF1_EN));

	ppe_drv_trace("%p: decap_entry: l2_proto = %d, dport = %d, sport = %d, tunnel_id = %d, session_id = %d\n",pth, decap_entry->l4_proto, decap_entry->dport, decap_entry->sport, decap_entry->udf0, decap_entry->udf1);

	return true;
}

/*
 * ppe_drv_tun_decap_vxlan_check_n_set
 *	Validate VxLAN header parameters and fill TL_TBL entry data.
 */
static bool ppe_drv_tun_decap_vxlan_check_n_set(struct ppe_drv_tun_decap *ptdc,
					struct ppe_drv_tun_cmn_ctx *th, fal_tunnel_rule_t *decap_entry)
{
	if (th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV4) {
		decap_entry->tunnel_type = FAL_TUNNEL_TYPE_VXLAN_OVER_IPV4;
	} else {
		decap_entry->tunnel_type = FAL_TUNNEL_TYPE_VXLAN_OVER_IPV6;
	}

	decap_entry->l4_proto = IPPROTO_UDP;
	decap_entry->dport = ntohs(th->tun.vxlan.dest_port);

	decap_entry->tunnel_info = ntohl(th->tun.vxlan.vni);
	decap_entry->key_bmp |= (PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_TLINFO_EN) | PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_DPORT_EN));

	return true;
}

/*
 * ppe_drv_tun_decap_enable
 *	Enable tunnel decapsulation
 */
bool ppe_drv_tun_decap_enable(struct ppe_drv_tun_decap *ptdc)
{
	sw_error_t err;

	err = fal_tunnel_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, A_TRUE);
	if (err != SW_OK) {
		ppe_drv_warn("%p: decap entry %d enable failed", ptdc, ptdc->tl_index);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_decap_disable
 *	Disable tunnel decapsulation
 */
bool ppe_drv_tun_decap_disable(struct ppe_drv_tun_decap *ptdc)
{
	sw_error_t err;

	err = fal_tunnel_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, A_FALSE);
	if (err != SW_OK) {
		ppe_drv_warn("%p: decap entry %d disable failed", ptdc, ptdc->tl_index);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_decap_set_tl_index
 *	Set Hw index associated with decap entry
 */
void ppe_drv_tun_decap_set_tl_index(struct ppe_drv_tun_decap *ptdc, uint32_t hwidx)
{
	ptdc->tl_index = hwidx;
}

/*
 * ppe_drv_tun_decap_set_tl_l3_idx
 *	Set TL L3 IF index mapped to decap entry
 */
void ppe_drv_tun_decap_set_tl_l3_idx(struct ppe_drv_tun_decap *ptdc, uint8_t tl_l3_idx)
{
	ptdc->tl_l3_if_idx = tl_l3_idx;
}

/*
 * ppe_drv_tun_decap_idx_activate
 *	Activate decap index
 */
bool ppe_drv_tun_decap_activate(struct ppe_drv_tun_decap *ptdc, struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr)
{
	/*
	 * Activate TL_TBL entry
	 */
	fal_tunnel_action_t ftde = {0};
	sw_error_t err;

	/*
	 * Update SVLAN Parameters
	 */
	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID) {
		/*
		 * Fill the SVLAN (primary VLAN)
		 */
		ftde.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_SVLAN_UPDATE);
		ftde.verify_entry.verify_bmp |= FAL_TUNNEL_SVLAN_CHECK_EN;
		ftde.verify_entry.svlan_fmt = PPE_DRV_TUN_FIELD_VALID;
		ftde.verify_entry.svlan_id = l2_hdr->vlan[0].tci;

		/*
		 * Fill the CVLAN (Secondary VLAN)
		 */
		ftde.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_CVLAN_UPDATE);
		ftde.verify_entry.verify_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		ftde.verify_entry.cvlan_fmt = PPE_DRV_TUN_FIELD_VALID;
		ftde.verify_entry.cvlan_id = l2_hdr->vlan[1].tci;

		ppe_drv_trace("%p: TL_TBL SVLAN_ID: %d", ptdc, ftde.verify_entry.svlan_id);
		ppe_drv_trace("%p: TL_TBL CVLAN_ID: %d", ptdc, ftde.verify_entry.cvlan_id);
	} else if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID) {
		/*
		 * Fill the CVLAN (Primary VLAN)
		 */
		ftde.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_CVLAN_UPDATE);
		ftde.verify_entry.verify_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		ftde.verify_entry.cvlan_fmt = PPE_DRV_TUN_FIELD_VALID;
		ftde.verify_entry.cvlan_id = l2_hdr->vlan[0].tci;
		ppe_drv_trace("%p: TL_TBL CVLAN_ID: %d", ptdc, ftde.verify_entry.cvlan_id);
	}

	/*
	 * TODO: Update PPPOE related information
	 */

	/*
	 * TL_L3_IF Check parameters
	 */
	ftde.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_L3IF_UPDATE);
	ftde.verify_entry.verify_bmp |= FAL_TUNNEL_L3IF_CHECK_EN;
	ftde.verify_entry.tl_l3_if = ptdc->tl_l3_if_idx;

	/*
	 * Suppose deacce_en bit got set for last rule by HW, before activating
	 * again we need to clear this bit.
	 */
	ftde.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_DEACCE_UPDATE);
	ftde.deacce_en = A_FALSE;

	err = fal_tunnel_decap_action_update(PPE_DRV_SWITCH_ID, ptdc->tl_index, &ftde);
	if (err != SW_OK) {
		ppe_drv_trace("%p: decap entry %d action update failed", ptdc, ptdc->tl_index);
		return false;
	}

	err = fal_tunnel_decap_en_set(PPE_DRV_SWITCH_ID, ptdc->tl_index, (a_bool_t)PPE_DRV_TUN_FIELD_VALID);
	if (err != SW_OK) {
		ppe_drv_trace("%p: decap entry %d enable failed", ptdc, ptdc->tl_index);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_decap_configure
 *	Get an entry index in decap table (TL_TBL)
 */
uint16_t ppe_drv_tun_decap_configure(struct ppe_drv_tun_decap *ptdc, struct ppe_drv_port *pp,
					struct ppe_drv_tun_cmn_ctx *pth)
{
	/*
	 * Add IP header parameters
	 */
	fal_tunnel_decap_entry_t ftde = {0};
	uint8_t vp_num = 0;
	sw_error_t err;

	/*
	 * Check if L3 header is IPV4 type
	 */
	if (pth->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV4) {
		ftde.decap_rule.sip.ip4_addr = ntohl(pth->l3.daddr[0]);
		ftde.decap_rule.dip.ip4_addr = ntohl(pth->l3.saddr[0]);
		ftde.decap_rule.ip_ver = PPE_DRV_TUN_TL_TBL_ENTRY_TYPE_IPV4;
	} else {

		/*
		 * L3 header is IPV6 type
		 */

		ftde.decap_rule.sip.ip6_addr.ul[0] = ntohl(pth->l3.daddr[0]);
		ftde.decap_rule.sip.ip6_addr.ul[1] = ntohl(pth->l3.daddr[1]);
		ftde.decap_rule.sip.ip6_addr.ul[2] = ntohl(pth->l3.daddr[2]);
		ftde.decap_rule.sip.ip6_addr.ul[3] = ntohl(pth->l3.daddr[3]);
		ftde.decap_rule.dip.ip6_addr.ul[0] = ntohl(pth->l3.saddr[0]);
		ftde.decap_rule.dip.ip6_addr.ul[1] = ntohl(pth->l3.saddr[1]);
		ftde.decap_rule.dip.ip6_addr.ul[2] = ntohl(pth->l3.saddr[2]);
		ftde.decap_rule.dip.ip6_addr.ul[3] = ntohl(pth->l3.saddr[3]);

		ftde.decap_rule.ip_ver = PPE_DRV_TUN_TL_TBL_ENTRY_TYPE_IPV6;
	}

	ftde.decap_action.ecn_mode = pth->l3.decap_ecn_mode;

	/*
	 * Check if tunnel type is GRE and do sanity checks
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP) {
		if (!ppe_drv_tun_decap_gre_check_n_set(ptdc, pth, &ftde.decap_rule)) {
			ppe_drv_trace("%p: GRE header validation failed", pp);
			return PPE_DRV_TUN_DECAP_INVALID_IDX;
		}
	} else if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN) {
		/*
		 * Sanity validation for VxLAN tunnels
		 */
		if (!ppe_drv_tun_decap_vxlan_check_n_set(ptdc, pth, &ftde.decap_rule)) {
			ppe_drv_trace("%p: VxLAN header validation failed", pp);
			return PPE_DRV_TUN_DECAP_INVALID_IDX;
		}
	} else if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6) {
		ftde.decap_rule.tunnel_type = FAL_TUNNEL_TYPE_IPV4_OVER_IPV6;
		ftde.decap_rule.l4_proto = IPPROTO_IPIP;
	} else if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2) {
		if (!ppe_drv_tun_decap_l2tp_check_n_set(ptdc, pth, &ftde.decap_rule)) {
			ppe_drv_trace("%p: GRE header validation failed", pp);
			return PPE_DRV_TUN_DECAP_INVALID_IDX;
		}
		ftde.decap_action.udp_csum_zero = A_TRUE;
		ftde.decap_action.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_UDP_CSUM_ZERO_UPDATE);
	} else {
		/*
		 * MAPT cases are not expected to use these tables only other
		 * tunnel supported is DS-Lite
		 */
		ppe_drv_warn("%p: Invalid tunnel type: %d", pp, pth->type);
		return PPE_DRV_TUN_DECAP_INVALID_IDX;
	}

	/*
	 * Set source interface.
	 */
	vp_num = ppe_drv_port_num_get(pp);
	ftde.decap_action.src_info_enable = A_TRUE;
	ftde.decap_action.src_info_type = PPE_DRV_TUN_TL_TBL_SRC_INFO_TYPE_VP;
	ftde.decap_action.src_info = vp_num;
	ftde.decap_action.verify_entry.verify_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_L3IF_CHECK_EN);
	ftde.decap_rule.key_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_SIP_EN) |
					PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_DIP_EN) |
					PPE_DRV_TUN_BIT(FAL_TUNNEL_KEY_L4PROTO_EN);

	/*
	 * Allow decapsulated GRETAP tunnel exception service code.
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP) {
		ftde.decap_action.service_code_en = A_TRUE;
		ftde.decap_action.service_code = PPE_DRV_SC_L2_TUNNEL_EXCEPTION;
	}

	/*
	 * Allow UDP checksum zero packets.
	 * VXLAN IPV4 will always allow the UDP checksum zero packets but,
	 * VXLAN IPV6 will allow UDP checksum zero packets only if PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM6_RX is set.
	 */
	if (pth->type == PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN) {
		ftde.decap_action.service_code_en = A_TRUE;
		ftde.decap_action.service_code = PPE_DRV_SC_L2_TUNNEL_EXCEPTION;
		ftde.decap_action.update_bmp |= PPE_DRV_TUN_BIT(FAL_TUNNEL_UDP_CSUM_ZERO_UPDATE);
		if (pth->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV4) {
			ftde.decap_action.udp_csum_zero = A_TRUE;
		} else if ((pth->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV6) && (pth->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM6_RX)) {
			ftde.decap_action.udp_csum_zero = A_TRUE;
		}
	}

	err = fal_tunnel_decap_entry_add(PPE_DRV_SWITCH_ID, FAL_TUNNEL_OP_MODE_HASH, &ftde);
	if (err != SW_OK) {
		ppe_drv_warn("%p: unable to allocate decap entry", pp);
		return PPE_DRV_TUN_DECAP_INVALID_IDX;
	}

	return (int16_t)ftde.decap_rule.entry_id;
}

/*
 * ppe_drv_tun_decap_alloc
 *	Allocate PPE tunnel and return.
 */
struct ppe_drv_tun_decap *ppe_drv_tun_decap_alloc(struct ppe_drv *p)
{
	uint8_t tun_idx;

	/*
	 * Return first free instance
	 */
	for (tun_idx = 0; tun_idx < PPE_DRV_TUN_DECAP_MAX_ENTRY; tun_idx++) {
		struct ppe_drv_tun_decap *ptdc = &p->ptun_dc[tun_idx];

		if (kref_read(&ptdc->ref)) {
			continue;
		}

		ptdc->index = tun_idx;
		kref_init(&ptdc->ref);
		ppe_drv_trace("%p: Free tunnel decap instance found, index: %d", ptdc, tun_idx);
		return ptdc;
	}

	ppe_drv_warn("%p: Free tunnel encap instance is not found", p);
	return NULL;
}

/*
 * ppe_drv_tun_decap_entries_free
 *	Free memory allocate for tunnel context entries.
 */
void ppe_drv_tun_decap_entries_free(struct ppe_drv_tun_decap *ptun_dc)
{
	vfree(ptun_dc);
}

/*
 * ppe_drv_tun_entries_alloc
 *	Allocate and initialize tunnel context entries.
 */
struct ppe_drv_tun_decap *ppe_drv_tun_decap_entries_alloc(struct ppe_drv *p)
{
	uint16_t index = 0;
	struct ppe_drv_tun_decap *ptun_dc;

	ppe_drv_assert(!p->ptun_dc, "%p: tunnel decap entries already allocated", p);

	ptun_dc = vzalloc(sizeof(struct ppe_drv_tun_decap) * PPE_DRV_TUN_DECAP_MAX_ENTRY);
	if (!ptun_dc) {
		ppe_drv_warn("%p: failed to allocate decap entries", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_DECAP_MAX_ENTRY; index++) {
		ptun_dc[index].index = index;
	}

	return ptun_dc;
}
