/*******************************************************************************
 *
 *  Copyright (c) 2008 Cavium Networks 
 * 
 *  This file is free software; you can redistribute it and/or modify 
 *  it under the terms of the GNU General Public License, Version 2, as 
 *  published by the Free Software Foundation. 
 *
 *  This file is distributed in the hope that it will be useful, 
 *  but AS-IS and WITHOUT ANY WARRANTY; without even the implied warranty of 
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, TITLE, or 
 *  NONINFRINGEMENT.  See the GNU General Public License for more details. 
 *
 *  You should have received a copy of the GNU General Public License 
 *  along with this file; if not, write to the Free Software 
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA or 
 *  visit http://www.gnu.org/licenses/. 
 *
 *  This file may also be available under a different license from Cavium. 
 *  Contact Cavium Networks for more information
 *
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/mach/pci.h>
#include <mach/cns3xxx.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <mach/pm.h>

DEFINE_SPINLOCK(pci_config_lock);

static int pcie_linked[2] = {0, 0};	// if 1, mean link ok. 

u32 cns3xxx_pcie0_irqs[2] = { IRQ_CNS3XXX_PCIE0_RC, IRQ_CNS3XXX_PCIE0_DEVICE,  };
u32 cns3xxx_pcie1_irqs[2] = { IRQ_CNS3XXX_PCIE1_RC, IRQ_CNS3XXX_PCIE1_DEVICE,  };

static u32 access_base[2][3] = { 
	{ CNS3XXX_PCIE0_HOST_BASE_VIRT, CNS3XXX_PCIE0_CFG0_BASE_VIRT, CNS3XXX_PCIE0_CFG1_BASE_VIRT},
	{ CNS3XXX_PCIE1_HOST_BASE_VIRT, CNS3XXX_PCIE1_CFG0_BASE_VIRT, CNS3XXX_PCIE1_CFG1_BASE_VIRT},
};

static int cns3xxx_pci_cfg_base(struct pci_bus *bus,
					 unsigned int devfn, int where)
{
	int domain = pci_domain_nr(bus);
	int slot = PCI_SLOT(devfn);	
	u32 base;

	if ((!pcie_linked[domain]) && (bus->number || slot))
		return 0;

	if (!(bus->number)) {
		if (slot > 1)
			return 0;
		// CFG0 Type
		base = access_base[domain][slot];
	} else {
		// CFG1 Type
		base = access_base[domain][2];
	}
	base += (((bus->number & 0xf) << 20)| (devfn << 12) | (where & 0xfc));
	return base;
}

static int cns3xxx_pci_read_config(struct pci_bus *bus,
				   unsigned int devfn, int where, int size,
				   u32 * val)
{
	u32 v = 0xffffffff;
	u32 base;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;	

	base = cns3xxx_pci_cfg_base(bus, devfn, where);
	if (!base) {
		*val = 0xFFFFFFFF;
		return PCIBIOS_SUCCESSFUL;
	}

	v = __raw_readl(base);
	if (bus->number == 0 && devfn == 0 &&
			(where & 0xffc) == PCI_CLASS_REVISION) {
  	/* RC's class is 0xb, but Linux PCI driver needs 0x604 for a PCIe bridge. */
		/* So we must dedicate the class code to 0x604 here */
		v &= 0xff;
		v |= (0x604 << 16);    		    
	} 

	*val = (v >> shift) & mask;
	return PCIBIOS_SUCCESSFUL;
}

