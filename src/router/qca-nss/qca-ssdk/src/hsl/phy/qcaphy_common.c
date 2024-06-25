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

/*qca808x_start*/
#include <linux/phy.h>
#include "sw.h"
#include "hsl_phy.h"
#include "qcaphy_common.h"
#include "ssdk_plat.h"

/*********************the function for GE PHY mii registers*********************/
/*
 * @brief reset phy
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_sw_reset(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	return hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL,
		QCAPHY_CTRL_SOFTWARE_RESET, QCAPHY_CTRL_SOFTWARE_RESET);
}
/*
 * @brief set local loopback
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	fal_port_speed_t cur_speed = 0;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE) {
		rv = qcaphy_get_speed(dev_id, phy_addr, &cur_speed);
		PHY_RTN_ON_ERROR(rv);
		if (cur_speed == FAL_SPEED_1000) {
			phy_data = QCAPHY_1000M_LOOPBACK;
		} else if (cur_speed == FAL_SPEED_100) {
			phy_data = QCAPHY_100M_LOOPBACK;
		} else if (cur_speed == FAL_SPEED_10) {
			phy_data = QCAPHY_10M_LOOPBACK;
		} else {
			return SW_FAIL;
		}
	} else {
		phy_data = QCAPHY_COMMON_CTRL;
	}

	return hsl_phy_mii_reg_write(dev_id, phy_addr, QCAPHY_CONTROL, phy_data);
}
/*
 * @brief get local loopback status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] enable A_TRUE or A_FALSE
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t *enable)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCAPHY_LOCAL_LOOPBACK_ENABLE) {
		*enable = A_TRUE;
	} else {
		*enable = A_FALSE;
	}

	return SW_OK;
}
/*
 * @brief configure the force speed
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed)
{
	a_uint16_t phy_data = 0, mask = 0;
	fal_port_duplex_t cur_duplex = QCAPHY_CTRL_FULL_DUPLEX;
	sw_error_t rv = SW_OK;

	switch(speed)
	{
		case FAL_SPEED_1000:
			rv = qcaphy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_1000T_FD);
			PHY_RTN_ON_ERROR(rv);
			phy_data |= QCAPHY_CTRL_FULL_DUPLEX;
			phy_data |= QCAPHY_CTRL_AUTONEGOTIATION_ENABLE;
			phy_data |= QCAPHY_CTRL_RESTART_AUTONEGOTIATION;
			mask = phy_data;
			break;
		case FAL_SPEED_100:
		case FAL_SPEED_10:
			mask = QCAPHY_CONTROL_SPEED_MASK | QCAPHY_CTRL_FULL_DUPLEX |
				QCAPHY_CTRL_AUTONEGOTIATION_ENABLE;
			if (speed == FAL_SPEED_100) {
				phy_data |= QCAPHY_CONTROL_SPEED_100M;
			} else {
				phy_data |= QCAPHY_CONTROL_SPEED_10M;
			}
			rv = qcaphy_get_duplex(dev_id, phy_addr, &cur_duplex);
			PHY_RTN_ON_ERROR(rv);

			if (cur_duplex == FAL_FULL_DUPLEX) {
				phy_data |= QCAPHY_CTRL_FULL_DUPLEX;
			}
			break;
		default:
			return SW_BAD_PARAM;
	}
	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL, mask, phy_data);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
	if(speed < FAL_SPEED_1000) {
		rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_FALSE, 0);
		PHY_RTN_ON_ERROR(rv);
	}
/*qca808x_start*/

	return SW_OK;
}
/*
 * @brief configure the force speed
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force duplex as full or half
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t duplex)
{
	a_uint16_t phy_data = 0, mask = 0;
	fal_port_speed_t cur_speed = 0;
	sw_error_t rv = SW_OK;

	rv = qcaphy_get_speed(dev_id, phy_addr, &cur_speed);
	PHY_RTN_ON_ERROR(rv);

	switch(cur_speed)
	{
		case FAL_SPEED_1000:
			if (duplex == FAL_FULL_DUPLEX) {
				phy_data |= QCAPHY_CTRL_FULL_DUPLEX;
			} else {
				return SW_NOT_SUPPORTED;
			}
			rv = qcaphy_set_autoneg_adv(dev_id, phy_addr,
				FAL_PHY_ADV_1000T_FD);
			PHY_RTN_ON_ERROR(rv);
			phy_data |= QCAPHY_CTRL_AUTONEGOTIATION_ENABLE;
			phy_data |= QCAPHY_CTRL_RESTART_AUTONEGOTIATION;
			mask = phy_data;
			break;
		case FAL_SPEED_100:
		case FAL_SPEED_10:
			mask = QCAPHY_CONTROL_SPEED_MASK | QCAPHY_CTRL_AUTONEGOTIATION_ENABLE
				| QCAPHY_CTRL_FULL_DUPLEX;
			if (cur_speed == FAL_SPEED_100) {
				phy_data |= QCAPHY_CONTROL_SPEED_100M;
			} else {
				phy_data |= QCAPHY_CONTROL_SPEED_10M;
			}
			if (duplex == FAL_FULL_DUPLEX) {
				phy_data |= QCAPHY_CTRL_FULL_DUPLEX;
			}
			break;
		default:
			return SW_FAIL;
	}
	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL, mask,
		phy_data);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
	if(cur_speed < FAL_SPEED_1000) {
		rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_FALSE, 0);
		PHY_RTN_ON_ERROR(rv);
	}
/*qca808x_start*/

	return SW_OK;
}
/*
 * @brief enable autoneg
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_autoneg_enable(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL,
		QCAPHY_CTRL_AUTONEGOTIATION_ENABLE, QCAPHY_CTRL_AUTONEGOTIATION_ENABLE);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
	rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, 0);
/*qca808x_start*/

	return rv;
}
/*
 * @brief get the phy autoneg status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
a_bool_t
qcaphy_autoneg_status(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_CONTROL);

	if (phy_data & QCAPHY_CTRL_AUTONEGOTIATION_ENABLE) {
		return A_TRUE;
	}

	return A_FALSE;
}
/*
 * @brief power off the phy
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	return hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL,
		QCAPHY_CTRL_POWER_MASK, QCAPHY_CTRL_POWER_DOWN);
}
/*
 * @brief power on the phy
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_poweron(a_uint32_t dev_id, a_uint32_t phy_addr)
{

	return hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL,
		QCAPHY_CTRL_POWER_MASK, QCAPHY_CTRL_POWER_UP);
}
/*
 * @brief restart autoneg
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_autoneg_restart(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_CONTROL,
		QCAPHY_CTRL_AUTONEGOTIATION_ENABLE | QCAPHY_CTRL_RESTART_AUTONEGOTIATION,
		QCAPHY_CTRL_AUTONEGOTIATION_ENABLE | QCAPHY_CTRL_RESTART_AUTONEGOTIATION);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
	rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, 0);
/*qca808x_start*/

	return rv;
}
/*
 * @brief get phy support ability
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] ability support ability
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_capability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap)
{
	a_uint16_t phy_data = 0;

	*cap = 0;
	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCAPHY_STATUS_AUTONEG_CAPS) {
		*cap |= FAL_PHY_ADV_AUTONEG;
	}
	if (phy_data & QCAPHY_STATUS_10T_HD_CAPS) {
		*cap |= FAL_PHY_ADV_10T_HD;
	}
	if (phy_data & QCAPHY_STATUS_10T_FD_CAPS) {
		*cap |= FAL_PHY_ADV_10T_FD;
	}
	if (phy_data & QCAPHY_STATUS_100TX_HD_CAPS) {
		*cap |= FAL_PHY_ADV_100TX_HD;
	}
	if (phy_data & QCAPHY_STATUS_100TX_FD_CAPS) {
		*cap |= FAL_PHY_ADV_100TX_FD;
	}
	if (phy_data & QCAPHY_STATUS_EXTENDED_STATUS) {
		phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr,
			QCAPHY_EXTENDED_STATUS);
		PHY_RTN_ON_READ_ERROR(phy_data);
		if (phy_data & QCAPHY_STATUS_1000T_FD_CAPS)
			*cap |= FAL_PHY_ADV_1000T_FD;
	}
	*cap |= (FAL_PHY_ADV_PAUSE | FAL_PHY_ADV_ASY_PAUSE);

	return SW_OK;
}
/*
 * @brief get phy id
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] phy_id phy id
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_phy_id(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *phy_id)
{
	a_uint16_t org_id = 0, rev_id = 0;
	org_id = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_ID1);
	PHY_RTN_ON_READ_ERROR(org_id);

	rev_id = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_ID2);
	PHY_RTN_ON_READ_ERROR(rev_id);

	*phy_id = ((org_id & 0xffff) << 16) | (rev_id & 0xffff);

	return SW_OK;
}
/*
 * @brief set phy autoadv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] autoneg_adv auto-negotiation adv bitmap
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg_adv)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if(!hsl_phy_autoneg_adv_check(dev_id, phy_addr, autoneg_adv))
		return SW_NOT_SUPPORTED;

	if (autoneg_adv & FAL_PHY_ADV_100TX_FD) {
		phy_data |= QCAPHY_ADVERTISE_100FULL;
	}

	if (autoneg_adv & FAL_PHY_ADV_100TX_HD) {
		phy_data |= QCAPHY_ADVERTISE_100HALF;
	}

	if (autoneg_adv & FAL_PHY_ADV_10T_FD) {
		phy_data |= QCAPHY_ADVERTISE_10FULL;
	}

	if (autoneg_adv & FAL_PHY_ADV_10T_HD) {
		phy_data |= QCAPHY_ADVERTISE_10HALF;
	}

	if (autoneg_adv & FAL_PHY_ADV_PAUSE) {
		phy_data |= QCAPHY_ADVERTISE_PAUSE;
	}

	if (autoneg_adv & FAL_PHY_ADV_ASY_PAUSE) {
		phy_data |= QCAPHY_ADVERTISE_ASYM_PAUSE;
	}
	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_AUTONEG_ADVERT,
		QCAPHY_ADVERTISE_MEGA_ALL, phy_data);

	phy_data = 0;
	if (autoneg_adv & FAL_PHY_ADV_1000T_FD) {
		phy_data |= QCAPHY_ADVERTISE_1000FULL;
	}
	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_1000BASET_CONTROL,
		QCAPHY_ADVERTISE_1000FULL, phy_data);
	PHY_RTN_ON_ERROR(rv);

/*qca808x_end*/
	rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, autoneg_adv);
/*qca808x_start*/

	return rv;
}
/*
 * @brief set phy autoadv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] autoneg_adv auto-negotiation adv bitmap
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *autoneg_adv)
{
	a_uint16_t phy_data = 0;

	*autoneg_adv = 0;
	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_AUTONEG_ADVERT);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCAPHY_ADVERTISE_100FULL) {
		*autoneg_adv |= FAL_PHY_ADV_100TX_FD;
	}

	if (phy_data & QCAPHY_ADVERTISE_100HALF) {
		*autoneg_adv |= FAL_PHY_ADV_100TX_HD;
	}

	if (phy_data & QCAPHY_ADVERTISE_10FULL) {
		*autoneg_adv |= FAL_PHY_ADV_10T_FD;
	}

	if (phy_data & QCAPHY_ADVERTISE_10HALF) {
		*autoneg_adv |= FAL_PHY_ADV_10T_HD;
	}

	if (phy_data & QCAPHY_ADVERTISE_PAUSE) {
		*autoneg_adv |= FAL_PHY_ADV_PAUSE;
	}

	if (phy_data & QCAPHY_ADVERTISE_ASYM_PAUSE) {
		*autoneg_adv |= FAL_PHY_ADV_ASY_PAUSE;
	}

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_1000BASET_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCAPHY_ADVERTISE_1000FULL) {
		*autoneg_adv |= FAL_PHY_ADV_1000T_FD;
	}

	return SW_OK;
}
/*
 * @brief get link partner ability
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] link partner ability bitmap
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_lp_capability_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap)
{
	a_uint16_t phy_data = 0;

	*cap = 0;
	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_LINK_PARTNER_ABILITY);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if(phy_data & QCAPHY_LINK_10BASETX_HALF_DUPLEX)
		*cap |= FAL_PHY_ADV_10T_HD;

	if(phy_data & QCAPHY_LINK_10BASETX_FULL_DUPLEX)
		*cap |= FAL_PHY_ADV_10T_FD;

	if(phy_data & QCAPHY_LINK_100BASETX_HALF_DUPLEX)
		*cap |= FAL_PHY_ADV_100TX_HD;

	if(phy_data & QCAPHY_LINK_100BASETX_FULL_DUPLEX)
		*cap |= FAL_PHY_ADV_100TX_FD;

	if(phy_data & QCAPHY_LINK_PAUSE)
		*cap |= FAL_PHY_ADV_PAUSE;

	if(phy_data & QCAPHY_LINK_ASYPAUSE)
		*cap |= FAL_PHY_ADV_ASY_PAUSE;

	if(phy_data & QCAPHY_LINK_LPACK)
		*cap |= FAL_PHY_ADV_AUTONEG;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_1000BASET_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(phy_data & QCAPHY_LINK_1000BASETX_FULL_DUPLEX)
		*cap |= FAL_PHY_ADV_1000T_FD;

	return SW_OK;
}
/*
 * @brief set phy mdix mode
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] mdix mode
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t mode)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (mode == PHY_MDIX_AUTO) {
		phy_data = QCAPHY_MDIX_AUTO;
	} else if (mode == PHY_MDIX_MDIX) {
		phy_data = QCAPHY_MDIX;
	} else if (mode == PHY_MDIX_MDI) {
		phy_data = QCAPHY_MDI;
	} else {
		return SW_BAD_PARAM;
	}

	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCAPHY_SPEC_CONTROL,
		QCAPHY_MDIX_AUTO, phy_data);
	PHY_RTN_ON_ERROR(rv);

	return qcaphy_sw_reset(dev_id, phy_addr);
}
/*
 * @brief get phy mdix mode
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] mdix mode
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t * mode)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_SPEC_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if ((phy_data & QCAPHY_MDIX_AUTO) == QCAPHY_MDIX_AUTO) {
		*mode = PHY_MDIX_AUTO;
	} else if ((phy_data & QCAPHY_MDIX_AUTO) == QCAPHY_MDIX) {
		*mode = PHY_MDIX_MDIX;
	} else {
		*mode = PHY_MDIX_MDI;
	}

	return SW_OK;

}
/*
 * @brief get phy mdix mode status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] mdix mode
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_mdix_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_status_t * mode)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_SPEC_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*mode = (phy_data & QCAPHY_MDIX_STATUS) ? PHY_MDIX_STATUS_MDIX :
		PHY_MDIX_STATUS_MDI;

	return SW_OK;

}
/*
 * @brief get phy status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] phy_status phy status
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_status_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct port_phy_status *phy_status)
{
	a_uint16_t phy_data = 0;
/*qca808x_end*/
	a_uint32_t new_adv = 0, old_adv = 0, phy_id = 0;
	struct phy_device *phydev = NULL;
	phy_type_t phy_type = MAX_PHY_CHIP;
	sw_error_t rv = SW_OK;

	rv = hsl_phy_phydev_get(dev_id, phy_addr, &phydev);
	if (rv == SW_OK) {
		SSDK_DEBUG("phy_addr:0x%x, phydev->drv->name:%s", phy_addr,
				phydev->drv->name);
		if(phydev->drv && !strcmp(phydev->drv->name, "Generic PHY") &&
				(phydev->autoneg == AUTONEG_ENABLE)) {
			rv = hsl_phy_linkmode_adv_to_adv(phydev->advertising, &old_adv);
			PHY_RTN_ON_ERROR (rv);
			rv = qcaphy_get_autoneg_adv(dev_id, phy_addr, &new_adv);
			PHY_RTN_ON_ERROR(rv);
			if(new_adv != old_adv) {
				rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr,
						A_TRUE, new_adv);
				PHY_RTN_ON_ERROR(rv);
			}
		}
	}
/*qca808x_start*/
	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCAPHY_SPEC_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	/*get phy link status*/
	if (phy_data & QCAPHY_STATUS_LINK_PASS) {
		phy_status->link_status = PORT_LINK_UP;
	} else {
		phy_status->link_status = PORT_LINK_DOWN;
		return SW_OK;
	}
/*qca808x_end*/
	/*get phy speed*/
	rv = qcaphy_get_phy_id(dev_id, phy_addr, &phy_id);
	PHY_RTN_ON_ERROR(rv);
	phy_type = hsl_phytype_get_by_phyid(dev_id, phy_id);
	if(phy_type == QCA808X_PHY_CHIP || phy_type == MPGE_PHY_CHIP) {
/*qca808x_start*/
		switch (phy_data & QCAPHY_STATUS_SPEED_MASK) {
			case QCAPHY_STATUS_SPEED_2500MBS:
				phy_status->speed = FAL_SPEED_2500;
				break;
			case QCAPHY_STATUS_SPEED_1000MBS:
				phy_status->speed = FAL_SPEED_1000;
				break;
			case QCAPHY_STATUS_SPEED_100MBS:
				phy_status->speed = FAL_SPEED_100;
				break;
			case QCAPHY_STATUS_SPEED_10MBS:
				phy_status->speed = FAL_SPEED_10;
				break;
			default:
				return SW_READ_ERROR;
		}
/*qca808x_end*/
	} else if(phy_type == F1_PHY_CHIP || MALIBU_PHY_CHIP ||
		phy_type == QCA803X_PHY_CHIP) {
		switch (phy_data & QCAPHY_STATUS_SPEED_MASK_1) {
			case QCAPHY_STATUS_SPEED_1000MBS_1:
				phy_status->speed = FAL_SPEED_1000;
				break;
			case QCAPHY_STATUS_SPEED_100MBS_1:
				phy_status->speed = FAL_SPEED_100;
				break;
			case QCAPHY_STATUS_SPEED_10MBS:
				phy_status->speed = FAL_SPEED_10;
				break;
			default:
				return SW_READ_ERROR;
		}
	}
/*qca808x_start*/
	/*get phy duplex*/
	if (phy_data & QCAPHY_STATUS_FULL_DUPLEX) {
		phy_status->duplex = FAL_FULL_DUPLEX;
	} else {
		phy_status->duplex = FAL_HALF_DUPLEX;
	}
	/* get phy flowctrl resolution status */
	if (phy_data & QCAPHY_RX_FLOWCTRL_STATUS) {
		phy_status->rx_flowctrl = A_TRUE;
	} else {
		phy_status->rx_flowctrl = A_FALSE;
	}
	if (phy_data & QCAPHY_TX_FLOWCTRL_STATUS) {
		phy_status->tx_flowctrl = A_TRUE;
	} else {
		phy_status->tx_flowctrl = A_FALSE;
	}

	return SW_OK;
}
/*
 * @brief get phy speed
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return A_TRUE for link up, A_FALS for link down
 */
a_bool_t
qcaphy_get_link_status(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	struct port_phy_status phy_status = {0};
	sw_error_t rv = SW_OK;

	rv = qcaphy_status_get(dev_id, phy_addr, &phy_status);

	if(rv == SW_OK && phy_status.link_status == PORT_LINK_UP)
		return A_TRUE;
	else
		return A_FALSE;
}
/*
 * @brief get phy speed
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t * speed)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = qcaphy_status_get(dev_id, phy_addr, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.link_status == PORT_LINK_UP) {
		*speed = phy_status.speed;
	} else {
		*speed = FAL_SPEED_10;
	}

	return SW_OK;
}
/*
 * @brief get phy duplex
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t * duplex)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = qcaphy_status_get(dev_id, phy_addr, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.link_status == PORT_LINK_UP) {
		*duplex = phy_status.duplex;
	} else {
		*duplex = FAL_HALF_DUPLEX;
	}

	return SW_OK;
}
/*
 * @brief set eee adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] eee adv
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (adv & FAL_PHY_EEE_100BASE_T) {
		phy_data |= QCAPHY_EEE_ADV_100M;
	}
	if (adv & FAL_PHY_EEE_1000BASE_T) {
		phy_data |= QCAPHY_EEE_ADV_1000M;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_8023AZ_EEE_CTRL, QCAPHY_EEE_MASK, phy_data);
	PHY_RTN_ON_ERROR(rv);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
	rv = hsl_phydev_eee_update(dev_id, phy_addr, adv);
	PHY_RTN_ON_ERROR(rv);
#endif

	return qcaphy_autoneg_restart(dev_id, phy_addr);
}

/******************************************************************************
 * @brief get eee adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] eee adv
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	a_uint16_t phy_data = 0;

	*adv = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_CTRL);

	if (phy_data & QCAPHY_EEE_ADV_100M) {
		*adv |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCAPHY_EEE_ADV_1000M) {
		*adv |= FAL_PHY_EEE_1000BASE_T;
	}

	return SW_OK;
}
/******************************************************************************
 * @brief get link partner eee adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] link partner eee adv
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	a_uint16_t phy_data = 0;

	*adv = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_PARTNER);

	if (phy_data & QCAPHY_EEE_PARTNER_ADV_100M) {
		*adv |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCAPHY_EEE_PARTNER_ADV_1000M) {
		*adv |= FAL_PHY_EEE_1000BASE_T;
	}

	return SW_OK;
}
/******************************************************************************
 * @brief get eee capability
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] eee capability
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap)
{
	a_uint16_t phy_data = 0;

	*cap = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE,
		QCAPHY_MMD3_NUM, QCAPHY_MMD3_8023AZ_EEE_CAPABILITY);

	if (phy_data & QCAPHY_EEE_CAPABILITY_100M) {
		*cap |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCAPHY_EEE_CAPABILITY_1000M) {
		*cap |= FAL_PHY_EEE_1000BASE_T;
	}

	return SW_OK;
}
/******************************************************************************
 * @brief get eee status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] eee status
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *status)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*status = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_STATUS);

	if (phy_data & QCAPHY_EEE_STATUS_100M) {
		*status |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCAPHY_EEE_STATUS_1000M) {
		*status |= FAL_PHY_EEE_1000BASE_T;
	}

	return rv;
}

/******************************************************************************
 * @brief set 8023 az
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] enable
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint32_t eee_adv = 0;

	if (enable == A_TRUE)
		eee_adv = (QCAPHY_EEE_ADV_100M | QCAPHY_EEE_ADV_1000M);
	return qcaphy_set_eee_adv(dev_id, phy_addr, eee_adv);
}

/******************************************************************************
 * @brief get 8023 az
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] enable
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	a_uint32_t eee_adv = 0;
	sw_error_t rv = SW_OK;

	*enable = A_FALSE;

	rv = qcaphy_get_eee_adv(dev_id, phy_addr, &eee_adv);
	PHY_RTN_ON_ERROR(rv);
	if ((eee_adv & QCAPHY_EEE_ADV_100M) &&
		(eee_adv & QCAPHY_EEE_ADV_1000M))
		*enable = A_TRUE;

	return SW_OK;
}

sw_error_t
qcaphy_led_pattern_map_from_phy(a_uint32_t dev_id, a_uint32_t *map,
	a_uint16_t phy_data)
{
	if(phy_data & QCAPHY_PHY_LINK_1000M_LIGHT_EN)
		*map |= BIT(LINK_1000M_LIGHT_EN);
	if(phy_data & QCAPHY_PHY_LINK_100M_LIGHT_EN)
		*map |= BIT(LINK_100M_LIGHT_EN);
	if(phy_data & QCAPHY_PHY_LINK_10M_LIGHT_EN)
		*map |= BIT(LINK_10M_LIGHT_EN);
	if (phy_data & QCAPHY_PHY_RX_TRAFFIC_BLINK_EN)
		*map |= BIT(RX_TRAFFIC_BLINK_EN);
	if (phy_data & QCAPHY_PHY_TX_TRAFFIC_BLINK_EN)
		*map |= BIT(TX_TRAFFIC_BLINK_EN);

	return SW_OK;
}

sw_error_t
qcaphy_led_pattern_map_to_phy(a_uint32_t dev_id, a_uint32_t map,
	a_uint16_t *phy_data)
{
	if (map & BIT(LINK_1000M_LIGHT_EN))
		*phy_data |=  QCAPHY_PHY_LINK_1000M_LIGHT_EN;
	if (map & BIT(LINK_100M_LIGHT_EN))
		*phy_data |=  QCAPHY_PHY_LINK_100M_LIGHT_EN;
	if (map & BIT(LINK_10M_LIGHT_EN))
		*phy_data |=  QCAPHY_PHY_LINK_10M_LIGHT_EN;
	if (map & BIT(RX_TRAFFIC_BLINK_EN))
		*phy_data |=  QCAPHY_PHY_RX_TRAFFIC_BLINK_EN;
	if (map & BIT(TX_TRAFFIC_BLINK_EN))
		*phy_data |=  QCAPHY_PHY_TX_TRAFFIC_BLINK_EN;

	return SW_OK;
}

sw_error_t
qcaphy_led_pattern_force_from_phy(a_uint32_t dev_id, a_uint32_t *force_mode,
	a_uint16_t phy_data)
{
	if(!(phy_data & QCAPHY_PHY_LED_FORCE_EN))
		return SW_BAD_PARAM;

	switch (phy_data & QCAPHY_PHY_LED_FORCE_MASK) {
		case QCAPHY_PHY_LED_FORCE_ALWAYS_OFF:
			*force_mode = LED_ALWAYS_OFF;
			break;
		case QCAPHY_PHY_LED_FORCE_ALWAYS_ON:
			*force_mode = LED_ALWAYS_ON;
			break;
		case QCAPHY_PHY_LED_FORCE_ALWAYS_BLINK:
			*force_mode = LED_ALWAYS_BLINK;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t
qcaphy_led_pattern_force_to_phy(a_uint32_t dev_id, a_uint32_t force_mode,
	a_uint16_t *phy_data)
{
	*phy_data |= QCAPHY_PHY_LED_FORCE_EN;

	if (force_mode == LED_ALWAYS_OFF)
		*phy_data |= QCAPHY_PHY_LED_FORCE_ALWAYS_OFF;
	else if (force_mode == LED_ALWAYS_ON)
		*phy_data |= QCAPHY_PHY_LED_FORCE_ALWAYS_ON;
	else if (force_mode == LED_ALWAYS_BLINK)
		*phy_data |= QCAPHY_PHY_LED_FORCE_ALWAYS_BLINK;
	else
		return SW_NOT_SUPPORTED;

	return SW_OK;
}

sw_error_t
qcaphy_led_active_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t active_level)
{
	a_uint16_t phy_data = 0;

	if(active_level == LED_ACTIVE_HIGH)
		phy_data |= QCAPHY_MMD7_LED_POLARITY_MASK;

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_LED_POLARITY_CTRL, QCAPHY_MMD7_LED_POLARITY_MASK,
		phy_data);
}

sw_error_t
qcaphy_led_active_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *active_level)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_LED_POLARITY_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(phy_data & QCAPHY_MMD7_LED_POLARITY_MASK)
	{
		*active_level = LED_ACTIVE_HIGH;
	}

	return SW_OK;
}

sw_error_t
qcaphy_led_blink_freq_set(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mode,
	a_uint32_t freq)
{
	a_uint16_t blink_freq = 0, blink_freq_mask = 0;

	if(mode == LED_ALWAYS_BLINK) {
		blink_freq_mask = QCAPHY_PHY_ALWAYS_BLINK_FREQ_MASK;
		blink_freq = freq << 3;
	} else if(mode == LED_PATTERN_MAP_EN) {
		blink_freq_mask = QCAPHY_PHY_MAP_BLINK_FREQ_MASK;
		blink_freq = freq << 9;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_LED_BLINK_FREQ_CTRL, blink_freq_mask,
		blink_freq);
}

sw_error_t
qcaphy_led_blink_freq_get(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mode,
	a_uint32_t *freq)
{
	a_uint16_t phy_data = 0;

	phy_data =  hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_LED_BLINK_FREQ_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(mode == LED_ALWAYS_BLINK)
		*freq = ((phy_data & QCAPHY_PHY_ALWAYS_BLINK_FREQ_MASK) >> 3);
	else if(mode == LED_PATTERN_MAP_EN) {
		*freq = ((phy_data & QCAPHY_PHY_MAP_BLINK_FREQ_MASK) >> 9);
	}

	return SW_OK;
}
/*qca808x_end*/
