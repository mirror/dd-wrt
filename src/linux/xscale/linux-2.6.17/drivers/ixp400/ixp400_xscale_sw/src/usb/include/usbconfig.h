/**
 * @file usbconfig.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This temporary file containes the USB driver's component configuration
 *
 * This file will be replaced or heavily modified once the standard BSP configuration is added.
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
 * @brief USB Driver component configuration file
 *
 * @{
 */

#ifndef usbconfig_H

#ifndef __doxygen_HIDE

#define usbconfig_H

#endif /* __doxygen_HIDE */

/** define to enable ixUSBErrorStringGet() */
#define IX_USB_HAS_GET_ERROR_STRING

/** define to enable ixUSBEndpointInfoShow() */
#define IX_USB_HAS_ENDPOINT_INFO_SHOW

/** define to enable dummy mBlk type */
#undef IX_USB_HAS_DUMMY_MBLK

/** define to enable ixUSBStatisticsShow() */
#define IX_USB_HAS_STATISTICS_SHOW

/** define to enable timestamp checks for transaction timeouts */
#undef IX_USB_HAS_TIMESTAMP_CHECKS

#ifdef IX_USB_HAS_TIMESTAMP_CHECKS

/** 
 *  Define to enable client-defined timestamp function
 *  when defining this make sure to add a 1ms resolution (or less)
 *  timestamp function with the following signature:
 *
 *  UINT32
 *  ixUSBTimestampGet(void);
 *
 *  Note: makes sense to use this only when IX_USB_HAS_TIMESTAMP_CHECKS is defined
 */
#undef IX_USB_HAS_CUSTOM_TIMESTAMP_GET

#endif /* IX_USB_HAS_TIMESTAMP_CHECKS */

/** define to enable per-endpoint information in ixUSBStatisticsShow() */
#define IX_USB_STATS_SHOW_PER_ENDPOINT_INFO

/** define to enable debug tracing */
#undef IX_USB_HAS_TRACE_MACRO

/** define to enable verbose debug tracing */
#undef IX_USB_HAS_VERBOSE_TRACE_MACRO

/** define to enable level 2 verbose debug tracing */
#undef IX_USB_HAS_VERBOSE_2_TRACE_MACRO

/** define to enable level 3 verbose debug tracing */
#undef IX_USB_HAS_VERBOSE_3_TRACE_MACRO

/** define to enable level 4 verbose debug tracing */
#undef IX_USB_HAS_VERBOSE_4_TRACE_MACRO

/** define to enable level 4 verbose debug tracing */
#undef IX_USB_HAS_VERBOSE_5_TRACE_MACRO

/** define to enable memory operations tracing */
#undef IX_USB_HAS_VERBOSE_MEM_TRACE_MACRO

/** define to enable verbose warning tracing */
#define IX_USB_HAS_VERBOSE_WARN_TRACE_MACRO

/** define to enable init verbose tracing */
#undef IX_USB_HAS_VERBOSE_INIT_TRACE_MACRO

/** define to enable critical data sections locking */
#ifdef __linux
#define IX_USB_HAS_CRITICAL_DATA_LOCKS
#else
#undef IX_USB_HAS_CRITICAL_DATA_LOCKS
#endif /* __linux */


/** define to enable assertion macro */
#define IX_USB_HAS_ASSERT_MACRO

/** define to enable compile-time assertion macro */
#define IX_USB_HAS_CT_ASSERT_MACRO

/** define to enable interrupt handler binding for VxWorks */
#define IX_USB_HAS_INT_BIND_MACRO

/* hardware emulation specifics */
#ifdef __HWEMU__

#include <stdlib.h>
#define logMsg printf
#undef IX_USB_HAS_INT_BIND_MACRO

#endif /* __HWEMU__ */

#ifdef __linux
#define logMsg printk
#endif

#endif /* usbconfig_H */

/**
 * @} addtogroup IxUsbAPI
 */
