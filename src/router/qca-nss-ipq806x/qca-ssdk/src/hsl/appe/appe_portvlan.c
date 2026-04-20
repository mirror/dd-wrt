/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
#include "hsl.h"
#include "hppe_reg_access.h"
#include "appe_portvlan_reg.h"
#include "appe_portvlan.h"

sw_error_t
appe_ipr_vp_parsing_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_vp_parsing_u *value)
{
	if (index >= IPR_VP_PARSING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				IPR_CSR_BASE_ADDR + IPR_VP_PARSING_ADDRESS + \
				index * IPR_VP_PARSING_INC,
				&value->val);
}

sw_error_t
appe_ipr_vp_parsing_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union ipr_vp_parsing_u *value)
{
	return hppe_reg_set(
				dev_id,
				IPR_CSR_BASE_ADDR + IPR_VP_PARSING_ADDRESS + \
				index * IPR_VP_PARSING_INC,
				value->val);
}

sw_error_t
appe_vlan_port_vp_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vlan_port_vp_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				INGRESS_VLAN_BASE_ADDR + VLAN_PORT_VP_TBL_ADDRESS + \
				index * VLAN_PORT_VP_TBL_INC,
				value->val,
				2);
}

sw_error_t
appe_vlan_port_vp_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vlan_port_vp_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				INGRESS_VLAN_BASE_ADDR + VLAN_PORT_VP_TBL_ADDRESS + \
				index * VLAN_PORT_VP_TBL_INC,
				value->val,
				2);
}

sw_error_t
appe_egress_vp_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_vp_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_VP_TBL_ADDRESS + \
				index * EG_VP_TBL_INC,
				value->val,
				sizeof(union eg_vp_tbl_u)/sizeof(a_uint32_t));
}

sw_error_t
appe_egress_vp_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_vp_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_VP_TBL_ADDRESS + \
				index * EG_VP_TBL_INC,
				value->val,
				sizeof(union eg_vp_tbl_u)/sizeof(a_uint32_t));
}

sw_error_t
appe_eg_vsi_vp_tag_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_vsi_vp_tag_u *value)
{
	if (index >= EG_VSI_VP_TAG_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_VSI_VP_TAG_ADDRESS + \
				index * EG_VSI_VP_TAG_INC,
				&value->val);
}

sw_error_t
appe_eg_vsi_vp_tag_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_vsi_vp_tag_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_VSI_VP_TAG_ADDRESS + \
				index * EG_VSI_VP_TAG_INC,
				value->val);
}

sw_error_t
appe_tpr_port_parsing_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_port_parsing_u *value)
{
	if (index >= TPR_PORT_PARSING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PORT_PARSING_ADDRESS + \
				index * TPR_PORT_PARSING_INC,
				&value->val);
}

sw_error_t
appe_tpr_port_parsing_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tpr_port_parsing_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_PORT_PARSING_ADDRESS + \
				index * TPR_PORT_PARSING_INC,
				value->val);
}

sw_error_t
appe_tpr_vlan_tpid_get(
		a_uint32_t dev_id,
		union tpr_vlan_tpid_u *value)
{
	return hppe_reg_get(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_VLAN_TPID_ADDRESS,
				&value->val);
}

sw_error_t
appe_tpr_vlan_tpid_set(
		a_uint32_t dev_id,
		union tpr_vlan_tpid_u *value)
{
	return hppe_reg_set(
				dev_id,
				TUNNEL_PARSER_BASE_ADDR + TPR_VLAN_TPID_ADDRESS,
				value->val);
}

sw_error_t
appe_vlan_port_vp_tbl_port_vlan_xlt_miss_fwd_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_vlan_xlt_miss_fwd_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_vlan_xlt_miss_fwd_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_vlan_xlt_miss_fwd_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_vsi_tag_mode_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vsi_tag_mode_en;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_vsi_tag_mode_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vsi_tag_mode_en = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_vlan_stag_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_eg_vlan_stag_mode;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_vlan_stag_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_eg_vlan_stag_mode = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_vlan_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_vlan_type;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_vlan_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_vlan_type = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_vlan_ctag_mode_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_eg_vlan_ctag_mode;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_vlan_ctag_mode_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_eg_vlan_ctag_mode = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vsi_vp_tag_tagged_mode_vp_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vsi_vp_tag_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_eg_vsi_vp_tag_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tagged_mode_vp_bitmap;
	return ret;
}

sw_error_t
appe_eg_vsi_vp_tag_tagged_mode_vp_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vsi_vp_tag_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_eg_vsi_vp_tag_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tagged_mode_vp_bitmap = value;
	ret = appe_eg_vsi_vp_tag_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_port_parsing_port_role_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union tpr_port_parsing_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_port_parsing_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_role;
	return ret;
}

sw_error_t
appe_tpr_port_parsing_port_role_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union tpr_port_parsing_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_port_parsing_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_role = value;
	ret = appe_tpr_port_parsing_set(dev_id, index, &reg_val);
	return ret;
}

#ifndef IN_PORTVLAN_MINI
sw_error_t
appe_vp_isol_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vp_isol_tbl_u *value)
{
	if (index >= VP_ISOL_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_get(
				dev_id,
				IPE_L2_BASE_ADDR + VP_ISOL_TBL_ADDRESS + \
				index * VP_ISOL_TBL_INC,
				value->val,
				2);
}

sw_error_t
appe_vp_isol_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vp_isol_tbl_u *value)
{
	if (index >= VP_ISOL_TBL_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_set(
				dev_id,
				IPE_L2_BASE_ADDR + VP_ISOL_TBL_ADDRESS + \
				index * VP_ISOL_TBL_INC,
				value->val,
				2);
}


sw_error_t
appe_vport_parsing_port_role_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union ipr_vp_parsing_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_ipr_vp_parsing_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_role;
	return ret;
}

