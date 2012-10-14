/*
 *  Atheros AR71xx SoC platform devices
 *
 *  Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros 2.6.15 BSP
 *  Parts of this file are based on Atheros 2.6.31 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/etherdevice.h>
#include <linux/platform_device.h>
#include <linux/serial_8250.h>
#include <linux/phy.h>
#include <linux/rtl8366.h>

#include <asm/mach-ar71xx/ar71xx.h>
#include <asm/mach-ar71xx/platform.h>

static struct resource ar71xx_mdio0_resources[] = {
	{
		.name	= "mdio_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE0_BASE,
		.end	= AR71XX_GE0_BASE + 0x200 - 1,
	}
};

static struct ag71xx_mdio_platform_data ar71xx_mdio0_data;

struct platform_device ar71xx_mdio0_device = {
	.name		= "ag71xx-mdio",
	.id		= 0,
	.resource	= ar71xx_mdio0_resources,
	.num_resources	= ARRAY_SIZE(ar71xx_mdio0_resources),
	.dev = {
		.platform_data = &ar71xx_mdio0_data,
	},
};

static struct resource ar71xx_mdio1_resources[] = {
	{
		.name	= "mdio_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE1_BASE,
		.end	= AR71XX_GE1_BASE + 0x200 - 1,
	}
};

static struct ag71xx_mdio_platform_data ar71xx_mdio1_data;

struct platform_device ar71xx_mdio1_device = {
	.name		= "ag71xx-mdio",
	.id		= 1,
	.resource	= ar71xx_mdio1_resources,
	.num_resources	= ARRAY_SIZE(ar71xx_mdio1_resources),
	.dev = {
		.platform_data = &ar71xx_mdio1_data,
	},
};

static void ar71xx_set_pll(u32 cfg_reg, u32 pll_reg, u32 pll_val, u32 shift)
{
	void __iomem *base;
	u32 t;

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);

	t = __raw_readl(base + cfg_reg);
	t &= ~(3 << shift);
	t |=  (2 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	__raw_writel(pll_val, base + pll_reg);

	t |= (3 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	t &= ~(3 << shift);
	__raw_writel(t, base + cfg_reg);
	udelay(100);

	printk(KERN_DEBUG "ar71xx: pll_reg %#x: %#x\n",
		(unsigned int)(base + pll_reg), __raw_readl(base + pll_reg));

	iounmap(base);
}

static void __init ar71xx_mii_ctrl_set_if(unsigned int reg,
					  unsigned int mii_if)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR71XX_MII_BASE, AR71XX_MII_SIZE);

	t = __raw_readl(base + reg);
	t &= ~(MII_CTRL_IF_MASK);
	t |= (mii_if & MII_CTRL_IF_MASK);
	__raw_writel(t, base + reg);

	iounmap(base);
}

static void ar71xx_mii_ctrl_set_speed(unsigned int reg, unsigned int speed)
{
	void __iomem *base;
	unsigned int mii_speed;
	u32 t;

	switch (speed) {
	case SPEED_10:
		mii_speed =  MII_CTRL_SPEED_10;
		break;
	case SPEED_100:
		mii_speed =  MII_CTRL_SPEED_100;
		break;
	case SPEED_1000:
		mii_speed =  MII_CTRL_SPEED_1000;
		break;
	default:
		BUG();
		return;
	}

	base = ioremap(AR71XX_MII_BASE, AR71XX_MII_SIZE);

	t = __raw_readl(base + reg);
	t &= ~(MII_CTRL_SPEED_MASK << MII_CTRL_SPEED_SHIFT);
	t |= mii_speed  << MII_CTRL_SPEED_SHIFT;
	__raw_writel(t, base + reg);

	iounmap(base);
}

void __init ar71xx_add_device_mdio(unsigned int id, u32 phy_mask)
{
	struct platform_device *mdio_dev;
	struct ag71xx_mdio_platform_data *mdio_data;
	unsigned int max_id;

	if (ar71xx_soc == AR71XX_SOC_AR9341 ||
	    ar71xx_soc == AR71XX_SOC_AR9342 ||
	    ar71xx_soc == AR71XX_SOC_AR9344)
		max_id = 1;
	else
		max_id = 0;

	if (id > max_id) {
		printk(KERN_ERR "ar71xx: invalid MDIO id %u\n", id);
		return;
	}

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7241:
	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		mdio_dev = &ar71xx_mdio1_device;
		mdio_data = &ar71xx_mdio1_data;
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		if (id == 0) {
			mdio_dev = &ar71xx_mdio0_device;
			mdio_data = &ar71xx_mdio0_data;
		} else {
			mdio_dev = &ar71xx_mdio1_device;
			mdio_data = &ar71xx_mdio1_data;
		}
		break;

	case AR71XX_SOC_AR7242:
		ar71xx_set_pll(AR71XX_PLL_REG_SEC_CONFIG,
			       AR7242_PLL_REG_ETH0_INT_CLOCK, 0x62000000,
			       AR71XX_ETH0_PLL_SHIFT);
		/* fall through */
	default:
		mdio_dev = &ar71xx_mdio0_device;
		mdio_data = &ar71xx_mdio0_data;
		break;
	}

	mdio_data->phy_mask = phy_mask;

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7240:
		mdio_data->is_ar7240 = 1;
		/* fall through */
	case AR71XX_SOC_AR7241:
		mdio_data->builtin_switch = 1;
		break;
	case AR71XX_SOC_AR9330:
		mdio_data->is_ar9330 = 1;
		/* fall through */
	case AR71XX_SOC_AR9331:
		mdio_data->builtin_switch = 1;
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		if (id == 1)
			mdio_data->builtin_switch = 1;
		mdio_data->is_ar934x = 1;
		break;

	default:
		break;
	}

	platform_device_register(mdio_dev);
}

