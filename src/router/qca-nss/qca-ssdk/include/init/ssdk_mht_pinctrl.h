/*
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

#ifndef _SSDK_MHT_PINCTRL_H_
#define _SSDK_MHT_PINCTRL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sw.h"
#include "hsl.h"
#include "ssdk_plat.h"
#include <linux/pinctrl/pinconf-generic.h>


/****************************************************************************
 *
 *  1) PinCtrl/TLMM Register Definition
 *
 ****************************************************************************/
/* TLMM_GPIO_CFGn */
#define TLMM_GPIO_CFGN
#define TLMM_GPIO_CFGN_OFFSET               0xC400000
#define TLMM_GPIO_CFGN_E_LENGTH             4
#define TLMM_GPIO_CFGN_E_OFFSET             0x1000
#define TLMM_GPIO_CFGN_NR_E                 80

#define GPIO_HIHYS_EN
#define TLMM_GPIO_CFGN_GPIO_HIHYS_EN_BOFFSET        10
#define TLMM_GPIO_CFGN_GPIO_HIHYS_EN_BLEN           1
#define TLMM_GPIO_CFGN_GPIO_HIHYS_EN_FLAG           HSL_RW

#define GPIO_OE
#define TLMM_GPIO_CFGN_GPIO_OE_BOFFSET              9
#define TLMM_GPIO_CFGN_GPIO_OE_BLEN                 1
#define TLMM_GPIO_CFGN_GPIO_OE_FLAG                 HSL_RW

#define DRV_STRENGTH
#define TLMM_GPIO_CFGN_DRV_STRENGTH_BOFFSET         6
#define TLMM_GPIO_CFGN_DRV_STRENGTH_BLEN            3
#define TLMM_GPIO_CFGN_DRV_STRENGTH_FLAG            HSL_RW

enum MHT_TLMM_GPIO_CFGN_DRV_STRENGTH {
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_2_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_4_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_6_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_8_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_10_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_12_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_14_MA,
    MHT_TLMM_GPIO_CFGN_DRV_STRENGTH_16_MA,
};

#define FUNC_SEL
#define TLMM_GPIO_CFGN_FUNC_SEL_BOFFSET             2
#define TLMM_GPIO_CFGN_FUNC_SEL_BLEN                4
#define TLMM_GPIO_CFGN_FUNC_SEL_FLAG                HSL_RW

#define GPIO_PULL
#define TLMM_GPIO_CFGN_GPIO_PULL_BOFFSET            0
#define TLMM_GPIO_CFGN_GPIO_PULL_BLEN               2
#define TLMM_GPIO_CFGN_GPIO_PULL_FLAG               HSL_RW

enum MHT_PIN_CONFIG_PARAM {
    MHT_TLMM_GPIO_CFGN_GPIO_PULL_DISABLE,   //Disables all pull
    MHT_TLMM_GPIO_CFGN_GPIO_PULL_DOWN,
    MHT_TLMM_GPIO_CFGN_GPIO_PULL_BUS_HOLD,  //Weak Keepers
    MHT_TLMM_GPIO_CFGN_GPIO_PULL_UP,
};


/* TLMM_GPIO_IN_OUTn */
#define TLMM_GPIO_IN_OUTN
#define TLMM_GPIO_IN_OUTN_OFFSET            0xC400004
#define TLMM_GPIO_IN_OUTN_E_LENGTH          4
#define TLMM_GPIO_IN_OUTN_E_OFFSET          0x1000
#define TLMM_GPIO_IN_OUTN_NR_E              80

#define GPIO_OUT
#define TLMM_GPIO_IN_OUTN_GPIO_OUT_BOFFSET          1
#define TLMM_GPIO_IN_OUTN_GPIO_OUT_BLEN             1
#define TLMM_GPIO_IN_OUTN_GPIO_OUT_FLAG             HSL_RW

#define GPIO_IN
#define TLMM_GPIO_IN_OUTN_GPIO_IN_BOFFSET           0
#define TLMM_GPIO_IN_OUTN_GPIO_IN_BLEN              1
#define TLMM_GPIO_IN_OUTN_GPIO_IN_FLAG              HSL_R

