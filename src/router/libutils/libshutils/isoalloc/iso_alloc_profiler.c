/* iso_alloc_profiler.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"

#if MEMCPY_SANITY || MEMSET_SANITY
#include "iso_alloc_sanity.h"
#endif

#if HEAP_PROFILER
#include "iso_alloc_profiler.h"
#endif

#include <dlfcn.h>

INTERNAL_HIDDEN uint64_t _iso_alloc_detect_leaks_in_zone(iso_alloc_zone_t *zone) {
    LOCK_ROOT();
    uint64_t leaks = _iso_alloc_zone_leak_detector(zone, false);
    UNLOCK_ROOT();
    return leaks;
}

INTERNAL_HIDDEN uint64_t _iso_alloc_mem_usage(void) {
    LOCK_ROOT();
    uint64_t mem_usage = __iso_alloc_mem_usage();
    mem_usage += _iso_alloc_big_zone_mem_usage();
    UNLOCK_ROOT();
    return mem_usage;
}

INTERNAL_HIDDEN uint64_t _iso_alloc_big_zone_mem_usage(void) {
    LOCK_BIG_ZONE_USED();
    uint64_t mem_usage = __iso_alloc_big_zone_mem_usage(_root->big_zone_used);
    UNLOCK_BIG_ZONE_USED();

    LOCK_BIG_ZONE_FREE();
    mem_usage += __iso_alloc_big_zone_mem_usage(_root->big_zone_free);
    UNLOCK_BIG_ZONE_FREE();

    return mem_usage;
}

INTERNAL_HIDDEN uint64_t _iso_alloc_zone_mem_usage(iso_alloc_zone_t *zone) {
    LOCK_ROOT();
    uint64_t zone_mem_usage = __iso_alloc_zone_mem_usage(zone);
    UNLOCK_ROOT();
    return zone_mem_usage;
}

#if DEBUG && MEM_USAGE
INTERNAL_HIDDEN size_t _iso_alloc_print_stats(void) {
    struct rusage _rusage = {0};

    int32_t ret = getrusage(RUSAGE_SELF, &_rusage);

    if(ret == ERR) {
        return ERR;
    }

#if __linux__ || __FreeBSD__
    LOG("RSS: %d (mb)", (_rusage.ru_maxrss / KILOBYTE_SIZE));
#elif __APPLE__
    LOG("RSS: %d (mb)", (_rusage.ru_maxrss / MEGABYTE_SIZE));
#endif
    LOG("Soft Page Faults: %d", _rusage.ru_minflt);
    LOG("Hard Page Faults: %d", _rusage.ru_majflt);
    return OK;
}
#endif

INTERNAL_HIDDEN uint64_t _iso_alloc_detect_leaks(void) {
    uint64_t total_leaks = 0;
    uint64_t big_leaks = 0;

    LOCK_ROOT();

    for(uint16_t i = 0; i < _root->zones_used; i++) {
        iso_alloc_zone_t *zone = &_root->zones[i];
        total_leaks += _iso_alloc_zone_leak_detector(zone, false);
    }

    UNLOCK_ROOT();
    LOCK_BIG_ZONE_USED();

    iso_alloc_big_zone_t *big = _root->big_zone_used;

    if(big != NULL) {
        big = UNMASK_BIG_ZONE_NEXT(_root->big_zone_used);
    }

    /* All allocations on the used list are 'leaked' */
    while(big != NULL) {
        big_leaks += big->size;
        LOG("Big zone leaked %lu bytes", big->size);

        if(big->next != NULL) {
            big = UNMASK_BIG_ZONE_NEXT(big->next);
        } else {
            big = NULL;
        }
    }

    UNLOCK_BIG_ZONE_USED();

    LOG("Total leaked in big zones: bytes (%lu) megabytes (%lu)", big_leaks, (big_leaks / MEGABYTE_SIZE));
    return total_leaks + big_leaks;
}

