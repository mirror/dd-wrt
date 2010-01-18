/*
 *  linux/arch/arm/mach-sl2312/pci_sl2312.c
 *
 *  PCI functions for sl2312 host PCI bridge
 *
 *  Copyright (C) 2003 StorLink Corp.
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
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#include <asm/sizes.h>
#include <asm/hardware.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <asm/mach/irq.h>
#include <asm/mach-types.h>

#include <asm/arch/pci.h>

//#define DEBUG

// sl2312 PCI bridge access routines

#define PCI_IOSIZE_REG	(*(volatile unsigned long *) (IO_ADDRESS(SL2312_PCI_IO_BASE)))
#define PCI_PROT_REG	(*(volatile unsigned long *) (IO_ADDRESS(SL2312_PCI_IO_BASE) + 0x04))
#define PCI_CTRL_REG	(*(volatile unsigned long *) (IO_ADDRESS(SL2312_PCI_IO_BASE) + 0x08))
#define PCI_SOFTRST_REG	(*(volatile unsigned long *) (IO_ADDRESS(SL2312_PCI_IO_BASE) + 0x10))
#define PCI_CONFIG_REG	(*(volatile unsigned long *) (IO_ADDRESS(SL2312_PCI_IO_BASE) + 0x28))
#define PCI_DATA_REG	(*(volatile unsigned long *) (IO_ADDRESS(SL2312_PCI_IO_BASE) + 0x2C))

static spinlock_t sl2312_pci_lock = SPIN_LOCK_UNLOCKED;
// for initialize PCI devices
struct resource pci_ioport_resource = {
		.name = "PCI I/O Space",
		.start = IO_ADDRESS(SL2312_PCI_IO_BASE) + 0x100,
		.end = IO_ADDRESS(SL2312_PCI_IO_BASE) + SZ_512K - 1,
		.flags = IORESOURCE_IO,
};
struct resource pci_iomem_resource = {
		.name = "PCI Mem Space",
		.start = SL2312_PCI_MEM_BASE,
		.end = SL2312_PCI_MEM_BASE + SZ_128M - 1,
		.flags = IORESOURCE_MEM,
};

static int sl2312_read_config(struct pci_bus *bus, unsigned int devfn, int where,int size, u32 *val)
{
	unsigned long addr,data;
	unsigned long flags;

	spin_lock_irqsave(&sl2312_pci_lock, flags);
    addr = 0x80000000 | (PCI_SLOT(devfn) << 11) | (PCI_FUNC(devfn) << 8) | (where & ~3);
	PCI_CONFIG_REG = addr;
	data = PCI_DATA_REG;

	switch (size) {
	case 1:
	    *val = (u8) (data >> ((where & 0x03) * 8));
		break;
	case 2:
	    *val = (u16) (data >> ((where & 0x02) * 8));
		break;
	case 4:
	    *val = data;
    	if ((where >= 0x10) && (where <= 0x24)) {
    		if ((*val & 0xfff00000) == SL2312_PCI_IO_BASE) {
    			*val &= 0x000fffff;
    			*val |= IO_ADDRESS(SL2312_PCI_IO_BASE);
    		}
    	}
		break;
	}
	spin_unlock_irqrestore(&sl2312_pci_lock, flags);
//	printk("READ==>slot=%d fn=%d where=%d value=%x\n",PCI_SLOT(devfn),PCI_FUNC(devfn),where,*val);
	return PCIBIOS_SUCCESSFUL;
}

static int sl2312_write_config(struct pci_bus *bus, unsigned int devfn, int where,int size, u32 val)
{
	unsigned long addr,data;
	unsigned long flags;

	spin_lock_irqsave(&sl2312_pci_lock, flags);
    addr = 0x80000000 | (PCI_SLOT(devfn) << 11) | (PCI_FUNC(devfn) << 8) | (where & ~3);
	PCI_CONFIG_REG = addr;
	data = PCI_DATA_REG;

	switch (size) {
	case 1:
    	data &= ~(0xff << ((where & 0x03) * 8));
    	data |= (val << ((where & 0x03) * 8));
    	PCI_DATA_REG = data;
		break;
	case 2:
    	data &= ~(0xffff << ((where & 0x02) * 8));
    	data |= (val << ((where & 0x02) * 8));
    	PCI_DATA_REG = data;
		break;
	case 4:
    	if ((where >= 0x10) && (where <= 0x24)) {
    		if ((val & 0xfff00000) == IO_ADDRESS(SL2312_PCI_IO_BASE)) {
    			val &= 0x000fffff;
    			val |= SL2312_PCI_IO_BASE;
    		}
    	}
	    PCI_DATA_REG = val;
		break;
	}
	spin_unlock_irqrestore(&sl2312_pci_lock, flags);

//	printk("WRITE==> slot=%d fn=%d where=%d value=%x \n",PCI_SLOT(devfn),PCI_FUNC(devfn),where,val);
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops sl2312_pci_ops = {
	.read	= sl2312_read_config,
	.write	= sl2312_write_config,
};


int __init sl2312_pci_setup_resources(struct resource **resource)
{
	PCI_IOSIZE_REG = 0;		// 1M IO size
	PCI_CTRL_REG = 0x06;

	resource[0] = &pci_ioport_resource;
	resource[1] = &pci_iomem_resource;
	resource[2] = NULL;

	return 1;
}

//static int sl2312_pci_fault(unsigned long addr, struct pt_regs *regs)
//{
//	return 1;
//}


/**********************************************************************
 * MASK(disable) PCI interrupt
 *    0: PCI INTA, 1: PCI INTB, ...		// for Linux interrupt routing
 *   16: PERR								// for PCI module internal use
 *   17: SERR,.. respect to PCI CTRL2 REG
 **********************************************************************/
