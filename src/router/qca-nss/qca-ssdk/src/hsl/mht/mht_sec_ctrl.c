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


/**
 * @defgroup mht_sec_ctrl MHT_SEC_CTRL
 * @{
 */
#include "sw.h"
#include "hsl.h"
#include "hsl_dev.h"
#include "mht_sec_ctrl.h"
#include "mht_interface_ctrl.h"
#include "ssdk_mht_pinctrl.h"

sw_error_t
qca_mht_work_mode_set(a_uint32_t dev_id, mht_work_mode_t work_mode)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	data = qca_mht_mii_read(dev_id, WORK_MODE_OFFSET);
	data &= ~MHT_WORK_MODE_MASK;
	data |= work_mode;

	qca_mht_mii_write(dev_id, WORK_MODE_OFFSET, data);

	return SW_OK;
}

sw_error_t
qca_mht_work_mode_get(a_uint32_t dev_id, mht_work_mode_t *work_mode)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	data = qca_mht_mii_read(dev_id, WORK_MODE_OFFSET);
	SSDK_DEBUG("work mode reg is 0x%x\n", data);

	*work_mode = data & MHT_WORK_MODE_MASK;

	return SW_OK;
}

sw_error_t
qca_mht_serdes_addr_get(a_uint32_t dev_id, a_uint32_t serdes_id,
	a_uint32_t *address)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);
	data = qca_mht_mii_read(dev_id, SERDES_CFG_OFFSET);
	switch(serdes_id)
	{
		case MHT_UNIPHY_SGMII_0:
			*address = (data >> SERDES_CFG_S0_ADDR_BOFFSET) & 0x1f;
			break;
		case MHT_UNIPHY_SGMII_1:
			*address = (data >> SERDES_CFG_S1_ADDR_BOFFSET) & 0x1f;
			break;
		case MHT_UNIPHY_XPCS:
			*address = (data >> SERDES_CFG_S1_XPCS_ADDR_BOFFSET) & 0x1f;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t
qca_mht_ephy_addr_get(a_uint32_t dev_id, a_uint32_t mht_port_id,
	a_uint32_t *phy_addr)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);
	data = qca_mht_mii_read(dev_id, EPHY_CFG_OFFSET);
	switch(mht_port_id)
	{
		case SSDK_PHYSICAL_PORT1:
			*phy_addr = (data >> EPHY_CFG_EPHY0_ADDR_BOFFSET) & 0x1f;
			break;
		case SSDK_PHYSICAL_PORT2:
			*phy_addr = (data >> EPHY_CFG_EPHY1_ADDR_BOFFSET) & 0x1f;
			break;
		case SSDK_PHYSICAL_PORT3:
			*phy_addr = (data >> EPHY_CFG_EPHY2_ADDR_BOFFSET) & 0x1f;
			break;
		case SSDK_PHYSICAL_PORT4:
			*phy_addr = (data >> EPHY_CFG_EPHY3_ADDR_BOFFSET) & 0x1f;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}

	return SW_OK;
}

sw_error_t
qca_mht_port_id_get(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t *mht_port_id)
{
	a_uint32_t port_index = 0, phy_addr_tmp = 0;
	sw_error_t rv = SW_OK;

	for(port_index = SSDK_PHYSICAL_PORT1; port_index <= SSDK_PHYSICAL_PORT4;
		port_index++)
	{
		rv = qca_mht_ephy_addr_get(dev_id, port_index, &phy_addr_tmp);
		SW_RTN_ON_ERROR (rv);
		if(phy_addr == phy_addr_tmp)
		{
			*mht_port_id = port_index;
			return SW_OK;
		}
	}

	return SW_NOT_FOUND;
}

