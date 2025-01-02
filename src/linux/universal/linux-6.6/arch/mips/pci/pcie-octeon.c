/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2007, 2008, 2009, 2010, 2011 Cavium Networks
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/time.h>
#include <linux/delay.h>
#include <linux/moduleparam.h>
#include <linux/syscore_ops.h>
#include <linux/irqdomain.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-npei-defs.h>
#include <asm/octeon/cvmx-pciercx-defs.h>
#include <asm/octeon/cvmx-pescx-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-pemx-defs.h>
#include <asm/octeon/cvmx-dpi-defs.h>
#include <asm/octeon/cvmx-rst-defs.h>
#include <asm/octeon/cvmx-sli-defs.h>
#include <asm/octeon/cvmx-sriox-defs.h>
#include <asm/octeon/cvmx-helper-errata.h>
#include <asm/octeon/pci-octeon.h>
#include <asm/octeon/cvmx-pcie.h>
/* Module parameter to disable PCI probing */
static int pcie_disable;
module_param(pcie_disable, int, S_IRUGO);

static int enable_pcie_14459_war;

struct octeon_pcie_interface {
	struct pci_controller controller;
	struct resource mem;
	struct resource io;
	char mem_name[24];
	char io_name[24];
	int node;
	int pem; /* port */
};

struct pcie_17400_chip_data {
	int node;
	int pem;
	int pin;
	int parent_irq;
	int irq;
	unsigned int intsn;
};

static struct octeon_pcie_interface *octeon_pcie_bus2interface(struct pci_bus *bus)
{
	struct octeon_pcie_interface *r;

	r = container_of(bus->sysdata, struct octeon_pcie_interface, controller);
	return r;
}

static void pcie_17400_enable(struct irq_data *data)
{
	struct pcie_17400_chip_data *cd = irq_data_get_irq_chip_data(data);
	enable_irq(cd->parent_irq);
}

static void pcie_17400_disable(struct irq_data *data)
{
	struct pcie_17400_chip_data *cd = irq_data_get_irq_chip_data(data);
	disable_irq(cd->parent_irq);
}

static int pcie_17400_set_affinity(struct irq_data *data,
				   const struct cpumask *dest, bool force)
{
	struct pcie_17400_chip_data *cd = irq_data_get_irq_chip_data(data);
	return irq_set_affinity(cd->parent_irq, dest);
}

static struct irq_chip pcie_17400_chip = {
	.name = "PCI-WAR",
	.irq_enable = pcie_17400_enable,
	.irq_disable = pcie_17400_disable,
#ifdef CONFIG_SMP
	.irq_set_affinity = pcie_17400_set_affinity,
#endif
};

static int pcie_17400_irqs[2][4][4];

static irqreturn_t pcie_17400_handler(int irq, void *data)
{
	u64 int_sum;
	struct pcie_17400_chip_data *cd = data;

	generic_handle_irq(cd->irq);

	int_sum = cvmx_read_csr_node(cd->node, CVMX_PEMX_INT_SUM(cd->pem));
	if (int_sum & (1ull << (60 + cd->pin))) {
		/* retrigger the irq */
		u64 w1s = CVMX_CIU3_ISCX_W1S(cd->intsn);
		cvmx_write_csr_node(cd->node, w1s, 1);
		cvmx_read_csr_node(cd->node, w1s);
	}

	return IRQ_HANDLED;
}

