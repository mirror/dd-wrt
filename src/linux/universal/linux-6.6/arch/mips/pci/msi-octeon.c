/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2005-2012 Cavium Inc.
 */
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpu.h>
#include <linux/msi.h>
#include <linux/irqdomain.h>

#include <asm/octeon/octeon.h>
#include <asm/octeon/cvmx-npi-defs.h>
#include <asm/octeon/cvmx-pci-defs.h>
#include <asm/octeon/cvmx-npei-defs.h>
#include <asm/octeon/cvmx-pexp-defs.h>
#include <asm/octeon/cvmx-sli-defs.h>
#include <asm/octeon/cvmx-ciu2-defs.h>
#include <asm/octeon/pci-octeon.h>

/* MSI major block number (8 MSBs of intsn) */
#define MSI_BLOCK_NUMBER	0x1e

#define MSI_IRQ_SIZE		256

/*
 * Data to save in the chip_data field of the irq description.
 */
struct msi_chip_data {
	int msi;
	int hwmsi;
};

/*
 * Each bit in msi_free_irq_bitmap represents a MSI interrupt that is
 * in use. Each node requires its own set of bits.
 */
static DECLARE_BITMAP(msi_free_irq_bitmap[CVMX_MAX_NODES], MSI_IRQ_SIZE);

/*
 * This lock controls updates to msi_free_irq_bitmap.
 */
static DEFINE_SPINLOCK(msi_free_irq_bitmap_lock);

/* MSI to IRQ lookup */
static int msi_to_irq[MSI_IRQ_SIZE];

/*
 * Find a contiguous aligned block of free msi interrupts and allocate
 * them (set them to one).
 *
 * @node:      Node to allocate msi interrupts for.
 * @nvec:      Number of msi interrupts to allocate.
 *
 * Returns:    Zero on success, error otherwise.
 */
static int msi_bitmap_alloc_hwirqs(int node, int nvec)
{
	unsigned long	flags;
	int		offset;
	int		order = get_count_order(nvec);

	spin_lock_irqsave(&msi_free_irq_bitmap_lock, flags);
	offset = bitmap_find_free_region(msi_free_irq_bitmap[node],
					 MSI_IRQ_SIZE, order);

	spin_unlock_irqrestore(&msi_free_irq_bitmap_lock, flags);

	if (unlikely(offset < 0)) {
		WARN(1, "Unable to find a free MSI interrupt");
		return offset;
	}

	return offset;
}

/*
 * Free a contiguous block of msi interrupts (set them to zero).
 *
 * @node:      Node to allocate msi interrupts for.
 * @offset:    Beginnning of msi interrupts to release.
 * @nvec:      Number of msi interrupts to release.
 */
static void msi_bitmap_free_hwirqs(int node, int offset, int nvec)
{
	unsigned long	flags;
	int		order = get_count_order(nvec);

	spin_lock_irqsave(&msi_free_irq_bitmap_lock, flags);
	bitmap_release_region(msi_free_irq_bitmap[node], offset, order);
	spin_unlock_irqrestore(&msi_free_irq_bitmap_lock, flags);
}

/**
 * Called when a device no longer needs its MSI interrupts. All
 * MSI interrupts for the device are freed.
 *
 * @irq:    The devices first irq number. There may be multple in sequence.
 */
void arch_teardown_msi_irq(unsigned int irq)
{
	int msi;
	int node = 0; /* Must use node device is in. TODO */

	if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
		struct octeon_ciu_chip_data *cd3 = irq_get_chip_data(irq);
		node = cd3->ciu_node;
		msi = cd3->intsn & 0xff;
	} else {
		struct msi_chip_data *cd = irq_get_chip_data(irq);
		msi = cd->msi;
		irq_free_descs(irq, 1);
	}

	msi_bitmap_free_hwirqs(node, msi, 1);
}

static DEFINE_RAW_SPINLOCK(octeon_irq_msi_lock);

static u64 msi_rcv_reg[4];
static u64 msi_ena_reg[4];

