/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HSL_PHY_H_
#define _HSL_PHY_H_

#ifdef __cplusplus
extern "C" {
#endif				/* __cplusplus */

#include "fal.h"

	typedef sw_error_t(*hsl_phy_init) (a_uint32_t dev_id,
					   a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_hibernation_set) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_bool_t enable);
	typedef sw_error_t(*hsl_phy_hibernation_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_speed_get) (a_uint32_t dev_id,
						a_uint32_t phy_id,
						fal_port_speed_t * speed);
	typedef sw_error_t(*hsl_phy_speed_set) (a_uint32_t dev_id,
						a_uint32_t phy_id,
						fal_port_speed_t speed);
	typedef sw_error_t(*hsl_phy_duplex_get) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 fal_port_duplex_t * duplex);
	typedef sw_error_t(*hsl_phy_duplex_set) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 fal_port_duplex_t duplex);
	typedef sw_error_t(*hsl_phy_autoneg_enable_set) (a_uint32_t dev_id,
							 a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_autoneg_enable_get) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_restart_autoneg) (a_uint32_t dev_id,
						      a_uint32_t phy_id);
	typedef a_bool_t(*hsl_phy_autoneg_status_get) (a_uint32_t dev_id,
							 a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_powersave_set) (a_uint32_t dev_id,
						    a_uint32_t phy_id,
						    a_bool_t enable);
	typedef sw_error_t(*hsl_phy_powersave_get) (a_uint32_t dev_id,
						    a_uint32_t phy_id,
						    a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_cdt) (a_uint32_t dev_id, a_uint32_t phy_id,
					  a_uint32_t mdi_pair,
					  fal_cable_status_t * cable_status,
					  a_uint32_t * cable_len);
	typedef a_bool_t(*hsl_phy_link_status_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_get_ability) (a_uint32_t dev_id,
						  a_uint32_t phy_id,
						  a_uint32_t * ability);
	typedef sw_error_t(*hsl_phy_mdix_set) (a_uint32_t dev_id,
					       a_uint32_t phy_id,
					       fal_port_mdix_mode_t mode);
	typedef sw_error_t(*hsl_phy_mdix_get) (a_uint32_t dev_id,
					       a_uint32_t phy_id,
					       fal_port_mdix_mode_t * mode);
	typedef sw_error_t(*hsl_phy_mdix_status_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      fal_port_mdix_status_t *
						      mode);
	typedef sw_error_t(*hsl_phy_8023az_set) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 a_bool_t enable);
	typedef sw_error_t(*hsl_phy_8023az_get) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_local_loopback_set) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 a_bool_t enable);
	typedef sw_error_t(*hsl_phy_local_loopback_get) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_remote_loopback_set) (a_uint32_t dev_id,
							  a_uint32_t phy_id,
							  a_bool_t enable);
	typedef sw_error_t(*hsl_phy_remote_loopback_get) (a_uint32_t dev_id,
							  a_uint32_t phy_id,
							  a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_master_set) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 fal_port_master_t master);
	typedef sw_error_t(*hsl_phy_master_get) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 fal_port_master_t * master);
	typedef sw_error_t(*hsl_phy_combo_prefer_medium_set) (a_uint32_t dev_id,
							      a_uint32_t phy_id,
							      fal_port_medium_t
							      phy_medium);
	typedef sw_error_t(*hsl_phy_combo_prefer_medium_get) (a_uint32_t dev_id,
							      a_uint32_t phy_id,
							      fal_port_medium_t
							      * phy_medium);
	typedef sw_error_t(*hsl_phy_combo_medium_status_get) (a_uint32_t dev_id,
							      a_uint32_t phy_id,
							      fal_port_medium_t
							      * phy_medium);
	typedef sw_error_t(*hsl_phy_combo_fiber_mode_set) (a_uint32_t dev_id,
							   a_uint32_t phy_id,
							   fal_port_fiber_mode_t
							   fiber_mode);
	typedef sw_error_t(*hsl_phy_combo_fiber_mode_get) (a_uint32_t dev_id,
							   a_uint32_t phy_id,
							   fal_port_fiber_mode_t
							   * fiber_mode);
	typedef sw_error_t(*hsl_phy_reset) (a_uint32_t dev_id,
					    a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_reset_status_get) (a_uint32_t dev_id,
						       a_uint32_t phy_id,
						       fal_port_reset_status_t *
						       status);
	typedef sw_error_t(*hsl_phy_power_off) (a_uint32_t dev_id,
					    a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_power_on) (a_uint32_t dev_id,
					    a_uint32_t phy_id);
	typedef sw_error_t(*hsl_phy_id_get) (a_uint32_t dev_id,
					     a_uint32_t phy_id,
					     a_uint16_t * org_id,
					     a_uint16_t * rev_id);
	typedef sw_error_t(*hsl_phy_autoneg_adv_set) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_uint32_t autoneg);
	typedef sw_error_t(*hsl_phy_autoneg_adv_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_uint32_t * autoneg);
	typedef sw_error_t(*hsl_phy_reg_write) (a_uint32_t dev_id,
						a_uint32_t phy_addr,
						a_uint32_t reg,
						a_uint16_t reg_val);
	typedef a_uint16_t(*hsl_phy_reg_read) (a_uint32_t dev_id,
					       a_uint32_t phy_addr,
					       a_uint32_t reg);
	typedef sw_error_t(*hsl_phy_debug_write) (a_uint32_t dev_id,
						  a_uint32_t phy_id,
						  a_uint16_t reg_id,
						  a_uint16_t reg_val);
	typedef a_uint16_t(*hsl_phy_debug_read) (a_uint32_t dev_id,
						 a_uint32_t phy_id,
						 a_uint16_t reg_id);
	typedef sw_error_t(*hsl_phy_mmd_write) (a_uint32_t dev_id,
						a_uint32_t phy_id,
						a_uint16_t mmd_num,
						a_uint16_t reg_id,
						a_uint16_t reg_val);
	typedef a_uint16_t(*hsl_phy_mmd_read) (a_uint32_t dev_id,
					       a_uint32_t phy_id,
					       a_uint16_t mmd_num,
					       a_uint16_t reg_id);

	typedef sw_error_t(*hsl_phy_magic_frame_mac_set) (a_uint32_t dev_id,
							  a_uint32_t phy_id,
							  fal_mac_addr_t * mac);

	typedef sw_error_t(*hsl_phy_magic_frame_mac_get) (a_uint32_t dev_id,
							  a_uint32_t phy_id,
							  fal_mac_addr_t * mac);
	typedef sw_error_t(*hsl_phy_wol_status_set) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 a_bool_t enable);
	typedef sw_error_t(*hsl_phy_wol_status_get) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_interface_mode_set) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 fal_port_interface_mode_t
							 interface_mode);
	typedef sw_error_t(*hsl_phy_interface_mode_get) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 fal_port_interface_mode_t
							 * interface_mode);
	typedef sw_error_t(*hsl_phy_interface_mode_status_get) (a_uint32_t dev_id,
							 a_uint32_t phy_id,
							 fal_port_interface_mode_t
							 * interface_mode);
	typedef sw_error_t(*hsl_phy_intr_mask_set) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_uint32_t mask);
	typedef sw_error_t(*hsl_phy_intr_mask_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_uint32_t * mask);
	typedef sw_error_t(*hsl_phy_intr_status_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_uint32_t * status);
	typedef sw_error_t(*hsl_phy_counter_set) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_bool_t enable);
	typedef sw_error_t(*hsl_phy_counter_get) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      a_bool_t * enable);
	typedef sw_error_t(*hsl_phy_counter_show) (a_uint32_t dev_id,
						      a_uint32_t phy_id,
						      fal_port_counter_info_t * counter_info);


	typedef struct hsl_phy_ops_s {

		hsl_phy_init phy_init;
		hsl_phy_hibernation_set phy_hibernation_set;
		hsl_phy_hibernation_get phy_hibernation_get;
		hsl_phy_speed_get phy_speed_get;
		hsl_phy_speed_set phy_speed_set;
		hsl_phy_duplex_get phy_duplex_get;
		hsl_phy_duplex_set phy_duplex_set;
		hsl_phy_autoneg_enable_set phy_autoneg_enable_set;
		hsl_phy_autoneg_enable_get phy_autoneg_enable_get;
		hsl_phy_restart_autoneg phy_restart_autoneg;
		hsl_phy_autoneg_status_get phy_autoneg_status_get;
		hsl_phy_autoneg_adv_set phy_autoneg_adv_set;
		hsl_phy_autoneg_adv_get phy_autoneg_adv_get;
		hsl_phy_powersave_set phy_powersave_set;
		hsl_phy_powersave_get phy_powersave_get;
		hsl_phy_cdt phy_cdt;
		hsl_phy_link_status_get phy_link_status_get;
		hsl_phy_get_ability phy_get_ability;
		hsl_phy_mdix_set phy_mdix_set;
		hsl_phy_mdix_get phy_mdix_get;
		hsl_phy_mdix_status_get phy_mdix_status_get;
		hsl_phy_8023az_set phy_8023az_set;
		hsl_phy_8023az_get phy_8023az_get;
		hsl_phy_local_loopback_set phy_local_loopback_set;
		hsl_phy_local_loopback_get phy_local_loopback_get;
		hsl_phy_remote_loopback_set phy_remote_loopback_set;
		hsl_phy_remote_loopback_get phy_remote_loopback_get;
		hsl_phy_master_set phy_master_set;
		hsl_phy_master_get phy_master_get;
		hsl_phy_combo_prefer_medium_set phy_combo_prefer_medium_set;
		hsl_phy_combo_prefer_medium_get phy_combo_prefer_medium_get;
		hsl_phy_combo_medium_status_get phy_combo_medium_status_get;
		hsl_phy_combo_fiber_mode_set phy_combo_fiber_mode_set;
		hsl_phy_combo_fiber_mode_get phy_combo_fiber_mode_get;
		hsl_phy_reset phy_reset;
		hsl_phy_power_off phy_power_off;
		hsl_phy_power_on phy_power_on;
		hsl_phy_reset_status_get phy_reset_status_get;
		hsl_phy_id_get phy_id_get;
		hsl_phy_reg_write phy_reg_write;
		hsl_phy_reg_read phy_reg_read;
		hsl_phy_debug_write phy_debug_write;
		hsl_phy_debug_read phy_debug_read;
		hsl_phy_mmd_write phy_mmd_write;
		hsl_phy_mmd_read phy_mmd_read;
		hsl_phy_magic_frame_mac_set phy_magic_frame_mac_set;
		hsl_phy_magic_frame_mac_get phy_magic_frame_mac_get;
		hsl_phy_wol_status_set phy_wol_status_set;
		hsl_phy_wol_status_get phy_wol_status_get;
		hsl_phy_interface_mode_set phy_interface_mode_set;
		hsl_phy_interface_mode_get phy_interface_mode_get;
		hsl_phy_interface_mode_status_get phy_interface_mode_status_get;
		hsl_phy_intr_mask_set  phy_intr_mask_set;
		hsl_phy_intr_mask_get  phy_intr_mask_get;
		hsl_phy_intr_status_get  phy_intr_status_get;
		hsl_phy_counter_set  phy_counter_set;
		hsl_phy_counter_get  phy_counter_get;
		hsl_phy_counter_show  phy_counter_show;
	} hsl_phy_ops_t;

	sw_error_t hsl_phy_api_ops_register(a_uint32_t dev_id, hsl_phy_ops_t * phy_api_ops);

	sw_error_t hsl_phy_api_ops_unregister(a_uint32_t dev_id, hsl_phy_ops_t * phy_api_ops);

	hsl_phy_ops_t *hsl_phy_api_ops_get(a_uint32_t dev_id);

	sw_error_t phy_api_ops_init(a_uint32_t dev_id);

#ifdef __cplusplus
}
#endif				/* __cplusplus */
#endif				/* _HSL_PHY_H_ */