struct ar71xx_eth_pll_data {
    u32 pll_10;
    u32 pll_100;
    u32 pll_1000;
};

static struct ar71xx_eth_pll_data ar71xx_eth0_pll_data;
static struct ar71xx_eth_pll_data ar71xx_eth1_pll_data;

static u32 ar71xx_get_eth_pll(unsigned int mac, int speed)
{
	struct ar71xx_eth_pll_data *pll_data;
	u32 pll_val;

	switch (mac) {
	case 0:
		pll_data = &ar71xx_eth0_pll_data;
		break;
	case 1:
		pll_data = &ar71xx_eth1_pll_data;
		break;
	default:
		BUG();
		return 0;
	}

	switch (speed) {
	case SPEED_10:
		pll_val = pll_data->pll_10;
		break;
	case SPEED_100:
		pll_val = pll_data->pll_100;
		break;
	case SPEED_1000:
		pll_val = pll_data->pll_1000;
		break;
	default:
		BUG();
		return 0;
	}

	return pll_val;
}

static void ar71xx_set_speed_ge0(int speed)
{
	u32 val = ar71xx_get_eth_pll(0, speed);

	ar71xx_set_pll(AR71XX_PLL_REG_SEC_CONFIG, AR71XX_PLL_REG_ETH0_INT_CLOCK,
			val, AR71XX_ETH0_PLL_SHIFT);
	ar71xx_mii_ctrl_set_speed(MII_REG_MII0_CTRL, speed);
}

static void ar71xx_set_speed_ge1(int speed)
{
	u32 val = ar71xx_get_eth_pll(1, speed);

	ar71xx_set_pll(AR71XX_PLL_REG_SEC_CONFIG, AR71XX_PLL_REG_ETH1_INT_CLOCK,
			 val, AR71XX_ETH1_PLL_SHIFT);
	ar71xx_mii_ctrl_set_speed(MII_REG_MII1_CTRL, speed);
}

static void ar724x_set_speed_ge0(int speed)
{
	/* TODO */
}

static void ar724x_set_speed_ge1(int speed)
{
	/* TODO */
}

static void ar7242_set_speed_ge0(int speed)
{
	u32 val = ar71xx_get_eth_pll(0, speed);
	void __iomem *base;

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + AR7242_PLL_REG_ETH0_INT_CLOCK);
	iounmap(base);
}

static void ar91xx_set_speed_ge0(int speed)
{
	u32 val = ar71xx_get_eth_pll(0, speed);

	ar71xx_set_pll(AR91XX_PLL_REG_ETH_CONFIG, AR91XX_PLL_REG_ETH0_INT_CLOCK,
			 val, AR91XX_ETH0_PLL_SHIFT);
	ar71xx_mii_ctrl_set_speed(MII_REG_MII0_CTRL, speed);
}

static void ar91xx_set_speed_ge1(int speed)
{
	u32 val = ar71xx_get_eth_pll(1, speed);

	ar71xx_set_pll(AR91XX_PLL_REG_ETH_CONFIG, AR91XX_PLL_REG_ETH1_INT_CLOCK,
			 val, AR91XX_ETH1_PLL_SHIFT);
	ar71xx_mii_ctrl_set_speed(MII_REG_MII1_CTRL, speed);
}

static void ar933x_set_speed_ge0(int speed)
{
	/* TODO */
}

static void ar933x_set_speed_ge1(int speed)
{
	/* TODO */
}

static void ar934x_set_speed_ge0(int speed)
{
	void __iomem *base;
	u32 val = ar71xx_get_eth_pll(0, speed);

	base = ioremap_nocache(AR71XX_PLL_BASE, AR71XX_PLL_SIZE);
	__raw_writel(val, base + AR934X_PLL_ETH_XMII_CONTROL_REG);
	iounmap(base);
}

