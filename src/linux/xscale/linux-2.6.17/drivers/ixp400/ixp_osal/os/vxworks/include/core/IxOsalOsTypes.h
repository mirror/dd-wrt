/**
 * @file IxOsalOsTypes.h
 *
 * @brief OS-specific types.
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

/* Include OS-specifc defines */
#include "time.h"
#include <vxWorks.h>
#include <semLib.h>
#include "semaphore.h"

/******************/

#include "time.h"
#include "vxWorks.h"
#include "semaphore.h"
#include "msgQLib.h"

/* Default stack limit is 10 KB */
#define IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE  (10240) 

/* Maximum stack limit is 32 MB */
#define IX_OSAL_OS_THREAD_MAX_STACK_SIZE      (33554432)  /* 32 MBytes */

/* Default thread priority */
#define IX_OSAL_OS_DEFAULT_THREAD_PRIORITY    (90)

/* Thread maximum priority (0 - 255). 0 - highest priority */
#define IX_OSAL_OS_MAX_THREAD_PRIORITY	      (255)

#define IX_OSAL_OS_WAIT_FOREVER WAIT_FOREVER

#define IX_OSAL_OS_WAIT_NONE	NO_WAIT


typedef unsigned int IxOsalOsThread;


typedef SEM_ID IxOsalOsSemaphore;


typedef SEM_ID IxOsalOsMutex;


typedef unsigned int IxOsalOsFastMutex;


typedef struct
{
    MSG_Q_ID queueHandle;
    UINT32 msgLen;
} IxOsalOsMessageQueue;


#endif /* IxOsalOsTypes_H */