/* This is the built-in leak detector. It works by scanning
 * the bitmap for every allocated zone and looking for
 * uncleared bits. This does not search for references from
 * a root like a GC, so if you purposefully did not free a
 * chunk then expect it to show up as leaked! */
INTERNAL_HIDDEN uint64_t _iso_alloc_zone_leak_detector(iso_alloc_zone_t *zone, bool profile) {
    uint32_t in_use = 0;

#if LEAK_DETECTOR || HEAP_PROFILER
    if(zone == NULL) {
        return 0;
    }

    UNMASK_ZONE_PTRS(zone);

    bitmap_index_t *bm = (bitmap_index_t *) zone->bitmap_start;
    uint32_t was_used = 0;
    int64_t bms = zone->bitmap_size / sizeof(bitmap_index_t);

    for(bitmap_index_t i = 0; i < bms; i++) {
        for(int j = 0; j < BITS_PER_QWORD; j += BITS_PER_CHUNK) {

            if(bm[i] == 0) {
                continue;
            }

            int64_t bit = GET_BIT(bm[i], j);
            int64_t bit_two = GET_BIT(bm[i], (j + 1));

            /* Chunk was used but is now free */
            if(bit == 0 && bit_two == 1) {
                was_used++;
            }

            if(bit == 1) {
                /* Theres no difference between a leaked and previously
                 * used chunk (11) and a canary chunk (11). So in order
                 * to accurately report on leaks we need to verify the
                 * canary value. If it doesn't validate then we assume
                 * its a true leak and increment the in_use counter */
                bit_slot_t bit_slot = (i * BITS_PER_QWORD) + j;
                const void *leak = (zone->user_pages_start + ((bit_slot >> 1) * zone->chunk_size));

                if(bit_two == 1 && (check_canary_no_abort(zone, leak) != ERR)) {
                    continue;
                } else {
                    in_use++;

                    if(profile == false) {
                        LOG("Leaked chunk (%d) in zone[%d] of %d bytes detected at 0x%p (bit position = %d)", in_use, zone->index, zone->chunk_size, leak, bit_slot);
                    }
                }
            }
        }
    }

    if(profile == false) {
        LOG("Zone[%d] Total number of %d byte chunks(%d) used and free'd (%d) (%d percent), in use = %d", zone->index, zone->chunk_size, zone->chunk_count,
            was_used, (int32_t) ((float) was_used / zone->chunk_count) * 100, in_use);
    }

    MASK_ZONE_PTRS(zone);
#endif

#if HEAP_PROFILER
    /* When profiling this zone we want to capture
     * the total number of allocations both currently
     * in use and previously used by this zone */
    if(profile == true) {
        uint64_t total = (in_use + was_used);
        return (uint64_t) ((float) (total / zone->chunk_count) * 100.0);
    }
#endif
    return in_use;
}

INTERNAL_HIDDEN uint64_t __iso_alloc_zone_mem_usage(iso_alloc_zone_t *zone) {
    uint64_t mem_usage = 0;
    mem_usage += zone->bitmap_size;
    mem_usage += ZONE_USER_SIZE;
    LOG("Zone[%d] holds %d byte chunks. Total bytes (%lu), megabytes (%lu)", zone->index, zone->chunk_size,
        mem_usage, (mem_usage / MEGABYTE_SIZE));
    return (mem_usage / MEGABYTE_SIZE);
}

INTERNAL_HIDDEN uint64_t __iso_alloc_mem_usage(void) {
    uint64_t mem_usage = 0;

    for(uint16_t i = 0; i < _root->zones_used; i++) {
        iso_alloc_zone_t *zone = &_root->zones[i];
        mem_usage += zone->bitmap_size;
        mem_usage += ZONE_USER_SIZE;
        LOG("Zone[%d] holds %d byte chunks, megabytes (%d) next zone = %d, total allocations = %d, in use = %d", zone->index, zone->chunk_size,
            (ZONE_USER_SIZE / MEGABYTE_SIZE), zone->next_sz_index, zone->alloc_count, zone->af_count);
    }

    return (mem_usage / MEGABYTE_SIZE);
}

