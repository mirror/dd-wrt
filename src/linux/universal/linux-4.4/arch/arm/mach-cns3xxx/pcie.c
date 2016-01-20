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
#include <linux/irq.h>
#include <linux/ptrace.h>
#include <asm/mach/map.h>
#include <mach/cns3xxx.h>
#include "core.h"

struct cns3xxx_pcie {
	void __iomem *host_regs; /* PCI config registers for host bridge */
	void __iomem *cfg0_regs; /* PCI Type 0 config registers */
	void __iomem *cfg1_regs; /* PCI Type 1 config registers */
	unsigned int irqs[5];
	struct resource res_io;
	struct resource res_mem;
	int port;
	bool linked;
};

static struct cns3xxx_pcie *sysdata_to_cnspci(void *sysdata)
{
	struct pci_sys_data *root = sysdata;

	return root->private_data;
}

static struct cns3xxx_pcie *pdev_to_cnspci(const struct pci_dev *dev)
{
	return sysdata_to_cnspci(dev->sysdata);
}

static struct cns3xxx_pcie *pbus_to_cnspci(struct pci_bus *bus)
{
	return sysdata_to_cnspci(bus->sysdata);
}

static void __iomem *cns3xxx_pci_map_bus(struct pci_bus *bus,
					 unsigned int devfn, int where)
{
	struct cns3xxx_pcie *cnspci = pbus_to_cnspci(bus);
	int busno = bus->number;
	int slot = PCI_SLOT(devfn);
	void __iomem *base;

	/* If there is no link, just show the CNS PCI bridge. */
	if (!cnspci->linked && busno > 0)
		return NULL;

	/*
	 * The CNS PCI bridge doesn't fit into the PCI hierarchy, though
	 * we still want to access it.
	 * We place the host bridge on bus 0, and the directly connected
	 * device on bus 1, slot 0.
	 */
	if (busno == 0) { /* internal PCIe bus, host bridge device */
		if (devfn == 0) /* device# and function# are ignored by hw */
			base = cnspci->host_regs;
		else
			return NULL; /* no such device */

	} else if (busno == 1) { /* directly connected PCIe device */
		if (slot == 0) /* device# is ignored by hw */
			base = cnspci->cfg0_regs;
		else
			return NULL; /* no such device */
	} else /* remote PCI bus */
		base = cnspci->cfg1_regs + ((busno & 0xf) << 20);

	return base + (where & 0xffc) + (devfn << 12);
}

