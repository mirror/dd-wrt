/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "ssdk_plat.h"
#include "qca808x_phy.h"
#include "qca808x_led.h"
#include "qcaphy_common.h"
#ifdef MHT
#include "ssdk_mht_pinctrl.h"
#include "mht_sec_ctrl.h"
#endif

static sw_error_t
_qca808x_phy_led_pattern_map_from_phy(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *map, a_uint16_t phy_data)
{
	if (qca808x_phy_2500caps(dev_id, phy_addr) == A_TRUE)
	{
		if(phy_data & QCA808X_PHY_LINK_2500M_LIGHT_EN)
		{
			*map |= BIT(LINK_2500M_LIGHT_EN);
		}
	}

	return qcaphy_led_pattern_map_from_phy(dev_id, map, phy_data);
}

static sw_error_t
_qca808x_phy_led_pattern_map_to_phy(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t map, a_uint16_t *phy_data)
{
	if (qca808x_phy_2500caps(dev_id, phy_addr) == A_TRUE)
	{
		if (map & BIT(LINK_2500M_LIGHT_EN))
		{
			*phy_data |=  QCA808X_PHY_LINK_2500M_LIGHT_EN;
		}
	}

	return qcaphy_led_pattern_map_to_phy(dev_id, map, phy_data);
}

a_uint32_t
qca808x_phy_led_source_map_mmd_reg_get(a_uint32_t dev_id, a_uint32_t source_id)
{
	a_uint16_t mmd_reg = 0;

	switch(source_id)
	{
		case QCAPHY_LED_SOURCE0:
			mmd_reg = QCA808X_PHY_MMD7_LED0_MAP_CTRL;
			break;
		case QCAPHY_LED_SOURCE1:
			mmd_reg = QCA808X_PHY_MMD7_LED1_MAP_CTRL;
			break;
		case QCAPHY_LED_SOURCE2:
			mmd_reg = QCA808X_PHY_MMD7_LED2_MAP_CTRL;
			break;
		default:
			SSDK_ERROR("source %d is not support\n", source_id);
			break;
	}

	return mmd_reg;
}

a_uint32_t
qca808x_phy_led_source_force_mmd_reg_get(a_uint32_t dev_id, a_uint32_t source_id)
{
	a_uint16_t mmd_reg = 0;

	switch(source_id)
	{
		case QCAPHY_LED_SOURCE0:
			mmd_reg = QCA808X_PHY_MMD7_LED0_FORCE_CTRL;
			break;
		case QCAPHY_LED_SOURCE1:
			mmd_reg = QCA808X_PHY_MMD7_LED1_FORCE_CTRL;
			break;
		case QCAPHY_LED_SOURCE2:
			mmd_reg = QCA808X_PHY_MMD7_LED2_FORCE_CTRL;
			break;
		default:
			SSDK_ERROR("source %d is not support\n", source_id);
			break;
	}

	return mmd_reg;
}

#ifdef MHT
#define QCA8084_LED_FUNC(lend_func, mht_port_id, source_id) \
{                                                           \
    if(mht_port_id == SSDK_PHYSICAL_PORT1)                  \
        lend_func = MHT_PIN_FUNC_P0_LED_##source_id;        \
    else if(mht_port_id == SSDK_PHYSICAL_PORT2)             \
        lend_func = MHT_PIN_FUNC_P1_LED_##source_id;        \
    else if(mht_port_id == SSDK_PHYSICAL_PORT3)             \
        lend_func = MHT_PIN_FUNC_P2_LED_##source_id;        \
    else                                                    \
        lend_func = MHT_PIN_FUNC_P3_LED_##source_id;        \
}

static sw_error_t
qca8084_phy_led_ctrl_source_pin_cfg(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mht_port_id = 0, led_start_pin = 0, led_pin = 0, led_func = 0;

	/*get the led pin and led func for mht port*/
	rv = qca_mht_port_id_get(dev_id, phy_addr, &mht_port_id);
	PHY_RTN_ON_ERROR(rv);
	if(source_id == QCAPHY_LED_SOURCE0)
	{
		led_start_pin = 2;
		QCA8084_LED_FUNC(led_func, mht_port_id, 0)
	}
	else if(source_id == QCAPHY_LED_SOURCE1)
	{
		led_start_pin = 16;
		QCA8084_LED_FUNC(led_func, mht_port_id, 1)
	}
	else
	{
		led_start_pin = 6;
		QCA8084_LED_FUNC(led_func, mht_port_id, 2)
	}
	led_pin = led_start_pin + (mht_port_id - SSDK_PHYSICAL_PORT1);
	SSDK_DEBUG("mht port:%d, led_pin:%d, led_func:%d", mht_port_id,
		led_pin, led_func);
	rv = mht_gpio_pin_mux_set(dev_id, led_pin, led_func);

	return rv;
}
#endif

