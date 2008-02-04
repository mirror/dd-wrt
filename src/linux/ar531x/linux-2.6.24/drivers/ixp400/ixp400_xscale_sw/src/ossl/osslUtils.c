/**
 * ============================================================================
 * = COPYRIGHT
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
 * = PRODUCT
 *      Intel(r) IXP425 Software Release 
 *
 * = LIBRARY
 *      OSSL ( Operating System Services)  Library
 *
 * = MODULE
 *      OSSL Utilities
 *
 * = FILENAME
 *      osslUtils.c  (Now contains mapping to OSAL functionalities)
 *
 * = DESCRIPTION
 *      
 *   OSSL API functions for timers and memory management:
 *
 *	    ix_ossl_sleep
 *        ix_ossl_time_get
 *        ix_ossl_malloc
 *        ix_ossl_free
 *        ix_ossl_memcpy
 *        ix_ossl_memset
 *        ix_ossl_tick_get
 *      
 *
 * = AUTHOR
 *     Intel Corporation
 *
 * = AKNOWLEDGEMENTS
 *      
 *
 * = CREATION TIME
 *      1/9/2002 1:56:24 PM
 *
 * = CHANGE HISTORY
 *
 * ============================================================================
 */

#ifdef __vxworks
#include <taskLib.h>
#endif

#include "IxOsalBackward.h"


/* Support for OSSL thread_create */
PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslThreadCreate (IxOsalVoidFnVoidPtr entryPoint,
    void *arg, IxOsalThread * ptrThread)
{
    IX_STATUS ixStatus = IX_FAIL;

    /*
     * First prepare thread attribute 
     */
    IxOsalThreadAttr threadAttr;
    threadAttr.stackSize = 10 * 1024;   /* Default value */
    threadAttr.priority = 90;

    /*
     * Now pass to OSAL 
     */
    ixStatus = ixOsalThreadCreate (ptrThread, &threadAttr, entryPoint, arg);
    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalOsIxp400BackwardOsslThreadCreate: fail to create thread. \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    /*
     * Start thread 
     */
    ixStatus = ixOsalThreadStart (ptrThread);

    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalOsIxp400BackwardOsslThreadCreate: fail to start thread. \n",
            0, 0, 0, 0, 0, 0);

        return IX_FAIL;
    }
    return IX_SUCCESS;

}

/* Support for OSSL thread kill */
PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslThreadKill (IxOsalThread tid)
{
    return (ixOsalThreadKill (&tid));
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslThreadSetPriority (IxOsalThread tid,
    UINT32 priority)
{
    return (ixOsalThreadPrioritySet (&tid, priority));

}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslSemaphoreWait (IxOsalSemaphore semaphore,
    INT32 timeout)
{
    return (ixOsalSemaphoreWait (&semaphore, timeout));
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslSemaphorePost (IxOsalSemaphore sid)
{
    return (ixOsalSemaphorePost (&sid));
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardSemaphoreDestroy (IxOsalSemaphore sid)
{
    return (ixOsalSemaphoreDestroy (&sid));

}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslMutexInit (ix_ossl_mutex_state start_state,
    IxOsalMutex * pMutex)
{
    IX_STATUS ixStatus;

    /*
     * Mutex should be unlocked after init 
     */
    ixStatus = ixOsalMutexInit (pMutex);

    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalOsIxp400BackwardOsslMutexInit: fail to init mutex \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (IX_OSSL_MUTEX_LOCK == start_state)
    {
        ixStatus = ixOsalMutexLock (pMutex, IX_OSAL_WAIT_NONE);
        if (ixStatus != IX_SUCCESS)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
                IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalOsIxp400BackwardOsslMutexInit: fail to lock mutex \n",
                0, 0, 0, 0, 0, 0);
            return IX_FAIL;

        }
    }                           /* End of start_state check */

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslMutexLock (IxOsalMutex mid, INT32 timeout)
{
    return (ixOsalMutexLock (&mid, timeout));
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslMutexUnlock (IxOsalMutex mid)
{
    return (ixOsalMutexUnlock (&mid));
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslMutexDestroy (IxOsalMutex mid)
{
    return (ixOsalMutexDestroy (&mid));
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslTickGet (int *pticks)
{
    *pticks = (int) ixOsalSysClockRateGet ();
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslThreadDelay (int ticks)
{

#ifdef __linux
    schedule_timeout (ticks);
#endif

#ifdef __vxworks
	if (OK != taskDelay(ticks))
	{
	    return IX_FAIL;
	}
#endif
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslSleepTick (UINT32 ticks)
{

#ifdef __linux
    schedule_timeout (ticks);
#endif

#ifdef __vxworks
	if (OK != taskDelay(ticks))
	{
	    return IX_FAIL;
	}
#endif

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalOsIxp400BackwardOsslTimeGet (IxOsalTimeval * pTv)
{
    ixOsalTimeGet (pTv);
    return IX_SUCCESS;
}
