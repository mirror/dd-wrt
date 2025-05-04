// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (C) 2020 Birger Koblitz <mail@birger-koblitz.de>
 * Copyright (C) 2020 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2020 John Crispin <john@phrozen.org>
 */

#include <linux/of_irq.h>
#include <linux/irqchip.h>
#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <linux/irqchip/chained_irq.h>

/* Global Interrupt Mask Register */
#define RTL_ICTL_GIMR		0x00
/* Global Interrupt Status Register */
#define RTL_ICTL_GISR		0x04
/* Interrupt Routing Registers */
#define RTL_ICTL_IRR0		0x08
#define RTL_ICTL_IRR1		0x0c
#define RTL_ICTL_IRR2		0x10
#define RTL_ICTL_IRR3		0x14

#define RTL_ICTL_NUM_INPUTS	32
#define RTL_ICTL_NUM_OUTPUTS	15

static DEFINE_RAW_SPINLOCK(irq_lock);

#define REG(offset, cpu)	(realtek_ictl_base[cpu] + offset)

static u32 realtek_ictl_unmask[NR_CPUS];
static void __iomem *realtek_ictl_base[NR_CPUS];
static cpumask_t realtek_ictl_cpu_configurable;

struct realtek_ictl_output {
	/* IRQ controller data */
	struct fwnode_handle *fwnode;
	/* Output specific data */
	unsigned int output_index;
	struct irq_domain *domain;
	u32 child_mask;
};

/*
 * Per CPU we have a set of 5 registers that determine interrupt handling for
 * 32 external interrupts. GIMR (enable/disable interrupt) plus IRR0-IRR3 that
 * contain "routing" or "priority" values. GIMR uses one bit for each interrupt
 * and IRRx store 4 bits per interrupt. Realtek uses inverted numbering,
 * placing IRQ 31 in the first four bits. The register combinations give the
 * following results for a single interrupt in the wild:
 *
 * a) GIMR = 0 / IRRx > 0 -> no interrupts
 * b) GIMR = 0 / IRRx = 0 -> no interrupts
 * c) GIMR = 1 / IRRx > 0 -> interrupts
 * d) GIMR = 1 / IRRx = 0 -> rare interrupts in SMP environment
 *
 * Combination d) seems to trigger interrupts only on a VPE if the other VPE
 * has GIMR = 0 and IRRx > 0. E.g. busy without interrupts allowed. To provide
 * IRQ balancing features in SMP this driver will handle the registers as
 * follows:
 *
 * 1) set IRRx > 0 for VPE where the interrupt is desired
 * 2) set IRRx = 0 for VPE where the interrupt is not desired
 * 3) set both GIMR = 0 to mask (disabled) interrupt
 * 4) set GIMR = 1 to unmask (enable) interrupt but only for VPE where IRRx > 0
 */

#define IRR_OFFSET(idx)		(4 * (3 - (idx * 4) / 32))
#define IRR_SHIFT(idx)		((idx * 4) % 32)

static inline u32 read_irr(void __iomem *irr0, int idx)
{
	return (readl(irr0 + IRR_OFFSET(idx)) >> IRR_SHIFT(idx)) & 0xf;
}

static inline void write_irr(void __iomem *irr0, int idx, u32 value)
{
	unsigned int offset = IRR_OFFSET(idx);
	unsigned int shift = IRR_SHIFT(idx);
	u32 irr;

	irr = readl(irr0 + offset) & ~(0xf << shift);
	irr |= (value & 0xf) << shift;
	writel(irr, irr0 + offset);
}

static inline void enable_gimr(int hwirq, int cpu)
{
	u32 value;

	value = readl(REG(RTL_ICTL_GIMR, cpu));
	value |= (BIT(hwirq) & realtek_ictl_unmask[cpu]);
	writel(value, REG(RTL_ICTL_GIMR, cpu));
}

static inline void disable_gimr(int hwirq, int cpu)
{
	u32 value;

	value = readl(REG(RTL_ICTL_GIMR, cpu));
	value &= ~BIT(hwirq);
	writel(value, REG(RTL_ICTL_GIMR, cpu));
}

static void realtek_ictl_unmask_irq(struct irq_data *i)
{
	unsigned long flags;
	int cpu;

	raw_spin_lock_irqsave(&irq_lock, flags);

	for_each_cpu(cpu, &realtek_ictl_cpu_configurable)
		enable_gimr(i->hwirq, cpu);

	raw_spin_unlock_irqrestore(&irq_lock, flags);
}

