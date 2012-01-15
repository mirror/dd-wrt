/**
 * @file usb.c
 *
 * @author Intel Corporation
 * @date 30-OCT-2001

 * @brief This file contains the implementation of the IXP400 USB Driver
 *
 * Design Notes:
 *  - DMA handling logic (not actual code) is embedded but #ifdef-ed
 *    on IX_USB_DMA in the code; remove before releasing
 *  - don't use the start-of-frame event as this puts the device into
 *    a non-functioning state
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
 * @addtogroup API
 * @{
 */

#include "IxOsal.h"
#include "IxFeatureCtrl.h"

#ifdef __HWEMU__

#define PRIVATE

#endif /* __HWEMU__ */

#include "usb.h"
#include "usbdeviceparam.h"
#include "usbprivate.h"
#include "usbprivatedata.h"
#include "usbmacros.h"

static USBDeviceContext *ixUSBGlobalContext;
static BOOL ixUSBBulkNakThrottling = FALSE;
#include "usb_util.c"

#ifndef IX_USB_HAS_DUMMY_MBLK

/* Pool definitions */
#define NUM_MBLKS (2000)
#define BLK_SIZE  (2048)
UINT32 ixUSBPoolCounter = NUM_MBLKS;

/* Treshold under which the pool is considered depleted
 * The treshold number represents the number of packets still available
 * for CONTROL transactions; BULK OUT transactions will be replied with
 * NAK packets until more buffers become available
 *
 * NOTE: this is used only if the device is initialized 
 * with the ENABLE_BULK_NAK_THROTTLE flag; ignored otherwise, in which case
 * pool depletion will result in an ASSERT(0) fatal error */
#define POOL_THRESHOLD (10) 

static  IX_OSAL_MBUF_POOL *pNetPool;

#endif /* IX_USB_HAS_DUMMY_MBLK */

/**  Endpoint interrupt sources on UISR0 and UISR1 */
static int UDCEPInterrupt[] =
{
/* UISR0 */
    /* EP_0 */  (0x1 << 0),
    /* EP_1 */  (0x1 << 1),
    /* EP_2 */  (0x1 << 2),
    /* EP_3 */  (0x1 << 3),
    /* EP_4 */  (0x1 << 4),
    /* EP_5 */  (0x1 << 5),
    /* EP_6 */  (0x1 << 6),
    /* EP_7 */  (0x1 << 7),
/* UISR1 */
    /* EP_8 */  (0x1 << 0),
    /* EP_9 */  (0x1 << 1),
    /* EP_10 */ (0x1 << 2),
    /* EP_11 */ (0x1 << 3),
    /* EP_12 */ (0x1 << 4),
    /* EP_13 */ (0x1 << 5),
    /* EP_14 */ (0x1 << 6),
    /* EP_15 */ (0x1 << 7)
};

static EPInterruptHandler EPInterruptHandlerTable[] =
{
    /* EP_0 */ ixUSBEP0InterruptHandler,
    /* EP_1 */ ixUSBINInterruptHandler,
    /* EP_2 */ ixUSBOUTInterruptHandler,
    /* EP_3 */ ixUSBINInterruptHandler,
    /* EP_4 */ ixUSBOUTInterruptHandler,
    /* EP_5 */ ixUSBINInterruptHandler,
    /* EP_6 */ ixUSBINInterruptHandler,
    /* EP_7 */ ixUSBOUTInterruptHandler,
    /* EP_8 */ ixUSBINInterruptHandler,
    /* EP_9 */ ixUSBOUTInterruptHandler,
    /* EP_10 */ ixUSBINInterruptHandler,
    /* EP_11 */ ixUSBINInterruptHandler,
    /* EP_12 */ ixUSBOUTInterruptHandler,
    /* EP_13 */ ixUSBINInterruptHandler,
    /* EP_14 */ ixUSBOUTInterruptHandler,
    /* EP_15 */ ixUSBINInterruptHandler,
};

static int epPriority[]= {0, 5, 10, 15, 1, 2, 3, 4, 6, 7, 8, 9, 11, 12, 13, 14};

static IxOsalMutex usbBufferSubmitMutex[16];

static UINT32 dataSendAllowed = TRUE;

static UINT32 cwInitiated = 0, cwCompleted = 0, cwFailed = 0;
static UINT32 crInitiated = 0, crCompleted = 0, crFailed = 0;
static UINT32 cnInitiated = 0, cnFailed = 0;

/**
 * Interrupt control registers' virtual addresses
 */
static UINT32 ixUSBIrqCtrlRegVirtAddr = 0;

PUBLIC IX_STATUS
ixUSBDriverInit(USBDevice *device)
{
    USBDeviceContext *context;
    UDCRegisters *registers;
    UINT32 epIndex;
    static UINT32 lastDeviceIndex = 0;

    if (device == NULL)
    {
        return IX_FAIL;
    }

    /* Check for USB device being present before proceeding*/
    if (ixFeatureCtrlComponentCheck(IX_FEATURECTRL_USB)==
        IX_FEATURE_CTRL_COMPONENT_DISABLED)
    {
        IX_USB_TRACE("Error: the USB component you specified "
            "does not exist\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    
    context                = CONTEXT(device);
    ixUSBGlobalContext     = context;
    ixUSBBulkNakThrottling = ((device->flags & ENABLE_BULK_NAK_THROTTLE) != 0);

    /* check redundant init */
    if (context->checkPattern == USB_DEVICE_CONTEXT_CHECK_PATTERN)
    {
        RETURN_REDUNDANT(device);
    }
    
    /* Compile-time assert size of context                      */
    /* This will give a "duplicate case" error if not satisfied */
    /* If so, change your USBDevice::deviceContext size to what */
    /* sizeof(USBDeviceContext) reports - or more               */
    IX_USB_CT_ASSERT(sizeof(device->deviceContext) >= sizeof(USBDeviceContext));
    
    /* I/O map UDC base address */
    device->baseIOAddress = (UINT32) IX_OSAL_MEM_MAP(IX_OSAL_IXP400_USB_PHYS_BASE, IX_OSAL_IXP400_USB_MAP_SIZE);
    
    if (device->baseIOAddress == 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_FATAL,
		   IX_OSAL_LOG_DEV_STDERR,
		   "USB: cannot map controller I/O address 0x%08X", 
                   IX_OSAL_IXP400_USB_PHYS_BASE, 0, 0, 0, 0, 0);
      
      return IX_FAIL;
    }
    
    /* I/O map Interrupt Control Register base address */
    ixUSBIrqCtrlRegVirtAddr = (UINT32) IX_OSAL_MEM_MAP(IX_OSAL_IXP400_INTC_PHYS_BASE, 4);
    if (ixUSBIrqCtrlRegVirtAddr == 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_FATAL,
                   IX_OSAL_LOG_DEV_STDERR,
                   "USB: cannot map interrupt controller I/O address 0x%08X",
                   IX_OSAL_IXP400_INTC_PHYS_BASE, 0, 0, 0, 0, 0);

      return IX_FAIL;
    }

    /* init USBDeviceContext structure */
    ixOsalMemSet(context, 0, sizeof(USBDeviceContext));

    /* add a check pattern to verify the context later */
    context->checkPattern = USB_DEVICE_CONTEXT_CHECK_PATTERN;

    /* device is initially disabled */
    context->enabled = FALSE;
    context->configured = FALSE; /*Set device configured to FALSE till SET_CONFIGURATION message received*/

    /* set UDC registers to base I/O address */
    registers          = (UDCRegisters *)(device->baseIOAddress);
    context->registers = registers;
    
    /* init EP0ControlData structure */
    context->ep0ControlData.currentToken = UNKNOWN_TOKEN;
    context->ep0ControlData.transferType = UNKNOWN_TRANSFER;
    context->ep0ControlData.state        = IDLE;

    /* init event processing */
    context->eventProcessor.eventCallback   = ixUSBNullEventCallback;
    context->eventProcessor.eventMap        = USB_DEVICE_EVENTS;
    context->eventProcessor.setupCallback   = ixUSBNullSetupCallback;
    context->eventProcessor.receiveCallback = ixUSBNullReceiveCallback;

    /* init per-endpoint EPStatusData structures */
    ixOsalMemSet(context->epStatusData, 0, sizeof(context->epStatusData));

    /* Populate EPStatusData structures */
    for (epIndex = ENDPOINT_0 ; epIndex < NUM_ENDPOINTS ; epIndex++)
    {
        EPStatusData *epData = &context->epStatusData[epIndex];

        /* link parent USBDevice */
        epData->device = device;

        /* set endpoint properties */
        epData->endpointNumber   = epIndex;
        epData->direction        = EP_DIRECTION(EPDescriptorTable[epIndex]);
        epData->type             = EP_TYPE(EPDescriptorTable[epIndex]);
        epData->transferAllowed = TRUE;

#ifdef IX_USB_DMA

        epData->dmaEnabled     = EPDMAEnabledTable[epIndex];

#endif /* IX_USB_DMA */

        /* setup buffer queue */
        ixUSBQueueInit(epData);

        if (epData->type == USB_CONTROL)
        {
            epData->fifoSize = CONTROL_FIFO_SIZE;
        }
        else if (epData->type == USB_BULK)
        {
            epData->fifoSize = BULK_FIFO_SIZE;

#ifdef IX_USB_DMA

            epData->dmaSize  = BULK_DMA_SIZE;

#endif /* IX_USB_DMA */
        }
        else if (epData->type == USB_ISOCHRONOUS)
        {
            epData->fifoSize = ISOCHRONOUS_FIFO_SIZE;

#ifdef IX_USB_DMA

            epData->dmaSize  = ISOCHRONOUS_DMA_SIZE;

#endif /* IX_USB_DMA */
        }
        else if (epData->type == USB_INTERRUPT)
        {
            epData->fifoSize = INTERRUPT_FIFO_SIZE;
        }

        /*
         * Warning: this switch block applies
         * only to the standard IXP4XX UDC
         * endpoint configuration.
         * Changes in EPDescriptorTable[] should
         * be reflected here.
         */
        switch (epIndex)
        {
            case ENDPOINT_0:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_CONTROL | USB_IN_OUT));

                epData->UDCCS = &registers->UDCCS0;
                epData->UDDR  = &registers->UDDR0;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_1:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_BULK | USB_IN));

                epData->UDCCS = &registers->UDCCS1;
                epData->UDDR  = &registers->UDDR1;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_2:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_BULK | USB_OUT));

                epData->UDCCS = &registers->UDCCS2;
                epData->UDDR  = &registers->UDDR2;
                epData->UBCR  = &registers->UBCR2;
                break;
            case ENDPOINT_3:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_ISOCHRONOUS | USB_IN));

                epData->UDCCS = &registers->UDCCS3;
                epData->UDDR  = &registers->UDDR3;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_4:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_ISOCHRONOUS | USB_OUT));

                epData->UDCCS = &registers->UDCCS4;
                epData->UDDR  = &registers->UDDR4;
                epData->UBCR  = &registers->UBCR4;
                break;
            case ENDPOINT_5:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_INTERRUPT | USB_IN));

                epData->UDCCS = &registers->UDCCS5;
                epData->UDDR  = &registers->UDDR5;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_6:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_BULK | USB_IN));

                epData->UDCCS = &registers->UDCCS6;
                epData->UDDR  = &registers->UDDR6;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_7:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_BULK | USB_OUT));

                epData->UDCCS = &registers->UDCCS7;
                epData->UDDR  = &registers->UDDR7;
                epData->UBCR  = &registers->UBCR7;
                break;
            case ENDPOINT_8:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_ISOCHRONOUS | USB_IN));

                epData->UDCCS = &registers->UDCCS8;
                epData->UDDR  = &registers->UDDR8;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_9:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_ISOCHRONOUS | USB_OUT));

                epData->UDCCS = &registers->UDCCS9;
                epData->UDDR  = &registers->UDDR9;
                epData->UBCR  = &registers->UBCR9;
                break;
            case ENDPOINT_10:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_INTERRUPT | USB_IN));

                epData->UDCCS = &registers->UDCCS10;
                epData->UDDR  = &registers->UDDR10;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_11:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_BULK | USB_IN));

                epData->UDCCS = &registers->UDCCS11;
                epData->UDDR  = &registers->UDDR11;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_12:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_BULK | USB_OUT));

                epData->UDCCS = &registers->UDCCS12;
                epData->UDDR  = &registers->UDDR12;
                epData->UBCR  = &registers->UBCR12;
                break;
            case ENDPOINT_13:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_ISOCHRONOUS | USB_IN));

                epData->UDCCS = &registers->UDCCS13;
                epData->UDDR  = &registers->UDDR13;
                epData->UBCR  = NULL;
                break;
            case ENDPOINT_14:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_ISOCHRONOUS | USB_OUT));

                epData->UDCCS = &registers->UDCCS14;
                epData->UDDR  = &registers->UDDR14;
                epData->UBCR  = &registers->UBCR14;
                break;
            case ENDPOINT_15:
                IX_USB_ASSERT((epData->direction | epData->type) == (USB_INTERRUPT | USB_IN));

                epData->UDCCS = &registers->UDCCS15;
                epData->UDDR  = &registers->UDDR15;
                epData->UBCR  = NULL;
                break;
            default:
                /* it shouldn't get here */
                IX_USB_ASSERT(FALSE);
        }

        IX_USB_VERBOSE_INIT_TRACE("epData[%p:%d]: %d\t%d\t%d\n",(UINT32)epData, epIndex, epData->type, epData->direction, epData->fifoSize, 0);
    }

    /* clear UDC interrupts */
    /* clear endpoints 0..7 */
    REG_SET(&registers->USIR0, UDC_CLEAR_ALL_INT);

    /* clear endpoint 8..15 */
    REG_SET(&registers->USIR1, UDC_CLEAR_ALL_INT);

    /* clear RESET, RESUME, SUSPEND */
    REG_SET(&registers->UDCCR, (UDC_UDCCR_RESIR | UDC_UDCCR_SUSIR | UDC_UDCCR_RSTIR));
    
    /* clear Start-Of-Frame */
    REG_SET(&registers->UFNHR, UDC_UFNHR_SIR);

    /* set UDC interrupt masks (all the events but SOF are enabled) */

    /* enable suspend/resume interrupt by zeroing the mask bit */
    REG_SET(&registers->UDCCR, ((REG_GET(&registers->UDCCR)) & ~UDC_UDCCR_SRM));

    /* enable reset interrupt by zeroing the mask bit */
    REG_SET(&registers->UDCCR, ((REG_GET(&registers->UDCCR)) & ~UDC_UDCCR_REM));

    /* enable endpoint interupts */
    REG_SET(&registers->UICR0, UDC_ENABLE_ALL_INT); /* 0..7  */
    REG_SET(&registers->UICR1, UDC_ENABLE_ALL_INT); /* 8..15 */

    /* don't enable the Start-Of-Frame interrupt by default */
    REG_SET(&registers->UFNHR, UDC_UFNHR_SIM);

    /* set device index */
    device->deviceIndex = lastDeviceIndex++;

    /* hook interrupt handler ixUSBInterruptHandler(device) to UDC IRQ */
    INT_BIND_MACRO(device->interruptLevel, ixUSBInterruptHandlerWrapper, device);

    
    {
	UINT32 i = 0;

	for (; i < 16 ; i++) ixOsalMutexInit(&usbBufferSubmitMutex[i]);
    }

