/**
 * @file IxOsalOs.h
 *
 * @brief linux-specific defines 
 *
 * Design Notes:
 *
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalOs_H
#define IxOsalOs_H

#ifndef IX_OSAL_CACHED

#error "Uncached memory not supported in linux environment"

#endif

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE

#include <linux/dma-mapping.h>

#include <asm/io.h>

#include <linux/pci.h>

#include <asm/div64.h>

#else /* ! KERNEL_VERSION 2.6 */

#include <linux/cache.h>

#endif

#include <linux/mm.h>

#include <asm/pgalloc.h>

/**
 * Linux implementations of macros.
 */

/**
 * Protect this with a flag that is valid only for 2.6.X and above
 */ 
#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE

#define IX_OSAL_OS_UDIV64_32(dividend, divisor) 	\
({						\
    UINT64 _div = dividend;			\
						\
    do_div(_div, divisor);			\
						\
    _div;					\
})

#define IX_OSAL_OS_UMOD64_32(dividend, divisor) 	\
({						\
    UINT64 _div = dividend;			\
						\
    do_div(_div, divisor);			\
})

#else  /* If less than 2.6, then empty macro */

#define IX_OSAL_OS_UDIV64_32(dividend, divisor)    \
			ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT, \
						"UDIV64: Macro not implemented\n", \
						0,0,0,0,0,0);  

#define IX_OSAL_OS_UMOD64_32(dividend, divisor)    \
			ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT, \
						"UMOD64: Macro not implemented\n", \
						0,0,0,0,0,0);  

#endif /* KERNEL_VERSION */ 


#define IX_OSAL_OS_MMU_VIRT_TO_PHYS(addr) ((addr) ? virt_to_phys((void*)(addr)) : 0)

#define IX_OSAL_OS_MMU_PHYS_TO_VIRT(addr)  ((addr) ? phys_to_virt((unsigned int)(addr)) : 0)

#ifndef IX_HW_COHERENT_MEMORY 

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE

extern void ixOsalCacheInvalidateRange(unsigned long start, unsigned long size); 
extern void ixOsalCacheFlushRange(unsigned long start, unsigned long size); 
 
#define IX_OSAL_OS_CACHE_INVALIDATE(addr, size) \
	ixOsalCacheInvalidateRange((unsigned long)addr, (unsigned long)addr + size) 

#define IX_OSAL_OS_CACHE_FLUSH(addr, size) \
	ixOsalCacheFlushRange((unsigned long)addr, (unsigned long)addr + size ) 

#else /* !KERNEL_VERSION 2.6 */

#define IX_OSAL_OS_CACHE_INVALIDATE(addr, size)  \
    (invalidate_dcache_range((__u32)addr, (__u32)addr + size )) 

#define IX_OSAL_OS_CACHE_FLUSH(addr, size) \
    (clean_dcache_range((__u32)addr, (__u32)addr + size))

#endif /* KERNEL_VERSION */

#else /* IX_HW_COHERENT_MEMORY */

/* 
 * The non-coherent memory region is exposed as uncacheable memory. 
 * So there is no need for cache invalidation or cache flushing
 */
#define IX_OSAL_OS_CACHE_INVALIDATE(addr, size)  do {  } while(0);

#define IX_OSAL_OS_CACHE_FLUSH(addr, size) 	 do {  } while(0);

#endif /* IX_HW_COHERENT_MEMORY  */											  

#define printf	printk /* For backword compatibility, needs to move to better location */

#define ixOsalStdLog(arg_pFmtString, args...) printk(arg_pFmtString, ##args) 

#define IX_OSAL_OS_CACHE_PRELOAD(addr,size)	 do {  } while(0);

#endif /* IxOsalOs_H */
