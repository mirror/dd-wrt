/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_KSTRTOX_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_KSTRTOX_H_

#include <linux/version.h>
#if (LINUX_VERSION_IS_GEQ(5, 10, 185) && LINUX_VERSION_IS_LESS(5, 11, 0)) || \
    LINUX_VERSION_IS_GEQ(5, 14, 0)
#include_next <linux/kstrtox.h>
#else
#include <linux/kernel.h>
#endif

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_KSTRTOX_H_ */