static void ar934x_set_speed_ge1(int speed)
{
	/* TODO */
}

static void ar71xx_ddr_flush_ge0(void)
{
	ar71xx_ddr_flush(AR71XX_DDR_REG_FLUSH_GE0);
}

static void ar71xx_ddr_flush_ge1(void)
{
	ar71xx_ddr_flush(AR71XX_DDR_REG_FLUSH_GE1);
}

static void ar724x_ddr_flush_ge0(void)
{
	ar71xx_ddr_flush(AR724X_DDR_REG_FLUSH_GE0);
}

static void ar724x_ddr_flush_ge1(void)
{
	ar71xx_ddr_flush(AR724X_DDR_REG_FLUSH_GE1);
}

static void ar91xx_ddr_flush_ge0(void)
{
	ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_GE0);
}

static void ar91xx_ddr_flush_ge1(void)
{
	ar71xx_ddr_flush(AR91XX_DDR_REG_FLUSH_GE1);
}

static void ar933x_ddr_flush_ge0(void)
{
	ar71xx_ddr_flush(AR933X_DDR_REG_FLUSH_GE0);
}

static void ar933x_ddr_flush_ge1(void)
{
	ar71xx_ddr_flush(AR933X_DDR_REG_FLUSH_GE1);
}

static void ar934x_ddr_flush_ge0(void)
{
	ar71xx_ddr_flush(AR934X_DDR_REG_FLUSH_GE0);
}

static void ar934x_ddr_flush_ge1(void)
{
	ar71xx_ddr_flush(AR934X_DDR_REG_FLUSH_GE1);
}

static struct resource ar71xx_eth0_resources[] = {
	{
		.name	= "mac_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE0_BASE,
		.end	= AR71XX_GE0_BASE + 0x200 - 1,
	}, {
		.name	= "mac_irq",
		.flags	= IORESOURCE_IRQ,
		.start	= AR71XX_CPU_IRQ_GE0,
		.end	= AR71XX_CPU_IRQ_GE0,
	},
};

struct ag71xx_platform_data ar71xx_eth0_data = {
	.reset_bit	= RESET_MODULE_GE0_MAC,
	.mac_addr = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 },
};

struct platform_device ar71xx_eth0_device = {
	.name		= "ag71xx",
	.id		= 0,
	.resource	= ar71xx_eth0_resources,
	.num_resources	= ARRAY_SIZE(ar71xx_eth0_resources),
	.dev = {
		.platform_data = &ar71xx_eth0_data,
	},
};

static struct resource ar71xx_eth1_resources[] = {
	{
		.name	= "mac_base",
		.flags	= IORESOURCE_MEM,
		.start	= AR71XX_GE1_BASE,
		.end	= AR71XX_GE1_BASE + 0x200 - 1,
	}, {
		.name	= "mac_irq",
		.flags	= IORESOURCE_IRQ,
		.start	= AR71XX_CPU_IRQ_GE1,
		.end	= AR71XX_CPU_IRQ_GE1,
	},
};

struct ag71xx_platform_data ar71xx_eth1_data = {
	.reset_bit	= RESET_MODULE_GE1_MAC,
	.mac_addr = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 },
};

struct platform_device ar71xx_eth1_device = {
	.name		= "ag71xx",
	.id		= 1,
	.resource	= ar71xx_eth1_resources,
	.num_resources	= ARRAY_SIZE(ar71xx_eth1_resources),
	.dev = {
		.platform_data = &ar71xx_eth1_data,
	},
};

struct ag71xx_switch_platform_data ar71xx_switch_data;

#define AR71XX_PLL_VAL_1000	0x00110000
#define AR71XX_PLL_VAL_100	0x00001099
#define AR71XX_PLL_VAL_10	0x00991099

#define AR724X_PLL_VAL_1000	0x00110000
#define AR724X_PLL_VAL_100	0x00001099
#define AR724X_PLL_VAL_10	0x00991099

#define AR7242_PLL_VAL_1000	0x16000000
#define AR7242_PLL_VAL_100	0x00000101
#define AR7242_PLL_VAL_10	0x00001616

#define AR91XX_PLL_VAL_1000	0x1a000000
#define AR91XX_PLL_VAL_100	0x13000a44
#define AR91XX_PLL_VAL_10	0x00441099

#define AR933X_PLL_VAL_1000	0x00110000
#define AR933X_PLL_VAL_100	0x00001099
#define AR933X_PLL_VAL_10	0x00991099

#define AR934X_PLL_VAL_1000	0x16000000
#define AR934X_PLL_VAL_100	0x00000101
#define AR934X_PLL_VAL_10	0x00001616

