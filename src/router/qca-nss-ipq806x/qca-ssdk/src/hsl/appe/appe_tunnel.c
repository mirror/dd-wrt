/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "appe_tunnel_reg.h"
#include "appe_tunnel.h"

sw_error_t
appe_tpr_udf_ctrl_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_udf_ctrl_0_u *value)
{
	if (index >= TPR_UDF_CTRL_0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_UDF_CTRL_0_ADDRESS + \
				index * TPR_UDF_CTRL_0_INC,
				&value->val);
}

sw_error_t
appe_tpr_udf_ctrl_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_udf_ctrl_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_UDF_CTRL_0_ADDRESS + \
				index * TPR_UDF_CTRL_0_INC,
				value->val);
}

sw_error_t
appe_tpr_udf_profile_base_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_udf_profile_base_u *value)
{
	if (index >= TPR_UDF_PROFILE_BASE_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_UDF_PROFILE_BASE_ADDRESS + \
				index * TPR_UDF_PROFILE_BASE_INC,
				&value->val);
}

sw_error_t
appe_tpr_udf_profile_base_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_udf_profile_base_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_UDF_PROFILE_BASE_ADDRESS + \
				index * TPR_UDF_PROFILE_BASE_INC,
				value->val);
}

sw_error_t
appe_tpr_udf_profile_offset_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_udf_profile_offset_u *value)
{
	if (index >= TPR_UDF_PROFILE_OFFSET_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_UDF_PROFILE_OFFSET_ADDRESS + \
				index * TPR_UDF_PROFILE_OFFSET_INC,
				&value->val);
}

sw_error_t
appe_tpr_udf_profile_offset_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_udf_profile_offset_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_UDF_PROFILE_OFFSET_ADDRESS + \
				index * TPR_UDF_PROFILE_OFFSET_INC,
				value->val);
}

sw_error_t
appe_tl_tbl_op_get(
		a_uint32_t dev_id,
		union tl_tbl_op_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_set(
		a_uint32_t dev_id,
		union tl_tbl_op_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data0_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data0_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA0_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data0_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA0_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data1_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data1_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA1_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data1_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA1_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data2_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data2_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA2_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data2_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data2_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA2_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data3_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data3_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA3_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data3_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data3_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA3_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data4_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data4_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA4_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data4_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data4_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA4_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data5_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data5_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA5_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data5_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data5_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA5_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data6_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data6_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA6_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data6_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data6_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA6_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data7_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data7_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA7_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data7_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data7_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA7_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data8_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data8_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA8_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data8_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data8_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA8_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data9_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data9_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA9_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data9_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data9_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA9_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data10_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data10_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA10_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data10_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data10_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA10_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data11_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data11_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA11_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data11_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data11_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA11_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data12_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data12_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA12_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data12_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data12_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA12_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data13_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data13_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA13_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data13_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data13_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA13_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data14_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data14_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA14_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data14_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data14_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA14_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data15_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data15_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA15_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data15_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data15_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA15_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_op_data16_get(
		a_uint32_t dev_id,
		union tl_tbl_op_data16_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA16_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_op_data16_set(
		a_uint32_t dev_id,
		union tl_tbl_op_data16_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_OP_DATA16_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data0_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data0_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA0_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data0_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA0_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data1_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data1_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA1_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data1_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA1_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data2_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data2_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA2_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data2_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data2_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA2_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data3_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data3_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA3_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data3_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data3_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA3_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data4_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data4_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA4_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data4_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data4_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA4_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data5_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data5_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA5_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data5_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data5_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA5_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data6_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data6_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA6_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data6_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data6_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA6_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data7_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data7_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA7_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data7_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data7_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA7_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data8_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data8_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA8_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data8_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data8_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA8_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data9_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data9_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA9_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data9_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data9_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA9_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data10_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data10_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA10_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data10_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data10_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA10_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data11_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data11_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA11_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data11_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data11_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA11_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data12_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data12_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA12_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data12_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data12_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA12_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data13_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data13_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA13_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data13_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data13_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA13_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data14_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data14_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA14_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data14_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data14_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA14_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data15_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data15_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA15_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data15_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data15_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA15_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data16_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data16_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA16_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_data16_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_data16_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_DATA16_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_op_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_op_set(
		a_uint32_t dev_id,
		union tl_tbl_rd_op_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_OP_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data0_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data0_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA0_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data1_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data1_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA1_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data2_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data2_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA2_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data3_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data3_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA3_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data4_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data4_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA4_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data5_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data5_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA5_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data6_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data6_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA6_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data7_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data7_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA7_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data8_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data8_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA8_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data9_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data9_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA9_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data10_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data10_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA10_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data11_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data11_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA11_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data12_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data12_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA12_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data13_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data13_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA13_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data14_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data14_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA14_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data15_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data15_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA15_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_tbl_rd_rslt_data16_get(
		a_uint32_t dev_id,
		union tl_tbl_rd_rslt_data16_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_RD_RSLT_DATA16_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_ctrl_get(
		a_uint32_t dev_id,
		union tl_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_CTRL_ADDRESS,
				&value->val);
}

sw_error_t
appe_tl_ctrl_set(
		a_uint32_t dev_id,
		union tl_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_CTRL_ADDRESS,
				value->val);
}

sw_error_t
appe_tl_l3_if_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_l3_if_tbl_u *value)
{
	if (index >= TL_L3_IF_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_L3_IF_TBL_ADDRESS + \
				index * TL_L3_IF_TBL_INC,
				&value->val);
}

sw_error_t
appe_tl_l3_if_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_l3_if_tbl_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_L3_IF_TBL_ADDRESS + \
				index * TL_L3_IF_TBL_INC,
				value->val);
}

sw_error_t
appe_tl_key_gen_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_key_gen_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_KEY_GEN_ADDRESS + \
				index * TL_KEY_GEN_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_key_gen_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_key_gen_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_KEY_GEN_ADDRESS + \
				index * TL_KEY_GEN_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_ADDRESS + \
				index * TL_TBL_INC,
				value->val,
				14);
}

