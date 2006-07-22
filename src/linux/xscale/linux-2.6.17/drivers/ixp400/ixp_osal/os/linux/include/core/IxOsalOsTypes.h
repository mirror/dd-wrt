/**
 * @file IxOsalOsTypes.h
 *
 * @brief Linux-specific data type
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

#ifndef IxOsalOsTypes_H
#define IxOsalOsTypes_H

#include <linux/types.h>
#include <asm/semaphore.h>

typedef long long INT64;	      /**< 64-bit signed integer */
typedef unsigned long long UINT64;    /**< 64-bit unsigned integer */
typedef int INT32;		      /**< 32-bit signed integer */
typedef unsigned int UINT32;	      /**< 32-bit unsigned integer */
typedef short INT16;		      /**< 16-bit signed integer */
typedef unsigned short UINT16;	      /**< 16-bit unsigned integer */
typedef char INT8;		      /**< 8-bit signed integer */
typedef unsigned char UINT8;	      /**< 8-bit unsigned integer */
typedef UINT32 ULONG;		      /**< alias for UINT32 */
typedef UINT16 USHORT;		      /**< alias for UINT16 */
typedef UINT8 UCHAR;		      /**< alias for UINT8 */
typedef UINT32 BOOL;		      /**< alias for UINT32 */

/* Default stack limit is 10 KB */
#define IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE  (10240) 

/* Maximum stack limit is 32 MB */
#define IX_OSAL_OS_THREAD_MAX_STACK_SIZE      (33554432)  /* 32 MBytes */

/* Default thread priority */
#define IX_OSAL_OS_DEFAULT_THREAD_PRIORITY    (90)

/* Thread maximum priority (0 - 255). 0 - highest priority */
#define IX_OSAL_OS_MAX_THREAD_PRIORITY	      (255)

#define IX_OSAL_OS_WAIT_FOREVER (-1L)  

#define IX_OSAL_OS_WAIT_NONE	0


/* Thread handle is eventually an int type */
typedef int IxOsalOsThread;

/* Semaphore handle */   
typedef struct semaphore *IxOsalOsSemaphore;

/* Mutex handle */
typedef struct semaphore *IxOsalOsMutex;

/* 
 * Fast mutex handle - fast mutex operations are implemented in
 * native assembler code using atomic test-and-set instructions 
 */
typedef int IxOsalOsFastMutex;

typedef struct
{
    UINT32 msgLen;	   /* Message Length */
	UINT32 maxNumMsg;  /* max number of msg in the queue */
	UINT32 currNumMsg; /* current number of msg in the queue */
	INT8   msgKey;     /* key used to generate the queue */
    INT8   queueId;	   /* queue ID */

} IxOsalOsMessageQueue;

#endif /* IxOsalOsTypes_H */
