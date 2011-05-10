/*
 *  linux/arch/arm/mach-cns3xxx/cns3xxx.c
 *
 *  Copyright (c) 2008 Cavium Networks 
 *  Copyright (C) 1999 - 2003 ARM Limited
 *  Copyright (C) 2000 Deep Blue Solutions Ltd
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
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/amba/bus.h>
#include <linux/delay.h>
#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/io.h>
#include <linux/ata_platform.h>
#include <linux/serial.h>
#include <linux/tty.h>
#include <linux/serial_8250.h>

#include <asm/clkdev.h>
#include <asm/system.h>
#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/leds.h>
#include <asm/mach-types.h>
#include <asm/hardware/arm_timer.h>
#include <asm/hardware/cache-l2cc.h>
#include <asm/smp_twd.h>
#include <asm/gpio.h>

#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <asm/mach/time.h>

#include <asm/hardware/gic.h>

#include <mach/platform.h>
#include <mach/irqs.h>
#include <mach/pm.h>
#include <asm/dma.h>
#include <mach/dmac.h>

#include "core.h"
#include "rdma.h"

static struct map_desc cns3xxx_io_desc[] __initdata = {
	{
		.virtual	= CNS3XXX_TC11MP_TWD_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TC11MP_TWD_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_TC11MP_GIC_CPU_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TC11MP_GIC_CPU_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TC11MP_GIC_DIST_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_I2S_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_I2S_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_TIMER1_2_3_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TIMER1_2_3_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_TC11MP_L220_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_TC11MP_L220_BASE),
		.length		= SZ_8K,
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
		.virtual	= CNS3XXX_DMC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_DMC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_SMC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_SMC_BASE),
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
		.virtual	= CNS3XXX_RTC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_RTC_BASE),
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
		.virtual	= CNS3XXX_UART0_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_UART0_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_UART1_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_UART1_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_UART2_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_UART2_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_UART3_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_UART3_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_DMAC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_DMAC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_CRYPTO_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_CRYPTO_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_HCIE_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_HCIE_BASE),
		.length		= SZ_32K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_RAID_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_RAID_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_AXI_IXC_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_AXI_IXC_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_CLCD_BASE_VIRT,
		.pfn		= __phys_to_pfn( CNS3XXX_CLCD_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_USBOTG_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_USBOTG_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_USB_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_USB_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_SATA2_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_SATA2_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_CAMERA_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_CAMERA_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_I2S_TDM_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_I2S_TDM_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_2DG_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_2DG_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_USB_OHCI_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_USB_OHCI_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_MEM_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_MEM_BASE),
		.length		= SZ_16M,		// 176MB
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_HOST_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_HOST_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_CFG0_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_CFG0_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_CFG1_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_CFG1_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_MSG_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_MSG_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE0_IO_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE0_IO_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, { 
		.virtual	= CNS3XXX_PCIE1_MEM_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_MEM_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE1_HOST_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_HOST_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE1_CFG0_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_CFG0_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE1_CFG1_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_CFG1_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE1_MSG_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_MSG_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PCIE1_IO_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PCIE1_IO_BASE),
		.length		= SZ_16M,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_L2C_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_L2C_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_PPE_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_PPE_BASE),
		.length		= SZ_4K,
		.type		= MT_DEVICE,
	}, {
		.virtual	= CNS3XXX_EMBEDDED_SRAM_BASE_VIRT,
		.pfn		= __phys_to_pfn(CNS3XXX_EMBEDDED_SRAM_BASE),
		.length		= SZ_8K,
		.type		= MT_DEVICE,
	},
};

void __init cns3xxx_map_io(void)
{
	iotable_init(cns3xxx_io_desc, ARRAY_SIZE(cns3xxx_io_desc));
}

/* used by entry-macro.S */
void __iomem *gic_cpu_base_addr;