/*
 * Up to 256 MSIs are supported. MSIs are allocated sequencially from 0 to 255.
 * The CIU has 4 interrupt lines each supporting 64 MSIs to handle the 256 MSI
 * interrupts.
 * Software might desire to map MSIs to different CIU interrupt lines to share
 * the load. For example, MSI 0 might be mapped to CIU interrupt line 0, MSI 1
 * to CIU interrupt line 1, and so on.
 * Hardware MSIs indicate the CIU interrupt line and the bit within the line a
 * particular MSI is mapped to.
 * These pointers point to the methods that performs the mapping to use.
 */
static int (*octeon_irq_msi_to_hwmsi)(int);
static int (*octeon_irq_hwmsi_to_msi)(int);

/*
 * MSI to hardware MSI linear mapping. No load sharing. First 64 allocated MSIs
 * go to CIU interrupt line 0, next 64 to the next CIU line and so on.
 */
static int octeon_irq_msi_to_hwmsi_linear(int msi)
{
	return msi;
}

static int octeon_irq_hwmsi_to_msi_linear(int hwmsi)
{
	return hwmsi;
}

/*
 * MSI to hardware MSI scatter mapping. MSI interrupt load is spread among all
 * CIU interrupt lines. MSI 0 goes to CIU line 0, MSI 1 to CIU line 1 and so on.
 */
static int octeon_irq_msi_to_hwmsi_scatter(int msi)
{
	return ((msi << 6) & 0xc0) | ((msi >> 2) & 0x3f);
}

static int octeon_irq_hwmsi_to_msi_scatter(int hwmsi)
{
	return (((hwmsi >> 6) & 0x3) | ((hwmsi << 2) & 0xfc));
}

#ifdef CONFIG_SMP
extern void irq_set_thread_affinity(struct irq_desc *desc);

static int octeon_irq_set_affinity(int irq, const struct cpumask *mask)
{
	struct irq_desc *desc = irq_to_desc(irq);
	struct irq_data *data = irq_desc_get_irq_data(desc);
	struct irq_chip *chip = irq_data_get_irq_chip(data);
	int ret;

	ret = chip->irq_set_affinity(data, mask, false);
	switch (ret) {
	case IRQ_SET_MASK_OK:
	case IRQ_SET_MASK_OK_DONE:
		cpumask_copy(desc->irq_common_data.affinity, mask);
		fallthrough;
	case IRQ_SET_MASK_OK_NOCOPY:
		irq_set_thread_affinity(desc);
		ret = 0;
	}

	return ret;
}




static atomic_t affinity_in_progress[4] = {
	ATOMIC_INIT(1),
	ATOMIC_INIT(1),
	ATOMIC_INIT(1),
	ATOMIC_INIT(1)};

static int octeon_irq_msi_set_affinity_pcie(struct irq_data *data,
					    const struct cpumask *dest,
					    bool force)
{
	struct msi_chip_data *cd = irq_get_chip_data(data->irq);
	int hwmsi = cd->hwmsi;
	int index = (hwmsi >> 6) & 0x3;
	int bit;
	int r;
	/*
	 * If we are in the middle of updating the set, the first call
	 * takes care of everything, do nothing successfully.
	 */
	if (atomic_dec_if_positive(&affinity_in_progress[index]) < 0)
		return 0;
	
	r = octeon_irq_set_affinity(OCTEON_IRQ_PCI_MSI0 + index, dest);
	for (bit = 0; bit < 64; bit++) {
		int msi = octeon_irq_hwmsi_to_msi(64 * index + bit);
		int partner = msi_to_irq[msi];
		if (partner && partner != data->irq) {
			octeon_irq_set_affinity(partner, dest);
		}
	}
	atomic_add(1, &affinity_in_progress[index]);
	return r;
}

