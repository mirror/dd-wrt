/** @file
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
 * @brief Standard USB Setup packet components for the IXP400 USB Device Driver
 *
 * @{
 */

#ifndef usbstd_H

#define usbstd_H

#include "usbbasictypes.h"

/**
 * Standard USB Setup packet components, see the USB Specification 1.1
 */
typedef struct /* USBSetupPacket */
{
    UCHAR bmRequestType;
    UCHAR bRequest;
    UINT16 wValue;
    UINT16 wIndex;
    UINT16 wLength;
} USBSetupPacket;

/**
 * Standard USB request types
 */
typedef enum
{
    GET_STATUS_REQUEST          = 0x00,
    CLEAR_FEATURE_REQUEST       = 0x01,
    SET_FEATURE_REQUEST         = 0x03,
    SET_ADDRESS_REQUEST         = 0x05,
    GET_DESCRIPTOR_REQUEST      = 0x06,
    SET_DESCRIPTOR_REQUEST      = 0x07,
    GET_CONFIGURATION_REQUEST   = 0x08,
    SET_CONFIGURATION_REQUEST   = 0x09,
    GET_INTERFACE_REQUEST       = 0x0a,
    SET_INTERFACE_REQUEST       = 0x0b,
    SYNCH_FRAME_REQUEST         = 0x0c
} USBStdRequestType;

/**
 * Standard USB descriptor types
 */
typedef enum
{
    USB_DEVICE_DESCRIPTOR           = 0x01,
    USB_CONFIGURATION_DESCRIPTOR    = 0x02,
    USB_STRING_DESCRIPTOR           = 0x03,
    USB_INTERFACE_DESCRIPTOR        = 0x04,
    USB_ENDPOINT_DESCRIPTOR         = 0x05
}   USBStdDescriptorType;

/**
 * Standard USB SET/CLEAR_FEATURE feature selector
 */
typedef enum
{
    ENDPOINT_STALL       = 0x0,
    DEVICE_REMOTE_WAKEUP = 0x1
} USBStdFeatureSelector;

/**
 * Standard language IDs used by USB
 */
typedef enum
{
    USB_ENGLISH_LANGUAGE = 0x09
} USBStdLanguageId;

/**
 * Standard USB endpoint types
 */
typedef enum
{
    USB_CONTROL_ENDPOINT     = 0x00,
    USB_ISOCHRONOUS_ENDPOINT = 0x01,
    USB_BULK_ENDPOINT        = 0x02,
    USB_INTERRUPT_ENDPOINT   = 0x03
} USBStdEndpointType;

/**
 * Standard USB directions
 */
typedef enum
{
    USB_ENDPOINT_OUT = 0x0,
    USB_ENDPOINT_IN  = 1
} USBStdEndpointDirection;
    
#ifndef __doxygen_HIDE

/* bmRequestType masks */
#define USB_REQ_DIR_HOST_TO_DEVICE (0x0 << 7)
#define USB_REQ_DIR_DEVICE_TO_HOST (0x1 << 7)

#define USB_REQ_TYPE_MASK     (0x3 << 5)
#define USB_REQ_TYPE_STANDARD (0x0 << 5)
#define USB_REQ_TYPE_CLASS    (0x1 << 5)
#define USB_REQ_TYPE_VENDOR   (0x2 << 5)
#define USB_REQ_TYPE_RESERVED (0x3 << 5)

#define USB_REQ_RCPT_DEVICE    (0x0)
#define USB_REQ_RCPT_INTERFACE (0x1)
#define USB_REQ_RCPT_ENDPOINT  (0x2)
#define USB_REQ_RCPT_OTHER     (0x3)

#endif /* doxygen_HIDE */

#endif /* usbstd_H */

/**
 * @} addtogroup IxUsbAPI
 */
