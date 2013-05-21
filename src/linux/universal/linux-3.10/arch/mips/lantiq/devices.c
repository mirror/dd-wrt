/*
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 *  Copyright (C) 2010 John Crispin <blogic@openwrt.org>
 */

#include <linux/init.h>
#include <linux/export.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/etherdevice.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/dma-mapping.h>

#include <asm/bootinfo.h>
#include <asm/irq.h>

#include <lantiq_soc.h>

#include "devices.h"

/* nor flash */
static struct resource ltq_nor_resource =
	MEM_RES("nor", LTQ_FLASH_START, LTQ_FLASH_MAX);

static struct platform_device ltq_nor = {
	.name		= "ltq_nor",
	.resource	= &ltq_nor_resource,
	.num_resources	= 1,
};

void __init ltq_register_nor(struct physmap_flash_data *data)
{
	ltq_nor.dev.platform_data = data;
	platform_device_register(&ltq_nor);
}

/* watchdog */
static struct resource ltq_wdt_resource =
	MEM_RES("watchdog", LTQ_WDT_BASE_ADDR, LTQ_WDT_SIZE);

void __init ltq_register_wdt(void)
{
	platform_device_register_simple("ltq_wdt", 0, &ltq_wdt_resource, 1);
}

/* asc ports */
static struct resource ltq_asc0_resources[] = {
	MEM_RES("asc0", LTQ_ASC0_BASE_ADDR, LTQ_ASC_SIZE),
	IRQ_RES(tx, LTQ_ASC_TIR(0)),
	IRQ_RES(rx, LTQ_ASC_RIR(0)),
	IRQ_RES(err, LTQ_ASC_EIR(0)),
};

static struct resource ltq_asc1_resources[] = {
	MEM_RES("asc1", LTQ_ASC1_BASE_ADDR, LTQ_ASC_SIZE),
	IRQ_RES(tx, LTQ_ASC_TIR(1)),
	IRQ_RES(rx, LTQ_ASC_RIR(1)),
	IRQ_RES(err, LTQ_ASC_EIR(1)),
};

void __init ltq_register_asc(int port)
{
	switch (port) {
	case 0:
		platform_device_register_simple("ltq_asc", 0,
			ltq_asc0_resources, ARRAY_SIZE(ltq_asc0_resources));
		break;
	case 1:
		platform_device_register_simple("ltq_asc", 1,
			ltq_asc1_resources, ARRAY_SIZE(ltq_asc1_resources));
		break;
	default:
		break;
	}
}

#ifdef CONFIG_PCI
/* pci */
static struct platform_device ltq_pci = {
	.name		= "ltq_pci",
	.num_resources	= 0,
};

void __init ltq_register_pci(struct ltq_pci_data *data)
{
	ltq_pci.dev.platform_data = data;
	platform_device_register(&ltq_pci);
}
#else
void __init ltq_register_pci(struct ltq_pci_data *data)
{
	pr_err("kernel is compiled without PCI support\n");
}
#endif

static unsigned int *cp1_base = 0;
unsigned int*
ltq_get_cp1_base(void)
{
	return cp1_base;
}
EXPORT_SYMBOL(ltq_get_cp1_base);

void __init
ltq_register_tapi(void)
{
#define CP1_SIZE       (1 << 20)
	dma_addr_t dma;
	cp1_base =
		(void*)CPHYSADDR(dma_alloc_coherent(NULL, CP1_SIZE, &dma, GFP_ATOMIC));
}
