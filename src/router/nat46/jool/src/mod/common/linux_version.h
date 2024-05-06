#ifndef SRC_MOD_COMMON_LINUX_VERSION_H_
#define SRC_MOD_COMMON_LINUX_VERSION_H_

#include <linux/version.h>

/*
 * A bunch of code has to be compiled differently depending on kernel version.
 * RHEL kernels, however, do not define the LINUX_VERSION_CODE macro correctly.
 *
 * LINUX_VERSION_CODE is therefore not always a good solution; use these macros
 * instead.
 */

#ifdef RHEL_RELEASE_CODE

#define LINUX_VERSION_AT_LEAST(a, b, c, ra, rb) \
		RHEL_RELEASE_CODE >= RHEL_RELEASE_VERSION(ra, rb)
#define LINUX_VERSION_LOWER_THAN(a, b, c, ra, rb) \
		RHEL_RELEASE_CODE < RHEL_RELEASE_VERSION(ra, rb)

#else

#define LINUX_VERSION_AT_LEAST(a, b, c, ra, rb) \
		LINUX_VERSION_CODE >= KERNEL_VERSION(a, b, c)
#define LINUX_VERSION_LOWER_THAN(a, b, c, ra, rb) \
		LINUX_VERSION_CODE < KERNEL_VERSION(a, b, c)

#endif

#endif /* SRC_MOD_COMMON_LINUX_VERSION_H_ */
