/* iso_alloc_signal.c - A secure memory allocator
 * Copyright 2023 - chris.rohlf@gmail.com */

#include "iso_alloc_internal.h"

#if SIGNAL_HANDLER
INTERNAL_HIDDEN void handle_signal(int sig, siginfo_t *si, void *ctx) {
    void *crash_addr = si->si_addr;

#if UAF_PTR_PAGE
    if(si->si_addr == NULL) {
        LOG_AND_ABORT("si->si_addr == NULL");
    }

    /* Check for the address within 2 pages in
     * either direction of _root->uaf_ptr_page */
    if(crash_addr >= _root->uaf_ptr_page - (g_page_size * 2) &&
       crash_addr <= _root->uaf_ptr_page + (g_page_size * 2)) {
        LOG_AND_ABORT("Use after free detected! Crashed at _root->uaf_ptr_page 0x%x", si->si_addr);
    }
#endif

    LOG_AND_ABORT("Unknown segmentation fault @ 0x%p", crash_addr);
}
#endif
