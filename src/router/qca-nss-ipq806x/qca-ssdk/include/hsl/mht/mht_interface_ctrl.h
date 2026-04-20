/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#define MHT_UNIPHY_SGMII_0                                               0
#define MHT_UNIPHY_SGMII_1                                               1
#define MHT_UNIPHY_XPCS                                                  2
/*UNIPHY MII registers*/
#define MHT_UNIPHY_PLL_POWER_ON_AND_RESET                                0
#define MHT_UNIPHY_PLL_LOOP_CONTROL                                      6

/*UNIPHY MII register field*/
#define MHT_UNIPHY_ANA_SOFT_RESET                                        0
#define MHT_UNIPHY_ANA_SOFT_RELEASE                                      0x40
#define MHT_UPHY_PLL_CML2CMS_IBSEL                                       0x30

/*UNIPHY MMD*/
#define MHT_UNIPHY_MMD1                                                  0x1
#define MHT_UNIPHY_MMD3                                                  0x3
#define MHT_UNIPHY_MMD26                                                 0x1a
#define MHT_UNIPHY_MMD27                                                 0x1b
#define MHT_UNIPHY_MMD28                                                 0x1c
#define MHT_UNIPHY_MMD31                                                 0x1f

/*UNIPHY MMD1 registers*/
#define MHT_UNIPHY_MMD1_CDA_CONTROL1                                     0x20
#define MHT_UNIPHY_MMD1_CALIBRATION4                                     0x78
#define MHT_UNIPHY_MMD1_BYPASS_TUNING_IPG                                0x189
#define MHT_UNIPHY_MMD1_MODE_CTRL                                        0x11b
#define MHT_UNIPHY_MMD1_CHANNEL0_CFG                                     0x120
#define MHT_UNIPHY_MMD1_GMII_DATAPASS_SEL                                0x180
#define MHT_UNIPHY_MMD1_USXGMII_RESET                                    0x18c

/*UNIPHY MMD1 register field*/
#define MHT_UNIPHY_MMD1_BYPASS_TUNING_IPG_EN                             0x0fff
#define MHT_UNIPHY_MMD1_XPCS_MODE                                        0x1000
#define MHT_UNIPHY_MMD1_SGMII_MODE                                       0x400
#define MHT_UNIPHY_MMD1_SGMII_PLUS_MODE                                  0x800
#define MHT_UNIPHY_MMD1_1000BASE_X                                       0x0
#define MHT_UNIPHY_MMD1_SGMII_PHY_MODE                                   0x10
#define MHT_UNIPHY_MMD1_SGMII_MAC_MODE                                   0x20
#define MHT_UNIPHY_MMD1_SGMII_MODE_CTRL_MASK                             0x1f70
#define MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_MASK                             0xe
#define MHT_UNIPHY_MMD1_CH0_AUTONEG_ENABLE                               0x0
#define MHT_UNIPHY_MMD1_CH0_FORCE_ENABLE                                 0x8
#define MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_1G                               0x4
#define MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_100M                             0x2
#define MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_10M                              0x0
#define MHT_UNIPHY_MMD1_DATAPASS_MASK                                    0x1
#define MHT_UNIPHY_MMD1_DATAPASS_USXGMII                                 0x1
#define MHT_UNIPHY_MMD1_DATAPASS_SGMII                                   0x0
#define MHT_UNIPHY_MMD1_CALIBRATION_DONE                                 0x80
#define MHT_UNIPHY_MMD1_SGMII_FUNC_RESET                                 0x10
#define MHT_UNIPHY_MMD1_SGMII_ADPT_RESET                                 0x800
#define MHT_UNIPHY_MMD1_SSCG_ENABLE                                      0x8

/*UNIPHY MMD3 registers*/
#define MHT_UNIPHY_MMD3_PCS_CTRL2                                        0x7
#define MHT_UNIPHY_MMD3_AN_LP_BASE_ABL2                                  0x14
#define MHT_UNIPHY_MMD3_10GBASE_R_PCS_STATUS1                            0x20
#define MHT_UNIPHY_MMD3_DIG_CTRL1                                        0x8000
#define MHT_UNIPHY_MMD3_EEE_MODE_CTRL                                    0x8006
#define MHT_UNIPHY_MMD3_VR_RPCS_TPC                                      0x8007
#define MHT_UNIPHY_MMD3_EEE_TX_TIMER                                     0x8008
#define MHT_UNIPHY_MMD3_EEE_RX_TIMER                                     0x8009
#define MHT_UNIPHY_MMD3_MII_AM_INTERVAL                                  0x800a
#define MHT_UNIPHY_MMD3_EEE_MODE_CTRL1                                   0x800b

