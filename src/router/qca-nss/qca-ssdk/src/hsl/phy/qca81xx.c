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


#include <linux/phy.h>
#include <linux/delay.h>
#include <linux/version.h>

#include "hsl_phy.h"
#include "qca808x.h"
#include "qca81xx.h"
#include "qcaphy_c45_common.h"
#include "qca81xx_phy.h"

struct qca81xx_phy_mdio_data {
	struct mii_bus *mii_bus;
	struct clk *mdio_clk;
	void __iomem *membase;
	int phy_irq[PHY_MAX_ADDR];
	int clk_div;
	bool force_c22;
	void (*preinit)(struct mii_bus *bus);
	u32 (*sw_read)(struct mii_bus *bus, u32 reg);
	void (*sw_write)(struct mii_bus *bus, u32 reg, u32 val);
};

enum qca81xx_addr_offset {
	PCS_ADDR_OFFSET = 1,
	SOC_ADDR_OFFSET = 2,
};

#define TO_C45_ADDR(devad, regnum)	(MII_ADDR_C45 | (devad << 16) | (regnum & 0xffff))

/* in QCOM MDIO bus driver, bit29~31 is for soc type, 2 is for laguna */
/* and bit24~28 is for phy address, 0~23 is for soc address */
#define TO_QCA81XX_PHY_SOC_ADDR(addr, reg)		((BIT(30) | reg) | (addr << 24))

/* below two registers are used to access PHY */
/* DEBUG registers indirectly */
#define QCA81XX_DEBUG_ADDR				0x1d
#define QCA81XX_DEBUG_DATA				0x1e

/*PHY DEBUG registers*/
#define QCA81XX_ANA_DEBUG_AFE_DAC8_DP			0x2f80
#define QCA81XX_ANA_DEBUG_AFE_DAC8_DP_VAL		0x5b56
#define QCA81XX_ANA_DEBUG_AFE_DAC9_DP			0x3080
#define QCA81XX_ANA_DEBUG_AFE_DAC9_DP_VAL		0x5b57
#define QCA81XX_ANA_DEBUG_AFE_DAC38_DP			0x4d80
#define QCA81XX_ANA_DEBUG_AFE_DAC38_DP_VAL		0x2a2a
#define QCA81XX_ANA_DEBUG_AFE_DAC39_DP			0x4e80
#define QCA81XX_ANA_DEBUG_AFE_DAC39_DP_VAL		0x2a2a

/*PHY MMD3 registers*/
#define QCA81XX_MMD3_CDT_THRESH_CTRL2			0x8073
#define QCA81XX_MMD3_CDT_THRESH_CTRL2_VAL		0xb03f
#define QCA81XX_MMD3_CDT_THRESH_CTRL3			0x8074
#define QCA81XX_MMD3_CDT_THRESH_CTRL3_VAL		0xc040
#define QCA81XX_MMD3_CDT_THRESH_CTRL4			0x8075
#define QCA81XX_MMD3_CDT_THRESH_CTRL4_VAL		0xa060
#define QCA81XX_MMD3_CDT_THRESH_CTRL5			0x8076
#define QCA81XX_MMD3_CDT_THRESH_CTRL5_VAL		0xc040
#define QCA81XX_MMD3_CDT_THRESH_CTRL6			0x8077
#define QCA81XX_MMD3_CDT_THRESH_CTRL6_VAL		0xa060
#define QCA81XX_MMD3_CDT_THRESH_CTRL7			0x8078
#define QCA81XX_MMD3_CDT_THRESH_CTRL7_VAL		0xae50
#define QCA81XX_MMD3_CDT_THRESH_CTRL9			0x807a
#define QCA81XX_MMD3_CDT_THRESH_CTRL9_VAL		0xc060
#define QCA81XX_MMD3_CDT_THRESH_CTRL13			0x807e
#define QCA81XX_MMD3_CDT_THRESH_CTRL13_VAL		0xb060
#define QCA81XX_MMD3_CDT_THRESH_CTRL14			0x807f
#define QCA81XX_MMD3_CDT_THRESH_CTRL14_VAL		0x9cb0
#define QCA81XX_MMD3_DEBUG5				0xa015
#define QCA81XX_MMD3_DEBUG5_VAL				0xce80
#define QCA81XX_MMD3_AZ_1G_AFE_CTRL			0x8007
#define QCA81XX_MMD3_AZ_1G_AFE_CTRL_MASK		GENMASK(8, 4)
#define QCA81XX_MMD3_AZ_1G_DAC_EN			BIT(4)
#define QCA81XX_MMD3_AZ_1G_VGA_EN			BIT(5)
#define QCA81XX_MMD3_AZ_1G_ADC_EN			BIT(6)
#define QCA81XX_MMD3_AZ_1G_ECHO_EN			BIT(7)
#define QCA81XX_MMD3_AZ_1G_FULL_ECHO_EN			BIT(8)

/*PHY MMD31 registers*/
#define QCA81XX_FIFO_CONTROL				0x19
#define QCA81XX_FIFO_RESET				0x3

#define QCA81XX_1000BASET_CONTROL			0x9
#define QCA81XX_ADVERTISE_1000FULL			0x200
#define QCA81XX_1000BASET_STATUS			0xa
#define QCA81XX_LP_ADVERTISE_1000FULL			0x2000

