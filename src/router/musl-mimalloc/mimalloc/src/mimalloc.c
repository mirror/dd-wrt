/* The Chimera Linux unified mimalloc configuration. */

/* enable our changes */
#define MI_LIBC_BUILD 1
/* the libc malloc should not read any env vars */
#define MI_NO_GETENV 1
/* this is a hardened build */
#define MI_SECURE 4
/* this would be nice to have, but unfortunately it
 * makes some things a lot slower (e.g. sort(1) becomes
 * roughly 2.5x slower) so disable unless we figure out
 * some way to make it acceptable...
 */
#define MI_PADDING 0

/* use smaller segments to accommodate smaller arenas */
#define MI_SEGMENT_SHIFT (7 + MI_SEGMENT_SLICE_SHIFT)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#include <features.h>
/* small workaround for musl includes */
#ifdef weak
#undef weak
#endif

#include "pthread_impl.h"

/* since we are internal we can make syscalls more direct (via macros) */
#include "syscall.h"
#define madvise __madvise
#define MADV_DONTNEED POSIX_MADV_DONTNEED

/* some verification whether we can make a valid build */
#include "stdatomic.h"

#if ATOMIC_LONG_LOCK_FREE != 2 || ATOMIC_CHAR_LOCK_FREE != 2
#error Words and bytes must always be lock-free in this context
#endif

/* arena purge timing stuff (may fix later), stats (can patch out) */
#if ATOMIC_LLONG_LOCK_FREE != 2
#error 64-bit atomics must be lock-free for now
#endif

/* the whole mimalloc source */
#include "static.c"

/* chimera entrypoints */

#define INTERFACE __attribute__((visibility("default")))

extern int __malloc_replaced;
extern int __aligned_alloc_replaced;

void * const __malloc_tls_default = (void *)&_mi_heap_empty;

void __malloc_init(pthread_t p) {
    mi_process_load();
}

void __malloc_tls_teardown(pthread_t p) {
    /* if we never allocated on it, don't do anything */
    if (p->malloc_tls == (void *)&_mi_heap_empty)
        return;
    /* otherwise finalize the thread and reset */
    _mi_thread_done(p->malloc_tls);
    p->malloc_tls = (void *)&_mi_heap_empty;
}

/* we have nothing to do here, mimalloc is lock-free */
void __malloc_atfork(int who) {
    if (who < 0) {
        /* disable */
    } else {
        /* enable */
    }
}

/* we have no way to implement this AFAICT */
void __malloc_donate(char *a, char *b) { (void)a; (void)b; }

void *__libc_calloc(size_t m, size_t n) {
    return mi_calloc(m, n);
}

void __libc_free(void *ptr) {
    mi_free(ptr);
}

void *__libc_malloc_impl(size_t len) {
    return mi_malloc(len);
}

void *__libc_realloc(void *ptr, size_t len) {
    return mi_realloc(ptr, len);
}

/* technically mi_aligned_alloc and mi_memalign are the same in mimalloc
 * which is good for us because musl implements memalign with aligned_alloc
 */
INTERFACE void *aligned_alloc(size_t align, size_t len) {
    if (mi_unlikely(__malloc_replaced && !__aligned_alloc_replaced)) {
        errno = ENOMEM;
        return NULL;
    }
    void *p = mi_malloc_aligned(len, align);
    mi_assert_internal(((uintptr_t)p % align) == 0);
    return p;
}

INTERFACE size_t malloc_usable_size(void *p) {
    return mi_usable_size(p);
}
