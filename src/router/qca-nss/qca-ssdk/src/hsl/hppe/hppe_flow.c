/*
 * Copyright (c) 2016-2017, 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hppe_reg_access.h"
#include "hppe_flow_reg.h"
#include "hppe_flow.h"
#include "hppe_ip.h"

static a_uint32_t flow_cmd_id = 0;
static a_uint32_t flow_host_cmd_id = 0;

sw_error_t
hppe_in_flow_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_cnt_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + IN_FLOW_CNT_TBL_ADDRESS + \
				index * IN_FLOW_CNT_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_cnt_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + IN_FLOW_CNT_TBL_ADDRESS + \
				index * IN_FLOW_CNT_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_flow_ctrl0_get(
		a_uint32_t dev_id,
		union flow_ctrl0_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_CTRL0_ADDRESS,
				&value->val);
}

sw_error_t
hppe_flow_ctrl0_set(
		a_uint32_t dev_id,
		union flow_ctrl0_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_CTRL0_ADDRESS,
				value->val);
}

sw_error_t
hppe_flow_ctrl1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union flow_ctrl1_u *value)
{
	if (index >= FLOW_CTRL1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_CTRL1_ADDRESS + \
				index * FLOW_CTRL1_INC,
				&value->val);
}

sw_error_t
hppe_flow_ctrl1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union flow_ctrl1_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_CTRL1_ADDRESS + \
				index * FLOW_CTRL1_INC,
				value->val);
}

sw_error_t
hppe_in_flow_tbl_op_get(
		a_uint32_t dev_id,
		union in_flow_tbl_op_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_OP_ADDRESS,
				&value->val);
}

sw_error_t
hppe_in_flow_tbl_op_set(
		a_uint32_t dev_id,
		union in_flow_tbl_op_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_OP_ADDRESS,
				value->val);
}

sw_error_t
hppe_in_flow_host_tbl_op_get(
		a_uint32_t dev_id,
		union in_flow_host_tbl_op_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_HOST_TBL_OP_ADDRESS,
				&value->val);
}

sw_error_t
hppe_in_flow_host_tbl_op_set(
		a_uint32_t dev_id,
		union in_flow_host_tbl_op_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_HOST_TBL_OP_ADDRESS,
				value->val);
}

sw_error_t
hppe_in_flow_tbl_op_data_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	if (index > IN_FLOW_TBL_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_OP_DATA_ADDRESS +
				IN_FLOW_TBL_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_in_flow_tbl_op_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	if (index > IN_FLOW_TBL_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_OP_DATA_ADDRESS +
				IN_FLOW_TBL_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_flow_host_tbl_op_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	if (index >= FLOW_HOST_TBL_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_OP_DATA_ADDRESS +
				FLOW_HOST_TBL_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_flow_host_tbl_op_data_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	if (index >= FLOW_HOST_TBL_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_OP_DATA_ADDRESS +
				FLOW_HOST_TBL_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_in_flow_tbl_op_rslt_get(
		a_uint32_t dev_id,
		union in_flow_tbl_op_rslt_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_OP_RSLT_ADDRESS,
				&value->val);
}

sw_error_t
hppe_in_flow_tbl_op_rslt_set(
		a_uint32_t dev_id,
		union in_flow_tbl_op_rslt_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_flow_host_tbl_op_rslt_get(
		a_uint32_t dev_id,
		union flow_host_tbl_op_rslt_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_OP_RSLT_ADDRESS,
				&value->val);
}

sw_error_t
hppe_flow_host_tbl_op_rslt_set(
		a_uint32_t dev_id,
		union flow_host_tbl_op_rslt_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_rd_op_get(
		a_uint32_t dev_id,
		union in_flow_tbl_rd_op_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_OP_ADDRESS,
				&value->val);
}

sw_error_t
hppe_in_flow_tbl_rd_op_set(
		a_uint32_t dev_id,
		union in_flow_tbl_rd_op_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_OP_ADDRESS,
				value->val);
}

sw_error_t
hppe_in_flow_host_tbl_rd_op_get(
		a_uint32_t dev_id,
		union in_flow_host_tbl_rd_op_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_HOST_TBL_RD_OP_ADDRESS,
				&value->val);
}

sw_error_t
hppe_in_flow_host_tbl_rd_op_set(
		a_uint32_t dev_id,
		union in_flow_host_tbl_rd_op_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_HOST_TBL_RD_OP_ADDRESS,
				value->val);
}

sw_error_t
hppe_in_flow_tbl_rd_op_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	if (index >= IN_FLOW_TBL_RD_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_OP_DATA_ADDRESS +
				IN_FLOW_TBL_RD_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_in_flow_tbl_rd_op_data_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	if (index >= IN_FLOW_TBL_RD_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_OP_DATA_ADDRESS +
				IN_FLOW_TBL_RD_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_flow_host_tbl_rd_op_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	if (index >= FLOW_HOST_TBL_RD_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_RD_OP_DATA_ADDRESS +
				FLOW_HOST_TBL_RD_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_flow_host_tbl_rd_op_data_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	if (index >= FLOW_HOST_TBL_RD_OP_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_set(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_RD_OP_DATA_ADDRESS +
				FLOW_HOST_TBL_RD_OP_DATA_INC * index,
				value);
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_get(
		a_uint32_t dev_id,
		union in_flow_tbl_rd_op_rslt_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_OP_RSLT_ADDRESS,
				&value->val);
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_set(
		a_uint32_t dev_id,
		union in_flow_tbl_rd_op_rslt_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_flow_host_tbl_rd_op_rslt_get(
		a_uint32_t dev_id,
		union flow_host_tbl_rd_op_rslt_u *value)
{
	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_RD_OP_RSLT_ADDRESS,
				&value->val);
}

sw_error_t
hppe_flow_host_tbl_rd_op_rslt_set(
		a_uint32_t dev_id,
		union flow_host_tbl_rd_op_rslt_u *value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_rd_rslt_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	if (index >= IN_FLOW_TBL_RD_RSLT_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_RSLT_DATA_ADDRESS +
				IN_FLOW_TBL_RD_RSLT_DATA_INC * index,
				value);
}

sw_error_t
hppe_flow_host_tbl_rd_rslt_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	if (index >= FLOW_HOST_TBL_RD_RSLT_DATA_NUM)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				IPE_L3_BASE_ADDR + FLOW_HOST_TBL_RD_RSLT_DATA_ADDRESS +
				FLOW_HOST_TBL_RD_RSLT_DATA_INC * index,
				value);
}

sw_error_t
hppe_in_flow_3tuple_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_3tuple_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_3TUPLE_TBL_ADDRESS + \
				index * IN_FLOW_3TUPLE_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_3tuple_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_3tuple_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_3TUPLE_TBL_ADDRESS + \
				index * IN_FLOW_3TUPLE_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_ipv6_3tuple_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_ipv6_3tuple_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_IPV6_3TUPLE_TBL_ADDRESS + \
				index * IN_FLOW_IPV6_3TUPLE_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_ipv6_3tuple_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_ipv6_3tuple_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_IPV6_3TUPLE_TBL_ADDRESS + \
				index * IN_FLOW_IPV6_3TUPLE_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_ipv6_5tuple_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_ipv6_5tuple_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_IPV6_5TUPLE_TBL_ADDRESS + \
				index * IN_FLOW_IPV6_5TUPLE_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_ipv6_5tuple_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_ipv6_5tuple_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_IPV6_5TUPLE_TBL_ADDRESS + \
				index * IN_FLOW_IPV6_5TUPLE_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_ADDRESS + \
				index * IN_FLOW_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_in_flow_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union in_flow_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_ADDRESS + \
				index * IN_FLOW_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

#if defined(APPE)
sw_error_t
hppe_eg_flow_tree_map_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_flow_tree_map_tbl_u *value)
{
	if (index >= EG_FLOW_TREE_MAP_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_tbl_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_FLOW_TREE_MAP_TBL_ADDRESS + \
				index * EG_FLOW_TREE_MAP_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}

sw_error_t
hppe_eg_flow_tree_map_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_flow_tree_map_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_FLOW_TREE_MAP_TBL_ADDRESS + \
				index * EG_FLOW_TREE_MAP_TBL_INC,
				value->val,
				ARRAY_SIZE(value->val));
}
#elif defined(HPPE)
sw_error_t
hppe_eg_flow_tree_map_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_flow_tree_map_tbl_u *value)
{
	if (index >= EG_FLOW_TREE_MAP_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_FLOW_TREE_MAP_TBL_ADDRESS + \
				index * EG_FLOW_TREE_MAP_TBL_INC,
				&value->val);
}

sw_error_t
hppe_eg_flow_tree_map_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_flow_tree_map_tbl_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_FLOW_TREE_MAP_TBL_ADDRESS + \
				index * EG_FLOW_TREE_MAP_TBL_INC,
				value->val);
}
#endif

#if 0
sw_error_t
hppe_flow_ctrl0_flow_hash_mode_0_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_hash_mode_0;
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_hash_mode_0_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_hash_mode_0 = value;
	ret = hppe_flow_ctrl0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_age_timer_unit_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_age_timer_unit;
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_age_timer_unit_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_age_timer_unit = value;
	ret = hppe_flow_ctrl0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_hash_mode_1_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_hash_mode_1;
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_hash_mode_1_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_hash_mode_1 = value;
	ret = hppe_flow_ctrl0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_age_timer_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_age_timer;
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_age_timer_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_age_timer = value;
	ret = hppe_flow_ctrl0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_en_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_en;
	return ret;
}

sw_error_t
hppe_flow_ctrl0_flow_en_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union flow_ctrl0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_en = value;
	ret = hppe_flow_ctrl0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_frag_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl1_frag_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_frag_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl1_frag_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_key_sel_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl4_key_sel;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_key_sel_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl4_key_sel = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_key_sel_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl1_key_sel;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_key_sel_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl1_key_sel = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_frag_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl0_frag_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_frag_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl0_frag_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_miss_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl0_miss_action;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_miss_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl0_miss_action = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_key_sel_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl0_key_sel;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_key_sel_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl0_key_sel = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl1_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl1_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl0_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl0_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_tcp_special_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl2_tcp_special;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_tcp_special_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl2_tcp_special = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_tcp_special_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl4_tcp_special;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_tcp_special_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl4_tcp_special = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_frag_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl3_frag_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_frag_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl3_frag_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl3_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl3_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_tcp_special_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl3_tcp_special;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_tcp_special_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl3_tcp_special = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_miss_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl1_miss_action;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_miss_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl1_miss_action = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_frag_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl4_frag_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_frag_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl4_frag_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_tcp_special_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl1_tcp_special;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl1_tcp_special_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl1_tcp_special = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_key_sel_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl2_key_sel;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_key_sel_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl2_key_sel = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_miss_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl2_miss_action;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_miss_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl2_miss_action = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl2_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl2_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl4_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl4_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_key_sel_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl3_key_sel;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_key_sel_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl3_key_sel = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_tcp_special_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl0_tcp_special;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl0_tcp_special_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl0_tcp_special = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_frag_bypass_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl2_frag_bypass;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl2_frag_bypass_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl2_frag_bypass = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_miss_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl3_miss_action;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl3_miss_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl3_miss_action = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_miss_action_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.flow_ctl4_miss_action;
	return ret;
}

sw_error_t
hppe_flow_ctrl1_flow_ctl4_miss_action_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union flow_ctrl1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_ctrl1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.flow_ctl4_miss_action = value;
	ret = hppe_flow_ctrl1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.entry_index;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.entry_index = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_cmd_id_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.cmd_id;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_cmd_id_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cmd_id = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_byp_rslt_en_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.byp_rslt_en;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_byp_rslt_en_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.byp_rslt_en = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_mode_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_mode;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_mode_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_mode = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_type_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_type;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_type_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_type = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_host_en_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_host_en;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_host_en_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_host_en = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_result_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_result;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_op_result_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_result = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_busy_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.busy;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_busy_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.busy = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_hash_block_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.hash_block_bitmap;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_hash_block_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hash_block_bitmap = value;
	ret = hppe_in_flow_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_op_host_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_host_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.host_entry_index;
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_op_host_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_host_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.host_entry_index = value;
	ret = hppe_in_flow_host_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_op_hash_block_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_host_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_op_get(dev_id, &reg_val);
	*value = reg_val.bf.hash_block_bitmap;
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_op_hash_block_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_host_tbl_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hash_block_bitmap = value;
	ret = hppe_in_flow_host_tbl_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_op_rslt_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.op_rslt;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_op_rslt_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_valid_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.valid_cnt;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_valid_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_flow_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_entry_index;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_flow_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_cmd_id_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.cmd_id;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_op_rslt_cmd_id_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_flow_host_tbl_op_rslt_host_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_rd_op_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.entry_index;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.entry_index = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_cmd_id_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.cmd_id;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_cmd_id_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cmd_id = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_byp_rslt_en_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.byp_rslt_en;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_byp_rslt_en_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.byp_rslt_en = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_mode_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_mode;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_mode_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_mode = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_type_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_type;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_type_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_type = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_host_en_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_host_en;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_host_en_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_host_en = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_result_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.op_result;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_op_result_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.op_result = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_busy_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.busy;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_busy_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.busy = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_hash_block_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.hash_block_bitmap;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_hash_block_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hash_block_bitmap = value;
	ret = hppe_in_flow_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_rd_op_host_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_host_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.host_entry_index;
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_rd_op_host_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_host_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.host_entry_index = value;
	ret = hppe_in_flow_host_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_rd_op_hash_block_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_host_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_rd_op_get(dev_id, &reg_val);
	*value = reg_val.bf.hash_block_bitmap;
	return ret;
}

sw_error_t
hppe_in_flow_host_tbl_rd_op_hash_block_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_flow_host_tbl_rd_op_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_host_tbl_rd_op_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hash_block_bitmap = value;
	ret = hppe_in_flow_host_tbl_rd_op_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_op_rslt_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.op_rslt;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_op_rslt_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_valid_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.valid_cnt;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_valid_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_flow_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.flow_entry_index;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_flow_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_cmd_id_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_flow_tbl_rd_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.cmd_id;
	return ret;
}

sw_error_t
hppe_in_flow_tbl_rd_op_rslt_cmd_id_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_flow_host_tbl_rd_op_rslt_host_entry_index_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	return SW_NOT_SUPPORTED;
}

sw_error_t
hppe_eg_flow_tree_map_tbl_tree_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_flow_tree_map_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_eg_flow_tree_map_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tree_id;
	return ret;
}

sw_error_t
hppe_eg_flow_tree_map_tbl_tree_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_flow_tree_map_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_eg_flow_tree_map_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tree_id = value;
	ret = hppe_eg_flow_tree_map_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_cnt_tbl_hit_byte_counter_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union in_flow_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_cnt_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.hit_byte_counter_1 << 32 | \
		reg_val.bf.hit_byte_counter_0;
	return ret;
}

sw_error_t
hppe_in_flow_cnt_tbl_hit_byte_counter_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union in_flow_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hit_byte_counter_1 = value >> 32;
	reg_val.bf.hit_byte_counter_0 = value & (((a_uint64_t)1<<32)-1);
	ret = hppe_in_flow_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
hppe_in_flow_cnt_tbl_hit_pkt_counter_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_flow_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_cnt_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.hit_pkt_counter;
	return ret;
}

sw_error_t
hppe_in_flow_cnt_tbl_hit_pkt_counter_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_flow_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_flow_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.hit_pkt_counter = value;
	ret = hppe_in_flow_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif

sw_error_t
hppe_flow_host_tbl_op_rslt_host_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_host_tbl_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_host_tbl_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.host_entry_index;
	return ret;
}

sw_error_t
hppe_flow_host_tbl_rd_op_rslt_host_entry_index_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union flow_host_tbl_rd_op_rslt_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_flow_host_tbl_rd_op_rslt_get(dev_id, &reg_val);
	*value = reg_val.bf.host_entry_index;
	return ret;
}

sw_error_t
hppe_flow_get_common(
		a_uint32_t dev_id,
		a_uint32_t op_mode,
		a_uint32_t *index,
		a_uint32_t *data,
		a_uint32_t num)
{
	union in_flow_tbl_rd_op_u op;
	union in_flow_tbl_rd_op_rslt_u result;
	a_uint32_t i = 0x100;
	sw_error_t rv;

	op.bf.cmd_id = flow_cmd_id;
	flow_cmd_id++;
	op.bf.byp_rslt_en = 0;
	op.bf.op_type = 2;
	op.bf.hash_block_bitmap = 3;
	op.bf.op_mode = op_mode;
	op.bf.entry_index = *index;
	op.bf.op_host_en = 0;

	rv = hppe_in_flow_tbl_rd_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;
	rv = hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &result);
	if (SW_OK != rv)
		return rv;
	while (!result.bf.valid_cnt && --i) {
		hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &result);
	}
	if (i == 0)
		return SW_BUSY;
	if (result.bf.op_rslt == 0) {
		hppe_reg_tbl_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_RSLT_DATA_ADDRESS,
				data, num);
		*index = result.bf.flow_entry_index;
		return SW_OK;
	}
	else
		return SW_FAIL;
	
}


sw_error_t
hppe_flow_flush_common(a_uint32_t dev_id)
{
	union in_flow_tbl_op_u op;
	union in_flow_tbl_op_rslt_u result;
	a_uint32_t i = 0x100 * 50;
	sw_error_t rv;

	op.bf.cmd_id = flow_cmd_id;
	flow_cmd_id++;
	op.bf.byp_rslt_en = 0;
	op.bf.op_type = 3;
	op.bf.hash_block_bitmap = 3;
	op.bf.op_mode = 0;
	op.bf.op_host_en = 0;

	rv = hppe_in_flow_tbl_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;
	rv = hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	if (SW_OK != rv)
		return rv;
	while (!result.bf.valid_cnt && --i) {
		hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	}
	if (i == 0)
		return SW_BUSY;
	if (result.bf.op_rslt == 0)
		return SW_OK;
	else
		return SW_FAIL;
	
	
}

sw_error_t
hppe_flow_op_common(
		a_uint32_t dev_id,
		a_uint32_t op_type,
		a_uint32_t op_mode,
		a_uint32_t *index)
{
	union in_flow_tbl_op_u op;
	union in_flow_tbl_op_rslt_u result;
	a_uint32_t i = 0x100;
	sw_error_t rv;

	op.bf.cmd_id = flow_cmd_id;
	flow_cmd_id++;
	op.bf.byp_rslt_en = 0;
	op.bf.op_type = op_type;
	op.bf.hash_block_bitmap = 3;
	op.bf.op_mode = op_mode;
	op.bf.entry_index = *index;
	op.bf.op_host_en = 0;

	rv = hppe_in_flow_tbl_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;
	rv = hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	if (SW_OK != rv)
		return rv;
	while (!result.bf.valid_cnt && --i) {
		hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	}
	if (i == 0)
		return SW_BUSY;
	if (result.bf.op_rslt == 0) {
		*index = result.bf.flow_entry_index;
		return SW_OK;
	}
	else
		return SW_FAIL;
	
}

sw_error_t
hppe_flow_host_get_common(
		a_uint32_t dev_id,
		a_uint32_t op_mode,
		a_uint32_t *index,
		a_uint32_t *data,
		a_uint32_t num)
{
	union in_flow_tbl_rd_op_u op;
	union in_flow_tbl_rd_op_rslt_u result;
	a_uint32_t i = 0x100;
	sw_error_t rv;

	op.bf.cmd_id = flow_host_cmd_id;
	flow_host_cmd_id++;
	op.bf.byp_rslt_en = 0;
	op.bf.op_type = 2;
	op.bf.hash_block_bitmap = 3;
	op.bf.op_mode = op_mode;
	op.bf.entry_index = *index;
	op.bf.op_host_en = 1;

	rv = hppe_in_flow_tbl_rd_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;
	rv = hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &result);
	if (SW_OK != rv)
		return rv;
	while (!result.bf.valid_cnt && --i) {
		hppe_in_flow_tbl_rd_op_rslt_get(dev_id, &result);
	}
	if (i == 0)
		return SW_BUSY;

	if (result.bf.op_rslt == 0) {
		hppe_reg_tbl_get(
				dev_id,
				IPE_L3_BASE_ADDR + IN_FLOW_TBL_RD_RSLT_DATA_ADDRESS,
				data, num);
		*index = result.bf.flow_entry_index;
		return SW_OK;
	}
	else
		return SW_FAIL;
}

sw_error_t
hppe_flow_host_flush_common(a_uint32_t dev_id)
{
	union in_flow_tbl_op_u op;
	union in_flow_tbl_op_rslt_u result;
	a_uint32_t i = 0x100 * 50;
	sw_error_t rv;

	op.bf.cmd_id = flow_host_cmd_id;
	flow_host_cmd_id++;
	op.bf.byp_rslt_en = 0;
	op.bf.op_type = 3;
	op.bf.hash_block_bitmap = 3;
	op.bf.op_mode = 0;
	op.bf.op_host_en = 1;

	rv = hppe_in_flow_tbl_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;
	rv = hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	if (SW_OK != rv)
		return rv;
	while (!result.bf.valid_cnt && --i) {
		hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	}
	if (i == 0)
		return SW_BUSY;
	if (result.bf.op_rslt == 0)
		return SW_OK;
	else
		return SW_FAIL;
}

sw_error_t
hppe_flow_host_op_both_common(
		a_uint32_t dev_id,
		a_uint32_t op_type,
		a_uint32_t op_mode,
		a_uint32_t *index)
{
	union in_flow_tbl_op_u op;
	union in_flow_tbl_op_rslt_u result;
	a_uint32_t i = 0x100;
	sw_error_t rv;

	op.bf.cmd_id = flow_host_cmd_id;
	flow_host_cmd_id++;
	op.bf.byp_rslt_en = 0;
	op.bf.op_type = op_type;
	op.bf.hash_block_bitmap = 3;
	op.bf.op_mode = op_mode;
	op.bf.entry_index = *index;
	op.bf.op_host_en = 1;

	rv = hppe_in_flow_tbl_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;
	rv = hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	if (SW_OK != rv)
		return rv;
	while (!result.bf.valid_cnt && --i) {
		hppe_in_flow_tbl_op_rslt_get(dev_id, &result);
	}
	if (i == 0)
		return SW_BUSY;
	if (result.bf.op_rslt == 0) {
		*index = result.bf.flow_entry_index;
		return SW_OK;
	}
	else
		return SW_FAIL;
}

sw_error_t
hppe_flow_entry_op(
		a_uint32_t dev_id,
		a_uint32_t op_type, a_uint32_t op_mode,
		a_uint32_t *index, a_uint32_t *entry, a_uint32_t entry_size, a_bool_t flow_host)
{
	a_uint32_t i = 0;

	if (op_mode == HASH_MODE) {
		while (i < entry_size) {
			hppe_in_flow_tbl_op_data_set(dev_id, i, entry[i]);
			i++;
		}
	}

	if (flow_host)
		return hppe_flow_host_op_both_common(dev_id, op_type, op_mode, index);

	return hppe_flow_op_common(dev_id, op_type, op_mode, index);
}

sw_error_t
hppe_flow_entry_host_op_ipv4_5tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv4_3tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv6_5tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_5tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv6_3tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv4_5tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv4_3tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv6_5tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_5tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv6_3tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, a_uint32_t *entry, a_uint32_t entry_size, a_bool_t flow_host)
{
	a_uint32_t i = 0;
	if (op_mode == HASH_MODE) {
		while (i < entry_size) {
			hppe_in_flow_tbl_rd_op_data_set(dev_id, i, entry[i]);
			i++;
		}
	}

	if (flow_host)
		return hppe_flow_host_get_common(dev_id, op_mode, index, entry, entry_size);

	return hppe_flow_get_common(dev_id, op_mode, index, entry, entry_size);
}


sw_error_t
hppe_flow_entry_host_op_ipv4_5tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv4_3tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv6_5tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_5tuple_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_entry_host_op_ipv6_3tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_TRUE);
}

sw_error_t
hppe_flow_host_data_op_common(
		a_uint32_t dev_id,
		a_uint32_t op_type,
		a_uint32_t op_mode,
		a_uint32_t *index)
{
	union in_flow_host_tbl_op_u op;
	sw_error_t rv;

	op.bf.hash_block_bitmap = 3;
	op.bf.host_entry_index = *index;

	rv = hppe_in_flow_host_tbl_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;

	return SW_OK;
}

sw_error_t
hppe_flow_host_data_rd_op_common(
		a_uint32_t dev_id,
		a_uint32_t op_type,
		a_uint32_t op_mode,
		a_uint32_t *index)
{
	union in_flow_host_tbl_rd_op_u op;
	sw_error_t rv;

	op.bf.hash_block_bitmap = 3;
	op.bf.host_entry_index = *index;

	rv = hppe_in_flow_host_tbl_rd_op_set(dev_id, &op);
	if (SW_OK != rv)
		return rv;

	return SW_OK;
}

sw_error_t
hppe_flow_host_entry_op(
		a_uint32_t dev_id,
		a_uint32_t op_type, a_uint32_t op_mode,
		a_uint32_t *index, a_uint32_t *entry, a_uint32_t entry_size)
{
	a_uint32_t i = 0;

	if (op_mode == HASH_MODE) {
		while (i < entry_size) {
			hppe_flow_host_tbl_op_data_set(dev_id, i, entry[i]);
			i++;
		}
	}

	return hppe_flow_host_data_op_common(dev_id, op_type, op_mode, index);
}

sw_error_t
hppe_flow_host_ipv4_data_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_tbl_u *entry)
{
	return hppe_flow_host_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val));
}

sw_error_t
hppe_flow_host_ipv6_data_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_ipv6_tbl_u *entry)
{
	return hppe_flow_host_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val));
}

sw_error_t
hppe_flow_host_entry_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, a_uint32_t *entry, a_uint32_t entry_size)
{
	a_uint32_t i = 0;

	if (op_mode == HASH_MODE) {
		while (i < entry_size) {
			hppe_flow_host_tbl_rd_op_data_set(dev_id, i, entry[i]);
			i++;
		}
	}

	return hppe_flow_host_data_rd_op_common(dev_id, OP_ADD, op_mode, index);
}

sw_error_t
hppe_flow_host_ipv4_data_rd_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_tbl_u *entry)
{
	return hppe_flow_host_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val));
}

sw_error_t
hppe_flow_host_ipv6_data_rd_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_ipv6_tbl_u *entry)
{
	return hppe_flow_host_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val));
}

sw_error_t
hppe_flow_host_ipv4_data_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_tbl_u *entry)
{
	a_uint32_t i = 0;

	while (i < ARRAY_SIZE(entry->val)) {
		hppe_flow_host_tbl_rd_rslt_data_get(dev_id, i, &(entry->val[i]));
		i++;
	}
	return SW_OK;
}

sw_error_t
hppe_flow_host_ipv6_data_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_ipv6_tbl_u *entry)
{
	a_uint32_t i = 0;

	while (i < ARRAY_SIZE(entry->val)) {
		hppe_flow_host_tbl_rd_rslt_data_get(dev_id, i, &(entry->val[i]));
		i++;
	}
	return SW_OK;
}

sw_error_t
hppe_flow_host_ipv4_data_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_tbl_u *entry)
{
	return hppe_flow_host_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val));
}

sw_error_t
hppe_flow_host_ipv6_data_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union host_ipv6_tbl_u *entry)
{
	return hppe_flow_host_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val));
}

sw_error_t
hppe_flow_ipv4_5tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv4_3tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv6_5tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_5tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv6_3tuple_add(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_ADD, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv4_5tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv4_3tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv6_5tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_5tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv6_3tuple_del(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_op(dev_id, OP_DEL, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv4_5tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv4_3tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv6_5tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_5tuple_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}

sw_error_t
hppe_flow_ipv6_3tuple_get(
		a_uint32_t dev_id, a_uint32_t op_mode,
		a_uint32_t *index, union in_flow_ipv6_3tuple_tbl_u *entry)
{
	return hppe_flow_entry_get(dev_id, op_mode, index,
			entry->val, ARRAY_SIZE(entry->val), A_FALSE);
}
