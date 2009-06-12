/****************************************************************************
*
*	Name:			time.h
*
*	Description:	
*
*	Copyright:		(c) 2002 Conexant Systems Inc.
*
*****************************************************************************

  This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the Free
Software Foundation; either version 2 of the License, or (at your option)
any later version.

  This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
more details.

  You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc., 59
Temple Place, Suite 330, Boston, MA 02111-1307 USA

****************************************************************************
*  $Author: gerg $
*  $Revision: 1.1 $
*  $Modtime: 2/28/02 9:56a $
****************************************************************************/

#ifndef __ASM_ARCH_TIME_H__
#define __ASM_ARCH_TIME_H__

#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/hardware.h>
#include <asm/arch/timex.h>

extern struct irqaction timer_irq;

extern unsigned long cnxt_gettimeoffset(void);
extern void cnxt_timer_interrupt(int irq, void *dev_id, struct pt_regs *regs);


void __inline__ setup_timer (void)
{

	/* disable timer interrupt (should already been disabled) */
	*CNXT_INT_MASK &= ~(1 << CNXT_INT_LVL_TIMER_1);

	/* Timeout 10ms */
	TIM_SET_RATE(TM_Lmt1,CLOCK_TICK_RATE);	

	//gettimeoffset = cnxt_gettimeoffset;	
	timer_irq.handler = cnxt_timer_interrupt;
	setup_arm_irq(IRQ_TIMER, &timer_irq);

}

#endif /* __ASM_ARCH_TIME_H__ */
