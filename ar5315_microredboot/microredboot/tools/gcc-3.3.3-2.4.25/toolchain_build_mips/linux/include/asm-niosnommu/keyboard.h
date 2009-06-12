/*
 *  linux/include/asm-niosnommu/keyboard.h
 *
  */

/*
 *  This file contains the nios architecture specific keyboard definitions
 */

#ifndef __NIOS_KEYBOARD_H
#define __NIOS_KEYBOARD_H

#define kbd_setkeycode(x...)	(-ENOSYS)
#define kbd_getkeycode(x...)	(-ENOSYS)
#define kbd_translate(x...)	(0)
#define kbd_unexpected_up(x...)	(1)
#define kbd_leds(x...)		do {;} while (0)
#define kbd_init_hw(x...)	do {;} while (0)
#define kbd_enable_irq(x...)	do {;} while (0)
#define kbd_disable_irq(x...)	do {;} while (0)


/* needed if MAGIC_SYSRQ is enabled for serial console */
#ifndef SYSRQ_KEY
#define SYSRQ_KEY		((unsigned char)(-1))
#endif
#define kbd_sysrq_xlate         ((unsigned char *)NULL)

#endif /* __NIOS_KEYBOARD_H */
