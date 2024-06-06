/*
 * Copyright (c) 2017, 2019, The Linux Foundation. All rights reserved.
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
#include "qca803x_phy.h"
#include "hsl_phy.h"
#include "qcaphy_common.h"
#include "ssdk_plat.h"

#define QCA803X_PHY_DELAYED_INIT_TICKS msecs_to_jiffies(1000)

typedef struct {
	a_uint32_t dev_id;
	a_uint32_t combo_phy_bmp;
	fal_port_medium_t combo_cfg[SW_MAX_NR_PORT];
	struct delayed_work phy_sync_dwork;
} qca803x_priv_t;

static qca803x_priv_t g_qca803x_phy;
static struct mutex qca803x_reg_lock;

#define QCA803X_LOCKER_INIT		mutex_init(&qca803x_reg_lock)
#define QCA803X_REG_LOCK		mutex_lock(&qca803x_reg_lock)
#define QCA803X_REG_UNLOCK		mutex_unlock(&qca803x_reg_lock)

/******************************************************************************
*
* qca803x_phy_cdt - cable diagnostic test
*
* cable diagnostic test
*/

static inline fal_cable_status_t _phy_cdt_status_mapping(a_uint16_t status)
{
	fal_cable_status_t status_mapping = FAL_CABLE_STATUS_INVALID;

	switch (status) {
		case 3:
			status_mapping = FAL_CABLE_STATUS_INVALID;
			break;
		case 2:
			status_mapping = FAL_CABLE_STATUS_OPENED;
			break;
		case 1:
			status_mapping = FAL_CABLE_STATUS_SHORT;
			break;
		case 0:
			status_mapping = FAL_CABLE_STATUS_NORMAL;
			break;
	}
	return status_mapping;
}

static sw_error_t qca803x_phy_cdt_start(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair)
{
	a_uint16_t status = 0;
	a_uint16_t ii = 100;
	a_uint16_t MDI_PAIR_S = (mdi_pair << 8) & CDT_PAIR_MASK;

	/* RUN CDT */
	hsl_phy_mii_reg_write(dev_id, phy_addr, QCA803X_PHY_CDT_CONTROL,
		QCA803X_RUN_CDT | MDI_PAIR_S);
	do {
		aos_mdelay(30);
		status = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_CDT_CONTROL);
	}
	while ((status & QCA803X_RUN_CDT) && (--ii));

	return SW_OK;
}

sw_error_t
qca803x_phy_cdt(a_uint32_t dev_id, a_uint32_t phy_addr, a_uint32_t mdi_pair,
	fal_cable_status_t * cable_status, a_uint32_t * cable_len)
{
	a_uint16_t status = 0;

	if (mdi_pair >= QCA803X_MDI_PAIR_NUM) {
		//There are only 4 mdi pairs in 1000BASE-T
		return SW_BAD_PARAM;
	}

	qca803x_phy_cdt_start(dev_id, phy_addr, mdi_pair);

	/* Get cable status */
	status = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_CDT_STATUS);
	*cable_status = _phy_cdt_status_mapping((status >> 8) & 0x3);
	/* the actual cable length equals to CableDeltaTime * 0.824 */
	*cable_len = ((status & 0xff) * 824) / 1000;

	return SW_OK;
}
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* qca803x_phy_set_remote_loopback
*
* set phy remote loopback
*/
sw_error_t
qca803x_phy_set_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= 0x0001;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL, BIT(0), phy_data);;
}

/******************************************************************************
*
* qca803x_phy_get_remote_loopback
*
* get phy remote loopback
*/
sw_error_t
qca803x_phy_get_remote_loopback(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE,
		QCA803X_PHY_MMD3_NUM, QCA803X_PHY_MMD3_ADDR_REMOTE_LOOPBACK_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & 0x0001) {
		*enable = A_TRUE;
	} else {
		*enable = A_FALSE;
	}

	return SW_OK;

}
#endif
#ifndef IN_PORTCONTROL_MINI
#if 0
/******************************************************************************
*
* qca803x_phy_reset_done - reset the phy
*
* reset the phy
*/
a_bool_t qca803x_phy_reset_done(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;
	a_uint16_t ii = 200;

	do {
		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_CONTROL);
		aos_mdelay(10);
	}
	while ((!QCA803X_RESET_DONE(phy_data)) && --ii);

	if (ii == 0)
		return A_FALSE;

	return A_TRUE;
}

/******************************************************************************
*
* qca803x_autoneg_done
*
* qca803x_autoneg_done
*/
a_bool_t qca803x_autoneg_done(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;
	a_uint16_t ii = 200;

	do {
		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_STATUS);
		aos_mdelay(10);
	}
	while ((!QCA803X_AUTONEG_DONE(phy_data)) && --ii);

	if (ii == 0)
		return A_FALSE;

	return A_TRUE;
}

/******************************************************************************
*
* qca803x_phy_Speed_Duplex_Resolved
 - reset the phy
*
* reset the phy
*/
a_bool_t qca803x_phy_speed_duplex_resolved(a_uint32_t dev_id, a_uint32_t phy_addr)
{
	a_uint16_t phy_data = 0;
	a_uint16_t ii = 200;

	do {
		phy_data =
			hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_SPEC_STATUS);
		aos_mdelay(10);
	}
	while ((!QCA803X_SPEED_DUPLEX_RESOVLED(phy_data)) && --ii);

	if (ii == 0)
		return A_FALSE;

	return A_TRUE;
}
#endif
#endif
#ifndef IN_PORTCONTROL_MINI
/******************************************************************************
*
* qca803x_phy_set_powersave - set power saving status
*
* set power saving status
*/
sw_error_t
qca803x_phy_set_powersave(a_uint32_t dev_id, a_uint32_t phy_addr, a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv =SW_OK;

	if (enable == A_TRUE)
		phy_data = phy_data | QCA803X_PWR_SAVE_EN;

	rv = hsl_phy_modify_mii(dev_id, phy_addr, QCA803X_PWR_SAVE,
		QCA803X_PWR_SAVE_EN, phy_data);
	PHY_RTN_ON_ERROR(rv);
	return qcaphy_autoneg_restart(dev_id, phy_addr);
}

/******************************************************************************
*
* qca803x_phy_get_powersave - get power saving status
*
* set power saving status
*/
sw_error_t
qca803x_phy_get_powersave(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PWR_SAVE);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & QCA803X_PWR_SAVE_EN)
		*enable = A_TRUE;
	else
		*enable = A_FALSE;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_set wol frame mac address
*
* set phy wol frame mac address
*/
sw_error_t
qca803x_phy_set_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1;
	a_uint16_t phy_data2;
	a_uint16_t phy_data3;

	phy_data1 = (mac->uc[0] << 8) | mac->uc[1];
	phy_data2 = (mac->uc[2] << 8) | mac->uc[3];
	phy_data3 = (mac->uc[4] << 8) | mac->uc[5];

	hsl_phy_mmd_reg_write(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_MAGIC_MAC_CTRL1, phy_data1);

	hsl_phy_mmd_reg_write(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_MAGIC_MAC_CTRL2, phy_data2);

	hsl_phy_mmd_reg_write(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_MAGIC_MAC_CTRL3, phy_data3);

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_get wol frame mac address
*
* get phy wol frame mac address
*/
sw_error_t
qca803x_phy_get_magic_frame_mac(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_mac_addr_t * mac)
{
	a_uint16_t phy_data1;
	a_uint16_t phy_data2;
	a_uint16_t phy_data3;

	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_MAGIC_MAC_CTRL1);

	phy_data2 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_MAGIC_MAC_CTRL2);

	phy_data3 = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_MAGIC_MAC_CTRL3);

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
* qca803x_phy_set wol enable or disable
*
* set phy wol enable or disable
*/
sw_error_t
qca803x_phy_set_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= 0x0020;
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_CTRL, BIT(5), phy_data);
}

/******************************************************************************
*
* qca803x_phy_get_wol status
*
* get wol status
*/
sw_error_t
qca803x_phy_get_wol_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data = 0;

	*enable = A_FALSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD3_NUM,
		QCA803X_PHY_MMD3_WOL_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & 0x0020)
		*enable = A_TRUE;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_set_hibernate - set hibernate status
*
* set hibernate status
*/
sw_error_t
qca803x_phy_set_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;

	if (enable == A_TRUE) {
		phy_data |= 0x8000;
	}
	return hsl_phy_modify_debug(dev_id, phy_addr,
		QCA803X_DEBUG_PHY_HIBERNATION_CTRL, BIT(15), phy_data);
}

/******************************************************************************
*
* qca803x_phy_get_hibernate - get hibernate status
*
* get hibernate status
*/
sw_error_t
qca803x_phy_get_hibernate(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data;

	*enable = A_FALSE;

	phy_data = hsl_phy_debug_reg_read(dev_id, phy_addr,
		QCA803X_DEBUG_PHY_HIBERNATION_CTRL);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (phy_data & 0x8000)
		*enable = A_TRUE;

	return SW_OK;
}
#endif
sw_error_t
__phy_chip_config_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	qca803x_cfg_type_t cfg_sel, qca803x_cfg_t *cfg_value)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_CHIP_CONFIG);
	PHY_RTN_ON_READ_ERROR(phy_data);

	if (cfg_sel == QCA803X_CHIP_CFG_STAT)
		*cfg_value = (phy_data & QCA803X_PHY_CHIP_MODE_STAT) >> 4;
	else
		*cfg_value = phy_data & QCA803X_PHY_CHIP_MODE_CFG;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_interface mode set
