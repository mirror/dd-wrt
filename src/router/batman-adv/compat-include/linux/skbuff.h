/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Marek Lindner, Simon Wunderlich
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_SKBUFF_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_SKBUFF_H_

#include <linux/version.h>
#include_next <linux/skbuff.h>

#if LINUX_VERSION_IS_LESS(5, 4, 0)

#define nf_reset_ct nf_reset

#endif /* LINUX_VERSION_IS_LESS(5, 4, 0) */

#endif	/* _NET_BATMAN_ADV_COMPAT_LINUX_SKBUFF_H_ */
