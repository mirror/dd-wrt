/*
 *  linux/arch/arm/lib/memchr.S
 *
 *  Copyright (C) 1995-2000 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  ASM optimised string functions
 */
#include <linux/linkage.h>
#include <asm/assembler.h>

	.text
	.align	5
ENTRY(memcmp)
	/* if ((len - 1) < 0) return 0 */
	subs	r2, r2, #1
	movmi	r0, #0
	movmi	pc, lr
	/* ip == last src address to compare */
	add	ip, r0, r2
1:
	ldrb	r2, [r0], #1
	ldrb	r3, [r1], #1
	cmp	ip, r0
	cmpcs	r2, r3
	beq	1b
	sub	r0, r2, r3
 	mov	pc, lr
