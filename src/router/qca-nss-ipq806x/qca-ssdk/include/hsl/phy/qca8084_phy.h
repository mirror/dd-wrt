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
 /*MII register*/
#define QCA8084_PHY_FIFO_CONTROL                                         0x19
/*MII register field*/
#define QCA8084_PHY_FIFO_RESET                                           0x3
/*MMD1 register*/
#define QCA8084_PHY_MMD1_NUM                                             0x1
#define QCA8084_PHY_MMD1_MSE_THRESH_DEBUG_12                             0x800a
/*MMD1 register field*/
#define QCA8084_PHY_MMD1_MSE_THRESH_ENERGY_DETECT                        0x51c6
/*MMD3 register*/
#define QCA8084_PHY_MMD3_NUM                                             0x3
#define QCA8084_PHY_MMD3_ADDR_8023AZ_EEE_2500M_CAPABILITY                0x15
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL3                                0x8074
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL4                                0x8075
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL5                                0x8076
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL6                                0x8077
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL7                                0x8078
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL9                                0x807a
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL13                               0x807e
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL14                               0x807f
/*MMD3 register field*/
#define QCA8084_PHY_EEE_CAPABILITY_2500M                                 0x1
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL3_VAL                            0xc040
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL4_VAL                            0xa060
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL5_VAL                            0xc040
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL6_VAL                            0xa060
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL7_VAL                            0xc050
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL9_VAL                            0xc060
#define QCA8084_PHY_MMD3_CDT_THRESH_CTRL13_VAL                           0xb060
#define QCA8084_PHY_MMD3_NEAR_ECHO_THRESH_VAL                            0x1eb0
/*MMD7 register*/
#define QCA8084_PHY_MMD7_NUM                                             0x7
#define QCA8084_PHY_MMD7_ADDR_8023AZ_EEE_2500M_CTRL                      0x3e
#define QCA8084_PHY_MMD7_ADDR_8023AZ_EEE_2500M_PARTNER                   0x3f
#define QCA8084_PHY_MMD7_IPG_10_11_ENABLE                                0x901d
/*MMD7 register field*/
#define QCA8084_PHY_8023AZ_EEE_2500BT                                    0x1
#define QCA8084_PHY_MMD7_IPG_10_EN                                       0
#define QCA8084_PHY_MMD7_IPG_11_EN                                       0x1

/*DEBUG port register*/
#define QCA8084_PHY_CONTROL_DEBUG_REGISTER0                              0x1f
#define QCA8084_PHY_1588_P2_EN                                           0x2

/*DEBUG port analog register*/
#define QCA8084_PHY_DEBUG_ANA_INTERFACE_CLK_SEL                          0x8b80
#define QCA8084_PHY_AFE25_CMN_6_MII                                      0x380
#define QCA8084_PHY_AFE25_PLL_EN                                         0x8000
#define QCA8084_PHY_DEBUG_ANA_ICC                                        0x280
#define QCA8084_PHY_DEBUG_ANA_ICC_MASK                                   0x1f
#define QCA8084_PHY_AFE25_CMN_2_MII                                      0x180
#define QCA8084_PHY_AFE25_LDO_EN                                         0x2000

typedef enum {
	ADC_RISING = 0,
	ADC_FALLING = 0xf0,
}
qca8084_adc_edge_t;

sw_error_t
qca8084_phy_interface_set_mode(a_uint32_t dev_id, a_uint32_t phy_id,
	fal_port_interface_mode_t interface_mode);

sw_error_t
qca8084_phy_set_8023az(a_uint32_t dev_id, a_uint32_t phy_id, a_bool_t enable);

sw_error_t
qca8084_phy_get_8023az(a_uint32_t dev_id, a_uint32_t phy_id, a_bool_t * enable);

sw_error_t
qca8084_phy_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t adv);

sw_error_t
qca8084_phy_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t *adv);

sw_error_t
qca8084_phy_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t *adv);

sw_error_t
qca8084_phy_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t *cap);

sw_error_t
qca8084_phy_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_id,
	a_uint32_t *status);

sw_error_t
qca8084_phy_ipg_config(a_uint32_t dev_id, a_uint32_t phy_id,
	fal_port_speed_t speed);

sw_error_t
qca8084_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_id,
	fal_port_interface_mode_t *interface_mode_status);

sw_error_t
qca8084_phy_hw_init(a_uint32_t dev_id,  a_uint32_t port_id);

sw_error_t
qca8084_phy_speed_fixup(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct port_phy_status *phy_status);
sw_error_t
qca8084_phy_fifo_reset(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable);
sw_error_t
qca8084_phy_fixup_ability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *ability);
sw_error_t
qca8084_phy_pll_on(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qca8084_phy_pll_off(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qca8084_phy_ldo_set(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable);
