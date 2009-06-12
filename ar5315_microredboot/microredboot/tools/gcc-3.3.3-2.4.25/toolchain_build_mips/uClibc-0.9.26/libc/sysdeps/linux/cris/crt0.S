;; Startup code compiant to the ELF CRIS ABI.
;;
;; Highly based on code from glibc.

#include <sysdep.h>

	.syntax no_register_prefix

	.text
	.globl	_start
	.type	_start,@function

#if defined L_crt0 || ! defined __UCLIBC_CTOR_DTOR__
	.type	__uClibc_main,@function
#else
	.weak	_init
	.weak	_fini
	.type	__uClibc_start_main,@function
#endif

	;; Setup a dummy reference to main so that if an application is linking
	;; when the main() function is in a static library we can be sure that
	;; the main() actually gets linked in.
	.type main,@function

_start:
	;; Clear the frame pointer, to mark the outermost frame.
	moveq	0, r8

	move.d	[sp],r11
	move.d	sp,[sp]
	move.d	sp,r12
	addq	4,r12
	push	r9

#ifdef __PIC__
	move.d	pc,r0
	sub.d	.:GOTOFF,r0
	move.d	_init:PLTG,r13
	add.d	r0,r13
	move.d	_fini:PLTG,r9
	add.d	r0,r9
	move.d	main:PLTG,r10
	add.d	r0,r10
#else
	move.d	_init,r13
	move.d	_fini,r9
	move.d	main,r10
#endif

	push r9

#if (defined L_crt1 || defined L_gcrt1) && defined __UCLIBC_CTOR_DTOR__
	PLTCALL(__uClibc_start_main)
#else
	PLTCALL(__uClibc_main)
#endif

	;; Crash if 'exit' returns.
	test.d	[6502]
0:
	ba 0b
	nop
	
	;; Define a symbol for the first piece of initialized data.
	.data
	.globl	__data_start

__data_start:
	.long	0
	.weak	data_start
	data_start = __data_start
