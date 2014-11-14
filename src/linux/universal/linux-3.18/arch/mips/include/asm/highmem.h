/*
 * highmem.h: virtual kernel memory mappings for high memory
 *
 * Used in CONFIG_HIGHMEM systems for memory pages which
 * are not addressable by direct kernel virtual addresses.
 *
 * Copyright (C) 1999 Gerhard Wichert, Siemens AG
 *		      Gerhard.Wichert@pdb.siemens.de
 *
 *
 * Redesigned the x86 32-bit VM architecture to deal with
 * up to 16 Terabyte physical memory. With current x86 CPUs
 * we now support up to 64 Gigabytes physical RAM.
 *
 * Copyright (C) 1999 Ingo Molnar <mingo@redhat.com>
 */
#ifndef _ASM_HIGHMEM_H
#define _ASM_HIGHMEM_H

#ifdef __KERNEL__

#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <asm/kmap_types.h>

/* undef for production */
#define HIGHMEM_DEBUG 1

/* declarations for highmem.c */
extern unsigned long highstart_pfn, highend_pfn;

extern pte_t *pkmap_page_table;

/*
 * Right now we initialize only a single pte table. It can be extended
 * easily, subsequent pte tables have to be allocated in one physical
 * chunk of RAM.
 */

/*  8 colors pages are here */
#ifdef  CONFIG_PAGE_SIZE_4KB
#define LAST_PKMAP 4096
#endif
#ifdef  CONFIG_PAGE_SIZE_8KB
#define LAST_PKMAP 2048
#endif
#ifdef  CONFIG_PAGE_SIZE_16KB
#define LAST_PKMAP 1024
#endif

/* 32KB and 64KB pages should have 4 and 2 colors to keep space under control */
#ifndef LAST_PKMAP
#define LAST_PKMAP 1024
#endif

#define LAST_PKMAP_MASK (LAST_PKMAP-1)
#define PKMAP_NR(virt)	((virt-PKMAP_BASE) >> PAGE_SHIFT)
#define PKMAP_ADDR(nr)	(PKMAP_BASE + ((nr) << PAGE_SHIFT))

#define ARCH_PKMAP_COLORING             1
#define     set_pkmap_color(pg,cl)      { cl = ((unsigned long)lowmem_page_address(pg) \
					   >> PAGE_SHIFT) & (FIX_N_COLOURS-1); }
#define     get_last_pkmap_nr(p,cl)     (last_pkmap_nr_arr[cl])
#define     get_next_pkmap_nr(p,cl)     (last_pkmap_nr_arr[cl] = \
					    ((p + FIX_N_COLOURS) & LAST_PKMAP_MASK))
#define     is_no_more_pkmaps(p,cl)     (p < FIX_N_COLOURS)
#define     get_next_pkmap_counter(c,cl)    (c - FIX_N_COLOURS)
extern unsigned int     last_pkmap_nr_arr[];


extern void * kmap_high(struct page *page);
extern void kunmap_high(struct page *page);

extern void *kmap(struct page *page);
extern void kunmap(struct page *page);
extern void *kmap_atomic(struct page *page);
extern void __kunmap_atomic(void *kvaddr);
extern void *kmap_atomic_pfn(unsigned long pfn);
extern struct page *kmap_atomic_to_page(void *ptr);

#define flush_cache_kmaps()	flush_cache_all()

extern void kmap_init(void);

#define kmap_prot PAGE_KERNEL

#endif /* __KERNEL__ */

#endif /* _ASM_HIGHMEM_H */
