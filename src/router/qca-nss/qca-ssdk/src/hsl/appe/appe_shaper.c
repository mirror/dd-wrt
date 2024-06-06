/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
#include "appe_shaper_reg.h"
#include "appe_shaper.h"
#include "hppe_shaper_reg.h"
#include "hppe_shaper.h"

sw_error_t
appe_min_max_mode_cfg_get(
		a_uint32_t dev_id,
		union min_max_mode_cfg_u *value)
{
	return hppe_reg_get(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + MIN_MAX_MODE_CFG_ADDRESS,
				&value->val);
}

sw_error_t
appe_min_max_mode_cfg_set(
		a_uint32_t dev_id,
		union min_max_mode_cfg_u *value)
{
	return hppe_reg_set(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + MIN_MAX_MODE_CFG_ADDRESS,
				value->val);
}

sw_error_t
appe_eco_reserve_0_get(
		a_uint32_t dev_id,
		union eco_reserve_0_u *value)
{
	return hppe_reg_get(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + ECO_RESERVE_0_ADDRESS,
				&value->val);
}

sw_error_t
appe_eco_reserve_0_set(
		a_uint32_t dev_id,
		union eco_reserve_0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + ECO_RESERVE_0_ADDRESS,
				value->val);
}

sw_error_t
appe_eco_reserve_1_get(
		a_uint32_t dev_id,
		union eco_reserve_1_u *value)
{
	return hppe_reg_get(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + ECO_RESERVE_1_ADDRESS,
				&value->val);
}

sw_error_t
appe_eco_reserve_1_set(
		a_uint32_t dev_id,
		union eco_reserve_1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + ECO_RESERVE_1_ADDRESS,
				value->val);
}

sw_error_t
appe_shp_cfg_l0_get(
		a_uint32_t dev_id,
		union shp_cfg_l0_u *value)
{
	return hppe_reg_get(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + SHP_CFG_L0_ADDRESS,
				&value->val);
}

sw_error_t
appe_shp_cfg_l0_set(
		a_uint32_t dev_id,
		union shp_cfg_l0_u *value)
{
	return hppe_reg_set(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + SHP_CFG_L0_ADDRESS,
				value->val);
}

sw_error_t
appe_shp_cfg_l1_get(
		a_uint32_t dev_id,
		union shp_cfg_l1_u *value)
{
	return hppe_reg_get(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + SHP_CFG_L1_ADDRESS,
				&value->val);
}

sw_error_t
appe_shp_cfg_l1_set(
		a_uint32_t dev_id,
		union shp_cfg_l1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TRAFFIC_MANAGER_BASE_ADDR + SHP_CFG_L1_ADDRESS,
				value->val);
}

sw_error_t
appe_min_max_mode_cfg_min_max_mode_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union min_max_mode_cfg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_min_max_mode_cfg_get(dev_id, &reg_val);
	*value = reg_val.bf.min_max_mode;
	return ret;
}

sw_error_t
appe_min_max_mode_cfg_min_max_mode_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union min_max_mode_cfg_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_min_max_mode_cfg_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.min_max_mode = value;
	ret = appe_min_max_mode_cfg_set(dev_id, &reg_val);
	return ret;
}


sw_error_t
appe_eco_reserve_0_eco_res_0_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union eco_reserve_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_eco_reserve_0_get(dev_id, &reg_val);
	*value = reg_val.bf.eco_res_0;
	return ret;
}

sw_error_t
appe_eco_reserve_0_eco_res_0_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union eco_reserve_0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_eco_reserve_0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eco_res_0 = value;
	ret = appe_eco_reserve_0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_eco_reserve_1_eco_res_1_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union eco_reserve_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_eco_reserve_1_get(dev_id, &reg_val);
	*value = reg_val.bf.eco_res_1;
	return ret;
}

sw_error_t
appe_eco_reserve_1_eco_res_1_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union eco_reserve_1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_eco_reserve_1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eco_res_1 = value;
	ret = appe_eco_reserve_1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_head_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union shp_cfg_l0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l0_get(dev_id, &reg_val);
	*value = reg_val.bf.l0_shp_ll_head;
	return ret;
}

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_head_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union shp_cfg_l0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.l0_shp_ll_head = value;
	ret = appe_shp_cfg_l0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_tail_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union shp_cfg_l0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l0_get(dev_id, &reg_val);
	*value = reg_val.bf.l0_shp_ll_tail;
	return ret;
}

sw_error_t
appe_shp_cfg_l0_l0_shp_ll_tail_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union shp_cfg_l0_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l0_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.l0_shp_ll_tail = value;
	ret = appe_shp_cfg_l0_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_head_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union shp_cfg_l1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l1_get(dev_id, &reg_val);
	*value = reg_val.bf.l1_shp_ll_head;
	return ret;
}

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_head_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union shp_cfg_l1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.l1_shp_ll_head = value;
	ret = appe_shp_cfg_l1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_tail_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union shp_cfg_l1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l1_get(dev_id, &reg_val);
	*value = reg_val.bf.l1_shp_ll_tail;
	return ret;
}

sw_error_t
appe_shp_cfg_l1_l1_shp_ll_tail_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union shp_cfg_l1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_shp_cfg_l1_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.l1_shp_ll_tail = value;
	ret = appe_shp_cfg_l1_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_grp_end_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.grp_end;
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_grp_end_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.grp_end = value;
	ret = hppe_l0_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_eir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eir_max;
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_eir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eir_max = value;
	ret = hppe_l0_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_grp_cf_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.grp_cf;
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_grp_cf_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.grp_cf = value;
	ret = hppe_l0_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_shp_refresh_nxt_ptr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.shp_refresh_nxt_ptr;
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_shp_refresh_nxt_ptr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.shp_refresh_nxt_ptr = value;
	ret = hppe_l0_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_cir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cir_max_1 << 14 | \
		reg_val.bf.cir_max_0;
	return ret;
}

sw_error_t
appe_l0_shp_cfg_tbl_cir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l0_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l0_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cir_max_1 = value >> 14;
	reg_val.bf.cir_max_0 = value & (((a_uint64_t)1<<14)-1);
	ret = hppe_l0_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_grp_end_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.grp_end;
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_grp_end_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.grp_end = value;
	ret =  hppe_l1_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_eir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eir_max;
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_eir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eir_max = value;
	ret =  hppe_l1_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_grp_cf_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.grp_cf;
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_grp_cf_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.grp_cf = value;
	ret =  hppe_l1_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_shp_refresh_nxt_ptr_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.shp_refresh_nxt_ptr;
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_shp_refresh_nxt_ptr_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.shp_refresh_nxt_ptr = value;
	ret =  hppe_l1_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_cir_max_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.cir_max_1 << 17 | \
		reg_val.bf.cir_max_0;
	return ret;
}

sw_error_t
appe_l1_shp_cfg_tbl_cir_max_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l1_shp_cfg_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret =  hppe_l1_shp_cfg_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.cir_max_1 = value >> 17;
	reg_val.bf.cir_max_0 = value & (((a_uint64_t)1<<17)-1);
	ret =  hppe_l1_shp_cfg_tbl_set(dev_id, index, &reg_val);
	return ret;
}
