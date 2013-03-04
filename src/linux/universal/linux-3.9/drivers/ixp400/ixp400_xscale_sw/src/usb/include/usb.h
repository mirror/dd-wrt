/**
 * @file usb.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001

 * @brief This file contains the public API of Intel(R) IXP400 Software USB Driver
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
 * @defgroup IxUsbAPI Intel(R) IXP400 Software USB Driver Public API
 *
 * @brief Intel(R) IXP400 Software USB Driver Public API
 * 
 * @{
 */

#ifndef usb_H

#ifndef __doxygen_HIDE

#define usb_H

#endif /* __doxygen_HIDE */

#include "usbconfig.h"

#include <IxOsal.h>

#include "usbtypes.h"
#include "usberrors.h"
#include "usbconstants.h"
#include "usbdriverparam.h"
#include "usbstd.h"

#ifndef __doxygen_HIDE

/* The PRIVATE macro has been removed as a workaround for SCR#2450 */
PUBLIC IX_USB_MBLK*
ixUSBBufferAlloc(size_t size);

#endif /* __doxygen_HIDE */

/**
 *
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC void ixUSBDataSendAllow (USBDevice *);
 *
 * @brief enable to send requests for USB_IN endpoints
 *
 * @param device USBDevice * (in) - a structure identifying the device
 *
 * @return none
 */
PUBLIC void
ixUSBDataSendAllow (USBDevice *);

/**
 *
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC void ixUSBDataSendlock (void);
 *
 * @brief disable to send requests for USB_IN endpoints
 *
 * @param none
 *
 * @return none
 */
PUBLIC void
ixUSBDataSendBlock (void);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBDriverInit(USBDevice *device)
 *
 * @brief Initialize driver and USB Device Controller
 *
 * @param device USBDevice * (inout) - a structure identifying the device
 *
 * This function initializes the UDC and all the data structures used
 * to interact with the controller.<br>
 * It is the responsibility of the caller to create the USBDevice structure
 * and fill in the correct <i>baseIOAddress</i> and <i>interruptLevel</i>
 * fields.<br>
 * After successful initialization the device will be inactive - use
 * @ref ixUSBDeviceEnable to activate the device.<br>
 * Use the <i>flags</i> component of the <i>device</i> structure to
 * pass in additional flags such as ENABLE_RX_SEQ or ENABLE_TX_SEQ.
 * Changing these flags later will have no effect.<br>
 * The driver will assign a device number which will be placed in the
 * <i>deviceIndex</i> field.<br>
 * The initialized <i>device</i> structure must be used for all interations
 * with the USB controller. The same <i>device</i> pointer will be passed
 * in to all the registered client callbacks.<br>
 * The <i>deviceIndex</i> and <i>deviceContext</i> should be treated as
 * read-only fields.
 * A check to verify that the USB device is present is performed and a warning 
 * is issued if the device is not present.
 *
 * @warning This function is not reentrant.
 *
 * @return IX_SUCCESS if the initialization was successful; a warning is issued
 * if the specified USB device is not present.
 * IX_FAIL otherwise,in which case a detailed error code will be set in the 
 * <i>lastError</i> field, unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBDriverInit(USBDevice *device);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBDeviceEnable(USBDevice *device, BOOL enableDevice)
 *
 * @brief Enable or disable the device
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param enableDevice BOOL (in) - <b>true</b> to enable the device and
 * <b>false</b> to disable it
 *
 * This function enables or disables the device. A disabled device doesn't
 * generate events and cannot send or receive data.<br>
 * Disabling the device frees and discards all existent Rx/Tx buffers 
 * (received buffers that weren't dispatched yet and buffers waiting to be 
 * transmitted)
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBDeviceEnable(USBDevice *device, BOOL enableDevice);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBEndpointStall(USBDevice *device, UINT16 endpointNumber, BOOL stallFlag)
 *
 * @brief Enable or disable endpoint stall (or <i>halt</i> feature)
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param endpointNumber UINT16 (in) - endpoint number
 * @param stallFlag BOOL (in) - <b>true</b> to set endpoint stall and 
 * <b>false</b> to clear it
 *
 * This function clears or sets the endpoint stall (or <i>halt</i>) feature.<br>
 * Both IN and OUT endpoints can be stalled. A stalled endpoint will not
 * send or receive data. Instead, it will send USB STALL packets in response to
 * IN or OUT tokens.<br>
 * Unstalling endpoints can be done only by using this function with the exception
 * of endpoint 0 which unstalls itself automatically upon receiving a new SETUP
 * packet, as required by the USB 1.1 Specification.
 * Isochronous endpoints cannot be stalled and attempting to do so will return
 * an IX_USB_NO_STALL_CAPABILITY failure.
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBEndpointStall(USBDevice *device, 
                   UINT16 endpointNumber, 
                   BOOL stallFlag);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBEndpointClear(USBDevice *device, UINT16 endpointNumber)
 *
 * @brief Free all Rx/Tx buffers associated with an endpoint
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param endpointNumber UINT16 (in) - endpoint number
 * 
 * This function discards and frees all Tx/Rx buffers associated with an endpoint.
 * The corresponding endpoint dropped packet counters will also be incremented.
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBEndpointClear(USBDevice *device, UINT16 endpointNumber);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBSignalResume(USBDevice *device)
 *
 * @brief Trigger signal resuming on the bus
 *
 * @param device USBDevice * (in) - a structure identifying the device
 *
 * This function triggers signal resuming on the bus, waking up the USB host.
 * Is should be used only if the host has enabled the device to do so using
 * the standard SET_FEATURE USB request, otherwise the function will return 
 * IX_FAIL and set the <i>lastError</i> field to IX_USB_NO_PERMISSION.
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBSignalResume(USBDevice *device);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBFrameCounterGet(USBDevice *device, UINT16 *counter)
 *
 * @brief Retrieve the 11-bit frame counter
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param counter UINT16 * (out) - the 11-bit frame counter
 *
 * This function returns the hardware USB frame counter.<br>
 * Since the counter is 11-bit wide it rolls over after every 2048 frames.
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBFrameCounterGet(USBDevice *device, UINT16 *counter);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBReceiveCallbackRegister(USBDevice *device, USBReceiveCallback callbackFunction)
 *
 * @brief Register a data receive callback
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param callbackFunction USBReceiveCallback (in) - receive callback function
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBReceiveCallbackRegister(USBDevice *device, 
                             USBReceiveCallback callbackFunction);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBSetupCallbackRegister(USBDevice *device, USBSetupCallback callbackFunction)
 *
 * @brief Register a setup receive callback
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param callbackFunction USBSetupCallback (in) - setup callback function
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBSetupCallbackRegister(USBDevice *device,
                           USBSetupCallback callbackFunction);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBBufferSubmit(USBDevice *device, UINT16 destinationEndpoint, IX_USB_MBLK *sendBuffer)
 *
 * @brief Submit a buffer for transmit
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param destinationEndpoint UINT16 (in) - endpoint to be used for transmitting the data buffer
 * @param sendBuffer IX_USB_MBLK * (in) - data buffer
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBBufferSubmit(USBDevice *device,
                  UINT16 destinationEndpoint,
                  IX_USB_MBLK *sendBuffer);