static void __init ar71xx_init_eth_pll_data(unsigned int id)
{
	struct ar71xx_eth_pll_data *pll_data;
	u32 pll_10, pll_100, pll_1000;

	switch (id) {
	case 0:
		pll_data = &ar71xx_eth0_pll_data;
		break;
	case 1:
		pll_data = &ar71xx_eth1_pll_data;
		break;
	default:
		BUG();
	}

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7130:
	case AR71XX_SOC_AR7141:
	case AR71XX_SOC_AR7161:
		pll_10 = AR71XX_PLL_VAL_10;
		pll_100 = AR71XX_PLL_VAL_100;
		pll_1000 = AR71XX_PLL_VAL_1000;
		break;

	case AR71XX_SOC_AR7240:
	case AR71XX_SOC_AR7241:
		pll_10 = AR724X_PLL_VAL_10;
		pll_100 = AR724X_PLL_VAL_100;
		pll_1000 = AR724X_PLL_VAL_1000;
		break;

	case AR71XX_SOC_AR7242:
		pll_10 = AR7242_PLL_VAL_10;
		pll_100 = AR7242_PLL_VAL_100;
		pll_1000 = AR7242_PLL_VAL_1000;
		break;

	case AR71XX_SOC_AR9130:
	case AR71XX_SOC_AR9132:
		pll_10 = AR91XX_PLL_VAL_10;
		pll_100 = AR91XX_PLL_VAL_100;
		pll_1000 = AR91XX_PLL_VAL_1000;
		break;

	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		pll_10 = AR933X_PLL_VAL_10;
		pll_100 = AR933X_PLL_VAL_100;
		pll_1000 = AR933X_PLL_VAL_1000;
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		pll_10 = AR934X_PLL_VAL_10;
		pll_100 = AR934X_PLL_VAL_100;
		pll_1000 = AR934X_PLL_VAL_1000;
		break;

	default:
		BUG();
		return;
	}

	if (!pll_data->pll_10)
		pll_data->pll_10 = pll_10;

	if (!pll_data->pll_100)
		pll_data->pll_100 = pll_100;

	if (!pll_data->pll_1000)
		pll_data->pll_1000 = pll_1000;
}

static int __init ar71xx_setup_phy_if_mode(unsigned int id,
					   struct ag71xx_platform_data *pdata)
{
	unsigned int mii_if;

	switch (id) {
	case 0:
		switch (ar71xx_soc) {
		case AR71XX_SOC_AR7130:
		case AR71XX_SOC_AR7141:
		case AR71XX_SOC_AR7161:
		case AR71XX_SOC_AR9130:
		case AR71XX_SOC_AR9132:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
				mii_if = MII0_CTRL_IF_MII;
				break;
			case PHY_INTERFACE_MODE_GMII:
				mii_if = MII0_CTRL_IF_GMII;
				break;
			case PHY_INTERFACE_MODE_RGMII:
				mii_if = MII0_CTRL_IF_RGMII;
				break;
			case PHY_INTERFACE_MODE_RMII:
				mii_if = MII0_CTRL_IF_RMII;
				break;
			default:
				return -EINVAL;
			}
			ar71xx_mii_ctrl_set_if(MII_REG_MII0_CTRL, mii_if);
			break;

		case AR71XX_SOC_AR7240:
		case AR71XX_SOC_AR7241:
		case AR71XX_SOC_AR9330:
		case AR71XX_SOC_AR9331:
			pdata->phy_if_mode = PHY_INTERFACE_MODE_MII;
			break;

		case AR71XX_SOC_AR7242:
			/* FIXME */

		case AR71XX_SOC_AR9341:
		case AR71XX_SOC_AR9342:
		case AR71XX_SOC_AR9344:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
			case PHY_INTERFACE_MODE_RGMII:
			case PHY_INTERFACE_MODE_RMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		default:
			BUG();
		}
		break;
	case 1:
		switch (ar71xx_soc) {
		case AR71XX_SOC_AR7130:
		case AR71XX_SOC_AR7141:
		case AR71XX_SOC_AR7161:
		case AR71XX_SOC_AR9130:
		case AR71XX_SOC_AR9132:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_RMII:
				mii_if = MII1_CTRL_IF_RMII;
				break;
			case PHY_INTERFACE_MODE_RGMII:
				mii_if = MII1_CTRL_IF_RGMII;
				break;
			default:
				return -EINVAL;
			}
			ar71xx_mii_ctrl_set_if(MII_REG_MII1_CTRL, mii_if);
			break;

		case AR71XX_SOC_AR7240:
		case AR71XX_SOC_AR7241:
		case AR71XX_SOC_AR9330:
		case AR71XX_SOC_AR9331:
			pdata->phy_if_mode = PHY_INTERFACE_MODE_GMII;
			break;

		case AR71XX_SOC_AR7242:
			/* FIXME */