#define QCA81XX_SPEC_STATUS				0x11
#define QCA81XX_INTR_DOWNSHIFT				0x20
#define QCA81XX_SS_LINK_STAT				0x400
#define QCA81XX_SS_DUPLEX_FULL				0x2000
#define QCA81XX_SS_SPEED_MASK				0x380
#define QCA81XX_SS_SPEED_10000				0x180
#define QCA81XX_SS_SPEED_5000				0x280
#define QCA81XX_SS_SPEED_2500				0x200
#define QCA81XX_SS_SPEED_1000				0x100
#define QCA81XX_SS_SPEED_100				0x80

#define QCA81XX_INTR_MASK				0x12
#define QCA81XX_INTR_STATUS				0x13
#define QCA81XX_INTR_STATUS_DOWN			0x800
#define QCA81XX_INTR_STATUS_UP				0x400

/*PCS MII registers*/
#define QCA81XX_PCS_PLL_POWER_ON_AND_RESET		0
#define QCA81XX_PCS_ANA_SOFT_RESET_MASK			0x40
#define QCA81XX_PCS_ANA_SOFT_RESET			0
#define QCA81XX_PCS_ANA_SOFT_RELEASE			0x40

#define QCA81XX_PCS_MII_DIG_CTRL			0x8000
#define QCA81XX_PCS_MMD3_USXG_FIFO_RESET		0x400

/*PCS MMD1 registers*/
#define QCA81XX_PCS_MMD1_MODE_CTRL			0x11b
#define QCA81XX_PCS_MMD1_MODE_MASK			0x1f00
#define QCA81XX_PCS_MMD1_XPCS_MODE			0x1000

#define QCA81XX_PCS_MMD1_CDA_CONTROL1			0x20
#define QCA81XX_PCS_MMD1_SSCG_ENABLE			0x8

#define QCA81XX_PCS_MMD1_CALIBRATION4			0x78
#define QCA81XX_PCS_MMD1_CALIBRATION_DONE		0x80

/*PCS MMD3 registers*/
#define QCA81XX_PCS_MMD3_AN_LP_BASE_ABL2		0x14
#define QCA81XX_PCS_MMD3_XPCS_EEE_CAP			0x40

#define QCA81XX_PCS_MMD3_PCS_CTRL2			0x7
#define QCA81XX_PCS_MMD3_PCS_TYPE_MASK			0xf
#define QCA81XX_PCS_MMD3_PCS_TYPE_10GBASE_R		0

#define QCA81XX_PCS_MMD3_10GBASE_R_PCS_STATUS1		0x20
#define QCA81XX_PCS_MMD3_10GBASE_R_UP			0x1000

#define QCA81XX_PCS_MMD3_DIG_CTRL1			0x8000
#define QCA81XX_PCS_MMD3_USXGMII_EN			0x200
#define QCA81XX_PCS_MMD3_XPCS_SOFT_RESET		0x8000

#define QCA81XX_PCS_MMD3_AN_LP_BASE_ABL2		0x14

#define QCA81XX_PCS_MMD3_EEE_MODE_CTRL			0x8006
#define QCA81XX_PCS_MMD3_EEE_RES_REGS			0x100
#define QCA81XX_PCS_MMD3_EEE_SIGN_BIT_REGS		0x40
#define QCA81XX_PCS_MMD3_EEE_EN				0x3

#define QCA81XX_PCS_MMD3_EEE_TX_TIMER			0x8008
#define QCA81XX_PCS_MMD3_EEE_TSL_REGS			0xa
#define QCA81XX_PCS_MMD3_EEE_TLU_REGS			0xc0
#define QCA81XX_PCS_MMD3_EEE_TWL_REGS			0x1600

#define QCA81XX_PCS_MMD3_EEE_MODE_CTRL1			0x800b
#define QCA81XX_PCS_MMD3_EEE_TRANS_LPI_MODE		0x1
#define QCA81XX_PCS_MMD3_EEE_TRANS_RX_LPI_MODE		0x100

#define QCA81XX_PCS_MMD3_EEE_RX_TIMER			0x8009
#define QCA81XX_PCS_MMD3_EEE_100US_REG_REGS		0xc8
#define QCA81XX_PCS_MMD3_EEE_RWR_REG_REGS		0x1c00

#define QCA81XX_PCS_MMD3_USXG_FIFO_RESET		0x400

/*PCS MMD31 register*/
#define QCA81XX_PCS_MMD31_MII_DIG_CTRL			0x8000
#define QCA81XX_PCS_MMD31_PHY_MODE_CTRL_EN		0x1

#define QCA81XX_PCS_MMD31_MII_AN_INT_MSK		0x8001
#define QCA81XX_PCS_MMD31_AN_COMPLETE_INT		0x1
#define QCA81XX_PCS_MMD31_MII_4BITS_CTRL		0
#define QCA81XX_PCS_MMD31_TX_CONFIG_CTRL		0x8

#define QCA81XX_PCS_MMD31_MII_ERR_SEL			0x8002
#define QCA81XX_PCS_MMD31_MII_XAUI_MODE_CTRL		0x8004
#define QCA81XX_PCS_MMD31_MII_CTRL			0
#define QCA81XX_PCS_MMD31_MII_AN_ENABLE			0x1000

#define QCA81XX_PCS_MMD31_MII_ERR_SEL			0x8002
#define QCA81XX_PCS_MMD31_XPCS_SPEED_MASK		0x2060
#define QCA81XX_PCS_MMD31_XPCS_SPEED_10000		0x2040
#define QCA81XX_PCS_MMD31_XPCS_SPEED_5000		0x2020
#define QCA81XX_PCS_MMD31_XPCS_SPEED_2500		0x20
#define QCA81XX_PCS_MMD31_XPCS_SPEED_1000		0x40
#define QCA81XX_PCS_MMD31_XPCS_SPEED_100		0x2000
#define QCA81XX_PCS_MMD31_AN_RESTART			0x200
#define QCA81XX_PCS_MMD31_MII_AN_COMPLETE_INT		0x1

