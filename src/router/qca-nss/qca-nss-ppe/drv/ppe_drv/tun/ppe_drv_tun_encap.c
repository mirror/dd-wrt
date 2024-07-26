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
#include <fal_tunnel.h>
#include <ppe_drv/ppe_drv.h>
#include "ppe_drv_tun.h"

/*
 * ppe_drv_tun_encap_hdr_ctrl_entry_free
 *	 clear header ctrl protomap instance
 */
void ppe_drv_tun_encap_hdr_ctrl_entry_free(struct kref *kref)
{
	sw_error_t err;
	fal_tunnel_encap_header_ctrl_t header_ctrl = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl = p->ecap_hdr_ctrl;

	err = fal_tunnel_encap_header_ctrl_get(PPE_DRV_SWITCH_ID, &header_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Unable to get header control configuration: %d", p, err);
		return;
	}

	/*
	 * Reset data to zero if ref count is zero. This will ensure the value is cleared while updating
	 * header ctrl data in "ppe_drv_tun_encap_hdr_ctrl_reset" function
	 */
	hdr_ctrl->udp_sport_base = (!kref_read(&hdr_ctrl->udp_sport_base_ref)) ? 0 : header_ctrl.udp_sport_base;
	hdr_ctrl->udp_sport_mask = (!kref_read(&hdr_ctrl->udp_sport_mask_ref)) ? 0 : header_ctrl.udp_sport_mask;
	hdr_ctrl->ipv4_addr_map_data = (!kref_read(&hdr_ctrl->ipv4_addr_map_ref)) ? 0 : header_ctrl.proto_map_data[0];
	hdr_ctrl->ipv4_proto_map_data = (!kref_read(&hdr_ctrl->ipv4_proto_map_ref)) ? 0 : header_ctrl.proto_map_data[1];
	hdr_ctrl->ipv6_addr_map_data = (!kref_read(&hdr_ctrl->ipv6_addr_map_ref)) ? 0 : header_ctrl.proto_map_data[2];
	hdr_ctrl->ipv6_proto_map_data = (!kref_read(&hdr_ctrl->ipv6_proto_map_ref)) ? 0 : header_ctrl.proto_map_data[3];

}

/*
 * ppe_drv_tun_encap_hdr_ctrl_ref
 *	free reference for header ctrl protomap instance
 */
bool ppe_drv_tun_encap_hdr_ctrl_deref(struct kref *kref)
{
	struct ppe_drv *p __maybe_unused = &ppe_drv_gbl;
	ppe_drv_assert(kref_read(kref), "%p: ref count under run for encap header ctrl", p);

	if (kref_put(kref, ppe_drv_tun_encap_hdr_ctrl_entry_free)) {
		ppe_drv_trace("%p: reference count is 0 for header ctrl ", p);
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_ref
 *	Get reference for header ctrl protomap instance
 */
bool ppe_drv_tun_encap_hdr_ctrl_ref(struct kref *kref)
{
	kref_get(kref);
	ppe_drv_assert(kref_read(kref), "%p: ref count rollover for encap header ctrl", &ppe_drv_gbl);

	return true;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_reset
 *	Reset global encap header control register value
 */
bool ppe_drv_tun_encap_hdr_ctrl_reset(uint8_t flags)
{
	fal_tunnel_encap_header_ctrl_t header_ctrl = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl_orig_cfg = p->ecap_hdr_ctrl;
	uint8_t hdr_ctrl_flag = 0;
	sw_error_t err;

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_ID_SEED;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		header_ctrl.ipv4_id_seed = 0;
	}

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_DF_SET;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		header_ctrl.ipv4_df_set = 0;
	}

	/*
	 * udp source port configurations and proto map configurations are dereferenced when reset
	 * request is received. It will be reset to zero once all the references are released
	 *
	 */
	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_BASE;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->udp_sport_base_ref);
		header_ctrl.udp_sport_base = hdr_ctrl_orig_cfg->udp_sport_base;
	}

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_MASK;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->udp_sport_mask_ref);
		header_ctrl.udp_sport_mask = hdr_ctrl_orig_cfg->udp_sport_mask;
	}

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_ADR_MAP;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv4_addr_map_ref);
		header_ctrl.proto_map_data[0] = hdr_ctrl_orig_cfg->ipv4_addr_map_data;
	}

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_PROTO_MAP;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv4_proto_map_ref);
		header_ctrl.proto_map_data[1] = hdr_ctrl_orig_cfg->ipv4_proto_map_data;
	}

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_ADR_MAP;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv6_addr_map_ref);
		header_ctrl.proto_map_data[2] = hdr_ctrl_orig_cfg->ipv6_addr_map_data;
	}

	hdr_ctrl_flag = PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_PROTO_MAP;
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(flags, hdr_ctrl_flag)) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv6_proto_map_ref);
		header_ctrl.proto_map_data[3] = hdr_ctrl_orig_cfg->ipv6_proto_map_data;
	}

	err = fal_tunnel_encap_header_ctrl_set(PPE_DRV_SWITCH_ID, &header_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure encap header control err: %d", p, err);
		return false;
	}
	return true;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_set
 *	set global encap header control register value
 */
