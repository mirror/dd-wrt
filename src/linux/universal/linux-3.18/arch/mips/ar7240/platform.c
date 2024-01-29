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
#include <linux/usb/ehci_pdriver.h>
#include <linux/usb/ohci_pdriver.h>

#include <asm/mach-ar7240/ar7240.h>
#include <asm/mach-ar71xx/ar71xx.h>
#include "nvram.h"
#include "devices.h"
#include <asm/mach-ar71xx/ar933x_uart_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/mtd/mtd.h>
#include <linux/platform_data/phy-at803x.h>

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


static struct usb_ehci_pdata ath79_ehci_pdata_v2 = {
	.caps_offset		= 0x100,
	.has_tt			= 1,
	.qca_force_host_mode	= 1,
	.qca_force_16bit_ptw	= 1,
};

static void enable_tx_tx_idp_violation_fix(unsigned base)
 {
	void __iomem *phy_reg;
	u32 t;
 
	phy_reg = ioremap(base, 4);
	if (!phy_reg)
 		return;
 
	t = ioread32(phy_reg);
	t &= ~0xff;
	t |= 0x58;
	iowrite32(t, phy_reg);

	iounmap(phy_reg);
}


static void qca955x_usb_reset_notifier(struct platform_device *pdev)
{
	u32 base;

	printk(KERN_INFO "Reset Notifier for %d\n",pdev->id);
	switch (pdev->id) {
	case 0:
		base = 0x18116c94;
		break;

	case 1:
		base = 0x18116e54;
		break;

	default:
		return;
	}

	enable_tx_tx_idp_violation_fix(base);
	printk(KERN_INFO "TX-TX IDP fix enabled\n");
}

static u64 ath79_usb_dmamask = DMA_BIT_MASK(32);

static void __init ath79_usb_register(const char *name, int id,
				      unsigned long base, unsigned long size,
				      int irq, const void *data,
				      size_t data_size)
{
	struct resource res[2];
	struct platform_device *pdev;

	memset(res, 0, sizeof(res));

	res[0].flags = IORESOURCE_MEM;
	res[0].start = base;
	res[0].end = base + size - 1;

	res[1].flags = IORESOURCE_IRQ;
	res[1].start = irq;
	res[1].end = irq;

	pdev = platform_device_register_resndata(NULL, name, id,
						 res, ARRAY_SIZE(res),
						 data, data_size);

	if (IS_ERR(pdev)) {
		pr_err("ath79: unable to register USB at %08lx, err=%d\n",
		       base, (int) PTR_ERR(pdev));
		return;
	}

	pdev->dev.dma_mask = &ath79_usb_dmamask;
	pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);
}

static void ar933x_usb_setup(void)
{
	ar71xx_device_stop(AR933X_RESET_USBSUS_OVERRIDE);
	mdelay(10);

	ar71xx_device_start(AR933X_RESET_USB_HOST);
	mdelay(10);

	ar71xx_device_start(AR933X_RESET_USB_PHY);
	mdelay(10);

	ath79_usb_register("ehci-platform", -1,
			   AR933X_EHCI_BASE, AR933X_EHCI_SIZE,
			   3,
			   &ath79_ehci_pdata_v2, sizeof(ath79_ehci_pdata_v2));
}


static void qca956x_usb_setup(void)
{
	ath79_usb_register("ehci-platform", 0,
			   QCA956X_EHCI0_BASE, QCA956X_EHCI_SIZE,
			   AR934X_IP3_IRQ(0),
			   &ath79_ehci_pdata_v2, sizeof(ath79_ehci_pdata_v2));

	ath79_usb_register("ehci-platform", 1,
			   QCA956X_EHCI1_BASE, QCA956X_EHCI_SIZE,
			   AR934X_IP3_IRQ(1),
			   &ath79_ehci_pdata_v2, sizeof(ath79_ehci_pdata_v2));
}

static void __init ath79_gpio_output_select(unsigned gpio, u8 val)
{
	void __iomem *base = ar71xx_gpio_base;
	unsigned long flags;
	unsigned int reg;
	u32 t, s;


	if (gpio >= AR934X_GPIO_COUNT)
		return;

	reg = AR934X_GPIO_REG_OUT_FUNC0 + 4 * (gpio / 4);
	s = 8 * (gpio % 4);

//	spin_lock_irqsave(&ar71xx_gpio_lock, flags);

	t = __raw_readl(base + reg);
	t &= ~(0xff << s);
	t |= val << s;
	__raw_writel(t, base + reg);

	/* flush write */
	(void) __raw_readl(base + reg);

//	spin_unlock_irqrestore(&ar71xx_gpio_lock, flags);
}

static void __init qca953x_usb_setup(void)
{
	u32 bootstrap;

	bootstrap = ar71xx_reset_rr(QCA953X_RESET_REG_BOOTSTRAP);

	ar71xx_device_stop(QCA953X_RESET_USBSUS_OVERRIDE);
	udelay(1000);

	ar71xx_device_start(QCA953X_RESET_USB_PHY);
	udelay(1000);

	ar71xx_device_start(QCA953X_RESET_USB_PHY_ANALOG);
	udelay(1000);

	ar71xx_device_start(QCA953X_RESET_USB_HOST);
	udelay(1000);

	ath79_usb_register("ehci-platform", -1,
			   QCA953X_EHCI_BASE, QCA953X_EHCI_SIZE,
			   AR71XX_CPU_IRQ_USB,
			   &ath79_ehci_pdata_v2, sizeof(ath79_ehci_pdata_v2));
}




static void qca_usbregister(void) {


	if (is_qca953x())
		qca953x_usb_setup();
	else if (is_qca956x()) {
		qca956x_usb_setup();
	} else {
	    
	ath79_ehci_pdata_v2.reset_notifier = qca955x_usb_reset_notifier;

	ath79_usb_register("ehci-platform", 0,
			   AR7240_USB_EHCI_BASE, QCA955X_EHCI_SIZE,
			   AR934X_IP3_IRQ(0),
			   &ath79_ehci_pdata_v2, sizeof(ath79_ehci_pdata_v2));

	ath79_usb_register("ehci-platform", 1,
			   AR7240_USB_EHCI_BASE + 0x400000, QCA955X_EHCI_SIZE,
			   AR934X_IP3_IRQ(1),
			   &ath79_ehci_pdata_v2, sizeof(ath79_ehci_pdata_v2));

	}
}

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
#if !defined(CONFIG_FMS2111)
	 .irq = AR7240_MISC_IRQ_UART,
#endif
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


#ifdef CONFIG_ARCHERC25
#include <linux/leds.h>
#include <linux/gpio.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/74x164.h>
void __init ath79_register_leds_gpio(int id,
				     unsigned num_leds,
				     struct gpio_led *leds)
{
	struct platform_device *pdev;
	struct gpio_led_platform_data pdata;
	struct gpio_led *p;
	int err;

	p = kmemdup(leds, num_leds * sizeof(*p), GFP_KERNEL);
	if (!p)
		return;

	pdev = platform_device_alloc("leds-gpio", id);
	if (!pdev)
		goto err_free_leds;

	memset(&pdata, 0, sizeof(pdata));
	pdata.num_leds = num_leds;
	pdata.leds = p;

	err = platform_device_add_data(pdev, &pdata, sizeof(pdata));
	if (err)
		goto err_put_pdev;

	err = platform_device_add(pdev);
	if (err)
		goto err_put_pdev;

	return;

err_put_pdev:
	platform_device_put(pdev);

err_free_leds:
	kfree(p);
}


#define ARCHER_C25_GPIO_SHIFT_OE	21 /* OE,   Output Enable */
#define ARCHER_C25_GPIO_SHIFT_SER	14 /* DS,   Data Serial Input */
#define ARCHER_C25_GPIO_SHIFT_SRCLK	15 /* SHCP, Shift Reg Clock Input */
#define ARCHER_C25_GPIO_SHIFT_SRCLR	19 /* MR,   Master Reset */
#define ARCHER_C25_GPIO_SHIFT_RCLK	16 /* STCP, Storage Reg Clock Input */

#define ARCHER_C25_74HC_GPIO_BASE		120
#define ARCHER_C25_74HC_GPIO_LED_WAN_AMBER	(ARCHER_C25_74HC_GPIO_BASE + 4)
#define ARCHER_C25_74HC_GPIO_LED_WAN_GREEN	(ARCHER_C25_74HC_GPIO_BASE + 5)
#define ARCHER_C25_74HC_GPIO_LED_WLAN2		(ARCHER_C25_74HC_GPIO_BASE + 6)
#define ARCHER_C25_74HC_GPIO_LED_WLAN5		(ARCHER_C25_74HC_GPIO_BASE + 7)
#define ARCHER_C25_74HC_GPIO_LED_LAN1		(ARCHER_C25_74HC_GPIO_BASE + 0)
#define ARCHER_C25_74HC_GPIO_LED_LAN2		(ARCHER_C25_74HC_GPIO_BASE + 1)
#define ARCHER_C25_74HC_GPIO_LED_LAN3		(ARCHER_C25_74HC_GPIO_BASE + 2)
#define ARCHER_C25_74HC_GPIO_LED_LAN4		(ARCHER_C25_74HC_GPIO_BASE + 3)

#define ARCHER_C25_V1_SSR_BIT_0			0
#define ARCHER_C25_V1_SSR_BIT_1			1
#define ARCHER_C25_V1_SSR_BIT_2			2
#define ARCHER_C25_V1_SSR_BIT_3			3
#define ARCHER_C25_V1_SSR_BIT_4			4
#define ARCHER_C25_V1_SSR_BIT_5			5
#define ARCHER_C25_V1_SSR_BIT_6			6
#define ARCHER_C25_V1_SSR_BIT_7			7

static struct spi_gpio_platform_data archer_c25_v1_spi_data = {
	.sck		= ARCHER_C25_GPIO_SHIFT_SRCLK,
	.miso		= SPI_GPIO_NO_MISO,
	.mosi		= ARCHER_C25_GPIO_SHIFT_SER,
	.num_chipselect	= 1,
};

static u8 archer_c25_v1_ssr_initdata[] = {
	BIT(ARCHER_C25_V1_SSR_BIT_7) |
	BIT(ARCHER_C25_V1_SSR_BIT_6) |
	BIT(ARCHER_C25_V1_SSR_BIT_5) |
	BIT(ARCHER_C25_V1_SSR_BIT_4) |
	BIT(ARCHER_C25_V1_SSR_BIT_3) |
	BIT(ARCHER_C25_V1_SSR_BIT_2) |
	BIT(ARCHER_C25_V1_SSR_BIT_1)
};

static struct gen_74x164_chip_platform_data archer_c25_v1_ssr_data = {
	.base = ARCHER_C25_74HC_GPIO_BASE,
	.num_registers = ARRAY_SIZE(archer_c25_v1_ssr_initdata),
	.init_data = archer_c25_v1_ssr_initdata,
};

static struct platform_device archer_c25_v1_spi_device = {
	.name		= "spi_gpio",
	.id		= 1,
	.dev = {
		.platform_data = &archer_c25_v1_spi_data,
	},
};

static struct spi_board_info archer_c25_v1_spi_info[] = {
	{
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 10000000,
		.modalias		= "74x164",
		.platform_data		= &archer_c25_v1_ssr_data,
		.controller_data	= (void *) ARCHER_C25_GPIO_SHIFT_RCLK,
	},
};

static struct gpio_led archer_c25_v1_leds_gpio[] __initdata = {
	{
		.name		= "wireless_generic_126",
		.gpio		= ARCHER_C25_74HC_GPIO_LED_WLAN2,
		.active_low	= 1,
	}, {
		.name		= "wireless_generic_127",
		.gpio		= ARCHER_C25_74HC_GPIO_LED_WLAN5,
		.active_low	= 1,
	}, {
		.name		= "generic_120",
		.gpio		= ARCHER_C25_74HC_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "generic_121",
		.gpio		= ARCHER_C25_74HC_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "generic_122",
		.gpio		= ARCHER_C25_74HC_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "generic_123",
		.gpio		= ARCHER_C25_74HC_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "generic_125",
		.gpio		=  ARCHER_C25_74HC_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "generic_124",
		.gpio		=  ARCHER_C25_74HC_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	},
};

#endif

static struct ar8327_led_cfg wr1043nd_v2_ar8327_led_cfg = {
	.led_ctrl0 = 0xcc35cc35,
	.led_ctrl1 = 0xca35ca35,
	.led_ctrl2 = 0xc935c935,
	.led_ctrl3 = 0x03ffff00,
	.open_drain = true,
};

static struct ar8327_led_cfg archer_c7_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_pad_cfg ap136_ar8327_pad0_cfg;
static struct ar8327_pad_cfg ap136_ar8327_pad6_cfg;

static struct ar8327_platform_data ap136_ar8327_data = {
	.pad0_cfg = &ap136_ar8327_pad0_cfg,
	.pad6_cfg = &ap136_ar8327_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
#ifdef CONFIG_ARCHERC7
	.led_cfg = &archer_c7_ar8327_led_cfg,
#elif defined(CONFIG_WR1043V2)
	.led_cfg = &wr1043nd_v2_ar8327_led_cfg,
#endif
};

static struct mdio_board_info ap136_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &ap136_ar8327_data,
	},
};


#define AP152_GPIO_MDC			3
#define AP152_GPIO_MDIO			4

static void __init ap152_mdio_setup(void)
{
	ath79_gpio_output_select(AP152_GPIO_MDC, QCA956X_GPIO_OUT_MUX_GE0_MDC);
	ath79_gpio_output_select(AP152_GPIO_MDIO, QCA956X_GPIO_OUT_MUX_GE0_MDO);

	ar71xx_add_device_mdio(0, 0x0);
}



static struct at803x_platform_data rb922gs_at803x_data = {
	.disable_smarteee = 1,
};
static struct mdio_board_info dap2330_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 4,
		.platform_data = &rb922gs_at803x_data,
	},
};


static struct at803x_platform_data cf_e380ac_at803x_data = {
	.disable_smarteee = 1,
//	.enable_rgmii_rx_delay = 1,
//	.enable_rgmii_tx_delay = 1,
};

static struct mdio_board_info cf_e380ac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &cf_e380ac_at803x_data,
	},
};


static struct at803x_platform_data ubnt_loco_m_xw_at803x_data = {
	.has_reset_gpio = 1,
	.reset_gpio = 0,
};

static struct mdio_board_info ubnt_loco_m_xw_mdio_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 1,
		.platform_data = &ubnt_loco_m_xw_at803x_data,
	},
};

static void __init ap136_gmac_setup(u32 mask)
{
	void __iomem *base;
	u32 t;

	base = ioremap(QCA955X_GMAC_BASE, QCA955X_GMAC_SIZE);

	t = __raw_readl(base + QCA955X_GMAC_REG_ETH_CFG);

	t &= ~(QCA955X_ETH_CFG_RGMII_EN | QCA955X_ETH_CFG_GE0_SGMII);
	t |= mask;

	__raw_writel(t, base + QCA955X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

static struct ar8327_pad_cfg cf_wr650ac_ar8327_pad0_cfg = {
	/* GMAC0 of the AR8337 switch is connected to GMAC0 via RGMII */
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_pad_cfg cf_wr650ac_ar8327_pad6_cfg = {
	/* GMAC6 of the AR8337 switch is connected to GMAC1 via SGMII */
	.mode = AR8327_PAD_MAC_SGMII,
	.rxclk_delay_en = true,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_platform_data cf_wr650ac_ar8327_data = {
	.pad0_cfg = &cf_wr650ac_ar8327_pad0_cfg,
	.pad6_cfg = &cf_wr650ac_ar8327_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info cf_wr650ac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &cf_wr650ac_ar8327_data,
	},
};


static struct ar8327_pad_cfg db120_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg dir825c1_ar8327_led_cfg = {
	.led_ctrl0 = 0x00000000,
	.led_ctrl1 = 0xc737c737,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x00c30c00,
	.open_drain = true,
};

static struct ar8327_led_cfg wndr4300_ar8327_led_cfg = {
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
#if defined(CONFIG_MTD_NAND_ATH)
	.led_cfg = &wndr4300_ar8327_led_cfg,
#else
	.led_cfg = &dir825c1_ar8327_led_cfg,
#endif
#endif
};

static struct at803x_platform_data  at803_data = {
	.disable_smarteee = 1,
};
static struct mdio_board_info at803_mdio_info[] = {
        {
                .bus_id = "ag71xx-mdio.0",
                .phy_addr = 4,
                .platform_data = &at803_data,
        },
};


static struct at803x_platform_data ubnt_rocket_m_ti_at803_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};
static struct mdio_board_info ubnt_rocket_m_ti_mdio_info[] = {
        {
                .bus_id = "ag71xx-mdio.0",
                .phy_addr = 4,
                .platform_data = &ubnt_rocket_m_ti_at803_data,
        },
};



static struct mdio_board_info db120_mdio0_info[] = {
	{
	 .bus_id = "ag71xx-mdio.0",
	 .phy_addr = 0,
	 .platform_data = &db120_ar8327_data,
	 },
};

static struct ar8327_pad_cfg wdr4300_ar8327_pad0_cfg = {
	.mac06_exchange_dis = true,
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wdr4300_ar8327_led_cfg = {
#ifdef CONFIG_DW02_412H
	.led_ctrl0 = 0xcf37cf37,
#else
	.led_ctrl0 = 0xc737c737,
#endif
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


static struct ar8327_pad_cfg ap152_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data ap152_ar8337_data = {
	.pad0_cfg = &ap152_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info ap152_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &ap152_ar8337_data,
	},
};


static struct ar8327_pad_cfg wpj344_ar8327_pad0_cfg = {
	.mac06_exchange_dis = true,
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wpj344_ar8327_led_cfg = {
	.led_ctrl0 = 0x00000000,
	.led_ctrl1 = 0xc737c737,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x00c30c00,
	.open_drain = true,
};

static struct ar8327_platform_data wpj344_ar8327_data = {
	.pad0_cfg = &wpj344_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &wpj344_ar8327_led_cfg,
};

static struct mdio_board_info wpj344_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &wpj344_ar8327_data,
	},
};



static struct ar8327_pad_cfg nanoac_ar8327_pad0_cfg = {
	.mac06_exchange_dis = true,
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg nanoac_ar8327_led_cfg = {

	.led_ctrl0 = 0xffb7ffb7,
	.led_ctrl1 = 0xffb7ffb7,
	.led_ctrl2 = 0xffb7ffb7,
	.led_ctrl3 = 0x03ffff00,

//	.led_ctrl0 = 0x00000000,
//	.led_ctrl1 = 0xc737c737,
//	.led_ctrl2 = 0x00000000,
//	.led_ctrl3 = 0x00c30c00,

//	.led_ctrl0 = 0x00000000,
//	.led_ctrl1 = 0x00000000,
//	.led_ctrl2 = 0xffb7ffb7,
//	.led_ctrl3 = 0x03ffff00,
	.open_drain = true,
};

static struct ar8327_platform_data nanoac_ar8327_data = {
	.pad0_cfg = &nanoac_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &nanoac_ar8327_led_cfg,
};

static struct mdio_board_info nanoac_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &nanoac_ar8327_data,
	},
};





static struct ar8327_pad_cfg ap120c_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data ap120c_ar8327_data = {
	.pad0_cfg = &ap120c_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info ap120c_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &ap120c_ar8327_data,
	},
};


static struct at803x_platform_data rambutan_ar8032_data = {
	.has_reset_gpio = 1,
	.reset_gpio = 17,
};

static struct mdio_board_info rambutan_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &rambutan_ar8032_data,
	},
};

static struct at803x_platform_data rambutan_ar8033_data = {
	.has_reset_gpio = 1,
	.reset_gpio = 23,
};

static struct mdio_board_info rambutan_mdio1_info[] = {
	{
		.bus_id = "ag71xx-mdio.1",
		.phy_addr = 0,
		.platform_data = &rambutan_ar8033_data,
	},
};

static struct ar8327_pad_cfg ubnt_unifiac_pro_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data ubnt_unifiac_pro_ar8327_data = {
	.pad0_cfg = &ubnt_unifiac_pro_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};


static struct mdio_board_info ubnt_unifiac_pro_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &ubnt_unifiac_pro_ar8327_data,
	},
};


#define ALFA_AP120C_LAN_PHYMASK		BIT(5)
#define ALFA_AP120C_MDIO_PHYMASK	ALFA_AP120C_LAN_PHYMASK
#define ALFA_AP120C_MAC_OFFSET		0x2


extern void __init ap91_pci_init(u8 *cal_data, u8 *mac_addr);
void ar9xxx_add_device_wmac(u8 *cal_data, u8 *mac_addr) __init;


static __init ath79_setup_ar933x_phy4_switch(bool mac, bool mdio)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR933X_GMAC_BASE, AR933X_GMAC_SIZE);

	t = __raw_readl(base + AR933X_GMAC_REG_ETH_CFG);
	t &= ~(AR933X_ETH_CFG_SW_PHY_SWAP | AR933X_ETH_CFG_SW_PHY_ADDR_SWAP);
	if (mac)
		t |= AR933X_ETH_CFG_SW_PHY_SWAP;
	if (mdio)
		t |= AR933X_ETH_CFG_SW_PHY_ADDR_SWAP;
	__raw_writel(t, base + AR933X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

void __init ath79_setup_ar934x_eth_cfg(u32 mask)
{
	void __iomem *base;
	u32 t;

	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);

	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 |
	       AR934X_ETH_CFG_MII_GMAC0 |
	       AR934X_ETH_CFG_GMII_GMAC0 |
	       AR934X_ETH_CFG_SW_ONLY_MODE |
	       AR934X_ETH_CFG_SW_PHY_SWAP);

	t |= mask;

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	iounmap(base);
}

void __init ath79_setup_ar934x_eth_rx_delay(unsigned int rxd,
					    unsigned int rxdv)
{
	void __iomem *base;
	u32 t;

	rxd &= AR934X_ETH_CFG_RXD_DELAY_MASK;
	rxdv &= AR934X_ETH_CFG_RDV_DELAY_MASK;

	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);

	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	t &= ~(AR934X_ETH_CFG_RXD_DELAY_MASK << AR934X_ETH_CFG_RXD_DELAY_SHIFT |
	       AR934X_ETH_CFG_RDV_DELAY_MASK << AR934X_ETH_CFG_RDV_DELAY_SHIFT);

	t |= (rxd << AR934X_ETH_CFG_RXD_DELAY_SHIFT |
	      rxdv << AR934X_ETH_CFG_RDV_DELAY_SHIFT);

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	iounmap(base);
}



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

