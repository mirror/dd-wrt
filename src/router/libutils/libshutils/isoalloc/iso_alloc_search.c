/* iso_alloc_search.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"

/* Search all zones for either the first instance of a pointer
 * value and return it or overwrite the first potentially
 * dangling pointer with the address of an unmapped page */
INTERNAL_HIDDEN void *_iso_alloc_ptr_search(void *n, bool poison) {
    uint8_t *search = NULL;
    uint8_t *end = NULL;
    const size_t zones_used = _root->zones_used;

#if MEMORY_TAGGING || (ARM_MTE == 1)
    /* It should be safe to clear these upper bits even
     * if the pointer wasn't returned by IsoAlloc. */
    n = (void *) ((uintptr_t) n & TAGGED_PTR_MASK);
#endif

    for(int32_t i = 0; i < zones_used; i++) {
        iso_alloc_zone_t *zone = &_root->zones[i];

        search = UNMASK_USER_PTR(zone);
        end = search + ZONE_USER_SIZE;

        while(search <= (uint8_t *) (end - sizeof(uint64_t))) {
            if(LIKELY((uint64_t) * (uint64_t *) search != (uint64_t) n)) {
                search++;
            } else {
                if(poison == false) {
                    return search;
                } else {
#if UAF_PTR_PAGE
                    *(uint64_t *) search = (uint64_t) (_root->uaf_ptr_page);
                    return search;
#endif
                }
            }
        }
    }

    return NULL;
}

#if EXPERIMENTAL
/* These functions are all experimental and subject to change */

/* Search the stack for pointers into IsoAlloc zones. If
 * stack_start is NULL then this function starts searching
 * from the environment variables which should be mapped
 * just below the stack */
INTERNAL_HIDDEN void _iso_alloc_search_stack(uint8_t *stack_start) {
    if(stack_start == NULL) {
        stack_start = (uint8_t *) ENVIRON;

        if(stack_start == NULL) {
            return;
        }
    }

    /* The end of our stack is the address of this local */
    uint8_t *stack_end;
    stack_end = (uint8_t *) &stack_end;
    const uint64_t tps = UINT32_MAX;

    uint8_t *current = stack_start;
    uint64_t max_ptr = 0x800000000000;

    while(current > stack_end) {
        /* Iterating through zones is expensive so this quickly
         * decides on values that are unlikely to be pointers
         * into zone user pages */
        if(*(int64_t *) current <= tps || *(int64_t *) current >= max_ptr || (*(int64_t *) current & 0xffffff) == 0) {
            // LOG("Ignoring pointer start=%p end=%p stack_ptr=%p value=%lx", stack_start, stack_end, current, *(int64_t *)current);
            current--;
            continue;
        }

        uint64_t *p = (uint64_t *) *(int64_t *) current;
        iso_alloc_zone_t *zone = iso_find_zone_range(p);
        current--;

        if(zone != NULL) {
            UNMASK_ZONE_PTRS(zone);

            /* Ensure the pointer is properly aligned */
            if(UNLIKELY(IS_ALIGNED((uintptr_t) p) != 0)) {
                LOG_AND_ABORT("Chunk at 0x%p of zone[%d] is not %d byte aligned", p, zone->index, SZ_ALIGNMENT);
            }

            uint64_t chunk_offset = (uint64_t) (p - (uint64_t *) zone->user_pages_start);
            LOG("zone[%d] user_pages_start=%p value=%p %lu %d", zone->index, zone->user_pages_start, p, chunk_offset, zone->chunk_size);

            /* Ensure the pointer is a multiple of chunk size */
            if(UNLIKELY((chunk_offset % zone->chunk_size) != 0)) {
                LOG("Chunk at %p is not a multiple of zone[%d] chunk size %d. Off by %" PRIu64 " bits", p, zone->index, zone->chunk_size, (chunk_offset % zone->chunk_size));
                MASK_ZONE_PTRS(zone);
                continue;
            }

            size_t chunk_number = (chunk_offset / zone->chunk_size);
            bit_slot_t bit_slot = (chunk_number * BITS_PER_CHUNK);
            bit_slot_t dwords_to_bit_slot = (bit_slot / BITS_PER_QWORD);

            if(UNLIKELY((zone->bitmap_start + dwords_to_bit_slot) >= (zone->bitmap_start + zone->bitmap_size))) {
                LOG("Cannot calculate this chunks location in the bitmap %p", p);
                MASK_ZONE_PTRS(zone);
                continue;
            }

            int64_t which_bit = (bit_slot % BITS_PER_QWORD);
            bitmap_index_t *bm = (bitmap_index_t *) zone->bitmap_start;
            bitmap_index_t b = bm[dwords_to_bit_slot];

            if(UNLIKELY((GET_BIT(b, which_bit)) == 0)) {
                LOG("Chunk at %p is in-use", p);
            } else {
                LOG("Chunk at %p is free", p);
            }

            MASK_ZONE_PTRS(zone);
        }

        zone = iso_find_zone_bitmap_range(p);

        if(zone != NULL) {
            LOG_AND_ABORT("Pointer to bitmap for zone[%d] found in stack @ %p", zone->index, p);
        }
    }
}
#endif
