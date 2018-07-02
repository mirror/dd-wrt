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

/** define to enable interrupt handler binding for VxWorks* */
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
