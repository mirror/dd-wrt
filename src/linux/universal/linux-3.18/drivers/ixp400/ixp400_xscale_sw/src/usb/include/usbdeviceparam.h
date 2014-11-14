/**
 * @file usbdeviceparam.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file containes the USB device parameters used by the USB driver
 *
 * Device parameters:
 * - base address, number of endpoints, FIFO and packet sizes are UDC-dependent
 * - DMA channel buffer sizes can be customized and depend on the DMA controller
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
 * @brief Device parameters for the IXP400 USB Controller
 *
 * @{
 */

#ifndef usbdeviceparam_H

#ifndef __doxygen_HIDE

#define usbdeviceparam_H

#endif /* __doxygen_HIDE */

#include "IxOsal.h"

/** Base I/O address */
#define UDC_REGISTERS_BASE IX_OSAL_IXP400_USB_PHYS_BASE 

/** IRQ */
#define UDC_IRQ IX_OSAL_IXP400_USB_IRQ

/** Number of endpoints */
#define NUM_ENDPOINTS 16

/** SETUP packet size */
#define SETUP_PACKET_SIZE 8

/** CONTROL endpoint FIFO depth */
#define CONTROL_FIFO_SIZE 16

/** CONTROL endpoint packet size */
#define CONTROL_PACKET_SIZE 16

/** INTERRUPT endpoint FIFO depth */
#define INTERRUPT_FIFO_SIZE 8

/** INTERRUPT endpoint packet size */
#define INTERRUPT_PACKET_SIZE 8

/** BULK endpoint FIFO depth */
#define BULK_FIFO_SIZE 64

/** BULK endpoint packet size */
#define BULK_PACKET_SIZE 64

#ifdef IX_USB_DMA

/** BULK DMA channel buffer size */
#define BULK_DMA_SIZE 512

#endif /* IX_USB_DMA */

/** ISOCHRONOUS endpoint FIFO depth */
#define ISOCHRONOUS_FIFO_SIZE 256

/** ISOCHRONOUS endpoint packet size */
#define ISOCHRONOUS_PACKET_SIZE 256

#ifdef IX_USB_DMA

/** ISOCHRONOUS DMA channel buffer size */
#define ISOCHRONOUS_DMA_SIZE 256

#endif /* IX_USB_DMA */

#endif /* usbdeviceparam_H */

/**
 * @} addtogroup IxUsbAPI
 */
