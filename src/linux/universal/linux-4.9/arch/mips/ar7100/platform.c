/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *  Copyright (C) 2007 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
 *  some portions:
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
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

#include <asm/mach-ar7100/ar7100.h>
#include <asm/mips_machine.h>
#include <linux/rtl8366.h>
#include <linux/ath9k_platform.h>

extern uint32_t ar71xx_ahb_freq;

/* 
 * OHCI (USB full speed host controller) 
 */
static struct resource ar7100_usb_ohci_resources[] = {
	[0] = {
	       .start = AR7100_USB_OHCI_BASE,
	       .end = AR7100_USB_OHCI_BASE + AR7100_USB_WINDOW - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = AR7100_MISC_IRQ_USB_OHCI,
	       .end = AR7100_MISC_IRQ_USB_OHCI,
	       .flags = IORESOURCE_IRQ,
	       },
};

/* 
 * The dmamask must be set for OHCI to work 
 */
static u64 ohci_dmamask = ~(u32)0;

static struct platform_device ar7100_usb_ohci_device = {
	.name = "ar71xx-ohci",
	.id = 0,
	.dev = {
		.dma_mask = &ohci_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = ARRAY_SIZE(ar7100_usb_ohci_resources),
	.resource = ar7100_usb_ohci_resources,
};

/* 
 * EHCI (USB full speed host controller) 
 */
static struct resource ar7100_usb_ehci_resources[] = {
	[0] = {
	       .start = AR7100_USB_EHCI_BASE,
	       .end = AR7100_USB_EHCI_BASE + AR7100_USB_WINDOW - 1,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = AR7100_CPU_IRQ_USB,
	       .end = AR7100_CPU_IRQ_USB,
	       .flags = IORESOURCE_IRQ,
	       },
};

/* 
 * The dmamask must be set for EHCI to work 
 */
static u64 ehci_dmamask = ~(u32)0;

static struct platform_device ar7100_usb_ehci_device = {
	.name = "ar71xx-ehci",
	.id = 0,
	.dev = {
		.dma_mask = &ehci_dmamask,
		.coherent_dma_mask = 0xffffffff,
		},
	.num_resources = ARRAY_SIZE(ar7100_usb_ehci_resources),
	.resource = ar7100_usb_ehci_resources,
};

static struct resource ar7100_uart_resources[] = {
	{
	 .start = AR7100_UART_BASE,
	 .end = AR7100_UART_BASE + 0x0fff,
	 .flags = IORESOURCE_MEM,
	 },
};

extern unsigned int ar7100_serial_in(int offset);
extern void ar7100_serial_out(int offset, int value);
unsigned int ar7100_plat_serial_in(struct uart_port *up, int offset)
{
	return ar7100_serial_in(offset);
}

void ar7100_plat_serial_out(struct uart_port *up, int offset, int value)
{
	ar7100_serial_out(offset, value);

}

#define AR71XX_UART_FLAGS (UPF_BOOT_AUTOCONF | UPF_SKIP_TEST | UPF_IOREMAP)

static struct plat_serial8250_port ar7100_uart_data[] = {
	{
	 .mapbase = AR7100_UART_BASE,
	 .irq = AR7100_MISC_IRQ_UART,
	 .flags = AR71XX_UART_FLAGS,
	 .iotype = UPIO_MEM32,
	 .regshift = 2,
	 .uartclk = 0,		/* ar7100_ahb_freq, */
	 },
	{},
};

static struct platform_device ar7100_uart = {
	.name = "serial8250",
	.id = 0,
	.dev.platform_data = ar7100_uart_data,
	.num_resources = 1,
	.resource = ar7100_uart_resources
};

#if !defined(CONFIG_AG71XX) && !defined(CONFIG_AG71XX_MODULE)

#define TL_WR1043ND_GPIO_RTL8366_SDA    18
#define TL_WR1043ND_GPIO_RTL8366_SCK    19

static struct rtl8366_platform_data tl_wr1043nd_rtl8366_smi_data = {
	.gpio_sda = TL_WR1043ND_GPIO_RTL8366_SDA,
	.gpio_sck = TL_WR1043ND_GPIO_RTL8366_SCK,
};

static struct platform_device tl_wr1043nd_rtl8366_smi_device = {
	.name = "rtl8366rb",
	.id = -1,
	.dev = {
		.platform_data = &tl_wr1043nd_rtl8366_smi_data,
		}
};

#endif

#ifdef CONFIG_AR9100
#define RESET_MODULE_AMBA2WMAC		BIT(22)

static int ar913x_wmac_reset(void)
{
	ar7100_reset(RESET_MODULE_AMBA2WMAC);
	return 0;
}

static struct ath9k_platform_data ath9k_pdata = {
	.macaddr = (u8 *)ath9k_pdata.eeprom_data + 0x20c,
	.external_reset = ar913x_wmac_reset,
};

static struct resource ath9k_wmac_res[] = {
	{
	 .start = AR9100_WMAC_BASE,
	 .end = AR9100_WMAC_BASE + AR9100_WMAC_LEN - 1,
	 .flags = IORESOURCE_MEM,
	 },
	{
	 .start = AR7100_CPU_IRQ_WMAC,
	 .end = AR7100_CPU_IRQ_WMAC,
	 .flags = IORESOURCE_IRQ,
	 },
};

static struct platform_device ath9k_platform_device = {
	.name = "ath9k",
	.id = -1,
	.resource = ath9k_wmac_res,
	.num_resources = ARRAY_SIZE(ath9k_wmac_res),
	.dev = {
		.platform_data = &ath9k_pdata,
		},
};
#endif

static struct platform_device *ar7100_platform_devices[] __initdata = {
	&ar7100_usb_ohci_device,
	&ar7100_usb_ehci_device,
	&ar7100_uart,
#ifdef CONFIG_AR9100
	&ath9k_platform_device
#endif
};

extern void ar7100_serial_setup(void);

#define AR71XX_USB_RESET_MASK \
	(RESET_MODULE_USB_HOST | RESET_MODULE_USB_PHY \
	| RESET_MODULE_USB_OHCI_DLL)

static char wmac_mac[6];

static void read_ascii_mac(u8 *dest, unsigned int src_addr)
{
	int ret;
	u8 *src = (u8 *)KSEG1ADDR(src_addr);
	if (src[0] == 0xff)
		ret = 0;
	else
		ret = sscanf(src, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]);

