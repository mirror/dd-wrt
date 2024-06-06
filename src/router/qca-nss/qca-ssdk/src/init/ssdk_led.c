/*
 * Copyright (c) 2012, 2014-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "ssdk_init.h"
#include "ssdk_led.h"
#include "ssdk_plat.h"
#include "ssdk_dts.h"

sw_error_t ssdk_led_init(a_uint32_t dev_id, a_uint32_t port_id)
{
	a_uint32_t src_index = 0;
	led_ctrl_pattern_t pattern = {0};
	sw_error_t rv = SW_OK;

	for(src_index = 0; src_index < PORT_LED_SOURCE_MAX; src_index++) {
		rv = ssdk_dt_port_source_pattern_get(dev_id, port_id, src_index, &pattern);
		if(rv != SW_OK)
			continue;
		SSDK_INFO("port id 0x%x, ssdk_led_mode:%x, ssdk_led_map:%x, ssdk_led_src_id:%x\n",
			port_id, pattern.mode, pattern.map, src_index);
		rv = fal_port_led_source_pattern_set(dev_id, port_id, src_index, &pattern);
		SW_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}
