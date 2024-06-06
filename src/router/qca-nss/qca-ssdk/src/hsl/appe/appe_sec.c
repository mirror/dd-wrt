/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "appe_sec_reg.h"
#include "appe_sec.h"


sw_error_t
appe_l2_flow_hit_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_exp_ctrl_u *value)
{
	if (index >= L2_FLOW_HIT_EXP_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + L2_FLOW_HIT_EXP_CTRL_ADDRESS + \
				index * L2_FLOW_HIT_EXP_CTRL_INC,
				&value->val);
}

sw_error_t
appe_l2_flow_hit_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_exp_ctrl_u *value)
{
	if (index >= L2_FLOW_HIT_EXP_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + L2_FLOW_HIT_EXP_CTRL_ADDRESS + \
				index * L2_FLOW_HIT_EXP_CTRL_INC,
				value->val);
}

sw_error_t
appe_l3_flow_hit_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_exp_ctrl_u *value)
{
	if (index >= L3_FLOW_HIT_EXP_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + L3_FLOW_HIT_EXP_CTRL_ADDRESS + \
				index * L3_FLOW_HIT_EXP_CTRL_INC,
				&value->val);
}

sw_error_t
appe_l3_flow_hit_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_exp_ctrl_u *value)
{
	if (index >= L3_FLOW_HIT_EXP_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + L3_FLOW_HIT_EXP_CTRL_ADDRESS + \
				index * L3_FLOW_HIT_EXP_CTRL_INC,
				value->val);
}

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_miss_exp_ctrl_u *value)
{
	if (index >= L3_FLOW_HIT_MISS_EXP_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + L3_FLOW_HIT_MISS_EXP_CTRL_ADDRESS + \
				index * L3_FLOW_HIT_MISS_EXP_CTRL_INC,
				&value->val);
}

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l3_flow_hit_miss_exp_ctrl_u *value)
{
    if (index >= L3_FLOW_HIT_MISS_EXP_CTRL_MAX_ENTRY)
            return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + L3_FLOW_HIT_MISS_EXP_CTRL_ADDRESS + \
				index * L3_FLOW_HIT_MISS_EXP_CTRL_INC,
				value->val);
}

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_miss_exp_ctrl_u *value)
{
	if (index >= L2_FLOW_HIT_MISS_EXP_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + L2_FLOW_HIT_MISS_EXP_CTRL_ADDRESS + \
				index * L2_FLOW_HIT_MISS_EXP_CTRL_INC,
				&value->val);
}

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_flow_hit_miss_exp_ctrl_u *value)
{
    if (index >= L2_FLOW_HIT_MISS_EXP_CTRL_MAX_ENTRY)
            return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + L2_FLOW_HIT_MISS_EXP_CTRL_ADDRESS + \
				index * L2_FLOW_HIT_MISS_EXP_CTRL_INC,
				value->val);
}

#ifndef IN_SEC_MINI
sw_error_t
appe_l2_excep_ctrl_get(
		a_uint32_t dev_id,
		union l2_excep_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L2_BASE_ADDR + L2_EXCEP_CTRL_ADDRESS,
				&value->val);
}

sw_error_t
appe_l2_excep_ctrl_set(
		a_uint32_t dev_id,
		union l2_excep_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L2_BASE_ADDR + L2_EXCEP_CTRL_ADDRESS,
				value->val);
}

sw_error_t
appe_l2_excep_ctrl_tunnel_excep_fwd_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union l2_excep_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_excep_ctrl_get(dev_id, &reg_val);
	*value = reg_val.bf.tunnel_excep_fwd;
	return ret;
}

