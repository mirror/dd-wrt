/**
 * @file IxOsal.h
 *
 * @brief Top include file for OSAL
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

#ifndef IxOsal_H
#define IxOsal_H
#ifdef __cplusplus
extern "C"{
#endif

/* Basic types */
#include "IxOsalTypes.h"

/* Include assert */
#include "IxOsalAssert.h"

/*
 * Config header gives users option to choose IO MEM
 * and buffer management modules
 */

#include "IxOsalConfig.h"


/*
 * Symbol file needed by some OS.
 */
#include "IxOsalUtilitySymbols.h"

/* OS-specific header */
#include "IxOsalOs.h"


/**
 * @defgroup IxOsal Operating System Abstraction Layer (OSAL) API
 *
 * @brief This service provides a thin layer of OS dependency services.
 *
 * This file contains the API to the functions which are some what OS dependant and would
 * require porting to a particular OS.
 * A primary focus of the component development is to make them as OS independent as possible.
 * All other components should abstract their OS dependency to this module.
 * Services overview
 *  -# Data types, constants, defines
 *  -# Interrupts
 *      - bind interrupts to handlers
 *      - unbind interrupts from handlers
 *      - disables all interrupts
 *      - enables all interrupts
 *      - selectively disables interrupts
 *      - enables an interrupt level
 *      - disables an interrupt level
 *      - atomic memory allocation
 *  -# Memory
 *      - allocates aligned memory
 *      - frees aligned memory
 *      - allocates memory
 *      - frees memory
 *      - copies memory zones
 *      - fills a memory zone
 *      - allocates cache-safe memory
 *      - frees cache-safe memory
 *      - physical to virtual address translation
 *      - virtual to physical address translation
 *      - cache to memory flush
 *      - cache line invalidate
 *  -# Threads
 *      - creates a new thread
 *      - starts a newly created thread
 *      - kills an existing thread
 *      - exits a running thread
 *      - sets the priority of an existing thread
 *      - suspends thread execution
 *      - resumes thread execution
 *  -# IPC
 *      - creates a message queue
 *      - deletes a message queue
 *      - sends a message to a message queue
 *      - receives a message from a message queue
 *  -# Thread Synchronisation
 *      - initializes a mutex
 *      - locks a mutex
 *      - unlocks a mutex
 *      - non-blocking attempt to lock a mutex
 *      - destroys a mutex object
 *      - initializes a fast mutex
 *      - non-blocking attempt to lock a fast mutex
 *      - unlocks a fast mutex
 *      - destroys a fast mutex object
 *      - initializes a semaphore
 *      - posts to (increments) a semaphore
 *      - waits on (decrements) a semaphore
 *      - non-blocking wait on semaphore
 *      - gets semaphore value
 *      - destroys a semaphore object
 *      - yields execution of current thread
 *  -# Time functions
 *      - yielding sleep for a number of milliseconds
 *      - busy sleep for a number of microseconds
 *      - value of the timestamp counter
 *      - resolution of the timestamp counter
 *      - system clock rate, in ticks
 *      - current system time
 *      - converts ixOsalTimeVal into ticks
 *      - converts ticks into ixOsalTimeVal
 *      - converts ixOsalTimeVal to milliseconds
 *      - converts milliseconds to IxOsalTimeval
 *      - "equal" comparison for IxOsalTimeval
 *      - "less than" comparison for IxOsalTimeval
 *      - "greater than" comparison for IxOsalTimeval
 *      - "add" operator for IxOsalTimeval
 *      - "subtract" operator for IxOsalTimeval
 *  -# Logging
 *      - sets the current logging verbosity level
 *      - interrupt-safe logging function
 *  -# Timer services
 *      - schedules a repeating timer
 *      - schedules a single-shot timer
 *      - cancels a running timer
 *      - displays all the running timers
 *  -# PCI Support
 *      - Find PCI device.
 *      - Read 8 bits from the configuration space
 *      - Read 16 bits from the configuration space
 *      - Read 32 bits from the configuration space
 *      - Write 8 bits to the configuration space
 *      - Write 16 bits to the configuration space
 *      - Write 32 bits to the configuration space
 *      - Free PCI device
 *  -# SpinLock Support
 *      - Initializes the SpinLock object
 *      - Acquires a spin lock
 *      - Releases the spin lock
 *      - Tries to acquire the spin lock
 *      - Destroy the spin lock object
 *  -# Atomic Support
 *              The following API's used for IA atomic only not for 
 *                IA to AE Atomics. 
 *      - read the value of atomic variable
 *      - set the value of atomic variable
 *      - add the value to atomic variable
 *      - subtract the value from atomic variable
 *      - subtract the value from atomic variable and test result
 *      - increment the value of atomic variable
 *      - decrement the value of atomic variable
 *      - decrement the value of atomic variable and test result
 *      - increment the value of atomic variable and test result
 *  -# Memory Barrier Support
 *      - read memory barrier which orders only memory reads
 *      - write memory barrier which orders only memory writes
 *  -# Optional Modules
 *      - Device drivers kernel
 *      - Buffer management module
 *      - I/O memory and endianess support module
 */


/*
 * Prototypes
 */

#if !defined(__linux_user) && !defined(__freebsd_user)
/* ==========================  Interrupts  ================================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Binds an interrupt handler to an interrupt level
 *
 * @param irqLevel (in)   - interrupt level
 * @param irqHandler (in) - interrupt handler
 * @param parameter (in)  - custom parameter to be passed to the
 *                          interrupt handler
 *
 * Binds an interrupt handler to an interrupt level. The operation will
 * fail if the wrong level is selected, if the handler is NULL, or if the
 * interrupt is already bound. This functions binds the specified C
 * routine to an interrupt level. When called, the "parameter" value will
 * be passed to the routine.
 *
 * Reentrant: yes
 * IRQ safe:  yes 
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS ixOsalIrqBind (UINT32 irqLevel,
                IxOsalVoidFnVoidPtr irqHandler,
                void *parameter);

/**
 * @ingroup IxOsal
 *
 * @brief Unbinds an interrupt handler from an interrupt level
 *
 * @param irqLevel (in)   - interrupt level
 *
 * Unbinds the selected interrupt level from any previously registered
 * handler
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC IX_STATUS ixOsalIrqUnbind (UINT32 irqLevel);


/**
 * @ingroup IxOsal
 *
 * @brief Disables all interrupts
 *
 * @param - none
 *
 * Disables all the interrupts and prevents tasks scheduling
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return interrupt enable status prior to locking
 */
PUBLIC UINT32 ixOsalIrqLock (void);

/**
 * @ingroup IxOsal
 *
 * @brief Enables all interrupts
 *
 * @param irqEnable (in) - interrupt enable status, prior to interrupt
 *                         locking
 *
 * Enables the interrupts and task scheduling, cancelling the effect
 * of ixOsalIrqLock()
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return None
 */
PUBLIC void ixOsalIrqUnlock (UINT32 irqEnable);

/**
 * @ingroup IxOsal
 *
 * @brief Selectively disables interrupts
 *
 * @param irqLevel � new interrupt level
 *
 * Disables the interrupts below the specified interrupt level
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @note Depending on the implementation this function can disable all
 *       the interrupts
 *
 * @return previous interrupt level
 */
PUBLIC UINT32 ixOsalIrqLevelSet (UINT32 irqLevel);

/**
 * @ingroup IxOsal
 *
 * @brief Enables an interrupt level
 *
 * @param irqLevel � interrupt level to enable
 *
 * Enables the specified interrupt level
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 */
PUBLIC void ixOsalIrqEnable (UINT32 irqLevel);

/**
 * @ingroup IxOsal
 *
 * @brief Disables an interrupt level
 *
 * @param irqLevel � interrupt level to disable
 *
 * Disables the specified interrupt level
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 */
PUBLIC void ixOsalIrqDisable (UINT32 irqLevel);

#endif /* __linux_user || __freebsd_user */
/* =============================  Memory  =================================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Allocates aligned memory
 *
 * @param space - (Unused right now ) kernel_space or user_space
 * @param size - malloc memory size required to be allocated
 * @param alignment - alignment required in bytes 
 *
 * Allocate an aligned memory zone
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - void pointer to malloced memory 
 */
PUBLIC VOID * ixOsalMemAllocAligned(UINT32 space, UINT32 size, UINT32 alignment);

/**
 * @ingroup IxOsal
 *
 * @brief Frees memory allocated by ixOsalMemAllocAligned
 *
 * @param ptr - pointer to contiguous aligned memory zone 
 * @param size - memory size allocated which needs to be freed.
 *
 * Frees a previously allocated Aligned memory zone
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none 
 */
