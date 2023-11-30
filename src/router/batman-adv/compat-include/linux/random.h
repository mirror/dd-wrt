/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_RANDOM_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_RANDOM_H_

#include <linux/version.h>
#include_next <linux/random.h>


#if LINUX_VERSION_IS_LESS(6, 2, 0)

static inline u32 batadv_get_random_u32_below(u32 ep_ro)
{
	return prandom_u32_max(ep_ro);
}

#define get_random_u32_below batadv_get_random_u32_below

#endif /* LINUX_VERSION_IS_LESS(6, 2, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_RANDOM_H_ */
