	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
.globl _init
	.type	_init, @function
_init:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#APP
	
	
	
	
	.section .fini
#NO_APP
.globl _fini
	.type	_fini, @function
_fini:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	call	__i686.get_pc_thunk.bx
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#APP
	
	
	
	.section	.gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
.globl __i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx, @function
__i686.get_pc_thunk.bx:
	movl	(%esp), %ebx
	ret
	.ident	"GCC: (GNU) 3.3.2 (Debian)"
