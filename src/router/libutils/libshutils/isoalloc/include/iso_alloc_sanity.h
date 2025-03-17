/* iso_alloc_sanity.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once

#include "iso_alloc_util.h"

#if ALLOC_SANITY
#if UNINIT_READ_SANITY
#include <fcntl.h>
#include <linux/userfaultfd.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <sys/syscall.h>
#endif

#define SANITY_SAMPLE_ODDS 10000
#define MAX_SANE_SAMPLES 1024
#define SANE_CACHE_SIZE 65535
#define SANE_CACHE_IDX(p) (((uint64_t) p >> 8) & 0xffff)
#define SANITY_CANARY_VALIDATE_MASK 0xffffffffffffff00
#define SANITY_CANARY_SIZE 8

#if THREAD_SUPPORT
#if USE_SPINLOCK
extern atomic_flag sane_cache_flag;
#define LOCK_SANITY_CACHE() \
    do {                    \
    } while(atomic_flag_test_and_set(&sane_cache_flag));

#define UNLOCK_SANITY_CACHE() \
    atomic_flag_clear(&sane_cache_flag);
#else
extern pthread_mutex_t sane_cache_mutex;
#define LOCK_SANITY_CACHE() \
    pthread_mutex_lock(&sane_cache_mutex);

#define UNLOCK_SANITY_CACHE() \
    pthread_mutex_unlock(&sane_cache_mutex);
#endif
#else
#define LOCK_SANITY_CACHE()
#define UNLOCK_SANITY_CACHE()
#endif

#if UNINIT_READ_SANITY
extern pthread_t _page_fault_thread;
extern struct uffdio_api _uffd_api;
extern int64_t _uf_fd;
#endif

extern int32_t _sane_sampled;
extern uint8_t _sane_cache[SANE_CACHE_SIZE];

typedef struct {
    void *address;
    size_t orig_size;
    bool right_aligned;
} _sane_allocation_t;

extern _sane_allocation_t _sane_allocations[MAX_SANE_SAMPLES];
extern uint64_t _sanity_canary;

#if UNINIT_READ_SANITY
INTERNAL_HIDDEN void _iso_alloc_setup_userfaultfd(void);
INTERNAL_HIDDEN void *_page_fault_thread_handler(void *uf_fd);
#endif

INTERNAL_HIDDEN INLINE void write_sanity_canary(void *p);
INTERNAL_HIDDEN INLINE void check_sanity_canary(_sane_allocation_t *sane_alloc);
INTERNAL_HIDDEN void *_iso_alloc_sample(const size_t size);
INTERNAL_HIDDEN int32_t _iso_alloc_free_sane_sample(void *p);
INTERNAL_HIDDEN int32_t _remove_from_sane_trace(void *p);
INTERNAL_HIDDEN _sane_allocation_t *_get_sane_alloc(void *p);
#endif

INTERNAL_HIDDEN INLINE void *__iso_memcpy(void *dest, const void *src, size_t n);
INTERNAL_HIDDEN void *_iso_alloc_memcpy(void *dest, const void *src, size_t n);
INTERNAL_HIDDEN INLINE void *__iso_memmove(void *dest, const void *src, size_t n);
INTERNAL_HIDDEN void *_iso_alloc_memmove(void *dest, const void *src, size_t n);
INTERNAL_HIDDEN INLINE void *__iso_memset(void *dest, int b, size_t n);
INTERNAL_HIDDEN void *_iso_alloc_memset(void *dest, int b, size_t n);
