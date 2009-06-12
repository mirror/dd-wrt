/*
 * include/asm-microblaze/entry.h -- Definitions used by low-level trap handlers
 *
 *  Copyright (C) 2003       John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001,2002  NEC Corporation
 *  Copyright (C) 2001,2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_ENTRY_H__
#define __MICROBLAZE_ENTRY_H__


#include <asm/ptrace.h>
#include <asm/machdep.h>


/* If true, system calls save and restore all registers (except result
   registers, of course).  If false, then `call clobbered' registers
   will not be preserved, on the theory that system calls are basically
   function calls anyway, and the caller should be able to deal with it.
   This is a security risk, of course, as `internal' values may leak out
   after a system call, but that certainly doesn't matter very much for
   a processor with no MMU protection!  For a protected-mode kernel, it
   would be faster to just zero those registers before returning.  */
#define TRAPS_PRESERVE_CALL_CLOBBERED_REGS	0

/* If TRAPS_PRESERVE_CALL_CLOBBERED_REGS is false, then zero `call
   clobbered' registers before returning from a system call.  */
#define TRAPS_ZERO_CALL_CLOBBERED_REGS		0


/* These are special variables using by the kernel trap/interrupt code
   to save registers in, at a time when there are no spare registers we
   can use to do so, and we can't depend on the value of the stack
   pointer.  This means that they must be within a signed 16-bit
   displacement of 0x00000000.  */

#define KERNEL_VAR_SPACE_ADDR	R0_RAM_ADDR

/* 
 * KERNEL_VAR is from the microblaze port.  To maintain compatability
 * we keep it, but the mb-gas doesn't do the reg[offset] syntax. 
 * instead, we should get away with something like addi reg, r0, offset.
 * Here's hoping.
 */
#ifdef __ASSEMBLY__
/* #define KERNEL_VAR(addr)	addr[r0] */
#define KERNEL_VAR(addr)	addr
#else
#define KERNEL_VAR(addr)	(*(volatile unsigned long *)(addr))
#endif

/* Kernel stack pointer, 4 bytes.  */
#define KSP_ADDR		(KERNEL_VAR_SPACE_ADDR +  0)
#define KSP			KERNEL_VAR (KSP_ADDR)
/* 1 if in kernel-mode, 0 if in user mode, 1 byte.  */
#define KM_ADDR 		(KERNEL_VAR_SPACE_ADDR +  4)
#define KM			KERNEL_VAR (KM_ADDR)
/* Temporary storage for interrupt handlers, 4 bytes.  */
#define INT_SCRATCH_ADDR	(KERNEL_VAR_SPACE_ADDR +  8)
#define INT_SCRATCH		KERNEL_VAR (INT_SCRATCH_ADDR)
/* Where the stack-pointer is saved when jumping to various sorts of
   interrupt handlers.  ENTRY_SP is used by everything except NMIs,
   which have their own location.  Higher-priority NMIs can clobber the
   value written by a lower priority NMI, since they can't be disabled,
   but that's OK, because only NMI0 (the lowest-priority one) is allowed
   to return.  */
#define ENTRY_SP_ADDR		(KERNEL_VAR_SPACE_ADDR + 12)
#define ENTRY_SP		KERNEL_VAR (ENTRY_SP_ADDR)
#define NMI_ENTRY_SP_ADDR	(KERNEL_VAR_SPACE_ADDR + 16)
#define NMI_ENTRY_SP		KERNEL_VAR (NMI_ENTRY_SP_ADDR)

/* Somewhere to save r11 upon entry to a syscall/irq */
#define R11_SAVE_ADDR		(KERNEL_VAR_SPACE_ADDR + 20)
#define R11_SAVE		KERNEL_VAR (R11_SAVE_ADDR)

#ifdef CONFIG_RESET_GUARD
/* Used to detect unexpected resets (since the microblaze has no MMU, any call
   through a null pointer will jump to the reset vector).  We detect
   such resets by checking for a magic value, RESET_GUARD_ACTIVE, in
   this location.  Properly resetting the machine stores zero there, so
   it shouldn't trigger the guard; the power-on value is uncertain, but
   it's unlikely to be RESET_GUARD_ACTIVE.  */
#define RESET_GUARD_ADDR	(KERNEL_VAR_SPACE_ADDR + 28)
#define RESET_GUARD		KERNEL_VAR (RESET_GUARD_ADDR)
#define RESET_GUARD_ACTIVE	0xFAB4BEEF
#endif /* CONFIG_RESET_GUARD */

#ifndef __ASSEMBLY__

#ifdef CONFIG_RESET_GUARD
/* Turn off reset guard, so that resetting the machine works normally.
   This should be called in the various machine_halt, etc., functions.  */
static inline void disable_reset_guard (void)
{
	RESET_GUARD = 0;
}
#endif /* CONFIG_RESET_GUARD */

#endif /* !__ASSEMBLY__ */


/* A `state save frame' is a struct pt_regs preceded by some extra space
   suitable for a function call stack frame.  */

/* Amount of room on the stack reserved for arguments and to satisfy the
   C calling conventions, in addition to the space used by the struct
   pt_regs that actually holds saved values.  */
#define STATE_SAVE_ARG_SPACE	(6*4) /* Up to six arguments.  */


#ifdef __ASSEMBLY__

/* The size of a state save frame.  */
#define STATE_SAVE_SIZE		(PT_SIZE + STATE_SAVE_ARG_SPACE)

#else /* !__ASSEMBLY__ */

/* The size of a state save frame.  */
#define STATE_SAVE_SIZE	       (sizeof (struct pt_regs) + STATE_SAVE_ARG_SPACE)

#endif /* __ASSEMBLY__ */


/* Offset of the struct pt_regs in a state save frame.  */
#define STATE_SAVE_PT_OFFSET	STATE_SAVE_ARG_SPACE


#endif /* __MICROBLAZE_ENTRY_H__ */
