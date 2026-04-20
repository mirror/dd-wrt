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
#ifndef _MPPE_ATHTAG_H_
#define _MPPE_ATHTAG_H_

#define EG_HDR_XMIT_PRI_MAPPING_MAX_ENTRY	16
#define PRX_PORT_TO_VP_MAPPING_MAX_ENTRY	8
#define PRX_HDR_RCV_PRI_MAPPING_MAX_ENTRY	8
sw_error_t
mppe_eg_hdr_xmit_pri_mapping_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_hdr_xmit_pri_mapping_u *value);

sw_error_t
mppe_eg_hdr_xmit_pri_mapping_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_hdr_xmit_pri_mapping_u *value);

sw_error_t
mppe_eg_gen_ctrl_set(
		a_uint32_t dev_id,
		union eg_gen_ctrl_u *value);

sw_error_t
mppe_eg_gen_ctrl_get(
		a_uint32_t dev_id,
		union eg_gen_ctrl_u *value);

sw_error_t
mppe_prx_port_to_vp_mapping_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_port_to_vp_mapping_u *value);

sw_error_t
mppe_prx_port_to_vp_mapping_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_port_to_vp_mapping_u *value);

sw_error_t
mppe_prx_hdr_rcv_pri_mapping_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_hdr_rcv_pri_mapping_u *value);

sw_error_t
mppe_prx_hdr_rcv_pri_mapping_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_hdr_rcv_pri_mapping_u *value);
#endif
