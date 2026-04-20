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

#ifndef _APPE_GLOBAL_H_
#define _APPE_GLOBAL_H_
#include "appe_global_reg.h"

#if defined(MPPE)
#define TX_BUFF_THRSH_MAX_ENTRY		3
#else
#define TX_BUFF_THRSH_MAX_ENTRY		8
#endif

sw_error_t
appe_port_mux_ctrl_get(
		a_uint32_t dev_id,
		union appe_port_mux_ctrl_u *value);

sw_error_t
appe_port_mux_ctrl_set(
		a_uint32_t dev_id,
		union appe_port_mux_ctrl_u *value);

sw_error_t
appe_tx_buff_thrsh_get(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tx_buff_thrsh_u *value);

sw_error_t
appe_tx_buff_thrsh_set(
		a_uint32_t dev_id,
		a_uint32_t index,
		union tx_buff_thrsh_u *value);
#endif
