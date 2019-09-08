
#ifndef __NDPI_KERNEL_COMPAT_H__
#define __NDPI_KERNEL_COMPAT_H__

#ifdef __KERNEL__

#include <asm/byteorder.h>
#include <linux/kernel.h>
#include <linux/time.h>

typedef size_t socklen_t;
static const char *
inet_ntop (int af, const void *src, char *dst, socklen_t size);
static int inet_pton(int af, const char *src, void *dst);
static int atoi(const char *);
static long int atol(const char *);
static void gettimeofday(struct timeval *tv, void *tz);

#define le32toh(v) le32_to_cpu(v)
#define le16toh(v) le16_to_cpu(v)

#endif
#endif