static int octeon_irq_msi_set_affinity_pci(struct irq_data *data,
					   const struct cpumask *dest,
					   bool force)
{
	struct msi_chip_data *cd = irq_get_chip_data(data->irq);
	int hwmsi = cd->hwmsi;
	int index = hwmsi >> 4;
	int bit;
	int r;

	/*
	 * If we are in the middle of updating the set, the first call
	 * takes care of everything, do nothing successfully.
	 */
	if (atomic_dec_if_positive(&affinity_in_progress[index]) < 0)
		return 0;

	r = octeon_irq_set_affinity(OCTEON_IRQ_PCI_MSI0 + index, dest);

	for (bit = 0; bit < 16; bit++) {
		int msi = octeon_irq_hwmsi_to_msi(64 * index + bit);
		int partner = msi_to_irq[msi];
		if (partner && partner != data->irq)
			octeon_irq_set_affinity(partner, dest);
	}
	atomic_add(1, &affinity_in_progress[index]);
	return r;
}
#endif /* CONFIG_SMP */

static void octeon_irq_msi_enable_pcie(struct irq_data *data)
{
	u64 en;
	unsigned long flags;
	struct msi_chip_data *cd = irq_get_chip_data(data->irq);
	int hwmsi = cd->hwmsi;
	int irq_index = hwmsi >> 6;
	int irq_bit = hwmsi & 0x3f;

	raw_spin_lock_irqsave(&octeon_irq_msi_lock, flags);
	en = cvmx_read_csr(msi_ena_reg[irq_index]);
	en |= 1ull << irq_bit;
	cvmx_write_csr(msi_ena_reg[irq_index], en);
	cvmx_read_csr(msi_ena_reg[irq_index]);
	raw_spin_unlock_irqrestore(&octeon_irq_msi_lock, flags);
	pci_msi_unmask_irq(data);
}

static void octeon_irq_msi_disable_pcie(struct irq_data *data)
{
	u64 en;
	unsigned long flags;
	struct msi_chip_data *cd = irq_get_chip_data(data->irq);
	int hwmsi = cd->hwmsi;
	int irq_index = hwmsi >> 6;
	int irq_bit = hwmsi & 0x3f;
	raw_spin_lock_irqsave(&octeon_irq_msi_lock, flags);
	en = cvmx_read_csr(msi_ena_reg[irq_index]);
	en &= ~(1ull << irq_bit);
	cvmx_write_csr(msi_ena_reg[irq_index], en);
	cvmx_read_csr(msi_ena_reg[irq_index]);
	raw_spin_unlock_irqrestore(&octeon_irq_msi_lock, flags);
	pci_msi_mask_irq(data);
}

static struct irq_chip octeon_irq_chip_msi_pcie = {
	.name = "MSI",
	.irq_enable = octeon_irq_msi_enable_pcie,
	.irq_disable = octeon_irq_msi_disable_pcie,
#ifdef CONFIG_SMP
	.irq_set_affinity = octeon_irq_msi_set_affinity_pcie,
#endif
};

static void octeon_irq_msi_enable_pci(struct irq_data *data)
{
	/*
	 * Octeon PCI doesn't have the ability to mask/unmask MSI
	 * interrupts individually. Instead of masking/unmasking them
	 * in groups of 16, we simple assume MSI devices are well
	 * behaved. MSI interrupts are always enable and the ACK is
	 * assumed to be enough
	 */
}

static void octeon_irq_msi_disable_pci(struct irq_data *data)
{
	/* See comment in enable */
}

static struct irq_chip octeon_irq_chip_msi_pci = {
	.name = "MSI",
	.irq_enable = octeon_irq_msi_enable_pci,
	.irq_disable = octeon_irq_msi_disable_pci,
#ifdef CONFIG_SMP
	.irq_set_affinity = octeon_irq_msi_set_affinity_pci,
#endif
};

/*
 * Update msg with the system specific address where the msi data is to be
 * written.
 *
 * @msg:    Updated with the mis message address.
 */
