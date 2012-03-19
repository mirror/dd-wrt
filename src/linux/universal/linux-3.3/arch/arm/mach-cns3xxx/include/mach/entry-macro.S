/*
 * Low-level IRQ helper macros for Cavium Networks platforms
 *
 * Copyright 2008 Cavium Networks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

		.macro	disable_fiq
		.endm

		.macro  arch_ret_to_user, tmp1, tmp2
		.endm

		.macro test_for_cache_ipi, irqnr, irqstat, base, tmp
		bic	\irqnr, \irqstat, #0x1c00
		mov 	\tmp, #0
		cmp	\irqnr, #1
		moveq	\tmp, #1
		streq	\irqstat, [\base, #GIC_CPU_EOI]
		cmp	\tmp, #0
		.endm