/*UNIPHY MMD3 register field*/
#define MHT_UNIPHY_MMD3_PCS_TYPE_10GBASE_R                               0
#define MHT_UNIPHY_MMD3_10GBASE_R_UP                                     0x1000
#define MHT_UNIPHY_MMD3_USXGMII_EN                                       0x200
#define MHT_UNIPHY_MMD3_QXGMII_EN                                        0x1400
#define MHT_UNIPHY_MMD3_MII_AM_INTERVAL_VAL                              0x6018
#define MHT_UNIPHY_MMD3_XPCS_SOFT_RESET                                  0x8000
#define MHT_UNIPHY_MMD3_XPCS_EEE_CAP                                     0x40
#define MHT_UNIPHY_MMD3_EEE_RES_REGS                                     0x100
#define MHT_UNIPHY_MMD3_EEE_SIGN_BIT_REGS                                0x40
#define MHT_UNIPHY_MMD3_EEE_EN                                           0x3
#define MHT_UNIPHY_MMD3_EEE_TSL_REGS                                     0xa
#define MHT_UNIPHY_MMD3_EEE_TLU_REGS                                     0xc0
#define MHT_UNIPHY_MMD3_EEE_TWL_REGS                                     0x1600
#define MHT_UNIPHY_MMD3_EEE_100US_REG_REGS                               0xc8
#define MHT_UNIPHY_MMD3_EEE_RWR_REG_REGS                                 0x1c00
#define MHT_UNIPHY_MMD3_EEE_TRANS_LPI_MODE                               0x1
#define MHT_UNIPHY_MMD3_EEE_TRANS_RX_LPI_MODE                            0x100
#define MHT_UNIPHY_MMD3_USXG_FIFO_RESET                                  0x400

/*UNIPHY MMD26 27 28 31 registers*/
#define MHT_UNIPHY_MMD_MII_CTRL                                          0
#define MHT_UNIPHY_MMD_MII_DIG_CTRL                                      0x8000
#define MHT_UNIPHY_MMD_MII_AN_INT_MSK                                    0x8001
#define MHT_UNIPHY_MMD_MII_ERR_SEL                                       0x8002
#define MHT_UNIPHY_MMD_MII_XAUI_MODE_CTRL                                0x8004

/*UNIPHY MMD26 27 28 31 register field*/
#define MHT_UNIPHY_MMD_AN_COMPLETE_INT                                   0x1
#define MHT_UNIPHY_MMD_MII_4BITS_CTRL                                    0x0
#define MHT_UNIPHY_MMD_TX_CONFIG_CTRL                                    0x8
#define MHT_UNIPHY_MMD_MII_AN_ENABLE                                     0x1000
#define MHT_UNIPHY_MMD_MII_AN_RESTART                                    0x200
#define MHT_UNIPHY_MMD_MII_AN_COMPLETE_INT                               0x1
#define MHT_UNIPHY_MMD_USXG_FIFO_RESET                                   0x20
#define MHT_UNIPHY_MMD_XPC_SPEED_MASK                                    0x2060
#define MHT_UNIPHY_MMD_XPC_SPEED_2500                                    0x20
#define MHT_UNIPHY_MMD_XPC_SPEED_1000                                    0x40
#define MHT_UNIPHY_MMD_XPC_SPEED_100                                     0x2000
#define MHT_UNIPHY_MMD_XPC_SPEED_10                                      0
#define MHT_UNIPHY_MMD_TX_IPG_CHECK_DISABLE                              0x1
#define MHT_UNIPHY_MMD_PHY_MODE_CTRL_EN                                  0x1

typedef enum {
	MHT_UNIPHY_MAC = MHT_UNIPHY_MMD1_SGMII_MAC_MODE,
	MHT_UNIPHY_PHY = MHT_UNIPHY_MMD1_SGMII_PHY_MODE,
	MHT_UNIPHY_SGMII = MHT_UNIPHY_MMD1_SGMII_MODE,
	MHT_UNIPHY_SGMII_PLUS = MHT_UNIPHY_MMD1_SGMII_PLUS_MODE,
	MHT_UNIPHY_UQXGMII = MHT_UNIPHY_MMD1_XPCS_MODE,
}mht_uniphy_mode_t;

a_bool_t mht_uniphy_mode_check(a_uint32_t dev_id, a_uint32_t uniphy_index,
	mht_uniphy_mode_t uniphy_mode);

sw_error_t
mht_uniphy_xpcs_autoneg_restart(a_uint32_t dev_id, a_uint32_t port_id);
#if 0
sw_error_t
mht_uniphy_xpcs_speed_set(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_speed_t speed);
#endif
sw_error_t
mht_uniphy_uqxgmii_function_reset(a_uint32_t dev_id, a_uint32_t port_id);

sw_error_t
mht_interface_uqxgmii_mode_set(a_uint32_t dev_id);

sw_error_t
mht_uniphy_sgmii_function_reset(a_uint32_t dev_id, a_uint32_t uniphy_index);

sw_error_t
mht_interface_sgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index,
	a_uint32_t mht_port_id, fal_mac_config_t *config);

sw_error_t
mht_port_speed_clock_set(a_uint32_t dev_id, a_uint32_t port_id,
	fal_port_speed_t speed);

sw_error_t
mht_interface_mac_mode_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_mac_config_t *config);
