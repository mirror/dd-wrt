/*
 * PCI-E support for CNS3xxx
 *
 * Copyright 2008 Cavium Networks
 *		  Richard Liu <richard.liu@caviumnetworks.com>
 * Copyright 2010 MontaVista Software, LLC.
 *		  Anton Vorontsov <avorontsov@mvista.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/bug.h>
#include <linux/pci.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/interrupt.h>
#include <linux/ptrace.h>
#include <asm/mach/map.h>
#include <mach/cns3xxx.h>
#include "core.h"

enum cns3xxx_access_type {
	CNS3XXX_HOST_TYPE = 0,
	CNS3XXX_CFG0_TYPE,
	CNS3XXX_CFG1_TYPE,
	CNS3XXX_NUM_ACCESS_TYPES,
};

struct cns3xxx_pcie {
	struct map_desc cfg_bases[CNS3XXX_NUM_ACCESS_TYPES];
	unsigned int irqs[2];
	struct resource res_io;
	struct resource res_mem;
	struct hw_pci hw_pci;

	bool linked;
};

static struct cns3xxx_pcie cns3xxx_pcie[]; /* forward decl. */

static struct cns3xxx_pcie *sysdata_to_cnspci(void *sysdata)
{
	struct pci_sys_data *root = sysdata;

	return &cns3xxx_pcie[root->domain];
}

static struct cns3xxx_pcie *pdev_to_cnspci(const struct pci_dev *dev)
{
	return sysdata_to_cnspci(dev->sysdata);
}

static struct cns3xxx_pcie *pbus_to_cnspci(struct pci_bus *bus)
{
	return sysdata_to_cnspci(bus->sysdata);
}

static void __iomem *cns3xxx_pci_cfg_base(struct pci_bus *bus,
				  unsigned int devfn, int where)
{
	struct cns3xxx_pcie *cnspci = pbus_to_cnspci(bus);
	int busno = bus->number;
	int slot = PCI_SLOT(devfn);
	int offset;
	enum cns3xxx_access_type type;
	void __iomem *base;

	/* If there is no link, just show the CNS PCI bridge. */
	if (!cnspci->linked && (busno > 0 || slot > 0))
		return NULL;

	/*
	 * The CNS PCI bridge doesn't fit into the PCI hierarchy, though
	 * we still want to access it. For this to work, we must place
	 * the first device on the same bus as the CNS PCI bridge.
	 */
	if (busno == 0) {
		if (slot > 1)
			return NULL;
		type = slot;
	} else {
		type = CNS3XXX_CFG1_TYPE;
	}

	base = (void __iomem *)cnspci->cfg_bases[type].virtual;
	offset = ((busno & 0xf) << 20) | (devfn << 12) | (where & 0xffc);

	return base + offset;
}

static inline int check_master_abort(struct pci_bus *bus, unsigned int devfn, int where)
{
	struct cns3xxx_pcie *cnspci = pbus_to_cnspci(bus);

  /* check PCI-compatible status register after access */
	if (cnspci->linked) {
		u32 host_base, sreg, ereg;

		host_base = cnspci->cfg_bases[CNS3XXX_HOST_TYPE].virtual;
		sreg = __raw_readw(host_base + 0x6) & 0xF900;
		ereg = __raw_readl(host_base + 0x104); // Uncorrectable Error Status Reg

		if (sreg | ereg) {
			/* SREG:
			 *  BIT15 - Detected Parity Error
			 *  BIT14 - Signaled System Error
			 *  BIT13 - Received Master Abort
			 *  BIT12 - Received Target Abort
			 *  BIT11 - Signaled Target Abort
			 *  BIT08 - Master Data Parity Error
			 *
			 * EREG:
			 *  BIT20 - Unsupported Request
			 *  BIT19 - ECRC
			 *  BIT18 - Malformed TLP
			 *  BIT17 - Receiver Overflow
			 *  BIT16 - Unexpected Completion
			 *  BIT15 - Completer Abort
			 *  BIT14 - Completion Timeout
			 *  BIT13 - Flow Control Protocol Error
			 *  BIT12 - Poisoned TLP
			 *  BIT04 - Data Link Protocol Error
			 *
			 * TODO: see Documentation/pci-error-recovery.txt
			 *    implement error_detected handler
			 */
			printk(KERN_ERR "pci error: %04d:%02x:%02x.%02x sreg=0x%04x ereg=0x%08x", pci_domain_nr(bus), bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), sreg, ereg);
			if (sreg & BIT(15)) printk(" <PERR"); 
			if (sreg & BIT(14)) printk(" >SERR"); 
			if (sreg & BIT(13)) printk(" <MABRT"); 
			if (sreg & BIT(12)) printk(" <TABRT"); 
			if (sreg & BIT(11)) printk(" >TABRT"); 
			if (sreg & BIT( 8)) printk(" MPERR"); 

			if (ereg & BIT(20)) printk(" Unsup"); 
			if (ereg & BIT(19)) printk(" ECRC"); 
			if (ereg & BIT(18)) printk(" MTLP"); 
			if (ereg & BIT(17)) printk(" OFLOW"); 
			if (ereg & BIT(16)) printk(" Unex"); 
			if (ereg & BIT(15)) printk(" ABRT"); 
			if (ereg & BIT(14)) printk(" COMPTO"); 
			if (ereg & BIT(13)) printk(" FLOW"); 
			if (ereg & BIT(12)) printk(" PTLP"); 
			if (ereg & BIT( 4)) printk(" DLINK"); 
			printk("\n");
			pr_debug("%s failed port%d sreg=0x%04x\n", __func__,
				cnspci->hw_pci.domain, sreg);

			/* make sure the status bits are reset */
			__raw_writew(sreg, host_base + 6);
			__raw_writel(ereg, host_base + 0x104);
			return 1;
		}
	}
	else
		return 1;

  return 0;
}