/*SOC GCC registers*/
#define GCC_E2S_TX_CMD_RCGR				0x800000
#define GCC_E2S_TX_CFG_RCGR				0x800004
#define GCC_E2S_TX_DIV_CDIVR				0x800008
#define GCC_E2S_SRDS_CH0_RX_CBCR			0x800010
#define GCC_E2S_GEPHY_TX_CBCR				0x800014
#define GCC_E2S_RX_CMD_RCGR				0x800018
#define GCC_E2S_RX_CFG_RCGR				0x80001c
#define GCC_E2S_RX_DIV_CDIVR				0x800020
#define GCC_E2S_SRDS_CH0_TX_CBCR			0x800028
#define GCC_E2S_GEPHY_RX_CBCR				0x80002c
#define GCC_AHB_CMD_RCGR				0x80003c
#define GCC_AHB_CFG_RCGR				0x800040
#define GCC_SRDS_SYS_CBCR				0x80007c
#define GCC_GEPHY_SYS_CBCR				0x800080
#define GCC_SEC_CTRL_CMD_RCGR				0x800088
#define GCC_SEC_CTRL_CFG_RCGR				0x80008c
#define GCC_SERDES_CTL					0x80030C

#define GCC_CLK_ENABLE					0x1
#define GCC_CLK_ARES					0x4
#define XPCS_PWR_ARES					0x1
#define GCC_E2S_SRC_MASK				GENMASK(10, 8)
#define GCC_E2S_SRC0_REF_50MCLK				0
#define GCC_E2S_SRC1_EPHY_TXCLK				1
#define GCC_E2S_SRC2_EPHY_RXCLK				2
#define GCC_E2S_SRC3_SRDS_TXCLK				3
#define GCC_E2S_SRC4_SRDS_RXCLK				4

#define SRC_DIV_MASK					GENMASK(4, 0)
#define CLK_DIV_MASK					GENMASK(3, 0)
#define CLK_CMD_UPDATE					BIT(0)

/*SOC SEC_TCSR registers*/
#define EPHY_CFG					0x90F018
#define EPHY_LDO_CTRL					BIT(20)
#define GLOBAL_INTR_CTRL				0x90f008
#define PHY_INTR_EN					BIT(7)
#define WOL_INTR_CTRL					0x90f010
#define WOL_INTR_EN					BIT(0)

static int qca81xx_phy_debug_write(struct phy_device *phydev,
		unsigned int reg, u16 val)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return hsl_phy_c45_debug_reg_write(pdata->dev_id, pdata->phy_addr,
				       reg, val);
}

int qca81xx_phy_debug_modify(struct phy_device *phydev,
			     unsigned int reg, u16 clear, u16 set)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return hsl_phy_c45_modify_debug(pdata->dev_id, pdata->phy_addr,
				    reg, clear, set);
}

static int qca81xx_phy_modify(struct phy_device *phydev, int devad,
			      u32 regnum, int mask, int set)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	if (devad < 0)
		return hsl_phy_modify_mii(pdata->dev_id, pdata->phy_addr,
					  regnum, mask, set);

	return hsl_phy_modify_mmd(pdata->dev_id, pdata->phy_addr, true,
				  devad, regnum, mask, set);
}

static int qca81xx_phy_read(struct phy_device *phydev, int devad, u32 regnum)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	if (devad < 0)
		return hsl_phy_mii_reg_read(pdata->dev_id, pdata->phy_addr, regnum);

	return hsl_phy_mmd_reg_read(pdata->dev_id, pdata->phy_addr, true,
				    devad, regnum);
}

static int qca81xx_phy_write(struct phy_device *phydev, int devad, u32 regnum, int val)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	if (devad < 0)
		return hsl_phy_mii_reg_write(pdata->dev_id, pdata->phy_addr,
					     regnum, val);

	return hsl_phy_mmd_reg_write(pdata->dev_id, pdata->phy_addr, true,
				     devad, regnum, val);
}

int qca81xx_soc_modify(struct phy_device *phydev, u32 reg,
		u32 mask, u32 set)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return hsl_phy_modify_soc(pdata->dev_id, (pdata->phy_addr + SOC_ADDR_OFFSET),
			TO_QCA81XX_PHY_SOC_ADDR((pdata->phy_addr + SOC_ADDR_OFFSET), reg),
			mask, set);
}

static int qca81xx_pcs_read_mmd(struct phy_device *phydev,
		int devad, u32 regnum)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return hsl_phy_mmd_reg_read(pdata->dev_id, (pdata->phy_addr + PCS_ADDR_OFFSET),
			true, devad, regnum);
}

static int qca81xx_pcs_modify_mmd(struct phy_device *phydev,
		int devad, u32 regnum, u16 mask, u16 set)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return hsl_phy_modify_mmd(pdata->dev_id, (pdata->phy_addr + PCS_ADDR_OFFSET),
			true, devad, regnum, mask, set);
}

static int qca81xx_pcs_modify(struct phy_device *phydev,
		u32 regnum, u16 mask, u16 set)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return hsl_phy_modify_mii(pdata->dev_id, (pdata->phy_addr + PCS_ADDR_OFFSET),
			regnum, mask, set);
}

