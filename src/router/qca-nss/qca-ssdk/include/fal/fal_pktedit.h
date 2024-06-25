/*
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup fal_pktedit FAL_PKTEDIT
 * @{
 */
#ifndef _FAL_PKTEDIT_H_
#define _FAL_PKTEDIT_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"

typedef struct {
	a_bool_t strip_padding_en; /* strip padding */;
	a_bool_t strip_padding_route_en; /* strip padding for route forward */
	a_bool_t strip_padding_bridge_en; /* strip padding for brige forward */
	a_bool_t strip_padding_checksum_en; /* payload cheksum update without padding,
					       used in tunnel encap tunnel header L4 checksum
					       update */
	a_bool_t strip_padding_snap_en; /* strip padding for snap */
	a_bool_t strip_tunnel_inner_padding_en; /* strip tunnel inner padding */
	a_bool_t tunnel_inner_padding_exp_en; /* tunnel inner padding exception */
	a_bool_t tunnel_ip_len_gap_exp_en; /* tunnel inner ip length larger than outer ip length,
					      or tunnel outer ip length is more than 255 bytes
					      than inner ip length */
} fal_pktedit_padding_t;

enum
{
	/* pktedit */
	FUNC_PKTEDIT_PADDING_SET = 0,
	FUNC_PKTEDIT_PADDING_GET,
};


sw_error_t
fal_pktedit_padding_set(a_uint32_t dev_id,
			fal_pktedit_padding_t *padding);

sw_error_t
fal_pktedit_padding_get(a_uint32_t dev_id,
			fal_pktedit_padding_t *padding);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_PKTEDIT_H_ */
/**
 * @}
 */

