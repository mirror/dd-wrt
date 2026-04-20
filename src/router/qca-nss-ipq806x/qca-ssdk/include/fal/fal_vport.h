/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

/**
 * @defgroup fal_vport FAL_VPORT
 * @{
 */
#ifndef _FAL_VPORT_H_
#define _FAL_VPORT_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"

typedef enum {
	FAL_VPORT_TYPE_TUNNEL = 0,
	FAL_VPORT_TYPE_NORMAL,
	FAL_VPORT_TYPE_BUTT,
} fal_vport_type_t;

typedef struct {
	a_bool_t check_en; /* check enable or not */
	fal_vport_type_t vp_type; /* 0: tunnel vp, 1: regular vp */
	a_bool_t vp_active; /* actived or not */
	a_bool_t eg_data_valid; /* eg_data valid or not for tunnel vp */
} fal_vport_state_t;

sw_error_t
fal_vport_state_check_set(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state);

sw_error_t
fal_vport_state_check_get(a_uint32_t dev_id, fal_port_t port_id, fal_vport_state_t *vp_state);

sw_error_t
fal_vport_physical_port_id_set(a_uint32_t dev_id, fal_port_t vport_id, fal_port_t phyport_id);

sw_error_t
fal_vport_physical_port_id_get(a_uint32_t dev_id, fal_port_t vport_id, fal_port_t *phyport_id);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_VPORT_H_ */
/**
 * @}
 */
