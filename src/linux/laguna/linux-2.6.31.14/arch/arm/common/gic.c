/*
 *  linux/arch/arm/common/gic.c
 *
 *  Copyright (C) 2002 ARM Limited, All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Interrupt architecture for the GIC:
 *
 * o There is one Interrupt Distributor, which receives interrupts
 *   from system devices and sends them to the Interrupt Controllers.
 *
 * o There is one CPU Interface per CPU, which sends interrupts sent
 *   by the Distributor, and interrupts generated locally, to the
 *   associated CPU. The base address of the CPU interface is usually
 *   aliased so that the same address points to different chips depending
 *   on the CPU it is accessed from.
 *
 * Note that IRQs 0-31 are special - they are local to each CPU.
 * As such, the enable set/clear, pending set/clear and active bit
 * registers are banked per-cpu for these sources.
 */
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/smp.h>
#include <linux/cpumask.h>
#include <linux/io.h>

#include <asm/irq.h>
#include <asm/mach/irq.h>
#include <asm/hardware/gic.h>
#include <linux/module.h>


static DEFINE_SPINLOCK(irq_controller_lock);

struct gic_chip_data {
	unsigned int irq_offset;
	void __iomem *dist_base;
	void __iomem *cpu_base;
};

#ifndef MAX_GIC_NR
#define MAX_GIC_NR	1
#endif

static struct gic_chip_data gic_data[MAX_GIC_NR];

static inline void __iomem *gic_dist_base(unsigned int irq)
{
	struct gic_chip_data *gic_data = get_irq_chip_data(irq);
	return gic_data->dist_base;
}

static inline void __iomem *gic_cpu_base(unsigned int irq)
{
	struct gic_chip_data *gic_data = get_irq_chip_data(irq);
	return gic_data->cpu_base;
}

static inline unsigned int gic_irq(unsigned int irq)
{
	struct gic_chip_data *gic_data = get_irq_chip_data(irq);
	return irq - gic_data->irq_offset;
}

/*
 * Routines to acknowledge, disable and enable interrupts
 *
 * Linux assumes that when we're done with an interrupt we need to
 * unmask it, in the same way we need to unmask an interrupt when
 * we first enable it.
 *
 * The GIC has a separate notion of "end of interrupt" to re-enable
 * an interrupt after handling, in order to support hardware
 * prioritisation.
 *
 * We can make the GIC behave in the way that Linux expects by making
 * our "acknowledge" routine disable the interrupt, then mark it as
 * complete.
 */
void gic_ack_irq(unsigned int irq)
{
	u32 mask = 1 << (irq % 32);

	spin_lock(&irq_controller_lock);
	writel(mask, gic_dist_base(irq) + GIC_DIST_ENABLE_CLEAR + (gic_irq(irq) / 32) * 4);
	writel(gic_irq(irq), gic_cpu_base(irq) + GIC_CPU_EOI);
	spin_unlock(&irq_controller_lock);
}

void gic_mask_irq(unsigned int irq)
{
	u32 mask = 1 << (irq % 32);

	spin_lock(&irq_controller_lock);
	writel(mask, gic_dist_base(irq) + GIC_DIST_ENABLE_CLEAR + (gic_irq(irq) / 32) * 4);
	spin_unlock(&irq_controller_lock);
}


void gic_unmask_irq(unsigned int irq)
{
	u32 mask = 1 << (irq % 32);

	spin_lock(&irq_controller_lock);
	writel(mask, gic_dist_base(irq) + GIC_DIST_ENABLE_SET + (gic_irq(irq) / 32) * 4);
	spin_unlock(&irq_controller_lock);
}

#ifdef CONFIG_SMP
static int gic_set_cpu(unsigned int irq, const struct cpumask *mask_val)
{
	void __iomem *reg = gic_dist_base(irq) + GIC_DIST_TARGET + (gic_irq(irq) & ~3);
	unsigned int shift = (irq % 4) * 8;
	unsigned int cpu = cpumask_first(mask_val);
	u32 val;

	spin_lock(&irq_controller_lock);
	irq_desc[irq].node = cpu;
	val = readl(reg) & ~(0xff << shift);
	val |= 1 << (cpu + shift);
	writel(val, reg);
	spin_unlock(&irq_controller_lock);

	return 0;
}
#endif

