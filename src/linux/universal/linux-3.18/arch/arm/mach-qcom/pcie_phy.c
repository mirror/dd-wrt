/* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/*
 * MSM PCIe PHY driver.
 */

#include <linux/io.h>
#include <linux/delay.h>
#include "pcie.h"
#include "pcie_phy.h"

#define MDIO_CTRL_0_REG			0x40
#define MDIO_CTRL_1_REG			0x44
#define MDIO_CTRL_2_REG			0x48
#define MDIO_CTRL_3_REG			0x4C
#define MDIO_CTRL_4_REG			0x50

#define MDIO_PCIE_PHY_ID		(0x5 << 13)
#define MDC_MODE			(0x1 << 12)
#define MDIO_CLAUSE_22			(0x0 << 8)
#define MDIO_CLAUSE_45			(0x1 << 8)
#define MDIO_PCIE_CLK_DIV		(0xF)
#define MDIO_MMD_ID			(0x1)

#define MDIO_ACCESS_BUSY		(0x1 << 16)
#define MDIO_ACCESS_START		(0x1 << 8)
#define MDIO_TIMEOUT_STATIC		1000

#define MDIO_ACCESS_22_WRITE		(0x1)
#define MDIO_ACCESS_22_READ		(0x0)
#define MDIO_ACCESS_45_WRITE		(0x2)
#define MDIO_ACCESS_45_READ		(0x1)
#define MDIO_ACCESS_45_READ_ADDR	(0x0)

static inline void write_phy(void *base, u32 offset, u32 value)
{
	writel_relaxed(value, base + offset);
	wmb();
}

#ifndef CONFIG_ARCH_MDM9630
static inline void pcie20_phy_init_default(struct msm_pcie_dev_t *dev)
{

	PCIE_DBG(dev, "RC%d: Initializing 28nm QMP phy - 19.2MHz\n",
		dev->rc_idx);

	write_phy(dev->phy, PCIE_PHY_POWER_DOWN_CONTROL,		0x03);
	write_phy(dev->phy, QSERDES_COM_SYSCLK_EN_SEL,		0x08);
	write_phy(dev->phy, QSERDES_COM_DEC_START1,			0x82);
	write_phy(dev->phy, QSERDES_COM_DEC_START2,			0x03);
	write_phy(dev->phy, QSERDES_COM_DIV_FRAC_START1,		0xd5);
	write_phy(dev->phy, QSERDES_COM_DIV_FRAC_START2,		0xaa);
	write_phy(dev->phy, QSERDES_COM_DIV_FRAC_START3,		0x13);
	write_phy(dev->phy, QSERDES_COM_PLLLOCK_CMP_EN,		0x01);
	write_phy(dev->phy, QSERDES_COM_PLLLOCK_CMP1,		0x2b);
	write_phy(dev->phy, QSERDES_COM_PLLLOCK_CMP2,		0x68);
	write_phy(dev->phy, QSERDES_COM_PLL_CRCTRL,			0xff);
	write_phy(dev->phy, QSERDES_COM_PLL_CP_SETI,		0x3f);
	write_phy(dev->phy, QSERDES_COM_PLL_IP_SETP,		0x07);
	write_phy(dev->phy, QSERDES_COM_PLL_CP_SETP,		0x03);
	write_phy(dev->phy, QSERDES_RX_CDR_CONTROL,			0xf3);
	write_phy(dev->phy, QSERDES_RX_CDR_CONTROL2,		0x6b);
	write_phy(dev->phy, QSERDES_COM_RESETSM_CNTRL,		0x10);
	write_phy(dev->phy, QSERDES_RX_RX_TERM_HIGHZ_CM_AC_COUPLE,	0x87);
	write_phy(dev->phy, QSERDES_RX_RX_EQ_GAIN12,		0x54);
	write_phy(dev->phy, PCIE_PHY_POWER_STATE_CONFIG1,		0xa3);
	write_phy(dev->phy, PCIE_PHY_POWER_STATE_CONFIG2,		0xcb);
	write_phy(dev->phy, QSERDES_COM_PLL_RXTXEPCLK_EN,		0x10);
	write_phy(dev->phy, PCIE_PHY_ENDPOINT_REFCLK_DRIVE,		0x10);
	write_phy(dev->phy, PCIE_PHY_SW_RESET,			0x00);
	write_phy(dev->phy, PCIE_PHY_START,				0x03);
}
#endif

