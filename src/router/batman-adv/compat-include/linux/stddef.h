/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_STDDEF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_STDDEF_H_

#include <linux/version.h>
#include_next <linux/stddef.h>

#if LINUX_VERSION_IS_LESS(4, 16, 0)

#ifndef sizeof_field
#define sizeof_field(TYPE, MEMBER) sizeof((((TYPE *)0)->MEMBER))
#endif

#endif /* LINUX_VERSION_IS_LESS(4, 16, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_STDDEF_H_ */
