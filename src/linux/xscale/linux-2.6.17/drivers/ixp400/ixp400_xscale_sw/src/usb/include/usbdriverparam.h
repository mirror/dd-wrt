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