void sl2312_pci_mask_irq(unsigned int irq)
{
    struct pci_bus bus;
	unsigned int tmp;

    bus.number = 0;
    sl2312_read_config(&bus, 0, SL2312_PCI_CTRL2, 4, &tmp);
	if (irq < 16) {						// for linux int routing
		tmp &= ~(1 << (irq + 16 + 6));
	}
	else {
		tmp &= ~(1 << irq);
	}
    sl2312_write_config(&bus, 0, SL2312_PCI_CTRL2, 4, tmp);
}

/* UNMASK(enable) PCI interrupt */
void sl2312_pci_unmask_irq(unsigned int irq)
{
    struct pci_bus bus;
	unsigned int tmp;

    bus.number = 0;
    sl2312_read_config(&bus, 0, SL2312_PCI_CTRL2, 4, &tmp);
	if (irq < 16) {						// for linux int routing
		tmp |= (1 << (irq + 16 + 6));
	}
	else {
		tmp |= (1 << irq);
	}
    sl2312_write_config(&bus, 0, SL2312_PCI_CTRL2, 4, tmp);
}

/* Get PCI interrupt source */
int sl2312_pci_get_int_src(void)
{
    struct pci_bus bus;
	unsigned int tmp=0;

    bus.number = 0;
    sl2312_read_config(&bus, 0, SL2312_PCI_CTRL2, 4, &tmp);
	if (tmp & (1 << 28)) { 		// PCI INTA
        sl2312_write_config(&bus, 0, SL2312_PCI_CTRL2, 4, tmp);
		return IRQ_PCI_INTA;
	}
	if (tmp & (1 << 29)) {		// PCI INTB
        sl2312_write_config(&bus, 0, SL2312_PCI_CTRL2, 4, tmp);
		return IRQ_PCI_INTB;
	}
	if (tmp & (1 << 30)) {		// PCI INTC
        sl2312_write_config(&bus, 0, SL2312_PCI_CTRL2, 4, tmp);
		return IRQ_PCI_INTC;
	}
	if (tmp & (1 << 31)) {		// PCI INTD
        sl2312_write_config(&bus, 0, SL2312_PCI_CTRL2, 4, tmp);
		return IRQ_PCI_INTD;
	}
	// otherwise, it should be a PCI error
	return IRQ_PCI;
}

