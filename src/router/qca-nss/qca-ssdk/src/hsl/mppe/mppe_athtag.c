/*
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
#include "mppe_athtag_reg.h"
#include "mppe_athtag.h"

sw_error_t
mppe_eg_hdr_xmit_pri_mapping_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_hdr_xmit_pri_mapping_u *value)
{
	if (index >= EG_HDR_XMIT_PRI_MAPPING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_HDR_XMIT_PRI_MAPPING_ADDRESS + \
				index * EG_HDR_XMIT_PRI_MAPPING_INC,
				&value->val,
				1);
}

sw_error_t
mppe_eg_hdr_xmit_pri_mapping_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union eg_hdr_xmit_pri_mapping_u *value)
{
	if (index >= EG_HDR_XMIT_PRI_MAPPING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_HDR_XMIT_PRI_MAPPING_ADDRESS + \
				index * EG_HDR_XMIT_PRI_MAPPING_INC,
				&value->val,
				1);
}

sw_error_t
mppe_eg_gen_ctrl_set(
		a_uint32_t dev_id,
		union eg_gen_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_GEN_CTRL_ADDRESS,
				value->val);
}

sw_error_t
mppe_eg_gen_ctrl_get(
		a_uint32_t dev_id,
		union eg_gen_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + EG_GEN_CTRL_ADDRESS,
				&value->val);
}

sw_error_t
mppe_prx_port_to_vp_mapping_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_port_to_vp_mapping_u *value)
{
	if (index >= PRX_PORT_TO_VP_MAPPING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_get(
				dev_id,
				NSS_PRX_CSR_BASE_ADDR + PRX_PORT_TO_VP_MAPPING_ADDRESS + \
				index * PRX_PORT_TO_VP_MAPPING_INC,
				&value->val,
				1);
}

sw_error_t
mppe_prx_port_to_vp_mapping_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_port_to_vp_mapping_u *value)
{
	if (index >= PRX_PORT_TO_VP_MAPPING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_set(
				dev_id,
				NSS_PRX_CSR_BASE_ADDR + PRX_PORT_TO_VP_MAPPING_ADDRESS + \
				index * PRX_PORT_TO_VP_MAPPING_INC,
				&value->val,
				1);
}

sw_error_t
mppe_prx_hdr_rcv_pri_mapping_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_hdr_rcv_pri_mapping_u *value)
{
	if (index >= PRX_HDR_RCV_PRI_MAPPING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_get(
				dev_id,
				NSS_PRX_CSR_BASE_ADDR + PRX_HDR_RCV_PRI_MAPPING_ADDRESS + \
				index * PRX_HDR_RCV_PRI_MAPPING_INC,
				&value->val,
				1);
}

sw_error_t
mppe_prx_hdr_rcv_pri_mapping_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union prx_hdr_rcv_pri_mapping_u *value)
{
	if (index >= PRX_HDR_RCV_PRI_MAPPING_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_tbl_set(
				dev_id,
				NSS_PRX_CSR_BASE_ADDR + PRX_HDR_RCV_PRI_MAPPING_ADDRESS + \
				index * PRX_HDR_RCV_PRI_MAPPING_INC,
				&value->val,
				1);
}