#ifdef CONFIG_ARCH_MDM9630
void pcie_phy_init(struct msm_pcie_dev_t *dev)
{

	PCIE_DBG(dev, "RC%d: Initializing 28nm QMP phy - 19.2MHz\n",
		dev->rc_idx);

	write_phy(dev->phy, PCIE_PHY_POWER_DOWN_CONTROL, 0x03);

	write_phy(dev->phy, QSERDES_COM_SYSCLK_EN_SEL_TXBAND, 0x08);
	write_phy(dev->phy, QSERDES_COM_DEC_START1, 0x82);
	write_phy(dev->phy, QSERDES_COM_DEC_START2, 0x03);
	write_phy(dev->phy, QSERDES_COM_DIV_FRAC_START1, 0xD5);
	write_phy(dev->phy, QSERDES_COM_DIV_FRAC_START2, 0xAA);
	write_phy(dev->phy, QSERDES_COM_DIV_FRAC_START3, 0x4D);
	write_phy(dev->phy, QSERDES_COM_PLLLOCK_CMP_EN, 0x01);
	write_phy(dev->phy, QSERDES_COM_PLLLOCK_CMP1, 0x2B);
	write_phy(dev->phy, QSERDES_COM_PLLLOCK_CMP2, 0x68);
	write_phy(dev->phy, QSERDES_COM_PLL_CRCTRL, 0x7C);
	write_phy(dev->phy, QSERDES_COM_PLL_CP_SETI, 0x02);
	write_phy(dev->phy, QSERDES_COM_PLL_IP_SETP, 0x1F);
	write_phy(dev->phy, QSERDES_COM_PLL_CP_SETP, 0x0F);
	write_phy(dev->phy, QSERDES_COM_PLL_IP_SETI, 0x01);
	write_phy(dev->phy, QSERDES_COM_IE_TRIM, 0x0F);
	write_phy(dev->phy, QSERDES_COM_IP_TRIM, 0x0F);
	write_phy(dev->phy, QSERDES_COM_PLL_CNTRL, 0x46);

	/* CDR Settings */
	write_phy(dev->phy, QSERDES_RX_CDR_CONTROL1, 0xF3);
	write_phy(dev->phy, QSERDES_RX_CDR_CONTROL_HALF, 0x2B);

	write_phy(dev->phy, QSERDES_COM_PLL_VCOTAIL_EN, 0xE1);

	/* Calibration Settings */
	write_phy(dev->phy, QSERDES_COM_RESETSM_CNTRL, 0x90);
	write_phy(dev->phy, QSERDES_COM_RESETSM_CNTRL2, 0x7);

	/* Additional writes */
	write_phy(dev->phy, QSERDES_COM_RES_CODE_START_SEG1, 0x20);
	write_phy(dev->phy, QSERDES_COM_RES_CODE_CAL_CSR, 0x77);
	write_phy(dev->phy, QSERDES_COM_RES_TRIM_CONTROL, 0x15);
	write_phy(dev->phy, QSERDES_TX_RCV_DETECT_LVL, 0x03);
	write_phy(dev->phy, QSERDES_RX_RX_EQ_GAIN1_LSB, 0xFF);
	write_phy(dev->phy, QSERDES_RX_RX_EQ_GAIN1_MSB, 0x1F);
	write_phy(dev->phy, QSERDES_RX_RX_EQ_GAIN2_LSB, 0xFF);
	write_phy(dev->phy, QSERDES_RX_RX_EQ_GAIN2_MSB, 0x00);
	write_phy(dev->phy, QSERDES_RX_RX_EQU_ADAPTOR_CNTRL2, 0x1A);
	write_phy(dev->phy, QSERDES_RX_RX_EQ_OFFSET_ADAPTOR_CNTRL1, 0x80);
	write_phy(dev->phy, QSERDES_RX_SIGDET_ENABLES, 0x40);
	write_phy(dev->phy, QSERDES_RX_SIGDET_CNTRL, 0x70);
	write_phy(dev->phy, QSERDES_RX_SIGDET_DEGLITCH_CNTRL, 0x06);
	write_phy(dev->phy, QSERDES_COM_PLL_RXTXEPCLK_EN, 0x10);
	write_phy(dev->phy, PCIE_PHY_ENDPOINT_REFCLK_DRIVE, 0x10);
	write_phy(dev->phy, PCIE_PHY_POWER_STATE_CONFIG1, 0x23);
	write_phy(dev->phy, PCIE_PHY_POWER_STATE_CONFIG2, 0xCB);
	write_phy(dev->phy, QSERDES_RX_RX_RCVR_IQ_EN, 0x31);

	write_phy(dev->phy, PCIE_PHY_SW_RESET, 0x00);
	write_phy(dev->phy, PCIE_PHY_START, 0x03);
}
#elif defined(CONFIG_ARCH_FSM9900)
void pcie_phy_init(struct msm_pcie_dev_t *dev)
{
	if (dev->ext_ref_clk == false) {
		pcie20_phy_init_default(dev);
		return;
	}

	PCIE_DBG(dev, "RC%d: Initializing 28nm ATE phy - 100MHz\n",
		dev->rc_idx);

	/*  1 */
	write_phy(dev->phy, PCIE_PHY_POWER_DOWN_CONTROL, 0x01);
	/*  2 */
	write_phy(dev->phy, QSERDES_COM_SYS_CLK_CTRL, 0x3e);
	/*  3 */
	write_phy(dev->phy, QSERDES_COM_PLL_CP_SETI, 0x0f);
	/*  4 */
	write_phy(dev->phy, QSERDES_COM_PLL_IP_SETP, 0x23);
	/*  5 */
	write_phy(dev->phy, QSERDES_COM_PLL_IP_SETI, 0x3f);
	/*  6 */
	write_phy(dev->phy, QSERDES_RX_CDR_CONTROL, 0xf3);
	/*  7 */
	write_phy(dev->phy, QSERDES_RX_CDR_CONTROL2, 0x6b);
	/*  8 */
	write_phy(dev->phy, QSERDES_COM_RESETSM_CNTRL, 0x10);
	/*  9 */
	write_phy(dev->phy, QSERDES_RX_RX_TERM_HIGHZ_CM_AC_COUPLE, 0x87);
	/* 10 */
	write_phy(dev->phy, QSERDES_RX_RX_EQ_GAIN12, 0x54);
	/* 11 */
	write_phy(dev->phy, PCIE_PHY_POWER_STATE_CONFIG1, 0xa3);
	/* 12 */
	write_phy(dev->phy, PCIE_PHY_POWER_STATE_CONFIG2, 0x1b);
	/* 13 */
	write_phy(dev->phy, PCIE_PHY_SW_RESET,		0x00);
	/* 14 */
	write_phy(dev->phy, PCIE_PHY_START,		0x03);
}
#else
void pcie_phy_init(struct msm_pcie_dev_t *dev)
{
	if (dev->is_emulation)
		pcie20_uni_phy_init(dev);
	else
		pcie20_phy_init_default(dev);
}

