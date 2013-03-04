/*
 * arch/arm/mach-ixp4xx/ixdp425-pci.c 
 *
 * IXDP425 board-level PCI initialization
 *
 * Copyright (C) 2002 Intel Corporation.
 * Copyright (C) 2003-2004 MontaVista Software, Inc.
 *
 * Maintainer: Deepak Saxena <dsaxena@plexity.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/delay.h>

#include <asm/mach/pci.h>
#include <asm/irq.h>
#include <mach/hardware.h>
#include <asm/mach-types.h>

#define IRQT_LOW IRQ_TYPE_LEVEL_LOW
#define set_irq_type irq_set_irq_type

#define IXP425_GPIO_REG_WRITE(regPtr,val) \
	(*((volatile int *)(regPtr)) = (val))

#define IXP425_GPIO_REG_READ(regPtr,res) \
	((res) = *(volatile int *)(regPtr))
	
#define REG_WRITE(b,o,v) (*(volatile int*)((b+o))=(v))
#define REG_READ(b,o,v)  ((v)=(*(volatile int*)((b+o))))
	
#define IXP425_GPIO_PIN_15 		15
#define IXP425_GPIO_PIN_14 		14
#define IXP425_GPIO_PIN_13 		13
#define IXP425_GPIO_PIN_12 		12
#define IXP425_GPIO_PIN_11 		11
#define IXP425_GPIO_PIN_10 		10
#define IXP425_GPIO_PIN_9 		9
#define IXP425_GPIO_PIN_8  		8
#define IXP425_GPIO_PIN_7  		7
#define IXP425_GPIO_PIN_6  		6
#define IXP425_GPIO_PIN_5  		5
#define IXP425_GPIO_PIN_4  		4
#define IXP425_GPIO_PIN_3  		3
#define IXP425_GPIO_PIN_2  		2
#define IXP425_GPIO_PIN_1  		1
#define IXP425_GPIO_PIN_0  		0

#define IXP425_PCI_RESET_GPIO   IXP425_GPIO_PIN_12
#define IXP425_GPIO_33_MHZ		0x1	/* Default */
#define INTA_PIN	IXP425_GPIO_PIN_11
#define INTB_PIN	IXP425_GPIO_PIN_10
#define	INTC_PIN	IXP425_GPIO_PIN_9
#define	INTD_PIN	IXP425_GPIO_PIN_8
#define IXP425_GPIO_OUT 		1
#define IXP425_GPIO_IN  		2
#define IXP425_GPIO_ACTIVE_HIGH		0x4 /* Default */
#define IXP425_GPIO_ACTIVE_LOW		0x8
#define IXP425_GPIO_RISING_EDGE		0x10
#define IXP425_GPIO_FALLING_EDGE 	0x20
#define IXP425_GPIO_TRANSITIONAL 	0x40
#define IXP425_GPIO_GPOUTR_OFFSET       0x00
#define IXP425_GPIO_GPOER_OFFSET        0x04
#define IXP425_GPIO_GPINR_OFFSET        0x08
#define IXP425_GPIO_GPISR_OFFSET        0x0C
#define IXP425_GPIO_GPIT1R_OFFSET	0x10
#define IXP425_GPIO_GPIT2R_OFFSET	0x14
#define IXP425_GPIO_GPCLKR_OFFSET	0x18
#define IXP425_GPIO_GPDBSELR_OFFSET	0x1C



#define IXP425_GPIO_CLK0_ENABLE 	0x100
#define IXP425_GPIO_CLK1_ENABLE 	0x1000000

#define IXP425_GPIO_CLK0TC_LSH		4
#define IXP425_GPIO_CLK1TC_LSH		20
#define IXP425_GPIO_CLK0DC_LSH		0
#define IXP425_GPIO_CLK1DC_LSH		16


void ixp425_gpio_clock_enable(int state)
{
    volatile int clkReg = 0;
	
	IXP425_GPIO_REG_READ(IXP4XX_GPIO_GPCLKR, clkReg);
               
	if(state)
		clkReg |= IXP425_GPIO_CLK0_ENABLE;
	else
		clkReg &= ~IXP425_GPIO_CLK0_ENABLE;
   	IXP425_GPIO_REG_WRITE(IXP4XX_GPIO_GPCLKR, clkReg);
   	return ;
}