sw_error_t
qca_mht_phy_intr_enable(a_uint32_t dev_id, a_uint32_t phy_addr,
	a_uint32_t intr_bmp)
{
	sw_error_t rv = SW_OK;
	a_uint32_t mht_port_id = 0;
	a_uint32_t data0 = 0, data1 = 0;

	rv = qca_mht_port_id_get(dev_id, phy_addr, &mht_port_id);
	SW_RTN_ON_ERROR (rv);
	if(mht_port_id > SSDK_PHYSICAL_PORT4 || mht_port_id < SSDK_PHYSICAL_PORT1)
		return SW_NOT_SUPPORTED;
	data0 = qca_mht_mii_read(dev_id, GLOBAL_INTR_ENABLE_OFFSET);
	data1 = qca_mht_mii_read(dev_id, WOL_INTR_ENABLE_OFFSET);
	if(!intr_bmp)
	{
		data0 &= ~(BIT(GLOBAL_INTR_ENABLE_PHY0_BOFFSET+1-mht_port_id));
		data1 &= ~(BIT(mht_port_id-1));
	}
	else
	{
		data0 |= BIT(GLOBAL_INTR_ENABLE_PHY0_BOFFSET+1-mht_port_id);
		if(intr_bmp & FAL_PHY_INTR_WOL_STATUS)
			data1 |= BIT(mht_port_id-1);
		else
			data1 &= BIT(mht_port_id-1);
	}

	qca_mht_mii_write(dev_id, GLOBAL_INTR_ENABLE_OFFSET, data0);
	qca_mht_mii_write(dev_id, WOL_INTR_ENABLE_OFFSET, data1);

	return SW_OK;
}

sw_error_t
qca_mht_switch_intr_set(a_uint32_t dev_id, a_bool_t enable)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	data = qca_mht_mii_read(dev_id, GLOBAL_INTR_ENABLE_OFFSET);

	if(enable)
		data |= BIT(GLOBAL_INTR_ENABLE_SWITCH_BOFFSET);
	else
		data &= ~(BIT(GLOBAL_INTR_ENABLE_SWITCH_BOFFSET));

	qca_mht_mii_write(dev_id, GLOBAL_INTR_ENABLE_OFFSET, data);

	return SW_OK;
}

sw_error_t
qca_mht_switch_intr_get(a_uint32_t dev_id, a_bool_t *enable)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	*enable = A_FALSE;
	data = qca_mht_mii_read(dev_id, GLOBAL_INTR_ENABLE_OFFSET);

	if(data & BIT(GLOBAL_INTR_ENABLE_SWITCH_BOFFSET))
		*enable = A_TRUE;

	return SW_OK;
}

sw_error_t
qca_mht_switch_intr_status_get(a_uint32_t dev_id, a_bool_t *enable)

{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);

	*enable = A_FALSE;
	data = qca_mht_mii_read(dev_id, GLOBAL_INTR_STATUS_OFFSET);

	if(data & BIT(GLOBAL_INTR_STATUS_SWITCH_BOFFSET))
		*enable = A_TRUE;

	return SW_OK;
}

#if defined(IN_PTP)
sw_error_t
qca_mht_ptp_sync_set(a_uint32_t dev_id, a_uint32_t mht_port_id, a_bool_t enable)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);
	switch (mht_port_id) {
		case SSDK_PHYSICAL_PORT1:
		case SSDK_PHYSICAL_PORT2:
		case SSDK_PHYSICAL_PORT3:
		case SSDK_PHYSICAL_PORT4:
			break;
		default:
			return SW_OUT_OF_RANGE;
	}

	data = qca_mht_mii_read(dev_id, PTP_MUX_OFFSET);
	if (enable) {
		data |= BIT(PTP_MUX_SEL_RTC_REF_CLK_EXT_0_BOFFSET+mht_port_id-1);
		data |= BIT(PTP_MUX_SEL_TOD_IN_EXT_0_BOFFSET+mht_port_id-1);
		data |= BIT(PTP_MUX_SEL_PPS_IN_EXT_0_BOFFSET+mht_port_id-1);
	} else {
		data &= ~BIT(PTP_MUX_SEL_RTC_REF_CLK_EXT_0_BOFFSET+mht_port_id-1);
		data &= ~BIT(PTP_MUX_SEL_TOD_IN_EXT_0_BOFFSET+mht_port_id-1);
		data &= ~BIT(PTP_MUX_SEL_PPS_IN_EXT_0_BOFFSET+mht_port_id-1);
	}

	qca_mht_mii_write(dev_id, PTP_MUX_OFFSET, data);

	return SW_OK;
}

