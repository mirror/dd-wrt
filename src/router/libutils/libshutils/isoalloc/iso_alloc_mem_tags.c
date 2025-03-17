/* iso_alloc_mem_tags.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

/* This file contains the software only implementation of
 * of memory tagging. See MEMORY_TAGGING.md for more info */
#include "iso_alloc_internal.h"

/* Returns a tag for a pointer p, which must be untagged
 * when passed to this function */
INTERNAL_HIDDEN uint8_t _iso_alloc_get_mem_tag(void *p, iso_alloc_zone_t *zone) {
#if MEMORY_TAGGING
    void *user_pages_start = UNMASK_USER_PTR(zone);

    if(user_pages_start > p || (user_pages_start + ZONE_USER_SIZE) < p) {
        LOG_AND_ABORT("Cannot get tag for pointer %p with wrong zone %d %p - %p", p, zone->index, user_pages_start, user_pages_start + ZONE_USER_SIZE);
    }

    uint8_t *_mtp = (user_pages_start - g_page_size - ROUND_UP_PAGE(zone->chunk_count * MEM_TAG_SIZE));
    const uint64_t chunk_offset = (uint64_t) (p - user_pages_start);

    /* Ensure the pointer is a multiple of chunk size */
    if(UNLIKELY((chunk_offset & (zone->chunk_size - 1)) != 0)) {
        LOG_AND_ABORT("Chunk offset %d not an alignment of %d", chunk_offset, zone->chunk_size);
    }

    _mtp += (chunk_offset / zone->chunk_size);

    return *_mtp;
#else
    return 0;
#endif
}

INTERNAL_HIDDEN void _iso_alloc_verify_tag(void *p, iso_alloc_zone_t *zone) {
#if MEMORY_TAGGING
    if(UNLIKELY(p == NULL || zone == NULL)) {
        return;
    }

    void *untagged_p = (void *) ((uintptr_t) p & TAGGED_PTR_MASK);
    const uint64_t tag = _iso_alloc_get_mem_tag(untagged_p, zone);

    if(tag != ((uintptr_t) p & IS_TAGGED_PTR_MASK)) {
        LOG_AND_ABORT("Pointer %p has wrong tag %x, expected %x", p, ((uintptr_t) p & IS_TAGGED_PTR_MASK), tag);
    }
#endif
    return;
}

INTERNAL_HIDDEN void *_tag_ptr(void *p, iso_alloc_zone_t *zone) {
#if MEMORY_TAGGING
    if(UNLIKELY(p == NULL || zone == NULL)) {
        return NULL;
    }

    const uint64_t tag = _iso_alloc_get_mem_tag(p, zone);
    return (void *) ((tag << UNTAGGED_BITS) | (uintptr_t) p);
#else
    return p;
#endif
}

INTERNAL_HIDDEN void *_untag_ptr(void *p, iso_alloc_zone_t *zone) {
#if MEMORY_TAGGING
    if(UNLIKELY(p == NULL || zone == NULL)) {
        return NULL;
    }

    void *untagged_p = (void *) ((uintptr_t) p & TAGGED_PTR_MASK);
    const uint64_t tag = _iso_alloc_get_mem_tag(untagged_p, zone);
    return (void *) ((tag << UNTAGGED_BITS) ^ (uintptr_t) p);
#else
    return p;
#endif
}

INTERNAL_HIDDEN bool _refresh_zone_mem_tags(iso_alloc_zone_t *zone) {
#if MEMORY_TAGGING
    /* This implements a similar policy to zone retirement.
     * The only difference is that we refresh all tags at
     * %25 of the configured zone retirement age */
    if(UNLIKELY(zone->af_count == 0 && zone->alloc_count > (zone->chunk_count << _root->zone_retirement_shf)) >> 2) {
        size_t s = ROUND_UP_PAGE(zone->chunk_count * MEM_TAG_SIZE);
        uint64_t *_mtp = (zone->user_pages_start - g_page_size - s);
        size_t tms = s / sizeof(uint64_t);

        for(uint64_t o = 0; o < tms; o++) {
            _mtp[o] = us_rand_uint64(&_root->seed);
        }

        return true;
    }
#endif
    return false;
}
