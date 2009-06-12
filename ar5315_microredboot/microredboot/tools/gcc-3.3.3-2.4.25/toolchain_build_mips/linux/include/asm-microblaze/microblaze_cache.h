/*
 * include/asm-microblaze/microblaze_cache.h -- Cache control for Microblaze
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of this
 * archive for more details.
 *
 * Microblaze port by John Williams
 */

#ifndef __MICROBLAZE_MICROBLAZE_CACHE_H__
#define __MICROBLAZE_MICROBLAZE_CACHE_H__

/* Size of a cache line in bytes.  */
/* Microblaze has direct mapped cache, so line size is a single word */
#define MICROBLAZE_CACHE_LINE_SIZE		4

/* Define standard definitions in terms of processor-specific ones.  */
/* Microblaze has no cache, so these all expand to nothing           */

/* For <asm/cache.h> */
#define L1_CACHE_BYTES				MICROBLAZE_CACHE_LINE_SIZE

/* For <asm/pgalloc.h> */
#define flush_cache_all()			
#define flush_cache_mm(mm)			
#define flush_cache_range(mm, start, end)	
#define flush_cache_page(vma, vmaddr)		
#define flush_page_to_ram(page)			
#define flush_dcache_page(page)			
#define flush_icache_range(start, end)		
#define flush_icache_page(vma,pg)		
#define flush_icache()				
#define flush_cache_sigtramp(vaddr)		

#endif /* __MICROBLAZE_MICROBLAZE_CACHE_H__ */
