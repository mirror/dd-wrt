/**
 * @file IxOsalOsSemaphore.c (eCos)
 *
 * @brief Implementation for semaphore and mutex.
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

#include "IxOsal.h"

/* Define a large number */
#define IX_OSAL_MAX_LONG (0x7FFFFFFF)

/* Max timeout in MS, used to guard against possible overflow */
#define IX_OSAL_MAX_TIMEOUT_MS (IX_OSAL_MAX_LONG/HZ)


PUBLIC IX_STATUS
ixOsalSemaphoreInit (IxOsalSemaphore * sid, UINT32 start_value)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_SUCCESS;
}

/**
 * DESCRIPTION: If the semaphore is 'empty', the calling thread is blocked. 
 *              If the semaphore is 'full', it is taken and control is returned
 *              to the caller. If the time indicated in 'timeout' is reached, 
 *              the thread will unblock and return an error indication. If the
 *              timeout is set to 'IX_OSAL_WAIT_NONE', the thread will never block;
 *              if it is set to 'IX_OSAL_WAIT_FOREVER', the thread will block until
 *              the semaphore is available. 
 *
 *
 */


PUBLIC IX_STATUS
ixOsalSemaphoreWait (IxOsalOsSemaphore * sid, INT32 timeout)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_SUCCESS;
}

/* 
 * Attempt to get semaphore, return immediately,
 * no error info because users expect some failures
 * when using this API.
 */
PUBLIC IX_STATUS
ixOsalSemaphoreTryWait (IxOsalSemaphore * sid)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

/**
 *
 * DESCRIPTION: This function causes the next available thread in the pend queue
 *              to be unblocked. If no thread is pending on this semaphore, the 
 *              semaphore becomes 'full'. 
 */
PUBLIC IX_STATUS
ixOsalSemaphorePost (IxOsalSemaphore * sid)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalSemaphoreGetValue (IxOsalSemaphore * sid, UINT32 * value)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalSemaphoreDestroy (IxOsalSemaphore * sid)
{
    diag_printf("%s called\n", __FUNCTION__);
    return IX_FAIL;
}

/****************************
 *    Mutex 
 ****************************/

PUBLIC IX_STATUS
ixOsalMutexInit (IxOsalMutex * mutex)
{
    cyg_drv_mutex_init(mutex);
    return IX_SUCCESS; 
}

PUBLIC IX_STATUS
ixOsalMutexLock (IxOsalMutex * mutex, INT32 timeout)
{
    int tries;

    if (timeout == IX_OSAL_WAIT_NONE) {
	if (cyg_drv_mutex_trylock(mutex))
	    return IX_SUCCESS;
	else
	    return IX_FAIL;
    }

    tries = (timeout * 1000) / 50;
    while (1) {
	if (cyg_drv_mutex_trylock(mutex))
	    return IX_SUCCESS;
	if (timeout != IX_OSAL_WAIT_FOREVER && tries-- <= 0)
	    break;
#ifdef CYGPKG_REDBOOT
	/* RedBoot runs with interrupts off. The IX_ETHDB_SYNC_SEND_NPE_MSG
	 * macro expects an ISR to unlock a mutex for an acknowledge. We
	 * have to poll for that incoming NPE message with RedBoot. Too bad
	 * we don't know which NPE to poll...
	 */
	{
	    int i;
	    for (i = 0; i < 3; i++)
		ixNpeMhMessagesReceive(i);
	}
#endif
	CYGACC_CALL_IF_DELAY_US(50);
    }
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMutexUnlock (IxOsalMutex * mutex)
{
    cyg_drv_mutex_unlock(mutex);
    return IX_SUCCESS;
}

/* 
 * Attempt to get mutex, return immediately,
 * no error info because users expect some failures
 * when using this API.
 */
PUBLIC IX_STATUS
ixOsalMutexTryLock (IxOsalMutex * mutex)
{
    if (cyg_drv_mutex_trylock(mutex))
	return IX_SUCCESS;
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMutexDestroy (IxOsalMutex * mutex)
{
    cyg_drv_mutex_destroy(mutex);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalFastMutexInit (IxOsalFastMutex * mutex)
{
    return ixOsalMutexInit(mutex);
}

PUBLIC IX_STATUS ixOsalFastMutexTryLock(IxOsalFastMutex *mutex)
{
    return ixOsalMutexTryLock(mutex);
}


PUBLIC IX_STATUS
ixOsalFastMutexUnlock (IxOsalFastMutex * mutex)
{
    return ixOsalMutexUnlock(mutex);
}

PUBLIC IX_STATUS
ixOsalFastMutexDestroy (IxOsalFastMutex * mutex)
{
    return ixOsalMutexDestroy(mutex);
}
