/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */


/**
 * @defgroup
 * @{
 */
#include "sw.h"
#include "fal_qos.h"
#include "hppe_portctrl_reg.h"
#include "hppe_portctrl.h"
#include "cppe_qos_reg.h"
#include "cppe_qos.h"
#include "adpt.h"

static sw_error_t
adpt_cppe_qos_mapping_get(a_uint32_t dev_id, a_uint32_t index,
			fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union qos_mapping_tbl_u qos_mapping_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	rv = cppe_qos_mapping_tbl_get(dev_id, index, &qos_mapping_tbl);
	if (rv != SW_OK)
		return rv;

	cosmap->internal_pcp = qos_mapping_tbl.bf.int_pcp;
	cosmap->internal_dei = qos_mapping_tbl.bf.int_dei;
	cosmap->internal_pri = qos_mapping_tbl.bf.int_pri;
	cosmap->internal_dscp = qos_mapping_tbl.bf.int_dscp_tc;
	cosmap->internal_dp = qos_mapping_tbl.bf.int_dp;
	cosmap->dscp_mask = qos_mapping_tbl.bf.dscp_tc_mask;
	cosmap->dscp_en = qos_mapping_tbl.bf.int_dscp_en;
	cosmap->pcp_en = qos_mapping_tbl.bf.int_pcp_en;
	cosmap->dei_en = qos_mapping_tbl.bf.int_dei_en;
	cosmap->pri_en = qos_mapping_tbl.bf.int_pri_en;
	cosmap->dp_en = qos_mapping_tbl.bf.int_dp_en;
	cosmap->qos_prec = qos_mapping_tbl.bf.qos_res_prec_0 |
			     qos_mapping_tbl.bf.qos_res_prec_1 << 1;

	return SW_OK;
}

static sw_error_t
adpt_cppe_qos_mapping_set(a_uint32_t dev_id, a_uint32_t index,
			fal_qos_cosmap_t *cosmap)
{
	sw_error_t rv = SW_OK;
	union qos_mapping_tbl_u qos_mapping_tbl;

	ADPT_DEV_ID_CHECK(dev_id);
        ADPT_NULL_POINT_CHECK(cosmap);

	memset(&qos_mapping_tbl, 0, sizeof(qos_mapping_tbl));
	rv = cppe_qos_mapping_tbl_get(dev_id, index, &qos_mapping_tbl);
	if (rv != SW_OK)
		return rv;

	qos_mapping_tbl.bf.int_pcp = cosmap->internal_pcp;
	qos_mapping_tbl.bf.int_dei = cosmap->internal_dei;
	qos_mapping_tbl.bf.int_pri = cosmap->internal_pri;
	qos_mapping_tbl.bf.int_dscp_tc = cosmap->internal_dscp;
	qos_mapping_tbl.bf.int_dp = cosmap->internal_dp;
	qos_mapping_tbl.bf.dscp_tc_mask = cosmap->dscp_mask;
	qos_mapping_tbl.bf.int_dscp_en = cosmap->dscp_en;
	qos_mapping_tbl.bf.int_pcp_en = cosmap->pcp_en;
	qos_mapping_tbl.bf.int_dei_en = cosmap->dei_en;
	qos_mapping_tbl.bf.int_pri_en = cosmap->pri_en;
	qos_mapping_tbl.bf.int_dp_en = cosmap->dp_en;
	qos_mapping_tbl.bf.qos_res_prec_0 = cosmap->qos_prec & 1;
	qos_mapping_tbl.bf.qos_res_prec_1 = (cosmap->qos_prec >> 1) & 3;

	return cppe_qos_mapping_tbl_set(dev_id, index, &qos_mapping_tbl);
}

