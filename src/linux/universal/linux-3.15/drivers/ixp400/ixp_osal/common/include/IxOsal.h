/**
 * @file IxOsal.h
 *
 * @brief Top include file for OSAL
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

#ifndef IxOsal_H
#define IxOsal_H

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
 *	-# Data types, constants, defines
 *	-# Interrupts
 *              - bind interrupts to handlers
 *              - unbind interrupts from handlers
 *             	- disables all interrupts
 *              - enables all interrupts
 *              - selectively disables interrupts
 *              - enables an interrupt level
 *              - disables an interrupt level
 *      -# Memory
 *              - allocates memory
 *              - frees memory
 *              - copies memory zones
 *              - fills a memory zone
 *              - allocates cache-safe memory
 *              - frees cache-safe memory
 *              - physical to virtual address translation
 *              - virtual to physical address translation
 *              - cache to memory flush
 *              - cache line invalidate
 *      -# Threads
 *              - creates a new thread
 *              - starts a newly created thread
 *              - kills an existing thread
 *              - exits a running thread
 *              - sets the priority of an existing thread
 *              - suspends thread execution
 *              - resumes thread execution
 *      -# IPC
 *              - creates a message queue
 *              - deletes a message queue
 *              - sends a message to a message queue
 *              - receives a message from a message queue
 *      -# Thread Synchronisation
 *              - initializes a mutex
 *              - locks a mutex
 *              - unlocks a mutex
 *              - non-blocking attempt to lock a mutex
 *              - destroys a mutex object
 *              - initializes a fast mutex
 *              - non-blocking attempt to lock a fast mutex
 *              - unlocks a fast mutex
 *              - destroys a fast mutex object
 *              - initializes a semaphore
 *              - posts to (increments) a semaphore
 *              - waits on (decrements) a semaphore
 *              - non-blocking wait on semaphore
 *              - gets semaphore value
 *              - destroys a semaphore object
 *              - yields execution of current thread
 *      -# Time functions
 *              - yielding sleep for a number of milliseconds
 *              - busy sleep for a number of microseconds
 *              - value of the timestamp counter
 *              - resolution of the timestamp counter
 *              - system clock rate, in ticks
 *              - current system time
 *              - converts ixOsalTimeVal into ticks
 *              - converts ticks into ixOsalTimeVal
 *              - converts ixOsalTimeVal to milliseconds
 *              - converts milliseconds to IxOsalTimeval
 *              - "equal" comparison for IxOsalTimeval
 *              - "less than" comparison for IxOsalTimeval
 *              - "greater than" comparison for IxOsalTimeval
 *              - "add" operator for IxOsalTimeval
 *              - "subtract" operator for IxOsalTimeval
 *      -# Logging
 *              - sets the current logging verbosity level
 *              - interrupt-safe logging function
 *      -# Timer services
 *              - schedules a repeating timer
 *              - schedules a single-shot timer
 *              - cancels a running timer
 *              - displays all the running timers
 *	-# PCI Support
 *		- Find PCI device.
 *		- Read 8 bits from the configuration space
 *		- Read 16 bits from the configuration space
 *		- Read 32 bits from the configuration space
 * 		- Write 8 bits to the configuration space
 *		- Write 16 bits to the configuration space
 *		- Write 32 bits to the configuration space
 *		- Free PCI device
 *	-# SpinLock Support
 *		- Initializes the SpinLock object
 *		- Acquires a spin lock
 *		- Releases the spin lock
 *		- Tries to acquire the spin lock
 * 		- Destroy the spin lock object
 *  -# Optional Modules
 *              - Device drivers kernel
 *              - Buffer management module
 *              - I/O memory and endianess support module
 *
 * @{
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
 * Reentrant: no
 * IRQ safe:  no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
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
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return IX_SUCCESS if the operation succeeded or IX_FAIL otherwise
 */
PUBLIC void ixOsalIrqUnlock (UINT32 irqEnable);

/**
 * @ingroup IxOsal
 *
 * @brief Selectively disables interrupts
 *
 * @param irqLevel ­ new interrupt level
 *
 * Disables the interrupts below the specified interrupt level
 *
 * @li Reentrant: no
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
 * @param irqLevel ­ interrupt level to enable
 *
 * Enables the specified interrupt level
 *
 * @li Reentrant: no
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
 * @param irqLevel ­ interrupt level to disable
 *
 * Disables the specified interrupt level
 *
 * @li Reentrant: no
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
 * @brief Allocates memory
 *
 * @param size - memory size to allocate, in bytes
 *
 * Allocates a memory zone of a given size
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return Pointer to the allocated zone or NULL if the allocation failed
 */
