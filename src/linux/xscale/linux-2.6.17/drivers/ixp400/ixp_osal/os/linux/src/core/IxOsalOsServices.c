/**
 * @file IxOsalOsServices.c (linux)
 *
 * @brief Implementation for Irq, Mem, sleep. 
 * 
 * 
 * @par
 * IXP400 SW Release Crypto version 2.1
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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

#include "IxOsal.h"

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
#include <linux/hardirq.h>
#include <linux/interrupt.h>
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */

#include <linux/delay.h>
#include <asm/hardirq.h>
#include <asm/system.h>
#include <asm/arch/irqs.h>
#include <asm/irq.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/slab.h>

#ifndef __ixp46X
#include "IxOsalOsIxp425Irq.h"
#else
#include "IxOsalOsIxp465Irq.h"
#endif

/* 
 * Note: being referenced by some release 1.4 linux 
 * components as global .
 */
PUBLIC UINT32 ixOsalLinuxInterruptedPc;

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
PRIVATE IxOsalLogLevel ixOsalCurrLogLevel = IX_OSAL_LOG_LVL_MESSAGE;

typedef struct IxOsalInfoType
{
    void (*routine) (void *);
    void *parameter;
} IxOsalInfoType;

static IxOsalInfoType IxOsalInfo[NR_IRQS];

/*
 * General interrupt handler
 */

/* 
 * Private utility function for ixOsalIrqBind to translate IRQ vector number to
 * IRQ name.
 */
PRIVATE char*
ixOsalGetIrqNameByVector(UINT32 vector)
{
    if (unlikely (ARRAY_SIZE(irq_name) < vector))
	return ((char*)invalid_irq_name);

    return ((char*)irq_name[vector]);
}

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
static irqreturn_t
#else
static void
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
ixOsalOsIsrProxy (int irq, void *dev_id, struct pt_regs *regs)
{
    IxOsalInfoType *isr_proxy_info = (IxOsalInfoType *) dev_id;

    IX_OSAL_ENSURE(isr_proxy_info && isr_proxy_info->routine,
		   "ixOsalOsIsrProxy: Interrupt used before ixOsalIrqBind was invoked");

    isr_proxy_info->routine (isr_proxy_info->parameter);

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    return IRQ_HANDLED;
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
}

/*
 * Interrupt handler for XScale PMU interrupts
 * This handler saves the interrupted Program Counter (PC)
 * into a global variable
 */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
static irqreturn_t
#else
static void
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
ixOsalOsIsrProxyWithPC (int irq, void *dev_id, struct pt_regs *regs)
{
    ixOsalLinuxInterruptedPc = regs->ARM_pc;
    ixOsalOsIsrProxy(irq, dev_id, regs);

#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    return IRQ_HANDLED;
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
}

/**************************************
 * Irq services 
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 vector, IxOsalVoidFnVoidPtr routine, void *parameter)
{
    if (IxOsalInfo[vector].routine)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqBind: NULL function routine. \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    IxOsalInfo[vector].routine = routine;
    IxOsalInfo[vector].parameter = parameter;

    /*
     * The PMU interrupt handler is a special case in the sense
     * that it needs to save the address of the interrupted PC
     * In the case of the XScale PMU interrupt, the ixOsalOsIsrProxyWithPC
     * function is registered
     */
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    if (vector == IRQ_IXP4XX_XSCALE_PMU)
#else
    if (vector == IRQ_IXP425_XSCALE_PMU)
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
    {
	/*
	 * request_irq will enable interrupt automatically 
	 * A non-zero return value suggest a failure.
	 */
	if (request_irq (vector, ixOsalOsIsrProxyWithPC, SA_SHIRQ, 
			ixOsalGetIrqNameByVector(vector),
			 &IxOsalInfo[vector]))
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
		       IX_OSAL_LOG_DEV_STDOUT,
		       "ixOsalIrqBind: Fail to request irq. \n",
		       0, 0, 0, 0, 0, 0);
	    return IX_FAIL;
	}
    }
    else
    {
	/*
	 * request_irq will enable interrupt automatically 
	 * A non-zero return value suggest a failure.
	 */
	if (request_irq (vector, ixOsalOsIsrProxy, SA_SHIRQ,
			ixOsalGetIrqNameByVector(vector),
			 &IxOsalInfo[vector]))
	{
	    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
		       IX_OSAL_LOG_DEV_STDOUT,
		       "ixOsalIrqBind: Fail to request irq. \n",
		       0, 0, 0, 0, 0, 0);
	    return IX_FAIL;
	}
    }
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalIrqUnbind (UINT32 vector)
{
    if (!IxOsalInfo[vector].routine)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqUnbind: NULL function routine. \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    free_irq (vector, &IxOsalInfo[vector]);
    IxOsalInfo[vector].routine = NULL;

    return IX_SUCCESS;
}

PUBLIC UINT32
ixOsalIrqLock ()
{
    UINT32 flags;
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    /* local_irq_save() is not used here due to compilation warning against s32
     * data type. */
    local_save_flags(flags);
    local_irq_disable();
#else
    save_flags (flags);
    cli ();
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
    return flags;
}

/* Enable interrupts and task scheduling,
 * input parameter: irqEnable status returned
 * by ixOsalIrqLock().
 */
PUBLIC void
ixOsalIrqUnlock (UINT32 lockKey)
{
#ifdef IX_OSAL_OS_LINUX_VERSION_2_6
    local_irq_restore(lockKey);
#else
    restore_flags (lockKey);
#endif /* IX_OSAL_OS_LINUX_VERSION_2_6 */
}

PUBLIC UINT32
ixOsalIrqLevelSet (UINT32 level)
{
    /*
     * Not supported 
     */
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqLevelSet: not supported \n", 0, 0, 0, 0, 0, 0);
    return 0;
}

PUBLIC void
ixOsalIrqEnable (UINT32 irqLevel)
{
    if (irqLevel < NR_IRQS)
    {
        enable_irq(irqLevel);
    }
    else
    {
        /*
         * Not supported 
         */
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqEnable: IRQ %d not supported \n", irqLevel, 0, 0, 0, 0, 0);
    }
}

PUBLIC void
ixOsalIrqDisable (UINT32 irqLevel)
{
    if (irqLevel < NR_IRQS)
    {
        disable_irq(irqLevel);
    }
    else
    {
        /*
         * Not supported 
         */
        ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalIrqDisable: IRQ %d not supported \n", irqLevel, 0, 0, 0, 0, 0);
    }
}

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
    return (kmalloc (size, GFP_KERNEL));
}

void
ixOsalMemFree (void *ptr)
{
    IX_OSAL_ASSERT (ptr != NULL);
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

/* Timestamp is implemented in OEM */
PUBLIC UINT32
ixOsalTimestampGet (void)
{
    return IX_OSAL_OEM_TIMESTAMP_GET ();
}

/* OEM-specific implementation */
PUBLIC UINT32
ixOsalTimestampResolutionGet (void)
{
    return IX_OSAL_OEM_TIMESTAMP_RESOLUTION_GET ();
}

PUBLIC UINT32
ixOsalSysClockRateGet (void)
{
    return IX_OSAL_OEM_SYS_CLOCK_RATE_GET ();
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
