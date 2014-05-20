/*
 * Copyright (C) Mikrotik 2007
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */

#include <linux/pci.h>
#include <linux/delay.h>
#include <linux/root_dev.h>
#include <linux/initrd.h>
#include <linux/interrupt.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <asm/time.h>
#include <asm/ipic.h>
#include <asm/udbg.h>
#include <asm/qe.h>
#include <asm/qe_ic.h>
#include <sysdev/fsl_soc.h>
#include <sysdev/fsl_pci.h>
#include "mpc83xx.h"

#define SYSCTL		0x100
#define SICRL		0x014

#define GTCFR2		0x04
#define GTMDR4		0x22
#define GTRFR4		0x26
#define GTCNR4		0x2e
#define GTVER4		0x36
#define GTPSR4		0x3e

#define GTCFR_BCM	0x40
#define GTCFR_STP4	0x20
#define GTCFR_RST4	0x10
#define GTCFR_STP3	0x02
#define GTCFR_RST3	0x01

#define GTMDR_ORI	0x10
#define GTMDR_FRR	0x08
#define GTMDR_ICLK16	0x04

#define SBIT(x) (0x80000000 >> (x))
#define DBIT(x, y) ((y) << (32 - (((x % 16) + 1) * 2)))

// rb300
#define GPIO_DIR_RB300 (immrs + (0x1408 >> 2))
#define GPIO_DATA_RB300 (immrs + (0x1404 >> 2))

// rb600
#define SICRL_RB600 (immrs + (0x114 >> 2))
#define GPIO_DIR_RB600 (immrs + (0xc00 >> 2))
#define GPIO_DATA_RB600 (immrs + (0xc08 >> 2))

extern int par_io_data_set(u8 port, u8 pin, u8 val);
extern int par_io_config_pin(u8 port, u8 pin, int dir, int open_drain,
			     int assignment, int has_irq);

static unsigned timer_freq;
static void *gtm;

static int beeper_irq;
static unsigned beeper_gpio_pin[2];

static __be32 __iomem *immrs;

irqreturn_t rbppc_timer_irq(int irq, void *ptr)
{
	static int toggle = 0;

	par_io_data_set(beeper_gpio_pin[0], beeper_gpio_pin[1], toggle);
	toggle = !toggle;

	/* ack interrupt */
	out_be16(gtm + GTVER4, 3);

	return IRQ_HANDLED;
}

void rbppc_beep(unsigned freq)
{
	unsigned gtmdr;

	if (freq > 5000) freq = 5000;

	if (!gtm)
		return;
	if (!freq) {
		out_8(gtm + GTCFR2, GTCFR_STP4 | GTCFR_STP3);
		return;
	}

	out_8(gtm + GTCFR2, GTCFR_RST4 | GTCFR_STP3);
	out_be16(gtm + GTPSR4, 255);
	gtmdr = GTMDR_FRR | GTMDR_ICLK16;
	if (beeper_irq != NO_IRQ) gtmdr |= GTMDR_ORI;
	out_be16(gtm + GTMDR4, gtmdr);
	out_be16(gtm + GTVER4, 3);
	
	out_be16(gtm + GTRFR4, timer_freq / 16 / 256 / freq / 2);
	out_be16(gtm + GTCNR4, 0);
}
EXPORT_SYMBOL(rbppc_beep);

static void __init rbppc_setup_arch(void)
{
	struct device_node *np;

	immrs = ioremap(get_immrbase(), 0x2000);

	np = of_find_node_by_name(NULL, "serial");
	if (np) {
		timer_freq =
		    *(unsigned *) of_get_property(np, "clock-frequency", NULL);
		of_node_put(np);
	}

#ifdef CONFIG_PCI
	np = of_find_node_by_type(np, "pci");
	if (np) {
		const u32 *reg = of_get_property(np, "reg", NULL);
		if (reg) ((u32 *) reg)[0] = of_translate_address(np, reg);
		if (np->parent) np->parent = np->parent->parent;
		mpc83xx_add_bridge(np);
	}
#endif

#ifdef CONFIG_QUICC_ENGINE
	np = of_find_node_by_name(np, "par_io");
	if (np) {
		qe_reset();
		par_io_init(np);
		of_node_put(np);

		np = NULL;
		while (1) {
			np = of_find_node_by_name(np, "ucc");
			if (!np) break;
			
			par_io_of_config(np);
		}
	}
#endif
}

