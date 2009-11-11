#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

/*
 * Glue code to save registers and get us to the interrupt dispatcher
 */
	.text
	.set	noat
	.align	5
NESTED(ar7240_interrupt_receive, PT_SIZE, sp)
	SAVE_ALL
	CLI
	.set	at

	move	a0, sp
	jal	ar7240_irq_dispatch

	j	ret_from_irq

	END(ar7240_interrupt_receive)