#ifndef IX_USB_HAS_DUMMY_MBLK

    /* init pool */
    if  ((pNetPool = IX_OSAL_MBUF_POOL_INIT(NUM_MBLKS, BLK_SIZE * 2, "USB Pool")) == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDERR,
                   "IxUSBDriverInit: netPoolInit failure\n",
                   0, 0, 0, 0, 0, 0);


        return IX_FAIL;
    }

#endif /* IX_USB_HAS_DUMMY_MBLK */

    
    RETURN_OK(device);
}

PUBLIC IX_STATUS
ixUSBDeviceEnable(USBDevice *device, BOOL enableDevice)
{
    UINT32 UDCCR;
    UDCRegisters *registers;
    USBDeviceContext *context;

    CHECK_DEVICE(device);

    context   = CONTEXT(device);
    registers = context->registers;
   
    UDCCR     = REG_GET(&registers->UDCCR);

    if (context->enabled == enableDevice)
    {
        /* device is already in desired state */
        RETURN_REDUNDANT(device);
    }

    if (enableDevice)
    {
        /* enable UDC */
        REG_SET(&registers->UDCCR, (UDCCR | UDC_UDCCR_UDE));

        /* mark as enabled */
        context->enabled = TRUE;
    }
    else
    {
        UINT16 epIndex;

        /* mark as disabled */
        context->enabled = FALSE;
        context->configured = FALSE; 

        /* disable UDC */
        REG_SET(&registers->UDCCR, UDCCR & ~UDC_UDCCR_UDE);

        /* clear queue and buffers */
        ixUSBEP0StateReset(device); /* Endpoint 0 is special, there's more to clean up */

        for (epIndex = ENDPOINT_0 ; epIndex < NUM_ENDPOINTS ; epIndex++)
        {
            ixUSBEndpointClear(device, epIndex);
        }
    }
    
    RETURN_OK(device);
}

PUBLIC IX_STATUS
ixUSBEndpointStall(USBDevice *device,
                   UINT16 endpointNumber,
                   BOOL stallFlag)
{
    volatile UINT32 *UDCCS_ptr; /* pointer to the endpoint UDCCS */
    UINT32 UDCCS;               /* UDCCS snapshot */

    CHECK_DEVICE(device);

    CHECK_DEVICE_ENABLED(device);

    CHECK_ENDPOINT(device, endpointNumber);

    UDCCS_ptr = EPSTATUS(device, endpointNumber)->UDCCS;
    UDCCS     = REG_GET(UDCCS_ptr); /* take register snapshot */

    /* check endpoint type - isochronous endpoints cannot be stalled according to the protocol */
    if (EP_TYPE(EPDescriptorTable[endpointNumber]) == USB_ISOCHRONOUS)
    {
        RETURN_NO_STALL_CAPABILITY(device);
    }

    /* the FST (force stall) bit is in different places depending on the endpoint */
    if (endpointNumber == ENDPOINT_0) /* endpoint 0 has UDC_UDCCS0_FST */
    {
        if (stallFlag)
        {
            if ((BOOL)((UDCCS & UDC_UDCCS0_FST) != 0) == stallFlag)
            {
                RETURN_REDUNDANT(device);
            }

            REG_SET(UDCCS_ptr, UDC_UDCCS0_FST);
        }
        else /* cannot 'unstall' endpoint 0, this happens automatically */
        {
            RETURN_INVALID_PARMS(device);
        }
    }
    else if (EP_DIRECTION(EPDescriptorTable[endpointNumber]) == USB_IN) /* USB_IN endpoints */
    {
        if ((BOOL)((UDCCS & UDC_UDCCS_FST_IN) != 0) == stallFlag)
        {
            RETURN_REDUNDANT(device);
        }

        if (stallFlag)
        {
            REG_SET(UDCCS_ptr, UDC_UDCCS_FST_IN); /* stall endpoint */
        }
        else
        {
            REG_SET(UDCCS_ptr, UDCCS & ~UDC_UDCCS_FST_IN); /* unstall endpoint */
        }
    }
    else /* USB_OUT endpoints */
    {
        IX_USB_ASSERT(EP_DIRECTION(EPDescriptorTable[endpointNumber]) == USB_OUT);

        if ((BOOL)((UDCCS & UDC_UDCCS_FST_OUT) != 0) == stallFlag)
        {
            RETURN_REDUNDANT(device);
        }

        if (stallFlag)
        {
            REG_SET(UDCCS_ptr, UDC_UDCCS_FST_OUT); /* stall endpoint */
        }
        else
        {
            REG_SET(UDCCS_ptr, UDCCS & ~UDC_UDCCS_FST_OUT); /* unstall endpoint */
        }
    }

    RETURN_OK(device);
}

PUBLIC IX_STATUS
ixUSBIsEndpointStalled(USBDevice *device, UINT16 endpointNumber, BOOL *stallState)
{
    UINT32 UDCCS;
    UINT16 direction;

    CHECK_DEVICE(device);

    CHECK_ENDPOINT(device, endpointNumber);

    if (stallState == NULL)
    {
        RETURN_INVALID_PARMS(device);
    }

    UDCCS     = REG_GET(EPSTATUS(device, endpointNumber)->UDCCS);
    direction = EP_DIRECTION(EPDescriptorTable[endpointNumber]);

    if (direction == USB_IN)
    {
        *stallState = (UDCCS & UDC_UDCCS_FST_IN) != 0;
    }
    else if (direction == USB_OUT)
    {
        *stallState = (UDCCS & UDC_UDCCS_FST_OUT) != 0;
    }
    else
    {
        IX_USB_ASSERT(direction == USB_IN_OUT);

        *stallState = (UDCCS & UDC_UDCCS0_FST) != 0;
    }

    RETURN_OK(device);
}

PUBLIC IX_STATUS 
ixUSBEndpointClear(USBDevice *device, UINT16 endpointNumber)
{
    EPStatusData *epData;

    CHECK_DEVICE(device);

    CHECK_DEVICE_ENABLED(device);

    CHECK_ENDPOINT(device, endpointNumber);

    epData = EPSTATUS(device, endpointNumber);

    /* discard data queue */
    ixUSBQueueDiscard(epData);

    /* discard current transfer */
    ixUSBTransferAbort(epData);

    RETURN_OK(device);
}

PUBLIC IX_STATUS 
ixUSBSignalResume(USBDevice *device)
{
    UDCRegisters *registers;

    CHECK_DEVICE(device);

    CHECK_DEVICE_ENABLED(device);

    registers = REGISTERS(device);

    /* check if the device was enabled to signal resume */
    if ((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_DRWF) == 0)
    {
        RETURN_NO_PERMISSION(device);
    }

    REG_SET(&registers->UDCCR, ((REG_GET(&registers->UDCCR)) | UDC_UDCCR_RSM));

    RETURN_OK(device);
}

PUBLIC IX_STATUS 
ixUSBFrameCounterGet(USBDevice *device, UINT16 *counter)
{
    UDCRegisters *registers;
    UINT32 UFNHR, UFNLR; /* frame number high (3 bits) and low (8 bits) */

    CHECK_DEVICE(device);

    CHECK_DEVICE_ENABLED(device);

    if (counter == NULL)
    {
        RETURN_INVALID_PARMS(device);
    }

    registers = REGISTERS(device);
    UFNHR     = REG_GET(&registers->UFNHR);
    UFNLR     = REG_GET(&registers->UFNLR);

    /* (UFNHR[2:0] & 0x7) << 8) + UFNLR[7..0] & 0xFF */
    *counter = (((UFNHR & UDC_UFNHR_FN_MASK) << UDC_UFNHR_FN_SHIFT) 
                | (UFNLR & UDC_UFNLR_FN_MASK));

    RETURN_OK(device);
}

