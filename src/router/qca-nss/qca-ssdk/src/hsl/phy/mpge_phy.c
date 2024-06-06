/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
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

#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "hsl_phy.h"
#include "qcaphy_common.h"
#include "ssdk_plat.h"
#include "qca808x_phy.h"
#include "mpge_phy.h"
#ifdef IN_LED
#include "mpge_led.h"
#endif

#define PHY_DAC(val) (val<<8)
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* mpge_phy_set_hibernate - set hibernate status
*
*/
static sw_error_t
mpge_phy_set_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	return qca808x_phy_set_hibernate (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_get_hibernate - get hibernate status
*
*/
static sw_error_t
mpge_phy_get_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	return qca808x_phy_get_hibernate (dev_id, phy_addr, enable);
}
#endif
/******************************************************************************
*
* mpge_phy_cdt - cable diagnostic test
*
*/
static sw_error_t
mpge_phy_cdt(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	return qca808x_phy_cdt (dev_id, phy_addr, mdi_pair,
		cable_status, cable_len);
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* mpge_phy_set_remote_loopback
*
*/
static sw_error_t
mpge_phy_set_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	return qca808x_phy_set_remote_loopback (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_get_remote_loopback
*
*/
static sw_error_t
mpge_phy_get_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	return qca808x_phy_get_remote_loopback (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_set_802.3az
*
*/
static sw_error_t
mpge_phy_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	return qca808x_phy_set_8023az (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_get_8023az status
*
*/
static sw_error_t
mpge_phy_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	return qca808x_phy_get_8023az (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_set wol-frame mac address
*
*/
static sw_error_t
mpge_phy_set_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	return qca808x_phy_set_magic_frame_mac (dev_id, phy_addr, mac);
}

/******************************************************************************
*
* mpge_phy_get wol - frame mac address
*
*/
static sw_error_t
mpge_phy_get_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	return qca808x_phy_get_magic_frame_mac (dev_id, phy_addr, mac);
}

/******************************************************************************
*
* mpge_phy_set wol - enable or disable
*
*/
static sw_error_t
mpge_phy_set_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	return qca808x_phy_set_wol_status (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_get_wol status - get wol status
*
*/
static sw_error_t
mpge_phy_get_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	return qca808x_phy_get_wol_status (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_set_counter - set counter status
*
*/
static sw_error_t
mpge_phy_set_counter(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	return qca808x_phy_set_counter (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_get_counter_status - get counter status
*
*/
static sw_error_t
mpge_phy_get_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	return qca808x_phy_get_counter (dev_id, phy_addr, enable);
}

/******************************************************************************
*
* mpge_phy_show show - counter statistics
*
*/
static sw_error_t
mpge_phy_show_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	return qca808x_phy_show_counter (dev_id, phy_addr, counter_infor);
}

/******************************************************************************
*
* mpge_phy_set_intr_mask - Set interrupt mask with the
* specified device.
*/
sw_error_t
mpge_phy_set_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_mask_flag)
{
	a_uint16_t phy_data = 0, mask = 0;

	mask = MPGE_INTR_STATUS_LINK_UP | MPGE_INTR_STATUS_LINK_DOWN |
		MPGE_INTR_SPEED_CHANGE | MPGE_INTR_DUPLEX_CHANGE |
		MPGE_INTR_WOL;
	if (intr_mask_flag & FAL_PHY_INTR_STATUS_UP_CHANGE) {
		phy_data |= MPGE_INTR_STATUS_LINK_UP;
	}

	if (intr_mask_flag & FAL_PHY_INTR_STATUS_DOWN_CHANGE) {
		phy_data |= MPGE_INTR_STATUS_LINK_DOWN;
	}

	if (intr_mask_flag & FAL_PHY_INTR_SPEED_CHANGE) {
		phy_data |= MPGE_INTR_SPEED_CHANGE;
	}

	if (intr_mask_flag & FAL_PHY_INTR_DUPLEX_CHANGE) {
		phy_data |= MPGE_INTR_DUPLEX_CHANGE;
	}

	if (intr_mask_flag & FAL_PHY_INTR_WOL_STATUS) {
		phy_data |= MPGE_INTR_WOL;
	}
	return hsl_phy_modify_mii(dev_id, phy_addr, MPGE_PHY_INTR_MASK, mask,
		phy_data);
}

/******************************************************************************
*
* mpge_phy_get_intr_mask - Get interrupt mask with the
* specified device.
*/
sw_error_t
mpge_phy_get_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_mask_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, MPGE_PHY_INTR_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_mask_flag = 0;
	if (phy_data & MPGE_INTR_STATUS_LINK_UP) {
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	if (phy_data & MPGE_INTR_STATUS_LINK_DOWN) {
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;
	}

	if (phy_data & MPGE_INTR_SPEED_CHANGE) {
		*intr_mask_flag |= FAL_PHY_INTR_SPEED_CHANGE;
	}

	if (phy_data & MPGE_INTR_DUPLEX_CHANGE) {
		*intr_mask_flag |= FAL_PHY_INTR_DUPLEX_CHANGE;
	}

	if (phy_data & MPGE_INTR_WOL) {
		*intr_mask_flag |= FAL_PHY_INTR_WOL_STATUS;
	}

	return SW_OK;
}

/******************************************************************************
*
* mpge_phy_get_intr_status - Get interrupt status with the
* specified device.
*/
sw_error_t
mpge_phy_get_intr_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_status_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, MPGE_PHY_INTR_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_status_flag = 0;
	if (phy_data & MPGE_INTR_STATUS_LINK_UP) {
		*intr_status_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	if (phy_data & MPGE_INTR_STATUS_LINK_DOWN) {
		*intr_status_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;
	}

	if (phy_data & MPGE_INTR_SPEED_CHANGE) {
		*intr_status_flag |= FAL_PHY_INTR_SPEED_CHANGE;
	}

	if (phy_data & MPGE_INTR_DUPLEX_CHANGE) {
		*intr_status_flag |= FAL_PHY_INTR_DUPLEX_CHANGE;
	}

	if (phy_data & MPGE_INTR_WOL) {
		*intr_status_flag |= FAL_PHY_INTR_WOL_STATUS;
	}

	return SW_OK;
}
#endif

/******************************************************************************
*
* mpge_phy_set_eee_advertisement
*
*/
static sw_error_t
mpge_phy_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv)
{
	return qca808x_phy_set_eee_adv (dev_id, phy_addr, adv);
}

/******************************************************************************
*
* mpge_phy_get_eee_advertisement
*
*/
static sw_error_t
mpge_phy_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	return qca808x_phy_get_eee_adv (dev_id, phy_addr, adv);
}

/******************************************************************************
*
* mpge_phy_get_eee_partner_advertisement
*
*/
static sw_error_t
mpge_phy_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	return qca808x_phy_get_eee_partner_adv (dev_id, phy_addr, adv);
}

/******************************************************************************
*
* mpge_phy_get_eee_capability
*
*/
static sw_error_t
mpge_phy_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap)
{
	return qca808x_phy_get_eee_cap (dev_id, phy_addr, cap);
}

