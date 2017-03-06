/**
 * @file IxOsalOsTypes.h
 *
 * @brief OS-specific types.
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

#define IX_OSAL_OS_ATTRIBUTE_PACKED


typedef unsigned int IxOsalOsThread;


typedef SEM_ID IxOsalOsSemaphore;


typedef SEM_ID IxOsalOsMutex;

#if CPU==SIMSPARCSOLARIS
typedef SEM_ID IxOsalOsFastMutex;
#else
typedef unsigned int IxOsalOsFastMutex;
#endif /*  CPU==SIMSPARCSOLARIS */

typedef struct
{
    MSG_Q_ID queueHandle;
    UINT32 msgLen;
} IxOsalOsMessageQueue;


#endif /* IxOsalOsTypes_H */