static int cns3xxx_pci_write_config(struct pci_bus *bus,
				    unsigned int devfn, int where, int size,
				    u32 val)
{
	u32 v;
	u32 base;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;	

	base = cns3xxx_pci_cfg_base(bus, devfn, where);
	if (!base)
		return PCIBIOS_SUCCESSFUL;
	
	v = __raw_readl(base);
	v &= ~(mask << shift);
	v |= (val & mask) << shift;
	__raw_writel(v, base);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops cns3xxx_pcie_ops = {
	.read = cns3xxx_pci_read_config,
	.write = cns3xxx_pci_write_config,
};

static struct resource cns3xxx_pcie0_io = {
	.name = "PCIe0 I/O space",
	.start = CNS3XXX_PCIE0_IO_BASE,
	.end = CNS3XXX_PCIE0_IO_BASE + SZ_16M - 1,
	.flags = IORESOURCE_IO,
};

static struct resource cns3xxx_pcie1_io = {
	.name = "PCIe1 I/O space",
	.start = CNS3XXX_PCIE1_IO_BASE,
	.end = CNS3XXX_PCIE1_IO_BASE + SZ_16M - 1,
	.flags = IORESOURCE_IO,
};

static struct resource cns3xxx_pcie0_mem = {
	.name = "PCIe0 non-prefetchable",
	.start = CNS3XXX_PCIE0_MEM_BASE,
	.end = CNS3XXX_PCIE0_MEM_BASE + SZ_16M - 1,
	.flags = IORESOURCE_MEM,
};

static struct resource cns3xxx_pcie1_mem = {
	.name = "PCIe1 non-prefetchable",
	.start = CNS3XXX_PCIE1_MEM_BASE,
	.end = CNS3XXX_PCIE1_MEM_BASE + SZ_16M - 1,
	.flags = IORESOURCE_MEM,
};

static int __init cns3xxx_pci_setup_resources(int nr, struct resource **resource)
{
	if(nr==0){
		BUG_ON(request_resource(&iomem_resource, &cns3xxx_pcie0_io) ||
					 request_resource(&iomem_resource, &cns3xxx_pcie0_mem));
		resource[0] = &cns3xxx_pcie0_io;
		resource[1] = &cns3xxx_pcie0_mem;
	}else{
		BUG_ON(request_resource(&iomem_resource, &cns3xxx_pcie1_io) ||
					 request_resource(&iomem_resource, &cns3xxx_pcie1_mem));
		resource[0] = &cns3xxx_pcie1_io;
		resource[1] = &cns3xxx_pcie1_mem;
	}
	return 0;
}

int __init cns3xxx_pci_setup(int nr, struct pci_sys_data *sys)
{
	BUG_ON(cns3xxx_pci_setup_resources(sys->domain,sys->resource));
	return 1;
}

struct pci_bus *cns3xxx_pci_scan_bus(int nr, struct pci_sys_data *sys)
{
  struct pci_bus *ret;
  ret = pci_scan_bus(sys->busnr, &cns3xxx_pcie_ops, sys);
  pci_assign_unassigned_resources();
  return ret;
}

/* 
 *   CNS3XXX PCIe device don't support hotplugin, and we will check the link at start up. 
 *
 */

#define CNS3XXX_PCIE0_PM_DEBUG			(CNS3XXX_MISC_BASE_VIRT + 0x960)
#define CNS3XXX_PCIE1_PM_DEBUG			(CNS3XXX_MISC_BASE_VIRT + 0xA60)

static void cns3xxx_pcie_check_link(int port)
{

	u32 reg;
	u32 time;

	time = jiffies;		/* set the start time for the receive */
	while (1) {
		reg = __raw_readl( port == 0 ? CNS3XXX_PCIE0_PM_DEBUG : CNS3XXX_PCIE1_PM_DEBUG);	/* check link up */
		reg = __raw_readl( port == 0 ? CNS3XXX_PCIE0_PM_DEBUG : CNS3XXX_PCIE1_PM_DEBUG);
		if (reg & 0x1) {
			pcie_linked[port]++;
			break;
		} else if (time_after(jiffies, (unsigned long)(time + 50))) {
			break;
		}
	}

}

static void cns3xxx_pcie_hw_init(int port){
	struct pci_bus bus;
	struct pci_sys_data sd;
	u32 devfn = 0;
	u8 pri_bus, sec_bus, sub_bus;
	u8 cp, u8tmp;
	u16 u16tmp,pos,dc;
	u32 mem_base, host_base, io_base, cfg0_base;

	bus.number = 0; 
	bus.ops    = &cns3xxx_pcie_ops;
	sd.domain = port;
	bus.sysdata = &sd;	

	mem_base = ( port == 0 ? CNS3XXX_PCIE0_MEM_BASE : CNS3XXX_PCIE1_MEM_BASE );
	mem_base = mem_base >> 16;

	io_base = ( port == 0 ? CNS3XXX_PCIE0_IO_BASE : CNS3XXX_PCIE1_IO_BASE );
	io_base = io_base >> 16;

	host_base = ( port == 0 ? CNS3XXX_PCIE0_HOST_BASE_VIRT : CNS3XXX_PCIE1_HOST_BASE_VIRT );
	host_base = ( host_base -1 ) >> 16;

	cfg0_base = ( port == 0 ? CNS3XXX_PCIE0_CFG0_BASE_VIRT : CNS3XXX_PCIE1_CFG0_BASE_VIRT );
	cfg0_base = ( cfg0_base -1 ) >> 16;

	pci_bus_write_config_byte(&bus, devfn, PCI_PRIMARY_BUS, 0);
	pci_bus_write_config_byte(&bus, devfn, PCI_SECONDARY_BUS, 1);
	pci_bus_write_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, 1);

	pci_bus_read_config_byte(&bus, devfn, PCI_PRIMARY_BUS, &pri_bus);
	pci_bus_read_config_byte(&bus, devfn, PCI_SECONDARY_BUS, &sec_bus);
	pci_bus_read_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, &sub_bus);

	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_BASE, mem_base);
	pci_bus_write_config_word(&bus, devfn, PCI_MEMORY_LIMIT, host_base);
	pci_bus_write_config_word(&bus, devfn, PCI_IO_BASE_UPPER16, io_base);
	pci_bus_write_config_word(&bus, devfn, PCI_IO_LIMIT_UPPER16, cfg0_base);

	pci_bus_read_config_byte(&bus, devfn, PCI_CAPABILITY_LIST, &cp);
	while (cp != 0) {
		pci_bus_read_config_byte(&bus, devfn, cp, &u8tmp);
		// Read Next ID
		pci_bus_read_config_word(&bus, devfn, cp, &u16tmp);
		cp = (u16tmp & 0xFF00) >> 8;
	}

	/* Modify device's Max_Read_Request size */
	devfn = PCI_DEVFN(1,0);
	if (!pcie_linked[port])
		return;
		
	pci_bus_read_config_byte(&bus, devfn, PCI_CAPABILITY_LIST, &cp);
	while (cp != 0) {
		pci_bus_read_config_byte(&bus, devfn, cp, &u8tmp);
		// Read Next ID
		pci_bus_read_config_word(&bus, devfn, cp, &u16tmp);
		cp = (u16tmp & 0xFF00) >> 8;
	}

	/* Set Device Max_Read_Request_Size to 128 byte */
	pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, &dc);
	dc &= ~(0x3 << 12);	/* Clear Device Control Register [14:12] */
	pci_bus_write_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, dc);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, &dc);
	
	if (!port) {
		/* Disable PCIe0 Interrupt Mask INTA to INTD */
		__raw_writel(~0x3FFF, CNS3XXX_MISC_BASE_VIRT + 0x978);
	} else {
		/* Disable PCIe1 Interrupt Mask INTA to INTD */
		__raw_writel(~0x3FFF, CNS3XXX_MISC_BASE_VIRT + 0xA78);
	}
}


