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