/******************************************************************************
*
* mpge_phy_get_eee_status - get eee status
*
*/
static sw_error_t
mpge_phy_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *status)
{
	return qca808x_phy_get_eee_status (dev_id, phy_addr, status);
}
/******************************************************************************
*
* mpge_phy_cdt_thresh_set - set CDT threshold
*
* set CDT threshold
*/
static sw_error_t
mpge_phy_cdt_thresh_init(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL3,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL3_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL4,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL4_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL5,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL5_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL6,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL6_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL7,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL7_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL9,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL9_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL13,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL13_VAL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
		MPGE_PHY_MMD3_CDT_THRESH_CTRL14,
		MPGE_PHY_MMD3_NEAR_ECHO_THRESH_VAL);

	return rv;
}

/******************************************************************************
*
* mpge_phy_function_reset - do function reset
*
*/
static sw_error_t
mpge_phy_function_reset(a_uint32_t dev_id, a_uint32_t phy_addr,
	hsl_phy_function_reset_t phy_reset_type)
{
	sw_error_t rv = SW_OK;

	switch (phy_reset_type)
	{
		case PHY_FIFO_RESET:
			rv = hsl_phy_modify_mii(dev_id, phy_addr, MPGE_PHY_FIFO_CONTROL,
				MPGE_PHY_FIFO_RESET, 0);
			PHY_RTN_ON_ERROR(rv);

			aos_mdelay(50);

			rv = hsl_phy_modify_mii(dev_id, phy_addr, MPGE_PHY_FIFO_CONTROL,
				MPGE_PHY_FIFO_RESET, MPGE_PHY_FIFO_RESET);
			PHY_RTN_ON_ERROR(rv);
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

static sw_error_t
mpge_phy_dac_set(a_uint32_t dev_id, a_uint32_t phy_addr, phy_dac_t phy_dac)
{
	sw_error_t rv = SW_OK;

	if(phy_dac.mdac != PHY_INVALID_DAC)
	{
		SSDK_INFO("phy mdac is set as 0x%x\n", phy_dac.mdac);
		/*set mdac value*/
		rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD1_NUM,
			MPGE_PHY_MMD1_DAC, BITS(8,8), PHY_DAC(phy_dac.mdac));
		PHY_RTN_ON_ERROR(rv);
	}
	if(phy_dac.edac != PHY_INVALID_DAC)
	{
		SSDK_INFO("phy edac is set as 0x%x\n", phy_dac.edac);
		/*set edac value*/
		rv = hsl_phy_modify_debug(dev_id, phy_addr, MPGE_PHY_DEBUG_EDAC,
			BITS(8,8), PHY_DAC(phy_dac.edac));
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

static void
mpge_phy_dac_init(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t port_id)
{
	phy_dac_t  phy_dac;

	hsl_port_phy_dac_get(dev_id, port_id, &phy_dac);
	mpge_phy_dac_set(dev_id, phy_addr, phy_dac);

	return;
}

static sw_error_t
mpge_phy_ldo_efuse_set(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t efuse_value)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	/*when set the register of MPGE_PHY_DEBUG_ANA_LDO_EFUSE, the register of
	MPGE_PHY_DEBUG_ANA_DAC_FILTER will be changed automatically, so need to
	save it and restore it*/
	phy_data = hsl_phy_debug_reg_read(dev_id, phy_addr, MPGE_PHY_DEBUG_ANA_DAC_FILTER);
	PHY_RTN_ON_READ_ERROR(phy_data);
	rv = hsl_phy_modify_debug(dev_id, phy_addr, MPGE_PHY_DEBUG_ANA_LDO_EFUSE,
		BITS(4,4), efuse_value);
	PHY_RTN_ON_ERROR(rv);
	return hsl_phy_debug_reg_write(dev_id, phy_addr, MPGE_PHY_DEBUG_ANA_DAC_FILTER,
		phy_data);
}

static sw_error_t
mpge_phy_hw_init(a_uint32_t dev_id,  a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0, phy_addr = 0;
	sw_error_t rv = SW_OK;

	for (port_id = SSDK_PHYSICAL_PORT0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
			PHY_RTN_ON_ERROR(rv);
			/*configure the CDT threshold*/
			rv = mpge_phy_cdt_thresh_init (dev_id, phy_addr);
			PHY_RTN_ON_ERROR(rv);
			/*set LDO efuse as default and make ICC efuse take effect only*/
			rv = mpge_phy_ldo_efuse_set(dev_id, phy_addr,
				MPGE_PHY_DEBUG_ANA_LDO_EFUSE_DEFAULT);
			/*special configuration for AZ*/
			rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
				MPGE_PHY_MMD3_AZ_CTRL1, MPGE_PHY_MMD3_AZ_CTRL1_VAL);
			PHY_RTN_ON_ERROR(rv);
			rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD3_NUM,
				MPGE_PHY_MMD3_AZ_CTRL2, MPGE_PHY_MMD3_AZ_CTRL2_VAL);
			PHY_RTN_ON_ERROR(rv);
			/*configure MSE threshold and over threshold times*/
			rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD1_NUM,
				MPGE_PHY_MMD1_MSE_THRESH1, MPGE_PHY_MMD1_MSE_THRESH1_VAL);
			PHY_RTN_ON_ERROR(rv);
			rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, MPGE_PHY_MMD1_NUM,
				MPGE_PHY_MMD1_MSE_THRESH2, MPGE_PHY_MMD1_MSE_THRESH2_VAL);
			PHY_RTN_ON_ERROR(rv);
			mpge_phy_dac_init(dev_id, phy_addr, port_id);
		}
	}

	return rv;
}

