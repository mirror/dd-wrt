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
#include "sw.h"
#include "adpt.h"
#include "adpt_appe_vsi.h"
#include "appe_vsi.h"
#include "appe_l2_vp.h"

sw_error_t
adpt_appe_vsi_bridge_vsi_get(a_uint32_t dev_id, a_uint32_t vsi_id,
	fal_vsi_bridge_vsi_t *bridge_vsi)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(bridge_vsi);

	rv = appe_vsi_remap_tbl_vsi_remap_en_get(dev_id, vsi_id,
		&(bridge_vsi->bridge_vsi_enable));
	SW_RTN_ON_ERROR(rv);

	rv = appe_vsi_remap_tbl_br_vsi_get(dev_id, vsi_id,
		&(bridge_vsi->bridge_vsi_id));

	return rv;
}

sw_error_t
adpt_appe_vsi_bridge_vsi_set(a_uint32_t dev_id, a_uint32_t vsi_id,
	fal_vsi_bridge_vsi_t *bridge_vsi)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);

	rv = appe_vsi_remap_tbl_vsi_remap_en_set(dev_id, vsi_id,
		bridge_vsi->bridge_vsi_enable);
	SW_RTN_ON_ERROR(rv);

	rv = appe_vsi_remap_tbl_br_vsi_set(dev_id, vsi_id,
		bridge_vsi->bridge_vsi_id);

	return rv;
}

sw_error_t
adpt_appe_vsi_vp_member_set(a_uint32_t dev_id, a_uint32_t vsi_id,
	fal_vsi_member_t *vsi_member)
{
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_2_set(dev_id, vsi_id,
		vsi_member->member_vports[0]));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_3_set(dev_id, vsi_id,
		vsi_member->member_vports[1]));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_4_set(dev_id, vsi_id,
		vsi_member->member_vports[2]));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_5_set(dev_id, vsi_id,
		vsi_member->member_vports[3]));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_6_set(dev_id, vsi_id,
		vsi_member->member_vports[4]));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_7_set(dev_id, vsi_id,
		vsi_member->member_vports[5]));

	return SW_OK;
}

sw_error_t
adpt_appe_vsi_vp_member_get(a_uint32_t dev_id, a_uint32_t vsi_id,
	fal_vsi_member_t *vsi_member)
{
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_2_get(dev_id, vsi_id,
		&(vsi_member->member_vports[0])));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_3_get(dev_id, vsi_id,
		&(vsi_member->member_vports[1])));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_4_get(dev_id, vsi_id,
		&(vsi_member->member_vports[2])));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_5_get(dev_id, vsi_id,
		&(vsi_member->member_vports[3])));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_6_get(dev_id, vsi_id,
		&(vsi_member->member_vports[4])));
	SW_RTN_ON_ERROR(appe_vsi_remap_tbl_member_port_bitmap_7_get(dev_id, vsi_id,
		&(vsi_member->member_vports[5])));

	return SW_OK;
}

sw_error_t
adpt_appe_vsi_invalidvsi_ctrl_get(a_uint32_t dev_id, fal_port_t port_id,
	fal_vsi_invalidvsi_ctrl_t *invalidvsi_ctrl)
{
	sw_error_t rv = SW_OK;
	a_uint32_t dest_port = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(invalidvsi_ctrl);

	port_id = FAL_PORT_ID_VALUE(port_id);
	rv = appe_l2_vp_port_tbl_invalid_vsi_forwarding_en_get(dev_id,
		port_id, &(invalidvsi_ctrl->dest_en));
	SW_RTN_ON_ERROR(rv);
	rv = appe_l2_vp_port_tbl_dst_info_get(dev_id, port_id, &dest_port);
	SW_RTN_ON_ERROR(rv);
	invalidvsi_ctrl->dest_info.dest_info_type = FAL_DEST_INFO_PORT_ID;
	invalidvsi_ctrl->dest_info.dest_info_value = dest_port;

	return rv;
}

sw_error_t
adpt_appe_vsi_invalidvsi_ctrl_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_vsi_invalidvsi_ctrl_t *invalidvsi_ctrl)
{
	sw_error_t rv = SW_OK;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(invalidvsi_ctrl);

	port_id = FAL_PORT_ID_VALUE(port_id);
	rv = appe_l2_vp_port_tbl_invalid_vsi_forwarding_en_set(dev_id,
			port_id, invalidvsi_ctrl->dest_en);
	SW_RTN_ON_ERROR(rv);
	if(invalidvsi_ctrl->dest_info.dest_info_type != FAL_DEST_INFO_PORT_ID)
	{
		SSDK_ERROR ("dest_info_type:0x%x is not supported\n",
			invalidvsi_ctrl->dest_info.dest_info_type);
		return SW_NOT_SUPPORTED;
	}
	rv = appe_l2_vp_port_tbl_dst_info_set(dev_id, port_id,
		invalidvsi_ctrl->dest_info.dest_info_value);

	return rv;
}
