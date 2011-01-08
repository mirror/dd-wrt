/*
 * file usbprivate.h
 *
 * author Intel Corporation
 * date 30-OCT-2001
 *
 * This file containes the private USB Driver API and global data
 *
 */

/** 
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

/*
 * Internal support API, part of the USB driver
 *
 */

#ifndef usbprivate_H
#define usbprivate_H

#include "usbprivatetypes.h"

PRIVATE UINT32
ixUSBIrqRead(void);

PRIVATE void 
ixUSBInterruptHandlerWrapper(USBDevice *device);

PRIVATE UINT32 
ixUSBInterruptHandler(USBDevice *device);

PRIVATE void 
ixUSBINInterruptHandler(EPStatusData *epData);

PRIVATE void 
ixUSBOUTInterruptHandler(EPStatusData *epData);

PRIVATE void 
ixUSBEP0InterruptHandler(EPStatusData *epData);

PRIVATE void 
ixUSBEP0TokenDecode(USBDevice *device);

PRIVATE UINT32 
ixUSBEP0SetupPacketDecode(USBDevice *device);

PRIVATE void 
ixUSBEP0StateReset(USBDevice *device);

PRIVATE void
ixUSBEP0RequestSend(USBDevice *device);

PRIVATE void 
ixUSBEP0DataDeliver(USBDevice *device);

PRIVATE void 
ixUSBEP0SendCleanup(USBDevice *device);

PRIVATE void 
ixUSBRequestSend(EPStatusData *epData);

PRIVATE void 
ixUSBSendCleanup(EPStatusData *epData);

PRIVATE void
ixUSBReceiveProcess(EPStatusData *epData);

PRIVATE void
ixUSBQueueInit(EPStatusData *epData);

PRIVATE BOOL 
ixUSBBufferEnqueue(EPStatusData *epData, IX_USB_MBLK *buf);

PRIVATE BOOL 
ixUSBBufferDequeue(EPStatusData *epData);

PRIVATE void
ixUSBQueueDiscard(EPStatusData *epData);

PRIVATE void
ixUSBTransferAbort(EPStatusData *epData);

PRIVATE UINT16
ixUSBTransferDirectionGet(EPStatusData *epData);

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS

#ifndef IX_USB_HAS_CUSTOM_TIMESTAMP_GET

PRIVATE UINT32
ixUSBTimestampGet(void);

#endif /* IX_USB_HAS_CUSTOM_TIMESTAMP_GET */

PRIVATE BOOL
ixUSBTimestampCheck(EPStatusData *epData);

#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

PRIVATE void
ixUSBNullReceiveCallback(
    USBDevice *device, 
    UINT16 sourceEndpoint, 
    IX_USB_MBLK *receiveBuffer);

PRIVATE void 
ixUSBNullEventCallback(USBDevice *device, USBEventSet events);

PRIVATE void 
ixUSBNullSetupCallback(USBDevice *device, const char *packet);

typedef void (*EPInterruptHandler) (EPStatusData *);

#endif /* usbprivate_H */