sw_error_t
adpt_cppe_qos_port_pri_set(a_uint32_t dev_id, fal_port_t port_id,
			fal_qos_pri_precedence_t *pri)
{
	union mru_mtu_ctrl_tbl_u mru_mtu_ctrl;

	memset(&mru_mtu_ctrl, 0, sizeof(mru_mtu_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pri);

	hppe_mru_mtu_ctrl_tbl_get(dev_id, port_id, &mru_mtu_ctrl);

	mru_mtu_ctrl.bf.pcp_res_prec = pri->pcp_pri;
	mru_mtu_ctrl.bf.dscp_res_prec = pri->dscp_pri;
	mru_mtu_ctrl.bf.preheader_res_prec = pri->preheader_pri;
	mru_mtu_ctrl.bf.flow_res_prec = pri->flow_pri;
	mru_mtu_ctrl.bf.pre_acl_res_prec = pri->acl_pri;
	mru_mtu_ctrl.bf.post_acl_res_prec = pri->post_acl_pri;
	mru_mtu_ctrl.bf.pcp_res_prec_force = pri->pcp_pri_force;
	mru_mtu_ctrl.bf.dscp_res_prec_force = pri->dscp_pri_force;
#if defined(APPE)
	mru_mtu_ctrl.bf.pre_ipo_outer_res_prec = pri->pre_acl_outer_pri;
	mru_mtu_ctrl.bf.pre_ipo_inner_res_prec_1 = pri->pre_acl_inner_pri >>
		SW_FIELD_OFFSET_IN_WORD(MRU_MTU_CTRL_TBL_PRE_IPO_INNER_RES_PREC_OFFSET);
	mru_mtu_ctrl.bf.pre_ipo_inner_res_prec_0 = pri->pre_acl_inner_pri;
#endif

	return hppe_mru_mtu_ctrl_tbl_set(dev_id, port_id, &mru_mtu_ctrl);
}

sw_error_t
adpt_cppe_qos_port_pri_get(a_uint32_t dev_id, fal_port_t port_id,
			fal_qos_pri_precedence_t *pri)
{
	sw_error_t rv = SW_OK;
	union mru_mtu_ctrl_tbl_u mru_mtu_ctrl;

	memset(&mru_mtu_ctrl, 0, sizeof(mru_mtu_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(pri);

	rv = hppe_mru_mtu_ctrl_tbl_get(dev_id, port_id, &mru_mtu_ctrl);
	if( rv != SW_OK )
		return rv;

	pri->pcp_pri = mru_mtu_ctrl.bf.pcp_res_prec;
	pri->dscp_pri = mru_mtu_ctrl.bf.dscp_res_prec;
	pri->preheader_pri = mru_mtu_ctrl.bf.preheader_res_prec;
	pri->flow_pri = mru_mtu_ctrl.bf.flow_res_prec;
	pri->acl_pri = mru_mtu_ctrl.bf.pre_acl_res_prec;
	pri->post_acl_pri = mru_mtu_ctrl.bf.post_acl_res_prec;
	pri->pcp_pri_force = mru_mtu_ctrl.bf.pcp_res_prec_force;
	pri->dscp_pri_force = mru_mtu_ctrl.bf.dscp_res_prec_force;
#if defined(APPE)
	pri->pre_acl_outer_pri = mru_mtu_ctrl.bf.pre_ipo_outer_res_prec;
	pri->pre_acl_inner_pri = mru_mtu_ctrl.bf.pre_ipo_inner_res_prec_1 <<
		SW_FIELD_OFFSET_IN_WORD(MRU_MTU_CTRL_TBL_PRE_IPO_INNER_RES_PREC_OFFSET) |
		mru_mtu_ctrl.bf.pre_ipo_inner_res_prec_0;
#endif

	return SW_OK;
}

#ifndef IN_QOS_MINI
sw_error_t
adpt_cppe_qos_cosmap_pcp_get(a_uint32_t dev_id, a_uint8_t group_id,
			a_uint8_t pcp,
			fal_qos_cosmap_t *cosmap)
{
	a_uint32_t index = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (group_id >= QOS_MAPPING_TBL_MAX_GROUP)
		return SW_BAD_PARAM;

	index = QOS_MAPPING_FLOW_TBL_MAX_ENTRY +
		2 * QOS_MAPPING_DSCP_TBL_MAX_ENTRY +
		group_id * QOS_MAPPING_PCP_TBL_MAX_ENTRY + pcp;

	return adpt_cppe_qos_mapping_get(dev_id, index, cosmap);
}

sw_error_t
adpt_cppe_qos_cosmap_pcp_set(a_uint32_t dev_id, a_uint8_t group_id,
			a_uint8_t pcp,
			fal_qos_cosmap_t *cosmap)
{
	a_uint32_t index = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (group_id >= QOS_MAPPING_TBL_MAX_GROUP)
		return SW_BAD_PARAM;

	index = QOS_MAPPING_FLOW_TBL_MAX_ENTRY +
		2 * QOS_MAPPING_DSCP_TBL_MAX_ENTRY +
		group_id * QOS_MAPPING_PCP_TBL_MAX_ENTRY + pcp;

	return adpt_cppe_qos_mapping_set(dev_id, index, cosmap);
}
#endif

sw_error_t
adpt_cppe_qos_cosmap_dscp_get(a_uint32_t dev_id, a_uint8_t group_id,
			a_uint8_t dscp,
			fal_qos_cosmap_t *cosmap)
{
	a_uint32_t index = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (group_id >= QOS_MAPPING_TBL_MAX_GROUP)
		return SW_BAD_PARAM;

	index = QOS_MAPPING_FLOW_TBL_MAX_ENTRY +
		group_id * QOS_MAPPING_DSCP_TBL_MAX_ENTRY +
		dscp;

	return adpt_cppe_qos_mapping_get(dev_id, index, cosmap);
}

sw_error_t
adpt_cppe_qos_cosmap_flow_set(a_uint32_t dev_id, a_uint8_t group_id,
			a_uint16_t flow,
			fal_qos_cosmap_t *cosmap)
{
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (flow >= QOS_MAPPING_FLOW_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	return adpt_cppe_qos_mapping_set(dev_id, flow, cosmap);
}

sw_error_t
adpt_cppe_qos_port_group_set(a_uint32_t dev_id, fal_port_t port_id,
			fal_qos_group_t *group)
{
	union mru_mtu_ctrl_tbl_u mru_mtu_ctrl;

	memset(&mru_mtu_ctrl, 0, sizeof(mru_mtu_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(group);

	hppe_mru_mtu_ctrl_tbl_get(dev_id, port_id, &mru_mtu_ctrl);

	mru_mtu_ctrl.bf.pcp_qos_group_id = group->pcp_group;
	mru_mtu_ctrl.bf.dscp_qos_group_id = group->dscp_group;

	return hppe_mru_mtu_ctrl_tbl_set(dev_id, port_id, &mru_mtu_ctrl);
}

sw_error_t
adpt_cppe_qos_cosmap_dscp_set(a_uint32_t dev_id, a_uint8_t group_id,
			a_uint8_t dscp,
			fal_qos_cosmap_t *cosmap)
{
	a_uint32_t index = 0;

	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (group_id >= QOS_MAPPING_TBL_MAX_GROUP)
		return SW_BAD_PARAM;

	index = QOS_MAPPING_FLOW_TBL_MAX_ENTRY +
		group_id * QOS_MAPPING_DSCP_TBL_MAX_ENTRY +
		dscp;

	return adpt_cppe_qos_mapping_set(dev_id, index, cosmap);
}

sw_error_t
adpt_cppe_qos_cosmap_flow_get(a_uint32_t dev_id, a_uint8_t group_id,
			a_uint16_t flow,
			fal_qos_cosmap_t *cosmap)
{
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(cosmap);

	if (flow >= QOS_MAPPING_FLOW_TBL_MAX_ENTRY)
		return SW_BAD_PARAM;

	return adpt_cppe_qos_mapping_get(dev_id, flow, cosmap);
}

sw_error_t
adpt_cppe_qos_port_group_get(a_uint32_t dev_id, fal_port_t port_id,
			fal_qos_group_t *group)
{
	sw_error_t rv = SW_OK;
	union mru_mtu_ctrl_tbl_u mru_mtu_ctrl;

	memset(&mru_mtu_ctrl, 0, sizeof(mru_mtu_ctrl));
	ADPT_DEV_ID_CHECK(dev_id);
	ADPT_NULL_POINT_CHECK(group);

	rv = hppe_mru_mtu_ctrl_tbl_get(dev_id, port_id, &mru_mtu_ctrl);
	if( rv != SW_OK )
		return rv;

	group->pcp_group = mru_mtu_ctrl.bf.pcp_qos_group_id;
	group->dscp_group = mru_mtu_ctrl.bf.dscp_qos_group_id;

	return SW_OK;
}

/**
 * @}
 */

