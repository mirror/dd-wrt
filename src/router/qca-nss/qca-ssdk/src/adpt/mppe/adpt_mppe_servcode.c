/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "mppe_servcode_reg.h"
#include "mppe_servcode.h"
#include "hppe_servcode_reg.h"
#include "hppe_servcode.h"
#include "adpt_mppe_servcode.h"

#define ADPT_MAX_SERVCODE_NUM 256

sw_error_t
adpt_mppe_port_servcode_set(a_uint32_t dev_id, fal_port_t port_id,
			a_uint32_t servcode_index)
{
	union tl_vp_service_code_gen_u reg_val = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	if((servcode_index >= ADPT_MAX_SERVCODE_NUM &&
		servcode_index != FAL_SERVCODE_INVALID)||
		(FAL_PORT_ID_VALUE(port_id) >= TL_VP_SERVICE_CODE_GEN_MAX_ENTRY))
	{
		return SW_OUT_OF_RANGE;
	}

	SW_RTN_ON_ERROR(mppe_tl_vp_service_code_gen_get(dev_id,
					FAL_PORT_ID_VALUE(port_id), &reg_val));

	if(FAL_SERVCODE_INVALID == servcode_index)
	{
		reg_val.bf.service_code_en = A_FALSE;
		reg_val.bf.service_code = 0;
	}
	else
	{
		reg_val.bf.service_code_en = A_TRUE;
		reg_val.bf.service_code = servcode_index;
	}
	return mppe_tl_vp_service_code_gen_set(dev_id,
					FAL_PORT_ID_VALUE(port_id), &reg_val);
}

sw_error_t
adpt_mppe_port_servcode_get(a_uint32_t dev_id, fal_port_t port_id,
			a_uint32_t *servcode_index)
{
	union tl_vp_service_code_gen_u reg_val={0};

	ADPT_DEV_ID_CHECK(dev_id);

	if(FAL_PORT_ID_VALUE(port_id) >= TL_VP_SERVICE_CODE_GEN_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	SW_RTN_ON_ERROR(mppe_tl_vp_service_code_gen_get(dev_id,
					FAL_PORT_ID_VALUE(port_id), &reg_val));

	if(reg_val.bf.service_code_en)
	{
		*servcode_index = reg_val.bf.service_code;
	}
	else
	{
		*servcode_index = FAL_SERVCODE_INVALID;
	}
	return SW_OK;
}

sw_error_t
adpt_mppe_servcode_athtag_set(a_uint32_t dev_id, a_uint32_t servcode_index,
			fal_servcode_athtag_t *entry)
{
	union eg_service_tbl_u eg_service_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	if (servcode_index >= EG_SERVICE_TBL_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	SW_RTN_ON_ERROR(hppe_eg_service_tbl_get(dev_id, servcode_index, &eg_service_tbl));

	if (entry->athtag_update_bitmap & BIT(FLD_UPDATE_ATH_TAG_INSERT))
	{
		eg_service_tbl.bf.ath_hdr_insert_dis = !entry->athtag_en;
		eg_service_tbl.bf.field_update_action |= ATHTAG_INSERT_UPDATE;
	}
	else
	{
		eg_service_tbl.bf.ath_hdr_insert_dis = 0;
		eg_service_tbl.bf.field_update_action &= ~ATHTAG_INSERT_UPDATE;
	}
	if (entry->athtag_update_bitmap & BIT(FLD_UPDATE_ATH_TAG_ACTION))
	{
		eg_service_tbl.bf.ath_hdr_type = entry->action;
		eg_service_tbl.bf.field_update_action |= ATHTAG_ACTION_UPDATE;
	}
	else
	{
		eg_service_tbl.bf.ath_hdr_type = 0;
		eg_service_tbl.bf.field_update_action &= ~ATHTAG_ACTION_UPDATE;
	}
	if (entry->athtag_update_bitmap & BIT(FLD_UPDATE_ATH_TAG_BYPASS_FWD_EN))
	{
		eg_service_tbl.bf.ath_from_cpu = entry->bypass_fwd_en;
		eg_service_tbl.bf.field_update_action |= ATHTAG_BYPASS_FWD_EN_UPDATE;
	}
	else
	{
		eg_service_tbl.bf.ath_from_cpu = 0;
		eg_service_tbl.bf.field_update_action &= ~ATHTAG_BYPASS_FWD_EN_UPDATE;
	}
	if (entry->athtag_update_bitmap & BIT(FLD_UPDATE_ATH_TAG_DEST_PORT))
	{
		eg_service_tbl.bf.ath_port_bitmap = entry->dest_port;
		eg_service_tbl.bf.field_update_action |= ATHTAG_DEST_PORT_UPDATE;
	}
	else
	{
		eg_service_tbl.bf.ath_port_bitmap = 0;
		eg_service_tbl.bf.field_update_action &= ~ATHTAG_DEST_PORT_UPDATE;
	}
	if (entry->athtag_update_bitmap & BIT(FLD_UPDATE_ATH_TAG_FIELD_DISABLE))
	{
		eg_service_tbl.bf.ath_disable_bit = entry->field_disable;
		eg_service_tbl.bf.field_update_action |= ATHTAG_FIELD_DISABLE_UPDATE;
	}
	else
	{
		eg_service_tbl.bf.ath_disable_bit = 0;
		eg_service_tbl.bf.field_update_action &= ~ATHTAG_FIELD_DISABLE_UPDATE;
	}
	return hppe_eg_service_tbl_set(dev_id, servcode_index, &eg_service_tbl);
}

sw_error_t
adpt_mppe_servcode_athtag_get(a_uint32_t dev_id, a_uint32_t servcode_index,
			fal_servcode_athtag_t *entry)
{
	union eg_service_tbl_u eg_service_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	if (servcode_index >= EG_SERVICE_TBL_MAX_ENTRY)
	{
		return SW_OUT_OF_RANGE;
	}

	SW_RTN_ON_ERROR(hppe_eg_service_tbl_get(dev_id, servcode_index, &eg_service_tbl));
	if (eg_service_tbl.bf.field_update_action & ATHTAG_INSERT_UPDATE)
	{
		entry->athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_INSERT);
		entry->athtag_en = !eg_service_tbl.bf.ath_hdr_insert_dis;
	}
	if (eg_service_tbl.bf.field_update_action & ATHTAG_ACTION_UPDATE)
	{
		entry->athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_ACTION);
		entry->action = eg_service_tbl.bf.ath_hdr_type;
	}
	if (eg_service_tbl.bf.field_update_action & ATHTAG_BYPASS_FWD_EN_UPDATE)
	{
		entry->athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_BYPASS_FWD_EN);
		entry->bypass_fwd_en = eg_service_tbl.bf.ath_from_cpu;
	}
	if (eg_service_tbl.bf.field_update_action & ATHTAG_DEST_PORT_UPDATE)
	{
		entry->athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_DEST_PORT);
		entry->dest_port = eg_service_tbl.bf.ath_port_bitmap;
	}
	if (eg_service_tbl.bf.field_update_action & ATHTAG_FIELD_DISABLE_UPDATE)
	{
		entry->athtag_update_bitmap |= BIT(FLD_UPDATE_ATH_TAG_FIELD_DISABLE);
		entry->field_disable = eg_service_tbl.bf.ath_disable_bit;
	}
	return SW_OK;
}

/**
 * @}
 */