/* TLMM_CLK_GATE_EN */
#define TLMM_CLK_GATE_EN
#define TLMM_CLK_GATE_EN_OFFSET             0xC500000
#define TLMM_CLK_GATE_EN_E_LENGTH           4
#define TLMM_CLK_GATE_EN_E_OFFSET           0
#define TLMM_CLK_GATE_EN_NR_E               1

#define AHB_HCLK_EN
#define TLMM_CLK_GATE_EN_AHB_HCLK_EN_BOFFSET        2
#define TLMM_CLK_GATE_EN_AHB_HCLK_EN_BLEN           1
#define TLMM_CLK_GATE_EN_AHB_HCLK_EN_FLAG           HSL_RW

#define SUMMARY_INTR_EN
#define TLMM_CLK_GATE_EN_SUMMARY_INTR_EN_BOFFSET    1
#define TLMM_CLK_GATE_EN_SUMMARY_INTR_EN_BLEN       1
#define TLMM_CLK_GATE_EN_SUMMARY_INTR_EN_FLAG       HSL_RW

#define CRIF_READ_EN
#define TLMM_CLK_GATE_EN_CRIF_READ_EN_BOFFSET       0
#define TLMM_CLK_GATE_EN_CRIF_READ_EN_BLEN          1
#define TLMM_CLK_GATE_EN_CRIF_READ_EN_FLAG          HSL_RW

/* TLMM_HW_REVISION_NUMBER */
#define TLMM_HW_REVISION_NUMBER
#define TLMM_HW_REVISION_NUMBER_OFFSET      0xC510010
#define TLMM_HW_REVISION_NUMBER_E_LENGTH    4
#define TLMM_HW_REVISION_NUMBER_E_OFFSET    0
#define TLMM_HW_REVISION_NUMBER_NR_E        1

#define VERSION_ID
#define TLMM_HW_REVISION_NUMBER_VERSION_ID_BOFFSET  28
#define TLMM_HW_REVISION_NUMBER_VERSION_ID_BLEN     4
#define TLMM_HW_REVISION_NUMBER_VERSION_ID_FLAG     HSL_R

#define PARTNUM
#define TLMM_HW_REVISION_NUMBER_PARTNUM_BOFFSET     12
#define TLMM_HW_REVISION_NUMBER_PARTNUM_BLEN        16
#define TLMM_HW_REVISION_NUMBER_PARTNUM_FLAG        HSL_R

#define MFG_ID
#define TLMM_HW_REVISION_NUMBER_MFG_ID_BOFFSET      1
#define TLMM_HW_REVISION_NUMBER_MFG_ID_BLEN         11
#define TLMM_HW_REVISION_NUMBER_MFG_ID_FLAG         HSL_R

#define START_BIT
#define TLMM_HW_REVISION_NUMBER_START_BIT_BOFFSET   0
#define TLMM_HW_REVISION_NUMBER_START_BIT_BLEN      1
#define TLMM_HW_REVISION_NUMBER_START_BIT_FLAG      HSL_R


/****************************************************************************
 *
 *  2) PINs Functions Selection  GPIO_CFG[5:2] (FUNC_SEL)
 *
 ****************************************************************************/
/*GPIO*/
#define MHT_PIN_FUNC_GPIO0  0
#define MHT_PIN_FUNC_GPIO1  0
#define MHT_PIN_FUNC_GPIO2  0
#define MHT_PIN_FUNC_GPIO3  0
#define MHT_PIN_FUNC_GPIO4  0
#define MHT_PIN_FUNC_GPIO5  0
#define MHT_PIN_FUNC_GPIO6  0
#define MHT_PIN_FUNC_GPIO7  0
#define MHT_PIN_FUNC_GPIO8  0
#define MHT_PIN_FUNC_GPIO9  0
#define MHT_PIN_FUNC_GPIO10 0
#define MHT_PIN_FUNC_GPIO11 0
#define MHT_PIN_FUNC_GPIO12 0
#define MHT_PIN_FUNC_GPIO13 0
#define MHT_PIN_FUNC_GPIO14 0
#define MHT_PIN_FUNC_GPIO15 0
#define MHT_PIN_FUNC_GPIO16 0
#define MHT_PIN_FUNC_GPIO17 0
#define MHT_PIN_FUNC_GPIO18 0
#define MHT_PIN_FUNC_GPIO19 0
#define MHT_PIN_FUNC_GPIO20 0
#define MHT_PIN_FUNC_GPIO21 0

