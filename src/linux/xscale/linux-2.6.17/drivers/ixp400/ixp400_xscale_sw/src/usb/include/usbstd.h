/** @file
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
