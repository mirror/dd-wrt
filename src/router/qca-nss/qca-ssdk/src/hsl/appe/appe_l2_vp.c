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
#include "hsl.h"
#include "hppe_reg_access.h"
#include "appe_l2_vp_reg.h"
#include "appe_l2_vp.h"

sw_error_t
appe_vp_lrn_limit_counter_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union vp_lrn_limit_counter_u *value)
{
	if (index >= VP_LRN_LIMIT_COUNTER_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
				dev_id,
				ALDER_L2_BASE_ADDR + VP_LRN_LIMIT_COUNTER_ADDRESS + \
				index * VP_LRN_LIMIT_COUNTER_INC,
				&value->val);
}

sw_error_t
appe_l2_vp_port_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_vp_port_tbl_u *value)
{
	if (index >= L2_VP_PORT_TBL_NUM)
		return SW_OUT_OF_RANGE;
	return hppe_reg_tbl_get(
				dev_id,
				ALDER_L2_BASE_ADDR + L2_VP_PORT_TBL_ADDRESS + \
				index * L2_VP_PORT_TBL_INC,
				value->val,
				4);
}

sw_error_t
appe_l2_vp_port_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union l2_vp_port_tbl_u *value)
{
	if (index >= L2_VP_PORT_TBL_NUM)
		return SW_OUT_OF_RANGE;
	return hppe_reg_tbl_set(
				dev_id,
				ALDER_L2_BASE_ADDR + L2_VP_PORT_TBL_ADDRESS + \
				index * L2_VP_PORT_TBL_INC,
				value->val,
				4);
}
#if 0
sw_error_t
appe_vp_lrn_limit_counter_lrn_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union vp_lrn_limit_counter_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_vp_lrn_limit_counter_get(dev_id, index, &reg_val);
	*value = reg_val.bf.lrn_cnt;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vp_type;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vp_type = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_context_active_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vp_context_active;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_context_active_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vp_context_active = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_station_move_lrn_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.station_move_lrn_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_station_move_lrn_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.station_move_lrn_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_enq_phy_port_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.enq_phy_port;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_enq_phy_port_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.enq_phy_port = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_policer_index_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.policer_index;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_policer_index_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.policer_index = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_enq_service_code_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.enq_service_code;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_enq_service_code_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.enq_service_code = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_eg_ctag_fmt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eg_ctag_fmt;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_eg_ctag_fmt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eg_ctag_fmt = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_extra_header_len_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.extra_header_len;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_extra_header_len_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.extra_header_len = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_state_check_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vp_state_check_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_state_check_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vp_state_check_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_station_move_fwd_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.station_move_fwd_cmd;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_station_move_fwd_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.station_move_fwd_cmd = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_isol_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.isol_profile;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_isol_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.isol_profile = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_new_addr_fwd_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.new_addr_fwd_cmd;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_new_addr_fwd_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.new_addr_fwd_cmd = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
sw_error_t
appe_l2_vp_port_tbl_promisc_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.promisc_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_promisc_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.promisc_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#if 0
sw_error_t
appe_l2_vp_port_tbl_new_addr_lrn_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.new_addr_lrn_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_new_addr_lrn_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.new_addr_lrn_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_enq_service_code_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.enq_service_code_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_enq_service_code_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.enq_service_code_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_lrn_lmt_exceed_fwd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.lrn_lmt_exceed_fwd;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_lrn_lmt_exceed_fwd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.lrn_lmt_exceed_fwd = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
sw_error_t
appe_l2_vp_port_tbl_port_isolation_bitmap_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.port_isolation_bitmap;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_port_isolation_bitmap_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.port_isolation_bitmap = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#if 0
sw_error_t
appe_l2_vp_port_tbl_eg_vlan_fltr_cmd_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eg_vlan_fltr_cmd;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_eg_vlan_fltr_cmd_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eg_vlan_fltr_cmd = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_policer_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.policer_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_policer_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.policer_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_eg_vlan_fmt_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eg_vlan_fmt_valid;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_eg_vlan_fmt_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eg_vlan_fmt_valid = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_eg_data_valid_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.vp_eg_data_valid;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_vp_eg_data_valid_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.vp_eg_data_valid = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
sw_error_t
appe_l2_vp_port_tbl_dst_info_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.dst_info;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_dst_info_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.dst_info = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#if 0
sw_error_t
appe_l2_vp_port_tbl_lrn_lmt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.lrn_lmt_cnt;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_lrn_lmt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.lrn_lmt_cnt = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_lrn_lmt_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.lrn_lmt_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_lrn_lmt_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.lrn_lmt_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_isol_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.isol_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_isol_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.isol_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_exception_fmt_ctrl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.exception_fmt_ctrl;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_exception_fmt_ctrl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.exception_fmt_ctrl = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
sw_error_t
appe_l2_vp_port_tbl_app_ctrl_profile_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.app_ctrl_profile_1 << 6 | \
		reg_val.bf.app_ctrl_profile_0;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_app_ctrl_profile_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.app_ctrl_profile_1 = value >> 6;
	reg_val.bf.app_ctrl_profile_0 = value & (((a_uint64_t)1<<6)-1);
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#if 0
sw_error_t
appe_l2_vp_port_tbl_mtu_check_type_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.mtu_check_type;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_mtu_check_type_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.mtu_check_type = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
sw_error_t
appe_l2_vp_port_tbl_invalid_vsi_forwarding_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.invalid_vsi_forwarding_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_invalid_vsi_forwarding_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.invalid_vsi_forwarding_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#if 0
sw_error_t
appe_l2_vp_port_tbl_physical_port_mtu_check_en_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.physical_port_mtu_check_en;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_physical_port_mtu_check_en_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.physical_port_mtu_check_en = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
sw_error_t
appe_l2_vp_port_tbl_physical_port_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.physical_port;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_physical_port_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.physical_port = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#if 0
sw_error_t
appe_l2_vp_port_tbl_eg_stag_fmt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.eg_stag_fmt;
	return ret;
}

sw_error_t
appe_l2_vp_port_tbl_eg_stag_fmt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union l2_vp_port_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_l2_vp_port_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.eg_stag_fmt = value;
	ret = appe_l2_vp_port_tbl_set(dev_id, index, &reg_val);
	return ret;
}
#endif