PUBLIC void ixOsalMemAlignedFree(void *ptr, UINT32 size);

/**
 * @ingroup IxOsal
 *
 * @brief Allocates memory
 *
 * @param size - memory size to allocate, in bytes
 *
 * Allocates a memory zone of a given size
 * The returned memory is garaunteed to be physically contiguos if the 
 * given size is less than 128Kb.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return Pointer to the allocated zone or NULL if the allocation failed
 */
PUBLIC void *ixOsalMemAlloc (UINT32 size);

/**
 * @ingroup IxOsal
 *
 * @brief Frees memory allocated by ixOsalMemAlloc
 *
 * @param ptr - pointer to the memory zone
 *
 * Frees a previously allocated memory zone
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC VOID ixOsalMemFree (VOID *ptr);

/**
 * @ingroup IxOsal
 *
 * @brief Copies data bytes from src memory zone to dest memory zone
 *
 * @param dest  - destination memory zone
 * @param src   - source memory zone
 * @param count - number of bytes to copy
 *
 * Copies count bytes from the source memory zone pointed by src into the
 * memory zone pointed by dest.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return Pointer to the destination memory zone
 */
PUBLIC void *ixOsalMemCopy (void *dest, const void *src, UINT32 count);

/**
 * @ingroup IxOsal
 *
 * @brief Fills a memory zone
 *
 * @param ptr - pointer to the memory zone
 * @param filler - byte to fill the memory zone with
 * @param count - number of bytes to fill
 *
 * Fills a memory zone with a given constant byte
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return Pointer to the memory zone
 */
PUBLIC void *ixOsalMemSet (void *ptr, UINT8 filler, UINT32 count);

/**
 * @ingroup IxOsal
 *
 * @brief Compares memory zones
 *
 * @param dest  - destination memory zone
 * @param src   - source memory zone
 * @param count - number of bytes to compare 
 *
 * Compares count bytes from the source memory zone pointed by src with the
 * memory zone pointed by dest.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return IX_SUCCESS/IX_FAIL 
 */
PUBLIC IX_STATUS ixOsalMemCmp (void *dest, void *src, UINT32 count);

#if !defined(__linux_user) && !defined(__freebsd_user) 

/**
 * @ingroup IxOsal
 *
 * @brief Allocates memory in IRQ safe context
 *
 * @param size - memory size to allocate, in bytes
 *
 * Allocates a memory zone of a given size without sleeping. 
 * If the required resourse is not immediatly available NULL pointer is returned
 * This is suitable for IRQ context or mutex/spinlock context when sleeping
 * is not allowed
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return Pointer to the allocated zone or NULL if the allocation failed
 */
PUBLIC void *ixOsalMemAllocAtomic (UINT32 size);

#if (!defined(__freebsd))
/**
 * @ingroup IxOsal
 *
 * @brief physical to virtual address translation
 *
 * @param physAddr - physical address
 *
 * Converts a physical address into its equivalent MMU-mapped virtual address
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return Corresponding virtual address, as UINT32
 */
#define IX_OSAL_MMU_PHYS_TO_VIRT(physAddr) \
    IX_OSAL_OS_MMU_PHYS_TO_VIRT(physAddr)

#endif /*!defined(__freebsd)*/
/**
 * @ingroup IxOsal
 *
 * @brief virtual to physical address translation
 *
 * @param virtAddr - virtual address
 *
 * Converts a virtual address into its equivalent MMU-mapped physical address
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return Corresponding physical address, as UINT32
 */
#define IX_OSAL_MMU_VIRT_TO_PHYS(virtAddr)  \
    IX_OSAL_OS_MMU_VIRT_TO_PHYS(virtAddr)

/**
 * @ingroup IxOsal
 *
 * @brief Allocates cache-safe memory
 *
 * @param size - size, in bytes, of the allocated zone
 *
 * Allocates a cache-safe memory zone of at least "size" bytes and returns
 * the pointer to the memory zone. This memory zone, depending on the
 * platform, is either uncached or aligned on a cache line boundary to make
 * the CACHE_FLUSH and CACHE_INVALIDATE macros safe to use. The memory
 * allocated with this function MUST be freed with ixOsalCacheDmaFree(),
 * otherwise memory corruption can occur. This function allocates 32byte aligned
 * physical contigeous memory. The minimum size memory it can allocate is 
 * 32byte and maximum size depends on the availability in OS.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return Pointer to the memory zone or NULL if allocation failed
 *
 * @note It is important to note that cache coherence is maintained in
 * software by using the IX_OSAL_CACHE_FLUSH and IX_OSAL_CACHE_INVALIDATE
 * macros to maintain consistency between cache and external memory.
 */
PUBLIC void *ixOsalCacheDmaMalloc (UINT32 size);

/**
 * @ingroup IxOsal
 *
 * @brief Allocates cache-safe memory
 *
 * @param size - size, in bytes, of the allocated zone
 *
 * Allocates a cache-safe memory zone of at least "size" bytes and returns
 * the pointer to the memory zone. This memory zone, depending on the
 * platform, is either uncached or aligned on a cache line boundary to make
 * the CACHE_FLUSH and CACHE_INVALIDATE macros safe to use. The memory
 * allocated with this function MUST be freed with ixOsalCacheDmaFree(),
 * otherwise memory corruption can occur. This function allocates 32byte aligned
 * physical contigeous memory. The minimum size memory it can allocate is 
 * 32byte and maximum size depends on the availability in OS.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return Pointer to the memory zone or NULL if allocation failed
 *
 * @note It is important to note that cache coherence is maintained in
 * software by using the IX_OSAL_CACHE_FLUSH and IX_OSAL_CACHE_INVALIDATE
 * macros to maintain consistency between cache and external memory.
 */
#define IX_OSAL_CACHE_DMA_MALLOC(size) ixOsalCacheDmaMalloc(size)

/**
 * @ingroup IxOsal
 *
 * @brief Frees cache-safe memory
 *
 * @param ptr   - pointer to the memory zone
 *
 * Frees a memory zone previously allocated with ixOsalCacheDmaMalloc()
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalCacheDmaFree (void *ptr);

/**
 * @ingroup IxOsal
 *
 * @brief Frees cache-safe memory
 *
 * @param ptr   - pointer to the memory zone
 *
 * Frees a memory zone previously allocated with ixOsalCacheDmaMalloc()
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
#define IX_OSAL_CACHE_DMA_FREE(ptr)     ixOsalCacheDmaFree(ptr)



/**
 * @ingroup IxOsal
 *
 * @brief cache to memory flush
 *
 * @param addr - memory address to flush from cache
 * @param size - number of bytes to flush (rounded up to a cache line)
 *
 * Flushes the cached value of the memory zone pointed by "addr" into memory,
 * rounding up to a cache line. Use before the zone is to be read by a
 * processing unit which is not cache coherent with the main CPU.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 */
#define IX_OSAL_CACHE_FLUSH(addr, size)  IX_OSAL_OS_CACHE_FLUSH(addr, size)



/**
 * @ingroup IxOsal
 *
 * @brief cache line invalidate
 *
 * @param addr - memory address to invalidate in cache
 * @param size - number of bytes to invalidate (rounded up to a cache line)
 *
 * Invalidates the cached value of the memory zone pointed by "addr",
 * rounding up to a cache line. Use before reading the zone from the main
 * CPU, if the zone has been updated by a processing unit which is not cache
 * coherent with the main CPU.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 */
#define IX_OSAL_CACHE_INVALIDATE(addr, size)  IX_OSAL_OS_CACHE_INVALIDATE(addr, size)

/**
 * @ingroup IxOsal
 *
 * @brief cache line preload
 *
 * @param addr - memory address to cache
 * @param size - number of bytes to cache (rounded up to a cache line)
 *
 *
 * Preloads a section of memory to the cache memory in multiples of cache line size.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 */
#define IX_OSAL_CACHE_PRELOAD(addr, size)  IX_OSAL_OS_CACHE_PRELOAD(addr, size)

#endif /* __linux_user || __freebsd_user */


