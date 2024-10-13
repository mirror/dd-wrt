/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022, 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "fal_qm.h"
#include "appe_qm_reg.h"
#include "appe_qm.h"
#include "appe_l2_vp_reg.h"
#include "appe_l2_vp.h"
#include "adpt.h"

#define FLOW_ENQUEUE_MAP_INDEX	512

sw_error_t
adpt_appe_qm_enqueue_config_set(a_uint32_t dev_id, fal_enqueue_cfg_t *enqueue_cfg)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_tbl;
	union port_vsi_enqueue_map_u enqueue_map;
	a_uint32_t index = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enqueue_cfg);

	aos_mem_zero(&l2_vp_tbl, sizeof(union l2_vp_port_tbl_u));
	aos_mem_zero(&enqueue_map, sizeof(union port_vsi_enqueue_map_u));

	switch (enqueue_cfg->rule_entry.enqueue_type) {
		case FAL_ENQUEUE_FLOW:
		case FAL_ENQUEUE_VSI:
			if (enqueue_cfg->rule_entry.enqueue_type == FAL_ENQUEUE_FLOW) {
				index = enqueue_cfg->rule_entry.flow_pri_profile +
					FLOW_ENQUEUE_MAP_INDEX;
			} else {
				index = (enqueue_cfg->rule_entry.vsi_dest.phy_port << 6) |
					enqueue_cfg->rule_entry.vsi_dest.vsi;
			}

			rv = appe_port_vsi_enqueue_map_get(dev_id, index, &enqueue_map);
			SW_RTN_ON_ERROR(rv);

			enqueue_map.bf.enqueue_vp = enqueue_cfg->index_entry.enqueue_vport;
			enqueue_map.bf.enqueue_valid = enqueue_cfg->index_entry.enqueue_en;

			rv = appe_port_vsi_enqueue_map_set(dev_id, index, &enqueue_map);
			SW_RTN_ON_ERROR(rv);
			break;
		case FAL_ENQUEUE_SERVCODE:
			index = enqueue_cfg->rule_entry.dst_port;
			rv = appe_l2_vp_port_tbl_get(dev_id, index, &l2_vp_tbl);
			SW_RTN_ON_ERROR(rv);

			l2_vp_tbl.bf.enq_service_code_en = enqueue_cfg->index_entry.enqueue_en;
			l2_vp_tbl.bf.enq_service_code =
				enqueue_cfg->index_entry.enqueue_servcode.service_code;
			l2_vp_tbl.bf.enq_phy_port =
				enqueue_cfg->index_entry.enqueue_servcode.phy_port;

			rv = appe_l2_vp_port_tbl_set(dev_id, index, &l2_vp_tbl);
			SW_RTN_ON_ERROR(rv);
			break;
		default:
			SSDK_ERROR("Unsupported enqueue type\n");
			break;
	}

	return rv;
}

sw_error_t
adpt_appe_qm_enqueue_config_get(a_uint32_t dev_id, fal_enqueue_cfg_t *enqueue_cfg)
{
	sw_error_t rv = SW_OK;
	union l2_vp_port_tbl_u l2_vp_tbl;
	union port_vsi_enqueue_map_u enqueue_map;
	a_uint32_t index = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enqueue_cfg);

	aos_mem_zero(&l2_vp_tbl, sizeof(union l2_vp_port_tbl_u));
	aos_mem_zero(&enqueue_map, sizeof(union port_vsi_enqueue_map_u));

	switch (enqueue_cfg->rule_entry.enqueue_type) {
		case FAL_ENQUEUE_FLOW:
		case FAL_ENQUEUE_VSI:
			if (enqueue_cfg->rule_entry.enqueue_type == FAL_ENQUEUE_FLOW) {
				index = enqueue_cfg->rule_entry.flow_pri_profile +
					FLOW_ENQUEUE_MAP_INDEX;
			} else {
				index = (enqueue_cfg->rule_entry.vsi_dest.phy_port << 6) |
					enqueue_cfg->rule_entry.vsi_dest.vsi;
			}

			rv = appe_port_vsi_enqueue_map_get(dev_id, index, &enqueue_map);
			SW_RTN_ON_ERROR(rv);

			enqueue_cfg->index_entry.enqueue_vport = enqueue_map.bf.enqueue_vp;
			enqueue_cfg->index_entry.enqueue_en = enqueue_map.bf.enqueue_valid;

			break;
		case FAL_ENQUEUE_SERVCODE:
			index = enqueue_cfg->rule_entry.dst_port;
			rv = appe_l2_vp_port_tbl_get(dev_id, index, &l2_vp_tbl);
			SW_RTN_ON_ERROR(rv);

			enqueue_cfg->index_entry.enqueue_en = l2_vp_tbl.bf.enq_service_code_en;
			enqueue_cfg->index_entry.enqueue_servcode.service_code =
				l2_vp_tbl.bf.enq_service_code;
			enqueue_cfg->index_entry.enqueue_servcode.phy_port =
				l2_vp_tbl.bf.enq_phy_port;

			break;
		default:
			SSDK_ERROR("Unsupported enqueue type\n");
			break;
	}

	return rv;
}
/**
 * @}
 */
