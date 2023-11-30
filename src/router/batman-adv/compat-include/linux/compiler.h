/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_

#include <linux/version.h>
#include_next <linux/compiler.h>

#if LINUX_VERSION_IS_LESS(5, 4, 0)

#ifndef fallthrough
#if __GNUC__ > 7 && !defined(__CHECKER__)
# define fallthrough                    __attribute__((__fallthrough__))
#else
# define fallthrough                    do {} while (0)  /* fallthrough */
#endif
#endif

#endif /* LINUX_VERSION_IS_LESS(5, 4, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_COMPILER_H_ */
