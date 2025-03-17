/* iso_alloc_interfaces.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc.h"
#include "iso_alloc_internal.h"
#include "iso_alloc_ds.h"
#include "iso_alloc_sanity.h"

#if HEAP_PROFILER
#include "iso_alloc_profiler.h"
#endif

EXTERNAL_API FLATTEN void iso_alloc_initialize(void) {
    _iso_alloc_initialize();
}

EXTERNAL_API FLATTEN void iso_alloc_destroy(void) {
    _iso_alloc_destroy();
}

EXTERNAL_API NO_DISCARD FLATTEN MALLOC_ATTR ALLOC_SIZE ASSUME_ALIGNED void *iso_alloc(size_t size) {
    return _iso_alloc(NULL, size);
}

EXTERNAL_API NO_DISCARD FLATTEN MALLOC_ATTR CALLOC_SIZE ASSUME_ALIGNED void *iso_calloc(size_t nmemb, size_t size) {
    return _iso_calloc(nmemb, size);
}

EXTERNAL_API FLATTEN void iso_free(void *p) {
    _iso_free(p, false);
}

EXTERNAL_API FLATTEN void iso_free_size(void *p, size_t size) {
    _iso_free_size(p, size);
}

EXTERNAL_API FLATTEN void iso_free_from_zone(void *p, iso_alloc_zone_handle *zone) {
    UNMASK_ZONE_HANDLE(zone);
    _iso_free_from_zone(p, zone, false);
}

EXTERNAL_API FLATTEN void iso_free_from_zone_permanently(void *p, iso_alloc_zone_handle *zone) {
    UNMASK_ZONE_HANDLE(zone);
    _iso_free_from_zone(p, zone, true);
}

EXTERNAL_API FLATTEN void iso_free_permanently(void *p) {
    _iso_free(p, true);
}

EXTERNAL_API FLATTEN size_t iso_chunksz(void *p) {
    return _iso_chunk_size(p);
}

EXTERNAL_API FLATTEN NO_DISCARD size_t iso_zone_chunk_count(iso_alloc_zone_handle *zone) {
    UNMASK_ZONE_HANDLE(zone);
    iso_alloc_zone_t *_zone = (iso_alloc_zone_t *) zone;
    size_t canaries = 0;

#if !DISABLE_CANARY
    canaries = _zone->chunk_count >> CANARY_COUNT_DIV;
#endif
    return (_zone->chunk_count - canaries);
}

EXTERNAL_API FLATTEN NO_DISCARD REALLOC_SIZE ASSUME_ALIGNED void *iso_realloc(void *p, size_t size) {
    if(size == 0) {
        iso_free(p);
        return NULL;
    }

    void *r = iso_alloc(size);

    if(r == NULL) {
        return r;
    }

    size_t chunk_size = iso_chunksz(p);

    if(size > chunk_size) {
        size = chunk_size;
    }

    if(p != NULL) {
        _iso_alloc_memcpy(r, p, size);
    }

#if PERM_FREE_REALLOC
    _iso_free(p, true);
#else
    _iso_free_size(p, chunk_size);
#endif

    return r;
}

EXTERNAL_API FLATTEN NO_DISCARD MALLOC_ATTR REALLOC_SIZE ASSUME_ALIGNED void *iso_reallocarray(void *p, size_t nmemb, size_t size) {
    unsigned int res;

    if(__builtin_mul_overflow(nmemb, size, &res)) {
        return NULL;
    }

    return iso_realloc(p, nmemb * size);
}

EXTERNAL_API FLATTEN NO_DISCARD ASSUME_ALIGNED char *iso_strdup(const char *str) {
    return iso_strdup_from_zone(NULL, str);
}

EXTERNAL_API FLATTEN NO_DISCARD ASSUME_ALIGNED char *iso_strdup_from_zone(iso_alloc_zone_handle *zone, const char *str) {
    if(str == NULL) {
        return NULL;
    }

    size_t size = strlen(str);

    if(zone != NULL) {
        UNMASK_ZONE_HANDLE(zone);
    }

    char *p = (char *) _iso_alloc(zone, size);

    if(p == NULL) {
        return NULL;
    }

    _iso_alloc_memcpy(p, str, size);
    return p;
}

EXTERNAL_API FLATTEN NO_DISCARD ASSUME_ALIGNED char *iso_strndup(const char *str, size_t n) {
    return iso_strndup_from_zone(NULL, str, n);
}

EXTERNAL_API FLATTEN NO_DISCARD ASSUME_ALIGNED char *iso_strndup_from_zone(iso_alloc_zone_handle *zone, const char *str, size_t n) {
    if(str == NULL) {
        return NULL;
    }

    size_t s_size = strlen(str);

    if(zone != NULL) {
        UNMASK_ZONE_HANDLE(zone);
    }

    char *p = (char *) _iso_alloc(zone, n);

    if(p == NULL) {
        return NULL;
    }

    if(s_size > n) {
        _iso_alloc_memcpy(p, str, n);
        p[n - 1] = '\0';
    } else {
        _iso_alloc_memcpy(p, str, s_size);
    }

    return p;
}

EXTERNAL_API FLATTEN NO_DISCARD MALLOC_ATTR ASSUME_ALIGNED void *iso_alloc_from_zone(iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return NULL;
    }

    UNMASK_ZONE_HANDLE(zone);
    iso_alloc_zone_t *_zone = (iso_alloc_zone_t *) zone;

    return _iso_alloc(zone, _zone->chunk_size);
}

EXTERNAL_API FLATTEN NO_DISCARD MALLOC_ATTR void *iso_alloc_from_zone_tagged(iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return NULL;
    }

    UNMASK_ZONE_HANDLE(zone);
    iso_alloc_zone_t *_zone = (iso_alloc_zone_t *) zone;

    void *p = _iso_alloc(zone, _zone->chunk_size);
    return _tag_ptr(p, zone);
}

EXTERNAL_API FLATTEN NO_DISCARD void *iso_alloc_tag_ptr(void *p, iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return NULL;
    }

    UNMASK_ZONE_HANDLE(zone);
    return _tag_ptr(p, zone);
}

EXTERNAL_API FLATTEN NO_DISCARD void *iso_alloc_untag_ptr(void *p, iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return NULL;
    }

    UNMASK_ZONE_HANDLE(zone);
    return _untag_ptr(p, zone);
}

EXTERNAL_API FLATTEN void iso_alloc_verify_ptr_tag(void *p, iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return;
    }

    UNMASK_ZONE_HANDLE(zone);
    _iso_alloc_verify_tag(p, zone);
    return;
}

EXTERNAL_API FLATTEN NO_DISCARD uint8_t iso_alloc_get_mem_tag(void *p, iso_alloc_zone_handle *zone) {
    if(zone == NULL || p == NULL) {
        return 0;
    }

    UNMASK_ZONE_HANDLE(zone);
    return _iso_alloc_get_mem_tag(p, zone);
}

EXTERNAL_API FLATTEN void iso_alloc_destroy_zone(iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return;
    }

    UNMASK_ZONE_HANDLE(zone);
    _iso_alloc_destroy_zone(zone);
}

EXTERNAL_API FLATTEN NO_DISCARD iso_alloc_zone_handle *iso_alloc_new_zone(size_t size) {
    iso_alloc_zone_handle *zone = (iso_alloc_zone_handle *) iso_new_zone(size, false);
    UNMASK_ZONE_HANDLE(zone);
    return zone;
}

EXTERNAL_API FLATTEN int32_t iso_alloc_name_zone(iso_alloc_zone_handle *zone, char *name) {
    if(zone == NULL) {
        return 0;
    } else {
        UNMASK_ZONE_HANDLE(zone);
    }

    iso_alloc_zone_t *_zone = (iso_alloc_zone_t *) zone;
    return name_mapping(_zone->user_pages_start, ZONE_USER_SIZE, name);
}

EXTERNAL_API FLATTEN void iso_alloc_protect_root(void) {
    _iso_alloc_protect_root();
}

EXTERNAL_API FLATTEN void iso_alloc_unprotect_root(void) {
    _iso_alloc_unprotect_root();
}

EXTERNAL_API FLATTEN uint64_t iso_alloc_detect_zone_leaks(iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return 0;
    } else {
        UNMASK_ZONE_HANDLE(zone);
    }

    return _iso_alloc_detect_leaks_in_zone(zone);
}

EXTERNAL_API FLATTEN uint64_t iso_alloc_detect_leaks(void) {
    return _iso_alloc_detect_leaks();
}

EXTERNAL_API FLATTEN uint64_t iso_alloc_zone_mem_usage(iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return 0;
    } else {
        UNMASK_ZONE_HANDLE(zone);
    }

    return _iso_alloc_zone_mem_usage(zone);
}

EXTERNAL_API FLATTEN uint64_t iso_alloc_mem_usage(void) {
    return _iso_alloc_mem_usage();
}

EXTERNAL_API FLATTEN void iso_verify_zones(void) {
    verify_all_zones();
}

EXTERNAL_API FLATTEN void iso_verify_zone(iso_alloc_zone_handle *zone) {
    if(zone == NULL) {
        return;
    } else {
        UNMASK_ZONE_HANDLE(zone);
    }

    verify_zone(zone);
}

EXTERNAL_API FLATTEN void iso_flush_caches(void) {
    flush_caches();
}

#if HEAP_PROFILER
EXTERNAL_API FLATTEN size_t iso_get_alloc_traces(iso_alloc_traces_t *traces_out) {
    return _iso_get_alloc_traces(traces_out);
}

EXTERNAL_API FLATTEN size_t iso_get_free_traces(iso_free_traces_t *traces_out) {
    return _iso_get_free_traces(traces_out);
}

EXTERNAL_API FLATTEN void iso_alloc_reset_traces(void) {
    _iso_alloc_reset_traces();
}
#endif

#if EXPERIMENTAL
EXTERNAL_API FLATTEN void iso_alloc_search_stack(void *p) {
    _iso_alloc_search_stack(p);
}
#endif