/* =============================  Threads  =================================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Creates a new thread
 *
 * @param thread - handle of the thread to be created
 * @param threadAttr - pointer to a thread attribute object
 * @param startRoutine - thread entry point
 * @param arg - argument to be passed to the startRoutine
 *
 * Creates a thread given a thread handle and a thread attribute object. The
 * same thread attribute object can be used to create separate threads. "NULL"
 * can be specified as the attribute, in which case the default values will
 * be used. The thread needs to be explicitly started using ixOsalThreadStart().
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadCreate (IxOsalThread * thread,
                     IxOsalThreadAttr * threadAttr,
                     IxOsalVoidFnVoidPtr startRoutine,
                     void *arg);

/**
 * @ingroup IxOsal
 *
 * @brief Starts a newly created thread
 *
 * @param thread - handle of the thread to be started
 *
 * Starts a thread given its thread handle. This function is to be called
 * only once, following the thread initialization.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadStart (IxOsalThread * thread);

/**
 * @ingroup IxOsal
 *
 * @brief Terminates a thread execution
 *
 * @param thread - handle of the thread to be terminated
 *
 * Kills a thread given its thread handle.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @note This function does not guarentee to kill the thread immediately. The
 * thread must use ixOsalThreadStopCheck() to check if it should perform
 * cleanup and suicide.
 *
 * @return -  IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadKill (IxOsalThread * thread);

/**
 * @ingroup IxOsal
 *
 * @brief Exits a running thread
 *
 * Terminates the calling thread
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - This function never returns
 */
PUBLIC void ixOsalThreadExit (void);

/**
 * @ingroup IxOsal
 *
 * @brief Sets the priority of a thread
 *
 * @param thread - handle of the thread
 * @param priority - new priority, between 0 and 255 (0 being the highest)
 *
 * Sets the thread priority
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadPrioritySet (IxOsalThread * thread,
                      UINT32 priority);

/**
 * @ingroup IxOsal
 *
 * @brief Suspends thread execution
 *
 * @param thread - handle of the thread
 *
 * Suspends the thread execution. The suspended thread can be resumed
 * by ixOsalThreadResume call
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadSuspend (IxOsalThread * thread);

/**
 * @ingroup IxOsal
 *
 * @brief Resumes thread execution
 *
 * @param thread - handle of the thread
 *
 * Resumes the thread execution
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadResume (IxOsalThread * thread);

/**
 * @ingroup IxOsal
 *
 * @brief Check if thread should stop execution
 *
 * Check if ixOsalThreadKill has been called. When this API return TRUE, the
 * thread should perform cleanup and exit.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - TRUE/FALSE
 */

PUBLIC BOOL ixOsalThreadStopCheck(void);


/* =======================  Message Queues (IPC) ==========================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Creates a message queue
 *
 * @param queue - queue handle
 * @param msgCount - maximum number of messages to hold in the queue
 * @param msgLen - maximum length of each message, in bytes
 *
 * Creates a message queue of msgCount messages, each containing msgLen bytes
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMessageQueueCreate (IxOsalMessageQueue * queue,
                       UINT32 msgCount, UINT32 msgLen);

/**
 * @ingroup IxOsal
 *
 * @brief Creates a message queue 
 *
 * @param queue - queue handle
 * @param msgCount - maximum number of messages to hold in the queue
 * @param msgLen - maximum length of each message, in bytes
 *
 * Creates a blocking message queue of msgCount messages, each containing 
 * msgLen bytes
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSyncMessageQueueCreate (IxOsalMessageQueue * queue, 
                       UINT32 msgCount, UINT32 msgLen);

/**
 * @ingroup IxOsal
 *
 * @brief Deletes a message queue
 *
 * @param queue - message queue handle
 *
 * Deletes a message queue
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalMessageQueueDelete (IxOsalMessageQueue * queue);

/**
 * @ingroup IxOsal
 *
 * @brief Sends a message to a message queue
 *
 * @param queue - message queue handle
 * @param message - message to send
 *
 * Sends a message to the message queue. The message will be copied (at the
 * configured size of the message) into the queue.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMessageQueueSend (IxOsalMessageQueue * queue,
                     UINT8 * message);

/**
 * @ingroup IxOsal
 *
 * @brief Receives a message from a message queue
 *
 * @param queue - queue handle
 * @param message - pointer to where the message should be copied to
 *
 * Retrieves the first message from the message queue
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 *           IX_EAGAIN  - specified message queue is empty
 *           IX_EBADF   - invalid message queue descriptor
 *           IX_EMSGSIZE    - insufficient message buffer size
 *           IX_EINTR   - operation interrupted
 *           IX_EINVAL  - invalid 
 *           IX_ETIMEDOUT   - timeout before any message arrived
 *           IX_EBADMSG - corrupt message
 */
PUBLIC IX_STATUS ixOsalMessageQueueReceive (IxOsalMessageQueue * queue,
                        UINT8 * message);

/**
 * @ingroup IxOsal
 *
 * @brief Receives a message from a message queue 
 *
 * @param queue - queue handle
 * @param message - pointer to where the message should be copied to
 * @param timeout - timeout time in millisec
 *
 * Retrieves the first message from the message queue within specified time
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS - success
 *           IX_EAGAIN  - specified message queue is empty
 *           IX_ETIMEDOUT   - timeout before any message arrived
 *           
 */
PUBLIC IX_STATUS ixOsalSyncMessageQueueReceive(IxOsalMessageQueue * queue, 
                        UINT8 * message, INT32 timeout);

/* =======================  Thread Synchronisation ========================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief initializes a mutex
 *
 * @param mutex - mutex handle
 *
 * Initializes a mutex object
 * Note: Mutex initialization ixOsalMutexInit API must be called 
 * first before using any OSAL Mutex APIs
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMutexInit (IxOsalMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief locks a mutex
 *
 * @param mutex - mutex handle
 * @param timeout - timeout in ms; IX_OSAL_WAIT_FOREVER (-1) to wait forever
 *                  or IX_OSAL_WAIT_NONE to return immediately
 *
 * Locks a mutex object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMutexLock (IxOsalMutex * mutex, INT32 timeout);

/**
 * @ingroup IxOsal
 *
 * @brief Unlocks a mutex
 *
 * @param mutex - mutex handle
 *
 * Unlocks a mutex object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no 
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMutexUnlock (IxOsalMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Non-blocking attempt to lock a mutex
 *
 * @param mutex - mutex handle
 *
 * Attempts to lock a mutex object, returning immediately with IX_SUCCESS if
 * the lock was successful or IX_FAIL if the lock failed
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMutexTryLock (IxOsalMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Destroys a mutex object
 *
 * @param mutex - mutex handle
 *
 * Destroys a mutex object; the caller should ensure that no thread is
 * blocked on this mutex. If call made when thread blocked on mutex the 
 * behaviour is unpredictable
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMutexDestroy (IxOsalMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Initializes a fast mutex
 *
 * @param mutex - fast mutex handle
 *
 * Initializes a fast mutex object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalFastMutexInit (IxOsalFastMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Non-blocking attempt to lock a fast mutex
 *
 * @param mutex - fast mutex handle
 *
 * Attempts to lock a fast mutex object, returning immediately with
 * IX_SUCCESS if the lock was successful or IX_FAIL if the lock failed
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalFastMutexTryLock (IxOsalFastMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Unlocks a fast mutex
 *
 * @param mutex - fast mutex handle
 *
 * Unlocks a fast mutex object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalFastMutexUnlock (IxOsalFastMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Destroys a fast mutex object
 *
 * @param mutex - fast mutex handle
 *
 * Destroys a fast mutex object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalFastMutexDestroy (IxOsalFastMutex * mutex);

/**
 * @ingroup IxOsal
 *
 * @brief Initializes a semaphore
 *
 * @param semaphore - semaphore handle
 * @param value - initial semaphore value
 *
 * Initializes a semaphore object
 * Note: Semaphore initialization ixOsalSemaphoreInit API must be called 
 * first before using any OSAL Semaphore APIs
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSemaphoreInit (IxOsalSemaphore * semaphore,
                      UINT32 value);

/**
 * @ingroup IxOsal
 *
 * @brief Posts to (increments) a semaphore
 *
 * @param semaphore - semaphore handle
 *
 * Increments a semaphore object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSemaphorePost (IxOsalSemaphore * semaphore);

/**
 * @ingroup IxOsal
 *
 * @brief Waits on (decrements) a semaphore
 *
 * @param semaphore - semaphore handle
 * @param timeout - timeout, in ms; IX_OSAL_WAIT_FOREVER (-1) if the thread
 * is to block indefinitely or IX_OSAL_WAIT_NONE (0) if the thread is to
 * return immediately even if the call fails
 *
 * Decrements a semaphore, blocking if the semaphore is
 * unavailable (value is 0).
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSemaphoreWait (IxOsalSemaphore * semaphore,
                      INT32 timeout);

/**
 * @ingroup IxOsal
 *
 * @brief Non-blocking wait on semaphore
 *
 * @param semaphore - semaphore handle
 *
 * Decrements a semaphore, not blocking the calling thread if the semaphore
 * is unavailable
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSemaphoreTryWait (IxOsalSemaphore * semaphore);

/**
 * @ingroup IxOsal
 *
 * @brief Gets semaphore value
 *
 * @param semaphore - semaphore handle
 * @param value - location to store the semaphore value
 *
 * Retrieves the current value of a semaphore object
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSemaphoreGetValue (IxOsalSemaphore * semaphore,
                      UINT32 * value);

/**
 * @ingroup IxOsal
 *
 * @brief Destroys a semaphore object
 *
 * @param semaphore - semaphore handle
 *
 * Destroys a semaphore object; the caller should ensure that no thread is
 * blocked on this semaphore. If call made when thread blocked on semaphore the 
 * behaviour is unpredictable
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSemaphoreDestroy (IxOsalSemaphore * semaphore);


/* ======================== SpinLock functions =========================
 *
 */
