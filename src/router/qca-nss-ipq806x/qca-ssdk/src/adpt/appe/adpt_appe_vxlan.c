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
#include "appe_vxlan_reg.h"
#include "appe_vxlan.h"
#ifdef IN_GENEVE
#include "appe_geneve_reg.h"
#include "appe_geneve.h"
#endif
#define ADPT_VXLAN_ENTRY_MAX_NUM 6

sw_error_t
adpt_appe_get_udp_entry_by_index(a_uint32_t dev_id,
			a_uint32_t index, fal_tunnel_udp_entry_t * entry)
{
	sw_error_t rv = SW_OK;
	union udp_port_cfg_u cfg_entry = {0};

	rv = appe_udp_port_cfg_get(dev_id, index, &cfg_entry);

	if(rv != SW_OK)
	{
		aos_mem_zero(&cfg_entry, sizeof (cfg_entry));
	}
	entry->ip_ver = cfg_entry.bf.ip_ver;
	entry->udp_type = cfg_entry.bf.udp_type;
	entry->l4_port_type = cfg_entry.bf.port_type;
	entry->l4_port = cfg_entry.bf.port_value;

	return rv;
}

sw_error_t
adpt_appe_set_udp_entry_by_index(a_uint32_t dev_id,
			a_uint32_t index, fal_tunnel_udp_entry_t * entry)
{
	union udp_port_cfg_u cfg_entry = {0};

	cfg_entry.bf.ip_ver = entry->ip_ver;
	cfg_entry.bf.udp_type = entry->udp_type;
	cfg_entry.bf.port_type = entry->l4_port_type;
	cfg_entry.bf.port_value = entry->l4_port;

	return appe_udp_port_cfg_set(dev_id, index, &cfg_entry);
}

a_bool_t
adpt_appe_is_udp_entry_equal(a_uint32_t dev_id,
			fal_tunnel_udp_entry_t * entry1, fal_tunnel_udp_entry_t * entry2)
{
	if (entry1->ip_ver == entry2->ip_ver &&
		entry1->udp_type == entry2->udp_type &&
		entry1->l4_port_type == entry2->l4_port_type &&
		entry1->l4_port == entry2->l4_port)
	{
		return A_TRUE;
	}
	else
	{
		return A_FALSE;
	}
}

a_bool_t
adpt_appe_is_udp_entry_inuse(a_uint32_t dev_id, a_uint32_t index)
{
	sw_error_t rv = SW_OK;
	a_uint32_t port_bitmap, port_bitmap0, port_bitmap1, port_bitmap2 = 0;

	rv = appe_tpr_vxlan_cfg_udp_port_map_get(dev_id, &port_bitmap0);
	if (rv != SW_OK)
	{
		port_bitmap0 = 0;
	}

	rv = appe_tpr_vxlan_gpe_cfg_udp_port_map_get(dev_id, &port_bitmap1);
	if (rv != SW_OK)
	{
		port_bitmap1 = 0;
	}
#ifdef IN_GENEVE
	rv = appe_tpr_geneve_cfg_udp_port_map_get(dev_id, &port_bitmap2);
	if (rv != SW_OK)
	{
		port_bitmap2 = 0;
	}
#endif
	port_bitmap = port_bitmap0 | port_bitmap1 | port_bitmap2;
	return SW_IS_PBMP_MEMBER(port_bitmap, index);
}

static sw_error_t
_adpt_appe_set_vxlan_udp_port_bitmap(a_uint32_t dev_id,
			fal_vxlan_type_t type, a_uint32_t port_bitmap)
{
	if (type == FAL_VXLAN)
	{
		return appe_tpr_vxlan_cfg_udp_port_map_set(dev_id, port_bitmap);
	}
	else if (type == FAL_VXLAN_GPE)
	{
		return appe_tpr_vxlan_gpe_cfg_udp_port_map_set(dev_id, port_bitmap);
	}
	else
	{
		SSDK_ERROR("Not supported vxlan type %d\n", type);
		return SW_BAD_VALUE;
	}
}

