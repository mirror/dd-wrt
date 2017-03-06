/**
 * @file IxOsalOsBufferMgt.h 
 *
 * @brief os-specific buffer management module definitions.
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


#ifndef IX_OSAL_OS_BUFFER_MGT_H
#define IX_OSAL_OS_BUFFER_MGT_H

#include <linux/skbuff.h>
/* 
 * Use the defaul bufferMgt provided by OSAL
 * framework.
 */
#define IX_OSAL_USE_DEFAULT_BUFFER_MGT

#include "IxOsalBufferMgtDefault.h"

/* Linux choose to allocate buffers 
 * separately by the defining IX_OSAL_BUFFER_ALLOC_SEPARATELY.
 * As it has a relatively small page limit when allocating a 
 * continuous block.
 */
#define IX_OSAL_BUFFER_ALLOC_SEPARATELY

/* Define os-specific buffer macros to access subfields */
#define IX_OSAL_OSBUF_MDATA(osBufPtr) \
             ( ((struct sk_buff *) osBufPtr)->data )

#define IX_OSAL_OSBUF_MLEN(osBufPtr) \
             ( ((struct sk_buff *) osBufPtr)->len )

/* Conversion utilities for linux-specific buffers */
#define IX_OSAL_OS_CONVERT_OSBUF_TO_IXPBUF( osBufPtr, ixpBufPtr) \
        { \
            IX_OSAL_MBUF_OSBUF_PTR( (IX_OSAL_MBUF *) ixpBufPtr) = (void *) osBufPtr; \
            IX_OSAL_MBUF_MDATA((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_MDATA(osBufPtr); \
            IX_OSAL_MBUF_PKT_LEN((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_MLEN(osBufPtr); \
            IX_OSAL_MBUF_MLEN((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_MLEN(osBufPtr); \
        }

#define IX_OSAL_OS_CONVERT_IXPBUF_TO_OSBUF( ixpBufPtr, osBufPtr) \
        { \
            if (ixpBufPtr == NULL) \
            { /* Do nothing */ } \
            else \
            { \
                (struct sk_buff *) osBufPtr = (struct sk_buff *) IX_OSAL_MBUF_OSBUF_PTR((IX_OSAL_MBUF *) ixpBufPtr); \
                if (osBufPtr == NULL) \
                { /* Do nothing */ } \
                else \
                { \
                    IX_OSAL_OSBUF_MLEN(osBufPtr) =IX_OSAL_MBUF_PKT_LEN((IX_OSAL_MBUF *) ixpBufPtr); \
                } \
            } \
        }


#endif /* #define IX_OSAL_OS_BUFFER_MGT_H */
