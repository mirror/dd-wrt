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
#include "appe_shaper_reg.h"
#include "appe_shaper.h"
#include "adpt.h"
#include "adpt_appe_shaper.h"

sw_error_t
adpt_appe_queue_shaper_ctrl_set(a_uint32_t dev_id,
	fal_shaper_ctrl_t *queue_shaper_ctrl)
{
	union shp_cfg_l0_u shp_cfg_l0;
	sw_error_t rv = SW_OK;

	memset(&shp_cfg_l0, 0, sizeof(shp_cfg_l0));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_shaper_ctrl);

	if ((queue_shaper_ctrl->head < APPE_SHAPER_QUEUE_ID_MIN) ||
		(queue_shaper_ctrl->head > APPE_SHAPER_QUEUE_ID_MAX) ||
		(queue_shaper_ctrl->tail < APPE_SHAPER_QUEUE_ID_MIN) ||
		(queue_shaper_ctrl->tail > APPE_SHAPER_QUEUE_ID_MAX)) {
		return SW_BAD_PARAM;
	}

	rv = appe_shp_cfg_l0_get(dev_id, &shp_cfg_l0);
	SW_RTN_ON_ERROR (rv);

	shp_cfg_l0.bf.l0_shp_ll_head = queue_shaper_ctrl->head;
	shp_cfg_l0.bf.l0_shp_ll_tail = queue_shaper_ctrl->tail;
	rv = appe_shp_cfg_l0_set(dev_id, &shp_cfg_l0);
	SW_RTN_ON_ERROR (rv);

	return rv;
}

sw_error_t
adpt_appe_flow_shaper_ctrl_set(a_uint32_t dev_id,
	fal_shaper_ctrl_t *flow_shaper_ctrl)
{
	union shp_cfg_l1_u shp_cfg_l1;
	sw_error_t rv = SW_OK;

	memset(&shp_cfg_l1, 0, sizeof(shp_cfg_l1));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_shaper_ctrl);

	if ((flow_shaper_ctrl->head < APPE_SHAPER_FLOW_ID_MIN) ||
		(flow_shaper_ctrl->head > APPE_SHAPER_FLOW_ID_MAX) ||
		(flow_shaper_ctrl->tail < APPE_SHAPER_FLOW_ID_MIN) ||
		(flow_shaper_ctrl->tail > APPE_SHAPER_FLOW_ID_MAX)) {
		return SW_BAD_PARAM;
	}

	rv = appe_shp_cfg_l1_get(dev_id, &shp_cfg_l1);
	SW_RTN_ON_ERROR (rv);

	shp_cfg_l1.bf.l1_shp_ll_head = flow_shaper_ctrl->head;
	shp_cfg_l1.bf.l1_shp_ll_tail = flow_shaper_ctrl->tail;
	rv = appe_shp_cfg_l1_set(dev_id, &shp_cfg_l1);
	SW_RTN_ON_ERROR (rv);

	return rv;
}

#ifndef IN_SHAPER_MINI
sw_error_t
adpt_appe_queue_shaper_ctrl_get(a_uint32_t dev_id,
	fal_shaper_ctrl_t *queue_shaper_ctrl)
{
	union shp_cfg_l0_u shp_cfg_l0;
	sw_error_t rv = SW_OK;

	memset(&shp_cfg_l0, 0, sizeof(shp_cfg_l0));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_shaper_ctrl);

	rv = appe_shp_cfg_l0_get(dev_id, &shp_cfg_l0);
	SW_RTN_ON_ERROR (rv);
	queue_shaper_ctrl->head = shp_cfg_l0.bf.l0_shp_ll_head;
	queue_shaper_ctrl->tail = shp_cfg_l0.bf.l0_shp_ll_tail;

	return rv;
}

sw_error_t
adpt_appe_flow_shaper_ctrl_get(a_uint32_t dev_id,
	fal_shaper_ctrl_t *flow_shaper_ctrl)
{
	union shp_cfg_l1_u shp_cfg_l1;
	sw_error_t rv = SW_OK;

	memset(&shp_cfg_l1, 0, sizeof(shp_cfg_l1));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(flow_shaper_ctrl);

	rv = appe_shp_cfg_l1_get(dev_id, &shp_cfg_l1);
	SW_RTN_ON_ERROR (rv);
	flow_shaper_ctrl->head = shp_cfg_l1.bf.l1_shp_ll_head;
	flow_shaper_ctrl->tail = shp_cfg_l1.bf.l1_shp_ll_tail;

	return rv;
}
#endif
