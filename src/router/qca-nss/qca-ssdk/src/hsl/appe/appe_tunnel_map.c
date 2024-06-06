/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "hsl.h"
#include "hppe_reg_access.h"
#include "appe_tunnel_map_reg.h"
#include "appe_tunnel_map.h"

sw_error_t
appe_tl_map_lpm_counter_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_lpm_counter_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_LPM_COUNTER_ADDRESS + \
				index * TL_MAP_LPM_COUNTER_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_map_lpm_counter_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_lpm_counter_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_LPM_COUNTER_ADDRESS + \
				index * TL_MAP_LPM_COUNTER_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_map_rule_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_rule_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_RULE_TBL_ADDRESS + \
				index * TL_MAP_RULE_TBL_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_map_rule_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_rule_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_RULE_TBL_ADDRESS + \
				index * TL_MAP_RULE_TBL_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_map_lpm_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_lpm_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_LPM_TBL_ADDRESS + \
				index * TL_MAP_LPM_TBL_INC,
				value->val,
				5);
}

sw_error_t
appe_tl_map_lpm_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_lpm_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_LPM_TBL_ADDRESS + \
				index * TL_MAP_LPM_TBL_INC,
				value->val,
				5);
}

sw_error_t
appe_tl_map_lpm_act_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_lpm_act_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_LPM_ACT_ADDRESS + \
				index * TL_MAP_LPM_ACT_INC,
				value->val,
				2);
}

sw_error_t
appe_tl_map_lpm_act_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_map_lpm_act_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_MAP_LPM_ACT_ADDRESS + \
				index * TL_MAP_LPM_ACT_INC,
				value->val,
				2);
}

#if 0
sw_error_t
appe_tl_map_lpm_counter_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union tl_map_lpm_counter_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_counter_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.byte_cnt_1 << 32 | \
		reg_val.bf.byte_cnt_0;
	return ret;
}

sw_error_t
appe_tl_map_lpm_counter_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union tl_map_lpm_counter_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_counter_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.byte_cnt_1 = value >> 32;
	reg_val.bf.byte_cnt_0 = value & (((a_uint64_t)1<<32)-1);
	ret = appe_tl_map_lpm_counter_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_counter_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_counter_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_counter_get(dev_id, index, &reg_val);
	*value = reg_val.bf.pkt_cnt;
	return ret;
}

sw_error_t
appe_tl_map_lpm_counter_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_counter_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_counter_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.pkt_cnt = value;
	ret = appe_tl_map_lpm_counter_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_start3_psid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.start3_psid;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_start3_psid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.start3_psid = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_width2_psid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.width2_psid;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_width2_psid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.width2_psid = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_start2_psid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.start2_psid;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_start2_psid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.start2_psid = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src3_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src3_valid;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src3_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src3_valid = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src2_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src2_valid;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src2_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src2_valid = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src1;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src1 = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src3_1 << 1 | \
		reg_val.bf.src3_0;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src3_1 = value >> 1;
	reg_val.bf.src3_0 = value & (((a_uint64_t)1<<1)-1);
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_check_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.check_en;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_check_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.check_en = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_width2_suffix_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.width2_suffix;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_width2_suffix_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.width2_suffix = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_pos2_suffix_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.pos2_suffix;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_pos2_suffix_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.pos2_suffix = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src2;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_src2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src2 = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_start2_suffix_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.start2_suffix;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_start2_suffix_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.start2_suffix = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_width3_psid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.width3_psid;
	return ret;
}

sw_error_t
appe_tl_map_rule_tbl_width3_psid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_rule_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_rule_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.width3_psid = value;
	ret = appe_tl_map_rule_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_tbl_prefix_len_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.prefix_len;
	return ret;
}

sw_error_t
appe_tl_map_lpm_tbl_prefix_len_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.prefix_len = value;
	ret = appe_tl_map_lpm_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_tbl_ipv6_addr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union tl_map_lpm_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.ipv6_addr_1 << 32 | \
		reg_val.bf.ipv6_addr_0;
	return ret;
}

sw_error_t
appe_tl_map_lpm_tbl_ipv6_addr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union tl_map_lpm_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ipv6_addr_1 = value >> 32;
	reg_val.bf.ipv6_addr_0 = value & (((a_uint64_t)1<<32)-1);
	ret = appe_tl_map_lpm_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif

sw_error_t
appe_tl_map_lpm_tbl_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.valid;
	return ret;
}

sw_error_t
appe_tl_map_lpm_tbl_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.valid = value;
	ret = appe_tl_map_lpm_tbl_set(dev_id, index, &reg_val);
	return ret;
}

#if 0
sw_error_t
appe_tl_map_lpm_act_exp_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.exp_profile;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_exp_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.exp_profile = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_cvlan_fmt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cvlan_fmt;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_cvlan_fmt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cvlan_fmt = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_tl_l3_if_check_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tl_l3_if_check_en;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_tl_l3_if_check_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tl_l3_if_check_en = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_svlan_check_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.svlan_check_en;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_svlan_check_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.svlan_check_en = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_cvlan_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cvlan_id;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_cvlan_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cvlan_id = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_ip_to_me_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ip_to_me;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_ip_to_me_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ip_to_me = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_map_rule_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.map_rule_id;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_map_rule_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.map_rule_id = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_src_info_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src_info_type;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_src_info_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src_info_type = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_svlan_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.svlan_id;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_svlan_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.svlan_id = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_tl_l3_if_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tl_l3_if;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_tl_l3_if_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tl_l3_if = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_svlan_fmt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.svlan_fmt;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_svlan_fmt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.svlan_fmt = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_cvlan_check_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cvlan_check_en;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_cvlan_check_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cvlan_check_en = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_src_info_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src_info;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_src_info_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src_info = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_src_info_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.src_info_valid;
	return ret;
}

sw_error_t
appe_tl_map_lpm_act_src_info_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.src_info_valid = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

#if defined(MPPE)
sw_error_t
mppe_tl_map_lpm_act_service_code_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.service_code;
	return ret;
}

sw_error_t
mppe_tl_map_lpm_act_service_code_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.service_code = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
mppe_tl_map_lpm_act_service_code_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	*value = reg_val.bf.service_code_en;
	return ret;
}

sw_error_t
mppe_tl_map_lpm_act_service_code_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_map_lpm_act_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_map_lpm_act_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.service_code_en = value;
	ret = appe_tl_map_lpm_act_set(dev_id, index, &reg_val);
	return ret;
}
#endif
#endif
