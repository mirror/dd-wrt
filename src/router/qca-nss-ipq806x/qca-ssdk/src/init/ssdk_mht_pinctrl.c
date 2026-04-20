/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include "ssdk_mht_pinctrl.h"


/****************************************************************************
 *
 * 1) PINs default Setting
 *
 ****************************************************************************/
#ifdef IN_PINCTRL_DEF_CONFIG
static a_ulong_t pin_configs[] = {
    PIN_CONFIG_OUTPUT_ENABLE,
    PIN_CONFIG_BIAS_PULL_DOWN,
};
#endif

static struct mht_pinctrl_setting mht_pin_settings[] = {
    /*PINs default MUX Setting*/
    MHT_PIN_SETTING_MUX(0,  MHT_PIN_FUNC_INTN_WOL),
    MHT_PIN_SETTING_MUX(1,  MHT_PIN_FUNC_INTN),
#if 0
    MHT_PIN_SETTING_MUX(2,  MHT_PIN_FUNC_P0_LED_0),
    MHT_PIN_SETTING_MUX(3,  MHT_PIN_FUNC_P1_LED_0),
    MHT_PIN_SETTING_MUX(4,  MHT_PIN_FUNC_P2_LED_0),
    MHT_PIN_SETTING_MUX(5,  MHT_PIN_FUNC_P3_LED_0),
    MHT_PIN_SETTING_MUX(6,  MHT_PIN_FUNC_PPS_IN),
    MHT_PIN_SETTING_MUX(7,  MHT_PIN_FUNC_TOD_IN),
    MHT_PIN_SETTING_MUX(8,  MHT_PIN_FUNC_RTC_REFCLK_IN),
    MHT_PIN_SETTING_MUX(9,  MHT_PIN_FUNC_P0_PPS_OUT),
#endif
    MHT_PIN_SETTING_MUX(10, MHT_PIN_FUNC_P1_PPS_OUT),
    MHT_PIN_SETTING_MUX(11, MHT_PIN_FUNC_P2_PPS_OUT),
    MHT_PIN_SETTING_MUX(12, MHT_PIN_FUNC_P3_PPS_OUT),
    MHT_PIN_SETTING_MUX(13, MHT_PIN_FUNC_P0_TOD_OUT),
    MHT_PIN_SETTING_MUX(14, MHT_PIN_FUNC_P0_CLK125_TDI),
    MHT_PIN_SETTING_MUX(15, MHT_PIN_FUNC_P0_SYNC_CLKO_PTP),
#if 0
    MHT_PIN_SETTING_MUX(16, MHT_PIN_FUNC_P0_LED_1),
    MHT_PIN_SETTING_MUX(17, MHT_PIN_FUNC_P1_LED_1),
    MHT_PIN_SETTING_MUX(18, MHT_PIN_FUNC_P2_LED_1),
    MHT_PIN_SETTING_MUX(19, MHT_PIN_FUNC_P3_LED_1),
#endif
    MHT_PIN_SETTING_MUX(20, MHT_PIN_FUNC_MDC_M),
    MHT_PIN_SETTING_MUX(21, MHT_PIN_FUNC_MDO_M),

    /*PINs default Config Setting*/
#ifdef IN_PINCTRL_DEF_CONFIG
    MHT_PIN_SETTING_CONFIG(0,  pin_configs),
    MHT_PIN_SETTING_CONFIG(1,  pin_configs),
    MHT_PIN_SETTING_CONFIG(2,  pin_configs),
    MHT_PIN_SETTING_CONFIG(3,  pin_configs),
    MHT_PIN_SETTING_CONFIG(4,  pin_configs),
    MHT_PIN_SETTING_CONFIG(5,  pin_configs),
    MHT_PIN_SETTING_CONFIG(6,  pin_configs),
    MHT_PIN_SETTING_CONFIG(7,  pin_configs),
    MHT_PIN_SETTING_CONFIG(8,  pin_configs),
    MHT_PIN_SETTING_CONFIG(9,  pin_configs),
    MHT_PIN_SETTING_CONFIG(10, pin_configs),
    MHT_PIN_SETTING_CONFIG(11, pin_configs),
    MHT_PIN_SETTING_CONFIG(12, pin_configs),
    MHT_PIN_SETTING_CONFIG(13, pin_configs),
    MHT_PIN_SETTING_CONFIG(14, pin_configs),
    MHT_PIN_SETTING_CONFIG(15, pin_configs),
    MHT_PIN_SETTING_CONFIG(16, pin_configs),
    MHT_PIN_SETTING_CONFIG(17, pin_configs),
    MHT_PIN_SETTING_CONFIG(18, pin_configs),
    MHT_PIN_SETTING_CONFIG(19, pin_configs),
    MHT_PIN_SETTING_CONFIG(20, pin_configs),
    MHT_PIN_SETTING_CONFIG(21, pin_configs),
#endif
};

