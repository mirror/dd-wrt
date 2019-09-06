#ifndef __BACKPORT_LINUX_TIME64_H
#define __BACKPORT_LINUX_TIME64_H
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#include_next <linux/time64.h>
#else
#include <linux/time.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
#define timespec64_equal		timespec_equal
#define timespec64_compare		timespec_compare
#define set_normalized_timespec64	set_normalized_timespec
#define timespec64_add_safe		timespec_add_safe
#define timespec64_add			timespec_add
#define timespec64_sub			timespec_sub
#define timespec64_valid		timespec_valid
#define timespec64_valid_strict		timespec_valid_strict
#define timespec64_to_ns		timespec_to_ns
#define ns_to_timespec64		ns_to_timespec
#define timespec64_add_ns		timespec_add_ns
#define timespec64			timespec
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0) */

#endif /* __BACKPORT_LINUX_TIME64_H */
