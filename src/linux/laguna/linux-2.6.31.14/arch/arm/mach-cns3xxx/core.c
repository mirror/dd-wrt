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
#include <mach/misc.h>

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

#ifdef CONFIG_SERIAL_8250_CONSOLE
static struct uart_port cns3xxx_serial_ports[] = {
	{
		.membase        = (char*) (CNS3XXX_UART0_BASE_VIRT),
		.mapbase        = (CNS3XXX_UART0_BASE),
		.irq            = IRQ_CNS3XXX_UART0,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.regshift       = 2,
		.uartclk        = 24000000,
		.line           = 0,
		.type           = PORT_16550A,
		.fifosize       = 16
	},
	{
		.membase        = (char*) (CNS3XXX_UART1_BASE_VIRT),
		.mapbase        = (CNS3XXX_UART1_BASE),
		.irq            = IRQ_CNS3XXX_UART1,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.regshift       = 2,
		.uartclk        = 24000000,
		.line           = 1,
		.type           = PORT_16550A,
		.fifosize       = 16
	},
	{
		.membase        = (char*) (CNS3XXX_UART2_BASE_VIRT),
		.mapbase        = (CNS3XXX_UART2_BASE),
		.irq            = IRQ_CNS3XXX_UART2,
		.iotype         = UPIO_MEM,
		.flags          = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST,
		.regshift       = 2,
		.uartclk        = 24000000,
		.line           = 2,
		.type           = PORT_16550A,
		.fifosize       = 16
	},
};
#endif


void __init cns3xxx_map_io(void)
{
	iotable_init(cns3xxx_io_desc, ARRAY_SIZE(cns3xxx_io_desc));

#ifdef CONFIG_SERIAL_8250_CONSOLE
	cns3xxx_pwr_power_up(CNS3XXX_PWR_PLL(PLL_USB));
	early_serial_setup(&cns3xxx_serial_ports[0]);
#if (1 < CONFIG_SERIAL_8250_NR_UARTS)
	HAL_MISC_ENABLE_UART1_PINS();
	cns3xxx_pwr_clk_en(CNS3XXX_PWR_CLK_EN(UART1));
	cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(UART1));
	early_serial_setup(&cns3xxx_serial_ports[1]);
#endif
#if (2 < CONFIG_SERIAL_8250_NR_UARTS)
	HAL_MISC_ENABLE_UART2_PINS();
	cns3xxx_pwr_clk_en(CNS3XXX_PWR_CLK_EN(UART2));
	cns3xxx_pwr_soft_rst(CNS3XXX_PWR_SOFTWARE_RST(UART2));
	early_serial_setup(&cns3xxx_serial_ports[2]);
#endif
#endif
}

/* used by entry-macro.S */
void __iomem *gic_cpu_base_addr;

void __init cns3xxx_init_irq(void)
{
	/* ARM11 MPCore test chip GIC */
	gic_cpu_base_addr = (void __iomem *) CNS3XXX_TC11MP_GIC_CPU_BASE_VIRT;
	gic_dist_init(0, (void __iomem *) CNS3XXX_TC11MP_GIC_DIST_BASE_VIRT, 29);
	gic_cpu_init(0, gic_cpu_base_addr);
//	set_interrupt_pri(1, 0);		// Set cache broadcast priority to the highest priority
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

#ifdef CONFIG_CNS3XXX_DMAC
	dmac_init();
#endif
#ifdef CONFIG_CNS3XXX_RAID
	cns_rdma_init();
#endif

	platform_device_register(&cns3xxx_gpio);
	platform_device_register(&cns3xxx_watchdog_device);
	gpiochip_add(&cns3xxx_gpio_chip);
}


/**********************************************************************
   ____            _                   _____ _                     
  / ___| _   _ ___| |_ ___ _ __ ___   |_   _(_)_ __ ___   ___ _ __ 
  \___ \| | | / __| __/ _ \ '_ ` _ \    | | | | '_ ` _ \ / _ \ '__|
   ___) | |_| \__ \ ||  __/ | | | | |   | | | | | | | | |  __/ |   
  |____/ \__, |___/\__\___|_| |_| |_|   |_| |_|_| |_| |_|\___|_|   
         |___/                         
	 
**********************************************************************/
/*
 * Where is the timer (VA)?
 */
