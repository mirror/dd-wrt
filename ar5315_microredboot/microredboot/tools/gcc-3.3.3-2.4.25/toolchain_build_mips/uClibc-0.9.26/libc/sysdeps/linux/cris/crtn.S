#NO_APP
	.file	"initfini.c"
#APP
	
	.section .init
#NO_APP
	.align 1
	.global _init
	.type	_init, @function
#NO_APP
	movem [$sp+],$r0
	Jump [$sp+]
	.size	_init, .-_init
#APP
	
	.section .fini
#NO_APP
	.align 1
	.global _fini
	.type	_fini, @function
#NO_APP
	movem [$sp+],$r0
	Jump [$sp+]
	.size	_fini, .-_fini
#APP
	
/*@TRAILER_BEGINS*/
