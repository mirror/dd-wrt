/* linux/include/asm-arm/arch-ebsa110/debug-macro.S
 *
 * Debugging macro include header
 *
 *  Copyright (C) 1994-1999 Russell King
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
**/

		.macro	addruart,rx
		mov	\rx, #0x42000000
		.endm

#define UART_SHIFT	2
#define FLOW_CONTROL
#include <asm/hardware/debug-8250.S>
