/**
 * @file IxOsalOsServices.c (linux)
 *
 * @brief Implementation for Mem and Sleep.
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
#include "IxOsal.h"
#include "IxOsalOsTypes.h"

#include <linux/version.h>

#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/mm.h>
#include <linux/mempool.h>
#include <linux/vmalloc.h>

/* More generic include */
#include <linux/highmem.h>
#include <asm/pgtable.h>
#include <linux/random.h>

/* Trace Message Logging Levels */

static char *traceHeaders[] = {
    "",
    "[fatal] ",
    "[error] ",
    "[warning] ",
    "[message] ",
    "[debug1] ",
    "[debug2] ",
    "[debug3] ",
    "[all]"
};

static CHAR osalModuleName[OSAL_MAX_MODULE_NAME_LENGTH] = "";

/* by default trace all but debug message */
PRIVATE unsigned int ixOsalCurrLogLevel = IX_OSAL_LOG_LVL_MESSAGE;

#define IS_VMALLOC_ADDR(addr) (((UINT32)(addr) >= VMALLOC_START) && \
        ((UINT32)(addr) < VMALLOC_END))

/* Maximum memory (in bytes) that can be allocated using kmalloc.
   Beyond this, vmalloc is to be used to allcoate memory */
#define IX_OSAL_MAX_KMALLOC_MEM      (1024 * 128)


/*********************
 * Log function
 *********************/

INT32
ixOsalLog (IxOsalLogLevel level,
    IxOsalLogDevice device,
    char *format, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
    /*
     * Return -1 for custom display devices
     */
    if ((device != IX_OSAL_LOG_DEV_STDOUT)
        && (device != IX_OSAL_LOG_DEV_STDERR))
    {
        printk
            ("ixOsalLog: only IX_OSAL_LOG_DEV_STDOUT and \
                       IX_OSAL_LOG_DEV_STDERR are supported \n");
        return (IX_OSAL_LOG_ERROR);
    }

    if (level <= ixOsalCurrLogLevel && level != IX_OSAL_LOG_LVL_NONE)
    {
        INT32 return_val;
        int headerByteCount =
            (level ==
            IX_OSAL_LOG_LVL_USER) ? 0 : printk (traceHeaders[level - 1]);

        if ( OSAL_OS_GET_STRING_LENGTH(((VOID *) osalModuleName)) != 0 )
        {
            headerByteCount +=
                printk("%s :",osalModuleName);
        }
        headerByteCount +=
           printk (format,
           arg1, arg2, arg3, arg4, arg5,arg6);
        return_val =  (INT32)headerByteCount;
        return return_val;
    }
    else
    {
        /*
         * Return zero byte printed
         */
        return (IX_OSAL_NO_LOG);
    }
}

PUBLIC UINT32
ixOsalLogLevelSet (UINT32 level)
{
    UINT32 oldLevel;

    /*
     * Check value first
     */
    if ((level < IX_OSAL_LOG_LVL_NONE) || (level > IX_OSAL_LOG_LVL_ALL))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalLogLevelSet: Log Level is between %d and%d \n",
            IX_OSAL_LOG_LVL_NONE, IX_OSAL_LOG_LVL_ALL, 0, 0, 0, 0);
        return IX_OSAL_LOG_LVL_NONE;
    }
    oldLevel = ixOsalCurrLogLevel;

    ixOsalCurrLogLevel = level;

    return oldLevel;
}

/**************************************
 * Task services
 *************************************/

PUBLIC void
ixOsalBusySleep (UINT32 microseconds)
{
    udelay (microseconds);
}

PUBLIC IX_STATUS
ixOsalSleep (UINT32 milliseconds)
{
    if (milliseconds != 0)
    {
        set_current_state((long)TASK_INTERRUPTIBLE);
        schedule_timeout ((milliseconds * HZ) / IX_OSAL_THOUSAND);
    }
    else
    {
        schedule ();
    }
    return IX_SUCCESS;
}

/**************************************
 * Memory functions
 *************************************/

void *
ixOsalMemAlloc (UINT32 memsize)
{
    if(memsize > IX_OSAL_MAX_KMALLOC_MEM)
    {
        return ( vmalloc(memsize) );
    }

    return (kmalloc (memsize, GFP_KERNEL));
}

void
ixOsalMemFree (void *ptr)
{
    IX_OSAL_MEM_ASSERT (ptr != NULL);

    if(IS_VMALLOC_ADDR(ptr))
     {
        vfree(ptr);
        return;
    }

    kfree (ptr);
}

/*
 * Copy count bytes from src to dest ,
 * returns pointer to the dest mem zone.
 */
void *
ixOsalMemCopy (void *dest, const void *src, UINT32 count)
{
    IX_OSAL_MEM_ASSERT (dest != NULL);
    IX_OSAL_MEM_ASSERT (src != NULL);
    return (memcpy (dest, src, count));
}

/*
 * Fills a memory zone with a given constant byte,
 * returns pointer to the memory zone.
 */
void *
ixOsalMemSet (void *ptr, UINT8 filler, UINT32 count)
{
    IX_OSAL_MEM_ASSERT (ptr != NULL);
    return (memset (ptr, filler, count));
}

/**
 * Compares count bytes from src and dest
 * returns IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalMemCmp (void *dest, void *src, UINT32 count)
{
    IX_STATUS status = IX_FAIL;
    IX_OSAL_MEM_ASSERT (dest != NULL);
    IX_OSAL_MEM_ASSERT (src != NULL);
    if( memcmp (dest, src, count) ==0)
    {
         status=IX_SUCCESS;
     }
    return status;
}

/*****************************
 *
 *  Time
 *
 *****************************/

/* Retrieve current system time */
IX_STATUS
ixOsalTimeGet (IxOsalTimeval * ptime)
{
    /*
     * linux struct timeval has subfields:
     * -- time_t   (type long, second)
     * -- suseconds_t ( type long, usecond)
     */
    do_gettimeofday ((struct timeval *) ptime);

    /*
     * Translate microsecond to nanosecond,
     * second field is identical so no translation
     * there.
     */
    ptime->nsecs *= IX_OSAL_THOUSAND;
    return IX_SUCCESS;
}


PUBLIC void
ixOsalYield (void)
{
    schedule ();
}

PUBLIC IX_STATUS
ixOsalOsNameGet (INT8* osName, INT32 maxSize)
{
    IX_STATUS status = IX_FAIL;

    /* Ensure that the input parameters are valid */
    if (osName == NULL || maxSize <= 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                   IX_OSAL_LOG_DEV_STDOUT,
                   "ixOsalOsNameGet: invalid input parameters\n",
                   0, 0, 0, 0, 0, 0);

        return status;
    }

    status = IX_OSAL_OEM_OS_NAME_GET(osName, maxSize);

    return status;
}

PUBLIC IX_STATUS
ixOsalOsVersionGet(INT8* osVersion, INT32 maxSize)
{
    IX_STATUS status = IX_FAIL;

    /* Ensure that the input parameters are valid */
    if (osVersion == NULL || maxSize <= 0)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                   "ixOsalOsVersionGet: invalid input parameters\n",
        0, 0, 0, 0, 0, 0);
        return status;
    }

    status = IX_OSAL_OEM_OS_VERSION_GET(osVersion, maxSize);
    return status;
}


PUBLIC
IX_STATUS ixOsalSleepTick(UINT32 sleeptime_ticks)
{
    if(sleeptime_ticks < 1)
    {
        schedule();
        return IX_SUCCESS;
    }
    else
    {
        set_current_state((long)TASK_UNINTERRUPTIBLE);
        schedule_timeout(sleeptime_ticks);
    }

    return IX_SUCCESS;
} /* ixOsalSleepTick */


PUBLIC
IX_STATUS ixOsalSleepUninterruptible(
             UINT32 sleeptime_ms)
{
    struct timespec value;

    if(sleeptime_ms < 1)
    {
        schedule();
        return IX_SUCCESS;
    }

    value.tv_sec  = sleeptime_ms/IX_OSAL_THOUSAND;
    {
     INT64 ll_sleeptime;
     ll_sleeptime = (sleeptime_ms % IX_OSAL_THOUSAND) * IX_OSAL_MILLION;
     ixOsalMemCopy(&value.tv_nsec, &ll_sleeptime, sizeof (value.tv_nsec));
    }
    /*Block added to remove parasoft issue*/
    {
     unsigned long l_sleep_time = timespec_to_jiffies(&value);
     ixOsalMemCopy(&sleeptime_ms, &l_sleep_time, sizeof (UINT32));
    }
    {
        struct task_struct* aTask = current;
        aTask->state = (long)TASK_UNINTERRUPTIBLE;
        schedule_timeout(sleeptime_ms);
    }
    return IX_SUCCESS;
} /* ixOsalSleepUninterruptible */

/*
 *  The function allocate a chunck of memory bigger that the required size
 *  It then calculate the aligned ptr that should be returned to the user.
 *  In the memory just above the returned chunck, the funcion stores the
 *  structure with the memory information
 *
 *  +---+-------------------------+------------------------------- +---+
 *  |xxx|ixOsalMemAllocInfoStruct | memory returned to user (size) |xxx|
 *  +---+-------------------------+--------------------------------+---+
 *  ^                             ^
 *  mAllocMemPtr                  Ptr returned to the caller of MemAlloc
 *
 */
