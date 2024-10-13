/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "appe_qm_reg.h"
#include "appe_qm.h"

sw_error_t
appe_port_vsi_enqueue_map_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union port_vsi_enqueue_map_u *value)
{
	if (index >= PORT_VSI_ENQUEUE_MAP_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPE_L2_BASE_ADDR + PORT_VSI_ENQUEUE_MAP_ADDRESS + \
				index * PORT_VSI_ENQUEUE_MAP_INC,
				&value->val);
}

sw_error_t
appe_port_vsi_enqueue_map_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union port_vsi_enqueue_map_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPE_L2_BASE_ADDR + PORT_VSI_ENQUEUE_MAP_ADDRESS + \
				index * PORT_VSI_ENQUEUE_MAP_INC,
				value->val);
}

#if 0
sw_error_t
appe_port_vsi_enqueue_map_enqueue_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union port_vsi_enqueue_map_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_vsi_enqueue_map_get(dev_id, index, &reg_val);
	*value = reg_val.bf.enqueue_valid;
	return ret;
}

sw_error_t
appe_port_vsi_enqueue_map_enqueue_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union port_vsi_enqueue_map_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_vsi_enqueue_map_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.enqueue_valid = value;
	ret = appe_port_vsi_enqueue_map_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_port_vsi_enqueue_map_enqueue_vp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union port_vsi_enqueue_map_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_vsi_enqueue_map_get(dev_id, index, &reg_val);
	*value = reg_val.bf.enqueue_vp;
	return ret;
}

sw_error_t
appe_port_vsi_enqueue_map_enqueue_vp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union port_vsi_enqueue_map_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_vsi_enqueue_map_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.enqueue_vp = value;
	ret = appe_port_vsi_enqueue_map_set(dev_id, index, &reg_val);
	return ret;
}
#endif