static void setup_msi_msg_address(struct msi_msg *msg)
{
	switch (octeon_dma_bar_type) {
	case OCTEON_DMA_BAR_TYPE_SMALL:
		/* When not using big bar, Bar 0 is based at 128MB */
		msg->address_lo =
			((128ul << 20) + CVMX_PCI_MSI_RCV) & 0xffffffff;
		msg->address_hi = ((128ul << 20) + CVMX_PCI_MSI_RCV) >> 32;
		break;
	case OCTEON_DMA_BAR_TYPE_BIG:
		/* When using big bar, Bar 0 is based at 0 */
		msg->address_lo = (0 + CVMX_PCI_MSI_RCV) & 0xffffffff;
		msg->address_hi = (0 + CVMX_PCI_MSI_RCV) >> 32;
		break;
	case OCTEON_DMA_BAR_TYPE_PCIE:
		/* When using PCIe, Bar 0 is based at 0 */
		/* FIXME CVMX_NPEI_MSI_RCV* other than 0? */
		msg->address_lo = (0 + CVMX_NPEI_PCIE_MSI_RCV) & 0xffffffff;
		msg->address_hi = (0 + CVMX_NPEI_PCIE_MSI_RCV) >> 32;
		break;
	case OCTEON_DMA_BAR_TYPE_PCIE2:
		/* When using PCIe2, Bar 0 is based at 0 */
		msg->address_lo = (0 + CVMX_SLI_PCIE_MSI_RCV) & 0xffffffff;
		msg->address_hi = (0 + CVMX_SLI_PCIE_MSI_RCV) >> 32;
		break;
	default:
		panic("setup_msi_msg_address: Invalid octeon_dma_bar_type");
	}
}

/*
 * Allocate and configure multiple irqs for MSI interrupts for OCTEON.
 *
 * @node:      Node to configure interrupts for.
 * @desc:      MSI descriptor.
 * @msi_base:  First msi number.
 * @nvec:      Number of MSI interrupts requested.
 *
 * Returns:    First irq number.
 */
static int arch_setup_msi_irq_ciu(int node, struct msi_desc *desc, int msi_base,
				  int nvec)
{
	int irq_base;
	struct irq_chip *chip;
	struct msi_chip_data *cd;
	int hwmsi;
	int i;
	irq_base = irq_alloc_descs(-1, 1, nvec, node);
	if (irq_base < 0) {
		WARN(1, "Unable to allocate %d irq(s)", nvec);
		return -ENOSPC;
	}

	for (i = 0; i < nvec; i++) {
		cd = kzalloc_node(sizeof(*cd), GFP_KERNEL, node);
		if (!cd) {
			for (i--; i >= 0; i--) {
				cd = irq_get_chip_data(irq_base + i);
				irq_set_chip_and_handler(irq_base + i, NULL,
							 NULL);
				irq_set_chip_data(irq_base + i, NULL);
				kfree(cd);
			}
			irq_free_descs(irq_base, nvec);
			return -ENOMEM;
		}

		cd->msi = msi_base + i;
		hwmsi = octeon_irq_msi_to_hwmsi(msi_base + i);
		cd->hwmsi = hwmsi;
		msi_to_irq[msi_base + i] = irq_base + i;

		/* Initialize the irq description */
		if (octeon_dma_bar_type == OCTEON_DMA_BAR_TYPE_PCIE2) {
			chip = &octeon_irq_chip_msi_pcie;
		}
		else if (octeon_dma_bar_type == OCTEON_DMA_BAR_TYPE_PCIE) {
			chip = &octeon_irq_chip_msi_pcie;
		} else {
			chip = &octeon_irq_chip_msi_pci;
		}
		irq_set_chip_and_handler(irq_base + i, chip, handle_simple_irq);
		irq_set_chip_data(irq_base + i, cd);
		irq_set_msi_desc(irq_base + i, desc);
	}

	return irq_base;
}

/*
 * Allocate and configure multiple irqs for MSI interrupts for OCTEON III.
 *
 * @node:      Node to configure interrupts for.
 * @desc:      MSI descriptor.
 * @msi_base:  First msi number.
 * @nvec:      Number of MSI interrupts requested.
 *
 * Returns:    First irq number.
 */
