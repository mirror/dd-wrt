/*
 *  Atheros AR9XXX SoCs built-in WMAC device support
 *
 *  Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros 2.6.15/2.6.31 BSP
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
#include <linux/ath9k_platform.h>

#include <asm/mach-ar71xx/ar71xx.h>

#include "dev-ar9xxx-wmac.h"

#define MHZ_25	(25 * 1000 * 1000)

static struct ath9k_platform_data ar9xxx_wmac_data = {
	.led_pin = -1,
};


void __init ath79_wmac_disable_2ghz(void)
{
	ar9xxx_wmac_data.disable_2ghz = true;
}

void __init ath79_wmac_disable_5ghz(void)
{
	ar9xxx_wmac_data.disable_5ghz = true;
}

static char ar9xxx_wmac_mac[6];

static struct resource ar9xxx_wmac_resources[] = {
	{
	 /* .start and .end fields are filled dynamically */
	 .flags = IORESOURCE_MEM,
	 }, {
	     .start = AR71XX_CPU_IRQ_IP2,
	     .end = AR71XX_CPU_IRQ_IP2,
	     .flags = IORESOURCE_IRQ,
	     },
};

static struct platform_device ar9xxx_wmac_device = {
	.name = "ath9k",
	.id = -1,
	.resource = ar9xxx_wmac_resources,
	.num_resources = ARRAY_SIZE(ar9xxx_wmac_resources),
	.dev = {
		.platform_data = &ar9xxx_wmac_data,
		},
};

static int ar913x_wmac_reset(void)
{
	ar71xx_device_stop(RESET_MODULE_AMBA2WMAC);
	mdelay(10);

	ar71xx_device_start(RESET_MODULE_AMBA2WMAC);
	mdelay(10);
	return 0;
}

static void __init ar913x_wmac_init(void)
{
	ar913x_wmac_reset();

	ar9xxx_wmac_resources[0].start = AR91XX_WMAC_BASE;
	ar9xxx_wmac_resources[0].end = AR91XX_WMAC_BASE + AR91XX_WMAC_SIZE - 1;

	ar9xxx_wmac_data.external_reset = ar913x_wmac_reset;

}

static int ar93xx_get_wmac_revision(void)
{
	return ar71xx_soc_rev;
}

static int ar933x_wmac_reset(void)
{
	unsigned retries = 0;

	ar71xx_device_stop(AR933X_RESET_WMAC);
	ar71xx_device_start(AR933X_RESET_WMAC);

	while (1) {
		u32 bootstrap;

		bootstrap = ar71xx_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
		if ((bootstrap & AR933X_BOOTSTRAP_EEPBUSY) == 0)
			return 0;

		if (retries > 20)
			break;

		udelay(10000);
		retries++;
	}

	printk(KERN_INFO "ar93xx: WMAC reset timed out");
	return -ETIMEDOUT;
}

static u32 ddr_reg_read(u32 reg)
{
	void __iomem *ddr_reg = ar71xx_ddr_base + reg;
	return __raw_readl(ddr_reg);
}


