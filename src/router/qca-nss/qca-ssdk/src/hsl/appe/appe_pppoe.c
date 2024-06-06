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
#include "appe_pppoe_reg.h"
#include "appe_pppoe.h"

sw_error_t
appe_pppoe_session_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_u *value)
{
	if (index >= PPPOE_SESSION_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_ADDRESS +
				index * PPPOE_SESSION_INC,
				&value->val);
}

sw_error_t
appe_pppoe_session_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_ADDRESS +
				index * PPPOE_SESSION_INC,
				value->val);
}

sw_error_t
appe_pppoe_session_ext_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_ext_u *value)
{
	if (index >= PPPOE_SESSION_EXT_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_EXT_ADDRESS +
				index * PPPOE_SESSION_EXT_INC,
				&value->val);
}

sw_error_t
appe_pppoe_session_ext_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_ext_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_EXT_ADDRESS +
				index * PPPOE_SESSION_EXT_INC,
				value->val);
}

sw_error_t
appe_pppoe_session_ext1_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_ext1_u *value)
{
	if (index >= PPPOE_SESSION_EXT1_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_EXT1_ADDRESS +
				index * PPPOE_SESSION_EXT1_INC,
				&value->val);
}

sw_error_t
appe_pppoe_session_ext1_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_ext1_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_EXT1_ADDRESS +
				index * PPPOE_SESSION_EXT1_INC,
				value->val);
}

sw_error_t
appe_pppoe_session_ext2_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_ext2_u *value)
{
	if (index >= PPPOE_SESSION_EXT2_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_EXT2_ADDRESS +
				index * PPPOE_SESSION_EXT2_INC,
				&value->val);
}

sw_error_t
appe_pppoe_session_ext2_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union pppoe_session_ext2_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + PPPOE_SESSION_EXT2_ADDRESS +
				index * PPPOE_SESSION_EXT2_INC,
				value->val);
}

#if 0
sw_error_t
appe_pppoe_session_session_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	*value = reg_val.bf2.session_id;
	return ret;
}

sw_error_t
appe_pppoe_session_session_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf2.session_id = value;
	ret = appe_pppoe_session_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_port_vp_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	*value = reg_val.bf1.port_vp_id;
	return ret;
}

sw_error_t
appe_pppoe_session_port_vp_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf1.port_vp_id = value;
	ret = appe_pppoe_session_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	*value = reg_val.bf2.port_type;
	return ret;
}

sw_error_t
appe_pppoe_session_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf2.port_type = value;
	ret = appe_pppoe_session_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_port_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_bitmap;
	return ret;
}

sw_error_t
appe_pppoe_session_port_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_bitmap = value;
	ret = appe_pppoe_session_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_vp_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	*value = reg_val.bf2.vp_profile;
	return ret;
}

sw_error_t
appe_pppoe_session_vp_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf2.vp_profile = value;
	ret = appe_pppoe_session_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext_uc_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	*value = reg_val.bf.uc_valid;
	return ret;
}

sw_error_t
appe_pppoe_session_ext_uc_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.uc_valid = value;
	ret = appe_pppoe_session_ext_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext_mc_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mc_valid;
	return ret;
}

sw_error_t
appe_pppoe_session_ext_mc_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mc_valid = value;
	ret = appe_pppoe_session_ext_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext_smac_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	*value = reg_val.bf.smac_valid;
	return ret;
}

sw_error_t
appe_pppoe_session_ext_smac_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.smac_valid = value;
	ret = appe_pppoe_session_ext_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext_smac_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	*value = reg_val.bf.smac;
	return ret;
}

sw_error_t
appe_pppoe_session_ext_smac_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.smac = value;
	ret = appe_pppoe_session_ext_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext1_smac_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext1_get(dev_id, index, &reg_val);
	*value = reg_val.bf.smac;
	return ret;
}

sw_error_t
appe_pppoe_session_ext1_smac_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext1_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext1_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.smac = value;
	ret = appe_pppoe_session_ext1_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_l3_if_index_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	*value = reg_val.bf.l3_if_index;
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_l3_if_index_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.l3_if_index = value;
	ret = appe_pppoe_session_ext2_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_tl_l3_if_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tl_l3_if_valid;
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_tl_l3_if_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tl_l3_if_valid = value;
	ret = appe_pppoe_session_ext2_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_l3_if_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	*value = reg_val.bf.l3_if_valid;
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_l3_if_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.l3_if_valid = value;
	ret = appe_pppoe_session_ext2_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_tl_l3_if_index_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tl_l3_if_index;
	return ret;
}

sw_error_t
appe_pppoe_session_ext2_tl_l3_if_index_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union pppoe_session_ext2_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_pppoe_session_ext2_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tl_l3_if_index = value;
	ret = appe_pppoe_session_ext2_set(dev_id, index, &reg_val);
	return ret;
}
#endif