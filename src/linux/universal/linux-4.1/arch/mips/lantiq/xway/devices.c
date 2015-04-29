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
#include <linux/mtd/physmap.h>
#include <linux/kernel.h>
#include <linux/reboot.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/etherdevice.h>
#include <linux/time.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>

#include <asm/bootinfo.h>
#include <asm/irq.h>

#include <lantiq_soc.h>
#include <lantiq_irq.h>
#include <lantiq_platform.h>

#include "devices.h"

/* gpio */
static struct resource ltq_gpio_resource[] = {
	MEM_RES("gpio0", LTQ_GPIO0_BASE_ADDR, LTQ_GPIO_SIZE),
	MEM_RES("gpio1", LTQ_GPIO1_BASE_ADDR, LTQ_GPIO_SIZE),
	MEM_RES("gpio2", LTQ_GPIO2_BASE_ADDR, LTQ_GPIO_SIZE),
	MEM_RES("gpio3", LTQ_GPIO3_BASE_ADDR, LTQ_GPIO3_SIZE),
};

void __init ltq_register_gpio(void)
{
	platform_device_register_simple("ltq_gpio", 0,
		&ltq_gpio_resource[0], 1);
	platform_device_register_simple("ltq_gpio", 1,
		&ltq_gpio_resource[1], 1);

	/* AR9 and VR9 have an extra gpio block */
	if (ltq_is_ar9() || ltq_is_vr9()) {
		platform_device_register_simple("ltq_gpio", 2,
			&ltq_gpio_resource[2], 1);
		platform_device_register_simple("ltq_gpio", 3,
			&ltq_gpio_resource[3], 1);
	}
}

/* serial to parallel conversion */
static struct resource ltq_stp_resource =
	MEM_RES("stp", LTQ_STP_BASE_ADDR, LTQ_STP_SIZE);

void __init ltq_register_gpio_stp(void)
{
	platform_device_register_simple("ltq_stp", 0, &ltq_stp_resource, 1);
}

/* asc ports - amazon se has its own serial mapping */
static struct resource ltq_ase_asc_resources[] = {
	MEM_RES("asc0", LTQ_ASC1_BASE_ADDR, LTQ_ASC_SIZE),
	IRQ_RES(tx, LTQ_ASC_ASE_TIR),
	IRQ_RES(rx, LTQ_ASC_ASE_RIR),
	IRQ_RES(err, LTQ_ASC_ASE_EIR),
};

void __init ltq_register_ase_asc(void)
{
	platform_device_register_simple("ltq_asc", 0,
		ltq_ase_asc_resources, ARRAY_SIZE(ltq_ase_asc_resources));
}

/* ethernet */
static struct resource ltq_etop_resources[] = {
	MEM_RES("etop", LTQ_ETOP_BASE_ADDR, LTQ_ETOP_SIZE),
	MEM_RES("gbit", LTQ_GBIT_BASE_ADDR, LTQ_GBIT_SIZE),
};

static struct platform_device ltq_etop = {
	.name		= "ltq_etop",
	.resource	= ltq_etop_resources,
	.num_resources	= 1,
};

void __init
ltq_register_etop(struct ltq_eth_data *eth)
{
	/* only register the gphy on socs that have one */
	if (ltq_is_ar9() | ltq_is_vr9())
		ltq_etop.num_resources = 2;
	if (eth) {
		ltq_etop.dev.platform_data = eth;
		platform_device_register(&ltq_etop);
	}
}

/* madwifi */
int lantiq_emulate_madwifi_eep = 0;
EXPORT_SYMBOL(lantiq_emulate_madwifi_eep);

void *lantiq_madwifi_eep_addr = NULL;
EXPORT_SYMBOL(lantiq_madwifi_eep_addr);

void __init
ltq_register_madwifi_eep(void *addr)
{
	lantiq_madwifi_eep_addr = addr;
	lantiq_emulate_madwifi_eep = 1;
}

/* ebu */
static struct resource ltq_ebu_resource =
{
	.name   = "gpio_ebu",
	.start  = LTQ_EBU_GPIO_START,
	.end    = LTQ_EBU_GPIO_START + LTQ_EBU_GPIO_SIZE - 1,
	.flags  = IORESOURCE_MEM,
};

static struct platform_device ltq_ebu =
{
	.name           = "ltq_ebu",
	.resource       = &ltq_ebu_resource,
	.num_resources  = 1,
};

void __init
ltq_register_gpio_ebu(unsigned int value)
{
	ltq_ebu.dev.platform_data = (void*) value;
	platform_device_register(&ltq_ebu);
}

/* gpio buttons */
static struct gpio_buttons_platform_data ltq_gpio_buttons_platform_data;

static struct platform_device ltq_gpio_buttons_platform_device =
{
	.name = "gpio-buttons",
	.id = 0,
	.dev = {
		.platform_data = (void *) &ltq_gpio_buttons_platform_data,
	},
};

void __init
ltq_register_gpio_buttons(struct gpio_button *buttons, int cnt)
{
	ltq_gpio_buttons_platform_data.buttons = buttons;
	ltq_gpio_buttons_platform_data.nbuttons = cnt;
	platform_device_register(&ltq_gpio_buttons_platform_device);
}

static struct resource ltq_spi_resources[] = {
	{
		.start  = LTQ_SSC_BASE_ADDR,
		.end    = LTQ_SSC_BASE_ADDR + LTQ_SSC_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	IRQ_RES(spi_tx, LTQ_SSC_TIR),
	IRQ_RES(spi_rx, LTQ_SSC_RIR),
	IRQ_RES(spi_err, LTQ_SSC_EIR),
};

static struct resource ltq_spi_resources_ar9[] = {
	{
		.start  = LTQ_SSC_BASE_ADDR,
		.end    = LTQ_SSC_BASE_ADDR + LTQ_SSC_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
	IRQ_RES(spi_tx, LTQ_SSC_TIR_AR9),
	IRQ_RES(spi_rx, LTQ_SSC_RIR_AR9),
	IRQ_RES(spi_err, LTQ_SSC_EIR),
};

static struct platform_device ltq_spi = {
	.name		= "ltq-spi",
	.resource	= ltq_spi_resources,
	.num_resources	= ARRAY_SIZE(ltq_spi_resources),
};

void __init ltq_register_spi(struct ltq_spi_platform_data *pdata,
		struct spi_board_info const *info, unsigned n)
{
	if(ltq_is_ar9())
		ltq_spi.resource = ltq_spi_resources_ar9;
	spi_register_board_info(info, n);
	ltq_spi.dev.platform_data = pdata;
	platform_device_register(&ltq_spi);
}
