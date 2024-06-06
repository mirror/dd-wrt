/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _ADPT_APPE_PPPOE_H_
#define _ADPT_APPE_PPPOE_H_

sw_error_t
adpt_appe_pppoe_session_table_add(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl);

sw_error_t
adpt_appe_pppoe_session_table_del(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl);

sw_error_t
adpt_appe_pppoe_session_table_get(a_uint32_t dev_id, fal_pppoe_session_t *session_tbl);

sw_error_t
adpt_appe_pppoe_l3_intf_set(a_uint32_t dev_id, a_uint32_t pppoe_index,
		fal_intf_type_t l3_type, fal_intf_id_t *pppoe_intf);

sw_error_t
adpt_appe_pppoe_l3_intf_get(a_uint32_t dev_id, a_uint32_t pppoe_index,
		fal_intf_type_t l3_type, fal_intf_id_t *pppoe_intf);

#endif
