/***************************************************************************
 *
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
 *
 ***************************************************************************/

/**
 *****************************************************************************
 * @file cpa_sample_utils.h
 *
 * @defgroup sampleUtils Macro and inline function definitions
 *
 * @ingroup sampleCode
 *
 * @description
 * Defines macros for printing and debugging and inline functions for memory
 * allocating and freeing
 *
 ***************************************************************************/

#if defined(__linux)
    #include <linux/init.h>
    #include <linux/module.h>
    #include <linux/moduleparam.h>
#elif defined(__freebsd)
    #include <sys/cdefs.h>
    #include <sys/param.h>
    #include <sys/kernel.h>
    #include <sys/module.h>
    #include <sys/types.h>
    #include <sys/systm.h>
    #include <sys/conf.h>
    #include <sys/sema.h>
    #include <sys/malloc.h>
#endif

#include "cpa.h"
#include "cpa_cy_im.h"


#if defined(__linux)
/**< Prints the name of the function and the arguments only if debugParam is 
 * TRUE.
 */
    #define PRINT_DBG(args...)          \
    do {                                \
        if (TRUE == debugParam) {       \
            printk("%s(): ", __func__); \
            printk(args);               \
        }                               \
    } while (0)

/**< Prints the name of the function and the arguments */
    #define PRINT_ERR(args...)          \
    do {                                \
            printk("%s(): ", __func__); \
            printk(args);               \
    } while (0)

#elif defined(__freebsd)
/**< Prints the name of the function and the arguments only if debugParam is 
 * TRUE.
 */
    #define PRINT_DBG(args...)          \
    do {                                \
        if (TRUE == debugParam) {       \
            printf("%s(): ", __func__); \
            printf(args);               \
        }                               \
    } while (0)

/**< Prints the name of the function and the arguments */
    #define PRINT_ERR(args...)          \
    do {                                \
            printf("%s(): ", __func__); \
            printf(args);               \
    } while (0)
#endif

#if defined(__freebsd)
/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      This function allocates memory in FreeBSD
 *
 * @param[in] sizeBytes     the size of the memory to be allocated
 *
 * @retval void*            pointer to memory that has been allocated
 *
 ******************************************************************************/
void * 
freebsdMemAlloc (Cpa32U sizeBytes);

/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      This function frees memory in FreeBSD
 *
 * @param[in] *ptr     pointer to the memory to be free'd
 *
 * @retval void
 *
 ******************************************************************************/
void 
freebsdMemFree (void *ptr);
#endif  

/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      This function and associated macro allocates the memory for the given 
 *      size and stores the address of the memory allocated in the pointer.
 *
 * @param[out] ppMemAddr    address of pointer where address will be stored
 * @param[in] sizeBytes     the size of the memory to be allocated
 *
 * @retval CPA_STATUS_RESOURCE  Macro failed to allocate Memory
 * @retval CPA_STATUS_SUCCESS   Macro executed successfully
 *
 ******************************************************************************/
static __inline CpaStatus
Mem_OsMemAlloc(void **ppMemAddr,
                  Cpa32U sizeBytes)
{
#if defined(__linux)
    *ppMemAddr = kmalloc(sizeBytes, GFP_ATOMIC);
#elif defined(__freebsd)
    *ppMemAddr = freebsdMemAlloc(sizeBytes);
#endif
    if (NULL == *ppMemAddr)
    {
        return CPA_STATUS_RESOURCE;
    }

    return CPA_STATUS_SUCCESS;
}

/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      Macro from the Mem_OsMemAlloc function
 *
 ******************************************************************************/
#define OS_MALLOC(ppMemAddr, sizeBytes) \
    Mem_OsMemAlloc((void *)(ppMemAddr), (sizeBytes))

/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      This function and associated macro frees the memory at the given address
 *      and resets the pointer to NULL
 *
 * @param[out] ppMemAddr    address of pointer where mem address is stored.
 *                          If pointer is NULL, the function will exit silently
 *
 * @retval void
 *
 ******************************************************************************/
static __inline void
Mem_OsMemFree(void **ppMemAddr)
{
    if (NULL != *ppMemAddr)
    {
#if defined(__linux)
        kfree(*ppMemAddr);
#elif defined(__freebsd)
        freebsdMemFree(*ppMemAddr);
#endif
        *ppMemAddr = NULL;
    }
}

/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      Macro from the Mem_OsMemFree function
 *
 ******************************************************************************/
#define OS_FREE(pMemAddr) \
    Mem_OsMemFree((void *)&pMemAddr)

/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      Completion definitions
 *
 ******************************************************************************/
#if defined(__linux)
    #define COMPLETION_STRUCT completion

    #define COMPLETION_INIT(c) init_completion(c)

    #define COMPLETION_WAIT(c, timeout)                         \
        wait_for_completion_interruptible_timeout(c, timeout)

    #define COMPLETE(c) complete(c)

    #define COMPLETION_DESTROY(s)

#elif defined(__freebsd)
    /* Use semaphores to signal completion of events */
    #define COMPLETION_STRUCT sema

    #define COMPLETION_INIT(s) sema_init(s, 0, "");

    #define COMPLETION_WAIT(s, timeout) (sema_timedwait(s, timeout) == 0)

    #define COMPLETE(s) sema_post(s)

    #define COMPLETION_DESTROY(s) sema_destroy(s)
#endif


/**
 *******************************************************************************
 * @ingroup sampleUtils
 *      FreeBSD memory allocation defines
 *
 ******************************************************************************/
#if defined(__freebsd)

#define MAX_KMALLOC_MEM      (1024 * 128)  
#define MAX_BOUNDARY     (4 * 1024 * 1024)
#define NUMBER_OF_SECONDS_IN_A_DAY 86400

#define MEM_PADDING(s, a) ( ( a - (s % a)) % a  )

typedef enum E_ALLOC_TYPE
{
    MALLOC          = 1,    /*memory allocated using malloc */
    CONTIGMALLOC    = 2     /*memory allocated using contigmalloc */
    
} ALLOC_TYPE;

typedef struct _sMemAllocInfo
{
    void*       mAllocMemPtr;   /* memory addr returned by the kernel */
    Cpa32U      mSize;          /* allocated size */
    ALLOC_TYPE  mAllocType;
    
} memAllocInfoStruct;

#endif

