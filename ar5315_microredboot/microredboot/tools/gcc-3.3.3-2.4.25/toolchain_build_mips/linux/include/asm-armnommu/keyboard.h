/*
 *  linux/include/asm-arm/keyboard.h
 *
 *  Copyright (C) 1998 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Keyboard driver definitions for ARM
 */
#ifndef __ASM_ARM_KEYBOARD_H
#define __ASM_ARM_KEYBOARD_H

/*
 * We provide a unified keyboard interface when in VC_MEDIUMRAW
 * mode.  This means that all keycodes must be common between
 * all supported keyboards.  This unfortunately puts us at odds
 * with the PC keyboard interface chip... but we can't do anything
 * about that now.
 */
#ifdef __KERNEL__

#include <asm/arch/keyboard.h>

#endif /* __KERNEL__ */

#endif /* __ASM_ARM_KEYBOARD_H */
