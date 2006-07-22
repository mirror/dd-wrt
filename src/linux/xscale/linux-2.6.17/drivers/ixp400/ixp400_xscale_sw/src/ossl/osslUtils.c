/**
 * ============================================================================
 * = COPYRIGHT
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
