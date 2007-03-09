/**
 * @file IxOsalOsBufferMgt.h 
 *
 * @brief vxworks-specific buffer management module definitions.
 *
 * Design Notes:
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


#ifndef IX_OSAL_OS_BUFFER_MGT_H
#define IX_OSAL_OS_BUFFER_MGT_H

/* 
 * use the defaul bufferMgt provided by OSAL framework.
 */
#define IX_OSAL_USE_DEFAULT_BUFFER_MGT

#include "IxOsalBufferMgtDefault.h"

#if 0  // FIXME
/* Define os-specific buffer macros for subfields */
#define IX_OSAL_OSBUF_MDATA(osBufPtr) IX_OSAL_MBUF_MDATA(osBufPtr) 
             ( ((M_BLK *) osBufPtr)->m_data )

#define IX_OSAL_OSBUF_MLEN(osBufPtr) \
             ( ((M_BLK *) osBufPtr)->m_len )

#define IX_OSAL_OSBUF_PKT_LEN(osBufPtr) \
             ( ((M_BLK *) osBufPtr)->m_pkthdr.len )

#define IX_OSAL_OS_CONVERT_OSBUF_TO_IXPBUF( osBufPtr, ixpBufPtr) \
        { \
            IX_OSAL_MBUF_OSBUF_PTR( (IX_OSAL_MBUF *) ixpBufPtr) = (void *) osBufPtr; \
            IX_OSAL_MBUF_MDATA((IX_OSAL_MBUF *) ixpBufPtr) =  IX_OSAL_OSBUF_MDATA(osBufPtr); \
            IX_OSAL_MBUF_PKT_LEN((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_PKT_LEN(osBufPtr); \
            IX_OSAL_MBUF_MLEN((IX_OSAL_MBUF *) ixpBufPtr) = IX_OSAL_OSBUF_MLEN(osBufPtr); \
        }

#define IX_OSAL_OS_CONVERT_IXPBUF_TO_OSBUF( ixpBufPtr, osBufPtr) \
        { \
            if (ixpBufPtr == NULL) \
            { /* Do nothing */ } \
            else \
            { \
                (M_BLK *) osBufPtr = (M_BLK *) IX_OSAL_MBUF_OSBUF_PTR((IX_OSAL_MBUF *) ixpBufPtr); \
                if (osBufPtr == NULL) \
                { /* Do nothing */ } \
                else \
                { \
                    IX_OSAL_OSBUF_MLEN(osBufPtr) =IX_OSAL_MBUF_MLEN((IX_OSAL_MBUF *) ixpBufPtr); \
                    IX_OSAL_OSBUF_PKT_LEN(osBufPtr) =IX_OSAL_MBUF_PKT_LEN((IX_OSAL_MBUF *) ixpBufPtr); \
                } \
            } \
        }

#endif // FIXME

#endif /* #define IX_OSAL_OS_BUFFER_MGT_H */
