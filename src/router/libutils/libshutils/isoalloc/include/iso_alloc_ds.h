/* iso_alloc_ds.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

/* This header contains the core data structures,
 * caches, and typedef used by the allocator */
#define ZONE_FREE_LIST_SZ 255

/* The size of this table scales with SMALL_SIZE_MAX. */
#define ZONE_LOOKUP_TABLE_SZ (SMALL_SIZE_MAX >> 4) * sizeof(uint32_t)
#define SZ_TO_ZONE_LOOKUP_IDX(size) size >> 4

#define CHUNK_TO_ZONE_TABLE_SZ (65535 * sizeof(uint16_t))
#define ADDR_TO_CHUNK_TABLE(p) (((uintptr_t) p >> 32) & 0xffff)

typedef int64_t bit_slot_t;
typedef int64_t bitmap_index_t;
typedef uint16_t zone_lookup_table_t;
typedef uint16_t chunk_lookup_table_t;

#if ZONE_FREE_LIST_SZ > 255
typedef uint16_t free_bit_slot_t;
#define FREE_LIST_SHF 16
#else
typedef uint8_t free_bit_slot_t;
#define FREE_LIST_SHF 8
#endif

typedef struct {
    void *user_pages_start;                       /* Start of the pages backing this zone */
    void *bitmap_start;                           /* Start of the bitmap */
    int64_t next_free_bit_slot;                   /* The last bit slot returned by get_next_free_bit_slot */
    bit_slot_t free_bit_slots[ZONE_FREE_LIST_SZ]; /* A cache of bit slots that point to freed chunks */
    uint64_t canary_secret;                       /* Each zone has its own canary secret */
    uint64_t pointer_mask;                        /* Each zone has its own pointer protection secret */
    bitmap_index_t max_bitmap_idx;                /* Max bitmap index for this bitmap */
    uint32_t chunk_size;                          /* Size of chunks managed by this zone */
    uint32_t bitmap_size;                         /* Size of the bitmap in bytes */
    uint32_t af_count;                            /* Increment/Decrement with each alloc/free operation */
    uint32_t chunk_count;                         /* Total number of chunks in this zone */
    uint32_t alloc_count;                         /* Total number of lifetime allocations */
    uint16_t index;                               /* Zone index */
    uint16_t next_sz_index;                       /* What is the index of the next zone of this size */
    free_bit_slot_t free_bit_slots_index;         /* Tracks how many entries in the cache are filled */
    free_bit_slot_t free_bit_slots_usable;        /* The oldest members of the free cache are served first */
    int8_t preallocated_bitmap_idx;               /* The bitmap is preallocated and its index */
#if CPU_PIN
    uint8_t cpu_core; /* What CPU core this zone is pinned to */
#endif
    bool internal; /* Zones can be managed by iso_alloc or private */
    bool is_full;  /* Flags whether this zone is full to avoid bit slot searches */
#if MEMORY_TAGGING
    bool tagged; /* Zone supports memory tagging */
#endif
} __attribute__((packed, aligned(sizeof(int64_t)))) iso_alloc_zone_t;

/* Meta data for big allocations are allocated near the
 * user pages themselves but separated via guard pages.
 * This meta data is stored at a random offset from the
 * beginning of the page it resides on */
typedef struct iso_alloc_big_zone_t {
    uint64_t canary_a;
    bool free;
    uint64_t size;
    uint32_t ttl;
    void *user_pages_start;
    struct iso_alloc_big_zone_t *next;
    uint64_t canary_b;
} __attribute__((packed, aligned(sizeof(int64_t)))) iso_alloc_big_zone_t;

#define BITMAP_SIZE_16 16
#define BITMAP_SIZE_32 32
#define BITMAP_SIZE_64 64
#define BITMAP_SIZE_128 128
#define BITMAP_SIZE_256 256
#define BITMAP_SIZE_512 512
#define BITMAP_SIZE_1024 1024

