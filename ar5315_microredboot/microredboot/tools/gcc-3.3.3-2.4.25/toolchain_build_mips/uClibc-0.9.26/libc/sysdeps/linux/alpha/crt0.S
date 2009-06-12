/* Startup code for Alpha/ELF.
   Copyright (C) 1993,1995,1996,1997,1998,2000,2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#define a0      $16     /* argument registers (caller-saved) */
#define a1      $17
#define a2      $18
#define a3      $19
#define a4      $20
#define a5      $21
#define ra      $26     /* return address register */

#define s6      $15
#define fp      s6      /* frame-pointer (s6 in frame-less procedures) */
#define gp      $29     /* global pointer */
#define sp      $30     /* stack pointer */
#define zero    $31     /* reads as zero, writes are noops */

	.text
	.align 3
	.globl _start
	.ent _start, 0
	.type _start,@function
#if defined L_crt0 || ! defined __UCLIBC_CTOR_DTOR__
	.type	__uClibc_main,@function
#else
	.weak	_init
	.weak	_fini
	.type	__uClibc_start_main,@function
#endif
/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
	.type	main,@function

_start:
	.frame fp, 0, zero
	mov	zero, fp
	br	gp, 1f
1:	ldgp	gp, 0(gp)
	subq	sp, 16, sp
	.prologue 0

	ldl	a0, 16(sp)	/* get argc */
	lda	a1, 24(sp)	/* get argv */
	addq    a0, 1, a2
	s8addq  a2, a1, a2      /* Calculate environ */

  /* Call the user's main function, and exit with its value.
     But let the libc call main.  */
	jsr	ra, __uClibc_main

  /* Die very horribly if exit returns.  Call_pal hlt is callable from
     kernel mode only; this will result in an illegal instruction trap.  */
	call_pal 0
	.end _start

/* For ECOFF backwards compatibility. */
.weak    __start;
	__start    =   _start

/* Define a symbol for the first piece of initialized data.  */
	.data
	.globl __data_start
__data_start:
	.long 0
	.weak data_start
	data_start = __data_start
