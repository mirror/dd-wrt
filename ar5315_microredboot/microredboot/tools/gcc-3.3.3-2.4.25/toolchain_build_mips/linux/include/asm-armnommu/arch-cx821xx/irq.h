/****************************************************************************
*
*	Name:			irq.h
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
*  $Modtime: 4/08/02 1:37p $
****************************************************************************/


#ifndef __ASM_ARCH_IRQ_H__
#define __ASM_ARCH_IRQ_H__


#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/mach/irq.h>
#include <asm/arch/irqs.h>


#define fixup_irq(x) (x)

__inline__ void cnxt_mask_irq(unsigned int irq);
__inline__ void cnxt_unmask_irq(unsigned int irq);
__inline__ void cnxt_mask_ack_irq(unsigned int irq);

extern struct irqdesc irq_desc[];

static __inline__ void irq_init_irq(void)
{
	int irq;

	/* disable all int sources */
	*CNXT_INT_MASK = 0x00000000;

	for (irq = 0; irq < NR_IRQS; irq++) {
		irq_desc[irq].valid	= 1;
		irq_desc[irq].probe_ok	= 1;
		irq_desc[irq].mask_ack	= cnxt_mask_ack_irq;
		irq_desc[irq].mask	= cnxt_mask_irq;
		irq_desc[irq].unmask	= cnxt_unmask_irq;
	}
}

#endif /* __ASM_ARCH_IRQ_H__ */
