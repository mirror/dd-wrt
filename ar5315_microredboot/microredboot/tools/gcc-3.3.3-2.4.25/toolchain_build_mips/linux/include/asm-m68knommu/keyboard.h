/*
 *  linux/include/asm-m68knommu/keyboard.h
 *  Created 04 Dec 2001 by Khaled Hassounah <khassounah@mediumware.net>
 *  This file contains the Dragonball architecture specific keyboard definitions
 */

#ifndef _M68KNOMMU_KEYBOARD_H
#define _M68KNOMMU_KEYBOARD_H

#include <linux/config.h>

#if 0
#define kbd_setkeycode(x...)    (-ENOSYS)
#define kbd_getkeycode(x...)    (-ENOSYS)
#define kbd_translate(sc_,kc_,rm_)	((*(kc_)=(sc_)),1)
#define kbd_unexpected_up(x...) (1)
#define kbd_leds(x...)		do { } while (0)
#define kbd_init_hw(x...)	do { } while (0)
#define kbd_enable_irq(x...)	do { } while (0)
#define kbd_disable_irq(x...)	do { } while (0)

#else

/* dummy i.e. no real keyboard */
#define kbd_setkeycode(x...)	(-ENOSYS)
#define kbd_getkeycode(x...)	(-ENOSYS)
#define kbd_translate(x...)	(0)
#define kbd_unexpected_up(x...)	(1)
#define kbd_leds(x...)		do {;} while (0)
#define kbd_init_hw(x...)	do {;} while (0)
#define kbd_enable_irq(x...)	do {;} while (0)
#define kbd_disable_irq(x...)	do {;} while (0)

#endif


/* needed if MAGIC_SYSRQ is enabled for serial console */
#ifndef SYSRQ_KEY
#define SYSRQ_KEY		((unsigned char)(-1))
#define kbd_sysrq_xlate         ((unsigned char *)NULL)
#endif


#endif  /* _M68KNOMMU_KEYBOARD_H */



