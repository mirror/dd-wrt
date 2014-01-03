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
