/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023, Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "appe_tunnel_program_reg.h"
#include "appe_tunnel_program.h"

sw_error_t
appe_tpr_hdr_match_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_0_u *value)
{
	if (index >= TPR_HDR_MATCH_0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_HDR_MATCH_0_ADDRESS + \
				index * TPR_HDR_MATCH_0_INC,
				&value->val);
}

sw_error_t
appe_tpr_hdr_match_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_HDR_MATCH_0_ADDRESS + \
				index * TPR_HDR_MATCH_0_INC,
				value->val);
}

sw_error_t
appe_tpr_hdr_match_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_1_u *value)
{
	if (index >= TPR_HDR_MATCH_1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_HDR_MATCH_1_ADDRESS + \
				index * TPR_HDR_MATCH_1_INC,
				&value->val);
}

sw_error_t
appe_tpr_hdr_match_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_HDR_MATCH_1_ADDRESS + \
				index * TPR_HDR_MATCH_1_INC,
				value->val);
}

sw_error_t
appe_tpr_hdr_match_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_2_u *value)
{
	if (index >= TPR_HDR_MATCH_2_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_HDR_MATCH_2_ADDRESS + \
				index * TPR_HDR_MATCH_2_INC,
				&value->val);
}

sw_error_t
appe_tpr_hdr_match_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_hdr_match_2_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_HDR_MATCH_2_ADDRESS + \
				index * TPR_HDR_MATCH_2_INC,
				value->val);
}

sw_error_t
appe_tpr_program_hdr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_hdr_u *value)
{
	if (index >= TPR_PROGRAM_HDR_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_HDR_ADDRESS + \
				index * TPR_PROGRAM_HDR_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_hdr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_hdr_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_HDR_ADDRESS + \
				index * TPR_PROGRAM_HDR_INC,
				value->val);
}

sw_error_t
appe_tpr_program_result_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_result_u *value)
{
	if (index >= TPR_PROGRAM_RESULT_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_RESULT_ADDRESS + \
				index * TPR_PROGRAM_RESULT_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_result_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_result_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_RESULT_ADDRESS + \
				index * TPR_PROGRAM_RESULT_INC,
				value->val);
}

sw_error_t
appe_tpr_program_udf_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_ctrl_u *value)
{
	if (index >= TPR_PROGRAM_UDF_CTRL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_CTRL_ADDRESS + \
				index * TPR_PROGRAM_UDF_CTRL_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_udf_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_CTRL_ADDRESS + \
				index * TPR_PROGRAM_UDF_CTRL_INC,
				value->val);
}

sw_error_t
appe_tpr_program_udf_data_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_0_u *value)
{
	if (index >= TPR_PROGRAM_UDF_DATA_0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_DATA_0_ADDRESS + \
				index * TPR_PROGRAM_UDF_DATA_0_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_udf_data_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_DATA_0_ADDRESS + \
				index * TPR_PROGRAM_UDF_DATA_0_INC,
				value->val);
}

sw_error_t
appe_tpr_program_udf_data_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_1_u *value)
{
	if (index >= TPR_PROGRAM_UDF_DATA_1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_DATA_1_ADDRESS + \
				index * TPR_PROGRAM_UDF_DATA_1_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_udf_data_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_data_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_DATA_1_ADDRESS + \
				index * TPR_PROGRAM_UDF_DATA_1_INC,
				value->val);
}

sw_error_t
appe_tpr_program_udf_mask_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_0_u *value)
{
	if (index >= TPR_PROGRAM_UDF_MASK_0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_MASK_0_ADDRESS + \
				index * TPR_PROGRAM_UDF_MASK_0_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_udf_mask_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_MASK_0_ADDRESS + \
				index * TPR_PROGRAM_UDF_MASK_0_INC,
				value->val);
}

sw_error_t
appe_tpr_program_udf_mask_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_1_u *value)
{
	if (index >= TPR_PROGRAM_UDF_MASK_1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_MASK_1_ADDRESS + \
				index * TPR_PROGRAM_UDF_MASK_1_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_udf_mask_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_mask_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_MASK_1_ADDRESS + \
				index * TPR_PROGRAM_UDF_MASK_1_INC,
				value->val);
}

sw_error_t
appe_tpr_program_udf_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_action_u *value)
{
	if (index >= TPR_PROGRAM_UDF_ACTION_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_ACTION_ADDRESS + \
				index * TPR_PROGRAM_UDF_ACTION_INC,
				&value->val);
}

sw_error_t
appe_tpr_program_udf_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_program_udf_action_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PROGRAM_UDF_ACTION_ADDRESS + \
				index * TPR_PROGRAM_UDF_ACTION_INC,
				value->val);
}

#if 0
sw_error_t
appe_tpr_hdr_match_0_ip_ver_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_hdr_match_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ip_ver;
	return ret;
}

sw_error_t
appe_tpr_hdr_match_0_ip_ver_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_hdr_match_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ip_ver = value;
	ret = appe_tpr_hdr_match_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_hdr_match_0_cur_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_hdr_match_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cur_hdr_type;
	return ret;
}

sw_error_t
appe_tpr_hdr_match_0_cur_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_hdr_match_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cur_hdr_type = value;
	ret = appe_tpr_hdr_match_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_hdr_match_1_protocol_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_hdr_match_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.protocol;
	return ret;
}

sw_error_t
appe_tpr_hdr_match_1_protocol_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_hdr_match_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.protocol = value;
	ret = appe_tpr_hdr_match_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_hdr_match_2_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_hdr_match_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_2_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mask;
	return ret;
}

