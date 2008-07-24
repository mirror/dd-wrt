/*
 *  linux/arch/arm/mm/cache-fa.S
 *
 *  Copyright (C) 2005 Faraday Corp.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  Processors: FA520 FA526 FA626	
 * 03/31/2005 :	Luke Lee created, modified from cache-v4wb.S
 * 04/06/2005 :	1. Read CR0-1 and determine the cache size dynamically,
 *		   to suit all Faraday CPU series
 *	        2. Fixed all functions
 * 04/08/2005 :	insert CONFIG_CPU_ICACHE_DISABLE and CONFIG_CPU_DCACHE_DISABLE
 * 04/12/2005 :	TODO: make this processor dependent or a self-modifying code to 
 *	        inline cache len/size info into the instructions, as reading cache 
 *	        size and len info in memory could cause another cache miss.
 * 05/05/2005 :	Modify fa_flush_user_cache_range to comply APCS.
 * 05/19/2005 :	Adjust for boundary conditions.
 */
#include <linux/linkage.h>
#include <linux/init.h>
#include <asm/hardware.h>
#include <asm/page.h>
#include "proc-macros.S"

#define CACHE_DLINESIZE	   16
#ifdef CONFIG_SL3516_ASIC
#define CACHE_DSIZE	   8192
#else
#define CACHE_DSIZE	   16384 
#endif 
#define CACHE_ILINESIZE	   16
#define CACHE_ISIZE	   16384

/* Luke Lee 04/06/2005 ins begin */
/*
 *	initialize_cache_info()
 *
 *	Automatic detection of DSIZE, DLEN, ISIZE, ILEN variables according to 
 *	system register CR0-1
 *	Destroyed register: r0, r1, r2, r3, ip
 */
	.align
ENTRY(fa_initialize_cache_info)
	mov	r3, #1				@ r3 always = 1
	adr	ip, __fa_cache_ilen
	
	mrc	p15, 0, r0, c0, c0, 1
	/* ILEN */
	and	r1, r0, #3			@ bits [1:0]
	add	r1, r1, #3			@ cache line size is at least 8 bytes (2^3)
	mov	r2, r3, lsl r1			@ r2 = 1<<r1
	str	r2, [ip], #4
	/* ISIZE */
	mov	r1, r0, lsr #6			@ bits [8:6]
	and	r1, r1, #7
	add	r1, r1, #9			@ cache size is at least 512 bytes (2^9)
	mov	r2, r3, lsl r1
	str	r2, [ip], #4
	/* DLEN */
	mov	r1, r0, lsr #12
	and	r1, r1, #3			@ bits [13:12]
	add	r1, r1, #3			@ cache line size is at least 8 bytes (2^3)
	mov	r2, r3, lsl r1			@ r2 = 1<<r1
	str	r2, [ip], #4
	/* DSIZE */
	mov	r1, r0, lsr #18			@ bits [20:18]
	and	r1, r1, #7
	add	r1, r1, #9			@ cache size is at least 512 bytes (2^9)
	mov	r2, r3, lsl r1
	str	r2, [ip]
	mov	pc, lr

	/* Warning : Do not change the order ! Successive codes depends on this */
	.align
	.globl __fa_cache_ilen, __fa_cache_isize, __fa_cache_dlen, __fa_cache_dsize
__fa_cache_ilen:
	.word	0				@ instruction cache line length
__fa_cache_isize:
	.word	0				@ instruction cache size
__fa_cache_dlen:
	.word	0				@ data cahce line length	
__fa_cache_dsize:
	.word	0				@ data cache size

/* Luke Lee 04/06/2005 ins end */

/*
 *	flush_user_cache_all()
 *
 *	Clean and invalidate all cache entries in a particular address
 *	space.
 */
ENTRY(fa_flush_user_cache_all)
	/* FALLTHROUGH */
/*
 *	flush_kern_cache_all()
 *
 *	Clean and invalidate the entire cache.
 */
ENTRY(fa_flush_kern_cache_all)
/* Luke Lee 04/06/2005 mod ok */
	mov	ip, #0
	
#ifndef CONFIG_CPU_ICACHE_DISABLE
	mcr	p15, 0, ip, c7, c5, 0		@ invalidate I cache
#endif
	
__flush_whole_cache:
	
#ifndef CONFIG_CPU_DCACHE_DISABLE
	mov	ip, #0
#  ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, ip, c7, c6, 0		@ invalidate D cache
#  else
	mcr	p15, 0, ip, c7,c14, 0		@ clean/invalidate D cache
#  endif
#endif /*CONFIG_CPU_DCACHE_DISABLE*/
	
#ifndef CONFIG_CPU_FA_WB_DISABLE	
	mcr	p15, 0, ip, c7, c10, 4		@ drain write buffer
#endif
	