INTERNAL_HIDDEN uint64_t __iso_alloc_big_zone_mem_usage(iso_alloc_big_zone_t *head) {
    uint64_t mem_usage = 0;
    iso_alloc_big_zone_t *big = head;

    if(big != NULL) {
        big = UNMASK_BIG_ZONE_NEXT(head);
    }

    while(big != NULL) {
        size_t total_size = big->size + (g_page_size * BIG_ZONE_META_DATA_PAGE_COUNT);
        /* Meta data has 2 guard pages, user data has 2 guard pages */
#if BIG_ZONE_META_DATA_GUARD
        total_size += (g_page_size * 4);
#else
        total_size += (g_page_size * 2);
#endif
        LOG("Big Zone Total bytes mapped (%lu), megabytes (%lu)", big->size, (big->size / MEGABYTE_SIZE));
        mem_usage += total_size;
        if(big->next != NULL) {
            big = UNMASK_BIG_ZONE_NEXT(big->next);
        } else {
            big = NULL;
        }
    }

    if(mem_usage != 0) {
        LOG("Total megabytes allocated (%lu)", (mem_usage / MEGABYTE_SIZE));
        return (mem_usage / MEGABYTE_SIZE);
    } else {
        return 0;
    }
}

#if HEAP_PROFILER

/* Returns a documented data structure that can
 * be used to interpret allocation patterns */
INTERNAL_HIDDEN size_t _iso_get_alloc_traces(iso_alloc_traces_t *traces_out) {
    LOCK_ROOT();
    __iso_memcpy(traces_out, _alloc_bts, sizeof(iso_alloc_traces_t));
    size_t sz = _alloc_bts_count;
    UNLOCK_ROOT();
    return sz;
}

INTERNAL_HIDDEN size_t _iso_get_free_traces(iso_free_traces_t *traces_out) {
    LOCK_ROOT();
    __iso_memcpy(traces_out, _free_bts, sizeof(iso_free_traces_t));
    size_t sz = _free_bts_count;
    UNLOCK_ROOT();
    return sz;
}

INTERNAL_HIDDEN void _iso_alloc_reset_traces(void) {
    LOCK_ROOT();
    __iso_memset(_alloc_bts, 0x0, sizeof(_alloc_bts));
    __iso_memset(_free_bts, 0x0, sizeof(_free_bts));
    _alloc_bts_count = 0;
    _free_bts_count = 0;
    UNLOCK_ROOT();
}

#define UPDATE_BT_HASH(frame, hash)                         \
    if(__builtin_frame_address(frame)) {                    \
        hash ^= (uint64_t) __builtin_return_address(frame); \
    } else {                                                \
        return hash;                                        \
    }

INTERNAL_HIDDEN INLINE uint64_t _get_backtrace_hash(void) {
    uint64_t hash = 0;
    UPDATE_BT_HASH(1, hash);
    UPDATE_BT_HASH(2, hash);
    UPDATE_BT_HASH(3, hash);
    UPDATE_BT_HASH(4, hash);
    UPDATE_BT_HASH(5, hash);
    UPDATE_BT_HASH(6, hash);
    UPDATE_BT_HASH(7, hash);
    UPDATE_BT_HASH(8, hash);
    return hash;
}

#define SAVE_BACKTRACE_FRAME(frame, bts)                                          \
    if(__builtin_frame_address(frame)) {                                          \
        uint64_t r = (uint64_t) __builtin_return_address(frame);                  \
        if(r > 0x1000) {                                                          \
            bts->callers[frame - 1] = (uint64_t) __builtin_return_address(frame); \
        }                                                                         \
    } else {                                                                      \
        return;                                                                   \
    }