bool ppe_drv_tun_encap_hdr_ctrl_set(struct ppe_drv_tun_encap_header_ctrl hdr_ctrl)
{
	fal_tunnel_encap_header_ctrl_t header_ctrl = {0};
	struct ppe_drv *p = &ppe_drv_gbl;
	struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl_orig_cfg = p->ecap_hdr_ctrl;
	bool ipv4_addr_ref = false, ipv4_proto_ref = false,  ipv6_addr_ref = false, ipv6_proto_ref = false;
	bool udp_sport_base_ref = false, udp_sport_mask_ref = false;
	sw_error_t err;

	err = fal_tunnel_encap_header_ctrl_get(PPE_DRV_SWITCH_ID, &header_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Unable to get header control configuration: %d", p, err);
		return false;
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_ID_SEED)) {
		header_ctrl.ipv4_id_seed = hdr_ctrl.ipv4_id_seed;
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_DF_SET)) {
		header_ctrl.ipv4_df_set = hdr_ctrl.ipv4_df_set;
	}

	/*
	 * If udp source port data or protomap data are to be configured for the first time then the value is updated
	 * and a reference is taken. For subsequent set references are incremented if the value matches the value configured
	 * already. This ensures the header control feilds are used for a single tunnel type.
	 * If the already configured values doesnt match with the value to be set the function returns error.
	 */
	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_BASE)) {
		if (header_ctrl.udp_sport_base == hdr_ctrl.udp_sport_base) {
			ppe_drv_tun_encap_hdr_ctrl_ref(&hdr_ctrl_orig_cfg->udp_sport_base_ref);
			udp_sport_base_ref = true;
		} else if (!kref_read(&hdr_ctrl_orig_cfg->udp_sport_base_ref)) {
			header_ctrl.udp_sport_base = hdr_ctrl.udp_sport_base;
			kref_init(&hdr_ctrl_orig_cfg->udp_sport_base_ref);
			udp_sport_base_ref = true;
			hdr_ctrl_orig_cfg->udp_sport_base = hdr_ctrl.udp_sport_base;
		} else {
			ppe_drv_trace("%p: header control udp sport base already configured", p);
			goto err_false;
		}
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_MASK)) {
		if (header_ctrl.udp_sport_mask == hdr_ctrl.udp_sport_mask) {
			ppe_drv_tun_encap_hdr_ctrl_ref(&hdr_ctrl_orig_cfg->udp_sport_mask_ref);
			udp_sport_mask_ref = true;
		} else if (!kref_read(&hdr_ctrl_orig_cfg->udp_sport_mask_ref)) {
			header_ctrl.udp_sport_mask = hdr_ctrl.udp_sport_mask;
			kref_init(&hdr_ctrl_orig_cfg->udp_sport_mask_ref);
			udp_sport_mask_ref = true;
			hdr_ctrl_orig_cfg->udp_sport_mask = hdr_ctrl.udp_sport_mask;
		} else {
			ppe_drv_trace("%p: header control udp sport mask already configured", p);
			goto err_false;
		}
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_ADR_MAP)) {
		if (header_ctrl.proto_map_data[0] == hdr_ctrl.ipv4_addr_map_data) {
			ppe_drv_tun_encap_hdr_ctrl_ref(&hdr_ctrl_orig_cfg->ipv4_addr_map_ref);
			ipv4_addr_ref = true;
		} else if (!kref_read(&hdr_ctrl_orig_cfg->ipv4_addr_map_ref)) {
			header_ctrl.proto_map_data[0] = hdr_ctrl.ipv4_addr_map_data;
			kref_init(&hdr_ctrl_orig_cfg->ipv4_addr_map_ref);
			ipv4_addr_ref = true;
			hdr_ctrl_orig_cfg->ipv4_addr_map_data = hdr_ctrl.ipv4_addr_map_data;
		} else {
			ppe_drv_trace("%p: header control ipv4 addr map data already configured", p);
			goto err_false;
		}
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_PROTO_MAP)) {
		if (header_ctrl.proto_map_data[1] == hdr_ctrl.ipv4_proto_map_data) {
			ppe_drv_tun_encap_hdr_ctrl_ref(&hdr_ctrl_orig_cfg->ipv4_proto_map_ref);
			ipv4_proto_ref = true;
		} else if (!kref_read(&hdr_ctrl_orig_cfg->ipv4_proto_map_ref)) {
			header_ctrl.proto_map_data[1] = hdr_ctrl.ipv4_proto_map_data;
			kref_init(&hdr_ctrl_orig_cfg->ipv4_proto_map_ref);
			ipv4_proto_ref = true;
			hdr_ctrl_orig_cfg->ipv4_proto_map_data = hdr_ctrl.ipv4_proto_map_data;
		} else {
			ppe_drv_warn("%p: header control ipv4 proto map data already configured", p);
			goto err_false;
		}
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_ADR_MAP)) {
		if (header_ctrl.proto_map_data[2] == hdr_ctrl.ipv6_addr_map_data) {
			ppe_drv_tun_encap_hdr_ctrl_ref(&hdr_ctrl_orig_cfg->ipv6_addr_map_ref);
			ipv6_addr_ref = true;
		} else if (!kref_read(&hdr_ctrl_orig_cfg->ipv6_addr_map_ref)) {
			header_ctrl.proto_map_data[2] = hdr_ctrl.ipv6_addr_map_data;
			kref_init(&hdr_ctrl_orig_cfg->ipv6_addr_map_ref);
			ipv6_addr_ref = true;
			hdr_ctrl_orig_cfg->ipv6_addr_map_data = hdr_ctrl.ipv6_addr_map_data;
		} else {
			ppe_drv_trace("%p: header control ipv6 addr map data already configured", p);
			goto err_false;
		}
	}

	if (ppe_drv_tun_encap_hdr_ctrl_flag_check(hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_PROTO_MAP)) {
		if (header_ctrl.proto_map_data[3] == hdr_ctrl.ipv6_proto_map_data) {
			ppe_drv_tun_encap_hdr_ctrl_ref(&hdr_ctrl_orig_cfg->ipv6_proto_map_ref);
			ipv6_proto_ref = true;
		} else if (!kref_read(&hdr_ctrl_orig_cfg->ipv6_proto_map_ref)) {
			header_ctrl.proto_map_data[3] = hdr_ctrl.ipv6_proto_map_data;
			kref_init(&hdr_ctrl_orig_cfg->ipv6_proto_map_ref);
			ipv6_proto_ref = true;
			hdr_ctrl_orig_cfg->ipv6_proto_map_data = hdr_ctrl.ipv6_proto_map_data;
		} else {
			ppe_drv_trace("%p: header control ipv6 proto map data already configured", p);
			goto err_false;
		}
	}

	err = fal_tunnel_encap_header_ctrl_set(PPE_DRV_SWITCH_ID, &header_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure encap header control err: %d", p, err);
		return false;
	}

	return true;