/****************************************************************************
 *
 * 2) PINs Operations
 *
 ****************************************************************************/
sw_error_t mht_gpio_set_bit(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t value)
{
    sw_error_t rv = SW_OK;

    MHT_REG_FIELD_SET(rv, dev_id, TLMM_GPIO_IN_OUTN, pin, GPIO_OUT,
                      (a_uint8_t *) (&value), sizeof(a_uint32_t));
    SSDK_DEBUG("[%s] select pin:%d value:%d\n", __func__, pin, value);

    return rv;
}

sw_error_t mht_gpio_get_bit(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t *data)
{
    sw_error_t rv = SW_OK;

    MHT_REG_FIELD_GET(rv, dev_id, TLMM_GPIO_IN_OUTN, pin, GPIO_IN,
                      (a_uint8_t *) (data), sizeof(a_uint32_t));
    SSDK_DEBUG("[%s] select pin:%d value:%d\n", __func__, pin, *data);

    return rv;
}

sw_error_t mht_gpio_pin_mux_set(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t func)
{
    sw_error_t rv = SW_OK;

    SSDK_DEBUG("[%s] select pin:%d func:%d\n", __func__, pin, func);
    MHT_REG_FIELD_SET(rv, dev_id, TLMM_GPIO_CFGN, pin, FUNC_SEL,
                      (a_uint8_t *) (&func), sizeof(a_uint32_t));

    return rv;
}

sw_error_t mht_gpio_pin_cfg_set_bias(a_uint32_t dev_id, a_uint32_t pin, enum pin_config_param bias)
{
    sw_error_t rv = SW_OK;
    a_uint32_t data = 0;

    switch (bias)
    {
        case PIN_CONFIG_BIAS_DISABLE:
            data = MHT_TLMM_GPIO_CFGN_GPIO_PULL_DISABLE;
            break;
        case PIN_CONFIG_BIAS_PULL_DOWN:
            data = MHT_TLMM_GPIO_CFGN_GPIO_PULL_DOWN;
            break;
        case PIN_CONFIG_BIAS_BUS_HOLD:
            data = MHT_TLMM_GPIO_CFGN_GPIO_PULL_BUS_HOLD;
            break;
        case PIN_CONFIG_BIAS_PULL_UP:
            data = MHT_TLMM_GPIO_CFGN_GPIO_PULL_UP;
            break;
        default:
            SSDK_ERROR("[%s] doesn't support bias:%d\n", __func__, bias);
            return -1;
    }

    MHT_REG_FIELD_SET(rv, dev_id, TLMM_GPIO_CFGN, pin, GPIO_PULL,
                      (a_uint8_t *) (&data), sizeof(a_uint32_t));
    SSDK_DEBUG("[%s]pin:%d bias:%d", __func__, pin, bias);

    return rv;
}

sw_error_t mht_gpio_pin_cfg_get_bias(a_uint32_t dev_id, a_uint32_t pin, enum pin_config_param *bias)
{
    sw_error_t rv = SW_OK;
    a_uint32_t data = 0;

    MHT_REG_FIELD_GET(rv, dev_id, TLMM_GPIO_CFGN, pin, GPIO_PULL,
                      (a_uint8_t *) (&data), sizeof (a_uint32_t));
    switch (data)
    {
        case MHT_TLMM_GPIO_CFGN_GPIO_PULL_DISABLE:
            *bias = PIN_CONFIG_BIAS_DISABLE;
            break;
        case MHT_TLMM_GPIO_CFGN_GPIO_PULL_DOWN:
            *bias = PIN_CONFIG_BIAS_PULL_DOWN;
            break;
        case MHT_TLMM_GPIO_CFGN_GPIO_PULL_BUS_HOLD:
            *bias = PIN_CONFIG_BIAS_BUS_HOLD;
            break;
        case MHT_TLMM_GPIO_CFGN_GPIO_PULL_UP:
            *bias = PIN_CONFIG_BIAS_PULL_UP;
            break;
        default:
            SSDK_ERROR("[%s] doesn't support bias:%d\n", __func__, data);
            return SW_FAIL;
    }
    SSDK_DEBUG("[%s]pin:%d bias:%d", __func__, pin, *bias);

    return rv;
}