static int qca81xx_pcs_txclk_en_set(struct phy_device *phydev,
		bool enable)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_TX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_pcs_rxclk_en_set(struct phy_device *phydev,
		bool enable)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_RX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_pcs_clk_en_set(struct phy_device *phydev,
		bool enable)
{
	int ret;

	ret = qca81xx_pcs_txclk_en_set(phydev, enable);
	if (ret < 0)
		return ret;

	return qca81xx_pcs_rxclk_en_set(phydev, enable);
}

static int qca81xx_pcs_clk_reset_update(struct phy_device *phydev,
		bool assert)
{
	int ret;

	ret = qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_RX_CBCR,
		GCC_CLK_ARES, assert ? GCC_CLK_ARES : 0);
	if (ret < 0)
		return ret;

	return qca81xx_soc_modify(phydev, GCC_E2S_SRDS_CH0_TX_CBCR,
		GCC_CLK_ARES, assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_pcs_clk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_pcs_clk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	usleep_range(1000, 1100);

	return qca81xx_pcs_clk_reset_update(phydev, false);
}

static int qca81xx_pcs_sysclk_en_set(struct phy_device *phydev,
		bool enable)
{
	return qca81xx_soc_modify(phydev, GCC_SRDS_SYS_CBCR, GCC_CLK_ENABLE,
			enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_pcs_sysclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_SRDS_SYS_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_pcs_sysclk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_pcs_sysclk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	usleep_range(1000, 1100);

	return qca81xx_pcs_sysclk_reset_update(phydev, false);
}

static int qca81xx_xpcs_clk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_SERDES_CTL, XPCS_PWR_ARES,
		assert ? XPCS_PWR_ARES : 0);
}

static int qca81xx_phy_clk_en_set(struct phy_device *phydev, bool enable)
{
	int ret;

	ret = qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_TX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
	if (ret < 0)
		return ret;

	return qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_RX_CBCR,
		GCC_CLK_ENABLE, enable ? GCC_CLK_ENABLE : 0);
}

static int qca81xx_phy_txclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_TX_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_phy_rxclk_reset_update(struct phy_device *phydev,
	bool assert)
{
	return qca81xx_soc_modify(phydev, GCC_E2S_GEPHY_RX_CBCR, GCC_CLK_ARES,
		assert ? GCC_CLK_ARES : 0);
}

static int qca81xx_phy_clk_reset_update(struct phy_device *phydev,
	bool assert)
{
	int ret;

	ret = qca81xx_phy_txclk_reset_update(phydev, assert);
	if (ret < 0)
		return ret;

	return qca81xx_phy_rxclk_reset_update(phydev, assert);
}

static int qca81xx_phy_clk_reset(struct phy_device *phydev)
{
	int ret;

	ret = qca81xx_phy_clk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	usleep_range(1000, 1100);

	return qca81xx_phy_clk_reset_update(phydev, false);
}

static int qca81xx_phy_speed_clk_set(struct phy_device *phydev)
{
	int ret, div0 = 0, div1 = 0;

	switch (phydev->speed) {
	case SPEED_100:
		/* 312.5 divided by 2.5*5 */
		div0 = 4;
		div1 = 4;
		break;
	case SPEED_1000:
		/* 312.5 divided by 2.5*1 */
		div0 = 4;
		div1 = 0;
		break;
	case SPEED_2500:
		/* 312.5 divided by 1*4 */
		div0 = 1;
		div1 = 3;
		break;
	case SPEED_5000:
		/* 312.5 divided by 1*2 */
		div0 = 1;
		div1 = 1;
		break;
	case SPEED_10000:
		/* 312.5 divided by 1*1 */
		div0 = 1;
		div1 = 0;
		break;
	default:
		break;
	}

	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_CFG_RCGR,
		SRC_DIV_MASK, div0);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_DIV_CDIVR,
		CLK_DIV_MASK, div1);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_CMD_RCGR,
		CLK_CMD_UPDATE, CLK_CMD_UPDATE);
	if (ret < 0)
		return ret;

	ret = qca81xx_soc_modify(phydev, GCC_E2S_RX_CFG_RCGR,
		SRC_DIV_MASK, div0);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_E2S_RX_DIV_CDIVR,
		CLK_DIV_MASK, div1);
	if (ret < 0)
		return ret;

	return qca81xx_soc_modify(phydev, GCC_E2S_RX_CMD_RCGR,
		CLK_CMD_UPDATE, CLK_CMD_UPDATE);
}

static int qca81xx_pcs_eee_enable(struct phy_device *phydev)
{
	int ret = 0;

	/*Configure the EEE related timer*/
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_MODE_CTRL,
		0xf40,
		QCA81XX_PCS_MMD3_EEE_RES_REGS |
		QCA81XX_PCS_MMD3_EEE_SIGN_BIT_REGS);
	if (ret < 0)
		return ret;

	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_TX_TIMER,
		0x1fff,
		QCA81XX_PCS_MMD3_EEE_TSL_REGS|
		QCA81XX_PCS_MMD3_EEE_TLU_REGS |
		QCA81XX_PCS_MMD3_EEE_TWL_REGS);
	if (ret < 0)
		return ret;

	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_RX_TIMER,
		0x1fff,
		QCA81XX_PCS_MMD3_EEE_100US_REG_REGS|
		QCA81XX_PCS_MMD3_EEE_RWR_REG_REGS);
	if (ret < 0)
		return ret;

	/*enable TRN_LPI*/
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_EEE_MODE_CTRL1,
		0x101,
		QCA81XX_PCS_MMD3_EEE_TRANS_LPI_MODE |
		QCA81XX_PCS_MMD3_EEE_TRANS_RX_LPI_MODE);
	if (ret < 0)
		return ret;

	/*enable TX/RX LPI pattern*/
	ret = qca81xx_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_PCS_MMD3_EEE_MODE_CTRL, 0x3,
		QCA81XX_PCS_MMD3_EEE_EN);

	return ret;
}

