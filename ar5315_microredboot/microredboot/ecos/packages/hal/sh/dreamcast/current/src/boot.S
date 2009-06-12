#------------------------------------------------------------------------------
# Dreamcast CD-ROM Boot entry

#include <cyg/hal/arch.inc>
	
	.section .entry,"ax"
	.org	0
FUNC_START(_dc_entry)
	mov.l	$reset,r0
	jmp	@r0
	nop
	.align	2
$reset:
	.long	CYG_LABEL_DEFN(_reset)

