/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
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

#include "sw.h"
#include "appe_policer_reg.h"
#include "appe_policer.h"
#include "adpt.h"
#include "adpt_appe_policer.h"

sw_error_t
adpt_appe_policer_ctrl_set(a_uint32_t dev_id, fal_policer_ctrl_t *ctrl)
{
	union in_meter_head_reg_u in_meter_head_cfg;
	sw_error_t rv = SW_OK;

	memset(&in_meter_head_cfg, 0, sizeof(in_meter_head_cfg));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	if ((ctrl->head < APPE_POLICER_ID_MIN) ||
		(ctrl->head > APPE_POLICER_ID_MAX) ||
		(ctrl->tail < APPE_POLICER_ID_MIN) ||
		(ctrl->tail > APPE_POLICER_ID_MAX)) {
		return SW_BAD_PARAM;
	}

	rv = appe_in_meter_head_reg_get(dev_id, &in_meter_head_cfg);
	SW_RTN_ON_ERROR (rv);

	in_meter_head_cfg.bf.meter_ll_head = ctrl->head;
	in_meter_head_cfg.bf.meter_ll_tail = ctrl->tail;

	rv = appe_in_meter_head_reg_set(dev_id, &in_meter_head_cfg);
	SW_RTN_ON_ERROR (rv);

	return rv;
}

sw_error_t
adpt_appe_policer_ctrl_get(a_uint32_t dev_id, fal_policer_ctrl_t *ctrl)
{
	union in_meter_head_reg_u in_meter_head_cfg;
	sw_error_t rv = SW_OK;

	memset(&in_meter_head_cfg, 0, sizeof(in_meter_head_cfg));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(ctrl);

	rv = appe_in_meter_head_reg_get(dev_id, &in_meter_head_cfg);
	SW_RTN_ON_ERROR (rv);

	ctrl->head = in_meter_head_cfg.bf.meter_ll_head;
	ctrl->tail = in_meter_head_cfg.bf.meter_ll_tail;

	return rv;
}

#ifndef IN_POLICER_MINI
sw_error_t
adpt_appe_policer_priority_remap_get(a_uint32_t dev_id, fal_policer_priority_t *priority,
	fal_policer_remap_t *remap)
{
	union dscp_remap_tbl_u dscp_remap_tbl;
	sw_error_t rv = SW_OK;
	a_uint32_t index;

	memset(&dscp_remap_tbl, 0, sizeof(dscp_remap_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(priority);
	ADPT_NULL_POINT_CHECK(remap);

	index = ((priority->internal_pri << 3) + (priority->internal_dp << 1) + (priority->meter_color));
	rv = appe_dscp_remap_tbl_get(dev_id, index, &dscp_remap_tbl);
	SW_RTN_ON_ERROR (rv);

	remap->dscp = dscp_remap_tbl.bf.dscp;
	remap->pcp = dscp_remap_tbl.bf.pcp;
	remap->dei = dscp_remap_tbl.bf.dei;

	return SW_OK;;
}

sw_error_t
adpt_appe_policer_priority_remap_set(a_uint32_t dev_id, fal_policer_priority_t *priority,
	fal_policer_remap_t *remap)
{
	union dscp_remap_tbl_u dscp_remap_tbl;
	sw_error_t rv = SW_OK;
	a_uint32_t index;

	memset(&dscp_remap_tbl, 0, sizeof(dscp_remap_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(priority);
	ADPT_NULL_POINT_CHECK(remap);

	index = ((priority->internal_pri << 3) + (priority->internal_dp << 1) + (priority->meter_color));
	rv = appe_dscp_remap_tbl_get(dev_id, index, &dscp_remap_tbl);
	SW_RTN_ON_ERROR (rv);

	dscp_remap_tbl.bf.dscp = remap->dscp;
	dscp_remap_tbl.bf.pcp = remap->pcp;
	dscp_remap_tbl.bf.dei = remap->dei;

	rv = appe_dscp_remap_tbl_set(dev_id, index, &dscp_remap_tbl);
	SW_RTN_ON_ERROR (rv);

	return rv;
}
#endif

