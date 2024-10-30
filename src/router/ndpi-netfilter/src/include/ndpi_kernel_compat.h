
#ifndef __NDPI_KERNEL_COMPAT_H__
#define __NDPI_KERNEL_COMPAT_H__

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>

#include <asm/byteorder.h>
#include <linux/types.h>
#include <linux/time.h>
#include <linux/in6.h>
#define UINT32_MAX	U32_MAX

#ifndef MIN
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))
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
#endif /* LINUX_VERSION_IS_LESS(3,17,0) */

typedef long intptr_t;
typedef size_t socklen_t;
NDPI_STATIC const char *
inet_ntop (int af, const void *src, char *dst, socklen_t size);
NDPI_STATIC int inet_pton(int af, const char *src, void *dst);
NDPI_STATIC uint32_t inet_addr(const char *ip);
NDPI_STATIC int atoi(const char *);
NDPI_STATIC long int atol(const char *);

NDPI_STATIC void gettimeofday64(struct timespec64 *tv, void *tz);
NDPI_STATIC char *strtok_r(char *str, const char *delim, char **saveptr);
static inline long int strtol(const char *nptr, char **endptr, int base) {
    long int ret;
    if(kstrtol(nptr,base,&ret)) ret = 0;
    return ret;
}

#define le64toh(v) le64_to_cpu(v)
#define le32toh(v) le32_to_cpu(v)
#define le16toh(v) le16_to_cpu(v)
#define be32toh(v) be32_to_cpu(v)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,17,0)
typedef __s64 time64_t;
#endif

#else

typedef int64_t time64_t;
struct timespec64 {
        time64_t        tv_sec;                 /* seconds */
        long            tv_nsec;                /* nanoseconds */
};
#endif
#endif