#if defined(CONFIG_DIR825C1) || defined(CONFIG_DIR615I) || defined(CONFIG_DAP2230)


#define AR934X_GPIO_REG_OUT_FUNC0	0x2c





#define DIR825C1_MAC_LOCATION_0			0x1ffe0004
#define DIR825C1_MAC_LOCATION_1			0x1ffe0018

#define DIR859_MAC_LOCATION_0			0x1f050051
#define DIR859_MAC_LOCATION_1			0x1f05006c

#define DIR862_MAC_LOCATION_0			0x1ffe0004
#define DIR862_MAC_LOCATION_1			0x1ffe0018

#define DHP1565A1_MAC_LOCATION_0			0x1ffeffa0
#define DHP1565A1_MAC_LOCATION_1			0x1ffeffb4

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

static char *scanmac(char *dest, char *base,char *str)
{
int i;
printk(KERN_INFO "scan mac from %p\n",base);
for (i=0;i<0x400;i++) {
    if (!memcmp(&base[i],str,strlen(str))) {
	    printk(KERN_INFO "found @%p\n",&base[i+strlen(str)]);
	    char *src = &base[i+strlen(str)];
	    int ret = sscanf(src, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]);
	    if (ret!=6) {
		printk(KERN_INFO "parse fail\n");
		return NULL;
	    }
	    return dest;
    }
}
printk(KERN_INFO "found nothing\n");
return NULL;
}

void __init ap91_wmac_disable_2ghz(void);
void __init ap91_wmac_disable_5ghz(void);
void ap91_set_tx_gain_buffalo(void);
void ap91_set_eeprom(void);

int __init ar7240_platform_init(void)
{
	int ret;
	void *ee = NULL;
#if defined(CONFIG_WR741) || defined(CONFIG_WDR4300) || defined(CONFIG_WDR2543) || defined(CONFIG_WR841V8) || defined(CONFIG_WR810N) && !defined(CONFIG_GL150)
	u8 *mac = (u8 *)KSEG1ADDR(0x1f01fc00);
#else
	u8 *mac = NULL;		//(u8 *) KSEG1ADDR(0x1fff0000);
#endif
#if defined(CONFIG_DW02_412H)
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
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
#ifdef CONFIG_MACH_HORNET
#if defined(CONFIG_WR710) || defined(CONFIG_ERC) || defined(CONFIG_GL150)
       ath79_setup_ar933x_phy4_switch(false, false);
#else
       ath79_setup_ar933x_phy4_switch(true, true);
#endif
#endif
#ifdef CONFIG_GL150
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
#endif
#ifdef CONFIG_E325N
	mac = (u8 *)KSEG1ADDR(0x1f010000);
	ee = (u8 *)KSEG1ADDR(0x1f011000);
#endif
#ifdef CONFIG_WR650AC
	mac = (u8 *)KSEG1ADDR(0x1f020000);
	ee = (u8 *)KSEG1ADDR(0x1f021000);
#endif
#ifdef CONFIG_WR615N
	mac = (u8 *)KSEG1ADDR(0x1f010000);
	ee = (u8 *)KSEG1ADDR(0x1f011000);
#endif
#ifdef CONFIG_XD9531
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ee = (u8 *)KSEG1ADDR(0x1fff1000);
#endif
#ifdef CONFIG_E355AC
	mac = (u8 *)KSEG1ADDR(0x1f010000);
	ee = (u8 *)KSEG1ADDR(0x1f011000);
#endif
#ifdef CONFIG_E380AC
	mac = (u8 *)KSEG1ADDR(0x1f020000);
	ee = (u8 *)KSEG1ADDR(0x1f021000);
#endif

#ifdef CONFIG_XD3200
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ee = (u8 *)KSEG1ADDR(0x1fff1000);
#endif

#ifdef CONFIG_AP120C
	mac = (u8 *)KSEG1ADDR(0x1fff1002);
	ee = (u8 *)KSEG1ADDR(0x1fff1000);
#endif
#ifdef CONFIG_LIMA
	mac = (u8 *)KSEG1ADDR(0x1f080000);
	ee = (u8 *)KSEG1ADDR(0x1f081000);
#endif

#ifdef CONFIG_WASP_SUPPORT
#define DB120_MAC0_OFFSET	0
#define DB120_MAC1_OFFSET	6
    #if defined(CONFIG_ARCHERC25)
//	u8 art = (u8 *)KSEG1ADDR(0x1fff0000);	
	mac = (u8 *)KSEG1ADDR(0x1f7e0008);
    #elif defined(CONFIG_ARCHERC7V5) && !defined(CONFIG_ARCHERA7V5)
	u8 *art = (u8 *)KSEG1ADDR(0x1f050000);
	ee = (u8 *)KSEG1ADDR(0x1f051000);
    #elif defined(CONFIG_DIR825C1)
	u8 *art = (u8 *)KSEG1ADDR(0x1fff1000);
    #elif defined(CONFIG_DW02_412H)
                u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
    #elif defined(CONFIG_WR841V8)
//              u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
    #elif defined(CONFIG_WR810N)
//              u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);
    #else
	u8 *art = (u8 *)KSEG1ADDR(0x1fff0000);
    #endif
	void __iomem *base;
	u32 t;

    #if !defined(CONFIG_WDR4300) && !defined(CONFIG_AP135) && !defined(CONFIG_UBNTXW) && !defined(CONFIG_DAP2230)
    #if defined(CONFIG_ARCHERC25)
//	u8 art = (u8 *)KSEG1ADDR(0x1fff0000);	
	mac = (u8 *)KSEG1ADDR(0x1f7e0008);
    #elif CONFIG_JWAP606
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_XD3200
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_AP120C
    	mac = (u8 *)KSEG1ADDR(0x1fff1000);
	ath79_init_mac(mac0, mac + ALFA_AP120C_MAC_OFFSET, 1);
	ath79_init_mac(mac1, mac + ALFA_AP120C_MAC_OFFSET, 2);	
    #elif CONFIG_E380AC
	mac = (u8 *)KSEG1ADDR(0x1f020000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_XD9531
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_WR615N
	mac = (u8 *)KSEG1ADDR(0x1f010000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_E355AC
	mac = (u8 *)KSEG1ADDR(0x1f010000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_E325N
	mac = (u8 *)KSEG1ADDR(0x1f010000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_WR650AC
	mac = (u8 *)KSEG1ADDR(0x1f020000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_UAPAC
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);	
    #elif CONFIG_DIR825C1
	#ifdef CONFIG_DIR859
	char tempmac[6];
	if (!scanmac(tempmac, (u8 *)KSEG1ADDR(0x1f050000),"wlan24mac=")) {
		printk(KERN_INFO "mac is dead, use fake mac\n");
		ath79_init_mac(mac0, "\x0\x1\x2\x3\x4\x5", 0);
	}else{
		ath79_init_mac(mac0, tempmac, 0);	
	}
	if (!scanmac(tempmac, (u8 *)KSEG1ADDR(0x1f050000),"wlan5mac=")) {
		printk(KERN_INFO "mac is dead, use fake mac\n");
		ath79_init_mac(mac1, "\x0\x1\x2\x3\x4\x5", 0);
	}else{
		ath79_init_mac(mac1, tempmac, 0);	
	}
	#else
	dir825b1_read_ascii_mac(mac0, DIR825C1_MAC_LOCATION_0);
	dir825b1_read_ascii_mac(mac1, DIR825C1_MAC_LOCATION_1);
	if (!memcmp(mac0,"\x0\x0\x0\x0\x0\x0",6)) {
		dir825b1_read_ascii_mac(mac0, DHP1565A1_MAC_LOCATION_0);
		dir825b1_read_ascii_mac(mac1, DHP1565A1_MAC_LOCATION_1);
	}
	#endif
    #endif

    #else
    
    #ifdef CONFIG_DAP2230
	char tempmac[6];
	if (!scanmac(tempmac, (u8 *)KSEG1ADDR(0x1f042000),"lanmac=")) {
	printk(KERN_INFO "mac is dead, use fake mac\n");
	ath79_init_mac(mac0, "\x0\x1\x2\x3\x4\x5", -1);
	ath79_init_mac(mac1, "\x0\x1\x2\x3\x4\x5", 0);
	}else {
	ath79_init_mac(mac0, tempmac, -1);
	ath79_init_mac(mac1, tempmac, 0);
	}
	printk(KERN_INFO "mac0 mac %02X:%02X:%02X:%02X:%02X:%02X\n",mac0[0]&0xff,mac0[1]&0xff,mac0[2]&0xff,mac0[3]&0xff,mac0[4]&0xff,mac0[5]&0xff);
	printk(KERN_INFO "mac1 mac %02X:%02X:%02X:%02X:%02X:%02X\n",mac1[0]&0xff,mac1[1]&0xff,mac1[2]&0xff,mac1[3]&0xff,mac1[4]&0xff,mac1[5]&0xff);
    #elif CONFIG_DAP3662
	char tempmac[6];
	if (!scanmac(tempmac, (u8 *)KSEG1ADDR(0x1f042000),"lanmac=")) {
	printk(KERN_INFO "mac is dead, use fake mac\n");
	ath79_init_mac(mac0, "\x0\x1\x2\x3\x4\x5", -1);
	ath79_init_mac(mac1, "\x0\x1\x2\x3\x4\x5", 0);
	}else {
	ath79_init_mac(mac0, tempmac, -1);
	ath79_init_mac(mac1, tempmac, 0);
	}
    #elif CONFIG_DIR862
	dir825b1_read_ascii_mac(mac0, DIR862_MAC_LOCATION_0);
	dir825b1_read_ascii_mac(mac1, DIR862_MAC_LOCATION_1);
	if (!memcmp(mac0,"\x0\x0\x0\x0\x0\x0",6)) {
		scanmac(mac0,(u8 *)KSEG1ADDR(0x1f040000),"lan_mac=");
		scanmac(mac1,(u8 *)KSEG1ADDR(0x1f040000),"wan_mac=");
	}
	
    #elif defined(CONFIG_ARCHERA7V5)
	mac = (u8 *)KSEG1ADDR(0x1ff40008);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif defined(CONFIG_ARCHERC7V5)
	mac = (u8 *)KSEG1ADDR(0x1f060008);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_ARCHERC7V4
	mac = (u8 *)KSEG1ADDR(0x1ff00008);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_WR1043V5
	mac = (u8 *)KSEG1ADDR(0x1ff00008);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_WR1043V4
	mac = (u8 *)KSEG1ADDR(0x1ff50008);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_WR1043V2
	mac = (u8 *)KSEG1ADDR(0x1f01fc00);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_UBNTXW
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #elif CONFIG_AP135
	mac = (u8 *)KSEG1ADDR(0x1fff0000);
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);
    #else
	ath79_init_mac(mac0, mac, -1);
	ath79_init_mac(mac1, mac, 0);    
    #endif
    #endif

    #ifdef CONFIG_DIR615I

	dir825b1_read_ascii_mac(mac0, DIR615I_MAC_LOCATION_0);
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);

	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE);

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
    #elif CONFIG_DAP2230
       ath79_setup_ar933x_phy4_switch(false, false);
    #elif CONFIG_WR941V6
    #elif CONFIG_WR841V9
    #ifndef CONFIG_ARCHERC25
       ath79_setup_ar933x_phy4_switch(false, false);
    #endif

    #elif CONFIG_JWAP606

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 | 0x14000);
    #elif CONFIG_WR810N
	base = ioremap(AR933X_GMAC_BASE, AR933X_GMAC_SIZE);
	t = __raw_readl(base + AR933X_GMAC_REG_ETH_CFG);
	t &= ~(AR933X_ETH_CFG_SW_PHY_SWAP | AR933X_ETH_CFG_SW_PHY_ADDR_SWAP);
	__raw_writel(t, base + AR933X_GMAC_REG_ETH_CFG);
	iounmap(base);
	
    #elif CONFIG_DW02_412H
//    	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN | QCA955X_ETH_CFG_GE0_SGMII  | (3 << QCA955X_ETH_CFG_RXD_DELAY_SHIFT) | (3 << QCA955X_ETH_CFG_RDV_DELAY_SHIFT) | (QCA955X_ETH_CFG_TXE_DELAY_SHIFT << 3) | (QCA955X_ETH_CFG_TXD_DELAY_SHIFT << 3));
    	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN);
    #elif CONFIG_RAMBUTAN
//	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);
    #elif CONFIG_LIMA
	//swap phy
	ath79_setup_ar933x_phy4_switch(true, true);
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
    #elif CONFIG_DAP3310
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);
	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_GMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE | AR934X_ETH_CFG_SW_PHY_SWAP);

	t |= AR934X_ETH_CFG_SW_PHY_SWAP;

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
    #elif CONFIG_XD3200
//    	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN);
    #elif CONFIG_E380AC
    	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN);
    #elif CONFIG_XD9531
    #elif CONFIG_WR615N
    #elif CONFIG_E325N
    #elif CONFIG_E355AC
    #elif CONFIG_WR650AC	
    	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN);


    #elif CONFIG_POWERBEAMAC_GEN2
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_setup_ar934x_eth_rx_delay(3, 3);

    #elif CONFIG_NANOAC
    #elif CONFIG_UAPAC
    	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN | QCA955X_ETH_CFG_GE0_SGMII | (3 << QCA955X_ETH_CFG_RXD_DELAY_SHIFT) | (3 << QCA955X_ETH_CFG_RDV_DELAY_SHIFT));
    #elif CONFIG_XWM400
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_setup_ar934x_eth_rx_delay(3, 3);

    #elif CONFIG_UBNTXW
	//swap phy
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);
	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_GMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE | AR934X_ETH_CFG_SW_PHY_SWAP);

	t |= AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0_SLAVE;

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
    #else
	#ifdef CONFIG_AP135
	#if !defined(CONFIG_MMS344) && !defined(CONFIG_ARCHERC7V4) && !defined(CONFIG_WR1043V4) || defined(CONFIG_DIR862)
	ap136_gmac_setup(QCA955X_ETH_CFG_RGMII_EN);
	#endif
	#else