static void realtek_ictl_mask_irq(struct irq_data *i)
{
	unsigned long flags;
	int cpu;

	raw_spin_lock_irqsave(&irq_lock, flags);

	for_each_cpu(cpu, &realtek_ictl_cpu_configurable)
		disable_gimr(i->hwirq, cpu);

	raw_spin_unlock_irqrestore(&irq_lock, flags);
}

static int __maybe_unused realtek_ictl_irq_affinity(struct irq_data *i,
	const struct cpumask *dest, bool force)
{
	struct realtek_ictl_output *output = i->domain->host_data;
	cpumask_t cpu_configure;
	cpumask_t cpu_disable;
	cpumask_t cpu_enable;
	unsigned long flags;
	int cpu;

	raw_spin_lock_irqsave(&irq_lock, flags);

	cpumask_and(&cpu_configure, cpu_present_mask, &realtek_ictl_cpu_configurable);

	cpumask_and(&cpu_enable, &cpu_configure, dest);
	cpumask_andnot(&cpu_disable, &cpu_configure, dest);

	for_each_cpu(cpu, &cpu_disable) {
		write_irr(REG(RTL_ICTL_IRR0, cpu), i->hwirq, 0);
		realtek_ictl_unmask[cpu] &= ~BIT(i->hwirq);
		disable_gimr(i->hwirq, cpu);
	}

	for_each_cpu(cpu, &cpu_enable) {
		write_irr(REG(RTL_ICTL_IRR0, cpu), i->hwirq, output->output_index + 1);
		realtek_ictl_unmask[cpu] |= BIT(i->hwirq);
		enable_gimr(i->hwirq, cpu);
	}

	irq_data_update_effective_affinity(i, &cpu_enable);

	raw_spin_unlock_irqrestore(&irq_lock, flags);

	return IRQ_SET_MASK_OK;
}

static struct irq_chip realtek_ictl_irq = {
	.name = "realtek-rtl-intc",
	.irq_mask = realtek_ictl_mask_irq,
	.irq_unmask = realtek_ictl_unmask_irq,
#ifdef CONFIG_SMP
	.irq_set_affinity = realtek_ictl_irq_affinity,
#endif
};

static int intc_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hw)
{
	struct realtek_ictl_output *output = d->host_data;
	unsigned long flags;

	irq_set_chip_and_handler(irq, &realtek_ictl_irq, handle_level_irq);

	raw_spin_lock_irqsave(&irq_lock, flags);

	output->child_mask |= BIT(hw);
	write_irr(REG(RTL_ICTL_IRR0, 0), hw, output->output_index + 1);
	realtek_ictl_unmask[0] |= BIT(hw);

	raw_spin_unlock_irqrestore(&irq_lock, flags);

	return 0;
}

static int intc_select(struct irq_domain *d, struct irq_fwspec *fwspec,
	enum irq_domain_bus_token bus_token)
{
	struct realtek_ictl_output *output = d->host_data;
	bool routed_elsewhere;
	unsigned long flags;
	u32 routing_old;
	int cpu;

	if (fwspec->fwnode != output->fwnode)
		return false;

	/* Original specifiers had only one parameter */
	if (fwspec->param_count < 2)
		return true;

	raw_spin_lock_irqsave(&irq_lock, flags);

	/*
	 * Inputs can only be routed to one output, so they shouldn't be
	 * allowed to end up in multiple domains.
	 */
	for_each_cpu(cpu, &realtek_ictl_cpu_configurable) {
		routing_old = read_irr(REG(RTL_ICTL_IRR0, cpu), fwspec->param[0]);
		routed_elsewhere = routing_old && fwspec->param[1] != routing_old - 1;
		if (routed_elsewhere) {
			pr_warn("soc int %d already routed to output %d\n",
				fwspec->param[0], routing_old - 1);
			break;
		}
	}

	raw_spin_unlock_irqrestore(&irq_lock, flags);

	return !routed_elsewhere && fwspec->param[1] == output->output_index;
}

static const struct irq_domain_ops irq_domain_ops = {
	.map = intc_map,
	.select = intc_select,
	.xlate = irq_domain_xlate_onecell,
};

static void realtek_irq_dispatch(struct irq_desc *desc)
{
	struct realtek_ictl_output *output = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	int cpu = smp_processor_id();
	unsigned long pending;
	unsigned int soc_int;

	chained_irq_enter(chip, desc);
	pending = readl(REG(RTL_ICTL_GIMR, cpu)) & readl(REG(RTL_ICTL_GISR, cpu))
		& output->child_mask;

	if (unlikely(!pending)) {
		spurious_interrupt();
		goto out;
	}

	for_each_set_bit(soc_int, &pending, RTL_ICTL_NUM_INPUTS)
		generic_handle_domain_irq(output->domain, soc_int);

out:
	chained_irq_exit(chip, desc);
}