PUBLIC VOID *
ixOsalMemAllocAligned(UINT32 space, UINT32 size, UINT32 alignment)
{

    VOID* ptr = NULL;
    UINT32 toPadSize = 0;
    UINT32 padding = 0;
    VOID* pRet = NULL;
    UINT32 alignment_offset = 0;

    ixOsalMemAllocInfoStruct memInfo = {0};

    if (size == 0 || alignment < 1)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "[ixOsalMemAllocAligned] size or alignment are zero \n", 
            size, alignment, 0, 0, 0, 0);
        return NULL;
    }

    if (alignment & (alignment-1))
    {
         ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "[ixOsalMemAllocAligned] Expecting alignment of a power "
         "of two but did not get one\n", 0, 0, 0, 0, 0, 0);
        return NULL;  
    }

    if (1 == alignment)
    {
        toPadSize = sizeof(ixOsalMemAllocInfoStruct);
        padding = 0;
    }
    else if (sizeof(ixOsalMemAllocInfoStruct) > alignment)
    {
        toPadSize = sizeof(ixOsalMemAllocInfoStruct) + alignment;
        padding = IX_OSAL_MEM_PADDING(toPadSize, alignment);
    }
    else
    {
        toPadSize = alignment;
        padding = 0;
    }

    memInfo.mSize = size + toPadSize + padding;

    if (memInfo.mSize > IX_OSAL_MAX_KMALLOC_MEM)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "[ixOsalMemAllocAligned] Total size needed for this "
        "set of size and alignment (%ld) exceeds the OS "
        "limit %ld\n", memInfo.mSize, IX_OSAL_MAX_KMALLOC_MEM, 
        0, 0, 0, 0);


        return NULL;
    }

    ptr = kmalloc (memInfo.mSize, GFP_KERNEL);

    if (ptr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "[ixOsalMemAllocAligned] memory allocation "
        "failed %ld/%d\n",
        size, alignment, 0, 0, 0, 0);

        return NULL;
    }


    alignment_offset = (UINT32)ptr % alignment;
    memInfo.mAllocMemPtr = ptr;

    pRet = (VOID*) ((CHAR*)ptr + toPadSize + padding + alignment_offset);
    memcpy((VOID*)((CHAR*)pRet - sizeof(ixOsalMemAllocInfoStruct)),
           (VOID*)(&memInfo),
           sizeof(ixOsalMemAllocInfoStruct));

    return pRet;

}

VOID
ixOsalMemAlignedFree (VOID *ptr, UINT32 size)
{
    ixOsalMemAllocInfoStruct *memInfo = NULL;

    memInfo = (ixOsalMemAllocInfoStruct *)((CHAR *)ptr - 
                                  sizeof(ixOsalMemAllocInfoStruct));

    if (memInfo->mSize == 0 || memInfo->mAllocMemPtr == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "[ixOsalMemAlignedFree] ERROR: Detected the corrupted "
        "data: memory leak!! \n", 0, 0, 0, 0, 0, 0);

        return;
    }

    kfree (memInfo->mAllocMemPtr);

}

PUBLIC  VOID
ixOsalGetRandomNum32(UINT32 *num)
{
    get_random_bytes((void *)num, sizeof (UINT32));
}

PUBLIC VOID
ixOsalLogSetPrefix(CHAR * moduleName)
{
  UINT8 stringLength; 

  if (moduleName == NULL)
  {
    return;
  }

  stringLength = (UINT8)OSAL_OS_GET_STRING_LENGTH(((VOID *) moduleName));

  if (stringLength >= OSAL_MAX_MODULE_NAME_LENGTH)
  {
      stringLength = OSAL_MAX_MODULE_NAME_LENGTH -1 ;
  }

  ixOsalMemCopy(osalModuleName,moduleName,stringLength);
  osalModuleName[stringLength] = '\0';
  return;
}

/**
 * @ingroup IxOsal
 *
 * @brief simple logging function
 *
 * @param arg_pFmtString - message format, in printk format
 * arg1 - argument 1
 * arg2 - argument 2
 * arg3 - argument 3
 * arg4 - argument 4
 * arg5 - argument 5
 * arg6 - argument 6
 * 
 * Logging function, similar to printk. This provides a barebones logging 
 * mechanism for users without differing verbosity levels. This interface
 * is not quaranteed to be IRQ safe.
 *
 * @li Reentrant: yes
 * @li IRQ safe:  no
 */


IX_STATUS ixOsalStdLog(
             const char* arg_pFmtString,
                       ...
                      )
{
    IX_STATUS err = IX_SUCCESS;
    va_list argList;

    va_start(argList, arg_pFmtString);
    if ( OSAL_OS_GET_STRING_LENGTH(((VOID *) osalModuleName)) != 0 )
    {
        printk("%s :",osalModuleName);
    }
    vprintk(arg_pFmtString, argList);
    va_end(argList);

    return err;
        
} /* ixOsalStdLog */