static void gic_handle_cascade_irq(unsigned int irq, struct irq_desc *desc)
{
	struct gic_chip_data *chip_data = get_irq_data(irq);
	struct irq_chip *chip = get_irq_chip(irq);
	unsigned int cascade_irq, gic_irq;
	unsigned long status;

	/* primary controller ack'ing */
	chip->ack(irq);

	spin_lock(&irq_controller_lock);
	status = readl(chip_data->cpu_base + GIC_CPU_INTACK);
	spin_unlock(&irq_controller_lock);

	gic_irq = (status & 0x3ff);
	if (gic_irq == 1023)
		goto out;

	cascade_irq = gic_irq + chip_data->irq_offset;
	if (unlikely(gic_irq < 32 || gic_irq > 1020 || cascade_irq >= NR_IRQS))
		do_bad_IRQ(cascade_irq, desc);
	else
		generic_handle_irq(cascade_irq);

 out:
	/* primary controller unmasking */
	chip->unmask(irq);
}

static struct irq_chip gic_chip = {
	.name		= "GIC",
	.ack		= gic_ack_irq,
	.mask		= gic_mask_irq,
	.unmask		= gic_unmask_irq,
#ifdef CONFIG_SMP
	.set_affinity	= gic_set_cpu,
#endif
};

void __init gic_cascade_irq(unsigned int gic_nr, unsigned int irq)
{
	if (gic_nr >= MAX_GIC_NR)
		BUG();
	if (set_irq_data(irq, &gic_data[gic_nr]) != 0)
		BUG();
	set_irq_chained_handler(irq, gic_handle_cascade_irq);
}


// type: level or edge 
// 0 - level high active, 1 - rising edge sensitive
void set_interrupt_type_by_base(void __iomem *base, int id, u32 type)
{
        unsigned char int_type_bit=0;
        u32 gic_v=0;

        // judge gic offset
        //printk("gic addr: %#x\n", id/16*4 + 0xc00);
        //printk("gic addr bits: %#x\n", id%16*2);
        int_type_bit=(id%16*2+1);
        
        gic_v = readl(base + GIC_DIST_CONFIG + id/16*4);

        gic_v &= (~(1 << int_type_bit));
        gic_v |= ( type << int_type_bit);

        writel(gic_v, base + GIC_DIST_CONFIG + id/16*4);
}

