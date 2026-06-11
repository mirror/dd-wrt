/*
 * Copyright (c) 2024-2025 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "hsl.h"
#include "ssdk_plat.h"
#include "fal_port_ctrl.h"
#include "hsl_phy.h"
#include "qcaphy_c45_common.h"
#include "qca81xx_phy.h"
#include "qca81xx.h"
#include "qca808x_lib.h"
#if defined(IN_PTP)
#include "qca808x_ptp.h"
#endif

static a_bool_t phy_ops_flag = A_FALSE;
static a_bool_t phy_driver_flag = A_FALSE;

sw_error_t
qca81xx_phy_status_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct port_phy_status *phy_status)
{
	a_uint16_t phy_data = 0;
	struct phy_device *phydev = NULL;

	hsl_port_phydev_get(dev_id, qca_ssdk_phy_addr_to_port(dev_id, phy_addr),
		&phydev);
	if (phydev && phydev->loopback_enabled)
		return SW_OK;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_SPEC_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	/*get phy link status*/
	if (phy_data & QCAPHY_STATUS_LINK_PASS) {
		phy_status->link_status = PORT_LINK_UP;
	} else {
		phy_status->link_status = PORT_LINK_DOWN;
		return SW_OK;
	}
	/*get phy speed*/
	switch (phy_data & QCAPHY_STATUS_SPEED_MASK) {
		case QCAPHY_STATUS_SPEED_10000MBS:
			phy_status->speed = FAL_SPEED_10000;
			break;
		case QCAPHY_STATUS_SPEED_5000MBS:
			phy_status->speed = FAL_SPEED_5000;
			break;
		case QCAPHY_STATUS_SPEED_2500MBS:
			phy_status->speed = FAL_SPEED_2500;
			break;
		case QCAPHY_STATUS_SPEED_1000MBS:
			phy_status->speed = FAL_SPEED_1000;
			break;
		case QCAPHY_STATUS_SPEED_100MBS:
			phy_status->speed = FAL_SPEED_100;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	/*get phy duplex*/
	if (phy_data & QCAPHY_STATUS_FULL_DUPLEX)
		phy_status->duplex = FAL_FULL_DUPLEX;
	else
		return SW_NOT_SUPPORTED;
	/* get phy flowctrl resolution status */
	if (phy_data & QCAPHY_RX_FLOWCTRL_STATUS)
		phy_status->rx_flowctrl = A_TRUE;
	else
		phy_status->rx_flowctrl = A_FALSE;
	if (phy_data & QCAPHY_TX_FLOWCTRL_STATUS)
		phy_status->tx_flowctrl = A_TRUE;
	else
		phy_status->tx_flowctrl = A_FALSE;

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if ((autoneg & FAL_PHY_ADV_10T_HD) || (autoneg & FAL_PHY_ADV_10T_FD) ||
		(autoneg & FAL_PHY_ADV_100TX_HD))
		return SW_NOT_SUPPORTED;

	rv = qcaphy_c45_set_autoneg_adv(dev_id, phy_addr, autoneg);
	PHY_RTN_ON_ERROR(rv);

	if (autoneg & FAL_PHY_ADV_1000T_FD)
		phy_data |= QCAPHY_ADVERTISE_1000FULL;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_1000BASET_CONTROL, QCAPHY_ADVERTISE_1000FULL, phy_data);
	PHY_RTN_ON_ERROR(rv);

	return hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE, autoneg);
}

sw_error_t
qca81xx_phy_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * autoneg)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*autoneg = 0;
	rv = qcaphy_c45_get_autoneg_adv(dev_id, phy_addr, autoneg);
	PHY_RTN_ON_ERROR(rv);
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_1000BASET_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCAPHY_ADVERTISE_1000FULL)
		*autoneg |= FAL_PHY_ADV_1000T_FD;

	return SW_OK;
}

sw_error_t
qca81xx_phy_get_speed(a_uint32_t dev_id, a_uint32_t phy_addr, fal_port_speed_t * speed)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = qca81xx_phy_status_get(dev_id, phy_addr, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	if (phy_status.link_status == PORT_LINK_UP)
		*speed = phy_status.speed;
	else
		*speed = FAL_SPEED_100;

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr, fal_port_speed_t speed)
{
	sw_error_t rv = SW_OK;

	switch(speed) {
		case FAL_SPEED_10000:
			rv = qca81xx_phy_set_autoneg_adv(dev_id, phy_addr, FAL_PHY_ADV_10000T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_5000:
			rv = qca81xx_phy_set_autoneg_adv(dev_id, phy_addr, FAL_PHY_ADV_5000T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_2500:
			rv = qca81xx_phy_set_autoneg_adv(dev_id, phy_addr, FAL_PHY_ADV_2500T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_1000:
			rv = qca81xx_phy_set_autoneg_adv(dev_id, phy_addr, FAL_PHY_ADV_1000T_FD);
			PHY_RTN_ON_ERROR(rv);
			break;
		case FAL_SPEED_100:
			rv = qcaphy_c45_force_speed_set(dev_id, phy_addr, speed);
			PHY_RTN_ON_ERROR(rv);
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	if(speed >= FAL_SPEED_1000) {
		rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_TRUE,
			hsl_phy_speed_duplex_to_auto_adv(dev_id, speed, FAL_FULL_DUPLEX));
		PHY_RTN_ON_ERROR(rv);
		rv = qcaphy_c45_autoneg_restart(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t duplex)
{
	fal_port_speed_t cur_speed;
	sw_error_t rv = SW_OK;

	if(duplex != FAL_FULL_DUPLEX)
		return SW_NOT_SUPPORTED;

	rv = qca81xx_phy_get_speed(dev_id, phy_addr, &cur_speed);
	PHY_RTN_ON_ERROR(rv);

	return qca81xx_phy_set_speed(dev_id, phy_addr, cur_speed);
}

sw_error_t
qca81xx_phy_get_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t *duplex)
{
	sw_error_t rv = SW_OK;
	struct port_phy_status phy_status = {0};

	rv = qca81xx_phy_status_get(dev_id, phy_addr, &phy_status);
	PHY_RTN_ON_ERROR(rv);

	*duplex = phy_status.duplex;

	return SW_OK;
}

static sw_error_t
qca81xx_phy_cdt_start(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t status = 0;
	a_uint16_t ii = 100;
	sw_error_t rv = SW_OK;

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCA81XX_PHY_CDT_CONTROL, QCA81XX_PHY_RUN_CDT | QCA81XX_PHY_CABLE_LENGTH_UNIT);
	PHY_RTN_ON_ERROR(rv);

	do {
		aos_mdelay(30);
		status = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCA81XX_PHY_CDT_CONTROL);
		PHY_RTN_ON_READ_ERROR(status);
	}
	while ((status & QCA81XX_PHY_RUN_CDT) && (--ii));

	if (ii == 0)
		return SW_TIMEOUT;

	return SW_OK;
}

static fal_cable_status_t
qca81xx_cdt_status_mapping(a_uint16_t status)
{
	fal_cable_status_t status_mapping = FAL_CABLE_STATUS_INVALID;

	switch (status) {
		case 0:
			status_mapping = FAL_CABLE_STATUS_INVALID;
			break;
		case 1:
			status_mapping = FAL_CABLE_STATUS_NORMAL;
			break;
		case 2:
			status_mapping = FAL_CABLE_STATUS_OPENED;
			break;
		case 3:
			status_mapping = FAL_CABLE_STATUS_SHORT;
			break;
	}

	return status_mapping;
}

static sw_error_t
qca81xx_phy_cdt_status_get(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	a_uint16_t cable_delta_time = 0;
	a_uint16_t status = 0;

	if ((mdi_pair >= QCA81XX_PHY_MDI_PAIR_NUM))
		return SW_BAD_PARAM;

	/* Get cable status */
	status = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_CDT_STATUS);
	PHY_RTN_ON_READ_ERROR(status);

	switch (mdi_pair) {
		case 0:
			*cable_status =
				qca81xx_cdt_status_mapping((status >> 12) & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
					QCA81XX_PHY_MMD3_CDT_PAIR0);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);
			break;
		case 1:
			*cable_status =
				qca81xx_cdt_status_mapping((status >> 8) & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
					QCA81XX_PHY_MMD3_CDT_PAIR1);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);
			break;
		case 2:
			*cable_status =
				qca81xx_cdt_status_mapping((status >> 4) & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
					QCA81XX_PHY_MMD3_CDT_PAIR2);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);
			break;
		case 3:
			*cable_status =
				qca81xx_cdt_status_mapping(status & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
					QCA81XX_PHY_MMD3_CDT_PAIR3);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);
			break;
	}

	/* the actual cable length equals to CableDeltaTime * 0.824 */
	*cable_len = ((cable_delta_time & 0xff) * 824) / 1000;

	return SW_OK;
}

sw_error_t
qca81xx_phy_cdt(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	sw_error_t rv;
	a_uint32_t dac8_tmp, dac9_tmp;

	/* if PHY link up, cdt status is noarmal and cable length is 0 */
	if (qcaphy_c45_get_link_status(dev_id, phy_addr) == A_TRUE) {
		*cable_status = FAL_CABLE_STATUS_NORMAL;
		*cable_len = 0;
		return 0;
	}
	dac8_tmp = hsl_phy_c45_debug_reg_read(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_AFE_DAC8_DP);
	dac9_tmp = hsl_phy_c45_debug_reg_read(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_AFE_DAC9_DP);

	rv = hsl_phy_c45_debug_reg_write(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_AFE_DAC8_DP, 0);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_c45_debug_reg_write(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_AFE_DAC9_DP, 0);
	PHY_RTN_ON_ERROR(rv);

	rv = qca81xx_phy_cdt_start(dev_id, phy_addr);
	if (rv != SW_OK) {
		*cable_status = FAL_CABLE_STATUS_INVALID;
		*cable_len = 0;
		return rv;
	}

	/* Get cable status */
	rv = qca81xx_phy_cdt_status_get(dev_id, phy_addr, mdi_pair, cable_status,
		cable_len);
	PHY_RTN_ON_ERROR(rv);

	/* recover the analog AFE DAC value */
	rv = hsl_phy_c45_debug_reg_write(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_AFE_DAC8_DP, dac8_tmp);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_c45_debug_reg_write(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_AFE_DAC9_DP, dac9_tmp);

	return rv;
}

sw_error_t
qca81xx_phy_soft_reset(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	/* enable auto soft reset when power on */
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_SPEC_CONTROL,
		QCA81XX_PHY_AUTO_SOFT_RESET_EN,
		QCA81XX_PHY_AUTO_SOFT_RESET_EN);
	PHY_RTN_ON_ERROR(rv);

	rv = qcaphy_c45_poweroff(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);
	mdelay(10);
	rv = qcaphy_c45_poweron(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);

	/* disable auto soft reset when power on */
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_SPEC_CONTROL,
		QCA81XX_PHY_AUTO_SOFT_RESET_EN, 0);

	return rv;
}

#ifndef IN_PORTCONTROL_MINI
sw_error_t
qca81xx_phy_set_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t mode)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (mode == PHY_MDIX_AUTO)
		phy_data = QCAPHY_MDIX_AUTO;
	else if (mode == PHY_MDIX_MDIX)
		phy_data = QCAPHY_MDIX;
	else if (mode == PHY_MDIX_MDI)
		phy_data = QCAPHY_MDI;
	else
		return SW_BAD_PARAM;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_SPEC_CONTROL, QCAPHY_MDIX_AUTO, phy_data);
	PHY_RTN_ON_ERROR(rv);

	return qca81xx_phy_soft_reset(dev_id, phy_addr);
}

sw_error_t
qca81xx_phy_get_mdix(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_mode_t * mode)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_SPEC_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if ((phy_data & QCAPHY_MDIX_AUTO) == QCAPHY_MDIX_AUTO)
		*mode = PHY_MDIX_AUTO;
	else if ((phy_data & QCAPHY_MDIX_AUTO) == QCAPHY_MDIX)
		*mode = PHY_MDIX_MDIX;
	else
		*mode = PHY_MDIX_MDI;

	return SW_OK;
}

sw_error_t
qca81xx_phy_get_mdix_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_mdix_status_t * mode)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_SPEC_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*mode = (phy_data & QCAPHY_MDIX_STATUS) ? PHY_MDIX_STATUS_MDIX :
		PHY_MDIX_STATUS_MDI;

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE)
		phy_data |= QCA81XX_PHY_HIBERNATION_CFG;

	return hsl_phy_c45_modify_debug(dev_id, phy_addr, QCA81XX_PHY_DEBUG_HIBERNATION_CTRL,
		QCA81XX_PHY_HIBERNATION_CFG, phy_data);
}

sw_error_t
qca81xx_phy_set_pma_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t speed, a_bool_t enable)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_CONTROL, QCAPHY_LOCAL_LOOPBACK_ENABLE,
		enable ? QCAPHY_LOCAL_LOOPBACK_ENABLE : 0);
	PHY_RTN_ON_ERROR(rv);
	rv = qcaphy_c45_autoneg_set(dev_id, phy_addr, !enable);
	PHY_RTN_ON_ERROR(rv);
	if (enable) {
		rv = qcaphy_c45_force_speed_set(dev_id, phy_addr, speed);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_pcs_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t speed, a_bool_t enable)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE,
		QCAPHY_MMD3_NUM, QCAPHY_CONTROL,
		QCAPHY_LOCAL_LOOPBACK_ENABLE,
		enable ? QCAPHY_LOCAL_LOOPBACK_ENABLE : 0);
	PHY_RTN_ON_ERROR(rv);
	rv = qcaphy_c45_autoneg_set(dev_id, phy_addr, !enable);
	PHY_RTN_ON_ERROR(rv);
	if (enable) {
		rv = qcaphy_c45_force_speed_set(dev_id, phy_addr, speed);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	struct phy_device *phydev = NULL;

	rv = hsl_port_phydev_get(dev_id, qca_ssdk_phy_addr_to_port(dev_id, phy_addr),
		&phydev);
	PHY_RTN_ON_ERROR(rv);

	if (phydev->speed < FAL_SPEED_2500) {
		return qca81xx_phy_set_pma_loopback(dev_id, phy_addr, phydev->speed, enable);
	} else {
		/* the link would drop when enable PCS loopback, so need special */
		/* sequence to work around it */
		phydev->loopback_enabled = enable;
		rv = qca81xx_phy_set_hibernate(dev_id, phy_addr, !enable);
		PHY_RTN_ON_ERROR(rv);
		rv = qca81xx_phy_set_pcs_loopback(dev_id, phy_addr, phydev->speed, enable);
		PHY_RTN_ON_ERROR(rv);
		rv = qca81xx_phy_soft_reset(dev_id, phy_addr);
		PHY_RTN_ON_ERROR(rv);
		/* the autoneg would be enabled after software reset, */
		/* so need to configure it again */
		rv = qcaphy_c45_autoneg_set(dev_id, phy_addr, !enable);
		PHY_RTN_ON_ERROR(rv);
		rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
			QCA81XX_PHY_MMD3_BYPASS_SIGNAL,
			QCA81XX_PHY_MMD3_PCS_BYPASS_LINK,
			enable ? QCA81XX_PHY_MMD3_PCS_BYPASS_LINK : 0);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
qca81xx_phy_get_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t *enable)
{
	a_uint16_t phy_data0 = 0, phy_data1 = 0;

	phy_data0 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCAPHY_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data0);
	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCAPHY_CONTROL);
	PHY_RTN_ON_READ_ERROR(phy_data0);

	if ((phy_data0 & QCAPHY_LOCAL_LOOPBACK_ENABLE) ||
		(phy_data1 & QCAPHY_LOCAL_LOOPBACK_ENABLE))
		*enable = A_TRUE;
	else
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_counter(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
	a_uint16_t phy_data0 = 0, phy_data1 = 0;

	if (enable == A_TRUE) {
		phy_data0 |= QCA81XX_PHY_MMD7_FRAME_CHECK_EN;
		phy_data0 |= QCA81XX_PHY_MMD7_XMIT_MAC_CNT_SELFCLR;
		phy_data1 |= QCA81XX_PHY_MMD3_10G_FRAME_CHECK_EN;
	}
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCA81XX_PHY_MMD7_COUNTER_CTRL,
		QCA81XX_PHY_MMD7_FRAME_CHECK_EN | QCA81XX_PHY_MMD7_XMIT_MAC_CNT_SELFCLR,
		phy_data0);
	PHY_RTN_ON_ERROR(rv);

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_10G_FRAME_CHECK_CTRL,
		QCA81XX_PHY_MMD3_10G_FRAME_CHECK_EN,
		phy_data1);

	return rv;
}

sw_error_t
qca81xx_phy_get_counter(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	a_uint16_t phy_data0 = 0, phy_data1  = 0;

	phy_data0 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCA81XX_PHY_MMD7_COUNTER_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data0);

	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_10G_FRAME_CHECK_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data1);

	if ((phy_data0 & QCA81XX_PHY_MMD7_FRAME_CHECK_EN) &&
		(phy_data1 & QCA81XX_PHY_MMD3_10G_FRAME_CHECK_EN))
		*enable = A_TRUE;
	else
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
qca81xx_phy_show_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	sw_error_t rv = SW_OK;
	a_uint16_t ingress_high_counter = 0;
	a_uint16_t ingress_middle_counter = 0;
	a_uint16_t ingress_low_counter = 0;
	a_uint16_t egress_high_counter = 0;
	a_uint16_t egress_middle_counter = 0;
	a_uint16_t egress_low_counter = 0;
	fal_port_speed_t speed = FAL_SPEED_BUTT;

	rv = qca81xx_phy_get_speed(dev_id, phy_addr, &speed);
	PHY_RTN_ON_ERROR(rv);
	if(speed >= FAL_SPEED_2500) {
		ingress_high_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_INGRESS_COUNTER_HIGH);
		ingress_middle_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_INGRESS_COUNTER_MIDDLE);
		ingress_low_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_INGRESS_COUNTER_LOW);
		counter_infor->RxGoodFrame = ((a_uint64_t)ingress_high_counter << 32 ) |
			(ingress_middle_counter << 16 ) | ingress_low_counter;
		counter_infor->RxBadCRC = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_INGRESS_ERROR_COUNTER);

		egress_high_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_EGRESS_COUNTER_HIGH);
		egress_middle_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_EGRESS_COUNTER_MIDDLE);
		egress_low_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_EGRESS_COUNTER_LOW);
		counter_infor->TxGoodFrame = ((a_uint64_t)egress_high_counter << 32 ) |
			(egress_middle_counter << 16 ) | egress_low_counter;
		counter_infor->TxBadCRC = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD3_NUM, QCA81XX_PHY_MMD3_10G_EGRESS_ERROR_COUNTER);
	} else {
		ingress_high_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD7_NUM, QCA81XX_PHY_MMD7_INGRESS_COUNTER_HIGH);
		ingress_low_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD7_NUM, QCA81XX_PHY_MMD7_INGRESS_COUNTER_LOW);
		counter_infor->RxGoodFrame = (ingress_high_counter << 16 ) | ingress_low_counter;
		counter_infor->RxBadCRC = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD7_NUM, QCA81XX_PHY_MMD7_INGRESS_ERROR_COUNTER);

		egress_high_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD7_NUM, QCA81XX_PHY_MMD7_EGRESS_COUNTER_HIGH);
		egress_low_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD7_NUM, QCA81XX_PHY_MMD7_EGRESS_COUNTER_LOW);
		counter_infor->TxGoodFrame = (egress_high_counter << 16 ) | egress_low_counter;
		counter_infor->TxBadCRC = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCAPHY_MMD7_NUM, QCA81XX_PHY_MMD7_EGRESS_ERROR_COUNTER);
	}

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_mask_flag)
{
	a_uint16_t phy_data = 0, mask = 0;
	sw_error_t rv = SW_OK;

	mask = QCA81XX_PHY_INTR_STATUS_UP_CHANGE | QCA81XX_PHY_INTR_STATUS_DOWN_CHANGE |
		QCA81XX_PHY_INTR_SPEED_CHANGE | QCA81XX_PHY_INTR_WOL | QCA81XX_PHY_INTR_POE;

	if (intr_mask_flag & FAL_PHY_INTR_STATUS_UP_CHANGE)
		phy_data |= QCA81XX_PHY_INTR_STATUS_UP_CHANGE;

	if (intr_mask_flag & FAL_PHY_INTR_STATUS_DOWN_CHANGE)
		phy_data |= QCA81XX_PHY_INTR_STATUS_DOWN_CHANGE;

	if (intr_mask_flag & FAL_PHY_INTR_SPEED_CHANGE)
		phy_data |= QCA81XX_PHY_INTR_SPEED_CHANGE;

	if (intr_mask_flag & FAL_PHY_INTR_WOL_STATUS)
		phy_data |= QCA81XX_PHY_INTR_WOL;

	if (intr_mask_flag & FAL_PHY_INTR_POE_STATUS)
		phy_data |= QCA81XX_PHY_INTR_POE;

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, QCAPHY_MMD31_NUM, A_TRUE,
		QCA81XX_PHY_INTR_MASK, mask, phy_data);

	return rv;
}

sw_error_t
qca81xx_phy_get_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_mask_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCA81XX_PHY_INTR_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_mask_flag = 0;
	if (phy_data & QCA81XX_PHY_INTR_STATUS_UP_CHANGE)
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;

	if (phy_data & QCA81XX_PHY_INTR_STATUS_DOWN_CHANGE)
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;

	if (phy_data & QCA81XX_PHY_INTR_SPEED_CHANGE)
		*intr_mask_flag |= FAL_PHY_INTR_SPEED_CHANGE;

	if (phy_data & QCA81XX_PHY_INTR_WOL)
		*intr_mask_flag |= FAL_PHY_INTR_WOL_STATUS;

	if (phy_data & QCA81XX_PHY_INTR_POE)
		*intr_mask_flag |= FAL_PHY_INTR_POE_STATUS;

	return SW_OK;
}

sw_error_t
qca81xx_phy_get_intr_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_status_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD31_NUM,
		QCA81XX_PHY_INTR_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_status_flag = 0;
	if (phy_data & QCA81XX_PHY_INTR_STATUS_UP_CHANGE)
		*intr_status_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;

	if (phy_data & QCA81XX_PHY_INTR_STATUS_DOWN_CHANGE)
		*intr_status_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;

	if (phy_data & QCA81XX_PHY_INTR_SPEED_CHANGE)
		*intr_status_flag |= FAL_PHY_INTR_SPEED_CHANGE;

	if (phy_data & QCA81XX_PHY_INTR_WOL)
		*intr_status_flag |= FAL_PHY_INTR_WOL_STATUS;

	if (phy_data & QCA81XX_PHY_INTR_POE)
		*intr_status_flag |= FAL_PHY_INTR_POE_STATUS;

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE)
		phy_data |= QCA81XX_PHY_MMD3_REMOTE_LOOPBACK_EN;

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL,
		QCA81XX_PHY_MMD3_REMOTE_LOOPBACK_EN,
		phy_data);
}

sw_error_t
qca81xx_phy_get_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA81XX_PHY_MMD3_REMOTE_LOOPBACK_EN)
		*enable = A_TRUE;
	else
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
qca81xx_phy_get_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	*enable = A_FALSE;

	phy_data = hsl_phy_c45_debug_reg_read(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_HIBERNATION_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA81XX_PHY_HIBERNATION_CFG)
		*enable = A_TRUE;

	return SW_OK;
}

sw_error_t
qca81xx_phy_get_hibernate_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data;

	*enable = A_TRUE;

	phy_data = hsl_phy_c45_debug_reg_read(dev_id, phy_addr,
		QCA81XX_PHY_DEBUG_HIBERNATION_STAT);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA81XX_PHY_HIBERNATION_STAT_EN)
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	a_uint16_t phy_data3 = 0;
	sw_error_t rv = SW_OK;

	phy_data1 = (mac->uc[0] << 8) | mac->uc[1];
	phy_data2 = (mac->uc[2] << 8) | mac->uc[3];
	phy_data3 = (mac->uc[4] << 8) | mac->uc[5];

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_MAGIC_MAC_CTRL1, phy_data1);
	PHY_RTN_ON_ERROR(rv);

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_MAGIC_MAC_CTRL2, phy_data2);
	PHY_RTN_ON_ERROR(rv);

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_MAGIC_MAC_CTRL3, phy_data3);

	return rv;
}

sw_error_t
qca81xx_phy_get_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	a_uint16_t phy_data3 = 0;

	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_MAGIC_MAC_CTRL1);
	PHY_RTN_ON_READ_ERROR(phy_data1);

	phy_data2 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_MAGIC_MAC_CTRL2);
	PHY_RTN_ON_READ_ERROR(phy_data2);

	phy_data3 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_MAGIC_MAC_CTRL3);
	PHY_RTN_ON_READ_ERROR(phy_data3);

	mac->uc[0] = (phy_data1 >> 8);
	mac->uc[1] = (phy_data1 & 0x00ff);
	mac->uc[2] = (phy_data2 >> 8);
	mac->uc[3] = (phy_data2 & 0x00ff);
	mac->uc[4] = (phy_data3 >> 8);
	mac->uc[5] = (phy_data3 & 0x00ff);

	return SW_OK;
}

sw_error_t
qca81xx_phy_set_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE)
		phy_data |= QCA81XX_PHY_MMD3_WOL_EN;

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_CTRL, QCA81XX_PHY_MMD3_WOL_EN, phy_data);
}

sw_error_t
qca81xx_phy_get_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	*enable = A_FALSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD3_NUM,
		QCA81XX_PHY_MMD3_WOL_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA81XX_PHY_MMD3_WOL_EN)
		*enable = A_TRUE;

	return SW_OK;
}
#endif

static sw_error_t
qca81xx_phy_led_force_reg_get (a_uint32_t dev_id, a_uint32_t source_id,
	a_uint32_t *reg_num)
{
	a_uint32_t led_ctrl[3] = {QCA81XX_PHY_MMD7_LED0_FORCE_CTRL,
		QCA81XX_PHY_MMD7_LED1_FORCE_CTRL,
		QCA81XX_PHY_MMD7_LED2_FORCE_CTRL
	};

	if (source_id > QCAPHY_LED_SOURCE2) {
		SSDK_ERROR("source %d is not support\n", source_id);
		return SW_NOT_SUPPORTED;
	}

	*reg_num = led_ctrl[source_id];

	return SW_OK;
}

static sw_error_t
qca81xx_phy_led_force_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, a_bool_t enable, a_uint32_t force_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	rv = qca81xx_phy_led_force_reg_get(dev_id, source_id, &mmd_reg);
	PHY_RTN_ON_ERROR(rv);
	if (enable) {
		rv = qcaphy_led_pattern_force_to_phy(dev_id, force_mode, &phy_data);
		PHY_RTN_ON_ERROR(rv);
	}
	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		mmd_reg, QCAPHY_PHY_LED_FORCE_EN | QCAPHY_PHY_LED_FORCE_MASK,
		phy_data);
}

static sw_error_t
qca81xx_phy_led_force_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, a_bool_t *enable, a_uint32_t *force_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	rv = qca81xx_phy_led_force_reg_get(dev_id, source_id, &mmd_reg);
	PHY_RTN_ON_ERROR(rv);

	phy_data =hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		mmd_reg);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCAPHY_PHY_LED_FORCE_EN) {
		*enable = A_TRUE;
		rv = qcaphy_led_pattern_force_from_phy(dev_id,
			force_mode, phy_data);
		PHY_RTN_ON_ERROR(rv);
	} else {
		*enable = A_FALSE;
	}

	return SW_OK;
}

static sw_error_t
qca81xx_phy_led_reg_get(a_uint32_t dev_id, a_uint32_t source_id,
	a_uint32_t *reg_num)
{
	a_uint32_t led_ctrl[3] = {QCA81XX_PHY_MMD7_LED0_CTRL,
		QCA81XX_PHY_MMD7_LED1_CTRL,
		QCA81XX_PHY_MMD7_LED2_CTRL
	};

	if (source_id > QCAPHY_LED_SOURCE2) {
		SSDK_ERROR("source %d is not support\n", source_id);
		return SW_NOT_SUPPORTED;
	}

	*reg_num = led_ctrl[source_id];

	return SW_OK;
}

static sw_error_t
qca81xx_phy_10g_led_ctrl_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	a_uint16_t phy_data = 0, mask = 0;

	mask = BIT(QCA81XX_PHY_MMD7_10G_SRC0_OFFSET + source_id * 2) |
		BIT(QCA81XX_PHY_MMD7_5G_SRC0_OFFSET + source_id * 2);

	if (pattern->map & BIT(LINK_10000M_LIGHT_EN))
		phy_data |= BIT(QCA81XX_PHY_MMD7_10G_SRC0_OFFSET +
			source_id * 2);
	if (pattern->map & BIT(LINK_5000M_LIGHT_EN))
		phy_data |= BIT(QCA81XX_PHY_MMD7_5G_SRC0_OFFSET +
			source_id * 2);

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCA81XX_PHY_MMD7_LED_10G_CTRL, mask, phy_data);
}

static sw_error_t
qca81xx_phy_10g_led_ctrl_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
		QCA81XX_PHY_MMD7_LED_10G_CTRL);
	if (phy_data & BIT(QCA81XX_PHY_MMD7_10G_SRC0_OFFSET +
		source_id * 2))
		pattern->map |= BIT(LINK_10000M_LIGHT_EN);
	if (phy_data & BIT(QCA81XX_PHY_MMD7_5G_SRC0_OFFSET +
		source_id * 2))
		pattern->map |= BIT(LINK_5000M_LIGHT_EN);

	return 0;
}

sw_error_t
qca81xx_phy_led_ctrl_source_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	if (source_id > QCAPHY_LED_SOURCE2)
		return SW_NOT_SUPPORTED;

	rv = qcaphy_led_active_set(dev_id, phy_addr, pattern->active_level);
	PHY_RTN_ON_ERROR(rv);
	rv = qcaphy_led_blink_freq_set(dev_id, phy_addr, pattern->mode,
		pattern->freq);
	PHY_RTN_ON_ERROR(rv);
	if (pattern->mode == LED_PATTERN_MAP_EN) {
		rv = qca81xx_phy_led_force_set(dev_id, phy_addr,
			source_id, A_FALSE, pattern->mode);
		PHY_RTN_ON_ERROR(rv);
		rv = qcaphy_led_pattern_map_to_phy(dev_id, pattern->map, &phy_data);
		PHY_RTN_ON_ERROR(rv);
		if (pattern->map & BIT(LINK_2500M_LIGHT_EN))
			phy_data |=  QCA81XX_PHY_MMD7_LINK_2500M_LIGHT_EN;
		rv = qca81xx_phy_led_reg_get(dev_id, source_id, &mmd_reg);
		PHY_RTN_ON_ERROR(rv);
		rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
			mmd_reg, phy_data);
		PHY_RTN_ON_ERROR(rv);
		rv = qca81xx_phy_10g_led_ctrl_set(dev_id, phy_addr, source_id, pattern);
		PHY_RTN_ON_ERROR(rv);
	} else {
		rv = qca81xx_phy_led_force_set(dev_id, phy_addr, source_id,
			A_TRUE, pattern->mode);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

sw_error_t
qca81xx_phy_led_ctrl_source_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t source_id, led_ctrl_pattern_t *pattern)
{
	sw_error_t rv = SW_OK;
	a_bool_t force_enable = A_FALSE;
	a_uint32_t mmd_reg = 0;
	a_uint16_t phy_data = 0;

	if (source_id > QCAPHY_LED_SOURCE2)
		return SW_NOT_SUPPORTED;

	rv = qcaphy_led_active_get(dev_id, phy_addr, &(pattern->active_level));
	PHY_RTN_ON_ERROR(rv);
	pattern->map = 0;
	rv = qca81xx_phy_led_force_get(dev_id, phy_addr, source_id,
		&force_enable, &(pattern->mode));
	PHY_RTN_ON_ERROR(rv);
	if (!force_enable) {
		pattern->mode = LED_PATTERN_MAP_EN;
		rv = qca81xx_phy_led_reg_get(dev_id, source_id, &mmd_reg);
		PHY_RTN_ON_ERROR(rv);
		phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCAPHY_MMD7_NUM,
			mmd_reg);
		PHY_RTN_ON_READ_ERROR(phy_data);
		rv = qcaphy_led_pattern_map_from_phy(dev_id, &(pattern->map), phy_data);
		PHY_RTN_ON_ERROR(rv);
		if (phy_data & QCA81XX_PHY_MMD7_LINK_2500M_LIGHT_EN)
			pattern->map |= BIT(LINK_2500M_LIGHT_EN);
		rv = qca81xx_phy_10g_led_ctrl_get(dev_id, phy_addr, source_id, pattern);
	}
	rv = qcaphy_led_blink_freq_get(dev_id, phy_addr, pattern->mode,
		&(pattern->freq));

	return rv;
}

sw_error_t
qca81xx_phy_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	if (!(qcaphy_c45_get_link_status(dev_id, phy_addr))) {
		rv = hsl_phy_modify_mii(dev_id, phy_addr + 1,
			QCA81XX_PHY_PCS_PLL_POWER_ON_AND_RESET,
			QCA81XX_PHY_PCS_ANA_SOFT_RESET_MASK,
			QCA81XX_PHY_PCS_ANA_SOFT_RESET);
		PHY_RTN_ON_ERROR(rv);
	}

	rv = qcaphy_c45_poweroff(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);

	return hsl_phydev_suspended_update(dev_id, phy_addr, A_TRUE);
}

sw_error_t
qca81xx_phy_poweron(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	/* make sure the PHY PCS is enabled */
	rv = hsl_phy_modify_mii(dev_id, phy_addr + 1,
		QCA81XX_PHY_PCS_PLL_POWER_ON_AND_RESET,
		QCA81XX_PHY_PCS_ANA_SOFT_RESET_MASK,
		QCA81XX_PHY_PCS_ANA_SOFT_RELEASE);
	PHY_RTN_ON_ERROR(rv);

	rv = qcaphy_c45_poweron(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);

	return hsl_phydev_suspended_update(dev_id, phy_addr, A_FALSE);
}

sw_error_t
qca81xx_phy_adjust_link_post(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t  rv = SW_OK;
	struct phy_device *phydev = NULL;

	rv = hsl_port_phydev_get(dev_id, qca_ssdk_phy_addr_to_port(dev_id, phy_addr),
		&phydev);
	SW_RTN_ON_ERROR(rv);

	/* disable PHY PCS if PHY is suspended and link is down */
	if (phydev && phydev->suspended &&
		!(qcaphy_c45_get_link_status(dev_id, phy_addr))) {
		rv = hsl_phy_modify_mii(dev_id, phy_addr + 1,
			QCA81XX_PHY_PCS_PLL_POWER_ON_AND_RESET,
			QCA81XX_PHY_PCS_ANA_SOFT_RESET_MASK,
			QCA81XX_PHY_PCS_ANA_SOFT_RESET);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}

static sw_error_t
qca81xx_phy_api_ops_init(void)
{
	sw_error_t  ret = SW_OK;
	hsl_phy_ops_t *qca81xx_phy_api_ops = NULL;

	qca81xx_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (qca81xx_phy_api_ops == NULL) {
		SSDK_ERROR("qca81xx phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(QCA81XX_PHY_CHIP);
	qca81xx_phy_api_ops->phy_id_get = qcaphy_c45_get_phy_id;
	qca81xx_phy_api_ops->phy_power_off = qca81xx_phy_poweroff;
	qca81xx_phy_api_ops->phy_power_on = qca81xx_phy_poweron;
	qca81xx_phy_api_ops->phy_link_status_get = qcaphy_c45_get_link_status;
	qca81xx_phy_api_ops->phy_autoneg_enable_set = qcaphy_c45_autoneg_enable;
	qca81xx_phy_api_ops->phy_restart_autoneg = qcaphy_c45_autoneg_restart;
	qca81xx_phy_api_ops->phy_autoneg_status_get = qcaphy_c45_autoneg_status;
	qca81xx_phy_api_ops->phy_reset = qca81xx_phy_soft_reset;
	qca81xx_phy_api_ops->phy_eee_adv_set = qcaphy_c45_set_eee_adv;
	qca81xx_phy_api_ops->phy_eee_adv_get = qcaphy_c45_get_eee_adv;
	qca81xx_phy_api_ops->phy_eee_partner_adv_get = qcaphy_c45_get_eee_partner_adv;
	qca81xx_phy_api_ops->phy_eee_cap_get = qcaphy_c45_get_eee_cap;
	qca81xx_phy_api_ops->phy_eee_status_get = qcaphy_c45_get_eee_status;
	qca81xx_phy_api_ops->phy_get_status = qca81xx_phy_status_get;
	qca81xx_phy_api_ops->phy_duplex_get = qca81xx_phy_get_duplex;
	qca81xx_phy_api_ops->phy_speed_get = qca81xx_phy_get_speed;
	qca81xx_phy_api_ops->phy_duplex_set = qca81xx_phy_set_duplex;
	qca81xx_phy_api_ops->phy_speed_set = qca81xx_phy_set_speed;
	qca81xx_phy_api_ops->phy_autoneg_adv_set = qca81xx_phy_set_autoneg_adv;
	qca81xx_phy_api_ops->phy_autoneg_adv_get = qca81xx_phy_get_autoneg_adv;
	qca81xx_phy_api_ops->phy_cdt = qca81xx_phy_cdt;
	qca81xx_phy_api_ops->phy_led_ctrl_source_set = qca81xx_phy_led_ctrl_source_set;
	qca81xx_phy_api_ops->phy_led_ctrl_source_get = qca81xx_phy_led_ctrl_source_get;
	qca81xx_phy_api_ops->phy_adjust_link_post = qca81xx_phy_adjust_link_post;
#ifndef IN_PORTCONTROL_MINI
	qca81xx_phy_api_ops->phy_8023az_set = qcaphy_c45_set_8023az;
	qca81xx_phy_api_ops->phy_8023az_get = qcaphy_c45_get_8023az;
	qca81xx_phy_api_ops->phy_mdix_set = qca81xx_phy_set_mdix;
	qca81xx_phy_api_ops->phy_mdix_get = qca81xx_phy_get_mdix;
	qca81xx_phy_api_ops->phy_mdix_status_get = qca81xx_phy_get_mdix_status;
	qca81xx_phy_api_ops->phy_local_loopback_set = qca81xx_phy_set_local_loopback;
	qca81xx_phy_api_ops->phy_local_loopback_get = qca81xx_phy_get_local_loopback;
	qca81xx_phy_api_ops->phy_remote_loopback_set = qca81xx_phy_set_remote_loopback;
	qca81xx_phy_api_ops->phy_remote_loopback_get = qca81xx_phy_get_remote_loopback;
	qca81xx_phy_api_ops->phy_hibernation_set = qca81xx_phy_set_hibernate;
	qca81xx_phy_api_ops->phy_hibernation_get = qca81xx_phy_get_hibernate;
	qca81xx_phy_api_ops->phy_magic_frame_mac_set = qca81xx_phy_set_magic_frame_mac;
	qca81xx_phy_api_ops->phy_magic_frame_mac_get = qca81xx_phy_get_magic_frame_mac;
	qca81xx_phy_api_ops->phy_wol_status_set = qca81xx_phy_set_wol_status;
	qca81xx_phy_api_ops->phy_wol_status_get = qca81xx_phy_get_wol_status;
	qca81xx_phy_api_ops->phy_counter_set = qca81xx_phy_set_counter;
	qca81xx_phy_api_ops->phy_counter_get = qca81xx_phy_get_counter;
	qca81xx_phy_api_ops->phy_counter_show = qca81xx_phy_show_counter;
	qca81xx_phy_api_ops->phy_intr_mask_set = qca81xx_phy_set_intr_mask;
	qca81xx_phy_api_ops->phy_intr_mask_get = qca81xx_phy_get_intr_mask;
	qca81xx_phy_api_ops->phy_intr_status_get = qca81xx_phy_get_intr_status;
#endif
#if defined(IN_PTP)
	qca808x_phy_ptp_api_ops_init(&qca81xx_phy_api_ops->phy_ptp_ops);
#endif

	ret = hsl_phy_api_ops_register(QCA81XX_PHY_CHIP, qca81xx_phy_api_ops);

	if (ret == SW_OK)
		SSDK_INFO("qca probe qca81xx phy driver succeeded!\n");
	else
		SSDK_ERROR("qca probe qca81xx phy driver failed! (code: %d)\n", ret);

	return ret;
}

int qca81xx_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id;

	if(phy_ops_flag == A_FALSE) {
		qca81xx_phy_api_ops_init();
		phy_ops_flag = A_TRUE;
	}

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id++) {
		if (port_bmp & BIT(port_id))
			qca808x_phydev_init(dev_id, port_id);
	}

	if (!phy_driver_flag) {
		qca81xx_phy_driver_register();
#if defined(IN_LINUX_STD_PTP)
		qca808x_ptp_hook_init();
#endif
		phy_driver_flag = A_TRUE;
	}

	return 0;
}

void qca81xx_phy_exit(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id;

	if (phy_driver_flag) {
#if defined(IN_LINUX_STD_PTP)
		qca808x_ptp_hook_cleanup();
#endif
		qca81xx_phy_driver_unregister();
		phy_driver_flag = A_FALSE;
	}

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id++) {
		if (port_bmp & BIT(port_id))
			qca808x_phydev_deinit(dev_id, port_id);
	}

	return;
}
