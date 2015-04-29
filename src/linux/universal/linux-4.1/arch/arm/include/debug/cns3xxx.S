/*
 * Debugging macro include header
 *
 * Copyright 1994-1999 Russell King
 * Copyright 2008 Cavium Networks
 * Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

		.macro	addruart,rp,rv,tmp
		ldr	\rv, =0xFEF09000
		ldr	\rp, =0x78000000
		.endm

#define UART_SHIFT	2
#include <asm/hardware/debug-8250.S>
