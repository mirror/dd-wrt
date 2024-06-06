/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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
#include "appe_portvlan.h"
#include "appe_l2_vp.h"

sw_error_t
adpt_appe_portvlan_vpmember_get(a_uint32_t dev_id, fal_port_t port_id, fal_pbmp_t * mem_port_map)
{
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(mem_port_map);
	port_id = FAL_PORT_ID_VALUE(port_id);

	return appe_l2_vp_port_tbl_port_isolation_bitmap_get(dev_id, port_id, mem_port_map);
}

sw_error_t
adpt_appe_portvlan_vpmember_update(a_uint32_t dev_id, fal_port_t port_id, fal_pbmp_t mem_port_map)
{
	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);

	return appe_l2_vp_port_tbl_port_isolation_bitmap_set(dev_id, port_id, (a_uint32_t)mem_port_map);
}

sw_error_t
adpt_appe_portvlan_vpmember_add(a_uint32_t dev_id, fal_port_t port_id, fal_port_t mem_port_id)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);
	mem_port_id = FAL_PORT_ID_VALUE(mem_port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.port_isolation_bitmap |= (0x1 << mem_port_id);

	rv = appe_l2_vp_port_tbl_set(dev_id, port_id, &l2_vp_port_tbl);

	return rv;
}

sw_error_t
adpt_appe_portvlan_vpmember_del(a_uint32_t dev_id, fal_port_t port_id, fal_port_t mem_port_id)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl = {0};

	ADPT_DEV_ID_CHECK(dev_id);
	port_id = FAL_PORT_ID_VALUE(port_id);
	mem_port_id = FAL_PORT_ID_VALUE(mem_port_id);

	rv = appe_l2_vp_port_tbl_get(dev_id, port_id, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.port_isolation_bitmap &= ~(0x1 << mem_port_id);

	rv = appe_l2_vp_port_tbl_set(dev_id, port_id, &l2_vp_port_tbl);

	return rv;
}

#ifndef IN_PORTVLAN_MINI
sw_error_t
adpt_appe_port_vlan_vpgroup_set(a_uint32_t dev_id, a_uint32_t vport_id,
		fal_port_vlan_direction_t direction, a_uint32_t vpgroup_id)
{
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

	switch (direction) {
		case FAL_PORT_VLAN_INGRESS:
			rtn = appe_vlan_port_vp_tbl_vlan_profile_set(dev_id, vport_id, vpgroup_id);
			break;
		case FAL_PORT_VLAN_EGRESS:
			rtn = appe_eg_vp_tbl_xlat_profile_set(dev_id, vport_id, vpgroup_id);
			SW_RTN_ON_ERROR(rtn);
			break;
		case FAL_PORT_VLAN_ALL:
		default:
			rtn = SW_BAD_PARAM;
			break;
	}

	return rtn;
}

sw_error_t
adpt_appe_port_vlan_vpgroup_get(a_uint32_t dev_id, a_uint32_t vport_id,
		fal_port_vlan_direction_t direction, a_uint32_t *vpgroup_id)
{
	sw_error_t rtn = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

	switch (direction) {
		case FAL_PORT_VLAN_INGRESS:
			rtn = appe_vlan_port_vp_tbl_vlan_profile_get(dev_id, vport_id, vpgroup_id);
			SW_RTN_ON_ERROR(rtn);
			break;
		case FAL_PORT_VLAN_EGRESS:
			rtn = appe_eg_vp_tbl_xlat_profile_get(dev_id, vport_id, vpgroup_id);
			SW_RTN_ON_ERROR(rtn);
			break;
		case FAL_PORT_VLAN_ALL:
		default:
			rtn = SW_BAD_PARAM;
			break;
	}

	return rtn;
}

sw_error_t
adpt_appe_portvlan_isol_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_portvlan_isol_ctrl_t *isol_ctrl)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(isol_ctrl);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_value, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.isol_en = isol_ctrl->enable;
	l2_vp_port_tbl.bf.isol_profile = isol_ctrl->group_id;

	rv = appe_l2_vp_port_tbl_set(dev_id, port_value, &l2_vp_port_tbl);

	return rv;
}

sw_error_t
adpt_appe_portvlan_isol_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_portvlan_isol_ctrl_t *isol_ctrl)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(isol_ctrl);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_value, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	isol_ctrl->enable = l2_vp_port_tbl.bf.isol_en;
	isol_ctrl->group_id = l2_vp_port_tbl.bf.isol_profile;

	return rv;
}

sw_error_t
adpt_appe_portvlan_isol_group_set(a_uint32_t dev_id,
		a_uint8_t isol_group_id, a_uint64_t *isol_group_bmp)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(isol_group_bmp);

	rv = appe_vp_isol_tbl_vp_profile_map_set(dev_id,
			isol_group_id, *isol_group_bmp);

	return rv;
}

sw_error_t
adpt_appe_portvlan_isol_group_get(a_uint32_t dev_id,
		a_uint8_t isol_group_id, a_uint64_t *isol_group_bmp)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(isol_group_bmp);

	rv = appe_vp_isol_tbl_vp_profile_map_get(dev_id,
			isol_group_id, isol_group_bmp);

	return rv;
}

sw_error_t
adpt_appe_port_egress_vlan_filter_set(a_uint32_t dev_id,
		fal_port_t port_id, fal_egress_vlan_filter_t *filter)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(filter);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_value, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	l2_vp_port_tbl.bf.eg_vlan_fltr_cmd = filter->membership_filter;

	rv = appe_l2_vp_port_tbl_set(dev_id, port_value, &l2_vp_port_tbl);

	return rv;
}

sw_error_t
adpt_appe_port_egress_vlan_filter_get(a_uint32_t dev_id,
		fal_port_t port_id, fal_egress_vlan_filter_t *filter)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_port_tbl;
	a_uint32_t port_value = FAL_PORT_ID_VALUE(port_id);

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(filter);

	aos_mem_zero(&l2_vp_port_tbl, sizeof(union l2_vp_port_tbl_u));

	rv = appe_l2_vp_port_tbl_get(dev_id, port_value, &l2_vp_port_tbl);
	SW_RTN_ON_ERROR(rv);

	filter->membership_filter = l2_vp_port_tbl.bf.eg_vlan_fltr_cmd;

	return rv;
}
#endif

/**
 * @}
 */
