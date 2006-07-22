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

/**
 * @addtogroup IxUsbAPI
 *
 * @brief Basic data types used by the USB driver
 *
 * @{
 */

#include <IxOsalTypes.h>

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
