/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
 * @defgroup fal_tunnel_program FAL_TUNNEL_PROGRAM
 * @{
 */
#ifndef _FAL_TUNNEL_PROGRAM_H_
#define _FAL_TUNNEL_PROGRAM_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#include "sw.h"
#include "fal_type.h"
#include "fal_tunnel.h"

typedef struct {
	a_uint8_t ip_ver;  /*ip version of outer packet, 1 ipv4, 2 ipv6, 3 ipv4 or ipv6*/
	fal_hdr_type_t outer_hdr_type; /*current(outer) hdr type*/
	a_uint32_t protocol; /*protocol value depends on the outer type*/
	a_uint32_t protocol_mask; /*protocol value mask*/
} fal_tunnel_program_entry_t;

#define TUNNEL_PROGRAM_UDF_NUM 3
typedef struct {
	a_uint8_t program_pos_mode; /*position mode of program offset;
				     *0 end of outer hdr, 1 start of outer hdr*/
	a_uint8_t inner_type_mode; /*0 fix mode, 1 udf mode*/
	fal_hdr_type_t inner_hdr_type; /*inner type, used in fix inner type mode*/
	a_uint8_t basic_hdr_len; /*basic length in program hdr*/
	a_uint8_t opt_len_unit; /*unit for opt len field in program hdr*/
	a_uint16_t opt_len_mask; /*mask of opt len field(udf0)in program hdr*/
	a_uint8_t udf_offset[TUNNEL_PROGRAM_UDF_NUM]; /*udf offset base from program offset*/
} fal_tunnel_program_cfg_t;

/* program udf rule filed bitmap opration */

typedef a_uint32_t fal_tunnel_program_udf_field_map_t;

#define FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(flag, field) \
    (flag) |= (0x1UL << (field))

#define FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_CLR(flag, field) \
    (flag) &= (~(0x1UL << (field)))

#define FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_TST(flag, field) \
    ((flag) & (0x1UL << (field))) ? 1 : 0

#define    FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF0            0
#define    FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF1            1
#define    FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF2            2
#define    FAL_TUNNEL_PROGRAM_UDF_FIELD_INVERSE         3

/* program udf action field bitmap opration */

typedef a_uint32_t fal_tunnel_program_udf_action_map_t;

#define FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(flag, action) \
    (flag) |= (0x1UL << (action))

#define FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_CLR(flag, action) \
    (flag) &= (~(0x1UL << (action)))

#define FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_TST(flag, action) \
    ((flag) & (0x1UL << (action))) ? 1 : 0

#define    FAL_TUNNEL_PROGRAM_UDF_ACTION_INNER_HDR_TYPE        0
#define    FAL_TUNNEL_PROGRAM_UDF_ACTION_UDF_HDR_LEN           1
#define    FAL_TUNNEL_PROGRAM_UDF_ACTION_EXCEPTION_EN          2

typedef struct {
	/*field of rule*/
	fal_tunnel_program_udf_field_map_t field_flag; /*Indicate which fields are included*/
	a_uint16_t udf_val[TUNNEL_PROGRAM_UDF_NUM]; /*udf value, 2 bytes*/
	a_uint16_t udf_mask[TUNNEL_PROGRAM_UDF_NUM];/*udf mask, 2 bytes*/

	/*field of action*/
	fal_tunnel_program_udf_action_map_t action_flag; /*indicate which action apply*/
	fal_hdr_type_t inner_hdr_type;/*user defined inner type, only used in inner type udf mode*/
	a_uint8_t udf_hdr_len; /*user defined length in program hdr*/
} fal_tunnel_program_udf_t;


enum
{
        /*tunnel program*/
	FUNC_TUNNEL_PROGRAM_ENTRY_ADD = 0,
	FUNC_TUNNEL_PROGRAM_ENTRY_DEL,
	FUNC_TUNNEL_PROGRAM_ENTRY_GETFIRST,
	FUNC_TUNNEL_PROGRAM_ENTRY_GETNEXT,
	FUNC_TUNNEL_PROGRAM_CFG_SET,
	FUNC_TUNNEL_PROGRAM_CFG_GET,
	FUNC_TUNNEL_PROGRAM_UDF_ADD,
	FUNC_TUNNEL_PROGRAM_UDF_DEL,
	FUNC_TUNNEL_PROGRAM_UDF_GETFIRST,
	FUNC_TUNNEL_PROGRAM_UDF_GETNEXT,
};


sw_error_t
fal_tunnel_program_entry_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry);

sw_error_t
fal_tunnel_program_entry_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry);

sw_error_t
fal_tunnel_program_entry_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry);

sw_error_t
fal_tunnel_program_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry);

sw_error_t
fal_tunnel_program_cfg_set(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg);

sw_error_t
fal_tunnel_program_cfg_get(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg);

sw_error_t
fal_tunnel_program_udf_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf);

sw_error_t
fal_tunnel_program_udf_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf);

sw_error_t
fal_tunnel_program_udf_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf);

sw_error_t
fal_tunnel_program_udf_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _FAL_TUNNEL_PROGRAM_H_ */
/**
 * @}
 */
