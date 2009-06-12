/* Copyright (C) 1996,1997,98,99,2000,02,03 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson (rth@tamu.edu)

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

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.

   Hacked up for uClibc by Erik Andersen <andersen@codepoet.org>
*/

#define _ERRNO_H	1
#include <features.h>
#include <bits/errno.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     pid_t *ptid, struct user_desc *tls, pid_t *ctid); */

#define LINKAGE     4
#define PTR_SIZE    4
#define PARMS       LINKAGE        /* no space for saved regs */
#define FUNC        PARMS
#define STACK       FUNC+4
#define FLAGS       STACK+PTR_SIZE
#define ARG         FLAGS+4
#define PTID        ARG+PTR_SIZE
#define TLS         PTID+PTR_SIZE
#define CTID        TLS+PTR_SIZE

#define __NR_clone        120
#define __NR_exit         1

.text
.type clone,@function;
.weak clone ; clone = __clone

.type __clone,@function;
.globl __clone;

__clone:
	/* Sanity check arguments.  */
	movl	$-EINVAL,%eax

	/* no NULL function pointers */
	movl	FUNC(%esp),%ecx
	jecxz	.Lclone_error

	/* no NULL stack pointers */
	movl	STACK(%esp),%ecx
	jecxz	.Lclone_error

	/* Insert the argument onto the new stack.  Make sure the new
	   thread is started with an alignment of (mod 16).  */
	andl	$0xfffffff0, %ecx
	subl	$24,%ecx
	movl	ARG(%esp),%eax		/* no negative argument counts */
	movl	%eax,12(%ecx)

	/* Save the function pointer as the zeroth argument.
	   It will be popped off in the child in the ebx frobbing below.  */
	movl	FUNC(%esp),%eax
	movl	%eax,8(%ecx)
	/* Don't leak any information.  */
	movl	$0,4(%ecx)
	movl	$0,(%ecx)

	/* Do the system call */
	pushl	%ebx
	pushl	%esi
	pushl	%edi
	movl	TLS+12(%esp),%esi
	movl	PTID+12(%esp),%edx
	movl	FLAGS+12(%esp),%ebx
	movl	CTID+12(%esp),%edi
	movl	$__NR_clone,%eax
	int	$0x80
	popl	%edi
	popl	%esi
	popl	%ebx

	test	%eax,%eax
	jl	.Lclone_error
	jz	.Lthread_start

.Lpseudo_end:
	ret

.Lthread_start:
	subl	%ebp,%ebp	/* terminate the stack frame */
	call	*%ebx
#ifdef __PIC__
	call	.Lhere
.Lhere:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.-.Lhere], %ebx
#endif
	movl	%eax, %ebx
	movl	$__NR_exit, %eax
	int	$0x80

#ifdef __PIC__

.Lthere:
	movl	(%esp), %ebx
	ret

.Lclone_error:
	pushl %ebx
	call .Lthere
	addl $_GLOBAL_OFFSET_TABLE_, %ebx
	xorl %edx, %edx
	subl %eax, %edx
	pushl %edx
	call __errno_location@PLT
	popl %ecx
	popl %ebx
	movl %ecx, (%eax)
	orl $-1, %eax
	jmp .Lpseudo_end

#else	/* __PIC__ */

.Lclone_error:
	negl    %eax
	pushl   %eax
	call	__errno_location
	popl	%ecx
	movl	%ecx, (%eax)
	xorl	%eax, %eax
	decl	%eax

#endif

.size __clone,.-__clone

