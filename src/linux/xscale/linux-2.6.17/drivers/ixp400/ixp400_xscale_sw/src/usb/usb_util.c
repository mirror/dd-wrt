/**
 * @file usb_util.c
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file contains various function which are not
 * part of the core USB driver but are either useful for debugging
 * or optional [minor] features. This file is #included by usb.c.
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
 * @addtogroup API
 * @{
 */

#include "IxOsal.h"

#ifdef IX_USB_HAS_GET_ERROR_STRING

PUBLIC const char * 
ixUSBErrorStringGet(UINT32 errorCode)
{
    switch (errorCode)
    {
        case IX_USB_INVALID_DEVICE :
            return "Invalid or uninitialized USBDevice structure";
        case IX_USB_NO_PERMISSION :
            return "No permission for attempted operation";
        case IX_USB_REDUNDANT :
            return "Redundant call (operation already completed)";
        case IX_USB_SEND_QUEUE_FULL :
            return "Send queue full on endpoint";
        case IX_USB_NO_ENDPOINT :
            return "Endpoint not found";
        case IX_USB_NO_IN_CAPABILITY :
            return "Endpoint has no IN capability";
        case IX_USB_NO_OUT_CAPABILITY :
            return "Endpoint has no OUT capability";
        case IX_USB_NO_TRANSFER_CAPABILITY :
            return "Cannot use requested transfer type with specified endpoint";
        case IX_USB_ENDPOINT_STALLED :
            return "Endpoint is stalled";
        case IX_USB_INVALID_PARMS :
            return "Invalid parameters to function call";
        default:
            return "Unknown error";
    }
}

#endif /* IX_USB_HAS_GET_ERROR_STRING */

#ifdef IX_USB_HAS_ENDPOINT_INFO_SHOW

PUBLIC IX_STATUS 
ixUSBEndpointInfoShow(USBDevice *device)
{
    UINT32 epIndex;
    
    CHECK_DEVICE(device);

    for (epIndex = ENDPOINT_0 ; epIndex < NUM_ENDPOINTS ; epIndex++)
    {
        EPStatusData *epData        = EPSTATUS(device, epIndex);
        const char *stringType      = "unknown";
        const char *stringDirection = "unknown";
        
        /* get direction */
        if (epData->direction == USB_IN)
        {
            stringDirection = "IN";
        }
        else if (epData->direction == USB_OUT)
        {
            stringDirection = "OUT";
        }
        else if (epData->direction == USB_IN_OUT)
        {
            stringDirection = "IN_OUT";
        }
                
        /* get type */
        if (epData->type == USB_CONTROL)
        {
            stringType = "CONTROL";
        }
        else if (epData->type == USB_BULK)
        {
            stringType = "BULK";
        }
        else if (epData->type == USB_ISOCHRONOUS)
        {
            stringType = "ISOCHRONOUS";
        }
        else if (epData->type == USB_INTERRUPT)
        {
            stringType = "INTERRUPT";
        }
        
        printf("endpoint[%2d]: %11.11s:%6.6s:%d\n", epIndex, stringType, stringDirection, epData->fifoSize);
    }

    return IX_SUCCESS;
}

#endif /* IX_USB_HAS_ENDPOINT_INFO_SHOW */

/**
 * @}
 */

#ifdef IX_USB_HAS_DUMMY_MBLK

PRIVATE IX_USB_MBLK *
alloc_IX_USB_MBLK(size_t size)
{
    IX_USB_MBLK *buf = (IX_USB_MBLK *)malloc(sizeof(IX_USB_MBLK));

    /* debug */
    IX_USB_VERBOSE_MEM_TRACE("=> allocated %d bytes into IX_USB_MBLK %p [ixUSBBufferAlloc]\n", sizeof(IX_USB_MBLK), buf, 0, 0, 0, 0);

    if (size < 1)
    {
        IX_USB_VERBOSE_MEM_TRACE("Warning, 0 bytes requested, switching to 1 for safety\n, 0, 0, 0, 0, 0, 0");

        size = 1;
    }

    buf->m_data = malloc(size);

    /* debug */
    IX_USB_VERBOSE_MEM_TRACE("=> allocated %d bytes into m_data %p [ixUSBBufferAlloc]\n", size, buf->m_data, 0, 0, 0, 0);

    buf->m_len  = size;
    buf->m_free = free_IX_USB_MBLK;

    return buf;
}

PUBLIC void 
free_IX_USB_MBLK(IX_USB_MBLK *this_IX_USB_MBLK)
{
    /* debug */
    IX_USB_VERBOSE_MEM_TRACE("<= freeing m_data %p\n", this_IX_USB_MBLK->m_data, 0, 0, 0, 0, 0);

    free(this_IX_USB_MBLK->m_data);

    /* debug */
    IX_USB_VERBOSE_MEM_TRACE("<= freeing IX_USB_MBLK %p\n", this_IX_USB_MBLK, 0, 0, 0, 0, 0);

    free(this_IX_USB_MBLK);
}

#endif /* IX_USB_HAS_DUMMY_MBLK */

PRIVATE void 
ixUSBNullReceiveCallback(
    USBDevice *device,
    UINT16 sourceEndpoint,
    IX_USB_MBLK *receiveBuffer)
{
    USBDeviceContext *context = CONTEXT(device);

    IX_USB_TRACE("Received USB packet (%d bytes) on endpoint %d\n",
                 IX_USB_MBLK_LEN(receiveBuffer), sourceEndpoint, 0, 0, 0, 0);

    IX_USB_MBLK_FREE(receiveBuffer);

    /* increment dropped packet counters */
    context->counters.DRx++;
    context->epStatusData[sourceEndpoint].counters.DRx++;
}