sw_error_t
qca_mht_ptp_sync_get(a_uint32_t dev_id, a_uint32_t mht_port_id, a_bool_t *enable)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);
	switch (mht_port_id) {
		case SSDK_PHYSICAL_PORT1:
		case SSDK_PHYSICAL_PORT2:
		case SSDK_PHYSICAL_PORT3:
		case SSDK_PHYSICAL_PORT4:
			break;
		default:
			return SW_OUT_OF_RANGE;
	}

	data = qca_mht_mii_read(dev_id, PTP_MUX_OFFSET);
	if ((data & BIT(PTP_MUX_SEL_RTC_REF_CLK_EXT_0_BOFFSET+mht_port_id-1)) &&
			(data & BIT(PTP_MUX_SEL_TOD_IN_EXT_0_BOFFSET+mht_port_id-1)) &&
			(data & BIT(PTP_MUX_SEL_PPS_IN_EXT_0_BOFFSET+mht_port_id-1)))
		*enable = A_TRUE;
	else
		*enable = A_FALSE;

	return SW_OK;
}

sw_error_t
qca_mht_ptp_async_set(a_uint32_t dev_id, a_uint32_t mht_port_id, a_uint32_t src_id)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);
	switch (mht_port_id) {
		case SSDK_PHYSICAL_PORT1:
		case SSDK_PHYSICAL_PORT2:
		case SSDK_PHYSICAL_PORT3:
		case SSDK_PHYSICAL_PORT4:
			break;
		default:
			return SW_OUT_OF_RANGE;
	}

	/* configure tod, refclk, pps from external */
	qca_mht_ptp_sync_set(dev_id, mht_port_id, A_FALSE);

	/* configure the external source */
	data = qca_mht_mii_read(dev_id, PTP_MUX_OFFSET);
	switch (src_id) {
		case SSDK_PHYSICAL_PORT0:
			/* PPS_IN from pad */
			data &= ~BITS(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET, PTP_MUX_EXT_PPS_IN_SEL_BLEN);
			data |= BIT(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET+1);
			/* refclk from pad */
			data &= ~BIT(PTP_MUX_EXT_TOD_SEL_BOFFSET);
			/* TOD_IN from pad */
			data &= ~BIT(PTP_MUX_EXT_RTC_REF_CLK_SEL_BOFFSET);
			break;
		case SSDK_PHYSICAL_PORT1:
			/* PPS_IN from port1 pps_out */
			data &= ~BIT(PTP_MUX_EXT_PPS_P12_SEL_BOFFSET);
			data &= ~BITS(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET, PTP_MUX_EXT_PPS_IN_SEL_BLEN);
			/* refclk from port clk125_tdi_out */
			data |= BIT(PTP_MUX_EXT_TOD_SEL_BOFFSET);
			/* TOD_IN from port tod_out */
			data |= BIT(PTP_MUX_EXT_RTC_REF_CLK_SEL_BOFFSET);

			/* select the tod, clk125, pps */
			mht_gpio_pin_mux_set(dev_id, 9, MHT_PIN_FUNC_P0_PPS_OUT);
			mht_gpio_pin_mux_set(dev_id, 13, MHT_PIN_FUNC_P0_TOD_OUT);
			mht_gpio_pin_mux_set(dev_id, 14, MHT_PIN_FUNC_P0_CLK125_TDI);
			break;
		case SSDK_PHYSICAL_PORT2:
			/* PPS_IN from port2 pps_out */
			data |= BIT(PTP_MUX_EXT_PPS_P12_SEL_BOFFSET);
			data &= ~BITS(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET, PTP_MUX_EXT_PPS_IN_SEL_BLEN);
			/* refclk from port clk125_tdi_out */
			data |= BIT(PTP_MUX_EXT_TOD_SEL_BOFFSET);
			/* TOD_IN from port tod_out */
			data |= BIT(PTP_MUX_EXT_RTC_REF_CLK_SEL_BOFFSET);

			/* select the tod, clk125, pps */
			mht_gpio_pin_mux_set(dev_id, 10, MHT_PIN_FUNC_P1_PPS_OUT);
			mht_gpio_pin_mux_set(dev_id, 13, MHT_PIN_FUNC_P1_TOD_OUT);
			mht_gpio_pin_mux_set(dev_id, 14, MHT_PIN_FUNC_P1_CLK125_TDI);
			break;
		case SSDK_PHYSICAL_PORT3:
			/* PPS_IN from port3 pps_out */
			data &= ~BIT(PTP_MUX_EXT_PPS_P34_SEL_BOFFSET);
			data &= ~BITS(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET, PTP_MUX_EXT_PPS_IN_SEL_BLEN);
			data |= BIT(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET);
			/* refclk from port clk125_tdi_out */
			data |= BIT(PTP_MUX_EXT_TOD_SEL_BOFFSET);
			/* TOD_IN from port tod_out */
			data |= BIT(PTP_MUX_EXT_RTC_REF_CLK_SEL_BOFFSET);

			/* select the tod, clk125, pps */
			mht_gpio_pin_mux_set(dev_id, 11, MHT_PIN_FUNC_P2_PPS_OUT);
			mht_gpio_pin_mux_set(dev_id, 13, MHT_PIN_FUNC_P2_TOD_OUT);
			mht_gpio_pin_mux_set(dev_id, 14, MHT_PIN_FUNC_P2_CLK125_TDI);
			break;
		case SSDK_PHYSICAL_PORT4:
			/* PPS_IN from port4 pps_out */
			data |= BIT(PTP_MUX_EXT_PPS_P34_SEL_BOFFSET);
			data &= ~BITS(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET, PTP_MUX_EXT_PPS_IN_SEL_BLEN);
			data |= BIT(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET);
			/* refclk from port clk125_tdi_out */
			data |= BIT(PTP_MUX_EXT_TOD_SEL_BOFFSET);
			/* TOD_IN from port tod_out */
			data |= BIT(PTP_MUX_EXT_RTC_REF_CLK_SEL_BOFFSET);

			/* select the tod, clk125, pps */
			mht_gpio_pin_mux_set(dev_id, 12, MHT_PIN_FUNC_P3_PPS_OUT);
			mht_gpio_pin_mux_set(dev_id, 13, MHT_PIN_FUNC_P3_TOD_OUT);
			mht_gpio_pin_mux_set(dev_id, 14, MHT_PIN_FUNC_P3_CLK125_TDI);
			break;
		default:
			SSDK_ERROR("Unsupported source id: %d\n", src_id);
			return SW_OUT_OF_RANGE;
	}

	qca_mht_mii_write(dev_id, PTP_MUX_OFFSET, data);
	return SW_OK;
}

