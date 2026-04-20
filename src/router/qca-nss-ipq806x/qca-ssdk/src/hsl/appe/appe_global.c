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

#include "sw.h"
#include "hsl.h"
#include "hppe_reg_access.h"
#include "appe_global_reg.h"
#include "appe_global.h"

sw_error_t
appe_port_mux_ctrl_get(
		a_uint32_t dev_id,
		union appe_port_mux_ctrl_u *value)
{
	return hppe_reg_get(
				dev_id,
				NSS_GLOBAL_BASE_ADDR + SWITCH_PORT_MUX_CTRL_ADDRESS,
				&value->val);
}
sw_error_t
appe_port_mux_ctrl_set(
		a_uint32_t dev_id,
		union appe_port_mux_ctrl_u *value)
{
	return hppe_reg_set(
				dev_id,
				NSS_GLOBAL_BASE_ADDR + SWITCH_PORT_MUX_CTRL_ADDRESS,
				value->val);
}

sw_error_t
appe_tx_buff_thrsh_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tx_buff_thrsh_u *value)
{
	if (index >= TX_BUFF_THRSH_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_get(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + TX_BUFF_THRSH_ADDRESS + \
				index * TX_BUFF_THRSH_INC,
				&value->val);
}

sw_error_t
appe_tx_buff_thrsh_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tx_buff_thrsh_u *value)
{
	if (index >= TX_BUFF_THRSH_MAX_ENTRY)
		return SW_OUT_OF_RANGE;

	return hppe_reg_set(
				dev_id,
				NSS_PTX_CSR_BASE_ADDR + TX_BUFF_THRSH_ADDRESS + \
				index * TX_BUFF_THRSH_INC,
				value->val);
}
