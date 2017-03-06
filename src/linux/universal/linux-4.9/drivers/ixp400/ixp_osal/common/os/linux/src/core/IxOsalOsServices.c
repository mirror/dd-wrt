/**
 * @file IxOsalOsServices.c (linux)
 *
 * @brief Implementation for Mem and Sleep.
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

#include <linux/version.h>


#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include <linux/mm.h>
#include <linux/mempool.h>
#include <linux/vmalloc.h>
/* #include <asm/highmem.h> */
/* More generic include */
#include <linux/highmem.h>
#include <asm/pgtable.h>

#include "IxOsal.h"

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


/* by default trace all but debug message */
PRIVATE unsigned int ixOsalCurrLogLevel = IX_OSAL_LOG_LVL_MESSAGE;



#define IS_VMALLOC_ADDR(addr) (((UINT32)(addr) >= VMALLOC_START) && ((UINT32)(addr) < VMALLOC_END))


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
            ("ixOsalLog: only IX_OSAL_LOG_DEV_STDOUT and IX_OSAL_LOG_DEV_STDERR are supported \n");
        return (IX_OSAL_LOG_ERROR);
    }

    if (level <= ixOsalCurrLogLevel && level != IX_OSAL_LOG_LVL_NONE)
    {
        int headerByteCount =
            (level ==
            IX_OSAL_LOG_LVL_USER) ? 0 : printk (traceHeaders[level - 1]);

        return headerByteCount + printk (format, arg1, arg2, arg3, arg4, arg5,
            arg6);
    }
    else
    {
        /*
         * Return error
         */
        return (IX_OSAL_LOG_ERROR);
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

PUBLIC void
ixOsalSleep (UINT32 milliseconds)
{
    if (milliseconds != 0)
    {
        set_current_state(TASK_INTERRUPTIBLE);
        schedule_timeout ((milliseconds * HZ) / 1000);
    }
    else
    {
        schedule ();
    }
}

/**************************************
 * Memory functions
 *************************************/

void *
ixOsalMemAlloc (UINT32 size)
{
    if(size > (1024 * 128))
        return(vmalloc(size));

    return (kmalloc (size, GFP_KERNEL));
}

void
ixOsalMemFree (void *ptr)
{
    IX_OSAL_ASSERT (ptr != NULL);
    
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
ixOsalMemCopy (void *dest, void *src, UINT32 count)
{
    IX_OSAL_ASSERT (dest != NULL);
    IX_OSAL_ASSERT (src != NULL);
    return (memcpy (dest, src, count));
}

/*
 * Fills a memory zone with a given constant byte,
 * returns pointer to the memory zone.
 */
void *
ixOsalMemSet (void *ptr, UINT8 filler, UINT32 count)
{
    IX_OSAL_ASSERT (ptr != NULL);
    return (memset (ptr, filler, count));
}

/*****************************
 *
 *  Time
 *
 *****************************/

/* Retrieve current system time */
void
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
    ptime->nsecs *= 1000;

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
	ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
		   IX_OSAL_LOG_DEV_STDOUT,
		   "ixOsalOsVersionGet: invalid input parameters\n",
		   0, 0, 0, 0, 0, 0);
	return status;
    }

    status = IX_OSAL_OEM_OS_VERSION_GET(osVersion, maxSize);

    return status;
}

#ifdef IX_OSAL_OSSL_SHIMLAYER_SUPPORT
/* Newly added OSAL Functions */

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
        set_current_state(TASK_UNINTERRUPTIBLE);
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

    value.tv_sec  = sleeptime_ms/1000;
    value.tv_nsec = (sleeptime_ms%1000) * 1000000;

    sleeptime_ms = timespec_to_jiffies(&value);

    {
        struct task_struct* aTask = current;
        aTask->state = TASK_UNINTERRUPTIBLE;
        sleeptime_ms = schedule_timeout(sleeptime_ms);
    }

    return IX_SUCCESS;

} /* ixOsalSleepUninterruptible */
#endif /* IX_OSAL_OSSL_SHIMLAYER_SUPPORT */