static int octeon_pcie78xx_pcibios_map_irq(const struct pci_dev *dev,
					      u8 slot, u8 pin)
{
	struct octeon_pcie_interface *pcie;
	unsigned int intsn;
	struct irq_domain *d;
	struct pcie_17400_chip_data *cd = NULL;
	int irq;
	int rv;
	/*
	 * Iterate all the way up the device chain and find
	 * the root bus.
	 */
	while (dev->bus && dev->bus->parent)
		dev = to_pci_dev(dev->bus->bridge);

	pcie = octeon_pcie_bus2interface(dev->bus);
	pin--; /* Adjust from 1 based to 0 based pinA */

	intsn = 0xc003c + pin + (0x1000 * pcie->pem);

	d = octeon_irq_get_block_domain(pcie->node, intsn >> 12);

	irq = irq_create_mapping(d, intsn);

	if (!OCTEON_IS_MODEL(OCTEON_CN78XX_PASS1_X)) {
		irq_set_irq_type(irq, IRQ_TYPE_LEVEL_HIGH);
		return irq;
	}

	WARN_ON(pcie->node >= ARRAY_SIZE(pcie_17400_irqs));
	WARN_ON(pin >= ARRAY_SIZE(pcie_17400_irqs[0]));
	WARN_ON(pcie->pem >= ARRAY_SIZE(pcie_17400_irqs[0][0]));
	if (pcie_17400_irqs[pcie->node][pin][pcie->pem])
		return pcie_17400_irqs[pcie->node][pin][pcie->pem];

	/* Else use the PCIE-17400 WAR */
	cd = kzalloc_node(sizeof(*cd), GFP_KERNEL, pcie->node);
	if (!cd)
		return -ENOMEM;
	cd->node = pcie->node;
	cd->pem = pcie->pem;
	cd->pin = pin;
	cd->parent_irq = irq;
	cd->intsn = intsn;

	cd->irq = irq_alloc_descs(-1, 1, 1, pcie->node);
	if (WARN(cd->irq < 0, "Unable to find a free irq\n")) {
		rv = -ENOSPC;
		goto err;
	}

	irq_set_irq_type(irq, IRQ_TYPE_EDGE_RISING);

	irq_set_status_flags(irq, IRQ_NOAUTOEN);
	rv = request_irq(irq, pcie_17400_handler, IRQF_NO_THREAD, "inta-war", cd);
	if (WARN(rv, "request_irq failed.\n"))
		goto err;

	irq_set_chip_and_handler(cd->irq, &pcie_17400_chip, handle_simple_irq);
	irq_set_chip_data(cd->irq, cd);
	pcie_17400_irqs[pcie->node][pin][pcie->pem] = cd->irq;

	return cd->irq;

err:
	kfree(cd);
	return rv;
}

int pcibus_to_node(struct pci_bus *bus)
{
#ifdef CONFIG_NUMA
	struct octeon_pcie_interface *pi;

	/* Only chips with PCIE have a possibility of nodes other than 0. */
	if (!octeon_has_feature(OCTEON_FEATURE_PCIE))
		return 0;

	while (bus->parent) {
		struct pci_dev *dev = to_pci_dev(bus->bridge);
		bus = dev->bus;
	}
	pi = octeon_pcie_bus2interface(bus);
	return pi->node;
#else
	return 0;
#endif
}
EXPORT_SYMBOL(pcibus_to_node);

/**
 * Map a PCI device to the appropriate interrupt line
 *
 * @dev:    The Linux PCI device structure for the device to map
 * @slot:   The slot number for this device on __BUS 0__. Linux
 *		 enumerates through all the bridges and figures out the
 *		 slot on Bus 0 where this device eventually hooks to.
 * @pin:    The PCI interrupt pin read from the device, then swizzled
 *		 as it goes through each bridge.
 * Returns Interrupt number for the device
 */
static int octeon_pcie_pcibios_map_irq(const struct pci_dev *dev,
					      u8 slot, u8 pin)
{
	/*
	 * The EBH5600 board with the PCI to PCIe bridge mistakenly
	 * wires the first slot for both device id 2 and interrupt
	 * A. According to the PCI spec, device id 2 should be C. The
	 * following kludge attempts to fix this.
	 */
	if (strstr(octeon_board_type_string(), "EBH5600") &&
	    dev->bus && dev->bus->parent) {
		/*
		 * Iterate all the way up the device chain and find
		 * the root bus.
		 */
		while (dev->bus && dev->bus->parent)
			dev = to_pci_dev(dev->bus->bridge);
		/*
		 * If the root bus is number 0 and the PEX 8114 is the
		 * root, assume we are behind the miswired bus. We
		 * need to correct the swizzle level by two. Yuck.
		 */
		if ((dev->bus->number == 1) &&
		    (dev->vendor == 0x10b5) && (dev->device == 0x8114)) {
			/*
			 * The pin field is one based, not zero. We
			 * need to swizzle it by minus two.
			 */
			pin = ((pin - 3) & 3) + 1;
		}
	}
	/*
	 * The -1 is because pin starts with one, not zero. It might
	 * be that this equation needs to include the slot number, but
	 * I don't have hardware to check that against.
	 */
	return pin - 1 + OCTEON_IRQ_PCI_INT0;
}

