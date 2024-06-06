/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
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
#ifndef _ADPT_HPPE_ACL_H_
#define _ADPT_HPPE_ACL_H_

#ifdef __cplusplus
extern "C" {
#endif                          /* __cplusplus */

#define ADPT_ACL_HPPE_UDF0_RULE 13
#define ADPT_ACL_HPPE_UDF1_RULE 14
#if defined(APPE)
#define ADPT_ACL_HPPE_RULE_TYPE_NUM 19
#else
#define ADPT_ACL_HPPE_RULE_TYPE_NUM 15
#endif

#define HPPE_ACL_DEST_INFO(type,value) (((type)<<12)|((value)&0xfff))
#define HPPE_ACL_DEST_TYPE(dest) (((dest)>>12)&0x3)
#define HPPE_ACL_DEST_VALUE(dest) ((dest)&0xfff)

enum{
	HPPE_ACL_ACTION_FWD = 0,
	HPPE_ACL_ACTION_DROP,
	HPPE_ACL_ACTION_COPYCPU,
	HPPE_ACL_ACTION_RDTCPU,
};

enum{
	HPPE_ACL_DEST_INVALID = 0,
	HPPE_ACL_DEST_NEXTHOP,
	HPPE_ACL_DEST_PORT_ID,
	HPPE_ACL_DEST_PORT_BMP,
};


typedef struct {
	a_uint32_t rule_type_map; /* rule type map */
	a_uint8_t inverse_rule_type_count[ADPT_ACL_HPPE_RULE_TYPE_NUM]; /* inverse rule type count */
} ADPT_HPPE_ACL_RULE_MAP;

a_uint32_t _acl_bit_index(a_uint32_t bits, a_uint32_t max, a_uint32_t type);

void _adpt_acl_reg_dump(a_uint8_t *reg, a_uint32_t len);

void acl_rule_field_convert(fal_acl_rule_t * rule,
	fal_acl_rule_field_t * rule_field, a_bool_t to_inner);

sw_error_t
_adpt_hppe_acl_rule_sw_2_hw(a_uint32_t dev_id, fal_acl_rule_t *rule, a_uint32_t rule_type,
	a_uint32_t *hw_entry, a_uint32_t *allocated_entries, a_uint8_t *range_en,
	void *hw_rule, void *hw_rule_mask, a_uint8_t inverse_en);

sw_error_t
_adpt_hppe_acl_rule_hw_2_sw(a_uint32_t dev_id, a_uint32_t rule_type,
	a_uint8_t range_en, a_uint8_t inverse_en,
	void * hw_rule, void *hw_rule_mask, fal_acl_rule_t * rule);

#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif
/**
 * @}
 */
