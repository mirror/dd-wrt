/**
 * @file usbmacros.h
 *
 * @author Intel Corporation
 * @date 30-OCT-2001
 *
 * @brief This file containes macros used by the USB driver implementation
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
 * @brief Definition of macros used by the USB driver code
 *
 * @{
 */

#ifndef usbmacros_H

#ifndef __doxygen_HIDE

#define usbmacros_H

#endif /* __doxygen_HIDE */

#include "IxOsal.h"

/* macros for extracting type and direction from EPDescriptorTable */

/**
 * @def EP_DIRECTION(x)
 *
 * @brief Macro used to extract the endpoint direction from
 * an EPDescriptorTable[] entry.
 *
 * @param x int (in) - the endpoint description entry
 *
 * @return the endpoint direction (USB_IN, USB_OUT or USB_IN_OUT)
 */
#define EP_DIRECTION(x) ((x) & (USB_IN | USB_OUT))

/**
 * @def EP_TYPE(x)
 *
 * @brief Macro used to extract the endpoint type from
 * an EPDescriptorTable[] entry.
 *
 * @param x int (in) - the endpoint description entry
 *
 * @return the endpoint type (USB_CONTROL, USB_BULK, USB_ISOCHRONOUS, USB_INTERRUPT)
 */
#define EP_TYPE(x) ((x) & (USB_CONTROL | USB_BULK | USB_ISOCHRONOUS | USB_INTERRUPT))

/* minimum */
#ifndef MIN
/**
 * @def MIN(a, b)
 *
 * @brief Compares two values and returns the minimum
 *
 * @param a - first value
 * @param b - second value
 *
 * @return minimum of the two input values
 */
#define MIN(a, b)       ((a) < (b)? (a) : (b))
#endif /* MIN */

/* maximum */
#ifndef MAX
/**
 * @def MAX(a, b)
 *
 * @brief Compares two values and returns the maximum
 *
 * @param a - first value
 * @param b - second value
 *
 * @return maximum of the two input values
 */
#define MAX(a, b)       ((a) > (b)? (a) : (b))
#endif /* MAX */

/* queue wrap */
/**
 * @def QUEUE_WRAP(tail)
 *
 * @brief Ajusts the tail of a queue implemented in a circular buffer
 * by wrapping at the buffer boundary
 *
 * @param tail int - virtual tail offset
 *
 * @return the real adjusted tail offset
 */
#define QUEUE_WRAP(tail)    ((tail) >= (MAX_QUEUE_SIZE) ? ((tail) - (MAX_QUEUE_SIZE)) : (tail))

#if defined(__BIG_ENDIAN)

/**
 * USB byte swapping routine for a big endian platform
 */
#define SWAP_USB_WORD(wPtr)     (*(wPtr)) = ((*(wPtr) & 0xFF00) >> 8) | \
                                            ((*(wPtr) & 0x00FF) << 8)

#else

/**
 * USB byte swapping routine for a little endian platform 
 */
#define SWAP_USB_WORD(wPtr)     if (0); 

#endif /* __HWEMU__ */

/* macros for reading/writing UDC registers */

#ifndef __HWEMU__

/** read generic register access via register pointers */
#define REG_GET(reg_ptr)          IX_OSAL_READ_LONG(reg_ptr)
/** write generic register access via register pointers */
#define REG_SET(reg_ptr, val)     IX_OSAL_WRITE_LONG(reg_ptr, val)

/** generic data register read access via register pointers */
#define DREG_GET(reg_ptr)         (IX_OSAL_READ_LONG(reg_ptr) & UDC_UDDR_RW_MASK)
/** generic data register write access via register pointers */
#define DREG_SET(reg_ptr, val)    IX_OSAL_WRITE_LONG(reg_ptr, val & UDC_UDDR_RW_MASK)

#else

/* prototypes */
/** Data register read access via register pointers */
UINT32 
reg32Get(volatile UINT32 *reg_ptr);

/** Data register write access via register pointers */
void
reg32Set(volatile UINT32 *reg_ptr, UINT32 val);

#define REG_GET(reg_ptr)          (reg32Get(reg_ptr))
#define REG_SET(reg_ptr, val)     (reg32Set(reg_ptr, val))

#define DREG_GET(reg_ptr)         REG_GET(reg_ptr)
#define DREG_SET(reg_ptr, val)    REG_SET(reg_ptr, val)

#endif /* __HWEMU__ */

/* macros to access device context data               */
/* NB: all return pointers so write access is allowed */

/** get context from device pointer */
#define CONTEXT(device)           ((USBDeviceContext *)((device)->deviceContext))

/** get registers from device pointer */
#define REGISTERS(device)         (CONTEXT(device)->registers)