sw_error_t
qca_mht_ptp_async_get(a_uint32_t dev_id, a_uint32_t mht_port_id, a_uint32_t *src_id)
{
	a_uint32_t data = 0;

	HSL_DEV_ID_CHECK(dev_id);
	switch (mht_port_id) {
		case SSDK_PHYSICAL_PORT1:
		case SSDK_PHYSICAL_PORT2:
		case SSDK_PHYSICAL_PORT3:
		case SSDK_PHYSICAL_PORT4:
			break;
		default:
			return SW_OUT_OF_RANGE;
	}

	data = qca_mht_mii_read(dev_id, PTP_MUX_OFFSET);
	if (data & BIT(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET+1)) {
		*src_id = SSDK_PHYSICAL_PORT0;
		return SW_OK;
	}

	if (data & BIT(PTP_MUX_EXT_PPS_IN_SEL_BOFFSET)) {
		if (data & BIT(PTP_MUX_EXT_PPS_P34_SEL_BOFFSET))
			*src_id = SSDK_PHYSICAL_PORT4;
		else
			*src_id = SSDK_PHYSICAL_PORT3;
	} else {
		if (data & BIT(PTP_MUX_EXT_PPS_P12_SEL_BOFFSET))
			*src_id = SSDK_PHYSICAL_PORT2;
		else
			*src_id = SSDK_PHYSICAL_PORT1;
	}

	return SW_OK;
}
#endif

