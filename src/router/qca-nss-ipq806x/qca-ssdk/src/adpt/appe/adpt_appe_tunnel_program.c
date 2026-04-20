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
 * @defgroup
 * @{
 */
#include "sw.h"
#include "adpt.h"
#include "appe_tunnel_program_reg.h"
#include "appe_tunnel_program.h"


#define ADPT_TUNNEL_PROGRAM_ENTRY_NUM  6
#define ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM 16

/* record the used program id map for one program udf entry */
static a_uint32_t g_program_entry_used_map[SW_MAX_NR_DEV][ADPT_TUNNEL_PROGRAM_ENTRY_NUM];

/* record if one program udf rule entry is inuse  */
static a_bool_t g_program_udf_entry_inuse[SW_MAX_NR_DEV][ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM];

enum{
	APPE_OUTER_HDR_TYPE_ETHERNET = 0,
	APPE_OUTER_HDR_TYPE_IPV4,
	APPE_OUTER_HDR_TYPE_IPV6,
	APPE_OUTER_HDR_TYPE_UDP,
	APPE_OUTER_HDR_TYPE_UDP_LITE,
	APPE_OUTER_HDR_TYPE_TCP,
	APPE_OUTER_HDR_TYPE_GRE,
	APPE_OUTER_HDR_TYPE_VXLAN,
	APPE_OUTER_HDR_TYPE_VXLAN_GPE,
	APPE_OUTER_HDR_TYPE_GENEVE,
	APPE_OUTER_HDR_TYPE_INVALID,
};

enum{
	APPE_INNER_HDR_TYPE_ETHERNET = 0,
	APPE_INNER_HDR_TYPE_ETHERNET_TAG,
	APPE_INNER_HDR_TYPE_IPV4,
	APPE_INNER_HDR_TYPE_IPV6,
	APPE_INNER_HDR_TYPE_INVALID,
};

static sw_error_t
_program_inner_hdr_type_sw_2_hw(a_uint32_t dev_id,
			fal_hdr_type_t sw_hdr_type, a_uint32_t * hw_hdr_type)
{
	switch(sw_hdr_type)
	{
		case FAL_ETHERNET_HDR:
			*hw_hdr_type = APPE_INNER_HDR_TYPE_ETHERNET;
			break;
		case FAL_ETHERNET_TAG_HDR:
			*hw_hdr_type = APPE_INNER_HDR_TYPE_ETHERNET_TAG;
			break;
		case FAL_IPV4_HDR:
			*hw_hdr_type = APPE_INNER_HDR_TYPE_IPV4;
			break;
		case FAL_IPV6_HDR:
			*hw_hdr_type = APPE_INNER_HDR_TYPE_IPV6;
			break;
		default:
			SSDK_ERROR("invalid sw inner hdr type %d\n", sw_hdr_type);
			return SW_BAD_PARAM;
	}
	return SW_OK;
}

static sw_error_t
_program_inner_hdr_type_hw_2_sw(a_uint32_t dev_id,
			a_uint32_t hw_hdr_type, fal_hdr_type_t * sw_hdr_type)
{
	switch(hw_hdr_type)
	{
		case APPE_INNER_HDR_TYPE_ETHERNET:
			*sw_hdr_type = FAL_ETHERNET_HDR;
			break;
		case APPE_INNER_HDR_TYPE_ETHERNET_TAG:
			*sw_hdr_type = FAL_ETHERNET_TAG_HDR;
			break;
		case APPE_INNER_HDR_TYPE_IPV4:
			*sw_hdr_type = FAL_IPV4_HDR;
			break;
		case APPE_INNER_HDR_TYPE_IPV6:
			*sw_hdr_type = FAL_IPV6_HDR;
			break;
		default:
			SSDK_ERROR("invalid hw inner hdr type %d\n", hw_hdr_type);
			return SW_BAD_PARAM;
	}
	return SW_OK;
}

static sw_error_t
_program_outer_hdr_type_sw_2_hw(a_uint32_t dev_id,
			fal_hdr_type_t sw_hdr_type, a_uint32_t * hw_hdr_type)
{
	switch(sw_hdr_type)
	{
		case FAL_ETHERNET_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_ETHERNET;
			break;
		case FAL_IPV4_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_IPV4;
			break;
		case FAL_IPV6_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_IPV6;
			break;
		case FAL_UDP_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_UDP;
			break;
		case FAL_UDP_LITE_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_UDP_LITE;
			break;
		case FAL_TCP_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_TCP;
			break;
		case FAL_GRE_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_GRE;
			break;
		case FAL_VXLAN_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_VXLAN;
			break;
		case FAL_VXLAN_GPE_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_VXLAN_GPE;
			break;
		case FAL_GENEVE_HDR:
			*hw_hdr_type = APPE_OUTER_HDR_TYPE_GENEVE;
			break;
		default:
			SSDK_ERROR("invalid sw outer hdr type %d\n", sw_hdr_type);
			return SW_BAD_PARAM;
	}
	return SW_OK;
}

static sw_error_t
_program_outer_hdr_type_hw_2_sw(a_uint32_t dev_id,
			a_uint32_t hw_hdr_type, fal_hdr_type_t * sw_hdr_type)
{
	switch(hw_hdr_type)
	{
		case APPE_OUTER_HDR_TYPE_ETHERNET:
			*sw_hdr_type = FAL_ETHERNET_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_IPV4:
			*sw_hdr_type = FAL_IPV4_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_IPV6:
			*sw_hdr_type = FAL_IPV6_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_UDP:
			*sw_hdr_type = FAL_UDP_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_UDP_LITE:
			*sw_hdr_type = FAL_UDP_LITE_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_TCP:
			*sw_hdr_type = FAL_TCP_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_GRE:
			*sw_hdr_type = FAL_GRE_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_VXLAN:
			*sw_hdr_type = FAL_VXLAN_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_VXLAN_GPE:
			*sw_hdr_type = FAL_VXLAN_GPE_HDR;
			break;
		case APPE_OUTER_HDR_TYPE_GENEVE:
			*sw_hdr_type = FAL_GENEVE_HDR;
			break;
		default:
			SSDK_ERROR("invalid hw outer hdr type %d\n", hw_hdr_type);
			return SW_BAD_PARAM;
	}
	return SW_OK;
}

static sw_error_t _program_type_2_id(a_uint32_t dev_id,
			fal_tunnel_program_type_t type, a_uint32_t * program_id)
{
	if (type == FAL_TUNNEL_PROGRAM_TYPE_0)
	{
		*program_id = 0;
	}
	else if (type == FAL_TUNNEL_PROGRAM_TYPE_1)
	{
		*program_id = 1;
	}
	else if (type == FAL_TUNNEL_PROGRAM_TYPE_2)
	{
		*program_id = 2;
	}
	else if (type == FAL_TUNNEL_PROGRAM_TYPE_3)
	{
		*program_id = 3;
	}
	else if (type == FAL_TUNNEL_PROGRAM_TYPE_4)
	{
		*program_id = 4;
	}
	else if (type == FAL_TUNNEL_PROGRAM_TYPE_5)
	{
		*program_id = 5;
	}
	else
	{
		SSDK_ERROR("Invalid program type %d\n", type);
		return SW_BAD_VALUE;
	}
	return SW_OK;
}

static a_bool_t
_is_program_entry_equal(a_uint32_t dev_id,
		fal_tunnel_program_entry_t * entry1, fal_tunnel_program_entry_t * entry2)
{
	if (entry1->ip_ver == entry2->ip_ver &&
		entry1->outer_hdr_type == entry2->outer_hdr_type &&
		entry1->protocol== entry2->protocol &&
		entry1->protocol_mask == entry2->protocol_mask)
	{ /* equal */
		return A_TRUE;
	}
	else
	{ /* not equal */
		return A_FALSE;
	}
}

static sw_error_t
_get_program_entry_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_tunnel_program_entry_t * entry)
{
	sw_error_t rv = SW_OK;
	union tpr_hdr_match_0_u match_0 = {0};
	union tpr_hdr_match_1_u match_1 = {0};
	union tpr_hdr_match_2_u match_2 = {0};
	a_uint32_t outer_hdr_type;

	rv = appe_tpr_hdr_match_0_get(dev_id, index, &match_0);
	if(rv != SW_OK)
	{
		aos_mem_zero(&match_0, sizeof (match_0));
	}
	rv = appe_tpr_hdr_match_1_get(dev_id, index, &match_1);
	if(rv != SW_OK)
	{
		aos_mem_zero(&match_1, sizeof (match_1));
	}
	rv = appe_tpr_hdr_match_2_get(dev_id, index, &match_2);
	if(rv != SW_OK)
	{
		aos_mem_zero(&match_2, sizeof (match_2));
	}

	entry->ip_ver = match_0.bf.ip_ver;
	entry->protocol = match_1.bf.protocol;
	entry->protocol_mask = match_2.bf.mask;
	outer_hdr_type = match_0.bf.cur_hdr_type;

	SW_RTN_ON_ERROR(_program_outer_hdr_type_hw_2_sw(dev_id,
				outer_hdr_type, &entry->outer_hdr_type));

	return SW_OK;
}