static int arch_setup_msi_irq_ciu3(int node, struct msi_desc *desc,
				   int msi_base, int nvec)
{
	struct irq_domain *domain;
	int irq_base = -1;
	int irq;
	int hwirq;
	int i;

	/* Get the domain for the msi interrupts */
	domain = octeon_irq_get_block_domain(node, MSI_BLOCK_NUMBER);

	for (i = 0; i < nvec; i++) {
		/* Get a irq for the msi intsn (hardware interrupt) */
		hwirq = MSI_BLOCK_NUMBER << 12 | (msi_base + i);
		irq = irq_create_mapping(domain, hwirq);
		irqd_set_trigger_type(irq_get_irq_data(irq),
				      IRQ_TYPE_EDGE_RISING);
		irq_set_msi_desc(irq, desc);

		if (i == 0)
			irq_base = irq;
	}

	return irq_base;
}

/*
 * Called when a driver request MSI interrupts instead of the
 * legacy INT A-D. This routine will allocate multiple MSI interrupts
 * for MSI devices that support them.
 *
 * @dev:       Device requesting MSI interrupts.
 * @desc:      MSI descriptor.
 * @nvec:      Number of interrupts requested.
 *
 * Returns 0 on success, error otherwise.
 */
static int arch_setup_multi_msi_irq(struct pci_dev *dev, struct msi_desc *desc,
				    int nvec)
{
	struct msi_msg msg;
	int irq_base;
	int msi_base;
	int node = pcibus_to_node(dev->bus);

	/* Get a free msi interrupt block */
	msi_base = msi_bitmap_alloc_hwirqs(node, nvec);
	if (msi_base < 0)
		return msi_base;
	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		irq_base = arch_setup_msi_irq_ciu3(node, desc, msi_base, nvec);
	else
		irq_base = arch_setup_msi_irq_ciu(node, desc, msi_base, nvec);

	if (irq_base < 0) {
		msi_bitmap_free_hwirqs(node, msi_base, 1);
		return irq_base;
	}

	/* Set the base of the irqs used by this device */
	irq_set_msi_desc(irq_base, desc);

	/* Update the config space msi(x) capability structure */
	desc->pci.msi_attrib.multiple = ilog2(nvec);
	msg.data = msi_base;
	setup_msi_msg_address(&msg);
	pci_write_msi_msg(irq_base, &msg);

	return 0;
}

/**
 * Called when a driver request MSI/MSIX interrupts instead of the
 * legacy INT A-D. This routine will allocate a single MSI/MSIX interrupt
 * for MSI devices that support them.
 *
 * @dev:    Device requesting MSI interrupts
 * @desc:   MSI descriptor
 *
 * Returns 0 on success, error otherwise.
 */
int arch_setup_msi_irq(struct pci_dev *dev, struct msi_desc *desc)
{
	struct msi_msg msg;
	int irq;
	int msi;
	int node = pcibus_to_node(dev->bus);

	/* Get a free msi interrupt */
	msi = msi_bitmap_alloc_hwirqs(node, 1);
	if (msi < 0)
		return msi;

	if (octeon_has_feature(OCTEON_FEATURE_CIU3))
		irq = arch_setup_msi_irq_ciu3(node, desc, msi, 1);
	else
		irq = arch_setup_msi_irq_ciu(node, desc, msi, 1);

	if (irq < 0) {
		msi_bitmap_free_hwirqs(node, msi, 1);
		return irq;
	}

	/* Update the config space msi(x) capability structure */
	desc->pci.msi_attrib.multiple = 0;
	msg.data = msi;
	setup_msi_msg_address(&msg);
	pci_write_msi_msg(irq, &msg);

	return 0;
}

/**
 * Called when a driver request MSI/MSIX interrupts instead of the
 * legacy INT A-D. This routine will allocate multiple MSI/MSIX interrupts
 * for MSI devices that support them.
 *
 * @dev:    Device requesting MSI interrupts
 * @nvec:   Number of MSI interrupts requested.
 * @type:   Interrupt type, MSI or MSIX.
 *
 * Returns 0 on success.
 */
