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

#ifndef _QCA808X_LED_H_
#define _QCA808X_LED_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */
#define QCA808X_PHY_LINK_2500M_LIGHT_EN         0x8000
#define QCA808X_PHY_MMD7_LED0_MAP_CTRL          0x8078
#define QCA808X_PHY_MMD7_LED1_MAP_CTRL          0x8074
#define QCA808X_PHY_MMD7_LED2_MAP_CTRL          0x8076
#define QCA808X_PHY_MMD7_LED0_FORCE_CTRL        0x8079
#define QCA808X_PHY_MMD7_LED1_FORCE_CTRL        0x8075
#define QCA808X_PHY_MMD7_LED2_FORCE_CTRL        0x8077

sw_error_t
qca808x_phy_led_ctrl_source_set(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern);
sw_error_t
qca808x_phy_led_ctrl_source_get(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern);
void qca808x_phy_led_api_ops_init(hsl_phy_ops_t *qca808x_phy_led_api_ops);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _qca808x_LED_H_ */
