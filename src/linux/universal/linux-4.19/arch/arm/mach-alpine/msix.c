/*
 * arch/arm/mach-alpine/msix.c
 *
 * Annapurna Labs MSIX support services
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

#include <linux/pci.h>
#include <linux/msi.h>
#include <asm/irq.h>
#include <linux/irqchip/arm-gic.h>

#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>

/*
 * The IRQ range currently supported overlaps legacy interrupts of units
 * capable of using MSIX. The assumption is that either all MSIX capable units
 * will use MSIX, or none of them.
 *
 * The services below currently support only the primary GIC and not the
 * secondary GIC.
 */

static u32 al_irq_msi_addr_high;
static u32 al_irq_msi_addr_low;
static int al_irq_msi_first;
static int al_irq_msi_last;
static int al_irq_num_msi_irqs;

static DECLARE_BITMAP(msi_irq_in_use, 1000);
struct irq_domain * getgicdomain(void);

/*
 * Dynamic irq allocate and deallocation
 */
static int al_msix_create_irq(void)
{
	int irq, pos;

again:
	pos = find_first_zero_bit(msi_irq_in_use, al_irq_num_msi_irqs);
	if (pos >= al_irq_num_msi_irqs)
		return -ENOSPC;


	irq = al_irq_msi_first + pos;
	if (test_and_set_bit(pos, msi_irq_in_use))
			goto again;
//	int virq = irq_create_mapping(getgicdomain(),irq);
//		int virq = irq_find_mapping(getgicdomain(), irq);
//		if (virq)
//			return virq;

	//int virq = irq_domain_alloc_irqs(getgicdomain(), 1, NUMA_NO_NODE, NULL);
//	int virq = irq_alloc_descs(0, irq, 1, 0);
//		int virq = irq_find_mapping(getgicdomain(), irq);
//		if (virq) {
//			printk(KERN_EMERG "irq %d to %d\n", irq, virq);
//			irq_set_irq_type(virq, IRQ_TYPE_EDGE_RISING);
//    			return virq;
//		}
//		virq = irq_domain_alloc_irqs(domain, 1, NUMA_NO_NODE, fwspec);
//		if (virq <= 0)
//			return 0;
//	virq = irq_create_mapping(getgicdomain(),irq);
//	irq_set_irq_type(virq, IRQ_TYPE_EDGE_RISING);


//	printk(KERN_EMERG "irq %d to %d\n", irq, virq);
	/* test_and_set_bit operates on 32-bits at a time */

	return irq;
}

void destroy_irq(unsigned int irq)
{
	clear_bit(irq - al_irq_msi_first, msi_irq_in_use);
}

static void al_msix_irq_mask(struct irq_data *d)
{

	printk(KERN_EMERG "mask irq %d\n",d->irq);
	//if (d->common && d->common->msi_desc){ 
		pci_msi_mask_irq(d);
	//	irq_chip_mask_parent(d);
	printk(KERN_EMERG "mask irq ack %d\n",d->irq);
	///	mask_msi_irq(d);
	//}
}

static void al_msix_irq_unmask(struct irq_data *d)
{
		printk(KERN_EMERG "unmask irq %d\n",d->irq);
	//if (d->common && d->common->msi_desc){ 
		printk(KERN_EMERG "unmask irq ack %d\n",d->irq);
	pci_msi_unmask_irq(d);
	//irq_chip_unmask_parent(d);
//		unmask_msi_irq(d);
	//}
}

void arch_teardown_msi_irq(unsigned int irq)
{
	struct irq_desc *irq_desc;

	pr_debug("%s(%d)\n", __func__, irq);

	irq_desc = irq_to_desc(irq);

	if (irq_desc)
		mask_msi_irq(&irq_desc->irq_data);
	else
		pr_err("%s: irq_to_desc failed!\n", __func__);

	destroy_irq(irq);
}
#define PCIE_BUS_PRIV_DATA(pdev) \
	(((struct pci_sys_data *)pdev->bus->sysdata)->private_data)

void gic_eoimode1_mask_irq(struct irq_data *d);
void gic_unmask_irq(struct irq_data *d);

static void imx_msi_irq_ack(struct irq_data *d)
{
	printk(KERN_EMERG "irq %s,%d\n",__func__,d->irq);
	return;
}

static void imx_msi_irq_enable(struct irq_data *d)
{
	printk(KERN_EMERG "irq %s %d\n",__func__,d->irq);
	//	gic_unmask_irq(d);
	pci_msi_unmask_irq(d);
	//	unmask_msi_irq(d);
	//	irq_chip_unmask_parent(d);
	return;
}

static void imx_msi_irq_disable(struct irq_data *d)
{
	printk(KERN_EMERG "irq %s,%d\n",__func__,d->irq);
	//	gic_eoimode1_mask_irq(d);
	pci_msi_mask_irq(d);
//		mask_msi_irq(d);
	//	irq_chip_mask_parent(d);
	return;
}

static void imx_msi_irq_mask(struct irq_data *d)
{
	printk(KERN_EMERG "irq %s,%d\n",__func__,d->irq);
	//	gic_eoimode1_mask_irq(d);
	pci_msi_mask_irq(d);
//		mask_msi_irq(d);
	//	irq_chip_mask_parent(d);
	return;
}

static void imx_msi_irq_unmask(struct irq_data *d)
{
	printk(KERN_EMERG "irq %s,%d\n",__func__,d->irq);
	//	gic_unmask_irq(d);
	pci_msi_unmask_irq(d);
	//	unmask_msi_irq(d);
	//	irq_chip_unmask_parent(d);
	return;
}

