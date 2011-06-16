/**
 * @file IxOsalTypes.h
 *
 * @brief Define OSAL basic data types.
 * 
 * This file contains fundamental data types used by OSAL.
 *
 * @par
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 */


#ifndef IxOsalTypes_H
#define IxOsalTypes_H
#ifdef __cplusplus
extern "C"{
#endif

/* 
 * Include the OS-specific type definitions 
 */
#include "IxOsalOsTypes.h"
/**
 * @defgroup IxOsalTypes Osal basic data types.
 *
 * @brief Basic data types for Osal
 *
 * @{
 */

/**
 * @typedef IX_STATUS 
 * @brief OSAL status
 *
 * @note Possible OSAL return status include IX_SUCCESS and IX_FAIL. 
 */
typedef UINT32 IX_STATUS; 

/**
 * @brief VUINT32
 *
 * @note volatile UINT32
 */
typedef volatile UINT32 VUINT32;

/**
 * @brief VINT32
 *
 * @note volatile INT32
 */
typedef volatile INT32 VINT32;


/**
 * @ingroup IxOsalTypes
 *
 * @def NUMELEMS 
 *
 * @brief  Calculate number of elements
 */
#ifndef NUMELEMS
#define NUMELEMS(x) (sizeof(x) / sizeof((x)[0]))
#endif


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_BILLION
 *
 * @brief  Alias for 1,000,000,000
 *
 */
#define IX_OSAL_BILLION (1000000000)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_MILLION
 *
 * @brief  Alias for 1,000,000
 *
 */
#define IX_OSAL_MILLION (1000000)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_THOUSAND
 *
 * @brief  Alias for 1,000
 *
 */
#define IX_OSAL_THOUSAND (1000)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_HUNDRED
 *
 * @brief  Alias for 100
 *
 */
#define IX_OSAL_HUNDRED (100)

/**
 * @ingroup IxOsalTypes
 *
 * @def TRUE
 *
 * @brief Define for True
 */
#ifndef TRUE
#define TRUE       1L 
#endif

#if TRUE != 1
#error TRUE is not defined to 1
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def FALSE
 *
 * @brief Define for False
 */
#ifndef FALSE
#define FALSE      0L 
#endif

#if FALSE != 0
#error FALSE is not defined to 0
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def NULL 
 *
 * @brief Define for Null
 */
#ifndef NULL
#define NULL       0L 
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_SUCCESS
 *
 * @brief Success status
 *
 */
#ifndef IX_SUCCESS
#define IX_SUCCESS 0L /**< #defined as 0L */
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_FAIL
 *
 * @brief Failure status 
 *
 */
#ifndef IX_FAIL
#define IX_FAIL    1L /**< #defined as 1L */
#endif


/**
 * @ingroup IxOsalTypes
 *
 * @def PRIVATE
 *
 * @brief Private define
 */
#ifndef PRIVATE
#ifdef IX_PRIVATE_OFF
#define PRIVATE			/* nothing */
#else
#define PRIVATE static	/**< #defined as static, except for debug builds */
#endif /* IX_PRIVATE_OFF */
#endif /* PRIVATE */

/*
 *  Placeholder for future use
 */
#ifndef RESTRICTED
#define RESTRICTED
#endif /* RESTRICTED */

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_INLINE
 *
 * @brief Alias for __inline
 *
 */
#ifndef _DIAB_TOOL

#ifndef IX_OSAL_INLINE
#define IX_OSAL_INLINE __inline
#endif /* IX_OSAL_INLINE */

/**
 * @ingroup IxOsalTypes
 *
 * @def __inline__ 
 *
 * @brief Alias for __inline 
 */
#ifndef __inline__
#define __inline__   IX_OSAL_INLINE
#endif

#else

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_INLINE
 *
 * @brief Alias for __inline 
 */
#ifndef IX_OSAL_INLINE
#define IX_OSAL_INLINE __inline__  /* Diab Compiler uses __inline__ (compiler di
                                      rective) */
#endif /* IX_OSAL_INLINE */

#endif /*_DIAB_TOOL*/


/* Each OS can define its own PUBLIC, otherwise it will be empty. */
#ifndef PUBLIC
#define PUBLIC
#endif /* PUBLIC */


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_INLINE_EXTERN
 *
 * @brief Alias for __inline extern
 *
 */
#ifndef IX_OSAL_INLINE_EXTERN
#define IX_OSAL_INLINE_EXTERN IX_OSAL_INLINE extern
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_USES_ATTRIBUTE_PACKED
 *
 * @brief Defining packed attribute type in compiler/OS that supports it.
 *
 */