/*MINIMUM CONCURRENCY SET FUNCTION*/
#define MHT_PIN_FUNC_INTN_WOL         1
#define MHT_PIN_FUNC_INTN             1
#define MHT_PIN_FUNC_P0_LED_0         1
#define MHT_PIN_FUNC_P1_LED_0         1
#define MHT_PIN_FUNC_P2_LED_0         1
#define MHT_PIN_FUNC_P3_LED_0         1
#define MHT_PIN_FUNC_PPS_IN           1
#define MHT_PIN_FUNC_TOD_IN           1
#define MHT_PIN_FUNC_RTC_REFCLK_IN    1
#define MHT_PIN_FUNC_P0_PPS_OUT       1
#define MHT_PIN_FUNC_P1_PPS_OUT       1
#define MHT_PIN_FUNC_P2_PPS_OUT       1
#define MHT_PIN_FUNC_P3_PPS_OUT       1
#define MHT_PIN_FUNC_P0_TOD_OUT       1
#define MHT_PIN_FUNC_P0_CLK125_TDI    1
#define MHT_PIN_FUNC_P0_SYNC_CLKO_PTP 1
#define MHT_PIN_FUNC_P0_LED_1         1
#define MHT_PIN_FUNC_P1_LED_1         1
#define MHT_PIN_FUNC_P2_LED_1         1
#define MHT_PIN_FUNC_P3_LED_1         1
#define MHT_PIN_FUNC_MDC_M            1
#define MHT_PIN_FUNC_MDO_M            1

/*ALT FUNCTION K*/
#define MHT_PIN_FUNC_EVENT_TRG_I        2
#define MHT_PIN_FUNC_P0_EVENT_TRG_O     2
#define MHT_PIN_FUNC_P1_EVENT_TRG_O     2
#define MHT_PIN_FUNC_P2_EVENT_TRG_O     2
#define MHT_PIN_FUNC_P3_EVENT_TRG_O     2
#define MHT_PIN_FUNC_P1_TOD_OUT         2
#define MHT_PIN_FUNC_P1_CLK125_TDI      2
#define MHT_PIN_FUNC_P1_SYNC_CLKO_PTP   2
#define MHT_PIN_FUNC_P0_INTN_WOL        2
#define MHT_PIN_FUNC_P1_INTN_WOL        2
#define MHT_PIN_FUNC_P2_INTN_WOL        2
#define MHT_PIN_FUNC_P3_INTN_WOL        2

/*ALT FUNCTION L*/
#define MHT_PIN_FUNC_P2_TOD_OUT         3
#define MHT_PIN_FUNC_P2_CLK125_TDI      3
#define MHT_PIN_FUNC_P2_SYNC_CLKO_PTP   3

/*ALT FUNCTION M*/
#define MHT_PIN_FUNC_P3_TOD_OUT         4
#define MHT_PIN_FUNC_P3_CLK125_TDI      4
#define MHT_PIN_FUNC_P3_SYNC_CLKO_PTP   4

/*ALT FUNCTION N*/
#define MHT_PIN_FUNC_P0_LED_2           3
#define MHT_PIN_FUNC_P1_LED_2           2
#define MHT_PIN_FUNC_P2_LED_2           2
#define MHT_PIN_FUNC_P3_LED_2           3

/*ALT FUNCTION O*/


