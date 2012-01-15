/**
 * @file IxOsalOsSemaphore.c
 *
 * @brief vxWorks-specific implementation for semaphore and mutex.
 *
 * Design Notes:
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

#include "semLib.h"

#include "IxOsal.h"

/* Header for private functions */
#include "IxOsalOs_private.h"

/* Use counting semaphore */
IX_STATUS
ixOsalSemaphoreInit (IxOsalSemaphore * sid, UINT32 value)
{

    SEM_ID semId;

    /*
     * Allocate counting semaphore, tasks are queued on FIFO. 
     */
    semId = semCCreate (SEM_Q_FIFO, value);

    if (semId == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreInit: NULL semId \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    *sid = semId;
    return IX_SUCCESS;
}

/* unlock a semaphore */
IX_STATUS
ixOsalSemaphorePost (IxOsalSemaphore * sid)
{

    int retVal;
    retVal = semGive (*(SEM_ID *) sid);
    if (retVal != OK)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphorePost(): ERROR - semGive, return error code = %d \n",
            retVal, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

IX_STATUS
ixOsalSemaphoreWait (IxOsalSemaphore * sid, INT32 timeout)
{

    int retVal;
	UINT32 ticks;

	if ( (timeout == IX_OSAL_WAIT_FOREVER) 
	   || (timeout == IX_OSAL_WAIT_NONE))
	{
        retVal = semTake (*(SEM_ID *) sid, timeout);
	}
	else if (timeout > 0)
	{
        ticks = (ixOsalSysClockRateGet () * timeout) / 1000;
        retVal = semTake (*(SEM_ID *) sid, ticks);
	}
	else
	{
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWait(): Illegal timeout value. \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
	}

    if (retVal != OK)
    {
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

/* 
 * Attempt to get semaphore, return immediately,
 * no error info because users expect some failures
 * when using this API.
 */
IX_STATUS
ixOsalSemaphoreTryWait (IxOsalSemaphore * sid)
{

    int retVal;

    retVal = semTake (*(SEM_ID *) sid, NO_WAIT);
    if (retVal != OK)
    {
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

IX_STATUS
ixOsalSemaphoreGetValue (IxOsalSemaphore * sid, UINT32 * value)
{
    ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
        IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalSemaphoreGetValue(): Not supported \n", 0, 0, 0, 0, 0, 0);
    return IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalSemaphoreDestroy (IxOsalSemaphore * sid)
{

    int retVal;
    retVal = semDelete (*(SEM_ID *) sid);
    if (retVal != OK)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreDestroy(): ERROR - semDelete, return error code = %d \n",
            retVal, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}

/**************************************
 * Mutex services 
 *************************************/

PUBLIC IX_STATUS
ixOsalMutexInit (IxOsalMutex * mutex)
{
    *mutex = (IxOsalMutex) semBCreate (SEM_Q_PRIORITY, SEM_FULL);

    return *mutex != NULL ? IX_SUCCESS : IX_FAIL;
}

PUBLIC IX_STATUS
ixOsalMutexLock (IxOsalMutex * mutex, INT32 timeout)
{
    UINT32 ticks;

    if (timeout > 0)
    {
        ticks = (ixOsalSysClockRateGet () * timeout) / 1000;
        return IX_OSAL_MAP_VX_RETCODE (semTake ((*mutex), (INT32) ticks));
    }
    else if (timeout == IX_OSAL_WAIT_FOREVER)
    {
        return IX_OSAL_MAP_VX_RETCODE (semTake ((*mutex), WAIT_FOREVER));
    }
    else if (timeout == IX_OSAL_WAIT_NONE)
    {
        return IX_OSAL_MAP_VX_RETCODE (semTake ((*mutex), NO_WAIT));
    }
    else
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexLock(): ERROR - wrong timeout value =  %d \n",
            timeout, 0, 0, 0, 0, 0);

        return IX_FAIL;
    }
}

PUBLIC IX_STATUS
ixOsalMutexUnlock (IxOsalMutex * mutex)
{
    return IX_OSAL_MAP_VX_RETCODE (semGive (*mutex));
}

PUBLIC IX_STATUS
ixOsalMutexTryLock (IxOsalMutex * mutex)
{
    return IX_OSAL_MAP_VX_RETCODE (semTake ((*mutex), NO_WAIT));
}

PUBLIC IX_STATUS
ixOsalMutexDestroy (IxOsalMutex * mutex)
{
    /*
     * semDelete takes SEM_ID as input.
     */
    return IX_OSAL_MAP_VX_RETCODE (semDelete (*mutex));
}

/**************************************
 * Fastmutex services 
 *************************************/
PUBLIC IX_STATUS
ixOsalFastMutexInit (IxOsalFastMutex * mutex)
{

#if CPU==SIMSPARCSOLARIS

    *mutex = semBCreate (SEM_Q_PRIORITY, SEM_FULL);

    return *mutex != NULL ? IX_SUCCESS : IX_FAIL;

#else

    if (mutex == NULL)
    {
        return IX_FAIL;
    }

    *mutex = 0;

    return IX_SUCCESS;

#endif

}

PUBLIC IX_STATUS
ixOsalFastMutexTryLock (IxOsalFastMutex * mutex)
{
#if CPU==SIMSPARCSOLARIS

    /*
     * For simulation it is OK to have generic implementation 
     */
    return IX_OSAL_MAP_VX_RETCODE (semTake (*mutex, NO_WAIT));

#endif
    /*
     * Call platform-specific fastMutexTryLock 
     */
    return ixOsalOemFastMutexTryLock (mutex);

}

PUBLIC IX_STATUS
ixOsalFastMutexUnlock (IxOsalFastMutex * mutex)
{
#if CPU==SIMSPARCSOLARIS

    return IX_OSAL_MAP_VX_RETCODE (semGive (*mutex));

#else

    if (mutex == NULL)
    {
        return IX_FAIL;
    }

    *mutex = 0;

    return IX_SUCCESS;

#endif
}

PUBLIC IX_STATUS
ixOsalFastMutexDestroy (IxOsalFastMutex * mutex)
{
    *mutex = 0;
    return IX_SUCCESS;
}