		case AR71XX_SOC_AR9341:
		case AR71XX_SOC_AR9342:
		case AR71XX_SOC_AR9344:
			switch (pdata->phy_if_mode) {
			case PHY_INTERFACE_MODE_MII:
			case PHY_INTERFACE_MODE_GMII:
				break;
			default:
				return -EINVAL;
			}
			break;

		default:
			BUG();
		}
		break;
	}

	return 0;
}

static int ar71xx_eth_instance __initdata;
void __init ar71xx_add_device_eth(unsigned int id)
{
	struct platform_device *pdev;
	struct ag71xx_platform_data *pdata;
	int err;

	if (id > 1) {
		printk(KERN_ERR "ar71xx: invalid ethernet id %d\n", id);
		return;
	}

	ar71xx_init_eth_pll_data(id);

	if (id == 0)
		pdev = &ar71xx_eth0_device;
	else
		pdev = &ar71xx_eth1_device;

	pdata = pdev->dev.platform_data;

	err = ar71xx_setup_phy_if_mode(id, pdata);
	if (err) {
		printk(KERN_ERR
		       "ar71xx: invalid PHY interface mode for GE%u\n", id);
		return;
	}

	switch (ar71xx_soc) {
	case AR71XX_SOC_AR7130:
		if (id == 0) {
			pdata->ddr_flush = ar71xx_ddr_flush_ge0;
			pdata->set_speed = ar71xx_set_speed_ge0;
		} else {
			pdata->ddr_flush = ar71xx_ddr_flush_ge1;
			pdata->set_speed = ar71xx_set_speed_ge1;
		}
		break;

	case AR71XX_SOC_AR7141:
	case AR71XX_SOC_AR7161:
		if (id == 0) {
			pdata->ddr_flush = ar71xx_ddr_flush_ge0;
			pdata->set_speed = ar71xx_set_speed_ge0;
		} else {
			pdata->ddr_flush = ar71xx_ddr_flush_ge1;
			pdata->set_speed = ar71xx_set_speed_ge1;
		}
		pdata->has_gbit = 1;
		break;

	case AR71XX_SOC_AR7242:
		if (id == 0) {
			pdata->reset_bit |= AR724X_RESET_GE0_MDIO |
					    RESET_MODULE_GE0_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge0;
			pdata->set_speed = ar7242_set_speed_ge0;
		} else {
			pdata->reset_bit |= AR724X_RESET_GE1_MDIO |
					    RESET_MODULE_GE1_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge1;
			pdata->set_speed = ar724x_set_speed_ge1;
		}
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	case AR71XX_SOC_AR7241:
		if (id == 0)
			pdata->reset_bit |= AR724X_RESET_GE0_MDIO;
		else
			pdata->reset_bit |= AR724X_RESET_GE1_MDIO;
		/* fall through */
	case AR71XX_SOC_AR7240:
		if (id == 0) {
			pdata->reset_bit |= RESET_MODULE_GE0_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge0;
			pdata->set_speed = ar724x_set_speed_ge0;

			pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit |= RESET_MODULE_GE1_PHY;
			pdata->ddr_flush = ar724x_ddr_flush_ge1;
			pdata->set_speed = ar724x_set_speed_ge1;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ar71xx_switch_data;
			ar71xx_switch_data.phy_poll_mask |= BIT(4);
		}
		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;
		if (ar71xx_soc == AR71XX_SOC_AR7240)
			pdata->is_ar7240 = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	case AR71XX_SOC_AR9130:
		if (id == 0) {
			pdata->ddr_flush = ar91xx_ddr_flush_ge0;
			pdata->set_speed = ar91xx_set_speed_ge0;
		} else {
			pdata->ddr_flush = ar91xx_ddr_flush_ge1;
			pdata->set_speed = ar91xx_set_speed_ge1;
		}
		pdata->is_ar91xx = 1;
		break;

	case AR71XX_SOC_AR9132:
		if (id == 0) {
			pdata->ddr_flush = ar91xx_ddr_flush_ge0;
			pdata->set_speed = ar91xx_set_speed_ge0;
		} else {
			pdata->ddr_flush = ar91xx_ddr_flush_ge1;
			pdata->set_speed = ar91xx_set_speed_ge1;
		}
		pdata->is_ar91xx = 1;
		pdata->has_gbit = 1;
		break;

	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		if (id == 0) {
			pdata->reset_bit = AR933X_RESET_GE0_MAC |
					   AR933X_RESET_GE0_MDIO;
			pdata->ddr_flush = ar933x_ddr_flush_ge0;
			pdata->set_speed = ar933x_set_speed_ge0;

			pdata->phy_mask = BIT(4);
		} else {
			pdata->reset_bit = AR933X_RESET_GE1_MAC |
					   AR933X_RESET_GE1_MDIO;
			pdata->ddr_flush = ar933x_ddr_flush_ge1;
			pdata->set_speed = ar933x_set_speed_ge1;

			pdata->speed = SPEED_1000;
			pdata->duplex = DUPLEX_FULL;
			pdata->switch_data = &ar71xx_switch_data;
			ar71xx_switch_data.phy_poll_mask |= BIT(4);
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		if (id == 0) {
			pdata->reset_bit = AR934X_RESET_GE0_MAC |
					   AR934X_RESET_GE0_MDIO;
			pdata->ddr_flush =ar934x_ddr_flush_ge0;
			pdata->set_speed = ar934x_set_speed_ge0;
		} else {
			pdata->reset_bit = AR934X_RESET_GE1_MAC |
					   AR934X_RESET_GE1_MDIO;
			pdata->ddr_flush = ar934x_ddr_flush_ge1;
			pdata->set_speed = ar934x_set_speed_ge1;

			pdata->switch_data = &ar71xx_switch_data;
		}

		pdata->has_gbit = 1;
		pdata->is_ar724x = 1;

		if (!pdata->fifo_cfg1)
			pdata->fifo_cfg1 = 0x0010ffff;
		if (!pdata->fifo_cfg2)
			pdata->fifo_cfg2 = 0x015500aa;
		if (!pdata->fifo_cfg3)
			pdata->fifo_cfg3 = 0x01f00140;
		break;

	default:
		BUG();
	}

	switch (pdata->phy_if_mode) {
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RGMII:
		if (!pdata->has_gbit) {
			printk(KERN_ERR "ar71xx: no gbit available on eth%d\n",
					id);
			return;
		}
		/* fallthrough */
	default:
		break;
	}

	if (!is_valid_ether_addr(pdata->mac_addr)) {
		eth_random_addr(pdata->mac_addr);
		printk(KERN_DEBUG
			"ar71xx: using random MAC address for eth%d\n",
			ar71xx_eth_instance);
	}

	if (pdata->mii_bus_dev == NULL) {
		switch (ar71xx_soc) {
		case AR71XX_SOC_AR9341:
		case AR71XX_SOC_AR9342:
		case AR71XX_SOC_AR9344:
			if (id == 0)
				pdata->mii_bus_dev = &ar71xx_mdio0_device.dev;
			else
				pdata->mii_bus_dev = &ar71xx_mdio1_device.dev;
			break;

		case AR71XX_SOC_AR7241:
		case AR71XX_SOC_AR9330:
		case AR71XX_SOC_AR9331:
			pdata->mii_bus_dev = &ar71xx_mdio1_device.dev;
			break;

		default:
			pdata->mii_bus_dev = &ar71xx_mdio0_device.dev;
			break;
		}
	}

	/* Reset the device */
	ar71xx_device_stop(pdata->reset_bit);
	mdelay(100);

	ar71xx_device_start(pdata->reset_bit);
	mdelay(100);

	platform_device_register(pdev);
	ar71xx_eth_instance++;
}

static enum ar71xx_soc_type ar71xx_get_soc_type(void)
{
	u32 id;
	u32 major;
	u32 minor;
	u32 rev = 0;

	id = ar71xx_reset_rr(AR71XX_RESET_REG_REV_ID);
	major = id & REV_ID_MAJOR_MASK;

	switch (major) {
	case REV_ID_MAJOR_AR71XX:
		minor = id & AR71XX_REV_ID_MINOR_MASK;
		rev = id >> AR71XX_REV_ID_REVISION_SHIFT;
		rev &= AR71XX_REV_ID_REVISION_MASK;
		switch (minor) {
		case AR71XX_REV_ID_MINOR_AR7130:
			return AR71XX_SOC_AR7130;

		case AR71XX_REV_ID_MINOR_AR7141:
			return AR71XX_SOC_AR7141;

		case AR71XX_REV_ID_MINOR_AR7161:
			return AR71XX_SOC_AR7161;
		}
		break;

	case REV_ID_MAJOR_AR7240:
		return AR71XX_SOC_AR7240;

	case REV_ID_MAJOR_AR7241:
		return AR71XX_SOC_AR7241;

	case REV_ID_MAJOR_AR7242:
		return AR71XX_SOC_AR7242;

	case REV_ID_MAJOR_AR913X:
		minor = id & AR91XX_REV_ID_MINOR_MASK;
		rev = id >> AR91XX_REV_ID_REVISION_SHIFT;
		rev &= AR91XX_REV_ID_REVISION_MASK;
		switch (minor) {
		case AR91XX_REV_ID_MINOR_AR9130:
			return AR71XX_SOC_AR9130;

		case AR91XX_REV_ID_MINOR_AR9132:
			return AR71XX_SOC_AR9132;
		}
		break;

	case REV_ID_MAJOR_AR9330:
		return AR71XX_SOC_AR9330;

	case REV_ID_MAJOR_AR9331:
		return AR71XX_SOC_AR9331;

	case REV_ID_MAJOR_AR9341:
		return AR71XX_SOC_AR9341;

	case REV_ID_MAJOR_AR9342:
		return AR71XX_SOC_AR9342;

	case REV_ID_MAJOR_AR9344:
		return AR71XX_SOC_AR9344;

	default:
		panic("ar71xx: unknown chip id:0x%08x\n", id);
	}

	return 0;
}


#ifdef CONFIG_WDR2543
#include <linux/rtl8367.h>

#define TL_WR2543N_GPIO_RTL8367_SDA	1
#define TL_WR2543N_GPIO_RTL8367_SCK	6

static struct rtl8367_extif_config tl_wr2543n_rtl8367_extif0_cfg = {
	.mode = RTL8367_EXTIF_MODE_RGMII,
	.txdelay = 1,
	.rxdelay = 0,
	.ability = {
		.force_mode = 1,
		.txpause = 1,
		.rxpause = 1,
		.link = 1,
		.duplex = 1,
		.speed = RTL8367_PORT_SPEED_1000,
	},
};

static struct rtl8367_platform_data tl_wr2543n_rtl8367_data = {
	.gpio_sda	= TL_WR2543N_GPIO_RTL8367_SDA,
	.gpio_sck	= TL_WR2543N_GPIO_RTL8367_SCK,
	.extif0_cfg	= &tl_wr2543n_rtl8367_extif0_cfg,
};

static struct platform_device tl_wr2543n_rtl8367_device = {
	.name		= RTL8367_DRIVER_NAME,
	.id		= -1,
	.dev = {
		.platform_data	= &tl_wr2543n_rtl8367_data,
	}
};


#endif


#if (defined(CONFIG_RTL8366_SMI) || defined(CONFIG_RTL8366_SMI_MODULE)) && !defined(CONFIG_WDR2543)

#ifdef CONFIG_TPLINK
/* TL-WR1043ND */
#define GPIO_RTL8366_SDA 18//gpio 19
#define GPIO_RTL8366_SCK 19//gpio 20
#elif CONFIG_BUFFALO
/* WZR-HP-G300NH */

#define GPIO_RTL8366_SDA 19//gpio 19
#define GPIO_RTL8366_SCK 20//gpio 20
#else
/* DLINK-DIR825 or WNDR3700*/
#define GPIO_RTL8366_SDA 5//gpio 19
#define GPIO_RTL8366_SCK 7//gpio 20
#endif

#ifdef CONFIG_DIR825
static struct rtl8366_initval dir825b1_rtl8366s_initvals[] = {
	{ .reg = 0x06, .val = 0x0108 },
};
#endif

static void tl_wr1043nd_rtl8366rb_hw_reset(bool active)
{
	if (active)
		ar71xx_device_stop(AR71XX_RESET_GE0_PHY);
	else
		ar71xx_device_start(AR71XX_RESET_GE0_PHY);
}


static struct rtl8366_platform_data rtl8366_dev_data = {
	.gpio_sda        = GPIO_RTL8366_SDA,
	.gpio_sck        = GPIO_RTL8366_SCK,
#ifdef CONFIG_TPLINK
	.hw_reset	= tl_wr1043nd_rtl8366rb_hw_reset,
#endif
#ifdef CONFIG_DIR825
	.num_initvals	= ARRAY_SIZE(dir825b1_rtl8366s_initvals),
	.initvals	= dir825b1_rtl8366s_initvals,
#endif
};

static struct platform_device rtl8366_device = {
	.name       = RTL8366RB_DRIVER_NAME,
	.id     = -1,
	.dev = {
		.platform_data  = &rtl8366_dev_data,
	}
};


static void phy_dev_init(void)
{
#if defined (CONFIG_BUFFALO) || defined(CONFIG_TPLINK) || defined(CONFIG_DIR825) || defined(CONFIG_WNDR3700)
	if (rtl8366_smi_detect(&rtl8366_dev_data) == RTL8366_TYPE_RB) {
#if defined (CONFIG_TPLINK)
		ar71xx_eth0_pll_data.pll_1000 = 0x1a000000;
#else
		ar71xx_eth0_pll_data.pll_1000 = 0x1f000000;
#endif
		ar71xx_eth1_pll_data.pll_1000 = 0x100;
	} else {
		rtl8366_device.name = RTL8366S_DRIVER_NAME;
		ar71xx_eth0_pll_data.pll_1000 = 0x1e000100;
		ar71xx_eth1_pll_data.pll_1000 = 0x1e000100;
	}
#endif
#if defined(CONFIG_DIR825) || defined(CONFIG_WNDR3700)
	ar71xx_eth0_pll_data.pll_1000 = 0x11110000;
	ar71xx_eth1_pll_data.pll_1000 = 0x11110000;
#endif
	ar71xx_eth0_data.mii_bus_dev = &rtl8366_device.dev;
	ar71xx_eth1_data.mii_bus_dev = &rtl8366_device.dev;
	ar71xx_eth1_data.phy_mask = 0x10;

	platform_device_register(&rtl8366_device);
}

#else
#ifdef CONFIG_MTD_AR7100_SPI_FLASH
extern unsigned int compex;
#endif
static inline void phy_dev_init(void)
{
#if defined(CONFIG_WDR2543)
	ar71xx_eth0_data.mii_bus_dev = &tl_wr2543n_rtl8367_device.dev;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_pll_data.pll_1000 = 0x1a000000;
	platform_device_register(&tl_wr2543n_rtl8367_device);
#elif CONFIG_ATHRS26_PHY
	ar71xx_eth1_data.phy_mask = BIT(4);
	ar71xx_add_device_mdio(0, 0x0);
#elif CONFIG_WA901
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = 0x00001000;
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_eth0_data.reset_bit = RESET_MODULE_GE0_MAC |
				     RESET_MODULE_GE0_PHY;
#elif CONFIG_WZRG450
	ar71xx_add_device_mdio(0, ~BIT(0));
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
#elif CONFIG_WP543
	ar71xx_eth0_data.phy_mask = 0xf;
	ar71xx_add_device_mdio(0, ~ar71xx_eth0_data.phy_mask);
#else
#ifdef CONFIG_MTD_AR7100_SPI_FLASH
	if (compex)
	{
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth1_data.phy_mask = BIT(1);
	ar71xx_add_device_mdio(0, ~(ar71xx_eth0_data.phy_mask | ar71xx_eth1_data.phy_mask));
//	ar71xx_eth0_data.reset_bit = AR71XX_RESET_GE0_MAC |
//				    AR71XX_RESET_GE0_PHY;
	}else
#endif
{
	/* defaults for many switches */
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth1_data.phy_mask = BIT(4);
	ar71xx_add_device_mdio(0, ~(ar71xx_eth0_data.phy_mask | ar71xx_eth1_data.phy_mask));
	}
#endif
}

#endif

static void __init tl_wr741ndv4_gmac_setup(void)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR933X_GMAC_BASE, AR933X_GMAC_SIZE);

	t = __raw_readl(base + AR933X_GMAC_REG_ETH_CFG);
	t |= (AR933X_ETH_CFG_SW_PHY_SWAP | AR933X_ETH_CFG_SW_PHY_ADDR_SWAP);
	__raw_writel(t, base + AR933X_GMAC_REG_ETH_CFG);

	iounmap(base);
}


static int __init ar71xx_eth_dev_register(void)
{
	ar71xx_soc = ar71xx_get_soc_type();

	ar71xx_eth0_data.speed = SPEED_1000;
#ifdef CONFIG_MACH_HORNET
#ifdef CONFIG_WR741
	tl_wr741ndv4_gmac_setup();
#endif
#endif
#if defined(CONFIG_AG7100_GE0_MII)
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
#elif defined(CONFIG_AG7100_GE0_RMII)
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
#elif defined(CONFIG_AG7100_GE0_GMII)
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
#elif defined(CONFIG_AG7100_GE0_RGMII)
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
#endif

#if defined(CONFIG_AG7240_GE0_MII)
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
#elif defined(CONFIG_AG7240_GE0_RMII)
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
#elif defined(CONFIG_AG7240_GE0_GMII)
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
#elif defined(CONFIG_AG7240_GE0_RGMII)
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
#endif

#if defined(CONFIG_AG7100_GE1_MII)
	ar71xx_eth1_data.speed = SPEED_100;
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
#elif defined(CONFIG_AG7100_GE1_RMII)
	ar71xx_eth1_data.speed = SPEED_100;
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
#elif defined(CONFIG_AG7100_GE1_GMII)
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
#elif defined(CONFIG_AG7100_GE1_RGMII)
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
#endif

#if defined(CONFIG_AG7240_GE1_MII)
	ar71xx_eth1_data.speed = SPEED_100;
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
#elif defined(CONFIG_AG7240_GE1_RMII)
	ar71xx_eth1_data.speed = SPEED_100;
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RMII;
#elif defined(CONFIG_AG7240_GE1_GMII)
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
#elif defined(CONFIG_AG7240_GE1_RGMII)
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
#endif

	ar71xx_eth0_data.duplex = DUPLEX_FULL;

	phy_dev_init();

	ar71xx_add_device_eth(0);
#ifdef CONFIG_AG7100_GE1_IS_CONNECTED
	ar71xx_add_device_eth(1);
#endif
#ifdef CONFIG_AG7240_GE1_IS_CONNECTED
	ar71xx_add_device_eth(1);
#endif

	return 0;
}
module_init(ar71xx_eth_dev_register);