sw_error_t
qca_mht_mem_ctrl_set(a_uint32_t dev_id, a_uint32_t dvs_value, a_uint32_t acc_value)
{
	a_uint32_t data = 0;

	/*configure dvs value*/
	data = qca_mht_mii_read(dev_id, MEM_CTRL_OFFSET);
	data &= ~MHT_MEM_CTRL_DVS_MASK;

	/*configure dvs value, for hw limit, the two bit need to be set one by one*/
	if(dvs_value & BIT(MEM_CTRL_DVS_SA_RELAX_BOFFSET))
	{
		data |= BIT(MEM_CTRL_DVS_SA_RELAX_BOFFSET);
		qca_mht_mii_write(dev_id, MEM_CTRL_OFFSET, data);
	}
	if(dvs_value & BIT(MEM_CTRL_DVS_RAWA_ASSERT_BOFFSET))
	{
		data |= BIT(MEM_CTRL_DVS_RAWA_ASSERT_BOFFSET);
		qca_mht_mii_write(dev_id, MEM_CTRL_OFFSET, data);
	}

	/*configure acc value*/
	qca_mht_mii_write(dev_id, MEM_ACC_0_OFFSET, acc_value);

	return SW_OK;
}

a_bool_t
qca_mht_sku_check(a_uint32_t dev_id, a_uint32_t mht_sku)
{
	a_uint32_t data = 0, sku_value = 0, freq = 0;

	ssdk_miibus_freq_get(dev_id, SSDK_MII_DEFAULT_BUS_ID, &freq);
	/*fuse register need use lower mdio clock to read*/
	ssdk_miibus_freq_set(dev_id, SSDK_MII_DEFAULT_BUS_ID, 0xff);
	data = qca_mht_mii_read(dev_id, QFPROM_RAW_PTE_ROW0_LSB_OFFSET);
	/*after read fuse, need recovery the mdio clock*/
	ssdk_miibus_freq_set(dev_id, SSDK_MII_DEFAULT_BUS_ID, freq);

	sku_value = data & MHT_SKU_MASK;
	SSDK_DEBUG("MHT SKU is 0x%x\n", sku_value);
	if(mht_sku == sku_value)
		return A_TRUE;

	return A_FALSE;
}

a_bool_t
qca_mht_sku_uniphy_enabled(a_uint32_t dev_id, a_uint32_t uniphy_index)
{
	if(qca_mht_sku_check(dev_id, MHT_SKU_8082) ||
		qca_mht_sku_check(dev_id, MHT_SKU_8084) ||
		qca_mht_sku_check(dev_id, MHT_SKU_8085))
	{
		if(uniphy_index == MHT_UNIPHY_SGMII_0)
			return A_FALSE;
	}

	return A_TRUE;;
}

a_bool_t
qca_mht_sku_switch_core_enabled(a_uint32_t dev_id)
{
	if(qca_mht_sku_check(dev_id, MHT_SKU_8082) ||
		qca_mht_sku_check(dev_id, MHT_SKU_8084) ||
		qca_mht_sku_check(dev_id, MHT_SKU_8085))
		return A_FALSE;

	return A_TRUE;
}