void __iomem *timer1_va_base;
u32 timer1_reload;
u64 timer1_ticks         = 0;

#define KHZ			(1000)
#define MHZ			(1000*1000)
#define CNS3XXX_PCLK		(cns3xxx_cpu_clock() >> 3)
/* CONFIG_HZ = 100 => 1 tick = 10 ms */
#define NR_CYCLES_PER_TICK	((CNS3XXX_PCLK * MHZ) / CONFIG_HZ)

/* Timer 1, 2, and 3 Control Register */
#define TIMER1_ENABLE		(1 << 0)
#define TIMER2_ENABLE		(1 << 3)
#define TIMER3_ENABLE		(1 << 6)
#define TIMER1_USE_1KHZ_SOURCE	(1 << 1)
#define TIMER2_USE_1KHZ_SOURCE	(1 << 4)
#define TIMER3_USE_1KHZ_SOURCE	(1 << 7)
#define TIMER1_INTR_ENABLE	(1 << 2)
#define TIMER2_INTR_ENABLE	(1 << 5)
#define TIMER3_INTR_ENABLE	(1 << 8)
#define TIMER1_DOWN_COUNT	(1 << 9)
#define TIMER2_DOWN_COUNT	(1 << 10)
#define TIMER3_DOWN_COUNT	(1 << 11)

/* Timer 1, 2, and 3 Interrupt Status */
#define TIMER1_OVERFLOW		(1 << 2)
#define TIMER2_OVERFLOW		(1 << 5)
#define TIMER3_OVERFLOW		(1 << 8)

/* Free running Timer */
#define TIMER4_ENABLE		(1 << 17)
#define TIMER4_RESET		(1 << 16)
#define TIMER4_COUNTER_BITS	(0x0000FFFF)

static void timer1_set_mode(enum clock_event_mode mode,
			   struct clock_event_device *clk)
{
	u32 ctrl = readl(timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);

	switch(mode) {
	case CLOCK_EVT_MODE_PERIODIC:
		ctrl |= TIMER1_ENABLE | TIMER1_INTR_ENABLE;
		break;
	case CLOCK_EVT_MODE_ONESHOT:
		/* period set, and timer enabled in 'next_event' hook */
		ctrl |= TIMER1_INTR_ENABLE;
		break;
	case CLOCK_EVT_MODE_UNUSED:
	case CLOCK_EVT_MODE_SHUTDOWN:
	default:
		ctrl &= ~(TIMER1_ENABLE | TIMER3_ENABLE);
	}

	writel(ctrl, timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);
}

static int timer1_set_next_event(unsigned long evt,
				struct clock_event_device *unused)
{
	u32 ctrl = readl(timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);

	writel(evt, timer1_va_base + TIMER1_AUTO_RELOAD_OFFSET);
	writel(ctrl | TIMER1_ENABLE, timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);

	return 0;
}

static struct clock_event_device timer1_ce =	 {
	.name		= "timer1",
	.rating		= 350,
	.shift		= 8,
	.features       = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
	.set_mode	= timer1_set_mode,
	.set_next_event	= timer1_set_next_event,
	.cpumask	= cpu_all_mask,
};

static cycle_t timer1_get_cycles(struct clocksource *cs)
{
	u32 current_counter = readl(timer1_va_base + TIMER1_COUNTER_OFFSET);
	u64 tmp = timer1_ticks * NR_CYCLES_PER_TICK;

	return (cycle_t)(tmp + timer1_reload - current_counter);
}


/* PCLK clock source (default is 75 MHz @ CPU 300 MHz) */
static struct clocksource timer1_cs = {
	.name		= "timer1",
	.rating		= 500,
	.read		= timer1_get_cycles,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 8,
	.flags		= 0
};