static int cns3xxx_pci_read_config(struct pci_bus *bus, unsigned int devfn,
				   int where, int size, u32 *val)
{
	u32 v;
	void __iomem *base;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	base = cns3xxx_pci_cfg_base(bus, devfn, where);
	if (!base) {
		*val = 0xffffffff;
		return PCIBIOS_SUCCESSFUL;
	}

	v = __raw_readl(base);

	if (check_master_abort(bus, devfn, where)) {
		printk("<<%04d:%02x:%02x.%02x %02x(%d)= master_abort on read\n", pci_domain_nr(bus), bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
		return PCIBIOS_DEVICE_NOT_FOUND;
	}


	if (bus->number == 0 && devfn == 0 &&
			(where & 0xffc) == PCI_CLASS_REVISION) {
		/*
		 * RC's class is 0xb, but Linux PCI driver needs 0x604
		 * for a PCIe bridge. So we must fixup the class code
		 * to 0x604 here.
		 */
		v &= 0xff;
		v |= 0x604 << 16;
	}

	*val = (v >> shift) & mask;

	return PCIBIOS_SUCCESSFUL;
}

static int cns3xxx_pci_write_config(struct pci_bus *bus, unsigned int devfn,
				    int where, int size, u32 val)
{
	u32 v;
	void __iomem *base;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	base = cns3xxx_pci_cfg_base(bus, devfn, where);
	if (!base)
		return PCIBIOS_SUCCESSFUL;

	v = __raw_readl(base);

	if (check_master_abort(bus, devfn, where)) {
		printk("<<%04d:%02x:%02x.%02x %02x(%d)= master_abort on read\n", pci_domain_nr(bus), bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	v &= ~(mask << shift);
	v |= (val & mask) << shift;

	__raw_writel(v, base);

	if (check_master_abort(bus, devfn, where)) {
		printk(">>%04d:%02x:%02x.%02x %02x(%d)=0x%08x master_abort on write\n", pci_domain_nr(bus), bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size, val);
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	return PCIBIOS_SUCCESSFUL;
}

static int cns3xxx_pci_setup(int nr, struct pci_sys_data *sys)
{
	struct cns3xxx_pcie *cnspci = sysdata_to_cnspci(sys);
	struct resource *res_io = &cnspci->res_io;
	struct resource *res_mem = &cnspci->res_mem;

	BUG_ON(request_resource(&iomem_resource, res_io) ||
	       request_resource(&iomem_resource, res_mem));

	pci_add_resource_offset(&sys->resources, res_io, sys->io_offset);
	pci_add_resource_offset(&sys->resources, res_mem, sys->mem_offset);

	return 1;
}

struct pci_ops cns3xxx_pcie_ops = {
	.read = cns3xxx_pci_read_config,
	.write = cns3xxx_pci_write_config,
};

static int cns3xxx_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct cns3xxx_pcie *cnspci = pdev_to_cnspci(dev);
	int irq = cnspci->irqs[slot];

	pr_info("PCIe map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
		pci_domain_nr(dev->bus), dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), slot, pin, irq);

	return irq;
}

static struct cns3xxx_pcie cns3xxx_pcie[] = {
	[0] = {
		.cfg_bases = {
			[CNS3XXX_HOST_TYPE] = {
				.virtual = CNS3XXX_PCIE0_HOST_BASE_VIRT,
				.pfn = __phys_to_pfn(CNS3XXX_PCIE0_HOST_BASE),
				.length = SZ_16M,
				.type = MT_DEVICE,
			},
			[CNS3XXX_CFG0_TYPE] = {
				.virtual = CNS3XXX_PCIE0_CFG0_BASE_VIRT,
				.pfn = __phys_to_pfn(CNS3XXX_PCIE0_CFG0_BASE),
				.length = SZ_16M,
				.type = MT_DEVICE,
			},
			[CNS3XXX_CFG1_TYPE] = {
				.virtual = CNS3XXX_PCIE0_CFG1_BASE_VIRT,
				.pfn = __phys_to_pfn(CNS3XXX_PCIE0_CFG1_BASE),
				.length = SZ_16M,
				.type = MT_DEVICE,
			},
		},
		.res_io = {
			.name = "PCIe0 I/O space",
			.start = CNS3XXX_PCIE0_IO_BASE,
			.end = CNS3XXX_PCIE0_IO_BASE + SZ_16M - 1,
			.flags = IORESOURCE_IO,
		},
		.res_mem = {
			.name = "PCIe0 non-prefetchable",
			.start = CNS3XXX_PCIE0_MEM_BASE,
			.end = CNS3XXX_PCIE0_MEM_BASE + SZ_16M - 1,
			.flags = IORESOURCE_MEM,
		},
		.irqs = { IRQ_CNS3XXX_PCIE0_RC, IRQ_CNS3XXX_PCIE0_DEVICE, },
		.hw_pci = {
			.domain = 0,
			.nr_controllers = 1,
			.ops = &cns3xxx_pcie_ops,
			.setup = cns3xxx_pci_setup,
			.map_irq = cns3xxx_pcie_map_irq,
		},
	},
	[1] = {
		.cfg_bases = {
			[CNS3XXX_HOST_TYPE] = {
				.virtual = CNS3XXX_PCIE1_HOST_BASE_VIRT,
				.pfn = __phys_to_pfn(CNS3XXX_PCIE1_HOST_BASE),
				.length = SZ_16M,
				.type = MT_DEVICE,
			},
			[CNS3XXX_CFG0_TYPE] = {
				.virtual = CNS3XXX_PCIE1_CFG0_BASE_VIRT,
				.pfn = __phys_to_pfn(CNS3XXX_PCIE1_CFG0_BASE),
				.length = SZ_16M,
				.type = MT_DEVICE,
			},
			[CNS3XXX_CFG1_TYPE] = {
				.virtual = CNS3XXX_PCIE1_CFG1_BASE_VIRT,
				.pfn = __phys_to_pfn(CNS3XXX_PCIE1_CFG1_BASE),
				.length = SZ_16M,
				.type = MT_DEVICE,
			},
		},
		.res_io = {
			.name = "PCIe1 I/O space",
			.start = CNS3XXX_PCIE1_IO_BASE,
			.end = CNS3XXX_PCIE1_IO_BASE + SZ_16M - 1,
			.flags = IORESOURCE_IO,
		},
		.res_mem = {
			.name = "PCIe1 non-prefetchable",
			.start = CNS3XXX_PCIE1_MEM_BASE,
			.end = CNS3XXX_PCIE1_MEM_BASE + SZ_16M - 1,
			.flags = IORESOURCE_MEM,
		},
		.irqs = { IRQ_CNS3XXX_PCIE1_RC, IRQ_CNS3XXX_PCIE1_DEVICE, },
		.hw_pci = {
			.domain = 1,
			.nr_controllers = 1,
			.ops = &cns3xxx_pcie_ops,
			.setup = cns3xxx_pci_setup,
			.map_irq = cns3xxx_pcie_map_irq,
		},
	},
};

static void __init cns3xxx_pcie_check_link(struct cns3xxx_pcie *cnspci)
{
	int port = cnspci->hw_pci.domain;
	u32 reg;
	unsigned long time;

	reg = __raw_readl(MISC_PCIE_CTRL(port));
	/*
	 * Enable Application Request to 1, it will exit L1 automatically,
	 * but when chip back, it will use another clock, still can use 0x1.
	 */
	reg |= 0x3;
	__raw_writel(reg, MISC_PCIE_CTRL(port));

	pr_info("PCIe: Port[%d] Enable PCIe LTSSM\n", port);
	pr_info("PCIe: Port[%d] Check data link layer...", port);

	time = jiffies;
	while (1) {
		reg = __raw_readl(MISC_PCIE_PM_DEBUG(port));
		if (reg & 0x1) {
			pr_info("Link up.\n");
			cnspci->linked = 1;
			break;
		} else if (time_after(jiffies, time + 50)) {
			pr_info("Device not found.\n");
			break;
		}
	}
}

static void __init cns3xxx_pcie_hw_init(struct cns3xxx_pcie *cnspci)
{
	int port = cnspci->hw_pci.domain;
	struct pci_sys_data sd = {
		.domain = port,
	};
	struct pci_bus bus = {
		.number = 0,
		.ops = &cns3xxx_pcie_ops,
		.sysdata = &sd,
	};
	u32 io_base = cnspci->res_io.start >> 16;
	u32 mem_base = cnspci->res_mem.start >> 16;
	u32 host_base = cnspci->cfg_bases[CNS3XXX_HOST_TYPE].pfn;
	u32 cfg0_base = cnspci->cfg_bases[CNS3XXX_CFG0_TYPE].pfn;
	u32 devfn = 0;
	u8 tmp8,cp,u8tmp;
	u16 pos,u16tmp;
	u16 dc;

	host_base = (__pfn_to_phys(host_base) - 1) >> 16;
	cfg0_base = (__pfn_to_phys(cfg0_base) - 1) >> 16;

	pci_bus_write_config_byte(&bus, devfn, PCI_PRIMARY_BUS, 0);
	pci_bus_write_config_byte(&bus, devfn, PCI_SECONDARY_BUS, 1);
	pci_bus_write_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, 1);

	pci_bus_read_config_byte(&bus, devfn, PCI_PRIMARY_BUS, &tmp8);
	pci_bus_read_config_byte(&bus, devfn, PCI_SECONDARY_BUS, &tmp8);
	pci_bus_read_config_byte(&bus, devfn, PCI_SUBORDINATE_BUS, &tmp8);

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

	if (!cnspci->linked)
		return;

	pci_bus_read_config_byte(&bus, devfn, PCI_CAPABILITY_LIST, &cp);
	while (cp != 0) {
		pci_bus_read_config_byte(&bus, devfn, cp, &u8tmp);
		// Read Next ID
		pci_bus_read_config_word(&bus, devfn, cp, &u16tmp);
		cp = (u16tmp & 0xFF00) >> 8;
	}

	/* Set Device Max_Read_Request_Size to 128 byte */
	devfn = PCI_DEVFN(1, 0);
	pos = pci_bus_find_capability(&bus, devfn, PCI_CAP_ID_EXP);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, &dc);
	dc &= ~(0x3 << 12);	/* Clear Device Control Register [14:12] */
	pci_bus_write_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, dc);
	pci_bus_read_config_word(&bus, devfn, pos + PCI_EXP_DEVCTL, &dc);
	if (!(dc & (0x3 << 12)))
		pr_info("PCIe: Set Device Max_Read_Request_Size to 128 byte\n");

	/* Disable PCIe0 Interrupt Mask INTA to INTD */
	__raw_writel(~0x3FFF, MISC_PCIE_INT_MASK(port));
}