#endif

bool pcie_phy_is_ready(struct msm_pcie_dev_t *dev)
{
	if (!dev->is_emulation &&
	    readl_relaxed(dev->phy + PCIE_PHY_PCS_STATUS) & BIT(6))
		return false;
	else
		return true;
}
/**
 * Write register
 *
 * @base - PHY base virtual address.
 * @offset - register offset.
 */
static u32 qca_uni_phy_read(void __iomem *base, u32 offset)
{
	u32 value;
	value = readl_relaxed(base + offset);
	return value;
}

/**
 * Write register
 * @base - PHY base virtual address.
 * @offset - register offset.
 * @val - value to write.
 */
static void qca_uni_phy_write(void __iomem *base, u32 offset, u32 val)
{
	writel(val, base + offset);
	udelay(100);
}

/**
 * Write register and read back masked value to confirm it is written
 *
 * @base - PHY base virtual address.
 * @offset - register offset.
 * @mask - register bitmask specifying what should be updated
 * @val - value to write.
 */
static void qca_uni_phy_write_readback(void __iomem *base, u32 offset,
		const u32 mask, u32 val)
{
	u32 write_val, tmp = readl(base + offset);

	tmp &= ~mask;       /* retain other bits */
	write_val = tmp | val;

	writel(write_val, base + offset);

	/* Read back to see if val was written */
	tmp = readl(base + offset);
	tmp &= mask;        /* clear other bits */

	if (tmp != val)
		pr_err("write: %x to UNI PHY: %x FAILED\n", val, offset);
}