// type: level or edge 
// 0 - level high active, 1 - rising edge sensitive
void set_interrupt_type(int id, u32 type)
{
	set_interrupt_type_by_base((void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT, id, type);
}

void get_interrupt_type_by_base(void __iomem *base, u32 id, u32 *type)
{
        unsigned char int_type_bit=0;
        u32 gic_v=0;
	//u16 offset=id/16*4;

        // judge gic offset
        int_type_bit=(id%16*2+1);
        
        //gic_v = readl(base + GIC_DIST_CONFIG + 4);
        gic_v = readl(base + GIC_DIST_CONFIG + id/16*4);

        *type = ((gic_v >> int_type_bit) & 0x1);

        //writel(0, base + GIC_DIST_CONFIG + id/16*4);
}

void get_interrupt_type(u32 id, u32 *type)
{
	get_interrupt_type_by_base((void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT, id, type);
}



// set interrupt priority
void set_interrupt_pri_by_base(void __iomem *base, u32 id, u32 pri)
{
        unsigned char int_type_bit=0;
        u32 gic_v=0;


        // judge gic offset
        int_type_bit=(id%4*8+4);

        gic_v = readl(base + GIC_DIST_PRI + id/4*4);

        gic_v &= (~(0xf << int_type_bit));
        gic_v |= (pri << int_type_bit);

        writel(gic_v, base + GIC_DIST_PRI + id/4*4);

        gic_v = 0;
        gic_v = readl(base + GIC_DIST_PRI + id/4*4);
	//printk("read gic_v: %x\n", gic_v);
}

void set_interrupt_pri(u32 id, u32 pri)
{
	set_interrupt_pri_by_base((void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT, id, pri);
}

void get_interrupt_pri_by_base(void __iomem *base, int id, u32 *type)
{
        unsigned char int_type_bit=0;
        u32 gic_v=0;

        // judge gic offset
        int_type_bit=(id%4*8+4);
        
        gic_v = readl(base + GIC_DIST_PRI + id/4*4);

	//printk("int_type_bit: %d\n", int_type_bit);
	//printk("gic_v: %#x\n", gic_v);
        *type = ((gic_v >> int_type_bit) & 0xf);
        //gic_v &= (~(1 << int_type_bit));
        //gic_v |= ( type << int_type_bit);

        //writel(0, base + GIC_DIST_CONFIG + id/16*4);
}

void get_interrupt_pri(int id, u32 *pri)
{
	get_interrupt_pri_by_base((void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT, id, pri);
}


void __init gic_dist_init(unsigned int gic_nr, void __iomem *base,
			  unsigned int irq_start)
{
	unsigned int max_irq, i;
	u32 cpumask = 1 << smp_processor_id();

	if (gic_nr >= MAX_GIC_NR)
		BUG();

	cpumask |= cpumask << 8;
	cpumask |= cpumask << 16;
	gic_data[gic_nr].dist_base = base;
	gic_data[gic_nr].irq_offset = (irq_start - 1) & ~31;

	writel(0, base + GIC_DIST_CTRL);

	/*
	 * Find out how many interrupts are supported.
	 */
	max_irq = readl(base + GIC_DIST_CTR) & 0x1f;
	max_irq = (max_irq + 1) * 32;

	/*
	 * The GIC only supports up to 1020 interrupt sources.
	 * Limit this to either the architected maximum, or the
	 * platform maximum.
	 */
	if (max_irq > max(1020, NR_IRQS))
		max_irq = max(1020, NR_IRQS);

	/*
	 * Set all global interrupts to be level triggered, active low.
	 */
	for (i = 32; i < max_irq; i += 16)
		writel(0, base + GIC_DIST_CONFIG + i * 4 / 16);

	/*
	 * Set all global interrupts to this CPU only.
	 */
	for (i = 32; i < max_irq; i += 4)
		writel(cpumask, base + GIC_DIST_TARGET + i * 4 / 4);

	/*
	 * Set priority on all interrupts.
	 */
	for (i = 0; i < max_irq; i += 4)
		writel(0xa0a0a0a0, base + GIC_DIST_PRI + i * 4 / 4);

	/*
	 * Disable all interrupts.
	 */
	for (i = 0; i < max_irq; i += 32)
		writel(0xffffffff, base + GIC_DIST_ENABLE_CLEAR + i * 4 / 32);

	/*
	 * Setup the Linux IRQ subsystem.
	 */
	for (i = irq_start; i < gic_data[gic_nr].irq_offset + max_irq; i++) {
		set_irq_chip(i, &gic_chip);
		set_irq_chip_data(i, &gic_data[gic_nr]);
		set_irq_handler(i, handle_level_irq);
		set_irq_flags(i, IRQF_VALID | IRQF_PROBE);
	}

	writel(1, base + GIC_DIST_CTRL);
}

void __cpuinit gic_cpu_init(unsigned int gic_nr, void __iomem *base)
{
	if (gic_nr >= MAX_GIC_NR)
		BUG();

	gic_data[gic_nr].cpu_base = base;

	writel(0xf0, base + GIC_CPU_PRIMASK);
	writel(1, base + GIC_CPU_CTRL);
}

void cns3xxx_write_pri_mask(u8 pri_mask)
{
        writel(pri_mask, (void __iomem *) CNS3XXX_TC11MP_GIC_CPU_BASE_VIRT + GIC_CPU_PRIMASK);
}

/* This is needed by AMP as softirq is raised by CPU0 TO CPU1 for WFI */
#if defined(CONFIG_SMP) || defined(CONFIG_AMP)
void gic_raise_softirq(const struct cpumask *mask, unsigned int irq)
{
	unsigned long map = *cpus_addr(*mask);

	/* this always happens on GIC0 */
	writel(map << 16 | irq, gic_data[0].dist_base + GIC_DIST_SOFTINT);
}
#endif

EXPORT_SYMBOL_GPL(cns3xxx_write_pri_mask);
EXPORT_SYMBOL_GPL(set_interrupt_type);
EXPORT_SYMBOL_GPL(get_interrupt_type);
EXPORT_SYMBOL_GPL(set_interrupt_pri);
EXPORT_SYMBOL_GPL(get_interrupt_pri);
EXPORT_SYMBOL_GPL(gic_ack_irq);
EXPORT_SYMBOL_GPL(gic_mask_irq);
EXPORT_SYMBOL_GPL(gic_unmask_irq);


