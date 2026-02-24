/*
 * Copyright 2022 Morse Micro
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "mmrc_osal.h"

void osal_mmrc_seed_random(void)
{
#if KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE
	prandom_seed(jiffies);
#else
	/*
	 * calling prandom_seed is no longer required, and the functionality was
	 * removed in commit d4150779e60fb6c49be25572596b2cdfc5d46a09.
	 */
#endif
}

u32 osal_mmrc_random_u32(u32 max)
{
#if KERNEL_VERSION(6, 2, 0) > LINUX_VERSION_CODE
	return prandom_u32_max(max);
#else
	return get_random_u32_below(max);
#endif
}