#ifdef ENABLE_SPINLOCK

/**
 * @ingroup IxOsal
 *
 * @brief Initializes the SpinLock object
 *
 * @param slock - Spinlock handle
 * @param slockType - Spinlock type
 *
 * Initializes the SpinLock object and its type.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSpinLockInit(IxOsalSpinLock *slock, IxOsalSpinLockType slockType);


/**
 * @ingroup IxOsal
 *
 * @brief Acquires a spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine acquires a spin lock so the 
 * caller can synchronize access to shared data in a 
 * multiprocessor-safe way by raising IRQL.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Returns IX_SUCCESS if the spinlock is acquired. Returns IX_FAIL if
 *           spinlock handle is NULL. If spinlock is already acquired by any 
 *           other thread of execution then it tries in busy loop/spins till it
 *           gets spinlock.
 */
PUBLIC IX_STATUS ixOsalSpinLockLock(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Releases the spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine releases the spin lock which the thread had acquired
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - return IX_SUCCESS if the spinlock is released. Returns IX_FAIL if
 *           spinlockhandle passed is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlock(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine attempts to acquire a spin lock but doesn't block the thread
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - return IX_SUCCESS if the spinlock is acquired. Return IX_FAIL if 
 *           spinlock is already acquired other thread of execution or if the
 *           spinlock handle is NULL
 */
PUBLIC IX_STATUS ixOsalSpinLockTry(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Destroy the spin lock object 
 *
 * @param slock - Spinlock handle
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSpinLockDestroy(IxOsalSpinLock *slock);
/**
 * @ingroup IxOsal
 *
 * @brief checks whether spinlock can be acquired
 *
 * @param slock - Spinlock handle
 *
 * This routine checks whether spinlock available for lock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS if spinlock is locked.  Returns IX_FAIL if spinlock
 * is not locked.
 */

PUBLIC IX_STATUS ixOsalSpinLockIsLocked(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Acquires a spinlock
 *
 * @param slock - Spinlock handle
 *
 * This routine disables local irqs & then acquires a slock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage  This API can be used in user context or bottom half when critical
 *         section is shared between user context or  bottom half and the
 *         irq handler
 *
 * @return - returns IX_SUCCESS if spinlock is acquired. If the spinlock is not
 *           available then it busy loops/spins till slock available.  If the
 *           spinlock handle passed is NULL then returns IX_FAIL.
 */

PUBLIC IX_STATUS ixOsalSpinLockLockIrq(IxOsalSpinLock *slock);
/**
 * @ingroup IxOsal
 *
 * @brief Releases the spinlock
 *
 * @param slock - Spinlock handle
 *
 * This routine releases the acquired spinlock & enables the local irqs
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context or bottom half when critical
 *          section is shared between user context or  bottom half and
 *          irq handler
 *
 * @return - returns IX_SUCCESS if slock is unlocked. Returns IX_FAIL if the
 *           slock is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlockIrq(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spinlock
 *
 * @param slock - Spinlock handle
 *
 * This routine disables local irq & attempts to acquire a spinlock but
 * doesn't block the thread if spinlock not available.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context or bottom half when critical
 *          section is shared between user context or  bottom half and
 *          irq handler
 *
 * @return -If spinlock is available then returns the IX_SUCCESS with
 *          spinlock locked. If spinlock not available then enables the
 *          local irqs & returns IX_FAIL
 *
 */
PUBLIC IX_STATUS ixOsalSpinLockTryIrq(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 * 
 * @brief Acquires a spinlock
 * 
 * @param slock - Spinlock handle
 * 
 * This routine disables bottom half & then acquires a slock
 * 
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context when critical section is
 *           shared between user context & bottom half handler
 *
 * @return - returns IX_SUCCESS if spinlock is acquired. If the spinlock is not
 *           available then it busy loops/spins till slock available.  If the
 *           spinlock handle passed is NULL then returns IX_FAIL.
 */
PUBLIC IX_STATUS ixOsalSpinLockLockBh(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Releases the spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine releases the acquired spinlock & enables the
 * bottom half handler
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context when critical section is
 *           shared between user context & bottom half handler
 *
 * @return - returns IX_SUCCESS if slock is released or unlocked.
 *           Returns IX_FAIL if the slock is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlockBh(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spin lock
 *
 * @param slock - Spinlock handle
 *
 * This routine disables bottom half handler & attempts to acquire a spinlock
 * but doesn't block the thread if spinlock not available. It enables the bh &
 * returns IX_FAIL if spinlock not available.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @usage   This API can be used in user context when critical section is
 *           shared between user context & bottom half handler
 *
 * @return -Returns the IX_SUCCESS with spinlock locked if the spinlock is
 *          available. Enables the local irqs  & return IX_FAIL
 *           if spinlock is not available.
 */
PUBLIC IX_STATUS ixOsalSpinLockTryBh(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Acquires a spinlock
 *
 * @param slock - Spinlock handle
 * @param flags - local irqs saved in flags
 *
 * @usage   This API can be used when critical section is shared between
 *          irq routines
 *
 * This routine saves local irqs in flags & then acquires a spinlock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - returns IX_SUCCESS if spinlock acquired. If the spinlock is not
 *           available then it busy loops/spins till slock available.
 *           If the spinlock handle passed is NULL then returns IX_FAIL.
 */
PUBLIC IX_STATUS ixOsalSpinLockLockIrqSave(IxOsalSpinLock *slock, \
                                           UINT32 *flags);

/**
 * @ingroup IxOsal
 *
 * @brief Releases the spin lock
 *
 * @param slock - Spinlock handle
 * @param flags - local irqs saved in flags
 *
 * @usage   This API can be used when critical section is shared between
 *          irq routines
 *
 * This routine releases the acquired spin lock & restores irqs in flags
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - returns IX_SUCCESS if slock is unlocked. Returns IX_FAIL if the
 *           slock is NULL.
 */
PUBLIC IX_STATUS ixOsalSpinLockUnlockIrqRestore(IxOsalSpinLock *slock, \
                                                UINT32 *flags);

/**
 * @ingroup IxOsal
 *
 * @brief Tries to acquire the spinlock
 *
 * @param slock - Spinlock handle
 * @param flags - local irqs saved in flags
 *
 * This routine saves irq in flags & attempts to acquire a spinlock but
 * doesn't block the thread if the spin lock not avialble. If the
 * spinlock not available then it restore the irqs & return IX_FAIL.
 * This API can be used when critical section is shared between irq routines
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return -Returns the IX_SUCCESS with spinlock locked if the spinlock is
 *          available. Enables the local irqs & returns IX_FAIL
 *           if spinlock not available.
 */
PUBLIC IX_STATUS ixOsalSpinLockTryIrqSave(IxOsalSpinLock *slock, UINT32 *flags);

#endif /* ENABLE_SPINLOCK */

/**
 * @ingroup IxOsal
 *
 * @brief Initializes value of atomic variable at compile time
 *
 * @param  val    IN  - value that needs to be assigned to atomic variable
 *
 * Initializes the value of atomicVar at compile time
 * Usage - IxOsalAtomic atomicVar = IX_OSAL_ATOMIC_INIT(0);
 * 
 * @li Reentrant: yes
 * @li IRQ safe:  yes 
 *
 * @return value initialized in atomic variable
 */
 
#define IX_OSAL_ATOMIC_INIT(val) \
    IX_OSAL_OS_ATOMIC_INIT(val)

/**
 * @ingroup IxOsal
 *
 * @brief Atomically read the value of atomic variable
 *
 * @param  atomicVar  IN   - atomic variable
 *
 * Atomically reads the value of atomicVar to the outValue 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return  atomicVar value
 */

PUBLIC UINT32 ixOsalAtomicGet(IxOsalAtomic *atomicVar);


/**
 * @ingroup IxOsal
 *
 * @brief Atomically set the value of atomic variable
 *
 * @param  inValue    IN   -  atomic variable to be set equal to inValue
 *
 * @param  atomicVar  OUT  - atomic variable
 *
 * Atomically sets the value of IxOsalAtomicVar to the value given
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalAtomicSet(UINT32 inValue,IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief add the value to atomic variable
 *
 * @param  inValue (in)   -  value to be added to the atomic variable
 *
 * @param  atomicVar (in & out)   - atomic variable
 *
 * Atomically adds the value of inValue to the IxOsalAtomicVar 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalAtomicAdd(UINT32 inValue, IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief subtract the value from atomic variable
 *
 * @param  inValue   IN     -  atomic variable value to be subtracted by value
 *
 * @param  atomicVar IN/OUT - atomic variable
 *
 * Atomically subtracts the value of IxOsalAtomicVar by inValue
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalAtomicSub(UINT32 inValue, IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief subtract the value from atomic variable and test result
 *
 * @param  inValue   IN     - value to be subtracted from the atomic variable
 *
 * @param  atomicVar IN/OUT - atomic variable
 *
 * Atomically subtracts the IxOsalAtomicVar value by inValue and
 * test the result.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return TRUE if the result is zero or FALSE for other cases.
 */

PUBLIC IX_STATUS ixOsalAtomicSubAndTest(
        UINT32 inValue, IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief increment value of atomic variable by 1
 *
 * @param  atomicVar IN/OUT   - atomic variable
 *
 * Atomically increments the value of IxOsalAtomicVar by 1.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalAtomicInc(IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief decrement value of atomic variable by 1 
 *
 * @param  atomicVar IN/OUT  - atomic variable
 *
 * Atomically decrements the value of IxOsalAtomicVar by 1.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalAtomicDec(IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief decrement atomic variable value by 1 and test result
 *
 * @param  atomicVar (IN/OUT)   - atomic variable
 *
 * Atomically decrements the value of IxOsalAtomicVar by 1 and test 
 * result for zero.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return TRUE if the result is zero or FALSE otherwise
 */

PUBLIC IX_STATUS ixOsalAtomicDecAndTest(IxOsalAtomic *atomicVar);

/**
 * @ingroup IxOsal
 *
 * @brief increment atomic variable by 1 and test result
 *
 * @param  atomicVar (IN/OUT)   - atomic variable
 *
 * Atomically increments the value of IxOsalAtomicVar by 1 and test 
 * result for zero.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return TRUE if the result is zero or FALSE otherwise
 */

PUBLIC IX_STATUS ixOsalAtomicIncAndTest(IxOsalAtomic *atomicVar);


/**
 * @ingroup IxOsal
 *
 * @brief memory barrier which orders both memory read and writes 
 *
 * memory barrier that orders both memory read and writes 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalMemBarrier(void);

/**
 * @ingroup IxOsal
 *
 * @brief memory barrier which orders memory reads 
 *
 * memory barrier that orders memory reads 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalReadMemBarrier(void);

/**
 * @ingroup IxOsal
 *
 * @brief memory barrier which orders memory writes 
 *
 * memory barrier that orders memory writes 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return none
 */

PUBLIC void ixOsalWriteMemBarrier(void);


/* ========================== Time functions  ===========================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Yields execution of current thread
 *
 * Yields the execution of the current thread
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalYield (void);

/**
 * @ingroup IxOsal
 *
 * @brief Yielding sleep for a number of milliseconds
 *
 * @param milliseconds - number of milliseconds to sleep
 *
 * The calling thread will sleep for the specified number of milliseconds.
 * This sleep is yielding, hence other tasks will be scheduled by the
 * operating system during the sleep period. Calling this function with an
 * argument of 0 will place the thread at the end of the current scheduling
 * loop.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC IX_STATUS ixOsalSleep (UINT32 milliseconds);

/**
 * @ingroup IxOsal
 *
 * @brief Busy sleep for a number of microseconds
 *
 * @param microseconds - number of microseconds to sleep
 *
 * Sleeps for the specified number of microseconds, without explicitly
 * yielding thread execution to the OS scheduler
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalBusySleep (UINT32 microseconds);

/**
 * @ingroup IxOsal
 *
 * @brief Retrieves the current timestamp
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - The current timestamp
 *
 * @note The implementation of this function is platform-specific. Not
 * all the platforms provide a high-resolution timestamp counter.
 */
PUBLIC UINT32 ixOsalTimestampGet (void);

/**
 * @ingroup IxOsal
 *
 * @brief Resolution of the timestamp counter
 *
 * Retrieves the resolution (frequency) of the timestamp counter.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - The resolution of the timestamp counter
 *
 * @note The implementation of this function is platform-specific. Not all
 * the platforms provide a high-resolution timestamp counter.
 */
PUBLIC UINT32 ixOsalTimestampResolutionGet (void);

/**
 * @ingroup IxOsal
 *
 * @brief System clock rate, in ticks
 *
 * Retrieves the resolution (number of ticks per second) of the system clock
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - The system clock rate
 *
 * @note The implementation of this function is platform and OS-specific.
 * The system clock rate is not always available - 
 */
PUBLIC UINT32 ixOsalSysClockRateGet (void);

/**
 * @ingroup IxOsal
 *
 * @brief Current system time
 *
 * @param tv - pointer to an IxOsalTimeval structure to store the current
 *             time in
 *
 * Retrieves the current system time (real-time)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 *
 * @note The implementation of this function is platform-specific. Not all
 * platforms have a real-time clock.
 */
PUBLIC IX_STATUS ixOsalTimeGet (IxOsalTimeval * tv);



/**
 * @ingroup IxOsalInternal
 *
 * @brief Converts ixOsalTimeVal into ticks
 *
 * @param tv - an IxOsalTimeval structure
 *
 * Internal function to convert an IxOsalTimeval structure into OS ticks
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Corresponding number of ticks
 *
 * Note: This function is OS-independent and internal to OSAL.
 */
PUBLIC UINT32 ixOsalTimevalToTicks (IxOsalTimeval tv);


/**
 * @ingroup IxOsal
 *
 * @brief Converts ixOsalTimeVal into ticks
 *
 * @param tv - an IxOsalTimeval structure
 *
 * Converts an IxOsalTimeval structure into OS ticks
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Corresponding number of ticks
 *
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TIMEVAL_TO_TICKS(tv)  ixOsalTimevalToTicks(tv)



/**
 * @ingroup IxOsalInternal
 *
 * @brief Converts ticks into ixOsalTimeVal
 *
 * @param ticks - number of ticks
 * @param pTv - pointer to the destination structure
 *
 * Internal function to convert the specified number of ticks into 
 * a IxOsalTimeval structure
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Corresponding IxOsalTimeval structure
 * Note: This function is OS-independent and internal to OSAL
 */

PUBLIC void ixOsalTicksToTimeval (UINT32 ticks, IxOsalTimeval * pTv);


/**
 * @ingroup IxOsal
 *
 * @brief Converts ticks into ixOsalTimeVal
 *
 * @param ticks - number of ticks
 * @param pTv - pointer to the destination structure
 *
 * Converts the specified number of ticks into an IxOsalTimeval structure
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Corresponding IxOsalTimeval structure
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TICKS_TO_TIMEVAL(ticks, pTv)  \
    ixOsalTicksToTimeval(ticks, pTv)




/**
 * @ingroup IxOsal
 *
 * @brief Converts ixOsalTimeVal to milliseconds
 *
 * @param tv - IxOsalTimeval structure to convert
 *
 * Converts an IxOsalTimeval structure into milliseconds
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Corresponding number of milliseconds
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TIMEVAL_TO_MS(tv)     ((tv.secs * 1000) + (tv.nsecs  / IX_OSAL_MILLION))


/**
 * @ingroup IxOsal
 *
 * @brief Converts milliseconds to IxOsalTimeval
 *
 * @param milliseconds - number of milliseconds to convert
 * @param pTv - pointer to the destination structure
 *
 * Converts a millisecond value into an IxOsalTimeval structure
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Corresponding IxOsalTimeval structure
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_MS_TO_TIMEVAL(milliseconds, pTv)  \
        ((IxOsalTimeval *) pTv)->secs = milliseconds / 1000;              \
        ((IxOsalTimeval *) pTv)->nsecs = (milliseconds % 1000) * 1000000


/**
 * @ingroup IxOsal
 *
 * @brief "equal" comparison for IxOsalTimeval
 *
 * @param tvA, tvB - IxOsalTimeval structures to compare
 *
 * Compares two IxOsalTimeval structures for equality
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - TRUE if the structures are equal
 *         - FALSE otherwise
 * Note: This function is OS-independant
 */
#define IX_OSAL_TIME_EQ(tvA, tvB)        \
        ((tvA).secs == (tvB).secs && (tvA).nsecs == (tvB).nsecs)


/**
 * @ingroup IxOsal
 *
 * @brief "less than" comparison for IxOsalTimeval
 *
 * @param tvA, tvB - IxOsalTimeval structures to compare
 *
 * Compares two IxOsalTimeval structures to determine if the first one is
 * less than the second one
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - TRUE if tvA < tvB
 *         - FALSE otherwise
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TIME_LT(tvA,tvB) \
        ((tvA).secs  < (tvB).secs ||    \
        ((tvA).secs == (tvB).secs && (tvA).nsecs < (tvB).nsecs))


/**
 * @ingroup IxOsal
 *
 * @brief "greater than" comparison for IxOsalTimeval
 *
 * @param tvA, tvB - IxOsalTimeval structures to compare
 *
 * Compares two IxOsalTimeval structures to determine if the first one is
 * greater than the second one
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - TRUE if tvA > tvB
 *         - FALSE  otherwise
 * Note: This function is OS-independent.
 */
#define IX_OSAL_TIME_GT(tvA, tvB)  \
        ((tvA).secs  > (tvB).secs ||    \
        ((tvA).secs == (tvB).secs && (tvA).nsecs > (tvB).nsecs))


/**
 * @ingroup IxOsal
 *
 * @brief "add" operator for IxOsalTimeval
 *
 * @param tvA, tvB - IxOsalTimeval structures to add
 *
 * Adds the second IxOsalTimevalStruct to the first one (equivalent to
 * tvA += tvB)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent.
 */
#define IX_OSAL_TIME_ADD(tvA, tvB)  \
        (tvA).secs += (tvB).secs;   \
        (tvA).nsecs += (tvB).nsecs; \
        if ((tvA).nsecs >= IX_OSAL_BILLION) \
        { \
        (tvA).secs++; \
        (tvA).nsecs -= IX_OSAL_BILLION; }


/**
 * @ingroup IxOsal
 *
 * @brief "subtract" operator for IxOsalTimeval
 *
 * @param tvA, tvB - IxOsalTimeval structures to subtract
 *
 * Subtracts the second IxOsalTimevalStruct from the first one (equivalent
 * to tvA -= tvB)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TIME_SUB(tvA, tvB)   \
        if ((tvA).nsecs >= (tvB).nsecs) \
        { \
          (tvA).secs -= (tvB).secs; \
          (tvA).nsecs -= (tvB).nsecs; \
        } \
        else \
        { \
          (tvA).secs -= ((tvB).secs + 1); \
          (tvA).nsecs += IX_OSAL_BILLION - (tvB).nsecs; \
        }

/**
 * @ingroup IxOsal
 *
 * @brief "zero" comparison for IxOsalTimeval
 *
 * @param tvA - IxOsalTimeval structure to check for zero
 *
 * Checks if the IxOsalTimevalStruct passed has value zero (equivalent
 * to tvA == 0)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TIME_ISZERO(tvA)  \
        ((tvA).secs == 0 && (tvA).nsecs == 0)

/**
 * @ingroup IxOsal
 *
 * @brief "set" operator for IxOsalTimeval
 *
 * @param tvA, tvB - IxOsalTimeval structures to use for set
 *
 * Set the value of second IxOsalTimevalStruct to the first 
 * IxOsalTimevalStruct (equivalent to tvA = tvB)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent. Implemented by core.
 */
#define IX_OSAL_TIME_SET(tvA,tvB) \
        { (tvA).secs = (tvB).secs; (tvA).nsecs = (tvB).nsecs; }

/**
 * @ingroup IxOsal
 *
 * @brief "normalize" operator for IxOsalTimeval
 *
 * @param tvA - IxOsalTimeval structure to normalize
 *
 * Check nanoseconds field of the IxOsalTimevalStruct and set it to a
 * positive value less than IX_OSAL_BILLION by adjusting 'seconds' field
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent. Implemented by core.
 */
/* IX_OSSL_TIME_NORMALIZE */
#define IX_OSAL_TIME_NORMALIZE(tvA) \
    { (tvA).secs += (tvA).nsecs / IX_OSAL_BILLION; \
      (tvA).nsecs = (tvA).nsecs % IX_OSAL_BILLION; }  

/**
 * @ingroup IxOsal
 *
 * @brief Validity check for IxOsalTimeval
 *
 * @param tvA - IxOsalTimeval structure to check for validity
 *
 * Checks if the nanoseconds field of IxOsalTimevalStruct has a valid
 * value
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent. Implemented by core.
 */
/* IX_OSSL_TIME_VALID */
#define IX_OSAL_TIME_VALID(tvA) \
        ((tvA).nsecs >= 0 && (tvA).nsecs < IX_OSAL_BILLION)

/**
 * @ingroup IxOsal
 *
 * @brief "set to zero" operator for IxOsalTimeval
 *
 * @param tvA - IxOsalTimeval structure to set to zero
 *
 * Set the IxOsalTimevalStruct to contain value zero (equivalent
 * to tvA = 0)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - none
 * Note: This function is OS-independent. Implemented by core.
 */
/* IX_OSSL_TIME_ZERO */
#define IX_OSAL_TIME_ZERO(tvA) \
        { (tvA).secs = 0; (tvA).nsecs = 0; }

/* ============================= Logging  ==============================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Setting the module name
 *
 * @param moduleName - the string to be prepended with OSAL log message
 *
 * A facility provided to the user to prepend module name with OSAL 
 * log messages. Example usage of this API to help the user to separate
 * messages from other modules. After the API called the subsequent calls to
 * ixOsalLog or ixOsalStdLog API's shall log the module name followed with 
 * regular OSAL log message. To disable module name prepend users need to 
 * invoke this API as ixOsalLogSetPrefix("");   
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - None. 
 *
 */
PUBLIC VOID ixOsalLogSetPrefix (CHAR * moduleName);

/**
 * @ingroup IxOsal
 *
 * @brief Interrupt-safe logging function
 *
 * @param level - identifier prefix for the message
 * @param device - output device
 * @param format - message format, in a printf format
 * @param arg1, arg2, arg3, arg4, arg5, arg6 - up to 6 arguments to be printed
 *
 * IRQ-safe logging function, similar to printf. Accepts up to 6 arguments
 * to print (excluding the level, device and the format). This function will
 * actually display the message only if the level is lower than the current
 * verbosity level or if the IX_OSAL_LOG_USER level is used. An output device
 * must be specified (see IxOsalTypes.h).
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Beside the exceptions documented in the note below, the returned
 * value is the number of printed characters, or -1 if the parameters are
 * incorrect (NULL format, unknown output device)
 *
 * @note The exceptions to the return value are:
 * VxWorks*: The return value is 32 if the specified level is 1 and 64
 * if the specified level is greater than 1 and less or equal than 9.
 * WinCE*: If compiled for EBOOT then the return value is always 0.
 *
 * @note The given print format should take into account the specified
 * output device. IX_OSAL_STDOUT supports all the usual print formats,
 * however a custom hex display specified by IX_OSAL_HEX would support
 * only a fixed number of hexadecimal digits.
 */
PUBLIC INT32 ixOsalLog (IxOsalLogLevel level,
            IxOsalLogDevice device,
            char *format,
            int arg1,
            int arg2, int arg3, int arg4, int arg5, int arg6);

/**
 * @ingroup IxOsal
 *
 * @brief sets the current logging verbosity level
 *
 * @param level - new log verbosity level
 *
 * Sets the log verbosity level. The default value is IX_OSAL_LOG_ERROR.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - Old log verbosity level
 */
PUBLIC UINT32 ixOsalLogLevelSet (UINT32 level);

/**
 * @ingroup IxOsal
 *
 * @brief simple logging function 
 *
 * @param arg_pFmtString  - message format, in printf format
 * @param ...             - variable arguments
 *
 * Logging function, similar to printf. This provides a barebones logging 
 * mechanism for users without differing verbosity levels. This interface
 * is not quaranteed to be IRQ safe.   
 * 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
IX_STATUS ixOsalStdLog(const char* arg_pFmtString, 
                        ...);


/* ============================= Logging  ==============================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Schedules a repeating timer
 *
 * @param timer - handle of the timer object
 * @param period - timer trigger period, in milliseconds
 * @param priority - timer priority (0 being the highest)
 * @param callback - user callback to invoke when the timer triggers
 * @param param - custom parameter passed to the callback
 *
 * Schedules a timer to be called every period milliseconds. The timer
 * will invoke the specified callback function possibly in interrupt
 * context, passing the given parameter. If several timers trigger at the
 * same time contention issues are dealt according to the specified timer
 * priorities.
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalRepeatingTimerSchedule (IxOsalTimer * timer,
                           UINT32 period,
                           UINT32 priority,
                           IxOsalVoidFnVoidPtr callback,
                           void *param);

/**
 * @ingroup IxOsal
 *
 * @brief Schedules a single-shot timer
 *
 * @param timer - handle of the timer object
 * @param period - timer trigger period, in milliseconds
 * @param priority - timer priority (0 being the highest)
 * @param callback - user callback to invoke when the timer triggers
 * @param param - custom parameter passed to the callback
 *
 * Schedules a timer to be called after period milliseconds. The timer
 * will cease to function past its first trigger. The timer will invoke
 * the specified callback function, possibly in interrupt context, passing
 * the given parameter. If several timers trigger at the same time contention
 * issues are dealt according to the specified timer priorities.
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSingleShotTimerSchedule (IxOsalTimer * timer,
                   UINT32 period,
                   UINT32 priority,
                   IxOsalVoidFnVoidPtr callback, void *param);

/**
 * @ingroup IxOsal
 *
 * @brief Cancels a running timer
 *
 * @param timer - handle of the timer object
 *
 * Cancels a single-shot or repeating timer.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalTimerCancel (IxOsalTimer * timer);

/**
 * @ingroup IxOsal
 *
 * @brief displays all the running timers
 *
 * Displays a list with all the running timers and their parameters (handle,
 * period, type, priority, callback and user parameter)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalTimersShow (void);


/* ============================= Version  ==============================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief provides the name of the Operating System running
 *
 * @param osName - Pointer to a NULL-terminated string of characters
 * that holds the name of the OS running.
 * This is both an input and an ouput parameter
 * @param maxSize - Input parameter that defines the maximum number of
 * bytes that can be stored in osName
 *
 * Returns a string of characters that describe the Operating System name
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * return - IX_SUCCESS for successful retrieval
 *        - IX_FAIL if (osType == NULL | maxSize =< 0)
 */
PUBLIC IX_STATUS ixOsalOsNameGet (INT8* osName, INT32 maxSize);

/**
 * @ingroup IxOsal
 *
 * @brief provides the version of the Operating System running
 *
 * @param osVersion - Pointer to a NULL terminated string of characters
 * that holds the version of the OS running.
 * This is both an input and an ouput parameter
 * @param maxSize - Input parameter that defines the maximum number of
 * bytes that can be stored in osVersion
 *
 * Returns a string of characters that describe the Operating System's version
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * return - IX_SUCCESS for successful retrieval
 *        - IX_FAIL if (osVersion == NULL | maxSize =< 0)
 */
PUBLIC IX_STATUS ixOsalOsVersionGet(INT8* osVersion, INT32 maxSize);


#ifdef ENABLE_PCI

/* ============================= PCI  ==============================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Find the PCI device for the particular vendor and device.
 *
 * @param vendor_id - The vendor ID of the device to be found.
 * @param device_id - The device ID of the device to be found.
 * @param pci_device -  The last found IxOsalPciDev or NULL.
 *
 *  This function will find the PCI device for the particular vendor and device.
 *  The pci_device parameter should be NULL when calling for the first time
 *  and the last returned value when searching for multiple devices of
 *  the same ID.
 *
 * @li Reentrant: yes 
 * @li IRQ safe:  yes
 *
 * @return - IxOsalPciDev handle if found/NULL.
 */
PUBLIC IxOsalPciDev ixOsalPciDeviceFind(UINT32 vendor_id,UINT32 device_id,IxOsalPciDev pci_dev);

/**
 * @ingroup IxOsal
 *
 * @brief Obtain the bus, slot and function of the PCI device.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param bus   -   The returned bus or NULL if the bus is not needed.
 * @param slot  -   The returned slot or NULL if the bus is not needed
 * @param func  -   The returned func or NULL if the bus is not needed.
 *
 *  This function will return the bus slot and function for an IxOsalPciDev
 *  previously obtained from ixOsalPciDeviceFind(). Any of the bus/slot/func
 *  parameters may be null if the information is not needed.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL.
 */
PUBLIC INT32 ixOsalPciSlotAddress(IxOsalPciDev pci_dev,UINT32 *bus,UINT32 *slot,UINT32 *func);

/**
 * @ingroup IxOsal
 *
 * @brief Retrieve byte of information from the PCI configuration space.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param offset     -  Offset from the base of the configuration space.
 * @param val        -  Byte value fetched from the mentioned location
 *
 *  This function retrieves a byte of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */
PUBLIC INT32 ixOsalPciConfigReadByte(IxOsalPciDev pci_dev,UINT32 offset,UINT8* val);

/**
 * @ingroup IxOsal
 *
 * @brief Retrieve word of information from the PCI configuration space.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param offset     -  Offset from the base of the configuration space.
 * @param val        -  Word value fetched from the mentioned location
 *
 *  This function retrieves a word of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */
PUBLIC INT32 ixOsalPciConfigReadShort(IxOsalPciDev pci_dev,UINT32 offset,UINT16* val);

/**
 * @ingroup IxOsal
 *
 * @brief Retrieve double word of information from the PCI configuration space.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param offset     -  Offset from the base of the configuration space.
 * @param val        -  Double word value fetched from the mentioned location
 *
 *  This function retrieves a double word of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */
PUBLIC INT32 ixOsalPciConfigReadLong(IxOsalPciDev pci_dev,UINT32 offset,UINT32* val);

/**
 * @ingroup IxOsal
 *
 * @brief Set byte information in the PCI configuration space.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param offset     -  Offset from the base of the configuration space.
 * @param val        -  Byte value to be set at the mentioned location
 *
 *  This function sets a byte of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */
PUBLIC INT32 ixOsalPciConfigWriteByte(IxOsalPciDev pci_dev,UINT32 offset,UINT8 val);

/**
 * @ingroup IxOsal
 *
 * @brief Set word information in the PCI configuration space.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param offset     -  Offset from the base of the configuration space.
 * @param val        -  Word value to be set at the mentioned location
 *
 *  This function sets a word of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */
PUBLIC INT32 ixOsalPciConfigWriteShort(IxOsalPciDev pci_dev,UINT32 offset,UINT16 val);

/**
 * @ingroup IxOsal
 *
 * @brief Set double word information in the PCI configuration space.
 *
 * @param pci_device -  The IxOsalPciDev to query.
 * @param offset     -  Offset from the base of the configuration space.
 * @param val        -  Double word value to be set at the mentioned location
 *
 *  This function sets a double word of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */
PUBLIC INT32 ixOsalPciConfigWriteLong(IxOsalPciDev pci_dev,UINT32 offset,UINT32 val);

/**
 * @ingroup IxOsal
 *
 * @brief Free a PCI device handle.
 *
 * @param pci_device -  The IxOsalPciDev to free.
 *
 *  This function free the IxOsalPciDev * that previously allocated with the
 *  ixOsalPciDeviceFind.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - None.
 */
PUBLIC void ixOsalPciDeviceFree(IxOsalPciDev pci_dev);

#endif /* ENABLE_PCI */

/* ============================= MATH ==============================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief UINT64 data type division with 32 bit divisor
 *
 * @param  dividend - dividend (UINT64)
 *
 * @param  divisor - divisor (UINT32)
 *
 *  This function enable UINT64 datatype division for 32 bit system.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - quotient (UINT64)
 */
#define IX_OSAL_UDIV64_32(dividend, divisor) \
    IX_OSAL_OS_UDIV64_32(dividend, divisor)

/**
 * @ingroup IxOsal
 *
 * @brief UINT64 data type mod with 32 bit divisor
 *
 * @param  dividend - dividend (UINT64)
 *
 * @param  divisor - divisor (UINT32)
 *
 *  This function enable UINT64 datatype mod for 32 bit system.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - remainder (UINT32)
 */
#define IX_OSAL_UMOD64_32(dividend, divisor) \
    IX_OSAL_OS_UMOD64_32(dividend, divisor)

#ifdef IX_OSAL_MEM_MAP_GLUECODE

/*
 *  Glue code for memory map
 */
PUBLIC void ixOsalGlueCodeMemoryMapInit(UINT32 index,UINT32 phyAddr,UINT32 mapSize,UINT32 virtAddr);


PUBLIC void ixOsalGlueCodeMemoryMapUnInit(UINT32 index,UINT32 *virtAddr);

#endif /* IX_OSAL_MEM_MAP_GLUECODE */

/**
 * @} IxOsal
 */


/**
 * @ingroup IxOsal
 *
 * @brief  get the Thread id of the current thread in execution. 
 *
 * @param   ptrTid - pointer IxOsalThread structure
 *
 *  This function returns the Thread id.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/Error code on failure.
 */

PUBLIC
IX_STATUS ixOsalThreadGetId(IxOsalThread *ptrTid);

/**
 * @ingroup IxOsal
 *
 * @brief   get the Policy and priority of the thread
 *
 * @param   tid - pointer IxOsalThread structure
 *
 * @param   policy (OUT)  -  Thread policy
 *
 * @param   priority (OUT) -  Thread priority
 *
 *  This function queries for the Policy and priority of the thread.
 *
 * @li Reentrant: yes
 * @li IRQ safe: no 
 *
 * @return - IX_SUCCESS/Error code on failure.
 */

PUBLIC
IX_STATUS ixOsalThreadGetPolicyAndPriority(
            IxOsalThread  *tid,
            UINT32        *policy,
            UINT32        *priority);

/**
 * @ingroup IxOsal
 *
 * @brief Waits on (decrements) a semaphore for specified time
 *
 * @param sid - semaphore handle
 * @param timeout - timeout, in ms; IX_OSAL_WAIT_FOREVER (-1) if the thread
 * is to block indefinitely or IX_OSAL_WAIT_NONE (0) if the thread is to
 * return immediately even if the call fails
 *
 * Decrements a semaphore, blocking for specified time if the semaphore is
 * unavailable (value is 0).
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalSemaphoreDownTimeout(
              IxOsalSemaphore *sid,
                  INT32       timeout);

/**
 * @ingroup IxOsal
 *
 * @brief Waits on (decrements) a semaphore for specified time.
 *
 * @param sid - semaphore handle
 * @param timeout - timeout, in ms; IX_OSAL_WAIT_FOREVER (-1) if the thread
 * is to block indefinitely or IX_OSAL_WAIT_NONE (0) if the thread is to
 * return immediately even if the call fails
 *
 * Decrements a semaphore, blocking for specified time if the semaphore is
 * unavailable (value is 0).The current thread or process can be interrupted
 * by a signal.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalSemaphoreWaitInterruptible(
              IxOsalSemaphore *sid,
              INT32           timeout);

/**
 * @ingroup IxOsal
 *
 * @brief  Wakes up thread blocked on semapohore. Increments a semaphore if  
 * the value leass than one and wakes up threads waiting on wait queue.
 *
 * @param sid - semaphore handle
 *
 * increments a semaphore if the value less than one and wakes up the thread
 * which is waiting for the semaphore.
 *
 * 
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalSemaphorePostWakeup(IxOsalSemaphore *sid);

/**
 * @ingroup IxOsal
 *
 * @brief  Wakes up thread blocked on semapohore. Increments a semaphore if 
 * the value leass than one and wakes up all threads waiting on wait queue.
 *
 * @param sid - semaphore handle
 *
 * increments a semaphore if the value less than one and wakes up the all threads 
 * which is waiting for the semaphore.
 *
 *  
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalSemaphoreFlush(IxOsalSemaphore *sid);

/**
 * @ingroup IxOsal
 *
 * @brief  thread goes to sleep for number of ticks specified
 *
 * @param sleeptime_ticks - number of ticks 
 *
 * Forces current thread of execution to sleep for speicfied number of ticks
 *
 * 
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalSleepTick(UINT32 sleeptime_ticks);

/**
 * @ingroup IxOsal
 * 
 * @brief  thread goes to sleep for number of mili seconds specified, but the thread
 * can be interrupted by a signal
 * 
 * @param sleeptime_ms - number of ms  
 *
 * Forces current thread of execution to sleep for speicfied number of ticks.
 * The thread of execution which goes to sleep can be interrupted by a signal.
 *
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */

PUBLIC IX_STATUS ixOsalSleepUninterruptible(
             UINT32 sleeptime_ms);

/**
 * @ingroup IxOsal
 *
 * @brief converts 16bit value from host to network byte order
 *
 * @param  uData - 16bit number 
 *
 *  This function converts 16bit value from host to network byte order
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - 16bit network byte ordered value of uData
 */
#define IX_OSAL_HOST_TO_NW_16(uData) IX_OSAL_OEM_HOST_TO_NW_16(uData)

/**
 * @ingroup IxOsal
 *
 * @brief converts 32bit value from host to network byte order
 *
 * @param  uData - 32bit number
 *
 *  This function converts 32bit value from host to network byte order
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - 32bit network byte ordered value of uData
 */
#define IX_OSAL_HOST_TO_NW_32(uData)  IX_OSAL_OEM_HOST_TO_NW_32(uData)

/**
 * @ingroup IxOsal
 *
 * @brief converts 64bit value from host to network byte order
 *
 * @param  uData - 64bit number 
 *
 *  This function converts 64bit value from host to network byte order
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - 64bit network byte ordered value of uData
 */
#define IX_OSAL_HOST_TO_NW_64(uData)  IX_OSAL_OEM_HOST_TO_NW_64(uData)

/**
 * @ingroup IxOsal
 *
 * @brief converts 128bit value from host to network byte order
 *
 * @param  uDataSrc - UINT128 type source 
 *
 * @param  uDataDest - UINT128 type dest
 *
 *  This function converts 128bit uDataSrc value from host type to 
 *  network byte order in uDataDest. It is expected that both the 
 *  function arguments are UINT128 type. To access the UINT28 users
 *  use the mUINT32 array. 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - None
 */
#define IX_OSAL_HOST_TO_NW_128(uDataSrc, uDataDest) \
              IX_OSAL_OEM_HOST_TO_NW_128(uDataSrc, uDataDest)

/**
 * @ingroup IxOsal
 *
 * @brief converts 16bit value from network to host byte order
 *
 * @param  uData - 16bit number  
 *
 *  This function converts 16bit value from network to host byte order
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - 16bit host byte ordered value of uData
 */
#define IX_OSAL_NW_TO_HOST_16(uData) IX_OSAL_OEM_NW_TO_HOST_16(uData)

/**
 * @ingroup IxOsal
 *
 * @brief converts 32bit value from network to host byte order
 *
 * @param  uData - 32bit number 
 *
 *  This function converts 32bit value from network to host byte order
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - 32bit host byte ordered value of uData
 */
#define IX_OSAL_NW_TO_HOST_32(uData)  IX_OSAL_OEM_NW_TO_HOST_32(uData)

/**
 * @ingroup IxOsal
 *
 * @brief converts 64bit value from network to host byte order
 *
 * @param  uData - 64bit number 
 *
 *  This function converts 64bit value from network to host byte order
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - 64bit host byte ordered value of uData
 */
#define IX_OSAL_NW_TO_HOST_64(uData)  IX_OSAL_OEM_NW_TO_HOST_64(uData)

/**
 * @ingroup IxOsal
 *
 * @brief converts 128bit value from network to host byte order
 *
 * @param  uDataSrc - UINT128 type source 
 *
 * @param  uDataDest - UINT128 type destination 
 *
 *  This function converts 128bit value from host to network
 *  byte order in uDataDest. It is expected that the function
 *  both the arguements are UINT128 type. To access the UINT128 users use
 *  mUINT32 array.

 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return - None
 */
#define IX_OSAL_NW_TO_HOST_128(uDataSrc, uDataDest) \
          IX_OSAL_OEM_NW_TO_HOST_128(uDataSrc, uDataDest)

/**
 * @ingroup IxOsal
 *
 * @brief generates 32bitrandom number
 *
 * @param  num - UINT32 pointer to number where the random number generated 
 *                      gets stored
 *
 *  This function generates 32bit random number.
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - None
 */
PUBLIC VOID ixOsalGetRandomNum32(UINT32 *num);

#ifdef __cplusplus
}
#endif

#endif /* IxOsal_H */