static int qca81xx_phy_probe(struct phy_device *phydev)
{
	int ret = 0;

#if defined(IN_LINUX_STD_PTP)
	if (phydev->priv != NULL)
		ret = qca808x_ptp_init((qca808x_priv*)(phydev->priv));
#endif
	return ret;
}

void qca81xx_phy_remove(struct phy_device *phydev)
{
#if defined(IN_LINUX_STD_PTP)
	qca808x_ptp_deinit((qca808x_priv*)(phydev->priv));
#endif
}

static int qca81xx_suspend(struct phy_device *phydev)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return qca81xx_phy_poweroff(pdata->dev_id, pdata->phy_addr);
}

static int qca81xx_resume(struct phy_device *phydev)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return qca81xx_phy_poweron(pdata->dev_id, pdata->phy_addr);

}

static int qca81xx_soft_reset(struct phy_device *phydev)
{
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	return qca81xx_phy_soft_reset(pdata->dev_id, pdata->phy_addr);
}

static int qca81xx_pcs_poll_timeout(struct phy_device *phydev, int devad,
		int reg, int cond)
{
	int val, retries = 1000;

	do {
		val = qca81xx_pcs_read_mmd(phydev, devad, reg);
		if (val < 0)
			return val;
		usleep_range(1000, 1100);
	} while (!(val & cond) && --retries);

	return (val && retries > 0) ? 0 : -ETIMEDOUT;
}

static int qca81xx_pcs_usxgmii_init(struct phy_device *phydev)
{
	int ret = 0;
	u16 phy_data = 0;
	int retries = 100;

	ret = qca81xx_phy_clk_en_set(phydev, false);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_clk_en_set(phydev, false);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_sysclk_en_set(phydev, true);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_sysclk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_xpcs_clk_reset_update(phydev, true);
	if (ret < 0)
		return ret;
	/* optional, would settle after SOD VI, write 1 to */
	/* CSR0 MMD1_reg0x7c[3],to invert pcs txclk */
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PMAPMD, QCA81XX_PCS_MMD1_MODE_CTRL,
		QCA81XX_PCS_MMD1_MODE_MASK,
		QCA81XX_PCS_MMD1_XPCS_MODE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify(phydev,
		QCA81XX_PCS_PLL_POWER_ON_AND_RESET,
		QCA81XX_PCS_ANA_SOFT_RESET_MASK,
		QCA81XX_PCS_ANA_SOFT_RESET);
	if (ret < 0)
		return ret;
	usleep_range(1000, 1100);
	ret |= qca81xx_pcs_modify(phydev,
		QCA81XX_PCS_PLL_POWER_ON_AND_RESET,
		QCA81XX_PCS_ANA_SOFT_RESET_MASK,
		QCA81XX_PCS_ANA_SOFT_RELEASE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_poll_timeout(phydev, MDIO_MMD_PMAPMD,
			QCA81XX_PCS_MMD1_CALIBRATION4,
			QCA81XX_PCS_MMD1_CALIBRATION_DONE);
	if (ret < 0)
		phydev_warn(phydev, "PCS callibration time out!\n");
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PMAPMD, QCA81XX_PCS_MMD1_CDA_CONTROL1,
		QCA81XX_PCS_MMD1_SSCG_ENABLE,
		QCA81XX_PCS_MMD1_SSCG_ENABLE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_txclk_en_set(phydev, true);
	if (ret < 0)
		return ret;
	ret = qca81xx_xpcs_clk_reset_update(phydev, false);
	if (ret < 0)
		return ret;
	ret = qca81xx_soft_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_PCS_CTRL2,
		QCA81XX_PCS_MMD3_PCS_TYPE_MASK,
		QCA81XX_PCS_MMD3_PCS_TYPE_10GBASE_R);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_poll_timeout(phydev,MDIO_MMD_PCS,
			QCA81XX_PCS_MMD3_10GBASE_R_PCS_STATUS1,
			QCA81XX_PCS_MMD3_10GBASE_R_UP);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev, MDIO_MMD_PCS,
		QCA81XX_PCS_MMD3_DIG_CTRL1,
		QCA81XX_PCS_MMD3_USXGMII_EN,
		QCA81XX_PCS_MMD3_USXGMII_EN);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MMD3_DIG_CTRL1,
		QCA81XX_PCS_MMD3_XPCS_SOFT_RESET,
		QCA81XX_PCS_MMD3_XPCS_SOFT_RESET);
	if (ret < 0)
		return ret;

	retries = 100;
	do {
		phy_data = qca81xx_pcs_read_mmd(phydev, MDIO_MMD_PCS, QCA81XX_PCS_MMD3_DIG_CTRL1);
		if (phy_data < 0) {
			phydev_err(phydev, "xpcs software reset read fail\n");
			return phy_data;
		}

		if (!(phy_data & QCA81XX_PCS_MMD3_XPCS_SOFT_RESET))
			break;
		usleep_range(1000, 1100);
	} while (--retries);

	if (retries <= 0) {
		phydev_err(phydev, "xpcs software reset timeout\n");
		return ret;
	}

	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_AN_INT_MSK,
		0x109,
		QCA81XX_PCS_MMD31_AN_COMPLETE_INT |
		QCA81XX_PCS_MMD31_MII_4BITS_CTRL |
		QCA81XX_PCS_MMD31_TX_CONFIG_CTRL);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_DIG_CTRL, BIT(0),
		QCA81XX_PCS_MMD31_PHY_MODE_CTRL_EN);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_CTRL,
		QCA81XX_PCS_MMD31_MII_AN_ENABLE,
		QCA81XX_PCS_MMD31_MII_AN_ENABLE);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_eee_enable(phydev);

	return ret;
}

