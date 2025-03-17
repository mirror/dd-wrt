/* iso_alloc_util.h - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#pragma once
#include "compiler.h"

#if !NAMED_MAPPINGS
#define SAMPLED_ALLOC_NAME ""
#define BIG_ZONE_UD_NAME ""
#define BIG_ZONE_MD_NAME ""
#define GUARD_PAGE_NAME ""
#define ROOT_NAME ""
#define ZONE_BITMAP_NAME ""
#define INTERNAL_UZ_NAME ""
#define PRIVATE_UZ_NAME ""
#endif

#if USE_MLOCK
#define MLOCK(p, s) mlock(p, s)
#else
#define MLOCK(p, s)
#endif

INTERNAL_HIDDEN void *create_guard_page(void *p);
INTERNAL_HIDDEN INLINE void darwin_reuse(void *p, size_t size);
INTERNAL_HIDDEN void unmap_guarded_pages(void *p, size_t size);
INTERNAL_HIDDEN ASSUME_ALIGNED void *mmap_guarded_rw_pages(size_t size, bool populate, const char *name);
INTERNAL_HIDDEN ASSUME_ALIGNED void *mmap_guarded_rw_mte_pages(size_t size, bool populate, const char *name);
INTERNAL_HIDDEN ASSUME_ALIGNED void *mmap_rw_pages(size_t size, bool populate, const char *name);
INTERNAL_HIDDEN ASSUME_ALIGNED void *mmap_rw_mte_pages(size_t size, bool populate, const char *name);
INTERNAL_HIDDEN ASSUME_ALIGNED void *mmap_pages(size_t size, bool populate, const char *name, int32_t prot);
INTERNAL_HIDDEN void mprotect_pages(void *p, size_t size, int32_t protection);
INTERNAL_HIDDEN int32_t name_mapping(void *p, size_t sz, const char *name);
INTERNAL_HIDDEN size_t next_pow2(size_t sz);
INTERNAL_HIDDEN uint32_t _log2(uint32_t v);

INTERNAL_HIDDEN int8_t *_fmt(uint64_t n, uint32_t base);
INTERNAL_HIDDEN void _iso_alloc_printf(int32_t fd, const char *f, ...);

#if CPU_PIN
INTERNAL_HIDDEN INLINE int _iso_getcpu(void);
#endif