PUBLIC IX_STATUS 
ixUSBReceiveCallbackRegister(USBDevice *device, 
                             USBReceiveCallback callbackFunction)
{
    CHECK_DEVICE(device);

    if (callbackFunction != NULL)
    {
        EVENTS(device)->receiveCallback = callbackFunction;
    }
    else
    {
        EVENTS(device)->receiveCallback = ixUSBNullReceiveCallback;
    }

    RETURN_OK(device);
}

PUBLIC IX_STATUS 
ixUSBSetupCallbackRegister(USBDevice *device,
                           USBSetupCallback callbackFunction)
{
    CHECK_DEVICE(device);

    if (callbackFunction != NULL)
    {
        EVENTS(device)->setupCallback = callbackFunction;
    }
    else
    {
        EVENTS(device)->setupCallback = ixUSBNullSetupCallback;
    }

    RETURN_OK(device);
}

PUBLIC void
ixUSBDataSendAllow(USBDevice *device)
{
    EPStatusData *epData = EPSTATUS(device, 1);

    dataSendAllowed = TRUE;

    IX_USB_VERBOSE4_TRACE("USB: Resuming data transfers\n", 0, 0, 0, 0, 0, 0);

    ixUSBRequestSend(epData);
}

PUBLIC void
ixUSBDataSendBlock()
{
	dataSendAllowed = FALSE;
}

PUBLIC IX_STATUS 
ixUSBBufferSubmit(USBDevice *device,
                  UINT16 destinationEndpoint,
                  IX_USB_MBLK *sendBuffer)
{
    EPStatusData *epData;

    IxOsalMutex *lock = &usbBufferSubmitMutex[destinationEndpoint];

    CHECK_DEVICE(device);

    CHECK_DEVICE_ENABLED(device);
    
    if(ENDPOINT_0 != destinationEndpoint)
    {
        CHECK_DEVICE_CONFIGURED(device); /*return error if SET_CONFIGURATION not received */
    }

    CHECK_ENDPOINT(device, destinationEndpoint);

    CHECK_ENDPOINT_STALL(device, destinationEndpoint);

    if (sendBuffer == NULL)
    {
        RETURN_INVALID_PARMS(device);
    }

    epData = EPSTATUS(device, destinationEndpoint);

    CHECK_ENDPOINT_IN_CAPABILITY(epData, device);

    IX_USB_VERBOSE4_TRACE("USB: Submitted %d bytes on endpoint %d, %d packets in queue\n",
                 IX_USB_MBLK_LEN(sendBuffer),
                 destinationEndpoint,
                 epData->queue.len,
                 0, 0, 0);

    ixOsalMutexLock(lock, IX_OSAL_WAIT_FOREVER);

    if (ixUSBBufferEnqueue(epData, sendBuffer))
    {
        if (destinationEndpoint == ENDPOINT_0)
        {
            ixUSBEP0RequestSend(device);
        }
        else
        {
            if (dataSendAllowed || destinationEndpoint != 1)
            {
                ixUSBRequestSend(epData);
	    }
	    else
	    {
	    	IX_USB_TRACE("USB: Oops, packet queued, data transfers blocked\n",
	    		     0, 0, 0, 0, 0, 0);
	    }
        }
	ixOsalMutexUnlock(lock);

	RETURN_OK(device);
    }
    else
    {
        IX_USB_TRACE("USB: RETURN SEND QUEUE FULL\n", 
                     0, 0, 0, 0, 0, 0);

	ixOsalMutexUnlock(lock);

        RETURN_SEND_QUEUE_FULL(device);
    }
}

PUBLIC IX_STATUS 
ixUSBBufferCancel(USBDevice *device,
                  UINT16 destinationEndpoint, 
                  IX_USB_MBLK *sendBuffer)
{
    USBDataQueue *queue;
    UINT32 local_index;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    UINT32 irqStatus;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    CHECK_DEVICE(device);

    CHECK_DEVICE_ENABLED(device);

    CHECK_ENDPOINT(device, destinationEndpoint);

    queue = EPQUEUE(device, destinationEndpoint);

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    /* lock section */
    irqStatus = IX_USB_LOCK;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    for (local_index = 0 ; local_index < queue->len ; local_index++)
    {
        UINT32 offset = QUEUE_WRAP(queue->head + local_index);

        if (queue->base[offset] == sendBuffer)
        {
            queue->base [offset] = NULL;

            IX_USB_MBLK_FREE(sendBuffer);
        }
    }

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    /* unlock section */
    IX_USB_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    if (local_index == queue->len)
    {
        IX_USB_TRACE("USB: BUFFER NOT FOUND\n", 0, 0, 0, 0, 0, 0);

        RETURN_ERROR(device); /* buffer not found */
    }
    else
    {
        RETURN_OK(device);
    }
}

PUBLIC IX_STATUS 
ixUSBEventCallbackRegister(USBDevice *device, 
                           USBEventCallback eventCallback, 
                           USBEventMap eventMap)
{
    USBEventProcessor *events;
    UDCRegisters *registers;

    CHECK_DEVICE(device);

    CHECK_EVENT_MASK(device, eventMap);

    events    = EVENTS(device);
    registers = REGISTERS(device);

    if (eventCallback != NULL)
    {
        UINT32 UFNHR = REG_GET(&registers->UFNHR);

        BOOL sofMaskSet     = (UFNHR & UDC_UFNHR_SIM) != 0;
        BOOL sofMaskDesired = (eventMap & USB_SOF) == 0;

        events->eventCallback = eventCallback;
        events->eventMap      = eventMap;

        /* check if we need to switch the SOF mask */
        if (sofMaskSet != sofMaskDesired)
        {
            if (sofMaskDesired)
            {
                /* set mask */
                UFNHR = UDC_UFNHR_SIM;
            }
            else
            {
                /* unset mask (this will enable SOF interrupts) */
                UFNHR = UFNHR & ~UDC_UFNHR_SIM;
            }

            REG_SET(&registers->UFNHR, UFNHR);
        }
    }
    else
    {
        events->eventCallback = ixUSBNullEventCallback;
        events->eventMap      = USB_DEVICE_EVENTS;

        /* mask start of frame interrupts */
        REG_SET(&registers->UFNHR, UDC_UFNHR_SIM);
    }

    RETURN_OK(device);
}

#ifdef IX_USB_HAS_STATISTICS_SHOW

PUBLIC IX_STATUS 
ixUSBStatisticsShow(USBDevice *device)
{
    USBDeviceCounters *devCounters;
    BOOL deviceEnabled;
    UDCRegisters *registers;

#ifdef IX_USB_STATS_SHOW_PER_ENDPOINT_INFO

    UINT16 epIndex;

#endif /* IX_USB_STATS_SHOW_PER_ENDPOINT_INFO */

    CHECK_DEVICE(device);

    devCounters   = COUNTERS(device);
    deviceEnabled = CONTEXT(device)->enabled;
    registers     = REGISTERS(device);

    /* device info */
    printf("USB controller %d (I/O %x, IRQ %d) - %s, %s, %d irqs, %d frames\n",
            device->deviceIndex,
            device->baseIOAddress,
            device->interruptLevel,
            deviceEnabled ? "enabled" : "disabled",
            (REG_GET(&registers->UDCCR) & UDC_UDCCR_UDA) ? "active" : "inactive",
            devCounters->irqCount,
            devCounters->frames);

    /* device stats */
    printf("packets Tx %d, Rx %d, dropped Tx %d, Rx %d - bytes Tx %d, Rx %d - setup %d\n",
            devCounters->Tx,
            devCounters->Rx,
            devCounters->DTx,
            devCounters->DRx,
            devCounters->bytesTx,
            devCounters->bytesRx,
            devCounters->setup);

#ifdef IX_USB_STATS_SHOW_PER_ENDPOINT_INFO

    printf("\n");

    /* endpoint stats - table header */
    printf("ep |   packets |   dropped |     bytes | irqs | flags   | FIFO o/u\n");
    printf("   |  Tx    Rx |  Tx    Rx |   Tx   Rx |      |         |\n");

    /* endpoint info */
    for (epIndex = ENDPOINT_0 ; epIndex < NUM_ENDPOINTS ; epIndex++)
    {
        USBEndpointCounters *epCounters = EPCOUNTERS(device, epIndex);
        BOOL stallState;

        ixUSBIsEndpointStalled(device, epIndex, &stallState);

        printf("%2d | %3d%s %3d%s | %3d%s %3d%s | %3d%s %3d%s | %4d | %s | %3d%s %3d%s\n",
                epIndex,
                SHOW_NUMBER(epCounters->Tx), SHOW_METRIC(epCounters->Tx),
                SHOW_NUMBER(epCounters->Rx), SHOW_METRIC(epCounters->Rx),
                SHOW_NUMBER(epCounters->DTx), SHOW_METRIC(epCounters->DTx),
                SHOW_NUMBER(epCounters->DRx), SHOW_METRIC(epCounters->DRx),
                SHOW_NUMBER(epCounters->bytesTx), SHOW_METRIC(epCounters->bytesTx),
                SHOW_NUMBER(epCounters->bytesRx),  SHOW_METRIC(epCounters->bytesRx),
                epCounters->irqCount,
                EP_TYPE(EPDescriptorTable[epIndex]) == USB_ISOCHRONOUS ? "    N/A" : stallState ? "stalled" : " active",
                SHOW_NUMBER(epCounters->fifoOverflows), SHOW_METRIC(epCounters->fifoOverflows),
                SHOW_NUMBER(epCounters->fifoUnderruns), SHOW_METRIC(epCounters->fifoUnderruns));
    }

#endif /* IX_USB_STATS_SHOW_PER_ENDPOINT_INFO */

    RETURN_OK(device);
}

#endif /* IX_USB_HAS_STATISTICS_SHOW */


PRIVATE UINT32 
ixUSBIrqRead(void)
{
    UINT32 intPending;

    if(ixUSBIrqCtrlRegVirtAddr == 0)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR,
                  IX_OSAL_LOG_DEV_STDERR,
                  "Irq Control Registers is not mem map\n",
                  0, 0, 0 ,0 ,0 ,0);
        return 0;
    }
    
    /**
     * Get the interrupt control register content. 
     * The register is mem map in ixUSBDriverInit. 
     */
    intPending = IX_OSAL_READ_LONG(ixUSBIrqCtrlRegVirtAddr);
    return intPending;
}

/**
 * @} addtogroup USBDriver
 */

/**
 * @addtogroup SupportAPI
 * @{
 */
PRIVATE void 
ixUSBInterruptHandlerWrapper(USBDevice *device)
{
    UINT32 intPend     = 0;
    UINT32 maxCount    = 0xFF;
    UINT32 ep0Activity = 0;

    intPend = ixUSBIrqRead();
    if (! (intPend & (1 << IX_OSAL_IXP400_INT_LVL_USB)))
    {
        IX_USB_VERBOSE_WARN_TRACE("Arrived in ixUSBInterruptHandlerWrapper but no interrupt\n",
   		  0, 0, 0, 0, 0, 0);
        return;
    }

    while ((intPend & (1 << IX_OSAL_IXP400_INT_LVL_USB)) && maxCount)
    {
        ep0Activity = ixUSBInterruptHandler(device);

        --maxCount;
        intPend = ixUSBIrqRead();
        intPend = ixUSBIrqRead();
        intPend = ixUSBIrqRead();
    }

    if (maxCount == 0)
    {
        IX_USB_VERBOSE2_TRACE("Leaving ixUSBInterruptHandlerWrapper after 0xffff iterations\n",
 	             0, 0, 0, 0, 0, 0);
    }
}