#ifndef HAVE_DIR859
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);
	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 |
	       AR934X_ETH_CFG_MII_GMAC0 |
	       AR934X_ETH_CFG_GMII_GMAC0 |
	       AR934X_ETH_CFG_SW_ONLY_MODE |
	       AR934X_ETH_CFG_SW_PHY_SWAP);
	    #ifdef CONFIG_WDR3500
	t |= AR934X_ETH_CFG_SW_ONLY_MODE;
	    #elif CONFIG_WDR4300
	t |= AR934X_ETH_CFG_RGMII_GMAC0;
	    #else
	t |= AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE;
	    #endif

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);
#endif
	#endif
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
    #elif CONFIG_JWAP606
	ap91_wmac_disable_2ghz();
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, -1);

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_pll_data.pll_1000 = 0x0e000000;
	ar71xx_add_device_eth(0);
	
    #elif CONFIG_DAP3310
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

    #elif CONFIG_AP120C

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 |
							   BIT(15) | BIT(17) | BIT(19) | BIT(21));

	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 0);

	mdiobus_register_board_info(ap120c_mdio0_info, ARRAY_SIZE(ap120c_mdio0_info));

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = ALFA_AP120C_LAN_PHYMASK;

	ar71xx_eth0_pll_data.pll_1000 = 0x42000000;
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;

	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;

	ar71xx_add_device_eth(0);

    #elif CONFIG_E325N
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);

	ar71xx_add_device_mdio(1, 0x0);

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, 0);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, 2);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_switch_data.phy_poll_mask = BIT(0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio1_device.dev;
	ar71xx_add_device_eth(0);

	/* GMAC1 is connected to the internal switch */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;	
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_eth1_data.speed = SPEED_100;
	
	ar71xx_add_device_eth(1);
    #elif CONFIG_XD9531
	ath79_setup_ar933x_phy4_switch(false, false);

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 0);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 2);

	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_add_device_mdio(1, 0x0);

	// wan
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_add_device_eth(0);

	// lan
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;	
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.phy_mask |= BIT(4);
	ar71xx_switch_data.phy4_mii_en = 1;
	//ar71xx_switch_data.phy_classab_en = 1;
	ar71xx_add_device_eth(1);
    #elif CONFIG_WR615N
	ath79_setup_ar933x_phy4_switch(false, false);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 2);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 0);
	ar71xx_add_device_mdio(1, 0x0);	
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
//	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	// wan
	ar71xx_add_device_eth(1);

	// lan
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_add_device_eth(0);	
    #elif CONFIG_E355AC
	ath79_setup_ar933x_phy4_switch(false, false);

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 2);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 0);
	ar71xx_add_device_mdio(1, 0x0);	
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
//	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	// wan
	ar71xx_add_device_eth(1);

	// lan
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_add_device_eth(0);	
    #elif CONFIG_XD3200
#define CF_WR680AC_LAN_PHYMASK              BIT(0)
#define CF_WR680AC_WAN_PHYMASK              BIT(5)
	ar71xx_add_device_mdio(0, 0x0);
	mdiobus_register_board_info(ap152_mdio0_info,
				    ARRAY_SIZE(ap152_mdio0_info));

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	/* GMAC0 is connected to the RMGII interface */
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.phy_mask = BIT(0);
	
	ar71xx_add_device_eth(0);
    #elif CONFIG_E380AC
#define CF_WR680AC_LAN_PHYMASK              BIT(0)
#define CF_WR680AC_WAN_PHYMASK              BIT(5)
	ar71xx_add_device_mdio(0, 0x0);
	mdiobus_register_board_info(cf_e380ac_mdio0_info,
				    ARRAY_SIZE(cf_e380ac_mdio0_info));

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 3);
	/* GMAC0 is connected to the RMGII interface */
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_pll_data.pll_10 = 0xB0001313;
	ar71xx_eth0_pll_data.pll_100 = 0xB0000101;
	ar71xx_eth0_pll_data.pll_1000 = 0xBE000000;	
	
	ar71xx_add_device_eth(0);
    #elif CONFIG_WR650AC
#define CF_WR650AC_LAN_PHYMASK              BIT(0)
#define CF_WR650AC_WAN_PHYMASK              BIT(5)
	ar71xx_add_device_mdio(0, 0x0);

	mdiobus_register_board_info(cf_wr650ac_mdio0_info,
				    ARRAY_SIZE(cf_wr650ac_mdio0_info));

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 3);
	/* GMAC0 is connected to the RMGII interface */
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = CF_WR650AC_LAN_PHYMASK;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_pll_data.pll_1000 = 0xa6000000;
	ar71xx_add_device_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_eth1_pll_data.pll_1000 = 0x03000101;
	ar71xx_add_device_eth(1);
    #elif CONFIG_POWERBEAMAC_GEN2
	mdiobus_register_board_info(ubnt_rocket_m_ti_mdio_info,
			ARRAY_SIZE(ubnt_rocket_m_ti_mdio_info));

	ar71xx_add_device_mdio(0, ~BIT(4));	

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_eth0_pll_data.pll_1000 = 0x02000000;
	ar71xx_eth0_pll_data.pll_100 = 0x00000101;
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;
	ar71xx_add_device_eth(0);
    
    
    
    #elif CONFIG_NANOAC

//	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE);
	ath79_setup_ar934x_eth_rx_delay(2, 2);

	ar71xx_add_device_mdio(0, 0);	

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_pll_data.pll_1000 = 0x06000000;
	ar71xx_eth0_pll_data.pll_100 = 0x00000101;
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;

//	mdiobus_register_board_info(at803_mdio_info,
//			ARRAY_SIZE(at803_mdio_info));
	mdiobus_register_board_info(nanoac_mdio0_info,
	                            ARRAY_SIZE(nanoac_mdio0_info));

	ar71xx_add_device_eth(0);
/*	ar71xx_add_device_mdio(1, 0x0);
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_eth1_data.mii_bus_dev = &ar71xx_mdio1_device.dev;
	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_eth1_data.phy_mask = BIT(4);
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_add_device_eth(1);*/
	//#error "check"
    #elif CONFIG_UAPACPRO
	ar71xx_add_device_mdio(0, 0);	

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;


	mdiobus_register_board_info(ubnt_unifiac_pro_mdio0_info,
	                            ARRAY_SIZE(ubnt_unifiac_pro_mdio0_info));

	ar71xx_add_device_eth(0);
    #elif CONFIG_UAPAC
	ar71xx_add_device_mdio(0, ~BIT(4));	

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;


	mdiobus_register_board_info(at803_mdio_info,
			ARRAY_SIZE(at803_mdio_info));

	ar71xx_add_device_eth(0);
    #elif CONFIG_UBNTXW
	#ifdef CONFIG_XWLOCO
	mdiobus_register_board_info(ubnt_loco_m_xw_mdio_info,
				    ARRAY_SIZE(ubnt_loco_m_xw_mdio_info));
	ar71xx_add_device_mdio(0, ~BIT(1));
	ar71xx_eth0_data.phy_mask = BIT(1);	
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	#elif CONFIG_XWM400
	mdiobus_register_board_info(ubnt_rocket_m_ti_mdio_info,
			ARRAY_SIZE(ubnt_rocket_m_ti_mdio_info));
	
	ar71xx_add_device_mdio(0, ~BIT(4));

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_eth0_pll_data.pll_1000 = 0x2000000;
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;	
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
	#else
	ar71xx_add_device_mdio(0, ~(BIT(0) | BIT(1) | BIT(5)));
	ar71xx_eth0_data.phy_mask = (BIT(0) | BIT(1) | BIT(5));
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	#endif
	ar71xx_add_device_eth(0);
    #elif CONFIG_DAP2230
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 0);

	/* LAN */
	ar71xx_add_device_eth(1);
	/* WAN */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_add_device_eth(0);
    #elif CONFIG_WR941V6
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, -1);
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_add_device_eth(1);
	/* WAN */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_add_device_eth(0);
    #elif CONFIG_WR841V9
//	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_add_device_mdio(1, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, 0);

	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_switch_data.phy_poll_mask |= BIT(4);
	/* LAN */
	ar71xx_add_device_eth(1);

	/* WAN */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_add_device_eth(0);
	
#ifdef CONFIG_ARCHERC25
	spi_register_board_info(archer_c25_v1_spi_info,
				ARRAY_SIZE(archer_c25_v1_spi_info));

	platform_device_register(&archer_c25_v1_spi_device);

	gpio_request_one(ARCHER_C25_GPIO_SHIFT_OE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "LED control");

	gpio_request_one(ARCHER_C25_GPIO_SHIFT_SRCLR,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "LED reset");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c25_v1_leds_gpio),
				 archer_c25_v1_leds_gpio);

#endif
    #elif CONFIG_WR810N
	ar71xx_add_device_mdio(1, 0x0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, -1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, 0);


	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_add_device_eth(0);

	/* GMAC1 is connected to the internal switch */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_switch_data.phy_poll_mask |= BIT(4);
	ar71xx_add_device_eth(1);
    #elif CONFIG_DW02_412H
	mdiobus_register_board_info(wdr4300_mdio0_info,
				    ARRAY_SIZE(wdr4300_mdio0_info));
	ar71xx_add_device_mdio(0, 0x0);

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_pll_data.pll_1000 = 0xa6000000;
	ar71xx_eth0_pll_data.pll_100 = 0x101;
	ar71xx_eth0_pll_data.pll_10 = 0x1616;
	ar71xx_add_device_eth(0);

    #elif CONFIG_RAMBUTAN
	mdiobus_register_board_info(rambutan_mdio0_info,
				    ARRAY_SIZE(rambutan_mdio0_info));
	mdiobus_register_board_info(rambutan_mdio1_info,
				    ARRAY_SIZE(rambutan_mdio1_info));
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_add_device_mdio(1, 0x0);

	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_add_device_eth(0);

	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth1_data.phy_mask = BIT(0);
	ar71xx_eth1_data.mii_bus_dev = &ar71xx_mdio1_device.dev;
	ar71xx_eth1_pll_data.pll_1000 = 0x17000000;
	ar71xx_eth1_pll_data.pll_10 = 0x1313;
	ar71xx_add_device_eth(1);

    #elif CONFIG_LIMA
#define LIMA_ETH_PHYS		(BIT(0) | BIT(1))
	ar71xx_add_device_mdio(1, ~LIMA_ETH_PHYS);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac + 0x0006, 0);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, 0);

	/* GMAC0 is connected to the PHY0 of the internal switch */
	ar71xx_switch_data.phy4_mii_en = 1;


	ar71xx_switch_data.phy_poll_mask |= BIT(0);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.speed = SPEED_100;
	ar71xx_add_device_eth(0);

	/* GMAC1 is connected to the internal switch */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_eth1_data.phy_mask = BIT(1);
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
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
	#ifdef CONFIG_AP135
	#ifdef CONFIG_ARCHERC7V4
	ap152_mdio_setup();
	mdiobus_register_board_info(ap152_mdio0_info,ARRAY_SIZE(ap152_mdio0_info));

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	/* GMAC0 is connected to the RMGII interface */
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_add_device_eth(0);
	
	#elif CONFIG_WR1043V4
	ar71xx_add_device_mdio(0, 0x0);
	mdiobus_register_board_info(ap152_mdio0_info,ARRAY_SIZE(ap152_mdio0_info));

	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	/* GMAC0 is connected to the RMGII interface */
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_add_device_eth(0);
	
	
	
	#else
	#ifdef CONFIG_DAP2330
//	pll-data = <0x82000000 0x80000101 0x80001313>;

//	ath79_setup_ar934x_eth_rx_delay(3, 3);
	mdiobus_register_board_info(dap2330_mdio0_info,
				    ARRAY_SIZE(dap2330_mdio0_info));

	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_eth0_pll_data.pll_10 = 0x81001313;
	ar71xx_eth0_pll_data.pll_100 = 0x81000101;
	ar71xx_eth0_pll_data.pll_1000 = 0x8f000000;

//	ar71xx_eth0_pll_data.pll_10 = 0x80001313;
//	ar71xx_eth0_pll_data.pll_100 = 0x80000101;
//	ar71xx_eth0_pll_data.pll_1000 = 0x82000000;
	ar71xx_add_device_eth(0);
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 0);
	#else
	
	#if defined(CONFIG_MMS344) && !defined(CONFIG_DIR862)
	base = ioremap(AR934X_GMAC_BASE, AR934X_GMAC_SIZE);
	t = __raw_readl(base + AR934X_GMAC_REG_ETH_CFG);

	t &= ~(AR934X_ETH_CFG_RGMII_GMAC0 |
	       AR934X_ETH_CFG_MII_GMAC0 |
	       AR934X_ETH_CFG_GMII_GMAC0 |
	       AR934X_ETH_CFG_SW_ONLY_MODE |
	       AR934X_ETH_CFG_SW_PHY_SWAP);
	t |= AR934X_ETH_CFG_RGMII_GMAC0 | AR934X_ETH_CFG_SW_ONLY_MODE;

	__raw_writel(t, base + AR934X_GMAC_REG_ETH_CFG);
	/* flush write */
	__raw_readl(base + AR934X_GMAC_REG_ETH_CFG);
	iounmap(base);



	ar71xx_add_device_mdio(0, 0x0);

	/* GMAC0 is connected to an AR8327 switch */
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;

#ifdef CONFIG_WILLY
	mdiobus_register_board_info(cf_e380ac_mdio0_info,
				    ARRAY_SIZE(cf_e380ac_mdio0_info));
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_pll_data.pll_10 = 0x00001313;
	ar71xx_eth0_pll_data.pll_100 = 0x00000101;
	ar71xx_eth0_pll_data.pll_1000 = 0x0e000000;
#else
	mdiobus_register_board_info(wpj344_mdio0_info,
					ARRAY_SIZE(wpj344_mdio0_info));

	ar71xx_eth0_pll_data.pll_1000 = 0x06000000;
	ar71xx_add_device_mdio(1, 0x0);
