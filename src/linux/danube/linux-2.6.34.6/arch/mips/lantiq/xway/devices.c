/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/mtd/physmap.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/etherdevice.h>
#include <linux/reboot.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/leds.h>

#include <asm/bootinfo.h>
#include <asm/irq.h>

#include <xway.h>
#include <xway_irq.h>
#include <lantiq_platform.h>

#define IRQ_RES(resname,irq) {.name=#resname,.start=(irq),.flags=IORESOURCE_IRQ}

/* gpio leds */
static struct gpio_led_platform_data lq_gpio_led_data;

static struct platform_device lq_gpio_leds =
{
	.name = "leds-gpio",
	.dev = {
		.platform_data = (void *) &lq_gpio_led_data,
	}
};

void __init
lq_register_gpio_leds(struct gpio_led *leds, int cnt)
{
	lq_gpio_led_data.leds = leds;
	lq_gpio_led_data.num_leds = cnt;
	platform_device_register(&lq_gpio_leds);
}

/* serial to parallel conversion */
static struct resource lq_stp_resource =
{
	.name	= "stp",
	.start	= LQ_STP_BASE,
	.end	= LQ_STP_BASE + LQ_STP_SIZE - 1,
	.flags	= IORESOURCE_MEM,
};

void __init
lq_register_gpio_stp(void)
{
	platform_device_register_simple("lq_stp", 0, &lq_stp_resource, 1);
}

/* nor flash */
static struct resource lq_nor_resource =
{
	.name	= "nor",
	.start	= LQ_FLASH_START,
	.end	= LQ_FLASH_START + LQ_FLASH_MAX - 1,
	.flags  = IORESOURCE_MEM,
};

static struct platform_device lq_nor =
{
	.name			= "lq_nor",
	.resource		= &lq_nor_resource,
	.num_resources	= 1,
};

void __init
lq_register_nor(struct physmap_flash_data *data)
{
	lq_nor.dev.platform_data = data;
	platform_device_register(&lq_nor);
}

/* watchdog */
static struct resource lq_wdt_resource =
{
	.name	= "watchdog",
	.start  = LQ_WDT_BASE,
	.end    = LQ_WDT_BASE + LQ_WDT_SIZE - 1,
	.flags  = IORESOURCE_MEM,
};

void __init
lq_register_wdt(void)
{
	platform_device_register_simple("lq_wdt", 0, &lq_wdt_resource, 1);
}

/* gpio */
static struct resource lq_gpio_resource[] = {
	{
		.name	= "gpio0",
		.start  = LQ_GPIO0_BASE_ADDR,
		.end    = LQ_GPIO0_BASE_ADDR + LQ_GPIO_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}, {
		.name	= "gpio1",
		.start  = LQ_GPIO1_BASE_ADDR,
		.end    = LQ_GPIO1_BASE_ADDR + LQ_GPIO_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	}
};

void __init
lq_register_gpio(void)
{
	platform_device_register_simple("lq_gpio", 0, &lq_gpio_resource[0], 1);
	platform_device_register_simple("lq_gpio", 1, &lq_gpio_resource[1], 1);
}

/* pci */
static struct platform_device lq_pci =
{
	.name			= "lq_pci",
	.num_resources	= 0,
};

void __init
lq_register_pci(struct lq_pci_data *data)
{
	lq_pci.dev.platform_data = data;
	platform_device_register(&lq_pci);
}

/* ebu */
static struct resource lq_ebu_resource =
{
	.name	= "gpio_ebu",
	.start	= LQ_EBU_GPIO_START,
	.end	= LQ_EBU_GPIO_START + LQ_EBU_GPIO_SIZE - 1,
	.flags	= IORESOURCE_MEM,
};

void __init
lq_register_gpio_ebu(unsigned int value)
{
	platform_device_register_simple("lq_ebu", 0, &lq_ebu_resource, 1);
}

/* ethernet */
unsigned char lq_ethaddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

static int __init
lq_set_ethaddr(char *str)
{
	sscanf(&str[8], "0%02hhx:0%02hhx:0%02hhx:0%02hhx:0%02hhx:0%02hhx",
		&lq_ethaddr[0], &lq_ethaddr[1], &lq_ethaddr[2],
		&lq_ethaddr[3], &lq_ethaddr[4], &lq_ethaddr[5]);
	return 0;
}
__setup("ethaddr=", lq_set_ethaddr);

static struct resource lq_ethernet_resources =
{
	.name	= "etop",
	.start  = LQ_PPE32_BASE_ADDR,
	.end    = LQ_PPE32_BASE_ADDR + LQ_PPE32_SIZE - 1,
	.flags  = IORESOURCE_MEM,
};

static struct platform_device lq_ethernet =
{
	.name			= "lq_etop",
	.resource		= &lq_ethernet_resources,
	.num_resources	= 1,
};

void __init
lq_register_ethernet(struct lq_eth_data *eth)
{
	if(!eth)
		return;
	if(!eth->mac)
		eth->mac = lq_ethaddr;
	if(!is_valid_ether_addr(eth->mac))
		random_ether_addr(eth->mac);
	lq_ethernet.dev.platform_data = eth;
	platform_device_register(&lq_ethernet);
}

/* tapi */
static struct resource mps_resources[] = {
	{
		.name = "voice-mem",
		.flags = IORESOURCE_MEM,
		.start = 0x1f107000,
		.end =   0x1f1073ff,
	},
	{
		.name = "voice-mailbox",
		.flags = IORESOURCE_MEM,
		.start = 0x1f200000,
		.end =   0x1f2007ff,
	},
};

static struct platform_device mps_device = {
	.name = "mps",
	.resource = mps_resources,
	.num_resources = ARRAY_SIZE(mps_resources),
};

static struct platform_device vmmc_device = {
	.name = "vmmc",
	.dev = {
		.parent = &mps_device.dev,
	},
};

void __init
lq_register_tapi(void)
{
#define CP1_SIZE	(1 << 20)
	dma_addr_t dma;
	mps_device.dev.platform_data =
		(void*)CPHYSADDR(dma_alloc_coherent(NULL, CP1_SIZE, &dma, GFP_ATOMIC));
	platform_device_register(&mps_device);
	platform_device_register(&vmmc_device);
}

/* asc ports */
static struct resource lq_asc0_resources[] =
{
	{
		.start  = LQ_ASC0_BASE,
		.end    = LQ_ASC0_BASE + LQ_ASC_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	IRQ_RES(tx, INT_NUM_IM3_IRL0),
	IRQ_RES(rx, INT_NUM_IM3_IRL0 + 1),
	IRQ_RES(err, INT_NUM_IM3_IRL0 + 2),
};

static struct resource lq_asc1_resources[] =
{
	{
		.start  = LQ_ASC1_BASE,
		.end    = LQ_ASC1_BASE + LQ_ASC_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	IRQ_RES(tx, INT_NUM_IM3_IRL0 + 8),
	IRQ_RES(rx, INT_NUM_IM3_IRL0 + 9),
	IRQ_RES(err, INT_NUM_IM3_IRL0 + 10),
};

void __init
lq_register_asc(int port)
{
	switch (port) {
	case 0:
		platform_device_register_simple("lq_asc", 0,
			lq_asc0_resources, ARRAY_SIZE(lq_asc0_resources));
		break;
	case 1:
		platform_device_register_simple("lq_asc", 1,
			lq_asc1_resources, ARRAY_SIZE(lq_asc1_resources));
		break;
	default:
		break;
	}
}

void __init
lq_register_crypto(const char *name)
{
	platform_device_register_simple(name, 0, 0, 0);
}
