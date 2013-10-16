/*
 * platform.c
 *
 * Copyright (C) 2012 Sebastian Gottschall <gottschall@dd-wrt.com>
 * Copyright (C) 2010-2011 Jaiganesh Narayanan <jnarayanan@atheros.com>
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2007 Atheros
 * mainly based on Atheros LSDK Code, some code taken from OpenWrt and ATH79 tree
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/resource.h>

#include <linux/console.h>
#include <asm/serial.h>

#include <linux/tty.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/etherdevice.h>

#include <asm/mach-ar7240/ar7240.h>
#include <asm/mach-ar71xx/ar71xx.h>
#include "nvram.h"
#include "devices.h"
#include <asm/mach-ar71xx/ar933x_uart_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/mtd/mtd.h>

void serial_print(char *fmt, ...);

#ifdef CONFIG_WASP_SUPPORT
extern uint32_t ath_ref_clk_freq;
#else
extern uint32_t ar7240_ahb_freq;
#endif

/* 
 * OHCI (USB full speed host controller) 
 */
static struct resource ar7240_usb_ohci_resources[] = {
	[0] = {
	       .start = AR7240_USB_OHCI_BASE,
	       .end = AR7240_USB_OHCI_BASE + AR7240_USB_WINDOW - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = AR7240_CPU_IRQ_USB,
	       .end = AR7240_CPU_IRQ_USB,
	       .flags = IORESOURCE_IRQ,
	       },
};

/* 
 * The dmamask must be set for OHCI to work 
 */
