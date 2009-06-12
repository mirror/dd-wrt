/*
 *  linux/include/asm-arm/arch-ti926/keyboard.h
 *
 *  Copyright (C) 2000 RidgeRun, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Keyboard driver definitions for ARM
 */
#ifndef __ASM_ARM_ARCH_TI925_KEYBOARD_H
#define __ASM_ARM_ARCH_TI925_KEYBOARD_H

#if 0
#define kbd_request_region
#define kbd_write_command
#define kbd_write_output
#define kbd_read_status
#define kbd_read_input
#define aux_request_irq
#define aux_free_irq
#endif

/*
 * Required by drivers/char/keyboard.c. I took these from arch-arc.
 * --gmcnutt
 */
#define kbd_setkeycode(sc,kc) (-EINVAL)
#define kbd_getkeycode(sc) (-EINVAL)
#define kbd_translate(sc,kcp,rm) ({ *(kcp) = (sc); 1; })
#define kbd_unexpected_up(kc) (0200)
#define kbd_leds(leds)
#define kbd_init_hw()
#define kbd_enable_irq() /* what irq? ;) --gmcnutt */
#define kbd_disable_irq()


#endif /* __ASM_ARM_ARCH_TI925_KEYBOARD_H */
