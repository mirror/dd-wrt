/**
 * @file IxEthAccDBLocks_p.h
 *
 * @brief Definition of transaction lock stacks and lock utility macros
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
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

#ifndef IxEthAccDBLocks_p_H
#define IxEthAccDBLocks_p_H

#include "IxOsPrintf.h"

/* Lock and lock stacks */
typedef struct
{
    IxOsalFastMutex* locks[MAX_LOCKS];
    UINT32 stackPointer, basePointer;
} LockStack;

#define TRY_LOCK(mutex) \
    { \
        if (ixOsalFastMutexTryLock(mutex) != IX_SUCCESS) \
        { \
            return IX_ETH_DB_BUSY; \
        } \
    }


#define UNLOCK(mutex) { ixOsalFastMutexUnlock(mutex); }

#define INIT_STACK(stack) \
    { \
        (stack)->basePointer  = 0; \
        (stack)->stackPointer = 0; \
    }

#define PUSH_LOCK(stack, lock) \
    { \
        if ((stack)->stackPointer == MAX_LOCKS) \
        { \
            ERROR_LOG("Ethernet DB: maximum number of elements in a lock stack has been exceeded on push, heavy chaining?\n"); \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_NOMEM; \
        } \
        \
        if (ixOsalFastMutexTryLock(lock) == IX_SUCCESS) \
        { \
            (stack)->locks[(stack)->stackPointer++] = (lock); \
        } \
        else \
        { \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
    }

#define POP_LOCK(stack) \
    { \
        ixOsalFastMutexUnlock((stack)->locks[--(stack)->stackPointer]); \
    }

#define UNROLL_STACK(stack) \
    { \
        while ((stack)->stackPointer > (stack)->basePointer) \
        { \
            POP_LOCK(stack); \
        } \
    }

#define SHIFT_STACK(stack) \
    { \
        if ((stack)->basePointer == MAX_LOCKS - 1) \
        { \
            ERROR_LOG("Ethernet DB: maximum number of elements in a lock stack has been exceeded on shift, heavy chaining?\n"); \
            UNROLL_STACK(stack); \
            \
            return IX_ETH_DB_BUSY; \
        } \
        \
        ixOsalFastMutexUnlock((stack)->locks[(stack)->basePointer++]); \
    }

#endif /* IxEthAccDBLocks_p_H */
