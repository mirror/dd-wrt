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

PRIVATE void 
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