static int scorpion_wmac_reset(void)
{

#define DDR_CTL_CONFIG_ADDRESS                                       0xb8000000
#define DDR_CTL_CONFIG_OFFSET                                        0x0108
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_RESERVED_MSB                  29
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_RESERVED_LSB                  29
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_RESERVED_MASK                 0x20000000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_CPU_MSB                       28
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_CPU_LSB                       28
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_CPU_MASK                      0x10000000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GMAC0_MSB                     27
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GMAC0_LSB                     27
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GMAC0_MASK                    0x08000000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GMAC1_MSB                     26
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GMAC1_LSB                     26
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GMAC1_MASK                    0x04000000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB1_I2S_NAND_MSB             25
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB1_I2S_NAND_LSB             25
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB1_I2S_NAND_MASK            0x02000000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_PCIE1_MSB                     24
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_PCIE1_LSB                     24
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_PCIE1_MASK                    0x01000000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_WMAC_MSB                      23
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_WMAC_LSB                      23
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_WMAC_MASK                     0x00800000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_PCIE2_MSB                     22
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_PCIE2_LSB                     22
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_PCIE2_MASK                    0x00400000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB2_CSUM_MSB                 21
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB2_CSUM_LSB                 21
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB2_CSUM_MASK                0x00200000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_MSB                           29
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_LSB                           21
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_MASK                          0x3fe00000
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_GET(x)                        (((x) & DDR_CTL_CONFIG_CLIENT_ACTIVITY_MASK) >> DDR_CTL_CONFIG_CLIENT_ACTIVITY_LSB)
#define DDR_CTL_CONFIG_CLIENT_ACTIVITY_SET(x)                        (((x) << DDR_CTL_CONFIG_CLIENT_ACTIVITY_LSB) & DDR_CTL_CONFIG_CLIENT_ACTIVITY_MASK)


		    u32 data = ddr_reg_read(DDR_CTL_CONFIG_OFFSET);
		
		    int count = 0;
                    u32 usb_mask = DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB1_I2S_NAND_MASK | 
                                DDR_CTL_CONFIG_CLIENT_ACTIVITY_USB2_CSUM_MASK;
                    data &= ~usb_mask;
		    while (DDR_CTL_CONFIG_CLIENT_ACTIVITY_GET(data)) {
			    //      AVE_DEBUG(0,"DDR Activity - HIGH\n");
			    printk(KERN_ERR "DDR Activity - HIGH\n");
			    count++;
			    udelay(10);
			    data = ddr_reg_read(DDR_CTL_CONFIG_OFFSET);
                            data &= ~usb_mask;
			    if (count > 10) {
				    printk(KERN_ERR "DDR Activity timeout\n");
				    break;
			    }
		    }

		ar71xx_device_stop(QCA955X_RESET_RTC); // rtc reset
		udelay(10);
		ar71xx_device_start(QCA955X_RESET_RTC);
		udelay(10);
	    return 0;
}

static void ar933x_wmac_init(void)
{
	ar9xxx_wmac_device.name = "ar933x_wmac";
	ar9xxx_wmac_resources[0].start = AR933X_WMAC_BASE;
	ar9xxx_wmac_resources[0].end = AR933X_WMAC_BASE + AR933X_WMAC_SIZE - 1;
	if (ar71xx_ref_freq == MHZ_25) {
		printk(KERN_INFO "25MHZ ref freq\n");
		ar9xxx_wmac_data.is_clk_25mhz = true;
	}
	if (ar71xx_soc_rev == 1)
		ar9xxx_wmac_data.get_mac_revision = ar93xx_get_wmac_revision;

	ar9xxx_wmac_data.external_reset = ar933x_wmac_reset;

	ar933x_wmac_reset();
}

static void ar934x_wmac_init(void)
{
	ar9xxx_wmac_device.name = "ar934x_wmac";
	ar9xxx_wmac_resources[0].start = AR934X_WMAC_BASE;
	ar9xxx_wmac_resources[0].end = AR934X_WMAC_BASE + AR934X_WMAC_SIZE - 1;
	ar9xxx_wmac_resources[1].start = AR934X_IP2_IRQ_WMAC;
	ar9xxx_wmac_resources[1].end = AR934X_IP2_IRQ_WMAC;
	if (ar71xx_ref_freq == MHZ_25) {
		printk(KERN_INFO "25MHZ ref freq\n");
		ar9xxx_wmac_data.is_clk_25mhz = true;
	}
}