static	void set_cfg_read_retry(u32 retry_cnt)
{
	union cvmx_pemx_ctl_status pemx_ctl;
	pemx_ctl.u64 = cvmx_read_csr(CVMX_PEMX_CTL_STATUS(1));
	pemx_ctl.cn63xx.cfg_rtry = retry_cnt;
	cvmx_write_csr(CVMX_PEMX_CTL_STATUS(1), pemx_ctl.u64);
}


static u32 disable_cfg_read_retry(void)
{
	u32 retry_cnt;

	union cvmx_pemx_ctl_status pemx_ctl;
	pemx_ctl.u64 = cvmx_read_csr(CVMX_PEMX_CTL_STATUS(1));
	retry_cnt =  pemx_ctl.cn63xx.cfg_rtry;
	pemx_ctl.cn63xx.cfg_rtry = 0;
	cvmx_write_csr(CVMX_PEMX_CTL_STATUS(1), pemx_ctl.u64);
	return retry_cnt;
}

static int is_cfg_retry(void)
{
	union cvmx_pemx_int_sum pemx_int_sum;
	pemx_int_sum.u64 = cvmx_read_csr(CVMX_PEMX_INT_SUM(1));
	if (pemx_int_sum.s.crs_dr)
		return 1;
	return 0;
}

static u32 octeon_pcie_pem_read_cfg(int node, int pem, int where_aligned)
{
	u64 addr, v;

	addr = where_aligned;
	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_write_csr(CVMX_PESCX_CFG_RD(pem), addr);
		v = cvmx_read_csr(CVMX_PESCX_CFG_RD(pem));
	} else {
		cvmx_write_csr_node(node, CVMX_PEMX_CFG_RD(pem), addr);
		v = cvmx_read_csr_node(node, CVMX_PEMX_CFG_RD(pem));
	}
	return (u32)(v >> 32);
}

static void octeon_pcie_pem_write_cfg(int node, int pem, int where_aligned, u32 val)
{
	u64 v;

	v = (u32)where_aligned | ((u64)val << 32);

	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		cvmx_write_csr(CVMX_PESCX_CFG_WR(pem), v);
	} else {
		cvmx_write_csr_node(node, CVMX_PEMX_CFG_WR(pem), v);
	}
}

static int octeon_pcie_pem_read(struct pci_bus *bus, unsigned int devfn,
				int where, int size, u32 *val)
{
	struct octeon_pcie_interface *pi = octeon_pcie_bus2interface(bus);
	u64 read_val;

	if (devfn != 0 || where >= 2048) {
		*val = ~0;
		return PCIBIOS_DEVICE_NOT_FOUND;
	}

	/*
	 * 32-bit accesses only.  Write the address to the low order
	 * bits of PEM_CFG_RD, then trigger the read by reading back.
	 * The config data lands in the upper 32-bits of PEM_CFG_RD.
	 */

	read_val = octeon_pcie_pem_read_cfg(pi->node, pi->pem, where & ~3ull);

	/*
	 * The config space contains some garbage, fix it up.  Also
	 * synthesize an EA capability for the BAR used by MSI-X.
	 */
	switch (where & ~3) {
	case 0x08:
		/* Override class code to be PCI-PCI bridge. */
		read_val &= 0x000000ff;
		read_val |= 0x06040000;
	case 0x40:
		read_val &= 0xffff00ff;
		read_val |= 0x00007000; /* Skip MSI CAP */
		break;
	case 0x70: /* Express Cap */
		read_val &= 0xffff00ff; /* Last CAP */
		break;
	case 0x100:
		read_val = 0; /* No PCIe Extended Capabilities. */
	default:
		break;
	}
	read_val >>= (8 * (where & 3));
	switch (size) {
	case 1:
		read_val &= 0xff;
		break;
	case 2:
		read_val &= 0xffff;
		break;
	default:
		break;
	}
	*val = read_val;
	return PCIBIOS_SUCCESSFUL;
}

/*
 * Read a value from configuration space
 *
 */
