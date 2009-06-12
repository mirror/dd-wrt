/*
 * include/asm-v850/current.h -- Current task
 *
 *  Copyright (C) 2001,02  NEC Corporation
 *  Copyright (C) 2001,02  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef __V850_CURRENT_H__
#define __V850_CURRENT_H__

#include <asm/macrology.h>


/* The size allocated for kernel stacks.  This _must_ be a power of two!  */
#define KERNEL_STACK_SIZE	0x2000


/* Register used to hold the current task pointer while in the kernel.
   Any `call clobbered' register without a special meaning should be OK,
   but check asm/v850/kernel/entry.S to be sure.  */
#define CURRENT_TASK_REGNUM	16
#define CURRENT_TASK 		macrology_paste (r, CURRENT_TASK_REGNUM)


/* The alignment of kernel stacks, with task structures at their base.
   Thus, a pointer for a task's task structure can be derived from its
   kernel stack pointer.  */
#define CURRENT_TASK_ALIGNMENT	KERNEL_STACK_SIZE
#define CURRENT_TASK_MASK	(-CURRENT_TASK_ALIGNMENT)


#ifdef __ASSEMBLY__

/* Put a pointer to the current task structure into REG.  Note that this
   definition requires CURRENT_TASK_MASK to be representable as a
   signed 16-bit value.  */
#define GET_CURRENT_TASK(reg)						      \
        /* Use `addi' and then `and' instead of just `andi', because	      \
	   `addi' sign-extends the immediate value, whereas `andi'	      \
	   zero-extends it.  */						      \
	addi	CURRENT_TASK_MASK, r0, reg;				      \
	and	sp, reg

#else /* !__ASSEMBLY__ */

/* A pointer to the current task.  */
register struct task_struct *current					      \
   __asm__ (macrology_stringify (CURRENT_TASK));

#endif /* __ASSEMBLY__ */


#endif /* _V850_CURRENT_H */