sw_error_t
qca_mht_ethphy_icc_efuse_get(a_uint32_t dev_id, a_uint32_t mht_port_id,
	a_uint32_t *icc_value)
{
	a_uint32_t data = 0, efuse_ver = 0;

	switch(mht_port_id)
	{
		case SSDK_PHYSICAL_PORT1:
			data = qca_mht_mii_read(dev_id,
				QFPROM_RAW_CALIBRATION_ROW4_LSB_OFFSET);
			*icc_value = (data & BITS(22, 5)) >> 22;
			break;
		case SSDK_PHYSICAL_PORT2:
			data = qca_mht_mii_read(dev_id,
				QFPROM_RAW_CALIBRATION_ROW7_LSB_OFFSET);
			*icc_value = (data & BITS(27, 5)) >> 27;
			break;
		case SSDK_PHYSICAL_PORT3:
			data = qca_mht_mii_read(dev_id,
				QFPROM_RAW_CALIBRATION_ROW8_LSB_OFFSET);
			*icc_value = (data & BITS(27, 5)) >> 27;
			break;
		case SSDK_PHYSICAL_PORT4:
			data = qca_mht_mii_read(dev_id,
				QFPROM_RAW_CALIBRATION_ROW6_MSB_OFFSET);
			*icc_value = (data & BITS(18, 5)) >> 18;
			break;
		default:
			return SW_NOT_SUPPORTED;
	}
	data = qca_mht_mii_read(dev_id, QFPROM_RAW_PTE_ROW2_MSB_OFFSET);
	efuse_ver = (data & BITS(16, 8)) >> 16;
	if(efuse_ver != 1 && efuse_ver != 2)
	{
		if(*icc_value & BIT(4))
			*icc_value &= ~BIT(4);
		else
			*icc_value |= BIT(4);
	}
	SSDK_DEBUG("mht port%d efuse version is %d, icc value is 0x%x\n",
		mht_port_id, efuse_ver, *icc_value);

	return SW_OK;
}

sw_error_t
qca_mht_mdio_cfg(a_uint32_t dev_id, a_uint32_t div, a_uint32_t timer, a_uint32_t preamble_length)
{
	a_uint32_t mdio_ctrl0= 0;

	HSL_DEV_ID_CHECK(dev_id);

	mdio_ctrl0 = qca_mht_mii_read(dev_id, MDIO_CTRL0_OFFSET);

	/* Enable control timmer or not */
	if (!timer)
		mdio_ctrl0 &= ~BIT(MDIO_CTRL0_TIMER_EN_BOFFSET);
	else
		mdio_ctrl0 |= BIT(MDIO_CTRL0_TIMER_EN_BOFFSET);

	/* Configure the MDIO frequency */
	mdio_ctrl0 &= ~BITS(MDIO_CTRL0_DIV_FACTOR_BOFFSET, MDIO_CTRL0_DIV_FACTOR_BLEN);
	mdio_ctrl0 |= (div << MDIO_CTRL0_DIV_FACTOR_BOFFSET) &
		BITS(MDIO_CTRL0_DIV_FACTOR_BOFFSET, MDIO_CTRL0_DIV_FACTOR_BLEN);

	/* Trigger MDIO transmission */
	mdio_ctrl0 |= BIT(MDIO_CTRL0_TRIGGER_BOFFSET);

	/* Configure the preamble length of MDIO frame */
	mdio_ctrl0 |= (preamble_length << MDIO_CTRL0_PREAMBLE_BITS_BOFFSET) &
		BITS(MDIO_CTRL0_PREAMBLE_BITS_BOFFSET, MDIO_CTRL0_PREAMBLE_BITS_BLEN);

	qca_mht_mii_write(dev_id, MDIO_CTRL0_OFFSET, mdio_ctrl0);

	/* Configure the timmer counter cycles(1/mdio_frequency)*/
	qca_mht_mii_write(dev_id, MDIO_CTRL1_OFFSET, timer);

	return SW_OK;
}
/**
 * @}
 */