static int octeon_pcie_read_config(struct pci_bus *bus, unsigned int devfn,
				   int reg, int size, u32 *val)
{
	union octeon_cvmemctl cvmmemctl;
	union octeon_cvmemctl cvmmemctl_save;
	int bus_number = bus->number;
	int cfg_retry = 0;
	int retry_cnt = 0;
	int max_retry_cnt = 10;
	u32 cfg_retry_cnt = 0;
	struct octeon_pcie_interface *pi = octeon_pcie_bus2interface(bus);
	int gport = pi->node << 4 | pi->pem;

	cvmmemctl_save.u64 = 0;

	if (bus_number == 0)
		return octeon_pcie_pem_read(bus, devfn, reg, size, val);

	/*
	 * The following is a workaround for the CN57XX, CN56XX,
	 * CN55XX, and CN54XX errata with PCIe config reads from non
	 * existent devices.  These chips will hang the PCIe link if a
	 * config read is performed that causes a UR response.
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1) ||
	    OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_1)) {
		/*
		 * For our EBH5600 board, port 0 has a bridge with two
		 * PCI-X slots. We need a new special checks to make
		 * sure we only probe valid stuff.  The PCIe->PCI-X
		 * bridge only respondes to device ID 0, function
		 * 0-1
		 */
		if ((bus->parent == NULL) && (devfn >= 2))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
		/*
		 * The PCI-X slots are device ID 2,3. Choose one of
		 * the below "if" blocks based on what is plugged into
		 * the board.
		 */
#if 1
		/* Use this option if you aren't using either slot */
		if (bus_number == 2)
			return PCIBIOS_FUNC_NOT_SUPPORTED;
#elif 0
		/*
		 * Use this option if you are using the first slot but
		 * not the second.
		 */
		if ((bus_number == 2) && (devfn >> 3 != 2))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
#elif 0
		/*
		 * Use this option if you are using the second slot
		 * but not the first.
		 */
		if ((bus_number == 2) && (devfn >> 3 != 3))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
#elif 0
		/* Use this opion if you are using both slots */
		if ((bus_number == 2) &&
		    !((devfn == (2 << 3)) || (devfn == (3 << 3))))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
#endif

		/* The following #if gives a more complicated example. This is
		   the required checks for running a Nitrox CN16XX-NHBX in the
		   slot of the EBH5600. This card has a PLX PCIe bridge with
		   four Nitrox PLX parts behind it */
#if 0
		/* PLX bridge with 4 ports */
		if ((bus_number == 4) &&
		    !((devfn >> 3 >= 1) && (devfn >> 3 <= 4)))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
		/* Nitrox behind PLX 1 */
		if ((bus_number == 5) && (devfn >> 3 != 0))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
		/* Nitrox behind PLX 2 */
		if ((bus_number == 6) && (devfn >> 3 != 0))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
		/* Nitrox behind PLX 3 */
		if ((bus_number == 7) && (devfn >> 3 != 0))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
		/* Nitrox behind PLX 4 */
		if ((bus_number == 8) && (devfn >> 3 != 0))
			return PCIBIOS_FUNC_NOT_SUPPORTED;
#endif

		/*
		 * Shorten the DID timeout so bus errors for PCIe
		 * config reads from non existent devices happen
		 * faster. This allows us to continue booting even if
		 * the above "if" checks are wrong.  Once one of these
		 * errors happens, the PCIe port is dead.
		 */
		cvmmemctl_save.u64 = __read_64bit_c0_register($11, 7);
		cvmmemctl.u64 = cvmmemctl_save.u64;
		cvmmemctl.s.didtto = 2;
		__write_64bit_c0_register($11, 7, cvmmemctl.u64);
	}

	if ((OCTEON_IS_MODEL(OCTEON_CN63XX)) && (enable_pcie_14459_war))
		cfg_retry_cnt = disable_cfg_read_retry();

	pr_debug("pcie_cfg_rd port=%d:%d b=%d devfn=0x%03x reg=0x%03x size=%d ...\n",
		 pi->node, pi->pem, bus_number, devfn, reg, size);
	do {
		switch (size) {
		case 4:
			*val = cvmx_pcie_config_read32(gport, bus_number,
				devfn >> 3, devfn & 0x7, reg);
		break;
		case 2:
			*val = cvmx_pcie_config_read16(gport, bus_number,
				devfn >> 3, devfn & 0x7, reg);
		break;
		case 1:
			*val = cvmx_pcie_config_read8(gport, bus_number,
				devfn >> 3, devfn & 0x7, reg);
		break;
		default:
			if (OCTEON_IS_MODEL(OCTEON_CN63XX))
				set_cfg_read_retry(cfg_retry_cnt);
			return PCIBIOS_FUNC_NOT_SUPPORTED;
		}
		if ((OCTEON_IS_MODEL(OCTEON_CN63XX)) &&
			(enable_pcie_14459_war)) {
			cfg_retry = is_cfg_retry();
			retry_cnt++;
			if (retry_cnt > max_retry_cnt) {
				pr_err(" pcie cfg_read retries failed. retry_cnt=%d\n",
				       retry_cnt);
				cfg_retry = 0;
			}
		}
	} while (cfg_retry);

	if ((OCTEON_IS_MODEL(OCTEON_CN63XX)) && (enable_pcie_14459_war))
		set_cfg_read_retry(cfg_retry_cnt);
	pr_debug("  pcie_cfg_rd -> val=%08x  : tries=%02d\n", *val, retry_cnt);
	if (OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1) ||
	    OCTEON_IS_MODEL(OCTEON_CN56XX_PASS1_1))
		write_c0_cvmmemctl(cvmmemctl_save.u64);
	return PCIBIOS_SUCCESSFUL;
}
/*
 * Some of the w1c_bits below also include read-only or non-writable
 * reserved bits, this makes the code simpler and is OK as the bits
 * are not affected by writing zeros to them.
 */
