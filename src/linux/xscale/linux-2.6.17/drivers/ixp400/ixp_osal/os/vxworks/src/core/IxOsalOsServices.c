/**
 * @file IxOsalOsServices.c (vxWorks)
 *
 * @brief Implementation for Irq, Mem, sleep. 
 * 
 * 
 * @par
 * IXP400 SW Release version  2.1
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright (c) 2002-2005 Intel Corporation All Rights Reserved.
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

#include <vxWorks.h>
#include <iv.h>
#include <intLib.h>
#include <sysLib.h>
#include <logLib.h>
#include <taskLib.h>
#include <string.h>

#include "IxOsal.h"

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

/* by default log level is log_message */
PRIVATE int IxOsalOsServicesLogLevel = IX_OSAL_LOG_LVL_MESSAGE;

/* dummy ISR to register during unBind */
PRIVATE void ixOsalDummyIsr (void *parameter);
PRIVATE void
ixOsalDummyIsr (void *parameter)
{
}

/**************************************
 * Irq services 
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 irqLevel, IxOsalVoidFnVoidPtr routine, void *parameter)
{

#if CPU==SIMSPARCSOLARIS
    /*
     * No Irq support in simulation env 
     */
    return IX_FAIL;
#else
    if (intConnect
        ((IxOsalVoidFnVoidPtr *)
            irqLevel, (IxOsalVoidFnVoidPtr) routine, (int) parameter) != OK)
    {
        return IX_FAIL;
    }
    intEnable (irqLevel);
    return IX_SUCCESS;

#endif

}

