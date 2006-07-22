/**
 * @file usbtypes.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file contains the public USB Driver data types
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
 * @brief Public data types used by the IXP400 USB driver
 *
 * @{
 */

#ifndef usbtypes_H

#ifndef __doxygen_HIDE

#define usbtypes_H

#endif /* __doxygen_HIDE */

#include "usbbasictypes.h"

/** USB context size */
#define USB_CONTEXT_SIZE 7968

/**
 * USBDevice 
 */
typedef struct
{
    UINT32 baseIOAddress;               /**< base I/O device address */
    UINT32 interruptLevel;              /**< device IRQ */
    UINT32 lastError;                   /**< detailed error of last function call */
    UINT32 deviceIndex;                 /**< USB device index */
    UINT32 flags;                       /**< initialization flags */
    UINT8  deviceContext[USB_CONTEXT_SIZE];  /**< used by the driver to identify the device */
} USBDevice;

typedef UINT16 USBEventSet;

typedef void (*USBEventCallback) (USBDevice *device, USBEventSet events);

typedef void (*USBSetupCallback) (USBDevice *device, const char *packet);

typedef void (*USBReceiveCallback) (
    USBDevice *device, 
    UINT16 sourceEndpoint, 
    IX_USB_MBLK *receiveBuffer);

#endif /* usbtypes_H */

/**
 * @} addtogroup IxUsbAPI
 */