/* Small bitmap sizes are in reverse order as
 * smaller chunks are more likely to be needed.
 * The extra 0 is for alignment, we subtract it
 * when iterating _root->bitmaps */
const static int small_bitmap_sizes[] = {
    BITMAP_SIZE_1024,
    BITMAP_SIZE_512,
    BITMAP_SIZE_256,
    BITMAP_SIZE_128,
    BITMAP_SIZE_64,
    BITMAP_SIZE_32,
    BITMAP_SIZE_16,
    0};

/* Preallocated pages for bitmaps are managed using
 * an array of these structures placed in the root */
typedef struct {
    void *bitmap;
    /* Our bitmap has a bitmap */
    uint32_t in_use;
    /* Bucket value determines how many times we divy up
     * the bitmap page. eg For BITMAP_SIZE_16 its 256 times */
    uint32_t bucket;
} __attribute__((packed, aligned(sizeof(int64_t)))) iso_alloc_bitmap_t;

/* There is only one iso_alloc root per-process.
 * It contains an array of zone structures. Each
 * Zone represents a number of contiguous pages
 * that hold chunks containing caller data */
typedef struct {
    iso_alloc_zone_t *zones;
    /* The chunk to zone lookup table provides a high hit
     * rate cache for finding which zone owns a user chunk.
     * It works by mapping the MSB of the chunk addressq
     * to a zone index. Misses are gracefully handled and
     * more common with a higher RSS and more mappings. */
    chunk_lookup_table_t *chunk_lookup_table;
    uintptr_t *chunk_quarantine;
    iso_alloc_big_zone_t *big_zone_free;
    iso_alloc_big_zone_t *big_zone_used;
#if NO_ZERO_ALLOCATIONS
    void *zero_alloc_page;
#endif
#if UAF_PTR_PAGE
    void *uaf_ptr_page;
#endif
    /* Zones are linked by their next_sz_index member which
     * tells the allocator where in the _root->zones array
     * it can find the next zone that holds the same size
     * chunks. The lookup table helps us find the first zone
     * that holds a specific size in O(1) time */
    zone_lookup_table_t zone_lookup_table[ZONE_LOOKUP_TABLE_SZ];
    /* For chunk sizes >= 1024 our bitmap size is smaller
    * than a page. This optimization preallocates pages to
    * hold multiple bitmaps for these zones */
    iso_alloc_bitmap_t bitmaps[sizeof(small_bitmap_sizes) / sizeof(int)];
    uint64_t zone_handle_mask;
    uint64_t big_zone_next_mask;
    uint64_t big_zone_canary_secret;
    uint64_t seed;
    size_t chunk_quarantine_count;
    size_t zones_size;
#if THREAD_SUPPORT
#if USE_SPINLOCK
    atomic_flag big_zone_free_flag;
    atomic_flag big_zone_used_flag;
#else
    pthread_mutex_t big_zone_free_mutex;
    pthread_mutex_t big_zone_used_mutex;
#endif
#endif
    uint32_t zone_retirement_shf;
    int32_t big_zone_free_count;
    int32_t big_zone_used_count;
    uint16_t zones_used;
#if ARM_MTE
    bool arm_mte_enabled;
#endif
} __attribute__((aligned(sizeof(int64_t)))) iso_alloc_root;

typedef struct {
    void *user_pages_start;
    void *bitmap_start;
    uint32_t bitmap_size;
    uint8_t ttl;
} __attribute__((aligned(sizeof(int64_t)))) zone_quarantine_t;

/* Each thread gets a local cache of the most recently
 * used zones. This can greatly speed up allocations
 * if your threads are reusing the same zones. This
 * cache is first in last out, and is populated during
 * both alloc and free operations */
typedef struct {
    size_t chunk_size;
    iso_alloc_zone_t *zone;
} __attribute__((aligned(sizeof(int64_t)))) _tzc;