void ixp425_gpio_clock_set (int freq)
{
    volatile int clkReg;
    int dutyCycle;
    		      
    IXP425_GPIO_REG_READ(IXP4XX_GPIO_GPCLKR, clkReg);
       	
    clkReg |= (freq << (IXP425_GPIO_CLK0TC_LSH));
	dutyCycle = (unsigned int)(freq/2);
	clkReg |= (dutyCycle << IXP425_GPIO_CLK0DC_LSH); 
    IXP425_GPIO_REG_WRITE(IXP4XX_GPIO_GPCLKR, clkReg);
    return ;
}

void ixp425_gpio_pin_config(void)
 {
	gpio_line_config(IXP425_GPIO_PIN_12, IXP425_GPIO_OUT);
	gpio_line_config(IXP425_GPIO_PIN_14, IXP425_GPIO_OUT);

 	gpio_line_config(INTA_PIN, IXP425_GPIO_IN | IXP425_GPIO_ACTIVE_LOW);
 	gpio_line_config(INTB_PIN, IXP425_GPIO_IN | IXP425_GPIO_ACTIVE_LOW);
 	gpio_line_config(INTC_PIN, IXP425_GPIO_IN | IXP425_GPIO_ACTIVE_LOW);
 	gpio_line_config(INTD_PIN, IXP425_GPIO_IN | IXP425_GPIO_ACTIVE_LOW);

      
 	gpio_line_isr_clear(INTA_PIN);
 	gpio_line_isr_clear(INTB_PIN);
 	gpio_line_isr_clear(INTC_PIN);
 	gpio_line_isr_clear(INTD_PIN);
}

static int __init compex_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	static int pci_irq_table[AVILA_PCI_IRQ_LINES] = {
		IRQ_IXDP425_PCI_INTA,
		IRQ_IXDP425_PCI_INTB,
		IRQ_IXDP425_PCI_INTC,
		IRQ_IXDP425_PCI_INTD
	};

	int irq = -1;
	if (slot >= 1 && slot <= (machine_is_loft() ? LOFT_PCI_MAX_DEV : AVILA_PCI_MAX_DEV) &&
		pin >= 1 && pin <= AVILA_PCI_IRQ_LINES) {
		irq = pci_irq_table[(slot + pin - 2) % 4];
	}


	return irq;
}


void __init compex_pci_preinit(void)
{

	gpio_line_set(IXP425_PCI_RESET_GPIO, 0);
 	ixp425_gpio_clock_enable(0);
	ixp425_gpio_pin_config();
	*(int*)PCI_INTEN = 0;   
	/* Wait 1ms to satisfy "minimum reset assertion time"
	 * of the PCI spec.
	 */
	mdelay(100);
	ixp425_gpio_clock_set(IXP425_GPIO_33_MHZ);
	ixp425_gpio_clock_enable(1);
	/*
     * Wait 100us to satisfy "minimum reset assertion time 
     * from clock stable" requirement of the PCI spec.
     */
	mdelay(100); 	
	gpio_line_set(IXP425_PCI_RESET_GPIO, 1);

	/* Delay enough time for next reset */
	mdelay(2000);
	/* Reset again for some special wireless cards */
	gpio_line_set(IXP425_PCI_RESET_GPIO, 0);
	mdelay(100); 	
	gpio_line_set(IXP425_PCI_RESET_GPIO, 1);
	mdelay(100); 
	set_irq_type(IRQ_IXDP425_PCI_INTA, IRQT_LOW);
	set_irq_type(IRQ_IXDP425_PCI_INTB, IRQT_LOW);
	set_irq_type(IRQ_IXDP425_PCI_INTC, IRQT_LOW);
	set_irq_type(IRQ_IXDP425_PCI_INTD, IRQT_LOW);


	ixp4xx_pci_preinit();
}


struct hw_pci compex_pci __initdata = {
	.nr_controllers = 1,
	.ops = &ixp4xx_ops,
	.preinit	= compex_pci_preinit,
	.setup		= ixp4xx_setup,
	.map_irq	= compex_map_irq,
};

int __init compex_pci_init(void)
{
 	if (machine_is_compex())
 	{
 	


		pci_common_init(&compex_pci);
	}
	return 0;
}

subsys_initcall(compex_pci_init);