/**
 * @fn PRIVATE UINT32 ixUSBInterruptHandler(USBDevice *device)
 *
 * @brief Main interrupt handler
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * Takes a snapshot of the USB status registers and analyzes the
 * source of the interrupt, calling the appropriate handler.
 * Depending on the event that caused the interrupt this can be
 * the endpoint 0 interrupt handler, USB_IN/USB_OUT interrupt handlers
 * or the client event callback.<br>
 * UDC interrupts are disabled during the execution of this function
 * as the handler and its support functions are not reentrant.
 *
 * @return none
 *
 * @internal
 */
 
/*
Note:     

    UDCCR     - UDC Control Register
    USIR0     - UDC Interrupt and Status Register 0
    USIR1     - UDC Interrupt and Status Register 1
    UFNHR     - UDC Frame Number High Register
 */

static INT32 ixUSBIPRset = 0;

PRIVATE UINT32 
ixUSBInterruptHandler(USBDevice *device)
{
    UINT16 epIndex;
    UINT32 eventSet    = USB_NO_EVENT;
    UINT32 ep0Activity = 0;

    USBDeviceContext *context      = CONTEXT(device);
    UDCRegisters *registers        = context->registers;
    USBEventCallback eventCallback = context->eventProcessor.eventCallback;
    USBEventMap eventMap           = context->eventProcessor.eventMap;

    UINT32 UDCCR; /* device control/status register */
    UINT32 USIR0; /* endpoints 0..7 interrupt status */
    UINT32 USIR1; /* endpoints 8..15 interrupt status */
    UINT32 UFNHR; /* device frame number high register (contains SOF IRQ) */

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    /* disable interrupts */
    UINT32 irqStatus;

    irqStatus = IX_USB_IRQ_LOCK;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    /* read UDCCR, UISR0, UISR1, UFNHR */
    UDCCR = REG_GET(&registers->UDCCR);
    USIR0 = REG_GET(&registers->USIR0);
    USIR1 = REG_GET(&registers->USIR1);
    UFNHR = REG_GET(&registers->UFNHR);

    ixUSBIPRset = 0;

    if ((UDCCR | USIR0 | USIR1) == 0)
    {
        IX_USB_VERBOSE2_TRACE("USB: Oops, servicing interrupt with no events\n",
                   0, 0, 0, 0, 0, 0);
    }

    /* debug */
    IX_USB_VERBOSE3_TRACE("Int handler: UDCCR %2X, USIR0 %2X, USIR1 %2X, UFNHR %2X\n", 
                 UDCCR, USIR0, USIR1, UFNHR, 0, 0);

    /* device IRQ counter */
    context->counters.irqCount++;

    /* Reset? */
    if (UDCCR & UDC_UDCCR_RSTIR)
    {
        eventSet |= USB_RESET;

        /* perform driver reset */
        
        /* clear endpoint information */
        for (epIndex = ENDPOINT_0 ; epIndex < NUM_ENDPOINTS ; epIndex++)
        {
            ixUSBEndpointClear(device, epIndex);
        }

        /* reset endpoint 0 state */
        ixUSBEP0StateReset(device);
        context->configured = FALSE;
    }

    /* Suspend? */
    if (UDCCR & UDC_UDCCR_SUSIR)
    {
        eventSet |= USB_SUSPEND;
    }

    /* Resume? */
    if (UDCCR & UDC_UDCCR_RESIR)
    {
        eventSet |= USB_RESUME;
    }

    /* Start of frame? */
    if (UFNHR & UDC_UFNHR_SIR)
    {
        eventSet |= USB_SOF;

        /* device frame counter */
        context->counters.frames++;
    }

    if ((eventCallback != NULL) && ((eventMap & eventSet) != 0))
    {
        eventCallback(device, eventSet);
    }

    /* Endpoint interrupt handling */
    for (epIndex = ENDPOINT_0 ; epIndex < NUM_ENDPOINTS ; epIndex++)
    {
        UINT32 intReg = (epPriority[epIndex] < ENDPOINT_8) ? USIR0 : USIR1;
        
        if (((intReg & UDCEPInterrupt[epPriority[epIndex]]) != 0))
        {
            EPStatusData *epData = &(context->epStatusData[epPriority[epIndex]]);

            ep0Activity = (epPriority[epIndex] == 0);

            /* endpoint IRQ count */
            epData->counters.irqCount++;

            /* debug */
            IX_HWEMU_TRACE("USB::ixUSBInterruptHandler(): servicing endpoint %d\n", 
                  epIndex, 0, 0, 0, 0, 0);

            /* service endpoint */
            EPInterruptHandlerTable[epPriority[epIndex]](epData);

            /* clear endpoint interrupt bit */

            if (epPriority[epIndex] < ENDPOINT_8  )
            {
                if (epPriority[epIndex] != 0 || ixUSBIPRset == 0)
                {
                    REG_SET(&registers->USIR0, UDCEPInterrupt[epPriority[epIndex]]);
                }
            }
            else
            {
                REG_SET(&registers->USIR1, UDCEPInterrupt[epPriority[epIndex]]);
            }

            ixUSBIPRset = 0;
        }
    }

    /* clear serviced interrupts */
    REG_SET(&registers->UDCCR, UDCCR);

    /* REG_SET(&registers->USIR0, USIR0); */
    /* REG_SET(&registers->USIR1, USIR1); */
    REG_SET(&registers->UFNHR, UFNHR);


#ifdef __HWEMU__ /* the emulator allows more time for processing */
    if (USIR0 != 0x01)
#endif /* __HWEMU__ */
    {
        REG_SET(&registers->USIR0, USIR0);
    }

#ifdef __HWEMU__
    if (USIR0 == 0x01) 
    {
        REG_SET(&registers->USIR0, USIR0);
    }
#endif /* __HWEMU__ */

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    IX_USB_IRQ_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    return ep0Activity;
}

/**
 * @fn PRIVATE void ixUSBINInterruptHandler(EPStatusData *epData)
 *
 * @brief Interrupt handler for USB_IN endpoints
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * Interrupt handler for USB_IN endpoints. Is is called after a packet was
 * sent to the host. Calls ixUSBSendCleanup() to release outgoing
 * buffers and service the next transfer.
 *
 * @return none
 *
 * @internal
 */
PRIVATE void 
ixUSBINInterruptHandler(EPStatusData *epData)
{
    ixUSBSendCleanup(epData);
}

/**
 * @fn PRIVATE void ixUSBOUTInterruptHandler(EPStatusData *epData)
 *
 * @brief Interrupt handler for USB_OUT endpoints
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * Interrupt handler for USB_OUT endpoints. Services Rx host requests by calling
 * ixUSBReceiveProcess().
 *
 * @internal
 */
PRIVATE void 
ixUSBOUTInterruptHandler(EPStatusData *epData)
{
    ixUSBReceiveProcess(epData);
}

/**
 * @fn PRIVATE void ixUSBEP0InterruptHandler(EPStatusData *epData)
 *
 * @brief Interrupt handler for endpoint 0
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * The endpoint 0 interrupt handler analyzes and updates the endpoint 0
 * state machine while taking appropriate actions to ensure Tx/Rx
 * on the control endpoint.<br>
 * Interacts with client code by calling the <i>setup receive</i> callback.
 *
 * @internal
 */