static void __init cns3xxx_clockevents_init(unsigned int timer_irq)
{
	timer1_ce.irq = timer_irq;
	timer1_ce.mult =
		div_sc( (CNS3XXX_PCLK * MHZ), NSEC_PER_SEC, timer1_ce.shift);
	timer1_ce.max_delta_ns = clockevent_delta2ns(0xffffffff, &timer1_ce);
	timer1_ce.min_delta_ns = clockevent_delta2ns(0xf       , &timer1_ce);

	clockevents_register_device(&timer1_ce);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t cns3xxx_timer1_interrupt(int irq, void *dev_id)
{
	u32 val;
	struct clock_event_device *evt = dev_id;

	val = readl(timer1_va_base + TIMER1_2_3_INTERRUPT_STATUS_OFFSET);
	if (val & TIMER1_OVERFLOW) {
		timer1_ticks++;
		evt->event_handler(evt);
		/* Clear the interrupt */
		writel(val & ~(TIMER1_OVERFLOW), timer1_va_base + TIMER1_2_3_INTERRUPT_STATUS_OFFSET);
	}
	else {
		printk("%s Unexpected interrupt(status=%08x)....", __func__, val);
		writel(val, timer1_va_base + TIMER1_2_3_INTERRUPT_STATUS_OFFSET);
	}

	return IRQ_HANDLED;
}

static struct irqaction cns3xxx_timer1_irq = {
	.name		= "timer1",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= cns3xxx_timer1_interrupt,
	.dev_id		= &timer1_ce,
};

static cycle_t timer2_get_cycles(struct clocksource *cs)
{
	return (cycle_t)readl(timer1_va_base + TIMER2_COUNTER_OFFSET);
}

/* Configured to use 1K Hz clock source */
static struct clocksource timer2_cs = {
	.name		= "timer2_1khz",
	.rating		= 100,
	.read		= timer2_get_cycles,
	.mask		= CLOCKSOURCE_MASK(32),
	.shift		= 8,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static cycle_t timer4_get_cycles(struct clocksource *cs)
{
	u64 tmp;
	tmp = (readl(timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET) & TIMER4_COUNTER_BITS) << 16;
	tmp |= readl(timer1_va_base + TIMER_FREERUN_OFFSET);

	return (cycle_t)tmp;
}

/* Timer4 is a free run 100K Hz timer. */
static struct clocksource timer4_cs = {
	.name		= "timer4_100khz",
	.rating		= 200,
	.read		= timer4_get_cycles,
	.mask		= CLOCKSOURCE_MASK(48),
	.shift		= 8,
	.flags		= CLOCK_SOURCE_IS_CONTINUOUS,
};

static void __init cns3xxx_clocksource_init(void)
{
	timer4_cs.mult = clocksource_hz2mult(100 * KHZ,            timer4_cs.shift);
	timer2_cs.mult = clocksource_hz2mult(1 * KHZ,              timer2_cs.shift);
	timer1_cs.mult = clocksource_hz2mult((CNS3XXX_PCLK * MHZ), timer1_cs.shift);

	clocksource_register(&timer4_cs);
	clocksource_register(&timer2_cs);
	clocksource_register(&timer1_cs);
}
#include <linux/tick.h>



void cns3xxx_timer1_change_clock(void)
{
	u32 counter=*(volatile unsigned int *) (CNS3XXX_TIMER1_2_3_BASE_VIRT + TIMER1_COUNTER_OFFSET);

	/************ timer1 ************/
#ifdef CONFIG_SILICON
	timer1_reload = NR_CYCLES_PER_TICK;
#else
	timer1_reload = 0x25000;
#endif

	if (timer1_reload < counter) {
		*(volatile unsigned int *) (CNS3XXX_TIMER1_2_3_BASE_VIRT + TIMER1_COUNTER_OFFSET)
			= timer1_reload;
	}

	/* for timer, because CPU clock is changed */
	*(volatile unsigned int *) (CNS3XXX_TIMER1_2_3_BASE_VIRT + TIMER1_AUTO_RELOAD_OFFSET)
			= timer1_reload;

	timer1_cs.mult = clocksource_hz2mult((CNS3XXX_PCLK * MHZ), timer1_cs.shift);
	timer1_cs.mult_orig = timer1_cs.mult;

	timer1_ce.mult =
		div_sc( (CNS3XXX_PCLK * MHZ), NSEC_PER_SEC, timer1_ce.shift);
	timer1_ce.max_delta_ns = clockevent_delta2ns(0xffffffff, &timer1_ce);
	timer1_ce.min_delta_ns = clockevent_delta2ns(0xf       , &timer1_ce);

	timer1_cs.cycle_last = 0;
	timer1_cs.cycle_last = clocksource_read(&timer1_cs);
	timer1_cs.error = 0;
	timer1_cs.xtime_nsec = 0;
	clocksource_calculate_interval(&timer1_cs, NTP_INTERVAL_LENGTH);

	tick_clock_notify();

	//printk("timer1_reload v22: %u", timer1_reload);
}


/*
 * Set up the clock source and clock events devices
 */
void __init cns3xxx_timer_init(unsigned int timer_irq)
{
	u32 val, irq_mask; 

	/*
	 * Initialise to a known state (all timers off)
	 */
	writel(0, timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);		/* Disable timer1, 2 and 3 */
	writel(0, timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET);	/* Stop free running timer4 */


	/************ timer1 ************/
#ifdef CONFIG_SILICON
	timer1_reload = NR_CYCLES_PER_TICK;
#else
	timer1_reload = 0x25000;
#endif
	writel(timer1_reload, timer1_va_base + TIMER1_COUNTER_OFFSET);
	writel(timer1_reload, timer1_va_base + TIMER1_AUTO_RELOAD_OFFSET);

	writel(0xFFFFFFFF, timer1_va_base + TIMER1_MATCH_V1_OFFSET);
	writel(0xFFFFFFFF, timer1_va_base + TIMER1_MATCH_V2_OFFSET);
	/* mask irq, non-mask timer1 overflow */
	irq_mask = readl(timer1_va_base + TIMER1_2_3_INTERRUPT_MASK_OFFSET);
	irq_mask &= ~(1 << 2);
	irq_mask |= 0x03;
	writel(irq_mask, timer1_va_base + TIMER1_2_3_INTERRUPT_MASK_OFFSET);
	/* down counter */
	val = readl(timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);
	val |= TIMER1_DOWN_COUNT;
	writel(val, timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);


	/************ timer2 ************/
	/* Configure timer2 as periodic free-running clocksource, interrupt disabled. */
	writel(0,          timer1_va_base + TIMER2_COUNTER_OFFSET);
	writel(0xFFFFFFFF, timer1_va_base + TIMER2_AUTO_RELOAD_OFFSET);

	writel(0xFFFFFFFF, timer1_va_base + TIMER2_MATCH_V1_OFFSET);
	writel(0xFFFFFFFF, timer1_va_base + TIMER2_MATCH_V2_OFFSET);
	/* mask irq */
	irq_mask = readl(timer1_va_base + TIMER1_2_3_INTERRUPT_MASK_OFFSET);
	irq_mask |= ((1 << 3) | (1 << 4) | (1 << 5));
	writel(irq_mask,   timer1_va_base + TIMER1_2_3_INTERRUPT_MASK_OFFSET);
	/* Enable timer2 /Use 1K Hz clock source / Up count */
	val = readl(timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);
	val |= TIMER2_ENABLE | TIMER2_USE_1KHZ_SOURCE;
	writel(val, timer1_va_base + TIMER1_2_3_CONTROL_OFFSET);


	/************ timer3 ************/
	/* Not enabled */

	/************ timer4 ************/
	writel(TIMER4_RESET,  timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET);
	writel(TIMER4_ENABLE, timer1_va_base + TIMER_FREERUN_CONTROL_OFFSET);


	/* 
	 * Make irqs happen for the system timer
	 */
	/* Clear all interrupts */
	writel(0x000001FF, timer1_va_base + TIMER1_2_3_INTERRUPT_STATUS_OFFSET);
	setup_irq(timer_irq, &cns3xxx_timer1_irq);

	cns3xxx_clocksource_init();
	cns3xxx_clockevents_init(timer_irq);
}

static void __init timer_init(void)
{
	timer1_va_base = (void __iomem *) CNS3XXX_TIMER1_2_3_BASE_VIRT;

#ifdef CONFIG_LOCAL_TIMERS
	twd_base = (void __iomem *) CNS3XXX_TC11MP_TWD_BASE_VIRT;
#endif
	cns3xxx_timer_init(IRQ_CNS3XXX_TIMER0);
}

struct sys_timer cns3xxx_timer = {
	.init		= timer_init,
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
