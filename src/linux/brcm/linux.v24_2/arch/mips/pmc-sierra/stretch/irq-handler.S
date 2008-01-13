/*
 * Copyright 2004 PMC-Sierra Inc.
 * Author: Manish Lachwani (lachwani@pmc-sierra.com)
 *
 * First-level interrupt dispatcher for the PMC-Sierra Stretch Board
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 */
#define __ASSEMBLY__
#include <linux/config.h>
#include <asm/asm.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <asm/regdef.h>
#include <asm/stackframe.h>

/*
 * First level interrupt dispatcher for the PMC-Sierra Stretch Board
 */

		.align	5
		NESTED(pmc_stretch_handle_int, PT_SIZE, sp)
		SAVE_ALL
		CLI
		.set	at

		mfc0	t0, CP0_CAUSE
		mfc0	t2, CP0_STATUS

		and	t0, t2

		andi	t1, t0, STATUSF_IP0	/* sw0 software interrupt */
		bnez	t1, ll_sw0_irq

		andi	t1, t0, STATUSF_IP1	/* sw1 software interrupt */
		bnez	t1, ll_sw1_irq

		andi	t1, t0, STATUSF_IP7	/* Rm7000 CPU timer */
		bnez	t1, ll_timer_irq

		.set	reorder

		/* wrong alarm or masked ... */
		j	spurious_interrupt
		nop
		END(pmc_stretch_handle_int)

		.align	5

ll_sw0_irq:
		li	a0, 1
		move	a1, sp
		jal	do_IRQ
		j	ret_from_irq

ll_sw1_irq:
		li	a0, 2
		move	a1, sp
		jal	do_IRQ
		j	ret_from_irq

ll_timer_irq:
		li	a0, 8
		move	a1, sp
		jal	do_IRQ
		j	ret_from_irq

			