err_false:
	if (ipv4_addr_ref) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv4_addr_map_ref);
	}

	if (ipv4_proto_ref) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv4_proto_map_ref);
	}

	if (ipv6_addr_ref) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv6_addr_map_ref);
	}

	if (ipv6_proto_ref) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->ipv6_proto_map_ref);
	}

	if (udp_sport_base_ref) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->udp_sport_base_ref);
	}

	if (udp_sport_mask_ref) {
		ppe_drv_tun_encap_hdr_ctrl_deref(&hdr_ctrl_orig_cfg->udp_sport_mask_ref);
	}

	return false;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_proto_map_configured
 *	check if proto map data settings are configured in encap header control
 */
static bool ppe_drv_tun_encap_hdr_ctrl_proto_map_configured(struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl)
{

	if (kref_read(&hdr_ctrl->ipv4_addr_map_ref) || kref_read(&hdr_ctrl->ipv4_proto_map_ref) ||
			kref_read(&hdr_ctrl->ipv6_addr_map_ref) ||
			kref_read(&hdr_ctrl->ipv6_proto_map_ref)) {
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_encap_header_ctrl_udp_sport_enabled
 *	check if udp sport settings are configured in encap header control
 */
static bool ppe_drv_tun_encap_header_ctrl_udp_sport_enabled(struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl)
{
	if (kref_read(&hdr_ctrl->udp_sport_base_ref) || kref_read(&hdr_ctrl->udp_sport_mask_ref) ) {
		return true;
	}

	return false;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_free
 *	free encap header control entry
 */
void ppe_drv_tun_encap_hdr_ctrl_free(struct ppe_drv_tun_encap_hdr_ctrl *hdr_ctrl)
{
	kfree(hdr_ctrl);
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_init
 *	initialize encap header control register
 */
bool ppe_drv_tun_encap_hdr_ctrl_init(struct ppe_drv *p)
{
	fal_tunnel_encap_header_ctrl_t header_ctrl = {0};
	sw_error_t err;

	p->ecap_hdr_ctrl = kzalloc(sizeof(struct ppe_drv_tun_encap_hdr_ctrl), GFP_ATOMIC);
	if (!p->ecap_hdr_ctrl) {
		ppe_drv_warn("%p: failed to allocate encap header control entry", p);
		return NULL;
	}

	err = fal_tunnel_encap_header_ctrl_set(PPE_DRV_SWITCH_ID, &header_ctrl);
	if (err != SW_OK) {
		ppe_drv_warn("%p: failed to configure encap header control err: %d", p, err);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_vxlan_configure
 *	configure encap header control for vxlan tunnel
 */
bool ppe_drv_tun_encap_hdr_ctrl_vxlan_configure(struct ppe_drv *p, struct ppe_drv_tun *tun)
{
	struct ppe_drv_tun_encap_header_ctrl hdr_ctrl = {0};

	/*
	 * check if the header control is configured already and  used by other tunnels
	 * for protomap  configuration. If its already configured exit
	 */
	if (ppe_drv_tun_encap_hdr_ctrl_proto_map_configured(p->ecap_hdr_ctrl)) {
		return false;
	}

	hdr_ctrl.udp_sport_base = FAL_TUNNEL_UDP_ENTROPY_SPORT_BASE;
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_BASE);
	hdr_ctrl.udp_sport_mask = FAL_TUNNEL_UDP_ENTROPY_SPORT_MASK;
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_MASK);

	if (!ppe_drv_tun_encap_hdr_ctrl_set(hdr_ctrl)) {
		ppe_drv_warn("%p encap header control set failed", p);
		return false;
	}

	/*
	 * Set the encap header control bitmap in tunnel structure
	 */
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&tun->encap_hdr_bitmap, PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_BASE);
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&tun->encap_hdr_bitmap, PPE_DRV_TUN_ENCAP_HDR_CTRL_UDP_SPORT_MASK);

	return true;
}

/*
 * ppe_drv_tun_encap_hdr_ctrl_l2tp_configure
 *	configure encap header control for l2tp tunnel
 */
bool ppe_drv_tun_encap_hdr_ctrl_l2tp_configure(struct ppe_drv *p, struct ppe_drv_tun *tun)
{
	struct ppe_drv_tun_encap_header_ctrl hdr_ctrl = {0};

	/*
	 * Check if any tunnel is offloaded with udp source port update configuration set.
	 * If so l2tp tunnel should not be offloaded to PPE as the source port gets overwritten
	 */
	if (ppe_drv_tun_encap_header_ctrl_udp_sport_enabled(p->ecap_hdr_ctrl)) {
		return false;
	}

	/*
	 * Configure header control global register to update PPP protocol
	 * for IPv4 packet proto_map_data[1] is used and for IPv6 proto_map_data[3] is used
	 */
	hdr_ctrl.ipv4_proto_map_data = PPP_IP;
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_PROTO_MAP);
	hdr_ctrl.ipv6_proto_map_data = PPP_IPV6;
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&hdr_ctrl.flags, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_PROTO_MAP);

	if (!ppe_drv_tun_encap_hdr_ctrl_set(hdr_ctrl)) {
		ppe_drv_warn("%p encap header control set failed", p);
		return false;
	}

	/*
	 * Set encap header control bitmap in tunnel structure.
	 */
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&tun->encap_hdr_bitmap, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV4_PROTO_MAP);
	ppe_drv_tun_encap_hdr_ctrl_flag_set(&tun->encap_hdr_bitmap, PPE_DRV_TUN_ENCAP_HDR_CTRL_IPV6_PROTO_MAP);

	return true;
}

