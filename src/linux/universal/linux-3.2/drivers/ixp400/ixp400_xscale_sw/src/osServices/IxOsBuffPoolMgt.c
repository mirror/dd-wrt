/**
 * @file IxOsBuffPoolMgt.c (Now mapped to OSAL )
 *
 * @date 9 Oct 2002
 *
 * @brief This file contains the mbuf pool management implementation.
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
 *
 * @sa IxOsBuffPoolMgt.h
 * @sa IxOsBuffMgt.h
 */

#include "IxOsal.h"
#include "IxOsBuffMgt.h"
 
IX_STATUS
ixOsalOsIxp400BackwardPoolInit (IX_OSAL_MBUF_POOL ** poolPtrPtr,
    UINT32 count, UINT32 size, const char *name)
{
    *poolPtrPtr = IX_OSAL_MBUF_POOL_INIT (count, size, name);
    if (*poolPtrPtr == NULL)
    {
        return IX_FAIL;
    }
    else
    {
        return IX_SUCCESS;
    }
}

IX_STATUS
ixOsalOsIxp400BackwardMbufPoolGet (IX_OSAL_MBUF_POOL * poolPtr,
    IX_OSAL_MBUF ** newBufPtrPtr)
{
    *newBufPtrPtr = IX_OSAL_MBUF_POOL_GET (poolPtr);
    if (*newBufPtrPtr == NULL)
    {
        return IX_FAIL;
    }
    else
    {
        return IX_SUCCESS;
    }
}

