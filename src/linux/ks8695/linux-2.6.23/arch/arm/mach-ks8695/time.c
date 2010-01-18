/*
 * arch/arm/mach-ks8695/time.c
 *
 * Copyright (C) 2006 Ben Dooks <ben@simtec.co.uk>
 * Copyright (C) 2006 Simtec Electronics
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

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <asm/mach/time.h>

#include <asm/arch/regs-timer.h>
#include <asm/arch/regs-irq.h>
#include <asm/arch/regs-mem.h>
#include <asm/arch/regs-gpio.h>

#include "generic.h"

 
 /*
  * How long is the timer interval?
  */
 #define TIMER_INTERVAL	     (TICKS_PER_uSEC * mSEC_10)
 //#define TIMER_DATA_VALUE     (TIMER_INTERVAL >> 1)
 #define TIMER_DATA_VALUE     249998
 //#define TIMER_PULSE_VALUE    (TIMER_INTERVAL - TIMER_DATA_VALUE)
 #define TIMER_PULSE_VALUE    2
 #define TIMER_VALUE          (TIMER_DATA_VALUE + TIMER_PULSE_VALUE)
 //#define TICKS2USECS(x)	     ((x) / TICKS_PER_uSEC)
 #define TICKS2USECS(x)		 (x / 2)
 //#define TIMER_GET_VALUE(x)   (__raw_readl(x + KS8695_TIMER1) + __raw_readl(x + KS8695_TIMER1_PCOUNT))
 #define TIMER_GET_VALUE(x)	 (__raw_readw(VSOPENRISC_VA_EPLD_TIMER_BASE))
 
  /*
  * Returns number of us since last clock interrupt.  Note that interrupts
   * will have been disabled by do_gettimeoffset()
   */
 static unsigned long ks8695_gettimeoffset(void)
  {
 	unsigned long ticks1, ticks2, status;
  
  	/*
  	 * Get the current number of ticks.  Note that there is a race
 	 * condition between us reading the timer and checking for
 	 * an interrupt.  We get around this by ensuring that the
 	 * counter has not reloaded between our two reads.
  	 */
 	ticks2 = TIMER_GET_VALUE();
  	do {
 		ticks1 = ticks2;
 		status = __raw_readl(KS8695_IRQ_VA + KS8695_INTST);
 		ticks2 = TIMER_GET_VALUE();
 	} while (ticks2 < ticks1);
 
 	/*
 	 * Interrupt pending?  If so, we've reloaded once already.
 	 */
 	if (status & (1 << KS8695_IRQ_TIMER1))
 		ticks2 = TIMER_GET_VALUE();
  
 	/*
 	 * Convert the ticks to usecs
 	 */
 	return TICKS2USECS(ticks2);
  }
  
 

/*
 * IRQ handler for the timer.
 */
static irqreturn_t ks8695_timer_interrupt(int irq, void *dev_id)
{
	write_seqlock(&xtime_lock);
	timer_tick();
	write_sequnlock(&xtime_lock);

	return IRQ_HANDLED;
}

static struct irqaction ks8695_timer_irq = {
	.name		= "ks8695_tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= ks8695_timer_interrupt,
};

static void ks8695_timer_setup(void)
{
	unsigned long pulse = 2;
	unsigned long tmout = (CLOCK_TICK_RATE / HZ) - pulse;
	unsigned long tmcon, val;

	/* disable timer1 */
	tmcon = __raw_readl(KS8695_TMR_VA + KS8695_TMCON);
	__raw_writel(tmcon & ~TMCON_T1EN, KS8695_TMR_VA + KS8695_TMCON);

	__raw_writel(tmout, KS8695_TMR_VA + KS8695_T1TC);
	__raw_writel(pulse, KS8695_TMR_VA + KS8695_T1PD);

	/* enable the timer output on GPIO5 */
	val = __raw_readl(KS8695_GPIO_VA + KS8695_IOPC);
	__raw_writel(val | 0x20000, KS8695_GPIO_VA + KS8695_IOPC);

	/* re-enable timer1 */
	__raw_writel(tmcon | TMCON_T1EN, KS8695_TMR_VA + KS8695_TMCON);
}

static void __init ks8695_timer_init (void)
{
	ks8695_timer_setup();

	/* Enable timer interrupts */
	setup_irq(KS8695_IRQ_TIMER1, &ks8695_timer_irq);
}

struct sys_timer ks8695_timer = {
	.init		= ks8695_timer_init,
	.offset		= ks8695_gettimeoffset,
	.resume		= ks8695_timer_setup,
};
