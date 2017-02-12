/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
 * @defgroup fal_trunk FAL_TRUNK
 * @{
 */
#ifndef _FAL_TRUNK_H_
#define _FAL_TRUNK_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "common/sw.h"
#include "fal/fal_type.h"


#define FAL_TRUNK_HASH_KEY_DA              0x1
#define FAL_TRUNK_HASH_KEY_SA              0x2
#define FAL_TRUNK_HASH_KEY_DIP             0x4
#define FAL_TRUNK_HASH_KEY_SIP             0x8


    sw_error_t
    fal_trunk_group_set(a_uint32_t dev_id, a_uint32_t trunk_id,
                        a_bool_t enable, fal_pbmp_t member);


    sw_error_t
    fal_trunk_group_get(a_uint32_t dev_id, a_uint32_t trunk_id,
                        a_bool_t * enable, fal_pbmp_t * member);


    sw_error_t
    fal_trunk_hash_mode_set(a_uint32_t dev_id, a_uint32_t hash_mode);


    sw_error_t
    fal_trunk_hash_mode_get(a_uint32_t dev_id, a_uint32_t * hash_mode);


    sw_error_t
    fal_trunk_manipulate_sa_set(a_uint32_t dev_id, fal_mac_addr_t * addr);


    sw_error_t
    fal_trunk_manipulate_sa_get(a_uint32_t dev_id, fal_mac_addr_t * addr);


#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_TRUNK_H_ */

/**
 * @}
 */