static int mdio_wait(void __iomem *base)
{
	unsigned int mdio_access;
	unsigned int timeout = MDIO_TIMEOUT_STATIC;

	do {
		mdio_access = qca_uni_phy_read(base, MDIO_CTRL_4_REG);
		if (!timeout--)
			return -EFAULT;
	} while (mdio_access & MDIO_ACCESS_BUSY);

	return 0;
}

static int mdio_mii_read(void __iomem *base, unsigned char regAddr,
			 unsigned short *data)
{
	unsigned short mdio_ctl_0 = (MDIO_PCIE_PHY_ID | MDC_MODE |
					MDIO_CLAUSE_22 | MDIO_PCIE_CLK_DIV);
	unsigned int regVal;

	qca_uni_phy_write(base, MDIO_CTRL_0_REG, mdio_ctl_0);
	qca_uni_phy_write(base, MDIO_CTRL_1_REG, regAddr);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_22_READ);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_22_READ |
						 MDIO_ACCESS_START);

	/* wait for access busy to be cleared */
	if (mdio_wait(base)) {
		pr_err("%s MDIO Access Busy Timeout %x\n", __func__, regAddr);
		return -EFAULT;
	}

	regVal =  qca_uni_phy_read(base, MDIO_CTRL_3_REG);
	*data = (unsigned short)regVal;
	return 0;
}

static int mdio_mii_write(void __iomem *base, unsigned char regAddr,
			  unsigned short data)
{
	unsigned short mdio_ctl_0 = (MDIO_PCIE_PHY_ID | MDC_MODE |
					MDIO_CLAUSE_22 | MDIO_PCIE_CLK_DIV);

	qca_uni_phy_write(base, MDIO_CTRL_0_REG, mdio_ctl_0);
	qca_uni_phy_write(base, MDIO_CTRL_1_REG, regAddr);
	qca_uni_phy_write(base, MDIO_CTRL_2_REG, data);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_22_WRITE);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_22_WRITE |
						 MDIO_ACCESS_START);

	/* wait for access busy to be cleared */
	if (mdio_wait(base)) {
		pr_err("%s MDIO Access Busy Timeout %x\n", __func__, regAddr);
		return -EFAULT;
	}

	return 0;
}

