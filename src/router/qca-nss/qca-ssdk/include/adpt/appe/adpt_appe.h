/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * @defgroup
 * @{
 */
#ifndef _ADPT_APPE_H_
#define _ADPT_APPE_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

sw_error_t adpt_appe_vport_init(a_uint32_t dev_id);
void adpt_appe_vport_func_bitmap_init(a_uint32_t dev_id);

sw_error_t adpt_appe_tunnel_init(a_uint32_t dev_id);
void adpt_appe_tunnel_func_bitmap_init(a_uint32_t dev_id);

void adpt_appe_vxlan_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_appe_vxlan_init(a_uint32_t dev_id);

void adpt_appe_geneve_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_appe_geneve_init(a_uint32_t dev_id);

void adpt_appe_tunnel_program_func_bitmap_init(a_uint32_t dev_id);
sw_error_t adpt_appe_tunnel_program_init(a_uint32_t dev_id);

sw_error_t adpt_appe_mapt_init(a_uint32_t dev_id);
void adpt_appe_mapt_func_bitmap_init(a_uint32_t dev_id);

void adpt_appe_led_func_bitmap_init(a_uint32_t dev_id);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
