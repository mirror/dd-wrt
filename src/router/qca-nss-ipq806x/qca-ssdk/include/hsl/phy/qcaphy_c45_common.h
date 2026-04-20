/*
 * Copyright (c) 2023-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef _QCAPHY_C45_COMMON_H_
#define _QCAPHY_C45_COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */
#include "qcaphy_common.h"

/*MMD register*/
#define QCAPHY_MMD7_AN_CONTROL1                                    0
#define QCAPHY_MMD7_AN_STATUS                                      0x1
#define QCAPHY_MMD7_AN_ADV                                         0x10
#define QCAPHY_MMD7_AN_10G_ADV                                     0x20
#define QCAPHY_MMD7_AN_LP_ADV                                      0x13
#define QCAPHY_MMD7_AN_LP_10G_ADV                                  0x21
#define QCAPHY_MMD1_PMA_CONTROL                                    0x0
#define QCAPHY_MMD1_PMA_TTYPE                                      0x7
#define QCAPHY_MMD31_CONTROL                                       0
/*MMD registers field*/
#define QCAPHY_AN_LINK_STATUS                                      0x4
#define QCAPHY_AN_AUTONEG_EN                                       0x1000
#define QCAPHY_AN_AUTONEG_RESTART                                  0x200
#define QCAPHY_AN_ADVERTISE_10HALF                                 0x20
#define QCAPHY_AN_ADVERTISE_10FULL                                 0x40
#define QCAPHY_AN_ADVERTISE_100HALF                                0x80
#define QCAPHY_AN_ADVERTISE_100FULL                                0x100
#define QCAPHY_AN_ADVERTISE_PAUSE                                  0x400
#define QCAPHY_AN_ADVERTISE_ASM_PAUSE                              0x800
#define QCAPHY_AN_MEGA_ADVERTISE    (QCAPHY_AN_ADVERTISE_10HALF |\
QCAPHY_AN_ADVERTISE_10FULL | QCAPHY_AN_ADVERTISE_100HALF | QCAPHY_AN_ADVERTISE_100FULL|\
QCAPHY_AN_ADVERTISE_PAUSE | QCAPHY_AN_ADVERTISE_ASM_PAUSE)
#define QCAPHY_AN_ADVERTISE_2500FULL                               0x80
#define QCAPHY_AN_ADVERTISE_5000FULL                               0x100
#define QCAPHY_AN_ADVERTISE_10000FULL                              0x1000
#define QCAPHY_AN_GIGA_PLUS_ALL     (QCAPHY_AN_ADVERTISE_2500FULL |\
QCAPHY_AN_ADVERTISE_5000FULL | QCAPHY_AN_ADVERTISE_10000FULL)
#define QCAPHY_AN_LP_ADVERTISE_10HALF                              0x20
#define QCAPHY_AN_LP_ADVERTISE_10FULL                              0x40
#define QCAPHY_AN_LP_ADVERTISE_100HALF                             0x80
#define QCAPHY_AN_LP_ADVERTISE_100FULL                             0x100
#define QCAPHY_AN_LP_ADVERTISE_PAUSE                               0x400
#define QCAPHY_AN_LP_ADVERTISE_ASM_PAUSE                           0x800
#define QCAPHY_AN_LP_ADVERTISE_2500FULL                            0x80
#define QCAPHY_AN_LP_ADVERTISE_5000FULL                            0x100
#define QCAPHY_AN_LP_ADVERTISE_10000FULL                           0x800
#define QCAPHY_PMA_SPEED_MASK                                      0x207c
#define QCAPHY_PMA_CONTROL_10000M                                  0x2040
#define QCAPHY_PMA_CONTROL_5000M                                   0x205c
#define QCAPHY_PMA_CONTROL_2500M                                   0x2058
#define QCAPHY_PMA_CONTROL_1000M                                   0x40
#define QCAPHY_PMA_CONTROL_100M                                    0x2000
#define QCAPHY_PMA_CONTROL_10M                                     0x0

#define QCAPHY_PMA_TYPE_MASK                                       0x3f
#define QCAPHY_PMA_TYPE_10000M                                     0x9
#define QCAPHY_PMA_TYPE_5000M                                      0x31
#define QCAPHY_PMA_TYPE_2500M                                      0x30
#define QCAPHY_PMA_TYPE_1000M                                      0xc
#define QCAPHY_PMA_TYPE_100M                                       0xe
#define QCAPHY_PMA_TYPE_10M                                        0xf

#define QCAPHY_POWER_DOWN                                          0x800
#define QCAPHY_CTRL_SOFTWARE_RESET                                 0x8000

a_bool_t
qcaphy_c45_autoneg_status(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_autoneg_restart(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_autoneg_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable);
sw_error_t
qcaphy_c45_autoneg_enable(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg);
sw_error_t
qcaphy_c45_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * autoneg);
sw_error_t
qcaphy_c45_get_partner_ability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * ability);
a_bool_t
qcaphy_c45_get_link_status(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_poweron(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_sw_reset(a_uint32_t dev_id, a_uint32_t phy_addr);
sw_error_t
qcaphy_c45_get_phy_id(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *phy_id);
sw_error_t
qcaphy_c45_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv);
sw_error_t
qcaphy_c45_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv);
sw_error_t
qcaphy_c45_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv);
sw_error_t
qcaphy_c45_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap);
sw_error_t
qcaphy_c45_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *status);
sw_error_t
qcaphy_c45_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable);
sw_error_t
qcaphy_c45_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable);
sw_error_t
qcaphy_c45_force_speed_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif
