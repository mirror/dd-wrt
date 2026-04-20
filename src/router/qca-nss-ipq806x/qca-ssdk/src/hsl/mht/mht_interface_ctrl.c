/*
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "hsl_phy.h"
#include "ssdk_plat.h"
#include "mht_interface_ctrl.h"
#include "mht_sec_ctrl.h"
#include "hsl_dev.h"
#include "qca808x_phy.h"
#include "qca8084_phy.h"
#include "ssdk_mht_clk.h"
#include "ssdk_clk.h"

static a_uint16_t
mht_uniphy_xpcs_mmd_read(a_uint32_t dev_id, a_uint16_t mmd_num,
	a_uint16_t mmd_reg)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_xpcs_addr = 0;

	rv = qca_mht_serdes_addr_get(dev_id, MHT_UNIPHY_XPCS, &uniphy_xpcs_addr);
	if(rv != SW_OK)
		return PHY_INVALID_DATA;

	return hsl_phy_mmd_reg_read(dev_id, uniphy_xpcs_addr, A_TRUE, mmd_num,
		mmd_reg);
}

static sw_error_t
mht_uniphy_xpcs_mmd_write(a_uint32_t dev_id, a_uint16_t mmd_num,
	a_uint16_t mmd_reg, a_uint16_t reg_val)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_xpcs_addr = 0;

	rv = qca_mht_serdes_addr_get(dev_id, MHT_UNIPHY_XPCS, &uniphy_xpcs_addr);
	PHY_RTN_ON_ERROR (rv);

	return hsl_phy_mmd_reg_write(dev_id, uniphy_xpcs_addr, A_TRUE, mmd_num,
		mmd_reg, reg_val);
}

static sw_error_t
mht_uniphy_xpcs_modify_mmd(a_uint32_t dev_id, a_uint32_t mmd_num, a_uint32_t mmd_reg,
	a_uint32_t mask, a_uint32_t value)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_xpcs_addr = 0;

	rv = qca_mht_serdes_addr_get(dev_id, MHT_UNIPHY_XPCS, &uniphy_xpcs_addr);
	PHY_RTN_ON_ERROR (rv);

	return hsl_phy_modify_mmd(dev_id, uniphy_xpcs_addr, A_TRUE, mmd_num, mmd_reg,
		mask, value);
}

a_bool_t mht_uniphy_mode_check(a_uint32_t dev_id, a_uint32_t uniphy_index,
	mht_uniphy_mode_t uniphy_mode)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_addr = 0;
	a_uint16_t uniphy_mode_ctrl_data = 0;

	rv = qca_mht_serdes_addr_get (dev_id, uniphy_index, &uniphy_addr);
	if(rv != SW_OK)
		return A_FALSE;

	uniphy_mode_ctrl_data = hsl_phy_mmd_reg_read(dev_id, uniphy_addr, A_TRUE,
		MHT_UNIPHY_MMD1, MHT_UNIPHY_MMD1_MODE_CTRL);
	if(uniphy_mode_ctrl_data == PHY_INVALID_DATA)
		return A_FALSE;

	if(!(uniphy_mode & uniphy_mode_ctrl_data))
		return A_FALSE;

	return A_TRUE;
}

static a_uint32_t
mht_uniphy_xpcs_port_to_mmd(a_uint32_t dev_id, a_uint32_t mht_port_id)
{
	a_uint32_t mmd_id = 0;

	switch(mht_port_id)
	{
		case SSDK_PHYSICAL_PORT1:
			mmd_id = MHT_UNIPHY_MMD31;
			break;
		case SSDK_PHYSICAL_PORT2:
			mmd_id = MHT_UNIPHY_MMD26;
			break;
		case SSDK_PHYSICAL_PORT3:
			mmd_id = MHT_UNIPHY_MMD27;
			break;
		case SSDK_PHYSICAL_PORT4:
			mmd_id = MHT_UNIPHY_MMD28;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return mmd_id;
}

static sw_error_t
mht_uniphy_xpcs_modify_port_mmd(a_uint32_t dev_id, a_uint32_t mht_port_id,
	a_uint32_t mmd_reg, a_uint32_t mask, a_uint32_t value)
{
	a_uint32_t mmd_id = 0;
	sw_error_t rv = SW_OK;

	mmd_id = mht_uniphy_xpcs_port_to_mmd(dev_id, mht_port_id);

	rv = mht_uniphy_xpcs_modify_mmd(dev_id, mmd_id, mmd_reg, mask, value);

	return rv;
}

sw_error_t
mht_port_speed_clock_set(a_uint32_t dev_id, a_uint32_t mht_port_id,
	fal_port_speed_t speed)
{
	sw_error_t rv = SW_OK;
	a_uint32_t clk_rate = 0;

	switch(speed)
	{
		case FAL_SPEED_2500:
			clk_rate = UQXGMII_SPEED_2500M_CLK;
			break;
		case FAL_SPEED_1000:
			clk_rate = UQXGMII_SPEED_1000M_CLK;
			break;
		case FAL_SPEED_100:
			clk_rate = UQXGMII_SPEED_100M_CLK;
			break;
		case FAL_SPEED_10:
			clk_rate = UQXGMII_SPEED_10M_CLK;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	rv = ssdk_mht_port_clk_rate_set(dev_id, mht_port_id, clk_rate);

	return rv;
}

static sw_error_t
mht_uniphy_xpcs_8023az_enable(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	a_uint16_t uniphy_data = 0;

	uniphy_data = mht_uniphy_xpcs_mmd_read(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_AN_LP_BASE_ABL2);
	PHY_RTN_ON_READ_ERROR(uniphy_data);
	if(!(uniphy_data & MHT_UNIPHY_MMD3_XPCS_EEE_CAP))
		return SW_NOT_SUPPORTED;

	/*Configure the EEE related timer*/
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_EEE_MODE_CTRL, 0x0f40, MHT_UNIPHY_MMD3_EEE_RES_REGS |
		MHT_UNIPHY_MMD3_EEE_SIGN_BIT_REGS);
	PHY_RTN_ON_ERROR (rv);

	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_EEE_TX_TIMER, 0x1fff, MHT_UNIPHY_MMD3_EEE_TSL_REGS|
		MHT_UNIPHY_MMD3_EEE_TLU_REGS | MHT_UNIPHY_MMD3_EEE_TWL_REGS);
	PHY_RTN_ON_ERROR (rv);

	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_EEE_RX_TIMER, 0x1fff, MHT_UNIPHY_MMD3_EEE_100US_REG_REGS|
		MHT_UNIPHY_MMD3_EEE_RWR_REG_REGS);
	PHY_RTN_ON_ERROR (rv);

	/*enable TRN_LPI*/
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_EEE_MODE_CTRL1, 0x101, MHT_UNIPHY_MMD3_EEE_TRANS_LPI_MODE|
		MHT_UNIPHY_MMD3_EEE_TRANS_RX_LPI_MODE);
	PHY_RTN_ON_ERROR (rv);

	/*enable TX/RX LPI pattern*/
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_EEE_MODE_CTRL, 0x3, MHT_UNIPHY_MMD3_EEE_EN);

	return rv;
}

static sw_error_t
mht_uniphy_calibration(a_uint32_t dev_id, a_uint32_t uniphy_addr)
{
	a_uint16_t uniphy_data = 0;
	a_uint32_t retries = 100, calibration_done = 0;

	/* wait calibration done to uniphy*/
	while (calibration_done != MHT_UNIPHY_MMD1_CALIBRATION_DONE) {
		mdelay(1);
		if (retries-- == 0)
		{
			SSDK_ERROR("uniphy callibration time out!\n");
			return SW_TIMEOUT;
		}
		uniphy_data = hsl_phy_mmd_reg_read(dev_id, uniphy_addr, A_TRUE,
			MHT_UNIPHY_MMD1, MHT_UNIPHY_MMD1_CALIBRATION4);
		PHY_RTN_ON_READ_ERROR(uniphy_data);

		calibration_done = (uniphy_data & MHT_UNIPHY_MMD1_CALIBRATION_DONE);
	}

	return SW_OK;
}

static sw_error_t
mht_uniphy_xpcs_10g_r_linkup(a_uint32_t dev_id)
{
	a_uint16_t uniphy_data = 0;
	a_uint32_t retries = 100, linkup = 0;

	/* wait 10G_R link up */
	while (linkup != MHT_UNIPHY_MMD3_10GBASE_R_UP) {
		mdelay(1);
		if (retries-- == 0)
		{
			SSDK_ERROR ("10g_r link up timeout\n");
			return SW_TIMEOUT;
		}
		uniphy_data = mht_uniphy_xpcs_mmd_read(dev_id, MHT_UNIPHY_MMD3,
			MHT_UNIPHY_MMD3_10GBASE_R_PCS_STATUS1);
		PHY_RTN_ON_READ_ERROR(uniphy_data);

		linkup = (uniphy_data & MHT_UNIPHY_MMD3_10GBASE_R_UP);
	}

	return SW_OK;
}

static sw_error_t
mht_uniphy_xpcs_soft_reset(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	a_uint16_t uniphy_data = 0;
	a_uint32_t retries = 100, reset_done = MHT_UNIPHY_MMD3_XPCS_SOFT_RESET;

	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_DIG_CTRL1, 0x8000, MHT_UNIPHY_MMD3_XPCS_SOFT_RESET);
	PHY_RTN_ON_ERROR (rv);

	while (reset_done) {
		mdelay(1);
		if (retries-- == 0)
			return SW_TIMEOUT;
		uniphy_data = mht_uniphy_xpcs_mmd_read(dev_id, MHT_UNIPHY_MMD3,
			MHT_UNIPHY_MMD3_DIG_CTRL1);
		PHY_RTN_ON_READ_ERROR(uniphy_data);

		reset_done = (uniphy_data & MHT_UNIPHY_MMD3_XPCS_SOFT_RESET);
	}

	return rv;
}

#if 0
sw_error_t
mht_uniphy_xpcs_speed_set(a_uint32_t dev_id, a_uint32_t mht_port_id,
	fal_port_speed_t speed)
{
	a_uint32_t xpcs_speed = 0;
	sw_error_t rv = SW_OK;

	switch(speed)
	{
		case FAL_SPEED_2500:
			xpcs_speed = MHT_UNIPHY_MMD_XPC_SPEED_2500;
			break;
		case FAL_SPEED_1000:
			xpcs_speed = MHT_UNIPHY_MMD_XPC_SPEED_1000;
			break;
		case FAL_SPEED_100:
			xpcs_speed = MHT_UNIPHY_MMD_XPC_SPEED_100;
			break;
		case FAL_SPEED_10:
			xpcs_speed = MHT_UNIPHY_MMD_XPC_SPEED_10;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	rv = mht_uniphy_xpcs_modify_port_mmd(dev_id, mht_port_id,
		MHT_UNIPHY_MMD_MII_CTRL, MHT_UNIPHY_MMD_XPC_SPEED_MASK,
		xpcs_speed);

	return rv;
}
#endif
sw_error_t
mht_uniphy_uqxgmii_function_reset(a_uint32_t dev_id, a_uint32_t mht_port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_addr = 0;

	rv = qca_mht_serdes_addr_get(dev_id, MHT_UNIPHY_SGMII_1, &uniphy_addr);
	PHY_RTN_ON_ERROR (rv);

	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_USXGMII_RESET, BIT(mht_port_id-1), 0);
	PHY_RTN_ON_ERROR (rv);
	mdelay(1);
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_USXGMII_RESET, BIT(mht_port_id-1),
		BIT(mht_port_id-1));
	PHY_RTN_ON_ERROR (rv);
	if(mht_port_id == SSDK_PHYSICAL_PORT1)
	{
		rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
			MHT_UNIPHY_MMD_MII_DIG_CTRL,
			0x400, MHT_UNIPHY_MMD3_USXG_FIFO_RESET);
		PHY_RTN_ON_ERROR (rv);
	}
	else
	{
		rv = mht_uniphy_xpcs_modify_port_mmd(dev_id, mht_port_id,
			MHT_UNIPHY_MMD_MII_DIG_CTRL,
			0x20, MHT_UNIPHY_MMD_USXG_FIFO_RESET);
		PHY_RTN_ON_ERROR (rv);
	}

	return SW_OK;
}

sw_error_t
mht_uniphy_xpcs_autoneg_restart(a_uint32_t dev_id, a_uint32_t mht_port_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t retries = 500, uniphy_data = 0, mmd_id = 0;

	mmd_id = mht_uniphy_xpcs_port_to_mmd(dev_id, mht_port_id);
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, mmd_id, MHT_UNIPHY_MMD_MII_CTRL,
		MHT_UNIPHY_MMD_MII_AN_RESTART, MHT_UNIPHY_MMD_MII_AN_RESTART);
	PHY_RTN_ON_ERROR (rv);
	mdelay(1);
	uniphy_data = mht_uniphy_xpcs_mmd_read(dev_id,mmd_id,
		MHT_UNIPHY_MMD_MII_ERR_SEL);
	PHY_RTN_ON_READ_ERROR(uniphy_data);
	while(!(uniphy_data & MHT_UNIPHY_MMD_MII_AN_COMPLETE_INT))
	{
		mdelay(1);
		if (retries-- == 0)
		{
			SSDK_ERROR ("xpcs uniphy autoneg restart timeout\n");
			return SW_TIMEOUT;
		}
		uniphy_data = mht_uniphy_xpcs_mmd_read(dev_id, mmd_id,
			MHT_UNIPHY_MMD_MII_ERR_SEL);
		PHY_RTN_ON_READ_ERROR(uniphy_data);
	}
	/*clear autoneg complete interrupt*/
	SSDK_DEBUG("clear autoneg complete interrupt\n");
	rv = mht_uniphy_xpcs_mmd_write(dev_id,mmd_id, MHT_UNIPHY_MMD_MII_ERR_SEL,
		uniphy_data & (~BIT(0)));

	return rv;
}

sw_error_t
mht_uniphy_sgmii_function_reset(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_addr = 0;

	rv = qca_mht_serdes_addr_get(dev_id, uniphy_index, &uniphy_addr);
	PHY_RTN_ON_ERROR (rv);

	/*sgmii channel0 adpt reset*/
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_CHANNEL0_CFG, MHT_UNIPHY_MMD1_SGMII_ADPT_RESET, 0);
	PHY_RTN_ON_ERROR (rv);
	mdelay(1);
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_CHANNEL0_CFG, MHT_UNIPHY_MMD1_SGMII_ADPT_RESET,
		MHT_UNIPHY_MMD1_SGMII_ADPT_RESET);
	PHY_RTN_ON_ERROR (rv);
	/*ipg tune reset*/
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_USXGMII_RESET, MHT_UNIPHY_MMD1_SGMII_FUNC_RESET, 0);
	PHY_RTN_ON_ERROR (rv);
	mdelay(1);
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_USXGMII_RESET, MHT_UNIPHY_MMD1_SGMII_FUNC_RESET,
		MHT_UNIPHY_MMD1_SGMII_FUNC_RESET);

	return rv;
}

static sw_error_t
_mht_interface_uqxgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_addr)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mht_port_id = 0, phy_addr = 0;

	/*reset xpcs*/
	SSDK_DEBUG("reset xpcs\n");
	rv = ssdk_mht_clk_assert(dev_id, MHT_UNIPHY_XPCS_RST);
	PHY_RTN_ON_ERROR (rv);
	/*fix PLL unlock issue with high temperature*/
	rv = hsl_phy_modify_mii(dev_id, uniphy_addr, MHT_UNIPHY_PLL_LOOP_CONTROL,
		MHT_UPHY_PLL_CML2CMS_IBSEL, MHT_UPHY_PLL_CML2CMS_IBSEL);
	PHY_RTN_ON_ERROR (rv);
	/*select xpcs mode*/
	SSDK_DEBUG("select xpcs mode\n");
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_MODE_CTRL, 0x1f00, MHT_UNIPHY_MMD1_XPCS_MODE);
	PHY_RTN_ON_ERROR (rv);
	/*config dapa pass as usxgmii*/
	SSDK_DEBUG("config dapa pass as usxgmii\n");
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_GMII_DATAPASS_SEL, MHT_UNIPHY_MMD1_DATAPASS_MASK,
		MHT_UNIPHY_MMD1_DATAPASS_USXGMII);
	PHY_RTN_ON_ERROR (rv);
	/*reset and release uniphy GMII/XGMII and ethphy GMII*/
	SSDK_DEBUG("reset and release uniphy GMII/XGMII and ethphy GMII\n");
	for(mht_port_id = SSDK_PHYSICAL_PORT1; mht_port_id <= SSDK_PHYSICAL_PORT4;
		mht_port_id++)
	{
		rv = ssdk_mht_port_clk_reset(dev_id, mht_port_id,
			MHT_CLK_TYPE_UNIPHY|MHT_CLK_TYPE_EPHY);
		PHY_RTN_ON_ERROR(rv);
	}

	/*ana sw reset and release*/
	SSDK_DEBUG("ana sw reset and release\n");
	rv = hsl_phy_modify_mii(dev_id, uniphy_addr,
		MHT_UNIPHY_PLL_POWER_ON_AND_RESET, 0x40, MHT_UNIPHY_ANA_SOFT_RESET);
	PHY_RTN_ON_ERROR (rv);
	mdelay(10);
	rv = hsl_phy_modify_mii(dev_id, uniphy_addr,
		MHT_UNIPHY_PLL_POWER_ON_AND_RESET, 0x40, MHT_UNIPHY_ANA_SOFT_RELEASE);
	PHY_RTN_ON_ERROR (rv);

	/*Wait calibration done*/
	SSDK_DEBUG("Wait calibration done\n");
	mht_uniphy_calibration(dev_id, uniphy_addr);
	/*Enable SSCG(Spread Spectrum Clock Generator)*/
	SSDK_DEBUG("enable uniphy sscg\n");
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, MHT_UNIPHY_MMD1, A_TRUE,
		MHT_UNIPHY_MMD1_CDA_CONTROL1, 0x8, MHT_UNIPHY_MMD1_SSCG_ENABLE);
	PHY_RTN_ON_ERROR (rv);
	/*release XPCS*/
	SSDK_DEBUG("release XPCS\n");
	rv = ssdk_mht_clk_deassert(dev_id, MHT_UNIPHY_XPCS_RST);
	PHY_RTN_ON_ERROR (rv);
	/*ethphy software reset*/
	SSDK_DEBUG("ethphy software reset\n");
	for(mht_port_id = SSDK_PHYSICAL_PORT1; mht_port_id <= SSDK_PHYSICAL_PORT4;
		mht_port_id++)
	{
		/* ethphy software reset will power on the phy, for erp low power port
		 * which has power off, can not power on it */
		if (hsl_port_feature_get(dev_id, mht_port_id, PHY_F_ERP_LOW_POWER))
			continue;
		rv = qca_mht_ephy_addr_get(dev_id, mht_port_id, &phy_addr);
		PHY_RTN_ON_ERROR (rv);
		rv = qca808x_phy_reset(dev_id, phy_addr);
		PHY_RTN_ON_ERROR (rv);
	}
	/*Set BaseR mode*/
	SSDK_DEBUG("Set BaseR mode\n");
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_PCS_CTRL2, 0xf, MHT_UNIPHY_MMD3_PCS_TYPE_10GBASE_R);
	PHY_RTN_ON_ERROR (rv);
	/*wait 10G base_r link up*/
	SSDK_DEBUG("wait 10G base_r link up\n");
	rv = mht_uniphy_xpcs_10g_r_linkup(dev_id);
	PHY_RTN_ON_ERROR (rv);
	/*enable UQXGMII mode*/
	SSDK_DEBUG("enable UQSXGMII mode\n");
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_DIG_CTRL1, 0x200, MHT_UNIPHY_MMD3_USXGMII_EN);
	PHY_RTN_ON_ERROR (rv);
	/*set UQXGMII mode*/
	SSDK_DEBUG("set QXGMII mode\n");
	rv = mht_uniphy_xpcs_modify_mmd(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_VR_RPCS_TPC, 0x1c00, MHT_UNIPHY_MMD3_QXGMII_EN);
	PHY_RTN_ON_ERROR (rv);
	/*set AM interval*/
	SSDK_DEBUG("set AM interval\n");
	rv = mht_uniphy_xpcs_mmd_write(dev_id, MHT_UNIPHY_MMD3,
		MHT_UNIPHY_MMD3_MII_AM_INTERVAL, MHT_UNIPHY_MMD3_MII_AM_INTERVAL_VAL);
	PHY_RTN_ON_ERROR (rv);
	/*xpcs software reset*/
	SSDK_DEBUG("xpcs software reset\n");
	rv = mht_uniphy_xpcs_soft_reset(dev_id);

	return rv;
}

sw_error_t
mht_interface_uqxgmii_mode_set(a_uint32_t dev_id)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_addr = 0, mht_port_id = 0;

	/* dassert serdes if it is asserted */
	if (ssdk_mht_clk_is_asserted(dev_id, MHT_SRDS1_SYS_CLK)) {
		rv = ssdk_mht_clk_deassert(dev_id, MHT_SRDS1_SYS_CLK);
		PHY_RTN_ON_ERROR (rv);
	}

	rv = qca_mht_serdes_addr_get(dev_id, MHT_UNIPHY_SGMII_1, &uniphy_addr);
	PHY_RTN_ON_ERROR (rv);

	/*disable IPG_tuning bypass*/
	SSDK_DEBUG("disable IPG_tuning bypass\n");
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_BYPASS_TUNING_IPG,
		MHT_UNIPHY_MMD1_BYPASS_TUNING_IPG_EN, 0);
	PHY_RTN_ON_ERROR (rv);
	/*disable uniphy GMII/XGMII clock and disable ethphy GMII clock*/
	SSDK_DEBUG("disable uniphy GMII/XGMII clock and ethphy GMII clock\n");
	for(mht_port_id = SSDK_PHYSICAL_PORT1; mht_port_id <= SSDK_PHYSICAL_PORT4;
		mht_port_id++)
	{
		rv = ssdk_mht_port_clk_en_set(dev_id, mht_port_id,
			MHT_CLK_TYPE_UNIPHY|MHT_CLK_TYPE_EPHY, A_FALSE);
		PHY_RTN_ON_ERROR (rv);
	}
	/*configure uqxgmii mode*/
	SSDK_DEBUG("configure uqxgmii mode\n");
	rv = _mht_interface_uqxgmii_mode_set(dev_id, uniphy_addr);
	PHY_RTN_ON_ERROR (rv);
	/*enable auto-neg complete interrupt,Mii using mii-4bits,
		configure as PHY mode, enable autoneg ability*/
	SSDK_DEBUG("enable auto-neg complete interrupt,Mii using mii-4bits,"
		"configure as PHY mode, enable autoneg ability, disable TICD\n");
	for (mht_port_id = SSDK_PHYSICAL_PORT1; mht_port_id <= SSDK_PHYSICAL_PORT4;
		mht_port_id++)
	{
		/*enable auto-neg complete interrupt,Mii using mii-4bits,configure as PHY mode*/
		rv = mht_uniphy_xpcs_modify_port_mmd(dev_id, mht_port_id,
			MHT_UNIPHY_MMD_MII_AN_INT_MSK, 0x109,
			MHT_UNIPHY_MMD_AN_COMPLETE_INT |
			MHT_UNIPHY_MMD_MII_4BITS_CTRL |
			MHT_UNIPHY_MMD_TX_CONFIG_CTRL);
		PHY_RTN_ON_ERROR (rv);

		/*enable autoneg ability*/
		rv = mht_uniphy_xpcs_modify_port_mmd(dev_id, mht_port_id,
			MHT_UNIPHY_MMD_MII_CTRL, 0x3060, MHT_UNIPHY_MMD_MII_AN_ENABLE |
			MHT_UNIPHY_MMD_XPC_SPEED_1000);
		PHY_RTN_ON_ERROR (rv);

		/*disable TICD*/
		rv = mht_uniphy_xpcs_modify_port_mmd(dev_id, mht_port_id,
			MHT_UNIPHY_MMD_MII_XAUI_MODE_CTRL, 0x1,
			MHT_UNIPHY_MMD_TX_IPG_CHECK_DISABLE);
		PHY_RTN_ON_ERROR (rv);

		/*enable phy mode control to sync phy link information to xpcs*/
		rv = mht_uniphy_xpcs_modify_port_mmd(dev_id, mht_port_id,
			MHT_UNIPHY_MMD_MII_DIG_CTRL, BIT(0), MHT_UNIPHY_MMD_PHY_MODE_CTRL_EN);
		PHY_RTN_ON_ERROR (rv);
	}

	/*enable EEE for xpcs*/
	SSDK_DEBUG("enable EEE for xpcs\n");
	rv = mht_uniphy_xpcs_8023az_enable(dev_id);

	return rv;
}

sw_error_t
mht_interface_sgmii_mode_set(a_uint32_t dev_id, a_uint32_t uniphy_index,
	a_uint32_t mht_port_id, fal_mac_config_t *config)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_addr = 0, mode_ctrl = 0, speed_mode = 0;
	a_uint32_t uniphy_port_id = 0, ethphy_clk_mask = 0;
	a_uint64_t raw_clk = 0;

	/* dassert serdes if it is asserted */
	if (uniphy_index == MHT_UNIPHY_SGMII_0) {
		if (ssdk_mht_clk_is_asserted(dev_id, MHT_SRDS0_SYS_CLK)) {
			rv = ssdk_mht_clk_deassert(dev_id, MHT_SRDS0_SYS_CLK);
			PHY_RTN_ON_ERROR (rv);
		}
	} else if (uniphy_index == MHT_UNIPHY_SGMII_1) {
		if (ssdk_mht_clk_is_asserted(dev_id, MHT_SRDS1_SYS_CLK)) {
			rv = ssdk_mht_clk_deassert(dev_id, MHT_SRDS1_SYS_CLK);
			PHY_RTN_ON_ERROR (rv);
		}
	}

	/*get the uniphy address*/
	rv = qca_mht_serdes_addr_get(dev_id, uniphy_index, &uniphy_addr);
	PHY_RTN_ON_ERROR (rv);

	if(config->mac_mode == FAL_MAC_MODE_SGMII)
	{
		mode_ctrl = MHT_UNIPHY_MMD1_SGMII_MODE;
		raw_clk = UNIPHY_CLK_RATE_125M;
	}
	else
	{
		mode_ctrl = MHT_UNIPHY_MMD1_SGMII_PLUS_MODE;
		raw_clk = UNIPHY_CLK_RATE_312M;
	}

	if(config->config.sgmii.clock_mode == FAL_INTERFACE_CLOCK_MAC_MODE)
		mode_ctrl |= MHT_UNIPHY_MMD1_SGMII_MAC_MODE;
	else
	{
		mode_ctrl |= MHT_UNIPHY_MMD1_SGMII_PHY_MODE;
		/*eththy clock should be accessed for phy mode*/
		ethphy_clk_mask = MHT_CLK_TYPE_EPHY;
	}

	SSDK_DEBUG("uniphy:%d,mode:%s,autoneg_en:%d,force_speed:%d,clk_mask:0x%x\n",
		uniphy_index, (config->mac_mode == FAL_MAC_MODE_SGMII)?"sgmii":"sgmii plus",
		config->config.sgmii.auto_neg, config->config.sgmii.force_speed,
		ethphy_clk_mask);

	/*GMII interface clock disable*/
	SSDK_DEBUG("GMII interface clock disable\n");
	rv = ssdk_mht_port_clk_en_set(dev_id, mht_port_id, ethphy_clk_mask,
		A_FALSE);
	PHY_RTN_ON_ERROR (rv);
	/*when access uniphy0 clock, port5 should be used, but for phy mode,
		the port 4 connect to uniphy0, so need to change the port id*/
	if(uniphy_index == MHT_UNIPHY_SGMII_0)
		uniphy_port_id = SSDK_PHYSICAL_PORT5;
	else
		uniphy_port_id = mht_port_id;
	rv = ssdk_mht_port_clk_en_set(dev_id, uniphy_port_id, MHT_CLK_TYPE_UNIPHY,
		A_FALSE);
	PHY_RTN_ON_ERROR (rv);
	/*uniphy1 xpcs reset, and configure raw clk*/
	if(uniphy_index == MHT_UNIPHY_SGMII_1)
	{
		SSDK_DEBUG("uniphy1 xpcs reset, confiugre raw clock as:%lld\n",
			raw_clk);
		rv = ssdk_mht_clk_assert(dev_id, MHT_UNIPHY_XPCS_RST);
		PHY_RTN_ON_ERROR (rv);
		ssdk_mht_uniphy_raw_clock_set(dev_id, MHT_P_UNIPHY1_RX, raw_clk);
		ssdk_mht_uniphy_raw_clock_set(dev_id, MHT_P_UNIPHY1_TX, raw_clk);
	}
	else
	{
		SSDK_DEBUG("uniphy0 configure raw clock as %lld\n",
			raw_clk);
		ssdk_mht_uniphy_raw_clock_set(dev_id, MHT_P_UNIPHY0_RX, raw_clk);
		ssdk_mht_uniphy_raw_clock_set(dev_id, MHT_P_UNIPHY0_TX, raw_clk);
	}
	/*fix PLL unlock issue with high temperature*/
	rv = hsl_phy_modify_mii(dev_id, uniphy_addr, MHT_UNIPHY_PLL_LOOP_CONTROL,
		MHT_UPHY_PLL_CML2CMS_IBSEL, MHT_UPHY_PLL_CML2CMS_IBSEL);
	PHY_RTN_ON_ERROR (rv);
	/*configure SGMII mode or SGMII+ mode*/
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_MODE_CTRL, MHT_UNIPHY_MMD1_SGMII_MODE_CTRL_MASK,
		mode_ctrl);
	PHY_RTN_ON_ERROR (rv);
	/*GMII datapass selection, 0 is for SGMII, 1 is for USXGMII*/
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_GMII_DATAPASS_SEL, MHT_UNIPHY_MMD1_DATAPASS_MASK,
		MHT_UNIPHY_MMD1_DATAPASS_SGMII);
	PHY_RTN_ON_ERROR (rv);
	/*configue force or autoneg*/
	if(!config->config.sgmii.auto_neg)
	{
		rv = mht_port_speed_clock_set(dev_id, mht_port_id,
			config->config.sgmii.force_speed);
		PHY_RTN_ON_ERROR(rv);
		switch (config->config.sgmii.force_speed)
		{
			case FAL_SPEED_10:
				speed_mode = MHT_UNIPHY_MMD1_CH0_FORCE_ENABLE |
					MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_10M;
				break;
			case FAL_SPEED_100:
				speed_mode = MHT_UNIPHY_MMD1_CH0_FORCE_ENABLE |
					MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_100M;
				break;
			case FAL_SPEED_1000:
			case FAL_SPEED_2500:
				speed_mode = MHT_UNIPHY_MMD1_CH0_FORCE_ENABLE |
					MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_1G;
				break;
			default:
				break;
		}
	}
	else
	{
		speed_mode = MHT_UNIPHY_MMD1_CH0_AUTONEG_ENABLE;
	}
	rv = hsl_phy_modify_mmd(dev_id, uniphy_addr, A_TRUE, MHT_UNIPHY_MMD1,
		MHT_UNIPHY_MMD1_CHANNEL0_CFG, MHT_UNIPHY_MMD1_CH0_FORCE_SPEED_MASK,
		speed_mode);
	PHY_RTN_ON_ERROR (rv);
	/*GMII interface clock reset and release\n*/
	SSDK_DEBUG("GMII interface clock reset and release\n");
	rv = ssdk_mht_port_clk_reset(dev_id, mht_port_id, ethphy_clk_mask);
	PHY_RTN_ON_ERROR (rv);
	rv = ssdk_mht_port_clk_reset(dev_id, uniphy_port_id, MHT_CLK_TYPE_UNIPHY);
	PHY_RTN_ON_ERROR (rv);
	/*analog software reset and release*/
	SSDK_DEBUG("analog software reset and release\n");
	rv = hsl_phy_modify_mii(dev_id, uniphy_addr,
		MHT_UNIPHY_PLL_POWER_ON_AND_RESET, 0x40, MHT_UNIPHY_ANA_SOFT_RESET);
	PHY_RTN_ON_ERROR (rv);
	mdelay(1);
	rv = hsl_phy_modify_mii(dev_id, uniphy_addr,
		MHT_UNIPHY_PLL_POWER_ON_AND_RESET, 0x40, MHT_UNIPHY_ANA_SOFT_RELEASE);
	PHY_RTN_ON_ERROR (rv);
	/*wait uniphy calibration done*/
	SSDK_DEBUG("wait uniphy calibration done\n");
	mht_uniphy_calibration(dev_id, uniphy_addr);
	/*GMII interface clock enable*/
	SSDK_DEBUG("GMII interface clock enable\n");
	rv = ssdk_mht_port_clk_en_set(dev_id, mht_port_id, ethphy_clk_mask, A_TRUE);
	PHY_RTN_ON_ERROR (rv);
	rv = ssdk_mht_port_clk_en_set(dev_id, uniphy_port_id, MHT_CLK_TYPE_UNIPHY,
		A_TRUE);

	return rv;
}