/*
 * ppe_drv_tun_encap_dump
 *	Dump contents of EG_TUN_CTRL table instance
 */
static void ppe_drv_tun_encap_dump(struct ppe_drv_tun_encap *ptec)
{
	sw_error_t err;
	fal_tunnel_encap_cfg_t encap_cfg;

	memset((void *)&encap_cfg, 0, sizeof(fal_tunnel_encap_cfg_t));

	err = fal_tunnel_encap_entry_get(PPE_DRV_SWITCH_ID, ptec->tun_idx, &encap_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p: Failed to dump encap entry at %u", ptec, ptec->tun_idx);
		return;
	}

	ppe_drv_trace("%p: EG_TUN_CTRL_IDX : [%d], TYPE[%d], RULE_ID[%d], RULE_TARGET[%d], DATA_LEN[%d], VLAN_OFFSET[%d], L3_OFFSET[%d]",
			ptec, ptec->tun_idx,
			encap_cfg.encap_type,
			encap_cfg.edit_rule_id,
			encap_cfg.encap_target,
			encap_cfg.tunnel_len,
			encap_cfg.vlan_offset,
			encap_cfg.l3_offset);

	ppe_drv_trace("%p: DF_MODE[%d], PPPOE_EN[%d], IP_VER[%d], DSCP_MODE[%d]",
			ptec,
			encap_cfg.ipv4_df_mode_ext,
			encap_cfg.pppoe_en,
			encap_cfg.ip_ver,
			encap_cfg.dscp_mode);

	ppe_drv_trace("%p: L4_OFFSET[%d], TUNNEL_OFFSET[%d], STAG_FMT[%d], CTAG_FMT[%d], L4_TYPE[%d]",
			ptec,
			encap_cfg.l4_offset,
			encap_cfg.tunnel_offset,
			encap_cfg.svlan_fmt,
			encap_cfg.cvlan_fmt,
			encap_cfg.l4_proto);

	ppe_drv_trace("%p: SPCP_MODE[%d], SDEI_MODE[%d], CPCP_MODE[%d], CDEI_MODE[%d]",
			ptec,
			encap_cfg.spcp_mode,
			encap_cfg.sdei_mode,
			encap_cfg.cpcp_mode,
			encap_cfg.cdei_mode);
}

/*
 * ppe_drv_tun_encap_free
 *	Deconfigure EG_XLAT_TUN_CTRL table and free entry
 */
void ppe_drv_tun_encap_free(struct kref *kref)
{
	sw_error_t err;
	struct ppe_drv_tun_encap *ptec = container_of(kref, struct ppe_drv_tun_encap, ref);

	err = fal_tunnel_encap_entry_del(PPE_DRV_SWITCH_ID, ptec->tun_idx);
	if (err != SW_OK) {
		ppe_drv_warn("%p: encap entry delete failed for tunnel index %u", ptec, ptec->tun_idx);
		return;
	}
	memset(ptec, 0, sizeof(*ptec));
}

/*
 * ppe_drv_tun_encap_deref
 *	Taken reference of tunnel encap instance
 */
bool ppe_drv_tun_encap_deref(struct ppe_drv_tun_encap *ptec)
{
	uint8_t tun_idx __maybe_unused = ptec->tun_idx;

	ppe_drv_assert(kref_read(&ptec->ref), "%p: ref count under run for ptec", ptec);

	if (kref_put(&ptec->ref, ppe_drv_tun_encap_free)) {
		/*
		 * Deconfigure EG_TUN_CTRL entry
		 */
		ppe_drv_trace("%p: reference count is 0 at index: %u", ptec, tun_idx);
		return true;
	}

	ppe_drv_trace("%p: tun_idx: %u ref dec:%u", ptec, tun_idx, kref_read(&ptec->ref));
	return false;
}

/*
 * ppe_drv_tun_encap_ref
 *	Taken reference of tunnel encap instance
 */
struct ppe_drv_tun_encap *ppe_drv_tun_encap_ref(struct ppe_drv_tun_encap *ptec)
{
	kref_get(&ptec->ref);

	ppe_drv_assert(kref_read(&ptec->ref), "%p: ref count rollover for ptec at index:%d", ptec, ptec->tun_idx);
	ppe_drv_trace("%p: ptec %u ref inc:%u", ptec, ptec->tun_idx, kref_read(&ptec->ref));
	return ptec;
}

/*
 * ppe_drv_tun_encap_set_rule_id
 *	Set EG EDIT rule id in eg_tun_ctrl instance
 */
void ppe_drv_tun_encap_set_rule_id(struct ppe_drv_tun_encap *ptec, uint8_t rule_id)
{
	ptec->rule_id = rule_id;
}

/*
 * ppe_drv_tun_encap_hdr_set
 *	Configure EG header data given tunnel header
 */
