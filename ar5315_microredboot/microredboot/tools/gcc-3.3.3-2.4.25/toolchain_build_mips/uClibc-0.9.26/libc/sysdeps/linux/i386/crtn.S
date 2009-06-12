	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
.globl _init
	.type	_init, @function
#NO_APP
	popl	%ebx
	popl	%ebp
	ret
	.size	_init, .-_init
#APP
	
	.section .fini
#NO_APP
.globl _fini
	.type	_fini, @function
#NO_APP
	popl	%ebx
	popl	%ebp
	ret
	.size	_fini, .-_fini
#APP
	
	.section	.gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
.globl __i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.ident	"GCC: (GNU) 3.3.2 (Debian)"