static sw_error_t
_set_program_entry_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_tunnel_program_entry_t * entry)
{
	union tpr_hdr_match_0_u match_0 = {0};
	union tpr_hdr_match_1_u match_1 = {0};
	union tpr_hdr_match_2_u match_2 = {0};
	a_uint32_t outer_hdr_type;

	SW_RTN_ON_ERROR(_program_outer_hdr_type_sw_2_hw(dev_id,
				entry->outer_hdr_type, &outer_hdr_type));

	match_0.bf.ip_ver = entry->ip_ver;
	match_0.bf.cur_hdr_type = outer_hdr_type;
	match_1.bf.protocol = entry->protocol;
	match_2.bf.mask = entry->protocol_mask;

	SW_RTN_ON_ERROR(appe_tpr_hdr_match_0_set(dev_id, index, &match_0));
	SW_RTN_ON_ERROR(appe_tpr_hdr_match_1_set(dev_id, index, &match_1));
	SW_RTN_ON_ERROR(appe_tpr_hdr_match_2_set(dev_id, index, &match_2));

	return SW_OK;
}

static a_bool_t
_is_program_udf_rule_equal(a_uint32_t dev_id,
		fal_tunnel_program_udf_t * udf1, fal_tunnel_program_udf_t * udf2)
{
	a_uint32_t idx;
	if (udf1->field_flag == udf2->field_flag)
	{
		for (idx = 0; idx < TUNNEL_PROGRAM_UDF_NUM; idx++)
		{
			if((udf1->udf_val[idx] != udf2->udf_val[idx]) ||
				(udf1->udf_mask[idx] != udf2->udf_mask[idx]))
			{ /* not equal */
				return A_FALSE;
			}
		}
		/* equal */
		return A_TRUE;
	}
	else
	{ /* not equal */
		return A_FALSE;
	}
}

