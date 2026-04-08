// compat.h
#ifndef _COMPAT_H
#define _COMPAT_H

#include <linux/version.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 10, 0)
#include <linux/vmalloc.h>
#define compat_const const
#else
#define compat_const
#endif

#endif /* _COMPAT_H */