static sw_error_t
_adpt_appe_get_vxlan_udp_port_bitmap(a_uint32_t dev_id,
			fal_vxlan_type_t type, a_uint32_t * port_bitmap)
{
	if (type == FAL_VXLAN)
	{
		return appe_tpr_vxlan_cfg_udp_port_map_get(dev_id, port_bitmap);
	}
	else if (type == FAL_VXLAN_GPE)
	{
		return appe_tpr_vxlan_gpe_cfg_udp_port_map_get(dev_id, port_bitmap);
	}
	else
	{
		SSDK_ERROR("Not supported vxlan type %d\n", type);
		return SW_BAD_VALUE;
	}
}

sw_error_t
adpt_appe_vxlan_entry_add(a_uint32_t dev_id, fal_vxlan_type_t type,
						fal_tunnel_udp_entry_t * entry)
{
	a_uint32_t idx, entry_idx=0, entry_sign;
	a_bool_t entry_inuse;
	a_int32_t vxlan_port_bitmap;
	fal_tunnel_udp_entry_t temp_entry;

	ADPT_DEV_ID_CHECK(dev_id);

	SW_RTN_ON_ERROR(_adpt_appe_get_vxlan_udp_port_bitmap(dev_id, type, &vxlan_port_bitmap));

	entry_sign = 0;
	for (idx = 0; idx < ADPT_VXLAN_ENTRY_MAX_NUM; idx++)
	{
		aos_mem_zero(&temp_entry, sizeof (fal_tunnel_udp_entry_t));
		entry_inuse = adpt_appe_is_udp_entry_inuse(dev_id, idx);
		if (A_TRUE == entry_inuse)
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == adpt_appe_is_udp_entry_equal(dev_id, &temp_entry, entry))
			{
				if (SW_IS_PBMP_MEMBER(vxlan_port_bitmap, idx))
				{
					return SW_ALREADY_EXIST;
				}
				else
				{
					vxlan_port_bitmap |= (0x1 << idx);
					return _adpt_appe_set_vxlan_udp_port_bitmap(dev_id,
									type, vxlan_port_bitmap);
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
	vxlan_port_bitmap |= (0x1 << entry_idx);
	return _adpt_appe_set_vxlan_udp_port_bitmap(dev_id, type, vxlan_port_bitmap);
}

sw_error_t
adpt_appe_vxlan_entry_del(a_uint32_t dev_id, fal_vxlan_type_t type,
						fal_tunnel_udp_entry_t * entry)
{
	a_int32_t idx, vxlan_port_bitmap;
	fal_tunnel_udp_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	SW_RTN_ON_ERROR(_adpt_appe_get_vxlan_udp_port_bitmap(dev_id, type, &vxlan_port_bitmap));

	for (idx = 0; idx < ADPT_VXLAN_ENTRY_MAX_NUM; idx++)
	{
		if (SW_IS_PBMP_MEMBER(vxlan_port_bitmap, idx))
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, &temp_entry));
			if (A_TRUE == adpt_appe_is_udp_entry_equal(dev_id, &temp_entry, entry))
			{
				vxlan_port_bitmap &= ~(0x1 << idx);
				SW_RTN_ON_ERROR(_adpt_appe_set_vxlan_udp_port_bitmap(dev_id,
									type, vxlan_port_bitmap));
				break;
			}
		}
	}

	if (idx == ADPT_VXLAN_ENTRY_MAX_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_vxlan_entry_getfirst(a_uint32_t dev_id, fal_vxlan_type_t type,
						fal_tunnel_udp_entry_t * entry)
{
	a_uint32_t idx, vxlan_port_bitmap;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	SW_RTN_ON_ERROR(_adpt_appe_get_vxlan_udp_port_bitmap(dev_id, type, &vxlan_port_bitmap));

	for (idx = 0; idx < ADPT_VXLAN_ENTRY_MAX_NUM; idx++)
	{
		if (SW_IS_PBMP_MEMBER(vxlan_port_bitmap, idx))
		{
			SW_RTN_ON_ERROR(adpt_appe_get_udp_entry_by_index(dev_id, idx, entry));
			break;
		}
	}

	if (idx == ADPT_VXLAN_ENTRY_MAX_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

sw_error_t
adpt_appe_vxlan_entry_getnext(a_uint32_t dev_id, fal_vxlan_type_t type,
						fal_tunnel_udp_entry_t * entry)
{
	a_bool_t sign_tag = A_FALSE;
	a_uint32_t idx, vxlan_port_bitmap = 0;
	fal_tunnel_udp_entry_t temp_entry = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(entry);

	SW_RTN_ON_ERROR(_adpt_appe_get_vxlan_udp_port_bitmap(dev_id, type, &vxlan_port_bitmap));

	for (idx = 0; idx < ADPT_VXLAN_ENTRY_MAX_NUM; idx++)
	{
		if (SW_IS_PBMP_MEMBER(vxlan_port_bitmap, idx))
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

	if (idx == ADPT_VXLAN_ENTRY_MAX_NUM)
	{
		return SW_NOT_FOUND;
	}
	return SW_OK;
}

#ifndef IN_VXLAN_MINI
sw_error_t
adpt_appe_vxlan_gpe_proto_cfg_set(a_uint32_t dev_id, fal_vxlan_gpe_proto_cfg_t * proto_cfg)
{
	union tpr_vxlan_gpe_prot_cfg_u vxlan_gpe_prot = {0};

	ADPT_DEV_ID_CHECK(dev_id);

	SW_RTN_ON_ERROR(appe_tpr_vxlan_gpe_prot_cfg_get(dev_id, &vxlan_gpe_prot));

	vxlan_gpe_prot.bf.ethernet = proto_cfg->ethernet;
	vxlan_gpe_prot.bf.ipv4 = proto_cfg->ipv4;
	vxlan_gpe_prot.bf.ipv6 =proto_cfg->ipv6;

	return appe_tpr_vxlan_gpe_prot_cfg_set(dev_id, &vxlan_gpe_prot);
}

sw_error_t
adpt_appe_vxlan_gpe_proto_cfg_get(a_uint32_t dev_id, fal_vxlan_gpe_proto_cfg_t * proto_cfg)
{
	union tpr_vxlan_gpe_prot_cfg_u vxlan_gpe_prot = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(proto_cfg);

	SW_RTN_ON_ERROR(appe_tpr_vxlan_gpe_prot_cfg_get(dev_id, &vxlan_gpe_prot));

	proto_cfg->ethernet = vxlan_gpe_prot.bf.ethernet;
	proto_cfg->ipv4 = vxlan_gpe_prot.bf.ipv4;
	proto_cfg->ipv6 = vxlan_gpe_prot.bf.ipv6;

	return SW_OK;
}
#endif

sw_error_t adpt_appe_vxlan_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	ADPT_NULL_POINT_CHECK(p_adpt_api);

	p_adpt_api->adpt_vxlan_entry_add = adpt_appe_vxlan_entry_add;
	p_adpt_api->adpt_vxlan_entry_del = adpt_appe_vxlan_entry_del;
	p_adpt_api->adpt_vxlan_entry_getfirst = adpt_appe_vxlan_entry_getfirst;
	p_adpt_api->adpt_vxlan_entry_getnext = adpt_appe_vxlan_entry_getnext;
#ifndef IN_VXLAN_MINI
	p_adpt_api->adpt_vxlan_gpe_proto_cfg_set = adpt_appe_vxlan_gpe_proto_cfg_set;
	p_adpt_api->adpt_vxlan_gpe_proto_cfg_get = adpt_appe_vxlan_gpe_proto_cfg_get;
#endif

	return SW_OK;
}


/**
 * @}
 */
