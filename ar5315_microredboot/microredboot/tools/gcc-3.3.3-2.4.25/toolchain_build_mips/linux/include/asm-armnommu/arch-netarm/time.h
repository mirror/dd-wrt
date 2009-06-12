/*
 * include/asm-armnommu/arch-netarm/time.h
 *
 * Copyright (C) 2000 NETsilicon, Inc.
 * Copyright (C) 2000 WireSpeed Communications Corporation
 *
 * This software is copyrighted by WireSpeed. LICENSEE agrees that
 * it will not delete this copyright notice, trademarks or protective
 * notices from any copy made by LICENSEE.
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
 * author(s) : Joe deBlaquiere
 * 
 */

#include	<asm/arch/netarm_registers.h>

/* patched into do_gettimeoffset() - which uses microseconds */
/* defined in arch/armnommu/mach-netarm/time.c          --rp */

extern unsigned long netarm_gettimeoffset (void);
extern void netarm_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);

extern int setup_arm_irq(int, struct irqaction *);


/*
 * No need to reset the timer at every irq - timer rolls itself
 */
extern __inline__ int reset_timer(void)
{
#if 0
	setup_timer();
#endif	
	return (1);
}

/*
 * Updating of the RTC.  There is none, so do nothing :o)
 */
#define update_rtc()

/*
 * Set up timer interrupt, and the current time in seconds.
 */

extern void netarm_dump_int_stats(void);

extern __inline__ void setup_timer(void)
{
	volatile unsigned int *timer_control = 
		(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + 
		NETARM_GEN_TIMER2_CONTROL) ;

#if 1
	*timer_control = NETARM_GEN_TIMER_SET_HZ( HZ ) | 
		NETARM_GEN_TCTL_ENABLE | NETARM_GEN_TCTL_INT_ENABLE |
		NETARM_GEN_TCTL_USE_IRQ | NETARM_GEN_TCTL_INIT_COUNT(0) ;
#else
	*timer_control = NETARM_GEN_TIMER_SET_HZ( 4 ) | 
		NETARM_GEN_TCTL_ENABLE | NETARM_GEN_TCTL_INT_ENABLE |
		NETARM_GEN_TCTL_USE_IRQ | NETARM_GEN_TCTL_INIT_COUNT(0) ;
#endif

	printk("setup_timer : T2 CTL = %08lX\n", ((unsigned long)*timer_control));

	/* store in kernel's function pointer --rp */
	gettimeoffset = netarm_gettimeoffset;	
	/* set up the timer interrupt */
	timer_irq.handler = netarm_timer_interrupt;

/*	netarm_dump_int_stats();*/
	printk("setting up timer IRQ\n");
	setup_arm_irq(IRQ_TIMER2, &timer_irq);
/*	netarm_dump_int_stats();*/
}

