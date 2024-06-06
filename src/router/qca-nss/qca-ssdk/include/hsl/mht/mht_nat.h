/*
 * Copyright (c) 2021,2023 Qualcomm Innovation Center, Inc. All rights reserved.
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


#ifndef _MHT_NAT_H_
#define _MHT_NAT_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "fal_nat.h"
extern aos_mutex_lock_t mht_nat_lock;

sw_error_t
mht_flow_add(a_uint32_t dev_id, fal_napt_entry_t * napt_entry);

sw_error_t
mht_flow_del(a_uint32_t dev_id, a_uint32_t del_mode, fal_napt_entry_t * napt_entry);

sw_error_t
mht_flow_get(a_uint32_t dev_id, a_uint32_t get_mode, fal_napt_entry_t * napt_entry);

sw_error_t
mht_flow_next(a_uint32_t dev_id, a_uint32_t next_mode, fal_napt_entry_t * napt_entry);

sw_error_t
mht_flow_counter_bind(a_uint32_t dev_id, a_uint32_t entry_id, a_uint32_t cnt_id, a_bool_t enable);

sw_error_t
mht_flow_cookie_set(a_uint32_t dev_id, fal_flow_cookie_t * flow_cookie);

sw_error_t
mht_flow_rfs_set(a_uint32_t dev_id, a_uint8_t action, fal_flow_rfs_t * rfs);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _MHT_NAT_H_ */
