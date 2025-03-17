/* libc_hook.c - Provides low level hooks for libc functions
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"
#include "iso_alloc_sanity.h"
#include "iso_alloc_util.h"

#if MEMCPY_SANITY
EXTERNAL_API void *memcpy(void *restrict dest, const void *restrict src, size_t n) {
    return _iso_alloc_memcpy(dest, src, n);
}

EXTERNAL_API void *memmove(void *dest, const void *src, size_t n) {
    return _iso_alloc_memmove(dest, src, n);
}
#endif

#if MEMSET_SANITY
EXTERNAL_API void *memset(void *dest, int b, size_t n) {
    return _iso_alloc_memset(dest, b, n);
}

/*
 * bzero is removed from the POSIX standard in IEEE Std 1003.1-2008, but still a valid *BSD extension
 */
#if(__FreeBSD__ || __NetBSD__ || __OpenBSD__ || __DragonFly__)
EXTERNAL_API void bzero(void *dest, size_t n) {
    (void) _iso_alloc_memset(dest, 0, n);
}
#endif
#endif