/** get endpoint 0 control data from device pointer */
#define EP0CONTROL(device)        (&(CONTEXT(device)->ep0ControlData))

/** get event processor from device pointer */
#define EVENTS(device)            (&(CONTEXT(device)->eventProcessor))

/** get device counters */
#define COUNTERS(device)          (&(CONTEXT(device)->counters))

/** get device operation */
#define OPERATION(device)         (&(CONTEXT(device)->deviceOperation))

/** get endpoint status from device pointer and endpoint number */
#define EPSTATUS(device, endpointNumber)    (&(CONTEXT(device)->epStatusData[endpointNumber]))

/** get endpoint queue from device pointer and endpoint number */
#define EPQUEUE(device, endpointNumber)     (&(EPSTATUS((device), (endpointNumber))->queue))

/** get endpoint counters from device pointer and endpoint number */
#define EPCOUNTERS(device, endpointNumber)  (&(EPSTATUS((device), (endpointNumber))->counters))

/** set IX_SUCCESS on device and return IX_SUCCESS */
#define RETURN_OK(device)           \
    device->lastError = IX_SUCCESS; \
    return IX_SUCCESS;

/** set IX_USB_ERROR on device and return IX_FAIL */
#define RETURN_ERROR(device)           \
    device->lastError = IX_USB_ERROR;  \
    return IX_FAIL;

/** set IX_USB_INVALID_PARAMS on device and return IX_FAIL */
#define RETURN_INVALID_PARMS(device)           \
    device->lastError = IX_USB_INVALID_PARMS;  \
    return IX_FAIL;

/** set IX_USB_REDUNDANT on device and return IX_FAIL */
#define RETURN_REDUNDANT(device)           \
    device->lastError = IX_USB_REDUNDANT;  \
    return IX_FAIL;

/** set IX_USB_INVALID_PARAMS on device and return IX_FAIL */
#define RETURN_INVALID_DEVICE(device)           \
    device->lastError = IX_USB_INVALID_DEVICE;  \
    return IX_FAIL;

/** set IX_USB_INVALID_PARAMS on device and return IX_FAIL */
#define RETURN_NO_ENDPOINT(device)           \
    device->lastError = IX_USB_NO_ENDPOINT;  \
    return IX_FAIL;

/** set IX_USB_ENDPOINT_STALLED on device and return IX_FAIL */
#define RETURN_ENDPOINT_STALLED(device)           \
    device->lastError = IX_USB_ENDPOINT_STALLED;  \
    return IX_FAIL;

/** set IX_USB_SEND_QUEUE_FULL on device and return IX_FAIL */
#define RETURN_SEND_QUEUE_FULL(device)           \
    device->lastError = IX_USB_SEND_QUEUE_FULL;  \
    return IX_FAIL;

/** set IX_USB_NO_IN_CAPABILITY on device and return IX_FAIL */
#define RETURN_NO_IN_CAPABILITY(device)          \
    device->lastError = IX_USB_NO_IN_CAPABILITY; \
    return IX_FAIL;

/** set IX_USB_NO_STALL_CAPABILITY on device and return IX_FAIL */
#define RETURN_NO_STALL_CAPABILITY(device)          \
    device->lastError = IX_USB_NO_STALL_CAPABILITY; \
    return IX_FAIL;

/** set IX_USB_NO_PERMISSION on device and return IX_FAIL */
#define RETURN_NO_PERMISSION(device)          \
    device->lastError = IX_USB_NO_PERMISSION; \
    return IX_FAIL;

/** sanity checks for device existence */
#define CHECK_DEVICE(device)                                                \
    if (device == NULL)                                                     \
    {                                                                       \
        return IX_FAIL;                                                     \
    }                                                                       \
                                                                            \
    if (CONTEXT(device)->checkPattern != USB_DEVICE_CONTEXT_CHECK_PATTERN)  \
    {                                                                       \
        return IX_FAIL;                                                     \
    }                                               

/** sanity checks for device enable status */
#define CHECK_DEVICE_ENABLED(device)                \
    if (!CONTEXT(device)->enabled)                  \
    {                                               \
        device->lastError = IX_USB_DEVICE_DISABLED; \
        return IX_FAIL;                             \
    }

/** sanity checks for device cofigure status */         
#define CHECK_DEVICE_CONFIGURED(device)                   \
    if (!CONTEXT(device)->configured)                     \
    {                                                     \
        device->lastError = IX_USB_DEVICE_NOT_CONFIGURED; \
	ixOsalLog(IX_OSAL_LOG_LVL_ERROR,		  \
                  IX_OSAL_LOG_DEV_STDERR,		  \
                  "Device is not configured\n",		  \
                  0, 0, 0, 0, 0, 0); 			  \
        return IX_FAIL;                                   \
    }

