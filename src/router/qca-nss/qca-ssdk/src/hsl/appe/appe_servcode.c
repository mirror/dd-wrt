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
#include "appe_servcode_reg.h"
#include "appe_servcode.h"

sw_error_t
appe_tl_service_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_service_tbl_u *value)
{
	if (index >= TL_SERVICE_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_SERVICE_TBL_ADDRESS + \
				index * TL_SERVICE_TBL_INC,
				&value->val);
}

sw_error_t
appe_tl_service_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_service_tbl_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_LOOKUP_BASE_ADDR + TL_SERVICE_TBL_ADDRESS + \
				index * TL_SERVICE_TBL_INC,
				value->val);
}

#if 0
sw_error_t
appe_tl_service_tbl_bypass_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tl_service_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_service_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.bypass_bitmap;
	return ret;
}

sw_error_t
appe_tl_service_tbl_bypass_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tl_service_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tl_service_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.bypass_bitmap = value;
	ret = appe_tl_service_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif