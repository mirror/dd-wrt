/*
 * Copyright (c) 2016-2017, 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "ssdk_dts.h"
#include "fal_qos.h"
#include "hppe_qos_reg.h"
#include "hppe_qos.h"
#include "hppe_shaper_reg.h"
#include "hppe_shaper.h"
#include "adpt.h"
#include "adpt_hppe.h"
#if defined(CPPE) || defined(APPE)
#include "adpt_cppe_qos.h"
#endif

static fal_queue_bmp_t port_queue_map[8] = {0};

sw_error_t
adpt_hppe_l1_flow_map_get(a_uint32_t dev_id,
					a_uint32_t node_id,
					fal_port_t *port_id,
					fal_qos_scheduler_cfg_t *scheduler_cfg)
{
	union l1_flow_map_tbl_u l1_flow_map_tbl;
	union l1_c_sp_cfg_tbl_u l1_c_sp_cfg_tbl;
	union l1_e_sp_cfg_tbl_u l1_e_sp_cfg_tbl;
	union l1_flow_port_map_tbl_u port_map;
	union l1_comp_cfg_tbl_u l1_comp_cfg_tbl;
	a_uint32_t c_sp_id, e_sp_id;

	memset(&l1_flow_map_tbl, 0, sizeof(l1_flow_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(scheduler_cfg);
	if (node_id >= L1_FLOW_MAP_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	hppe_l1_flow_map_tbl_get(dev_id, node_id, &l1_flow_map_tbl);
	scheduler_cfg->e_drr_wt = l1_flow_map_tbl.bf.e_drr_wt;
	scheduler_cfg->c_drr_wt = l1_flow_map_tbl.bf.c_drr_wt;
	scheduler_cfg->e_pri = l1_flow_map_tbl.bf.e_pri;
	scheduler_cfg->c_pri = l1_flow_map_tbl.bf.c_pri;
	scheduler_cfg->sp_id = l1_flow_map_tbl.bf.sp_id;
	
	c_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->c_pri;
	hppe_l1_c_sp_cfg_tbl_get(dev_id, c_sp_id, &l1_c_sp_cfg_tbl);
	scheduler_cfg->c_drr_unit = l1_c_sp_cfg_tbl.bf.drr_credit_unit;
	scheduler_cfg->c_drr_id = l1_c_sp_cfg_tbl.bf.drr_id;

	e_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->e_pri;
	hppe_l1_e_sp_cfg_tbl_get(dev_id, e_sp_id, &l1_e_sp_cfg_tbl);
	scheduler_cfg->e_drr_unit = l1_e_sp_cfg_tbl.bf.drr_credit_unit;
	scheduler_cfg->e_drr_id = l1_e_sp_cfg_tbl.bf.drr_id;

	hppe_l1_flow_port_map_tbl_get(dev_id, node_id, &port_map);
	*port_id = port_map.bf.port_num;

	hppe_l1_comp_cfg_tbl_get(dev_id, node_id, &l1_comp_cfg_tbl);
	scheduler_cfg->drr_frame_mode = l1_comp_cfg_tbl.bf.drr_meter_len;

	return SW_OK;
}
#ifndef IN_QOS_MINI
sw_error_t
adpt_hppe_qos_port_mode_pri_set(a_uint32_t dev_id, fal_port_t port_id,
				fal_qos_mode_t mode, a_uint32_t pri)
{
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);

	if (mode == FAL_QOS_UP_MODE)
		port_qos_ctrl.bf.port_pcp_qos_pri = pri;
	else if (mode == FAL_QOS_DSCP_MODE)
		port_qos_ctrl.bf.port_dscp_qos_pri = pri;
	else if (mode == FAL_QOS_FLOW_MODE)
		port_qos_ctrl.bf.port_flow_qos_pri = pri;
	else
		return SW_NOT_SUPPORTED;
	
	return hppe_port_qos_ctrl_set(dev_id, port_id, &port_qos_ctrl);
}

sw_error_t
adpt_hppe_qos_port_mode_pri_get(a_uint32_t dev_id, fal_port_t port_id,
				fal_qos_mode_t mode, a_uint32_t *pri)
{
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);

	hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);

	if (mode == FAL_QOS_UP_MODE)
		*pri = port_qos_ctrl.bf.port_pcp_qos_pri;
	else if (mode == FAL_QOS_DSCP_MODE)
		*pri = port_qos_ctrl.bf.port_dscp_qos_pri;
	else if (mode == FAL_QOS_FLOW_MODE)
		*pri = port_qos_ctrl.bf.port_flow_qos_pri;
	else
		return SW_NOT_SUPPORTED;
	
	return SW_OK;
}
#endif

static sw_error_t
adpt_hppe_qos_port_pri_set(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_pri_precedence_t *pri)
{
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pri);

	hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);

	port_qos_ctrl.bf.port_pcp_qos_pri = pri->pcp_pri;
	port_qos_ctrl.bf.port_dscp_qos_pri = pri->dscp_pri;
	port_qos_ctrl.bf.port_preheader_qos_pri = pri->preheader_pri;
	port_qos_ctrl.bf.port_flow_qos_pri = pri->flow_pri;
	port_qos_ctrl.bf.port_acl_qos_pri = pri->acl_pri;
	
	return hppe_port_qos_ctrl_set(dev_id, port_id, &port_qos_ctrl);
}

sw_error_t
adpt_ppe_qos_port_pri_set(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_pri_precedence_t *pri)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pri);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_port_pri_set(dev_id, port_id, pri);
#endif
	} else {
		return adpt_hppe_qos_port_pri_set(dev_id, port_id, pri);
	}

	return SW_NOT_SUPPORTED;
}

static sw_error_t
adpt_hppe_qos_port_pri_get(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_pri_precedence_t *pri)
{
	sw_error_t rv = SW_OK;
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pri);

	rv = hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);
	if( rv != SW_OK )
		return rv;

	pri->pcp_pri = port_qos_ctrl.bf.port_pcp_qos_pri;
	pri->dscp_pri = port_qos_ctrl.bf.port_dscp_qos_pri;
	pri->preheader_pri = port_qos_ctrl.bf.port_preheader_qos_pri;
	pri->flow_pri = port_qos_ctrl.bf.port_flow_qos_pri;
	pri->acl_pri = port_qos_ctrl.bf.port_acl_qos_pri;

	return SW_OK;
}

sw_error_t
adpt_ppe_qos_port_pri_get(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_pri_precedence_t *pri)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(pri);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_port_pri_get(dev_id, port_id, pri);
#endif
	} else {
		return adpt_hppe_qos_port_pri_get(dev_id, port_id, pri);
	}

	return SW_NOT_SUPPORTED;
}

#ifndef IN_QOS_MINI
static sw_error_t
adpt_hppe_qos_cosmap_pcp_get(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t pcp, fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union pcp_qos_group_0_u pcp_qos_group_0;
	union pcp_qos_group_1_u pcp_qos_group_1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (pcp >= PCP_QOS_GROUP_0_MAX_ENTRY)
		return SW_BAD_PARAM;

	if (group_id == 0) {
		rv = hppe_pcp_qos_group_0_get(dev_id, pcp, &pcp_qos_group_0);
		if( rv != SW_OK )
			return rv;
		cosmap->internal_pcp = pcp_qos_group_0.bf.qos_info & 7;
		cosmap->internal_dei = (pcp_qos_group_0.bf.qos_info >> 3) & 1;
		cosmap->internal_pri = (pcp_qos_group_0.bf.qos_info >> 4) & 0xf;
		cosmap->internal_dscp = (pcp_qos_group_0.bf.qos_info >> 8) & 0x3f;
		cosmap->internal_dp = (pcp_qos_group_0.bf.qos_info >> 14) & 0x3;
	} else if (group_id == 1) {
		rv = hppe_pcp_qos_group_1_get(dev_id, pcp, &pcp_qos_group_1);
		if( rv != SW_OK )
			return rv;
		cosmap->internal_pcp = pcp_qos_group_1.bf.qos_info & 7;
		cosmap->internal_dei = (pcp_qos_group_1.bf.qos_info >> 3) & 1;
		cosmap->internal_pri = (pcp_qos_group_1.bf.qos_info >> 4) & 0xf;
		cosmap->internal_dscp = (pcp_qos_group_1.bf.qos_info >> 8) & 0x3f;
		cosmap->internal_dp = (pcp_qos_group_1.bf.qos_info >> 14) & 0x3;
	} else
		return SW_BAD_PARAM;

	return SW_OK;
}

sw_error_t
adpt_ppe_qos_cosmap_pcp_get(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t pcp, fal_qos_cosmap_t *cosmap)
{
	a_uint32_t chip_ver = 0, chip_type;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(cosmap);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_cosmap_pcp_get(dev_id, group_id,
				pcp, cosmap);
#endif
	} else {
		return adpt_hppe_qos_cosmap_pcp_get(dev_id, group_id,
				pcp, cosmap);
	}

	return SW_NOT_SUPPORTED;
}
#endif

sw_error_t
adpt_hppe_l0_queue_map_set(a_uint32_t dev_id,
					a_uint32_t node_id,
					fal_port_t port_id,
					fal_qos_scheduler_cfg_t *scheduler_cfg)
{
	union l0_flow_map_tbl_u l0_flow_map_tbl;
	union l0_c_sp_cfg_tbl_u l0_c_sp_cfg_tbl;
	union l0_e_sp_cfg_tbl_u l0_e_sp_cfg_tbl;
	union l0_flow_port_map_tbl_u l0_flow_port_map_tbl;
	union l0_comp_cfg_tbl_u l0_comp_cfg_tbl;
	a_uint32_t c_sp_id, e_sp_id;
	a_uint32_t i, j, k;

	memset(&l0_flow_map_tbl, 0, sizeof(l0_flow_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(scheduler_cfg);
	if (node_id >= L0_FLOW_MAP_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	l0_flow_map_tbl.bf.e_drr_wt= scheduler_cfg->e_drr_wt;
	l0_flow_map_tbl.bf.c_drr_wt = scheduler_cfg->c_drr_wt;
	l0_flow_map_tbl.bf.e_pri = scheduler_cfg->e_pri;
	l0_flow_map_tbl.bf.c_pri = scheduler_cfg->c_pri;
	l0_flow_map_tbl.bf.sp_id = scheduler_cfg->sp_id;
	hppe_l0_flow_map_tbl_set(dev_id, node_id, &l0_flow_map_tbl);

	c_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->c_pri;
	l0_c_sp_cfg_tbl.bf.drr_credit_unit = scheduler_cfg->c_drr_unit;
	l0_c_sp_cfg_tbl.bf.drr_id = scheduler_cfg->c_drr_id;
	hppe_l0_c_sp_cfg_tbl_set(dev_id, c_sp_id, &l0_c_sp_cfg_tbl);

	e_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->e_pri;
	l0_e_sp_cfg_tbl.bf.drr_credit_unit = scheduler_cfg->e_drr_unit;
	l0_e_sp_cfg_tbl.bf.drr_id = scheduler_cfg->e_drr_id;
	hppe_l0_e_sp_cfg_tbl_set(dev_id, e_sp_id, &l0_e_sp_cfg_tbl);

	l0_flow_port_map_tbl.bf.port_num = port_id;
	hppe_l0_flow_port_map_tbl_set(dev_id, node_id, &l0_flow_port_map_tbl);

	hppe_l0_comp_cfg_tbl_get(dev_id, node_id, &l0_comp_cfg_tbl);
	l0_comp_cfg_tbl.bf.drr_meter_len = scheduler_cfg->drr_frame_mode;
	hppe_l0_comp_cfg_tbl_set(dev_id, node_id, &l0_comp_cfg_tbl);

	i = node_id / 32;
	j = node_id % 32;
	port_queue_map[port_id].bmp[i] |= 1 << j;
	for (k = 0; k < 8; k++) {
		if (k != port_id) {
			port_queue_map[k].bmp[i] &= ~(1 << j);
		}
	}

	return SW_OK;
}

sw_error_t
adpt_hppe_l0_queue_map_get(a_uint32_t dev_id,
					a_uint32_t node_id,
					fal_port_t *port_id,
					fal_qos_scheduler_cfg_t *scheduler_cfg)
{
	union l0_flow_map_tbl_u l0_flow_map_tbl;
	union l0_c_sp_cfg_tbl_u l0_c_sp_cfg_tbl;
	union l0_e_sp_cfg_tbl_u l0_e_sp_cfg_tbl;
	union l0_flow_port_map_tbl_u l0_flow_port_map_tbl;
	union l0_comp_cfg_tbl_u l0_comp_cfg_tbl;
	a_uint32_t c_sp_id, e_sp_id;

	memset(&l0_flow_map_tbl, 0, sizeof(l0_flow_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(scheduler_cfg);
	if (node_id >= L0_FLOW_MAP_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	hppe_l0_flow_map_tbl_get(dev_id, node_id, &l0_flow_map_tbl);
	scheduler_cfg->e_drr_wt = l0_flow_map_tbl.bf.e_drr_wt;
	scheduler_cfg->c_drr_wt = l0_flow_map_tbl.bf.c_drr_wt;
	scheduler_cfg->e_pri = l0_flow_map_tbl.bf.e_pri;
	scheduler_cfg->c_pri = l0_flow_map_tbl.bf.c_pri;
	scheduler_cfg->sp_id = l0_flow_map_tbl.bf.sp_id;
	
	c_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->c_pri;
	hppe_l0_c_sp_cfg_tbl_get(dev_id, c_sp_id, &l0_c_sp_cfg_tbl);
	scheduler_cfg->c_drr_unit = l0_c_sp_cfg_tbl.bf.drr_credit_unit;
	scheduler_cfg->c_drr_id = l0_c_sp_cfg_tbl.bf.drr_id;

	e_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->e_pri;
	hppe_l0_e_sp_cfg_tbl_get(dev_id, e_sp_id, &l0_e_sp_cfg_tbl);
	scheduler_cfg->e_drr_unit = l0_e_sp_cfg_tbl.bf.drr_credit_unit;
	scheduler_cfg->e_drr_id = l0_e_sp_cfg_tbl.bf.drr_id;

	hppe_l0_flow_port_map_tbl_get(dev_id, node_id, &l0_flow_port_map_tbl);
	*port_id = l0_flow_port_map_tbl.bf.port_num;

	hppe_l0_comp_cfg_tbl_get(dev_id, node_id, &l0_comp_cfg_tbl);
	scheduler_cfg->drr_frame_mode = l0_comp_cfg_tbl.bf.drr_meter_len;

	return SW_OK;
}

#ifndef IN_QOS_MINI
static sw_error_t
adpt_hppe_qos_cosmap_pcp_set(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t pcp, fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union pcp_qos_group_0_u pcp_qos_group_0;
	union pcp_qos_group_1_u pcp_qos_group_1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (pcp >= PCP_QOS_GROUP_0_MAX_ENTRY)
		return SW_BAD_PARAM;

	if (group_id == 0) {
		pcp_qos_group_0.bf.qos_info = cosmap->internal_pcp | \
								(cosmap->internal_dei << 3) | \
								(cosmap->internal_pri << 4) | \
								(cosmap->internal_dscp << 8) | \
								(cosmap->internal_dp << 14);
		rv = hppe_pcp_qos_group_0_set(dev_id, pcp, &pcp_qos_group_0);
	} else if (group_id == 1) {
		pcp_qos_group_1.bf.qos_info = cosmap->internal_pcp | \
								(cosmap->internal_dei << 3) | \
								(cosmap->internal_pri << 4) | \
								(cosmap->internal_dscp << 8) | \
								(cosmap->internal_dp << 14);
		rv = hppe_pcp_qos_group_1_set(dev_id, pcp, &pcp_qos_group_1);
	} else
		return SW_BAD_PARAM;

	return rv;
}

sw_error_t
adpt_ppe_qos_cosmap_pcp_set(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t pcp, fal_qos_cosmap_t *cosmap)
{
	a_uint32_t chip_ver = 0, chip_type;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(cosmap);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_cosmap_pcp_set(dev_id, group_id,
				pcp, cosmap);
#endif
	} else {
		return adpt_hppe_qos_cosmap_pcp_set(dev_id, group_id,
				pcp, cosmap);
	}

	return SW_NOT_SUPPORTED;;
}

sw_error_t
adpt_hppe_qos_port_remark_get(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_remark_enable_t *remark)
{
	sw_error_t rv = SW_OK;
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(remark);

	rv = hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);

	if( rv != SW_OK )
		return rv;

	remark->pcp_change_en = port_qos_ctrl.bf.port_pcp_change_en;
	remark->dei_chage_en = port_qos_ctrl.bf.port_dei_change_en;
	remark->dscp_change_en = port_qos_ctrl.bf.port_dscp_change_en;

	return SW_OK;
}
#endif

static sw_error_t
adpt_hppe_qos_cosmap_dscp_get(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t dscp, fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union dscp_qos_group_0_u dscp_qos_group_0;
	union dscp_qos_group_1_u dscp_qos_group_1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (dscp >= DSCP_QOS_GROUP_0_MAX_ENTRY)
		return SW_BAD_PARAM;

	if (group_id == 0) {
		rv = hppe_dscp_qos_group_0_get(dev_id, dscp, &dscp_qos_group_0);
		if( rv != SW_OK )
			return rv;
		cosmap->internal_pcp = dscp_qos_group_0.bf.qos_info & 7;
		cosmap->internal_dei = (dscp_qos_group_0.bf.qos_info >> 3) & 1;
		cosmap->internal_pri = (dscp_qos_group_0.bf.qos_info >> 4) & 0xf;
		cosmap->internal_dscp = (dscp_qos_group_0.bf.qos_info >> 8) & 0x3f;
		cosmap->internal_dp = (dscp_qos_group_0.bf.qos_info >> 14) & 0x3;
	} else if (group_id == 1) {
		rv = hppe_dscp_qos_group_1_get(dev_id, dscp, &dscp_qos_group_1);
		if( rv != SW_OK )
			return rv;
		cosmap->internal_pcp = dscp_qos_group_1.bf.qos_info & 7;
		cosmap->internal_dei = (dscp_qos_group_1.bf.qos_info >> 3) & 1;
		cosmap->internal_pri = (dscp_qos_group_1.bf.qos_info >> 4) & 0xf;
		cosmap->internal_dscp = (dscp_qos_group_1.bf.qos_info >> 8) & 0x3f;
		cosmap->internal_dp = (dscp_qos_group_1.bf.qos_info >> 14) & 0x3;
	} else
		return SW_BAD_PARAM;

	return SW_OK;
}

sw_error_t
adpt_ppe_qos_cosmap_dscp_get(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t dscp, fal_qos_cosmap_t *cosmap)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(cosmap);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_cosmap_dscp_get(dev_id, group_id,
				dscp, cosmap);
#endif
	} else {
		return adpt_hppe_qos_cosmap_dscp_get(dev_id, group_id,
				dscp, cosmap);
	}

	return SW_NOT_SUPPORTED;
}

static sw_error_t
adpt_hppe_qos_cosmap_flow_set(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint16_t flow, fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union flow_qos_group_0_u flow_qos_group_0;
	union flow_qos_group_1_u flow_qos_group_1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (flow >= FLOW_QOS_GROUP_0_MAX_ENTRY)
		return SW_BAD_PARAM;

	if (group_id == 0) {
		flow_qos_group_0.bf.qos_info = cosmap->internal_pcp | \
								(cosmap->internal_dei << 3) | \
								(cosmap->internal_pri << 4) | \
								(cosmap->internal_dscp << 8) | \
								(cosmap->internal_dp << 14);
		rv = hppe_flow_qos_group_0_set(dev_id, flow, &flow_qos_group_0);
	} else if (group_id == 1) {
		flow_qos_group_1.bf.qos_info = cosmap->internal_pcp | \
								(cosmap->internal_dei << 3) | \
								(cosmap->internal_pri << 4) | \
								(cosmap->internal_dscp << 8) | \
								(cosmap->internal_dp << 14);
		rv = hppe_flow_qos_group_1_set(dev_id, flow, &flow_qos_group_1);
	} else
		return SW_BAD_PARAM;

	return rv;
}

sw_error_t
adpt_ppe_qos_cosmap_flow_set(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint16_t flow, fal_qos_cosmap_t *cosmap)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(cosmap);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_cosmap_flow_set(dev_id, group_id,
				flow, cosmap);
#endif
	} else {
		return adpt_hppe_qos_cosmap_flow_set(dev_id, group_id,
				flow, cosmap);
	}

	return SW_NOT_SUPPORTED;
}

static sw_error_t
adpt_hppe_qos_port_group_set(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_group_t *group)
{
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(group);

	hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);

	port_qos_ctrl.bf.pcp_qos_group_id = group->pcp_group;
	port_qos_ctrl.bf.dscp_qos_group_id = group->dscp_group;
	port_qos_ctrl.bf.flow_qos_group_id = group->flow_group;
	
	return hppe_port_qos_ctrl_set(dev_id, port_id, &port_qos_ctrl);
}

sw_error_t
adpt_ppe_qos_port_group_set(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_group_t *group)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(group);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_port_group_set(dev_id, port_id, group);
#endif
	} else {
		return adpt_hppe_qos_port_group_set(dev_id, port_id, group);
	}

	return SW_NOT_SUPPORTED;
}

sw_error_t
adpt_hppe_ring_queue_map_set(a_uint32_t dev_id, 
					a_uint32_t ring_id, fal_queue_bmp_t *queue_bmp)
{
	union ring_q_map_tbl_u ring_q_map_tbl;

	memset(&ring_q_map_tbl, 0, sizeof(ring_q_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_bmp);
	if (ring_id >= RING_Q_MAP_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	memcpy(ring_q_map_tbl.val, queue_bmp->bmp, sizeof(ring_q_map_tbl.val));
	return hppe_ring_q_map_tbl_set(dev_id, ring_id, &ring_q_map_tbl);
}

static sw_error_t
adpt_hppe_qos_cosmap_dscp_set(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint8_t dscp, fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union dscp_qos_group_0_u dscp_qos_group_0;
	union dscp_qos_group_1_u dscp_qos_group_1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (dscp >= DSCP_QOS_GROUP_0_MAX_ENTRY)
		return SW_BAD_PARAM;

	if (group_id == 0) {
		dscp_qos_group_0.bf.qos_info = cosmap->internal_pcp | \
								(cosmap->internal_dei << 3) | \
								(cosmap->internal_pri << 4) | \
								(cosmap->internal_dscp << 8) | \
								(cosmap->internal_dp << 14);
		rv = hppe_dscp_qos_group_0_set(dev_id, dscp, &dscp_qos_group_0);
	} else if (group_id == 1) {
		dscp_qos_group_1.bf.qos_info = cosmap->internal_pcp | \
								(cosmap->internal_dei << 3) | \
								(cosmap->internal_pri << 4) | \
								(cosmap->internal_dscp << 8) | \
								(cosmap->internal_dp << 14);
		rv = hppe_dscp_qos_group_1_set(dev_id, dscp, &dscp_qos_group_1);
	} else
		return SW_BAD_PARAM;

	return rv;
}

sw_error_t
adpt_ppe_qos_cosmap_dscp_set(a_uint32_t dev_id, a_uint8_t group_id,
				a_uint8_t dscp, fal_qos_cosmap_t *cosmap)
{
	a_uint32_t chip_ver = 0, chip_type;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(cosmap);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_cosmap_dscp_set(dev_id, group_id,
				dscp, cosmap);
#endif
	} else {
		return adpt_hppe_qos_cosmap_dscp_set(dev_id, group_id,
				dscp, cosmap);
	}

	return SW_NOT_SUPPORTED;
}

#ifndef IN_QOS_MINI
sw_error_t
adpt_hppe_qos_port_remark_set(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_remark_enable_t *remark)
{
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(remark);

	hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);

	port_qos_ctrl.bf.port_pcp_change_en = remark->pcp_change_en;
	port_qos_ctrl.bf.port_dei_change_en = remark->dei_chage_en;
	port_qos_ctrl.bf.port_dscp_change_en = remark->dscp_change_en;
	
	return hppe_port_qos_ctrl_set(dev_id, port_id, &port_qos_ctrl);
}
#endif

sw_error_t
adpt_hppe_l1_flow_map_set(a_uint32_t dev_id,
					a_uint32_t node_id,
					fal_port_t port_id,
					fal_qos_scheduler_cfg_t *scheduler_cfg)
{
	union l1_flow_map_tbl_u l1_flow_map_tbl;
	union l1_c_sp_cfg_tbl_u l1_c_sp_cfg_tbl;
	union l1_e_sp_cfg_tbl_u l1_e_sp_cfg_tbl;
	union l1_flow_port_map_tbl_u l1_flow_port_map_tbl;
	union l1_comp_cfg_tbl_u l1_comp_cfg_tbl;
	a_uint32_t c_sp_id, e_sp_id;

	memset(&l1_flow_map_tbl, 0, sizeof(l1_flow_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(scheduler_cfg);
	if (node_id >= L1_FLOW_MAP_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	l1_flow_map_tbl.bf.e_drr_wt= scheduler_cfg->e_drr_wt;
	l1_flow_map_tbl.bf.c_drr_wt = scheduler_cfg->c_drr_wt;
	l1_flow_map_tbl.bf.e_pri = scheduler_cfg->e_pri;
	l1_flow_map_tbl.bf.c_pri = scheduler_cfg->c_pri;
	l1_flow_map_tbl.bf.sp_id = scheduler_cfg->sp_id;
	hppe_l1_flow_map_tbl_set(dev_id, node_id, &l1_flow_map_tbl);

	c_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->c_pri;
	l1_c_sp_cfg_tbl.bf.drr_credit_unit = scheduler_cfg->c_drr_unit;
	l1_c_sp_cfg_tbl.bf.drr_id = scheduler_cfg->c_drr_id;
	hppe_l1_c_sp_cfg_tbl_set(dev_id, c_sp_id, &l1_c_sp_cfg_tbl);

	e_sp_id = scheduler_cfg->sp_id * 8 + scheduler_cfg->e_pri;
	l1_e_sp_cfg_tbl.bf.drr_credit_unit = scheduler_cfg->e_drr_unit;
	l1_e_sp_cfg_tbl.bf.drr_id = scheduler_cfg->e_drr_id;
	hppe_l1_e_sp_cfg_tbl_set(dev_id, e_sp_id, &l1_e_sp_cfg_tbl);

	l1_flow_port_map_tbl.bf.port_num = port_id;
	hppe_l1_flow_port_map_tbl_set(dev_id, node_id, &l1_flow_port_map_tbl);

	hppe_l1_comp_cfg_tbl_get(dev_id, node_id, &l1_comp_cfg_tbl);
	l1_comp_cfg_tbl.bf.drr_meter_len = scheduler_cfg->drr_frame_mode;
	hppe_l1_comp_cfg_tbl_set(dev_id, node_id, &l1_comp_cfg_tbl);

	return SW_OK;
}

sw_error_t
adpt_hppe_queue_scheduler_set(a_uint32_t dev_id, a_uint32_t node_id,
					fal_queue_scheduler_level_t level,
					fal_port_t port_id,
					fal_qos_scheduler_cfg_t *scheduler_cfg)
{
	if (level == FAL_QUEUE_SCHEDULER_LEVEL0)
		return adpt_hppe_l0_queue_map_set(dev_id, node_id, port_id, scheduler_cfg);
	else if ((level == FAL_QUEUE_SCHEDULER_LEVEL1))
		return adpt_hppe_l1_flow_map_set(dev_id, node_id, port_id, scheduler_cfg);
	else
		return SW_FAIL;
}

sw_error_t
adpt_hppe_queue_scheduler_get(a_uint32_t dev_id, a_uint32_t node_id,
					fal_queue_scheduler_level_t level,
					fal_port_t *port_id,
					fal_qos_scheduler_cfg_t *scheduler_cfg)
{
	if (level == FAL_QUEUE_SCHEDULER_LEVEL0)
		return adpt_hppe_l0_queue_map_get(dev_id, node_id, port_id, scheduler_cfg);
	else if ((level == FAL_QUEUE_SCHEDULER_LEVEL1))
		return adpt_hppe_l1_flow_map_get(dev_id, node_id, port_id, scheduler_cfg);
	else
		return SW_FAIL;
}

static sw_error_t
adpt_hppe_qos_cosmap_flow_get(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint16_t flow, fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union flow_qos_group_0_u flow_qos_group_0;
	union flow_qos_group_1_u flow_qos_group_1;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (flow >= FLOW_QOS_GROUP_0_MAX_ENTRY)
		return SW_BAD_PARAM;

	if (group_id == 0) {
		rv = hppe_flow_qos_group_0_get(dev_id, flow, &flow_qos_group_0);
		if( rv != SW_OK )
			return rv;
		cosmap->internal_pcp = flow_qos_group_0.bf.qos_info & 7;
		cosmap->internal_dei = (flow_qos_group_0.bf.qos_info >> 3) & 1;
		cosmap->internal_pri = (flow_qos_group_0.bf.qos_info >> 4) & 0xf;
		cosmap->internal_dscp = (flow_qos_group_0.bf.qos_info >> 8) & 0x3f;
		cosmap->internal_dp = (flow_qos_group_0.bf.qos_info >> 14) & 0x3;
	} else if (group_id == 1) {
		rv = hppe_flow_qos_group_1_get(dev_id, flow, &flow_qos_group_1);
		if( rv != SW_OK )
			return rv;
		cosmap->internal_pcp = flow_qos_group_1.bf.qos_info & 7;
		cosmap->internal_dei = (flow_qos_group_1.bf.qos_info >> 3) & 1;
		cosmap->internal_pri = (flow_qos_group_1.bf.qos_info >> 4) & 0xf;
		cosmap->internal_dscp = (flow_qos_group_1.bf.qos_info >> 8) & 0x3f;
		cosmap->internal_dp = (flow_qos_group_1.bf.qos_info >> 14) & 0x3;
	} else
		return SW_BAD_PARAM;

	return SW_OK;
}

sw_error_t
adpt_ppe_qos_cosmap_flow_get(a_uint32_t dev_id, a_uint8_t group_id,
					a_uint16_t flow, fal_qos_cosmap_t *cosmap)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_cosmap_flow_get(dev_id, group_id,
				flow, cosmap);
#endif
	} else {
		return adpt_hppe_qos_cosmap_flow_get(dev_id, group_id,
				flow, cosmap);
	}

	return SW_NOT_SUPPORTED;
}

static sw_error_t
adpt_hppe_qos_port_group_get(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_group_t *group)
{
	sw_error_t rv = SW_OK;
	union port_qos_ctrl_u port_qos_ctrl;

	memset(&port_qos_ctrl, 0, sizeof(port_qos_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(group);

	rv = hppe_port_qos_ctrl_get(dev_id, port_id, &port_qos_ctrl);
	if( rv != SW_OK )
		return rv;

	group->pcp_group = port_qos_ctrl.bf.pcp_qos_group_id;
	group->dscp_group = port_qos_ctrl.bf.dscp_qos_group_id;
	group->flow_group = port_qos_ctrl.bf.flow_qos_group_id;

	return SW_OK;
}

sw_error_t
adpt_ppe_qos_port_group_get(a_uint32_t dev_id, fal_port_t port_id,
					fal_qos_group_t *group)
{
	a_uint32_t chip_ver = 0, chip_type = 0;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(group);

	chip_type = adpt_chip_type_get(dev_id);
	chip_ver = adpt_chip_revision_get(dev_id);
	if ((chip_type == CHIP_HPPE && chip_ver == CPPE_REVISION) ||
			chip_type == CHIP_APPE) {
#if defined(CPPE) || defined(APPE)
		return adpt_cppe_qos_port_group_get(dev_id, port_id, group);
#endif
	} else {
		return adpt_hppe_qos_port_group_get(dev_id, port_id, group);
	}

	return SW_NOT_SUPPORTED;
}

sw_error_t
adpt_hppe_ring_queue_map_get(a_uint32_t dev_id, 
					a_uint32_t ring_id, fal_queue_bmp_t *queue_bmp)
{
	union ring_q_map_tbl_u ring_q_map_tbl;

	memset(&ring_q_map_tbl, 0, sizeof(ring_q_map_tbl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_bmp);
	if (ring_id >= RING_Q_MAP_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	hppe_ring_q_map_tbl_get(dev_id, ring_id, &ring_q_map_tbl);
	memcpy(queue_bmp->bmp, ring_q_map_tbl.val, sizeof(ring_q_map_tbl.val));
	return SW_OK;
}

#ifndef IN_QOS_MINI
sw_error_t
adpt_hppe_port_queues_get(a_uint32_t dev_id, 
				fal_port_t port_id, fal_queue_bmp_t *queue_bmp)
{
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(queue_bmp);

	*queue_bmp = port_queue_map[port_id];
	
	return SW_OK;
}
#endif

sw_error_t
adpt_hppe_tdm_tick_num_set(a_uint32_t dev_id, a_uint32_t tick_num)
{
	union tdm_depth_cfg_u tdm_depth_cfg;

	ADPT_DEV_ID_CHECK(dev_id);

	tdm_depth_cfg.bf.tdm_depth = tick_num;
	return hppe_tdm_depth_cfg_set(dev_id, &tdm_depth_cfg);
}
#ifndef IN_QOS_MINI
sw_error_t
adpt_hppe_tdm_tick_num_get(a_uint32_t dev_id, a_uint32_t *tick_num)
{
	union tdm_depth_cfg_u tdm_depth_cfg;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(tick_num);

	hppe_tdm_depth_cfg_get(dev_id, &tdm_depth_cfg);
	*tick_num = tdm_depth_cfg.bf.tdm_depth;

	return SW_OK;
}
#endif
sw_error_t
adpt_hppe_port_scheduler_cfg_reset(a_uint32_t dev_id,
				fal_port_t port_id)
{
	ssdk_dt_scheduler_cfg *dt_cfg;
	fal_qos_scheduler_cfg_t cfg;
	a_uint32_t i;

	dt_cfg = ssdk_bootup_shceduler_cfg_get(dev_id);
	if (!dt_cfg)
		return SW_FAIL;

	/* L1 shceduler */
	for (i = 0; i < SSDK_L1SCHEDULER_CFG_MAX; i++) {
		if (dt_cfg->l1cfg[i].valid && dt_cfg->l1cfg[i].port_id == port_id) {
			cfg.sp_id = dt_cfg->l1cfg[i].port_id;
			cfg.c_pri = dt_cfg->l1cfg[i].cpri;
			cfg.e_pri = dt_cfg->l1cfg[i].epri;
			cfg.c_drr_id = dt_cfg->l1cfg[i].cdrr_id;
			cfg.e_drr_id = dt_cfg->l1cfg[i].edrr_id;
			cfg.c_drr_wt = 1;
			cfg.e_drr_wt = 1;
			adpt_hppe_queue_scheduler_set(dev_id, i, 1,
					dt_cfg->l1cfg[i].port_id, &cfg);
		}
	}

	/* L0 shceduler */
	for (i = 0; i < SSDK_L0SCHEDULER_CFG_MAX; i++) {
		if (dt_cfg->l0cfg[i].valid && dt_cfg->l0cfg[i].port_id == port_id) {
			cfg.sp_id = dt_cfg->l0cfg[i].sp_id;
			cfg.c_pri = dt_cfg->l0cfg[i].cpri;
			cfg.e_pri = dt_cfg->l0cfg[i].epri;
			cfg.c_drr_id = dt_cfg->l0cfg[i].cdrr_id;
			cfg.e_drr_id = dt_cfg->l0cfg[i].edrr_id;
			cfg.c_drr_wt = 1;
			cfg.e_drr_wt = 1;
			adpt_hppe_queue_scheduler_set(dev_id, i,
					0, dt_cfg->l0cfg[i].port_id, &cfg);
		}
	}

	return SW_OK;
}