static u32 octeon_pem_bridge_w1c_bits(u64 where_aligned)
{
	u32 w1c_bits = 0;

	switch (where_aligned) {
	case 0x04: /* Command/Status */
	case 0x1c: /* Base and I/O Limit/Secondary Status */
		w1c_bits = 0xff000000;
		break;
	case 0x44: /* Power Management Control and Status */
		w1c_bits = 0xfffffe00;
		break;
	case 0x78: /* Device Control/Device Status */
	case 0x80: /* Link Control/Link Status */
	case 0x88: /* Slot Control/Slot Status */
	case 0x90: /* Root Status */
	case 0xa0: /* Link Control 2 Registers/Link Status 2 */
		w1c_bits = 0xffff0000;
		break;
	case 0x104: /* Uncorrectable Error Status */
	case 0x110: /* Correctable Error Status */
	case 0x130: /* Error Status */
	case 0x160: /* Link Control 4 */
		w1c_bits = 0xffffffff;
		break;
	default:
		break;
	}
	return w1c_bits;
}

/* Some bits must be written to one so they appear to be read-only. */
static u32 octeon_pem_bridge_w1_bits(u64 where_aligned)
{
	u32 w1_bits;

	switch (where_aligned) {
	case 0x1c: /* I/O Base / I/O Limit, Secondary Status */
		/* Force 32-bit I/O addressing. */
		w1_bits = 0x0101;
		break;
	case 0x24: /* Prefetchable Memory Base / Prefetchable Memory Limit */
		/* Force 64-bit addressing */
		w1_bits = 0x00010001;
		break;
	default:
		w1_bits = 0;
		break;
	}
	return w1_bits;
}