static inline int check_master_abort(struct pci_bus *bus, unsigned int devfn, int where)
{
	struct cns3xxx_pcie *cnspci = pbus_to_cnspci(bus);

  /* check PCI-compatible status register after access */
	if (cnspci->linked) {
		void __iomem *host_base;
		u32 sreg, ereg;

		host_base = (void __iomem *) cnspci->host_regs;
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
/*
			printk("pci error: %04d:%02x:%02x.%02x sreg=0x%04x ereg=0x%08x", pci_domain_nr(bus), bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), sreg, ereg);
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
*/
			pr_debug("%s failed port%d sreg=0x%04x\n", __func__,
				pci_domain_nr(bus), sreg);

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
	int ret;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	ret = pci_generic_config_read32(bus, devfn, where, size, val);

	if (check_master_abort(bus, devfn, where)) {
		printk(KERN_ERR "pci error: %04d:%02x:%02x.%02x %02x(%d)= master_abort on read\n", pci_domain_nr(bus), bus->number, PCI_SLOT(devfn), PCI_FUNC(devfn), where, size);
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	if (ret == PCIBIOS_SUCCESSFUL && !bus->number && !devfn &&
	    (where & 0xffc) == PCI_CLASS_REVISION)
		/*
		 * RC's class is 0xb, but Linux PCI driver needs 0x604
		 * for a PCIe bridge. So we must fixup the class code
		 * to 0x604 here.
		 */
		*val = ((((*val << shift) & 0xff) | (0x604 << 16)) >> shift) & mask;

	return ret;
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

static struct pci_ops cns3xxx_pcie_ops = {
	.map_bus = cns3xxx_pci_map_bus,
	.read = cns3xxx_pci_read_config,
	.write = pci_generic_config_write,
};

static int cns3xxx_pcie_map_irq(const struct pci_dev *dev, u8 slot, u8 pin)
{
	struct cns3xxx_pcie *cnspci = pdev_to_cnspci(dev);
	int irq = cnspci->irqs[!!dev->bus->number + pin - 1];

	pr_info("PCIe map irq: %04d:%02x:%02x.%02x slot %d, pin %d, irq: %d\n",
		pci_domain_nr(dev->bus), dev->bus->number, PCI_SLOT(dev->devfn),
		PCI_FUNC(dev->devfn), slot, pin, irq);

	return irq;
}

static struct cns3xxx_pcie cns3xxx_pcie[] = {
	[0] = {
		.host_regs = (void __iomem *)CNS3XXX_PCIE0_HOST_BASE_VIRT,
		.cfg0_regs = (void __iomem *)CNS3XXX_PCIE0_CFG0_BASE_VIRT,
		.cfg1_regs = (void __iomem *)CNS3XXX_PCIE0_CFG1_BASE_VIRT,
		.res_io = {
			.name = "PCIe0 I/O space",
			.start = CNS3XXX_PCIE0_IO_BASE,
			.end = CNS3XXX_PCIE0_CFG0_BASE - 1, /* 16 MiB */
			.flags = IORESOURCE_IO,
		},
		.res_mem = {
			.name = "PCIe0 non-prefetchable",
			.start = CNS3XXX_PCIE0_MEM_BASE,
			.end = CNS3XXX_PCIE0_HOST_BASE - 1, /* 176 MiB */
			.flags = IORESOURCE_MEM,
		},
		.irqs = {
			IRQ_CNS3XXX_PCIE0_RC,
			IRQ_CNS3XXX_PCIE0_DEVICE,
			IRQ_CNS3XXX_PCIE0_DEVICE,
			IRQ_CNS3XXX_PCIE0_DEVICE,
			IRQ_CNS3XXX_PCIE0_DEVICE,
		},
		.port = 0,
	},
	[1] = {
		.host_regs = (void __iomem *)CNS3XXX_PCIE1_HOST_BASE_VIRT,
		.cfg0_regs = (void __iomem *)CNS3XXX_PCIE1_CFG0_BASE_VIRT,
		.cfg1_regs = (void __iomem *)CNS3XXX_PCIE1_CFG1_BASE_VIRT,
		.res_io = {
			.name = "PCIe1 I/O space",
			.start = CNS3XXX_PCIE1_IO_BASE,
			.end = CNS3XXX_PCIE1_CFG0_BASE - 1, /* 16 MiB */
			.flags = IORESOURCE_IO,
		},
		.res_mem = {
			.name = "PCIe1 non-prefetchable",
			.start = CNS3XXX_PCIE1_MEM_BASE,
			.end = CNS3XXX_PCIE1_HOST_BASE - 1, /* 176 MiB */
			.flags = IORESOURCE_MEM,
		},
		.irqs = {
			IRQ_CNS3XXX_PCIE1_RC,
			IRQ_CNS3XXX_PCIE1_DEVICE,
			IRQ_CNS3XXX_PCIE1_DEVICE,
			IRQ_CNS3XXX_PCIE1_DEVICE,
			IRQ_CNS3XXX_PCIE1_DEVICE,
		},
		.port = 1,
	},
};

static void __init cns3xxx_pcie_check_link(struct cns3xxx_pcie *cnspci)
{
	int port = cnspci->port;
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

static void cns3xxx_write_config(struct cns3xxx_pcie *cnspci,
					 int where, int size, u32 val)
{
	void __iomem *base = cnspci->host_regs + (where & 0xffc);
	u32 v;
	u32 mask = (0x1ull << (size * 8)) - 1;
	int shift = (where % 4) * 8;

	v = readl_relaxed(base + (where & 0xffc));

	v &= ~(mask << shift);
	v |= (val & mask) << shift;

	writel_relaxed(v, base + (where & 0xffc));
	readl_relaxed(base + (where & 0xffc));
}

static void __init cns3xxx_pcie_hw_init(struct cns3xxx_pcie *cnspci)
{
	u16 mem_base  = cnspci->res_mem.start >> 16;
	u16 mem_limit = cnspci->res_mem.end   >> 16;
	u16 io_base   = cnspci->res_io.start  >> 16;
	u16 io_limit  = cnspci->res_io.end    >> 16;

	cns3xxx_write_config(cnspci, PCI_PRIMARY_BUS, 1, 0);
	cns3xxx_write_config(cnspci, PCI_SECONDARY_BUS, 1, 1);
	cns3xxx_write_config(cnspci, PCI_SUBORDINATE_BUS, 1, 1);
	cns3xxx_write_config(cnspci, PCI_MEMORY_BASE, 2, mem_base);
	cns3xxx_write_config(cnspci, PCI_MEMORY_LIMIT, 2, mem_limit);
	cns3xxx_write_config(cnspci, PCI_IO_BASE_UPPER16, 2, io_base);
	cns3xxx_write_config(cnspci, PCI_IO_LIMIT_UPPER16, 2, io_limit);

	if (!cnspci->linked)
		return;

	/* Set Device Max_Read_Request_Size to 128 byte */
	pcie_bus_config = PCIE_BUS_PEER2PEER;

	/* Disable PCIe0 Interrupt Mask INTA to INTD */
	__raw_writel(~0x3FFF, MISC_PCIE_INT_MASK(cnspci->port));
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

void __init cns3xxx_pcie_set_irqs(int bus, int *irqs)
{
	int i;

	for (i = 0; i < 4; i++)
		cns3xxx_pcie[bus].irqs[i + 1] = irqs[i];
}

void __init cns3xxx_pcie_init_late(void)
{
	int i;
	void *private_data;
	struct hw_pci hw_pci = {
	       .nr_controllers = 1,
	       .ops = &cns3xxx_pcie_ops,
	       .setup = cns3xxx_pci_setup,
	       .map_irq = cns3xxx_pcie_map_irq,
	       .private_data = &private_data,
	};

	pcibios_min_io = 0;
	pcibios_min_mem = 0;

	hook_fault_code(16 + 6, cns3xxx_pcie_abort_handler, SIGBUS, 0,
			"imprecise external abort");

	for (i = 0; i < ARRAY_SIZE(cns3xxx_pcie); i++) {
		cns3xxx_pcie_check_link(&cns3xxx_pcie[i]);
		if (!cns3xxx_pcie[i].linked)
			continue;
		cns3xxx_pcie_hw_init(&cns3xxx_pcie[i]);
		private_data = &cns3xxx_pcie[i];
		pci_common_init(&hw_pci);
	}

	pci_assign_unassigned_resources();
}
