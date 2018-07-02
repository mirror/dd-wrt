/**
 * @file IxOsalOsTypes.h
 *
 * @brief Linux-specific data type
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

#include <linux/types.h>
#include <linux/semaphore.h>

#include <linux/version.h>

#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE

#include <linux/sched.h>
#include <linux/sched/rt.h>
#include <linux/kthread.h>

#endif /* KERNEL_VERSION_2.6 */

#ifdef ENABLE_SPINLOCK

#include <linux/spinlock.h>

#endif

#include "IxOsal.h"

#include "IxOsalUtilitySymbols.h"

typedef s64 	INT64;	/**< 64-bit signed integer */
typedef u64	UINT64;	/**< 64-bit unsigned integer */
typedef s32 	INT32;	/**< 32-bit signed integer */
typedef u32 	UINT32;	/**< 32-bit unsigned integer */
typedef s16 	INT16;	/**< 16-bit signed integer */
typedef u16 	UINT16;	/**< 16-bit unsigned integer */
typedef s8 	INT8;	/**< 8-bit signed integer */
typedef u8 	UINT8;	/**< 8-bit unsigned integer */
typedef UINT32 	ULONG;	/**< alias for UINT32 */
typedef UINT16 	USHORT;	/**< alias for UINT16 */
typedef UINT8 	UCHAR;	/**< alias for UINT8 */
typedef UINT32 	BOOL;	/**< alias for UINT32 */
typedef void    VOID;

/*
 * Detecting the kernel version that we compiled against. 
 * We did not lock down specifically to any revision here.
 * We do it for some specific revisions now. 
 */
#if (KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE) && \
    (KERNEL_VERSION(4,7,0) > LINUX_VERSION_CODE)

#define IX_OSAL_OS_LINUX_VERSION_2_6   1  /* Kernel 2.6 */
#undef IX_OSAL_OS_LINUX_VERSION_2_4

#if (KERNEL_VERSION(2,6,16) == LINUX_VERSION_CODE)

#define IX_OSAL_OS_LINUX_VERSION_2_6_16     1

#endif
    
#elif (KERNEL_VERSION(2,4,0) <= LINUX_VERSION_CODE) && \
              (KERNEL_VERSION(2,5,0) > LINUX_VERSION_CODE)

#define IX_OSAL_OS_LINUX_VERSION_2_4   1  /* Kernel 2.4 */
#undef IX_OSAL_OS_LINUX_VERSION_2_6

#else /* KERNEL_VERSION */

#error "Non supported Linux kernel version"

#endif /* KERNEL_VERSION */


/* Default stack limit is 10 KB */
#define IX_OSAL_OS_THREAD_DEFAULT_STACK_SIZE  (10240) 

/* Maximum stack limit is 32 MB */
#define IX_OSAL_OS_THREAD_MAX_STACK_SIZE      (33554432)  /* 32 MBytes */

/* Thread minimum priority */
#define IX_OSAL_OS_MIN_THREAD_PRIORITY        (0)

/* Default thread priority */
#define IX_OSAL_OS_DEFAULT_THREAD_PRIORITY    (MAX_RT_PRIO)

/* Thread maximum priority (0 - 255). 0 - highest priority */
#define IX_OSAL_OS_MAX_THREAD_PRIORITY	      (MAX_PRIO)

/* Default scheduling policy */
#define IX_OSAL_OS_THREAD_DEFAULT_SCHED_POLICY SCHED_RR


#define IX_OSAL_OS_WAIT_FOREVER               (-1L)  
#define IX_OSAL_OS_WAIT_NONE                  0
#undef IX_OSAL_ATTRIBUTE_PACKED
#define IX_OSAL_ATTRIBUTE_PACKED              __attribute__((__packed__))


#ifdef IX_OSAL_OS_LINUX_VERSION_2_6

/* Thread handle is a task_struct pointer */
typedef struct task_struct *IxOsalOsThread;

#else /* !KERNEL_VERSION 2.6 */

/* Thread handle is an int type */
typedef int IxOsalOsThread;

#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */


/* Semaphore handle */   
typedef struct semaphore *IxOsalOsSemaphore;

/* Mutex handle */
typedef struct semaphore *IxOsalOsMutex;

#ifdef IX_OSAL_OEM_FAST_MUTEX

typedef int IxOsalOsFastMutex;

#else /* ! IX_OSAL_OEM_FAST_MUTEX -> Generic */

/* 
 * Fast mutex handle - fast mutex operations are implemented
 * using the linux atomic instructions.
 */
typedef atomic_t IxOsalOsFastMutex;

#endif /* IX_OSAL_OEM_FAST_MUTEX */
#ifdef ENABLE_SPINLOCK

typedef spinlock_t IxOsalOsSpinLock;

#endif

typedef struct
{
    UINT32 msgLen;	   /* Message Length */
    UINT32 maxNumMsg;  /* max number of msg in the queue */
    UINT32 currNumMsg; /* current number of msg in the queue */
    INT8   msgKey;     /* key used to generate the queue */
    INT8   queueId;	   /* queue ID */

} IxOsalOsMessageQueue;

#endif /* IxOsalOsTypes_H */