static int octeon_pcie_pem_write(struct pci_bus *bus, unsigned int devfn,
				 int where, int size, u32 val)
{
	struct octeon_pcie_interface *pi = octeon_pcie_bus2interface(bus);
	u64 read_val;
	u64 where_aligned = where & ~3ull;
	u32 mask = 0;

	if (devfn != 0 || where >= 2048)
		return PCIBIOS_DEVICE_NOT_FOUND;

	/*
	 * 32-bit accesses only.  If the write is for a size smaller
	 * than 32-bits, we must first read the 32-bit value and merge
	 * in the desired bits and then write the whole 32-bits back
	 * out.
	 */
	switch (size) {
	case 1:
		read_val = octeon_pcie_pem_read_cfg(pi->node, pi->pem, where_aligned);
		mask = ~(0xff << (8 * (where & 3)));
		read_val &= mask;
		val = (val & 0xff) << (8 * (where & 3));
		val |= (u32)read_val;
		break;
	case 2:
		read_val = octeon_pcie_pem_read_cfg(pi->node, pi->pem, where_aligned);
		mask = ~(0xffff << (8 * (where & 3)));
		read_val &= mask;
		val = (val & 0xffff) << (8 * (where & 3));
		val |= (u32)read_val;
		break;
	default:
		break;
	}

	/*
	 * By expanding the write width to 32 bits, we may
	 * inadvertently hit some W1C bits that were not intended to
	 * be written.  Calculate the mask that must be applied to the
	 * data to be written to avoid these cases.
	 */
	if (mask) {
		u32 w1c_bits = octeon_pem_bridge_w1c_bits(where);

		if (w1c_bits) {
			mask &= w1c_bits;
			val &= ~mask;
		}
	}

	/*
	 * Some bits must be read-only with value of one.  Since the
	 * access method allows these to be cleared if a zero is
	 * written, force them to one before writing.
	 */
	val |= octeon_pem_bridge_w1_bits(where_aligned);

	octeon_pcie_pem_write_cfg(pi->node, pi->pem, where_aligned, val);

	return PCIBIOS_SUCCESSFUL;
}

/*
 * Write a value to PCI configuration space
 */