	if (ret != 6)
		memset(dest, 0, 6);
}

int __init ar7100_platform_init(void)
{
#ifdef CONFIG_AR9100
	memcpy(&ath9k_pdata.eeprom_data, (void *)KSEG1ADDR(0x1fff1000), sizeof(ath9k_pdata.eeprom_data));
#endif
#ifdef CONFIG_WA901
	u8 *mac = (u8 *)KSEG1ADDR(0x1f01fc00);
	memcpy(wmac_mac, mac, sizeof(wmac_mac));
	ath9k_pdata.macaddr = wmac_mac;
#elif CONFIG_WR941
	u8 *mac = (u8 *)KSEG1ADDR(0x1f01fc00);
	memcpy(wmac_mac, mac, sizeof(wmac_mac));
	ath9k_pdata.macaddr = wmac_mac;
#endif
#ifdef HAVE_E2100
	unsigned int firstoffset = 0x1f03f29a;
	unsigned int secondoffset = 0x1f03f288;
#else
	unsigned int firstoffset = 0x1f03f288;
	unsigned int secondoffset = 0x1f03f29a;
#endif

#ifdef CONFIG_WRT160NL
	read_ascii_mac(wmac_mac, firstoffset);
	if (wmac_mac[0] == 0xff)
		read_ascii_mac(wmac_mac, secondoffset);

	ath9k_pdata.macaddr = wmac_mac;
#endif
#ifdef CONFIG_E2100L
	read_ascii_mac(wmac_mac, firstoffset);
	if (wmac_mac[0] == 0xff)
		read_ascii_mac(wmac_mac, secondoffset);
	ath9k_pdata.macaddr = wmac_mac;
#endif

	/* need to set clock appropriately */
	ar7100_uart_data[0].uartclk = ar71xx_ahb_freq;

	platform_add_devices(ar7100_platform_devices, ARRAY_SIZE(ar7100_platform_devices));

#if !defined(CONFIG_AG71XX) && !defined(CONFIG_AG71XX_MODULE)
#ifdef CONFIG_RTL8366_SMI
	platform_device_register(&tl_wr1043nd_rtl8366_smi_device);
#endif
#ifdef CONFIG_RTL8366_SMI_MODULE
	platform_device_register(&tl_wr1043nd_rtl8366_smi_device);
#endif
#endif
	platform_device_register_simple("ar71xx-wdt", -1, NULL, 0);

//      mips_machine_setup();
	return 0;
}

arch_initcall(ar7100_platform_init);
