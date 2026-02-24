/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 */

#include <linux/device.h>

#include "debug.h"
#include "dot11ah.h"

u32 dot11ah_debug_mask;

#define __dot11ah_fn(fn)							\
void __dot11ah_ ##fn(u32 level, const char *func, int line, const char *fmt, ...)	\
{									\
	struct va_format vaf = {					\
		.fmt = fmt,						\
	};								\
	va_list args;							\
									\
	va_start(args, fmt);						\
	vaf.va = &args;							\
	if (level)							\
		pr_ ##fn("%s:%d: %pV", func, line, &vaf);			\
	va_end(args);							\
}

__dot11ah_fn(debug)
__dot11ah_fn(info)
__dot11ah_fn(warn)
__dot11ah_fn(warn_ratelimited)
__dot11ah_fn(err)

void morse_dot11ah_debug_set_mask(u32 mask)
{
	dot11ah_debug_mask = mask;
}
EXPORT_SYMBOL(morse_dot11ah_debug_set_mask);

void morse_dot11ah_debug_init(u32 mask)
{
	morse_dot11ah_debug_set_mask(mask);
}
EXPORT_SYMBOL(morse_dot11ah_debug_init);