PUBLIC IX_STATUS
ixOsalIrqUnbind (UINT32 irqLevel)
{
    /*
     * disable interrupts for this vector 
     */
    if (intDisable (IVEC_TO_INUM (irqLevel)) != OK)
    {
        return IX_FAIL;
    }

    /*
     * register a dummy ISR 
     */
    if (intConnect ((IxOsalVoidFnVoidPtr
                *) irqLevel, (IxOsalVoidFnVoidPtr) ixOsalDummyIsr, 0) != OK)
    {
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

PUBLIC UINT32
ixOsalIrqLock ()
{
#if CPU==SIMSPARCSOLARIS

    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqLock: not supported in simulation  \n", 0, 0, 0, 0, 0, 0);

    return 0;

#else
    return intLock ();
#endif

}

/* Enable interrupts and task scheduling,
 * input parameter: irqEnable status returned
 * by ixOsalIrqLock().
 */
PUBLIC void
ixOsalIrqUnlock (UINT32 irqEnable)
{

#if CPU==SIMSPARCSOLARIS
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqUnlock: not supported in simulation  \n", 0, 0, 0, 0, 0, 0);
#else
    intUnlock (irqEnable);
#endif

}

PUBLIC void
ixOsalIrqEnable (UINT32 irqLevel)
{
    intEnable (irqLevel);
}

PUBLIC void
ixOsalIrqDisable (UINT32 irqLevel)
{
    intDisable (irqLevel);
}

/** 
 * Return previous int level, return IX_FAIL if fail.
 * Interrupt are locked out at or below that level.
 */
PUBLIC UINT32
ixOsalIrqLevelSet (UINT32 irqLevel)
{

#if CPU==SIMSPARCSOLARIS
    /*
     * No irq support in simulation, log error and return any 
     * * number, in this case we return zero.
     */
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalIrqLevelSet: not supported in simulation  \n",
        0, 0, 0, 0, 0, 0);
    return 0;
#else
    return intLevelSet (irqLevel);
#endif
}

/**************************************
 * Log services 
 *************************************/
INT32
ixOsalLog (IxOsalLogLevel level,
    IxOsalLogDevice device,
    char *format, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6)
{
    /*
     * Return -1 for custom display devices 
     */
    if ((device !=
            IX_OSAL_LOG_DEV_STDOUT) && (device != IX_OSAL_LOG_DEV_STDERR))
    {
        logMsg
            ("ixOsalLog: only IX_OSAL_LOG_DEV_STDOUT and IX_OSAL_DEV_STD_ERR are supported \n",
            0, 0, 0, 0, 0, 0);
        return (IX_OSAL_LOG_ERROR);
    }
    if (level <= IxOsalOsServicesLogLevel && level != IX_OSAL_LOG_LVL_NONE)
    {
        int headerByteCount =
            (level ==
            IX_OSAL_LOG_LVL_USER) ? 0 : logMsg (traceHeaders[level - 1], 0,
            0, 0, 0, 0, 0);
        return headerByteCount +
            logMsg (format, arg1, arg2, arg3, arg4, arg5, arg6);
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
    UINT32 oldLevel = IxOsalOsServicesLogLevel;

    IxOsalOsServicesLogLevel = level;

    return oldLevel;
}

/**************************************
 * Memory functions 
 *************************************/

/* 
 * Allocates a memory zone of a given size 
 */
void *
ixOsalMemAlloc (UINT32 size)
{
    void *mem_ptr;
    mem_ptr = malloc (size);
    return mem_ptr;
}

/* 
 * Frees a previously allocated memory zone 
 */
void
ixOsalMemFree (void *ptr)
{
    free (ptr);
}

/* 
 * Copy count bytes from src to dest ,
 * returns pointer to the dest mem zone.
 */
void *
ixOsalMemCopy (void *dest, void *src, UINT32 count)
{
    void *copy_result;
    copy_result = memcpy (dest, src, count);
    return copy_result;
}

/* 
 * Fills a memory zone with a given constant byte,
 * returns pointer to the memory zone.
 */
void *
ixOsalMemSet (void *ptr, UINT8 filter, UINT32 count)
{
    void *retPtr;
    retPtr = memset (ptr, filter, count);
    return (retPtr);
}

/**************************************
 * Task services 
 *************************************/

PUBLIC void
ixOsalBusySleep (UINT32 microseconds)
{

    UINT32 delta;
    UINT32 lastTimestamp, currentTimestamp;
    UINT32 delay = 0;

    lastTimestamp = ixOsalTimestampGet ();

    while (delay < (UINT32) microseconds * IX_OSAL_OEM_APB_CLOCK)
    {
        currentTimestamp = ixOsalTimestampGet ();
        delta =
            currentTimestamp >
            lastTimestamp ?
            currentTimestamp -
            lastTimestamp : 0xffffffff - lastTimestamp + currentTimestamp;

        delay += delta;

        lastTimestamp = currentTimestamp;
    }
}

PUBLIC void
ixOsalSleep (UINT32 milliseconds)
{
    UINT32 delay = (ixOsalSysClockRateGet () * milliseconds) / 1000;

    /*
     * Cover a rounding down to zero 
     */
    if (delay == 0 && milliseconds != 0)
        ++delay;

    taskDelay (delay);
}

PUBLIC UINT32
ixOsalTimestampGet (void)
{
#if CPU==SIMSPARCSOLARIS

    /*
     * no timestamp under VxSim 
     */
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalTimestampGet: not supported in simulation  \n",
        0, 0, 0, 0, 0, 0);
    return 0;

#else
    /*
     * Call OEM timestampGet 
     */
    return IX_OSAL_OEM_TIMESTAMP_GET ();

#endif
}

PUBLIC UINT32
ixOsalTimestampResolutionGet (void)
{
    return IX_OSAL_OEM_TIMESTAMP_RESOLUTION_GET ();
}

/* Retrieve current system time */
void
ixOsalTimeGet (IxOsalTimeval * tv)
{

    int retVal;

    retVal = clock_gettime (CLOCK_REALTIME, (struct timespec *) tv);

    if (retVal != OK)
    {
        ixOsalLog
            (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimeGet(): ERROR  \n", 0, 0, 0, 0, 0, 0);
    }
}

/* Retrieve number of ticks per second */
UINT32
ixOsalSysClockRateGet (void)
{
    return ((UINT32) sysClkRateGet ());
}

PUBLIC void
ixOsalYield (void)
{
    taskDelay (0);
}

PUBLIC IX_STATUS
ixOsalOsNameGet(INT8* osName, INT32 maxSize)
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

#if !defined __XSCALE__
IX_STATUS ixOsalOemFastMutexTryLock (IxOsalFastMutex * mutex)
{
    logMsg
	("ixOsalLog: ixOsalFastMutexTryLock is only supported on XScale! \n",
	 0, 0, 0, 0, 0, 0);
    return (IX_FAIL);
}

#endif /* !defined __XSCALE__ */
