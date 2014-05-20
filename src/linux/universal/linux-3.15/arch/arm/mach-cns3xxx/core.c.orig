/*
 * Copyright 1999 - 2003 ARM Limited
 * Copyright 2000 Deep Blue Solutions Ltd
 * Copyright 2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/clockchips.h>
#include <linux/io.h>
#include <linux/irqchip/arm-gic.h>
#include <linux/export.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>
#include <asm/mach/irq.h>
#include <asm/hardware/cache-l2x0.h>
#include <asm/smp_twd.h>
#include <linux/gpio.h>
#include <mach/cns3xxx.h>
#include "core.h"



static struct map_desc cns3xxx_io_desc[] __initdata = {
	{
		.virtual	= CNS3XXX_TC11MP_TWD_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TC11MP_TWD_BASE),
		.length		= SZ_8K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_TIMER1_2_3_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TIMER1_2_3_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_GPIOA_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_GPIOA_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_GPIOB_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_GPIOB_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_MISC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_MISC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PM_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PM_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_SWITCH_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_SWITCH_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_SSP_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_SSP_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_L2C_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_L2C_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_IO_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_IO_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE1_IO_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_IO_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	},
};

int irq2gpio(int irq)
{
	if (irq == IRQ_CNS3XXX_GPIOA)
		return 0;
	else if (irq == IRQ_CNS3XXX_GPIOB)
		return 32;
	else
		return -EINVAL;
}
EXPORT_SYMBOL(irq2gpio);

static int cns3xxx_gpio_direction_input(struct gpio_chip *chip, unsigned gpio)
{
	gpio_line_config(gpio, CNS3XXX_GPIO_IN);
	return 0;
}

static int cns3xxx_gpio_direction_output(struct gpio_chip *chip, unsigned gpio, int level)
{
	gpio_line_set(gpio, level);
	gpio_line_config(gpio, CNS3XXX_GPIO_OUT);
	return 0;
}

static int cns3xxx_gpio_get_value(struct gpio_chip *chip, unsigned gpio)
{
	int value;
	gpio_line_get(gpio,&value);
	return value;
}

static void cns3xxx_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	gpio_line_set(gpio, value);
}


static int cns3xxx_gpio_to_irq(struct gpio_chip *chip, unsigned pin)
{

	if (pin > 63)
		return -EINVAL;

	if (pin < 32)
		return IRQ_CNS3XXX_GPIOA;
	else
		return IRQ_CNS3XXX_GPIOB;
}

static struct gpio_chip cns3xxx_gpio_chip = {
	.label			= "CNS3XXX_GPIO_CHIP",
	.direction_input	= cns3xxx_gpio_direction_input,
	.direction_output	= cns3xxx_gpio_direction_output,
	.get			= cns3xxx_gpio_get_value,
	.set			= cns3xxx_gpio_set_value,
	.base			= 0,
	.ngpio			= 64,
	.to_irq			= cns3xxx_gpio_to_irq,
};


static DEFINE_TWD_LOCAL_TIMER(twd_local_timer,
			      CNS3XXX_TC11MP_TWD_BASE,
			      IRQ_LOCALTIMER);

void __init cns3xxx_common_init(void)
{
	iotable_init(cns3xxx_io_desc, ARRAY_SIZE(cns3xxx_io_desc));
#ifdef CONFIG_CACHE_L2X0
	void __iomem *l2x0_base = (void __iomem *) CNS3XXX_L2C_BASE_VIRT;

	/* set RAM latencies to 1 cycle for this core tile. */
	writel(0, l2x0_base + L2X0_TAG_LATENCY_CTRL);
	writel(0, l2x0_base + L2X0_DATA_LATENCY_CTRL);

	l2x0_init(l2x0_base, 0x00400000, 0xfe0fffff);
#endif
#ifdef CONFIG_CACHE_L2CC
	l2cc_init((void __iomem *) CNS3XXX_L2C_BASE_VIRT);