*
* set qca803x phy interface mode
*/
sw_error_t
qca803x_phy_interface_set_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t interface_mode)
{
	a_uint16_t phy_data = 0;

	QCA803X_REG_LOCK;
	switch (interface_mode) {
		case PORT_RGMII_BASET:
			phy_data |= QCA803X_PHY_RGMII_BASET;
			break;
		case PHY_SGMII_BASET:
			phy_data |= QCA803X_PHY_SGMII_BASET;
			break;
		case PORT_RGMII_BX1000:
			phy_data |= QCA803X_PHY_BX1000_RGMII_50;
			break;
		case PORT_RGMII_FX100:
			phy_data |= QCA803X_PHY_FX100_RGMII_50;
			break;
		case PORT_RGMII_AMDET:
			phy_data |= QCA803X_PHY_RGMII_AMDET;
			break;
		default:
			QCA803X_REG_UNLOCK;
			return SW_BAD_PARAM;
	}

	hsl_phy_modify_mii(dev_id, phy_addr, QCA803X_PHY_CHIP_CONFIG,
		BITS(0, 4), phy_data);
	QCA803X_REG_UNLOCK;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_interface mode get
*
* get qca803x phy interface mode
*/
sw_error_t
qca803x_phy_interface_get_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode)
{
	qca803x_cfg_t cfg_value;

	PHY_RTN_ON_ERROR(__phy_chip_config_get(dev_id, phy_addr,
		QCA803X_CHIP_CFG_SET, &cfg_value));
	switch (cfg_value) {
		case QCA803X_PHY_RGMII_BASET:
			*interface_mode = PORT_RGMII_BASET;
			break;
		case  QCA803X_PHY_SGMII_BASET:
			*interface_mode = PHY_SGMII_BASET;
			break;
		case QCA803X_PHY_BX1000_RGMII_50:
			*interface_mode = PORT_RGMII_BX1000;
			break;
		case QCA803X_PHY_FX100_RGMII_50:
			*interface_mode = PORT_RGMII_FX100;
			break;
		case QCA803X_PHY_RGMII_AMDET:
			*interface_mode = PORT_RGMII_AMDET;
			break;
		default:
			*interface_mode = PORT_INTERFACE_MODE_MAX;
			break;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_interface mode status get
*
* get qca803x phy interface mode status
*/
sw_error_t
qca803x_phy_interface_get_mode_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_interface_mode_t *interface_mode_status)
{
	qca803x_cfg_t cfg_value;

	PHY_RTN_ON_ERROR(__phy_chip_config_get(dev_id, phy_addr,
		QCA803X_CHIP_CFG_STAT, &cfg_value));

	switch (cfg_value) {
		case QCA803X_PHY_RGMII_BASET:
			*interface_mode_status = PORT_RGMII_BASET;
			break;
		case QCA803X_PHY_SGMII_BASET:
			*interface_mode_status = PHY_SGMII_BASET;
			break;
		case QCA803X_PHY_BX1000_RGMII_50:
			*interface_mode_status = PORT_RGMII_BX1000;
			break;
		case QCA803X_PHY_FX100_RGMII_50:
			*interface_mode_status = PORT_RGMII_FX100;
			break;
		case QCA803X_PHY_RGMII_AMDET:
			*interface_mode_status = PORT_RGMII_AMDET;
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
* qca803x_phy_set_intr_mask - Set interrupt mask with the
* specified device.
*/
sw_error_t
qca803x_phy_set_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_mask_flag)
{
	a_uint16_t phy_data = 0;
	a_uint32_t mask = 0;

	mask = QCA803X_INTR_STATUS_UP_CHANGE | QCA803X_INTR_STATUS_DOWN_CHANGE |
		QCA803X_INTR_SPEED_CHANGE | QCA803X_INTR_BX_FX_STATUS_UP_CHANGE |
		QCA803X_INTR_BX_FX_STATUS_DOWN_CHANGE | QCA803X_INTR_WOL |
		QCA803X_INTR_POE;

	if (FAL_PHY_INTR_STATUS_UP_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_STATUS_UP_CHANGE;
	}

	if (FAL_PHY_INTR_STATUS_DOWN_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_STATUS_DOWN_CHANGE;
	}

	if (FAL_PHY_INTR_SPEED_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_SPEED_CHANGE;
	}

	/* DUPLEX INTR bit is reserved for AR803X phy
	if (FAL_PHY_INTR_DUPLEX_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_DUPLEX_CHANGE;
	} else {
		phy_data &= (~QCA803X_INTR_DUPLEX_CHANGE);
	}
	*/

	if (FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_BX_FX_STATUS_UP_CHANGE;
	}

	if (FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_BX_FX_STATUS_DOWN_CHANGE;
	} else {
		phy_data &= (~QCA803X_INTR_BX_FX_STATUS_DOWN_CHANGE);
	}

	if (FAL_PHY_INTR_MEDIA_STATUS_CHANGE & intr_mask_flag) {
		phy_data |= QCA803X_INTR_MEDIA_STATUS_CHANGE;
	}

	if (FAL_PHY_INTR_WOL_STATUS & intr_mask_flag) {
		phy_data |= QCA803X_INTR_WOL;
	}

	if (FAL_PHY_INTR_POE_STATUS & intr_mask_flag) {
		phy_data |= QCA803X_INTR_POE;
	}
	return hsl_phy_modify_mii(dev_id, phy_addr, QCA803X_PHY_INTR_MASK,
		mask, phy_data);
}

/******************************************************************************
*
* qca803x_phy_get_intr_mask - Get interrupt mask with the
* specified device.
*/
sw_error_t
qca803x_phy_get_intr_mask(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_mask_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_INTR_MASK);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_mask_flag = 0;
	if (QCA803X_INTR_STATUS_UP_CHANGE & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	if (QCA803X_INTR_STATUS_DOWN_CHANGE & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;
	}

	if (QCA803X_INTR_SPEED_CHANGE & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_SPEED_CHANGE;
	}

	/* DUPLEX INTR bit is reserved for AR803X phy
	if (QCA803X_INTR_DUPLEX_CHANGE & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_DUPLEX_CHANGE;
	}
	*/

	if (QCA803X_INTR_BX_FX_STATUS_UP_CHANGE & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE;
	}

	if (QCA803X_INTR_BX_FX_STATUS_DOWN_CHANGE & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE;
	}

	if (QCA803X_INTR_MEDIA_STATUS_CHANGE  & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_MEDIA_STATUS_CHANGE;
	}

	if (QCA803X_INTR_WOL  & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_WOL_STATUS;
	}

	if (QCA803X_INTR_POE  & phy_data) {
		*intr_mask_flag |= FAL_PHY_INTR_POE_STATUS;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_get_intr_status - Get interrupt status with the
* specified device.
*/
sw_error_t
qca803x_phy_get_intr_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t * intr_status_flag)
{
	a_uint16_t phy_data = 0;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_INTR_STATUS);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*intr_status_flag = 0;
	if (QCA803X_INTR_STATUS_UP_CHANGE & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_STATUS_UP_CHANGE;
	}

	if (QCA803X_INTR_STATUS_DOWN_CHANGE & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_STATUS_DOWN_CHANGE;
	}

	if (QCA803X_INTR_SPEED_CHANGE & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_SPEED_CHANGE;
	}

	/* DUPLEX INTR bit is reserved for AR803X phy
	if (QCA803X_INTR_DUPLEX_CHANGE & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_DUPLEX_CHANGE;
	}
	*/

	if (QCA803X_INTR_BX_FX_STATUS_UP_CHANGE & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_BX_FX_STATUS_UP_CHANGE;
	}

	if (QCA803X_INTR_BX_FX_STATUS_DOWN_CHANGE & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_BX_FX_STATUS_DOWN_CHANGE;
	}
	if (QCA803X_INTR_MEDIA_STATUS_CHANGE  & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_MEDIA_STATUS_CHANGE;
	}

	if (QCA803X_INTR_WOL  & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_WOL_STATUS;
	}

	if (QCA803X_INTR_POE  & phy_data) {
		*intr_status_flag |= FAL_PHY_INTR_POE_STATUS;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_set combo medium type
*
* set combo medium fiber or copper
*/
sw_error_t
qca803x_phy_set_combo_prefer_medium(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_medium_t phy_medium)
{
	a_uint16_t phy_data = 0;

	QCA803X_REG_LOCK;
	if (phy_medium == PHY_MEDIUM_FIBER)
		phy_data |= QCA803X_PHY_PREFER_FIBER;
	else if (phy_medium == PHY_MEDIUM_COPPER)
		phy_data &= ~QCA803X_PHY_PREFER_FIBER;
	else {
		QCA803X_REG_UNLOCK;
		return SW_BAD_PARAM;
	}

	hsl_phy_modify_mii(dev_id, phy_addr, QCA803X_PHY_CHIP_CONFIG,
		QCA803X_PHY_PREFER_FIBER, phy_data);
	QCA803X_REG_UNLOCK;

	/* soft reset after switching combo medium*/
	return qcaphy_sw_reset(dev_id, phy_addr);
}

/******************************************************************************
*
* qca803x_phy_get combo medium type
*
* get combo medium fiber or copper
*/
sw_error_t
qca803x_phy_get_combo_prefer_medium(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_medium_t * phy_medium)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_CHIP_CONFIG);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*phy_medium =
		(phy_data & QCA803X_PHY_PREFER_FIBER) ? PHY_MEDIUM_FIBER :
		PHY_MEDIUM_COPPER;

	return SW_OK;
}

/******************************************************************************
*
*  qca803x phy activer medium
*
*  get qca803x phy current active medium, fiber or copper;
*/
static fal_port_medium_t __phy_active_medium_get(a_uint32_t dev_id,
	a_uint32_t phy_addr)
{
	qca803x_cfg_t cfg_value;
	sw_error_t rv = SW_OK;

	rv = __phy_chip_config_get(dev_id, phy_addr,
			QCA803X_CHIP_CFG_STAT, &cfg_value);
	if (rv != SW_OK)
		return PHY_MEDIUM_MAX;

	switch (cfg_value) {
		case QCA803X_PHY_RGMII_BASET:
		case QCA803X_PHY_SGMII_BASET:
			return PHY_MEDIUM_COPPER;
		case QCA803X_PHY_BX1000_RGMII_50:
		case QCA803X_PHY_FX100_RGMII_50:
			return PHY_MEDIUM_FIBER;
		case QCA803X_PHY_RGMII_AMDET:
		default:
			return PHY_MEDIUM_MAX;
	}
}

/******************************************************************************
*
* qca803x_phy_get current combo medium type copper or fiber
*
* get current combo medium type
*/
sw_error_t
qca803x_phy_get_combo_current_medium_type(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_medium_t * phy_medium)
{
	fal_port_medium_t phy_cur_meduim = __phy_active_medium_get(dev_id, phy_addr);

	/* auto media select is not done
	 * or link down, then return prefer medium */
	if (phy_cur_meduim == PHY_MEDIUM_MAX)
		qca803x_phy_get_combo_prefer_medium(dev_id, phy_addr, phy_medium);
	else
		*phy_medium = phy_cur_meduim;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_set fiber mode 1000bx or 100fx
*
* set combo fbier mode
*/
sw_error_t
qca803x_phy_set_combo_fiber_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_fiber_mode_t fiber_mode)
{
	a_uint16_t phy_data = 0;

	QCA803X_REG_LOCK;
	if (fiber_mode == PHY_FIBER_1000BX) {
		phy_data |= QCA803X_PHY_FIBER_MODE_1000BX;
	} else if (fiber_mode == PHY_FIBER_100FX) {
		phy_data &= ~QCA803X_PHY_FIBER_MODE_1000BX;
	} else {
		QCA803X_REG_UNLOCK;
		return SW_BAD_PARAM;
	}

	hsl_phy_modify_mii(dev_id, phy_addr, QCA803X_PHY_CHIP_CONFIG,
		QCA803X_PHY_FIBER_MODE_1000BX, phy_data);
	QCA803X_REG_UNLOCK;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_get fiber mode 1000bx or 100fx
*
* get combo fbier mode
*/
sw_error_t
qca803x_phy_get_combo_fiber_mode(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_fiber_mode_t * fiber_mode)
{
	a_uint16_t phy_data;

	phy_data = hsl_phy_mii_reg_read(dev_id, phy_addr, QCA803X_PHY_CHIP_CONFIG);
	PHY_RTN_ON_READ_ERROR(phy_data);

	*fiber_mode =
		(phy_data & QCA803X_PHY_FIBER_MODE_1000BX) ? PHY_FIBER_1000BX :
		PHY_FIBER_100FX;

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_set_counter_status - set counter status
*
*/
sw_error_t
qca803x_phy_set_counter_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t enable)
{
	a_uint16_t phy_data = 0;
	static a_uint16_t frame_dir = 0;

	/*for qca803x phy, tx and rx cannot be all enabled one time,
	  so we will enable tx or rx based current state, enable rx if
	  current is tx and enable tx if current is rx*/
	if (enable == A_TRUE)
	{
		phy_data |= QCA803X_PHY_MMD7_FRAME_CHECK;
		/*enable RX counter*/
		if(frame_dir)
		{
			SSDK_INFO("ENABLE QCA803X RX COUNTER\n");
			phy_data &= ~QCA803X_PHY_MMD7_FRAME_DIR;
			frame_dir = 0;
		}
		/*enable TX counter*/
		else
		{
			SSDK_INFO("ENABLE QCA803X TX COUNTER\n");
			phy_data |= QCA803X_PHY_MMD7_FRAME_DIR;
			frame_dir = 1;
		}
	}

	return hsl_phy_modify_mmd(dev_id, phy_addr, A_FALSE, QCA803X_PHY_MMD7_NUM,
		QCA803X_PHY_MMD7_FRAME_CTRL,
		QCA803X_PHY_MMD7_FRAME_CHECK | QCA803X_PHY_MMD7_FRAME_DIR, phy_data);
}

/******************************************************************************
*
* qca803x_phy_get_counter_status - get counter status
*
*/
sw_error_t
qca803x_phy_get_counter_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_bool_t * enable)
{
	a_uint16_t phy_data;
	*enable = A_FALSE;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr,  A_FALSE,
		QCA803X_PHY_MMD7_NUM, QCA803X_PHY_MMD7_FRAME_CTRL);

	if (phy_data & QCA803X_PHY_MMD7_FRAME_CHECK) {
		*enable = A_TRUE;
	}

	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_show_counter - show counter statistics
*
*/
sw_error_t
qca803x_phy_show_counter(a_uint32_t dev_id, a_uint32_t phy_addr,
	fal_port_counter_info_t * counter_infor)
{
	a_uint16_t phy_data, phy_data1;
	a_uint16_t frame_dir = 0;

	phy_data = hsl_phy_mmd_reg_read(dev_id, phy_addr,  A_FALSE,
		QCA803X_PHY_MMD7_NUM, QCA803X_PHY_MMD7_FRAME_CTRL);
	phy_data1 = hsl_phy_mmd_reg_read(dev_id, phy_addr,  A_FALSE,
		QCA803X_PHY_MMD7_NUM, QCA803X_PHY_MMD7_FRAME_DATA);
	frame_dir = (phy_data & QCA803X_PHY_MMD7_FRAME_DIR) >> 14;

	if(frame_dir)
	{
		/*get the counter of tx*/
		counter_infor->TxGoodFrame = phy_data1 & QCA803X_PHY_FRAME_CNT;
		counter_infor->TxBadCRC = (phy_data1 & QCA803X_PHY_FRAME_ERROR) >> 8;
		counter_infor->RxGoodFrame = 0;
		counter_infor->RxBadCRC = 0;
	}
	else
	{
		/*get the counter of rx*/
		counter_infor->TxGoodFrame = 0;
		counter_infor->TxBadCRC = 0;
		counter_infor->RxGoodFrame = phy_data1 & QCA803X_PHY_FRAME_CNT;
		counter_infor->RxBadCRC = (phy_data1 & QCA803X_PHY_FRAME_ERROR) >> 8;
	}

	return SW_OK;
}

#endif
/******************************************************************************
*
* qca803x_phy_get status
*
* get phy status
*/
sw_error_t
qca803x_phy_get_status(a_uint32_t dev_id, a_uint32_t phy_addr,
	struct port_phy_status *phy_status)
{
	a_uint16_t phy_data = 0;
	sw_error_t rv = SW_OK;

	rv = qcaphy_status_get(dev_id, phy_addr, phy_status);
	PHY_RTN_ON_ERROR(rv);
	if(phy_status->link_status) {
		phy_data |= QCA803X_PHY_MSE_THRESH_LINK_UP;
	} else {
		phy_data |= QCA803X_PHY_MSE_THRESH_LINK_DOWN;
	}
	return hsl_phy_modify_debug(dev_id, phy_addr, QCA803X_PHY_SPEC_STATUS,
		QCA803X_PHY_MSE_THRESH_MASK, phy_data);
}

static sw_error_t qca803x_phy_api_ops_init(void)
{
	sw_error_t  ret = SW_OK;
	hsl_phy_ops_t *qca803x_phy_api_ops = NULL;

	qca803x_phy_api_ops = kzalloc(sizeof(hsl_phy_ops_t), GFP_KERNEL);
	if (qca803x_phy_api_ops == NULL) {
		SSDK_ERROR("qca803x phy ops kzalloc failed!\n");
		return -ENOMEM;
	}

	phy_api_ops_init(QCA803X_PHY_CHIP);

	qca803x_phy_api_ops->phy_speed_get = qcaphy_get_speed;
	qca803x_phy_api_ops->phy_speed_set = qcaphy_set_speed;
	qca803x_phy_api_ops->phy_duplex_get = qcaphy_get_duplex;
	qca803x_phy_api_ops->phy_duplex_set = qcaphy_set_duplex;
	qca803x_phy_api_ops->phy_autoneg_enable_set = qcaphy_autoneg_enable;
	qca803x_phy_api_ops->phy_restart_autoneg = qcaphy_autoneg_restart;
	qca803x_phy_api_ops->phy_autoneg_status_get = qcaphy_autoneg_status;
	qca803x_phy_api_ops->phy_autoneg_adv_set = qcaphy_set_autoneg_adv;
	qca803x_phy_api_ops->phy_autoneg_adv_get = qcaphy_get_autoneg_adv;
	qca803x_phy_api_ops->phy_link_status_get = qcaphy_get_link_status;
	qca803x_phy_api_ops->phy_reset = qcaphy_sw_reset;
#ifndef IN_PORTCONTROL_MINI
	qca803x_phy_api_ops->phy_powersave_set = qca803x_phy_set_powersave;
	qca803x_phy_api_ops->phy_powersave_get = qca803x_phy_get_powersave;
#endif
	qca803x_phy_api_ops->phy_cdt = qca803x_phy_cdt;
#ifndef IN_PORTCONTROL_MINI
	qca803x_phy_api_ops->phy_mdix_set = qcaphy_set_mdix;
	qca803x_phy_api_ops->phy_mdix_get = qcaphy_get_mdix;
	qca803x_phy_api_ops->phy_mdix_status_get = qcaphy_get_mdix_status;
	qca803x_phy_api_ops->phy_local_loopback_set = qcaphy_set_local_loopback;
	qca803x_phy_api_ops->phy_local_loopback_get = qcaphy_get_local_loopback;
	qca803x_phy_api_ops->phy_remote_loopback_set = qca803x_phy_set_remote_loopback;
	qca803x_phy_api_ops->phy_remote_loopback_get = qca803x_phy_get_remote_loopback;
#endif
	qca803x_phy_api_ops->phy_id_get = qcaphy_get_phy_id;
	qca803x_phy_api_ops->phy_power_off = qcaphy_poweroff;
	qca803x_phy_api_ops->phy_power_on = qcaphy_poweron;
#ifndef IN_PORTCONTROL_MINI
	qca803x_phy_api_ops->phy_8023az_set = qcaphy_set_8023az;
	qca803x_phy_api_ops->phy_8023az_get = qcaphy_get_8023az;
	qca803x_phy_api_ops->phy_hibernation_set = qca803x_phy_set_hibernate;
	qca803x_phy_api_ops->phy_hibernation_get = qca803x_phy_get_hibernate;
	qca803x_phy_api_ops->phy_magic_frame_mac_set = qca803x_phy_set_magic_frame_mac;
	qca803x_phy_api_ops->phy_magic_frame_mac_get = qca803x_phy_get_magic_frame_mac;
	qca803x_phy_api_ops->phy_wol_status_set = qca803x_phy_set_wol_status;
	qca803x_phy_api_ops->phy_wol_status_get = qca803x_phy_get_wol_status;
#endif
	qca803x_phy_api_ops->phy_interface_mode_set = qca803x_phy_interface_set_mode;
	qca803x_phy_api_ops->phy_interface_mode_get = qca803x_phy_interface_get_mode;
	qca803x_phy_api_ops->phy_interface_mode_status_get = qca803x_phy_interface_get_mode_status;
#ifndef IN_PORTCONTROL_MINI
	qca803x_phy_api_ops->phy_intr_mask_set = qca803x_phy_set_intr_mask;
	qca803x_phy_api_ops->phy_intr_mask_get = qca803x_phy_get_intr_mask;
	qca803x_phy_api_ops->phy_intr_status_get = qca803x_phy_get_intr_status;
	qca803x_phy_api_ops->phy_combo_prefer_medium_set = qca803x_phy_set_combo_prefer_medium;
	qca803x_phy_api_ops->phy_combo_prefer_medium_get = qca803x_phy_get_combo_prefer_medium;
	qca803x_phy_api_ops->phy_combo_medium_status_get = qca803x_phy_get_combo_current_medium_type;
	qca803x_phy_api_ops->phy_combo_fiber_mode_set = qca803x_phy_set_combo_fiber_mode;
	qca803x_phy_api_ops->phy_combo_fiber_mode_get = qca803x_phy_get_combo_fiber_mode;
	qca803x_phy_api_ops->phy_counter_set = qca803x_phy_set_counter_status;
	qca803x_phy_api_ops->phy_counter_get = qca803x_phy_get_counter_status;
	qca803x_phy_api_ops->phy_counter_show = qca803x_phy_show_counter;
#endif
	qca803x_phy_api_ops->phy_get_status = qca803x_phy_get_status;
	qca803x_phy_api_ops->phy_eee_adv_set = qcaphy_set_eee_adv;
	qca803x_phy_api_ops->phy_eee_adv_get = qcaphy_get_eee_adv;
	qca803x_phy_api_ops->phy_eee_partner_adv_get = qcaphy_get_eee_partner_adv;
	qca803x_phy_api_ops->phy_eee_cap_get = qcaphy_get_eee_cap;
	qca803x_phy_api_ops->phy_eee_status_get = qcaphy_get_eee_status;

	ret = hsl_phy_api_ops_register(QCA803X_PHY_CHIP, qca803x_phy_api_ops);

	if (ret == SW_OK)
		SSDK_INFO("qca probe qca803x phy driver succeeded!\n");
	else
		SSDK_ERROR("qca probe qca803x phy driver failed! (code: %d)\n", ret);

	return ret;
}

static sw_error_t
_qca803x_phy_set_combo_page_regs(a_uint32_t dev_id, a_uint32_t phy_id,
	fal_port_medium_t phy_medium)
{
	a_uint16_t phy_data = 0;

	if (phy_medium == PHY_MEDIUM_FIBER) {
		phy_data &= ~QCA803X_PHY_COPPER_PAGE_SEL;
	}
	else if (phy_medium == PHY_MEDIUM_COPPER) {
		phy_data |= QCA803X_PHY_COPPER_PAGE_SEL;
	}
	else {
		return SW_BAD_PARAM;
	}

	return hsl_phy_modify_mii(dev_id, phy_id, QCA803X_PHY_CHIP_CONFIG,
		QCA803X_PHY_COPPER_PAGE_SEL, phy_data);
}

void qca803x_combo_phy_polling(qca803x_priv_t *priv)
{

	qca803x_cfg_t cfg_value;
	a_uint32_t combo_phy_addr = 0;
	a_uint32_t combo_bits = priv->combo_phy_bmp;
	fal_port_medium_t combo_cfg_new = PHY_MEDIUM_COPPER;

	while (combo_bits) {
		if (combo_bits & 1) {
			QCA803X_REG_LOCK;
			__phy_chip_config_get(priv->dev_id, combo_phy_addr,
					QCA803X_CHIP_CFG_STAT, &cfg_value);

			switch (cfg_value) {
				case QCA803X_PHY_RGMII_BASET:
				case QCA803X_PHY_SGMII_BASET:
					combo_cfg_new = PHY_MEDIUM_COPPER;
					break;
				case QCA803X_PHY_BX1000_RGMII_50:
				case QCA803X_PHY_FX100_RGMII_50:
					combo_cfg_new = PHY_MEDIUM_FIBER;
					break;
				default:
					combo_cfg_new = PHY_MEDIUM_COPPER;
			}

			if (priv->combo_cfg[combo_phy_addr] != combo_cfg_new) {
				priv->combo_cfg[combo_phy_addr] = combo_cfg_new;
				_qca803x_phy_set_combo_page_regs(priv->dev_id, combo_phy_addr, combo_cfg_new);
			}

			QCA803X_REG_UNLOCK;
		}
		combo_bits >>= 1;
		combo_phy_addr++;
	}
}

void
qca803x_phy_polling_work(struct work_struct *work)
{
	qca803x_priv_t *priv = container_of(work, qca803x_priv_t,
					phy_sync_dwork.work);
	qca803x_combo_phy_polling(priv);

	schedule_delayed_work(&priv->phy_sync_dwork,
					QCA803X_PHY_DELAYED_INIT_TICKS);
}

sw_error_t
qca803x_phy_work_start(a_uint32_t dev_id)
{
	qca803x_priv_t *priv = &g_qca803x_phy;
	priv->dev_id = dev_id;

	INIT_DELAYED_WORK(&priv->phy_sync_dwork,
					qca803x_phy_polling_work);
	schedule_delayed_work(&priv->phy_sync_dwork,
					QCA803X_PHY_DELAYED_INIT_TICKS);
	return SW_OK;
}

/******************************************************************************
*
* qca803x_phy_hw_init
*
*/
sw_error_t
qca803x_phy_hw_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	sw_error_t  ret = SW_OK;
	a_uint32_t port_id = 0, phy_addr = 0, mac_mode = 0;

	for (port_id = 0; port_id < SW_MAX_NR_PORT; port_id ++)
	{
		if (port_bmp & (0x1 << port_id))
		{
			/*config phy mode based on the mac mode DT*/
			switch (port_id) {
				case SSDK_PHYSICAL_PORT0:
					mac_mode = ssdk_dt_global_get_mac_mode(dev_id,
						SSDK_UNIPHY_INSTANCE0);
					break;
				case SSDK_PHYSICAL_PORT6:
					mac_mode = ssdk_dt_global_get_mac_mode(dev_id,
						SSDK_UNIPHY_INSTANCE2);
					break;
				default:
					mac_mode = PORT_WRAPPER_MAX;
			}

			phy_addr = qca_ssdk_port_to_phy_addr(dev_id, port_id);
			if (mac_mode == PORT_WRAPPER_SGMII_CHANNEL0)
				qca803x_phy_interface_set_mode(dev_id, phy_addr, PHY_SGMII_BASET);
			else if (mac_mode == PORT_WRAPPER_RGMII)
				qca803x_phy_interface_set_mode(dev_id, phy_addr, PORT_RGMII_BASET);

			if (A_TRUE == hsl_port_phy_combo_capability_get(dev_id, port_id)) {
				g_qca803x_phy.combo_phy_bmp |= (0x1 << phy_addr);
				qca803x_phy_interface_set_mode(dev_id, phy_addr, PORT_RGMII_AMDET);
			}
			/*config the times that MSE is over threshold as max*/
			ret = hsl_phy_modify_debug(dev_id, phy_addr,
				QCA803X_DEBUG_MSE_OVER_THRESH_TIMES,
				QCA803X_PHY_MSE_OVER_THRESH_TIMES_MAX,
				QCA803X_PHY_MSE_OVER_THRESH_TIMES_MAX);
			PHY_RTN_ON_ERROR(ret);
			/*disable Extended next page*/
			ret = hsl_phy_modify_mii(dev_id, phy_addr, QCA803X_AUTONEG_ADVERT,
				QCA803X_EXTENDED_NEXT_PAGE_EN, 0);
			PHY_RTN_ON_ERROR(ret);
		}
	}

	/* start polling task for the combo port */
	if (g_qca803x_phy.combo_phy_bmp)
		qca803x_phy_work_start(dev_id);

	return ret;
}

/******************************************************************************
*
* qca803x_phy_init -
*
*/
int qca803x_phy_init(a_uint32_t dev_id, a_uint32_t port_bmp)
{
	static a_uint32_t phy_ops_flag = 0;

	if(phy_ops_flag == 0) {
		QCA803X_LOCKER_INIT;
		qca803x_phy_api_ops_init();
		phy_ops_flag = 1;
	}

	qca803x_phy_hw_init(dev_id, port_bmp);
	return 0;
}