PUBLIC void *ixOsalMemAlloc (UINT32 size);

/**
 * @ingroup IxOsal
 *
 * @brief Frees memory
 *
 * @param ptr - pointer to the memory zone
 *
 * Frees a previously allocated memory zone
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalMemFree (void *ptr);

/**
 * @ingroup IxOsal
 *
 * @brief Copies memory zones
 *
 * @param dest  - destination memory zone
 * @param src   - source memory zone
 * @param count - number of bytes to copy
 *
 * Copies count bytes from the source memory zone pointed by src into the
 * memory zone pointed by dest.
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return Pointer to the destination memory zone
 */
PUBLIC void *ixOsalMemCopy (void *dest, void *src, UINT32 count);

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
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return Pointer to the memory zone
 */
PUBLIC void *ixOsalMemSet (void *ptr, UINT8 filler, UINT32 count);

#if !defined(__linux_user) && !defined(__freebsd_user) 

/**
 * @ingroup IxOsal
 *
 * @brief physical to virtual address translation
 *
 * @param physAddr - physical address
 *
 * Converts a physical address into its equivalent MMU-mapped virtual address
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return Corresponding virtual address, as UINT32
 */
#define IX_OSAL_MMU_PHYS_TO_VIRT(physAddr) \
    IX_OSAL_OS_MMU_PHYS_TO_VIRT(physAddr)

/**
 * @ingroup IxOsal
 *
 * @brief virtual to physical address translation
 *
 * @param virtAddr - virtual address
 *
 * Converts a virtual address into its equivalent MMU-mapped physical address
 *
 * @li Reentrant: no
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
 * otherwise memory corruption can occur.
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return Pointer to the memory zone or NULL if allocation failed
 *
 * @note It is important to note that cache coherence is maintained in
 * software by using the IX_OSAL_CACHE_FLUSH and IX_OSAL_CACHE_INVALIDATE
 * macros to maintain consistency between cache and external memory.
 */
PUBLIC void *ixOsalCacheDmaMalloc (UINT32 size);

/* Macros for ixOsalCacheDmaMalloc*/
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalCacheDmaFree (void *ptr);

#define IX_OSAL_CACHE_DMA_FREE(ptr)		ixOsalCacheDmaFree(ptr)



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
 * @li Reentrant: no
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
 * @li Reentrant: no
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
 * @li Reentrant: no
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
 * @param arg - argument given to the thread
 *
 * Creates a thread given a thread handle and a thread attribute object. The
 * same thread attribute object can be used to create separate threads. "NULL"
 * can be specified as the attribute, in which case the default values will
 * be used. The thread needs to be explicitly started using ixOsalThreadStart().
 *
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalThreadStart (IxOsalThread * thread);

/**
 * @ingroup IxOsal
 *
 * @brief Kills an existing thread
 *
 * @param thread - handle of the thread to be killed
 *
 * Kills a thread given its thread handle.
 *
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - This function never returns
 */
PUBLIC void ixOsalThreadExit (void);

/**
 * @ingroup IxOsal
 *
 * @brief Sets the priority of an existing thread
 *
 * @param thread - handle of the thread
 * @param priority - new priority, between 0 and 255 (0 being the highest)
 *
 * Sets the thread priority
 *
 * @li Reentrant: no
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
 * Suspends the thread execution
 *
 * @li Reentrant: no
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
 * @li Reentrant: no
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
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMessageQueueCreate (IxOsalMessageQueue * queue,
					   UINT32 msgCount, UINT32 msgLen);

/**
 * @ingroup IxOsal
 *
 * @brief Deletes a message queue
 *
 * @param queue - queue handle
 *
 * Deletes a message queue
 *
 * @li Reentrant: no
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
 * @param queue - queue handle
 * @param message - message to send
 *
 * Sends a message to the message queue. The message will be copied (at the
 * configured size of the message) into the queue.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
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
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMessageQueueReceive (IxOsalMessageQueue * queue,
					    UINT8 * message);


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
 *
 * @li Reentrant: no
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
 * @li IRQ safe:  yes
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
 * @param
 *
 * Destroys a mutex object; the caller should ensure that no thread is
 * blocked on this mutex
 *
 * @li Reentrant: no
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
 * @li IRQ safe:  yes
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
 * @li IRQ safe:  yes
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
 *
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  yes
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
 * @li Reentrant: no
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
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
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
 * blocked on this semaphore
 *
 * @li Reentrant: no
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
 * @li IRQ safe:  no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSpinLockTry(IxOsalSpinLock *slock);

/**
 * @ingroup IxOsal
 *
 * @brief Destroy the spin lock object 
 *
 * @param slock - Spinlock handle
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalSpinLockDestroy(IxOsalSpinLock *slock);

#endif /* ENABLE_SPINLOCK */


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
 * @li Reentrant: no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalSleep (UINT32 milliseconds);

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
 * @li IRQ safe:  yes
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
 * @li Reentrant: no
 * @li IRQ safe:  no
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
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - none
 *
 * @note The implementation of this function is platform-specific. Not all
 * platforms have a real-time clock.
 */