void __init cns3xxx_init_irq(void)
{
	/* ARM11 MPCore test chip GIC */
	gic_cpu_base_addr = (void __iomem *) CNS3XXX_TC11MP_GIC_CPU_BASE_VIRT;
	gic_dist_init(0, (void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT, 29);
	gic_cpu_init(0, gic_cpu_base_addr);
	set_interrupt_pri(1, 0);		// Set cache broadcast priority to the highest priority
}

int gpio_to_irq(int gpio)
{
	if (gpio > 63)
		return -EINVAL;
		
	if (gpio < 32)
		return IRQ_CNS3XXX_GPIOA;
	else
		return IRQ_CNS3XXX_GPIOB;
}

int irq2gpio(int irq)
{
	if (irq == IRQ_CNS3XXX_GPIOA)
		return 0;
	else if (irq == IRQ_CNS3XXX_GPIOB)
		return 32;
	else
		return -EINVAL;
}

static inline void gpio_line_config(u8 line, u32 direction)
{
	u32 reg;
	if (direction) {
		if (line < 32) {
			reg = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR);
			reg |= (1 << line);
			__raw_writel(reg, CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR);
		} else {
			reg = __raw_readl(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR);
			reg |= (1 << (line - 32));
			__raw_writel(reg, CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR);		
		}
	} else {
		if (line < 32) {
			reg = __raw_readl(CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR);
			reg &= ~(1 << line);
			__raw_writel(reg, CNS3XXX_GPIOA_BASE_VIRT + CNS3XXX_GPIO_DIR);
		} else {
			reg = __raw_readl(CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR);
			reg &= ~(1 << (line - 32));
			__raw_writel(reg, CNS3XXX_GPIOB_BASE_VIRT + CNS3XXX_GPIO_DIR);		
		}
	}
}

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
	return gpio_get_value(gpio);
}

static void cns3xxx_gpio_set_value(struct gpio_chip *chip, unsigned gpio, int value)
{
	gpio_set_value(gpio, value);
}

static struct gpio_chip cns3xxx_gpio_chip = {
	.label      = "CNS3XXX_GPIO_CHIP",
	.direction_input  = cns3xxx_gpio_direction_input,
	.direction_output = cns3xxx_gpio_direction_output,
	.get      = cns3xxx_gpio_get_value,
	.set      = cns3xxx_gpio_set_value,
	.base     = 0,
	.ngpio      = 64,
};

/* Watchdog */
static struct resource cns3xxx_watchdog_resources[] = {
	{
		.start = CNS3XXX_TC11MP_TWD_BASE,
		.end   = CNS3XXX_TC11MP_TWD_BASE + SZ_4K - 1,
		.flags = IORESOURCE_MEM,
	},{
		.start = IRQ_LOCALWDOG,
		.end   = IRQ_LOCALWDOG,
		.flags = IORESOURCE_IRQ,
	}
};

static struct platform_device cns3xxx_watchdog_device = {
	.name   = "cns3xxx-wdt",
	.id   = -1,
	.num_resources  = ARRAY_SIZE(cns3xxx_watchdog_resources),
	.resource = cns3xxx_watchdog_resources,
};

static struct resource cns3xxx_gpio_resources[] = {
	{
		.name = "gpio",
		.start = 0xFFFFFFFF,
		.end = 0xFFFFFFFF,
		.flags = 0,
	},
};

static struct platform_device cns3xxx_gpio = {
	.name = "GPIODEV",
	.id = -1,
	.num_resources = ARRAY_SIZE(cns3xxx_gpio_resources),
	.resource = cns3xxx_gpio_resources,
};

void __init cns3xxx_sys_init(void)
{
	l2cc_init((void __iomem *) CNS3XXX_L2C_BASE_VIRT);

	dmac_init();
//	cns_rdma_init();

	platform_device_register(&cns3xxx_gpio);
	platform_device_register(&cns3xxx_watchdog_device);
	gpiochip_add(&cns3xxx_gpio_chip);
}

void __iomem *timer1_va_base;

static void timer_set_mode(enum clock_event_mode mode,
			   struct clock_event_device *clk)
{
	unsigned long ctrl = readl(timer1_va_base + TIMER1_2_CONTROL_OFFSET); 
	int reload;
	int pclk = (cns3xxx_cpu_clock() >> 3);

	switch(mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		/* pclk is cpu clock/8 */
		reload=pclk*1000000/HZ;
		writel(reload, timer1_va_base + TIMER1_AUTO_RELOAD_OFFSET);
		ctrl |= (1 << 0) | (1 << 2) | (1 << 9);
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		writel(0, timer1_va_base + TIMER1_AUTO_RELOAD_OFFSET);
		ctrl |= (1 << 2) | (1 << 9);
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		ctrl = 0;
	}

	writel(ctrl, timer1_va_base + TIMER1_2_CONTROL_OFFSET);
}

static int timer_set_next_event(unsigned long evt,
				struct clock_event_device *unused)
{
	unsigned long ctrl = readl(timer1_va_base + TIMER1_2_CONTROL_OFFSET); 