static u64 ohci_dmamask = ~(u32)0;
static struct platform_device ar7240_usb_ohci_device = {
	.name = "ar7240-ohci",
	.id = 0,
	.dev = {
		.dma_mask = &ohci_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = ARRAY_SIZE(ar7240_usb_ohci_resources),
	.resource = ar7240_usb_ohci_resources,
};

/* 
 * EHCI (USB full speed host controller) 
 */
static struct resource ar7240_usb_ehci_resources[] = {
	[0] = {
	       .start = AR7240_USB_EHCI_BASE,
	       .end = AR7240_USB_EHCI_BASE + AR7240_USB_WINDOW - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = AR7240_CPU_IRQ_USB,
	       .end = AR7240_CPU_IRQ_USB,
	       .flags = IORESOURCE_IRQ,
	       },
};

/* 
 * The dmamask must be set for EHCI to work 
 */
static u64 ehci_dmamask = ~(u32)0;

static struct platform_device ar7240_usb_ehci_device = {
	.name = "ar71xx-ehci",
	.id = 0,
	.dev = {
		.dma_mask = &ehci_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = ARRAY_SIZE(ar7240_usb_ehci_resources),
	.resource = ar7240_usb_ehci_resources,
};

static struct resource ar7240_uart_resources[] = {
	{
	 .start = AR7240_UART_BASE,
	 .end = AR7240_UART_BASE + 0x0fff,
	 .flags = IORESOURCE_MEM,
	 },
};

#define AR71XX_UART_FLAGS (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_IOREMAP)

static struct plat_serial8250_port ar7240_uart_data[] = {
	{
	 .mapbase = AR7240_UART_BASE,
	 .irq = AR7240_MISC_IRQ_UART,
	 .flags = AR71XX_UART_FLAGS,
	 .iotype = UPIO_MEM32,
	 .regshift = 2,
	 .uartclk = 0,		/* ar7240_ahb_freq, */
	 },
	{},
};

static struct platform_device ar7240_uart = {
	.name = "serial8250",
	.id = 0,
	.dev.platform_data = ar7240_uart_data,
	.num_resources = 1,
	.resource = ar7240_uart_resources
};

#ifdef CONFIG_MACH_HORNET

static struct resource ar933x_uart_resources[] = {
	{
	 .start = AR933X_UART_BASE,
	 .end = AR933X_UART_BASE + AR71XX_UART_SIZE - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{
	 .start = AR7240_MISC_IRQ_UART,
	 .end = AR7240_MISC_IRQ_UART,
	 .flags = IORESOURCE_IRQ,
	 },
};

static struct ar933x_uart_platform_data ar933x_uart_data;
static struct platform_device ar933x_uart_device = {
	.name = "ar933x-uart",
	.id = -1,
	.resource = ar933x_uart_resources,
	.num_resources = ARRAY_SIZE(ar933x_uart_resources),
	.dev = {
		.platform_data = &ar933x_uart_data,
		},
};

static struct resource ath_uart_resources[] = {
	{
	 .start = AR933X_UART_BASE,
	 .end = AR933X_UART_BASE + 0x0fff,
	 .flags = IORESOURCE_MEM,
	 },
};

static struct plat_serial8250_port ath_uart_data[] = {
	{
	 .mapbase = (u32)KSEG1ADDR(AR933X_UART_BASE),
	 .membase = (void __iomem *)((u32)(KSEG1ADDR(AR933X_UART_BASE))),
	 .irq = AR7240_MISC_IRQ_UART,
	 .flags = (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST),
	 .iotype = UPIO_MEM32,
	 .regshift = 2,
	 .uartclk = 0,		/* ath_ahb_freq, */
	 },
	{},
};

static struct platform_device ath_uart = {
	.name = "serial8250",
	.id = 0,
	.dev.platform_data = ath_uart_data,
	.num_resources = 1,
	.resource = ath_uart_resources
};

#endif

static struct platform_device *ar7241_platform_devices[] __initdata = {
	&ar7240_usb_ehci_device
};

static struct platform_device *ar7240_platform_devices[] __initdata = {
	&ar7240_usb_ohci_device
};

#if defined(CONFIG_MTD_NAND_ATH)
static struct platform_device ar9344_nand_device = {
	.name = "ar9344-nand",
	.id = -1,
};
#endif
static struct platform_device *ar724x_platform_devices[] __initdata = {
#ifdef CONFIG_MACH_HORNET
	&ar933x_uart_device,
	&ath_uart,
#else
	&ar7240_uart,
#endif
#ifdef CONFIG_RB2011
	&ar9344_nand_device,
#endif
};

static struct ar8327_pad_cfg db120_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg dir825c1_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_platform_data db120_ar8327_data = {
	.pad0_cfg = &db120_ar8327_pad0_cfg,
	.port0_cfg = {
		      .force_link = 1,

#ifdef CONFIG_DIR615I
		      .speed = AR8327_PORT_SPEED_100,
#else
		      .speed = AR8327_PORT_SPEED_1000,
#endif
		      .duplex = 1,
		      .txpause = 1,
		      .rxpause = 1,
		      },
#ifdef CONFIG_DIR825C1
	.led_cfg = &dir825c1_ar8327_led_cfg,
#endif
};

static struct mdio_board_info db120_mdio0_info[] = {
	{
	 .bus_id = "ag71xx-mdio.0",
	 .phy_addr = 0,
	 .platform_data = &db120_ar8327_data,
	 },
};

static struct ar8327_pad_cfg wdr4300_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wdr4300_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_platform_data wdr4300_ar8327_data = {
	.pad0_cfg = &wdr4300_ar8327_pad0_cfg,
	.port0_cfg = {
		      .force_link = 1,
		      .speed = AR8327_PORT_SPEED_1000,
		      .duplex = 1,
		      .txpause = 1,
		      .rxpause = 1,
		      },
	.led_cfg = &wdr4300_ar8327_led_cfg,
};

static struct mdio_board_info wdr4300_mdio0_info[] = {
	{
	 .bus_id = "ag71xx-mdio.0",
	 .phy_addr = 0,
	 .platform_data = &wdr4300_ar8327_data,
	 },
};

extern void __init ap91_pci_init(u8 *cal_data, u8 *mac_addr);
void ar9xxx_add_device_wmac(u8 *cal_data, u8 *mac_addr) __init;

#if !defined(CONFIG_MACH_HORNET) && !defined(CONFIG_WASP_SUPPORT)
static void *getCalData(int slot)
{
#ifdef CONFIG_WDR2543
	u8 *base = KSEG1ADDR(0x1fff1000);
	printk(KERN_INFO "found calibration data for slot %d on 0x%08X\n", slot, base);
	return base;
#else
	u8 *base;
	for (base = (u8 *)KSEG1ADDR(0x1f000000); base < (u8 *)KSEG1ADDR(0x1ffff000); base += 0x1000) {
		u32 *cal = (u32 *)base;
		if (*cal == 0xa55a0000 || *cal == 0x5aa50000) {	//protection bit is always zero on inflash devices, so we can use for match it
			if (slot) {
				base += 0x4000;
			}
			printk(KERN_INFO "found calibration data for slot %d on 0x%08X\n", slot, base);
			return base;
		}
	}
	return NULL;
#endif
}

#endif
enum ar71xx_soc_type ar71xx_soc;
EXPORT_SYMBOL_GPL(ar71xx_soc);

void __init ath79_init_mac(unsigned char *dst, const unsigned char *src, int offset)
{
	int t;

	if (!is_valid_ether_addr(src)) {
		memset(dst, '\0', ETH_ALEN);
		return;
	}

	t = (((u32)src[3]) << 16) + (((u32)src[4]) << 8) + ((u32)src[5]);
	t += offset;

	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = (t >> 16) & 0xff;
	dst[4] = (t >> 8) & 0xff;
	dst[5] = t & 0xff;
}

#if defined(CONFIG_DIR825C1) || defined(CONFIG_DIR615I)
#define DIR825C1_MAC_LOCATION_0			0x1ffe0004
#define DIR825C1_MAC_LOCATION_1			0x1ffe0018
#define DIR615I_MAC_LOCATION_0			0x1fffffb4
static u8 mac0[6];
static u8 mac1[6];

static void dir825b1_read_ascii_mac(u8 *dest, unsigned int src_addr)
{
	int ret;
	u8 *src = (u8 *)KSEG1ADDR(src_addr);

	ret = sscanf(src, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]);

