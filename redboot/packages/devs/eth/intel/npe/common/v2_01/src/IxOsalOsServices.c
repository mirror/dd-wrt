/**
 * @file IxOsalOsServices.c (linux)
 *
 * @brief Implementation for Irq, Mem, sleep. 
 * 
 * 
 * @par
 * IXP400 SW Release version 1.5
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2004 Intel Corporation All Rights Reserved.
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

#include <redboot.h>
#include "IxOsal.h"
#include <IxEthAcc.h>
#include <IxEthDB.h>
#include <IxNpeDl.h>
#include <IxQMgr.h>
#include <IxNpeMh.h>

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
PRIVATE int ixOsalCurrLogLevel = IX_OSAL_LOG_LVL_MESSAGE;

/**************************************
 * Irq services 
 *************************************/

PUBLIC IX_STATUS
ixOsalIrqBind (UINT32 vector, IxOsalVoidFnVoidPtr routine, void *parameter)
{
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalIrqUnbind (UINT32 vector)
{
    return IX_FAIL;
}

PUBLIC UINT32
ixOsalIrqLock ()
{
    int old;
    HAL_DISABLE_INTERRUPTS(old);
    return old;
}

/* Enable interrupts and task scheduling,
 * input parameter: irqEnable status returned
 * by ixOsalIrqLock().
 */
PUBLIC void
ixOsalIrqUnlock (UINT32 lockKey)
{
    HAL_RESTORE_INTERRUPTS(lockKey)
}

PUBLIC UINT32
ixOsalIrqLevelSet (UINT32 level)
{
    return IX_FAIL;   
}

PUBLIC void
ixOsalIrqEnable (UINT32 irqLevel)
{
}

PUBLIC void
ixOsalIrqDisable (UINT32 irqLevel)
{
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
        diag_printf
            ("ixOsalLog: only IX_OSAL_LOG_DEV_STDOUT and IX_OSAL_LOG_DEV_STDERR are supported \n");
        return (IX_OSAL_LOG_ERROR);
    }

    if (level <= ixOsalCurrLogLevel && level != IX_OSAL_LOG_LVL_NONE)
    {
        int headerByteCount =
            (level ==
            IX_OSAL_LOG_LVL_USER) ? 0 : diag_printf(traceHeaders[level - 1]);

        return headerByteCount + diag_printf (format, arg1, arg2, arg3, arg4, arg5,
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
    CYGACC_CALL_IF_DELAY_US(microseconds);
}

PUBLIC void
ixOsalSleep (UINT32 milliseconds)
{
    if (milliseconds != 0) {
#ifdef CYGPKG_REDBOOT
	// We poll while we wait because interrupts are off in RedBoot
	// and CSR expects messages, etc to be dispatched while sleeping.
	int i;
	IxQMgrDispatcherFuncPtr qDispatcherFunc;

	ixQMgrDispatcherLoopGet(&qDispatcherFunc);

	while (milliseconds--) {
	    for (i = 0; i < IX_ETH_ACC_NUMBER_OF_PORTS; i++)
		ixNpeMhMessagesReceive(i);
	    (*qDispatcherFunc)(IX_QMGR_QUELOW_GROUP);
	    CYGACC_CALL_IF_DELAY_US(1000);
	}
#else
        CYGACC_CALL_IF_DELAY_US(milliseconds * 1000);
#endif
    }
}

/**************************************
 * Memory functions 
 *************************************/

void *
ixOsalMemAlloc (UINT32 size)
{
    return (void *)0;
}

void
ixOsalMemFree (void *ptr)
{
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
    diag_printf("%s called\n", __FUNCTION__);
    return 0;
}

PUBLIC void
ixOsalYield (void)
{
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

unsigned int
ixQMgrCountLeadingZeros(UINT32 word)
{
    unsigned int leadingZerosCount = 0;

    if (word == 0)
	return 32;

    /* search the first bit set by testing the MSB and shifting the input word */
    while ((word & 0x80000000) == 0)
    {
    	word <<= 1;
	leadingZerosCount++;
    }
    return leadingZerosCount;
}