/*ALT FUNCTION DEBUG BUS OUT*/
#define MHT_PIN_FUNC_DBG_OUT_CLK        2
#define MHT_PIN_FUNC_DBG_BUS_OUT0       2
#define MHT_PIN_FUNC_DBG_BUS_OUT1       2
#define MHT_PIN_FUNC_DBG_BUS_OUT12      2
#define MHT_PIN_FUNC_DBG_BUS_OUT13      2
#define MHT_PIN_FUNC_DBG_BUS_OUT2       3
#define MHT_PIN_FUNC_DBG_BUS_OUT3       4
#define MHT_PIN_FUNC_DBG_BUS_OUT4       3
#define MHT_PIN_FUNC_DBG_BUS_OUT5       3
#define MHT_PIN_FUNC_DBG_BUS_OUT6       3
#define MHT_PIN_FUNC_DBG_BUS_OUT7       5
#define MHT_PIN_FUNC_DBG_BUS_OUT8       5
#define MHT_PIN_FUNC_DBG_BUS_OUT9       5
#define MHT_PIN_FUNC_DBG_BUS_OUT10      3
#define MHT_PIN_FUNC_DBG_BUS_OUT11      3
#define MHT_PIN_FUNC_DBG_BUS_OUT14      2
#define MHT_PIN_FUNC_DBG_BUS_OUT15      2


/****************************************************************************
 *
 *  2) PINs Functions Selection  GPIO_CFG[5:2] (FUNC_SEL)
 *
 ****************************************************************************/
struct mht_pinctrl_setting_mux {
	a_uint32_t pin;
	a_uint32_t func;
};

struct mht_pinctrl_setting_configs {
	a_uint32_t pin;
	a_uint32_t num_configs;
	a_ulong_t *configs;
};

struct mht_pinctrl_setting {
	enum pinctrl_map_type type;
	union {
		struct mht_pinctrl_setting_mux mux;
		struct mht_pinctrl_setting_configs configs;
	} data;
};

#define MHT_PIN_SETTING_MUX(pin_id, function)		\
	{								\
		.type = PIN_MAP_TYPE_MUX_GROUP,				\
		.data.mux = {						\
			.pin = pin_id,					\
			.func = function				\
		},							\
	}

#define MHT_PIN_SETTING_CONFIG(pin_id, cfgs)		\
	{								\
		.type = PIN_MAP_TYPE_CONFIGS_PIN,				\
		.data.configs = {						\
			.pin = pin_id,					\
			.configs = cfgs,				\
			.num_configs = ARRAY_SIZE(cfgs)				\
		},							\
	}

sw_error_t mht_gpio_set_bit(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t value);
sw_error_t mht_gpio_get_bit(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t *data);
sw_error_t mht_gpio_pin_mux_set(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t func);
sw_error_t mht_gpio_pin_cfg_set_bias(a_uint32_t dev_id, a_uint32_t pin, enum pin_config_param bias);
sw_error_t mht_gpio_pin_cfg_get_bias(a_uint32_t dev_id, a_uint32_t pin, enum pin_config_param *bias);
sw_error_t mht_gpio_pin_cfg_set_drvs(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t drvs);
sw_error_t mht_gpio_pin_cfg_get_drvs(a_uint32_t dev_id, a_uint32_t pin, a_uint32_t *drvs);
sw_error_t mht_gpio_pin_cfg_set_oe(a_uint32_t dev_id, a_uint32_t pin, a_bool_t oe);
sw_error_t mht_gpio_pin_cfg_get_oe(a_uint32_t dev_id, a_uint32_t pin, a_bool_t *oe);
sw_error_t mht_gpio_pin_cfg_set_hihys(a_uint32_t dev_id, a_uint32_t pin, a_bool_t hihys_en);
sw_error_t mht_gpio_pin_cfg_get_hihys(a_uint32_t dev_id, a_uint32_t pin, a_bool_t *hihys);
sw_error_t ssdk_mht_pinctrl_init(a_uint32_t dev_id);


#ifdef __cplusplus
}
#endif                          /* __cplusplus */
#endif                          /* _SSDK_MHT_PINCTRL_H_ */