void __init cns3xxx_pcie0_preinit(void)
{
	cns3xxx_pcie_check_link(0);
	cns3xxx_pcie_hw_init(0);
}

void __init cns3xxx_pcie1_preinit(void)
{
	cns3xxx_pcie_check_link(1);
	cns3xxx_pcie_hw_init(1);
}

/*
 * map the specified device/slot/pin to an IRQ.   Different backplanes may need to modify this.
 */

static int __init cns3xxx_pcie0_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	return cns3xxx_pcie0_irqs[slot];
}

static int __init cns3xxx_pcie1_map_irq(struct pci_dev *dev, u8 slot, u8 pin)
{
	return cns3xxx_pcie1_irqs[slot];
}

static struct hw_pci cns3xxx_pcie[2] __initdata = {
	{
		.swizzle = pci_std_swizzle,
		.map_irq = cns3xxx_pcie0_map_irq,
		.nr_controllers = 1,
		.domain = 0,
		.setup = cns3xxx_pci_setup,
		.scan = cns3xxx_pci_scan_bus,
		.preinit = cns3xxx_pcie0_preinit,
	},
	{
		.swizzle = pci_std_swizzle,
		.map_irq = cns3xxx_pcie1_map_irq,
		.nr_controllers = 1,
		.domain = 1,
		.setup = cns3xxx_pci_setup,
		.scan = cns3xxx_pci_scan_bus,
		.preinit = cns3xxx_pcie1_preinit,
	}
};

static int cns3xxx_pcie_abort_handler(unsigned long addr, unsigned int fsr,
                                      struct pt_regs *regs)
{
  if (fsr & (1 << 10))
    regs->ARM_pc += 4;
  return 0;
}

//extern void pci_common_init(struct hw_pci *);
int cns3xxx_pcie_init(u8 ports)
{
	hook_fault_code(16 + 6, cns3xxx_pcie_abort_handler, SIGBUS, 0, "imprecise external abort");

	if (ports & 0x1)
		pci_common_init(&cns3xxx_pcie[0]);
	if (ports & 0x2)
		pci_common_init(&cns3xxx_pcie[1]);

	return 0;
}

//device_initcall(cns3xxx_pcie_init);
