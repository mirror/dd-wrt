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

#if defined(CONFIG_MV78200)
#include "mv78200/mvSocUnitMap.h"
#endif

#ifdef MV_DEBUG
#	define DB(x) x
#else
#	define DB(x) 
#endif

#define MV_PEX_MASK_ABCD              (BIT24 | BIT25 | BIT26 | BIT27)

static int __init mv_map_irq_0(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_1(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_2(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_3(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_4(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_5(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_6(struct pci_dev *dev, u8 slot, u8 pin);
static int __init mv_map_irq_7(struct pci_dev *dev, u8 slot, u8 pin);
static void __init mv_pci_shutdown(unsigned int pciIf);

extern u32 mv_pci_mem_size_get(int ifNum);
extern u32 mv_pci_io_base_get(int ifNum);
extern u32 mv_pci_io_size_get(int ifNum);
extern u32 mv_pci_mem_base_get(int ifNum);
extern u32 mv_pci_irqnum_get(int ifNum);
extern int mv_is_pci_io_mapped(int ifNum);
extern MV_TARGET mv_pci_io_target_get(int ifNum);
extern int mv_pci_if_num_to_skip(void);

static void* mv_get_irqmap_func[] __initdata =
{
	mv_map_irq_0,
	mv_map_irq_1,
	mv_map_irq_2,
	mv_map_irq_3,
	mv_map_irq_4,
	mv_map_irq_5,
	mv_map_irq_6,
	mv_map_irq_7
};

#if defined(CONFIG_MV78200)
int mv_pci_is_mapped_to_this_cpu(int pciIf)
{
	if (pciIf < 4) 
	{
		if (MV_FALSE == mvSocUnitIsMappedToThisCpu(PEX00))
			return 0;
	}
	else
	{
		if (MV_FALSE == mvSocUnitIsMappedToThisCpu(PEX10))
			return 0;
	}
	return 1;
}
#endif

void __init mv_pci_preinit(void)
{
	unsigned int pciIf, temp;
	MV_ADDR_WIN pciIoRemap;
	MV_U32 		maxif = mvCtrlPciIfMaxIfGet();

	if (!PCI0_IS_QUAD_X1)
	{
		mvCtrlPwrClckSet(PEX_UNIT_ID, 1, MV_FALSE);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 2, MV_FALSE);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 3, MV_FALSE);		
	}

	if (!PCI1_IS_QUAD_X1)
	{
		mvCtrlPwrClckSet(PEX_UNIT_ID, 5, MV_FALSE);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 6, MV_FALSE);
		mvCtrlPwrClckSet(PEX_UNIT_ID, 7, MV_FALSE);
	}

	for (pciIf = 0; pciIf < maxif; pciIf++) 
	{
#ifdef CONFIG_MV78200
		if (!mv_pci_is_mapped_to_this_cpu(pciIf))
		{
			printk(KERN_INFO"PCI-E %d not mapped to this CPU.\n", pciIf);
			continue;
		}
#endif

		if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, pciIf))	
		{			
			continue;
		}
		if (pciIf == mv_pci_if_num_to_skip())
		{
			mv_pci_shutdown(pciIf);			
			continue;
		}
		/* init the PCI interface */
		temp = mvPexInit(pciIf, MV_PEX_ROOT_COMPLEX);
		if (MV_NO_SUCH == temp)
		{
			/*No Link - shutdown interface*/
			mv_pci_shutdown(pciIf);
			continue;
		}
		else if ((MV_OK != temp) && (MV_NO_SUCH != temp)){			
			printk("PCI-E %d: Init Failed.\n", pciIf);
		}
			
		MV_REG_BIT_SET(PEX_MASK_REG(pciIf), MV_PEX_MASK_ABCD);
		if (mv_is_pci_io_mapped(pciIf))
		{
			pciIoRemap.baseLow = mv_pci_io_base_get(pciIf) - IO_SPACE_REMAP;
			pciIoRemap.baseHigh = 0; 		
			pciIoRemap.size = mv_pci_io_size_get(pciIf);
			mvCpuIfPexRemap(mv_pci_io_target_get(pciIf), &pciIoRemap);
		}
	}
}


static void __init mv_pci_shutdown(unsigned int pciIf)
{
#ifdef CONFIG_MV78200
	if (!mv_pci_is_mapped_to_this_cpu(pciIf))
	{
		return;
	}
#endif
	if (pciIf < 4)
	{
		if (pciIf > 0 || !PCI0_IS_QUAD_X1)
		{
			mvCtrlPwrClckSet(PEX_UNIT_ID, pciIf, MV_FALSE);
		}
	} 
	else 
	{	
		if (pciIf > 4 || !PCI1_IS_QUAD_X1) 
		{
			mvCtrlPwrClckSet(PEX_UNIT_ID, pciIf, MV_FALSE);
		}
	}
}

static int is_pex_pci_bridge(u32 pciIf)
{
	if (mv_pci_if_num_to_skip() == 3)
		return 0;	
	return ((pciIf == 3)? (mvBoardIdGet() == DB_78XX0_ID || 
			       mvBoardIdGet() == DB_78200_ID) :0);
}



/* Currentlly the PCI config read/write are implemented as read modify write
   to 32 bit.
   TBD: adjust it to realy use 1/2/4 byte(partial) read/write, after the pex
	read config WA will be removed.
*/
static int mv_pci_read_config(struct pci_bus *bus, 
							  unsigned int devfn, int where,
							  int size, u32 *val)
{
	u32 bus_num,func,regOff,dev_no,temp, localBus;		
	struct pci_sys_data *sysdata = (struct pci_sys_data *)bus->sysdata;	
	u32 pciIf = sysdata->mv_controller_num;
	*val = 0xffffffff;
#ifdef CONFIG_MV78200
	if (!mv_pci_is_mapped_to_this_cpu(pciIf))
	{
		return 0;
	}	
#endif
	if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, pciIf) || 	
		(pciIf == mv_pci_if_num_to_skip()))
		return 0;
	bus_num = bus->number;
	dev_no = PCI_SLOT(devfn);

	/* don't return for our device */
	localBus = mvPexLocalBusNumGet(pciIf);
	if ((dev_no == 0) && ( bus_num == localBus))
	{
		DB(printk("PCI %d read from our own dev return 0xffffffff \n", pciIf));
		return 0xffffffff;
	}

	func = PCI_FUNC(devfn); 
	regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

	if (is_pex_pci_bridge(pciIf))
	{	
		/* WA: use only the first function of the bridge and the first bus*/
		if( (bus_num == mvPexLocalBusNumGet(pciIf)) && 
			(dev_no == 1) && (func != 0) )
		{
			DB(printk("PCI %d read from bridge func != 0 return 0xffffffff \n", pciIf));
			return 0xffffffff;
		}
	}
	DB(printk("PCI %d read: bus = %x dev = %x func = %x regOff = %x ",pciIf, bus_num,dev_no,func,regOff));
	
	temp = (u32) mvPexConfigRead(pciIf, bus_num, dev_no, func, regOff);
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

	DB(printk(" got %x \n",temp));
	
    return 0;
}