sw_error_t
adpt_hppe_port_scheduler_cfg_set(a_uint32_t dev_id,
				a_uint32_t tick_index,
				fal_port_scheduler_cfg_t *cfg)
{
	union psch_tdm_cfg_tbl_u psch_tdm_cfg;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	psch_tdm_cfg.bf.ens_port_bitmap = cfg->en_scheduler_port_bmp;
	psch_tdm_cfg.bf.ens_port = cfg->en_scheduler_port;
	psch_tdm_cfg.bf.des_port = cfg->de_scheduler_port;
#if defined(APPE)
	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		psch_tdm_cfg.bf.des_second_port_en = cfg->de_scheduler_2nd_port_en;
		psch_tdm_cfg.bf.des_second_port = cfg->de_scheduler_2nd_port;
	}
#endif

	return hppe_psch_tdm_cfg_tbl_set(dev_id, tick_index, &psch_tdm_cfg);
}
#ifndef IN_QOS_MINI
sw_error_t
adpt_hppe_port_scheduler_cfg_get(a_uint32_t dev_id,
				a_uint32_t tick_index,
				fal_port_scheduler_cfg_t *cfg)
{
	union psch_tdm_cfg_tbl_u psch_tdm_cfg;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	hppe_psch_tdm_cfg_tbl_get(dev_id, tick_index, &psch_tdm_cfg);
	cfg->en_scheduler_port_bmp = psch_tdm_cfg.bf.ens_port_bitmap;
	cfg->en_scheduler_port = psch_tdm_cfg.bf.ens_port;
	cfg->de_scheduler_port = psch_tdm_cfg.bf.des_port;
#if defined(APPE)
	if (adpt_chip_type_get(dev_id) == CHIP_APPE) {
		cfg->de_scheduler_2nd_port_en = psch_tdm_cfg.bf.des_second_port_en;
		cfg->de_scheduler_2nd_port = psch_tdm_cfg.bf.des_second_port;
	}
#endif

	return SW_OK;
}
#endif
sw_error_t
adpt_hppe_scheduler_dequeue_ctrl_get(a_uint32_t dev_id,
				a_uint32_t queue_id,
				a_bool_t *enable)
{
	union deq_dis_tbl_u deq;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(enable);

	hppe_deq_dis_tbl_get(dev_id, queue_id, &deq);
	*enable = !(deq.bf.deq_dis);

	return SW_OK;
}

sw_error_t
adpt_hppe_scheduler_dequeue_ctrl_set(a_uint32_t dev_id,
				a_uint32_t queue_id,
				a_bool_t enable)
{
	union deq_dis_tbl_u deq;

	ADPT_DEV_ID_CHECK(dev_id);

	deq.bf.deq_dis = !enable;
	return hppe_deq_dis_tbl_set(dev_id, queue_id, &deq);
}

sw_error_t
adpt_hppe_port_scheduler_resource_get(a_uint32_t dev_id,
				fal_port_t port_id,
				fal_portscheduler_resource_t *cfg)
{
	ssdk_dt_scheduler_cfg *dt_cfg;
	ssdk_dt_portscheduler_cfg *port_resource;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	if (port_id >= SSDK_MAX_PORT_NUM)
		return SW_BAD_PARAM;

	dt_cfg = ssdk_bootup_shceduler_cfg_get(dev_id);
	if (!dt_cfg)
		return SW_NOT_SUPPORTED;

	port_resource = &dt_cfg->pool[port_id];
	cfg->ucastq_start = port_resource->ucastq_start;
	cfg->ucastq_num = port_resource->ucastq_end - port_resource->ucastq_start + 1;
	cfg->mcastq_start = port_resource->mcastq_start;
	cfg->mcastq_num = port_resource->mcastq_end - port_resource->mcastq_start + 1;
	cfg->l0sp_start = port_resource->l0sp_start;
	cfg->l0sp_num = port_resource->l0sp_end - port_resource->l0sp_start + 1;
	cfg->l0cdrr_start = port_resource->l0cdrr_start;
	cfg->l0cdrr_num = port_resource->l0cdrr_end - port_resource->l0cdrr_start + 1;
	cfg->l0edrr_start = port_resource->l0edrr_start;
	cfg->l0edrr_num = port_resource->l0edrr_end - port_resource->l0edrr_start + 1;
	cfg->l1sp_start = port_id;
	cfg->l1sp_num = 1;
	cfg->l1cdrr_start = port_resource->l1cdrr_start;
	cfg->l1cdrr_num = port_resource->l1cdrr_end - port_resource->l1cdrr_start + 1;
	cfg->l1edrr_start = port_resource->l1edrr_start;
	cfg->l1edrr_num = port_resource->l1edrr_end - port_resource->l1edrr_start + 1;

	return SW_OK;
}

#ifndef IN_QOS_MINI
sw_error_t
adpt_ppe_reservedpool_scheduler_resource_get(a_uint32_t dev_id,
				fal_portscheduler_resource_t *cfg)
{
	ssdk_dt_scheduler_cfg *dt_cfg;
	ssdk_dt_portscheduler_cfg *reserved_resource;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cfg);

	dt_cfg = ssdk_bootup_shceduler_cfg_get(dev_id);
	if (!dt_cfg)
		return SW_NOT_SUPPORTED;

	reserved_resource = &dt_cfg->reserved_pool;
	cfg->ucastq_start = reserved_resource->ucastq_start;
	cfg->ucastq_num = reserved_resource->ucastq_end - reserved_resource->ucastq_start + 1;
	cfg->mcastq_start = reserved_resource->mcastq_start;
	cfg->mcastq_num = reserved_resource->mcastq_end - reserved_resource->mcastq_start + 1;
	cfg->l0sp_start = reserved_resource->l0sp_start;
	cfg->l0sp_num = reserved_resource->l0sp_end - reserved_resource->l0sp_start + 1;
	cfg->l0cdrr_start = reserved_resource->l0cdrr_start;
	cfg->l0cdrr_num = reserved_resource->l0cdrr_end - reserved_resource->l0cdrr_start + 1;
	cfg->l0edrr_start = reserved_resource->l0edrr_start;
	cfg->l0edrr_num = reserved_resource->l0edrr_end - reserved_resource->l0edrr_start + 1;
	cfg->l1cdrr_start = reserved_resource->l1cdrr_start;
	cfg->l1cdrr_num = reserved_resource->l1cdrr_end - reserved_resource->l1cdrr_start + 1;
	cfg->l1edrr_start = reserved_resource->l1edrr_start;
	cfg->l1edrr_num = reserved_resource->l1edrr_end - reserved_resource->l1edrr_start + 1;

	return SW_OK;
}
#endif

sw_error_t adpt_hppe_qos_init(a_uint32_t dev_id)
{
	adpt_api_t *p_adpt_api = NULL;

	p_adpt_api = adpt_api_ptr_get(dev_id);

	if(p_adpt_api == NULL)
		return SW_FAIL;

	p_adpt_api->adpt_qos_port_pri_set = adpt_ppe_qos_port_pri_set;
	p_adpt_api->adpt_qos_port_pri_get = adpt_ppe_qos_port_pri_get;
#ifndef IN_QOS_MINI

	p_adpt_api->adpt_qos_cosmap_pcp_get = adpt_ppe_qos_cosmap_pcp_get;
#endif
	p_adpt_api->adpt_queue_scheduler_set = adpt_hppe_queue_scheduler_set;
	p_adpt_api->adpt_queue_scheduler_get = adpt_hppe_queue_scheduler_get;
#ifndef IN_QOS_MINI
	p_adpt_api->adpt_port_queues_get = adpt_hppe_port_queues_get;
	p_adpt_api->adpt_qos_cosmap_pcp_set = adpt_ppe_qos_cosmap_pcp_set;
	if (adpt_chip_type_get(dev_id) == CHIP_HPPE &&
			adpt_chip_revision_get(dev_id) == HPPE_REVISION) {
		p_adpt_api->adpt_qos_port_remark_get = adpt_hppe_qos_port_remark_get;
	}
#endif
	p_adpt_api->adpt_qos_cosmap_dscp_get = adpt_ppe_qos_cosmap_dscp_get;
	p_adpt_api->adpt_qos_cosmap_flow_set = adpt_ppe_qos_cosmap_flow_set;
	p_adpt_api->adpt_qos_port_group_set = adpt_ppe_qos_port_group_set;
	p_adpt_api->adpt_ring_queue_map_set = adpt_hppe_ring_queue_map_set;
	p_adpt_api->adpt_qos_cosmap_dscp_set = adpt_ppe_qos_cosmap_dscp_set;
#ifndef IN_QOS_MINI
	if (adpt_chip_type_get(dev_id) == CHIP_HPPE &&
			adpt_chip_revision_get(dev_id) == HPPE_REVISION) {
		p_adpt_api->adpt_qos_port_remark_set = adpt_hppe_qos_port_remark_set;
	}
#endif
	p_adpt_api->adpt_qos_cosmap_flow_get = adpt_ppe_qos_cosmap_flow_get;
	p_adpt_api->adpt_qos_port_group_get = adpt_ppe_qos_port_group_get;
	p_adpt_api->adpt_ring_queue_map_get = adpt_hppe_ring_queue_map_get;
	p_adpt_api->adpt_tdm_tick_num_set = adpt_hppe_tdm_tick_num_set;
#ifndef IN_QOS_MINI
	p_adpt_api->adpt_tdm_tick_num_get = adpt_hppe_tdm_tick_num_get;
#endif
	p_adpt_api->adpt_port_scheduler_cfg_set = adpt_hppe_port_scheduler_cfg_set;
#ifndef IN_QOS_MINI
	p_adpt_api->adpt_port_scheduler_cfg_get = adpt_hppe_port_scheduler_cfg_get;
#endif
	p_adpt_api->adpt_scheduler_dequeue_ctrl_get = adpt_hppe_scheduler_dequeue_ctrl_get;
	p_adpt_api->adpt_scheduler_dequeue_ctrl_set = adpt_hppe_scheduler_dequeue_ctrl_set;
#ifndef IN_QOS_MINI
	if (adpt_chip_type_get(dev_id) == CHIP_HPPE &&
			adpt_chip_revision_get(dev_id) == HPPE_REVISION) {
		p_adpt_api->adpt_qos_port_mode_pri_get = adpt_hppe_qos_port_mode_pri_get;
		p_adpt_api->adpt_qos_port_mode_pri_set = adpt_hppe_qos_port_mode_pri_set;
	}
#endif
	p_adpt_api->adpt_port_scheduler_cfg_reset = adpt_hppe_port_scheduler_cfg_reset;
	p_adpt_api->adpt_port_scheduler_resource_get =
		adpt_hppe_port_scheduler_resource_get;
#ifndef IN_QOS_MINI
	p_adpt_api->adpt_reservedpool_scheduler_resource_get =
		adpt_ppe_reservedpool_scheduler_resource_get;
#endif

	return SW_OK;
}

/**
 * @}
 */
