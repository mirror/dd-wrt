/*
 * Copyright (C) 2003 by Erik Andersen
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

/* Integer registers.  */
#define r0      0
#define r1      1
#define r2      2
#define r3      3
#define r4      4
#define r5      5
#define r6      6
#define r7      7
#define r8      8
#define r9      9
#define r13     13
#define r31     31

#include <features.h>

	.section ".text"
	.globl	_start
	.type	_start,@function
#if defined L_crt0 || ! defined __UCLIBC_CTOR_DTOR__
	.type	__uClibc_main,%function
#else
	.weak	_init
	.weak	_fini
	.type	__uClibc_start_main,%function
#endif
/* Stick in a dummy reference to main(), so that if an application
 * is linking when the main() function is in a static library (.a)
 * we can be sure that main() actually gets linked in */
	.type	main,%function

_start:
	/* Save the stack pointer, in case we're statically linked under Linux.  */
	mr	r9,r1
	/* Set up an initial stack frame, and clear the LR.  */
	clrrwi	r1,r1,4
	li	r0,0
	stwu	r1,-16(r1)
	mtlr	r0
	stw	r0,0(r1)

	/* find argc from the stack pointer */
	lwz	r3,0(r9)
	/* find argv one word offset from the stack pointer */
	addi	r4,r9,4
	/* find environment pointer (argv+argc+1) */
	lwz	r5,0(r9)
	addi	r5,r5,1
	rlwinm	r5,r5,2,0,29
	add	r5,r5,r4

	/* Ok, now run uClibc's main() -- shouldn't return */
#if (defined L_crt1 || defined L_gcrt1 ) && defined __UCLIBC_CTOR_DTOR__
	lis     r6,_init@ha	# load top 16 bits
	addi    r6,r6,_init@l	# load bottom 16 bits
	lis     r7,_fini@ha	# load top 16 bits of &msg
	addi    r7,r7,_fini@l	# load bottom 16 bits
	bl	__uClibc_start_main
#else
	bl	__uClibc_main
#endif
.size _start,.-_start

	.section ".data"
	.globl  __data_start
__data_start:

#if defined L_gcrt1 && defined __UCLIBC_PROFILING__
# include "./gmon-start.S"
#endif


