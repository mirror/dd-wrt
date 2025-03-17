/* iso_alloc.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#if !__aarch64__ && !__x86_64__
#pragma error "IsoAlloc is untested and unsupported on 32 bit platforms"
#endif

static_assert(sizeof(size_t) == 8, "IsoAlloc requires 64 bit size_t");

#ifndef EXTERNAL_API
#define EXTERNAL_API __attribute__((visibility("default")))
#endif

#define NO_DISCARD __attribute__((warn_unused_result))

#define MALLOC_ATTR __attribute__((malloc))
#define ALLOC_SIZE __attribute__((alloc_size(1)))
#define CALLOC_SIZE __attribute__((alloc_size(1, 2)))
#define REALLOC_SIZE __attribute__((alloc_size(2)))
#define ZONE_ALLOC_SIZE __attribute__((alloc_size(2)))
#define ASSUME_ALIGNED __attribute__((assume_aligned(8)))

#if MASK_PTRS
#define UNMASK_ZONE_HANDLE(zone) \
    zone = (iso_alloc_zone_handle *) ((uintptr_t) zone ^ (uintptr_t) _root->zone_handle_mask);
#else
#define UNMASK_ZONE_HANDLE(zone) zone = zone;
#endif

typedef void iso_alloc_zone_handle;

#if CPP_SUPPORT
extern "C" {
#endif

/* See https://github.com/struct/isoalloc/blob/master/README.md#api for
 * detailed information on how to use these functions */
EXTERNAL_API void iso_alloc_initialize(void);
EXTERNAL_API void iso_alloc_destroy(void);
EXTERNAL_API NO_DISCARD MALLOC_ATTR ALLOC_SIZE ASSUME_ALIGNED void *iso_alloc(size_t size);
EXTERNAL_API NO_DISCARD MALLOC_ATTR CALLOC_SIZE ASSUME_ALIGNED void *iso_calloc(size_t nmemb, size_t size);
EXTERNAL_API NO_DISCARD MALLOC_ATTR REALLOC_SIZE ASSUME_ALIGNED void *iso_realloc(void *p, size_t size);
EXTERNAL_API NO_DISCARD MALLOC_ATTR REALLOC_SIZE ASSUME_ALIGNED void *iso_reallocarray(void *p, size_t nmemb, size_t size);
EXTERNAL_API void iso_free(void *p);
EXTERNAL_API void iso_free_size(void *p, size_t size);
EXTERNAL_API void iso_free_permanently(void *p);
EXTERNAL_API void iso_free_from_zone(void *p, iso_alloc_zone_handle *zone);
EXTERNAL_API void iso_free_from_zone_permanently(void *p, iso_alloc_zone_handle *zone);
EXTERNAL_API size_t iso_chunksz(void *p);
EXTERNAL_API void iso_alloc_verify_ptr_tag(void *p, iso_alloc_zone_handle *zone);
EXTERNAL_API NO_DISCARD uint8_t iso_alloc_get_mem_tag(void *p, iso_alloc_zone_handle *zone);
EXTERNAL_API NO_DISCARD ASSUME_ALIGNED char *iso_strdup(const char *str);
EXTERNAL_API NO_DISCARD ASSUME_ALIGNED char *iso_strdup_from_zone(iso_alloc_zone_handle *zone, const char *str);
EXTERNAL_API NO_DISCARD ASSUME_ALIGNED char *iso_strndup(const char *str, size_t n);
EXTERNAL_API NO_DISCARD ASSUME_ALIGNED char *iso_strndup_from_zone(iso_alloc_zone_handle *zone, const char *str, size_t n);
EXTERNAL_API NO_DISCARD MALLOC_ATTR ASSUME_ALIGNED void *iso_alloc_from_zone(iso_alloc_zone_handle *zone);
EXTERNAL_API NO_DISCARD MALLOC_ATTR void *iso_alloc_from_zone_tagged(iso_alloc_zone_handle *zone);
EXTERNAL_API NO_DISCARD void *iso_alloc_tag_ptr(void *p, iso_alloc_zone_handle *zone);
EXTERNAL_API NO_DISCARD void *iso_alloc_untag_ptr(void *p, iso_alloc_zone_handle *zone);
EXTERNAL_API NO_DISCARD iso_alloc_zone_handle *iso_alloc_new_zone(size_t size);
EXTERNAL_API NO_DISCARD size_t iso_zone_chunk_count(iso_alloc_zone_handle *zone);
EXTERNAL_API void iso_alloc_destroy_zone(iso_alloc_zone_handle *zone);
EXTERNAL_API void iso_alloc_protect_root(void);
EXTERNAL_API void iso_alloc_unprotect_root(void);
EXTERNAL_API uint64_t iso_alloc_detect_zone_leaks(iso_alloc_zone_handle *zone);
EXTERNAL_API uint64_t iso_alloc_detect_leaks(void);
EXTERNAL_API uint64_t iso_alloc_zone_mem_usage(iso_alloc_zone_handle *zone);
EXTERNAL_API uint64_t iso_alloc_mem_usage(void);
EXTERNAL_API void iso_verify_zones(void);
EXTERNAL_API void iso_verify_zone(iso_alloc_zone_handle *zone);
EXTERNAL_API int32_t iso_alloc_name_zone(iso_alloc_zone_handle *zone, char *name);
EXTERNAL_API void iso_flush_caches(void);

#if HEAP_PROFILER
#define BACKTRACE_DEPTH 8

typedef struct {
    /* The address of the last 8 callers as referenced by stack frames */
    uint64_t callers[BACKTRACE_DEPTH];
    /* The smallest allocation size requested by this call path */
    size_t lower_bound_size;
    /* The largest allocation size requested by this call path */
    size_t upper_bound_size;
    /* A 16 bit hash of the back trace */
    uint16_t backtrace_hash;
    /* Call count */
    size_t call_count;
} iso_alloc_traces_t;

typedef struct {
    /* The address of the last 8 callers as referenced by stack frames */
    uint64_t callers[BACKTRACE_DEPTH];
    /* A 16 bit hash of the back trace */
    uint16_t backtrace_hash;
    /* Call count */
    size_t call_count;
} iso_free_traces_t;

EXTERNAL_API size_t iso_get_alloc_traces(iso_alloc_traces_t *traces_out);
EXTERNAL_API size_t iso_get_free_traces(iso_free_traces_t *traces_out);
EXTERNAL_API void iso_alloc_reset_traces(void);
#endif

#if EXPERIMENTAL
EXTERNAL_API void iso_alloc_search_stack(void *p);
#endif

#if CPP_SUPPORT
}
#endif