/* adjust the AFE DAC parameter and the CDT threshold for cable */
/* status precision such as open, short, normal */
static int qca81xx_phy_cdt_thresh_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL2,
		QCA81XX_MMD3_CDT_THRESH_CTRL2_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL3,
		QCA81XX_MMD3_CDT_THRESH_CTRL3_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL4,
		QCA81XX_MMD3_CDT_THRESH_CTRL4_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL5,
		QCA81XX_MMD3_CDT_THRESH_CTRL5_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL6,
		QCA81XX_MMD3_CDT_THRESH_CTRL6_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL7,
		QCA81XX_MMD3_CDT_THRESH_CTRL7_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL9,
		QCA81XX_MMD3_CDT_THRESH_CTRL9_VAL);
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL13,
		QCA81XX_MMD3_CDT_THRESH_CTRL13_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev, MDIO_MMD_PCS,
		QCA81XX_MMD3_CDT_THRESH_CTRL14,
		QCA81XX_MMD3_CDT_THRESH_CTRL14_VAL);

	return ret;
}

static int qca81xx_phy_gcc_pre_init(struct phy_device *phydev)
{
	int ret;

	/* enable efuse loading into analog circuit */
	ret = qca81xx_soc_modify(phydev, EPHY_CFG, EPHY_LDO_CTRL, 0);
	usleep_range(10000, 11000);

	return ret;
}

static int qca81xx_phy_gcc_post_init(struct phy_device *phydev)
{
	int ret;

	/* security control clock switch as 25M */
	ret = qca81xx_soc_modify(phydev, GCC_SEC_CTRL_CFG_RCGR,
		GCC_E2S_SRC_MASK | SRC_DIV_MASK, 0x3);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, GCC_SEC_CTRL_CMD_RCGR,
		CLK_CMD_UPDATE, CLK_CMD_UPDATE);
	if (ret < 0)
		return ret;

	/*select uphy rx, ephy tx clock source as srds_rxclk*/
	ret = qca81xx_soc_modify(phydev, GCC_E2S_TX_CFG_RCGR,
		GCC_E2S_SRC_MASK, GCC_E2S_SRC4_SRDS_RXCLK << 8);
	if (ret < 0)
		return ret;

	/*select uphy tx, ephy rx clock source as srds_txclk*/
	ret = qca81xx_soc_modify(phydev, GCC_E2S_RX_CFG_RCGR,
		GCC_E2S_SRC_MASK, GCC_E2S_SRC3_SRDS_TXCLK << 8);

	return ret;
}

/* Fix some chip can not link to 10G automatically with long cable */
static int qca81xx_phy_afe_dac_config_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_phy_debug_write(phydev,
			QCA81XX_ANA_DEBUG_AFE_DAC8_DP,
			QCA81XX_ANA_DEBUG_AFE_DAC8_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev,
			QCA81XX_ANA_DEBUG_AFE_DAC9_DP,
			QCA81XX_ANA_DEBUG_AFE_DAC9_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev,
			QCA81XX_ANA_DEBUG_AFE_DAC38_DP,
			QCA81XX_ANA_DEBUG_AFE_DAC38_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_debug_write(phydev,
			QCA81XX_ANA_DEBUG_AFE_DAC39_DP,
			QCA81XX_ANA_DEBUG_AFE_DAC39_DP_VAL);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_write(phydev,
			MDIO_MMD_PCS, QCA81XX_MMD3_DEBUG5,
			QCA81XX_MMD3_DEBUG5_VAL);

	return ret;
}

static int qca81xx_sec_ctrl_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_soc_modify(phydev, GLOBAL_INTR_CTRL,
		PHY_INTR_EN, PHY_INTR_EN);
	if (ret < 0)
		return ret;
	ret = qca81xx_soc_modify(phydev, WOL_INTR_CTRL,
		WOL_INTR_EN, WOL_INTR_EN);

	return ret;
}

static int qca81xx_tlmm_init(struct phy_device *phydev)
{
	int ret = 0, pin_id = 0;

	/* the GPIO function bit2~5 is set 1 means the expected function */
	/* such as GPIO0 is WOL INT function and GPIO2 is LED0 function */
	for (pin_id  = GPIO0_WOL_INT; pin_id <= GPIO4_LED3; pin_id++) {
		ret = qca81xx_soc_modify(phydev, TO_TLMM_CFG_REG(pin_id),
			TLMM_FUNC_MASK, BIT(2));
		if (ret < 0)
			return ret;
	}

	return 0;
}

static int qca81xx_phy_eee_config_init(struct phy_device *phydev)
{
	/* disable AFE control for 1G EEE to keep FULLECHO, ECHO, ADC, VGA */
	/* and DAC always on */
	return qca81xx_phy_modify(phydev,
			MDIO_MMD_PCS, QCA81XX_MMD3_AZ_1G_AFE_CTRL,
			QCA81XX_MMD3_AZ_1G_AFE_CTRL_MASK, 0);
}