PRIVATE void 
ixUSBNullEventCallback(USBDevice *device, USBEventSet events)
{
    IX_USB_TRACE("Received USB event set %d\n", events, 0, 0, 0, 0, 0);
}

PRIVATE void 
ixUSBNullSetupCallback(USBDevice *device, const char *packet)
{
    IX_USB_TRACE("Received USB SETUP packet, discarding\n", 0, 0, 0, 0, 0, 0);
}

#ifdef __HWEMU__

/**
 * HwEmu support function - register pointer to name conversion function
 *
 * @param device USBDevice * (in) - a structure identifying the device
 * @param regPtr volatile UINT32 * (in) - unknown register pointer
 *
 * @return register name (e.g. "UDCCR") or NULL if no corresponding register was found
 */
PRIVATE const char *
ixUSBRegisterNameGet(USBDevice *device, volatile UINT32 *regPtr)
{
    UDCRegisters *registers = REGISTERS(device);

    if (regPtr == &registers->UDCCR)
    {
        return "UDCCR";
    }
    else if (regPtr == &registers->UDCCS0)
    {
        return "UDCCS0";
    }
    else if (regPtr == &registers->UDCCS1)
    {
        return "UDCCS1";
    }
    else if (regPtr == &registers->UDCCS2)
    {
        return "UDCCS2";
    }
    else if (regPtr == &registers->UDCCS3)
    {
        return "UDCCS3";
    }
    else if (regPtr == &registers->UDCCS4)
    {
        return "UDCCS4";
    }
    else if (regPtr == &registers->UDCCS5)
    {
        return "UDCCS5";
    }
    else if (regPtr == &registers->UDCCS6)
    {
        return "UDCCS6";
    }
    else if (regPtr == &registers->UDCCS7)
    {
        return "UDCCS7";
    }
    else if (regPtr == &registers->UDCCS8)
    {
        return "UDCCS8";
    }
    else if (regPtr == &registers->UDCCS9)
    {
        return "UDCCS9";
    }
    else if (regPtr == &registers->UDCCS10)
    {
        return "UDCCS10";
    }
    else if (regPtr == &registers->UDCCS11)
    {
        return "UDCCS11";
    }
    else if (regPtr == &registers->UDCCS12)
    {
        return "UDCCS12";
    }
    else if (regPtr == &registers->UDCCS13)
    {
        return "UDCCS13";
    }
    else if (regPtr == &registers->UDCCS14)
    {
        return "UDCCS14";
    }
    else if (regPtr == &registers->UDCCS15)
    {
        return "UDCCS15";
    }
    else if (regPtr == &registers->UICR0)
    {
        return "UICR0";
    }
    else if (regPtr == &registers->UICR1)
    {
        return "UICR1";
    }
    else if (regPtr == &registers->USIR0)
    {
        return "USIR0";
    }
    else if (regPtr == &registers->USIR1)
    {
        return "USIR1";
    }
    else if (regPtr == &registers->UFNHR)
    {
        return "UFNHR";
    }
    else if (regPtr == &registers->UFNLR)
    {
        return "UFNLR";
    }
    else if (regPtr == &registers->UBCR2)
    {
        return "UBCR2";
    }
    else if (regPtr == &registers->UBCR4)
    {
        return "UBCR4";
    }
    else if (regPtr == &registers->UBCR7)
    {
        return "UBCR7";
    }
    else if (regPtr == &registers->UBCR9)
    {
        return "UBCR9";
    }
    else if (regPtr == &registers->UBCR12)
    {
        return "UBCR12";
    }
    else if (regPtr == &registers->UBCR14)
    {
        return "UBCR14";
    }
    else if (regPtr == &registers->UDDR0)
    {
        return "UDDR0";
    }
    else if (regPtr == &registers->UDDR1)
    {
        return "UDDR1";
    }
    else if (regPtr == &registers->UDDR2)
    {
        return "UDDR2";
    }
    else if (regPtr == &registers->UDDR3)
    {
        return "UDDR3";
    }
    else if (regPtr == &registers->UDDR4)
    {
        return "UDDR4";
    }
    else if (regPtr == &registers->UDDR5)
    {
        return "UDDR5";
    }
    else if (regPtr == &registers->UDDR6)
    {
        return "UDDR6";
    }
    else if (regPtr == &registers->UDDR7)
    {
        return "UDDR7";
    }
    else if (regPtr == &registers->UDDR8)
    {
        return "UDDR8";
    }
    else if (regPtr == &registers->UDDR9)
    {
        return "UDDR9";
    }
    else if (regPtr == &registers->UDDR10)
    {
        return "UDDR10";
    }
    else if (regPtr == &registers->UDDR11)
    {
        return "UDDR11";
    }
    else if (regPtr == &registers->UDDR12)
    {
        return "UDDR12";
    }
    else if (regPtr == &registers->UDDR13)
    {
        return "UDDR13";
    }
    else if (regPtr == &registers->UDDR14)
    {
        return "UDDR14";
    }
    else if (regPtr == &registers->UDDR15)
    {
        return "UDDR15";
    }

    return NULL;
}

#endif /* __HWEMU */

