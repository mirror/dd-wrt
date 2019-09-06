#ifndef __BACKPORT_LINUX_LOCKDEP_H
#define __BACKPORT_LINUX_LOCKDEP_H
#include_next <linux/lockdep.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
#undef lockdep_assert_held
#ifdef CONFIG_LOCKDEP
#define lockdep_assert_held(l)	do {				\
		WARN_ON(debug_locks && !lockdep_is_held(l));	\
	} while (0)
#else
#define lockdep_assert_held(l)			do { (void)(l); } while (0)
#endif /* CONFIG_LOCKDEP */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0) */

#endif /* __BACKPORT_LINUX_LOCKDEP_H */