void __init rbppc_init_irq(void)
{
	struct device_node *np;

	np = of_find_node_by_type(NULL, "ipic");
	if (np) {
		ipic_init(np, 0);
		ipic_set_default_priority();
		of_node_put(np);
	}

#ifdef CONFIG_QUICC_ENGINE
	np = of_find_node_by_type(NULL, "qeic");
	if (np) {
		qe_ic_init(np, 0,
			   qe_ic_cascade_low_ipic, qe_ic_cascade_high_ipic);
		of_node_put(np);
	}
#endif
}

static int __init rbppc_probe(void)
{
	char *model;

	model = of_get_flat_dt_prop(of_get_flat_dt_root(), "model", NULL);

	if (!model)
		return 0;

	if (strcmp(model, "RB333") == 0)
		return 1;
	
	if (strcmp(model, "RB600") == 0)
		return 1;
	
	return 0;
}

static void __init rbppc_beeper_init(struct device_node *beeper)
{
	    
	struct resource res;
	struct device_node *gpio;
	const unsigned *pin;
	const unsigned *gpio_id;

	if (of_address_to_resource(beeper, 0, &res)) {
		printk("beeper error: no region specified\n");
		return;
	}

	pin = of_get_property(beeper, "gpio", NULL);
	if (pin) {
		gpio = of_find_node_by_phandle(pin[0]);

		if (!gpio) {
			printk("beeper error: gpio handle %x not found\n",
			       pin[0]);
			return;
		}

		gpio_id = of_get_property(gpio, "device-id", NULL);
		if (!gpio_id) {
			printk("beeper error: no device-id specified"
			       " in gpio\n");
			return;
		}

		beeper_gpio_pin[0] = *gpio_id;
		beeper_gpio_pin[1] = pin[1];
		    
		par_io_config_pin(*gpio_id, pin[1], 1, 0, 0, 0);
	} else {
		void *sysctl;

		sysctl = ioremap_nocache(get_immrbase() + SYSCTL, 0x100);
		out_be32(sysctl + SICRL,
			 in_be32(sysctl + SICRL) | (1 << (31 - 19)));
		iounmap(sysctl);
	}

	gtm = ioremap_nocache(res.start, res.end - res.start + 1);
	
	beeper_irq = irq_of_parse_and_map(beeper, 0);
	if (beeper_irq != NO_IRQ) {
	    int e = request_irq(beeper_irq, rbppc_timer_irq, 0, "beeper", NULL);
	    if (e) printk(KERN_ERR "Request of beeper irq failed!\n");
	}
}

static void rb_restart(char *cmd)
{
    unsigned rb_model;
    struct device_node *root;
    unsigned int size;    

    root = of_find_node_by_path("/");
    if (root) {
	const char *prop = (char *) of_get_property(root, "model", &size);
	rb_model = prop[sizeof("RB") - 1] - '0';
	of_node_put(root);
	switch (rb_model) {
	case 3:
	    local_irq_disable();
	    out_be32(GPIO_DIR_RB300,
		     (in_be32(GPIO_DIR_RB300) & ~DBIT(4, 3)) | DBIT(4, 1));
	    out_be32(GPIO_DATA_RB300, in_be32(GPIO_DATA_RB300) & ~SBIT(4));
	    break;
	case 6:
	    local_irq_disable();
	    out_be32(SICRL_RB600, in_be32(SICRL_RB600) & ~0x00800000);
	    out_be32(GPIO_DIR_RB600, in_be32(GPIO_DIR_RB600) | SBIT(2));
	    out_be32(GPIO_DATA_RB600, in_be32(GPIO_DATA_RB600) & ~SBIT(2));
	    break;
	default:
	    mpc83xx_restart(cmd);
	    break;
	}
    }
    else mpc83xx_restart(cmd);
    
    for (;;) ;
}

static struct resource rbppc_led_resources[2] = {
	[0] = {
		.flags		= IORESOURCE_IO,
	},
	[1] = {
		.name		= "user-led",
	},
};

static const unsigned rb333_uled[2] = { 0x4003, 0x0f };
static const unsigned rb600_uled[2] = { 0x400, 0x08 };

