/*
 *  linux/arch/arm/mach-ebsa110/leds.c
 *
 *  Copyright (C) 1998 Russell King
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  EBSA-110 LED control routines.  We use the led as follows:
 *
 *   - Red - toggles state every 50 timer interrupts
 */
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/init.h>

#include <asm/hardware.h>
#include <asm/leds.h>
#include <asm/system.h>
#include <asm/mach-types.h>

#ifdef CONFIG_LEDS

static  spinlock_t 	leds_lock = SPIN_LOCK_UNLOCKED;
static  u32   		led_val = 0;
static	u32		led_count = 0;
extern void mvBoardDebug7Seg(u32);

static void mv_leds_event(led_event_t ledevt)
{
	unsigned long flags;

	spin_lock_irqsave(&leds_lock, flags);

	switch(ledevt) {
	case led_timer:
		if(led_count == 0)
		{
			led_count = 1;
            mvBoardDebug7Seg(led_val);
            led_val++;
            if(led_val == 0xa)
                led_val =0;
		}
		else
			led_count = 0;
		break;

	default:
			break;
	}

	spin_unlock_irqrestore(&leds_lock, flags);
}

static int __init leds_init(void)
{
	leds_event = mv_leds_event;

	return 0;
}

__initcall(leds_init);

#endif /* CONFIG_LEDS */