sw_error_t
appe_l2_excep_ctrl_tunnel_excep_fwd_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union l2_excep_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_excep_ctrl_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tunnel_excep_fwd = value;
	ret = appe_l2_excep_ctrl_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_l2_flow_hit_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_flow_hit_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_flow_hit_exp_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_l2_flow_hit_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_flow_hit_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_flow_hit_exp_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_l2_flow_hit_exp_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l3_flow_hit_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l3_flow_hit_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l3_flow_hit_exp_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_l3_flow_hit_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l3_flow_hit_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l3_flow_hit_exp_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_l3_flow_hit_exp_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l3_flow_hit_miss_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l3_flow_hit_miss_exp_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_l3_flow_hit_miss_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l3_flow_hit_miss_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l3_flow_hit_miss_exp_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_l3_flow_hit_miss_exp_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_flow_hit_miss_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_flow_hit_miss_exp_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_l2_flow_hit_miss_exp_ctrl_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_flow_hit_miss_exp_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_flow_hit_miss_exp_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_l2_flow_hit_miss_exp_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_exception_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exception_cmd_u *value)
{
	if (index >= TL_EXCEPTION_CMD_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXCEPTION_CMD_ADDRESS + \
				index * TL_EXCEPTION_CMD_INC,
				&value->val);
}

sw_error_t
appe_tl_exception_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exception_cmd_u *value)
{
	if (index >= TL_EXCEPTION_CMD_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXCEPTION_CMD_ADDRESS + \
				index * TL_EXCEPTION_CMD_INC,
				value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile0_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE0_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE0_INC,
				&value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile0_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE0_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE0_INC,
				value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile1_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE1_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE1_INC,
				&value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile1_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE1_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE1_INC,
				value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile2_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE2_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE2_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE2_INC,
				&value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile2_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE2_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE2_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE2_INC,
				value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile3_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE3_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE3_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE3_INC,
				&value->val);
}

sw_error_t
appe_tl_exp_ctrl_profile3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_exp_ctrl_profile3_u *value)
{
	if (index >= TL_EXP_CTRL_PROFILE3_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_EXP_CTRL_PROFILE3_ADDRESS + \
				index * TL_EXP_CTRL_PROFILE3_INC,
				value->val);
}

sw_error_t
appe_tl_exception_cmd_tl_excep_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_exception_cmd_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exception_cmd_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tl_excep_cmd;
	return ret;
}

sw_error_t
appe_tl_exception_cmd_tl_excep_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_exception_cmd_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exception_cmd_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tl_excep_cmd = value;
	ret = appe_tl_exception_cmd_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_exception_cmd_de_acce_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_exception_cmd_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exception_cmd_get(dev_id, index, &reg_val);
	*value = reg_val.bf.de_acce;
	return ret;
}

sw_error_t
appe_tl_exception_cmd_de_acce_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_exception_cmd_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exception_cmd_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.de_acce = value;
	ret = appe_tl_exception_cmd_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile0_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_exp_ctrl_profile0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile0_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_exp_ctrl_profile0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_tl_exp_ctrl_profile0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile1_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_exp_ctrl_profile1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile1_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_exp_ctrl_profile1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_tl_exp_ctrl_profile1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile2_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_exp_ctrl_profile2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile2_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile2_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_exp_ctrl_profile2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile2_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_tl_exp_ctrl_profile2_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile3_excep_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_exp_ctrl_profile3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile3_get(dev_id, index, &reg_val);
	*value = reg_val.bf.excep_en;
	return ret;
}

sw_error_t
appe_tl_exp_ctrl_profile3_excep_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_exp_ctrl_profile3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_exp_ctrl_profile3_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.excep_en = value;
	ret = appe_tl_exp_ctrl_profile3_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_0_u *value)
{
	if (index >= TPR_EXCEPTION_CTRL_0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_EXCEPTION_CTRL_0_ADDRESS + \
				index * TPR_EXCEPTION_CTRL_0_INC,
				&value->val);
}

sw_error_t
appe_tpr_exception_ctrl_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_0_u *value)
{
	if (index >= TPR_EXCEPTION_CTRL_0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_EXCEPTION_CTRL_0_ADDRESS + \
				index * TPR_EXCEPTION_CTRL_0_INC,
				value->val);
}

sw_error_t
appe_tpr_exception_ctrl_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_1_u *value)
{
	if (index >= TPR_EXCEPTION_CTRL_1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_EXCEPTION_CTRL_1_ADDRESS + \
				index * TPR_EXCEPTION_CTRL_1_INC,
				&value->val);
}

