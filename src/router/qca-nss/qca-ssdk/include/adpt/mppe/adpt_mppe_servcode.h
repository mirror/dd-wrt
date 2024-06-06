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
#ifndef _ADPT_MPPE_SERVCODE_H_
#define _ADPT_MPPE_SERVCODE_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

/*athtag bit location in field_update_action*/
#define ATHTAG_INSERT_UPDATE	BIT(23)
#define ATHTAG_ACTION_UPDATE	BIT(28)
#define ATHTAG_BYPASS_FWD_EN_UPDATE	BIT(29)
#define ATHTAG_DEST_PORT_UPDATE	BIT(30)
#define ATHTAG_FIELD_DISABLE_UPDATE	BIT(31)
#define ATHTAG_UPDATE	ATHTAG_INSERT_UPDATE | ATHTAG_ACTION_UPDATE | ATHTAG_BYPASS_FWD_EN_UPDATE | ATHTAG_DEST_PORT_UPDATE | ATHTAG_FIELD_DISABLE_UPDATE

sw_error_t
adpt_mppe_port_servcode_set(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t servcode_index);

sw_error_t
adpt_mppe_port_servcode_get(a_uint32_t dev_id, fal_port_t port_id, a_uint32_t *servcode_index);

sw_error_t
adpt_mppe_servcode_athtag_set(a_uint32_t dev_id,
		a_uint32_t servcode_index, fal_servcode_athtag_t *entry);

sw_error_t
adpt_mppe_servcode_athtag_get(a_uint32_t dev_id,
		a_uint32_t servcode_index, fal_servcode_athtag_t *entry);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
