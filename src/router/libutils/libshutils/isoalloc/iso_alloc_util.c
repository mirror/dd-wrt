/* iso_alloc_util.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"

#if CPU_PIN
/* sched_getcpu's performance depends on the
 * architecture/kernel version, so we lower
 * the cost of feature's abstraction here. */
int _iso_getcpu(void) {
#if defined(SCHED_GETCPU)
    return sched_getcpu();
#elif defined(__x86_64__)
    /* rdtscp is not always available and is pretty slow
     * we instead load from the global descriptor table
     * then "mov" it to 'a' */
    unsigned int a;
    const unsigned int cpunodesegment = 15 * 8 + 3;
    __asm__ volatile("lsl %1, %0"
                     : "=r"(a)
                     : "r"(cpunodesegment));
    return (int) (a & 0xfff);
#elif defined(__aarch64__)
#if __APPLE__
    /* unlike other operating systems, the tpidr_el0 register on macOs
     * is unused data stored for the current thread is instead fetchable
     * from "tpidrro_el0". */
    uintptr_t a;
    __asm__ volatile("mrs %x0, tpidrro_el0"
                     : "=r"(a)::"memory");
    return (int) ((a & 0x8));
#else
    /* TODO most likely different register/making on other platforms */
    return -1;
#endif
#else
    return -1;
#endif
}
#endif

void darwin_reuse(void *p, size_t size) {
#if __APPLE__
    while(madvise(p, size, MADV_FREE_REUSE) && errno == EAGAIN) {
    }
#endif
}

void *create_guard_page(void *p) {
    if(p == NULL) {
        p = mmap_rw_pages(g_page_size, false, NULL);

        if(p == NULL) {
            LOG_AND_ABORT("Could not allocate guard page");
        }
    }

    mprotect_pages(p, g_page_size, PROT_NONE);
    name_mapping(p, g_page_size, GUARD_PAGE_NAME);
    return p;
}

/* Assumes p is page aligned and surrounded by guard pages */
void unmap_guarded_pages(void *p, size_t size) {
    size_t sz = ROUND_UP_PAGE(size);
    munmap(p - g_page_size, sz + (g_page_size << 1));
}

/* Assumes size for guard pages is NOT accounted for.
 * Returns a pointer to a contiguous set of RW pages
 * with guard pages mapped on top and bottom. */
void *mmap_guarded_rw_pages(size_t size, bool populate, const char *name) {
    size_t sz = ROUND_UP_PAGE(size);

    if(sz < size) {
        return NULL;
    }

    void *p = mmap_rw_pages(sz + (g_page_size * 2), populate, name);

    create_guard_page(p);
    create_guard_page(p + (g_page_size + sz));
    return (p + g_page_size);
}

#if ARM_MTE
void *mmap_guarded_rw_mte_pages(size_t size, bool populate, const char *name) {
    size_t sz = ROUND_UP_PAGE(size);

    if(sz < size) {
        return NULL;
    }

    void *p = mmap_rw_mte_pages(sz + (g_page_size * 2), populate, name);

    create_guard_page(p);
    create_guard_page(p + (g_page_size + sz));
    return (p + g_page_size);
}
#endif

void *mmap_rw_pages(size_t size, bool populate, const char *name) {
    return mmap_pages(size, populate, name, PROT_READ | PROT_WRITE);
}

#if ARM_MTE
void *mmap_rw_mte_pages(size_t size, bool populate, const char *name) {
    return mmap_pages(size, populate, name, PROT_READ | PROT_WRITE | PROT_MTE);
}
#endif

void *mmap_pages(size_t size, bool populate, const char *name, int32_t prot) {
#if !ENABLE_ASAN
    /* Produce a random page address as a hint for mmap */
    uint64_t hint = ROUND_DOWN_PAGE(rand_uint64());
    hint &= 0x3FFFFFFFF000;
    void *p = (void *) hint;
#else
    void *p = NULL;
#endif
    size_t sz = ROUND_UP_PAGE(size);

    if(sz < size) {
        return NULL;
    }

    int32_t flags = (MAP_PRIVATE | MAP_ANONYMOUS);
    int fd = -1;

#if __linux__
#if PRE_POPULATE_PAGES
    if(populate == true) {
        flags |= MAP_POPULATE;
    }
#endif

#if MAP_HUGETLB && HUGE_PAGES
    /* If we are allocating pages for a user zone
     * then take advantage of the huge TLB */
    if(sz == ZONE_USER_SIZE || sz == (ZONE_USER_SIZE >> 1)) {
        flags |= MAP_HUGETLB;
    }
#endif
#elif __APPLE__
#if VM_FLAGS_SUPERPAGE_SIZE_2MB && HUGE_PAGES
    /* If we are allocating pages for a user zone
     * we are going to use the 2 MB superpage flag */
    if(sz == ZONE_USER_SIZE || sz == (ZONE_USER_SIZE >> 1)) {
        fd = VM_FLAGS_SUPERPAGE_SIZE_2MB;
    }
#endif
#endif

    p = mmap(p, sz, prot, flags, fd, 0);

    if(p == MAP_FAILED) {
        LOG_AND_ABORT("Failed to mmap rw pages");
    }

#if __linux__ && MAP_HUGETLB && HUGE_PAGES && THP_PAGES && MADV_HUGEPAGE
    if(sz == ZONE_USER_SIZE || sz == (ZONE_USER_SIZE >> 1)) {
        madvise(p, sz, MADV_HUGEPAGE);
    }
#endif

    /* All pages are mapped as if we will never need
     * them. This is to ensure RSS stays managable */
    dont_need_pages(p, sz);

    if(name != NULL) {
        name_mapping(p, sz, name);
    }

    return p;
}

void dont_need_pages(void *p, size_t size) {
    madvise(p, size, FREE_OR_DONTNEED);

#if __APPLE__
    while(madvise(p, size, MADV_FREE_REUSE) == -1 && errno == EAGAIN) {
    }
#endif
}

void mprotect_pages(void *p, size_t size, int32_t protection) {
    if((mprotect(p, size, protection)) == ERR) {
        LOG_AND_ABORT("Failed to mprotect pages @ 0x%p (%s)", p, strerror(errno));
    }
}

int32_t name_mapping(void *p, size_t sz, const char *name) {
#if NAMED_MAPPINGS && (__ANDROID__ || KERNEL_VERSION_SEQ_5_17)
    return prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, p, sz, name);
#else
    return 0;
#endif
}

bool is_pow2(uint64_t sz) {
    return (sz & (sz - 1)) == 0;
}

size_t next_pow2(size_t sz) {
    sz |= sz >> 1;
    sz |= sz >> 2;
    sz |= sz >> 4;
    sz |= sz >> 8;
    sz |= sz >> 16;
    sz |= sz >> 32;
    return sz + 1;
}

const uint32_t _log_table[32] = {
    0, 9, 1, 10, 13, 21, 2, 29,
    11, 14, 16, 18, 22, 25, 3, 30,
    8, 12, 20, 28, 15, 17, 24, 7,
    19, 27, 23, 6, 26, 5, 4, 31};

/* Fast log2() implementation for 32 bit integers */
uint32_t _log2(uint32_t v) {
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    return _log_table[(uint32_t) (v * 0x07C4ACDD) >> 27];
}