static int cns3xxx_pcie_abort_handler(unsigned long addr, unsigned int fsr,
				      struct pt_regs *regs)
{
#if 0
/* R14_ABORT = PC+4 for XSCALE but not ARM11MPCORE
 * ignore imprecise aborts and use PCI-compatible Status register to
 * determine errors instead
 */
	if (fsr & (1 << 10))
		regs->ARM_pc += 4;
#endif
	return 0;
}

static int __init cns3xxx_pcie_init(void)
{
	int i;

	pcibios_min_io = 0;
	pcibios_min_mem = 0;

	hook_fault_code(16 + 6, cns3xxx_pcie_abort_handler, SIGBUS, 0,
			"imprecise external abort");

	for (i = 0; i < ARRAY_SIZE(cns3xxx_pcie); i++) {

		iotable_init(cns3xxx_pcie[i].cfg_bases,
			     ARRAY_SIZE(cns3xxx_pcie[i].cfg_bases));
		cns3xxx_pcie_check_link(&cns3xxx_pcie[i]);
		if (cns3xxx_pcie[i].linked) {
			cns3xxx_pcie_hw_init(&cns3xxx_pcie[i]);
			pci_common_init(&cns3xxx_pcie[i].hw_pci);
		}
	}

	pci_assign_unassigned_resources();

	return 0;
}

device_initcall(cns3xxx_pcie_init);