int arch_setup_msi_irqs(struct pci_dev *dev, int nvec, int type)
{
	struct msi_desc *entry;
	int rc = -1;

	if (type == PCI_CAP_ID_MSI && nvec > 1) {
		entry = msi_first_desc(&dev->dev, MSI_DESC_ALL);
		rc = arch_setup_multi_msi_irq(dev, entry, nvec);
	} else {
		msi_for_each_desc(entry, &dev->dev, MSI_DESC_ALL) { 
			rc = arch_setup_msi_irq(dev, entry);
			if (rc)
				return rc;
		}
	}

	return rc;
}

/*
 * Called by the interrupt handling code when an MSI interrupt
 * occurs.
 */
static irqreturn_t __octeon_msi_do_interrupt(int index, u64 msi_bits)
{
	int bit;
	int msi;
	int irq;

	bit = fls64(msi_bits);
	if (bit) {
		bit--;
		/* Acknowledge it first. */
		cvmx_write_csr(msi_rcv_reg[index], 1ull << bit);

		msi = octeon_irq_hwmsi_to_msi(bit + 64 * index);
		irq = msi_to_irq[msi];

		generic_handle_irq(irq);
		return IRQ_HANDLED;
	}
	return IRQ_NONE;
}

#define OCTEON_MSI_INT_HANDLER_X(x)					\
static irqreturn_t octeon_msi_interrupt##x(int cpl, void *dev_id)	\
{									\
	u64 msi_bits = cvmx_read_csr(msi_rcv_reg[(x)]);			\
	return __octeon_msi_do_interrupt((x), msi_bits);		\
}

/*
 * Create octeon_msi_interrupt{0-3} function body
 */
OCTEON_MSI_INT_HANDLER_X(0);
OCTEON_MSI_INT_HANDLER_X(1);
OCTEON_MSI_INT_HANDLER_X(2);
OCTEON_MSI_INT_HANDLER_X(3);


static void octeon_msi_ciu3_ack_msi(struct irq_data *data)
{
	u64 csr_addr;
	struct octeon_ciu_chip_data *cd;
	int msi;

	cd = irq_data_get_irq_chip_data(data);

	/* Acknowledge MSI interrupt (get the node from cd) */
	msi = cd->intsn & 0xff;
	csr_addr = msi_rcv_reg[msi >> 6];
	cvmx_write_csr_node(cd->ciu_node, csr_addr, 1 << (msi & 0x3f));
}

static void octeon_msi_ciu3_ack(struct irq_data *data)
{
	octeon_irq_ciu3_ack(data);
	octeon_msi_ciu3_ack_msi(data);

}

static void octeon_msi_ciu3_mask_ack(struct irq_data *data)
{
	octeon_irq_ciu3_mask_ack(data);
	octeon_msi_ciu3_ack_msi(data);
}

static void octeon_msi_ciu3_enable(struct irq_data *data)
{
	octeon_irq_ciu3_enable(data);
	pci_msi_unmask_irq(data);
}

static void octeon_msi_ciu3_disable(struct irq_data *data)
{
	octeon_irq_ciu3_disable(data);
	pci_msi_mask_irq(data);
}

static struct irq_chip octeon_irq_msi_chip_ciu3 = {
	.name = "MSI-X",
	.irq_enable = octeon_msi_ciu3_enable,
	.irq_disable = octeon_msi_ciu3_disable,
	.irq_ack = octeon_msi_ciu3_ack,
	.irq_mask = octeon_irq_ciu3_mask,
	.irq_mask_ack = octeon_msi_ciu3_mask_ack,
	.irq_unmask = octeon_irq_ciu3_enable,
#ifdef CONFIG_SMP
	.irq_set_affinity = octeon_irq_ciu3_set_affinity,
#endif
};

static int octeon_msi_ciu3_map(struct irq_domain *d,
			       unsigned int virq, irq_hw_number_t hw)
{
	return octeon_irq_ciu3_mapx(d, virq, hw, &octeon_irq_msi_chip_ciu3);
}

