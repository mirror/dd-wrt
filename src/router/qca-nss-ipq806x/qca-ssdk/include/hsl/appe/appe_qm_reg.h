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

#if defined(CONFIG_CPU_BIG_ENDIAN)
#include "appe_qm_reg_be.h"
#else
#ifndef APPE_QM_REG_H
#define APPE_QM_REG_H

/*[table] PORT_VSI_ENQUEUE_MAP*/
#define PORT_VSI_ENQUEUE_MAP
#define PORT_VSI_ENQUEUE_MAP_ADDRESS 0x3d000
#define PORT_VSI_ENQUEUE_MAP_NUM     544
#define PORT_VSI_ENQUEUE_MAP_INC     0x4
#define PORT_VSI_ENQUEUE_MAP_TYPE    REG_TYPE_RW
#define PORT_VSI_ENQUEUE_MAP_DEFAULT 0x0
	/*[field] ENQUEUE_VP*/
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VP
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VP_OFFSET  0
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VP_LEN     8
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VP_DEFAULT 0x0
	/*[field] ENQUEUE_VALID*/
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VALID
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VALID_OFFSET  8
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VALID_LEN     1
	#define PORT_VSI_ENQUEUE_MAP_ENQUEUE_VALID_DEFAULT 0x0

struct port_vsi_enqueue_map {
	a_uint32_t  enqueue_vp:8;
	a_uint32_t  enqueue_valid:1;
	a_uint32_t  _reserved0:23;
};

union port_vsi_enqueue_map_u {
	a_uint32_t val;
	struct port_vsi_enqueue_map bf;
};
#endif
#endif