PRIVATE void 
ixUSBEP0InterruptHandler(EPStatusData *epData)
{
    USBDevice *device          = epData->device;
    USBDeviceContext *context  = CONTEXT(device);
    UDCRegisters *registers    = context->registers;
    EP0ControlData *ep0Control = &context->ep0ControlData;
    UINT32 UDCCS0              = REG_GET(&registers->UDCCS0);
    static UINT32 controlWriteError = FALSE;
    UINT32 bRequest = 0;

    IX_HWEMU_TRACE("Ep0 handler: UDCCS0 is 0x%08X\n", UDCCS0, 0, 0, 0, 0, 0);

    IX_USB_VERBOSE3_TRACE("::> UDCCS0 is 0x%02X\n", UDCCS0, 0, 0, 0, 0, 0);

    ixOsalMutexLock(&usbBufferSubmitMutex[0], IX_OSAL_WAIT_FOREVER);
    
    /* clear sent stall bit unless endpoint 0 doesn't unstall itself */
    if (UDCCS0 & UDC_UDCCS0_SST)
    {
        REG_SET(&registers->UDCCS0, UDC_UDCCS0_SST);
    }

    /* decode token */
    ixUSBEP0TokenDecode(device);
    
    if (ep0Control->currentToken == SETUP_TOKEN) /* SETUP active */
    {
        UINT32 offset = 0; /* setup packet offset */
       
        controlWriteError = FALSE; /* reset control write error flag on SETUP token */

        /* debug */
        IX_USB_TRACE("::> SETUP\n", 0, 0, 0, 0, 0, 0);

        /* increment SETUP counter */
        context->counters.setup++;
 
        if (ep0Control->state != IDLE)
        {
	    if (ep0Control->transferType == CONTROL_READ)
	    {
		crFailed++;

		IX_USB_VERBOSE_WARN_TRACE("USB: SETUP token prematurely ended Control Read transaction\n",
			  0, 0, 0, 0, 0, 0);
            }
	    else if (ep0Control->transferType == CONTROL_WRITE)
            {
		cwFailed++;

		IX_USB_VERBOSE_WARN_TRACE("USB: SETUP token prematurely ended Control Write transaction\n",
			   0, 0, 0, 0, 0, 0);
            }
            else if (ep0Control->transferType == CONTROL_NO_DATA)
	    {
		cnFailed++;

		IX_USB_VERBOSE_WARN_TRACE("USB: SETUP token prematurely ended Control No Data transaction\n",
			   0, 0, 0, 0, 0, 0);
            }
            else
            {
		IX_USB_VERBOSE_WARN_TRACE("USB: SETUP token in an *******>INVALID<******** transaction\n",
			   0, 0, 0, 0, 0, 0);
            }

            /* SETUP occurred in the middle of another transaction, reset endpoint */
            ixUSBEP0StateReset(device);
            ixUSBEndpointClear(device, ENDPOINT_0);
        }

	IX_USB_VERBOSE4_TRACE("USB: control packet: ", 0, 0, 0, 0, 0, 0);

        /* read packet */
        
        /* tm - workaround for 0x81 sillicon bug (SA = 1, OPR = 1, RNE = 0) */
        if ((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) == 0)
        {
             /* ignore RNE, forcibly read 8 bytes */
             for (offset = 0 ; offset < SETUP_PACKET_SIZE ; offset++)
             {
                  ep0Control->setupBuffer[offset] = DREG_GET(&registers->UDDR0);                    
	
	          if (offset == 0 && (REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE))
                  {
                      IX_USB_TRACE("USB: Oops, RNE came back up.\n", 0, 0, 0, 0, 0, 0);
                  }
             }
        }
        else
        {
          while (((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) != 0) /* receive FIFO not empty */
                  && (offset < SETUP_PACKET_SIZE))                     /* copy maximum 8 bytes */
          {
              ep0Control->setupBuffer[offset] = DREG_GET(&registers->UDDR0);

              IX_USB_VERBOSE4_TRACE("0x%02x ", (unsigned char) ep0Control->setupBuffer[offset],
              		   0, 0, 0, 0, 0);

              offset++;
          }
        }

   	/* we should exactly 8 bytes in the FIFO; if we have more then the incoming OUT
	   token has rewritten the FIFO with a payload and we have to discard the setup */
        if ((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) != 0)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                       IX_OSAL_LOG_DEV_STDERR,  
                       "USB: Control write error, there's still data in the EP0 FIFO:",
		       0, 0, 0, 0, 0, 0);

	    /* drain and discard FIFO */
            while((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) != 0)
            {
                 unsigned char extraData = (unsigned char) DREG_GET(&registers->UDDR0);

		 ixOsalLog(IX_OSAL_LOG_LVL_WARNING,
                           IX_OSAL_LOG_DEV_STDOUT,
                           "0x%02x", extraData, 0, 0, 0, 0, 0);
            }

            /* indicate the error condition further in this transaction */
            controlWriteError = TRUE;
	}

        /* a valid setup packet has exactly 8 bytes */
        if (offset != SETUP_PACKET_SIZE)
        {
            IX_USB_VERBOSE_WARN_TRACE("Warning, did not read a full setup packet (8 expected, %d read)\n", 
            	      offset, 0, 0, 0, 0, 0);
 
            /* mangled packet, clear OPR and SA, set IPR in case host follows with IN token - tm */
            REG_SET(&registers->UDCCS0, (UDC_UDCCS0_OPR | UDC_UDCCS0_SA | UDC_UDCCS0_IPR));

            ixUSBEP0StateReset(device);
            ixUSBEndpointClear(device, ENDPOINT_0);

            ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);

            return;
        }

	if (controlWriteError)
	{
	/* we'll discard all incoming payload within this transaction */
		ep0Control->expected = 0;

		/* erase junk setup buffer to known values */
		ep0Control->setupBuffer[0] = (unsigned char) 0xff;
		ep0Control->setupBuffer[1] = (unsigned char) 0xff;
		ep0Control->setupBuffer[2] = (unsigned char) 0xff;
		ep0Control->setupBuffer[3] = (unsigned char) 0xff;
		ep0Control->setupBuffer[4] = (unsigned char) 0xff;
		ep0Control->setupBuffer[5] = (unsigned char) 0xff;
		ep0Control->setupBuffer[6] = (unsigned char) 0xff;
		ep0Control->setupBuffer[7] = (unsigned char) 0xff;
	}
	else
	{
		/* decode the transfer type and direction from packet */
		bRequest = ixUSBEP0SetupPacketDecode(device);
		/* Set configured flag to TRUE on receiving SET_CONFIGURATION message from host driver*/
                if(bRequest == SET_CONFIGURATION_REQUEST) 
        	{
            	    context->configured = TRUE;
        	}
	}

	IX_USB_VERBOSE4_TRACE("Read SETUP packet, %d bytes\n", offset, 0, 0, 0, 0, 0);

	/* show control stats */
	IX_USB_TRACE("USB: Control read (%d/%d/%d)\n",
		      crInitiated, crCompleted, crFailed, 0, 0, 0);

	IX_USB_TRACE("USB: Control write (%d/%d/%d)\n",
		      cwInitiated, cwCompleted, cwFailed, 0, 0, 0);

	IX_USB_TRACE("USB: Control nodata (%d/%d)\n",
		      cnInitiated, cnFailed, 0, 0, 0, 0);

	/* reset transfer counter for data stage */
	ep0Control->transferred = 0;

	epData->transferAllowed = FALSE;

	ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);

	/* dispatch setup buffer */
	if (context->eventProcessor.setupCallback != NULL)
	{
            IX_USB_TRACE("USB: Callback on 8 bytes SETUP packet\n", 
                         0, 0, 0, 0, 0, 0);

	    context->eventProcessor.setupCallback(device, ep0Control->setupBuffer);
	}

	ixOsalMutexLock(&usbBufferSubmitMutex[0], IX_OSAL_WAIT_FOREVER);

        /* clear OPR and SA */
	REG_SET(&registers->UDCCS0, UDC_UDCCS0_OPR );
	REG_SET(&registers->UDCCS0, UDC_UDCCS0_SA );


        /* allow premature STATUS USB_IN on Control Write */
        if (ep0Control->transferType == CONTROL_WRITE)
        {
            /* set IPR */
            REG_SET(&registers->UDCCS0, UDC_UDCCS0_IPR);
        }
    	epData->transferAllowed = TRUE;

	ixUSBEP0RequestSend(device);
    }
    else if (ep0Control->currentToken == OUT_TOKEN) /* USB_OUT token received */
    {
        /* debug */
        IX_USB_TRACE("::> OUT\n", 0, 0, 0, 0, 0, 0);
 
        /* debug sanity check */
        if (ep0Control->transferType == CONTROL_READ && ep0Control->state != END_IN_XFER && ep0Control->state != ACTIVE_IN)
        {
            /* state machine in invalid mode */
            IX_USB_TRACE("USB: internal error in state machine (current: %d)\n", 
                       ep0Control->state,
            	       0, 0, 0, 0, 0);
        }

        if (ep0Control->state == END_IN_XFER)
        {
            /* consume token and set state machine to IDLE */

            ixUSBEP0SendCleanup(device);

            IX_USB_VERBOSE5_TRACE("::> UDCCS0 is 0x%02X on EXIT\n", UDCCS0, 0, 0, 0, 0, 0);

            /* clear OPR */
            REG_SET(&registers->UDCCS0, UDC_UDCCS0_OPR);

	    ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);

	    crCompleted++;
        
            ixUSBDataSendAllow(device);

            return;
        }
        else if (ep0Control->state == ACTIVE_IN)
        {
            /* premature STATUS, host won't accept more data */

            if (ep0Control->transferred == ep0Control->expected)
            {
                /* transfer was completed but no 0-length
                   packet was sent */

                ixUSBEP0SendCleanup(device);
                crCompleted++;
            }
            else
            {
                /* ERROR, STATUS stage entered before transfer completion */

                ixUSBEP0StateReset(device);
                ixUSBEndpointClear(device, ENDPOINT_0);
                crFailed++;
            }

            /* clear OPR */
            REG_SET(&registers->UDCCS0, UDC_UDCCS0_OPR);
            UDCCS0 = REG_GET(&registers->UDCCS0);
            IX_USB_VERBOSE5_TRACE("::> UDCCS0 is 0x%02X on EXIT\n", UDCCS0, 0, 0, 0, 0, 0);

	    ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);

            return;
        }
        else if (ep0Control->state == IDLE) /* First USB_OUT packet */
        {
            ep0Control->state = ACTIVE_OUT;

            IX_USB_VERBOSE5_TRACE("::> Beginning OUT transaction, %d bytes expected\n", ep0Control->expected, 0, 0, 0, 0, 0);
            
            /* alloc recv buffer */
            epData->currentBuffer = ixUSBBufferAlloc(ep0Control->expected);
        }

        IX_USB_VERBOSE5_TRACE("::> Reading FIFO at offset %d\n", epData->currentOffset, 0, 0, 0, 0, 0);

	if (controlWriteError)
	{
	    /* invalid transaction, drain FIFO and discard data */
	    while ((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) != 0)
	    {
	    *((unsigned char *)(IX_USB_MBLK_DATA(epData->currentBuffer) + epData->currentOffset))
				= DREG_GET(&registers->UDDR0);

	    epData->currentOffset++;
	    }
	}
	else
	{
            /* read FIFO until RNE == 0, appending to recv buffer */
            while ((REG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) != 0     /* receive FIFO not empty */
                	&& epData->currentOffset < ep0Control->expected) /* stop when all expected data was received */
            {
                *((unsigned char *)(IX_USB_MBLK_DATA(epData->currentBuffer) + epData->currentOffset)) = DREG_GET(&registers->UDDR0);

                epData->currentOffset++;
                ep0Control->transferred++;
            }
	}

	IX_USB_TRACE("USB: => Read %d bytes so far in this control write, %d expected\n",
		     ep0Control->transferred, ep0Control->expected, 0, 0, 0, 0);

        /* check for end of transfer */
        if (ep0Control->transferred == ep0Control->expected)
        {
            ep0Control->state = END_OUT_XFER;
        }

        /* allow USB_IN STATUS - set IPR */
        REG_SET(&registers->UDCCS0, UDC_UDCCS0_IPR);

        /* clear OPR */
        REG_SET(&registers->UDCCS0, UDC_UDCCS0_OPR);

        /* Check for end of transfer and ship data *before* status stage
           WHY: Might miss the status stage which is signalled by an IN token */
        if (ep0Control->transferred == ep0Control->expected)
        {
            ixUSBEP0DataDeliver(device);
        }
    }
    else if (ep0Control->currentToken == IN_TOKEN) /* USB_IN token */
    {
        /* debug */
        if (controlWriteError)
        {
            IX_USB_TRACE("::> IN\n", 0, 0, 0, 0, 0, 0);
        }
 
        if (ep0Control->state == ACTIVE_OUT) 
        {
            /* Premature USB_IN stage */;
            ixUSBEP0StateReset(device);
            ixUSBEndpointClear(device, ENDPOINT_0);
            crFailed ++;
	    ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);

            return;
        }
        else if (ep0Control->state == END_OUT_XFER)
        {
    	    ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);

	    cwCompleted++;

            /* Control Write ended, deliver data */
            /* ixUSBEP0DataDeliver(device); */

            return;
        }

        /* unlock FIFO for next packet */
        epData->transferAllowed = TRUE;

        ixUSBEP0RequestSend(device); /* send data, if available */
    }
    else
    {
        IX_USB_VERBOSE4_TRACE("::> UNKNOWN TOKEN", 0, 0, 0, 0, 0, 0);
    }
    
    ixOsalMutexUnlock(&usbBufferSubmitMutex[0]);
}

/**
 * @fn PRIVATE void ixUSBEP0TokenDecode(USBDevice *device)
 *
 * @brief Decode the received USB token
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * @return none
 *
 * Decodes the USB token received on endpoint 0 based on the
 * state of the <b>SA</b> (Setup Active) and <b>OPR</b> (USB_OUT Packet Ready)
 * bits of <b>UDCCS0</b> (endpoint 0 control/status register).<br>
 * The decoded value is placed in the <i>ep0ControlData.currentToken</i>
 * field, component of <i>device->deviceContext</i>.
 *
 * @internal
 */

PRIVATE void 
ixUSBEP0TokenDecode(USBDevice *device)
{
    EP0ControlData *ep0Data = EP0CONTROL(device);
    UDCRegisters *registers = REGISTERS(device);
    UINT32 UDCCS0           = REG_GET(&registers->UDCCS0);
    BOOL SA                 = UDCCS0 & UDC_UDCCS0_SA;
    BOOL OPR                = UDCCS0 & UDC_UDCCS0_OPR;

    IX_USB_VERBOSE5_TRACE("::> Decode UDCCS0 is 0x%02X\n", UDCCS0, 0, 0, 0, 0, 0);
  
    if (SA && OPR)
    {
        ep0Data->currentToken = SETUP_TOKEN;
    }
    else if (OPR) /* || RNE is a hack */
    {
        ep0Data->currentToken = OUT_TOKEN;
    }
    else
    {
        ep0Data->currentToken = IN_TOKEN;
    }
}

/**
 * @fn PRIVATE void ixUSBEP0SetupPacketDecode(USBDevice *device)
 *
 * @brief Decode the received USB SETUP packet
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * @return none
 *
 * Decodes the 8-byte SETUP packet received on endpoint 0, extracting
 * the expected transfer type and length for the data stage of the
 * ongoing control transaction.<br>
 * Populates the <i>expected</i> and <i>transferType</i> fields of
 * the device->deviceContext.ep0ControlData structure.
 *
 * @internal
 */