sw_error_t
appe_vport_parsing_port_role_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union ipr_vp_parsing_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_ipr_vp_parsing_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_role = value;
	ret = appe_ipr_vp_parsing_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_cvlan_untag_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_cvlan_untag_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_cvlan_untag_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_cvlan_untag_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_in_pcp_prop_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_in_pcp_prop_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_in_pcp_prop_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_in_pcp_prop_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_sdei_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_sdei;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_sdei_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_sdei = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cvid_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_cvid_en;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cvid_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_cvid_en = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cdei_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_cdei;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cdei_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_cdei = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_svid_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_svid_en;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_svid_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_svid_en = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_spcp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_spcp;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_spcp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_spcp = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_untag_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_untag_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_untag_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_untag_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_in_dei_prop_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_in_dei_prop_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_in_dei_prop_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_in_dei_prop_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_svid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_svid;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_svid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_svid = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cvid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_cvid;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cvid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_cvid = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_pri_tag_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_pri_tag_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_pri_tag_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_pri_tag_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_in_vlan_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_in_vlan_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_in_vlan_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_in_vlan_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_tag_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_tag_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_tag_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_tag_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cpcp_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_cpcp_1 << 2 | \
		reg_val.bf.port_def_cpcp_0;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_def_cpcp_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_cpcp_1 = value >> 2;
	reg_val.bf.port_def_cpcp_0 = value & (((a_uint64_t)1<<2)-1);
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_cvlan_pri_tag_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_cvlan_pri_tag_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_cvlan_pri_tag_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_cvlan_pri_tag_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_vlan_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vlan_profile;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_vlan_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vlan_profile = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_cvlan_tag_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_cvlan_tag_fltr_cmd;
	return ret;
}

sw_error_t
appe_vlan_port_vp_tbl_port_cvlan_tag_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union vlan_port_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vlan_port_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_cvlan_tag_fltr_cmd = value;
	ret = appe_vlan_port_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_xlat_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.xlat_profile;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_xlat_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.xlat_profile = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_tx_counting_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tx_counting_en;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_tx_counting_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tx_counting_en = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_pcp_prop_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_eg_pcp_prop_cmd;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_pcp_prop_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_eg_pcp_prop_cmd = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_tunnel_id_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tunnel_id;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_tunnel_id_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tunnel_id = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_dei_prop_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_eg_dei_prop_cmd;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_eg_dei_prop_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_eg_dei_prop_cmd = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_cvid_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_cvid_en;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_cvid_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_cvid_en = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_svid_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_svid_en;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_svid_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_svid_en = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_tunnel_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.tunnel_valid;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_tunnel_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.tunnel_valid = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_svid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_svid;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_svid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_svid = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_cvid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_def_cvid;
	return ret;
}

sw_error_t
appe_eg_vp_tbl_port_def_cvid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_def_cvid = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

#if 0
#if defined(MPPE)
sw_error_t
mppe_eg_vp_tbl_ath_hdr_insert_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ath_hdr_insert;
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_insert_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ath_hdr_insert = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_ver_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ath_hdr_ver;
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_ver_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ath_hdr_ver = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_default_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ath_hdr_default_type;
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_default_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ath_hdr_default_type = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_port_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ath_port_bitmap_1 << 6 | \
		reg_val.bf.ath_port_bitmap_0;
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_port_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ath_port_bitmap_1 = value >> 6;
	reg_val.bf.ath_port_bitmap_0 = value & (((a_uint64_t)1<<6)-1);
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_disable_bit_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ath_hdr_disable_bit;
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_disable_bit_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ath_hdr_disable_bit = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_from_cpu_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.ath_hdr_from_cpu;
	return ret;
}

sw_error_t
mppe_eg_vp_tbl_ath_hdr_from_cpu_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union eg_vp_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_egress_vp_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ath_hdr_from_cpu = value;
	ret = appe_egress_vp_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
#endif

sw_error_t
appe_tpr_vlan_tpid_stag_tpid_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_vlan_tpid_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_vlan_tpid_get(dev_id, &reg_val);
	*value = reg_val.bf.stag_tpid;
	return ret;
}

sw_error_t
appe_tpr_vlan_tpid_stag_tpid_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_vlan_tpid_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_vlan_tpid_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.stag_tpid = value;
	ret = appe_tpr_vlan_tpid_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_tpr_vlan_tpid_ctag_tpid_get(
		a_uint32_t dev_id,
		a_uint32_t *value)
{
	union tpr_vlan_tpid_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_vlan_tpid_get(dev_id, &reg_val);
	*value = reg_val.bf.ctag_tpid;
	return ret;
}

sw_error_t
appe_tpr_vlan_tpid_ctag_tpid_set(
		a_uint32_t dev_id,
		a_uint32_t value)
{
	union tpr_vlan_tpid_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_tpr_vlan_tpid_get(dev_id, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.ctag_tpid = value;
	ret = appe_tpr_vlan_tpid_set(dev_id, &reg_val);
	return ret;
}

sw_error_t
appe_vp_isol_tbl_vp_profile_map_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union vp_isol_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vp_isol_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	*value = (a_uint64_t)reg_val.bf.vp_profile_map_1 << 32 | \
		reg_val.bf.vp_profile_map_0;
	return ret;
}

sw_error_t
appe_vp_isol_tbl_vp_profile_map_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union vp_isol_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vp_isol_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vp_profile_map_1 = value >> 32;
	reg_val.bf.vp_profile_map_0 = value & (((a_uint64_t)1<<32)-1);
	ret = appe_vp_isol_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
