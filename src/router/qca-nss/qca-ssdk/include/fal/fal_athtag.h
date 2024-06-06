/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @defgroup fal_athtag FAL_ATHTAG
 * @{
 */
#ifndef _FAL_ATHTAG_H_
#define _FAL_ATHTAG_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"

#define MHT_ATHTAG_TYPE		0xaaaa
typedef struct {
	a_uint8_t ath_pri; /*athtag priority 0-7*/
	a_uint8_t int_pri; /*internal priority 0-15*/
} fal_athtag_pri_mapping_t;

typedef struct {
	fal_pbmp_t ath_port; /*athtag portmap*/
	fal_port_t int_port; /*internal port*/
} fal_athtag_port_mapping_t;

typedef struct {
	a_bool_t athtag_en; /*enable rx athtag or not*/
	a_uint16_t athtag_type; /*two bytes athtag type*/
} fal_athtag_rx_cfg_t;

typedef enum {
	FAL_ATHTAG_VER2 = 0,
	FAL_ATHTAG_VER3 = 1,
} fal_athtag_version_t;

typedef enum
{
	FAL_ATHTAG_ACTION_NORMAL = 0,
	FAL_ATHTAG_ACTION_READ_WRITE_REG,
	FAL_ATHTAG_ACTION_DISABLE_LEARN,
	FAL_ATHTAG_ACTION_DISABLE_OFFLOAD,
	FAL_ATHTAG_ACTION_DISABLE_LEARN_OFFLOAD,
} fal_athtag_action_t;

typedef struct {
	a_bool_t athtag_en; /*enable insert athtag or not*/
	a_uint16_t athtag_type; /*two bytes athtag type*/
	fal_athtag_version_t version; /*version field*/
	fal_athtag_action_t action; /*ation field*/
	a_bool_t bypass_fwd_en; /*bypass fwd engine field*/
	a_bool_t field_disable; /*ver3 fields disable, only vchannel id is valid*/
} fal_athtag_tx_cfg_t;

sw_error_t
fal_athtag_pri_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t *pri_mapping);

sw_error_t
fal_athtag_pri_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_pri_mapping_t *pri_mapping);

sw_error_t
fal_athtag_port_mapping_set(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t *port_mapping);

sw_error_t
fal_athtag_port_mapping_get(a_uint32_t dev_id,
		fal_direction_t direction, fal_athtag_port_mapping_t *port_mapping);

sw_error_t
fal_port_athtag_rx_set(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_rx_cfg_t *cfg);

sw_error_t
fal_port_athtag_rx_get(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_rx_cfg_t *cfg);

sw_error_t
fal_port_athtag_tx_set(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_tx_cfg_t *cfg);

sw_error_t
fal_port_athtag_tx_get(a_uint32_t dev_id, fal_port_t port_id, fal_athtag_tx_cfg_t *cfg);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_ATHTAG_H_ */
/**
 * @}
 */