static int octeon_pcie_write_config(struct pci_bus *bus, unsigned int devfn,
				    int reg, int size, u32 val)
{
	int bus_number = bus->number;
	struct octeon_pcie_interface *pi = octeon_pcie_bus2interface(bus);
	int gport = pi->node << 4 | pi->pem;


	if (bus_number == 0)
		return octeon_pcie_pem_write(bus, devfn, reg, size, val);

	pr_debug("pcie_cfg_wr port=%d:%d b=%d devfn=0x%03x reg=0x%03x size=%d val=%08x\n",
		 pi->node, pi->pem, bus_number, devfn,
		 reg, size, val);


	switch (size) {
	case 4:
		cvmx_pcie_config_write32(gport, bus_number, devfn >> 3,
					 devfn & 0x7, reg, val);
		break;
	case 2:
		cvmx_pcie_config_write16(gport, bus_number, devfn >> 3,
					 devfn & 0x7, reg, val);
		break;
	case 1:
		cvmx_pcie_config_write8(gport, bus_number, devfn >> 3,
					devfn & 0x7, reg, val);
		break;
	default:
		return PCIBIOS_FUNC_NOT_SUPPORTED;
	}
	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops octeon_pcie_ops = {
	.read	= octeon_pcie_read_config,
	.write	= octeon_pcie_write_config,
};

static struct octeon_pcie_interface octeon_pcie[2][4]; /* node, port */

static void octeon_pcie_interface_init(struct octeon_pcie_interface *iface, unsigned node, unsigned pem)
{
	snprintf(iface->mem_name, sizeof(iface->mem_name), "OCTEON PCIe-%u:%u MEM", node, pem);
	iface->mem.name = iface->mem_name;
	iface->mem.flags = IORESOURCE_MEM;

	snprintf(iface->io_name, sizeof(iface->io_name), "OCTEON PCIe-%u:%u IO", node, pem);
	iface->io.name = iface->io_name;
	iface->io.flags = IORESOURCE_IO;

	iface->controller.pci_ops = &octeon_pcie_ops;
	iface->controller.mem_resource = &iface->mem;
	iface->controller.io_resource = &iface->io;

	iface->node = node;
	iface->pem = pem;
}

static void octeon_pcie_setup_port(unsigned int node, unsigned int port)
{
	int result;
	int host_mode = 0;
	int srio_war15205 = 0;
	union cvmx_sli_ctl_portx sli_ctl_portx;
	union cvmx_sriox_status_reg sriox_status_reg;
	int gport = (node << 4) | port;


	WARN_ON(node >= ARRAY_SIZE(octeon_pcie) ||
		port >= ARRAY_SIZE(octeon_pcie[0]));

	pr_notice("PCIe: Initializing port %u:%u\n", node, port);

	if (octeon_has_feature(OCTEON_FEATURE_NPEI)) {
		if (port == 1) {
			host_mode = 1;
			/*
			 * Skip the 2nd port on CN52XX if port is in
			 * 4 lane mode
			 */
			if (OCTEON_IS_MODEL(OCTEON_CN52XX)) {
				union cvmx_npei_dbg_data dbg_data;
				dbg_data.u64 = cvmx_read_csr(CVMX_PEXP_NPEI_DBG_DATA);
				if (dbg_data.cn52xx.qlm0_link_width)
					host_mode = 0;
			}
		} else {
			union cvmx_npei_ctl_status npei_ctl_status;
			npei_ctl_status.u64 =
				cvmx_read_csr(CVMX_PEXP_NPEI_CTL_STATUS);
			host_mode = npei_ctl_status.s.host_mode;
		}
	} else {
		union cvmx_mio_rst_ctlx mio_rst_ctl;
		if (OCTEON_IS_OCTEON3())
			mio_rst_ctl.u64 = cvmx_read_csr_node(node, CVMX_RST_CTLX(port));
		else
			mio_rst_ctl.u64 = cvmx_read_csr(CVMX_MIO_RST_CTLX(port));
		host_mode = mio_rst_ctl.s.host_mode;
	}

	if (host_mode) {
		uint32_t device;

		/* CN63XX pass 1_x/2.0 errata PCIe-15205 */
		if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X) ||
		    OCTEON_IS_MODEL(OCTEON_CN63XX_PASS2_0)) {
			sriox_status_reg.u64 = cvmx_read_csr(CVMX_SRIOX_STATUS_REG(port));
			if (sriox_status_reg.s.srio)
				/* Port is SRIO */
				srio_war15205 += 1;
		}
		result = cvmx_pcie_rc_initialize(gport);
		if (result < 0)
			return;

		/*
		 * Set bus numbers back to zero to undo any breakage
		 * caused by cvmx initialization code.
		 */
		cvmx_pcie_cfgx_write_node(node, port, 0x18, 0);

		/* Set IO offsets, Memory/IO resource start and end limits */
		octeon_pcie_interface_init(&octeon_pcie[node][port], node, port);
		/* Memory offsets are physical addresses */
		octeon_pcie[node][port].controller.mem_offset = cvmx_pcie_get_mem_base_address(gport);
		/*
		 * To calculate the address for accessing the 2nd PCIe device,
		 * either 'io_map_base' (pci_iomap()), or 'mips_io_port_base'
		 * (ioport_map()) value is added to
		 * pci_resource_start(dev,bar)). The 'mips_io_port_base' is set
		 * only once based on first PCIe. Also changing 'io_map_base'
		 * based on first slot's value so that both the routines will
		 * work properly.
		 */
		octeon_pcie[node][port].controller.io_map_base =
			CVMX_ADD_IO_SEG(cvmx_pcie_get_io_base_address(0));
		/*
		 * To keep things similar to PCI, we start
		 * device addresses at the same place as PCI
		 * uisng big bar support. This normally
		 * translates to 4GB-256MB, which is the same
		 * as most x86 PCs.
		 */
		octeon_pcie[node][port].mem.start =
			cvmx_pcie_get_mem_base_address(gport) + (4ul << 30) - (OCTEON_PCI_BAR1_HOLE_SIZE << 20);
		octeon_pcie[node][port].mem.end =
			cvmx_pcie_get_mem_base_address(gport) + cvmx_pcie_get_mem_size(gport) - 1;
		if (gport == 0) {
			/* IO offsets are Mips virtual addresses */
			octeon_pcie[node][port].controller.io_offset = 0;
			/*
			 * Ports must be above 16KB for the ISA bus
			 * filtering in the PCI-X to PCI bridge.
			 */
			octeon_pcie[node][port].io.start = 4 << 10;
			octeon_pcie[node][port].io.end = cvmx_pcie_get_io_size(gport) - 1;
		} else {
			u64 io_offset = ((u64)port) << 32 | ((u64)node) << 36;
			octeon_pcie[node][port].controller.io_offset = io_offset;
			octeon_pcie[node][port].io.start = io_offset;
			octeon_pcie[node][port].io.end =
				octeon_pcie[node][port].io.start + cvmx_pcie_get_io_size(gport) - 1;
		}
		msleep(100); /* Some devices need extra time */
		octeon_pcie[node][port].controller.index = gport;
		register_pci_controller(&octeon_pcie[node][port].controller);

		device = cvmx_pcie_config_read32(gport, 0, 0, 0, 0);
	} else {
		pr_notice("PCIe: Port %d:%d in endpoint mode, skipping.\n", node, port);
		/* CN63XX pass 1_x/2.0 errata PCIe-15205 */
		if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X) ||
		    OCTEON_IS_MODEL(OCTEON_CN63XX_PASS2_0)) {
			srio_war15205 += 1;
		}
	}

	/*
	 * CN63XX pass 1_x/2.0 errata PCIe-15205 requires setting all
	 * of SRIO MACs SLI_CTL_PORT*[INT*_MAP] to similar value and
	 * all of PCIe Macs SLI_CTL_PORT*[INT*_MAP] to different value
	 * from the previous set values
	 */
	if (OCTEON_IS_MODEL(OCTEON_CN63XX_PASS1_X) ||
	    OCTEON_IS_MODEL(OCTEON_CN63XX_PASS2_0)) {
		if (srio_war15205 == 1) {
			sli_ctl_portx.u64 = cvmx_read_csr(CVMX_PEXP_SLI_CTL_PORTX(port));
			sli_ctl_portx.s.inta_map = 1;
			sli_ctl_portx.s.intb_map = 1;
			sli_ctl_portx.s.intc_map = 1;
			sli_ctl_portx.s.intd_map = 1;
			cvmx_write_csr(CVMX_PEXP_SLI_CTL_PORTX(port), sli_ctl_portx.u64);

			sli_ctl_portx.u64 = cvmx_read_csr(CVMX_PEXP_SLI_CTL_PORTX(!port));
			sli_ctl_portx.s.inta_map = 0;
			sli_ctl_portx.s.intb_map = 0;
			sli_ctl_portx.s.intc_map = 0;
			sli_ctl_portx.s.intd_map = 0;
			cvmx_write_csr(CVMX_PEXP_SLI_CTL_PORTX(!port), sli_ctl_portx.u64);
		}
	}

}