static int mv_pci_write_config(struct pci_bus *bus, unsigned int devfn, int where,
                           int size, u32 val)
{
	u32 bus_num,func,regOff,dev_no,temp, mask , shift;
	struct pci_sys_data *sysdata = (struct pci_sys_data *)bus->sysdata;	
	u32 pciIf = sysdata->mv_controller_num;		
#ifdef CONFIG_MV78200
	if (!mv_pci_is_mapped_to_this_cpu(pciIf))
	{
		return 0xFFFFFFFF;
	}
#endif
	if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, pciIf) || 
	    (pciIf == mv_pci_if_num_to_skip()))
		return 0xFFFFFFFF;
	bus_num = bus->number;
	dev_no = PCI_SLOT(devfn);
	func = PCI_FUNC(devfn);
	regOff = (MV_U32)where & PXCAR_REG_NUM_MASK;

	DB(printk("PCI %d: writing data %x size %x to bus %x dev %x func %x offs %x \n",
			  pciIf, val,size,bus_num,dev_no,func,regOff));
	if (size != 4)
	{
		temp = (u32) mvPexConfigRead(pciIf, bus_num, dev_no, func, regOff);
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
	mvPexConfigWrite(pciIf, bus_num, dev_no, func, regOff, temp);
	return 0;
}


static struct pci_ops mv_pci_ops = {
        .read   = mv_pci_read_config,
        .write  = mv_pci_write_config,
};