PRIVATE UINT32 
ixUSBEP0SetupPacketDecode(USBDevice *device)
{
    EP0ControlData *ep0Data = EP0CONTROL(device);
    USBSetupPacket *controlPacket;
    
    controlPacket = (USBSetupPacket *) ep0Data->setupBuffer;

    IX_USB_VERBOSE4_TRACE("SETUP packet:\n[%02x %02x %02x %02x",
        ep0Data->setupBuffer[0],
        ep0Data->setupBuffer[1],
        ep0Data->setupBuffer[2],
        ep0Data->setupBuffer[3],
        0, 0);

    IX_USB_VERBOSE4_TRACE(" %02x %02x %02x %02x]\n",
        ep0Data->setupBuffer[4],
        ep0Data->setupBuffer[5],
        ep0Data->setupBuffer[6],
        ep0Data->setupBuffer[7],
        0, 0);

    SWAP_USB_WORD(&controlPacket->wIndex);
    SWAP_USB_WORD(&controlPacket->wLength);
    SWAP_USB_WORD(&controlPacket->wValue);

    /* debug */
    IX_USB_VERBOSE4_TRACE("SETUP packet decoding:\n  bmRequestType 0x%02X, bRequest %02X, wValue %04X, wIndex %04X, wLength %04X\n",
        controlPacket->bmRequestType, 
        controlPacket->bRequest, 
        controlPacket->wValue, 
        controlPacket->wIndex, 
        controlPacket->wLength,
        0);

    /* get expected transfer length */
    ep0Data->expected = controlPacket->wLength;

    /* get transfer type */
    if (ep0Data->expected == 0)
    {
        /* no data stage */
        ep0Data->transferType = CONTROL_NO_DATA;

        cnInitiated++;
    }
    else
    {
        /* there's data, check direction */
        if ((controlPacket->bmRequestType & UDC_DIRECTION_MASK) == USB_REQ_DIR_DEVICE_TO_HOST)
        {
            /* USB_IN transfer */
            ep0Data->transferType = CONTROL_READ;
            crInitiated++;
        }
        else
        {
            /* USB_OUT transfer */
            ep0Data->transferType = CONTROL_WRITE;
            cwInitiated++;            
        }
    }
    return(controlPacket->bRequest); 
}

/**
 * @fn PRIVATE void ixUSBEP0StateReset(USBDevice *device)
 *
 * @brief Reset the state of endpoint 0
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * @return none
 *
 * Resets the state of endpoint 0:
 *      - resets the endpoint 0 state machine
 *      - resets the current transfer indicators and buffers
 *
 * @internal
 */
PRIVATE void 
ixUSBEP0StateReset(USBDevice *device)
{
    EP0ControlData *ep0Control = EP0CONTROL(device);

    /* reset state machine */
    ep0Control->state = IDLE;

    /* clear control transfer traffic counters */
    ep0Control->transferred = 0;
    ep0Control->expected    = 0;

    /* clear transfer type */
    ep0Control->transferType = UNKNOWN_TRANSFER;

    /* clear active token type */
    ep0Control->currentToken = UNKNOWN_TOKEN;

    /* clear current SETUP packet */
    ixOsalMemSet(ep0Control->setupBuffer, 0, SETUP_PACKET_SIZE);
}

/**
 * @fn PRIVATE void ixUSBEP0RequestSend(USBDevice *device)
 *
 * @brief Service send requests for endpoint 0
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * @return none
 *
 * Fills the endpoint 0 FIFO, triggers device Tx, updates counters and
 * the endpoint 0 state machine when servicing send requests.
 *
 * @internal
 */
PRIVATE void 
ixUSBEP0RequestSend(USBDevice *device)
{
    USBDeviceContext *context       = CONTEXT(device);
    EPStatusData *epData            = &context->epStatusData[ENDPOINT_0];
    UDCRegisters *registers         = context->registers;
    EP0ControlData *ep0Control      = &context->ep0ControlData;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    UINT32 irqStatus;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
	/* EnterCriticalSection(&device->usbCriticalSection); */
     irqStatus = IX_USB_IRQ_LOCK;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

	if (epData->currentBuffer == NULL)
	{
		if (ixUSBBufferDequeue(epData))
		{
			/* set state machine status */
			ep0Control->state = ACTIVE_IN;
		}
		else
		{
#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
			/* queue is empty */
 			IX_USB_IRQ_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

			return;
		}
	}

    if (epData->transferAllowed && epData->currentBuffer != NULL)
    {
        unsigned int sendIndex;

        /* disable FIFO filling until the packet is shipped */
        epData->transferAllowed = FALSE;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
        IX_USB_IRQ_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

        epData->currentTransferSize = MIN(epData->fifoSize, IX_USB_MBLK_LEN(epData->currentBuffer) - epData->currentOffset);

        /* fill FIFO from send buffer */
        for (sendIndex = 0 ; sendIndex < epData->currentTransferSize ; sendIndex++)
        {
            UCHAR fifoData   = *(UCHAR *)(IX_USB_MBLK_DATA(epData->currentBuffer) + epData->currentOffset);
            UINT32 fifoWData = fifoData;

            DREG_SET(&registers->UDDR0, fifoWData);

            epData->currentOffset++;
            ep0Control->transferred++;
        }

        if (epData->currentTransferSize < epData->fifoSize) /* short packet? */
        {
            ep0Control->state = END_IN_XFER;
        }

        /* set IPR */
        REG_SET(&registers->UDCCS0, UDC_UDCCS0_IPR);
        	
	{
		UINT32 IPR = REG_GET(&registers->UDCCS0) & UDC_UDCCS0_IPR;

		do
		{
			IPR = REG_GET(&registers->UDCCS0) & UDC_UDCCS0_IPR;
		} while (IPR);

		REG_SET(&registers->USIR0, 1);

		ixUSBIPRset = 1;
	}

	IX_USB_VERBOSE4_TRACE("USB:=>Wrote %d so far in this read transaction out of %d [buffer 0x%08x offset %d]\n",
		   ep0Control->transferred,
		   IX_USB_MBLK_LEN(epData->currentBuffer),
		   (UINT32)epData->currentBuffer,
		   epData->currentOffset, 
                   0, 0);
    }
    else
    {
#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
        IX_USB_IRQ_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */
    }
}

/**
 * @fn PRIVATE void ixUSBEP0SendCleanup(USBDevice *device)
 *
 * @brief Finalize and clean up Tx transactions on endpoint 0
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * @return none
 *
 * Finalizes a control transfer:
 *      - sets endpoint 0 state machine to <b>IDLE</b>
 *      - increments Tx counters
 *      - frees sent buffer
 *
 * @internal
 */
PRIVATE void 
ixUSBEP0SendCleanup(USBDevice *device)
{
    USBDeviceContext *context = CONTEXT(device);
    EPStatusData *epData      = &context->epStatusData[ENDPOINT_0];

    /* increment sent counters */
    epData->counters.Tx++; 
    epData->counters.bytesTx += IX_USB_MBLK_LEN(epData->currentBuffer);
    context->counters.Tx++;
    context->counters.bytesTx += IX_USB_MBLK_LEN(epData->currentBuffer);

    /* free send buffer */
    IX_USB_MBLK_FREE(context->epStatusData[ENDPOINT_0].currentBuffer);

    /* reset transfer data */
    epData->currentBuffer       = NULL;
    epData->currentOffset       = 0;
    epData->currentTransferSize = 0;

    /* reset endpoint 0 state */
    ixUSBEP0StateReset(device);

    /* unlock FIFO */
    epData->transferAllowed = TRUE;
}

/**
 * @fn PRIVATE void ixUSBEP0DataDeliver(USBDevice *device)
 *
 * @brief Finalize and dispatch data for Rx transactions on endpoint 0
 *
 * @param device USBDevice * (in) - structure identifying the device
 *
 * @return none
 *
 * Sets the endpoint 0 state machine to IDLE, updates counters
 * and delivers incoming data by calling the registered client Rx callback.
 *
 * @internal
 */
PRIVATE void 
ixUSBEP0DataDeliver(USBDevice *device)
{
    USBDeviceContext *context = CONTEXT(device);
    EPStatusData *epData      = &context->epStatusData[ENDPOINT_0];

    context->ep0ControlData.state = IDLE; /* set state machine to idle */

    /* increment receive counters */
    epData->counters.Rx++; 
    epData->counters.bytesRx += IX_USB_MBLK_LEN(epData->currentBuffer);
    context->counters.Rx++;
    context->counters.bytesRx += IX_USB_MBLK_LEN(epData->currentBuffer);

    if (IX_USB_MBLK_LEN(epData->currentBuffer))
    {
	/* dispatch data - client is required to free buffer */
        IX_USB_VERBOSE4_TRACE("USB: Build 1003: Callback on EP0, %d bytes\n",
                   IX_USB_MBLK_LEN(epData->currentBuffer),
                   0, 0, 0, 0, 0);

    	context->eventProcessor.receiveCallback(device, ENDPOINT_0, epData->currentBuffer);
    }
    else
    {
        IX_USB_VERBOSE4_TRACE("USB: Build 1003: Ignoring 0 bytes payload\n", 
                   0, 0, 0, 0, 0, 0);

        IX_USB_MBLK_FREE(epData->currentBuffer);
    }

    /* reset transfer data */
    epData->currentBuffer       = NULL;
    epData->currentOffset       = 0;
    epData->currentTransferSize = 0;
}

/**
 * @fn PRIVATE void ixUSBQueueDiscard(EPStatusData *epData)
 *
 * @brief Discard and free the data queue of an endpoint
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * Frees the endpoint data queue (used only for Tx) and updates the
 * endpoint counters.
 *
 * @internal
 */
PRIVATE void
ixUSBQueueDiscard(EPStatusData *epData)
{
    USBDataQueue *queue = &(epData->queue);
    UINT32 local_index;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    UINT32 irqStatus;
    /* USBDevice *device = epData->device; */
    irqStatus = IX_USB_LOCK;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    for (local_index = 0 ; local_index < queue->len ; local_index++)
    {
        UINT32 offset = QUEUE_WRAP(queue->head + local_index);

        IX_USB_MBLK_FREE(queue->base[offset]);

        queue->base[offset] = NULL;
    }

    /* update counters - queues are actually used only for Tx */
    epData->counters.DTx += epData->queue.len;
    COUNTERS(epData->device)->DTx += epData->queue.len;

    queue->head = 0;
    queue->len  = 0;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    IX_USB_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */
}

/**
 * @fn PRIVATE void ixUSBQueueInit(EPStatusData *epData)
 *
 * @brief Initialize the data queue of an endpoint
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * @internal
 */
PRIVATE void
ixUSBQueueInit(EPStatusData *epData)
{
    epData->queue.head = 0;
    epData->queue.len  = 0;

    ixOsalMemSet(epData->queue.base, 0, MAX_QUEUE_SIZE * sizeof (IX_USB_MBLK *));
}

/**
 * @fn PRIVATE BOOL ixUSBBufferEnqueue(EPStatusData *epData, IX_USB_MBLK *buf)
 *
 * @brief Add a buffer to an endpoint data queue
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 * @param buf IX_USB_MBLK * (in) - buffer to enqueue
 *
 * @return <b>TRUE</b> if the buffer was successfully queued and 
 * <b>FALSE</b> otherwise (if the queue is full)
 *
 * @internal
 */