static void octeon_pcie_setup_ports(void)
{
	int node, port;

	for_each_online_node (node)
		for (port = 0; port < CVMX_PCIE_PORTS; port++)
			octeon_pcie_setup_port(node, port);
}

static int octeon_pcie_suspend(void)
{
	int node, port;

	for_each_online_node (node)
		for (port = 0; port < CVMX_PCIE_PORTS; port++)
			cvmx_pcie_rc_shutdown((node << 2) | (port & 3));
	return 0;
}

static void octeon_pcie_teardown(void)
{
	octeon_pcie_suspend();
}

static struct syscore_ops updown = {
	.suspend = octeon_pcie_suspend,
	.resume = octeon_pcie_setup_ports,
	.shutdown = octeon_pcie_teardown,
};

extern void octeon_pci_dma_init(void);

/**
 * Initialize the Octeon PCIe controllers
 *
 * Returns
 */
static int __init octeon_pcie_setup(void)
{
	/* These chips don't have PCIe */
	if (!octeon_has_feature(OCTEON_FEATURE_PCIE))
		return 0;

	/* No PCIe simulation */
	if (octeon_is_simulation())
		return 0;

	/* Disable PCI if instructed on the command line */
	if (pcie_disable)
		return 0;

	/* Point pcibios_map_irq() to the PCIe version of it */
	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		octeon_pcibios_map_irq = octeon_pcie78xx_pcibios_map_irq;
	else
		octeon_pcibios_map_irq = octeon_pcie_pcibios_map_irq;

	/*
	 * PCIe I/O range. It is based on port 0 but includes up until
	 * port 1's end.
	 */
	set_io_port_base(CVMX_ADD_IO_SEG(cvmx_pcie_get_io_base_address(0)));
	ioport_resource.start = 0;
	ioport_resource.end = (1ull << 37) - 1;

	octeon_pcie_setup_ports();
	octeon_pci_dma_init();
	register_syscore_ops(&updown);

	return 0;
}
arch_initcall(octeon_pcie_setup);