static int mdio_mmd_read(void __iomem *base, unsigned short regAddr,
			 unsigned short *data)
{
	unsigned short mdio_ctl_0 = (MDIO_PCIE_PHY_ID | MDC_MODE |
					MDIO_CLAUSE_45 | MDIO_PCIE_CLK_DIV);
	unsigned int regVal;

	qca_uni_phy_write(base, MDIO_CTRL_0_REG, mdio_ctl_0);
	qca_uni_phy_write(base, MDIO_CTRL_1_REG, MDIO_MMD_ID);
	qca_uni_phy_write(base, MDIO_CTRL_2_REG, regAddr);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_READ_ADDR);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_READ_ADDR |
						 MDIO_ACCESS_START);

	/* wait for access busy to be cleared */
	if (mdio_wait(base)) {
		pr_err("%s MDIO Access Busy Timeout %x\n", __func__, regAddr);
		return -EFAULT;
	}

	qca_uni_phy_write(base, MDIO_CTRL_1_REG, MDIO_MMD_ID);
	qca_uni_phy_write(base, MDIO_CTRL_2_REG, regAddr);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_WRITE);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_WRITE |
						 MDIO_ACCESS_START);

	/* wait for access busy to be cleared */
	if (mdio_wait(base)) {
		pr_err("%s MDIO Access Busy Timeout %x\n", __func__, regAddr);
		return -EFAULT;
	}

	regVal =  qca_uni_phy_read(base, MDIO_CTRL_3_REG);
	*data = (unsigned short)regVal;

	return 0;
}

static int mdio_mmd_write(void __iomem *base, unsigned short regAddr,
			  unsigned short data)
{

	unsigned short mdio_ctl_0 = (MDIO_PCIE_PHY_ID | MDC_MODE |
					MDIO_CLAUSE_45 | MDIO_PCIE_CLK_DIV);

	qca_uni_phy_write(base, MDIO_CTRL_0_REG, mdio_ctl_0);
	qca_uni_phy_write(base, MDIO_CTRL_1_REG, MDIO_MMD_ID);
	qca_uni_phy_write(base, MDIO_CTRL_2_REG, regAddr);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_READ_ADDR);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_READ_ADDR |
						 MDIO_ACCESS_START);

	/* wait for access busy to be cleared */
	if (mdio_wait(base)) {
		pr_err("%s MDIO Access Busy Timeout %x\n", __func__, regAddr);
		return -EFAULT;
	}

	qca_uni_phy_write(base, MDIO_CTRL_2_REG, data);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_READ);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_READ |
						 MDIO_ACCESS_START);

	qca_uni_phy_write(base, MDIO_CTRL_2_REG, regAddr);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_WRITE);
	qca_uni_phy_write(base, MDIO_CTRL_4_REG, MDIO_ACCESS_45_WRITE |
						 MDIO_ACCESS_START);

	/* wait for access busy to be cleared */
	if (mdio_wait(base)) {
		pr_err("%s MDIO Access Busy Timeout %x\n", __func__, regAddr);
		return -EFAULT;
	}

	return 0;
}

void pcie20_uni_phy_init(struct msm_pcie_dev_t *dev)
{
	unsigned short data;

	mdio_mii_write(dev->phy, 0x1, 0x801c);
	mdio_mii_write(dev->phy, 0xb, 0x300d);

	mdio_mmd_write(dev->phy, 0x2d, 0x681a);
	mdio_mmd_write(dev->phy, 0x7d, 0x8);
	mdio_mmd_write(dev->phy, 0x7f, 0x5ed5);
	mdio_mmd_write(dev->phy, 0x87, 0xaa0a);
	mdio_mmd_write(dev->phy, 0x4, 0x0802);
	mdio_mmd_write(dev->phy, 0x8, 0x0280);
	mdio_mmd_write(dev->phy, 0x9, 0x8854);
	mdio_mmd_write(dev->phy, 0xa, 0x2815);
	mdio_mmd_write(dev->phy, 0xb, 0x0120);
	mdio_mmd_write(dev->phy, 0xc, 0x0480);
	mdio_mmd_write(dev->phy, 0x13, 0x8000);

	mdio_mmd_read(dev->phy, 0x7e, &data);

	mdio_mii_read(dev->phy, 0x7, &data);
}

bool pcie_phy_detect(struct msm_pcie_dev_t *dev)
{
	unsigned short data;

	mdio_mii_read(dev->phy, 0x0, &data);

	if (data == 0x7f) {
		pr_info("PCIe UNI PHY detected\n");
		return true;
	} else {
		return false;
	}
}
