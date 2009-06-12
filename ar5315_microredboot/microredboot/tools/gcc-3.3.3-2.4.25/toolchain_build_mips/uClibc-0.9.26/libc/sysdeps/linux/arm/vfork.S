/* vi: set sw=4 ts=4: */
/* vfork for uClibc
 *
 * Copyright (C) 2000 by Lineo, inc. and Erik Andersen
 * Copyright (C) 2000,2001 by Erik Andersen <andersen@uclibc.org>
 * Written by Erik Andersen <andersen@uclibc.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include <asm/errno.h>
#include <sys/syscall.h>



		.text
		.global vfork;
		.type vfork,%function
		.align 4;                                                               \



vfork:

#ifdef __NR_vfork
		swi		__NR_vfork
		cmn		r0, #4096
		movcc	pc, lr

		/* Check if vfork even exists.  */
		ldr     r1, =-ENOSYS
		teq     r0, r1
		bne     __syscall_error
#endif

		/* If we don't have vfork, use fork.  */
		swi     __NR_fork
		cmn     r0, #4096

		/* Syscal worked.  Return to child/parent */
		movcc   pc, lr

__syscall_error:

		/* Looks like the syscall choked -- set errno */
		ldr  r3, .L4
		/* Calculate the - of the syscall result, in case we need it */
		rsb  r2, r0, $0

		/* errno = -result */
		str  r2, [r9,r3]

		/* return -1 */
		mvn  r0, $0
		mov  pc, lr

.L4:  .word errno

