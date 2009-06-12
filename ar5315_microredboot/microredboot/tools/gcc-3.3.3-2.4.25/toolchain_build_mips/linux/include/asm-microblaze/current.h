/*
 * include/asm-microblaze/current.h -- Current task
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_CURRENT_H__
#define __MICROBLAZE_CURRENT_H__

#include <asm/macrology.h>

// #define CONFIG_REGISTER_TASK_PTR 1

/* The size allocated for kernel stacks.  This _must_ be a power of two!  */
#define KERNEL_STACK_SIZE	0x2000


#ifdef CONFIG_REGISTER_TASK_PTR
/* Register used to hold the current task pointer while in the kernel.
   Any `call clobbered' register without a special meaning should be OK,
   but check asm/microblaze/kernel/entry.S to be sure.  */
	#define CURRENT_TASK_REGNUM	31
	#define CURRENT_TASK 		macrology_paste (r, CURRENT_TASK_REGNUM)
#else
/* Memory location where current task ptr is stored */
	#define CURRENT_TASK		C_SYMBOL_NAME(current)
#endif

/* The alignment of kernel stacks, with task structures at their base.
   Thus, a pointer for a task's task structure can be derived from its
   kernel stack pointer.  */
#define CURRENT_TASK_ALIGNMENT	KERNEL_STACK_SIZE
#define CURRENT_TASK_MASK	(-CURRENT_TASK_ALIGNMENT)


#ifdef __ASSEMBLY__

#ifdef CONFIG_REGISTER_TASK_PTR							
/* Return the current task pointer into the specified register */
#define RETRIEVE_CURRENT_TASK(reg)					      \
	add	reg, r0, CURRENT_TASK;	
#else
#define RETRIEVE_CURRENT_TASK(reg)					      \
	lwi	reg, r0, CURRENT_TASK;
#endif

#ifdef CONFIG_REGISTER_TASK_PTR							
/* Put a pointer to the current task structure into REG. */
#define GET_CURRENT_TASK(reg)						      \
	andi	reg, r1, CURRENT_TASK_MASK;
#else
/* Morph current stack pointer into task pointer, and store in addr.	      
   clobbers r11 but should only be done within a saved state. */	      
#define GET_CURRENT_TASK(addr)						      \
	andi	r11, r1, CURRENT_TASK_MASK;				      \
	swi	r11, r0, addr;
#endif

#else /* !__ASSEMBLY__ */

/* A pointer to the current task.  */
#ifdef CONFIG_REGISTER_TASK_PTR
register struct task_struct *current 
		__asm__ (macrology_stringify (CURRENT_TASK)); 
#else
extern struct task_struct *current;
#endif

#endif /* __ASSEMBLY__ */


#endif /* _MICROBLAZE_CURRENT_H */
