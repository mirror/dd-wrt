/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */


/*  Based on the code from GNU libc, but hacked up by John Beppu and Erik Andersen */
/*  adapted by PaX Team for ET_DYN/PIE binaries */

/*
    When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL
*/

#include <features.h>

.text
	.align 4

	.global	_start
	.type	_start,%function
#if defined L_crt0 || defined L_Scrt0 || ! defined __UCLIBC_CTOR_DTOR__
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
	/* locate the start of the environment variables */
	popl %ecx	/* Store argc into %ecx */
	movl %esp,%ebx  /* Store argv into ebx */
	movl %esp,%eax  /* Store argv into eax as well*/
	movl %ecx,%edx	/* Stick argc into %edx so we can do some math in a sec */
	leal 4(%eax,%edx,4),%eax

	/*  [ register layout ]

	    sizeof(char*) == 4
	    %ecx = argc	    ; 0(esp)
	    %ebx = argv	    ; 4(esp)
	    %eax = env	    ; argv + (argc * 4) + 4
	*/

	/* Set up an invalid (NULL return address, NULL frame pointer)
	   callers stack frame so anybody unrolling the stack knows where
	   to stop */
	xorl %ebp,%ebp  /* NULL */
	pushl %ebp      /* callers %cs */
	pushl %ebp      /* callers %eip (return address) */
	pushl %ebp      /* callers %ebp (frame pointer) */
	movl %esp,%ebp  /* mark callers stack frame as invalid */

#if defined L_Scrt0 || defined L_Scrt1
	call .L0
.L0:
	pop %edx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L0],%edx
#endif

#if (defined L_crt1 || defined L_Scrt1 || defined L_gcrt1 ) && defined __UCLIBC_CTOR_DTOR__
	/* Push .init and .fini arguments to __uClibc_start_main() on the stack */
#ifdef L_Scrt1
	pushl _fini@GOT(%edx)
	pushl _init@GOT(%edx)
#else
	pushl $_fini
	pushl $_init
#endif

	/* Push envp, argc, and argc arguments to __uClibc_start_main() on the stack */ 
	pushl %eax	/* Environment pointer */
	pushl %ebx	/* Argument pointer */
	pushl %ecx	/* And the argument count */

	/* Ok, now run uClibc's main() -- shouldn't return */
#ifdef L_Scrt1
	call *__uClibc_start_main@GOT(%edx)
#else
	call __uClibc_start_main
#endif
#else
	/* Push envp, argc, and argc arguments to __uClibc_start_main() on the stack */ 
	pushl %eax	/* Environment pointer */
	pushl %ebx	/* Argument pointer */
	pushl %ecx	/* And the argument count */

#ifdef L_Scrt0
	call *__uClibc_main@GOT(%edx)
#else
	call __uClibc_main
#endif
#endif

	/* Crash if somehow `exit' returns anyways.  */
	hlt
.size _start,.-_start


#if defined L_gcrt1 && defined __UCLIBC_PROFILING__
# include "./gmon-start.S"
#endif

