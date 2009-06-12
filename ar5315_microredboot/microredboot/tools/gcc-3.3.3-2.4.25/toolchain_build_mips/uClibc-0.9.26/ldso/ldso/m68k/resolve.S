/*
 * These are various helper routines that are needed to run an ELF image.
 */

.text
.even

.globl _dl_linux_resolve
	.type	_dl_linux_resolve,@function
_dl_linux_resolve:
	moveml	%a0/%a1,%sp@-
#ifdef __PIC__
	bsrl	_dl_linux_resolver@PLTPC
#else
	jbsr	_dl_linux_resolver
#endif
	moveml	%sp@+,%a0/%a1
	addql 	#8,%sp
	jmp	@(%d0)
.LFE2:
	.size _dl_linux_resolve,.LFE2-_dl_linux_resolve
