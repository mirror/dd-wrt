/*
 * include/asm-armnommu/arch-ta7s/time.h
 *
 * This software is provided "AS-IS" and any express or implied 
 * warranties or conditions, including but not limited to any
 * implied warranties of merchantability and fitness for a particular
 * purpose regarding this software. In no event shall WireSpeed
 * be liable for any indirect, consequential, or incidental damages,
 * loss of profits or revenue, loss of use or data, or interruption
 * of business, whether the alleged damages are labeled in contract,
 * tort, or indemnity.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * author(s) :
 * 
 */
#ifndef _ASM_ARCH_TIME_H
#include <linux/autoconf.h>
#include <asm/arch/arch.h>
#include <linux/interrupt.h>

#ifdef CONFIG_USE_A7HAL

#include <asm/arch/a7hal/clock.h>
#include <asm/arch/a7hal/timer.h>

#else

extern unsigned long a7hal_clock_getFreq( int noRAM );
extern void a7hal_timer_start( unsigned short count, int whichTimer, unsigned long control );
void a7hal_timer_clearInt( int whichTimer );
unsigned short a7hal_timer_read( int whichTimer );

#define A7HAL_DIV_16            0x04
#define A7HAL_DIV_256           0x08
#define A7HAL_PERIODIC          0x40

#endif

extern unsigned long ta7_gettimeoffset (void);
extern void ta7_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);
extern int setup_arm_irq(int, struct irqaction *);

/*
 * Set up timer interrupt, and the current time in seconds.
 */
extern __inline__ void setup_timer(void)
{
	if( a7hal_clock_getFreq(0) < 0x100000 )
	{
		a7hal_timer_start( ((a7hal_clock_getFreq(0)/16)/HZ) - 1,
                                   0,
                                   A7HAL_DIV_256 | A7HAL_PERIODIC ) ;
	}
	else
	{
		a7hal_timer_start( ((a7hal_clock_getFreq(0)/16)/HZ) - 1,
                                   0,
                                   A7HAL_DIV_16 | A7HAL_PERIODIC ) ;
	} 

	gettimeoffset = ta7_gettimeoffset;	

	timer_irq.handler = ta7_timer_interrupt;

	setup_arm_irq(IRQ_TIMER_0, &timer_irq);
}

#endif