INTERNAL_HIDDEN INLINE void _save_alloc_backtrace(iso_alloc_traces_t *abts) {
    SAVE_BACKTRACE_FRAME(1, abts);
    SAVE_BACKTRACE_FRAME(2, abts);
    SAVE_BACKTRACE_FRAME(3, abts);
    SAVE_BACKTRACE_FRAME(4, abts);
    SAVE_BACKTRACE_FRAME(5, abts);
    SAVE_BACKTRACE_FRAME(6, abts);
    SAVE_BACKTRACE_FRAME(7, abts);
    SAVE_BACKTRACE_FRAME(8, abts);
    return;
}

INTERNAL_HIDDEN INLINE void _save_free_backtrace(iso_free_traces_t *fbts) {
    SAVE_BACKTRACE_FRAME(1, fbts);
    SAVE_BACKTRACE_FRAME(2, fbts);
    SAVE_BACKTRACE_FRAME(3, fbts);
    SAVE_BACKTRACE_FRAME(4, fbts);
    SAVE_BACKTRACE_FRAME(5, fbts);
    SAVE_BACKTRACE_FRAME(6, fbts);
    SAVE_BACKTRACE_FRAME(7, fbts);
    SAVE_BACKTRACE_FRAME(8, fbts);
    return;
}

INTERNAL_HIDDEN void _iso_output_profile() {
    _iso_alloc_printf(profiler_fd, "allocated=%d\n", _alloc_count);
    _iso_alloc_printf(profiler_fd, "alloc_sampled=%d\n", _alloc_sampled_count);
    _iso_alloc_printf(profiler_fd, "freed=%d\n", _free_count);
    _iso_alloc_printf(profiler_fd, "free_sampled=%d\n", _free_sampled_count);

    for(uint16_t i = 0; i < _root->zones_used; i++) {
        iso_alloc_zone_t *zone = &_root->zones[i];
        _zone_profiler_map[zone->chunk_size].total++;
    }

    for(size_t i = 0; i < _alloc_bts_count; i++) {
        iso_alloc_traces_t *abts = &_alloc_bts[i];
        _iso_alloc_printf(profiler_fd, "alloc_backtrace=%d,backtrace_hash=0x%x,calls=%d,lower_bound_size=%d,upper_bound_size=%d\n",
                          i, abts->backtrace_hash, abts->call_count, abts->lower_bound_size, abts->upper_bound_size);

        for(int j = 0; j < BACKTRACE_DEPTH; j++) {
            if(abts->callers[j] < 0x1000) {
                continue;
            }

            Dl_info dl;
            dladdr((void *) abts->callers[j], &dl);

            if(dl.dli_sname != NULL) {
                _iso_alloc_printf(profiler_fd, "\t0x%x -> %s %s\n", abts->callers[j], dl.dli_sname, dl.dli_fname);
            } else {
                _iso_alloc_printf(profiler_fd, "\t0x%x -> [?]\n", abts->callers[j]);
            }
        }
    }

    for(size_t i = 0; i < _free_bts_count; i++) {
        iso_free_traces_t *fbts = &_free_bts[i];
        _iso_alloc_printf(profiler_fd, "free_backtrace=%d,backtrace_hash=0x%x,calls=%d\n",
                          i, fbts->backtrace_hash, fbts->call_count);

        for(int j = 0; j < BACKTRACE_DEPTH; j++) {
            if(fbts->callers[j] < 0x1000) {
                continue;
            }

            Dl_info dl;
            dladdr((void *) fbts->callers[j], &dl);

            if(dl.dli_sname != NULL) {
                _iso_alloc_printf(profiler_fd, "\t0x%x -> %s %s\n", fbts->callers[j], dl.dli_sname, dl.dli_fname);
            } else {
                _iso_alloc_printf(profiler_fd, "\t0x%x -> [?]\n", fbts->callers[j]);
            }
        }
    }

    for(int i = 0; i < SMALL_SIZE_MAX; i++) {
        if(_zone_profiler_map[i].count != 0) {
            _iso_alloc_printf(profiler_fd, "%d,%d,%d\n", i, _zone_profiler_map[i].total, _zone_profiler_map[i].count);
        }
    }

    if(profiler_fd != ERR) {
        close(profiler_fd);
        profiler_fd = ERR;
    }
}