static void ppe_drv_tun_encap_hdr_set(struct ppe_drv_tun_encap *ptec,
					struct ppe_drv_tun_cmn_ctx *th,
					struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr)
{
	/*
	 * Create EG header buffer as seen on wire
	 *	<DMAC><SMAC<VLAN_HDR><TYPE><IP_HDR>{<GRE_HDR>, <UDP_HDR><VxLAN_HDR>}
	 */
	struct vlan_ethhdr *pri_vlan;
	bool l4_offset_valid = false;
	struct vlan_hdr *sec_vlan;
	uint8_t l3_offset = 0;
	uint8_t l4_offset = 0;
	uint8_t tun_len = 0;
	struct ethhdr *eth;
	uint8_t *tun_hdr;

	tun_hdr = (uint8_t *)&ptec->hdr[0];
	memset((void *)tun_hdr, 0, sizeof(ptec->hdr));

	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID) {
		/*
		 * Fill the SVLAN (primary VLAN)
		 */
		pri_vlan = (struct vlan_ethhdr *)tun_hdr;
		memcpy((void *)pri_vlan->h_dest, (void *)&l2_hdr->dmac, ETH_ALEN);
		memcpy((void *)pri_vlan->h_source, (void *)&l2_hdr->smac, ETH_ALEN);
		pri_vlan->h_vlan_proto = htons(l2_hdr->vlan[0].tpid);
		pri_vlan->h_vlan_TCI = htons(l2_hdr->vlan[0].tci);
		pri_vlan->h_vlan_encapsulated_proto = htons(l2_hdr->vlan[1].tpid);
		tun_hdr += sizeof(*pri_vlan);
		tun_len += sizeof(*pri_vlan);

		/*
		 * Fill the CVLAN (Secondary VLAN)
		 */
		sec_vlan = (struct vlan_hdr *)tun_hdr;
		sec_vlan->h_vlan_TCI = htons(l2_hdr->vlan[1].tci);
		sec_vlan->h_vlan_encapsulated_proto = htons(l2_hdr->eth_type);
		tun_hdr += sizeof(*sec_vlan);
		tun_len += sizeof(*sec_vlan);
	} else if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID) {
		/*
		 * fill the CVLAN (primary VLAN)
		 */
		pri_vlan = (struct vlan_ethhdr *)tun_hdr;
		memcpy((void *)pri_vlan->h_dest, (void *)&l2_hdr->dmac, ETH_ALEN);
		memcpy((void *)pri_vlan->h_source, (void *)&l2_hdr->smac, ETH_ALEN);
		pri_vlan->h_vlan_proto = htons(l2_hdr->vlan[0].tpid);
		pri_vlan->h_vlan_TCI = htons(l2_hdr->vlan[0].tci);
		pri_vlan->h_vlan_encapsulated_proto = htons(l2_hdr->eth_type);
		tun_hdr += sizeof(*pri_vlan);
		tun_len += sizeof(*pri_vlan);
	} else {
		/*
		 * Copy MAC header
		 */
		eth = (struct ethhdr *)tun_hdr;
		memcpy((void *)eth->h_dest, (void *)&l2_hdr->dmac, ETH_ALEN);
		memcpy((void *)eth->h_source, (void *)&l2_hdr->smac, ETH_ALEN);
		eth->h_proto = htons(l2_hdr->eth_type);
		tun_len += ETH_HLEN;
		tun_hdr += ETH_HLEN;
	}

	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID) {
		memcpy((void *)tun_hdr, (void *)&l2_hdr->pppoe.ph, sizeof(l2_hdr->pppoe.ph));
		tun_hdr += sizeof(l2_hdr->pppoe.ph);
		memcpy((void *)tun_hdr, (void *)&l2_hdr->pppoe.ppp_proto, sizeof(l2_hdr->pppoe.ppp_proto));
		tun_len += PPPOE_SES_HLEN;
		tun_hdr += sizeof(l2_hdr->pppoe.ppp_proto);
	}

	l3_offset = tun_len;

	/*
	 * Copy L3 header
	 */
	if (th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV4) {
		struct iphdr iph;

		memset((void *)&iph, 0, sizeof(iph));
		iph.version = 0x4;
		iph.ihl = 0x5;
		iph.id = 0;
		iph.saddr = th->l3.saddr[0];
		iph.daddr = th->l3.daddr[0];
		iph.protocol = th->l3.proto;

		if (!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL)) {
			iph.ttl = th->l3.ttl;
		}

		if (!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP)) {
			iph.tos = (th->l3.dscp << 2);
		}


		memcpy((void *)tun_hdr, (void *)&iph, sizeof(iph));
		tun_hdr += sizeof(iph);
		tun_len += sizeof(iph);
		l4_offset = l3_offset + sizeof(iph);
	} else {
		struct ipv6hdr ip6h = {0};
		uint8_t tclass = 0;

		memset((void *)&ip6h, 0, sizeof(ip6h));

		memcpy((void *)&ip6h.saddr, (void *)&th->l3.saddr, sizeof(struct in6_addr));
		memcpy((void *)&ip6h.daddr, (void *)&th->l3.daddr, sizeof(struct in6_addr));
		ip6_flow_hdr(&ip6h, 0, 0);
		ip6h.nexthdr = (th->l3.proto);

		if (!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL)) {
			ip6h.hop_limit = th->l3.ttl;
		}

		if (!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP)) {
			tclass = th->l3.dscp;
		}

		ip6_flow_hdr(&ip6h, tclass, 0);

		ip6h.payload_len = htons(sizeof(struct ipv6hdr));
		memcpy((void *)tun_hdr, (void *)&ip6h, sizeof(struct ipv6hdr));

		tun_hdr += sizeof(struct ipv6hdr);
		tun_len += sizeof(struct ipv6hdr);
		l4_offset = l3_offset + sizeof(struct ipv6hdr);
	}

	if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP) {
		struct gre_base_hdr greh;
		uint16_t gre_hdr_flags = 0;
		uint32_t gre_key = 0;

		memset((void *)&greh, 0, sizeof(greh));

		if (th->tun.gre.flags & PPE_DRV_TUN_CMN_CTX_GRE_L_KEY) {
			gre_hdr_flags |= GRE_KEY;
			gre_key = th->tun.gre.local_key;

			ppe_drv_trace("%p: GRE Local key: %d", ptec, gre_key);
		}

		greh.protocol = htons(ETH_P_TEB);
		greh.flags = gre_hdr_flags;

		memcpy((void *)tun_hdr, (void *)&greh, sizeof(greh));
		tun_hdr += sizeof(greh);
		tun_len += sizeof(greh);
		if (gre_key) {
			memcpy((void *)tun_hdr, (void *)&gre_key, sizeof(gre_key));
			tun_hdr += sizeof(gre_key);
			tun_len += sizeof(gre_key);
		}
		l4_offset_valid = true;
	} else if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN) {
		struct udphdr udph;
		struct vxlanhdr vxh;
		struct vxlanhdr_gbp vxh_gbp;

		memset(&udph, 0, sizeof(struct udphdr));
		memset(&vxh, 0, sizeof(struct vxlanhdr));
		memset(&vxh_gbp, 0, sizeof(struct vxlanhdr_gbp));

		udph.dest = th->tun.vxlan.dest_port;
		memcpy((void *)tun_hdr, (void *)&udph, sizeof(udph));
		tun_hdr += sizeof(udph);
		tun_len += sizeof(udph);

		vxh.vx_flags = th->tun.vxlan.flags;
		vxh.vx_vni = th->tun.vxlan.vni;

		if (th->tun.vxlan.flags & VXLAN_F_GBP) {
			vxh_gbp.policy_id = th->tun.vxlan.policy_id;
			vxh_gbp.vx_flags = th->tun.vxlan.flags;
			vxh_gbp.vx_vni = th->tun.vxlan.vni;
			vxh_gbp.policy_id = th->tun.vxlan.policy_id;
			memcpy((void *)tun_hdr, (void *)&vxh_gbp, sizeof(vxh_gbp));
			tun_hdr += sizeof(vxh_gbp);
			tun_len += sizeof(vxh_gbp);
		}

		memcpy((void *)tun_hdr, (void *)&vxh, sizeof(vxh));
		tun_hdr += sizeof(vxh);
		tun_len += sizeof(vxh);
		l4_offset_valid = true;
	} else if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2) {
		struct udphdr udph;
		struct ppe_drv_tun_encap_l2tp_hdr l2tphdr;
		struct ppe_drv_tun_encap_ppp_hdr ppphdr;

		memset(&udph, 0, sizeof(struct udphdr));
		memset(&l2tphdr, 0, sizeof(struct ppe_drv_tun_encap_l2tp_hdr));
		memset(&ppphdr, 0, sizeof(struct ppe_drv_tun_encap_ppp_hdr));

		udph.source = th->tun.l2tp.sport;
		udph.dest = th->tun.l2tp.dport;
		memcpy((void *)tun_hdr, (void *)&udph, sizeof(udph));
		tun_hdr += sizeof(udph);
		tun_len += sizeof(udph);

		l2tphdr.flags = htons(PPE_DRV_TUN_L2TP_V2_PACKET_TYPE);
		l2tphdr.tunnel_id = htons(th->tun.l2tp.peer_tunnel_id);
		l2tphdr.session_id = htons(th->tun.l2tp.peer_session_id);
		memcpy((void *)tun_hdr, (void *)&l2tphdr, sizeof(struct ppe_drv_tun_encap_l2tp_hdr));
		tun_hdr += sizeof(struct ppe_drv_tun_encap_l2tp_hdr);
		tun_len += sizeof(struct ppe_drv_tun_encap_l2tp_hdr);

		ppphdr.address = PPE_DRV_TUN_L2TP_PPP_ADDRESS;
		ppphdr.control = PPE_DRV_TUN_L2TP_PPP_CONTROL;
		memcpy((void *)tun_hdr, (void *)&ppphdr, sizeof(struct ppe_drv_tun_encap_ppp_hdr));

		tun_hdr += sizeof(struct ppe_drv_tun_encap_ppp_hdr);
		tun_len += sizeof(struct ppe_drv_tun_encap_ppp_hdr);
		l4_offset_valid = true;
	}

	ptec->tun_len = tun_len;
	ptec->l3_offset = l3_offset;
	ptec->l4_offset = l4_offset;
	ptec->l4_offset_valid = l4_offset_valid;

	ppe_drv_trace("%p: PPE_DRV_EG_HEADER_DATA_TBL Entry tun_len: %d", ptec, tun_len);
}

