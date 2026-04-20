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

#include "sw.h"
#include "hsl.h"
#include "hppe_reg_access.h"
#include "appe_policer_reg.h"
#include "appe_policer.h"
#include "hppe_policer_reg.h"
#include "hppe_policer.h"

sw_error_t
appe_in_meter_head_reg_get(
		a_uint32_t dev_id,
		union in_meter_head_reg_u *value)
{
	return hppe_reg_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + IN_METER_HEAD_REG_ADDRESS,
				&value->val);
}

sw_error_t
appe_in_meter_head_reg_set(
		a_uint32_t dev_id,
		union in_meter_head_reg_u *value)
{
	return hppe_reg_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + IN_METER_HEAD_REG_ADDRESS,
				value->val);
}

#ifndef IN_POLICER_MINI
sw_error_t
appe_dscp_remap_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union dscp_remap_tbl_u *value)
{
	if (index >= DSCP_REMAP_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + DSCP_REMAP_TBL_ADDRESS + \
				index * DSCP_REMAP_TBL_INC,
				&value->val);
}

sw_error_t
appe_dscp_remap_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union dscp_remap_tbl_u *value)
{
	return hppe_reg_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + DSCP_REMAP_TBL_ADDRESS + \
				index * DSCP_REMAP_TBL_INC,
				value->val);
}

sw_error_t
appe_in_meter_head_reg_mef10dot3_en_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_meter_head_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_in_meter_head_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.mef10dot3_en;
	return ret;
}

sw_error_t
appe_in_meter_head_reg_mef10dot3_en_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_meter_head_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_in_meter_head_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mef10dot3_en = value;
	ret = appe_in_meter_head_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_in_meter_head_reg_meter_ll_head_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_meter_head_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_in_meter_head_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.meter_ll_head;
	return ret;
}

sw_error_t
appe_in_meter_head_reg_meter_ll_head_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_meter_head_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_in_meter_head_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.meter_ll_head = value;
	ret = appe_in_meter_head_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_in_meter_head_reg_meter_ll_tail_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union in_meter_head_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_in_meter_head_reg_get(dev_id, &reg_val);
	*value = reg_val.bf.meter_ll_tail;
	return ret;
}

sw_error_t
appe_in_meter_head_reg_meter_ll_tail_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union in_meter_head_reg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_in_meter_head_reg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.meter_ll_tail = value;
	ret = appe_in_meter_head_reg_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_dscp_remap_tbl_dei_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union dscp_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_dscp_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.dei;
	return ret;
}

sw_error_t
appe_dscp_remap_tbl_dei_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union dscp_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_dscp_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.dei = value;
	ret = appe_dscp_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_dscp_remap_tbl_dscp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union dscp_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_dscp_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.dscp;
	return ret;
}

sw_error_t
appe_dscp_remap_tbl_dscp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union dscp_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_dscp_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.dscp = value;
	ret = appe_dscp_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_dscp_remap_tbl_pcp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union dscp_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_dscp_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.pcp;
	return ret;
}

sw_error_t
appe_dscp_remap_tbl_pcp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union dscp_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_dscp_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.pcp = value;
	ret = appe_dscp_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_chg_dscp_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.exceed_chg_dscp_cmd;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_chg_dscp_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.exceed_chg_dscp_cmd = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_dscp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.exceed_dscp;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_dscp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.exceed_dscp = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_chg_dscp_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.violate_chg_dscp_cmd;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_chg_dscp_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.violate_chg_dscp_cmd = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_dscp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.violate_dscp;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_dscp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.violate_dscp = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_remap_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.violate_remap_cmd;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_violate_remap_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.violate_remap_cmd = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_remap_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.exceed_remap_cmd;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_exceed_remap_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.exceed_remap_cmd = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_cir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cir_max_1 << 7 | \
		reg_val.bf.cir_max_0;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_cir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cir_max_1 = value >> 7;
	reg_val.bf.cir_max_0 = value & (((a_uint64_t)1<<7)-1);
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_eir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eir_max;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_eir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eir_max = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_end_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.grp_end;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_end_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.grp_end = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_cf_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.grp_cf;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_grp_cf_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.grp_cf = value;
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_nxt_ptr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.nxt_ptr_1 << 1 | \
		reg_val.bf.nxt_ptr_0;
	return ret;
}

sw_error_t
appe_in_acl_meter_cfg_tbl_nxt_ptr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union in_acl_meter_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_in_acl_meter_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.nxt_ptr_1 = value >> 1;
	reg_val.bf.nxt_ptr_0 = value & (((a_uint64_t)1<<1)-1);
	ret = hppe_in_acl_meter_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif

