/*
 * Copyright (c) 2018, 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "sw.h"
#include "fal_port_ctrl.h"
#include "hsl_api.h"
#include "hsl.h"
#include "hsl_phy.h"
#include "qcaphy_common.h"
#include "ssdk_plat.h"
#include "qca808x_phy.h"
/*qca808x_end*/
#if defined(IN_PTP)
#include "qca808x_ptp.h"
#endif
#include "qca808x.h"
#ifdef IN_LED
#include "qca808x_led.h"
#endif
#if defined(MHT)
#include "mht_sec_ctrl.h"
#include "qca8084_phy.h"
#endif

static a_bool_t qca808x_ssdk_phy_drv_registered = A_FALSE;
/*qca808x_start*/
static a_bool_t phy_ops_flag = A_FALSE;

static sw_error_t
qca808x_phy_ms_random_seed_set(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 0))
	phy_data |= (get_random_u32()%QCA808X_MASTER_SLAVE_SEED_RANGE) << 2;
#else
	phy_data |= (prandom_u32()%QCA808X_MASTER_SLAVE_SEED_RANGE) << 2;
#endif

	return hsl_phy_modify_debug(dev_id, phy_addr, QCA808X_DEBUG_LOCAL_SEED,
		QCA808X_MASTER_SLAVE_SEED_CFG, phy_data);
}

static sw_error_t
qca808x_phy_ms_seed_enable(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if(enable)
		phy_data = QCA808X_MASTER_SLAVE_SEED_ENABLE;

	return hsl_phy_modify_debug(dev_id, phy_addr, QCA808X_DEBUG_LOCAL_SEED,
		QCA808X_MASTER_SLAVE_SEED_ENABLE, phy_data);
}

a_bool_t
qca808x_phy_2500caps(a_uint32_t dev_id, a_uint32_t phy_addr)
{
#if 0
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_MMD1_PMA_CAP_REG);

#ifdef MHT
	/*the bit0 of mmd7 reg0x901d is used for ipq runing and 1G napa flag,
	and qca8084 support 2.5G always*/
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
		return A_TRUE;
#endif

	if (phy_data & QCA808X_STATUS_2500T_FD_CAPS) {
		phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
			QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_CHIP_TYPE);
		if(!(phy_data & QCA808X_PHY_1G_CHIP_TYPE))
			return A_TRUE;
	}

	return A_FALSE;
#endif

	return hsl_phy_autoneg_adv_check(dev_id, phy_addr, FAL_PHY_ADV_2500T_FD);
}

a_bool_t
qca808x_phy_id_check(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t phy_id)
{
	a_uint32_t phy_id_tmp = 0;

	qcaphy_get_phy_id (dev_id, phy_addr, &phy_id_tmp);

	if(phy_id_tmp == phy_id)
		return A_TRUE;

	return A_FALSE;
}

static sw_error_t
qca808x_phy_fifo_reset(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data = 0;

/*qca808x_end*/
#ifdef MHT
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
		return qca8084_phy_fifo_reset(dev_id, phy_addr, enable);
#endif
/*qca808x_start*/
	if(!enable)
		phy_data |= BIT(11);
	/*qca808x serdes address is phy address+1*/
	return hsl_phy_modify_mmd(dev_id, phy_addr+1, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_FIFO_REST_REG, BIT(11), phy_data);
}

/******************************************************************************
*
* qca808x_phy_get status
*
* get phy status
*/
sw_error_t
qca808x_phy_get_status(a_uint32_t dev_id, a_uint32_t phy_addr,
		struct port_phy_status *phy_status)
{
	a_uint16_t phy_data = 0;

	qcaphy_status_get(dev_id, phy_addr, phy_status);
	if (phy_status->link_status) {
#if defined(MHT)
		if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
		{
			qca8084_phy_speed_fixup(dev_id, phy_addr, phy_status);
		}
		else
#endif
		{
			qca808x_phy_fifo_reset(dev_id, phy_addr, A_FALSE);
		}
	}
	else {
#if defined(MHT)
		if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY)) {
			/*when link down, phy speed is set as 10M*/
			phy_status->speed = FAL_SPEED_10;
			qca8084_phy_speed_fixup(dev_id, phy_addr, phy_status);
		}
		else
#endif
		{
			qca808x_phy_fifo_reset(dev_id, phy_addr, A_TRUE);
			if (qca808x_phy_2500caps(dev_id, phy_addr) == A_TRUE) {
				PHY_RTN_ON_ERROR(
					qca808x_phy_ms_random_seed_set (dev_id, phy_addr));
				/*protect logic, if MASTER_SLAVE_CONFIG_FAULT is 1,
					then disable this logic*/
				phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr,
					QCA808X_1000BASET_STATUS);
				if ((phy_data & QCA808X_MASTER_SLAVE_CONFIG_FAULT) >> 15)
				{
					PHY_RTN_ON_ERROR(
						qca808x_phy_ms_seed_enable (dev_id,phy_addr,
							A_FALSE));
					SSDK_INFO("master_slave_config_fault was set\n");
				}
			}
		}
	}

	return SW_OK;
}
/******************************************************************************
*
* qca808x_phy_set_force_speed - Force the speed of qca808x phy ports associated with the
* specified device.
*/
sw_error_t
qca808x_phy_set_force_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	sw_error_t rv = SW_OK;

	switch(speed)
	{
		case FAL_SPEED_2500:
			if(!qca808x_phy_2500caps(dev_id, phy_addr))
				return SW_NOT_SUPPORTED;
			phy_data1 |= QCA808X_PMA_CONTROL_2500M;
			phy_data2 |= QCA808X_PMA_TYPE_2500M;
			break;
		case FAL_SPEED_1000:
			phy_data1 |= QCA808X_PMA_CONTROL_1000M;
			phy_data2 |= QCA808X_PMA_TYPE_1000M;
			break;
		case FAL_SPEED_100:
			phy_data1 |= QCA808X_PMA_CONTROL_100M;
			phy_data2 |= QCA808X_PMA_TYPE_100M;
			break;
		case FAL_SPEED_10:
			phy_data1 |= QCA808X_PMA_CONTROL_10M;
			phy_data2 |= QCA808X_PMA_TYPE_10M;
			break;
		default:
			return SW_BAD_PARAM;
	}
	/* the speed of qca808x controled by MMD1 PMA/PMD control register */
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_PMA_CONTROL, QCA808X_PMA_CONTROL_SPEED_MASK,
		phy_data1);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_PMA_TYPE, QCA808X_PMA_TYPE_MASK, phy_data2);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
	rv = hsl_phy_phydev_autoneg_update(dev_id, phy_addr, A_FALSE, 0);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_start*/
	return SW_OK;
}

sw_error_t
_qca808x_phy_set_autoneg_adv_ext(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg)
{
	a_uint16_t phy_data = 0;

	if (autoneg & FAL_PHY_ADV_2500T_FD) {
		phy_data |= QCA808X_ADVERTISE_2500FULL;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_AUTONEGOTIATION_CONTROL, QCA808X_ADVERTISE_2500FULL,
		phy_data);
}

/******************************************************************************
*
* qca808x_phy_set_speed - Determines the speed of phy ports associated with the
* specified device.
*/
sw_error_t
qca808x_phy_set_speed(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_speed_t speed)
{
	a_uint16_t phy_data = 0, mask = 0;
	fal_port_duplex_t cur_duplex = QCA808X_CTRL_FULL_DUPLEX;
	a_uint32_t ability = 0;
	sw_error_t rv = SW_OK;

	switch(speed)
	{
		case FAL_SPEED_2500:
		case FAL_SPEED_1000:
			if (speed == FAL_SPEED_2500) {
				if(!qca808x_phy_2500caps(dev_id, phy_addr))
					return SW_NOT_SUPPORTED;
				rv = qca808x_phy_set_autoneg_adv(dev_id, phy_addr,
					FAL_PHY_ADV_2500T_FD);
				PHY_RTN_ON_ERROR(rv);
			} else {
				rv = qca808x_phy_set_autoneg_adv(dev_id, phy_addr,
						FAL_PHY_ADV_1000T_FD);
				PHY_RTN_ON_ERROR(rv);
			}
			phy_data |= QCA808X_CTRL_FULL_DUPLEX;
			phy_data |= QCA808X_CTRL_AUTONEGOTIATION_ENABLE;
			phy_data |= QCA808X_CTRL_RESTART_AUTONEGOTIATION;
			mask = phy_data;
			break;
		case FAL_SPEED_100:
		case FAL_SPEED_10:
			/* set qca808x phy speed by pma control registers */
			rv = qca808x_phy_set_force_speed(dev_id, phy_addr, speed);
			PHY_RTN_ON_ERROR(rv);
			rv = qcaphy_get_duplex(dev_id, phy_addr, &cur_duplex);
			PHY_RTN_ON_ERROR(rv);

			if (cur_duplex == FAL_FULL_DUPLEX) {
				phy_data |= QCA808X_CTRL_FULL_DUPLEX;
			} else if (cur_duplex == FAL_HALF_DUPLEX) {
				rv = qca808x_phy_get_ability(dev_id, phy_addr, &ability);
				PHY_RTN_ON_ERROR(rv);
				if(!((speed == FAL_SPEED_10 &&
				(ability & FAL_PHY_ADV_10T_HD)) ||
				(speed == FAL_SPEED_100 &&
				(ability & FAL_PHY_ADV_100TX_HD))))
					phy_data |= QCA808X_CTRL_FULL_DUPLEX;
			}
			else
				return SW_NOT_SUPPORTED;
			mask = QCA808X_CTRL_FULL_DUPLEX | QCA808X_CTRL_AUTONEGOTIATION_ENABLE;
			break;
		default:
			return SW_BAD_PARAM;
	}

	return hsl_phy_modify_mii(dev_id, phy_addr, QCA808X_PHY_CONTROL, mask,
		phy_data);
}

/******************************************************************************
*
* qca808x_phy_set_duplex - Determines the duplex of phy ports associated with the
* specified device.
*/
sw_error_t
qca808x_phy_set_duplex(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_duplex_t duplex)
{
	a_uint16_t phy_data = 0, mask = 0;
	fal_port_speed_t cur_speed = FAL_SPEED_BUTT;
	sw_error_t rv = SW_OK;
	a_uint32_t ability = 0;

	rv = qcaphy_get_speed(dev_id, phy_addr, &cur_speed);
	PHY_RTN_ON_ERROR(rv);

	switch(cur_speed)
	{
		case FAL_SPEED_2500:
		case FAL_SPEED_1000:
			if (duplex == FAL_FULL_DUPLEX) {
				phy_data |= QCA808X_CTRL_FULL_DUPLEX;
			} else {
				return SW_NOT_SUPPORTED;
			}
			phy_data |= QCA808X_CTRL_AUTONEGOTIATION_ENABLE;
			phy_data |= QCA808X_CTRL_RESTART_AUTONEGOTIATION;
			mask |= phy_data;
			if (cur_speed == FAL_SPEED_2500) {
				rv = qca808x_phy_set_autoneg_adv(dev_id, phy_addr,
						FAL_PHY_ADV_2500T_FD);
				PHY_RTN_ON_ERROR(rv);
			} else {
				rv = qca808x_phy_set_autoneg_adv(dev_id, phy_addr,
						FAL_PHY_ADV_1000T_FD);
				PHY_RTN_ON_ERROR(rv);
			}
			break;
		case FAL_SPEED_100:
		case FAL_SPEED_10:
			/* force the speed */
			rv = qca808x_phy_set_force_speed(dev_id, phy_addr, cur_speed);
			PHY_RTN_ON_ERROR(rv);
			mask |= QCA808X_CTRL_AUTONEGOTIATION_ENABLE | QCA808X_CTRL_FULL_DUPLEX;
			if (duplex == FAL_FULL_DUPLEX) {
				phy_data |= QCA808X_CTRL_FULL_DUPLEX;
			} else {
				rv = qca808x_phy_get_ability(dev_id, phy_addr, &ability);
				PHY_RTN_ON_ERROR(rv);
				if((cur_speed == FAL_SPEED_10 &&
				!(ability & FAL_PHY_ADV_10T_HD)) ||
				(cur_speed == FAL_SPEED_100 &&
				!(ability & FAL_PHY_ADV_100TX_HD)))
					return SW_NOT_SUPPORTED;
			}
			break;
		default:
			return SW_FAIL;
	}
	return hsl_phy_modify_mii(dev_id, phy_addr, QCA808X_PHY_CONTROL, mask,
		phy_data);
}

/******************************************************************************
*
* qca808x_phy_reset - reset the phy
*
* reset the phy
*/
sw_error_t qca808x_phy_reset(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = qcaphy_sw_reset(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8081_PHY_V1_1) &&
		qca808x_phy_2500caps(dev_id, phy_addr))
	{
		/*the configure will lost when reset for 2.5G napa*/
		rv = qca808x_phy_ms_seed_enable(dev_id, phy_addr, A_TRUE);
		PHY_RTN_ON_ERROR(rv);
	}

	return SW_OK;
}
/******************************************************************************
*
* qca808x_phy_cdt - cable diagnostic test
*
* cable diagnostic test
*/

static inline fal_cable_status_t _phy_cdt_status_mapping(a_uint16_t status)
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

static sw_error_t qca808x_phy_cdt_start(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t status = 0;
	a_uint16_t ii = 100;
	sw_error_t rv = SW_OK;

	/* RUN CDT */
	rv = hsl_phy_mii_reg_write(dev_id, phy_addr, QCA808X_PHY_CDT_CONTROL,
		QCA808X_RUN_CDT | QCA808X_CABLE_LENGTH_UNIT);
	PHY_RTN_ON_ERROR(rv);

	do {
		aos_mdelay(30);
		status = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA808X_PHY_CDT_CONTROL);
		PHY_RTN_ON_READ_ERROR(status);
	}
	while ((status & QCA808X_RUN_CDT) && (--ii));

	if (ii == 0) {
		return SW_TIMEOUT;
	} else {
		return SW_OK;
	}
}

sw_error_t
qca808x_phy_cdt(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	a_uint16_t cable_delta_time = 0;
	a_uint16_t status;
	sw_error_t rv;

	if ((mdi_pair >= QCA808X_MDI_PAIR_NUM)) {
		return SW_BAD_PARAM;
	}

	rv = qca808x_phy_cdt_start(dev_id, phy_addr);

	if (rv != SW_OK) {
		*cable_status = FAL_CABLE_STATUS_INVALID;
		*cable_len = 0;
		return rv;
	}

	/* Get cable status */
	status = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD3_NUM, QCA808X_PHY_CDT_STATUS);
	PHY_RTN_ON_READ_ERROR(status);

	switch (mdi_pair) {
		case 0:
			*cable_status =
				_phy_cdt_status_mapping((status >> 12) & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
						QCA808X_PHY_CDT_DIAG_PAIR0);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);

			break;
		case 1:
			*cable_status =
				_phy_cdt_status_mapping((status >> 8) & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
						QCA808X_PHY_CDT_DIAG_PAIR1);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);

			break;
		case 2:
			*cable_status =
				_phy_cdt_status_mapping((status >> 4) & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
						QCA808X_PHY_CDT_DIAG_PAIR2);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);

			break;
		case 3:
			*cable_status =
				_phy_cdt_status_mapping(status & 0x3);
			cable_delta_time =
				hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
						QCA808X_PHY_CDT_DIAG_PAIR3);
			PHY_RTN_ON_READ_ERROR(cable_delta_time);

			break;
	}
#ifdef MHT
	/*CDT status open and short are reversed for MHT PHY at analog level*/
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		if(*cable_status == FAL_CABLE_STATUS_OPENED)
			*cable_status = FAL_CABLE_STATUS_SHORT;
		else if(*cable_status == FAL_CABLE_STATUS_SHORT)
			*cable_status = FAL_CABLE_STATUS_OPENED;
	}
#endif
	/* the actual cable length equals to CableDeltaTime * 0.824 */
	*cable_len = ((cable_delta_time & 0xff) * 824) / 1000;

	return rv;
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* qca808x_phy_set_local_loopback
*
* set phy local loopback
*/
sw_error_t
qca808x_phy_set_local_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0, mask = 0;
	fal_port_speed_t cur_speed = 0;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE) {
		/* get the link speed first, then force the corresponding
		 * speed to enable local loopback */
		rv = qcaphy_get_speed(dev_id, phy_addr, &cur_speed);
		PHY_RTN_ON_ERROR(rv);
		rv = qca808x_phy_set_force_speed(dev_id, phy_addr, cur_speed);
		PHY_RTN_ON_ERROR(rv);
		phy_data |= QCA808X_LOCAL_LOOPBACK_ENABLE | QCA808X_CTRL_FULL_DUPLEX;
	} else {
		phy_data |= QCA808X_COMMON_CTRL;
	}
	mask = QCA808X_CTRL_AUTONEGOTIATION_ENABLE | QCA808X_LOCAL_LOOPBACK_ENABLE |
		QCA808X_CTRL_FULL_DUPLEX;
	return hsl_phy_modify_mii(dev_id, phy_addr, QCA808X_PHY_CONTROL, mask,
		phy_data);
}
/******************************************************************************
*
* qca808x_phy_set_remote_loopback
*
* set phy remote loopback
*/
sw_error_t
qca808x_phy_set_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= QCA808X_PHY_REMOTE_LOOPBACK_EN;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL,
		QCA808X_PHY_REMOTE_LOOPBACK_EN, phy_data);
}

/******************************************************************************
*
* qca808x_phy_get_remote_loopback
*
* get phy remote loopback
*/
sw_error_t
qca808x_phy_get_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA808X_PHY_REMOTE_LOOPBACK_EN) {
		*enable = A_TRUE;
	} else {
		*enable = A_FALSE;
	}

	return SW_OK;
}
#endif

sw_error_t
qca808x_phy_get_partner_ability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *ability)
{
	sw_error_t rv = SW_OK;
	a_uint16_t phy_data = 0;

	*ability = 0;
	rv = qcaphy_lp_capability_get(dev_id, phy_addr, ability);
	PHY_RTN_ON_ERROR(rv);
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_LP_2500M_ABILITY);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if(phy_data & QCA808X_LINK_2500BASETX_FULL_DUPLEX)
		*ability |= FAL_PHY_ADV_2500T_FD;

	return SW_OK;
}

sw_error_t
qca808x_phy_get_ability(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *ability)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*ability = 0;
	rv = qcaphy_get_capability(dev_id, phy_addr, ability);
	PHY_RTN_ON_ERROR(rv);

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD1_NUM, QCA808X_MMD1_PMA_CAP_REG);
	PHY_RTN_ON_READ_ERROR(phy_data);
	if (phy_data & QCA808X_STATUS_2500T_FD_CAPS)
	{
		*ability |= FAL_PHY_ADV_2500T_FD;
	}
#ifdef MHT
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_fixup_ability(dev_id, phy_addr, ability);
	}
#endif

	return rv;
}

/******************************************************************************
*
* qca808x_set_autoneg_adv - set the phy autoneg Advertisement
*
*/
sw_error_t
qca808x_phy_set_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t autoneg)
{
	sw_error_t rv = SW_OK;
	a_uint32_t ability = 0;

	rv = qca808x_phy_get_ability(dev_id, phy_addr, &ability);
	PHY_RTN_ON_ERROR(rv);
	if((autoneg & ability) != autoneg)
		return SW_NOT_SUPPORTED;
	rv = qcaphy_set_autoneg_adv(dev_id, phy_addr, autoneg);
	PHY_RTN_ON_ERROR(rv);
	if (qca808x_phy_2500caps(dev_id, phy_addr) == A_TRUE) {
		rv = _qca808x_phy_set_autoneg_adv_ext(dev_id, phy_addr, autoneg);
		PHY_RTN_ON_ERROR(rv);
	} else {
		if(autoneg & FAL_PHY_ADV_2500T_FD) {
			SSDK_ERROR("2.5G auto adv is not supported\n");
			return SW_NOT_SUPPORTED;
		}
	}

	return SW_OK;
}

sw_error_t
_qca808x_phy_get_autoneg_adv_ext(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint16_t *phy_data)
{
	*phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
			QCA808X_PHY_MMD7_AUTONEGOTIATION_CONTROL);
	PHY_RTN_ON_READ_ERROR(*phy_data);

	return SW_OK;
}

/******************************************************************************
*
* qca808x_get_autoneg_adv - get the phy autoneg Advertisement
*
*/
sw_error_t
qca808x_phy_get_autoneg_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * autoneg)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*autoneg = 0;
	rv = qcaphy_get_autoneg_adv(dev_id, phy_addr, autoneg);
	PHY_RTN_ON_ERROR(rv);
	rv = _qca808x_phy_get_autoneg_adv_ext(dev_id, phy_addr, &phy_data);
	if ((rv == SW_OK) &&
			(phy_data & QCA808X_ADVERTISE_2500FULL)) {
		*autoneg |= FAL_PHY_ADV_2500T_FD;
	}

	return rv;
}
/******************************************************************************
*
* qca808x_restart_autoneg - restart the phy autoneg
*
*/
sw_error_t qca808x_phy_restart_autoneg(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	/*before autoneg restart, need to reset fifo to avoid garbage signal*/
	rv = qca808x_phy_fifo_reset(dev_id, phy_addr, A_TRUE);
	PHY_RTN_ON_ERROR(rv);
	return qcaphy_autoneg_restart(dev_id, phy_addr);
}
/******************************************************************************
*
* qca808x_phy_off - power off the phy
*
* Power off the phy
*/
sw_error_t qca808x_phy_poweroff(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

#ifdef MHT
	/*power off PHY with traffic may cause PPE logic issue, so restart
	 autoneg before power off phy to work around the issue*/
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca808x_phy_restart_autoneg(dev_id, phy_addr);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
	rv = qcaphy_poweroff(dev_id, phy_addr);

	return rv;
}

sw_error_t
qca808x_phy_pll_on(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;
#ifdef MHT
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_pll_on(dev_id, phy_addr);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
	return rv;
}

sw_error_t
qca808x_phy_pll_off(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;
#ifdef MHT
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_pll_off(dev_id, phy_addr);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
	return rv;
}

sw_error_t
qca808x_phy_ldo_set(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	sw_error_t rv = SW_OK;
#ifdef MHT
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_ldo_set(dev_id, phy_addr, enable);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
	return rv;
}

#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* qca808x_phy_set_802.3az
*
* set 802.3az status
*/
sw_error_t
qca808x_phy_set_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (enable == A_TRUE) {
		phy_data |= QCA808X_PHY_8023AZ_EEE_1000BT;
		phy_data |= QCA808X_PHY_8023AZ_EEE_100BT;
	}

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_ADDR_8023AZ_EEE_CTRL,
		QCA808X_PHY_8023AZ_EEE_1000BT | QCA808X_PHY_8023AZ_EEE_100BT,
		phy_data);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_set_8023az(dev_id, phy_addr, enable);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
/*qca808x_start*/
	rv = qca808x_phy_restart_autoneg(dev_id, phy_addr);

	return rv;
}

/******************************************************************************
*
* qca808x_phy_get_8023az status
*
* get 8023az status
*/
sw_error_t
qca808x_phy_get_8023az(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	a_uint16_t phy_data = 0;
	a_bool_t enable1 = A_TRUE;
	sw_error_t rv = SW_OK;

	*enable = A_FALSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_ADDR_8023AZ_EEE_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_get_8023az(dev_id, phy_addr, &enable1);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
/*qca808x_start*/
	if ((phy_data & QCA808X_PHY_8023AZ_EEE_1000BT) &&
			(phy_data & QCA808X_PHY_8023AZ_EEE_100BT) && enable1) {
		*enable = A_TRUE;
	}

	return rv;
}

/******************************************************************************
*
* _qca808x_phy_get_8023az_status status
*
* get 8023az status
*/
sw_error_t
_qca808x_phy_get_8023az_status(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t * enable)
{
	a_uint16_t phy_data;
	*enable = A_FALSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD3_NUM, QCA808X_PHY_MMD3_ADDR_8023AZ_EEE_DB);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA808X_PHY_8023AZ_EEE_LP_STAT) {
		*enable = A_TRUE;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_set wol frame mac address
*
* set phy wol frame mac address
*/
sw_error_t
qca808x_phy_set_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	a_uint16_t phy_data3 = 0;
	sw_error_t rv = SW_OK;

	phy_data1 = (mac->uc[0] << 8) | mac->uc[1];
	phy_data2 = (mac->uc[2] << 8) | mac->uc[3];
	phy_data3 = (mac->uc[4] << 8) | mac->uc[5];

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_MAGIC_MAC_CTRL1, phy_data1);
	PHY_RTN_ON_ERROR(rv);

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_MAGIC_MAC_CTRL2, phy_data2);
	PHY_RTN_ON_ERROR(rv);

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_MAGIC_MAC_CTRL3, phy_data3);

	return rv;
}

/******************************************************************************
*
* qca808x_phy_get wol frame mac address
*
* get phy wol frame mac address
*/
sw_error_t
qca808x_phy_get_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1 = 0;
	a_uint16_t phy_data2 = 0;
	a_uint16_t phy_data3 = 0;

	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_MAGIC_MAC_CTRL1);
	PHY_RTN_ON_READ_ERROR(phy_data1);

	phy_data2 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_MAGIC_MAC_CTRL2);
	PHY_RTN_ON_READ_ERROR(phy_data2);

	phy_data3 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_MAGIC_MAC_CTRL3);
	PHY_RTN_ON_READ_ERROR(phy_data3);

	mac->uc[0] = (phy_data1 >> 8);
	mac->uc[1] = (phy_data1 & 0x00ff);
	mac->uc[2] = (phy_data2 >> 8);
	mac->uc[3] = (phy_data2 & 0x00ff);
	mac->uc[4] = (phy_data3 >> 8);
	mac->uc[5] = (phy_data3 & 0x00ff);

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_set wol enable or disable
*
* set phy wol enable or disable
*/
sw_error_t
qca808x_phy_set_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= QCA808X_PHY_WOL_EN;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_CTRL, QCA808X_PHY_WOL_EN, phy_data);
}

/******************************************************************************
*
* qca808x_phy_get_wol status
*
* get wol status
*/
sw_error_t
qca808x_phy_get_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	*enable = A_FALSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_WOL_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA808X_PHY_WOL_EN) {
		*enable = A_TRUE;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_set_hibernate - set hibernate status
*
* set hibernate status
*/
sw_error_t
qca808x_phy_set_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= QCA808X_PHY_HIBERNATION_CFG;
	}

	return hsl_phy_modify_debug(dev_id, phy_addr, QCA808X_DEBUG_PHY_HIBERNATION_CTRL,
		QCA808X_PHY_HIBERNATION_CFG, phy_data);
}

/******************************************************************************
*
* qca808x_phy_get_hibernate - get hibernate status
*
* get hibernate status
*/
sw_error_t
qca808x_phy_get_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	*enable = A_FALSE;

	phy_data = hsl_phy_debug_reg_read(dev_id, phy_addr,
		QCA808X_DEBUG_PHY_HIBERNATION_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA808X_PHY_HIBERNATION_CFG) {
		*enable = A_TRUE;
	}

	return SW_OK;
}

/******************************************************************************
*
* _qca808x_phy_get_hibernate_status - get hibernate status
*
* get hibernate status
*/
sw_error_t
_qca808x_phy_get_hibernate_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data;

	*enable = A_TRUE;

	phy_data = hsl_phy_debug_reg_read(dev_id, phy_addr,
		QCA808X_DEBUG_PHY_HIBERNATION_STAT);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA808X_PHY_HIBERNATION_STAT_EN) {
		*enable = A_FALSE;
	}

	return SW_OK;
}
#endif
/******************************************************************************
*
* qca808x_phy_interface mode set
*
* set qca808x phy interface mode
*/
sw_error_t
qca808x_phy_interface_set_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t interface_mode)
{
	/* qca808x phy will automatically switch the interface mode according
	 * to the speed, 2.5G works on SGMII+, other works on SGMII.
	 */
	sw_error_t rv = SW_OK;
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_interface_set_mode(dev_id, phy_addr,
			interface_mode);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
/*qca808x_start*/
	return rv;
}

/******************************************************************************
*
* qca808x_phy_interface mode get
*
* get qca808x phy interface mode
*/
sw_error_t
qca808x_phy_interface_get_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode)
{
	a_uint16_t phy_data = 0;
	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA808X_PHY_CHIP_CONFIG);
	PHY_RTN_ON_READ_ERROR(phy_data);

	phy_data &= QCA808X_PHY_CHIP_MODE_CFG;
	if (phy_data == QCA808X_PHY_SGMII_BASET) {
		*interface_mode = PHY_SGMII_BASET;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_interface mode status get
*
* get qca808x phy interface mode status
*/
sw_error_t
qca808x_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode_status)
{
	a_uint16_t phy_data = 0;
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		return qca8084_phy_interface_get_mode_status(dev_id, phy_addr,
			interface_mode_status);
	}
#endif
/*qca808x_start*/
	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA808X_PHY_CHIP_CONFIG);
	PHY_RTN_ON_READ_ERROR(phy_data);

	phy_data &= QCA808X_PHY_MODE_MASK;
	switch (phy_data) {
		case QCA808X_PHY_SGMII_PLUS_MODE:
			*interface_mode_status = PORT_SGMII_PLUS;
			break;
		case QCA808X_PHY_SGMII_MODE:
			*interface_mode_status = PHY_SGMII_BASET;
			break;
		default:
			*interface_mode_status = PORT_INTERFACE_MODE_MAX;
			break;
	}

	return SW_OK;
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* qca808x_phy_set_intr_mask - Set interrupt mask with the
* specified device.
*/
sw_error_t
qca808x_phy_set_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_mask_flag)
{
	a_uint16_t phy_data = 0, mask = 0;
	sw_error_t rv = SW_OK;

	mask = QCA808X_INTR_STATUS_UP_CHANGE | QCA808X_INTR_STATUS_DOWN_CHANGE |
		QCA808X_INTR_SPEED_CHANGE | QCA808X_INTR_LINK_SUCCESS_SG |
		QCA808X_INTR_LINK_FAIL_SG | QCA808X_INTR_WOL | QCA808X_INTR_POE;

	if (intr_mask_flag & FAL_PHY_INTR_STATUS_UP_CHANGE) {
		phy_data |= QCA808X_INTR_STATUS_UP_CHANGE;
	}

	if (intr_mask_flag & FAL_PHY_INTR_STATUS_DOWN_CHANGE) {
		phy_data |= QCA808X_INTR_STATUS_DOWN_CHANGE;
	}

	if (intr_mask_flag & FAL_PHY_INTR_SPEED_CHANGE) {
		phy_data |= QCA808X_INTR_SPEED_CHANGE;
	}

	if (intr_mask_flag & FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE) {
		phy_data |= QCA808X_INTR_LINK_SUCCESS_SG;
	}

	if (intr_mask_flag & FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE) {
		phy_data |= QCA808X_INTR_LINK_FAIL_SG;
	}

	if (intr_mask_flag & FAL_PHY_INTR_WOL_STATUS) {
		phy_data |= QCA808X_INTR_WOL;
	}

	if (intr_mask_flag & FAL_PHY_INTR_POE_STATUS) {
		phy_data |= QCA808X_INTR_POE;
	}

	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCA808X_PHY_INTR_MASK, mask,
		phy_data);
	PHY_RTN_ON_ERROR(rv);
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca_mht_phy_intr_enable(dev_id, phy_addr, intr_mask_flag);
		PHY_RTN_ON_ERROR(rv);
	}
#endif

	return rv;
}

/******************************************************************************
*
* qca808x_phy_get_intr_mask - Get interrupt mask with the
* specified device.
*/
sw_error_t
qca808x_phy_get_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_mask_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA808X_PHY_INTR_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_mask_flag = 0;
	if (phy_data & QCA808X_INTR_STATUS_UP_CHANGE) {
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	if (phy_data & QCA808X_INTR_STATUS_DOWN_CHANGE) {
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;
	}

	if (phy_data & QCA808X_INTR_SPEED_CHANGE) {
		*intr_mask_flag |= FAL_PHY_INTR_SPEED_CHANGE;
	}

	if (phy_data & QCA808X_INTR_LINK_SUCCESS_SG) {
		*intr_mask_flag |= FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE;
	}

	if (phy_data & QCA808X_INTR_LINK_FAIL_SG) {
		*intr_mask_flag |= FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE;
	}

	if (phy_data & QCA808X_INTR_WOL) {
		*intr_mask_flag |= FAL_PHY_INTR_WOL_STATUS;
	}

	if (phy_data & QCA808X_INTR_POE) {
		*intr_mask_flag |= FAL_PHY_INTR_POE_STATUS;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_get_intr_status - Get interrupt status with the
* specified device.
*/
sw_error_t
qca808x_phy_get_intr_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_status_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA808X_PHY_INTR_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_status_flag = 0;
	if (phy_data & QCA808X_INTR_STATUS_UP_CHANGE) {
		*intr_status_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	if (phy_data & QCA808X_INTR_STATUS_DOWN_CHANGE) {
		*intr_status_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;
	}

	if (phy_data & QCA808X_INTR_SPEED_CHANGE) {
		*intr_status_flag |= FAL_PHY_INTR_SPEED_CHANGE;
	}

	if (phy_data & QCA808X_INTR_LINK_SUCCESS_SG) {
		*intr_status_flag |= FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE;
	}

	if (phy_data & QCA808X_INTR_LINK_FAIL_SG) {
		*intr_status_flag |= FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE;
	}

	if (phy_data & QCA808X_INTR_WOL) {
		*intr_status_flag |= FAL_PHY_INTR_WOL_STATUS;
	}

	if (phy_data & QCA808X_INTR_POE) {
		*intr_status_flag |= FAL_PHY_INTR_POE_STATUS;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_set_counter - set counter status
*
* set counter  status
*/
sw_error_t
qca808x_phy_set_counter(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= QCA808X_PHY_FRAME_CHECK_EN;
		phy_data |= QCA808X_PHY_XMIT_MAC_CNT_SELFCLR;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_COUNTER_CTRL,
		QCA808X_PHY_FRAME_CHECK_EN | QCA808X_PHY_XMIT_MAC_CNT_SELFCLR,
		phy_data);
}

/******************************************************************************
*
* qca808x_phy_get_counter_status - get counter status
*
* set counter status
*/
sw_error_t
qca808x_phy_get_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
			 a_bool_t * enable)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_COUNTER_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA808X_PHY_FRAME_CHECK_EN) {
		*enable = A_TRUE;
	} else {
		*enable = A_FALSE;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca808x_phy_show show counter statistics
*
* show counter statistics
*/
sw_error_t
qca808x_phy_show_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	a_uint16_t ingress_high_counter = 0;
	a_uint16_t ingress_low_counter = 0;
	a_uint16_t egress_high_counter = 0;
	a_uint16_t egress_low_counter = 0;

	ingress_high_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_INGRESS_COUNTER_HIGH);
	ingress_low_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_INGRESS_COUNTER_LOW);
	counter_infor->RxGoodFrame = (ingress_high_counter << 16 ) | ingress_low_counter;
	counter_infor->RxBadCRC = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_INGRESS_ERROR_COUNTER);

	egress_high_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_EGRESS_COUNTER_HIGH);
	egress_low_counter = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_EGRESS_COUNTER_LOW);
	counter_infor->TxGoodFrame = (egress_high_counter << 16 ) | egress_low_counter;
	counter_infor->TxBadCRC = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_EGRESS_ERROR_COUNTER);

	return SW_OK;
}
#endif
/******************************************************************************
*
* qca808x_phy_set_eee_advertisement
*
* set eee advertisement
*/
sw_error_t
qca808x_phy_set_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adv)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	if (adv & FAL_PHY_EEE_100BASE_T) {
		phy_data |= QCA808X_PHY_EEE_ADV_100M;
	}
	if (adv & FAL_PHY_EEE_1000BASE_T) {
		phy_data |= QCA808X_PHY_EEE_ADV_1000M;
	}

	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_ADDR_8023AZ_EEE_CTRL,
		QCA808X_PHY_EEE_ADV_100M | QCA808X_PHY_EEE_ADV_1000M, phy_data);
	PHY_RTN_ON_ERROR(rv);
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_set_eee_adv(dev_id, phy_addr, adv);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
/*qca808x_start*/
	rv = qca808x_phy_restart_autoneg(dev_id, phy_addr);

	return rv;
}

/******************************************************************************
*
* qca808x_phy_get_eee_advertisement
*
* get eee advertisement
*/
sw_error_t
qca808x_phy_get_eee_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*adv = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_ADDR_8023AZ_EEE_CTRL);

	if (phy_data & QCA808X_PHY_EEE_ADV_100M) {
		*adv |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCA808X_PHY_EEE_ADV_1000M) {
		*adv |= FAL_PHY_EEE_1000BASE_T;
	}
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_get_eee_adv(dev_id, phy_addr, adv);
		PHY_RTN_ON_ERROR (rv);
	}
#endif
/*qca808x_start*/

	return rv;
}
/******************************************************************************
*
* qca808x_phy_get_eee_partner_advertisement
*
* get eee partner advertisement
*/
sw_error_t
qca808x_phy_get_eee_partner_adv(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *adv)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*adv = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_ADDR_8023AZ_EEE_PARTNER);

	if (phy_data & QCA808X_PHY_EEE_PARTNER_ADV_100M) {
		*adv |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCA808X_PHY_EEE_PARTNER_ADV_1000M) {
		*adv |= FAL_PHY_EEE_1000BASE_T;
	}
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_get_eee_partner_adv(dev_id, phy_addr, adv);
		PHY_RTN_ON_ERROR(rv);
	}
#endif
/*qca808x_start*/
	return rv;
}
/******************************************************************************
*
* qca808x_phy_get_eee_capability
*
* get eee capability
*/
sw_error_t
qca808x_phy_get_eee_cap(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *cap)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*cap = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_ADDR_8023AZ_EEE_CAPABILITY);

	if (phy_data & QCA808X_PHY_EEE_CAPABILITY_100M) {
		*cap |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCA808X_PHY_EEE_CAPABILITY_1000M) {
		*cap |= FAL_PHY_EEE_1000BASE_T;
	}
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_get_eee_cap(dev_id, phy_addr, cap);
		PHY_RTN_ON_ERROR(rv);
	}
#endif
/*qca808x_start*/
	return rv;
}
/******************************************************************************
*
* qca808x_phy_get_eee_status
*
* get eee status
*/
sw_error_t
qca808x_phy_get_eee_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *status)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	*status = 0;
	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_TRUE,
		QCA808X_PHY_MMD7_NUM, QCA808X_PHY_MMD7_ADDR_8023AZ_EEE_STATUS);

	if (phy_data & QCA808X_PHY_EEE_STATUS_100M) {
		*status |= FAL_PHY_EEE_100BASE_T;
	}
	if (phy_data & QCA808X_PHY_EEE_STATUS_1000M) {
		*status |= FAL_PHY_EEE_1000BASE_T;
	}
/*qca808x_end*/
#if defined(MHT)
	if(qca808x_phy_id_check(dev_id, phy_addr, QCA8084_PHY))
	{
		rv = qca8084_phy_get_eee_status(dev_id, phy_addr, status);
		PHY_RTN_ON_ERROR(rv);
	}
#endif
/*qca808x_start*/
	return rv;
}

/******************************************************************************
*
* qca808x_phy_led_init - set led behavior
*
* set led behavior
*/
static sw_error_t
qca808x_phy_led_init(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_LED_POLARITY_CTRL,
		QCA808X_PHY_MMD7_LED_POLARITY_ACTIVE_HIGH);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_LED0_CTRL,
		QCA808X_PHY_MMD7_LED0_CTRL_ENABLE);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_LED1_CTRL,
		QCA808X_PHY_MMD7_LED1_CTRL_DISABLE);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_LED2_CTRL,
		QCA808X_PHY_MMD7_LED2_CTRL_DISABLE);
	PHY_RTN_ON_ERROR(rv);

	return SW_OK;
}

static sw_error_t
qca808x_phy_fast_retrain_cfg(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_AUTONEGOTIATION_CONTROL,
		QCA808X_ADVERTISE_2500FULL |
		QCA808X_PHY_FAST_RETRAIN_2500BT |
		QCA808X_PHY_ADV_LOOP_TIMING);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_FAST_RETRAIN_STATUS_CTL,
		QCA808X_PHY_FAST_RETRAIN_CTRL);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_MSE_THRESHOLD_20DB,
		QCA808X_PHY_MSE_THRESHOLD_20DB_VALUE);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_MSE_THRESHOLD_17DB,
		QCA808X_PHY_MSE_THRESHOLD_17DB_VALUE);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_MSE_THRESHOLD_27DB,
		QCA808X_PHY_MSE_THRESHOLD_27DB_VALUE);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD1_NUM,
		QCA808X_PHY_MMD1_MSE_THRESHOLD_28DB,
		QCA808X_PHY_MSE_THRESHOLD_28DB_VALUE);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_ADDR_EEE_LP_ADVERTISEMENT,
		QCA808X_PHY_EEE_ADV_THP);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD7_NUM,
		QCA808X_PHY_MMD7_TOP_OPTION1,
		QCA808X_PHY_TOP_OPTION1_DATA);
	PHY_RTN_ON_ERROR(rv);
	/*adjust the threshold for link down*/
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		0xa100, 0x9203);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		0xa105, 0x8001);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		0xa106, 0x1111);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		0xa103, 0x1698);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		0xa011, 0x5f85);
	PHY_RTN_ON_ERROR(rv);
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		0xa101, 0x48ad);

	return rv;
}

static sw_error_t
qca808x_phy_adc_threshold_set(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t adc_thresold)
{
	return hsl_phy_modify_debug(dev_id, phy_addr, QCA808X_PHY_ADC_THRESHOLD,
		BITS(0, 8), adc_thresold);
}

static sw_error_t
qca8081_phy_hw_init(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	sw_error_t rv = SW_OK;

	/*enable vga when init napa to fix 8023az issue*/
	rv = hsl_phy_modify_mmd(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_ADDR_CLD_CTRL7, QCA808X_PHY_8023AZ_AFE_CTRL_MASK,
		QCA808X_PHY_8023AZ_AFE_EN);
	PHY_RTN_ON_ERROR(rv);
	/*set napa led pin behavior on HK board*/
	rv = qca808x_phy_led_init(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);
	/*special configuration for AZ under 1G speed mode*/
	rv = hsl_phy_mmd_reg_write(dev_id, phy_addr, A_TRUE, QCA808X_PHY_MMD3_NUM,
		QCA808X_PHY_MMD3_AZ_TRAINING_CTRL, QCA808X_PHY_MMD3_AZ_TRAINING_VAL);
	PHY_RTN_ON_ERROR(rv);
	/*config the fast retrain*/
	rv = qca808x_phy_fast_retrain_cfg(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);
	/*enable seed and configure ramdom seed in order that napa can be
		as slave easier*/
	rv = qca808x_phy_ms_seed_enable(dev_id, phy_addr, A_TRUE);
	PHY_RTN_ON_ERROR(rv);
	rv = qca808x_phy_ms_random_seed_set(dev_id, phy_addr);
	PHY_RTN_ON_ERROR(rv);
	/*set adc threshold as 100mv for 10M*/
	rv = qca808x_phy_adc_threshold_set(dev_id, phy_addr,
		QCA808X_PHY_ADC_THRESHOLD_100MV);

	return rv;
}

static sw_error_t
qca808x_phy_hw_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	a_uint32_t port_id = 0, phy_addr = 0, phy_id = 0;
	sw_error_t rv = SW_OK;

	for (port_id = SSDK_PHYSICAL_PORT0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
			rv = qcaphy_get_phy_id(dev_id, phy_addr, &phy_id);
			PHY_RTN_ON_ERROR(rv);
			switch(phy_id)
			{
				case QCA8081_PHY_V1_1:
					rv = qca8081_phy_hw_init (dev_id, phy_addr);
					break;
/*qca808x_end*/
#if defined(MHT)
				case QCA8084_PHY:
					rv = qca8084_phy_hw_init (dev_id, phy_addr);
					break;
#endif
/*qca808x_start*/
				default:
					rv = SW_NOT_SUPPORTED;
			}
		}
	}

	return rv;
}

static sw_error_t
qca808x_phy_function_reset(a_uint32_t dev_id, a_uint32_t phy_addr,
	hsl_phy_function_reset_t phy_reset_type)
{
	sw_error_t rv = SW_OK;

	rv = qca808x_phy_fifo_reset(dev_id, phy_addr, A_TRUE);
	PHY_RTN_ON_ERROR(rv);
	aos_mdelay(50);
	rv = qca808x_phy_fifo_reset(dev_id, phy_addr, A_FALSE);

	return rv;
}

static sw_error_t qca808x_phy_api_ops_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	sw_error_t  ret = SW_OK;
	hsl_phy_ops_t *qca808x_phy_api_ops = NULL;

	qca808x_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (qca808x_phy_api_ops == NULL) {
		SSDK_ERROR("qca808x phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(QCA808X_PHY_CHIP);

	qca808x_phy_api_ops->phy_get_status = qca808x_phy_get_status;
	qca808x_phy_api_ops->phy_speed_get = qcaphy_get_speed;
	qca808x_phy_api_ops->phy_speed_set = qca808x_phy_set_speed;
	qca808x_phy_api_ops->phy_duplex_get = qcaphy_get_duplex;
	qca808x_phy_api_ops->phy_duplex_set = qca808x_phy_set_duplex;
	qca808x_phy_api_ops->phy_autoneg_enable_set = qcaphy_autoneg_enable;
	qca808x_phy_api_ops->phy_restart_autoneg = qca808x_phy_restart_autoneg;
	qca808x_phy_api_ops->phy_autoneg_status_get = qcaphy_autoneg_status;
	qca808x_phy_api_ops->phy_autoneg_adv_set = qca808x_phy_set_autoneg_adv;
	qca808x_phy_api_ops->phy_autoneg_adv_get = qca808x_phy_get_autoneg_adv;
	qca808x_phy_api_ops->phy_link_status_get = qcaphy_get_link_status;
	qca808x_phy_api_ops->phy_reset = qca808x_phy_reset;
	qca808x_phy_api_ops->phy_cdt = qca808x_phy_cdt;
#ifndef IN_PORTCONTROL_MINI
	qca808x_phy_api_ops->phy_mdix_set = qcaphy_set_mdix;
	qca808x_phy_api_ops->phy_mdix_get = qcaphy_get_mdix;
	qca808x_phy_api_ops->phy_mdix_status_get = qcaphy_get_mdix_status;
	qca808x_phy_api_ops->phy_local_loopback_set = qca808x_phy_set_local_loopback;
	qca808x_phy_api_ops->phy_local_loopback_get = qcaphy_get_local_loopback;
	qca808x_phy_api_ops->phy_remote_loopback_set = qca808x_phy_set_remote_loopback;
	qca808x_phy_api_ops->phy_remote_loopback_get = qca808x_phy_get_remote_loopback;
#endif
	qca808x_phy_api_ops->phy_id_get = qcaphy_get_phy_id;
	qca808x_phy_api_ops->phy_power_off = qca808x_phy_poweroff;
	qca808x_phy_api_ops->phy_power_on = qcaphy_poweron;
#ifndef IN_PORTCONTROL_MINI
	qca808x_phy_api_ops->phy_8023az_set = qca808x_phy_set_8023az;
	qca808x_phy_api_ops->phy_8023az_get = qca808x_phy_get_8023az;
	qca808x_phy_api_ops->phy_hibernation_set = qca808x_phy_set_hibernate;
	qca808x_phy_api_ops->phy_hibernation_get = qca808x_phy_get_hibernate;
	qca808x_phy_api_ops->phy_magic_frame_mac_set = qca808x_phy_set_magic_frame_mac;
	qca808x_phy_api_ops->phy_magic_frame_mac_get = qca808x_phy_get_magic_frame_mac;
	qca808x_phy_api_ops->phy_wol_status_set = qca808x_phy_set_wol_status;
	qca808x_phy_api_ops->phy_wol_status_get = qca808x_phy_get_wol_status;
#endif
	qca808x_phy_api_ops->phy_interface_mode_set = qca808x_phy_interface_set_mode;
	qca808x_phy_api_ops->phy_interface_mode_get = qca808x_phy_interface_get_mode;
	qca808x_phy_api_ops->phy_interface_mode_status_get = qca808x_phy_interface_get_mode_status;
#ifndef IN_PORTCONTROL_MINI
	qca808x_phy_api_ops->phy_intr_mask_set = qca808x_phy_set_intr_mask;
	qca808x_phy_api_ops->phy_intr_mask_get = qca808x_phy_get_intr_mask;
	qca808x_phy_api_ops->phy_intr_status_get = qca808x_phy_get_intr_status;
	qca808x_phy_api_ops->phy_counter_set = qca808x_phy_set_counter;
	qca808x_phy_api_ops->phy_counter_get = qca808x_phy_get_counter;
	qca808x_phy_api_ops->phy_counter_show = qca808x_phy_show_counter;
#endif
	qca808x_phy_api_ops->phy_eee_adv_set = qca808x_phy_set_eee_adv;
	qca808x_phy_api_ops->phy_eee_adv_get = qca808x_phy_get_eee_adv;
	qca808x_phy_api_ops->phy_eee_partner_adv_get = qca808x_phy_get_eee_partner_adv;
	qca808x_phy_api_ops->phy_eee_cap_get = qca808x_phy_get_eee_cap;
	qca808x_phy_api_ops->phy_eee_status_get = qca808x_phy_get_eee_status;
	qca808x_phy_api_ops->phy_function_reset = qca808x_phy_function_reset;
	qca808x_phy_api_ops->phy_pll_on = qca808x_phy_pll_on;
	qca808x_phy_api_ops->phy_pll_off = qca808x_phy_pll_off;
	qca808x_phy_api_ops->phy_ldo_set = qca808x_phy_ldo_set;
#ifdef IN_LED
	qca808x_phy_led_api_ops_init(qca808x_phy_api_ops);
#endif
/*qca808x_end*/
#if defined(IN_PTP)
	qca808x_phy_ptp_api_ops_init(&qca808x_phy_api_ops->phy_ptp_ops);
#endif
/*qca808x_start*/
	ret = hsl_phy_api_ops_register(QCA808X_PHY_CHIP, qca808x_phy_api_ops);

	if (ret == SW_OK) {
		SSDK_INFO("qca probe qca808x phy driver succeeded!\n");
	} else {
		SSDK_ERROR("qca probe qca808x phy driver failed! (code: %d)\n", ret);
	}

	return ret;
}

/******************************************************************************
*
* qca808x_phy_init -
*
*/
int qca808x_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
/*qca808x_end*/
	a_uint32_t port_id = 0;
/*qca808x_start*/
	int ret = 0;

	if(phy_ops_flag == A_FALSE &&
			qca808x_phy_api_ops_init(dev_id, port_bmp) == SW_OK) {
		phy_ops_flag = A_TRUE;
	}
	qca808x_phy_hw_init(dev_id, port_bmp);

/*qca808x_end*/
	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id)) {
			qca808x_phydev_init(dev_id, port_id);
		}
	}

	if (qca808x_ssdk_phy_drv_registered == A_FALSE) {
#if defined(IN_LINUX_STD_PTP)
		ret = qca808x_ptp_hook_init();
#endif
		ret |= qca808x_phy_driver_register();
		qca808x_ssdk_phy_drv_registered = A_TRUE;
	}

/*qca808x_start*/
	return ret;
}

void qca808x_phy_exit(a_uint32_t dev_id, a_uint32_t port_bmp)
{
/*qca808x_end*/
	a_uint32_t port_id = 0;

	if (qca808x_ssdk_phy_drv_registered == A_TRUE) {
		qca808x_phy_driver_unregister();
#if defined(IN_LINUX_STD_PTP)
		qca808x_ptp_hook_cleanup();
#endif
		qca808x_ssdk_phy_drv_registered = A_FALSE;
	}

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id)) {
			qca808x_phydev_deinit(dev_id, port_id);
		}
	}
/*qca808x_start*/
}
/*qca808x_end*/