static struct irq_chip msi_chip = {
	.name = "PCIe-MSI",
	.irq_ack = imx_msi_irq_ack,
	.irq_enable = imx_msi_irq_enable,
	.irq_disable = imx_msi_irq_disable,
	.irq_mask = imx_msi_irq_mask,
	.irq_unmask = imx_msi_irq_unmask,
};


int arch_setup_msi_irq(struct pci_dev *pdev, struct msi_desc *desc)
{
	struct device *dev = &pdev->dev;
	struct irq_desc *irq_desc;
	struct irq_data *irq_data;
	struct irq_domain *domain;
	int irq;
	int sgi;
	struct msi_msg msg;

	dev_dbg(dev, "%s()\n", __func__);

	irq = al_msix_create_irq();

	if (irq < 0)
		return irq;
	
	irq_set_msi_desc(irq, desc);

	/*get the hwirq from irq using the domain*/
	irq_data = irq_get_irq_data(irq);
//	domain = irq_data->domain;
//	if (domain->revmap_type) /*revmap type is not legacy*/
//		return -1;
	
	printk(KERN_EMERG "msi irq %d = hw irq %d/%d\n",irq,irq_data->hwirq,irq_data->irq);
	sgi = irq - 16 + 16;
	//irq - domain->revmap_data.legacy.first_irq +
		//		domain->revmap_data.legacy.first_hwirq;
	sgi = irq - 16;//irq_find_mapping(pci_host_bridge_of_msi_domain(pdev->bus), irq_data->hwirq);
	printk(KERN_EMERG "sgi %d\n",sgi);

	/*
	 * MSIX message address format:
	 * [63:20] - MSIx TBAR
	 *           Same value as the MSIx Translation Base  Address Register
	 * [19]    - WFE_EXIT
	 *           Once set by MSIx message, an EVENTI is signal to the CPUs
	 *           cluster specified by ‘Local GIC Target List’
	 * [18:17] - Target GIC ID
	 *           Specifies which IO-GIC (external shared GIC) is targeted
	 *           0: Local GIC, as specified by the Local GIC Target List
	 *           1: IO-GIC 0
	 *           2: Reserved
	 *           3: Reserved
	 * [16:13] - Local GIC Target List
	 *           Specifies the Local GICs list targeted by this MSIx
	 *           message.
	 *           [16]  If set, SPIn is set in Cluster 0 local GIC
	 *           [15:13] Reserved
	 *           [15]  If set, SPIn is set in Cluster 1 local GIC
	 *           [14]  If set, SPIn is set in Cluster 2 local GIC
	 *           [13]  If set, SPIn is set in Cluster 3 local GIC
	 * [12:3]  - SPIn
	 *           Specifies the SPI (Shared Peripheral Interrupt) index to
	 *           be set in target GICs
	 *           Notes:
	 *           If targeting any local GIC than only SPI[249:0] are valid
	 * [2]     - Function vector
	 *           MSI Data vector extension hint
	 * [1:0]   - Reserved
	 *           Must be set to zero
	 *
	 * In the case below:
	 * Cluster 0 local GIC. 'irq' is subtracted by 32, because the first 32
	 * interrupt IDs are for SGI and PPI.
	 */
	msg.address_hi = al_irq_msi_addr_high;

	/* Only the PPIs of the main gic are used.
	 * PPIS are hw-irqs 17-31.
	 * first_hwirq will be 16 for main gic and 32 for secondary gic.
	 * */
//	if (sgi == 16)
		msg.address_lo = al_irq_msi_addr_low + (1<<16) + (sgi << 3);
//	else if (sgi == 32)
//		msg.address_lo = al_irq_msi_addr_low + (1<<17) + (sgi << 3);
//	else
//		return -1;

	msg.data = 0;

	write_msi_msg(irq, &msg);

	irq_set_chip_and_handler(irq, &msi_chip, handle_simple_irq);

	return 0;
}
int al_msix_init(void)
{
	int status = 0;
	int irq;
	struct device_node *np;
	struct resource res;

	/* TODO: do for primary CPU only - what about sync? */
	np = of_find_compatible_node(NULL, NULL, "annapurna-labs,al-msix");
	BUG_ON(!np);

	if (of_address_to_resource(np, 0, &res))
		BUG_ON(1);

	al_irq_msi_addr_high = ((u64)res.start) >> 32;
	al_irq_msi_addr_low = res.start & 0xffffffff;
//	int irq_base = irq_alloc_descs(16, 0, 96, 0);
	al_irq_msi_first = 96;//irq_of_parse_and_map(np, 0);
	al_irq_msi_last = 159; //al_irq_msi_first + 159 - 96; //irq_of_parse_and_map(np, 1);
	al_irq_num_msi_irqs = al_irq_msi_last - al_irq_msi_first + 1;
//	irq_create_strict_mappings(getgicdomain(), al_irq_msi_first, 0, al_irq_num_msi_irqs);
	printk(KERN_EMERG "irq_base %d %d\n",al_irq_msi_first,al_irq_msi_last);
	int virq=0;
	for (irq = al_irq_msi_first; irq <= al_irq_msi_last; irq++) {
//		virq = irq_create_mapping(getgicdomain(),irq);
		printk(KERN_EMERG "add irq %d to virq %d\n",irq, virq);
//		status = irq_set_irq_type(virq, IRQ_TYPE_EDGE_RISING );

		if (status < 0) {
			pr_err("%s: set_irq_type(%d) failed!\n", __func__, irq);
			break;
		}
	}
	al_irq_msi_first = virq - (159 - 96);

//	gic_arch_extn.irq_mask = al_msix_irq_mask;
//	gic_arch_extn.irq_unmask = al_msix_irq_unmask;

	return status;
}
