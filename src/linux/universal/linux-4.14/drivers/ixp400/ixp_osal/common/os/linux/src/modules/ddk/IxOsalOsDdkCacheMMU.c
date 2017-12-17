/**
 * @file IxOsalOsDdkCacheMMU.c (linux)
 *
 * @brief Cache MemAlloc and MemFree.
 * 
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

#include <linux/kernel.h>
#include <linux/slab.h>

#include "IxOsal.h"

/*
 * Definition of what is deemed a small memory allocation request.
 * Memory requests for up to this size are deemed small and are
 * handled differently from larger memory requests
 */
#define IX_OSAL_OS_SMALL_MEM_SIZE (512 - 32)

/* Arbitrary numbers to detect memory corruption */
#define IX_OSAL_OS_MAGIC_ALLOC_NUMBER (0xBABEFACE)
#define IX_OSAL_OS_MAGIC_DEALLOC_NUMBER (0xCAFEBABE)

/* Number of information words maintained behind the user buffer */
#define IX_OSAL_OS_NUM_INFO_WORDS (4)

/* Macro to round up a size to a multiple of a cache line */
#define IX_OSAL_OS_CL_ROUND_UP(s) \
(((s) + (IX_OSAL_CACHE_LINE_SIZE - 1)) & ~(IX_OSAL_CACHE_LINE_SIZE - 1))

/* 
 * Allocate on a cache line boundary (null pointers are
 * not affected by this operation). This operation is NOT cache safe.
 */
void *
ixOsalCacheDmaMalloc (UINT32 size)
{
    struct page *page;
    UINT32 order;
    UINT32 *userPtr;
    UINT32 *myPtr;
    UINT32 mySize;

    /* The minimum allocation size is 32 */
    if (size < IX_OSAL_CACHE_LINE_SIZE)
    {
	size = IX_OSAL_CACHE_LINE_SIZE;
    }

    /*
     * myPtr        userPtr                            end of last cache line
     * _________________________________________________________
     * |  |   |  |  |                                 |         |
     * |Or|Ptr|Sz|Ma|      USER BUFFER                |         |
     * |__|___|__|__|_________________________________|_________|
     * 
     * myPtr: The pointer returned by kmalloc. This may not be 32 byte aligned
     * userPtr: The pointer returned to the user. This is guaranteed
     *          to be 32 byte aligned
     * Or: The order of pages that was allocated. This info is needed
     *     for deallocating the buffer
     * Ma: Arbitrary number 0xBABEFACE that allows to check against
     *     memory corruption
     * Sz: The value of the requested memory allocation size
     * Ptr: This 4-byte field records the value of myPtr. This info is
     *      needed in order to deallocate the buffer
     */

    /* Check whether the request is for a "small" memory chunck */
    if (size <= IX_OSAL_OS_SMALL_MEM_SIZE)
    {
	/*
	 * Ensure that the size is rounded up to a multiple of a cache line
	 * and add to it a cache line for storing internal information
	 */
	mySize = IX_OSAL_OS_CL_ROUND_UP(size);
	mySize += IX_OSAL_CACHE_LINE_SIZE;
	myPtr = kmalloc(mySize, GFP_KERNEL);
	if (!myPtr)
	{
	    ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
		      "ixOsalCacheDmaMalloc():  Fail to alloc small memory \n",
		      0, 0, 0, 0, 0, 0);
	    return NULL;
	}

	/* Pass to the user a pointer that is cache line aligned */
	userPtr = myPtr + IX_OSAL_OS_NUM_INFO_WORDS;
	userPtr = (UINT32 *) IX_OSAL_OS_CL_ROUND_UP((UINT32)userPtr);

	/* It is imperative that the user pointer be 32 byte aligned */
	IX_OSAL_ENSURE(((UINT32) userPtr % IX_OSAL_CACHE_LINE_SIZE) == 0,
		       "Error memory allocated is not 32 byte aligned\n");
    }
    else
    {
	/*
	 * Increase the size by a full cacheline for size information.
	 */
	size = PAGE_ALIGN (size + IX_OSAL_CACHE_LINE_SIZE);
	order = get_order (size);
	page = alloc_pages (GFP_KERNEL, order);
	if (!page)
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
		       IX_OSAL_LOG_DEV_STDOUT,
		       "ixOsalCacheDmaMalloc():  Fail to alloc page \n",
		       0, 0, 0, 0, 0, 0);
	    return NULL;
	}
	myPtr = page_address (page);

	/* The client's pointer is 32 bytes ahead of my pointer */
	userPtr = (UINT32 *)((UINT32) myPtr + IX_OSAL_CACHE_LINE_SIZE);
	/* Store the page order 4 words behind the client's pointer */
	userPtr[-4] = order;
    }
    
    /* Store the allocated pointer 3 words behind the client's pointer */
    userPtr[-3] = (UINT32)myPtr;
    /* Store the requested size 2 words behind the client's pointer */
    userPtr[-2] = size;
    /* Store the allocation identifier 1 word behind the client's pointer */
    userPtr[-1] = IX_OSAL_OS_MAGIC_ALLOC_NUMBER;

    return ((void *)userPtr);
}

