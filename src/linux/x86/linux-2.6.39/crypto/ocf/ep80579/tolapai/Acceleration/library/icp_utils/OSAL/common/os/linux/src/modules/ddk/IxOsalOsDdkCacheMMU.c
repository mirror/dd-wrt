/**
 * @file IxOsalOsDdkCacheMMU.c (linux)
 *
 * @brief Cache MemAlloc and MemFree.
 * 
 * 
 * @par
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 */
#include "IxOsal.h"
#include <linux/kernel.h>
#include <linux/slab.h>



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
        mySize = size + 
                (IX_OSAL_OS_NUM_INFO_WORDS * IX_OSAL_OS_BYTES_PER_WORD);
        mySize = IX_OSAL_OS_CL_ROUND_UP(mySize);
        mySize += IX_OSAL_CACHE_LINE_SIZE;
        myPtr = (UINT32 *)kmalloc(mySize, GFP_KERNEL);
        
        IX_OSAL_LOCAL_ENSURE( (NULL != myPtr), 
                "ixOsalCacheDmaMalloc():  Fail to alloc small memory \n",
                NULL);

         /* Pass to the user a pointer that is cache line aligned */
         userPtr = myPtr + IX_OSAL_OS_NUM_INFO_WORDS;
         userPtr = (UINT32 *) IX_OSAL_OS_CL_ROUND_UP((UINT32)userPtr);

         /* It is imperative that the user pointer be 32 byte aligned */
         IX_OSAL_LOCAL_ENSURE(
                (((UINT32) userPtr % IX_OSAL_CACHE_LINE_SIZE) == 0),
                "ixOsalCacheDmaMalloc(): "           
                "Error memory allocated is not 32 byte aligned\n",
                NULL);
   }
   else
   {
        /*
         * Increase the size by a full cacheline for size information.
         */
        {
         ULONG temp_size = PAGE_ALIGN (size + IX_OSAL_CACHE_LINE_SIZE);
         /*size = PAGE_ALIGN (size + IX_OSAL_CACHE_LINE_SIZE);*/
         ixOsalMemCopy(&size,&temp_size,sizeof(UINT32));
        }
        order = (UINT32)get_order (size);
        page = alloc_pages (GFP_KERNEL, order);
        if (!page)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                       "ixOsalCacheDmaMalloc():  Fail to alloc page \n",
                       0, 0, 0, 0, 0, 0);
            return NULL;
         }
         myPtr = (UINT32 *) page_address (page);

         /* The client's pointer is 32 bytes ahead of my pointer */
         userPtr = (UINT32 *)((UINT32) myPtr + IX_OSAL_CACHE_LINE_SIZE);
         /* Store the page order 4 words behind the client's pointer */
         userPtr[IX_OSAL_OS_ORDER_OF_PAGES_INDEX] = order;
    }
    
    /* Store the allocated pointer 3 words behind the client's pointer */
    userPtr[IX_OSAL_OS_MYPTR_INDEX] = (UINT32)myPtr;
    /* Store the requested size 2 words behind the client's pointer */
    userPtr[IX_OSAL_OS_REQUESTED_SIZE_INDEX] = size;
    /* Store the allocation identifier 1 word behind the client's pointer */
    userPtr[IX_OSAL_OS_MAGIC_NUMBER_INDEX] = IX_OSAL_OS_MAGIC_ALLOC_NUMBER;

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

    if ( NULL == ptr)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                  "ixOsalCacheDmaFree():  NULL Ptr passed \n",
                  0, 0, 0, 0, 0, 0);
        return;
    }

    /* Make sure that the pointer passed in belongs to us */
    if (clientPtr[IX_OSAL_OS_MAGIC_NUMBER_INDEX] 
            != IX_OSAL_OS_MAGIC_ALLOC_NUMBER)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                  "ixOsalCacheDmaFree():  Memory being freed is invalid \n",
                  0, 0, 0, 0, 0, 0);
        return;
    }

    /* Detect multiple deallocation */
    clientPtr[IX_OSAL_OS_MAGIC_NUMBER_INDEX] = IX_OSAL_OS_MAGIC_DEALLOC_NUMBER;

    /* Rewind ptr to retrieve requested-size information */
    memptr = (UINT32 *)clientPtr[IX_OSAL_OS_MYPTR_INDEX];
    size = clientPtr[IX_OSAL_OS_REQUESTED_SIZE_INDEX];

    /* The requested size will determine how the memory will be freed */
    if (size <= IX_OSAL_OS_SMALL_MEM_SIZE)
    {
        /* Free the "small" page */
        kfree(memptr);
    }
    else
    {
        /* Get the order information */
        order = clientPtr[IX_OSAL_OS_ORDER_OF_PAGES_INDEX];
        /* Free the memory page(s) */
        free_pages ((unsigned int) memptr, order);
    }
}