static int qca81xx_phy_config_init(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_phy_gcc_pre_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_afe_dac_config_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_usxgmii_init(phydev);
	if (ret < 0)
		return ret;
	phydev->interface = PHY_INTERFACE_MODE_USXGMII;

	ret = qca81xx_phy_gcc_post_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_eee_config_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_sec_ctrl_init(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_tlmm_init(phydev);
	if (ret < 0)
		return ret;

	ret = qca81xx_phy_cdt_thresh_init(phydev);
	if (ret < 0)
		return ret;

	return 0;
}

static int qca81xx_phy_get_features(struct phy_device *phydev)
{
	unsigned long *supported = phydev->supported;

	linkmode_or(supported, supported, phy_10gbit_full_features);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT, supported);

	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Half_BIT,
		supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
		supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
		supported);
	linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Half_BIT,
		supported);

	return 0;
}

static u32 qca81xx_phydev_negtiation_cap_get(struct phy_device *phydev)
{
	a_uint32_t autoneg = 0;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(advertising) = { 0, };

	linkmode_and(advertising, phydev->advertising, phydev->supported);

	if (linkmode_test_bit(ETHTOOL_LINK_MODE_Pause_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_PAUSE;
	}
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_Asym_Pause_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_ASY_PAUSE;
	}
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_100TX_FD;
	}
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_1000T_FD;
	}
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_2500T_FD;
	}
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_5000baseT_Full_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_5000T_FD;
	}
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_10000baseT_Full_BIT, advertising)) {
		autoneg |= FAL_PHY_ADV_10000T_FD;
	}

	return autoneg;
}

static int qca81xx_phy_config_aneg(struct phy_device *phydev)
{
	u32 advertise = 0, advertise_new = 0;
	struct qca808x_phy_info *pdata;
	qca808x_priv *priv;
	int autoneg, ret;


	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;

	if (phydev->autoneg == AUTONEG_DISABLE)
		return qcaphy_c45_force_speed_set(pdata->dev_id, pdata->phy_addr, phydev->speed);

	/* get the current autoneg status. */
	autoneg = qcaphy_c45_autoneg_status(pdata->dev_id, pdata->phy_addr);
	ret = qcaphy_c45_get_autoneg_adv(pdata->dev_id, pdata->phy_addr, &advertise);
	if (ret)
		return ret;

	/* get required autoneg capabilities. */
	advertise_new = qca81xx_phydev_negtiation_cap_get(phydev);
	if (advertise_new == advertise && phydev->autoneg == autoneg)
		return 0;

	if (advertise_new != advertise) {
		ret = qcaphy_c45_set_autoneg_adv(pdata->dev_id, pdata->phy_addr, advertise_new);
		if (ret)
			return ret;
	}

	/* Clause 45 has no standardized support for 1000BaseT, */
	/* therefore use vendor registers. */
	if (linkmode_test_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT, phydev->advertising))
		autoneg = QCA81XX_ADVERTISE_1000FULL;
	else
		autoneg = 0;

	ret = qca81xx_phy_modify(phydev, MDIO_MMD_VEND2, QCA81XX_1000BASET_CONTROL,
			QCA81XX_ADVERTISE_1000FULL, autoneg);
	if (ret < 0)
		return ret;

	return qcaphy_c45_autoneg_restart(pdata->dev_id, pdata->phy_addr);
}

static int qca81xx_phy_fifo_reset(struct phy_device *phydev,
	bool enable)
{
	return qca81xx_phy_modify(phydev,
			MDIO_MMD_VEND2, QCA81XX_FIFO_CONTROL,
			QCA81XX_FIFO_RESET, enable ? 0 : QCA81XX_FIFO_RESET);
}

static int qca81xx_phy_speed_fixup(struct phy_device *phydev)
{
	int ret = 0;
	bool port_clock_en = false;

	qca81xx_pcs_poll_timeout(phydev,MDIO_MMD_VEND2,
			QCA81XX_PCS_MMD31_MII_ERR_SEL,
			QCA81XX_PCS_MMD31_MII_AN_COMPLETE_INT);
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_VEND2, QCA81XX_PCS_MMD31_MII_ERR_SEL,
		QCA81XX_PCS_MMD31_MII_AN_COMPLETE_INT, 0);
	if (ret < 0)
		return ret;
	usleep_range(10000, 11000);
	if (phydev->link) {
		ret = qca81xx_phy_speed_clk_set(phydev);
		if (ret < 0)
			return ret;
		/*avoid garbe data transmit out, need to assert ephy tx clock*/
		qca81xx_phy_txclk_reset_update(phydev, true);
		port_clock_en = true;
	}
	ret = qca81xx_pcs_clk_en_set(phydev, port_clock_en);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_clk_en_set(phydev, port_clock_en);
	if (ret < 0)
		return ret;
	usleep_range(10000, 11000);

	ret = qca81xx_pcs_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_clk_reset(phydev);
	if (ret < 0)
		return ret;
	ret = qca81xx_pcs_modify_mmd(phydev,
		MDIO_MMD_PCS, QCA81XX_PCS_MII_DIG_CTRL,
		QCA81XX_PCS_MMD3_USXG_FIFO_RESET,
		QCA81XX_PCS_MMD3_USXG_FIFO_RESET);
	if (ret < 0)
		return ret;
	ret = qca81xx_phy_fifo_reset(phydev, true);
	if (ret < 0)
		return ret;
	usleep_range(1000, 1100);
	if (phydev->link) {
		ret = qca81xx_phy_fifo_reset(phydev, false);
		if (ret < 0)
			return ret;

	}

	return 0;
}