/** sanity check for endpoint existence */
#define CHECK_ENDPOINT(device, endpointNumber)  \
    if (endpointNumber >= NUM_ENDPOINTS)        \
    {                                           \
        RETURN_NO_ENDPOINT(device);             \
    }

/** sanity check for endpoint stall */
#define CHECK_ENDPOINT_STALL(device, endpointNumber)                    \
    {                                                                   \
        BOOL stallState;                                                \
        ixUSBIsEndpointStalled(device, endpointNumber, &stallState);    \
                                                                        \
        if (stallState)                                                 \
        {                                                               \
            RETURN_ENDPOINT_STALLED(device);                            \
        }                                                               \
    }

/** sanity check for event masks */
#define CHECK_EVENT_MASK(device, eventMask)                       \
    if ((eventMask & ~(USB_BUS_EVENTS | USB_DEVICE_EVENTS)) != 0) \
    {                                                             \
        RETURN_INVALID_PARMS(device);                             \
    }

/** sanity check for endpoint queue size */
#define CHECK_ENDPOINT_QUEUE(epData)                \
    if (epData->queue.len == MAX_QUEUE_SIZE)        \
    {                                               \
        RETURN_SEND_QUEUE_FULL(epData->device);     \
    }

/** sanity check for endpoint IN capability */
#define CHECK_ENDPOINT_IN_CAPABILITY(epData, device) \
    if ((epData->direction & USB_IN) == 0)           \
    {                                                \
        RETURN_NO_IN_CAPABILITY(device);             \
    } 


#define IX_USB_LOG_DEVICE IX_OSAL_LOG_DEV_STDOUT

#ifdef __HWEMU__

/* enable all debugging */

