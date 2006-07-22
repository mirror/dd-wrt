/**
 * @file IxOsalOsCacheMMU.c (linux)
 *
 * @brief Cache MemAlloc and MemFree.
 * 
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
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