#endif
	ar71xx_add_device_eth(0);
	#ifndef CONFIG_WILLY
	/* GMAC1 is connected to the internal switch */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;
	ar71xx_add_device_eth(1);
	#endif
	#else
	/* GMAC0 of the AR8327 switch is connected to GMAC1 via SGMII */
	ap136_ar8327_pad0_cfg.mode = AR8327_PAD_MAC_SGMII;
	ap136_ar8327_pad0_cfg.sgmii_delay_en = true;
	ar71xx_add_device_mdio(0, 0x0);
	ar71xx_eth0_pll_data.pll_1000 = 0x56000000;
	ar71xx_eth1_pll_data.pll_1000 = 0x03000101;


	/* GMAC6 of the AR8327 switch is connected to GMAC0 via RGMII */
	ap136_ar8327_pad6_cfg.mode = AR8327_PAD_MAC_RGMII;
	ap136_ar8327_pad6_cfg.txclk_delay_en = true;
	ap136_ar8327_pad6_cfg.rxclk_delay_en = true;
	ap136_ar8327_pad6_cfg.txclk_delay_sel = AR8327_CLK_DELAY_SEL1;
	ap136_ar8327_pad6_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2;


	mdiobus_register_board_info(ap136_mdio0_info,
				    ARRAY_SIZE(ap136_mdio0_info));

	/* GMAC0 is connected to the RMGII interface */
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;

	ar71xx_add_device_eth(0);
	/* GMAC1 is connected tot eh SGMII interface */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth1_data.speed = SPEED_1000;
	ar71xx_eth1_data.duplex = DUPLEX_FULL;

	ar71xx_eth1_pll_data.pll_1000 = 0x03000101;
	ar71xx_add_device_eth(1);
	
	#endif
	#endif
	    

	    #ifdef CONFIG_DIR862
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac1, 0);
	    #elif CONFIG_ARCHERC7V4
	    #elif CONFIG_WR1043V4
	    #elif CONFIG_WR1043V2
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, 1);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, 0);
	    #else
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac + DB120_MAC0_OFFSET, 0);
	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac + DB120_MAC1_OFFSET, 0);
	    #endif

	#endif
	#else
	    #ifdef CONFIG_WDR3500
	ar71xx_add_device_mdio(1, 0x0);

	ar71xx_init_mac(ar71xx_eth1_data.mac_addr, mac, -1);

	/* GMAC1 is connected to the internal switch */
	ar71xx_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;

	ar71xx_add_device_eth(1);

	/* WAN */
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac, 2);

	/* GMAC0 is connected to the PHY4 of the internal switch */
	ar71xx_switch_data.phy4_mii_en = 1;
	ar71xx_switch_data.phy_poll_mask = BIT(4);
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ar71xx_eth0_data.phy_mask = BIT(4);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio1_device.dev;

	ar71xx_add_device_eth(0);
	    #else
		#ifndef CONFIG_WDR4300
		    #ifndef CONFIG_DIR825C1
	ar71xx_add_device_mdio(1, 0x0);
		    #endif
		#endif
	#ifndef CONFIG_WR650AC
	#ifndef CONFIG_E355AC
	#ifndef CONFIG_XD9531
	#ifndef CONFIG_WR615N
	#ifndef CONFIG_E380AC
	#ifndef CONFIG_AP120C
	#ifndef CONFIG_XD3200
	#ifndef CONFIG_E325N
	#ifdef CONFIG_DIR859
	printk(KERN_INFO "gpio mdio for AP152\n");
	ath79_gpio_output_select(3,33);
	ath79_gpio_output_select(4,32);
	ar71xx_add_device_mdio(1, 0x0);
	#endif
	ar71xx_add_device_mdio(0, 0x0);
		#ifdef CONFIG_MMS344
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);		
		#elif CONFIG_DIR825C1
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, mac0, 0);
		#else
	ar71xx_init_mac(ar71xx_eth0_data.mac_addr, art + DB120_MAC0_OFFSET, 0);
		#endif

		#ifdef CONFIG_WDR4300
	mdiobus_register_board_info(wdr4300_mdio0_info, ARRAY_SIZE(wdr4300_mdio0_info));
		#elif CONFIG_DIR859
	mdiobus_register_board_info(ap152_mdio0_info, ARRAY_SIZE(ap152_mdio0_info));		
		#else
	mdiobus_register_board_info(db120_mdio0_info, ARRAY_SIZE(db120_mdio0_info));
		#endif
	#ifdef CONFIG_DIR859	
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.speed = SPEED_1000;
	ar71xx_eth0_data.duplex = DUPLEX_FULL;
	ar71xx_eth0_data.force_link = 1;
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
//	ar71xx_eth0_pll_data.pll_1000 = 0x06000000;
	ar71xx_add_device_eth(0);
	#else		
	ar71xx_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ar71xx_eth0_data.phy_mask = BIT(0);
	ar71xx_eth0_data.mii_bus_dev = &ar71xx_mdio0_device.dev;
	ar71xx_eth0_pll_data.pll_1000 = 0x06000000;
	ar71xx_add_device_eth(0);
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
	#endif
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
    #endif
#endif
#if CONFIG_FMS2111
	u32 *ar9_gpiof2;
	ar9_gpiof2 = (u32 *) KSEG1ADDR(0x18040030);
	*ar9_gpiof2 |= 0x70300; 
	ret = platform_add_devices(ar724x_platform_devices, ARRAY_SIZE(ar724x_platform_devices));
#else
	ret = platform_add_devices(ar724x_platform_devices, ARRAY_SIZE(ar724x_platform_devices));
#endif

	if (ret < 0)
		return ret;

	if (is_ar7241() || is_ar7242() || is_ar934x()) {
		ret = platform_add_devices(ar7241_platform_devices, ARRAY_SIZE(ar7241_platform_devices));
	}
	if (is_ar7240()) {
		ret = platform_add_devices(ar7240_platform_devices, ARRAY_SIZE(ar7240_platform_devices));
	}

	if (is_ar933x()) {
		ar933x_usb_setup();
	}
	if (is_qca955x() || is_qca956x() || is_qca953x()) {
		qca_usbregister();
	}

	platform_device_register_simple("ar71xx-wdt", -1, NULL, 0);

#ifdef CONFIG_MACH_HORNET
	ee = (u8 *)KSEG1ADDR(0x1fff1000);
	ar9xxx_add_device_wmac(ee, mac);
#elif CONFIG_WASP_SUPPORT
#if !defined(CONFIG_MTD_NAND_ATH) || defined(CONFIG_DW02_412H)
	if (!ee)
	    ee = (u8 *)KSEG1ADDR(0x1fff1000);
#if defined(CONFIG_DIR862)
	ar9xxx_add_device_wmac(ee, mac0);
#elif defined(CONFIG_XD9531)
	ar9xxx_add_device_wmac(ee, ee + 2);
#elif defined(CONFIG_MMS344)
	ar9xxx_add_device_wmac(ee, art + 2);
#elif defined(CONFIG_DIR825C1)
	ar9xxx_add_device_wmac(ee, mac0);
#elif defined(CONFIG_DIR615I)
	ar9xxx_add_device_wmac(ee, mac0);
#elif defined(CONFIG_DAP2230)
	printk(KERN_INFO "wmac mac %02X:%02X:%02X:%02X:%02X:%02X\n",mac0[0]&0xff,mac0[1]&0xff,mac0[2]&0xff,mac0[3]&0xff,mac0[4]&0xff,mac0[5]&0xff);
	ar9xxx_add_device_wmac(ee, mac0);
#elif defined(CONFIG_WR841V9)
	ar9xxx_add_device_wmac(ee, mac);
#elif defined(CONFIG_WR810N)
	ar9xxx_add_device_wmac(ee, mac);
#elif defined(CONFIG_LIMA)
	ar9xxx_add_device_wmac(ee, ee + 2);
#elif defined(CONFIG_WR841V8)
	ar9xxx_add_device_wmac(ee, mac);
#else
	ar9xxx_add_device_wmac(ee, NULL);
#endif

#if defined(CONFIG_AP120C)
	ap91_pci_init(NULL, mac0);
	ap91_set_eeprom();

#elif defined(CONFIG_DIR825C1)
	ap91_pci_init(ee + 0x4000, mac1);
#elif defined(CONFIG_ARCHERC25)
	ap91_pci_init(NULL, NULL);
#elif defined(CONFIG_DW02_412H)
	ap91_pci_init(NULL, NULL);
#elif !defined(CONFIG_DIR615I) && !defined(CONFIG_WR841V8) || defined(CONFIG_LIMA)
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
#if defined(CONFIG_WZRG450NH) && !defined(CONFIG_WZRG300NH2)
	ap91_set_tx_gain_buffalo();
#endif
	return ret;
}
int pcibios_init(void);

#if defined(CONFIG_MTD_NAND_ATH)
void nand_postinit(struct mtd_info *mtd)
{
#ifndef CONFIG_DW02_412H
	u8 *ee = (u8 *)kmalloc(0x9000, GFP_ATOMIC);
	int i;
	int mtdlen;
#ifdef CONFIG_RAMBUTAN
	mtd_read(mtd, 0x500000, 0x9000, &mtdlen, ee);
	ar9xxx_add_device_wmac(ee + 0x1000, ee + 6);
	ap91_pci_init(NULL, NULL);
#else
	mtd_read(mtd, 0x80000, 0x9000, &mtdlen, ee);
	ar9xxx_add_device_wmac(ee + 0x1000, ee + 6);
	ap91_pci_init(ee + 0x5000, ee + 12);
#endif
#endif
}
#endif
arch_initcall(ar7240_platform_init);