#ifndef IX_OSAL_USES_ATTRIBUTE_PACKED
#define IX_OSAL_USES_ATTRIBUTE_PACKED        TRUE
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_MAX_ALIGNMENT
 *
 * @brief Defining maximum alignment supported for aligned memory allocation.
 *
 */
#define IX_OSAL_MAX_ALIGNMENT       256

#ifndef MILLISEC_TO_SEC_FACTOR
#define MILLISEC_TO_SEC_FACTOR 1000 
#endif /* MILLISEC_TO_SEC_FACTOR */



/**
 * @ingroup IxOsalTypes
 * @enum IxOsalLogDevice
 * @brief This is an emum for OSAL log devices.
 */
typedef enum
{
    IX_OSAL_LOG_DEV_STDOUT = 0,	       /**< standard output (implemented by default) */
    IX_OSAL_LOG_DEV_STDERR = 1,	       /**< standard error (implemented */
    IX_OSAL_LOG_DEV_HEX_DISPLAY = 2,   /**< hexadecimal display (not implemented) */
    IX_OSAL_LOG_DEV_ASCII_DISPLAY = 3  /**< ASCII-capable display (not implemented) */
} IxOsalLogDevice;


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_LOG_ERROR
 *
 * @brief Alias for -1, used as log function error status
 *
 */
#define IX_OSAL_LOG_ERROR   (-1)
#define IX_OSAL_NO_LOG      (0)

/**
 * @ingroup IxOsalTypes
 * @enum IxOsalLogLevel
 * @brief This is an emum for OSAL log trace level.
 */
typedef enum
{
    IX_OSAL_LOG_LVL_NONE = 0,	 /**<No trace level */
    IX_OSAL_LOG_LVL_USER = 1,	 /**<Set trace level to user */
    IX_OSAL_LOG_LVL_FATAL = 2,	 /**<Set trace level to fatal */
    IX_OSAL_LOG_LVL_ERROR = 3,	 /**<Set trace level to error */
    IX_OSAL_LOG_LVL_WARNING = 4, /**<Set trace level to warning */
    IX_OSAL_LOG_LVL_MESSAGE = 5, /**<Set trace level to message */
    IX_OSAL_LOG_LVL_DEBUG1 = 6,	 /**<Set trace level to debug1 */
    IX_OSAL_LOG_LVL_DEBUG2 = 7,	 /**<Set trace level to debug2 */
    IX_OSAL_LOG_LVL_DEBUG3 = 8,	 /**<Set trace level to debug3 */
    IX_OSAL_LOG_LVL_ALL	/**<Set trace level to all */
} IxOsalLogLevel;


/**
 * @ingroup IxOsalTypes
 * @brief Void function pointer prototype
 *
 * @note accepts a void pointer parameter
 * and does not return a value.
 */
typedef void (*IxOsalVoidFnVoidPtr) (void *);

/**
 * @ingroup IxOsalTypes
 * @brief Void function pointer prototype
 *
 * @note accepts a void parameter
 * and does not return a value.
 */
typedef void (*IxOsalVoidFnPtr) (void);


/**
 * @brief Timeval structure
 *
 * @note Contain subfields of seconds and nanoseconds..
 */
typedef struct
{
    UINT32 secs;		/**< seconds */
    UINT32 nsecs;		/**< nanoseconds */
} IxOsalTimeval;


/**
 * @ingroup IxOsalTypes
 * @brief IxOsalTimer 
 *
 * @note OSAL timer handle
 * 
 */
#ifndef USE_NATIVE_OS_TIMER_API
typedef UINT32 IxOsalTimer;
#else
typedef IxOsalOsTimer IxOsalTimer;
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_WAIT_FOREVER
 *
 * @brief Definition for timeout forever, OS-specific.
 *
 */
#define IX_OSAL_WAIT_FOREVER IX_OSAL_OS_WAIT_FOREVER

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_WAIT_NONE
 *
 * @brief Definition for timeout 0, OS-specific.
 *
 */
#define IX_OSAL_WAIT_NONE    IX_OSAL_OS_WAIT_NONE


/**
 * @ingroup IxOsalTypes
 * @brief IxOsalMutex 
 *
 * @note Mutex handle, OS-specific
 * 
 */
typedef IxOsalOsMutex IxOsalMutex;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalFastMutex 
 *
 * @note FastMutex handle, OS-specific
 * 
 */