sw_error_t
appe_tpr_hdr_match_2_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_hdr_match_2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_hdr_match_2_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mask = value;
	ret = appe_tpr_hdr_match_2_set(dev_id, index, &reg_val);
	return ret;
}
#endif

sw_error_t
appe_tpr_program_hdr_hdr_type_map_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_hdr_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_hdr_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hdr_type_map;
	return ret;
}

sw_error_t
appe_tpr_program_hdr_hdr_type_map_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_hdr_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_hdr_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hdr_type_map = value;
	ret = appe_tpr_program_hdr_set(dev_id, index, &reg_val);
	return ret;
}

#if 0
sw_error_t
appe_tpr_program_result_len_unit_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	*value = reg_val.bf.len_unit;
	return ret;
}

sw_error_t
appe_tpr_program_result_len_unit_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.len_unit = value;
	ret = appe_tpr_program_result_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_result_next_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	*value = reg_val.bf.next_hdr_type;
	return ret;
}

sw_error_t
appe_tpr_program_result_next_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.next_hdr_type = value;
	ret = appe_tpr_program_result_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_result_next_hdr_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	*value = reg_val.bf.next_hdr_mode;
	return ret;
}

sw_error_t
appe_tpr_program_result_next_hdr_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.next_hdr_mode = value;
	ret = appe_tpr_program_result_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_result_hdr_len_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hdr_len;
	return ret;
}

sw_error_t
appe_tpr_program_result_hdr_len_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hdr_len = value;
	ret = appe_tpr_program_result_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_result_len_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	*value = reg_val.bf.len_mask;
	return ret;
}

sw_error_t
appe_tpr_program_result_len_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.len_mask = value;
	ret = appe_tpr_program_result_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_result_hdr_pos_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hdr_pos_mode;
	return ret;
}

sw_error_t
appe_tpr_program_result_hdr_pos_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_result_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_result_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hdr_pos_mode = value;
	ret = appe_tpr_program_result_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_ctrl_udf1_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf1_offset;
	return ret;
}

sw_error_t
appe_tpr_program_udf_ctrl_udf1_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf1_offset = value;
	ret = appe_tpr_program_udf_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_ctrl_udf0_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf0_offset;
	return ret;
}

sw_error_t
appe_tpr_program_udf_ctrl_udf0_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf0_offset = value;
	ret = appe_tpr_program_udf_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_ctrl_udf2_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_ctrl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf2_offset;
	return ret;
}

sw_error_t
appe_tpr_program_udf_ctrl_udf2_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_ctrl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_ctrl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf2_offset = value;
	ret = appe_tpr_program_udf_ctrl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_0_data1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.data1;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_0_data1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.data1 = value;
	ret = appe_tpr_program_udf_data_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_0_data0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.data0;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_0_data0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.data0 = value;
	ret = appe_tpr_program_udf_data_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_program_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.program_id;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_program_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.program_id = value;
	ret = appe_tpr_program_udf_data_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_data2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.data2;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_data2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.data2 = value;
	ret = appe_tpr_program_udf_data_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_udf1_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf1_valid;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_udf1_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf1_valid = value;
	ret = appe_tpr_program_udf_data_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_udf2_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf2_valid;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_udf2_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf2_valid = value;
	ret = appe_tpr_program_udf_data_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_udf0_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf0_valid;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_udf0_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf0_valid = value;
	ret = appe_tpr_program_udf_data_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_comp_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.comp_mode;
	return ret;
}

sw_error_t
appe_tpr_program_udf_data_1_comp_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_data_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_data_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.comp_mode = value;
	ret = appe_tpr_program_udf_data_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_0_mask1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_mask_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mask1;
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_0_mask1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_mask_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mask1 = value;
	ret = appe_tpr_program_udf_mask_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_0_mask0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_mask_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_0_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mask0;
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_0_mask0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_mask_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_0_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mask0 = value;
	ret = appe_tpr_program_udf_mask_0_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_udf2_valid_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf2_valid_mask;
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_udf2_valid_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf2_valid_mask = value;
	ret = appe_tpr_program_udf_mask_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_udf0_valid_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf0_valid_mask;
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_udf0_valid_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf0_valid_mask = value;
	ret = appe_tpr_program_udf_mask_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_udf1_valid_mask_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.udf1_valid_mask;
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_udf1_valid_mask_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.udf1_valid_mask = value;
	ret = appe_tpr_program_udf_mask_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_mask2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mask2;
	return ret;
}

sw_error_t
appe_tpr_program_udf_mask_1_mask2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_mask_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_mask_1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mask2 = value;
	ret = appe_tpr_program_udf_mask_1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	*value = reg_val.bf.next_hdr_type_valid;
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.next_hdr_type_valid = value;
	ret = appe_tpr_program_udf_action_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_exception_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	*value = reg_val.bf.exception_en;
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_exception_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.exception_en = value;
	ret = appe_tpr_program_udf_action_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	*value = reg_val.bf.next_hdr_type;
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_next_hdr_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.next_hdr_type = value;
	ret = appe_tpr_program_udf_action_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_hdr_len_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hdr_len_valid;
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_hdr_len_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hdr_len_valid = value;
	ret = appe_tpr_program_udf_action_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_hdr_len_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hdr_len;
	return ret;
}

sw_error_t
appe_tpr_program_udf_action_hdr_len_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_program_udf_action_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_program_udf_action_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hdr_len = value;
	ret = appe_tpr_program_udf_action_set(dev_id, index, &reg_val);
	return ret;
}
#endif
