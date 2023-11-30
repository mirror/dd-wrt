/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) B.A.T.M.A.N. contributors:
 *
 * Antonio Quartulli
 *
 * This file contains macros for maintaining compatibility with older versions
 * of the Linux kernel.
 */

#ifndef _NET_BATMAN_ADV_COMPAT_LINUX_RCUPDATE_H_
#define _NET_BATMAN_ADV_COMPAT_LINUX_RCUPDATE_H_

#include <linux/version.h>
#include_next <linux/rcupdate.h>

#if LINUX_VERSION_IS_LESS(5, 5, 0)

#undef rcu_replace_pointer
#define rcu_replace_pointer(rcu_ptr, ptr, c)				\
({									\
	typeof(ptr) __tmp = rcu_dereference_protected((rcu_ptr), (c));	\
	rcu_assign_pointer((rcu_ptr), (ptr));				\
	__tmp;								\
})

#endif /* LINUX_VERSION_IS_LESS(5, 5, 0) */

#endif /* _NET_BATMAN_ADV_COMPAT_LINUX_RCUPDATE_H_ */
