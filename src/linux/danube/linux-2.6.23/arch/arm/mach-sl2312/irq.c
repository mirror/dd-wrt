/*
 *  linux/arch/arm/mach-epxa10db/irq.c
 *
 *  Copyright (C) 2001 Altera Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/stddef.h>
#include <linux/list.h>
#include <linux/sched.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/platform.h>
#include <asm/arch/int_ctrl.h>

#ifdef CONFIG_PCI
#include <asm/arch/pci.h>
#endif

int fixup_irq(unsigned int irq)
{
#ifdef CONFIG_PCI
	if (irq == IRQ_PCI) {
		return sl2312_pci_get_int_src();
	}
#endif
	return irq;
}

static void sl2312_ack_irq(unsigned int irq)
{
   __raw_writel(1 << irq, IRQ_CLEAR(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
}

static void sl2312_mask_irq(unsigned int irq)
{
	unsigned int mask;

#ifdef CONFIG_PCI
	if (irq >= PCI_IRQ_OFFSET)
	{
		mask = __raw_readl(IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
		mask &= ~IRQ_PCI_MASK ;
		__raw_writel(mask, IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
		sl2312_pci_mask_irq(irq - PCI_IRQ_OFFSET);
	}
	else
#endif
	if(irq >= FIQ_OFFSET)
	{
           mask = __raw_readl(FIQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
           mask &= ~(1 << (irq - FIQ_OFFSET));
           __raw_writel(mask, FIQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        }
        else
        {
           mask = __raw_readl(IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
           mask &= ~(1 << irq);
           __raw_writel(mask, IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        }

}

static void sl2312_unmask_irq(unsigned int irq)
{
	unsigned int mask;

#ifdef CONFIG_PCI
	if (irq >= PCI_IRQ_OFFSET)
	{
		mask = __raw_readl(IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
		mask |= IRQ_PCI_MASK ;
		__raw_writel(mask, IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
		sl2312_pci_unmask_irq(irq - PCI_IRQ_OFFSET);
	}
	else
#endif
	if(irq >= FIQ_OFFSET)
        {
          mask = __raw_readl(FIQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
          mask |= (1 << (irq - FIQ_OFFSET));
          __raw_writel(mask, FIQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
	}
        else
        {
          mask = __raw_readl(IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
          mask |= (1 << irq);
          __raw_writel(mask, IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
        }
}

static struct irq_chip sl2312_level_irq = {
        .ack            = sl2312_mask_irq,
        .mask           = sl2312_mask_irq,
        .unmask         = sl2312_unmask_irq,
//		.set_type	= ixp4xx_set_irq_type,
};

static struct irq_chip sl2312_edge_irq = {
        .ack            = sl2312_ack_irq,
        .mask           = sl2312_mask_irq,
        .unmask         = sl2312_unmask_irq,
//		.set_type	= ixp4xx_set_irq_type,
};

static struct resource irq_resource = {
        .name   = "irq_handler",
        .start  = IO_ADDRESS(SL2312_INTERRUPT_BASE),
        .end    = IO_ADDRESS(FIQ_STATUS(SL2312_INTERRUPT_BASE))+4,
};

void __init sl2312_init_irq(void)
{
	unsigned int i, mode, level;

    request_resource(&iomem_resource, &irq_resource);

	for (i = 0; i < NR_IRQS; i++)
	{
	    if((i>=IRQ_TIMER1 && i<=IRQ_TIMER3)||(i>=IRQ_SERIRQ0 && i<=IRQ_SERIRQ_MAX))
        {
	        set_irq_chip(i, &sl2312_edge_irq);
	        set_irq_handler(i, handle_edge_irq);
        }
	    else
        {
	        set_irq_chip(i, &sl2312_level_irq);
            set_irq_handler(i,handle_level_irq);
        }
        set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}

	/* Disable all interrupt */
	__raw_writel(0,IRQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
	__raw_writel(0,FIQ_MASK(IO_ADDRESS(SL2312_INTERRUPT_BASE)));

	/* Set interrupt mode */
    /* emac & ipsec type is level trigger and high active */
    mode = __raw_readl(IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
    level = __raw_readl(IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE)));

	mode &= ~IRQ_GMAC0_MASK;
	level &= ~IRQ_GMAC0_MASK;

	mode &= ~IRQ_GMAC1_MASK;
	level &= ~IRQ_GMAC1_MASK;

	mode &= ~IRQ_IPSEC_MASK;
	level &= ~IRQ_IPSEC_MASK;

	// for IDE0,1, high active and level trigger
	mode &= ~IRQ_IDE0_MASK;
	level &= ~IRQ_IDE0_MASK;
	mode &= ~IRQ_IDE1_MASK;
	level &= ~IRQ_IDE1_MASK;


	// for PCI, high active and level trigger
	mode &= ~IRQ_PCI_MASK;
	level &= ~IRQ_PCI_MASK;

	// for USB, high active and level trigger
	mode &= ~IRQ_USB0_MASK;
	level &= ~IRQ_USB0_MASK;

	mode &= ~IRQ_USB1_MASK;
	level &= ~IRQ_USB1_MASK;

	// for LPC, high active and edge trigger
	mode |= 0xffff0000;
	level &= 0x0000ffff;

	// for GPIO, high active and level trigger
	mode &= ~(IRQ_GPIO_MASK);
	level &= ~(IRQ_GPIO_MASK);

	mode &= ~(IRQ_GPIO1_MASK);
	level &= ~(IRQ_GPIO1_MASK);

	mode &= ~(IRQ_GPIO2_MASK);
	level &= ~(IRQ_GPIO2_MASK);

	__raw_writel(mode,IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE)));
	__raw_writel(level,IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE)));

}
