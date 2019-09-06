#ifndef __BACKPORT_TYPES
#define __BACKPORT_TYPES
#include <linux/version.h>
#include_next <linux/types.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
typedef __s64 time64_t;
#endif

#endif /* __BACKPORT_TYPES */
