/*
 * Copyright (c) 2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*qca808x_start*/
#ifndef _QCAPHY_COMMON_H_
#define _QCAPHY_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
#include "fal.h"

/*MII register*/
#define QCAPHY_CONTROL                                             0
#define QCAPHY_STATUS                                              1
#define QCAPHY_ID1                                                 2
#define QCAPHY_ID2                                                 3
#define QCAPHY_AUTONEG_ADVERT                                      4
#define QCAPHY_LINK_PARTNER_ABILITY                                5
#define QCAPHY_1000BASET_CONTROL                                   9
#define QCAPHY_1000BASET_STATUS                                    10
#define QCAPHY_EXTENDED_STATUS                                     15
#define QCAPHY_SPEC_CONTROL                                        16
#define QCAPHY_SPEC_STATUS                                         17

/*MII register field*/
#define QCAPHY_CTRL_SOFTWARE_RESET                                 0x8000
#define QCAPHY_COMMON_CTRL                                         0x1040
#define QCAPHY_10M_LOOPBACK                                        0x4100
#define QCAPHY_100M_LOOPBACK                                       0x6100
#define QCAPHY_1000M_LOOPBACK                                      0x4140
#define QCAPHY_LOCAL_LOOPBACK_ENABLE                               0x4000

#define QCAPHY_CTRL_AUTONEGOTIATION_ENABLE                         0x1000
#define QCAPHY_CTRL_RESTART_AUTONEGOTIATION                        0x0200
#define QCAPHY_CTRL_FULL_DUPLEX                                    0x0100
#define QCAPHY_CONTROL_SPEED_MASK                                  0x2040
#define QCAPHY_CONTROL_SPEED_100M                                  0x2000
#define QCAPHY_CONTROL_SPEED_10M                                   0
#define QCAPHY_CTRL_AUTONEGOTIATION_ENABLE                         0x1000
#define QCAPHY_CTRL_POWER_MASK                                     0x0800
#define QCAPHY_CTRL_POWER_DOWN                                     0x0800
#define QCAPHY_CTRL_POWER_UP                                       0x0
#define QCAPHY_CTRL_AUTONEGOTIATION_ENABLE                         0x1000
#define QCAPHY_STATUS_AUTONEG_CAPS                                 0x0008
#define QCAPHY_STATUS_10T_HD_CAPS                                  0x0800
#define QCAPHY_STATUS_10T_FD_CAPS                                  0x1000
#define QCAPHY_STATUS_100TX_HD_CAPS                                0x2000
#define QCAPHY_STATUS_100TX_FD_CAPS                                0x4000
#define QCAPHY_STATUS_1000T_FD_CAPS                                0x2000
#define QCAPHY_STATUS_EXTENDED_STATUS                              0x0100
#define QCAPHY_ADVERTISE_1000FULL                                  0x0200
#define QCAPHY_ADVERTISE_100FULL                                   0x0100
#define QCAPHY_ADVERTISE_100HALF                                   0x0080
#define QCAPHY_ADVERTISE_10FULL                                    0x0040
#define QCAPHY_ADVERTISE_10HALF                                    0x0020
#define QCAPHY_ADVERTISE_PAUSE                                     0x0400
#define QCAPHY_ADVERTISE_ASYM_PAUSE                                0x0800
#define QCAPHY_ADVERTISE_MEGA_ALL \
	(QCAPHY_ADVERTISE_10HALF | QCAPHY_ADVERTISE_10FULL | \
	QCAPHY_ADVERTISE_100HALF | QCAPHY_ADVERTISE_100FULL | \
	QCAPHY_ADVERTISE_PAUSE | QCAPHY_ADVERTISE_ASYM_PAUSE)
