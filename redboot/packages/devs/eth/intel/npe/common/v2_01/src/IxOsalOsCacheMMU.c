/**
 * @file IxOsalOsCacheMMU.c (linux)
 *
 * @brief Cache MemAlloc and MemFree.
 * 
 * 
 * @par
 * IXP400 SW Release version 1.5
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2004 Intel Corporation All Rights Reserved.
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

#include "IxOsal.h"

#ifdef CYGPKG_REDBOOT

#define __POOL_SIZE 0x10000;

extern unsigned char *workspace_end;
static unsigned char *next_free;
static int nleft;
static unsigned char *last_alloc;
static int last_size;

/* 
 * Allocate on a cache line boundary (null pointers are
 * not affected by this operation). This operation is NOT cache safe.
 */
void *
ixOsalCacheDmaMalloc (UINT32 n)
{
    static int requested;

    requested += n;

#if 0
    diag_printf("ixOsalCacheDmaMalloc: %d bytes (%d total)\n", n, requested);
#endif

    if (next_free == NULL) {
	next_free = workspace_end - __POOL_SIZE;
	workspace_end -= __POOL_SIZE;
	nleft = __POOL_SIZE;
    }

    if (nleft < n) {
	diag_printf("ixOsServCacheDmaAlloc failed! req[%d] avail[%d]\n",
		n, nleft);
	return NULL;
    }

    last_alloc = next_free;
    last_size = (n+31) & ~31; /* round to cacheline boundary */

    next_free += last_size;

    if (last_size <= nleft)
	nleft -= last_size;
    else
	nleft = 0;

    return last_alloc;
}

/*
 * 
 */
void
ixOsalCacheDmaFree (void *ptr)
{
    if (ptr != last_alloc || last_size == 0) {
#if 0
	diag_printf("ixOsServCacheDmaFree called!\n");
#endif
	return;
    }
    next_free = last_alloc - last_size;
    nleft += last_size;
    last_size = 0;
}

#else
#error "Need to do something for !RedBoot case."
#endif
