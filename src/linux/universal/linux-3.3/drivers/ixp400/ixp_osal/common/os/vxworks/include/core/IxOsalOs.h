/**
 * @file IxOsalOs.h
 *
 * @brief vxWorks-specific defines 
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

#include <cacheLib.h>
#include <stdio.h>
#include <string.h>

/* Support for unsigned int 64-bit division and mod operations */

#define IX_OSAL_OS_UDIV64_32(dividend, divisor) ((dividend) / (UINT32)(divisor))
#define IX_OSAL_OS_UMOD64_32(dividend, divisor) \
    ((UINT32)((dividend) % (UINT32)(divisor)))

/* vxWorks virt <--> phys address mapping */
#define IX_OSAL_OS_MMU_PHYS_TO_VIRT(addr) (addr)
#define IX_OSAL_OS_MMU_VIRT_TO_PHYS(addr) (addr)


#ifdef IX_OSAL_CACHED
#define IX_OSAL_OS_CACHE_INVALIDATE(addr,size) \
    cacheInvalidate(DATA_CACHE, (void*)addr, size)

#define IX_OSAL_OS_CACHE_FLUSH(addr,size) \
    do { cacheFlush(DATA_CACHE, (void*)addr, size); cachePipeFlush(); } while(0)

/* Cache preload not available*/
#define IX_OSAL_OS_CACHE_PRELOAD(addr,size) {}

#else /* IX_OSAL_CACHED */

#define IX_OSAL_OS_CACHE_INVALIDATE(addr,size) {}
#define IX_OSAL_OS_CACHE_FLUSH(addr,size) {}
#define IX_OSAL_OS_CACHE_PRELOAD(addr,size) {}

#endif /* IX_OSAL_CACHED */

/* 
 * Only available for vxworks, not an OSAL top API yet. 
 */
PUBLIC IX_STATUS ixOsalThreadIdGet (IxOsalThread * ptrTid);

#endif /* IxOsalOs_H */