	if (ret != 6)
		memset(dest, 0, 6);
}

#endif
static void enable_uart(void)
{

	if (is_ar7240() || is_ar7241() || is_ar7242())
		ar71xx_gpio_function_enable(AR724X_GPIO_FUNC_UART_EN);
	else if (is_ar933x())
		ar71xx_gpio_function_enable(AR933X_GPIO_FUNC_UART_EN);

	/* need to set clock appropriately */
#ifdef CONFIG_MACH_HORNET
	ath_uart_data[0].uartclk = ar71xx_ref_freq;
	ar933x_uart_data.uartclk = ar71xx_ref_freq;
#endif
#ifdef CONFIG_WASP_SUPPORT
	ar7240_uart_data[0].uartclk = ath_ref_clk_freq;
#else
	ar7240_uart_data[0].uartclk = ar7240_ahb_freq;
#endif

}

int __init ar7240_platform_init(void)
{
	int ret;
	void *ee;
#if defined(CONFIG_WR741) || defined(CONFIG_WDR4300) || defined(CONFIG_WDR2543) || defined(CONFIG_WR841V8)
	u8 *mac = (u8 *)KSEG1ADDR(0x1f01fc00);
#else
	u8 *mac = NULL;		//(u8 *) KSEG1ADDR(0x1fff0000);
#endif

#if defined(CONFIG_AR7242_RTL8309G_PHY) || defined(CONFIG_DIR615E)
#ifdef CONFIG_DIR615E
	const char *config = (char *)KSEG1ADDR(0x1f030000);
#else
	const char *config = (char *)KSEG1ADDR(0x1f040000);
#endif
	u8 wlan_mac[6];
	if (nvram_parse_mac_addr(config, 0x10000, "lan_mac=", wlan_mac) == 0) {
		mac = wlan_mac;
	}
#endif
	enable_uart();

#ifdef CONFIG_WASP_SUPPORT
#define DB120_MAC0_OFFSET	0
#define DB120_MAC1_OFFSET	6
#ifdef CONFIG_DIR825C1
	u8 *art = (u8 *)KSEG1ADDR(0x1fff1000);
#elif CONFIG_WR841V8
//              u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
#else
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
#endif
	void __iomem *base;
	u32 t;

#ifndef CONFIG_WDR4300
#ifdef CONFIG_DIR825C1
	dir825b1_read_ascii_mac(mac0, DIR825C1_MAC_LOCATION_0);
	dir825b1_read_ascii_mac(mac1, DIR825C1_MAC_LOCATION_1);
#endif

#else
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
#endif

#ifdef CONFIG_DIR615I

	dir825b1_read_ascii_mac(mac0, DIR615I_MAC_LOCATION_0);
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);

	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE);

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
#elif CONFIG_WR841V8
	//swap phy
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);
	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_GMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE | AR934X_ETH_CFG_SW_PHY_SWAP);

	t |= AR934X_ETH_CFG_SW_PHY_SWAP;

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
#else
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);
	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE);
#ifdef CONFIG_WDR4300
	t |= AR934X_ETH_CFG_RGMII_GMAC0;
#else
	t |= AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE;
#endif
	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
#endif

#ifdef CONFIG_DIR615I
	ar71xx_add_device_mdio(1, 0x0);
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio1_device.dev;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_switch_data.phy4_mii_en = 1;

	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ar71xx_add_device_eth(0);
	ar71xx_add_device_eth(1);