#define IX_HWEMU_TRACE(format, a, b, c, d, e, f)                  \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                      \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_TRACE (format, a, b, c, d, e, f)                   \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                      \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_VERBOSE2_TRACE(format, a, b, c, d, e, f)           \
        (ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1,                       \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_VERBOSE3_TRACE(format, a, b, c, d, e, f)           \
        (ixOsalLog (IX_OSAL_LOG_LVL_DEBUG2,                       \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_VERBOSE4_TRACE(format, a, b, c, d, e, f)           \
        (ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3,                       \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_VERBOSE_MEM_TRACE(format, a, b, c, d, e, f)        \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                      \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_VERBOSE_INIT_TRACE(format, a, b, c, d, e, f)       \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                      \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#define IX_USB_VERBOSE_WARN_TRACE(format, a, b, c, d, e, f)       \
        (ixOsalLog (IX_OSAL_LOG_LVL_WARNING,                      \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#ifndef NDEBUG

#undef IX_USB_HAS_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_INIT_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_WARN_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_MEM_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_2_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_3_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_4_TRACE_MACRO
#undef IX_USB_HAS_VERBOSE_5_TRACE_MACRO

#endif

#define IX_USB_HAS_VERBOSE_TRACE_MACRO

#ifndef __doxygen_HIDE

#define IX_HWEMU_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* ndef __doxygen_HIDE */

#ifdef IX_USB_HAS_TRACE_MACRO

/** debug trace macro */
#define IX_USB_TRACE(format, a, b, c, d, e, f)               \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                 \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

/** no trace macro */
#define IX_USB_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_TRACE_MACRO */


#ifndef __doxygen_HIDE

#ifdef IX_USB_HAS_VERBOSE_TRACE_MACRO

#define IX_USB_VERBOSE_TRACE (format, a, b, c, d, e, f)        \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                   \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#define IX_USB_VERBOSE_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_TRACE_MACRO */

#ifndef IX_USB_HAS_VERBOSE_2_TRACE_MACRO

#define IX_USB_VERBOSE2_TRACE(format, a, b, c, d, e, f)        \
        (ixOsalLog (IX_OSAL_LOG_LVL_DEBUG1,                    \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#define IX_USB_VERBOSE2_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_2_TRACE_MACRO */

#ifdef IX_USB_HAS_VERBOSE_3_TRACE_MACRO

#define IX_USB_VERBOSE3_TRACE(format, a, b, c, d, e, f)         \
        (ixOsalLog (IX_OSAL_LOG_LVL_DEBUG2,                     \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#define IX_USB_VERBOSE3_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_3_TRACE_MACRO */

#ifdef IX_USB_HAS_VERBOSE_4_TRACE_MACRO

int usbTraceVerbose4 = 0;

#define IX_USB_VERBOSE4_TRACE(format, a, b, c, d, e, f)          \
         if (usbTraceVerbose4)                                   \
         {                                                       \
             ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3,                  \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f) \
         }

#else

#define IX_USB_VERBOSE4_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_4_TRACE_MACRO */

#ifdef IX_USB_HAS_VERBOSE_5_TRACE_MACRO

int usbTraceVerbose5 = 0;

#define IX_USB_VERBOSE5_TRACE(format, a, b, c, d, e, f)          \
         if (usbTraceVerbose5)                                   \
         {                                                       \
             ixOsalLog (IX_OSAL_LOG_LVL_DEBUG3,                  \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f) \
         }

#else

#define IX_USB_VERBOSE5_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_5_TRACE_MACRO */

#ifdef IX_USB_HAS_VERBOSE_MEM_TRACE_MACRO

#define IX_USB_VERBOSE_MEM_TRACE(format, a, b, c, d, e, f)      \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                    \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#define IX_USB_VERBOSE_MEM_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_MEM_TRACE_MACRO */

#ifdef IX_USB_HAS_VERBOSE_WARN_TRACE_MACRO

#define IX_USB_VERBOSE_WARN_TRACE(format, a, b, c, d, e, f)      \
        (ixOsalLog (IX_OSAL_LOG_LVL_WARNING,                     \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#define IX_USB_VERBOSE_WARN_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_WARN_TRACE_MACRO */

#ifdef IX_USB_HAS_VERBOSE_INIT_TRACE_MACRO

#define IX_USB_VERBOSE_INIT_TRACE(format, a, b, c, d, e, f)     \
        (ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,                    \
                    IX_USB_LOG_DEVICE, format, a, b, c, d, e, f))

#else

#define IX_USB_VERBOSE_INIT_TRACE(format, a, b, c, d, e, f) if (0); /* nothing */

#endif /* IX_USB_HAS_VERBOSE_INIT_TRACE_MACRO */

#endif /* __HWEMU__ */

#ifdef IX_USB_HAS_ASSERT_MACRO

/** assert macro */
#define IX_USB_ASSERT(expr) \
    if (expr == FALSE) \
    { \
        IX_USB_TRACE("Assertion failed\n", 0, 0, 0, 0, 0, 0); \
        IX_OSAL_ASSERT(expr); \
    }

#else

#ifndef __doxygen_HIDE

#define IX_USB_ASSERT(expr) if (0); /* nothing */

#endif /* ndef  __doxygen_HIDE */

#endif /* IX_USB_HAS_ASSERT_MACRO */

#ifdef IX_USB_HAS_CT_ASSERT_MACRO

/** compile-time assert macro (hack) */
#define IX_USB_CT_ASSERT(expr) \
    switch (0) { case 0: case (expr):; }

#else

#ifndef __doxygen_HIDE

#define IX_USB_CT_ASSERT(expr) if (0); /* nothing */

#endif /* ndef  __doxygen_HIDE */

#endif /* IX_USB_HAS_CT_ASSERT_MACRO */

#endif /* ndef __doxygen_HIDE */


#ifdef IX_USB_HAS_CRITICAL_DATA_LOCKS

/** critial data section lock */
#define IX_USB_LOCK ixOsalIrqLock()

/** critial data section unlock */
#define IX_USB_UNLOCK(state) ixOsalIrqUnlock(state)

/** irq lock */
#define IX_USB_IRQ_LOCK ixOsalIrqLock()

/** irq unlock */
#define IX_USB_IRQ_UNLOCK(state) ixOsalIrqUnlock(state)

#else

/** dummy critial data section lock */
#define IX_USB_LOCK 0

/** dummy critial data section unlock */
#define IX_USB_UNLOCK(state)

/** dummy irq lock */
#define IX_USB_IRQ_LOCK 0

/** dummy irq unlock */
#define IX_USB_IRQ_UNLOCK(state)

#endif /* IX_USB_HAS_CRITICAL_DATA_LOCKS */

#ifdef IX_USB_HAS_INT_BIND_MACRO

/** USB bind macro */
#define INT_BIND_MACRO(level, callback, instance) { ixOsalIrqBind((int) level, (void (*) (void *)) callback, (void *) instance); }

#else
#ifndef __doxygen_HIDE

#define INT_BIND_MACRO(level, callback, instance)   /* nothing */

#endif /* ndef __doxygen_HIDE */

#endif /* ndef IX_USB_HAS_INT_BIND_MACRO */

#ifndef __doxygen_HIDE

/* show macros */
#define SHOW_NUMBER(x)    ((x) > 1000 ? (x) > 1000000 ? ((x) / 1000000) : ((x) / 1000) : (x))
#define SHOW_METRIC(x)    ((x) > 1000 ? (x) > 1000000 ? "M" : "k" : " ")

#endif /* ndif doxygen_HIDE */

#define IX_USB_DRAIN_FIFO(registers) \
        while (DREG_GET(&registers->UDCCS0) & UDC_UDCCS0_RNE) \
            DREG_GET(&registers->UDDR0);

#endif /* usbmacros_H */

/**
 * @} addtogroup IxUsbAPI 
 */