struct irq_domain_ops octeon_msi_domain_ciu3_ops = {
	.map = octeon_msi_ciu3_map,
	.unmap = octeon_irq_free_cd,
	.xlate = octeon_irq_ciu3_xlat,
};

/*
 * Initializes the MSI interrupt handling code
 */
int __init octeon_msi_initialize(void)
{
	struct irq_domain *domain;
	u64 msi_map_reg;
	int i;

	/* Clear msi irq bitmap */
	for (i = 0; i < CVMX_MAX_NODES; i++)
		bitmap_zero(msi_free_irq_bitmap[i], MSI_IRQ_SIZE);

	if (octeon_has_feature(OCTEON_FEATURE_CIU3)) {
		int	node;
		int	irq_base;

		/* Registers to acknowledge msi interrupts */
		msi_rcv_reg[0] = CVMX_PEXP_SLI_MSI_RCV0;
		msi_rcv_reg[1] = CVMX_PEXP_SLI_MSI_RCV1;
		msi_rcv_reg[2] = CVMX_PEXP_SLI_MSI_RCV2;
		msi_rcv_reg[3] = CVMX_PEXP_SLI_MSI_RCV3;

		for_each_online_node(node) {
			/* MSI interrupts use their own domain */
			irq_base = irq_alloc_descs(-1, 0, MSI_IRQ_SIZE, 0);
			WARN_ON(irq_base < 0);
			domain = irq_domain_add_legacy(NULL, MSI_IRQ_SIZE, irq_base,
						       MSI_BLOCK_NUMBER << 12,
						       &octeon_msi_domain_ciu3_ops,
						       octeon_irq_get_ciu3_info(node));
			WARN_ON(!domain);
			octeon_irq_add_block_domain(node, MSI_BLOCK_NUMBER, domain);

		}
		return 0;
	}

	if (octeon_dma_bar_type == OCTEON_DMA_BAR_TYPE_PCIE2) {
		msi_rcv_reg[0] = CVMX_PEXP_SLI_MSI_RCV0;
		msi_rcv_reg[1] = CVMX_PEXP_SLI_MSI_RCV1;
		msi_rcv_reg[2] = CVMX_PEXP_SLI_MSI_RCV2;
		msi_rcv_reg[3] = CVMX_PEXP_SLI_MSI_RCV3;
		msi_ena_reg[0] = CVMX_PEXP_SLI_MSI_ENB0;
		msi_ena_reg[1] = CVMX_PEXP_SLI_MSI_ENB1;
		msi_ena_reg[2] = CVMX_PEXP_SLI_MSI_ENB2;
		msi_ena_reg[3] = CVMX_PEXP_SLI_MSI_ENB3;
		octeon_irq_msi_to_hwmsi = octeon_irq_msi_to_hwmsi_scatter;
		octeon_irq_hwmsi_to_msi = octeon_irq_hwmsi_to_msi_scatter;
		msi_map_reg = CVMX_PEXP_SLI_MSI_WR_MAP;
	} else if (octeon_dma_bar_type == OCTEON_DMA_BAR_TYPE_PCIE) {
		msi_rcv_reg[0] = CVMX_PEXP_NPEI_MSI_RCV0;
		msi_rcv_reg[1] = CVMX_PEXP_NPEI_MSI_RCV1;
		msi_rcv_reg[2] = CVMX_PEXP_NPEI_MSI_RCV2;
		msi_rcv_reg[3] = CVMX_PEXP_NPEI_MSI_RCV3;
		msi_ena_reg[0] = CVMX_PEXP_NPEI_MSI_ENB0;
		msi_ena_reg[1] = CVMX_PEXP_NPEI_MSI_ENB1;
		msi_ena_reg[2] = CVMX_PEXP_NPEI_MSI_ENB2;
		msi_ena_reg[3] = CVMX_PEXP_NPEI_MSI_ENB3;
		octeon_irq_msi_to_hwmsi = octeon_irq_msi_to_hwmsi_scatter;
		octeon_irq_hwmsi_to_msi = octeon_irq_hwmsi_to_msi_scatter;
		msi_map_reg = CVMX_PEXP_NPEI_MSI_WR_MAP;
	} else {
		msi_rcv_reg[0] = CVMX_NPI_NPI_MSI_RCV;
#define INVALID_GENERATE_ADE 0x8700000000000000ULL;
		msi_rcv_reg[1] = INVALID_GENERATE_ADE;
		msi_rcv_reg[2] = INVALID_GENERATE_ADE;
		msi_rcv_reg[3] = INVALID_GENERATE_ADE;
		msi_ena_reg[0] = INVALID_GENERATE_ADE;
		msi_ena_reg[1] = INVALID_GENERATE_ADE;
		msi_ena_reg[2] = INVALID_GENERATE_ADE;
		msi_ena_reg[3] = INVALID_GENERATE_ADE;
		octeon_irq_msi_to_hwmsi = octeon_irq_msi_to_hwmsi_linear;
		octeon_irq_hwmsi_to_msi = octeon_irq_hwmsi_to_msi_linear;
		msi_map_reg = 0;
	}

	if (msi_map_reg) {
		int msi;
		int ciu;
		u64 e;

		for (msi = 0; msi < 256; msi++) {
			ciu = (msi >> 2) | ((msi << 6) & 0xc0);
			e = (ciu << 8) | msi;
			cvmx_write_csr(msi_map_reg, e);
		}
	}

	if (octeon_has_feature(OCTEON_FEATURE_PCIE)) {
		if (request_irq(OCTEON_IRQ_PCI_MSI0, octeon_msi_interrupt0,
				IRQF_NO_THREAD, "MSI[0:63]", octeon_msi_interrupt0))
			panic("request_irq(OCTEON_IRQ_PCI_MSI0) failed");

		if (request_irq(OCTEON_IRQ_PCI_MSI1, octeon_msi_interrupt1,
				IRQF_NO_THREAD, "MSI[64:127]", octeon_msi_interrupt1))
			panic("request_irq(OCTEON_IRQ_PCI_MSI1) failed");

		if (request_irq(OCTEON_IRQ_PCI_MSI2, octeon_msi_interrupt2,
				IRQF_NO_THREAD, "MSI[127:191]", octeon_msi_interrupt2))
			panic("request_irq(OCTEON_IRQ_PCI_MSI2) failed");

		if (request_irq(OCTEON_IRQ_PCI_MSI3, octeon_msi_interrupt3,
				IRQF_NO_THREAD, "MSI[192:255]", octeon_msi_interrupt3))
			panic("request_irq(OCTEON_IRQ_PCI_MSI3) failed");
	} else if (octeon_is_pci_host()) {
		if (request_irq(OCTEON_IRQ_PCI_MSI0, octeon_msi_interrupt0,
				IRQF_NO_THREAD, "MSI[0:15]", octeon_msi_interrupt0))
			panic("request_irq(OCTEON_IRQ_PCI_MSI0) failed");

		if (request_irq(OCTEON_IRQ_PCI_MSI1, octeon_msi_interrupt0,
				IRQF_NO_THREAD, "MSI[16:31]", octeon_msi_interrupt0))
			panic("request_irq(OCTEON_IRQ_PCI_MSI1) failed");

		if (request_irq(OCTEON_IRQ_PCI_MSI2, octeon_msi_interrupt0,
				IRQF_NO_THREAD, "MSI[32:47]", octeon_msi_interrupt0))
			panic("request_irq(OCTEON_IRQ_PCI_MSI2) failed");

		if (request_irq(OCTEON_IRQ_PCI_MSI3, octeon_msi_interrupt0,
				IRQF_NO_THREAD, "MSI[48:63]", octeon_msi_interrupt0))
			panic("request_irq(OCTEON_IRQ_PCI_MSI3) failed");
	}
	return 0;
}
subsys_initcall(octeon_msi_initialize);
