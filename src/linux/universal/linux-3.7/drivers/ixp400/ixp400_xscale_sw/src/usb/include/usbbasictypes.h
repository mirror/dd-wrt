/**
 * @file usbbasictypes.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001

 * @brief This temporary file contains basic types used by the USB driver
 *
 * This file will be replaced or heavily modified once the standard BSP types are added.
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

/**
 * @addtogroup IxUsbAPI
 *
 * @brief Basic data types used by the USB driver
 *
 * @{
 */

#include <IxOsal.h>

#ifndef usbbasictypes_H

#ifndef __doxygen_HIDE

#define usbbasictypes_H

#endif /* __doxygen_HIDE */

#ifdef IX_USB_HAS_DUMMY_MBLK

typedef struct struct_mBlk
{
    void *m_data;
    int m_len;
    int pktlen;
    void (*m_free)(struct struct_mBlk *this_mBlk);
} IX_USB_MBLK;

#define IX_USB_MBLK_DATA(buf)      ((buf)->m_data)
#define IX_USB_MBLK_LEN(buf)       ((buf)->m_len)
#define IX_USB_MBLK_FREE(buf)      if ((buf) != NULL) {(buf)->m_free(buf);}
#define IX_USB_MBLK_PKT_LEN(buf)   ((buf)->pktlen)

#else

#include <IxOsal.h>

/** Memory buffer */
#define IX_USB_MBLK		   IX_OSAL_MBUF

/** Return pointer to the data in the mbuf */
#define IX_USB_MBLK_DATA(buf)      IX_OSAL_MBUF_MDATA(buf)

/** Return pointer to the data length */
#define IX_USB_MBLK_LEN(buf)       IX_OSAL_MBUF_MLEN(buf)

/** Returns a buffer to the buffer pool */
void ixUSBMblkFree(IX_USB_MBLK *); 
#define IX_USB_MBLK_FREE(buf)      ixUSBMblkFree(buf)

/** Return pointer to the total length of all the data in the mbuf chain for this packet */
#define IX_USB_MBLK_PKT_LEN(buf)   IX_OSAL_MBUF_PKT_LEN(buf)


#endif /* IX_USB_HAS_DUMMY_MBLK */

#endif /* usbbasictypes_H */

/**
 * @} addtogroup IxUsbAPI
 */
