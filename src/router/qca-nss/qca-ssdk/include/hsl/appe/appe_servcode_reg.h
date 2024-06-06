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
#include "appe_servcode_reg_be.h"
#else
/**
 * @defgroup
 * @{
 */
#ifndef APPE_SERVCODE_REG_H
#define APPE_SERVCODE_REG_H

/*[table] TL_SERVICE_TBL*/
#define TL_SERVICE_TBL
#define TL_SERVICE_TBL_ADDRESS 0x6000
#define TL_SERVICE_TBL_NUM     256
#define TL_SERVICE_TBL_INC     0x4
#define TL_SERVICE_TBL_TYPE    REG_TYPE_RW
#define TL_SERVICE_TBL_DEFAULT 0x0
	/*[field] BYPASS_BITMAP*/
	#define TL_SERVICE_TBL_BYPASS_BITMAP
	#define TL_SERVICE_TBL_BYPASS_BITMAP_OFFSET  0
	#define TL_SERVICE_TBL_BYPASS_BITMAP_LEN     32
	#define TL_SERVICE_TBL_BYPASS_BITMAP_DEFAULT 0x0

struct tl_service_tbl {
	a_uint32_t  bypass_bitmap:32;
};

union tl_service_tbl_u {
	a_uint32_t val;
	struct tl_service_tbl bf;
};
#endif
#endif