static a_bool_t
_is_program_udf_action_equal(a_uint32_t dev_id,
		fal_tunnel_program_udf_t * udf1, fal_tunnel_program_udf_t * udf2)
{
	if (udf1->action_flag == udf2->action_flag &&
		udf1->inner_hdr_type == udf2->inner_hdr_type &&
		udf1->udf_hdr_len == udf2->udf_hdr_len)
	{ /* equal */
		return A_TRUE;
	}
	else
	{ /* not equal */
		return A_FALSE;
	}
}

static sw_error_t
_get_program_udf_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_tunnel_program_udf_t * udf, a_uint32_t * program_id)
{
	a_uint32_t inner_hdr_type;
	union tpr_program_udf_data_0_u data_0 = {0};
	union tpr_program_udf_data_1_u data_1 = {0};
	union tpr_program_udf_mask_0_u mask_0 = {0};
	union tpr_program_udf_mask_1_u mask_1 = {0};
	union tpr_program_udf_action_u action = {0};

	SW_RTN_ON_ERROR(appe_tpr_program_udf_data_0_get(dev_id, index, &data_0));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_data_1_get(dev_id, index, &data_1));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_mask_0_get(dev_id, index, &mask_0));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_mask_1_get(dev_id, index, &mask_1));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_action_get(dev_id, index, &action));

	/* get udf rule */
	if(data_1.bf.udf0_valid == 1)
	{
		FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(udf->field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF0);
		udf->udf_val[0] = data_0.bf.data0;
		udf->udf_mask[0] = mask_0.bf.mask0;
	}
	if(data_1.bf.udf1_valid == 1)
	{
		FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(udf->field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF1);
		udf->udf_val[1] = data_0.bf.data1;
		udf->udf_mask[1] = mask_0.bf.mask1;
	}
	if(data_1.bf.udf2_valid == 1)
	{
		FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(udf->field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF2);
		udf->udf_val[2] = data_1.bf.data2;
		udf->udf_mask[2] = mask_1.bf.mask2;
	}
	if(data_1.bf.comp_mode == 1)
	{
		FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_SET(udf->field_flag,
					FAL_TUNNEL_PROGRAM_UDF_FIELD_INVERSE);
	}

	/* get udf action */
	if(action.bf.next_hdr_type_valid)
	{
		FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(udf->action_flag,
				FAL_TUNNEL_PROGRAM_UDF_ACTION_INNER_HDR_TYPE);
		inner_hdr_type = action.bf.next_hdr_type;
		SW_RTN_ON_ERROR(_program_inner_hdr_type_hw_2_sw(dev_id,
				inner_hdr_type, &udf->inner_hdr_type));
	}
	if(action.bf.hdr_len_valid)
	{
		FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(udf->action_flag,
				FAL_TUNNEL_PROGRAM_UDF_ACTION_UDF_HDR_LEN);
		udf->udf_hdr_len = action.bf.hdr_len*2;
	}
	if(action.bf.exception_en)
	{
		FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_SET(udf->action_flag,
				FAL_TUNNEL_PROGRAM_UDF_ACTION_EXCEPTION_EN);
	}

	/* get udf program_id */
	*program_id = data_1.bf.program_id;

	return SW_OK;
}