static void qca953x_wmac_init(void)
{
	u32 t;

	ar9xxx_wmac_device.name = "qca953x_wmac";

	ar9xxx_wmac_resources[0].start = QCA953X_WMAC_BASE;
	ar9xxx_wmac_resources[0].end = QCA953X_WMAC_BASE + QCA953X_WMAC_SIZE - 1;
	ar9xxx_wmac_resources[1].start = AR934X_IP2_IRQ(1);
	ar9xxx_wmac_resources[1].end = AR934X_IP2_IRQ(1);

	t = ar71xx_reset_rr(QCA953X_RESET_REG_BOOTSTRAP);
	if (t & QCA953X_BOOTSTRAP_REF_CLK_40)
		ar9xxx_wmac_data.is_clk_25mhz = false;
	else
		ar9xxx_wmac_data.is_clk_25mhz = true;
	
	ar9xxx_wmac_data.get_mac_revision = ar93xx_get_wmac_revision;
}
static void qca955x_wmac_init(void)
{
	u32 t;

	ar9xxx_wmac_device.name = "qca955x_wmac";

	ar9xxx_wmac_resources[0].start = QCA955X_WMAC_BASE;
	ar9xxx_wmac_resources[0].end = QCA955X_WMAC_BASE + QCA955X_WMAC_SIZE - 1;
	ar9xxx_wmac_resources[1].start = AR934X_IP2_IRQ(1);
	ar9xxx_wmac_resources[1].end = AR934X_IP2_IRQ(1);
	
	t = ar71xx_reset_rr(QCA955X_RESET_REG_BOOTSTRAP);
	if (t & QCA955X_BOOTSTRAP_REF_CLK_40)
		ar9xxx_wmac_data.is_clk_25mhz = false;
	else
		ar9xxx_wmac_data.is_clk_25mhz = true;
	ar9xxx_wmac_data.external_reset = scorpion_wmac_reset;

}


static void qca956x_wmac_init(void)
{
	u32 t;

	ar9xxx_wmac_device.name = "qca956x_wmac";

	ar9xxx_wmac_resources[0].start = QCA956X_WMAC_BASE;
	ar9xxx_wmac_resources[0].end = QCA956X_WMAC_BASE + QCA956X_WMAC_SIZE - 1;
	ar9xxx_wmac_resources[1].start = AR934X_IP2_IRQ(1);
	ar9xxx_wmac_resources[1].end = AR934X_IP2_IRQ(1);
	
	t = ar71xx_reset_rr(QCA956X_RESET_REG_BOOTSTRAP);
	if (t & QCA956X_BOOTSTRAP_REF_CLK_40)
		ar9xxx_wmac_data.is_clk_25mhz = false;
	else
		ar9xxx_wmac_data.is_clk_25mhz = true;

}

void __init ar9xxx_add_device_wmac(u8 *cal_data, u8 *mac_addr)
{
	switch (ar71xx_soc) {
	case AR71XX_SOC_AR9130:
	case AR71XX_SOC_AR9132:
		ar913x_wmac_init();
		break;

	case AR71XX_SOC_AR9330:
	case AR71XX_SOC_AR9331:
		ar933x_wmac_init();
		break;

	case AR71XX_SOC_AR9341:
	case AR71XX_SOC_AR9342:
	case AR71XX_SOC_AR9344:
		ar934x_wmac_init();
		break;
	case AR71XX_SOC_QCA9533:
		qca953x_wmac_init();
		break;
	case AR71XX_SOC_QCA9556:
	case AR71XX_SOC_QCA9558:
		qca955x_wmac_init();
		break;
	case AR71XX_SOC_QCA9563:
	case AR71XX_SOC_QCN550X:
	case AR71XX_SOC_TP9343:
		qca956x_wmac_init();
		break;

	default:
		BUG();
	}

	if (cal_data)
		memcpy(ar9xxx_wmac_data.eeprom_data, cal_data, sizeof(ar9xxx_wmac_data.eeprom_data));

	if (mac_addr) {
		memcpy(ar9xxx_wmac_mac, mac_addr, sizeof(ar9xxx_wmac_mac));
		ar9xxx_wmac_data.macaddr = ar9xxx_wmac_mac;
	}

	platform_device_register(&ar9xxx_wmac_device);
}
