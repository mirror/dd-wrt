
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

typedef long intptr_t;
typedef size_t socklen_t;
const char *
inet_ntop (int af, const void *src, char *dst, socklen_t size);
int inet_pton(int af, const char *src, void *dst);
int atoi(const char *);
long int atol(const char *);

void gettimeofday64(struct timespec64 *tv, void *tz);
char *strtok_r(char *str, const char *delim, char **saveptr);

#define le64toh(v) le64_to_cpu(v)
#define le32toh(v) le32_to_cpu(v)
#define le16toh(v) le16_to_cpu(v)

#else
typedef int64_t time64_t;
struct timespec64 {
        time64_t        tv_sec;                 /* seconds */
        long            tv_nsec;                /* nanoseconds */
};
#endif
#endif