/*
 * ppe_drv_tun_encap_tun_idx_set
 *	Set tunnel_idx in port
 */
bool ppe_drv_tun_encap_tun_idx_configure(struct ppe_drv_tun_encap *ptec, uint32_t port_num, bool tunnel_id_valid)
{
	sw_error_t err;
	fal_tunnel_id_t encap_tun_id = {0};

	/*
	 * configure encap_tun_idx
	 */
	encap_tun_id.tunnel_id_valid = tunnel_id_valid;
	encap_tun_id.tunnel_id = ptec->tun_idx;

	err = fal_tunnel_encap_port_tunnelid_set(PPE_DRV_SWITCH_ID, port_num, &encap_tun_id);
	if (err != SW_OK) {
		ppe_drv_warn("%p: tunnel encapsulation idx set failed for port %u", ptec, port_num);
		return false;
	}

	return true;
}

/*
 * ppe_drv_tun_encap_get_len
 *	Get tunnel encap length
 */
uint16_t ppe_drv_tun_encap_get_len(struct ppe_drv_tun_encap *ptec)
{
	return ptec->tun_len;
}

/*
 * ppe_drv_tun_encap_configure
 *	Configure EG_XLAT_TUN_CTRL table
 */
bool ppe_drv_tun_encap_configure(struct ppe_drv_tun_encap *ptec,
					struct ppe_drv_tun_cmn_ctx *th,
					struct ppe_drv_tun_cmn_ctx_l2 *l2_hdr)
{
	sw_error_t err;
	fal_tunnel_encap_cfg_t encap_cfg = {0};
	fal_tunnel_encap_rule_t encap_rule = {0};

