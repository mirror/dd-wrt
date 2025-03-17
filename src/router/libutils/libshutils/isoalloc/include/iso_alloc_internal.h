/* iso_alloc_internal.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#if !__aarch64__ && !__x86_64__
#pragma error "IsoAlloc is untested and unsupported on 32 bit platforms"
assert(sizeof(size_t) >= 64)
#endif

#if __linux__
#include "os/linux.h"
#endif

#if __APPLE__
#include "os/macos.h"
#endif

#if __ANDROID__
#include "os/android.h"
#endif

#if __FreeBSD__
#include "os/freebsd.h"
#endif

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/types.h>

#if MEM_USAGE
#include <sys/resource.h>
#endif

#if THREAD_SUPPORT
#include <pthread.h>
#ifdef __cplusplus
#include <atomic>
    using namespace std;
#else
#include <stdatomic.h>
#endif
#endif

#if HEAP_PROFILER
#include <fcntl.h>
#endif

#include "conf.h"
#include "iso_alloc.h"
#include "iso_alloc_sanity.h"
#include "iso_alloc_util.h"
#include "iso_alloc_ds.h"
#include "iso_alloc_profiler.h"
#include "compiler.h"

#ifndef MADV_DONTNEED
#define MADV_DONTNEED POSIX_MADV_DONTNEED
#endif

#ifndef MADV_FREE
#define FREE_OR_DONTNEED MADV_DONTNEED
#else
#define FREE_OR_DONTNEED MADV_FREE
#endif

#if ENABLE_ASAN
#include <sanitizer/asan_interface.h>

#define POISON_ZONE(zone)                                                     \
    if(IS_POISONED_RANGE(zone->user_pages_start, ZONE_USER_SIZE) == 0) {      \
        ASAN_POISON_MEMORY_REGION(zone->user_pages_start, ZONE_USER_SIZE);    \
    }                                                                         \
    if(IS_POISONED_RANGE(zone->bitmap_start, zone->bitmap_size) == 0) {       \
        ASAN_POISON_MEMORY_REGION(zone->user_pages_start, zone->bitmap_size); \
    }

#define UNPOISON_ZONE(zone)                                                  \
    if(IS_POISONED_RANGE(zone->user_pages_start, ZONE_USER_SIZE) != 0) {     \
        ASAN_UNPOISON_MEMORY_REGION(zone->user_pages_start, ZONE_USER_SIZE); \
    }                                                                        \
    if(IS_POISONED_RANGE(zone->bitmap_start, zone->bitmap_size) != 0) {      \
        ASAN_UNPOISON_MEMORY_REGION(zone->bitmap_start, zone->bitmap_size);  \
    }

#define POISON_ZONE_CHUNK(zone, ptr)                      \
    if(IS_POISONED_RANGE(ptr, zone->chunk_size) == 0) {   \
        ASAN_POISON_MEMORY_REGION(ptr, zone->chunk_size); \
    }

#define UNPOISON_ZONE_CHUNK(zone, ptr)                      \
    if(IS_POISONED_RANGE(ptr, zone->chunk_size) != 0) {     \
        ASAN_UNPOISON_MEMORY_REGION(ptr, zone->chunk_size); \
    }

#define POISON_BIG_ZONE(zone)                                          \
    if(IS_POISONED_RANGE(zone->user_pages_start, zone->size) == 0) {   \
        ASAN_POISON_MEMORY_REGION(zone->user_pages_start, zone->size); \
    }

#define UNPOISON_BIG_ZONE(zone)                                          \
    if(IS_POISONED_RANGE(zone->user_pages_start, zone->size) != 0) {     \
        ASAN_UNPOISON_MEMORY_REGION(zone->user_pages_start, zone->size); \
    }

#define IS_POISONED_RANGE(ptr, size) \
    __asan_region_is_poisoned(ptr, size)
#else
#define POISON_ZONE(zone)
#define UNPOISON_ZONE(zone)
#define POISON_ZONE_CHUNK(ptr, zone)
#define UNPOISON_ZONE_CHUNK(ptr, zone)
#define POISON_BIG_ZONE(zone)
#define UNPOISON_BIG_ZONE(zone)
#define IS_POISONED_RANGE(ptr, size) 0
#endif

#define OK 0
#define ERR -1

#ifndef HAVE_SYSLOG
#define dd_syslog(a, args...) \
	do {                  \
	} while (0)
#define dd_loginfo(a, fmt, args...) \
	do {                        \
	} while (0)
#define dd_logdebug(a, fmt, args...) \
	do {                         \
	} while (0)
#define dd_logerror(a, fmt, args...) \
	do {                         \
	} while (0)
#define dd_logstart(a, ret) \
	do {                \
	} while (0)
#else
#define dd_syslog(a, args...) syslog(a, ##args);
void dd_loginfo(const char *servicename, const char *fmt, ...);
void dd_logdebug(const char *servicename, const char *fmt, ...);
void dd_logerror(const char *servicename, const char *fmt, ...);
void dd_logstart(const char *servicename, int ret);
#endif

/* GCC complains if your constructor priority is
 * 0-100 but Clang does not. We need the lowest
 * priority constructor for MALLOC_HOOK */
