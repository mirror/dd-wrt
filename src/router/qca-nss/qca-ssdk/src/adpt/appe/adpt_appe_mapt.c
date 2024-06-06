/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "fal_mapt.h"
#include "appe_tunnel_reg.h"
#include "appe_tunnel.h"
#include "appe_tunnel_map_reg.h"
#include "appe_tunnel_map.h"
#include "adpt.h"
#include <net/ipv6.h>

sw_error_t
adpt_appe_mapt_decap_ctrl_set(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
	sw_error_t rv = SW_OK;
	union tl_ctrl_u tl_ctl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(decap_ctrl);

	aos_mem_zero(&tl_ctl, sizeof(union tl_ctrl_u));

	rv = appe_tl_ctrl_get(dev_id, &tl_ctl);
	SW_RTN_ON_ERROR(rv);

	tl_ctl.bf.tl_map_src_check_cmd = decap_ctrl->src_check_action;
	tl_ctl.bf.tl_map_dst_check_cmd = decap_ctrl->dst_check_action;
	tl_ctl.bf.tl_map_non_tcp_udp_cmd = decap_ctrl->no_tcp_udp_action;
	tl_ctl.bf.tl_map_udp_csum_zero_cmd = decap_ctrl->udp_csum_zero_action;
	tl_ctl.bf.ipv4_df_set = decap_ctrl->ipv4_df_set;

	rv = appe_tl_ctrl_set(dev_id, &tl_ctl);

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_ctrl_get(a_uint32_t dev_id, fal_mapt_decap_ctrl_t *decap_ctrl)
{
	sw_error_t rv = SW_OK;
	union tl_ctrl_u tl_ctl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(decap_ctrl);

	aos_mem_zero(&tl_ctl, sizeof(union tl_ctrl_u));

	rv = appe_tl_ctrl_get(dev_id, &tl_ctl);
	SW_RTN_ON_ERROR(rv);

	decap_ctrl->src_check_action = tl_ctl.bf.tl_map_src_check_cmd;
	decap_ctrl->dst_check_action = tl_ctl.bf.tl_map_dst_check_cmd;
	decap_ctrl->no_tcp_udp_action = tl_ctl.bf.tl_map_non_tcp_udp_cmd;
	decap_ctrl->udp_csum_zero_action = tl_ctl.bf.tl_map_udp_csum_zero_cmd;
	decap_ctrl->ipv4_df_set = tl_ctl.bf.ipv4_df_set;

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_rule_entry_set(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
	sw_error_t rv = SW_OK;
	union tl_map_rule_tbl_u tl_map_rule;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_rule_entry);

	aos_mem_zero(&tl_map_rule, sizeof(union tl_map_rule_tbl_u));

	rv = appe_tl_map_rule_tbl_get(dev_id, rule_id, &tl_map_rule);
	SW_RTN_ON_ERROR(rv);

	tl_map_rule.bf.src1 = mapt_rule_entry->ip4_addr;

	tl_map_rule.bf.src2 = mapt_rule_entry->ip6_addr_src;
	tl_map_rule.bf.start2_suffix = mapt_rule_entry->ip6_suffix_sel.src_start;
	/* hardware adds 1 based on width, so minus 1 from configuration */
	tl_map_rule.bf.width2_suffix = mapt_rule_entry->ip6_suffix_sel.src_width - 1;
	tl_map_rule.bf.pos2_suffix = mapt_rule_entry->ip6_suffix_sel.dest_pos;

	tl_map_rule.bf.src2_valid = mapt_rule_entry->ip6_proto_sel.enable;
	tl_map_rule.bf.start2_psid = mapt_rule_entry->ip6_proto_sel.src_start;
	tl_map_rule.bf.width2_psid = mapt_rule_entry->ip6_proto_sel.src_width - 1;
	tl_map_rule.bf.src3_0 = mapt_rule_entry->proto_src;
	tl_map_rule.bf.src3_1 = mapt_rule_entry->proto_src >>
		SW_FIELD_OFFSET_IN_WORD(TL_MAP_RULE_TBL_SRC3_OFFSET);

	tl_map_rule.bf.start3_psid = mapt_rule_entry->proto_sel.src_start;
	tl_map_rule.bf.width3_psid = mapt_rule_entry->proto_sel.src_width - 1;
	tl_map_rule.bf.src3_valid = mapt_rule_entry->proto_sel.enable;

	tl_map_rule.bf.check_en = mapt_rule_entry->check_proto_enable;

	rv = appe_tl_map_rule_tbl_set(dev_id, rule_id, &tl_map_rule);

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_rule_entry_get(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
	sw_error_t rv = SW_OK;
	union tl_map_rule_tbl_u tl_map_rule;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_rule_entry);

	aos_mem_zero(&tl_map_rule, sizeof(union tl_map_rule_tbl_u));

	rv = appe_tl_map_rule_tbl_get(dev_id, rule_id, &tl_map_rule);
	SW_RTN_ON_ERROR(rv);

	mapt_rule_entry->ip4_addr = tl_map_rule.bf.src1;
	mapt_rule_entry->ip6_addr_src = tl_map_rule.bf.src2;
	mapt_rule_entry->ip6_suffix_sel.src_start = tl_map_rule.bf.start2_suffix;
	mapt_rule_entry->ip6_suffix_sel.src_width = tl_map_rule.bf.width2_suffix + 1;
	mapt_rule_entry->ip6_suffix_sel.dest_pos = tl_map_rule.bf.pos2_suffix;

	mapt_rule_entry->ip6_proto_sel.enable = tl_map_rule.bf.src2_valid;
	mapt_rule_entry->ip6_proto_sel.src_start = tl_map_rule.bf.start2_psid;
	mapt_rule_entry->ip6_proto_sel.src_width = tl_map_rule.bf.width2_psid + 1;

	mapt_rule_entry->proto_src = tl_map_rule.bf.src3_0 |
		tl_map_rule.bf.src3_1 << SW_FIELD_OFFSET_IN_WORD(TL_MAP_RULE_TBL_SRC3_OFFSET);

	mapt_rule_entry->proto_sel.src_start = tl_map_rule.bf.start3_psid;
	mapt_rule_entry->proto_sel.src_width = tl_map_rule.bf.width3_psid + 1;
	mapt_rule_entry->proto_sel.enable = tl_map_rule.bf.src3_valid;
	mapt_rule_entry->check_proto_enable = tl_map_rule.bf.check_en;

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_rule_entry_del(a_uint32_t dev_id,
		a_uint32_t rule_id, fal_mapt_decap_edit_rule_entry_t *mapt_rule_entry)
{
	sw_error_t rv = SW_OK;
	union tl_map_rule_tbl_u tl_map_rule;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_rule_entry);

	aos_mem_zero(&tl_map_rule, sizeof(union tl_map_rule_tbl_u));

	rv = appe_tl_map_rule_tbl_set(dev_id, rule_id, &tl_map_rule);

	return rv;
}

static sw_error_t
adpt_mapt_decap_entry_convert(fal_mapt_decap_entry_t *mapt_entry, a_bool_t to_hsl,
		union tl_map_lpm_tbl_u *tl_map_lpm, union tl_map_lpm_act_u *tl_map_act)
{
	sw_error_t rv = SW_OK;
	if (to_hsl) {
		tl_map_lpm->bf.ipv6_addr_0 = mapt_entry->ip6_addr.ul[3];
		tl_map_lpm->bf.ipv6_addr_1 = mapt_entry->ip6_addr.ul[2];
		tl_map_lpm->bf.ipv6_addr_2 = mapt_entry->ip6_addr.ul[1];
		tl_map_lpm->bf.ipv6_addr_3 = mapt_entry->ip6_addr.ul[0];
		tl_map_lpm->bf.prefix_len = mapt_entry->ip6_prefix_len;
		tl_map_lpm->bf.valid = A_TRUE;

		tl_map_act->bf.map_rule_id = mapt_entry->edit_rule_id;
		tl_map_act->bf.ip_to_me = mapt_entry->dst_is_local;
		tl_map_act->bf.svlan_fmt = mapt_entry->verify_entry.svlan_fmt;
		tl_map_act->bf.svlan_id = mapt_entry->verify_entry.svlan_id;
		tl_map_act->bf.cvlan_fmt = mapt_entry->verify_entry.cvlan_fmt;
		tl_map_act->bf.cvlan_id = mapt_entry->verify_entry.cvlan_id;
		tl_map_act->bf.tl_l3_if = mapt_entry->verify_entry.tl_l3_if;
		tl_map_act->bf.svlan_check_en = (mapt_entry->verify_entry.verify_bmp &
				FAL_TUNNEL_SVLAN_CHECK_EN) ? A_TRUE : A_FALSE;
		tl_map_act->bf.cvlan_check_en = (mapt_entry->verify_entry.verify_bmp &
				FAL_TUNNEL_CVLAN_CHECK_EN) ? A_TRUE : A_FALSE;
		tl_map_act->bf.tl_l3_if_check_en = (mapt_entry->verify_entry.verify_bmp &
				FAL_TUNNEL_L3IF_CHECK_EN) ? A_TRUE : A_FALSE;
		tl_map_act->bf.src_info_valid = mapt_entry->src_info_enable;
		tl_map_act->bf.src_info_type = mapt_entry->src_info_type;
		tl_map_act->bf.src_info = mapt_entry->src_info;
		tl_map_act->bf.exp_profile = mapt_entry->exp_profile;
#if defined(MPPE)
		tl_map_act->bf.service_code = mapt_entry->service_code;
		tl_map_act->bf.service_code_en = mapt_entry->service_code_en;
#endif
	} else {
		mapt_entry->ip6_addr.ul[3] = tl_map_lpm->bf.ipv6_addr_0;
		mapt_entry->ip6_addr.ul[2] = tl_map_lpm->bf.ipv6_addr_1;
		mapt_entry->ip6_addr.ul[1] = tl_map_lpm->bf.ipv6_addr_2;
		mapt_entry->ip6_addr.ul[0] = tl_map_lpm->bf.ipv6_addr_3;
		mapt_entry->ip6_prefix_len = tl_map_lpm->bf.prefix_len;

		mapt_entry->edit_rule_id = tl_map_act->bf.map_rule_id;
		mapt_entry->dst_is_local = tl_map_act->bf.ip_to_me;
		mapt_entry->verify_entry.svlan_fmt = tl_map_act->bf.svlan_fmt;
		mapt_entry->verify_entry.svlan_id = tl_map_act->bf.svlan_id;
		mapt_entry->verify_entry.cvlan_fmt = tl_map_act->bf.cvlan_fmt;
		mapt_entry->verify_entry.cvlan_id = tl_map_act->bf.cvlan_id;
		mapt_entry->verify_entry.tl_l3_if = tl_map_act->bf.tl_l3_if;

		if (tl_map_act->bf.svlan_check_en)
			mapt_entry->verify_entry.verify_bmp |= FAL_TUNNEL_SVLAN_CHECK_EN;
		else
			mapt_entry->verify_entry.verify_bmp &= ~FAL_TUNNEL_SVLAN_CHECK_EN;

		if (tl_map_act->bf.cvlan_check_en)
			mapt_entry->verify_entry.verify_bmp |= FAL_TUNNEL_CVLAN_CHECK_EN;
		else
			mapt_entry->verify_entry.verify_bmp &= ~FAL_TUNNEL_CVLAN_CHECK_EN;

		if (tl_map_act->bf.tl_l3_if_check_en)
			mapt_entry->verify_entry.verify_bmp |= FAL_TUNNEL_L3IF_CHECK_EN;
		else
			mapt_entry->verify_entry.verify_bmp &= ~FAL_TUNNEL_L3IF_CHECK_EN;

		mapt_entry->src_info_enable = tl_map_act->bf.src_info_valid;
		mapt_entry->src_info_type = tl_map_act->bf.src_info_type;
		mapt_entry->src_info = tl_map_act->bf.src_info;
		mapt_entry->exp_profile = tl_map_act->bf.exp_profile;
#if defined(MPPE)
		mapt_entry->service_code = tl_map_act->bf.service_code;
		mapt_entry->service_code_en = tl_map_act->bf.service_code_en;
#endif
	}

	return rv;
}

static a_bool_t adpt_lpm_compare(fal_mapt_decap_entry_t mapt_entry,
		union tl_map_lpm_tbl_u tl_map_lpm)
{
	struct in6_addr addr1, addr2;
	a_bool_t result = A_FALSE;

	if (mapt_entry.ip6_prefix_len != tl_map_lpm.bf.prefix_len)
		return A_FALSE;

	aos_mem_zero(&addr1, sizeof(struct in6_addr));
	aos_mem_zero(&addr2, sizeof(struct in6_addr));

	addr1.in6_u.u6_addr32[0] = tl_map_lpm.bf.ipv6_addr_3;
	addr1.in6_u.u6_addr32[1] = tl_map_lpm.bf.ipv6_addr_2;
	addr1.in6_u.u6_addr32[2] = tl_map_lpm.bf.ipv6_addr_1;
	addr1.in6_u.u6_addr32[3] = tl_map_lpm.bf.ipv6_addr_0;

	addr2.in6_u.u6_addr32[0] = mapt_entry.ip6_addr.ul[0];
	addr2.in6_u.u6_addr32[1] = mapt_entry.ip6_addr.ul[1];
	addr2.in6_u.u6_addr32[2] = mapt_entry.ip6_addr.ul[2];
	addr2.in6_u.u6_addr32[3] = mapt_entry.ip6_addr.ul[3];

	result = ipv6_prefix_equal(&addr1, &addr2, (unsigned int)mapt_entry.ip6_prefix_len);

	return result;
}

sw_error_t
adpt_appe_mapt_decap_entry_add(a_uint32_t dev_id,
		fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv = SW_OK;
	union tl_map_lpm_tbl_u tl_map_lpm;
	union tl_map_lpm_act_u tl_map_act;
	a_uint32_t index, update_index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_entry);

	aos_mem_zero(&tl_map_lpm, sizeof(union tl_map_lpm_tbl_u));
	aos_mem_zero(&tl_map_act, sizeof(union tl_map_lpm_act_u));

	index = 0;
	update_index = TL_MAP_LPM_TBL_MAX_ENTRY;

	if (mapt_entry->op_mode == FAL_TUNNEL_OP_MODE_INDEX) {
		update_index = mapt_entry->entry_index;
	} else {
		while (index < TL_MAP_LPM_TBL_MAX_ENTRY) {
			rv = appe_tl_map_lpm_tbl_get(dev_id, index, &tl_map_lpm);
			SW_RTN_ON_ERROR(rv);

			if (!tl_map_lpm.bf.valid && update_index == TL_MAP_LPM_TBL_MAX_ENTRY) {
				update_index = index;
			}

			if (adpt_lpm_compare(*mapt_entry, tl_map_lpm))
				return SW_ALREADY_EXIST;
			index++;
		}
	}

	if (update_index != TL_MAP_LPM_TBL_MAX_ENTRY) {
		rv = adpt_mapt_decap_entry_convert(mapt_entry, A_TRUE, &tl_map_lpm, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		rv = appe_tl_map_lpm_tbl_set(dev_id, update_index, &tl_map_lpm);
		SW_RTN_ON_ERROR(rv);

		rv = appe_tl_map_lpm_act_set(dev_id, update_index, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		mapt_entry->entry_index = update_index;
	} else {
		return SW_FULL;
	}

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_entry_del(a_uint32_t dev_id,
		fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv = SW_OK;
	union tl_map_lpm_tbl_u tl_map_lpm;
	union tl_map_lpm_act_u tl_map_act;
	union tl_map_lpm_counter_u tl_map_cnt;
	a_uint32_t index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_entry);

	aos_mem_zero(&tl_map_lpm, sizeof(union tl_map_lpm_tbl_u));
	aos_mem_zero(&tl_map_act, sizeof(union tl_map_lpm_act_u));
	aos_mem_zero(&tl_map_cnt, sizeof(union tl_map_lpm_counter_u));

	if (mapt_entry->op_mode == FAL_TUNNEL_OP_MODE_INDEX) {
		index = mapt_entry->entry_index;
	} else {
		index = 0;
		while (index < TL_MAP_LPM_TBL_MAX_ENTRY) {
			rv = appe_tl_map_lpm_tbl_get(dev_id, index, &tl_map_lpm);
			SW_RTN_ON_ERROR(rv);

			if (adpt_lpm_compare(*mapt_entry, tl_map_lpm))
				break;
			index++;
		}
	}

	if (index != TL_MAP_LPM_TBL_MAX_ENTRY) {
		aos_mem_zero(&tl_map_lpm, sizeof(union tl_map_lpm_tbl_u));

		rv = appe_tl_map_lpm_tbl_set(dev_id, index, &tl_map_lpm);
		SW_RTN_ON_ERROR(rv);

		rv = appe_tl_map_lpm_act_set(dev_id, index, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		rv = appe_tl_map_lpm_counter_set(dev_id, index, &tl_map_cnt);
		SW_RTN_ON_ERROR(rv);
	} else {
		return SW_NOT_FOUND;
	}

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_entry_getfirst(a_uint32_t dev_id,
		fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv = SW_OK;
	union tl_map_lpm_tbl_u tl_map_lpm;
	union tl_map_lpm_act_u tl_map_act;
	union tl_map_lpm_counter_u tl_map_cnt;
	a_uint32_t index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_entry);

	aos_mem_zero(&tl_map_lpm, sizeof(union tl_map_lpm_tbl_u));
	aos_mem_zero(&tl_map_act, sizeof(union tl_map_lpm_act_u));
	aos_mem_zero(&tl_map_cnt, sizeof(union tl_map_lpm_counter_u));

	index = 0;
	while (index < TL_MAP_LPM_TBL_MAX_ENTRY) {
		rv = appe_tl_map_lpm_tbl_get(dev_id, index, &tl_map_lpm);
		SW_RTN_ON_ERROR(rv);

		if (tl_map_lpm.bf.valid)
			break;
		index++;
	}

	if (index != TL_MAP_LPM_TBL_MAX_ENTRY) {
		rv = appe_tl_map_lpm_act_get(dev_id, index, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		rv = adpt_mapt_decap_entry_convert(mapt_entry, A_FALSE, &tl_map_lpm, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		mapt_entry->entry_index = index;

		rv = appe_tl_map_lpm_counter_get(dev_id, index, &tl_map_cnt);
		SW_RTN_ON_ERROR(rv);

		mapt_entry->pkt_counter = tl_map_cnt.bf.pkt_cnt;
		mapt_entry->byte_counter = ((a_uint64_t)tl_map_cnt.bf.byte_cnt_1 <<
				SW_FIELD_OFFSET_IN_WORD(TL_MAP_LPM_COUNTER_BYTE_CNT_OFFSET)) |
			tl_map_cnt.bf.byte_cnt_0;
	} else {
		return SW_NOT_FOUND;
	}

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_entry_getnext(a_uint32_t dev_id,
		fal_mapt_decap_entry_t *mapt_entry)
{
	sw_error_t rv = SW_OK;
	a_bool_t get_next = A_FALSE;
	union tl_map_lpm_tbl_u tl_map_lpm;
	union tl_map_lpm_act_u tl_map_act;
	union tl_map_lpm_counter_u tl_map_cnt;
	a_uint32_t index;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mapt_entry);

	aos_mem_zero(&tl_map_lpm, sizeof(union tl_map_lpm_tbl_u));
	aos_mem_zero(&tl_map_act, sizeof(union tl_map_lpm_act_u));
	aos_mem_zero(&tl_map_cnt, sizeof(union tl_map_lpm_counter_u));

	index = 0;
	while (index < TL_MAP_LPM_TBL_MAX_ENTRY) {
		rv = appe_tl_map_lpm_tbl_get(dev_id, index, &tl_map_lpm);
		SW_RTN_ON_ERROR(rv);

		if (!tl_map_lpm.bf.valid) {
			index++;
			continue;
		}

		if (get_next && tl_map_lpm.bf.valid)
			break;

		if (adpt_lpm_compare(*mapt_entry, tl_map_lpm))
			get_next = A_TRUE;

		index++;
	}

	if (index != TL_MAP_LPM_TBL_MAX_ENTRY) {
		rv = appe_tl_map_lpm_act_get(dev_id, index, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		rv = adpt_mapt_decap_entry_convert(mapt_entry, A_FALSE, &tl_map_lpm, &tl_map_act);
		SW_RTN_ON_ERROR(rv);

		mapt_entry->entry_index = index;

		rv = appe_tl_map_lpm_counter_get(dev_id, index, &tl_map_cnt);
		SW_RTN_ON_ERROR(rv);

		mapt_entry->pkt_counter = tl_map_cnt.bf.pkt_cnt;
		mapt_entry->byte_counter = ((a_uint64_t)tl_map_cnt.bf.byte_cnt_1 <<
				SW_FIELD_OFFSET_IN_WORD(TL_MAP_LPM_COUNTER_BYTE_CNT_OFFSET)) |
			tl_map_cnt.bf.byte_cnt_0;
	} else {
		return SW_NOT_FOUND;
	}

	return rv;
}

sw_error_t
adpt_appe_mapt_decap_en_set(a_uint32_t dev_id,
		a_uint32_t mapt_index, a_bool_t en)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

	if (mapt_index >= TL_MAP_LPM_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	rv = appe_tl_map_lpm_tbl_valid_set(dev_id, mapt_index, en);
	return rv;
}

sw_error_t
adpt_appe_mapt_decap_en_get(a_uint32_t dev_id,
		a_uint32_t mapt_index, a_bool_t *en)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(en);

	if (mapt_index >= TL_MAP_LPM_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	rv = appe_tl_map_lpm_tbl_valid_get(dev_id, mapt_index, en);
	return rv;
}

sw_error_t
adpt_appe_mapt_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

	p_adpt_api->adpt_mapt_decap_ctrl_set = adpt_appe_mapt_decap_ctrl_set;
	p_adpt_api->adpt_mapt_decap_ctrl_get = adpt_appe_mapt_decap_ctrl_get;
	p_adpt_api->adpt_mapt_decap_rule_entry_set = adpt_appe_mapt_decap_rule_entry_set;
	p_adpt_api->adpt_mapt_decap_rule_entry_get = adpt_appe_mapt_decap_rule_entry_get;
	p_adpt_api->adpt_mapt_decap_rule_entry_del = adpt_appe_mapt_decap_rule_entry_del;
	p_adpt_api->adpt_mapt_decap_entry_add = adpt_appe_mapt_decap_entry_add;
	p_adpt_api->adpt_mapt_decap_entry_del = adpt_appe_mapt_decap_entry_del;
	p_adpt_api->adpt_mapt_decap_entry_getfirst = adpt_appe_mapt_decap_entry_getfirst;
	p_adpt_api->adpt_mapt_decap_entry_getnext = adpt_appe_mapt_decap_entry_getnext;
	p_adpt_api->adpt_mapt_decap_en_set = adpt_appe_mapt_decap_en_set;
	p_adpt_api->adpt_mapt_decap_en_get = adpt_appe_mapt_decap_en_get;

	return SW_OK;
}

/**
 * @}
 */