static sw_error_t mpge_phy_api_ops_init(void)
{
	sw_error_t ret = SW_OK;
	hsl_phy_ops_t *mpge_phy_api_ops = NULL;

	mpge_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (mpge_phy_api_ops == NULL)
	{
		SSDK_ERROR("mpge phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(MPGE_PHY_CHIP);
	mpge_phy_api_ops->phy_get_status = qcaphy_status_get;
	mpge_phy_api_ops->phy_speed_get = qcaphy_get_speed;
	mpge_phy_api_ops->phy_speed_set = qcaphy_set_speed;
	mpge_phy_api_ops->phy_duplex_get = qcaphy_get_duplex;
	mpge_phy_api_ops->phy_duplex_set = qcaphy_set_duplex;
	mpge_phy_api_ops->phy_autoneg_enable_set = qcaphy_autoneg_enable;
	mpge_phy_api_ops->phy_restart_autoneg = qcaphy_autoneg_restart;
	mpge_phy_api_ops->phy_autoneg_status_get = qcaphy_autoneg_status;
	mpge_phy_api_ops->phy_autoneg_adv_set = qcaphy_set_autoneg_adv;
	mpge_phy_api_ops->phy_autoneg_adv_get = qcaphy_get_autoneg_adv;
	mpge_phy_api_ops->phy_link_status_get = qcaphy_get_link_status;
	mpge_phy_api_ops->phy_reset = qcaphy_sw_reset;
	mpge_phy_api_ops->phy_id_get = qcaphy_get_phy_id;
	mpge_phy_api_ops->phy_power_off = qcaphy_poweroff;
	mpge_phy_api_ops->phy_power_on = qcaphy_poweron;
	mpge_phy_api_ops->phy_cdt = mpge_phy_cdt;
#ifndef IN_PORTCONTROL_MINI
	mpge_phy_api_ops->phy_mdix_set = qcaphy_set_mdix;
	mpge_phy_api_ops->phy_mdix_get = qcaphy_get_mdix;
	mpge_phy_api_ops->phy_mdix_status_get = qcaphy_get_mdix_status;
	mpge_phy_api_ops->phy_local_loopback_set = qcaphy_set_local_loopback;
	mpge_phy_api_ops->phy_local_loopback_get = qcaphy_get_local_loopback;
	mpge_phy_api_ops->phy_remote_loopback_set = mpge_phy_set_remote_loopback;
	mpge_phy_api_ops->phy_remote_loopback_get = mpge_phy_get_remote_loopback;
	mpge_phy_api_ops->phy_8023az_set = mpge_phy_set_8023az;
	mpge_phy_api_ops->phy_8023az_get = mpge_phy_get_8023az;
	mpge_phy_api_ops->phy_hibernation_set = mpge_phy_set_hibernate;
	mpge_phy_api_ops->phy_hibernation_get = mpge_phy_get_hibernate;
	mpge_phy_api_ops->phy_magic_frame_mac_set = mpge_phy_set_magic_frame_mac;
	mpge_phy_api_ops->phy_magic_frame_mac_get = mpge_phy_get_magic_frame_mac;
	mpge_phy_api_ops->phy_counter_set = mpge_phy_set_counter;
	mpge_phy_api_ops->phy_counter_get = mpge_phy_get_counter;
	mpge_phy_api_ops->phy_counter_show = mpge_phy_show_counter;
	mpge_phy_api_ops->phy_wol_status_set = mpge_phy_set_wol_status;
	mpge_phy_api_ops->phy_wol_status_get = mpge_phy_get_wol_status;
	mpge_phy_api_ops->phy_intr_mask_set = mpge_phy_set_intr_mask;
	mpge_phy_api_ops->phy_intr_mask_get = mpge_phy_get_intr_mask;
	mpge_phy_api_ops->phy_intr_status_get = mpge_phy_get_intr_status;
#endif
	mpge_phy_api_ops->phy_eee_adv_set = mpge_phy_set_eee_adv;
	mpge_phy_api_ops->phy_eee_adv_get = mpge_phy_get_eee_adv;
	mpge_phy_api_ops->phy_eee_partner_adv_get = mpge_phy_get_eee_partner_adv;
	mpge_phy_api_ops->phy_eee_cap_get = mpge_phy_get_eee_cap;
	mpge_phy_api_ops->phy_eee_status_get = mpge_phy_get_eee_status;
	mpge_phy_api_ops->phy_function_reset = mpge_phy_function_reset;
#ifdef IN_LED
	mpge_phy_led_api_ops_init(mpge_phy_api_ops);
#endif
	ret = hsl_phy_api_ops_register(MPGE_PHY_CHIP, mpge_phy_api_ops);

	if (ret == SW_OK)
	{
		SSDK_INFO("qca probe mpge phy driver succeeded!\n");
	}
	else
	{
		SSDK_ERROR("qca probe mpge phy driver failed! (code: %d)\n", ret);
	}

	return ret;
}

/******************************************************************************
*
* mpge_phy_init -
*
*/
int mpge_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_int32_t ret = 0;
	static a_bool_t phy_ops_flag = A_FALSE;

	if(phy_ops_flag == A_FALSE &&
			mpge_phy_api_ops_init() == SW_OK) {
		phy_ops_flag = A_TRUE;
	}
	mpge_phy_hw_init(dev_id, port_bmp);

	return ret;
}