int __init mv_pci_setup(int nr, struct pci_sys_data *sys)
{
	struct resource *res;
	u32 membase, iobase, index = 0;	
#ifdef CONFIG_MV78200
	if (!mv_pci_is_mapped_to_this_cpu(nr))
	{
		return 0;
	}
#endif
	if (MV_FALSE == mvCtrlPwrClckGet(PEX_UNIT_ID, nr) || 
	    (nr == mv_pci_if_num_to_skip()))
		return 0;
	sys->map_irq = mv_get_irqmap_func[nr];

	res = kmalloc(sizeof(struct resource) * 2, GFP_KERNEL);
	if (!res)
	{
		panic("PCI: unable to alloc resources");
		return 0;
	}
                                                                                                                             
	memset(res, 0, sizeof(struct resource) * 2);
	
	membase = mv_pci_mem_base_get(nr);
	if (mv_is_pci_io_mapped(nr))
	{
	
		iobase = mv_pci_io_base_get(nr);
		res[index].start = iobase - IO_SPACE_REMAP;
		res[index].end   = iobase - IO_SPACE_REMAP + mv_pci_io_size_get(nr)-1;
		res[index].name  = "PCIx IO Primary";
		res[index].flags = IORESOURCE_IO;		
		if (request_resource(&ioport_resource, &res[index]))
		{	
			printk ("IO Request resource failed - Pci If %x\n",nr);
		}
		else
			index++;
	}
	res[index].start = membase;
	res[index].end   = membase + mv_pci_mem_size_get(nr)-1;
	res[index].name  = "PCIx Memory Primary";
	res[index].flags = IORESOURCE_MEM;

	if (request_resource(&iomem_resource, &res[index]))
	{	
		printk ("Memory Request resource failed - Pci If %x\n",nr);
	}
 
	sys->resource[0] = &res[0];
	if (index > 0) 
	{
		sys->resource[1] = &res[1];
		sys->resource[2] = NULL;
	}
	else
		sys->resource[1] = NULL;
	sys->io_offset   = 0x0;
	sys->mv_controller_num = nr;
	return 1;
}


struct pci_bus *mv_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
	struct pci_ops *ops = &mv_pci_ops;	
	struct pci_bus *bus;		
	int scanbus = sys->mv_controller_num;

	if (sys->mv_controller_num <= sys->busnr)
	{
		scanbus = sys->busnr;
	}
	bus = pci_scan_bus(scanbus, ops, sys);	
	if (scanbus < mvCtrlPciIfMaxIfGet() - 1)
	{
#ifdef CONFIG_MV78200
		if (!mv_pci_is_mapped_to_this_cpu(scanbus+1))
		{
			return bus;
		}
#endif

		if (MV_TRUE == mvCtrlPwrClckGet(PEX_UNIT_ID, scanbus+1) && 
		    (scanbus+1 != mv_pci_if_num_to_skip()))
		{
			mvPexLocalBusNumSet(scanbus+1, 
			      bus->number + bus->subordinate - bus->secondary + 1);		
		}	
	}
	return bus;
}


static int __init mv_map_irq_0(struct pci_dev *dev, u8 slot, u8 pin)
{	
	return mv_pci_irqnum_get(0);
}

static int __init mv_map_irq_1(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(1);
}

static int __init mv_map_irq_2(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(2);
}

static int __init mv_map_irq_3(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(3);
}

static int __init mv_map_irq_4(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(4);
}

static int __init mv_map_irq_5(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(5);
}

static int __init mv_map_irq_6(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(6);
}

static int __init mv_map_irq_7(struct pci_dev *dev, u8 slot, u8 pin)
{
	return mv_pci_irqnum_get(7);
}


static struct hw_pci mv_pci __initdata = {
	.swizzle        	= pci_std_swizzle,
        .setup                  = mv_pci_setup,
        .scan                   = mv_pci_scan_bus,
        .preinit                = mv_pci_preinit,
};


static int __init mv_pci_init(void)
{
    mv_pci.nr_controllers = mvCtrlPciIfMaxIfGet();
    mv_pci.swizzle        = pci_std_swizzle;
    mv_pci.map_irq         = mv_map_irq_0;
    mv_pci.setup           = mv_pci_setup;
    mv_pci.scan            = mv_pci_scan_bus;
    mv_pci.preinit         = mv_pci_preinit;
    pci_common_init(&mv_pci);
    return 0;
}


subsys_initcall(mv_pci_init);

