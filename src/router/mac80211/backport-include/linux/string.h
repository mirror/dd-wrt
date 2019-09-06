#ifndef __BACKPORT_LINUX_STRING_H
#define __BACKPORT_LINUX_STRING_H
#include_next <linux/string.h>
#include <linux/version.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0))
#define memdup_user_nul LINUX_BACKPORT(memdup_user_nul)
extern void *memdup_user_nul(const void __user *, size_t);
#endif

/* this was added in v3.2.65, v3.4.106, v3.10.60, v3.12.33, v3.14.24,
 * v3.17.3 and v3.18 */
#if !(LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,3) || \
      (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,24) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)) || \
      (LINUX_VERSION_CODE >= KERNEL_VERSION(3,12,33) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)) || \
      (LINUX_VERSION_CODE >= KERNEL_VERSION(3,10,60) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)) || \
      (LINUX_VERSION_CODE >= KERNEL_VERSION(3,4,106) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(3,5,0)) || \
      (LINUX_VERSION_CODE >= KERNEL_VERSION(3,2,65) && \
      LINUX_VERSION_CODE < KERNEL_VERSION(3,3,0)))
#define memzero_explicit LINUX_BACKPORT(memzero_explicit)
void memzero_explicit(void *s, size_t count);
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0))
ssize_t strscpy(char *dest, const char *src, size_t count);
#endif

#endif /* __BACKPORT_LINUX_STRING_H */