PRIVATE BOOL 
ixUSBBufferEnqueue(EPStatusData *epData, IX_USB_MBLK *buf)
{
    USBDataQueue *queue = &(epData->queue);
    BOOL result;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    /* lock section */
    /* USBDevice *device = epData->device; */
    UINT32 irqStatus = IX_USB_LOCK;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    if (queue->len < MAX_QUEUE_SIZE)
    {
        UINT32 tail = QUEUE_WRAP(queue->head + queue->len);

        queue->base[tail] = buf;
        queue->len++;

        result = TRUE;
    }
    else
    {
        result = FALSE; /* queue is full */
    }

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    IX_USB_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    return result;
}

/**
 * @fn PRIVATE BOOL ixUSBBufferDequeue(EPStatusData *epData)
 *
 * @brief Moves the top buffer from the endpoint queue into the
 * current transaction slot
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return <b>TRUE</b> if the dequeueing was completed successfully and
 * <b>FALSE</b> otherwise (if the queue is empty)
 *
 * @internal
 */
PRIVATE BOOL 
ixUSBBufferDequeue(EPStatusData *epData)
{
    USBDataQueue *queue = &(epData->queue);
    UINT32 lock;

    lock = IX_USB_LOCK;

    /* NULL buffer pointers can exist in the queue if buffers are cancelled */
    /* retry until a non-NULL buffer pointer is extracted or queue becomes empty */
    while (TRUE)
    {
        if (queue->len > 0)
        {
            epData->currentBuffer = queue->base[queue->head];

            /* remove entry from queue */
            queue->base[queue->head] = NULL;

            queue->len--;
            queue->head = QUEUE_WRAP(queue->head + 1);

            if (epData->currentBuffer != NULL)
            {
               epData->currentOffset = 0;
  	       
  	       IX_USB_UNLOCK(lock);

               return TRUE;
            }
        }
        else
        {
	    IX_USB_UNLOCK(lock);
        	
            return FALSE; /* queue is empty */
        }
    }
     IX_USB_UNLOCK(lock);
}


/**
 * @fn PRIVATE void ixUSBRequestSend(EPStatusData *epData)
 *
 * @brief Service send requests for USB_IN endpoints
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * Fills the endpoint FIFO and triggers UDC send.<br>
 * If transactioning is enabled it also automatically sends 
 * end-of-transfer short packets.<br>
 * Checks transaction timeouts if compiled with <b>IX_USB_HAS_TIMESTAMP_CHECKS</b>.
 *
 * @internal
 */
/*
Note:
TSP        - Transmit Short Packet
TPC        - Transmit Packet Complete
RSP        - Receive Short Packet
RPC        - Receive Packet Complete

All transfer-specific data is part of the EPStatusData structure.
This includes currentBuffer, currentOffset, currentTransferSize etc.
*/
PRIVATE void 
ixUSBRequestSend(EPStatusData *epData)
{
#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    /* USBDevice *device = epData->device; */
    UINT32 irqStatus = IX_USB_IRQ_LOCK;
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    /* Get current send buffer */
    if (epData->currentBuffer == NULL)
    {
        if (ixUSBBufferDequeue(epData) == FALSE)
        {
#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
            IX_USB_IRQ_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

            return; /* nothing to send */
        }
    }

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS

    else
    {
        /* check timeout */
        if (ixUSBTimestampCheck(epData) == FALSE)
        {
            /* dump current transaction */
            ixUSBTransferAbort(epData);

            /* re-service endpoint */
            ixUSBRequestSend(epData);
        }
    }

#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

    if (epData->transferAllowed)
    {
         /* lock FIFO until packet is shipped */
         epData->transferAllowed = FALSE;

#ifdef IX_USB_DMA

        if (epData->dmaEnabled) /* DMA mode */
        {
            /* clear TPC */

            /* current DMA packet:
                     offset = currentBuffer + currentOffset
             length = min(dmaSize, size(currentBuffer) - currentOffset)

             update currentTransferSize = length;
            */

            /* setup DMA channel, passing ixUSBSendCleanup(epData)
               as transfer_finished handler */
        }
        else /* IRQ mode */

#endif /* IX_USB_DMA */

        {
            /* current packet:
                offset = currentBuffer + currentOffset
                length = min(fifoSize, size(currentBuffer) - currentOffset)

                update currentTransferSize = length;
            */
            unsigned int sendIndex;

            epData->currentTransferSize = MIN(epData->fifoSize, IX_USB_MBLK_LEN(epData->currentBuffer) - epData->currentOffset);

            /* fill FIFO */
            for (sendIndex = 0 ; sendIndex < epData->currentTransferSize ; sendIndex++)
            {
                UCHAR value = *(UCHAR *)(IX_USB_MBLK_DATA(epData->currentBuffer) + epData->currentOffset);
                DREG_SET(epData->UDDR, (UINT32) value);

                epData->currentOffset++;
            }

            /* set TSP for short packet or 
             (last packet && USB_BULK USB_IN && enable Tx seq) */
	    if (epData->currentTransferSize < epData->fifoSize)
            {
                REG_SET(epData->UDCCS, UDC_UDCCS_TSP_IN);
            }
        }
    }

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS

    /* set packet timestamp */
    epData->lastTimestamp = ixUSBTimestampGet();

#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
        IX_USB_IRQ_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */
}

/**
 * @fn PRIVATE void ixUSBSendCleanup(EPStatusData *epData)
 *
 * @brief Finalize Tx transactions for USB_IN endpoints
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * Frees sent buffer and updates endpoint counters.
 *
 * @internal
 */
PRIVATE void 
ixUSBSendCleanup(EPStatusData *epData)
{
    USBDevice *device         = epData->device;
    USBDeviceContext *context = CONTEXT(device);
    UINT32 UDCCS              = REG_GET(epData->UDCCS);
	UINT32 zeroByteTx		  =	0;  

#ifdef __wince
    ixOsalMutexLock(&usbBufferSubmitMutex[epData->endpointNumber], IX_OSAL_WAIT_FOREVER);
#endif
    /* clear sent stall bit */
    if (epData->type != USB_ISOCHRONOUS && (UDCCS & UDC_UDCCS_SST_IN))
    {
        REG_SET(epData->UDCCS, UDC_UDCCS_SST_IN);
    }

    /* clear transmit underrun bit */
    if (UDCCS & UDC_UDCCS_TUR_IN)
    {
        REG_SET(epData->UDCCS, UDC_UDCCS_TUR_IN);

        epData->counters.fifoUnderruns++;
    }

    /* if we've actually sent data then cleanup */
    if (epData->currentBuffer != NULL
        && epData->currentOffset == (UINT32) IX_USB_MBLK_LEN(epData->currentBuffer))
    {

		if ( epData->type == USB_BULK    
           && epData->direction == USB_IN  
           && epData->currentTransferSize >= epData->fifoSize)
		    {
			     REG_SET(epData->UDCCS, UDC_UDCCS_TSP_IN);			
			     zeroByteTx = 1;
			}
#ifdef IX_USB_DMA

        if (epData->dmaEnabled)
        {
            /* set TSP - UDC won't fire until we set this */
            REG_SET(epData->UDCCS, UDC_UDCCS_TSP_IN);
        }

#endif /* IX_USB_DMA */

        /* update counters */
        epData->counters.Tx++;
        epData->counters.bytesTx  += IX_USB_MBLK_LEN(epData->currentBuffer);
        context->counters.Tx++;
        context->counters.bytesTx += IX_USB_MBLK_LEN(epData->currentBuffer);

        /* cleanup */
        epData->currentOffset       = 0;
        epData->currentTransferSize = 0;
        
        /* free send buffer */
        IX_USB_MBLK_FREE(epData->currentBuffer);
        epData->currentBuffer = NULL;
    }

#ifdef IX_USB_DMA

    /* DMA clears TPC automatically */
    if (!epData->dmaEnabled)

#endif

    {
        /* UDC won't accept more data until we clear TPC */
        REG_SET(epData->UDCCS, UDC_UDCCS_TPC_IN);
    }

    if(zeroByteTx==0)
	{                 
		/* unlock FIFO for next packet */
		epData->transferAllowed = TRUE;
		
		ixUSBRequestSend(epData); /* load next transfer */
	}

#ifdef __wince
    ixOsalMutexUnlock(&usbBufferSubmitMutex[epData->endpointNumber]);
#endif

}

/**
 * @fn PRIVATE void ixUSBReceiveProcess(EPStatusData *epData)
 *
 * @brief Process incoming data for USB_OUT endpoints
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * Reads incoming data from the endpoint FIFO and enables the UDC
 * to receive additional data by clearing the <b>RPC</b> (Receive
 * Packet Complete) bit.<br>
 * If Rx transaction sequencing is enabled it assembles received
 * data into larger blocks.<br>
 * Checks transaction timeouts if compiled with <b>IX_USB_HAS_TIMESTAMP_CHECKS</b>.
 *
 * @internal
 */
