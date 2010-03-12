/*
 *  linux/include/asm-arm/arch-epxa10db/time.h
 *
 *  Copyright (C) 2001 Altera Corporation
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
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/leds.h>
#include <asm/arch/hardware.h>
#include <asm/mach/time.h>
#define TIMER_TYPE (volatile unsigned int*)
#include <asm/arch/timer.h>
// #define FIQ_PLUS     1


/*
 * IRQ handler for the timer
 */
static irqreturn_t sl2312_timer_interrupt(int irq, void *dev_id)
{
//        unsigned int led;
	// ...clear the interrupt
/*#ifdef FIQ_PLUS
	*((volatile unsigned int *)FIQ_CLEAR(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_TIMER1_MASK);
#else
	*((volatile unsigned int *)IRQ_CLEAR(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_TIMER2_MASK);
#endif*/

    timer_tick();
    return IRQ_HANDLED;
}

static struct irqaction sl2312_timer_irq = {
	.name		= "SL2312 Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER,
	.handler	= sl2312_timer_interrupt,
};

unsigned long sl2312_gettimeoffset (void)
{
    return 0L;
}
#define REG_TO_AHB_SPEED(reg)		((((reg) >> 15) & 0x7) * 10 + 130)

/*
 * Set up timer interrupt, and return the current time in seconds.
 */
void __init sl2312_time_init(void)
{
	// For clock rate adjusting
	unsigned int tick_rate=0;

#ifdef CONFIG_SL3516_ASIC
//	unsigned int clock_rate_base = 130000000;
	unsigned int reg_v=0;

	//--> Add by jason for clock adjust

	reg_v = __raw_readl(IO_ADDRESS((SL2312_GLOBAL_BASE + GLOBAL_STATUS)));
	tick_rate = REG_TO_AHB_SPEED(reg_v) * 1000000;

//	reg_v = readl(IO_ADDRESS((SL2312_GLOBAL_BASE+GLOBAL_STATUS)));
//	reg_v >>= 15;
//	tick_rate = (clock_rate_base + (reg_v & 0x07)*10000000);

	//  FPGA use AHB bus tick rate

	printk("Bus: %dMHz",tick_rate/1000000);

	tick_rate /= 6;				// APB bus run AHB*(1/6)

	switch((reg_v>>3)&3){
		case 0:	printk("(1/1)\n") ;
					break;
		case 1:	printk("(3/2)\n") ;
					break;
		case 2:	printk("(24/13)\n") ;
					break;
		case 3:	printk("(2/1)\n") ;
					break;
	}
	//<--
#else
	printk("Bus: %dMHz(1/1)\n",CLOCK_TICK_RATE/1000000);		// FPGA use 20MHz
	tick_rate = CLOCK_TICK_RATE;
#endif


	/*
	 * Make irqs happen for the system timer
	 */
	// initialize timer interrupt
	// low active and edge trigger
#ifdef FIQ_PLUS
	*((volatile unsigned int *)FIQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_TIMER1_MASK);
	*((volatile unsigned int *)FIQ_LEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_TIMER1_MASK);
	setup_irq(IRQ_TIMER1, &sl2312_timer_irq);
	/* Start the timer */
	*TIMER_COUNT(IO_ADDRESS(SL2312_TIMER1_BASE))=(unsigned int)(tick_rate/HZ);
	*TIMER_LOAD(IO_ADDRESS(SL2312_TIMER1_BASE))=(unsigned int)(tick_rate/HZ);
	*TIMER_CR(IO_ADDRESS(SL2312_TIMER1_BASE))=(unsigned int)(TIMER_1_CR_ENABLE_MSK|TIMER_1_CR_INT_MSK);
#else
	*((volatile unsigned int *)IRQ_TMODE(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_TIMER2_MASK);
	*((volatile unsigned int *)IRQ_TLEVEL(IO_ADDRESS(SL2312_INTERRUPT_BASE))) |= (unsigned int)(IRQ_TIMER2_MASK);
	setup_irq(IRQ_TIMER2, &sl2312_timer_irq);
	/* Start the timer */
	*TIMER_COUNT(IO_ADDRESS(SL2312_TIMER2_BASE))=(unsigned int)(tick_rate/HZ);
	*TIMER_LOAD(IO_ADDRESS(SL2312_TIMER2_BASE))=(unsigned int)(tick_rate/HZ);
	*TIMER_CR(IO_ADDRESS(SL2312_TIMER1_BASE))=(unsigned int)(TIMER_2_CR_ENABLE_MSK|TIMER_2_CR_INT_MSK);
#endif

}


