/**
 * @file IxOsalOsCacheMMU.c (vxWorks)
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
