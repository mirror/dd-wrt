/* iso_alloc_internal.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

/* Modifying the values in this configuration header
 * can significantly improve the performance of your
 * workload, or the security of your runtime. Please
 * read the comments and documentation carefully before
 * modifying these values as many of them are core to
 * how the underlying memory allocator functions */

/* This controls what % of chunks are canaries in a
 * zone. For example, if a zone holds 128 byte chunks
 * then it has (ZONE_USER_SIZE / 128) = 32768 total
 * chunks available for it. The number of canaries is
 * calculated as (32768 / CANARY_COUNT_DIV) = 327.
 * When CANARY_COUNT_DIV = 7 we set aside < %1 of user
 * chunks as canaries because we right shift zone
 * chunk count by this value, e.g. (65535 >> 7 = 511) */
#define CANARY_COUNT_DIV 7

/* The minimum number of free bit slots needed in the
 * cache in order to randomize it. RANDOMIZE_FREELIST
 * must be enabled for this to be used */
#define MIN_RAND_FREELIST 4

/* If you're compiling for Android and want custom names
 * for internal mappings that are viewable from procfs
 * (i.e. /proc/pid/maps) you can modify those names here */
#if NAMED_MAPPINGS
#define SAMPLED_ALLOC_NAME "isoalloc sampled allocation"
#define BIG_ZONE_UD_NAME "isoalloc big zone user data"
#define BIG_ZONE_MD_NAME "isoalloc big zone metadata"
#define GUARD_PAGE_NAME "guard page"
#define ROOT_NAME "isoalloc root"
#define ZONE_BITMAP_NAME "isoalloc zone bitmap"
#define INTERNAL_UZ_NAME "internal isoalloc user zone"
#define PRIVATE_UZ_NAME "private isoalloc user zone"
#define MEM_TAG_NAME "isoalloc zone mem tags"
#define PREALLOC_BITMAPS "isoalloc small bitmaps"
#endif

/* If you're using the UAF_PTR_PAGE functionality and
 * want to change the frequency it is triggered or the
 * magic value that is written */
#if UAF_PTR_PAGE
#define UAF_PTR_PAGE_ODDS 1000000
#endif

/* Zones can be retired after a certain number of
 * allocations. This is computed as the total count
 * of chunks the zone can hold multiplied by this
 * value. The zone is replaced at that point if all
 * of its current chunks are free */
#define ZONE_ALLOC_RETIRE 32

/* This byte value will overwrite the contents
 * of all free'd user chunks if -DSANITIZE_CHUNKS
 * is enabled in the Makefile. The value is completely
 * arbitrary, but non-zero since this could mask
 * some bugs. */
#define POISON_BYTE 0xde

/* See PERFORMANCE.md for notes on huge page sizes.
 * If your system uses a non-default value for huge
 * page sizes you will need to adjust that here */
#if(__linux__ && MAP_HUGETLB) || (__APPLE__ && VM_FLAGS_SUPERPAGE_SIZE_2MB) || (__FreeBSD__ && MAP_HUGETLB) && HUGE_PAGES
#define HUGE_PAGE_SZ 2097152
#endif

/* Size of the zone cache documented in PERFORMANCE.md */
#define ZONE_CACHE_SZ 8

/* Size of the chunk quarantine cache documented in PERFORMANCE.md */
#define CHUNK_QUARANTINE_SZ 64

/* This is the maximum number of zones iso_alloc can
 * create. This is a completely arbitrary number but
 * it does correspond to the size of the _root.zones
 * array. Currently the iso_alloc_zone_t structure is
 * roughly 2112 bytes so this results in 17301504 bytes
 * (~17 MB) for zone meta data. See PERFORMANCE.md for
 * more information on this value. Max is 65535 */
#define MAX_ZONES 8192

/* Anything above this size will need to go through the
 * big zone path. Maximum value here is 131072 due to how
 * we construct the zone bitmap. You can think of this
 * value as roughly equivalent to M_MMAP_THRESHOLD. Valid
 * values for SMALL_SIZE_MAX are powers of 2 through 131072 */
#define SMALL_SIZE_MAX 65536

/* Big zones are for any allocation bigger than SMALL_SIZE_MAX.
 * We reuse them when possible but not if the reuse would
 * result in unused memory that exceeds this value */
#define BIG_ZONE_WASTE 8192

/* The maximum number of big zone free list entries.
 * We want to limit the number of these because they
 * are often backed by a large number of pages */
#define BIG_ZONE_MAX_FREE_LIST 64

/* Big zones can be retired after a certain number of
 * allocations. Big zones pages will be unmapped and
 * not added to the free list after being used N times */
#define BIG_ZONE_ALLOC_RETIRE 16

/* We allocate zones at startup for common sizes.
 * Each of these default zones is 4mb (ZONE_USER_SIZE)
 * so ZONE_8192 would hold less chunks than ZONE_128 */
#define ZONE_16 16
#define ZONE_32 32
#define ZONE_64 64
#define ZONE_128 128
#define ZONE_256 256
#define ZONE_512 512
#define ZONE_1024 1024
#define ZONE_2048 2048
#define ZONE_4096 4096
#define ZONE_8192 8192

/* Default zones should ideally never be above 8192
 * bytes in size. This is because the allocator makes
 * certain decisions based on this value such as the
 * number of canary values in a zone. It is safe to
 * modify to a larger value but you will likely be
 * wasting memory by doing so. */
#define MAX_DEFAULT_ZONE_SZ ZONE_8192

/* If you have specific allocation pattern requirements
 * then you may want a custom set of default zones. These
 * example are provided to get you started. Zone creation
 * at runtime is *not* limited to these sizes, this defines
 * the default zones that will be created at startup time.
 * Each of these examples is 4 default zones which will
 * consume 16 mb of memory for user chunks, plus 2 guard
 * pages per zone, a bitmap, and 2 guard pages per bitmap.
 * You also need to define SMALLEST_CHUNK_SZ which should
 * correspond to the smallest value in your default_zones
 * array. It's value should never be less than 16 */
#if SMALL_MEM_STARTUP
/* ZONE_USER_SIZE * sizeof(default_zones) = ~16 mb */
/* SZ_ALIGNMENT = 32 */
#define SMALLEST_CHUNK_SZ SZ_ALIGNMENT
const static uint64_t default_zones[] = {ZONE_64, ZONE_256, ZONE_512, ZONE_1024};
#else
/* ZONE_USER_SIZE * sizeof(default_zones) = ~40 mb */
#define SMALLEST_CHUNK_SZ SZ_ALIGNMENT
const static uint64_t default_zones[] = {ZONE_32, ZONE_64, ZONE_128, ZONE_256, ZONE_512,
                                         ZONE_1024, ZONE_2048, ZONE_4096, ZONE_8192};
#endif

/* Additional default zone example configurations are below */

#if 0
/* Only small allocations between 16 and 28 bytes are expected */
#define SMALLEST_CHUNK_SZ ZONE_16
static uint64_t default_zones[] = {ZONE_32, ZONE_32, ZONE_64, ZONE_64, ZONE_128, ZONE_128};
#endif

#if 0
/* Large allocations but smaller than a page */
#define SMALLEST_CHUNK_SZ ZONE_512
static uint64_t default_zones[] = {ZONE_512, ZONE_1024, ZONE_2048, ZONE_4096};
#endif