/**
 * @fn PUBLIC IX_STATUS ixUSBBufferCancel(USBDevice *device, UINT16 destinationEndpoint, IX_USB_MBLK *sendBuffer)
 *
 * @brief Cancel a buffer previously submitted for transmitting
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param destinationEndpoint UINT16 (in) - endpoint originally used for transmitting the data buffer
 * @param sendBuffer IX_USB_MBLK * (in) - sumbitted data buffer
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS
ixUSBBufferCancel(USBDevice *device,
                  UINT16 destinationEndpoint, 
                  IX_USB_MBLK *sendBuffer);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBEventCallbackRegister(USBDevice *device, USBEventCallback eventCallback, USBEventMap eventMap)
 *
 * @brief Register an event callback
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param eventCallback USBEventCallback (in) - event callback function
 * @param eventMap USBEventMap (in) - event map
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBEventCallbackRegister(USBDevice *device, 
                           USBEventCallback eventCallback, 
                           USBEventMap eventMap);

/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_STATUS ixUSBIsEndpointStalled(USBDevice *device, UINT16 endpointNumber, BOOL *stallState)
 *
 * @brief Retrieve an endpoint's stall status
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param endpointNumber UINT16 (in) - endpoint number
 * @param stallState BOOL * (out) - stall state; <b>true</b> if the endpoint is stalled (<i>halted</i>) or
 * <b>false</b> otherwise<b>true</b>
 * 
 * @return IX_SUCCESS or IX_FAIL if the device pointer is invalid or the endpoint doesn't exist
 */
PUBLIC IX_STATUS
ixUSBIsEndpointStalled(USBDevice *device, UINT16 endpointNumber, BOOL *stallState);


/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC IX_USB_MBLK *ixUSBBufferAlloc(size_t size)
 *
 * @brief Allocates a buffer from the USB driver buffer pool
 *
 * @param size size_t (in) - desired size, in bytes, of the buffer to be allocated
 * 
 * @return a pointer to a newly allocated buffer or NULL if the allocation failed
 */
PUBLIC IX_USB_MBLK*
ixUSBBufferAlloc(size_t size);

#ifdef IX_USB_HAS_STATISTICS_SHOW
/**
 * @fn PUBLIC IX_STATUS ixUSBStatisticsShow(USBDevice *device)
 *
 * @brief Display device state and statistics
 *
 * @param device USBDevice * (in) - a structure identifying the device
 *
 * @return IX_SUCCESS if the initialization was successful, IX_FAIL otherwise,
 * in which case a detailed error code will be set in the <i>lastError</i> field,
 * unless the <i>device</i> parameter is NULL.
 */
PUBLIC IX_STATUS 
ixUSBStatisticsShow(USBDevice *device);
#endif

#ifdef IX_USB_HAS_GET_ERROR_STRING
/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC const char * ixUSBErrorStringGet(UINT32 errorCode)
 *
 * @brief Convert an error code into a human-readable string error message
 *
 * @param errorCode UINT32 (in) - erorr code as defined in usberrors.h
 *
 * @return a <tt>const char *</tt> pointer to the error message
 */
PUBLIC const char * 
ixUSBErrorStringGet(UINT32 errorCode);
#endif

#ifdef IX_USB_HAS_ENDPOINT_INFO_SHOW
/**
 * @fn PUBLIC void ixUSBEndpointInfoShow(USBDevice *device)
 *
 * @brief Display endpoint information table
 *
 * @param device USBDevice * (in) - a structure identifying the device
 *
 * @return none
 */
PUBLIC IX_STATUS 
ixUSBEndpointInfoShow(USBDevice *device);
#endif

#ifdef IX_USB_HAS_DUMMY_MBLK
/**
 * @ingroup IxUsbAPI
 *
 * @fn PUBLIC void free_IX_USB_MBLK(IX_USB_MBLK *this_IX_USB_MBLK)
 *
 * @brief Free an IX_USB_MBLK buffer
 *
 * @param this_IX_USB_MBLK IX_USB_MBLK * (in) - buffer
 *
 * This function is temporary and it is meant to be used as member function
 * inside an IX_USB_MBLK structure. Use the @ref MBLK_FREE() macro instead.
 *
 * @return none
 *
 * @internal
 */
PUBLIC void 
free_IX_USB_MBLK(IX_USB_MBLK *this_IX_USB_MBLK);
#endif

#endif /* usb_H */

/**
 * @} defgroup IxUsbAPI
 */
