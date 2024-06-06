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
#include "hsl.h"
#include "hppe_reg_access.h"
#include "appe_counter_reg.h"
#include "appe_counter.h"
sw_error_t
appe_port_rx_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union port_rx_cnt_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + PORT_RX_CNT_TBL_ADDRESS + \
				index * PORT_RX_CNT_TBL_INC,
				value->val,
				5);
}

sw_error_t
appe_port_rx_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union port_rx_cnt_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + PORT_RX_CNT_TBL_ADDRESS + \
				index * PORT_RX_CNT_TBL_INC,
				value->val,
				5);
}

sw_error_t
appe_port_vp_rx_cnt_mode_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union port_vp_rx_cnt_mode_tbl_u *value)
{
	if (index >= PORT_VP_RX_CNT_MODE_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_get(
			dev_id,
			INGRESS_POLICER_BASE_ADDR + PORT_VP_RX_CNT_MODE_ADDRESS + \
			index * PORT_VP_RX_CNT_MODE_INC,
			&value->val);
}

sw_error_t
appe_port_vp_rx_cnt_mode_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union port_vp_rx_cnt_mode_tbl_u *value)
{
	if (index >= PORT_VP_RX_CNT_MODE_MAX_ENTRY)
		return SW_OUT_OF_RANGE;
	return hppe_reg_set(
			dev_id,
			INGRESS_POLICER_BASE_ADDR + PORT_VP_RX_CNT_MODE_ADDRESS + \
			index * PORT_VP_RX_CNT_MODE_INC,
			value->val);
}

sw_error_t
appe_port_rx_cnt_tbl_rx_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.rx_pkt_cnt;
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_pkt_cnt = value;
	ret = appe_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.rx_byte_cnt_1 << 32 | \
		reg_val.bf.rx_byte_cnt_0;
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_byte_cnt_1 = value >> 32;
	reg_val.bf.rx_byte_cnt_0 = value & (((a_uint64_t)1<<32)-1);
	ret = appe_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_drop_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.rx_drop_pkt_cnt_1 << 24 | \
		reg_val.bf.rx_drop_pkt_cnt_0;
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_drop_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_drop_pkt_cnt_1 = value >> 24;
	reg_val.bf.rx_drop_pkt_cnt_0 = value & (((a_uint64_t)1<<24)-1);
	ret = appe_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_drop_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.rx_drop_byte_cnt_1 << 24 | \
		reg_val.bf.rx_drop_byte_cnt_0;
	return ret;
}

sw_error_t
appe_port_rx_cnt_tbl_rx_drop_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_drop_byte_cnt_1 = value >> 24;
	reg_val.bf.rx_drop_byte_cnt_0 = value & (((a_uint64_t)1<<24)-1);
	ret = appe_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union phy_port_rx_cnt_tbl_u *value)
{
	return hppe_reg_tbl_get(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + PHY_PORT_RX_CNT_TBL_ADDRESS + \
				index * PHY_PORT_RX_CNT_TBL_INC,
				value->val,
				5);
}

sw_error_t
appe_phy_port_rx_cnt_tbl_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union phy_port_rx_cnt_tbl_u *value)
{
	return hppe_reg_tbl_set(
				dev_id,
				INGRESS_POLICER_BASE_ADDR + PHY_PORT_RX_CNT_TBL_ADDRESS + \
				index * PHY_PORT_RX_CNT_TBL_INC,
				value->val,
				5);
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.rx_pkt_cnt;
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_pkt_cnt = value;
	ret = appe_phy_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.rx_byte_cnt_1 << 32 | \
		reg_val.bf.rx_byte_cnt_0;
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_byte_cnt_1 = value >> 32;
	reg_val.bf.rx_byte_cnt_0 = value & (((a_uint64_t)1<<32)-1);
	ret = appe_phy_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_drop_pkt_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t *value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = reg_val.bf.rx_drop_pkt_cnt_1 << 24 | \
		reg_val.bf.rx_drop_pkt_cnt_0;
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_drop_pkt_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint32_t value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_drop_pkt_cnt_1 = value >> 24;
	reg_val.bf.rx_drop_pkt_cnt_0 = value & (((a_uint64_t)1<<24)-1);
	ret = appe_phy_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_drop_byte_cnt_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t *value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	*value = (a_uint64_t)reg_val.bf.rx_drop_byte_cnt_1 << 24 | \
		reg_val.bf.rx_drop_byte_cnt_0;
	return ret;
}

sw_error_t
appe_phy_port_rx_cnt_tbl_rx_drop_byte_cnt_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		a_uint64_t value)
{
	union phy_port_rx_cnt_tbl_u reg_val;
	sw_error_t ret = SW_OK;

	ret = appe_phy_port_rx_cnt_tbl_get(dev_id, index, &reg_val);
	if (SW_OK != ret)
		return ret;
	reg_val.bf.rx_drop_byte_cnt_1 = value >> 24;
	reg_val.bf.rx_drop_byte_cnt_0 = value & (((a_uint64_t)1<<24)-1);
	ret = appe_phy_port_rx_cnt_tbl_set(dev_id, index, &reg_val);
	return ret;
}