INTERNAL_HIDDEN void _iso_alloc_profile(size_t size) {
    _alloc_count++;

    /* Don't run the profiler on every allocation */
    if(LIKELY((us_rand_uint64(&_root->seed) % PROFILER_ODDS) != 1)) {
        return;
    }

    _alloc_sampled_count++;

    for(uint16_t i = 0; i < _root->zones_used; i++) {
        uint32_t used = 0;
        iso_alloc_zone_t *zone = &_root->zones[i];

        /* For the purposes of the profiler we don't care about
         * the differences between canary and leaked chunks.
         * So lets just use the full count */
        if(zone->is_full) {
            used = zone->chunk_count;
        } else {
            used = _iso_alloc_zone_leak_detector(zone, true);
        }

        used = (int32_t) ((float) (used / zone->chunk_count) * 100.0);

        if(used > CHUNK_USAGE_THRESHOLD) {
            _zone_profiler_map[zone->chunk_size].count++;
        }
    }

    if(_alloc_bts_count < BACKTRACE_DEPTH_SZ) {
        iso_alloc_traces_t *abts = NULL;
        uint16_t hash = (_get_backtrace_hash() & HG_SIZE);

        /* Take the backtrace hash and determine if its already been seen */
        for(size_t i = 0; i < _alloc_bts_count; i++) {
            if(_alloc_bts[i].backtrace_hash == hash) {
                abts = &_alloc_bts[i];

                if(abts->lower_bound_size == 0 || size < abts->lower_bound_size) {
                    abts->lower_bound_size = size;
                }

                if(abts->upper_bound_size == 0 || size > abts->upper_bound_size) {
                    abts->upper_bound_size = size;
                }

                abts->call_count++;
                break;
            }
        }

        /* We haven't seen this backtrace before */
        if(abts == NULL) {
            abts = &_alloc_bts[_alloc_bts_count];
            abts->backtrace_hash = hash;

            _save_alloc_backtrace(abts);
            _alloc_bts_count++;
        }
    }
}

INTERNAL_HIDDEN void _iso_free_profile(void) {
    _free_count++;

    /* Don't run the profiler on every allocation */
    if(LIKELY((us_rand_uint64(&_root->seed) % PROFILER_ODDS) != 1)) {
        return;
    }

    _free_sampled_count++;

    if(_free_bts_count < BACKTRACE_DEPTH_SZ) {
        iso_free_traces_t *fbts = NULL;
        uint16_t hash = (_get_backtrace_hash() & HG_SIZE);

        /* Take the backtrace hash and determine if its already been seen */
        for(size_t i = 0; i < _free_bts_count; i++) {
            if(_free_bts[i].backtrace_hash == hash) {
                fbts = &_free_bts[i];
                fbts->call_count++;
                break;
            }
        }

        /* We haven't seen this backtrace before */
        if(fbts == NULL) {
            fbts = &_free_bts[_free_bts_count];
            fbts->backtrace_hash = hash;

            _save_free_backtrace(fbts);
            _free_bts_count++;
        }
    }
}

INTERNAL_HIDDEN void _initialize_profiler(void) {
    /* We don't need thread safety for this file descriptor
     * as long as we guarantee to never use it if the root
     * is not locked */
    if(getenv(PROFILER_ENV_STR) != NULL) {
        profiler_fd = open(getenv(PROFILER_ENV_STR), O_RDWR | O_CREAT | O_SYNC, 0666);
    } else {
        profiler_fd = open(PROFILER_FILE_PATH, O_RDWR | O_CREAT | O_SYNC, 0666);
    }

    if(profiler_fd == ERR) {
        LOG_AND_ABORT("Cannot open file descriptor for %s", PROFILER_FILE_PATH);
    }
}
#endif
