/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * NTFS kernel debug support. Part of the Linux-NTFS project.
 *
 * Copyright (c) 2001-2004 Anton Altaparmakov
 */

#ifndef _LINUX_NTFS_MISC_H
#define _LINUX_NTFS_MISC_H

#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/highmem.h>

#include "runlist.h"

#ifdef DEBUG

extern int debug_msgs;

extern __printf(4, 5)
void __ntfs_debug(const char *file, int line, const char *function,
		  const char *format, ...);
/**
 * ntfs_debug - write a debug level message to syslog
 * @f:		a printf format string containing the message
 * @...:	the variables to substitute into @f
 *
 * ntfs_debug() writes a DEBUG level message to the syslog but only if the
 * driver was compiled with -DDEBUG. Otherwise, the call turns into a NOP.
 */
#define ntfs_debug(f, a...)						\
	__ntfs_debug(__FILE__, __LINE__, __func__, f, ##a)

void ntfs_debug_dump_runlist(const struct runlist_element *rl);

#else	/* !DEBUG */

#define ntfs_debug(fmt, ...)						\
do {									\
	if (0)								\
		no_printk(fmt, ##__VA_ARGS__);				\
} while (0)

#define ntfs_debug_dump_runlist(rl)					\
do {									\
	if (0)								\
		(void)rl;						\
} while (0)

#endif	/* !DEBUG */

extern  __printf(3, 4)
void __ntfs_warning(const char *function, const struct super_block *sb,
		    const char *fmt, ...);
#define ntfs_warning(sb, f, a...)	__ntfs_warning(__func__, sb, f, ##a)

extern  __printf(3, 4)
void __ntfs_error(const char *function, struct super_block *sb,
		  const char *fmt, ...);
#define ntfs_error(sb, f, a...)		__ntfs_error(__func__, sb, f, ##a)

void ntfs_handle_error(struct super_block *sb);

#if defined(DEBUG) && defined(CONFIG_SYSCTL)
int ntfs_sysctl(int add);
#else
/* Just return success. */
static inline int ntfs_sysctl(int add)
{
	return 0;
}
#endif

#define NTFS_TIME_OFFSET ((s64)(369 * 365 + 89) * 24 * 3600 * 10000000)

/**
 * utc2ntfs - convert Linux UTC time to NTFS time
 * @ts:		Linux UTC time to convert to NTFS time
 *
 * Convert the Linux UTC time @ts to its corresponding NTFS time and return
 * that in little endian format.
 *
 * Linux stores time in a struct timespec64 consisting of a time64_t tv_sec
 * and a long tv_nsec where tv_sec is the number of 1-second intervals since
 * 1st January 1970, 00:00:00 UTC and tv_nsec is the number of 1-nano-second
 * intervals since the value of tv_sec.
 *
 * NTFS uses Microsoft's standard time format which is stored in a s64 and is
 * measured as the number of 100-nano-second intervals since 1st January 1601,
 * 00:00:00 UTC.
 */
static inline __le64 utc2ntfs(const struct timespec64 ts)
{
	/*
	 * Convert the seconds to 100ns intervals, add the nano-seconds
	 * converted to 100ns intervals, and then add the NTFS time offset.
	 */
	return cpu_to_le64((s64)ts.tv_sec * 10000000 + ts.tv_nsec / 100 +
			NTFS_TIME_OFFSET);
}

/**
 * ntfs2utc - convert NTFS time to Linux time
 * @time:	NTFS time (little endian) to convert to Linux UTC
 *
 * Convert the little endian NTFS time @time to its corresponding Linux UTC
 * time and return that in cpu format.
 *
 * Linux stores time in a struct timespec64 consisting of a time64_t tv_sec
 * and a long tv_nsec where tv_sec is the number of 1-second intervals since
 * 1st January 1970, 00:00:00 UTC and tv_nsec is the number of 1-nano-second
 * intervals since the value of tv_sec.
 *
 * NTFS uses Microsoft's standard time format which is stored in a s64 and is
 * measured as the number of 100 nano-second intervals since 1st January 1601,
 * 00:00:00 UTC.
 */
static inline struct timespec64 ntfs2utc(const __le64 time)
{
	struct timespec64 ts;

	/* Subtract the NTFS time offset. */
	u64 t = (u64)(le64_to_cpu(time) - NTFS_TIME_OFFSET);
	/*
	 * Convert the time to 1-second intervals and the remainder to
	 * 1-nano-second intervals.
	 */
	ts.tv_nsec = do_div(t, 10000000) * 100;
	ts.tv_sec = t;
	return ts;
}

/**
 * __ntfs_malloc - allocate memory in multiples of pages
 * @size:	number of bytes to allocate
 * @gfp_mask:	extra flags for the allocator
 *
 * Internal function.  You probably want ntfs_malloc_nofs()...
 *
 * Allocates @size bytes of memory, rounded up to multiples of PAGE_SIZE and
 * returns a pointer to the allocated memory.
 *
 * If there was insufficient memory to complete the request, return NULL.
 * Depending on @gfp_mask the allocation may be guaranteed to succeed.
 */
static inline void *__ntfs_malloc(unsigned long size, gfp_t gfp_mask)
{
	if (likely(size <= PAGE_SIZE)) {
		if (!size)
			return NULL;
		/* kmalloc() has per-CPU caches so is faster for now. */
		return kmalloc(PAGE_SIZE, gfp_mask & ~__GFP_HIGHMEM);
		/* return (void *)__get_free_page(gfp_mask); */
	}
	if (likely((size >> PAGE_SHIFT) < totalram_pages()))
		return __vmalloc(size, gfp_mask);
	return NULL;
}

/**
 * ntfs_malloc_nofs - allocate memory in multiples of pages
 * @size:	number of bytes to allocate
 *
 * Allocates @size bytes of memory, rounded up to multiples of PAGE_SIZE and
 * returns a pointer to the allocated memory.
 *
 * If there was insufficient memory to complete the request, return NULL.
 */
static inline void *ntfs_malloc_nofs(unsigned long size)
{
	return __ntfs_malloc(size, GFP_NOFS | __GFP_HIGHMEM | __GFP_ZERO);
}

/**
 * ntfs_malloc_nofs_nofail - allocate memory in multiples of pages
 * @size:	number of bytes to allocate
 *
 * Allocates @size bytes of memory, rounded up to multiples of PAGE_SIZE and
 * returns a pointer to the allocated memory.
 *
 * This function guarantees that the allocation will succeed.  It will sleep
 * for as long as it takes to complete the allocation.
 *
 * If there was insufficient memory to complete the request, return NULL.
 */
static inline void *ntfs_malloc_nofs_nofail(unsigned long size)
{
	return __ntfs_malloc(size, GFP_NOFS | __GFP_HIGHMEM | __GFP_NOFAIL);
}

static inline void ntfs_free(void *addr)
{
	kvfree(addr);
}

static inline void *ntfs_realloc_nofs(void *addr, unsigned long new_size,
		unsigned long cpy_size)
{
	void *pnew_addr;

	if (new_size == 0) {
		ntfs_free(addr);
		return NULL;
	}

	pnew_addr = ntfs_malloc_nofs(new_size);
	if (pnew_addr == NULL)
		return NULL;
	if (addr) {
		cpy_size = min(cpy_size, new_size);
		if (cpy_size)
			memcpy(pnew_addr, addr, cpy_size);
		ntfs_free(addr);
	}
	return pnew_addr;
}
#endif /* _LINUX_NTFS_MISC_H */
