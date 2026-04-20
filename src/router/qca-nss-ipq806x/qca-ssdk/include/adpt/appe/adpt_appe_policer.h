/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
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

#ifndef _ADPT_APPE_POLICER_H_
#define _ADPT_APPE_POLICER_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define APPE_POLICER_ID_MIN                  0
#define APPE_POLICER_ID_MAX                  511
#define APPE_POLICER_TIME_SLOT_MAX           4095
#if defined(MPPE)
#define APPE_POLICER_TIME_SLOT_MIN           256
#else
#define APPE_POLICER_TIME_SLOT_MIN           1024
#endif


sw_error_t
adpt_appe_policer_ctrl_set(a_uint32_t dev_id, fal_policer_ctrl_t *ctrl);

sw_error_t
adpt_appe_policer_ctrl_get(a_uint32_t dev_id, fal_policer_ctrl_t *ctrl);

#ifndef IN_POLICER_MINI
sw_error_t
adpt_appe_policer_priority_remap_get(a_uint32_t dev_id,
	fal_policer_priority_t *priority, fal_policer_remap_t *remap);

sw_error_t
adpt_appe_policer_priority_remap_set(a_uint32_t dev_id,
	fal_policer_priority_t *priority, fal_policer_remap_t *remap);
#endif
#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