sw_error_t mht_gpio_pin_cfg_set_drvs(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t drvs)
{
    sw_error_t rv = SW_OK;

    if((drvs < MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_2_MA) ||
        (drvs > MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_16_MA)) {
        SSDK_ERROR("[%s] doesn't support drvs:%d\n", __func__, drvs);
        return SW_BAD_PARAM;
    }

    MHT_REG_FIELD_SET(rv, dev_id, TLMM_GPIO_CFGN, pin, DRV_STRENGTH,
                      (a_uint8_t *) (&drvs), sizeof(a_uint32_t));
    SSDK_DEBUG("[%s]%d", __func__, pin);

    return rv;
}

sw_error_t mht_gpio_pin_cfg_get_drvs(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t *drvs)
{
    sw_error_t rv = SW_OK;

    MHT_REG_FIELD_GET(rv, dev_id, TLMM_GPIO_CFGN, pin, DRV_STRENGTH,
                      (a_uint8_t *) (drvs), sizeof(a_uint32_t));
    SSDK_DEBUG("[%s]%d", __func__, pin);

    return rv;
}

sw_error_t mht_gpio_pin_cfg_set_hihys(a_uint32_t dev_id, a_uint32_t pin, a_bool_t hihys_en)
{
    sw_error_t rv = SW_OK;

    SSDK_DEBUG("pin:%d hihys_en:%d", pin, hihys_en);

    MHT_REG_FIELD_SET(rv, dev_id, TLMM_GPIO_CFGN, pin, GPIO_HIHYS_EN,
                      (a_uint8_t *) (&hihys_en), sizeof(a_uint32_t));

    return rv;
}

sw_error_t mht_gpio_pin_cfg_get_hihys(a_uint32_t dev_id, a_uint32_t pin, a_bool_t *hihys_en)
{
    sw_error_t rv = SW_OK;
    a_uint32_t data = 0;

    MHT_REG_FIELD_GET(rv, dev_id, TLMM_GPIO_CFGN, pin, GPIO_HIHYS_EN,
                      (a_uint8_t *) (&data), sizeof (a_uint32_t));
    *hihys_en = data ? A_TRUE : A_FALSE;

    SSDK_DEBUG("pin:%d hihys_en:%d", pin, *hihys_en);

    return rv;
}

sw_error_t mht_gpio_pin_cfg_set_oe(a_uint32_t dev_id, a_uint32_t pin, a_bool_t oe)
{
    sw_error_t rv = SW_OK;

    SSDK_DEBUG("[%s]%d oe:%d", __func__, pin, oe);

    MHT_REG_FIELD_SET(rv, dev_id, TLMM_GPIO_CFGN, pin, GPIO_OE,
                      (a_uint8_t *) (&oe), sizeof(a_uint32_t));

    return rv;
}

sw_error_t mht_gpio_pin_cfg_get_oe(a_uint32_t dev_id, a_uint32_t pin, a_bool_t *oe)
{
    sw_error_t rv = SW_OK;
    a_uint32_t data = 0;

    MHT_REG_FIELD_GET(rv, dev_id, TLMM_GPIO_CFGN, pin, GPIO_OE,
                      (a_uint8_t *) (&data), sizeof (a_uint32_t));
    *oe = data ? A_TRUE : A_FALSE;

    SSDK_DEBUG("[%s]%d oe:%d", __func__, pin, *oe);

    return rv;
}

static sw_error_t mht_gpio_pin_cfg_set(a_uint32_t dev_id, a_uint32_t pin,
                  a_ulong_t *configs, a_uint32_t num_configs)
{
    sw_error_t rv = SW_OK;
    enum pin_config_param param;
    a_uint32_t i, arg;

    for (i = 0; i < num_configs; i++) {
        param = pinconf_to_config_param(configs[i]);
        arg = pinconf_to_config_argument(configs[i]);

        switch (param) {
            case PIN_CONFIG_BIAS_BUS_HOLD:
            case PIN_CONFIG_BIAS_DISABLE:
            case PIN_CONFIG_BIAS_PULL_DOWN:
            case PIN_CONFIG_BIAS_PULL_UP:
                rv = mht_gpio_pin_cfg_set_bias(dev_id, pin, param);
                break;

            case PIN_CONFIG_DRIVE_STRENGTH:
                rv = mht_gpio_pin_cfg_set_drvs(dev_id, pin, arg);
                break;

            case PIN_CONFIG_OUTPUT:
                rv = mht_gpio_pin_cfg_set_oe(dev_id, pin, A_TRUE);
                rv = mht_gpio_set_bit(dev_id, pin, arg);
                break;

            case PIN_CONFIG_INPUT_ENABLE:
                rv = mht_gpio_pin_cfg_set_oe(dev_id, pin, A_FALSE);
                break;

            case PIN_CONFIG_OUTPUT_ENABLE:
                rv = mht_gpio_pin_cfg_set_oe(dev_id, pin, A_TRUE);
                break;

            default:
                SSDK_ERROR("%s doesn't support:%d\n", __func__, param);
                return SW_BAD_PARAM;
        }
    }

    return SW_OK;
}


