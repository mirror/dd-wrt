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
 * @defgroup
 * @{
 */
#if defined(CONFIG_CPU_BIG_ENDIAN)
#include "mppe_servcode_reg_be.h"
#else
#ifndef MPPE_SERVCODE_REG_H
#define MPPE_SERVCODE_REG_H

/*[table] TL_VP_SERVICE_CODE_GEN*/
#define TL_VP_SERVICE_CODE_GEN
#define TL_VP_SERVICE_CODE_GEN_ADDRESS 0x800
#define TL_VP_SERVICE_CODE_GEN_NUM     256
#define TL_VP_SERVICE_CODE_GEN_INC     0x4
#define TL_VP_SERVICE_CODE_GEN_TYPE    REG_TYPE_RW
#define TL_VP_SERVICE_CODE_GEN_DEFAULT 0x0
	/*[field] SERVICE_CODE*/
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_OFFSET  0
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_LEN     8
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_DEFAULT 0x0
	/*[field] SERVICE_CODE_EN*/
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_EN
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_EN_OFFSET  8
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_EN_LEN     1
	#define TL_VP_SERVICE_CODE_GEN_SERVICE_CODE_EN_DEFAULT 0x0

struct tl_vp_service_code_gen {
	a_uint32_t  service_code:8;
	a_uint32_t  service_code_en:1;
	a_uint32_t  _reserved0:23;
};

union tl_vp_service_code_gen_u {
	a_uint32_t val;
	struct tl_vp_service_code_gen bf;
};
#endif
#endif