static sw_error_t
qca808x_phy_led_force_pattern_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, a_bool_t enable, a_uint32_t force_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	mmd_reg = qca808x_phy_led_source_force_mmd_reg_get(dev_id, source_id);
	if(enable) {
		rv = qcaphy_led_pattern_force_to_phy(dev_id, force_mode, &phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		mmd_reg, QCAPHY_PHY_LED_FORCE_EN | QCAPHY_PHY_LED_FORCE_MASK, phy_data);
}

static sw_error_t
qca808x_phy_led_force_pattern_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, a_bool_t *enable, a_uint32_t *force_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	mmd_reg = qca808x_phy_led_source_force_mmd_reg_get(dev_id, source_id);
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		mmd_reg);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(phy_data  & QCAPHY_PHY_LED_FORCE_EN) {
		*enable = A_TRUE;
		rv = qcaphy_led_pattern_force_from_phy(dev_id, force_mode, phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	else
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
qca808x_phy_led_ctrl_source_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	if(pattern->mode == LED_PATTERN_MODE_BUTT || source_id > QCAPHY_LED_SOURCE2)
		return SW_NOT_SUPPORTED;

	rv = qcaphy_led_active_set(dev_id,  phy_addr, pattern->active_level);
	PHY_RTN_ON_ERROR(rv);
	/*set blink frequency*/
	rv = qcaphy_led_blink_freq_set(dev_id, phy_addr, pattern->mode, pattern->freq);
	PHY_RTN_ON_ERROR(rv);
	if(pattern->mode == LED_PATTERN_MAP_EN) {
		rv = qca808x_phy_led_force_pattern_set(dev_id, phy_addr, source_id, A_FALSE,
			pattern->mode);
		PHY_RTN_ON_ERROR(rv);
		rv = _qca808x_phy_led_pattern_map_to_phy(dev_id, phy_addr, pattern->map,
			&phy_data);
		PHY_RTN_ON_ERROR(rv);
		mmd_reg = qca808x_phy_led_source_map_mmd_reg_get(dev_id, source_id);
		rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
			mmd_reg, phy_data);
		PHY_RTN_ON_ERROR(rv);
	} else {
		rv = qca808x_phy_led_force_pattern_set(dev_id, phy_addr, source_id, A_TRUE,
			pattern->mode);
		PHY_RTN_ON_ERROR(rv);
	}
#ifdef MHT
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_led_ctrl_source_pin_cfg(dev_id, phy_addr, source_id);
		PHY_RTN_ON_ERROR(rv);
	}
#endif
	return SW_OK;
}
/******************************************************************************
*
* qca808x_phy_led_source_pattern_get
*
*/
sw_error_t
qca808x_phy_led_ctrl_source_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;
	a_bool_t force_enable = A_FALSE;

	if(source_id > QCAPHY_LED_SOURCE2)
		return SW_NOT_SUPPORTED;

	rv = qcaphy_led_active_get(dev_id, phy_addr, &(pattern->active_level));
	PHY_RTN_ON_ERROR(rv);
	pattern->map = 0;
	rv = qca808x_phy_led_force_pattern_get(dev_id, phy_addr, source_id,
		&force_enable, &(pattern->mode));
	PHY_RTN_ON_ERROR(rv);
	if(!force_enable) {
		pattern->mode = LED_PATTERN_MAP_EN;
		mmd_reg = qca808x_phy_led_source_map_mmd_reg_get(dev_id, source_id);
		phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCA808X_PHY_MMD7_NUM, mmd_reg);
		PHY_RTN_ON_READ_ERROR(phy_data);
		rv = _qca808x_phy_led_pattern_map_from_phy(dev_id, phy_addr, &(pattern->map),
			phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	rv = qcaphy_led_blink_freq_get(dev_id, phy_addr, pattern->mode, &(pattern->freq));
	PHY_RTN_ON_ERROR(rv);

	return SW_OK;
}

void qca808x_phy_led_api_ops_init(hsl_phy_ops_t *qca808x_phy_led_api_ops)
{
	if (!qca808x_phy_led_api_ops) {
		return;
	}
	qca808x_phy_led_api_ops->phy_led_ctrl_source_set = qca808x_phy_led_ctrl_source_set;
	qca808x_phy_led_api_ops->phy_led_ctrl_source_get = qca808x_phy_led_ctrl_source_get;

	return;
}