static irqreturn_t sl2312_pci_irq(int irq, void *devid)
{
    struct irq_desc *desc;
	struct irqaction *action;
	int retval = 0;

    return 1;

	irq = sl2312_pci_get_int_src();
	desc = &irq_desc[irq];
	action = desc->action;
	do {
		retval |= action->handler(irq, devid);
		action = action->next;
	} while (action);

    return 1;
}

//extern int (*external_fault)(unsigned long addr, struct pt_regs *regs);

void __init sl2312_pci_preinit(void)
{
    struct pci_bus bus;
	unsigned long flags;
	unsigned int temp;
	int ret;

	/*
	 * Hook in our fault handler for PCI errors
	 */
//	external_fault = sl2312_pci_fault;

	spin_lock_irqsave(&sl2312_pci_lock, flags);

	/*
	 * Grab the PCI interrupt.
	 */
	ret = request_irq(IRQ_PCI, sl2312_pci_irq, 0, "sl2312 pci int", NULL);
	if (ret)
		printk(KERN_ERR "PCI: unable to grab PCI error "
		       "interrupt: %d\n", ret);

	spin_unlock_irqrestore(&sl2312_pci_lock, flags);

	// setup pci bridge
    bus.number = 0;   /* device 0, function 0 */
	temp = (SL2312_PCI_DMA_MEM1_BASE & 0xfff00000) | (SL2312_PCI_DMA_MEM1_SIZE << 16);
    sl2312_write_config(&bus, 0, SL2312_PCI_MEM1_BASE_SIZE, 4, temp);
}

/*
 * 	No swizzle on SL2312
 */
static u8 __init sl2312_pci_swizzle(struct pci_dev *dev, u8 *pinp)
{
	return PCI_SLOT(dev->devfn);
}

/*
 * map the specified device/slot/pin to an IRQ.  This works out such
 * that slot 9 pin 1 is INT0, pin 2 is INT1, and slot 10 pin 1 is INT1.
 */
static int __init sl2312_pci_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	int intnr = ((slot  + (pin - 1)) & 3) + 4;  /* the IRQ number of PCI bridge */

	// printk("%s : slot = %d  pin = %d \n",__func__,slot,pin);
    switch (slot)
    {
        case 12:
        	if (pin==1)
        	{
		    	intnr = 3;
		    }
		    else
		    {
		    	intnr = 0;
		    }
            break;
        case 11:
		    intnr = (2 + (pin - 1)) & 3;
            break;
        case 10:
		    intnr = (1 + (pin - 1)) & 3;
            break;
        case  9:
		    intnr = (pin - 1) & 3;
            break;
    }
//	if (slot == 10)
//		intnr = (1 + (pin - 1)) & 3;
//	else if (slot == 9)
//		intnr = (pin - 1) & 3;
	return (IRQ_PCI_INTA + intnr);
}

struct pci_bus * __init sl2312_pci_scan_bus(int nr, struct pci_sys_data *sysdata)
{
	return (pci_scan_bus(0, &sl2312_pci_ops, sysdata));

}

int __init sl2312_pci_setup(int nr, struct pci_sys_data *sys)
{
	int ret = 0;

	if (nr == 0) {
		ret = sl2312_pci_setup_resources(sys->resource);
	}

	return ret;
}


struct hw_pci sl2312_pci __initdata = {
	.setup          =	sl2312_pci_setup,
	.preinit		=	sl2312_pci_preinit,
	.nr_controllers =   1,
	.swizzle		=	sl2312_pci_swizzle,
	.map_irq		=	sl2312_pci_map_irq,
	.scan           =   sl2312_pci_scan_bus,
};

static __init int __init sl2312_pci_init(void)
{
	if (machine_is_sl2312())
		pci_common_init(&sl2312_pci);
	return 0;
}

subsys_initcall(sl2312_pci_init);
