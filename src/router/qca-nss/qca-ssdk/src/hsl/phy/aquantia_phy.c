/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
 *
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
 *
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

#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "aquantia_phy.h"
#include "hsl_phy.h"
#include "qcaphy_c45_common.h"
#include "ssdk_plat.h"

sw_error_t
aquantia_phy_get_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t * speed)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = aquantia_phy_get_status(dev_id, phy_addr, &phy_status);
	PHY_RTN_ON_ERROR(rv);
	if (phy_status.link_status == PORT_LINK_DOWN) {
		/*the speed register(0x4007c800) is not stable when aquantia phy is down,
		 but some APIs such as aquantia_phy_set_duplex() aquantia_phy_interface_set_mode()
		 need to get the speed, so set the speed default value as 100M when link down*/
		*speed = FAL_SPEED_100;
		return SW_OK;
	}
	*speed = phy_status.speed;

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_get_duplex - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
aquantia_phy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t * duplex)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = aquantia_phy_get_status(dev_id, phy_addr, &phy_status);
	PHY_RTN_ON_ERROR(rv);
	*duplex = phy_status.duplex;

	return SW_OK;
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* aquantia_phy_reset - reset the phy
*
* reset the phy
*/
sw_error_t aquantia_phy_reset(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = qcaphy_c45_sw_reset(dev_id, phy_addr);
	aos_mdelay(100);

	return rv;
}

/******************************************************************************
*
* aquantia_phy_set_powersave - set power saving status
*
* set power saving status
*/
sw_error_t
aquantia_phy_set_powersave(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE)
	{
		phy_data |= AQUANTIA_POWER_SAVE;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		AQUANTIA_GLOBAL_RESERVED_PROVISIONING6, AQUANTIA_POWER_SAVE, phy_data);
	PHY_RTN_ON_ERROR(rv);
	return aquantia_phy_restart_autoneg(dev_id, phy_addr);
}

/******************************************************************************
*
* aquantia_phy_get_powersave - get power saving status
*
* set power saving status
*/
sw_error_t
aquantia_phy_get_powersave(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GLOBAL_REGISTERS, AQUANTIA_GLOBAL_RESERVED_PROVISIONING6);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data& AQUANTIA_POWER_SAVE)
	{
		*enable = A_TRUE;
	}
	else
	{
		*enable = A_FALSE;
	}

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_set_mdix -
*
* set phy mdix configuraiton
*/
sw_error_t
aquantia_phy_set_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t mode)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	switch(mode)
	{
		case PHY_MDIX_AUTO:
			phy_data |= AQUANTIA_PHY_MDIX_AUTO;
			break;
		case PHY_MDIX_MDIX:
			phy_data |= AQUANTIA_PHY_MDIX;
			break;
		case PHY_MDIX_MDI:
			phy_data |= AQUANTIA_PHY_MDI;
			break;
		default:
			return SW_BAD_PARAM;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
		AQUANTIA_RESERVED_VENDOR_PROVISIONING1, BITS(0,2), phy_data);
	PHY_RTN_ON_ERROR(rv);
	return aquantia_phy_restart_autoneg(dev_id, phy_addr);
}

/******************************************************************************
*
* aquantia_phy_get_mdix
*
* get phy mdix configuration
*/
sw_error_t
aquantia_phy_get_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t * mode)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
		AQUANTIA_RESERVED_VENDOR_PROVISIONING1);
	PHY_RTN_ON_READ_ERROR(phy_data);
	phy_data  &= BITS(0,2);
	switch(phy_data)
	{
		case AQUANTIA_PHY_MDIX_AUTO:
			*mode = PHY_MDIX_AUTO;
			break;
		case AQUANTIA_PHY_MDIX:
			*mode = PHY_MDIX_MDIX;
			break;
		case AQUANTIA_PHY_MDI:
			*mode = PHY_MDIX_MDI;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_get_mdix status
*
* get phy mdix status
*/
sw_error_t
aquantia_phy_get_mdix_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_status_t * mode)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
		AQUANTIA_RESERVED_VENDOR_STATUS1);
	PHY_RTN_ON_READ_ERROR(phy_data);
	*mode = (phy_data &  AQUANTIA_PHY_MDIX_STATUS) ? PHY_MDIX_STATUS_MDIX :
		PHY_MDIX_STATUS_MDI;

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_set_local_loopback
*
* set phy local loopback
*/
sw_error_t
aquantia_phy_set_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	fal_port_speed_t old_speed = FAL_SPEED_BUTT;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE)
	{
		phy_data |= AQUANTIA_INTERNAL_LOOPBACK;
		rv = aquantia_phy_get_speed(dev_id, phy_addr, &old_speed);
		PHY_RTN_ON_ERROR(rv);
		switch(old_speed)
		{
			case FAL_SPEED_10:
				phy_data |= AQUANTIA_10M_LOOPBACK;
				break;
			case FAL_SPEED_100:
				phy_data |= AQUANTIA_100M_LOOPBACK;
				break;
			case FAL_SPEED_1000:
				 phy_data |= AQUANTIA_1000M_LOOPBACK;
				 break;
			case FAL_SPEED_10000:
				phy_data |= AQUANTIA_10000M_LOOPBACK;
				break;
			case FAL_SPEED_2500:
				 phy_data |= AQUANTIA_2500M_LOOPBACK;
				 break;
			case FAL_SPEED_5000:
				phy_data |= AQUANTIA_5000M_LOOPBACK;
				break;
			default:
				return SW_FAIL;
		}
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_PHY_XS_REGISTERS,
		AQUANTIA_PHY_XS_TRANAMIT_RESERVED_VENDOR_PROVISION5,
		AQUANTIA_INTERNAL_LOOPBACK | AQUANTIA_ALL_SPEED_LOOPBACK, phy_data);
}