	/*
	 * Update the tunnel encapsulation header
	 */

	ppe_drv_tun_encap_hdr_set(ptec, th, l2_hdr);

	if (th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_IPV4) {
		encap_cfg.ipv4_df_mode_ext = FAL_TUNNEL_ENCAP_EXT_DF_MODE_FIX; /* Fixed value */
		encap_cfg.ipv4_df_mode = FAL_TUNNEL_ENCAP_DF_MODE_COPY; /* Copy from inner */
		encap_cfg.ipv4_id_mode = 1; /* Random value */
	} else {
		encap_cfg.ip_ver = FAL_TUNNEL_IP_VER_6; /* IPV6 ; 0 is for IPV4 */

		/*
		 * TODO: Should we have configuration for below flags
		 */
		encap_cfg.ipv6_flowlable_mode = FAL_TUNNEL_ENCAP_FLOWLABLE_MODE_COPY; /* Copy from inner */
	}

	if (th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_INHERIT_DSCP) {
		encap_cfg.dscp_mode =  FAL_TUNNEL_UNIFORM_MODE; /* uniform dscp mode */
	}

	if (th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_INHERIT_TTL) {
		encap_cfg.ttl_mode = FAL_TUNNEL_UNIFORM_MODE; /* uniform ttl mode */
	}

	/*
	 * TODO: check if DEI and PCP needs to be set to uniform mode
	 */
	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID) {
		encap_cfg.svlan_fmt = 1;
	}

	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID) {
		encap_cfg.cvlan_fmt = 1;
	}

	/*
	 * Set the VLAN offset if SVLAN or CVLAN is enabled
	 */
	if (encap_cfg.svlan_fmt || encap_cfg.cvlan_fmt) {
		encap_cfg.vlan_offset = 12;
	}

	if (ptec->l4_offset) {
		encap_cfg.l4_offset = ptec->l4_offset;
	}

	if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID) {
		encap_cfg.pppoe_en = A_TRUE;
	}

	/*
	 * Set tunnel specific bits
	 */
	if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_IPIP6) {
		encap_cfg.payload_inner_type = FAL_TUNNEL_INNER_IP;

	/*
	 * PPE currently supports only the default source port range (49152 to 65535)
	 */
	} else if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_VXLAN) {
		encap_cfg.l4_proto = FAL_TUNNEL_ENCAP_L4_PROTO_UDP; /* 0:Non;1:TCP;2:UDP;3:UDP-Lite;4:Reserved (ICMP);5:GRE; */
		encap_cfg.sport_entry_en = 1;  /* TODO: FAL API should be entropy */
		encap_cfg.payload_inner_type = FAL_TUNNEL_INNER_ETHERNET;

		if (!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM_TX)) {
			encap_cfg.l4_checksum_en = A_TRUE;
		}

	} else if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_GRETAP) {
		encap_cfg.payload_inner_type = FAL_TUNNEL_INNER_ETHERNET;
		encap_cfg.l4_proto = 5; /* 0:Non;1:TCP;2:UDP;3:UDP-Lite;4:Reserved (ICMP);5:GRE; */

	} else if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		encap_cfg.ip_proto_update = 1;
		encap_cfg.l4_checksum_en = A_TRUE;
		encap_cfg.payload_inner_type = FAL_TUNNEL_INNER_TRANSPORT;

	} else if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_L2TP_V2) {
		/*
		 * 0:Non;1:TCP;2:UDP;3:UDP-Lite;4:Reserved (ICMP);5:GRE;
		 */
		encap_cfg.l4_proto = FAL_TUNNEL_ENCAP_L4_PROTO_UDP;
		encap_cfg.payload_inner_type = FAL_TUNNEL_INNER_IP;
		encap_cfg. encap_target = FAL_TUNNEL_ENCAP_TARGET_TUNNEL_INFO;
		encap_cfg.tunnel_offset = PPE_DRV_TUN_ENCAP_L2TP_TUN_OFFSET;

		/*
		 * Tunnel Offset must be configured accordingly if there are any additional
		 * vlan or PPPoE headers
		 */
		if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_SVLAN_VALID) {
			encap_cfg.tunnel_offset += sizeof(struct vlan_hdr) * 2;
		} else if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_CVLAN_VALID) {
			encap_cfg.tunnel_offset += sizeof(struct vlan_hdr);
		}

		if (l2_hdr->flags & PPE_DRV_TUN_CMN_CTX_L2_PPPOE_VALID) {
			encap_cfg.tunnel_offset += PPPOE_SES_HLEN;
		}

		if (!(th->l3.flags & PPE_DRV_TUN_CMN_CTX_L3_UDP_ZERO_CSUM_TX)) {
			encap_cfg.l4_checksum_en = A_TRUE;
		}

		/*
		 * L2TP inner packet payload in PPP can be either ipv4 or ipv6.
		 * Based on the payload type. PPP header within L2TP frame must be updated.
		 * To update the PPP protocol feild based on the inner payload EG edit rules are configured
		 * src1_sel = FAL_TUNNEL_RULE_SRC1_FROM_HEADER_DATA, Start header parsing from SRC1 header data
		 * src1_start points to start of encap header parsing i.e 20(ETH HDR)+14(IP HDR)+8(UDP HDR) = 42 Bytes
		 * If there are any additional vlan/PPPoe headers the src1_start will be adjusted accordingly by adding the
		 * header size to offset (CVLAN = 4bytes, SVAL = 4+4 bytes, PPPoE = 8 Bytes)
		 * src3_entry is used to update the protocol  feild src_start would be from end of UDP header
		 * src_width is width to be updated which is 8 bits.
		 * dest_pos should point to the PPP protocol field. The position must be offset from Least significant bit
		 * of the selected 16 bytes data in src1.
		 * des_pos = (16Bytes - 8 Bytes (L2TP header) - 2Bytes(PPP address + PPP control)) = 6Bytes (48 bits)
		 */
		encap_rule.src1_sel = FAL_TUNNEL_RULE_SRC1_FROM_HEADER_DATA;
		encap_rule.src1_start = encap_cfg.tunnel_offset;
		encap_rule.src2_sel = FAL_TUNNEL_RULE_SRC2_ZERO_DATA;
		encap_rule.src3_sel = FAL_TUNNEL_RULE_SRC3_PROTO_MAP1;
		encap_rule.src3_entry[0].enable = A_TRUE;
		encap_rule.src3_entry[0].src_start = PPE_DRV_TUN_ENCAP_L2TP_SRC_START;
		encap_rule.src3_entry[0].src_width = PPE_DRV_TUN_ENCAP_L2TP_SRC_WIDTH;
		encap_rule.src3_entry[0].dest_pos = PPE_DRV_TUN_ENCAP_L2TP_DEST_POS;

		err = fal_tunnel_encap_rule_entry_set(PPE_DRV_SWITCH_ID, ptec->rule_id, &encap_rule);
		if(err != SW_OK) {
			ppe_drv_warn("%p: fal_tunnel_encap_rule_entry_set failed %d\n", ptec, err);
			return false;
		}
		encap_cfg.edit_rule_id = ptec->rule_id;
	}

	if (th->type == PPE_DRV_TUN_CMN_CTX_TYPE_MAPT) {
		encap_cfg.edit_rule_id = ptec->rule_id;
		encap_cfg.encap_target = FAL_TUNNEL_ENCAP_TARGET_DIP;

	}

	encap_cfg.tunnel_len = ptec->tun_len;
	encap_cfg.l3_offset = ptec->l3_offset;
	if (ptec->l4_offset_valid) {
		encap_cfg.l4_offset = ptec->l4_offset;
	}

	/*
	 * Configure vport information in encap if xmit port is vp
	 */
	if (PPE_DRV_VIRTUAL_PORT_CHK(l2_hdr->xmit_port)) {
		encap_cfg.vport_en = 1;
		encap_cfg.vport = l2_hdr->xmit_port;
	}

	/*
	 * Copy tunnel header information
	 */
	memcpy((uint8_t *)&encap_cfg.pkt_header, (uint8_t *)&ptec->hdr[0], FAL_TUNNEL_ENCAP_HEADER_MAX_LEN);

	encap_cfg.ecn_mode = (uint8_t)th->l3.encap_ecn_mode;

	/*
	 * Configure encap entry into HW
	 */
	err = fal_tunnel_encap_entry_add(PPE_DRV_SWITCH_ID, ptec->tun_idx, &encap_cfg);
	if (err != SW_OK) {
		ppe_drv_warn("%p failed to configure encap entry at %u", ptec, ptec->tun_idx);
		return false;
	}

	ppe_drv_tun_encap_dump(ptec);
	return true;
}