#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, ip, c7, c5, 6		@ invalidate BTB
	nop
	nop
#endif
	
/* Luke Lee 04/06/2005 que todo tofix : should iscratchpad and dscratchpad be invalidated ? */
	mov	pc, lr

/*
 *	flush_user_cache_range(start, end, flags)
 *
 *	Invalidate a range of cache entries in the specified
 *	address space.
 *
 *	- start - start address (inclusive, page aligned)
 *	- end	- end address (exclusive, page aligned)
 *	- flags	- vma_area_struct flags describing address space
 */
ENTRY(fa_flush_user_cache_range)

/* Luke Lee 04/06/2005 mod ok */
	/* Luke Lee 04/07/2005 ins 1 */
	mov	ip, #0
	sub	r3, r1, r0			@ calculate total size
#ifndef CONFIG_CPU_ICACHE_DISABLE
	tst	r2, #VM_EXEC			@ executable region?
	mcrne	p15, 0, ip, c7, c5, 0		@ invalidate I cache
#endif

#ifndef CONFIG_CPU_DCACHE_DISABLE	
	/* Luke Lee 04/06/2005 ins 2 mod 1 */
	cmp	r3, #CACHE_DSIZE		@ total size >= limit?
	bhs	__flush_whole_cache		@ flush whole D cache

	//debug_Aaron
        bic     r0, r0, #CACHE_DLINESIZE-1  
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate boundary D entry
        bic     r1, r1, #CACHE_DLINESIZE-1  
	mcr	p15, 0, r1, c7, c14, 1		@ clean and invalidate boundary D entry


1:	/* Luke Lee 04/06/2005 del 2 ins 5 */
	
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#else
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
#endif
	/* Luke Lee 04/06/2005 mod 1 */
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	bls	1b				@ Luke Lee 05/19/2005
#endif	/* CONFIG_CPU_DCACHE_DISABLE */
	
#ifndef CONFIG_CPU_FA_WB_DISABLE
	tst	r2, #VM_EXEC
	/* Luke Lee 04/06/2005 mod 1 tofix todo : ne->eq */
	mcreq	p15, 0, r4, c7, c10, 4		@ drain write buffer
#endif

	/* Luke Lee 04/06/2005 ins block */
#ifdef CONFIG_CPU_FA_BTB
        tst     r2, #VM_EXEC
	mov	ip, #0
	mcrne	p15, 0, ip, c7, c5, 6		@ invalidate BTB
	nop
	nop
#endif
	mov	pc, lr

/*
 *	flush_kern_dcache_page(void *page)
 *
 *	Ensure no D cache aliasing occurs, either with itself or
 *	the I cache
 *
 *	- addr	- page aligned address
 */
ENTRY(fa_flush_kern_dcache_page)
	add	r1, r0, #PAGE_SZ
	/* fall through */

/*
 *	coherent_kern_range(start, end)
 *
 *	Ensure coherency between the Icache and the Dcache in the
 *	region described by start.  If you have non-snooping
 *	Harvard caches, you need to implement this function.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(fa_coherent_kern_range)
	/* fall through */

/*
 *	coherent_user_range(start, end)
 *
 *	Ensure coherency between the Icache and the Dcache in the
 *	region described by start.  If you have non-snooping
 *	Harvard caches, you need to implement this function.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(fa_coherent_user_range)

/* Luke Lee 04/06/2005 mod ok */
	/* Luke Lee 04/06/2005 ins 3 mod 1 */
	bic	r0, r0, #CACHE_DLINESIZE-1

 //debug_Aaron
        bic     r0, r0, #CACHE_DLINESIZE-1
        mcr     p15, 0, r0, c7, c14, 1          @ clean and invalidate boundary D entry
        bic     r1, r1, #CACHE_DLINESIZE-1
        mcr     p15, 0, r1, c7, c14, 1          @ clean and invalidate boundary D entry

#if !(defined(CONFIG_CPU_DCACHE_DISABLE) && defined(CONFIG_CPU_ICACHE_DISABLE))
1:	/* Luke Lee 04/06/2005 del 2 ins 5 mod 1 */
#ifndef CONFIG_CPU_DCACHE_DISABLE	
#ifdef CONFIG_CPU_DCACHE_WRITETHROUGH
	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
#else
	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry
#endif
#endif /* CONFIG_CPU_DCACHE_DISABLE */
	
#ifndef CONFIG_CPU_ICACHE_DISABLE
	mcr	p15, 0, r0, c7, c5, 1		@ invalidate I entry
#endif
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	bls	1b				@ Luke Lee 05/19/2005 blo->bls	
#endif /* !(defined(CONFIG_CPU_DCACHE_DISABLE) && defined(CONFIG_CPU_ICACHE_DISABLE)) */

	mov	ip, #0