PRIVATE void 
ixUSBReceiveProcess(EPStatusData *epData)
{
    USBDevice *device           = epData->device;
    USBDeviceContext *context   = CONTEXT(device);
    UINT32 UDCCS                = REG_GET(epData->UDCCS);
    UINT32 UBCR                 = REG_GET(epData->UBCR);
    BOOL RNE                    = UDCCS & UDC_UDCCS_RNE_OUT;
    BOOL RSP                    = UDCCS & UDC_UDCCS_RSP_OUT;



#ifdef IX_USB_DMA

    UINT32 fullTransferSize     = epData->dmaEnabled ? epData->dmaSize : epData->fifoSize;

#else

    UINT32 fullTransferSize     = epData->fifoSize;

#endif /* IX_USB_DMA */

    /* debug */
    IX_USB_VERBOSE2_TRACE("entering ISR Rx: UBCR is %d, UDCCS is %x\n", UBCR, UDCCS, 0, 0, 0, 0);

    /* clear sent stall bit */
    if (epData->type != USB_ISOCHRONOUS && (UDCCS & UDC_UDCCS_SST_OUT))
    {
        REG_SET(epData->UDCCS, UDC_UDCCS_SST_OUT);
    }

    /* reset current transfer size */
    epData->currentTransferSize = 0;

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS
    if (epData->currentOffset > 0) /* transaction in progress? */
    {
        /* check timeout */
        if (ixUSBTimestampCheck(epData) == FALSE)
        {
            /* dump current transaction */
            ixUSBTransferAbort(epData);
        }
    }
#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

    /* silence isochronous fifo receive overflow as it keeps the ISR alive */
    if (epData->type == USB_ISOCHRONOUS && (UDCCS & UDC_UDCCS_ROF_OUT))
    {
        /* clear receive overflow bit */
        REG_SET(epData->UDCCS, UDC_UDCCS_ROF_OUT);

        epData->counters.fifoOverflows++;
    }


    /* abandon this transfer if the FIFO is empty unless we have a zero-length packet */
    if (!RSP && !RNE)
    {
        /* it's an error to have RPC set here but let's avoid getting stuck in infinite loops */
        REG_SET(epData->UDCCS, UDC_UDCCS_RPC_OUT);

        return;
    }


    if (epData->currentBuffer == NULL)
    {
        UINT32 expectedSize;

        /* get buffer for this transaction */
        if (epData->type == USB_BULK)
        {
            if ((epData->device->flags & ENABLE_RX_SEQ) != 0)
            {
                expectedSize = MAX_TRANSFER_SIZE;
            }
            else
            {
                expectedSize = BULK_PACKET_SIZE;
            }
        }
        else if (epData->type == USB_ISOCHRONOUS)
        {
            expectedSize = ISOCHRONOUS_PACKET_SIZE;
        }
        else
        {
            IX_USB_ASSERT(epData->type == USB_INTERRUPT);

            expectedSize = INTERRUPT_PACKET_SIZE;
        }

        if (ixUSBBulkNakThrottling && ixUSBPoolCounter <= POOL_THRESHOLD)
        {
            /* cannot allocate, we have only enough buffers left for control transactions */
            epData->currentBuffer = NULL;
        }
        else
        {
            epData->currentBuffer = ixUSBBufferAlloc(expectedSize);
        }

        epData->currentOffset = 0;

        /* check for allocation failure */
        if (epData->currentBuffer == NULL)
        {
            if (ixUSBBulkNakThrottling)
            {
                if (epData->type == USB_BULK)
                {
                    /* disable endpoint until there are more buffers */
                    if (epData->endpointNumber < 8)
                    {
                        /* use UICR0 */
                        REG_SET(&context->registers->UICR0, REG_GET(context->registers->UICR0) | (1 << epData->endpointNumber));
                    }
                    else
                    {
                        /* use UICR1 */
                        REG_SET(&context->registers->UICR1, REG_GET(context->registers->UICR1) | (1 << (epData->endpointNumber - 8)));
                    }

                    return;
                }
            }
            else
            {
                ixOsalLog(IX_OSAL_LOG_LVL_FATAL, IX_OSAL_LOG_DEV_STDOUT, "USB: Buffer pool depleted or allocation failure\n", 0, 0, 0, 0, 0, 0);
                IX_OSAL_ASSERT(0);
            }
        }
    }

#ifdef IX_USB_DMA

    if (epData->dmaEnabled && !RSP)
    {
        /* DMA transfer complete, update offset */
        epData->currentOffset = epData->currentOffset + epData->dmaSize;

        epData->currentTransferSize = epData->dmaSize;
    }
    else

#endif
    if (RNE)
    {
        UINT32 readIndex;

        /* debug */
        IX_USB_VERBOSE2_TRACE("ISR Rx: %d bytes in FIFO, %d in the current packet, current offset %d\n",
            UBCR + 1, IX_USB_MBLK_LEN(epData->currentBuffer), epData->currentOffset, 0, 0, 0);
        
        /* read UBCR + 1 bytes from FIFO as UBCR (i.e. UDC Byte Count Register) is biased by 1 */
        for (readIndex = 0 
             ; (readIndex < UBCR + 1) && (epData->currentOffset < (UINT32) IX_USB_MBLK_LEN(epData->currentBuffer)) 
             ; readIndex++)
        {
            /* read FIFO into currentBuffer + currentOffset */
            *((UCHAR *)(IX_USB_MBLK_DATA(epData->currentBuffer) + epData->currentOffset)) 
                = DREG_GET(epData->UDDR);

            epData->currentOffset++;
            epData->currentTransferSize++;
        }

        /* debug */
        IX_USB_VERBOSE2_TRACE("ISR Rx: read %d bytes from FIFO\n", readIndex, 0, 0, 0, 0, 0);
    }
    else
    {
        IX_USB_VERBOSE2_TRACE("RNE not Set in RECEIVE PROCESS.......\n", 0, 0, 0, 0, 0, 0);
    }


    /* check for transaction completion */
    if (epData->type != USB_BULK 
        || epData->currentTransferSize < fullTransferSize
        || (epData->device->flags & ENABLE_RX_SEQ) == 0)
    {
        /* transaction complete, increment counters */
        epData->counters.Rx++;
        epData->counters.bytesRx += epData->currentOffset;
        context->counters.Rx++;
        context->counters.bytesRx += epData->currentOffset;

        /* adjust the IX_USB_MBLK length to the current transfer size */
        IX_USB_MBLK_LEN(epData->currentBuffer) = epData->currentOffset;

       /* call receive callback, client frees buffer */
        IX_USB_VERBOSE2_TRACE("USB: Callback on EP%d, %dbytes, 0x%x\n",
                epData->endpointNumber,
                IX_USB_MBLK_LEN(epData->currentBuffer),
                (UINT32)IX_USB_MBLK_DATA(epData->currentBuffer),
                0, 0, 0);

	{
		IX_USB_MBLK *receivedBuffer = epData->currentBuffer;

		epData->currentBuffer = NULL;

	        context->eventProcessor.receiveCallback(device, epData->endpointNumber, receivedBuffer);
		IX_USB_VERBOSE2_TRACE("Rx Callback Done\n", 0, 0, 0, 0, 0, 0);
	}

    }
    else
    {
        /* more packets to come in this transaction */
#ifdef IX_USB_HAS_TIMESTAMP_CHECKS
        /* set timestamp */
        epData->lastTimestamp = ixUSBTimestampGet();
#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */
    }

#ifdef IX_USB_DMA

    if (epData->dmaEnabled)
    {
        /* @na set up DMA transfer for the next packet 
            into currentBuffer for dmaSize bytes */
    }

#endif

    /* until RPC is cleared the UDC rejects all incoming data */
    REG_SET(epData->UDCCS, UDC_UDCCS_RPC_OUT);

    /* debug */
    IX_USB_VERBOSE2_TRACE("exiting ISR Rx: UBCR is %d, UDCCS is %x\n", REG_GET(epData->UBCR), REG_GET(epData->UDCCS), 0, 0, 0, 0);
}

/**
 * @fn PRIVATE void ixUSBTransferAbort(EPStatusData *epData)
 *
 * @brief Abort a transfer on an endpoint
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return none
 *
 * Discards and frees the current transfer on an endpoint.
 * Updates the endpoint counters.
 *
 * @warning Does not discard the entire Tx data queue
 *
 * @internal
 */
PRIVATE void
ixUSBTransferAbort(EPStatusData *epData)
{
    USBDeviceContext *context = CONTEXT(epData->device);
    UDCRegisters *registers   = context->registers;
    UINT16 direction          = ixUSBTransferDirectionGet(epData);

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    /* USBDevice *device = epData->device; */
    UINT32 irqStatus;
    irqStatus   = IX_USB_LOCK;

#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

    IX_USB_MBLK_FREE(epData->currentBuffer);

    /* clear FIFO */
    if (epData->endpointNumber == ENDPOINT_0)
    {
        /* set EP0 FTF to empty FIFO */
        REG_SET(&registers->UDCCS0, UDC_UDCCS0_FTF);
    }
    else
    {
        /* set USB_IN/USB_OUT FTF to empty FIFO */
        REG_SET(epData->UDCCS, UDC_UDCCS_FTF);
    }

    /* if there's a buffer in transit increment the dropped packet counters */
    if (epData->currentBuffer != NULL)
    {
        if (direction == USB_IN)
        {
            epData->counters.DTx++;
            context->counters.DTx++;
        }
        else if (direction == USB_OUT)
        {
            epData->counters.DRx++;
            context->counters.DRx++;
        }
    }

    epData->currentBuffer       = NULL;
    epData->currentOffset       = 0;
    epData->currentTransferSize = 0;

    /* unlock FIFO */
    epData->transferAllowed = TRUE;

#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS
    IX_USB_UNLOCK(irqStatus);
#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */
}

/**
 * @fn PRIVATE UINT16 ixUSBTransferDirectionGet(EPStatusData *epData)
 *
 * @brief Retrieve the transfer direction for an enpoint
 *
 * @param epData EPStatusData * (in) - structure identifying the endpoint
 *
 * @return UBS_NO_DATA, USB_IN or USB_OUT
 *
 * Simple USB_IN/USB_OUT endpoints can perform only their built-in transfer types
 * but for the control endpoint we need to peek at the control transfer
 * type as endpoint 0 is capable of both.
 *
 * @internal
 */
PRIVATE UINT16
ixUSBTransferDirectionGet(EPStatusData *epData)
{
    if (epData->type != USB_CONTROL)
    {
        return epData->direction;
    }
    else
    {
        UINT16 controlTransferType = EP0CONTROL(epData->device)->transferType;

        if (controlTransferType == CONTROL_READ)
        {
            return USB_IN;
        }
        else if (controlTransferType == CONTROL_WRITE)
        {
            return USB_OUT;
        }
        else
        {
            return USB_NO_DATA;
        }
    }
}

/**
 * @fn PRIVATE IX_USB_MBLK* ixUSBBufferAlloc(size_t size)
 *
 * @brief Alloc an IX_USB_MBLK buffer for incoming data
 *
 * @param size size_t (in) - buffer size
 *
 * @return pointer to the new IX_USB_MBLK buffer
 *
 * @warning Temporary function, uses malloc()
 *
 * @internal
 */
/* The PRIVATE macro has been removed as a workaround for SCR#2450 */
PUBLIC
IX_USB_MBLK* 
ixUSBBufferAlloc(size_t size)
{
    UINT32 irqStatus;

#ifdef IX_USB_HAS_DUMMY_MBLK

    return alloc_IX_USB_MBLK(size);

#else
    IX_USB_MBLK *mbuf;

    mbuf = IX_OSAL_MBUF_POOL_GET(pNetPool);
 
    if (mbuf != NULL)
    {
        /* adjust amount of available room */
        IX_USB_MBLK_LEN(mbuf) = size;

        /* lock section */ 
        irqStatus = IX_USB_LOCK; 

        ixUSBPoolCounter--;

		/* unlock section */
		IX_USB_UNLOCK(irqStatus);

    }
    else
    {
        /* this unfortunately is a non-recoverable error */
        IX_USB_VERBOSE_WARN_TRACE("USB: Fatal error, IX_OSAL_MBUF_POOL_GET(%d) returned NULL - no memory left\n", size, 0, 0, 0, 0 ,0);
    }
    
    return mbuf;

#endif
}

void ixUSBMblkFree(IX_USB_MBLK *buf)
{
    UINT32 irqStatus;

    if (buf != NULL)
    {
        /* lock section */ 
        irqStatus = IX_USB_LOCK; 

        IX_OSAL_MBUF_POOL_PUT(buf);
        ixUSBPoolCounter++;

        if (ixUSBBulkNakThrottling && (ixUSBPoolCounter > POOL_THRESHOLD))
        {
            REG_SET(&ixUSBGlobalContext->registers->UICR0, 0);
            REG_SET(&ixUSBGlobalContext->registers->UICR1, 0);
        }

		/* unlock section */
		IX_USB_UNLOCK(irqStatus);
    }

}

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS

#ifndef IX_USB_HAS_CUSTOM_TIMESTAMP_GET

/**
 * @fn PRIVATE UINT32 ixUSBTimestampGet()
 *
 * @brief Retrieve a 32-bit timestamp
 *
 * @return the 32-bit timestamp
 *
 * @internal
 */
PRIVATE UINT32
ixUSBTimestampGet()
{
    return IxOsalTimestampGet();
}

#endif /* IX_USB_HAS_CUSTOM_TIMESTAMP_GET */

/**
 * PRIVATE BOOL ixUSBTimestampCheck(EPStatusData *epData)
 *
 * @brief Check if an endpoint transaction has timed out
 *
 * @param epData EPStatusData * (in) - a structure identifying the endpoint
 *
 * @return <b>TRUE</b> if the transaction has timed out and
 * <b>FALSE</b> otherwise
 *
 * @internal
 */

PRIVATE BOOL
ixUSBTimestampCheck(EPStatusData *epData)
{
    UINT32 timeout   = 0;
    UINT16 direction = ixUSBTransferDirectionGet(epData);

    if (direction == USB_IN)
    {
        timeout = TRANSACTION_TIMEOUT_TX;
    }
    else if (direction == USB_OUT)
    {
        timeout = TRANSACTION_TIMEOUT_RX;
    }

    if (timeout > 0 &&
        (abs(ixUSBTimestampGet() - epData->lastTimestamp) > timeout))
    {
        /* transfer has timed out */
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

/**
 * @} addtogroup SupportAPI
 */