/******************************************************************************
*
* aquantia_phy_get_local_loopback
*
* get phy local loopback
*/
sw_error_t
aquantia_phy_get_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PHY_XS_REGISTERS,
		AQUANTIA_PHY_XS_TRANAMIT_RESERVED_VENDOR_PROVISION5);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & AQUANTIA_INTERNAL_LOOPBACK)
	{
		*enable = A_TRUE;
	}
	else
	{
		*enable = A_FALSE;
	}

	return SW_OK;
}

sw_error_t
aquantia_phy_set_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	fal_port_speed_t speed = FAL_SPEED_BUTT;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE)
	{
		rv = aquantia_phy_get_speed(dev_id,  phy_addr, &speed);
		PHY_RTN_ON_ERROR(rv);
		switch(speed)
		{
			case FAL_SPEED_10:
				phy_data |= AQUANTIA_10M_LOOPBACK;
				break;
			case FAL_SPEED_100:
				phy_data |= AQUANTIA_100M_LOOPBACK;
				break;
			case FAL_SPEED_1000:
				phy_data |= AQUANTIA_1000M_LOOPBACK;
				break;
			case FAL_SPEED_2500:
				phy_data |= AQUANTIA_2500M_LOOPBACK;
				break;
			case FAL_SPEED_5000:
				phy_data |= AQUANTIA_5000M_LOOPBACK;
				break;
			case FAL_SPEED_10000:
				phy_data |= AQUANTIA_10000M_LOOPBACK;
				break;
			default:
				break;
		}
		phy_data |= AQUANTIA_PHY_REMOTE_LOOPBACK;
	}
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_PHY_XS_REGISTERS,
		AQUANTIA_PHY_XS_TRANAMIT_RESERVED_VENDOR_PROVISION5,
		AQUANTIA_PHY_REMOTE_LOOPBACK | AQUANTIA_ALL_SPEED_LOOPBACK, phy_data);
}

/******************************************************************************
*
* aquantia_phy_get_remote_loopback
*
* get phy remote loopback
*/
sw_error_t
aquantia_phy_get_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PHY_XS_REGISTERS,
		AQUANTIA_PHY_XS_TRANAMIT_RESERVED_VENDOR_PROVISION5);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & AQUANTIA_PHY_REMOTE_LOOPBACK)
	{
		*enable = A_TRUE;
	}
	else
	{
		*enable = A_FALSE;
	}

	return SW_OK;
}
#endif
/******************************************************************************
*
* aquantia_phy_cdt - cable diagnostic test
*
* cable diagnostic test
*/
static inline fal_cable_status_t _phy_cdt_status_mapping(a_uint32_t pair_type, a_uint16_t status)
{
	fal_cable_status_t status_mapping = FAL_CABLE_STATUS_INVALID;

	switch(status)
	{
		case 0:
			status_mapping = FAL_CABLE_STATUS_NORMAL;
			break;
		case 1:
			if(pair_type == CABLE_PAIR_B)
				status_mapping = FAL_CABLE_STATUS_CROSSOVERA;
			else if(pair_type == CABLE_PAIR_C)
				status_mapping = FAL_CABLE_STATUS_CROSSOVERB;
			else if(pair_type == CABLE_PAIR_D)
				status_mapping = FAL_CABLE_STATUS_CROSSOVERC;
			else
				status_mapping = FAL_CABLE_STATUS_INVALID;
			break;
		case 2:
			if(pair_type == CABLE_PAIR_C)
				status_mapping = FAL_CABLE_STATUS_CROSSOVERA;
			else if(pair_type == CABLE_PAIR_D)
				status_mapping = FAL_CABLE_STATUS_CROSSOVERB;
			else
				status_mapping = FAL_CABLE_STATUS_INVALID;
			break;
		case 3:
			if(pair_type == CABLE_PAIR_D)
				status_mapping = FAL_CABLE_STATUS_CROSSOVERA;
			else
				status_mapping = FAL_CABLE_STATUS_INVALID;
			break;
		case 4:
			status_mapping = FAL_CABLE_STATUS_SHORT;
			break;
		case 5:
			status_mapping = FAL_CABLE_STATUS_LOW_MISMATCH;
			break;
		case 6:
			status_mapping = FAL_CABLE_STATUS_HIGH_MISMATCH;
			break;
		case 7:
			status_mapping = FAL_CABLE_STATUS_OPENED;
			break;
		default:
			status_mapping = FAL_CABLE_STATUS_INVALID;
			break;
	}

	return status_mapping;
}

sw_error_t
aquantia_phy_cdt_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_cdt_t * port_cdt)
{
	a_uint16_t status = 0;
	a_uint16_t phy_data = 0;

	if (!port_cdt) {
		return SW_FAIL;
	}
	/* Get cable status */
	status = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		AQUANTIA_CABLE_DIAGNOSTIC_STATUS1);
	PHY_RTN_ON_READ_ERROR(status);
	port_cdt->pair_a_status =  (status & AQUANTIA_CABLE_DIAGNOSTIC_STATUS_PAIRA) >> 12
		& BITS(0, 3);
	port_cdt->pair_b_status = (status & AQUANTIA_CABLE_DIAGNOSTIC_STATUS_PAIRB) >> 8
		& BITS(0, 3);
	port_cdt->pair_c_status = (status & AQUANTIA_CABLE_DIAGNOSTIC_STATUS_PAIRC) >> 4
		& BITS(0, 3);
	port_cdt->pair_d_status = (status & AQUANTIA_CABLE_DIAGNOSTIC_STATUS_PAIRD)
		& BITS(0, 3);
	SSDK_DEBUG("status:%x, pair_a_status:%x,pair_b_status:%x,pair_c_status:%x, pair_d_status:%x\n",
		status, port_cdt->pair_a_status,port_cdt->pair_b_status,
		port_cdt->pair_c_status, port_cdt->pair_d_status);
	/* Get Cable Length value */
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		AQUANTIA_CABLE_DIAGNOSTIC_STATUS2);
	PHY_RTN_ON_READ_ERROR(phy_data);
	port_cdt->pair_a_len = phy_data >> 8 & BITS(0, 8);

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		AQUANTIA_CABLE_DIAGNOSTIC_STATUS4);
	PHY_RTN_ON_READ_ERROR(phy_data);
	port_cdt->pair_b_len = phy_data >> 8 & BITS(0, 8);

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		AQUANTIA_CABLE_DIAGNOSTIC_STATUS6);
	PHY_RTN_ON_READ_ERROR(phy_data);
	port_cdt->pair_c_len = phy_data >> 8 & BITS(0, 8);

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		AQUANTIA_CABLE_DIAGNOSTIC_STATUS8);
	PHY_RTN_ON_READ_ERROR(phy_data);
	port_cdt->pair_d_len = phy_data >> 8 & BITS(0, 8);

	return SW_OK;
}

sw_error_t aquatia_phy_cdt_start(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t status = 0, phy_data = 0;
	a_uint32_t aq_phy_id = 0;
	a_uint16_t ii = 300;
	sw_error_t rv = SW_OK;

	/*select mode0 if aq107, and select mode2 if aq109*/
	rv = qcaphy_get_phy_id(dev_id, phy_addr, &aq_phy_id);
	PHY_RTN_ON_ERROR(rv);
	if(aq_phy_id == AQUANTIA_PHY_109 || aq_phy_id == AQUANTIA_PHY_113C_B0 ||
		aq_phy_id == AQUANTIA_PHY_113C_B1)
	{
		phy_data |= AQUANTIA_PHY_CDT_MODE2;
	}
	else
	{
		phy_data |= AQUANTIA_PHY_CDT_MODE0;
	}

	phy_data |= AQUANTIA_NORMAL_CABLE_DIAGNOSTICS;
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
		 AQUANTIA_GLOBAL_CDT_CONTROL, 0x13, phy_data);
	PHY_RTN_ON_ERROR(rv);
	do {
		aos_mdelay(30);
		status  = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			AQUANTIA_MMD_GLOBAL_REGISTERS,
			AQUANTIA_GLOBAL_GENERAL_STATUS);
		PHY_RTN_ON_READ_ERROR(status);
	}
	while ((status & AQUANTIA_CABLE_DIAGNOSTICS_STATUS) && (--ii));

	return SW_OK;
}

sw_error_t
aquantia_phy_cdt(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	fal_port_cdt_t aquantia_port_cdt = {0};
	sw_error_t rv = SW_OK;

	if (mdi_pair >= 4) {
		return SW_BAD_PARAM;
	}
	rv = aquatia_phy_cdt_start(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);
	rv = aquantia_phy_cdt_get(dev_id, phy_addr, &aquantia_port_cdt);
	PHY_RTN_ON_ERROR(rv);
	switch (mdi_pair)
	{
		case 0:
			*cable_status =
				 _phy_cdt_status_mapping(CABLE_PAIR_A, aquantia_port_cdt.pair_a_status);
			/* Get Cable Length value */
			*cable_len = aquantia_port_cdt.pair_a_len;
			break;
		case 1:
			*cable_status =
				 _phy_cdt_status_mapping(CABLE_PAIR_B, aquantia_port_cdt.pair_b_status);
			/* Get Cable Length value */
			*cable_len = aquantia_port_cdt.pair_b_len;
			break;
		case 2:
			*cable_status =
				_phy_cdt_status_mapping(CABLE_PAIR_C, aquantia_port_cdt.pair_c_status);
			/* Get Cable Length value */
			*cable_len = aquantia_port_cdt.pair_c_len;
			break;
		case 3:
			*cable_status =
				 _phy_cdt_status_mapping(CABLE_PAIR_D, aquantia_port_cdt.pair_d_status);
			/* Get Cable Length value */
			*cable_len = aquantia_port_cdt.pair_d_len;
			break;
		default:
			break;
	}

	return SW_OK;
}
/******************************************************************************
*
* AQUANTIA_set_autoneg_adv - set the phy autoneg Advertisement
*
*/
sw_error_t
aquantia_phy_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg)
{
	a_uint16_t phy_data = 0, phy_data1 = 0 ;
	sw_error_t rv = SW_OK;

	if ((autoneg & FAL_PHY_ADV_10T_HD) || (autoneg & FAL_PHY_ADV_100TX_HD))
	{
		return SW_NOT_SUPPORTED;
	}
	rv = qcaphy_c45_set_autoneg_adv(dev_id, phy_addr, autoneg);
	PHY_RTN_ON_ERROR(rv);

	if (autoneg & FAL_PHY_ADV_1000T_FD)
	{
		phy_data |= AQUANTIA_ADVERTISE_1000FULL;
	}
	if (autoneg & FAL_PHY_ADV_2500T_FD)
	{
		phy_data |= AQUANTIA_ADVERTISE_2500FULL;
	}
	if (autoneg & FAL_PHY_ADV_5000T_FD)
	{
		phy_data |= AQUANTIA_ADVERTISE_5000FULL;
	}
	if (autoneg & FAL_PHY_ADV_10000T_FD)
	{
		phy_data1 |= AQUANTIA_ADVERTISE_10000FULL;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
		AQUANTIA_AUTONEG_VENDOR_PROVISION1, AQUANTIA_ADVERTISE_GIGA_ALL, phy_data);
	PHY_RTN_ON_ERROR(rv);

	return hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, autoneg);
}

/******************************************************************************
*
* AQUANTIA_get_autoneg_adv - get the phy autoneg Advertisement
*
*/
sw_error_t
aquantia_phy_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * autoneg)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*autoneg = 0;
	rv = qcaphy_c45_get_autoneg_adv(dev_id, phy_addr, autoneg);
	PHY_RTN_ON_ERROR(rv);
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
		AQUANTIA_AUTONEG_VENDOR_PROVISION1);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & AQUANTIA_ADVERTISE_1000FULL)
	{
		*autoneg |= FAL_PHY_ADV_1000T_FD;
	}

	if ((phy_data & AQUANTIA_ADVERTISE_2500FULL) && (*autoneg & FAL_PHY_ADV_2500T_FD))
	{
		*autoneg |= FAL_PHY_ADV_2500T_FD;
	}
	else
	{
		*autoneg &= (~FAL_PHY_ADV_2500T_FD);
	}

	if ((phy_data & AQUANTIA_ADVERTISE_5000FULL) && (*autoneg & FAL_PHY_ADV_5000T_FD))
	{
		*autoneg |= FAL_PHY_ADV_5000T_FD;
	}
	else
	{
		*autoneg &= (~FAL_PHY_ADV_5000T_FD);
	}

	return SW_OK;
}

/******************************************************************************
*
* AQUANTIA_restart_autoneg - restart the phy autoneg
*
*/
sw_error_t
aquantia_phy_restart_autoneg(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_PHY_XS_REGISTERS,
		AQUANTIA_PHY_XS_USX_TRANSMIT, AQUANTIA_PHY_USX_AUTONEG_ENABLE,
		AQUANTIA_PHY_USX_AUTONEG_ENABLE);
	PHY_RTN_ON_ERROR(rv);

	return qcaphy_c45_autoneg_restart(dev_id, phy_addr);
}

#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* aquantia_phy_set_802.3az
*
* set 802.3az status
*/
sw_error_t
aquantia_phy_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t eee_adv = 0;

	if(enable == A_TRUE)
		eee_adv |= (FAL_PHY_EEE_1000BASE_T | FAL_PHY_EEE_2500BASE_T |
			FAL_PHY_EEE_5000BASE_T | FAL_PHY_EEE_10000BASE_T);
	rv = qcaphy_c45_set_eee_adv(dev_id, phy_addr, eee_adv);
	PHY_RTN_ON_ERROR(rv);

	return aquantia_phy_restart_autoneg(dev_id, phy_addr);
}

/******************************************************************************
*
* aquantia_phy_get_8023az status
*
* get 8023az status
*/
sw_error_t
aquantia_phy_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	sw_error_t rv = SW_OK;
	a_uint32_t eee_adv = 0;

	rv = qcaphy_c45_get_eee_adv(dev_id, phy_addr, &eee_adv);
	PHY_RTN_ON_ERROR(rv);

	if(eee_adv & (FAL_PHY_EEE_1000BASE_T | FAL_PHY_EEE_2500BASE_T |
		FAL_PHY_EEE_5000BASE_T | FAL_PHY_EEE_10000BASE_T))
	{
		*enable = A_TRUE;
	}
	else
	{
		*enable = A_FALSE;
	}

	return SW_OK;
}
#endif
sw_error_t
aquantia_phy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed)
{
	sw_error_t rv = SW_OK;

	switch(speed)
	{
		case FAL_SPEED_10000:
			rv = aquantia_phy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_10000T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_5000:
			rv = aquantia_phy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_5000T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_2500:
			rv = aquantia_phy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_2500T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_1000:
			rv = aquantia_phy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_1000T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_100:
		case FAL_SPEED_10:
			rv = qcaphy_c45_force_speed_set(dev_id, phy_addr, speed);
			PHY_RTN_ON_ERROR(rv);
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	if(speed >= FAL_SPEED_1000)
	{
		rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE,
			hsl_phy_speed_duplex_to_auto_adv(dev_id, speed, FAL_FULL_DUPLEX));
		PHY_RTN_ON_ERROR(rv);
		rv = aquantia_phy_restart_autoneg(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_set_duplex - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
aquantia_phy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t duplex)
{
	fal_port_speed_t old_speed;
	sw_error_t rv = SW_OK;

	if(duplex != FAL_FULL_DUPLEX)
		return SW_NOT_SUPPORTED;

	rv = aquantia_phy_get_speed(dev_id, phy_addr, &old_speed);
	PHY_RTN_ON_ERROR(rv);

	return aquantia_phy_set_speed(dev_id, phy_addr, old_speed);
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* aquantia_phy_set wol enable or disable
*
* set phy wol enable or disable
*/
sw_error_t
aquantia_phy_set_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data0 = 0, phy_data1 = 0, phy_data2 = 0;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE)
	{
		phy_data0 |= AQUANTIA_PHY_WOL_ENABLE;
		phy_data1 |= AQUANTIA_MAGIC_PACKETS_ENABLE;
		phy_data2 |= AQUANTIA_MAGIC_PACKETS_ENABLE;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_AUTONEG, AQUANTIA_RESERVED_VENDOR_PROVISIONING1,
		AQUANTIA_PHY_WOL_ENABLE, phy_data0);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS, AQUANTIA_MAGIC_ENGINE_REGISTER1,
		AQUANTIA_MAGIC_PACKETS_ENABLE, phy_data1);
	PHY_RTN_ON_ERROR(rv);
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS, AQUANTIA_MAGIC_ENGINE_REGISTER2,
		AQUANTIA_MAGIC_PACKETS_ENABLE, phy_data2);
}

/******************************************************************************
*
* aquantia_phy_get_wol status
*
* get wol status
*/
sw_error_t
aquantia_phy_get_wol_status(a_uint32_t dev_id, a_uint32_t phy_id, a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	*enable = A_FALSE;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_id, A_TRUE, AQUANTIA_MMD_AUTONEG,
		AQUANTIA_RESERVED_VENDOR_PROVISIONING1);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & AQUANTIA_PHY_WOL_ENABLE)
	{
		*enable = A_TRUE;
	}

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_set wol frame mac address
*
* set phy wol frame mac address
*/
sw_error_t
aquantia_phy_set_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_id,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	a_uint16_t phy_data3 = 0;
	sw_error_t rv = SW_OK;

	phy_data1 = (mac->uc[1] << 8) | mac->uc[0];
	phy_data2 = (mac->uc[3] << 8) | mac->uc[2];
	phy_data3 = (mac->uc[5] << 8) | mac->uc[4];
	rv = hsl_phy_mmd_reg_write(dev_id, phy_id, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS, AQUANTIA_MAGIC_FRAME_MAC0, phy_data1);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_id, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS, AQUANTIA_MAGIC_FRAME_MAC1, phy_data2);
	PHY_RTN_ON_ERROR(rv);
	return hsl_phy_mmd_reg_write(dev_id, phy_id, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS, AQUANTIA_MAGIC_FRAME_MAC2, phy_data3);
}

/******************************************************************************
*
* aquantia_phy_get wol frame mac address
*
* get phy wol frame mac address
*/
sw_error_t
aquantia_phy_get_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	a_uint16_t phy_data3 = 0;

	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS,
		AQUANTIA_MAGIC_FRAME_MAC0);
	PHY_RTN_ON_READ_ERROR(phy_data1);
	phy_data2 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS,
		AQUANTIA_MAGIC_FRAME_MAC1);
	PHY_RTN_ON_READ_ERROR(phy_data2);
	phy_data3 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GBE_STANDARD_REGISTERS,
		AQUANTIA_MAGIC_FRAME_MAC2);
	PHY_RTN_ON_READ_ERROR(phy_data3);
	mac->uc[0] = (phy_data1 & BITS(0, 8));
	mac->uc[1] = (phy_data1 >> 8) & BITS(0, 8);
	mac->uc[2] = (phy_data2 & BITS(0, 8));
	mac->uc[3] = (phy_data2 >> 8) & BITS(0, 8);
	mac->uc[4] = (phy_data3 & BITS(0, 8));
	mac->uc[5] = (phy_data3 >> 8) & BITS(0, 8);

	return SW_OK;
}
#endif
sw_error_t
aquantia_phy_interface_set_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t interface_mode)
{
	a_uint16_t phy_data = 0;
	a_uint32_t phy_register = 0;
	fal_port_speed_t speed = FAL_SPEED_BUTT;
	sw_error_t rv =SW_OK;

	rv = aquantia_phy_get_speed(dev_id, phy_addr, &speed);
	PHY_RTN_ON_ERROR(rv);
	switch (speed)
	{
		case FAL_SPEED_10:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_10M;
			break;
		case FAL_SPEED_100:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_100M;
			break;
		case FAL_SPEED_1000:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_1000M;
			break;
		case FAL_SPEED_2500:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_2500M;
			break;
		case FAL_SPEED_5000:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_5000M;
			break;
		case FAL_SPEED_10000:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_10000M;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	switch(interface_mode)
	{
		case PHY_SGMII_BASET:
			if(speed == FAL_SPEED_10 || speed == FAL_SPEED_100 ||
				speed == FAL_SPEED_1000)
			{
				phy_data |= AQUANTIA_SERDES_MODE_SGMII;
			}
			else
			{
				return SW_NOT_SUPPORTED;
			}
			break;
		case PORT_USXGMII:
			phy_data |= AQUANTIA_SERDES_MODE_XFI;
			break;
		case PORT_SGMII_PLUS:
			if(speed == FAL_SPEED_2500)
			{
				phy_data |= AQUANTIA_SERDES_MODE_OCSGMII;
			}
			else
			{
				return SW_NOT_SUPPORTED;
			}
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GLOBAL_REGISTERS, phy_register, BITS(0, 3), phy_data);
}

/******************************************************************************
*
* aquantia_phy_interface mode status get
*
* get aquantia phy interface mode status
*/
sw_error_t
aquantia_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode_status)
{
	a_uint16_t phy_data = 0;
	a_uint32_t phy_register = 0;
	fal_port_speed_t speed = FAL_SPEED_BUTT;
	sw_error_t rv = SW_OK;

	rv = aquantia_phy_get_speed(dev_id, phy_addr, &speed);
	PHY_RTN_ON_ERROR(rv);
	switch (speed)
	{
		case FAL_SPEED_10:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_10M;
			break;
		case FAL_SPEED_100:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_100M;
			break;
		case FAL_SPEED_1000:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_1000M;
			break;
		case FAL_SPEED_2500:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_2500M;
			break;
		case FAL_SPEED_5000:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_5000M;
			break;
		case FAL_SPEED_10000:
			phy_register = AQUANTIA_GLOBAL_SYS_CONFIG_FOR_10000M;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GLOBAL_REGISTERS, phy_register);
	PHY_RTN_ON_READ_ERROR(phy_data);
	phy_data &= (BITS(0, 3));
	switch(phy_data)
	{
		case AQUANTIA_SERDES_MODE_SGMII:
			*interface_mode_status = PHY_SGMII_BASET;
			break;
		case AQUANTIA_SERDES_MODE_XFI:
			*interface_mode_status = PORT_USXGMII;
			break;
		case AQUANTIA_SERDES_MODE_OCSGMII:
			*interface_mode_status = PORT_SGMII_PLUS;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* aquantia_phy_intr_mask_set - Set interrupt mask with the
* specified device.
*/
sw_error_t
aquantia_phy_intr_mask_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_mask_flag)
{
	sw_error_t rv = SW_OK;

	if ((FAL_PHY_INTR_STATUS_DOWN_CHANGE | FAL_PHY_INTR_STATUS_UP_CHANGE)
		& intr_mask_flag)
	{
		rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
			AQUANTIA_AUTONEG_TRANSMIT_VENDOR_INTR_MASK, AQUANTIA_INTR_LINK_STATUS_CHANGE,
			AQUANTIA_INTR_LINK_STATUS_CHANGE);
		PHY_RTN_ON_ERROR(rv);

		rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
			AQUANTIA_GLOBAL_INTR_VENDOR_MASK, AQUANTIA_AUTO_AND_ALARMS_INTR_MASK,
			AQUANTIA_AUTO_AND_ALARMS_INTR_MASK);
		PHY_RTN_ON_ERROR(rv);

		rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_GLOBAL_REGISTERS,
			AQUANTIA_GLOBAL_INTR_STANDARD_MASK, AQUANTIA_ALL_VENDOR_ALARMS_INTR_MASK,
			AQUANTIA_ALL_VENDOR_ALARMS_INTR_MASK);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_intr_mask_get - Get interrupt mask with the
* specified device.
*/
sw_error_t
aquantia_phy_intr_mask_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_mask_flag)
{
	a_uint16_t phy_data1 = 0, phy_data2 = 0, phy_data3 = 0;

	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_AUTONEG, AQUANTIA_AUTONEG_TRANSMIT_VENDOR_INTR_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data1);
	phy_data2 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GLOBAL_REGISTERS, AQUANTIA_GLOBAL_INTR_VENDOR_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data2);

	phy_data3 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_GLOBAL_REGISTERS, AQUANTIA_GLOBAL_INTR_STANDARD_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data3);
	if ((AQUANTIA_INTR_LINK_STATUS_CHANGE & phy_data1) &&
		(AQUANTIA_AUTO_AND_ALARMS_INTR_MASK & phy_data2) &&
		(AQUANTIA_ALL_VENDOR_ALARMS_INTR_MASK & phy_data3))
	{
		*intr_mask_flag = FAL_PHY_INTR_STATUS_DOWN_CHANGE |
			FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	return SW_OK;
}
static sw_error_t
_aquantia_phy_line_side_counter_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	a_uint16_t msw_counter = 0;
	a_uint16_t lsw_counter = 0;

	/*get line side tx good packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_TRANSMIT_GOOD_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_TRANSMIT_GOOD_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->TxGoodFrame = (msw_counter << 16) | lsw_counter;

	/*get line side tx bad packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_TRANSMIT_ERROR_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_TRANSMIT_ERROR_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->TxBadCRC = (msw_counter << 16) | lsw_counter;

	/*get line side rx good packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_RECEIVE_GOOD_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_RECEIVE_GOOD_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->RxGoodFrame = (msw_counter << 16) | lsw_counter;

	/*get line side rx bad packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_RECEIVE_ERROR_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_LINE_SIDE_RECEIVE_ERROR_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->RxBadCRC = (msw_counter << 16) | lsw_counter;

	return SW_OK;
}

static sw_error_t
_aquantia_phy_system_side_counter_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	a_uint16_t msw_counter = 0;
	a_uint16_t lsw_counter = 0;

	/*get system tx good packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_TRANSMIT_GOOD_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_TRANSMIT_GOOD_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->SysTxGoodFrame = (msw_counter << 16) | lsw_counter;

	/*get system tx bad packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_TRANSMIT_ERROR_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_TRANSMIT_ERROR_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->SysTxBadCRC = (msw_counter << 16) | lsw_counter;

	/*get system rx good packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_RECEIVE_GOOD_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_RECEIVE_GOOD_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->SysRxGoodFrame = (msw_counter << 16) | lsw_counter;

	/*get system rx bad packets*/
	msw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_RECEIVE_ERROR_FRAME_COUNTER2);
	PHY_RTN_ON_READ_ERROR(msw_counter);
	lsw_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_PCS_REGISTERS,
		AQUANTIA_SYSTEM_SIDE_RECEIVE_ERROR_FRAME_COUNTER1);
	PHY_RTN_ON_READ_ERROR(lsw_counter);
	counter_infor->SysRxBadCRC = (msw_counter << 16) | lsw_counter;

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_show show counter statistics
*
* show counter statistics
*/
sw_error_t
aquantia_phy_show_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	sw_error_t rv = SW_OK;
	fal_port_speed_t speed;

	rv = aquantia_phy_get_speed(dev_id, phy_addr, &speed);
	PHY_RTN_ON_ERROR(rv);
	if(speed == FAL_SPEED_2500 || speed == FAL_SPEED_5000 ||
		speed == FAL_SPEED_10000)
	{
		rv = _aquantia_phy_line_side_counter_get(dev_id, phy_addr, counter_infor);
		PHY_RTN_ON_ERROR(rv);
	}
	return _aquantia_phy_system_side_counter_get(dev_id, phy_addr, counter_infor);
}
#endif
/******************************************************************************
*
* aquantia_phy_get_status
*
* get phy status
*/
sw_error_t
aquantia_phy_get_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct port_phy_status *phy_status)
{
	a_uint16_t phy_data = 0;

	phy_status->link_status = qcaphy_c45_get_link_status(dev_id, phy_addr);
	/*get phy speed and duplex*/
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_AUTONEG,
		AQUANTIA_REG_AUTONEG_VENDOR_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);
	switch ((phy_data & AQUANTIA_STATUS_SPEED_MASK) >>1)
	{
		case AQUANTIA_STATUS_SPEED_10MBS:
			phy_status->speed = FAL_SPEED_10;
			break;
		case AQUANTIA_STATUS_SPEED_100MBS:
			phy_status->speed = FAL_SPEED_100;
			break;
		case AQUANTIA_STATUS_SPEED_1000MBS:
			phy_status->speed = FAL_SPEED_1000;
			break;
		case AQUANTIA_STATUS_SPEED_2500MBS:
			phy_status->speed = FAL_SPEED_2500;
			break;
		case AQUANTIA_STATUS_SPEED_5000MBS:
			phy_status->speed = FAL_SPEED_5000;
			break;
		case AQUANTIA_STATUS_SPEED_10000MBS:
			phy_status->speed = FAL_SPEED_10000;
			break;
		default:
			return SW_READ_ERROR;
	}
	if (phy_data & AQUANTIA_STATUS_FULL_DUPLEX)
	{
		phy_status->duplex = FAL_FULL_DUPLEX;
	}
	else
	{
		phy_status->duplex = FAL_HALF_DUPLEX;
	}
	/* get phy tx flowctrl and rx flowctrl resolution status */
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		AQUANTIA_MMD_AUTONEG,
		AQUANTIA_RESERVED_VENDOR_STATUS1);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(phy_data & AQUANTIA_PHY_TX_FLOWCTRL_STATUS)
	{
		phy_status->tx_flowctrl = A_TRUE;
	}
	else
	{
		phy_status->tx_flowctrl = A_FALSE;
	}
	if(phy_data & AQUANTIA_PHY_RX_FLOWCTRL_STATUS)
	{
		phy_status->rx_flowctrl = A_TRUE;
	}
	else
	{
		phy_status->rx_flowctrl = A_FALSE;
	}

	return SW_OK;
}

/******************************************************************************
*
* aquantia_phy_set_eee_advertisement
*
* set eee advertisement
*/
sw_error_t
aquantia_phy_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv)
{
	sw_error_t rv = SW_OK;

	if(adv & FAL_PHY_EEE_100BASE_T)
		return SW_NOT_SUPPORTED;
	rv = qcaphy_c45_set_eee_adv(dev_id, phy_addr, adv);
	PHY_RTN_ON_ERROR(rv);
	return aquantia_phy_restart_autoneg(dev_id, phy_addr);
}
/******************************************************************************
*
* aquantia_phy_hw_register init to avoid packet loss
*
*/
sw_error_t
aquantia_phy_hw_init(a_uint32_t dev_id,  a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0, phy_addr = 0, aq_phy_id = 0;
	sw_error_t rv = SW_OK;

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
			/*set auto neg of aq*/
			rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
				AQUANTIA_MMD_PHY_XS_REGISTERS, AQUANTIA_PHY_XS_USX_TRANSMIT,
				AQUANTIA_PHY_USX_AUTONEG_ENABLE, AQUANTIA_PHY_USX_AUTONEG_ENABLE);
			PHY_RTN_ON_ERROR(rv);
			/*config interrupt of aq*/
			rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, AQUANTIA_MMD_AUTONEG,
				AQUANTIA_AUTONEG_TRANSMIT_VENDOR_INTR_MASK,
				AQUANTIA_INTR_LINK_STATUS_CHANGE, AQUANTIA_INTR_LINK_STATUS_CHANGE);
			PHY_RTN_ON_ERROR(rv);
			rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
				AQUANTIA_MMD_GLOBAL_REGISTERS, AQUANTIA_GLOBAL_INTR_STANDARD_MASK,
				AQUANTIA_ALL_VENDOR_ALARMS_INTR_MASK,
				AQUANTIA_ALL_VENDOR_ALARMS_INTR_MASK);
			rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
				AQUANTIA_MMD_GLOBAL_REGISTERS, AQUANTIA_GLOBAL_INTR_VENDOR_MASK,
				AQUANTIA_AUTO_AND_ALARMS_INTR_MASK,
				AQUANTIA_AUTO_AND_ALARMS_INTR_MASK);
			PHY_RTN_ON_ERROR(rv);

			/* config aq phy ACT and LINK led behavior*/
			rv = qcaphy_c45_get_phy_id (dev_id, phy_addr, &aq_phy_id);
			PHY_RTN_ON_ERROR(rv);
			if(aq_phy_id == AQUANTIA_PHY_113C_B0 || aq_phy_id == AQUANTIA_PHY_113C_B1)
			{
				rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE,
					AQUANTIA_MMD_GLOBAL_REGISTERS,
					AQUANTIA_PROVISIONING_LED0_STATUS,
					AQUANTIA_LINK_ACT_LED_DISABLE_VALUE);
				PHY_RTN_ON_ERROR(rv);
				rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE,
					AQUANTIA_MMD_GLOBAL_REGISTERS,
					AQUANTIA_PROVISIONING_LED1_STATUS,
					AQUANTIA_LINK_ACT_LED_VALUE);
				PHY_RTN_ON_ERROR(rv);
				rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE,
					AQUANTIA_MMD_GLOBAL_REGISTERS,
					AQUANTIA_PROVISIONING_LED2_STATUS,
					AQUANTIA_LINK_ACT_LED_DISABLE_VALUE);
				PHY_RTN_ON_ERROR(rv);
			}
			else
			{
				rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE,
					AQUANTIA_MMD_GLOBAL_REGISTERS,
					AQUANTIA_PROVISIONING_LED0_STATUS,
					AQUANTIA_LINK_ACT_LED_VALUE);
				PHY_RTN_ON_ERROR(rv);
				rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE,
					AQUANTIA_MMD_GLOBAL_REGISTERS,
					AQUANTIA_PROVISIONING_LED1_STATUS,
					AQUANTIA_LINK_LED_VALUE);
				PHY_RTN_ON_ERROR(rv);
			}
			/*add all ability of aq phy*/
			rv = aquantia_phy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_XGE_SPEED_ALL | FAL_PHY_ADV_100TX_FD |
				FAL_PHY_ADV_10T_FD | FAL_PHY_ADV_1000T_FD);
			PHY_RTN_ON_ERROR(rv);
		}
	}

	return rv;
}

