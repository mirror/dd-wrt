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
#include "appe_vsi_reg.h"
#include "appe_vsi.h"

sw_error_t
appe_vsi_remap_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vsi_remap_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				IPE_L2_BASE_ADDR + VSI_REMAP_TBL_ADDRESS + \
				index * VSI_REMAP_TBL_INC,
				value->val,
				9);
}

sw_error_t
appe_vsi_remap_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vsi_remap_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				IPE_L2_BASE_ADDR + VSI_REMAP_TBL_ADDRESS + \
				index * VSI_REMAP_TBL_INC,
				value->val,
				9);
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_7_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_7;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_7_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_7 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_6_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_6;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_6_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_6 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_5_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_5;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_5_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_5 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_4_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_4;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_4_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_4 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_3_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_3;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_3_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_3 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_2;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_2 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_1;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_1 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_0_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.member_port_bitmap_0;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_member_port_bitmap_0_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.member_port_bitmap_0 = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_vsi_remap_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vsi_remap_en;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_vsi_remap_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vsi_remap_en = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_br_vsi_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.br_vsi;
	return ret;
}

sw_error_t
appe_vsi_remap_tbl_br_vsi_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vsi_remap_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vsi_remap_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.br_vsi = value;
	ret = appe_vsi_remap_tbl_set(dev_id, index, &reg_val);
	return ret;
}
