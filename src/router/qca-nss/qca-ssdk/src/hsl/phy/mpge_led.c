/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
#include "hsl_phy.h"
#include "qca808x_led.h"

sw_error_t
mpge_phy_led_ctrl_source_set(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t source_id, led_ctrl_pattern_t * pattern)
{
	return qca808x_phy_led_ctrl_source_set(dev_id, phy_id, source_id, pattern);
}

sw_error_t
mpge_phy_led_ctrl_source_get(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t source_id, led_ctrl_pattern_t * pattern)
{
	return qca808x_phy_led_ctrl_source_get(dev_id, phy_id, source_id, pattern);
}

void mpge_phy_led_api_ops_init(hsl_phy_ops_t *mpge_phy_led_api_ops)
{
	if (!mpge_phy_led_api_ops) {
		return;
	}
	mpge_phy_led_api_ops->phy_led_ctrl_source_set = mpge_phy_led_ctrl_source_set;
	mpge_phy_led_api_ops->phy_led_ctrl_source_get = mpge_phy_led_ctrl_source_get;

	return;
}