#define QCAPHY_LINK_10BASETX_HALF_DUPLEX                           0x0020
#define QCAPHY_LINK_10BASETX_FULL_DUPLEX                           0x0040
#define QCAPHY_LINK_100BASETX_HALF_DUPLEX                          0x0080
#define QCAPHY_LINK_100BASETX_FULL_DUPLEX                          0x0100
#define QCAPHY_LINK_1000BASETX_FULL_DUPLEX                         0x0800
#define QCAPHY_LINK_PAUSE                                          0x0400
#define QCAPHY_LINK_ASYPAUSE                                       0x0800
#define QCAPHY_LINK_LPACK                                          0x4000
#define QCAPHY_LINK_1000BASETX_FULL_DUPLEX                         0x0800
#define QCAPHY_MDI                                                 0
#define QCAPHY_MDIX                                                0x0020
#define QCAPHY_MDIX_AUTO                                           0x0060
#define QCAPHY_MDIX_STATUS                                         0x0040
#define QCAPHY_STATUS_LINK_PASS                                    0x0400
#define QCAPHY_STATUS_SPEED_MASK                                   0x380
#define QCAPHY_STATUS_SPEED_2500MBS                                0x200
#define QCAPHY_STATUS_SPEED_1000MBS                                0x100
#define QCAPHY_STATUS_SPEED_100MBS                                 0x80
#define QCAPHY_STATUS_SPEED_10MBS                                  0
#define QCAPHY_STATUS_SPEED_MASK_1                                 0xC000
#define QCAPHY_STATUS_SPEED_100MBS_1                               0x4000
#define QCAPHY_STATUS_SPEED_1000MBS_1                              0x8000
#define QCAPHY_STATUS_FULL_DUPLEX                                  0x2000
#define QCAPHY_RX_FLOWCTRL_STATUS                                  0x4
#define QCAPHY_TX_FLOWCTRL_STATUS                                  0x8

/*MMD number*/
#define QCAPHY_MMD1_NUM                                            0x1
#define QCAPHY_MMD3_NUM                                            0x3
#define QCAPHY_MMD7_NUM                                            0x7
#define QCAPHY_MMD31_NUM                                           0x1e
/*MMD register*/
#define QCAPHY_MMD7_8023AZ_EEE_CTRL                                0x3c
#define QCAPHY_MMD7_8023AZ_EEE_PARTNER                             0x3d
#define QCAPHY_MMD7_8023AZ_EEE_PARTNER1                            0x3e
#define QCAPHY_MMD7_8023AZ_EEE_CTRL1                               0x3f
#define QCAPHY_MMD7_8023AZ_EEE_STATUS                              0x8000
#define QCAPHY_MMD3_8023AZ_EEE_CAPABILITY                          0x14
#define QCAPHY_MMD3_8023AZ_EEE_CAPABILITY1                         0x15
#define QCAPHY_MMD7_LED_BLINK_FREQ_CTRL                            0x8073
#define QCAPHY_MMD7_LED_POLARITY_CTRL                              0x901a
#define QCAPHY_MMD7_LED_POLARITY_MASK                              0x40

/*MMD registers field*/
#define QCAPHY_EEE_MASK                                            0x000e
#define QCAPHY_EEE_ADV_100M                                        0x0002
#define QCAPHY_EEE_ADV_1000M                                       0x0004
#define QCAPHY_EEE_ADV_10000M                                      0x0008
#define QCAPHY_EEE_MASK1                                           0x0003
#define QCAPHY_EEE_ADV_2500M                                       0x0001
#define QCAPHY_EEE_ADV_5000M                                       0x0002
#define QCAPHY_EEE_PARTNER_ADV_100M                                0x0002
#define QCAPHY_EEE_PARTNER_ADV_1000M                               0x0004
#define QCAPHY_EEE_PARTNER_ADV_10000M                              0x0008
#define QCAPHY_EEE_PARTNER_ADV_2500M                               0x0001
#define QCAPHY_EEE_PARTNER_ADV_5000M                               0x0002
#define QCAPHY_EEE_CAPABILITY_100M                                 0x0002
#define QCAPHY_EEE_CAPABILITY_1000M                                0x0004
#define QCAPHY_EEE_CAPABILITY_10000M                               0x0008
#define QCAPHY_EEE_CAPABILITY_2500M                                0x0001
#define QCAPHY_EEE_CAPABILITY_5000M                                0x0002
#define QCAPHY_EEE_STATUS_100M                                     0x0002
#define QCAPHY_EEE_STATUS_1000M                                    0x0004
#define QCAPHY_PHY_LINK_1000M_LIGHT_EN                             0x40
#define QCAPHY_PHY_LINK_100M_LIGHT_EN                              0x20
#define QCAPHY_PHY_LINK_10M_LIGHT_EN                               0x10
#define QCAPHY_PHY_RX_TRAFFIC_BLINK_EN                             0x200
#define QCAPHY_PHY_TX_TRAFFIC_BLINK_EN                             0x400
#define QCAPHY_PHY_MAP_BLINK_FREQ_MASK                             0xe00
#define QCAPHY_PHY_ALWAYS_BLINK_FREQ_MASK                          0x38
#define QCAPHY_PHY_BLINK_FREQ_2HZ                                  0x0
#define QCAPHY_PHY_BLINK_FREQ_4HZ                                  0x200
#define QCAPHY_PHY_BLINK_FREQ_8HZ                                  0x400
#define QCAPHY_PHY_BLINK_FREQ_16HZ                                 0x600
#define QCAPHY_PHY_BLINK_FREQ_32HZ                                 0x800
#define QCAPHY_PHY_BLINK_FREQ_64HZ                                 0xa00
#define QCAPHY_PHY_BLINK_FREQ_128HZ                                0xc00
#define QCAPHY_PHY_BLINK_FREQ_256HZ                                0xe00