sw_error_t
appe_tl_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_TBL_ADDRESS + \
				index * TL_TBL_INC,
				value->val,
				14);
}

sw_error_t
appe_eg_ipv4_hdr_ctrl_get(
		a_uint32_t dev_id,
		union eg_ipv4_hdr_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_IPV4_HDR_CTRL_ADDRESS,
				&value->val);
}

sw_error_t
appe_eg_ipv4_hdr_ctrl_set(
		a_uint32_t dev_id,
		union eg_ipv4_hdr_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_IPV4_HDR_CTRL_ADDRESS,
				value->val);
}

sw_error_t
appe_eg_udp_entropy_ctrl_get(
		a_uint32_t dev_id,
		union eg_udp_entropy_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_UDP_ENTROPY_CTRL_ADDRESS,
				&value->val);
}

sw_error_t
appe_eg_udp_entropy_ctrl_set(
		a_uint32_t dev_id,
		union eg_udp_entropy_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_UDP_ENTROPY_CTRL_ADDRESS,
				value->val);
}

#ifndef IN_TUNNEL_MINI
sw_error_t
appe_ecn_profile_get(
		a_uint32_t dev_id,
		union ecn_profile_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + ECN_PROFILE_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_profile_set(
		a_uint32_t dev_id,
		union ecn_profile_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + ECN_PROFILE_ADDRESS,
				value->val);
}
#endif

sw_error_t
appe_eg_proto_mapping0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_proto_mapping0_u *value)
{
	if (index >= EG_PROTO_MAPPING0_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_PROTO_MAPPING0_ADDRESS + \
				index * EG_PROTO_MAPPING0_INC,
				&value->val);
}

sw_error_t
appe_eg_proto_mapping0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_proto_mapping0_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_PROTO_MAPPING0_ADDRESS + \
				index * EG_PROTO_MAPPING0_INC,
				value->val);
}

sw_error_t
appe_eg_proto_mapping1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_proto_mapping1_u *value)
{
	if (index >= EG_PROTO_MAPPING1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_PROTO_MAPPING1_ADDRESS + \
				index * EG_PROTO_MAPPING1_INC,
				&value->val);
}

sw_error_t
appe_eg_proto_mapping1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_proto_mapping1_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_PROTO_MAPPING1_ADDRESS + \
				index * EG_PROTO_MAPPING1_INC,
				value->val);
}

sw_error_t
appe_dbg_addr_get(
		a_uint32_t dev_id,
		union dbg_addr_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + DBG_ADDR_ADDRESS,
				&value->val);
}

sw_error_t
appe_dbg_addr_set(
		a_uint32_t dev_id,
		union dbg_addr_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + DBG_ADDR_ADDRESS,
				value->val);
}

sw_error_t
appe_dbg_data_get(
		a_uint32_t dev_id,
		union dbg_data_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + DBG_DATA_ADDRESS,
				&value->val);
}

sw_error_t
appe_eg_header_data_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_header_data_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_HEADER_DATA_ADDRESS + \
				index * EG_HEADER_DATA_INC,
				value->val,
				32);
}

sw_error_t
appe_eg_header_data_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_header_data_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_HEADER_DATA_ADDRESS + \
				index * EG_HEADER_DATA_INC,
				value->val,
				32);
}

sw_error_t
appe_eg_xlat_tun_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_xlat_tun_ctrl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_XLAT_TUN_CTRL_ADDRESS + \
				index * EG_XLAT_TUN_CTRL_INC,
				value->val,
				3);
}

sw_error_t
appe_eg_xlat_tun_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_xlat_tun_ctrl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_XLAT_TUN_CTRL_ADDRESS + \
				index * EG_XLAT_TUN_CTRL_INC,
				value->val,
				3);
}

