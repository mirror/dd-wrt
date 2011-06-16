/**
 * @file IxOsalOsTypes.h
 *
 * @brief Linux-specific data type
 * 
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

#ifndef IxOsalOsTypes_H
#define IxOsalOsTypes_H

#include <linux/types.h>
#include <linux/version.h>
 
#if LINUX_VERSION_CODE  >= KERNEL_VERSION(2,6,18)
//#include <linux/autoconf.h>
#elif KERNEL_VERSION(2,6,16) >= LINUX_VERSION_CODE
#include <linux/config.h>
#endif
 
#if KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE

#include <linux/sched.h>
#include <linux/kthread.h>

#endif /* KERNEL_VERSION_2.6 */

#ifdef ENABLE_SPINLOCK

#include <linux/spinlock.h>

#if LINUX_VERSION_CODE  < KERNEL_VERSION(2,6,18)
#include <linux/interrupt.h>
#endif  /*< KERNEL_VERSION(2,6,18*/

#endif  /* ENABLE_SPINLOCK */

#include <asm/atomic.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

#ifdef USE_NATIVE_OS_TIMER_API
#include <linux/timer.h>
#endif

#include <linux/wait.h>

#include "IxOsalUtilitySymbols.h"

#ifndef __ACTYPES_H__
typedef u8 	UINT8;	/**< 8-bit unsigned integer */
typedef u16 	UINT16;	/**< 16-bit unsigned integer */
typedef u32 	UINT32;	/**< 32-bit unsigned integer */
typedef u64	UINT64;	/**< 64-bit unsigned integer */
typedef s64 	INT64;	/**< 64-bit signed integer */
typedef s16 	INT16;	/**< 16-bit signed integer */
typedef s32 	INT32;	/**< 32-bit signed integer */
#endif /* __ACTYPES_H__ */

typedef s8 	INT8;	/**< 8-bit signed integer */
typedef UINT32 	ULONG;	/**< alias for UINT32 */
typedef UINT16 	USHORT;	/**< alias for UINT16 */
typedef UINT8 	UCHAR;	/**< alias for UINT8 */
typedef UINT32 	BOOL;	/**< alias for UINT32 */
typedef INT8    CHAR;   /**< alias for INT8*/
typedef void    VOID;


/*
 * Detecting the kernel version that we compiled against. 
 * We did not lock down specifically to any revision here.
 * We do it for some specific revisions now. 
 */
#if (KERNEL_VERSION(2,6,0) <= LINUX_VERSION_CODE) && \
    (KERNEL_VERSION(2,7,0) > LINUX_VERSION_CODE)

#define IX_OSAL_OS_LINUX_VERSION_2_6   1  /* Kernel 2.6 */
#undef IX_OSAL_OS_LINUX_VERSION_2_4

#if (KERNEL_VERSION(2,6,16) == LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VERSION_2_6_16     1
#endif
    
#if (KERNEL_VERSION(2,6,18) == LINUX_VERSION_CODE) 
#define IX_OSAL_OS_LINUX_VERSION_2_6_18     1
#endif 

#if (KERNEL_VERSION(2,6,20) == LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VERSION_2_6_20     1 
#endif 

#if (KERNEL_VERSION(2,6,27) == LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VERSION_2_6_27     1 
#endif 

#if (KERNEL_VERSION(2,6,28) == LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VERSION_2_6_28     1 
#endif 
 
/* Defines for version greater than a specific minor ver number */
#if (KERNEL_VERSION(2,6,18) <= LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VER_GT_2_6_18      1
#endif    
    
#if (KERNEL_VERSION(2,6,20) <= LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VER_GT_2_6_20      1
#endif    

#if (KERNEL_VERSION(2,6,27) <= LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VER_GT_2_6_27      1
#endif    

#if (KERNEL_VERSION(2,6,28) <= LINUX_VERSION_CODE)
#define IX_OSAL_OS_LINUX_VER_GT_2_6_28      1
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
#define IX_OSAL_OS_DEFAULT_THREAD_PRIORITY    (MAX_RT_PRIO-1)

/* Thread maximum priority (0 - 255). 0 - highest priority */
#define IX_OSAL_OS_MAX_THREAD_PRIORITY	      (MAX_PRIO)

/* Maximum input value for priority */
#define IX_OSAL_PRIO_SET_MAX_VALID_VAL        (255)

/* Maximum supported priority value in ThreadPrioritySet */
#define IX_OSAL_PRIO_SET_MAX_VAL              (39)

/* Difference of actual nice value and input value */
#define IX_OSAL_NICE_VAL_DIFFERENCE           (20)

/* Default scheduling policy */
#define IX_OSAL_OS_THREAD_DEFAULT_SCHED_POLICY SCHED_RR

/* Thread scheduling policy - Round Robin */
#define IX_OSAL_THREAD_SCHED_RR                 SCHED_RR

/* Thread scheduling policy - FiFo */
#define IX_OSAL_THREAD_SCHED_FIFO               SCHED_FIFO

 /* Thread scheduling policy - Other */
#define IX_OSAL_THREAD_SCHED_OTHER              SCHED_OTHER

#define IX_OSAL_OS_WAIT_FOREVER               (-1)  
#define IX_OSAL_OS_WAIT_NONE                  0


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
#endif /* ENABLE_SPINLOCK */


typedef struct
{
    UINT32 msgLen;	   /* Message Length */
    UINT32 maxNumMsg;  /* max number of msg in the queue */
    UINT32 currNumMsg; /* current number of msg in the queue */
    INT8   msgKey;     /* key used to generate the queue */
    INT8   queueId;	   /* queue ID */

} IxOsalOsMessageQueue;

typedef atomic_t IxOsalOsAtomic;

/* Dummy typedef for OsThreadAttr - This is not used in linux currently. 
   This needs to be defined appropriately when it is planned to be used */
typedef int IxOsalOsThreadAttr;

typedef void (*voidFnVoidPtr) (void *);
typedef void (*voidFnVoid) (void);

#ifdef USE_NATIVE_OS_TIMER_API
typedef void (*voidFnULongPtr)(unsigned long);

typedef struct
{
    BOOL  inUse;             /* status of timer active or cancel */
    BOOL  isRepeating;       /* Timer is repeating type */
    voidFnVoidPtr  callback;  /* Function to be called back after period ms */
    UINT32 priority;          /* priority */
    void  *callbackParam;     /* parameter to be passed to callback function*/
    UINT32 period;            /* period in mili seconds */
    struct timer_list timer;  /* Linux OS timer struct */
} IxOsalTimerRec;

typedef  IxOsalTimerRec *IxOsalOsTimer;
#endif /* USE_NATIVE_OS_TIMER_API */

/*
 * On Linux kmalloc can allocat a max of 128 KB
 */
#define IX_OSAL_MAX_KMALLOC_MEM      (1024 * 128)  

/*
 *  linux data struct to store the information on the 
 *  memory allocated. This structure is stored at the beginning of 
 *  the allocated chunck of memory
 *  size is the no of byte passed to the memory allocation functions
 *  mSize is the real size of the memory required to the OS
 * 
 *  +--------------------------+--------------------------------+
 *  | ixOsalMemAllocInfoStruct | memory returned to user (size) |
 *  +--------------------------+--------------------------------+
 *  ^                          ^
 *  mAllocMemPtr               Ptr returned to the caller of MemAlloc*
 * 
 */
typedef struct _sMemAllocInfo
{
    VOID*               mAllocMemPtr;   /* memory addr returned by the kernel */
    UINT32              mSize;          /* allocated size */
    
} ixOsalMemAllocInfoStruct;

#endif /* IxOsalOsTypes_H */

