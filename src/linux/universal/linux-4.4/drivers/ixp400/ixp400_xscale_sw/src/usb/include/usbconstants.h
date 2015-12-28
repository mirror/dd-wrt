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
