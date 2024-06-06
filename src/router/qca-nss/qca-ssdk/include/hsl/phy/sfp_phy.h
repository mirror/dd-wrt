/*
 * Copyright (c) 2018, 2020-2021, The Linux Foundation. All rights reserved.
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

#ifndef _SFP_PHY_H_
#define _SFP_PHY_H_

#ifdef __cplusplus
extern "C"
{
#endif				/* __cplusplus */


#define SFP_ANEG_DONE                  0x20
#define SFP_E2PROM_ADDR                0x50
#define SFP_SPEED_ADDR                 0xc
#define SFP_SPEED_1000M                10
#define SFP_SPEED_2500M                25
#define SFP_SPEED_5000M                50
#define SFP_SPEED_10000M               100
#define SFP_TYPE_ADDR                  0x6
#define SFP_TYPE_1000MBASE_T           0x08
#define SFP_E2PROM_EXTEND_ADDR         0x56
#define SFP_E2PROM_PART_NUM_OFFSET     138
#define SFP_EXTEND_USXGMII_OFFSET      130
#define SFP_EXTEND_LINK_OFFSET         160
#define SFP_EXTEND_SPEED_OFFSET        163
#define SFP_PHY_SPEED_10000M           0x0
#define SFP_PHY_SPEED_5000M            0x1
#define SFP_PHY_SPEED_2500M            0x2
#define SFP_PHY_SPEED_1000M            0x3
#define SFP_PHY_SPEED_100M             0x4

#define SFP_TO_SFP_SPEED(reg_data) ((reg_data >> 8) & 0xff)
#define SFP_TO_SFP_TYPE(reg_data) ((reg_data >> 8) & 0xff)

int sfp_phy_device_setup(a_uint32_t dev_id, a_uint32_t port_id, a_uint32_t phy_id,
	void *priv);
void sfp_phy_device_remove(a_uint32_t dev_id, a_uint32_t port);
int sfp_phy_driver_register(void);
void sfp_phy_driver_unregister(void);

int sfp_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp);
void sfp_phy_exit(a_uint32_t dev_id, a_uint32_t port_bmp);

sw_error_t sfp_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_id,
	fal_port_interface_mode_t *interface_mode);
sw_error_t
sfp_phy_phydev_adv_update(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t adv_mask,
	a_uint32_t adv);
sw_error_t
sfp_phy_rx_los_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t *rx_los_status);
sw_error_t
sfp_phy_tx_dis_status_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t tx_dis_status);
sw_error_t
sfp_phy_tx_dis_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t *tx_dis_status);
sw_error_t
sfp_phy_mod_present_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t *mod_present_status);
sw_error_t
sfp_port_status_get_from_uniphy(a_uint32_t dev_id, a_uint32_t port_id,
	struct port_phy_status *phy_status);
sw_error_t
sfp_phy_medium_status_set(a_uint32_t dev_id, a_uint32_t port_id,
	a_bool_t sfp_medium_status);
sw_error_t
sfp_phy_port_status_get(a_uint32_t dev_id, a_uint32_t port_id,
	struct port_phy_status *phy_status);
#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _SFP_PHY_H_ */