static int __init rbppc_init_leds(void)
{
	struct device_node *np;
	const unsigned *uled;

	np = of_find_node_by_name(NULL, "led");
	if (np) {
		uled = of_get_property(np, "user_led", NULL);
		of_node_put(np);
		if (!uled) {
			printk("rbppc led error: "
			       "user_led property is missing\n");
			return -1;
		}
	}
	else {
		/* detect routerboard type */
		np = of_find_node_by_name(NULL, "qe");
		if (np) {
			of_node_put(np);
			uled = rb333_uled;
		}
		else {
			uled = rb600_uled;
		}
	}

	rbppc_led_resources[1].start = uled[1];
	rbppc_led_resources[1].end = uled[1];

	np = of_find_node_by_phandle(uled[0]);
	if (!np) {
		printk("rbppc led error: no gpio<%x> node found\n", *uled);
		return -1;
	}
	if (of_address_to_resource(np, 0, &rbppc_led_resources[0])) {
		of_node_put(np);
		printk("rbppc led error: no reg property in gpio found\n");
		return -1;
	}
	of_node_put(np);

	platform_device_register_simple("rbppc-led", 0,
					rbppc_led_resources, 2);
	return 0;
}

static struct of_device_id rbppc_ids[] = {
	{ .type = "soc", },
	{ .compatible = "soc", },
	{ .type = "qe", },
	{ .type = "mdio", },
	{},
};

static int __init rbppc_declare_of_platform_devices(void)
{
	struct device_node *np;
	unsigned idx;

	of_platform_bus_probe(NULL, rbppc_ids, NULL);

	/* fix MDIO region */
	np = of_find_node_by_type(NULL, "mdio");
	if (np) {
		unsigned len;
		unsigned *res;
		const unsigned *eres;
		struct device_node *ep;
		
		ep = of_find_compatible_node(NULL, "network", "ucc_geth");
		if (ep) {
			eres = of_get_property(ep, "reg", &len);
			res = (unsigned *) of_get_property(np, "reg", &len);
			if (res && eres)
				res[0] = eres[0] + 0x120;;
	    }
	}

	np = of_find_node_by_name(NULL, "nand");
	if (np) of_platform_device_create(np, "nand", NULL);

	idx = 0;
	for_each_node_by_type(np, "rb,cf") {
		char dev_name[12];
		snprintf(dev_name, sizeof(dev_name), "cf.%u", idx);
		of_platform_device_create(np, dev_name, NULL);
		++idx;		
	}

	np = of_find_node_by_name(NULL, "beeper");
	if (np) rbppc_beeper_init(np);

	rbppc_init_leds();

	return 0;
}
machine_device_initcall(rb333, rbppc_declare_of_platform_devices);

static void rb_halt(void)
{
	while (1);
}

define_machine(rb333) {
	.name 		= "RB333/RB600",
	.probe 		= rbppc_probe,
	.setup_arch 	= rbppc_setup_arch,
	.init_IRQ 	= rbppc_init_irq,
	.get_irq 	= ipic_get_irq,
	.restart 	= rb_restart,
	.halt		= rb_halt,
	.time_init 	= mpc83xx_time_init,
	.calibrate_decr	= generic_calibrate_decr,
};

static void fixup_pci(struct pci_dev *dev)
{
	if ((dev->class >> 8) == PCI_CLASS_BRIDGE_PCI) {
		/* let the kernel itself set right memory windows */
		pci_write_config_word(dev, PCI_MEMORY_BASE, 0);
		pci_write_config_word(dev, PCI_MEMORY_LIMIT, 0);
		pci_write_config_word(dev, PCI_PREF_MEMORY_BASE, 0);
		pci_write_config_word(dev, PCI_PREF_MEMORY_LIMIT, 0);
		pci_write_config_byte(dev, PCI_IO_BASE, 0);
		pci_write_config_byte(dev, PCI_IO_LIMIT, 4 << 4);

		pci_write_config_byte(
		    dev, PCI_COMMAND,
		    PCI_COMMAND_MASTER | PCI_COMMAND_MEMORY | PCI_COMMAND_IO);
		pci_write_config_byte(dev, PCI_CACHE_LINE_SIZE, 8);
	} else {
		pci_write_config_byte(dev, PCI_LATENCY_TIMER, 0x40);
	}
}

static void fixup_rb604(struct pci_dev *dev)
{
	pci_write_config_byte(dev, 0xC0, 0x01);
}

void change_latch(unsigned char set, unsigned char clear) {
    printk("Hello, I am dummy! (%02x %02x)\n", set, clear);
}

EXPORT_SYMBOL(change_latch);

DECLARE_PCI_FIXUP_HEADER(PCI_ANY_ID, PCI_ANY_ID, fixup_pci)
DECLARE_PCI_FIXUP_HEADER(0x3388, 0x0021, fixup_rb604)