#elif CONFIG_WR841V8
	ar71xx_add_device_mdio(1, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, -1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, 0);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_switch_data.phy_poll_mask = BIT(0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio1_device.dev;
	ar71xx_add_device_eth(0);

	/* GMAC1 is connected to the internal switch */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_add_device_eth(1);
#else
#ifndef CONFIG_WDR4300
#ifndef CONFIG_DIR825C1
	ar71xx_add_device_mdio(1, 0x0);
#endif
#endif
	ar71xx_add_device_mdio(0, 0x0);

#ifdef CONFIG_DIR825C1
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 0);
#else
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
#endif

#ifdef CONFIG_WDR4300
	mdiobus_register_board_info(wdr4300_mdio0_info, ARRAY_SIZE(wdr4300_mdio0_info));
#else

	mdiobus_register_board_info(db120_mdio0_info, ARRAY_SIZE(db120_mdio0_info));
#endif
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_pll_data.pll_1000 = 0x06000000;
	ar71xx_add_device_eth(0);

#ifndef CONFIG_WDR4300
#ifndef CONFIG_DIR825C1
	/* GMAC1 is connected to the internal switch */
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, art + DB120_MAC1_OFFSET, 0);
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;

	ar71xx_add_device_eth(1);
#endif
#endif
#endif
#endif

	ret = platform_add_devices(ar724x_platform_devices, ARRAY_SIZE(ar724x_platform_devices));

	if (ret < 0)
		return ret;

	if (is_ar7241() || is_ar7242() || is_ar933x() || is_ar934x()) {
		ret = platform_add_devices(ar7241_platform_devices, ARRAY_SIZE(ar7241_platform_devices));
	}
	if (is_ar7240()) {
		ret = platform_add_devices(ar7240_platform_devices, ARRAY_SIZE(ar7240_platform_devices));
	}
	platform_device_register_simple("ar71xx-wdt", -1, NULL, 0);

#ifdef CONFIG_MACH_HORNET
	ee = (u8 *)KSEG1ADDR(0x1fff1000);
	ar9xxx_add_device_wmac(ee, mac);
#elif CONFIG_WASP_SUPPORT
#if !defined(CONFIG_MTD_NAND_ATH)
	ee = (u8 *)KSEG1ADDR(0x1fff1000);
#if defined(CONFIG_DIR825C1)
	ar9xxx_add_device_wmac(ee, mac0);
#elif defined(CONFIG_DIR615I)
	ar9xxx_add_device_wmac(ee, mac0);
#elif defined(CONFIG_WR841V8)
	ar9xxx_add_device_wmac(ee, mac);
#else
	ar9xxx_add_device_wmac(ee, NULL);
#endif

#if defined(CONFIG_DIR825C1)
	ap91_pci_init(ee + 0x4000, mac1);
#elif !defined(CONFIG_DIR615I) && !defined(CONFIG_WR841V8)
	ap91_pci_init(NULL, NULL);
#endif
#endif
#else				// WASP_SUPPORT
	ee = getCalData(0);
	if (ee && !mac) {
		if (!memcmp(((u8 *)ee) + 0x20c, "\xff\xff\xff\xff\xff\xff", 6) || !memcmp(((u8 *)ee) + 0x20c, "\x00\x00\x00\x00\x00\x00", 6) || !memcmp(((u8 *)ee) + 0x20c, "\x08\x00\x00\x00\x00\x00", 6)) {
			printk("Found empty mac address in calibration dataset, leave the responsibility to the driver to use the correct one\n");
			mac = ((u8 *)ee) - 0x1000;
		}
		if (mac && (!memcmp(mac, "\xff\xff\xff\xff\xff\xff", 6) || !memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6))) {
			printk("Found empty mac address in dataset, leave the responsibility to the driver to use the correct one\n");
			mac = NULL;
		}

	}
	ap91_pci_init(ee, mac);
#endif
	return ret;
}

#if defined(CONFIG_MTD_NAND_ATH)
void nand_postinit(struct mtd_info *mtd)
{

	u8 *ee = (u8 *)kmalloc(0x9000, GFP_ATOMIC);
	int i;
	int mtdlen;
	mtd_read(mtd, 0x80000, 0x9000, &mtdlen, ee);
	ar9xxx_add_device_wmac(ee + 0x1000, ee + 6);
	ap91_pci_init(ee + 0x5000, ee + 12);
}
#endif
arch_initcall(ar7240_platform_init);
