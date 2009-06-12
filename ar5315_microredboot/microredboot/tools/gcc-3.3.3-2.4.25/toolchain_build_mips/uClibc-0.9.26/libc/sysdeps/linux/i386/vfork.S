/*
 * June 3, 2003             Erik Andersen
 *
 */

#define _ERRNO_H	1
#include <bits/errno.h>
#include <sys/syscall.h>

#ifndef __NR_vfork

/* No vfork so use fork instead */
.weak vfork ; vfork = __libc_fork

#else

        .text
	.globl __vfork;
	.type __vfork,@function;
	.align 1<<4;
	
__vfork:
	popl %ecx
	movl $__NR_vfork,%eax
	int $0x80
	pushl %ecx
	cmpl $-4095,%eax
	jae __syscall_error
	ret

__syscall_error:
	negl    %eax
	pushl   %eax
#ifdef __PIC__
	call .Lthere
.Lthere:
	popl	%ebx
	addl	$_GLOBAL_OFFSET_TABLE_+[.- .Lthere  ], %ebx
	call    __errno_location@PLT
#else
	call	__errno_location
#endif
	popl	%ecx
	movl	%ecx, (%eax)
	xorl	%eax, %eax
	decl	%eax

.Lsize:
	.size	 __vfork,.Lsize-__vfork


.weak vfork ; vfork = __vfork
#endif
