/*
 * include/asm-microblaze/microblaze_intc.h -- Microblaze cpu core interrupt controller (INTC)
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by John Williams <jwilliams@itee.uq.edu.au>
 *
 * The following functions were previously all "extern inline", but that
 * was causing some wierd problems.   
 * The function bodies are now found in /arch/microblaze/microblaze_intc.c
 */

#ifndef __MICROBLAZE_INTC_H__
#define __MICROBLAZE_INTC_H__

#ifndef __ASSEMBLY__

/* Address map for Microblaze interrupt controller */
#define MICROBLAZE_INTC_ISR_ADDR MICROBLAZE_INTC_BASE_ADDR
#define MICROBLAZE_INTC_IPR_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x04)
#define MICROBLAZE_INTC_IER_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x08)
#define MICROBLAZE_INTC_IAR_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x0C)
#define MICROBLAZE_INTC_SIE_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x10)
#define MICROBLAZE_INTC_CIE_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x14)
#define MICROBLAZE_INTC_IVR_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x18)
#define MICROBLAZE_INTC_MER_ADDR (MICROBLAZE_INTC_BASE_ADDR + 0x1C)

/* Various status bits for writing to the intc */
#define MICROBLAZE_INTC_MASTER_ENABLE_MASK      0x1UL
#define MICROBLAZE_INTC_HARDWARE_ENABLE_MASK    0x2UL /* once set cannot be cleared */

/* These should link in to the intc core peripheral setup */
#define CONFIG_MICROBLAZE_INTC_HAS_IVR
#define CONFIG_MICROBLAZE_INTC_HAS_SIE
#define CONFIG_MICROBLAZE_INTC_HAS_CIE

/* 
 * Master enable on interrupt controller 
 */
extern void microblaze_intc_master_enable(void);

/* 
 * Get active IRQ from controller 
 * Returns -1 if no IRQ is active.  This shouldn't happen because 
 * this is only called from handle_irq()
 */
extern int microblaze_intc_get_active_irq(void);

/* Acknowledge irq */
extern void microblaze_intc_ack_irq(unsigned irq);

/* Enable interrupt handling for interrupt IRQ.  */
extern void microblaze_intc_enable_irq (unsigned irq);

/* Disable interrupt handling for interrupt IRQ.  Note that any
   interrupts received while disabled will be delivered once the
   interrupt is enabled again, unless they are explicitly cleared using
   `microblaze_intc_clear_pending_irq'.  */
extern void microblaze_intc_disable_irq (unsigned irq);

/* Disable specified IRQ and acknolwedge (used on IRQ entry) */
extern void microblaze_intc_disable_and_ack_irq(unsigned irq);

/* Enable specified IRQ (if not disabled by kernel) and acknowledge */
extern void microblaze_intc_end(unsigned irq);

/* Return true if interrupt handling for interrupt IRQ is enabled.  */
extern int microblaze_intc_irq_enabled (unsigned irq);

/* Disable irqs from 0 until LIMIT.  */
extern void _microblaze_intc_disable_irqs (unsigned limit);

/* Disable all irqs.  This is purposely a macro, because NUM_MACH_IRQS
   will be only be defined later.  */
#define microblaze_intc_disable_irqs()   _microblaze_intc_disable_irqs (NUM_MACH_IRQS)

/* Clear any pending interrupts for IRQ.  */
extern void microblaze_intc_clear_pending_irq (unsigned irq);

/* Return true if interrupt IRQ is pending (but disabled).  */
extern int microblaze_intc_irq_pending (unsigned irq);

struct microblaze_intc_irq_init {
	const char *name;	/* name of interrupt type */
	unsigned base, num;	/* range of kernel irq numbers for this type */
	unsigned priority;	/* interrupt priority to assign */
};
struct hw_interrupt_type;	/* fwd decl */

/* Initialize HW_IRQ_TYPES for INTC-controlled irqs described in array
   INITS (which is terminated by an entry with the name field == 0).  */
extern void microblaze_intc_init_irq_types (struct microblaze_intc_irq_init *inits,
				       struct hw_interrupt_type *hw_irq_types);


#endif /* !__ASSEMBLY__ */


#endif /* __MICROBLAZE_INTC_H__ */