PUBLIC void ixOsalTimeGet (IxOsalTimeval * tv);



/* Internal function to convert timer val to ticks.
 * NOTE - This should not be called by the user.
 * Use the macro IX_OSAL_TIMEVAL_TO_TICKS
 * OS-independent, implemented in framework.
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



/* Internal function to convert ticks to timer val
 * NOTE - This should not be called by the user.
 * Use the macro IX_OSAL_TICKS_TO_TIMEVAL
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
#define IX_OSAL_TIMEVAL_TO_MS(tv)     ((tv.secs * 1000) + (tv.nsecs / 1000000))


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
   	    ((IxOsalTimeval *) pTv)->secs = milliseconds / 1000;			  \
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
        ((tvA).secs  < (tvB).secs ||	\
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
        ((tvA).secs  > (tvB).secs ||	\
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


/* ============================= Logging  ==============================
 *
 */

/**
 * @ingroup IxOsal
 *
 * @brief Interrupt-safe logging function
 *
 * @param level - identifier prefix for the message
 * @param device - output device
 * @param format - message format, in a printf format
 * @param ... - up to 6 arguments to be printed
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
PUBLIC IX_STATUS
ixOsalSingleShotTimerSchedule (IxOsalTimer * timer,
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
 * @li Reentrant: no
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
 * @li Reentrant: no
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
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param bus	-	The returned bus or NULL if the bus is not needed.
 * @param slot	-	The returned slot or NULL if the bus is not needed
 * @param func 	-	The returned func or NULL if the bus is not needed.
 *
 *  This function will return the bus slot and function for an IxOsalPciDev
 *  previously obtained from ixOsalPciDeviceFind(). Any of the bus/slot/func
 *  parameters may be null if the information is not needed.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param offset     - 	Offset from the base of the configuration space.
 * @param val	     -  Byte value fetched from the mentioned location
 *
 *  This function retrieves a byte of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param offset     - 	Offset from the base of the configuration space.
 * @param val	     -  Word value fetched from the mentioned location
 *
 *  This function retrieves a word of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param offset     - 	Offset from the base of the configuration space.
 * @param val	     -  Double word value fetched from the mentioned location
 *
 *  This function retrieves a double word of information, starting at the specified
 *  offset, from the PCI configuration space on a particular PCI device.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param offset     - 	Offset from the base of the configuration space.
 * @param val	     -  Byte value to be set at the mentioned location
 *
 *  This function sets a byte of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param offset     - 	Offset from the base of the configuration space.
 * @param val	     -  Word value to be set at the mentioned location
 *
 *  This function sets a word of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @param offset     - 	Offset from the base of the configuration space.
 * @param val	     -  Double word value to be set at the mentioned location
 *
 *  This function sets a double word of data, starting at the specified offset, to
 *  the PCI configuration space for a particular PCI device.
 *
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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
 * @li Reentrant: XXX
 * @li IRQ safe:  YYY
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


#ifdef IX_OSAL_OSSL_SHIMLAYER_SUPPORT

/* New OSAL Functions for supporting OSSL-SHIM */

PUBLIC
IX_STATUS ixOsalThreadGetId(IxOsalThread *ptrTid);

PUBLIC
IX_STATUS ixOsalThreadSetPolicyAndPriority(
         	IxOsalThread *tid,
            UINT32 		  policy,
            UINT32 		  priority);


PUBLIC
IX_STATUS _ixOsalSemaphoreDownTimeout(
          	  IxOsalSemaphore *sid,
              UINT32           timeout);

PUBLIC
IX_STATUS ixOsalSemaphoreWaitInterruptible(
              IxOsalSemaphore *sid,
              UINT32           timeout);

PUBLIC
IX_STATUS ixOsalSemaphorePostWakeup(IxOsalSemaphore *sid);

PUBLIC
IX_STATUS ixOsalSemaphoreFlush(IxOsalSemaphore *sid);

PUBLIC
IX_STATUS ixOsalSleepTick(UINT32 sleeptime_ticks);

PUBLIC
IX_STATUS ixOsalSleepUninterruptible(
             UINT32 sleeptime_ms);

#endif /* IX_OSAL_OSSL_SHIMLAYER_SUPPORT */

#endif /* IxOsal_H */