	writel(evt, timer1_va_base + TIMER1_COUNTER_OFFSET);
	writel(ctrl | (1 << 0), timer1_va_base + TIMER1_2_CONTROL_OFFSET);

	return 0;
}

static struct clock_event_device timer1_clockevent =	 {
	.name		= "timer1",
	.shift		= 32,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer_set_mode,
	.set_next_event	= timer_set_next_event,
	.rating		= 300,
	.cpumask	= cpu_all_mask,
};

static void __init cns3xxx_clockevents_init(unsigned int timer_irq)
{
	timer1_clockevent.irq = timer_irq;
	timer1_clockevent.mult =
		div_sc( (cns3xxx_cpu_clock() >> 3)*1000000, NSEC_PER_SEC, timer1_clockevent.shift);
	timer1_clockevent.max_delta_ns =
		clockevent_delta2ns(0xffffffff, &timer1_clockevent);
	timer1_clockevent.min_delta_ns =
		clockevent_delta2ns(0xf, &timer1_clockevent);

	clockevents_register_device(&timer1_clockevent);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t cns3xxx_timer_interrupt(int irq, void *dev_id)
{
	u32 val;
	struct clock_event_device *evt = &timer1_clockevent;
	
	/* Clear the interrupt */
	val = readl(timer1_va_base + TIMER1_2_INTERRUPT_STATUS_OFFSET);
	writel(val & ~(1 << 2), timer1_va_base + TIMER1_2_INTERRUPT_STATUS_OFFSET);
	
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

	val = readl(timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET);
	val &= 0xffff;

	return ((val << 32) | readl(timer1_va_base + TIMER_FREERUN_OFFSET));
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
	writel((1 << 16), timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET);
	
	clocksource_cns3xxx.mult =
		clocksource_khz2mult(100, clocksource_cns3xxx.shift);
	clocksource_register(&clocksource_cns3xxx);
}

/*
 * Set up the clock source and clock events devices
 */
void __init __cns3xxx_timer_init(unsigned int timer_irq)
{
	unsigned long val, irq_mask; 

	/*
	 * Initialise to a known state (all timers off)
	 */
	writel(0, timer1_va_base + TIMER1_2_CONTROL_OFFSET);		/* disable timer1 and timer2 */
	writel(0, timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET);	/* stop free running timer3 */
	writel(0, timer1_va_base + TIMER1_MATCH_V1_OFFSET);
	writel(0, timer1_va_base + TIMER1_MATCH_V2_OFFSET);

	val = (cns3xxx_cpu_clock() >> 3) * 1000000 / HZ;
	writel(val, timer1_va_base + TIMER1_COUNTER_OFFSET);
	
	/* mask irq, non-mask timer1 overflow */
	irq_mask = readl(timer1_va_base + TIMER1_2_INTERRUPT_MASK_OFFSET);
	irq_mask &= ~(1 << 2);
	irq_mask |= 0x03;
	writel(irq_mask, timer1_va_base + TIMER1_2_INTERRUPT_MASK_OFFSET);
	/* down counter */
	val = readl(timer1_va_base + TIMER1_2_CONTROL_OFFSET);
	val |= (1 << 9);
	writel(val, timer1_va_base + TIMER1_2_CONTROL_OFFSET);

	/* 
	 * Make irqs happen for the system timer
	 */
	setup_irq(timer_irq, &cns3xxx_timer_irq);

	cns3xxx_clocksource_init();
	cns3xxx_clockevents_init(timer_irq);
}

void __init cns3xxx_timer_init(void)
{
	timer1_va_base = (void __iomem *) CNS3XXX_TIMER1_2_3_BASE_VIRT;
	twd_base = (void __iomem *) CNS3XXX_TC11MP_TWD_BASE_VIRT;
	__cns3xxx_timer_init(IRQ_CNS3XXX_TIMER0);
}

struct sys_timer cns3xxx_timer = {
	.init		= cns3xxx_timer_init,
};


void cns3xxx_power_off(void)
{
	__u32 clkctrl;

	printk(KERN_INFO "powering system down...\n");

	clkctrl = readl(CNS3XXX_PM_BASE_VIRT + PM_SYS_CLK_CTRL_OFFSET);
	clkctrl &= 0xfffff1ff;
	clkctrl |= (0x5 << 9);		/* Hibernate */
	writel(clkctrl, CNS3XXX_PM_BASE_VIRT + PM_SYS_CLK_CTRL_OFFSET);
}
