/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_STDARG_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_STDARG_H_

#include <linux/version.h>
#if LINUX_VERSION_IS_GEQ(5, 15, 0)
#include_next <linux/stdarg.h>
#else
#include <stdarg.h>
#endif

#endif
