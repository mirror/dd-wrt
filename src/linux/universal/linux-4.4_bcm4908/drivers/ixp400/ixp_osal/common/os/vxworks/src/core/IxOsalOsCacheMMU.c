/**
 * @file IxOsalOsCacheMMU.c (vxWorks)
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

#include <vxWorks.h>
#include <sysLib.h>
#include <memLib.h>
#include <cacheLib.h>

#include "IxOsal.h"

#ifdef IX_OSAL_CACHED

/* 
 * Allocate on a cache line boundary (null pointers are
 * not affected by this operation). This operation is NOT cache safe.
 */
void *
ixOsalCacheDmaMalloc (UINT32 size)
{
    return memalign (IX_OSAL_CACHE_LINE_SIZE, size + IX_OSAL_CACHE_LINE_SIZE);
}

void
ixOsalCacheDmaFree (void *ptr)
{
    free (ptr);
}

#else /* IX_OSAL_CACHED */

/* 
 * Allocate on a cache line boundary (null pointers are
 * not affected by this operation). This operation is NOT cache safe.
 */
void *
ixOsalCacheDmaMalloc (UINT32 size)
{
    void *address;
    void **returnAddress;

    /*
     * get an area of memory 
     */
    address = cacheDmaMalloc (IX_OSAL_CACHE_LINE_SIZE +
        sizeof (void *) + size + IX_OSAL_CACHE_LINE_SIZE);

    if (address == NULL)
    {
        return NULL;
    }

    /*
     * cacheDmaMalloc does not always return an aligned part of memory 
     */
    returnAddress = (void **) ROUND_UP (((UINT32) address),
        IX_OSAL_CACHE_LINE_SIZE);

    /*
     * aligned to the next aligned part of memory 
     */
    returnAddress += IX_OSAL_CACHE_LINE_SIZE / sizeof (UINT32);

    /*
     * store the pointer in the word before the memory 
     * * area returned to the user
     */
    returnAddress[-1] = address;
    return (void *) returnAddress;
}

void
ixOsalCacheDmaFree (void *ptr)
{
    free (((void **) ptr)[-1]);
}

#endif /* IX_OSAL_CACHED */
