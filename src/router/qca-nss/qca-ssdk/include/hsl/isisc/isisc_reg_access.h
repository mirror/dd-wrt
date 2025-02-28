/*
 * Copyright (c) 2012,2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _ISISC_REG_ACCESS_H_
#define _ISISC_REG_ACCESS_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "sw.h"
    sw_error_t
    isisc_reg_get(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
                 a_uint32_t value_len);

    sw_error_t
    isisc_reg_set(a_uint32_t dev_id, a_uint32_t reg_addr, a_uint8_t value[],
                 a_uint32_t value_len);

    sw_error_t
    isisc_reg_field_get(a_uint32_t dev_id, a_uint32_t reg_addr,
                       a_uint32_t bit_offset, a_uint32_t field_len,
                       a_uint8_t value[], a_uint32_t value_len);

    sw_error_t
    isisc_reg_field_set(a_uint32_t dev_id, a_uint32_t reg_addr,
                       a_uint32_t bit_offset, a_uint32_t field_len,
                       const a_uint8_t value[], a_uint32_t value_len);

    sw_error_t
    isisc_regsiter_dump(a_uint32_t dev_id,a_uint32_t register_idx, fal_reg_dump_t * reg_dump);

    sw_error_t
    isisc_debug_regsiter_dump(a_uint32_t dev_id, fal_debug_reg_dump_t * reg_dump);

    sw_error_t
    isisc_reg_access_init(a_uint32_t dev_id, hsl_access_mode mode);

    sw_error_t
    isisc_access_mode_set(a_uint32_t dev_id, hsl_access_mode mode);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ISISC_REG_ACCESS_H_ */

