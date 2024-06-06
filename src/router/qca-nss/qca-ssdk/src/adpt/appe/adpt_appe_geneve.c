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
#include "appe_geneve_reg.h"
#include "appe_geneve.h"
#include "adpt_appe_vxlan.h"

#define ADPT_GENEVE_ENTRY_MAX_NUM 6

sw_error_t
adpt_appe_geneve_entry_add(a_uint32_t dev_id, fal_tunnel_udp_entry_t * entry)
{
	a_uint32_t idx, entry_idx=0, entry_sign;
	a_bool_t entry_inuse;
	a_int32_t geneve_port_bitmap;
	fal_tunnel_udp_entry_t temp_entry;

	ADPT_DEV_ID_CHECK(dev_id);

	SW_RTN_ON_ERROR(appe_tpr_geneve_cfg_udp_port_map_get(dev_id, &geneve_port_bitmap));

	entry_sign = 0;
	for (idx = 0; idx < ADPT_GENEVE_ENTRY_MAX_NUM; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udp_entry_t));
		entry_inuse = adpt_appe_is_udp_entry_inuse(dev_id, idx);
		if (A_TRUE == entry_inuse)
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == adpt_appe_is_udp_entry_equal(dev_id, &temp_entry, entry))
			{
				if (SW_IS_PBMP_MEMBER(geneve_port_bitmap, idx))
				{
					return SW_ALREADY_EXIST;
				}
				else
				{
					geneve_port_bitmap |= (0x1 << idx);
					return appe_tpr_geneve_cfg_udp_port_map_set(dev_id,
									geneve_port_bitmap);
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
	SW_RTN_ON_ERROR(adpt_appe_set_udp_entry_by_index(dev_id, entry_idx, entry));
	geneve_port_bitmap |= (0x1 << entry_idx);
	return appe_tpr_geneve_cfg_udp_port_map_set(dev_id, geneve_port_bitmap);
}

sw_error_t
adpt_appe_geneve_entry_del(a_uint32_t dev_id, fal_tunnel_udp_entry_t * entry)
{
	a_int32_t idx, geneve_port_bitmap;
	fal_tunnel_udp_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	SW_RTN_ON_ERROR(appe_tpr_geneve_cfg_udp_port_map_get(dev_id, &geneve_port_bitmap));

	for (idx = 0; idx < ADPT_GENEVE_ENTRY_MAX_NUM; idx++)
	{
		if (SW_IS_PBMP_MEMBER(geneve_port_bitmap, idx))
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == adpt_appe_is_udp_entry_equal(dev_id, &temp_entry, entry))
			{
				geneve_port_bitmap &= ~(0x1 << idx);
				SW_RTN_ON_ERROR(appe_tpr_geneve_cfg_udp_port_map_set(dev_id,
									geneve_port_bitmap));
				break;
			}
		}
	}

	if (idx == ADPT_GENEVE_ENTRY_MAX_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_geneve_entry_getfirst(a_uint32_t dev_id, fal_tunnel_udp_entry_t * entry)
{
	a_uint32_t idx, geneve_port_bitmap;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	SW_RTN_ON_ERROR(appe_tpr_geneve_cfg_udp_port_map_get(dev_id, &geneve_port_bitmap));

	for (idx = 0; idx < ADPT_GENEVE_ENTRY_MAX_NUM; idx++)
	{
		if (SW_IS_PBMP_MEMBER(geneve_port_bitmap, idx))
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, entry));
			break;
		}
	}

	if (idx == ADPT_GENEVE_ENTRY_MAX_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_geneve_entry_getnext(a_uint32_t dev_id, fal_tunnel_udp_entry_t * entry)
{
	a_bool_t sign_tag = A_FALSE;
	a_uint32_t idx, geneve_port_bitmap = 0;
	fal_tunnel_udp_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	SW_RTN_ON_ERROR(appe_tpr_geneve_cfg_udp_port_map_get(dev_id, &geneve_port_bitmap));

	for (idx = 0; idx < ADPT_GENEVE_ENTRY_MAX_NUM; idx++)
	{
		if (SW_IS_PBMP_MEMBER(geneve_port_bitmap, idx))
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == sign_tag)
			{
				aos_mem_copy(entry, &temp_entry,
						sizeof (fal_tunnel_udp_entry_t));
				break;
			}
			if (A_TRUE == adpt_appe_is_udp_entry_equal(dev_id, &temp_entry, entry))
			{
				sign_tag = A_TRUE;
			}
		}
	}

	if (idx == ADPT_GENEVE_ENTRY_MAX_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t adpt_appe_geneve_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

	p_adpt_api->adpt_geneve_entry_add = adpt_appe_geneve_entry_add;
	p_adpt_api->adpt_geneve_entry_del = adpt_appe_geneve_entry_del;
	p_adpt_api->adpt_geneve_entry_getfirst = adpt_appe_geneve_entry_getfirst;
	p_adpt_api->adpt_geneve_entry_getnext = adpt_appe_geneve_entry_getnext;

	return SW_OK;
}


/**
 * @}
 */