sw_error_t
appe_tpr_exception_ctrl_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_exception_ctrl_1_u *value)
{
	if (index >= TPR_EXCEPTION_CTRL_1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_EXCEPTION_CTRL_1_ADDRESS + \
				index * TPR_EXCEPTION_CTRL_1_INC,
				value->val);
}

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_get(
		a_uint32_t dev_id,
		union tpr_l3_exception_parsing_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L3_EXCEPTION_PARSING_CTRL_ADDRESS,
				&value->val);
}

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_set(
		a_uint32_t dev_id,
		union tpr_l3_exception_parsing_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L3_EXCEPTION_PARSING_CTRL_ADDRESS,
				value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_0_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_0_ADDRESS,
				&value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_0_ADDRESS,
				value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_1_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_1_ADDRESS,
				&value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_1_ADDRESS,
				value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_2_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_2_ADDRESS,
				&value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_2_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_2_ADDRESS,
				value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_get(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_3_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_3_ADDRESS,
				&value->val);
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_set(
		a_uint32_t dev_id,
		union tpr_l4_exception_parsing_ctrl_3_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_L4_EXCEPTION_PARSING_CTRL_3_ADDRESS,
				value->val);
}

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_hop_limit_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l3_exception_parsing_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l3_exception_parsing_ctrl_get(dev_id, &reg_val);
	*value = reg_val.bf.small_hop_limit;
	return ret;
}

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_hop_limit_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l3_exception_parsing_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l3_exception_parsing_ctrl_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.small_hop_limit = value;
	ret = appe_tpr_l3_exception_parsing_ctrl_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_ttl_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l3_exception_parsing_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l3_exception_parsing_ctrl_get(dev_id, &reg_val);
	*value = reg_val.bf.small_ttl;
	return ret;
}

sw_error_t
appe_tpr_l3_exception_parsing_ctrl_small_ttl_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l3_exception_parsing_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l3_exception_parsing_ctrl_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.small_ttl = value;
	ret = appe_tpr_l3_exception_parsing_ctrl_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags1_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags1_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags0_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags0_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags0;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags0_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags0 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags1;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_0_tcp_flags1_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags1 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags2;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags2 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags3;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags3 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags3_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags3_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags3_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags2_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_1_tcp_flags2_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags2_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags4_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags4_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_2_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags4;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags4_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags4 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_2_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags5;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags5 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_2_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags5_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_2_tcp_flags5_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_2_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags5_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_2_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags6_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags6_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_3_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags7;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags7 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_3_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_mask_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags7_mask;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags7_mask_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags7_mask = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_3_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	*value = reg_val.bf.tcp_flags6;
	return ret;
}

sw_error_t
appe_tpr_l4_exception_parsing_ctrl_3_tcp_flags6_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_l4_exception_parsing_ctrl_3_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_l4_exception_parsing_ctrl_3_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tcp_flags6 = value;
	ret = appe_tpr_l4_exception_parsing_ctrl_3_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_0_flags_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_exception_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flags;
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_0_flags_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_exception_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flags = value;
	ret = appe_tpr_exception_ctrl_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_0_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_exception_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mask;
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_0_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_exception_ctrl_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mask = value;
	ret = appe_tpr_exception_ctrl_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_1_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_exception_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hdr_type;
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_1_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_exception_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hdr_type = value;
	ret = appe_tpr_exception_ctrl_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_1_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_exception_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.valid;
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_1_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_exception_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.valid = value;
	ret = appe_tpr_exception_ctrl_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_1_comp_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_exception_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.comp_mode;
	return ret;
}

sw_error_t
appe_tpr_exception_ctrl_1_comp_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_exception_ctrl_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_exception_ctrl_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.comp_mode = value;
	ret = appe_tpr_exception_ctrl_1_set(dev_id, index, &reg_val);
	return ret;
}
#endif


