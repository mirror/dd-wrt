/**
 * @file usbconstants.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file containes the constants used by the USB driver
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
 * @brief Definition of constants used by the USB driver
 *
 * @{
 */

#ifndef usbconstants_H

#ifndef __doxygen_HIDE

#define usbconstants_H

#endif /* __doxygen_HIDE */

/**
 * USB endpoint direction
 */
typedef enum /* USBEndpointDirection */
{
    USB_NO_DATA = 0,
    USB_IN      = 0x01,
    USB_OUT     = 0x02,
    USB_IN_OUT  = USB_IN | USB_OUT
} USBEndpointDirection;

/**
 * Note: the values are set for compatibility with USBEndpointDirection 
 *
 * NB: THESE ARE NOT STANDARD USB ENDPOINT TYPES TO BE USED IN DESCRIPTORS, see usbstd.h
 */
typedef enum /* USBEndpointType */ 
{
    USB_CONTROL     = 0x10,
    USB_BULK        = 0x20,
    USB_INTERRUPT   = 0x40,
    USB_ISOCHRONOUS = 0x80
} USBEndpointType;

/**
 * USB Event Map 
 */
typedef enum /* USBEventMap */
{
    USB_NO_EVENT      = 0,
    USB_RESET         = 0x01,
    USB_SUSPEND       = 0x02,
    USB_RESUME        = 0x04,
    USB_SOF           = 0x08,  /**< Start Of Frame */
    USB_DEVICE_EVENTS = USB_RESET | USB_SUSPEND | USB_RESUME,
    USB_BUS_EVENTS    = USB_SOF,
    USB_ALL_EVENTS    = USB_DEVICE_EVENTS | USB_BUS_EVENTS
} USBEventMap;

/**
 * USB Device Flags 
 */
typedef enum /* USBDeviceFlags */
{
    ENABLE_RX_SEQ            = 0x01,
    ENABLE_TX_SEQ            = 0x02,
    ENABLE_BULK_NAK_THROTTLE = 0x04
} USBDeviceFlags;

/**
 * USB endpoint number 
 */
typedef enum /* USBEndpointNumber */
{
    ENDPOINT_0 = 0,
    ENDPOINT_1,
    ENDPOINT_2,
    ENDPOINT_3,
    ENDPOINT_4,
    ENDPOINT_5,
    ENDPOINT_6,
    ENDPOINT_7,
    ENDPOINT_8,
    ENDPOINT_9,
    ENDPOINT_10,
    ENDPOINT_11,
    ENDPOINT_12,
    ENDPOINT_13,
    ENDPOINT_14,
    ENDPOINT_15
} USBEndpointNumber;

#endif /* usbconstants_H */

/**
 * @} addtogroup IxUsbAPI
 */