sw_error_t
appe_eg_edit_rule_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_edit_rule_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_EDIT_RULE_ADDRESS + \
				index * EG_EDIT_RULE_INC,
				value->val,
				3);
}

sw_error_t
appe_eg_edit_rule_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_edit_rule_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_EDIT_RULE_ADDRESS + \
				index * EG_EDIT_RULE_INC,
				value->val,
				3);
}

sw_error_t
appe_tl_tbl_decap_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf0.decap_en;
	return ret;
}

sw_error_t
appe_tl_tbl_decap_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf0.decap_en = value;
	ret = appe_tl_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tl_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_cnt_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + TL_CNT_TBL_ADDRESS + \
				index * TL_CNT_TBL_INC,
				value->val,
				sizeof(union tl_cnt_tbl_u) / sizeof(a_uint32_t));
}

sw_error_t
appe_tl_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_cnt_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + TL_CNT_TBL_ADDRESS + \
				index * TL_CNT_TBL_INC,
				value->val,
				sizeof(union tl_cnt_tbl_u) / sizeof(a_uint32_t));
}

sw_error_t
appe_tl_port_vp_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_port_vp_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_PORT_VP_TBL_ADDRESS + \
				index * TL_PORT_VP_TBL_INC,
				value->val,
				sizeof(union tl_port_vp_tbl_u) / sizeof(a_uint32_t));
}

sw_error_t
appe_tl_port_vp_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_port_vp_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_PORT_VP_TBL_ADDRESS + \
				index * TL_PORT_VP_TBL_INC,
				value->val,
				sizeof(union tl_port_vp_tbl_u) / sizeof(a_uint32_t));
}

sw_error_t
appe_tl_port_vp_tbl_pre_ipo_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.pre_ipo_profile;
	return ret;
}

sw_error_t
appe_tl_port_vp_tbl_pre_ipo_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.pre_ipo_profile = value;
	ret = appe_tl_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

#ifndef IN_TUNNEL_MINI
sw_error_t
appe_tl_vlan_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_vlan_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_VLAN_TBL_ADDRESS + \
				index * TL_VLAN_TBL_INC,
				value->val,
				sizeof(union tl_vlan_tbl_u) / sizeof(a_uint32_t));
}

sw_error_t
appe_tl_vlan_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_vlan_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_VLAN_TBL_ADDRESS + \
				index * TL_VLAN_TBL_INC,
				value->val,
				sizeof(union tl_vlan_tbl_u) / sizeof(a_uint32_t));
}

sw_error_t
appe_ecn_map_mode0_0_get(
		a_uint32_t dev_id,
		union ecn_map_mode0_0_u *value)
{
	return hppe_reg_get(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE0_0_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_map_mode0_0_set(
		a_uint32_t dev_id,
		union ecn_map_mode0_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE0_0_ADDRESS,
				value->val);
}

sw_error_t
appe_ecn_map_mode0_1_get(
		a_uint32_t dev_id,
		union ecn_map_mode0_1_u *value)
{
	return hppe_reg_get(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE0_1_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_map_mode0_1_set(
		a_uint32_t dev_id,
		union ecn_map_mode0_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE0_1_ADDRESS,
				value->val);
}

sw_error_t
appe_ecn_map_mode1_0_get(
		a_uint32_t dev_id,
		union ecn_map_mode1_0_u *value)
{
	return hppe_reg_get(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE1_0_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_map_mode1_0_set(
		a_uint32_t dev_id,
		union ecn_map_mode1_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE1_0_ADDRESS,
				value->val);
}

sw_error_t
appe_ecn_map_mode1_1_get(
		a_uint32_t dev_id,
		union ecn_map_mode1_1_u *value)
{
	return hppe_reg_get(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE1_1_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_map_mode1_1_set(
		a_uint32_t dev_id,
		union ecn_map_mode1_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE1_1_ADDRESS,
				value->val);
}

sw_error_t
appe_ecn_map_mode2_0_get(
		a_uint32_t dev_id,
		union ecn_map_mode2_0_u *value)
{
	return hppe_reg_get(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE2_0_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_map_mode2_0_set(
		a_uint32_t dev_id,
		union ecn_map_mode2_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE2_0_ADDRESS,
				value->val);
}

sw_error_t
appe_ecn_map_mode2_1_get(
		a_uint32_t dev_id,
		union ecn_map_mode2_1_u *value)
{
	return hppe_reg_get(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE2_1_ADDRESS,
				&value->val);
}

sw_error_t
appe_ecn_map_mode2_1_set(
		a_uint32_t dev_id,
		union ecn_map_mode2_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				PRE_IPO_CSR_BASE_ADDR + ECN_MAP_MODE2_1_ADDRESS,
				value->val);
}
#endif