typedef IxOsalOsFastMutex IxOsalFastMutex;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalThread
 *
 * @note Thread handle, OS-specific
 * 
 */
typedef IxOsalOsThread IxOsalThread;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalSemaphore
 *
 * @note Semaphore handle, OS-specific
 * 
 */
typedef IxOsalOsSemaphore IxOsalSemaphore;

/**
 * @ingroup IxOsalTypes
 * @brief IxOsalMessageQueue
 *
 * @note Message Queue handle, OS-specific
 * 
 */
typedef IxOsalOsMessageQueue IxOsalMessageQueue;


#ifdef ENABLE_SPINLOCK
/**
 * @ingroup IxOsalTypes
 * @brief IxOsalOsSpinLock
 *
 * @note SpinLock handle, OS-specific
 */
typedef IxOsalOsSpinLock IxOsalSpinLock;

#endif /* ENABLE_SPINLOCK */


/**
 * @brief Thread Attribute
 * @note Default thread attribute
 */
typedef struct
{
    char *name;        /**< name */
    UINT32 stackSize;  /**< stack size */
    UINT32 priority;   /**< priority */
    INT32 policy;      /**< policy */
} IxOsalThreadAttr;


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_THREAD_DEFAULT_SCHED_POLICY
 *
 * @brief Default Thread Scheduling Policy, OS-specific.
 *
 */
#define IX_OSAL_THREAD_DEFAULT_SCHED_POLICY (IX_OSAL_OS_THREAD_DEFAULT_SCHED_POLICY)


/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_THREAD_DEFAULT_STACK_SIZE
 *
 * @brief Default thread stack size, OS-specific.
 *
 */
#define IX_OSAL_THREAD_DEFAULT_STACK_SIZE (IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_THREAD_MAX_STACK_SIZE
 *
 * @brief Max stack size, OS-specific.
 *
 */
#define IX_OSAL_THREAD_MAX_STACK_SIZE (IX_OSAL_OS_THREAD_MAX_STACK_SIZE)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_MAX_THREAD_NAME_LEN
 *
 * @brief Max size of thread name
 *
 */
#define IX_OSAL_MAX_THREAD_NAME_LEN  16

/**
 * @ingroup IxOsalTypes
 *
 * @def OSAL_MAX_MODULE_NAME_LENGTH
 *
 * @brief Max size of module Name Length to be prefixed for OSAL Log 
 *
 */
#define OSAL_MAX_MODULE_NAME_LENGTH  20

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_MIN_THREAD_PRIORITY
 *
 * @brief Min thread priority, OS-specific.
 *
 */
#ifdef IX_OSAL_OS_MIN_THREAD_PRIORITY 
#define IX_OSAL_MIN_THREAD_PRIORITY 	(IX_OSAL_OS_MIN_THREAD_PRIORITY)
#else
#define IX_OSAL_MIN_THREAD_PRIORITY 	0
#endif

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_DEFAULT_THREAD_PRIORITY
 *
 * @brief Default thread priority, OS-specific.
 *
 */
#define IX_OSAL_DEFAULT_THREAD_PRIORITY (IX_OSAL_OS_DEFAULT_THREAD_PRIORITY)

/**
 * @ingroup IxOsalTypes
 *
 * @def IX_OSAL_MAX_THREAD_PRIORITY
 *
 * @brief Max thread priority, OS-specific.
 *
 */
#define IX_OSAL_MAX_THREAD_PRIORITY (IX_OSAL_OS_MAX_THREAD_PRIORITY)

/**
 * @ingroup IxOsalTypes
 *
 * @typedef IxOsalPciDev
 * @brief IxOsalPciDev
 *
 * @note This is a data type that serves as a handle for allocated PCI device.
 * 
 */
typedef UINT8 *IxOsalPciDev;

/**
 * @ingroup IxOsalTypes
 *
 * @typedef IxOsalAtomic
 * @brief IxOsalAtomic
 *
 * @note IxOsalAtomic Variable, OS-specific
 * 
 */
 typedef IxOsalOsAtomic IxOsalAtomic;

/**
 * @brief UINT128 
 *
 * @note Union to hold UINT128 value
 */
typedef union UINT128_t
{
    UINT8       mUINT8[16];  /**< 16 UINT8 values */
    UINT16      mUINT16[8];  /**< 8 UINT16 values */
    UINT32      mUINT32[4];  /**< 4 UINT32 values */
    UINT64      mUINT64[2];  /**< 2 UINT64 values */
} UINT128;

#ifdef __cplusplus
}
#endif
#endif /* IxOsalTypes_H */
