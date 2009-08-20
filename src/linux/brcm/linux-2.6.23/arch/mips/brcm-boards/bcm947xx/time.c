/*
 * Copyright (C) 2008, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: time.c,v 1.8 2008/07/04 01:06:30 Exp $
 */
#include <linux/config.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/serial_reg.h>
#include <linux/interrupt.h>
#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/time.h>

#include <typedefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <siutils.h>
#include <hndmips.h>
#include <mipsinc.h>
#include <hndcpu.h>
#include <bcmdevs.h>

/* Global SB handle */
extern void *bcm947xx_sih;
extern spinlock_t bcm947xx_sih_lock;

/* Convenience */
#define sih bcm947xx_sih
#define sih_lock bcm947xx_sih_lock

extern int panic_timeout;
int watchdog_timer = 0;
#ifndef	CONFIG_HWSIM
static u8 *mcr = NULL;
#endif /* CONFIG_HWSIM */

void __init
bcm947xx_time_init(void)
{
	unsigned int hz;

	/*
	 * Use deterministic values for initial counter interrupt
	 * so that calibrate delay avoids encountering a counter wrap.
	 */
	write_c0_count(0);
	write_c0_compare(0xffff);

	if (!(hz = si_cpu_clock(sih)))
		hz = 100000000;

	printk("CPU: BCM%04x rev %d at %d MHz\n", ((si_t *)sih)->chip, ((si_t *)sih)->chiprev,
	       (hz + 500000) / 1000000);

	/* Set MIPS counter frequency for fixed_rate_gettimeoffset() */
	if (((si_t *)sih)->chip == BCM5354_CHIP_ID && nvram_match ("Fix_WL520GUGC_clock", "1") )
		mips_hpt_frequency = 100000000;
	else
		mips_hpt_frequency = hz / 2;

	/* Set watchdog interval in ms */
	watchdog_timer = -1;//simple_strtoul(nvram_safe_get("watchdog"), NULL, 0);

	/* Please set the watchdog to 3 sec if it is less than 3 but not equal to 0 */
//	if (watchdog > 0) {
//		if (watchdog < 3000)
//			watchdog = 3000;
//	}

	/* Set panic timeout in seconds */
	panic_timeout = watchdog_timer / 1000;
}

#ifdef CONFIG_HND_BMIPS3300_PROF
extern bool hndprofiling;
#ifdef CONFIG_MIPS64
typedef u_int64_t sbprof_pc;
#else
typedef u_int32_t sbprof_pc;
#endif
extern void sbprof_cpu_intr(sbprof_pc restartpc);
#endif	/* CONFIG_HND_BMIPS3300_PROF */

static irqreturn_t
bcm947xx_timer_interrupt(int irq, void *dev_id)
{
#ifdef CONFIG_HND_BMIPS3300_PROF
	/*
	 * Are there any ExcCode or other mean(s) to determine what has caused
	 * the timer interrupt? For now simply stop the normal timer proc if
	 * count register is less than compare register.
	 */
	if (hndprofiling) {
		sbprof_cpu_intr(read_c0_epc() +
		                ((read_c0_cause() >> (CAUSEB_BD - 2)) & 4));
		if (read_c0_count() < read_c0_compare())
			return (IRQ_HANDLED);
	}
#endif	/* CONFIG_HND_BMIPS3300_PROF */

	/* Generic MIPS timer code */
	timer_interrupt(irq, dev_id);

	/* Set the watchdog timer to reset after the specified number of ms */
	if (watchdog_timer > 0) {
		watchdog_timer--;
		if (((si_t *)sih)->chip == BCM5354_CHIP_ID)
			si_watchdog(sih, WATCHDOG_CLOCK_5354 / 1000 * 5000);
		else
			si_watchdog(sih, WATCHDOG_CLOCK / 1000 * 5000);
	}

#ifdef	CONFIG_HWSIM
	(*((int *)0xa0000f1c))++;
#else
	/* Blink one of the LEDs in the external UART */
	if (mcr && !(jiffies % (HZ/2)))
		writeb(readb(mcr) ^ UART_MCR_OUT2, mcr);
#endif

	return (IRQ_HANDLED);
}

static struct irqaction bcm947xx_timer_irqaction = {
	bcm947xx_timer_interrupt,
	IRQF_DISABLED,
	{ { 0 } },
	"timer",
	NULL,
	NULL,
	0,
	NULL
};

void __init
plat_timer_setup(struct irqaction *irq)
{
	/* Enable the timer interrupt */
	setup_irq(7, &bcm947xx_timer_irqaction);
}