/****************************************************************************
 *
 * 3) PINs Init
 *
 ****************************************************************************/
sw_error_t mht_pinctrl_clk_gate_set(a_uint32_t dev_id, a_bool_t gate_en)
{
    sw_error_t rv = SW_OK;

    MHT_REG_FIELD_SET(rv, dev_id, TLMM_CLK_GATE_EN, 0, AHB_HCLK_EN,
                      (a_uint8_t *) (&gate_en), sizeof (a_bool_t));
    MHT_REG_FIELD_SET(rv, dev_id, TLMM_CLK_GATE_EN, 0, SUMMARY_INTR_EN,
                      (a_uint8_t *) (&gate_en), sizeof (a_bool_t));
    MHT_REG_FIELD_SET(rv, dev_id, TLMM_CLK_GATE_EN, 0, CRIF_READ_EN,
                      (a_uint8_t *) (&gate_en), sizeof (a_bool_t));

    SSDK_INFO("[%s] gate_en:%d", __func__, gate_en);

    return rv;
}

static sw_error_t mht_pinctrl_rev_check(a_uint32_t dev_id)
{
    sw_error_t rv = SW_OK;
    a_uint32_t version_id = 0, mfg_id = 0, start_bit = 0;

    MHT_REG_FIELD_GET(rv, dev_id, TLMM_HW_REVISION_NUMBER, 0, VERSION_ID,
                      (a_uint8_t *) (&version_id), sizeof (a_uint32_t));
    MHT_REG_FIELD_GET(rv, dev_id, TLMM_HW_REVISION_NUMBER, 0, MFG_ID,
                      (a_uint8_t *) (&mfg_id), sizeof (a_uint32_t));
    MHT_REG_FIELD_GET(rv, dev_id, TLMM_HW_REVISION_NUMBER, 0, START_BIT,
                      (a_uint8_t *) (&start_bit), sizeof (a_uint32_t));

    SSDK_INFO("[%s] version_id:0x%x mfg_id:0x%x start_bit:0x%x",
                                __func__, version_id, mfg_id, start_bit);

    if((version_id == 0x0) && (mfg_id == 0x70) && (start_bit == 0x1)) {
        SSDK_INFO(" Pinctrl Version Check Pass\n");
    } else {
        SSDK_INFO(" Pinctrl Version Check Fail\n");
        rv = SW_FAIL;
    }

    return rv;
}

static sw_error_t mht_pinctrl_hw_init(a_uint32_t dev_id)
{
    sw_error_t rv = SW_OK;

    rv = mht_pinctrl_clk_gate_set(dev_id, A_TRUE);
    rv = mht_pinctrl_rev_check(dev_id);

	return rv;
}

static sw_error_t mht_pinctrl_setting_init(a_uint32_t dev_id,
                                        const struct mht_pinctrl_setting *pin_settings,
                                        a_uint32_t num_setting)
{
    sw_error_t rv = SW_OK;
    a_uint32_t i;

    for(i = 0; i < num_setting; i++) {
        const struct mht_pinctrl_setting *setting = &pin_settings[i];
        if (setting->type == PIN_MAP_TYPE_MUX_GROUP) {
            rv = mht_gpio_pin_mux_set(dev_id, setting->data.mux.pin, setting->data.mux.func);

        } else if (setting->type == PIN_MAP_TYPE_CONFIGS_PIN) {
            rv = mht_gpio_pin_cfg_set(dev_id,
                                      setting->data.configs.pin,
                                      setting->data.configs.configs,
                                      setting->data.configs.num_configs);
        }
    }

    return rv;
}

sw_error_t ssdk_mht_pinctrl_init(a_uint32_t dev_id)
{
	mht_pinctrl_hw_init(dev_id);
	mht_pinctrl_setting_init(dev_id, mht_pin_settings, ARRAY_SIZE(mht_pin_settings));

	SSDK_INFO(" %s done\n", __func__);

    return SW_OK;
}

