/*
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
#ifndef _MPPE_SERVCODE_H_
#define _MPPE_SERVCODE_H_

#define TL_VP_SERVICE_CODE_GEN_MAX_ENTRY	256

sw_error_t
mppe_tl_vp_service_code_gen_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_vp_service_code_gen_u *value);

sw_error_t
mppe_tl_vp_service_code_gen_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tl_vp_service_code_gen_u *value);

#if 0
sw_error_t
mppe_tl_vp_service_code_gen_service_code_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
mppe_tl_vp_service_code_gen_service_code_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);

sw_error_t
mppe_tl_vp_service_code_gen_service_code_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value);

sw_error_t
mppe_tl_vp_service_code_gen_service_code_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value);
#endif
#endif