/*
 * SoC interrupts are cascaded to MIPS CPU interrupts according to the
 * interrupt-map in the device tree. Each SoC interrupt gets 4 bits for
 * the CPU interrupt in an Interrupt Routing Register. Max 32 SoC interrupts
 * thus go into 4 IRRs. A routing value of '0' means the interrupt is left
 * disconnected. Routing values {1..15} connect to output lines {0..14}.
 */
static int __init setup_parent_interrupts(struct device_node *node, int *parents,
	unsigned int num_parents)
{
	struct realtek_ictl_output *outputs;
	struct realtek_ictl_output *output;
	struct irq_domain *domain;
	unsigned int p;

	outputs = kcalloc(num_parents, sizeof(*outputs), GFP_KERNEL);
	if (!outputs)
		return -ENOMEM;

	for (p = 0; p < num_parents; p++) {
		output = outputs + p;

		domain = irq_domain_add_linear(node, RTL_ICTL_NUM_INPUTS, &irq_domain_ops, output);
		if (!domain)
			goto domain_err;

		output->fwnode = of_node_to_fwnode(node);
		output->output_index = p;
		output->domain = domain;

		irq_set_chained_handler_and_data(parents[p], realtek_irq_dispatch, output);
	}

	return 0;

domain_err:
	while (p--) {
		irq_set_chained_handler_and_data(parents[p], NULL, NULL);
		irq_domain_remove(outputs[p].domain);
	}

	kfree(outputs);

	return -ENOMEM;
}

static int __init realtek_rtl_of_init(struct device_node *node, struct device_node *parent)
{
	int parent_irqs[RTL_ICTL_NUM_OUTPUTS];
	struct of_phandle_args oirq;
	unsigned int num_parents;
	unsigned int soc_irq;
	unsigned int p;
	int cpu;

	cpumask_clear(&realtek_ictl_cpu_configurable);

	for (cpu = 0; cpu < NR_CPUS; cpu++) {
		realtek_ictl_base[cpu] = of_iomap(node, cpu);
		if (realtek_ictl_base[cpu]) {
			cpumask_set_cpu(cpu, &realtek_ictl_cpu_configurable);

			/* Disable all cascaded interrupts and clear routing */
			for (soc_irq = 0; soc_irq < RTL_ICTL_NUM_INPUTS; soc_irq++) {
				write_irr(REG(RTL_ICTL_IRR0, cpu), soc_irq, 0);
				realtek_ictl_unmask[cpu] &= ~BIT(soc_irq);
				disable_gimr(soc_irq, cpu);
			}
		}
	}

	if (cpumask_empty(&realtek_ictl_cpu_configurable))
		return -ENXIO;

	num_parents = of_irq_count(node);
	if (num_parents > RTL_ICTL_NUM_OUTPUTS) {
		pr_err("too many parent interrupts\n");
		return -EINVAL;
	}

	for (p = 0; p < num_parents; p++)
		parent_irqs[p] = of_irq_get(node, p);

	if (WARN_ON(!num_parents)) {
		/*
		 * If DT contains no parent interrupts, assume MIPS CPU IRQ 2
		 * (HW0) is connected to the first output. This is the case for
		 * all known hardware anyway. "interrupt-map" is deprecated, so
		 * don't bother trying to parse that.
		 * Since this is to account for old devicetrees with one-cell
		 * interrupt specifiers, only one output domain is needed.
		 */
		oirq.np = of_find_compatible_node(NULL, NULL, "mti,cpu-interrupt-controller");
		if (oirq.np) {
			oirq.args_count = 1;
			oirq.args[0] = 2;

			parent_irqs[0] = irq_create_of_mapping(&oirq);
			num_parents = 1;
		}

		of_node_put(oirq.np);
	}

	/* Ensure we haven't collected any errors before proceeding */
	for (p = 0; p < num_parents; p++) {
		if (parent_irqs[p] < 0)
			return parent_irqs[p];
		if (!parent_irqs[p])
			return -ENODEV;
	}

	return setup_parent_interrupts(node, &parent_irqs[0], num_parents);
}

IRQCHIP_DECLARE(realtek_rtl_intc, "realtek,rtl-intc", realtek_rtl_of_init);