/*
 * ppe_drv_tun_encap_alloc
 *	Allocate PPE tunnel and return.
 */
struct ppe_drv_tun_encap *ppe_drv_tun_encap_alloc(struct ppe_drv *p)
{
	uint8_t tun_idx;

	/*
	 * Return first free instance
	 */
	for (tun_idx = 0; tun_idx < PPE_DRV_TUN_ENCAP_ENTRIES; tun_idx++) {
		struct ppe_drv_tun_encap *ptec = &p->ptun_ec[tun_idx];

		if (kref_read(&ptec->ref)) {
			continue;
		}

		ptec->tun_idx = tun_idx;
		kref_init(&ptec->ref);
		ppe_drv_trace("%p: Free tunnel encap instance found, index: %d", ptec, tun_idx);
		return ptec;
	}

	ppe_drv_warn("%p: Free tunnel encap instance is not found", p);
	return NULL;
}

/*
 * ppe_drv_tun_encap_entries_free
 *	Free memory allocated for tunnel encap instances.
 */
void ppe_drv_tun_encap_entries_free(struct ppe_drv_tun_encap *ptun_ec)
{
	vfree(ptun_ec);
}

/*
 * ppe_drv_tun_encap_entries_alloc
 *	Allocate and initialize tunnel encap table entries.
 */
struct ppe_drv_tun_encap *ppe_drv_tun_encap_entries_alloc(struct ppe_drv *p)
{
	uint16_t index;
	struct ppe_drv_tun_encap *ptun_ec;

	ptun_ec = vzalloc(sizeof(struct ppe_drv_tun_encap) * PPE_DRV_TUN_ENCAP_ENTRIES);
	if (!ptun_ec) {
		ppe_drv_warn("%p: failed to allocate ptun encap entries", p);
		return NULL;
	}

	for (index = 0; index < PPE_DRV_TUN_ENCAP_ENTRIES; index++) {
		ptun_ec[index].tun_idx = index;
	}

	return ptun_ec;
}
