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

#include "sw.h"
#include "hsl_phy.h"
#include "qcaphy_c45_common.h"

/*
 * @brief get the phy autoneg status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
a_bool_t
qcaphy_c45_autoneg_status(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_CONTROL1);
	if (phy_data & QCAPHY_AN_AUTONEG_EN)
		return A_TRUE;

	return A_FALSE;
}
/*
 * @brief restart autoneg
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_autoneg_restart(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_CONTROL1,
		QCAPHY_AN_AUTONEG_EN | QCAPHY_AN_AUTONEG_RESTART,
		QCAPHY_AN_AUTONEG_EN | QCAPHY_AN_AUTONEG_RESTART);
	SW_RTN_ON_ERROR(rv);

	return hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, 0);
}
/*
 * @brief set phy autoneg enable/disable
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_autoneg_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if(enable)
		phy_data |= QCAPHY_AN_AUTONEG_EN;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_CONTROL1, QCAPHY_AN_AUTONEG_EN, phy_data);
	SW_RTN_ON_ERROR(rv);

	return hsl_phy_phydev_autoneg_update(dev_id, phy_addr, enable, 0);
}
/*
 * @brief enable autoneg
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_autoneg_enable(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	return qcaphy_c45_autoneg_set(dev_id, phy_addr, A_TRUE);
}
/*
 * @brief set autoneg adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (autoneg & FAL_PHY_ADV_10T_HD)
		phy_data |= QCAPHY_AN_ADVERTISE_10HALF;
	if (autoneg & FAL_PHY_ADV_10T_FD)
		phy_data |= QCAPHY_AN_ADVERTISE_10FULL;
	if (autoneg & FAL_PHY_ADV_100TX_HD)
		phy_data |= QCAPHY_AN_ADVERTISE_100HALF;
	if (autoneg & FAL_PHY_ADV_100TX_FD)
		phy_data |= QCAPHY_AN_ADVERTISE_100FULL;
	if (autoneg & FAL_PHY_ADV_PAUSE)
		phy_data |= QCAPHY_AN_ADVERTISE_PAUSE;
	if (autoneg & FAL_PHY_ADV_ASY_PAUSE)
		phy_data |= QCAPHY_AN_ADVERTISE_ASM_PAUSE;
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_ADV, QCAPHY_AN_MEGA_ADVERTISE, phy_data);
	SW_RTN_ON_ERROR(rv);

	phy_data = 0;
	if (autoneg & FAL_PHY_ADV_2500T_FD)
		phy_data |= QCAPHY_AN_ADVERTISE_2500FULL;
	if (autoneg & FAL_PHY_ADV_5000T_FD)
		phy_data |= QCAPHY_AN_ADVERTISE_5000FULL;
	if (autoneg & FAL_PHY_ADV_10000T_FD)
		phy_data |= QCAPHY_AN_ADVERTISE_10000FULL;
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_10G_ADV, QCAPHY_AN_GIGA_PLUS_ALL, phy_data);
	SW_RTN_ON_ERROR(rv);

	return hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, autoneg);
}
/*
 * @brief get autoneg adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * autoneg)
{
	a_uint16_t phy_data = 0;

	*autoneg = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_ADV);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCAPHY_AN_ADVERTISE_10HALF)
		*autoneg |= FAL_PHY_ADV_10T_HD;
	if (phy_data & QCAPHY_AN_ADVERTISE_10FULL)
		*autoneg |= FAL_PHY_ADV_10T_FD;
	if (phy_data & QCAPHY_AN_ADVERTISE_100HALF)
		*autoneg |= FAL_PHY_ADV_100TX_HD;
	if (phy_data & QCAPHY_AN_ADVERTISE_100FULL)
		*autoneg |= FAL_PHY_ADV_100TX_FD;
	if (phy_data & QCAPHY_AN_ADVERTISE_PAUSE)
		*autoneg |= FAL_PHY_ADV_PAUSE;
	if (phy_data & QCAPHY_AN_ADVERTISE_ASM_PAUSE)
		*autoneg |= FAL_PHY_ADV_ASY_PAUSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_10G_ADV);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCAPHY_AN_ADVERTISE_2500FULL)
		*autoneg |= FAL_PHY_ADV_2500T_FD;
	if (phy_data & QCAPHY_AN_ADVERTISE_5000FULL)
		*autoneg |= FAL_PHY_ADV_5000T_FD;
	if (phy_data & QCAPHY_AN_ADVERTISE_10000FULL)
		*autoneg |= FAL_PHY_ADV_10000T_FD;

	return SW_OK;
}
/*
 * @brief get lp autoneg adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_get_partner_ability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * ability)
{
	a_uint16_t phy_data = 0;

	*ability = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_LP_ADV);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_10HALF)
		*ability |= FAL_PHY_ADV_10T_HD;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_10FULL)
		*ability |= FAL_PHY_ADV_10T_FD;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_100HALF)
		*ability |= FAL_PHY_ADV_100TX_HD;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_100FULL)
		*ability |= FAL_PHY_ADV_100TX_FD;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_PAUSE)
		*ability |= FAL_PHY_ADV_PAUSE;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_ASM_PAUSE)
		*ability |= FAL_PHY_ADV_ASY_PAUSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_LP_10G_ADV);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_2500FULL)
		*ability |= FAL_PHY_ADV_2500T_FD;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_5000FULL)
		*ability |= FAL_PHY_ADV_5000T_FD;
	if (phy_data & QCAPHY_AN_LP_ADVERTISE_10000FULL)
		*ability |= FAL_PHY_ADV_10000T_FD;

	return SW_OK;
}
/*
 * @brief get the link status
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force speed
 * @return SW_OK or error code
 */
a_bool_t
qcaphy_c45_get_link_status(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t phy_data = 0;

	/*get phy link status, in order to get the  link status of real time,
	  need to read the link status two times */
	phy_data= hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_STATUS);
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_AN_STATUS);
	if (phy_data & QCAPHY_AN_LINK_STATUS)
		return A_TRUE;
	else
		return A_FALSE;
}
/*
 * @brief poweron the phy
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_poweron(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_MMD31_CONTROL, QCAPHY_POWER_DOWN, 0);
}
/*
 * @brief poweroff the phy
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_MMD31_CONTROL, QCAPHY_POWER_DOWN, QCAPHY_POWER_DOWN);
}
/*
 * @brief reset the phy
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_sw_reset(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_MMD31_CONTROL, QCAPHY_CTRL_SOFTWARE_RESET,
		QCAPHY_CTRL_SOFTWARE_RESET);
}
/*
 * @brief get phy id
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] phy_id phy id
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_get_phy_id(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *phy_id)
{
	a_uint16_t org_id = 0, rev_id = 0;

	org_id= hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD7_NUM, QCAPHY_ID1);
	PHY_RTN_ON_READ_ERROR(org_id);
	rev_id = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD7_NUM, QCAPHY_ID2);
	PHY_RTN_ON_READ_ERROR(rev_id);

	*phy_id = ((org_id & 0xffff) << 16) | (rev_id & 0xffff);

	return SW_OK;
}
/******************************************************************************
 * @brief set eee adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] eee adv
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (adv & FAL_PHY_EEE_100BASE_T)
		phy_data |= QCAPHY_EEE_ADV_100M;
	if (adv & FAL_PHY_EEE_1000BASE_T)
		phy_data |= QCAPHY_EEE_ADV_1000M;
	if (adv & FAL_PHY_EEE_10000BASE_T)
		phy_data |= QCAPHY_EEE_ADV_10000M;
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_8023AZ_EEE_CTRL, QCAPHY_EEE_MASK, phy_data);
	SW_RTN_ON_ERROR(rv);

	phy_data = 0;
	if (adv & FAL_PHY_EEE_2500BASE_T)
		phy_data |= QCAPHY_EEE_ADV_2500M;
	if (adv & FAL_PHY_EEE_5000BASE_T)
		phy_data |= QCAPHY_EEE_ADV_5000M;
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCAPHY_MMD7_8023AZ_EEE_CTRL1, QCAPHY_EEE_MASK1, phy_data);
	SW_RTN_ON_ERROR(rv);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0))
	rv = hsl_phydev_eee_update(dev_id, phy_addr, adv);
	SW_RTN_ON_ERROR(rv);
#endif
	return qcaphy_c45_autoneg_restart(dev_id, phy_addr);
}
/******************************************************************************
 * @brief get eee adv
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] eee adv
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	a_uint16_t phy_data = 0;

	*adv = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_CTRL);
	if (phy_data & QCAPHY_EEE_ADV_100M)
		*adv |= FAL_PHY_EEE_100BASE_T;
	if (phy_data & QCAPHY_EEE_ADV_1000M)
		*adv |= FAL_PHY_EEE_1000BASE_T;
	if (phy_data & QCAPHY_EEE_ADV_10000M)
		*adv |= FAL_PHY_EEE_10000BASE_T;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_CTRL1);
	if (phy_data & QCAPHY_EEE_ADV_2500M)
		*adv |= FAL_PHY_EEE_2500BASE_T;
	if (phy_data & QCAPHY_EEE_ADV_5000M)
		*adv |= FAL_PHY_EEE_5000BASE_T;

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
qcaphy_c45_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	a_uint16_t phy_data = 0;

	*adv = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_PARTNER);
	if (phy_data & QCAPHY_EEE_PARTNER_ADV_100M)
		*adv |= FAL_PHY_EEE_100BASE_T;
	if (phy_data & QCAPHY_EEE_PARTNER_ADV_1000M)
		*adv |= FAL_PHY_EEE_1000BASE_T;
	if (phy_data & QCAPHY_EEE_PARTNER_ADV_10000M)
		*adv |= FAL_PHY_EEE_10000BASE_T;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD7_NUM, QCAPHY_MMD7_8023AZ_EEE_PARTNER1);
	if (phy_data & QCAPHY_EEE_PARTNER_ADV_2500M)
		*adv |= FAL_PHY_EEE_2500BASE_T;
	if (phy_data & QCAPHY_EEE_PARTNER_ADV_5000M)
		*adv |= FAL_PHY_EEE_5000BASE_T;

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
qcaphy_c45_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap)
{
	a_uint16_t phy_data = 0;

	*cap = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD3_NUM, QCAPHY_MMD3_8023AZ_EEE_CAPABILITY);
	if (phy_data & QCAPHY_EEE_CAPABILITY_100M)
		*cap |= FAL_PHY_EEE_100BASE_T;
	if (phy_data & QCAPHY_EEE_CAPABILITY_1000M)
		*cap |= FAL_PHY_EEE_1000BASE_T;
	if (phy_data & QCAPHY_EEE_CAPABILITY_10000M)
		*cap |= FAL_PHY_EEE_10000BASE_T;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD3_NUM, QCAPHY_MMD3_8023AZ_EEE_CAPABILITY1);
	if (phy_data & QCAPHY_EEE_CAPABILITY_2500M)
		*cap |= FAL_PHY_EEE_2500BASE_T;
	if (phy_data & QCAPHY_EEE_CAPABILITY_5000M)
		*cap |= FAL_PHY_EEE_5000BASE_T;

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
qcaphy_c45_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *status)
{
	sw_error_t rv = SW_OK;
	a_uint32_t adv = 0, lp_adv = 0;

	rv = qcaphy_get_eee_adv(dev_id, phy_addr, &adv);
	SW_RTN_ON_ERROR(rv);

	rv = qcaphy_get_eee_partner_adv(dev_id, phy_addr, &lp_adv);
	SW_RTN_ON_ERROR(rv);

	*status = (adv & lp_adv);

	return SW_OK;
}
/******************************************************************************
 * @brief set 8023 az
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] enable
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint32_t eee_adv = 0;

	if (enable == A_TRUE)
		eee_adv = FAL_PHY_EEE_ALL_ADV;
	return qcaphy_c45_set_eee_adv(dev_id, phy_addr, eee_adv);
}

/******************************************************************************
 * @brief get 8023 az
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[out] enable
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	a_uint32_t eee_adv = 0;
	sw_error_t rv = SW_OK;

	*enable = A_FALSE;

	rv = qcaphy_get_eee_adv(dev_id, phy_addr, &eee_adv);
	PHY_RTN_ON_ERROR(rv);
	if (eee_adv && FAL_PHY_EEE_ALL_ADV)
		*enable = A_TRUE;

	return SW_OK;
}
/*
 * @brief configure the force speed for C45 PHY
 * @param[in] dev_id device id
 * @param[in] phy_addr phy address
 * @param[in] speed force speed
 * @return SW_OK or error code
 */
sw_error_t
qcaphy_c45_force_speed_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed)
{
	a_uint16_t phy_speed_ctrl = 0, phy_speed_type = 0;
	sw_error_t rv = SW_OK;

	switch(speed)
	{
		case FAL_SPEED_10:
			phy_speed_ctrl = QCAPHY_PMA_CONTROL_10M;
			phy_speed_type = QCAPHY_PMA_TYPE_10M;
			break;
		case FAL_SPEED_100:
			phy_speed_ctrl = QCAPHY_PMA_CONTROL_100M;
			phy_speed_type = QCAPHY_PMA_TYPE_100M;
			break;
		case FAL_SPEED_1000:
			phy_speed_ctrl = QCAPHY_PMA_CONTROL_1000M;
			phy_speed_type = QCAPHY_PMA_TYPE_1000M;
			break;
		case FAL_SPEED_2500:
			phy_speed_ctrl = QCAPHY_PMA_CONTROL_2500M;
			phy_speed_type = QCAPHY_PMA_TYPE_2500M;
			break;
		case FAL_SPEED_5000:
			phy_speed_ctrl = QCAPHY_PMA_CONTROL_5000M;
			phy_speed_type = QCAPHY_PMA_TYPE_5000M;
			break;
		case FAL_SPEED_10000:
			phy_speed_ctrl = QCAPHY_PMA_CONTROL_10000M;
			phy_speed_type = QCAPHY_PMA_TYPE_10000M;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD1_NUM,
		QCAPHY_MMD1_PMA_CONTROL, QCAPHY_PMA_SPEED_MASK, phy_speed_ctrl);
	SW_RTN_ON_ERROR(rv);

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD1_NUM,
		QCAPHY_MMD1_PMA_TTYPE, QCAPHY_PMA_TYPE_MASK, phy_speed_type);
	SW_RTN_ON_ERROR(rv);

	return qcaphy_c45_autoneg_set(dev_id, phy_addr, A_FALSE);
}
