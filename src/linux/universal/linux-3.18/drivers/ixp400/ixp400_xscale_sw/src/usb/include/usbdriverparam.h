/**
 * @file usbdriverparam.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file containes the USB driver parameters used by the USB driver
 *
 * Driver parameters - change as needed
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
 * @brief Control parameters for the IXP400 USB Device Driver
 *
 * @{
 */

#ifndef usbdriverparam_H

#ifndef __doxygen_HIDE

#define usbdriverparam_H

#endif /* __doxygen_HIDE */

/** Maximum data size for one transaction in bytes (bulk or control) */
#define MAX_TRANSFER_SIZE 2048

/** 
 * Maximum outgoing queue size per endpoint, in elements
 * Uses MAX_QUEUE_SIZE * (sizeof(void *)) bytes 
 */
#define MAX_QUEUE_SIZE 100

/** Memory pool for data transactions */
#define MEM_POOL_SIZE 10240

/** Maximum acceptable delay in transactions (timestamp ticks), Rx, 0 disables */
#define TRANSACTION_TIMEOUT_RX 500

/** Maximum acceptable delay in transactions (timestamp ticks), Tx, 0 disables */
#define TRANSACTION_TIMEOUT_TX 500

#ifdef IX_USB_DMA

/** Global DMA switch (#undef to disable) */
#undef IX_USB_ENABLE_DMA

/** DMA on/off constants */
#define DMA_ENABLED TRUE
#define DMA_DISABLED FALSE
#define NO_DMA_CHANNEL FALSE

/** Global DMA table - edit to allow only specific endpoints */
static BOOL EPDMAEnabledTable[] = 
{
#ifdef ENABLE_DMA
		
    /* EP_0 */ NO_DMA_CHANNEL,
    /* EP_1 */ DMA_ENABLED,
    /* EP_2 */ DMA_ENABLED,
    /* EP_3 */ DMA_ENABLED,
    /* EP_4 */ DMA_ENABLED,
    /* EP_5 */ NO_DMA_CHANNEL,
    /* EP_6 */ DMA_ENABLED,
    /* EP_7 */ DMA_ENABLED,
    /* EP_8 */ DMA_ENABLED,
    /* EP_9 */ DMA_ENABLED,
    /* EP_10 */ NO_DMA_CHANNEL,
    /* EP_11 */ DMA_ENABLED,
    /* EP_12 */ DMA_ENABLED,
    /* EP_13 */ DMA_ENABLED,
    /* EP_14 */ DMA_ENABLED,
    /* EP_15 */ NO_DMA_CHANNEL

#else

    /* EP_0 */ NO_DMA_CHANNEL,
    /* EP_1 */ DMA_DISABLED,
    /* EP_2 */ DMA_DISABLED,
    /* EP_3 */ DMA_DISABLED,
    /* EP_4 */ DMA_DISABLED,
    /* EP_5 */ NO_DMA_CHANNEL,
    /* EP_6 */ DMA_DISABLED,
    /* EP_7 */ DMA_DISABLED,
    /* EP_8 */ DMA_DISABLED,
    /* EP_9 */ DMA_DISABLED,
    /* EP_10 */ NO_DMA_CHANNEL,
    /* EP_11 */ DMA_DISABLED,
    /* EP_12 */ DMA_DISABLED,
    /* EP_13 */ DMA_DISABLED,
    /* EP_14 */ DMA_DISABLED,
    /* EP_15 */ NO_DMA_CHANNEL

#endif
};

#endif /* IX_USB_DMA */

#endif /* usbdriverparam_H */

/**
 * @} addtogroup IxUsbAPI
 */
