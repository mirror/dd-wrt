/* Startup code for SH & ELF.
   Copyright (C) 1999 Free Software Foundation, Inc.
   Copyright (C) 2001 Hewlett-Packard Australia
   Copyright (C) 2002 Stefan Allius

 This program is free software; you can redistribute it and/or modify it under
 the terms of the GNU Library General Public License as published by the Free
 Software Foundation; either version 2 of the License, or (at your option) any
 later version.

 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 details.

 You should have received a copy of the GNU Library General Public License
 along with this program; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/


/* This is the canonical entry point, usually the first thing in the text
   segment.

	At this entry point, most registers' values are unspecified, except:

   sp		The stack contains the arguments and environment:
   		0(sp)			argc
		4(sp)			argv[0]
		...
		(4*argc)(sp)		NULL
		(4*(argc+1))(sp)	envp[0]
		...
					NULL
*/

#include <features.h>

	.file "crt0.S"
	.text
	.globl _start
	.type _start,@function
	.type main,@function
	.size _start,_start_end - _start
_start:
	/* Clear the frame pointer since this is the outermost frame. */
	mov #0, r14

	/* Pop argc off the stack and save a pointer to argv */
	mov.l @r15+,r4
	mov r15, r5

#if defined L_crt0 || ! defined __UCLIBC_CTOR_DTOR__
	/*
	 * Setup the value for the environment pointer:
	 * r6 = (argc + 1) * 4
	 * r6 += argv (in delay slot)
	 */
	mov r4,r6
	add #1,r6
	shll2 r6

	/* jump to __uClibc_main (argc, argv, envp) */
	mov.l L_main, r0
	jsr @r0
	add r5, r6 /* delay slot */

	/* We should not get here. */
	mov.l L_abort, r0
	jmp @r0
	nop

_start_end:	
	.align	2

L_main:
	.long   __uClibc_main /* in libuClibc.*.so */

#else /* (L_crt1 || L_gcrt1) && __UCLIBC_CTOR_DTOR__ */
	/* Push the finip argument to __uClibc_start_main() onto the stack */
	mov.l L_fini,r6
	mov.l r6,@-r15

	/* Setup the value for the initp argument */
	mov.l L_init, r7

	/*
	 * Setup the value for the environment pointer:
	 * r6 = (argc + 1) * 4
	 * r6 += argv (in delay slot)
	 */
	mov r4,r6
	add #1,r6
	shll2 r6

	/* jump to __uClibc_start_main (argc, argv, envp, app_init, app_fini) */
	mov.l L_main, r0
	jsr @r0
	add r5, r6  /* delay slot */
	/* We should not get here. */
	mov.l L_abort, r0
	jmp @r0
	nop
_start_end:	
	.align	2

L_main:
	.long   __uClibc_start_main /* in libuClibc.*.so */

	.weak   _init
	.type   _init,@function
_init:
	rts
	nop

	.size   _init,.-_init
	.weak   _fini
	.set    _fini,_init

L_init:
      .long   _init
L_fini:
      .long   _fini

#endif

L_abort: 
      .long   abort   

#if defined L_gcrt1 && defined __UCLIBC_PROFILING__
# include "./gmon-start.S"
#endif