static sw_error_t
_set_program_udf_by_index(a_uint32_t dev_id,
		a_uint32_t index, fal_tunnel_program_udf_t * udf, a_uint32_t program_id)
{
	a_uint32_t inner_hdr_type;
	union tpr_program_udf_data_0_u data_0 = {0};
	union tpr_program_udf_data_1_u data_1 = {0};
	union tpr_program_udf_mask_0_u mask_0 = {0};
	union tpr_program_udf_mask_1_u mask_1 = {0};
	union tpr_program_udf_action_u action = {0};

	if(udf->udf_hdr_len%2)
	{
		SSDK_ERROR("Only support even data udf hdr len %d\n", udf->udf_hdr_len);
		return SW_BAD_VALUE;
	}

	/* set udf rule field */
	if(FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_TST(udf->field_flag,
				FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF0))
	{
		data_1.bf.udf0_valid = 1;
		mask_1.bf.udf0_valid_mask =1;
		data_0.bf.data0 = udf->udf_val[0];
		mask_0.bf.mask0 = udf->udf_mask[0];
	}
	if(FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_TST(udf->field_flag,
				FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF1))
	{
		data_1.bf.udf1_valid = 1;
		mask_1.bf.udf1_valid_mask =1;
		data_0.bf.data1= udf->udf_val[1];
		mask_0.bf.mask1= udf->udf_mask[1];
	}
	if(FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_TST(udf->field_flag,
				FAL_TUNNEL_PROGRAM_UDF_FIELD_UDF2))
	{
		data_1.bf.udf2_valid = 1;
		mask_1.bf.udf2_valid_mask =1;
		data_1.bf.data2= udf->udf_val[2];
		mask_1.bf.mask2= udf->udf_mask[2];
	}
	if(FAL_TUNNEL_PROGRAM_UDF_FIELD_FLG_TST(udf->field_flag,
				FAL_TUNNEL_PROGRAM_UDF_FIELD_INVERSE))
	{
		data_1.bf.comp_mode = 1;
	}

	/* set udf action */
	if(FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_TST(udf->action_flag,
				FAL_TUNNEL_PROGRAM_UDF_ACTION_INNER_HDR_TYPE))
	{
		action.bf.next_hdr_type_valid = 1;
		SW_RTN_ON_ERROR(_program_inner_hdr_type_sw_2_hw(dev_id,
				udf->inner_hdr_type, &inner_hdr_type));
		action.bf.next_hdr_type = inner_hdr_type;
	}
	if(FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_TST(udf->action_flag,
				FAL_TUNNEL_PROGRAM_UDF_ACTION_UDF_HDR_LEN))
	{
		action.bf.hdr_len_valid = 1;
		action.bf.hdr_len = udf->udf_hdr_len/2;
	}
	if(FAL_TUNNEL_PROGRAM_UDF_ACTION_FLG_TST(udf->action_flag,
				FAL_TUNNEL_PROGRAM_UDF_ACTION_EXCEPTION_EN))
	{
		action.bf.exception_en = 1;
	}

	/* set udf program id */
	data_1.bf.program_id = program_id;

	SW_RTN_ON_ERROR(appe_tpr_program_udf_data_0_set(dev_id, index, &data_0));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_data_1_set(dev_id, index, &data_1));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_mask_0_set(dev_id, index, &mask_0));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_mask_1_set(dev_id, index, &mask_1));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_action_set(dev_id, index, &action));

	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_entry_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
	a_uint32_t idx, entry_idx=0, entry_sign;
	a_int32_t program_port_bitmap, program_id;
	fal_tunnel_program_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));
	SW_RTN_ON_ERROR(appe_tpr_program_hdr_hdr_type_map_get(dev_id,
					program_id, &program_port_bitmap));

	entry_sign = 0;
	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_ENTRY_NUM; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_program_entry_t));
		if (g_program_entry_used_map[dev_id][idx] != 0)
		{
			SW_RTN_ON_ERROR(_get_program_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == _is_program_entry_equal(dev_id, &temp_entry, entry))
			{
				if (program_port_bitmap & (1 << idx))
				{
					return SW_ALREADY_EXIST;
				}
				else
				{
					SSDK_DEBUG("Find entry idx=%d, used_map=0x%x",
							idx, g_program_entry_used_map[dev_id][idx]);
					program_port_bitmap |= (0x1 << idx);
					g_program_entry_used_map[dev_id][idx] |= (0x1 << program_id);
					return appe_tpr_program_hdr_hdr_type_map_set(dev_id,
							program_id, program_port_bitmap);
				}
			}
		}
		else
		{
			if (entry_sign == 0)
			{
				entry_idx = idx;
				entry_sign = 1;
			}
		}
	}

	if (entry_sign == 0)
	{
		return SW_NO_RESOURCE;
	}

	/* set an entry and map it*/
	SW_RTN_ON_ERROR(_set_program_entry_by_index(dev_id, entry_idx, entry));
	program_port_bitmap |= (0x1 << entry_idx);
	g_program_entry_used_map[dev_id][entry_idx] |= (0x1 << program_id);
	return appe_tpr_program_hdr_hdr_type_map_set(dev_id,
					program_id, program_port_bitmap);
}

