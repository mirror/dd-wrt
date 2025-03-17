/* iso_alloc_mte.c - A secure memory allocator
 * Copyright 2024 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"

/* The majority of this code is adapted from Scudos implementation
 * of ARM MTE support. That code can be found here:
 * https://android.googlesource.com/platform/external/scudo/+/refs/tags/android-14.0.0_r1/standalone/ 
 * Its license (The LLVM Project is under the Apache License v2.0 with LLVM Exceptions) can be found here
 * https://android.googlesource.com/platform/external/scudo/+/refs/tags/android-14.0.0_r1/LICENSE.TXT */

#if ARM_MTE == 1

#define MTE_GRANULE 16
#define UNTAGGED_BITS 56

#ifndef HWCAP2_MTE
#define HWCAP2_MTE (1 << 18)
#endif

void *iso_mte_untag_ptr(void *p) {
    return (void *) ((uintptr_t) p & ((1ULL << UNTAGGED_BITS) - 1));
}

uint8_t iso_mte_extract_tag(void *p) {
    return ((uintptr_t) p >> UNTAGGED_BITS) & 0xF;
}

/* Check for the MTE bit in the ELF Auxv */
bool iso_is_mte_supported(void) {
    return getauxval(AT_HWCAP2) & HWCAP2_MTE;
}

void *iso_mte_set_tag_range(void *p, size_t size) {
    void *tagged_ptr = iso_mte_create_tag(iso_mte_untag_ptr(p), 0x0);

    for(int i = 0; i < size; i += MTE_GRANULE) {
        iso_mte_set_tag(tagged_ptr + i);
    }

    return tagged_ptr;
}

/* Uses IRG to create a random tag */
void *iso_mte_create_tag(void *p, uint64_t exclusion_mask) {
    exclusion_mask |= 1;
    void *tagged_ptr;
    __asm__ __volatile__(
        ".arch_extension memtag\n"
        "irg %[tagged_ptr], %[p], %[exclusion_mask]\n"
        : [tagged_ptr] "=r"(tagged_ptr)
        : [p] "r"(p), [exclusion_mask] "r"(exclusion_mask));
    return tagged_ptr;
}

/* Uses STG to lock a tag to an address */
void iso_mte_set_tag(void *p) {
    __asm__ __volatile__(
        ".arch_extension memtag\n"
        "stg %0, [%0]\n"
        :
        : "r"(p)
        : "memory");
}

/* Uses LDG to load a tag */
void *iso_mte_get_tag(void *p) {
    void *tagged_ptr = p;
    __asm__ __volatile__(
        ".arch_extension memtag\n"
        "ldg %0, [%0]\n"
        : "+r"(tagged_ptr)
        :
        : "memory");
    return tagged_ptr;
}
#endif