#define QCAPHY_PHY_LED_FORCE_EN                                    0x8000
#define QCAPHY_PHY_LED_FORCE_MASK                                  0x6000
#define QCAPHY_PHY_LED_FORCE_ALWAYS_OFF                            0
#define QCAPHY_PHY_LED_FORCE_ALWAYS_ON                             0x2000
#define QCAPHY_PHY_LED_FORCE_ALWAYS_BLINK                          0x6000

#define QCAPHY_LED_SOURCE0                                         0x0
#define QCAPHY_LED_SOURCE1                                         0x1
#define QCAPHY_LED_SOURCE2                                         0x2

sw_error_t
qcaphy_sw_reset(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_set_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable);
sw_error_t
qcaphy_get_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t *enable);
sw_error_t
qcaphy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed);
sw_error_t
qcaphy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t duplex);
sw_error_t
qcaphy_autoneg_enable(a_uint32_t dev_id, a_uint32_t phy_addr);
a_bool_t
qcaphy_autoneg_status(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_poweron(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_autoneg_restart(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_get_capability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap);
sw_error_t
qcaphy_get_phy_id(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *phy_id);
sw_error_t
qcaphy_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg_adv);
sw_error_t
qcaphy_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *autoneg_adv);
sw_error_t
qcaphy_lp_capability_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap);
sw_error_t
qcaphy_set_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t mode);
sw_error_t
qcaphy_get_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t * mode);
sw_error_t
qcaphy_get_mdix_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_status_t * mode);
sw_error_t
qcaphy_status_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct port_phy_status *phy_status);
a_bool_t
qcaphy_get_link_status(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_get_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t * speed);
sw_error_t
qcaphy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t * duplex);

sw_error_t
qcaphy_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv);
sw_error_t
qcaphy_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv);
sw_error_t
qcaphy_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv);
sw_error_t
qcaphy_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap);
sw_error_t
qcaphy_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *status);
sw_error_t
qcaphy_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable);
sw_error_t
qcaphy_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable);
sw_error_t
qcaphy_led_pattern_map_from_phy(a_uint32_t dev_id, a_uint32_t *map,
	a_uint16_t phy_data);
sw_error_t
qcaphy_led_pattern_map_to_phy(a_uint32_t dev_id, a_uint32_t map,
	a_uint16_t *phy_data);
sw_error_t
qcaphy_led_pattern_force_to_phy(a_uint32_t dev_id, a_uint32_t force_mode,
	a_uint16_t *phy_data);
sw_error_t
qcaphy_led_pattern_force_from_phy(a_uint32_t dev_id, a_uint32_t *force_mode,
	a_uint16_t phy_data);
sw_error_t
qcaphy_led_blink_freq_set(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mode,
	a_uint32_t freq);
sw_error_t
qcaphy_led_blink_freq_get(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mode,
	a_uint32_t *freq);
sw_error_t
qcaphy_led_active_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t active_level);
sw_error_t
qcaphy_led_active_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *active_level);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _HSL_PHY_H_ */
/*qca808x_end*/