#endif
	gpiochip_add(&cns3xxx_gpio_chip);
}

/* used by entry-macro.S */
void __init cns3xxx_init_irq(void)
{
	gic_init(0, 29, (void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT,
		 (void __iomem *) CNS3XXX_TC11MP_GIC_CPU_BASE_VIRT);
}

void cns3xxx_power_off(void)
{
	u32 __iomem *pm_base = (void __iomem *) CNS3XXX_PM_BASE_VIRT;
	u32 clkctrl;

	printk(KERN_INFO "powering system down...\n");

	clkctrl = readl(pm_base + PM_SYS_CLK_CTRL_OFFSET);
	clkctrl &= 0xfffff1ff;
	clkctrl |= (0x5 << 9);		/* Hibernate */
	writel(clkctrl, pm_base + PM_SYS_CLK_CTRL_OFFSET);

}

/*
 * Timer
 */
static void __iomem *cns3xxx_tmr1;

static void cns3xxx_timer_set_mode(enum clock_event_mode mode,
				   struct clock_event_device *clk)
{
	unsigned long ctrl = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	int pclk = cns3xxx_cpu_clock() / 8;
	int reload;

	switch (mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		reload = pclk * 1000000 / HZ;
		writel(reload, cns3xxx_tmr1 + TIMER1_AUTO_RELOAD_OFFSET);
		ctrl |= (1 << 0) | (1 << 2) | (1 << 9);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		writel(0, cns3xxx_tmr1 + TIMER1_AUTO_RELOAD_OFFSET);
		ctrl |= (1 << 2) | (1 << 9);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		ctrl = 0;
	}

	writel(ctrl, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
}

static int cns3xxx_timer_set_next_event(unsigned long evt,
					struct clock_event_device *unused)
{
	unsigned long ctrl = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	writel(evt, cns3xxx_tmr1 + TIMER1_AUTO_RELOAD_OFFSET);
	writel(ctrl | (1 << 0), cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	return 0;
}

static struct clock_event_device cns3xxx_tmr1_clockevent = {
	.name		= "cns3xxx timer1",
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= cns3xxx_timer_set_mode,
	.set_next_event	= cns3xxx_timer_set_next_event,
	.rating		= 300,
	.cpumask	= cpu_all_mask,
};

static void __init cns3xxx_clockevents_init(unsigned int timer_irq)
{
	cns3xxx_tmr1_clockevent.irq = timer_irq;
	clockevents_config_and_register(&cns3xxx_tmr1_clockevent,
					(cns3xxx_cpu_clock() >> 3) * 1000000,
					0xf, 0xffffffff);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t cns3xxx_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *evt = &cns3xxx_tmr1_clockevent;
	u32 __iomem *stat = cns3xxx_tmr1 + TIMER1_2_INTERRUPT_STATUS_OFFSET;
	u32 val;

	/* Clear the interrupt */
	val = readl(stat);
	writel(val & ~(1 << 2), stat);

	evt->event_handler(evt);

	return IRQ_HANDLED;
}

static struct irqaction cns3xxx_timer_irq = {
	.name		= "timer",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= cns3xxx_timer_interrupt,
};

static cycle_t cns3xxx_get_cycles(struct clocksource *cs)
{
  u64 val;

  val = readl(cns3xxx_tmr1 + TIMER_FREERUN_CONTROL_OFFSET);
  val &= 0xffff;

  return ((val << 32) | readl(cns3xxx_tmr1 + TIMER_FREERUN_OFFSET));
}

static struct clocksource clocksource_cns3xxx = {
	.name = "freerun",
	.rating = 200,
	.read = cns3xxx_get_cycles,
	.mask = CLOCKSOURCE_MASK(48),
	.shift  = 16,
	.flags  = CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init cns3xxx_clocksource_init(void)
{
	/* Reset the FreeRunning counter */
	writel((1 << 16), cns3xxx_tmr1 + TIMER_FREERUN_CONTROL_OFFSET);

	clocksource_cns3xxx.mult =
		clocksource_khz2mult(100, clocksource_cns3xxx.shift);
	clocksource_register(&clocksource_cns3xxx);
}
/*
 * Set up the clock source and clock events devices
 */
static void __init __cns3xxx_timer_init(unsigned int timer_irq)
{
	u32 val;
	u32 irq_mask;

	/*
	 * Initialise to a known state (all timers off)
	 */

	/* disable timer1 and timer2 */
	writel(0, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	/* stop free running timer3 */
	writel(0, cns3xxx_tmr1 + TIMER_FREERUN_CONTROL_OFFSET);

	writel(0, cns3xxx_tmr1 + TIMER1_MATCH_V1_OFFSET);
	writel(0, cns3xxx_tmr1 + TIMER1_MATCH_V2_OFFSET);

	val = (cns3xxx_cpu_clock() >> 3) * 1000000 / HZ;
	writel(val, cns3xxx_tmr1 + TIMER1_COUNTER_OFFSET);

	/* mask irq, non-mask timer1 overflow */
	irq_mask = readl(cns3xxx_tmr1 + TIMER1_2_INTERRUPT_MASK_OFFSET);
	irq_mask &= ~(1 << 2);
	irq_mask |= 0x03;
	writel(irq_mask, cns3xxx_tmr1 + TIMER1_2_INTERRUPT_MASK_OFFSET);

	/* down counter */
	val = readl(cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);
	val |= (1 << 9);
	writel(val, cns3xxx_tmr1 + TIMER1_2_CONTROL_OFFSET);

	setup_irq(timer_irq, &cns3xxx_timer_irq);

	cns3xxx_clocksource_init();
	cns3xxx_clockevents_init(timer_irq);
#ifdef CONFIG_LOCAL_TIMERS
	int err = twd_local_timer_register(&twd_local_timer);
	if (err)
		pr_err("twd_local_timer_register failed %d\n", err);
//	twd_base = (void __iomem *) CNS3XXX_TC11MP_TWD_BASE_VIRT;
#endif
}

void __init cns3xxx_timer_init(void)
{
	cns3xxx_tmr1 = (void __iomem *) CNS3XXX_TIMER1_2_3_BASE_VIRT;

	__cns3xxx_timer_init(IRQ_CNS3XXX_TIMER0);
}

#ifdef CONFIG_CACHE_L2X0

void __init cns3xxx_l2x0_init(void)
{
	void __iomem *base = ioremap(CNS3XXX_L2C_BASE, SZ_4K);
	u32 val;

	if (WARN_ON(!base))
		return;

	/*
	 * Tag RAM Control register
	 *
	 * bit[10:8]	- 1 cycle of write accesses latency
	 * bit[6:4]	- 1 cycle of read accesses latency
	 * bit[3:0]	- 1 cycle of setup latency
	 *
	 * 1 cycle of latency for setup, read and write accesses
	 */
	val = readl(base + L2X0_TAG_LATENCY_CTRL);
	val &= 0xfffff888;
	writel(val, base + L2X0_TAG_LATENCY_CTRL);

	/*
	 * Data RAM Control register
	 *
	 * bit[10:8]	- 1 cycles of write accesses latency
	 * bit[6:4]	- 1 cycles of read accesses latency
	 * bit[3:0]	- 1 cycle of setup latency
	 *
	 * 1 cycle of latency for setup, read and write accesses
	 */
	val = readl(base + L2X0_DATA_LATENCY_CTRL);
	val &= 0xfffff888;
	writel(val, base + L2X0_DATA_LATENCY_CTRL);

	/* 32 KiB, 8-way, parity disable */
	l2x0_init(base, 0x00540000, 0xfe000fff);
}

#endif /* CONFIG_CACHE_L2X0 */
