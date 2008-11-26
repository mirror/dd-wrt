/*
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
#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>
                                                                                                                             
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <asm/arch/irqs.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvSysPex.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"

#undef DEBUG
#ifdef DEBUG
#	define DB(x) x
#else
#	define DB(x) 
#endif

static int __init mv_pri_map_irq(struct pci_dev *dev, u8 slot, u8 pin);

void __init mv_pci_preinit(void)
{
	MV_ADDR_WIN win;

	/* Check Power State */
	if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, 0))	
		return;

        if (mvPexInit(0, MV_PEX_ROOT_COMPLEX) != MV_OK)
        {
               	printk("pci_init:Error calling mvPexInit for PEX-0\n");
        }

	/* unmask inter A/B/C/D */
	MV_REG_WRITE(MV_PCI_MASK_REG, MV_PCI_MASK_ABCD );

	/* remmap IO !! */
	win.baseLow = 0x0;
	win.baseHigh = 0x0;
	mvCpuIfPciIfRemap(PCI_IF0_IO, &win);
}


/* Currentlly the PCI config read/write are implemented as read modify write
   to 32 bit.
   TBD: adjust it to realy use 1/2/4 byte(partial) read/write, after the pex
	read config WA will be removed.
*/
static int mv_pci0_read_config(struct pci_bus *bus, unsigned int devfn, int where,
                          int size, u32 *val)
{

        MV_U32 bus_num,func,regOff,dev_no,temp;
	MV_U32 localBus;
 
	*val = 0xffffffff;

	/* Check Power State */
	if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, 0))
		return 0;

        bus_num = bus->number;
        dev_no = PCI_SLOT(devfn);
 
	/* don't return for our device */
	localBus = mvPexLocalBusNumGet(0);
	if((dev_no == 0) && ( bus_num == localBus))
	{
		DB(printk("PCI 0 read from our own dev return 0xffffffff \n"));
		return 0xffffffff;
	}

        func = PCI_FUNC(devfn); 
        regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

#if PCI0_IF_PTP
	/* WA: use only the first function of the bridge and te first bus*/
	if( (bus_num == mvPexLocalBusNumGet(0)) && (dev_no == 1) && (func != 0) )
	{
		DB(printk("PCI 0 read from bridge func != 0 return 0xffffffff \n"));
		return 0xffffffff;
	}
#endif
	if ((func == 0)&&(dev_no < 2))
	{
		DB(printk("PCI 0 read: bus = %x dev = %x func = %x regOff = %x ",bus_num,dev_no,func,regOff));
	}
	

        temp = (u32) mvPexConfigRead(0, bus_num, dev_no, func, regOff);

        switch (size) {
        case 1:
                temp = (temp >>  (8*(where & 0x3))) & 0xff;
                break;
 
        case 2:
                temp = (temp >>  (8*(where & 0x2))) & 0xffff;
                break;
 
        default:
                break;
        }
	
	
	*val = temp;

	if ((func == 0)&&(dev_no < 2))
	{
		DB(printk(" got %x \n",temp));
	}
	
        return 0;
}

static int mv_pci0_write_config(struct pci_bus *bus, unsigned int devfn, int where,
                           int size, u32 val)
{
        MV_U32 bus_num,func,regOff,dev_no,temp, mask , shift;
 
	bus_num = bus->number;
	dev_no = PCI_SLOT(devfn); 
	func = PCI_FUNC(devfn); 
	regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

	DB(printk("PCI 0: writing data %x size %x to bus %x dev %x func %x offs %x \n",val,size,bus_num,dev_no,func,regOff));
	if( size != 4)
	{
        	temp = (u32) mvPexConfigRead(0, bus_num, dev_no, func, regOff);
	}
	else
	{
		temp = val;
	}

        switch (size) {
        case 1:
		shift = (8*(where & 0x3));
		mask = 0xff;
                break;
 
        case 2:
		shift = (8*(where & 0x2));
                mask = 0xffff; 
                break;
 
        default:
		shift = 0;
		mask = 0xffffffff;
                break;
        }
	
	temp = (temp & (~(mask<<shift))) | ((val & mask) << shift);
	mvPexConfigWrite(0,bus_num,dev_no,func,regOff,temp);

        return 0;

}




static struct pci_ops mv_primary_ops = {
        .read   = mv_pci0_read_config,
        .write  = mv_pci0_write_config,
};


int __init mv_pci_setup(int nr, struct pci_sys_data *sys)
{
        struct resource *res;

        switch (nr) {
        case 0:
                sys->map_irq = mv_pri_map_irq;
                break;
        default:
		printk("mv_pci_setup: nr(%d) out of scope\n",nr);
                return 0;
        }

	res = kmalloc(sizeof(struct resource) * 2, GFP_KERNEL);
        if (!res)
                panic("PCI: unable to alloc resources");
                                                                                                                             
        memset(res, 0, sizeof(struct resource) * 2);
                                                                                                                             
        res[0].start = PEX0_IO_BASE - IO_SPACE_REMAP;
        res[0].end   = PEX0_IO_BASE - IO_SPACE_REMAP + PEX0_IO_SIZE - 1;
        res[0].name  = "PEX IO";
        res[0].flags = IORESOURCE_IO;
                                                                                                                             
        res[1].start = PEX0_MEM_BASE;
        res[1].end   = PEX0_MEM_BASE + PEX0_MEM_SIZE - 1;
        res[1].name  = "PEX Memory";
        res[1].flags = IORESOURCE_MEM;
     
        if (request_resource(&ioport_resource, &res[0]))
	{	
		printk ("IO Request resource failed - Pci If %x\n",nr);
	}
	if (request_resource(&iomem_resource, &res[1]))
	{	
		printk ("Memory Request resource failed - Pci If %x\n",nr);
	}
 
        sys->resource[0] = &res[0];
        sys->resource[1] = &res[1];
        sys->resource[2] = NULL;
        sys->io_offset   = 0x0;
 
        return 1;

}

struct pci_bus *mv_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
	struct pci_ops *ops;
	struct pci_bus *bus;

        if (nr > 0) {
		return NULL;
	}
      
        ops = &mv_primary_ops;
	bus = pci_scan_bus(sys->busnr, ops, sys);

	return bus;
}




static int __init mv_pri_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	return IRQ_PEX0_INT;
}


static struct hw_pci mv_pci __initdata = {
	.swizzle        	= pci_std_swizzle,
        .map_irq                = mv_pri_map_irq,
        .setup                  = mv_pci_setup,
        .scan                   = mv_pci_scan_bus,
        .preinit                = mv_pci_preinit,
};
 
static int __init mv_pci_init(void)
{
    mv_pci.nr_controllers = MV_PCI_IF_MAX_IF; 
    pci_common_init(&mv_pci);

    return 0;
}


subsys_initcall(mv_pci_init);