static int qca81xx_phy_read_status(struct phy_device *phydev)
{
	struct qca808x_phy_info *pdata;
	unsigned old_link = 0;
	qca808x_priv *priv;
	int ret = 0;

	priv = phydev->priv;
	pdata = priv->phy_info;
	if (!pdata)
		return -EINVAL;
	/* if loopback is enabled, no need to read PHY */
	if (phydev->loopback_enabled)
		return 0;
	old_link = phydev->link;

	/* Clause 45 has no standardized support for 1000BaseT, */
	/* therefore use vendor registers. */
	if (phydev->autoneg == AUTONEG_ENABLE) {
		ret = qca81xx_phy_read(phydev, MDIO_MMD_VEND2,
			QCA81XX_1000BASET_STATUS);
		if (ret < 0)
			return ret;
		linkmode_mod_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			 phydev->lp_advertising,
			 ret & QCA81XX_LP_ADVERTISE_1000FULL);
	}

	ret = qca81xx_phy_read(phydev, MDIO_MMD_VEND2,
		QCA81XX_SPEC_STATUS);
	if (ret < 0)
		return ret;

	if (ret & QCA81XX_SS_LINK_STAT)
		phydev->link = 1;
	else
		phydev->link = 0;

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (phydev->link) {
		switch (ret & QCA81XX_SS_SPEED_MASK) {
			case QCA81XX_SS_SPEED_10000:
				phydev->speed = SPEED_10000;
				break;
			case QCA81XX_SS_SPEED_5000:
				phydev->speed = SPEED_5000;
				break;
			case QCA81XX_SS_SPEED_2500:
				phydev->speed = SPEED_2500;
				break;
			case QCA81XX_SS_SPEED_1000:
				phydev->speed = SPEED_1000;
				break;
			case QCA81XX_SS_SPEED_100:
				phydev->speed = SPEED_100;
				break;
			default:
				phydev->speed = SPEED_UNKNOWN;
		}

		if (ret & QCA81XX_SS_DUPLEX_FULL)
			phydev->duplex = DUPLEX_FULL;
		else
			phydev->duplex = DUPLEX_UNKNOWN;

		/* get pause status */
		ret = qca81xx_phy_read(phydev, MDIO_MMD_AN, MDIO_AN_LPA);
		if (ret < 0)
			return ret;

		mii_adv_mod_linkmode_adv_t(phydev->lp_advertising, ret);
		phy_resolve_aneg_pause(phydev);
	}

	ret = qca81xx_phy_read(phydev, MDIO_MMD_AN, MDIO_STAT1);
	if (ret < 0)
		return ret;

	if (!(ret & MDIO_AN_STAT1_COMPLETE)) {
		linkmode_clear_bit(ETHTOOL_LINK_MODE_Autoneg_BIT,
				phydev->lp_advertising);
		mii_10gbt_stat_mod_linkmode_lpa_t(phydev->lp_advertising, 0);
		mii_adv_mod_linkmode_adv_t(phydev->lp_advertising, 0);
		phydev->pause = 0;
		phydev->asym_pause = 0;
	} else {
		ret = qca81xx_phy_read(phydev, MDIO_MMD_AN, MDIO_AN_10GBT_STAT);
		if (ret < 0)
			return ret;

		mii_10gbt_stat_mod_linkmode_lpa_t(phydev->lp_advertising, ret);
	}

	if (phydev->link != old_link)
		qca81xx_phy_speed_fixup(phydev);

	return 0;
}

static int qca81xx_phy_ack_interrupt(struct phy_device *phydev)
{
	int ret = 0;

	ret = qca81xx_phy_read(phydev, MDIO_MMD_VEND2,
		QCA81XX_INTR_STATUS);

	return (ret < 0) ? ret : 0;
}

static int qca81xx_phy_config_intr(struct phy_device *phydev)
{
	int ret = 0;
	u16 phy_data = 0;

	phy_data = qca81xx_phy_read(phydev, MDIO_MMD_VEND2,
		QCA81XX_INTR_MASK);

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED) {
		ret = qca81xx_phy_ack_interrupt(phydev);
		if (ret < 0)
			return ret;
		phy_data = QCA81XX_INTR_STATUS_DOWN |
			QCA81XX_INTR_STATUS_UP;
		ret = qca81xx_phy_write(phydev, MDIO_MMD_VEND2,
			QCA81XX_INTR_MASK, phy_data);
	} else {
		ret = qca81xx_phy_write(phydev, MDIO_MMD_VEND2,
			QCA81XX_INTR_MASK, 0);
		if (ret < 0)
			return ret;
		ret = qca81xx_phy_ack_interrupt(phydev);
	}

	return ret;
}

static struct phy_driver qca81xx_phy_driver = {
	PHY_ID_MATCH_EXACT(QCA8111_PHY),
	.name		= QCA81XX_PHY_DRIVER_NAME,
	.probe		= qca81xx_phy_probe,
	.remove		= qca81xx_phy_remove,
	.config_init	= qca81xx_phy_config_init,
	.get_features	= qca81xx_phy_get_features,
	.config_aneg	= qca81xx_phy_config_aneg,
	.config_intr	= qca81xx_phy_config_intr,
	.read_status	= qca81xx_phy_read_status,
	.suspend	= qca81xx_suspend,
	.resume		= qca81xx_resume,
	.soft_reset	= qca81xx_soft_reset,
};

int qca81xx_phy_driver_register(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0))
	return phy_driver_register(&qca81xx_phy_driver, THIS_MODULE);
#else
	return phy_driver_register(&qca81xx_phy_driver);
#endif
}

void qca81xx_phy_driver_unregister(void)
{
	phy_driver_unregister(&qca81xx_phy_driver);
}
