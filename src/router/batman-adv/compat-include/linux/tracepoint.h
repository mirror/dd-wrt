/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_TRACEPOINT_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_TRACEPOINT_H_

#include <linux/version.h>
#include_next <linux/tracepoint.h>

#if LINUX_VERSION_IS_LESS(6, 0, 0)

#define __vstring(item, fmt, ap) __dynamic_array(char, item, 256)
#define __assign_vstr(dst, fmt, va) \
	WARN_ON_ONCE(vsnprintf(__get_dynamic_array(dst), 256, fmt, *va) >= 256)

#endif /* LINUX_VERSION_IS_LESS(6, 0, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_TRACEPOINT_H_ */
