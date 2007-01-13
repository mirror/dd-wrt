/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2003 Atheros Communications, Inc.,  All Rights Reserved.
 * Copyright (C) 2006 FON Technology, SL.
 * Copyright (C) 2006 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2006 Felix Fietkau <nbd@openwrt.org>
 */

/*
 * Initialization for ar531x SOC.
 */

#include <linux/autoconf.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/pm.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/types.h>
#include <linux/string.h>

#include <asm/reboot.h>
#include <asm/io.h>
#include <asm/time.h>
#include <asm/pgtable.h>
#include <asm/processor.h>
#include <asm/reboot.h>
#include <asm/system.h>
#include <asm/serial.h>

#include "ar531xlnx.h"

void
ar531x_restart(char *command)
{
	 for(;;) {
		 /* 
		 ** Cold reset does not work,work around is to use the GPIO reset bit.  
		 */
		 unsigned int reg;

		 /* AR2317 reset */
		 sysRegWrite(AR5315_COLD_RESET,AR5317_RESET_SYSTEM);

		 reg = sysRegRead(AR5315_GPIO_DO);
		 reg &= ~(1 << AR5315_RESET_GPIO);
		 sysRegWrite(AR5315_GPIO_DO, reg);
		 (void)sysRegRead(AR5315_GPIO_DO); /* flush write to hardware */
	 }
}

void
ar531x_halt(void)
{
	 printk(KERN_NOTICE "\n** You can safely turn off the power\n");
	 while (1);
}

void
ar531x_power_off(void)
{
	 ar531x_halt();
}

char *get_system_type(void)
{
	return "Atheros AR5315";
}

/*
 * This table is indexed by bits 5..4 of the CLOCKCTL1 register
 * to determine the predevisor value.
 */
static int __initdata CLOCKCTL1_PREDIVIDE_TABLE[4] = {
    1,
    2,
    4,
    5
};

static int __initdata PLLC_DIVIDE_TABLE[5] = {
    2,
    3,
    4,
    6,
    3
};

static unsigned int __init
ar531x_sys_clk(unsigned int clockCtl)
{
    unsigned int pllcCtrl,cpuDiv;
    unsigned int pllcOut,refdiv,fdiv,divby2;
	unsigned int clkDiv;

    pllcCtrl = sysRegRead(AR5315_PLLC_CTL);
    refdiv = (pllcCtrl & PLLC_REF_DIV_M) >> PLLC_REF_DIV_S;
    refdiv = CLOCKCTL1_PREDIVIDE_TABLE[refdiv];
    fdiv = (pllcCtrl & PLLC_FDBACK_DIV_M) >> PLLC_FDBACK_DIV_S;
    divby2 = (pllcCtrl & PLLC_ADD_FDBACK_DIV_M) >> PLLC_ADD_FDBACK_DIV_S;
    divby2 += 1;
    pllcOut = (40000000/refdiv)*(2*divby2)*fdiv;


    /* clkm input selected */
	switch(clockCtl & CPUCLK_CLK_SEL_M) {
		case 0:
		case 1:
			clkDiv = PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKM_DIV_M) >> PLLC_CLKM_DIV_S];
			break;
		case 2:
			clkDiv = PLLC_DIVIDE_TABLE[(pllcCtrl & PLLC_CLKC_DIV_M) >> PLLC_CLKC_DIV_S];
			break;
		default:
			pllcOut = 40000000;
			clkDiv = 1;
			break;
	}
	cpuDiv = (clockCtl & CPUCLK_CLK_DIV_M) >> CPUCLK_CLK_DIV_S;  
	cpuDiv = cpuDiv * 2 ?: 1;
	return (pllcOut/(clkDiv * cpuDiv));
}
		
static inline unsigned int ar531x_cpu_frequency(void)
{
    return ar531x_sys_clk(sysRegRead(AR5315_CPUCLK));
}

static inline unsigned int ar531x_apb_frequency(void)
{
    return ar531x_sys_clk(sysRegRead(AR5315_AMBACLK));
}


void __init serial_setup(void)
{
	struct uart_port s;
	
	memset(&s, 0, sizeof(s));

	s.flags = UPF_BOOT_AUTOCONF | UPF_SKIP_TEST;
	s.iotype = UPIO_MEM;
	s.uartclk = AR5315_UART_CLOCK_RATE;
	s.irq = AR531X_MISC_IRQ_UART0;
	s.regshift = 2;
	s.mapbase = KSEG1ADDR(AR5315_UART0);
	s.membase = (void __iomem *)s.mapbase;

	early_serial_setup(&s);
}

void __init plat_timer_setup(struct irqaction *irq)
{
	unsigned int count;

	/* Usually irq is timer_irqaction (timer_interrupt) */
	setup_irq(AR531X_IRQ_CPU_CLOCK, irq);

	/* to generate the first CPU timer interrupt */
	count = read_c0_count();
	write_c0_compare(count + 1000);
}

static void __init
ar531x_time_init(void)
{
	mips_hpt_frequency = ar531x_cpu_frequency() / 2;
}

void __init plat_mem_setup(void)
{
	unsigned int config = read_c0_config();

	/* Clear any lingering AHB errors */
	write_c0_config(config & ~0x3);
	sysRegWrite(AR5315_AHB_ERR0,AHB_ERROR_DET);
	sysRegRead(AR5315_AHB_ERR1);
	sysRegWrite(AR5315_WDC, WDC_IGNORE_EXPIRATION);

	/* Disable data watchpoints */
	write_c0_watchlo0(0);

	board_time_init = ar531x_time_init;

	_machine_restart = ar531x_restart;
	_machine_halt = ar531x_halt;
	pm_power_off = ar531x_power_off;

	serial_setup();
}

EXPORT_SYMBOL(get_system_type);