#define FIRST_CTOR 101
#define LAST_DTOR 65535

#if DEBUG
#define LOG(msg, ...) \
    dd_logerror("iso_alloc", "[LOG][%d](%s:%d %s()) " msg "\n", getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__);
#else
#define LOG(msg, ...)
#endif

#define LOG_AND_ABORT(msg, ...)                                                                                                      \
    dd_logerror("iso_alloc",  "[ABORTING][%d](%s:%d %s()) " msg "\n", getpid(), __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    abort();

/* The number of bits in the bitmap that correspond
 * to a user chunk. We use 2 bits:
 *  00 free, never used
 *  10 currently in use
 *  01 was used, now free
 *  11 canary chunk / permanently free'd */
#define BITS_PER_CHUNK 2
#define BITS_PER_CHUNK_SHIFT 1

#define BITS_PER_BYTE 8
#define BITS_PER_BYTE_SHIFT 3

#define BITS_PER_QWORD 64
#define BITS_PER_QWORD_SHIFT 6

#define BITS_PER_ODWORD 128
#define BITS_PER_ODWORD_SHIFT 7

#define CANARY_SIZE 8

#define USED_BIT_VECTOR 0x5555555555555555

/* All chunks are 8 byte aligned */
#define CHUNK_ALIGNMENT 8

#define SZ_ALIGNMENT 32

#define WHICH_BIT(bit_slot) \
    (bit_slot & (BITS_PER_QWORD - 1))

#define IS_ALIGNED(v) \
    (v & (CHUNK_ALIGNMENT - 1))

#define IS_PAGE_ALIGNED(v) \
    (v & (g_page_size - 1))

#define GET_BIT(n, k) \
    (n >> k) & 1UL

#define SET_BIT(n, k) \
    n |= 1UL << k;

#define UNSET_BIT(n, k) \
    n &= ~(1UL << k);

#define ALIGN_SZ_UP(n) \
    ((n + SZ_ALIGNMENT - 1) & ~(SZ_ALIGNMENT - 1))

#define ALIGN_SZ_DOWN(n) \
    (((n + SZ_ALIGNMENT - 1) & ~(SZ_ALIGNMENT - 1)) - SZ_ALIGNMENT)

#define ROUND_UP_PAGE(n) \
    ((((n + g_page_size) - 1) >> g_page_size_shift) * (g_page_size))

#define ROUND_DOWN_PAGE(n) \
    (ROUND_UP_PAGE(n) - g_page_size)

#if MASK_PTRS
#define MASK_ZONE_PTRS(zone) \
    MASK_BITMAP_PTRS(zone);  \
    MASK_USER_PTRS(zone);

#define UNMASK_ZONE_PTRS(zone) \
    MASK_ZONE_PTRS(zone);

#define MASK_BITMAP_PTRS(zone) \
    zone->bitmap_start = (void *) ((uintptr_t) zone->bitmap_start ^ (uintptr_t) zone->pointer_mask);

#define MASK_USER_PTRS(zone) \
    zone->user_pages_start = (void *) ((uintptr_t) zone->user_pages_start ^ (uintptr_t) zone->pointer_mask);

#define UNMASK_USER_PTR(zone) \
    (void *) ((uintptr_t) zone->user_pages_start ^ (uintptr_t) zone->pointer_mask)

#define UNMASK_BITMAP_PTR(zone) \
    (void *) ((uintptr_t) zone->bitmap_start ^ (uintptr_t) zone->pointer_mask)

#define MASK_BIG_ZONE_NEXT(bnp) \
    UNMASK_BIG_ZONE_NEXT(bnp)

#define UNMASK_BIG_ZONE_NEXT(bnp) \
    ((iso_alloc_big_zone_t *) ((uintptr_t) _root->big_zone_next_mask ^ (uintptr_t) bnp))
#else
#define MASK_ZONE_PTRS(zone)
#define UNMASK_ZONE_PTRS(zone)
#define MASK_BITMAP_PTRS(zone)
#define MASK_USER_PTRS(zone)
#define UNMASK_USER_PTR(zone) (void *) zone->user_pages_start
#define UNMASK_BITMAP_PTR(zone) (void *) zone->bitmap_start
#define MASK_BIG_ZONE_NEXT(bnp) bnp
#define UNMASK_BIG_ZONE_NEXT(bnp) bnp
#endif

/* Cap our big zones at 4GB of memory */
#define BIG_SZ_MAX 4294967296

#define MIN_BITMAP_IDX 8

#define WASTED_SZ_MULTIPLIER 8
#define WASTED_SZ_MULTIPLIER_SHIFT 3

#define BIG_ZONE_META_DATA_PAGE_COUNT 1
#define BIG_ZONE_USER_PAGE_COUNT 2
#define BIG_ZONE_USER_PAGE_COUNT_SHIFT 1

#if MEMORY_TAGGING || (ARM_MTE == 1)
#define TAGGED_PTR_MASK 0x00ffffffffffffff
#define IS_TAGGED_PTR_MASK 0xff00000000000000
#define UNTAGGED_BITS 56
#define MEM_TAG_SIZE 1
#endif

#define MEGABYTE_SIZE 1048576
#define KILOBYTE_SIZE 1024

/* We don't validate the last byte of the canary.
 * It is always 0 to prevent an out of bounds read
 * from exposing it's value */
#define CANARY_VALIDATE_MASK 0xffffffffffffff00

#define BAD_BIT_SLOT -1

/* Calculate the user pointer given a zone and a bit slot */
#define POINTER_FROM_BITSLOT(zone, bit_slot) \
    ((void *) zone->user_pages_start + ((bit_slot >> 1) * zone->chunk_size));

/* This global is used by the page rounding macros.
 * The value stored in _root->system_page_size is
 * preferred but we need this to setup the root. */
extern uint32_t g_page_size;

/* We need to know what power of 2 the page size is */
extern uint32_t g_page_size_shift;

/* iso_alloc makes a number of default zones for common
 * allocation sizes. Allocations are 'first fit' up until
 * ZONE_1024 at which point a new zone is created for that
 * specific size request. */
#define DEFAULT_ZONE_COUNT sizeof(default_zones) >> 3

/* Each user allocation zone we make is 4MB in size.
 * With MAX_ZONES at 8192 this means we top out at
 * about 32~ gb of heap. If you adjust this then
 * you need to make sure that SMALL_SIZE_MAX is correctly
 * adjusted or you will calculate chunks outside of
 * the zone user memory! */
#define ZONE_USER_SIZE 4194304

static_assert(SMALLEST_CHUNK_SZ >= 16, "SMALLEST_CHUNK_SZ is too small, must be at least 16");
static_assert(SMALL_SIZE_MAX <= 131072, "SMALL_SIZE_MAX is too big, cannot exceed 131072");

#if THREAD_SUPPORT
#if USE_SPINLOCK
extern atomic_flag root_busy_flag;
#define LOCK_ROOT() \
    do {            \
    } while(atomic_flag_test_and_set(&root_busy_flag));

#define UNLOCK_ROOT() \
    atomic_flag_clear(&root_busy_flag);

#define LOCK_BIG_ZONE_FREE() \
    do {                     \
    } while(atomic_flag_test_and_set(&_root->big_zone_free_flag));

#define UNLOCK_BIG_ZONE_FREE() \
    atomic_flag_clear(&_root->big_zone_free_flag);

#define LOCK_BIG_ZONE_USED() \
    do {                     \
    } while(atomic_flag_test_and_set(&_root->big_zone_used_flag));

#define UNLOCK_BIG_ZONE_USED() \
    atomic_flag_clear(&_root->big_zone_used_flag);

#else
extern pthread_mutex_t root_busy_mutex;
#define LOCK_ROOT() \
    pthread_mutex_lock(&root_busy_mutex);

#define UNLOCK_ROOT() \
    pthread_mutex_unlock(&root_busy_mutex);

#define LOCK_BIG_ZONE_FREE() \
    pthread_mutex_lock(&_root->big_zone_free_mutex);

#define UNLOCK_BIG_ZONE_FREE() \
    pthread_mutex_unlock(&_root->big_zone_free_mutex);

#define LOCK_BIG_ZONE_USED() \
    pthread_mutex_lock(&_root->big_zone_used_mutex);

#define UNLOCK_BIG_ZONE_USED() \
    pthread_mutex_unlock(&_root->big_zone_used_mutex);

#endif
#else
#define LOCK_ROOT()
#define UNLOCK_ROOT()
#define LOCK_BIG_ZONE()
#define UNLOCK_BIG_ZONE()
#define LOCK_BIG_ZONE_FREE()
#define UNLOCK_BIG_ZONE_FREE()
#define LOCK_BIG_ZONE_USED()
#define UNLOCK_BIG_ZONE_USED()
#endif

/* The global root */
extern iso_alloc_root *_root;

INTERNAL_HIDDEN INLINE void check_big_canary(iso_alloc_big_zone_t *big);
INTERNAL_HIDDEN INLINE void check_canary(iso_alloc_zone_t *zone, const void *p);
INTERNAL_HIDDEN INLINE void iso_clear_user_chunk(uint8_t *p, size_t size);
INTERNAL_HIDDEN INLINE void insert_free_bit_slot(iso_alloc_zone_t *zone, int64_t bit_slot);
INTERNAL_HIDDEN INLINE void write_canary(iso_alloc_zone_t *zone, void *p);
INTERNAL_HIDDEN INLINE void populate_zone_cache(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN INLINE void flush_chunk_quarantine(void);
INTERNAL_HIDDEN INLINE void clear_zone_cache(void);
INTERNAL_HIDDEN iso_alloc_big_zone_t *iso_find_big_zone(void *p, bool remove);
INTERNAL_HIDDEN iso_alloc_zone_t *is_zone_usable(iso_alloc_zone_t *zone, size_t size);
INTERNAL_HIDDEN iso_alloc_zone_t *find_suitable_zone(size_t size);
INTERNAL_HIDDEN iso_alloc_zone_t *iso_new_zone(size_t size, bool internal);
INTERNAL_HIDDEN iso_alloc_zone_t *_iso_new_zone(size_t size, bool internal, int32_t index);
INTERNAL_HIDDEN iso_alloc_zone_t *iso_find_zone_bitmap_range(const void *p);
INTERNAL_HIDDEN iso_alloc_zone_t *iso_find_zone_range(void *p);
INTERNAL_HIDDEN iso_alloc_zone_t *search_chunk_lookup_table(const void *p);
INTERNAL_HIDDEN bit_slot_t iso_scan_zone_free_slot_slow(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN bit_slot_t iso_scan_zone_free_slot(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN bit_slot_t get_next_free_bit_slot(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN iso_alloc_root *iso_alloc_new_root(void);
INTERNAL_HIDDEN bool is_pow2(uint64_t sz);
INTERNAL_HIDDEN bool _is_zone_retired(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN bool _refresh_zone_mem_tags(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN iso_alloc_zone_t *_iso_free_internal_unlocked(void *p, bool permanent, iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void fill_free_bit_slots(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void flush_caches(void);
INTERNAL_HIDDEN void iso_free_chunk_from_zone(iso_alloc_zone_t *zone, void *p, bool permanent);
INTERNAL_HIDDEN void create_canary_chunks(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void iso_alloc_initialize_global_root(void);
INTERNAL_HIDDEN void _iso_alloc_destroy_zone_unlocked(iso_alloc_zone_t *zone, bool flush_caches, bool replace);
INTERNAL_HIDDEN void _iso_alloc_destroy_zone(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void _verify_zone(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void _verify_all_zones(void);
INTERNAL_HIDDEN void verify_zone(iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void verify_all_zones(void);
INTERNAL_HIDDEN void _iso_free(void *p, bool permanent);
INTERNAL_HIDDEN void _iso_free_internal(void *p, bool permanent);
INTERNAL_HIDDEN void _iso_free_size(void *p, size_t size);
INTERNAL_HIDDEN void _iso_free_from_zone(void *p, iso_alloc_zone_t *zone, bool permanent);
INTERNAL_HIDDEN void iso_free_big_zone(iso_alloc_big_zone_t *big_zone, bool permanent);
INTERNAL_HIDDEN void _iso_alloc_protect_root(void);
INTERNAL_HIDDEN void _iso_free_quarantine(void *p);
INTERNAL_HIDDEN void _iso_alloc_unprotect_root(void);
INTERNAL_HIDDEN INLINE void dont_need_pages(void *p, size_t size);
INTERNAL_HIDDEN void *_tag_ptr(void *p, iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void *_untag_ptr(void *p, iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void _free_big_zone_list(iso_alloc_big_zone_t *head);
INTERNAL_HIDDEN ASSUME_ALIGNED void *_iso_big_alloc(size_t size);
INTERNAL_HIDDEN ASSUME_ALIGNED void *_iso_alloc(iso_alloc_zone_t *zone, size_t size);
INTERNAL_HIDDEN ASSUME_ALIGNED void *_iso_alloc_bitslot_from_zone(bit_slot_t bitslot, iso_alloc_zone_t *zone);
INTERNAL_HIDDEN ASSUME_ALIGNED void *_iso_calloc(size_t nmemb, size_t size);
INTERNAL_HIDDEN void *_iso_alloc_ptr_search(void *n, bool poison);
INTERNAL_HIDDEN INLINE uint64_t us_rand_uint64(uint64_t *seed);
INTERNAL_HIDDEN INLINE uint64_t rand_uint64(void);
INTERNAL_HIDDEN uint8_t _iso_alloc_get_mem_tag(void *p, iso_alloc_zone_t *zone);
INTERNAL_HIDDEN void _iso_alloc_verify_tag(void *p, iso_alloc_zone_t *zone);
INTERNAL_HIDDEN size_t _iso_alloc_print_stats(void);
INTERNAL_HIDDEN size_t _iso_chunk_size(void *p);
INTERNAL_HIDDEN int64_t check_canary_no_abort(iso_alloc_zone_t *zone, const void *p);
INTERNAL_HIDDEN void _iso_alloc_initialize(void);
INTERNAL_HIDDEN void _iso_alloc_destroy(void);

#if ARM_MTE
INLINE void *iso_mte_untag_ptr(void *p);
INLINE uint8_t iso_mte_extract_tag(void *p);
INLINE bool iso_is_mte_supported(void);
INLINE void *iso_mte_set_tag_range(void *p, size_t size);
INLINE void *iso_mte_create_tag(void *p, uint64_t exclusion_mask);
INLINE void iso_mte_set_tag(void *p);
INLINE void *iso_mte_get_tag(void *p);
#endif

#if SIGNAL_HANDLER
INTERNAL_HIDDEN void handle_signal(int sig, siginfo_t *si, void *ctx);
#endif

#if EXPERIMENTAL
INTERNAL_HIDDEN void _iso_alloc_search_stack(uint8_t *stack_start);
#endif

#if UNIT_TESTING
EXTERNAL_API iso_alloc_root *_get_root(void);
#endif