#ifdef CONFIG_CPU_FA_BTB
	mcr	p15, 0, ip, c7, c5, 6		@ invalidate BTB
	nop
	nop
#endif

/* Luke Lee 04/08/2005 ins 1 skp 1 ins 1 */
#ifndef CONFIG_CPU_FA_WB_DISABLE
	mcr	p15, 0, ip, c7, c10, 4		@ drain WB
#endif

	mov	pc, lr

/*
 *	dma_inv_range(start, end)
 *
 *	Invalidate (discard) the specified virtual address range.
 *	May not write back any entries.  If 'start' or 'end'
 *	are not cache line aligned, those lines must be written
 *	back.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(fa_dma_inv_range)

/* Luke Lee 04/06/2005 mod ok */

#ifndef CONFIG_CPU_DCACHE_DISABLE	

 //debug_Aaron
        bic     r0, r0, #CACHE_DLINESIZE-1
        mcr     p15, 0, r0, c7, c6, 1          @ invalidate boundary D entry
        bic     r1, r1, #CACHE_DLINESIZE-1
        mcr     p15, 0, r1, c7, c6, 1          @ invalidate boundary D entry

	/* Luke Lee 04/06/2005 ins 4 mod 2 */
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	tst	r0, #CACHE_DLINESIZE -1
	bic	r0, r0, #CACHE_DLINESIZE -1

//debug_Aaron
	//mcrne	p15, 0, r0, c7, c10, 1		@ clean boundary D entry

	/* Luke Lee 04/06/2005 mod 1 */
	/* Luke Lee 05/19/2005 always clean the end-point boundary mcrne->mcr */
	////tst	r1, #CACHE_DLINESIZE -1
	//mcr	p15, 0, r1, c7, c10, 1		@ clean boundary D entry
	/* Luke Lee 04/06/2005 ins 1 */
#else
	bic	r0, r0, #CACHE_DLINESIZE -1	
#endif

//debug_Aaron	
1:	mcr	p15, 0, r0, c7, c6, 1		@ invalidate D entry
//1:	mcr	p15, 0, r0, c7, c14, 1		@ clean and invalidate D entry

	/* Luke Lee 04/06/2005 mod 1 */	
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	bls	1b				@ Luke Lee 05/19/2005 blo->bls
#endif /* CONFIG_CPU_DCACHE_DISABLE */

	/* Luke Lee 04/06/2005 ins 1 */		
#ifndef CONFIG_CPU_FA_WB_DISABLE
	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
#endif	

	mov	pc, lr

/*
 *	dma_clean_range(start, end)
 *
 *	Clean (write back) the specified virtual address range.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 */
ENTRY(fa_dma_clean_range)

/* Luke Lee 04/06/2005 mod ok */
#ifndef CONFIG_CPU_DCACHE_DISABLE	

 //debug_Aaron
        bic     r0, r0, #CACHE_DLINESIZE-1
        mcr     p15, 0, r0, c7, c10, 1          @ clean boundary D entry
        bic     r1, r1, #CACHE_DLINESIZE-1
        mcr     p15, 0, r1, c7, c10, 1          @ clean boundary D entry

	/* Luke Lee 04/06/2005 ins 4 mod 2 */
#ifndef CONFIG_CPU_DCACHE_WRITETHROUGH
	bic	r0, r0, #CACHE_DLINESIZE - 1

//debug_Aaron
1:	mcr	p15, 0, r0, c7, c10, 1		@ clean D entry
//1:	mcr	p15, 0, r0, c7, c14, 1		@ clean D entry
	add	r0, r0, #CACHE_DLINESIZE
	cmp	r0, r1
	bls	1b				@ Luke Lee 05/19/2005 blo->bls
	/* Luke Lee 04/06/2005 ins 2 */
#endif
#endif /* CONFIG_CPU_DCACHE_DISABLE */

#ifndef CONFIG_CPU_FA_WB_DISABLE
	mov	r0, #0	
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
#endif	

	mov	pc, lr

/*
 *	dma_flush_range(start, end)
 *
 *	Clean and invalidate the specified virtual address range.
 *
 *	- start  - virtual start address
 *	- end	 - virtual end address
 *
 *	This is actually the same as fa_coherent_kern_range()
 */
	.globl	fa_dma_flush_range
	.set	fa_dma_flush_range, fa_coherent_kern_range

	__INITDATA

	.type	fa_cache_fns, #object
ENTRY(fa_cache_fns)
	.long	fa_flush_kern_cache_all
	.long	fa_flush_user_cache_all
	.long	fa_flush_user_cache_range
	.long	fa_coherent_kern_range
	.long	fa_coherent_user_range
	.long	fa_flush_kern_dcache_page
	.long	fa_dma_inv_range
	.long	fa_dma_clean_range
	.long	fa_dma_flush_range
	.size	fa_cache_fns, . - fa_cache_fns
