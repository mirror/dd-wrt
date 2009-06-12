/*
 * June 27, 2001             Manuel Novoa III
 *
 * This is a heavily modified version of gcc's output for the _syscall5 macro.
 * The idea (originally from dietlibc) is that all syscall functions simply set
 * the syscall number as the first argument, then set the syscall arguments as
 * the next up-to-five arguments, and then jump here.  All the common work is
 * done by syscall(), saving a fair amount of generated code when a number of
 * syscalls are used.  The (potential) cost is some unnecessary pushes, pops,
 * and movs but the execution time penalty should be relatively small compared
 * to the cost of the syscall itself.
 *
 * July 24, 2002 
 *
 * Modified by Erik Andersen to take all function parameters from off the stack
 * like a proper function and eliminates the old 255 syscall number limit.  So
 * now we can just call this as a function as syscall() per the function
 * prototype in unistd.h, so to call _exit(42) you can just call.
 *         syscall(__NR_exit, 42);
 * and things will just work.
 */
	
.text
	.align 4
.globl syscall
	.type	 syscall,@function
syscall:
	pushl %edi
	pushl %esi
	pushl %ebx
	movl  36(%esp),%edi;	/* Load the 5 syscall argument registers */
	movl  32(%esp),%esi;
	movl  28(%esp),%edx;
	movl  24(%esp),%ecx;
	movl  20(%esp),%ebx;
	movl  16(%esp),%eax	/* Load syscall number into %eax.  */
#APP
	int $0x80
#NO_APP
	cmpl $-4095,%eax
	jbe .Ldone

#ifdef __PIC__
	call Lhere
Lhere:
	popl %ebx
	addl $_GLOBAL_OFFSET_TABLE_+[.-Lhere],%ebx
	negl %eax
	movl %eax,%ecx
#ifdef __UCLIBC_HAS_THREADS__
	call __errno_location@PLT
#else
	movl errno@GOT(%ebx),%eax
#endif /* __UCLIBC_HAS_THREADS__ */
	movl %ecx,(%eax)
#else
	negl %eax
#ifdef __UCLIBC_HAS_THREADS__
	movl %eax,%ecx
	call __errno_location
	movl %ecx,(%eax)
#else
	movl %eax,errno
#endif /* __UCLIBC_HAS_THREADS__ */

#endif /* __PIC__ */

	movl $-1,%eax
	.p2align 4,,7
.Ldone:
	popl %ebx
	popl %esi
	popl %edi
	ret
.Lsize:
	.size	 syscall,.Lsize-syscall
