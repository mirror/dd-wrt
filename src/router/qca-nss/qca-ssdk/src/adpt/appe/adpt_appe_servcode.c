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


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "adpt.h"
#include "appe_servcode_reg.h"
#include "appe_servcode.h"
#include "adpt_appe_servcode.h"

sw_error_t
adpt_appe_servcode_tl_config_set(a_uint32_t dev_id, a_uint32_t servcode_index,
					fal_servcode_config_t *entry)
{
	union tl_service_tbl_u tl_service_tbl = {0};
	tl_service_tbl.bf.bypass_bitmap = entry->bypass_bitmap[3];

	SW_RTN_ON_ERROR(appe_tl_service_tbl_set(dev_id, servcode_index, &tl_service_tbl));
	return SW_OK;
}

sw_error_t
adpt_appe_servcode_tl_config_get(a_uint32_t dev_id, a_uint32_t servcode_index,
					fal_servcode_config_t *entry)
{
	union tl_service_tbl_u tl_service_tbl = {0};

	SW_RTN_ON_ERROR(appe_tl_service_tbl_get(dev_id, servcode_index, &tl_service_tbl));

	entry->bypass_bitmap[3] = tl_service_tbl.bf.bypass_bitmap;
	return SW_OK;
}

/**
 * @}
 */