static int aquantia_phy_api_ops_init(void)
{
	int ret;
	hsl_phy_ops_t *aquantia_phy_api_ops = NULL;

	aquantia_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (aquantia_phy_api_ops == NULL) {
		SSDK_ERROR("aquantia phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(AQUANTIA_PHY_CHIP);

	aquantia_phy_api_ops->phy_speed_get = aquantia_phy_get_speed;
	aquantia_phy_api_ops->phy_speed_set = aquantia_phy_set_speed;
	aquantia_phy_api_ops->phy_duplex_get = aquantia_phy_get_duplex;
	aquantia_phy_api_ops->phy_duplex_set = aquantia_phy_set_duplex;
	aquantia_phy_api_ops->phy_autoneg_enable_set = qcaphy_c45_autoneg_enable;
	aquantia_phy_api_ops->phy_restart_autoneg = aquantia_phy_restart_autoneg;
	aquantia_phy_api_ops->phy_autoneg_status_get = qcaphy_c45_autoneg_status;
	aquantia_phy_api_ops->phy_autoneg_adv_set = aquantia_phy_set_autoneg_adv;
	aquantia_phy_api_ops->phy_autoneg_adv_get = aquantia_phy_get_autoneg_adv;
#ifndef IN_PORTCONTROL_MINI
	aquantia_phy_api_ops->phy_powersave_set = aquantia_phy_set_powersave;
	aquantia_phy_api_ops->phy_powersave_get = aquantia_phy_get_powersave;
	aquantia_phy_api_ops->phy_8023az_set = aquantia_phy_set_8023az;
	aquantia_phy_api_ops->phy_8023az_get = aquantia_phy_get_8023az;
#endif
	aquantia_phy_api_ops->phy_power_on = qcaphy_c45_poweron;
	aquantia_phy_api_ops->phy_power_off = qcaphy_c45_poweroff;
	aquantia_phy_api_ops->phy_cdt = aquantia_phy_cdt;
	aquantia_phy_api_ops->phy_link_status_get = qcaphy_c45_get_link_status;
#ifndef IN_PORTCONTROL_MINI
	aquantia_phy_api_ops->phy_mdix_set = aquantia_phy_set_mdix;
	aquantia_phy_api_ops->phy_mdix_get = aquantia_phy_get_mdix;
	aquantia_phy_api_ops->phy_mdix_status_get = aquantia_phy_get_mdix_status;
	aquantia_phy_api_ops->phy_local_loopback_set = aquantia_phy_set_local_loopback;
	aquantia_phy_api_ops->phy_local_loopback_get = aquantia_phy_get_local_loopback;
	aquantia_phy_api_ops->phy_remote_loopback_set = aquantia_phy_set_remote_loopback;
	aquantia_phy_api_ops->phy_remote_loopback_get = aquantia_phy_get_remote_loopback;
	aquantia_phy_api_ops->phy_reset = qcaphy_c45_sw_reset;
	aquantia_phy_api_ops->phy_wol_status_set = aquantia_phy_set_wol_status;
	aquantia_phy_api_ops->phy_wol_status_get = aquantia_phy_get_wol_status;
	aquantia_phy_api_ops->phy_magic_frame_mac_get = aquantia_phy_get_magic_frame_mac;
	aquantia_phy_api_ops->phy_magic_frame_mac_set = aquantia_phy_set_magic_frame_mac;
	aquantia_phy_api_ops->phy_intr_mask_set = aquantia_phy_intr_mask_set;
	aquantia_phy_api_ops->phy_intr_mask_get = aquantia_phy_intr_mask_get;
	aquantia_phy_api_ops->phy_id_get = qcaphy_c45_get_phy_id;
#endif
	aquantia_phy_api_ops->phy_interface_mode_set = aquantia_phy_interface_set_mode;
	aquantia_phy_api_ops->phy_interface_mode_status_get=aquantia_phy_interface_get_mode_status;
	aquantia_phy_api_ops->phy_get_status = aquantia_phy_get_status;
#ifndef IN_PORTCONTROL_MINI
	aquantia_phy_api_ops->phy_counter_show = aquantia_phy_show_counter;
#endif
	aquantia_phy_api_ops->phy_eee_adv_set = aquantia_phy_set_eee_adv;
	aquantia_phy_api_ops->phy_eee_adv_get = qcaphy_c45_get_eee_adv;
	aquantia_phy_api_ops->phy_eee_partner_adv_get = qcaphy_c45_get_eee_partner_adv;
	aquantia_phy_api_ops->phy_eee_cap_get = qcaphy_c45_get_eee_cap;
	aquantia_phy_api_ops->phy_eee_status_get = qcaphy_c45_get_eee_status;
	ret = hsl_phy_api_ops_register(AQUANTIA_PHY_CHIP, aquantia_phy_api_ops);
	if (ret == 0)
		SSDK_INFO("qca probe aquantia phy driver succeeded!\n");
	else
		SSDK_ERROR("qca probe aquantia phy driver failed! (code: %d)\n", ret);

	return ret;
}

/******************************************************************************
*
* aquantia_phy_init -
*
*/
int aquantia_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	static a_uint32_t phy_ops_flag = 0;

	if(phy_ops_flag == 0) {
		aquantia_phy_api_ops_init();
		phy_ops_flag = 1;
	}
	aquantia_phy_hw_init(dev_id, port_bmp);

	return 0;

}