sw_error_t
adpt_appe_tunnel_program_entry_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
	a_int32_t idx, program_port_bitmap, program_id;
	fal_tunnel_program_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));
	SW_RTN_ON_ERROR(appe_tpr_program_hdr_hdr_type_map_get(dev_id,
					program_id, &program_port_bitmap));

	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_ENTRY_NUM; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_program_entry_t));
		if (program_port_bitmap & (1 << idx))
		{
			SW_RTN_ON_ERROR(_get_program_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == _is_program_entry_equal(dev_id, &temp_entry, entry))
			{
				program_port_bitmap &= ~(0x1 << idx);
				g_program_entry_used_map[dev_id][idx] &= ~(0x1 <<program_id);
				SW_RTN_ON_ERROR(appe_tpr_program_hdr_hdr_type_map_set(dev_id,
								program_id, program_port_bitmap));
				break;
			}
		}
	}

	if (idx == ADPT_TUNNEL_PROGRAM_ENTRY_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_entry_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
	a_uint32_t idx, program_id, program_port_bitmap = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));
	SW_RTN_ON_ERROR(appe_tpr_program_hdr_hdr_type_map_get(dev_id,
					program_id, &program_port_bitmap));

	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_ENTRY_NUM; idx++)
	{
		if (program_port_bitmap & (1 << idx))
		{
			SW_RTN_ON_ERROR(_get_program_entry_by_index(dev_id, idx, entry));
			break;
		}
	}

	if (idx == ADPT_TUNNEL_PROGRAM_ENTRY_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_entry_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_entry_t * entry)
{
	a_bool_t sign_tag = A_FALSE;
	a_uint32_t idx, program_id, program_port_bitmap = 0;
	fal_tunnel_program_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));
	SW_RTN_ON_ERROR(appe_tpr_program_hdr_hdr_type_map_get(dev_id,
					program_id, &program_port_bitmap));

	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_ENTRY_NUM; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_program_entry_t));
		if (program_port_bitmap & (1 << idx))
		{
			SW_RTN_ON_ERROR(_get_program_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == sign_tag)
			{
				aos_mem_copy(entry, &temp_entry,
						sizeof (fal_tunnel_program_entry_t));
				break;
			}
			if (A_TRUE == _is_program_entry_equal(dev_id, &temp_entry, entry))
			{
				sign_tag = A_TRUE;
			}
		}
	}

	if (idx == ADPT_TUNNEL_PROGRAM_ENTRY_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_cfg_set(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg)
{
	union tpr_program_result_u rlt = {0};
	union tpr_program_udf_ctrl_u ctl = {0};
	a_uint32_t i, program_id, inner_hdr_type;

	ADPT_DEV_ID_CHECK(dev_id);

	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));

	for (i = 0; i < TUNNEL_PROGRAM_UDF_NUM; i++)
	{
		if(cfg->udf_offset[i]%2)
		{
			SSDK_ERROR("Only support even data offset %d\n", cfg->udf_offset[i]);
			return SW_BAD_VALUE;
		}
	}
	if(cfg->basic_hdr_len%2)
	{
		SSDK_ERROR("Only support even data basic hdr length %d\n",
							cfg->basic_hdr_len);
		return SW_BAD_VALUE;
	}
	SW_RTN_ON_ERROR(_program_inner_hdr_type_sw_2_hw(dev_id,
				cfg->inner_hdr_type, &inner_hdr_type));

	rlt.bf.next_hdr_mode = cfg->inner_type_mode;
	rlt.bf.next_hdr_type = inner_hdr_type;
	rlt.bf.hdr_len = cfg->basic_hdr_len/2;
	rlt.bf.hdr_pos_mode = cfg->program_pos_mode;
	rlt.bf.len_unit = cfg->opt_len_unit;
	rlt.bf.len_mask = cfg->opt_len_mask;

	ctl.bf.udf0_offset = cfg->udf_offset[0]/2;
	ctl.bf.udf1_offset = cfg->udf_offset[1]/2;
	ctl.bf.udf2_offset = cfg->udf_offset[2]/2;

	SW_RTN_ON_ERROR(appe_tpr_program_result_set(dev_id, program_id, &rlt));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_ctrl_set(dev_id, program_id, &ctl));

	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_cfg_get(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_cfg_t * cfg)
{
	union tpr_program_result_u rlt = {0};
	union tpr_program_udf_ctrl_u ctl = {0};
	a_uint32_t program_id, inner_hdr_type;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));
	SW_RTN_ON_ERROR(appe_tpr_program_result_get(dev_id, program_id, &rlt));
	SW_RTN_ON_ERROR(appe_tpr_program_udf_ctrl_get(dev_id, program_id, &ctl));

	cfg->inner_type_mode = rlt.bf.next_hdr_mode;
	inner_hdr_type = rlt.bf.next_hdr_type;
	cfg->basic_hdr_len = rlt.bf.hdr_len*2;/*unit is 2bytes*/
	cfg->program_pos_mode = rlt.bf.hdr_pos_mode;
	cfg->opt_len_unit = rlt.bf.len_unit;
	cfg->opt_len_mask = rlt.bf.len_mask;

	cfg->udf_offset[0] = ctl.bf.udf0_offset*2;
	cfg->udf_offset[1] = ctl.bf.udf1_offset*2;
	cfg->udf_offset[2] = ctl.bf.udf2_offset*2;

	SW_RTN_ON_ERROR(_program_inner_hdr_type_hw_2_sw(dev_id,
				inner_hdr_type, &cfg->inner_hdr_type));

	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_udf_add(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
	a_uint32_t idx, entry_idx=0;
	a_int32_t program_id, temp_program_id;
	fal_tunnel_program_udf_t temp_udf = {0};
	a_bool_t entry_sign = A_FALSE;

	ADPT_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));

	if (A_TRUE == _is_program_udf_action_equal(dev_id, &temp_udf, udf))
	{ /* action is empty */
		SSDK_DEBUG("invalid program udf!\n");
		return SW_NOT_SUPPORTED;
	}
	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM; idx++)
	{
		aos_mem_zero(&temp_udf, sizeof(fal_tunnel_program_udf_t));
		if (A_TRUE == g_program_udf_entry_inuse[dev_id][idx])
		{
			SW_RTN_ON_ERROR(_get_program_udf_by_index(dev_id,
						idx, &temp_udf, &temp_program_id));
			if (A_TRUE == _is_program_udf_rule_equal(dev_id, &temp_udf, udf))
			{
				if (temp_program_id == program_id)
				{
					if (A_TRUE == _is_program_udf_action_equal(dev_id,
						&temp_udf, udf))
					{ /* this udf rule has existing for the program type */
						return SW_ALREADY_EXIST;
					}
					else
					{ /* update action */
						SW_RTN_ON_ERROR(_set_program_udf_by_index(dev_id,
						idx, udf, program_id));
						return SW_OK;
					}
				}
			}
		}
		else
		{
			if (entry_sign == A_FALSE)
			{
				entry_idx = idx;
				entry_sign = A_TRUE;
			}
		}
	}

	if (entry_sign == A_FALSE)
	{
		return SW_NO_RESOURCE;
	}

	/* set an program udf rule */
	SW_RTN_ON_ERROR(_set_program_udf_by_index(dev_id, entry_idx, udf, program_id));
	/* record this udf rule entry has been used */
	g_program_udf_entry_inuse[dev_id][entry_idx] = A_TRUE;
	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_udf_del(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
	a_uint32_t idx;
	a_int32_t program_id, temp_program_id;
	fal_tunnel_program_udf_t temp_udf = {0};
	fal_tunnel_program_udf_t empty_udf = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));

	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM; idx++)
	{
		aos_mem_zero(&temp_udf, sizeof(fal_tunnel_program_udf_t));
		if (A_TRUE == g_program_udf_entry_inuse[dev_id][idx])
		{
			SW_RTN_ON_ERROR(_get_program_udf_by_index(dev_id,
						idx, &temp_udf, &temp_program_id));
			if (A_TRUE == _is_program_udf_rule_equal(dev_id, &temp_udf, udf) &&
				A_TRUE == _is_program_udf_action_equal(dev_id, &temp_udf, udf))
			{
				if (temp_program_id == program_id)
				{ /* delete this udf rule entry */
					SW_RTN_ON_ERROR(_set_program_udf_by_index(dev_id,
						idx, &empty_udf, 0));
					g_program_udf_entry_inuse[dev_id][idx] = A_FALSE;
					return SW_OK;
				}
			}
		}
	}

	if (idx == ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
_adpt_appe_tunnel_program_udf_get(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf, a_bool_t sign_tag)
{
	a_uint32_t idx;
	a_int32_t program_id, temp_program_id;
	fal_tunnel_program_udf_t temp_udf = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(udf);
	SW_RTN_ON_ERROR(_program_type_2_id(dev_id, type, &program_id));

	for (idx = 0; idx < ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM; idx++)
	{
		aos_mem_zero(&temp_udf, sizeof(fal_tunnel_program_udf_t));
		if (A_TRUE == g_program_udf_entry_inuse[dev_id][idx])
		{
			SW_RTN_ON_ERROR(_get_program_udf_by_index(dev_id,
					idx, &temp_udf, &temp_program_id));
			if (temp_program_id == program_id)
			{
				if (A_TRUE == sign_tag)
				{
					aos_mem_copy(udf, &temp_udf,
							sizeof (fal_tunnel_program_udf_t));
					break;
				}
				if (A_TRUE == _is_program_udf_rule_equal(dev_id, &temp_udf, udf) &&
				A_TRUE == _is_program_udf_action_equal(dev_id, &temp_udf, udf))
				{
					sign_tag = A_TRUE;
				}
			}
		}
	}

	if (idx == ADPT_TUNNEL_PROGRAM_UDF_ENTRY_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_tunnel_program_udf_getfirst(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
	return _adpt_appe_tunnel_program_udf_get(dev_id, type, udf, A_TRUE);
}

sw_error_t
adpt_appe_tunnel_program_udf_getnext(a_uint32_t dev_id,
		fal_tunnel_program_type_t type, fal_tunnel_program_udf_t * udf)
{
	return _adpt_appe_tunnel_program_udf_get(dev_id, type, udf, A_FALSE);
}

sw_error_t adpt_appe_tunnel_program_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

	p_adpt_api->adpt_tunnel_program_entry_add = adpt_appe_tunnel_program_entry_add;
	p_adpt_api->adpt_tunnel_program_entry_del = adpt_appe_tunnel_program_entry_del;
	p_adpt_api->adpt_tunnel_program_entry_getfirst =
					adpt_appe_tunnel_program_entry_getfirst;
	p_adpt_api->adpt_tunnel_program_entry_getnext =
					adpt_appe_tunnel_program_entry_getnext;
	p_adpt_api->adpt_tunnel_program_cfg_set = adpt_appe_tunnel_program_cfg_set;
	p_adpt_api->adpt_tunnel_program_cfg_get = adpt_appe_tunnel_program_cfg_get;
	p_adpt_api->adpt_tunnel_program_udf_add = adpt_appe_tunnel_program_udf_add;
	p_adpt_api->adpt_tunnel_program_udf_del = adpt_appe_tunnel_program_udf_del;
	p_adpt_api->adpt_tunnel_program_udf_getfirst =
					adpt_appe_tunnel_program_udf_getfirst;
	p_adpt_api->adpt_tunnel_program_udf_getnext =
					adpt_appe_tunnel_program_udf_getnext;

	return SW_OK;
}


/**
 * @}
 */