/*
 * 
 * Frees the memory buffer allocated in previous function
 */
void
ixOsalCacheDmaFree (void *ptr)
{
    UINT32 order;
    UINT32 *memptr;
    UINT32 size;
    UINT32 *clientPtr = ptr;

    IX_OSAL_ENSURE((clientPtr != NULL), "Null pointer being freed");

    /* Make sure that the pointer passed in belongs to us */
    if (clientPtr[-1] != IX_OSAL_OS_MAGIC_ALLOC_NUMBER)
    {
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,
		  IX_OSAL_LOG_DEV_STDOUT,
		  "ixOsalCacheDmaFree():  Memory being freed is invalid \n",
		  0, 0, 0, 0, 0, 0);
	return;
    }

    /* Detect multiple deallocation */
    clientPtr[-1] = IX_OSAL_OS_MAGIC_DEALLOC_NUMBER;

    /* Rewind ptr to retrieve requested-size information */
    memptr = (UINT32 *)clientPtr[-3];
    size = clientPtr[-2];

    /* The requested size will determine how the memory will be freed */
    if (size <= IX_OSAL_OS_SMALL_MEM_SIZE)
    {
	/* Free the "small" page */
	kfree(memptr);
    }
    else
    {
	/* Get the order information */
	order = clientPtr[-4];
	/* Free the memory page(s) */
	free_pages ((unsigned int) memptr, order);
    }
}
/*
 * 2.6 kernels do not export the required cache functions.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0))

#define _IX_STR(x) #x
#define IX_STR(x) _IX_STR(x)
#define IX_CLM IX_STR(IX_OSAL_CACHE_LINE_SIZE-1)

/*
 * reimplementation of kernel's invalidate_dcache_range()
 */
void
ixOsalCacheInvalidateRange(unsigned long start, unsigned long size)
{
  __asm__
    ("    tst    %0, #" IX_CLM "\n"
     "    mcrne  p15, 0, %0, c7, c10, 1      @ clean D cache line\n"
     "    bic    %0, %0, #" IX_CLM "\n"
     "    tst    %1, #" IX_CLM "\n"
     "    mcrne  p15, 0, %1, c7, c10, 1      @ clean D cache line\n"
     "1:  mcr    p15, 0, %0, c7, c6, 1       @ invalidate D cache line\n"
     "    add    %0, %0, #" IX_STR(IX_OSAL_CACHE_LINE_SIZE) "\n"
     "    cmp    %0, %1\n"
     "    blo    1b\n"
     "    mcr    p15, 0, %0, c7, c10, 4      @ drain write & fill buffer\n"
     : /* no output */
     : "r"(start), "r"(size)
     : "cc");
}

/*
 * reimplementation of kernel's invalidate_dcache_range()
 */
void
ixOsalCacheFlushRange(unsigned long start, unsigned long size)
{
  __asm__
    ("    bic    %0, %0, #" IX_CLM "\n"     
    "1:  mcr    p15, 0, %0, c7, c10, 1      @ clean D cache line\n"
     "    add    %0, %0, #" IX_STR(IX_OSAL_CACHE_LINE_SIZE) "\n"
     "    cmp    %0, %1\n"
     "    blo    1b\n"
     "    mcr    p15, 0, %0, c7, c10, 4      @ drain write & fill buffer\n"
     : /* no output */
     : "r"(start), "r"(size)
     : "cc");
}

#undef _IX_STR
#undef IX_STR
#undef IX_CLM

#endif /* (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)) */