sw_error_t
mht_interface_mac_mode_set(a_uint32_t dev_id, fal_port_t port_id,
	fal_mac_config_t *config)
{
	sw_error_t rv = SW_OK;
	a_uint32_t uniphy_index = 0;

	if(port_id == SSDK_PHYSICAL_PORT5)
	{
		/*if uniphy0 is used as switch bypass, then will do nothing here*/
		if(mht_uniphy_mode_check(dev_id, MHT_UNIPHY_SGMII_0, MHT_UNIPHY_PHY))
		{
			SSDK_INFO("the uniphy0 is used for switch bypass\n");
			return SW_OK;
		}
		else
		{
			if(config->mac_mode == FAL_MAC_MODE_MAX)
			{
				SSDK_INFO("the uniphy for port %d is not used\n", port_id);
				/* assert serdes0 to save power */
				ssdk_mht_clk_assert(dev_id, MHT_SRDS0_SYS_CLK);
				return SW_OK;
			}
		}
	}

	if(config->mac_mode != FAL_MAC_MODE_SGMII && config->mac_mode !=
		FAL_MAC_MODE_SGMII_PLUS)
		return SW_NOT_SUPPORTED;

	/*get uniphy index*/
	if(port_id == SSDK_PHYSICAL_PORT0)
	{
		/*for switch mode, the uniphy1 must be initialized firstly and initialized
		only one time, so configure dvs and acc for memory before uniphy1 initialization*/
		rv = qca_mht_mem_ctrl_set(dev_id, MHT_MEM_CTRL_DVS_SWITCH_MODE,
			MHT_MEM_ACC_0_SWITCH_MODE);
		PHY_RTN_ON_ERROR (rv);
		uniphy_index = MHT_UNIPHY_SGMII_1;
	}
	else if(port_id == SSDK_PHYSICAL_PORT5)
		uniphy_index = MHT_UNIPHY_SGMII_0;
	else
		return SW_NOT_SUPPORTED;

	rv = mht_interface_sgmii_mode_set(dev_id, uniphy_index, port_id, config);
	PHY_RTN_ON_ERROR (rv);
	/*do sgmii function reset*/
	SSDK_DEBUG("ipg_tune reset and function reset\n");
	rv = mht_uniphy_sgmii_function_reset(dev_id, uniphy_index);

	return rv;
}
