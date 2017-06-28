/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2013 by John Crispin <john@phrozen.org>
 */

#include <linux/clockchips.h>
#include <linux/clocksource.h>
#include <linux/interrupt.h>
#include <linux/reset.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>

#include <asm/mach-ralink/ralink_regs.h>

#define SYSTICK_FREQ		(50 * 1000)

#define SYSTICK_CONFIG		0x00
#define SYSTICK_COMPARE		0x04
#define SYSTICK_COUNT		0x08

/* route systick irq to mips irq 7 instead of the r4k-timer */
#define CFG_EXT_STK_EN		0x2
/* enable the counter */
#define CFG_CNT_EN		0x1

/* mt7620 frequency scaling defines */
#define CLK_LUT_CFG	0x40
#define SLEEP_EN	BIT(31)

struct systick_device {
	void __iomem *membase;
	struct clock_event_device dev;
	int irq_requested;
	int freq_scale;
};

static void (*systick_freq_scaling)(struct systick_device *sdev, int status);

static int systick_set_oneshot(struct clock_event_device *evt);
static int systick_shutdown(struct clock_event_device *evt);

static inline void mt7620_freq_scaling(struct systick_device *sdev, int status)
{
	if (sdev->freq_scale == status)
		return;

	sdev->freq_scale = status;

	pr_info("%s: %s autosleep mode\n", sdev->dev.name,
			(status) ? ("enable") : ("disable"));
	if (status)
		rt_sysc_w32(rt_sysc_r32(CLK_LUT_CFG) | SLEEP_EN, CLK_LUT_CFG);
	else
		rt_sysc_w32(rt_sysc_r32(CLK_LUT_CFG) & ~SLEEP_EN, CLK_LUT_CFG);
}

static inline unsigned int read_count(struct systick_device *sdev)
{
	return ioread32(sdev->membase + SYSTICK_COUNT);
}

static inline unsigned int read_compare(struct systick_device *sdev)
{
	return ioread32(sdev->membase + SYSTICK_COMPARE);
}

static inline void write_compare(struct systick_device *sdev, unsigned int val)
{
	iowrite32(val, sdev->membase + SYSTICK_COMPARE);
}

static int systick_next_event(unsigned long delta,
				struct clock_event_device *evt)
{
	struct systick_device *sdev;
	int res;

	sdev = container_of(evt, struct systick_device, dev);
	delta += read_count(sdev);
	write_compare(sdev, delta);
	res = ((int)(read_count(sdev) - delta) >= 0) ? -ETIME : 0;

	return res;
}

static void systick_event_handler(struct clock_event_device *dev)
{
	/* noting to do here */
}

static irqreturn_t systick_interrupt(int irq, void *dev_id)
{
	int ret = 0;
	struct clock_event_device *cdev;
	struct systick_device *sdev;

	if (read_c0_cause() & STATUSF_IP7) {
		cdev = (struct clock_event_device *) dev_id;
		sdev = container_of(cdev, struct systick_device, dev);

		/* Clear Count/Compare Interrupt */
		write_compare(sdev, read_compare(sdev));
		cdev->event_handler(cdev);
		ret = 1;
	}

	return IRQ_RETVAL(ret);
}

static struct systick_device systick = {
	.dev = {
		.features		= CLOCK_EVT_FEAT_ONESHOT,
		.set_next_event		= systick_next_event,
		.set_state_shutdown	= systick_shutdown,
		.set_state_oneshot	= systick_set_oneshot,
		.event_handler		= systick_event_handler,
	},
};

static struct irqaction systick_irqaction = {
	.handler = systick_interrupt,
	.flags = IRQF_PERCPU | IRQF_TIMER,
	.dev_id = &systick.dev,
};

static int systick_shutdown(struct clock_event_device *evt)
{
	struct systick_device *sdev;

	sdev = container_of(evt, struct systick_device, dev);

	if (sdev->irq_requested)
		remove_irq(systick.dev.irq, &systick_irqaction);
	sdev->irq_requested = 0;
	iowrite32(CFG_CNT_EN, systick.membase + SYSTICK_CONFIG);

	if (systick_freq_scaling)
		systick_freq_scaling(sdev, 0);

	if (systick_freq_scaling)
		systick_freq_scaling(sdev, 1);

	return 0;
}

static int systick_set_oneshot(struct clock_event_device *evt)
{
	struct systick_device *sdev;

	sdev = container_of(evt, struct systick_device, dev);

	if (!sdev->irq_requested)
		setup_irq(systick.dev.irq, &systick_irqaction);
	sdev->irq_requested = 1;
	iowrite32(CFG_EXT_STK_EN | CFG_CNT_EN,
		  systick.membase + SYSTICK_CONFIG);

	return 0;
}

static const struct of_device_id systick_match[] = {
	{ .compatible = "ralink,mt7620a-systick", .data = mt7620_freq_scaling},
	{},
};

static int __init ralink_systick_init(struct device_node *np)
{
	const struct of_device_id *match;
	int rating = 200;
	int ret;

	systick.membase = of_iomap(np, 0);
	if (!systick.membase)
		return -ENXIO;

	match = of_match_node(systick_match, np);
	if (match) {
		systick_freq_scaling = match->data;
		/*
		 * cevt-r4k uses 300, make sure systick
		 * gets used if available
		 */
		rating = 310;
	}

	/* enable counter than register clock source */
	iowrite32(CFG_CNT_EN, systick.membase + SYSTICK_CONFIG);
	clocksource_mmio_init(systick.membase + SYSTICK_COUNT, np->name,
			SYSTICK_FREQ, rating, 16, clocksource_mmio_readl_up);

	/* register clock event */
	systick.dev.irq = irq_of_parse_and_map(np, 0);
	if (!systick.dev.irq) {
		pr_err("%s: request_irq failed", np->name);
		return -EINVAL;
	}

	systick_irqaction.name = np->name;
	systick.dev.name = np->name;
	systick.dev.rating = rating;
	systick.dev.cpumask = cpumask_of(0);
	clockevents_config_and_register(&systick.dev, SYSTICK_FREQ, 0x3, 0x7fff);

	pr_info("%s: running - mult: %d, shift: %d\n",
			np->name, systick.dev.mult, systick.dev.shift);

	return 0;
}

CLOCKSOURCE_OF_DECLARE(systick, "ralink,cevt-systick", ralink_systick_init);
