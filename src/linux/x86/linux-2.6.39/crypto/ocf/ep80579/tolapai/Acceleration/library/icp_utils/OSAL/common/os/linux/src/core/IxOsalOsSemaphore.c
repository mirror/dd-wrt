/**
 * @file IxOsalOsSemaphore.c (linux)
 *
 * @brief Implementation for semaphore and mutex.
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
#include <linux/slab.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#include <asm/semaphore.h>
#else
#include <linux/semaphore.h>
#endif

#include <asm/atomic.h>



#ifdef IX_OSAL_OS_LINUX_VERSION_2_6

#include <linux/hardirq.h>

#else /* !KERNEL VERSION 2.6 */

#include <asm/hardirq.h>

#endif /* KERNEL_VERSION */


/* Define a large number */
#define IX_OSAL_MAX_LONG (0x7FFFFFFF)

/* Max timeout in MS, used to guard against possible overflow */
#define IX_OSAL_MAX_TIMEOUT_MS (IX_OSAL_MAX_LONG/HZ)


PUBLIC IX_STATUS
ixOsalSemaphoreInit (IxOsalSemaphore * sid, UINT32 start_value)
{

    if (NULL == sid)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreInit: NULL semaphore pointer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    *sid = kmalloc (sizeof (struct semaphore), GFP_KERNEL);
    if (!(*sid))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreInit: fail to allocate for semaphore \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    sema_init (*sid, start_value);

    return IX_SUCCESS;
}

/**
 * DESCRIPTION: If the semaphore is 'empty', the calling thread is blocked.
 *         If the semaphore is 'full', it is taken and control is returned
 *         to the caller. If the time indicated in 'timeout' is reached,
 *         the thread will unblock and return an error indication. If the
 *         timeout is set to 'IX_OSAL_WAIT_NONE', the thread will never block;
 *         if it is set to 'IX_OSAL_WAIT_FOREVER', the thread will block until
 *         the semaphore is available.
 *
 *
 */


PUBLIC IX_STATUS
ixOsalSemaphoreWait (IxOsalOsSemaphore * sid, INT32 timeout)
{

    IX_STATUS ixStatus = IX_SUCCESS;
    unsigned long timeoutTime;

    if (sid == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWait(): NULL semaphore handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    /*
     * Guard against illegal timeout values
     */
    if ((timeout < 0) && (timeout != IX_OSAL_WAIT_FOREVER))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWait(): illegal timeout value \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (timeout == IX_OSAL_WAIT_FOREVER)
    {
        down (*sid);
    }
    else if (timeout == IX_OSAL_WAIT_NONE)
    {
        if (down_trylock (*sid))
        {
            ixStatus = IX_FAIL;
        }
    }
    else if (timeout > IX_OSAL_MAX_TIMEOUT_MS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWait(): use a smaller timeout value to avoid \
            overflow \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    else
    {
        /* Convert timeout in milliseconds to HZ */
        timeoutTime = jiffies + (timeout * HZ) /IX_OSAL_THOUSAND;
        while (1)
        {
            if (!down_trylock (*sid))
            {
                break;
            }
            else
            {
                if (time_after(jiffies, timeoutTime))
                {
                    ixStatus = IX_FAIL;
                    break;
                }
            }
            /* Switch to next running process instantly */
            set_current_state((long)TASK_INTERRUPTIBLE);
            schedule_timeout(1);

        }  /* End of while loop */
    }      /* End of if */

    return ixStatus;

}

/*
 * Attempt to get semaphore, return immediately,
 * no error info because users expect some failures
 * when using this API.
 */
PUBLIC IX_STATUS
ixOsalSemaphoreTryWait (IxOsalSemaphore * sid)
{
    if (sid == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreTryWait(): NULL semaphore handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    if (down_trylock (*sid))
    {
        return IX_FAIL;
    }
    return IX_SUCCESS;
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
    if (sid == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphorePost(): NULL semaphore handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    up (*sid);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalSemaphoreGetValue (IxOsalSemaphore * sid, UINT32 * value)
{
    if (sid == NULL)
    {
         ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalSemaphoreGetValue(): NULL semaphore handle \n",
                0, 0, 0, 0, 0, 0);
         return IX_FAIL;
    } 
   
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
	*value = atomic_read(&((*sid)->count));
#else
	*value = (*sid)->count;
#endif
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalSemaphoreDestroy (IxOsalSemaphore * sid)
{
    if (sid == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreDestroy(): NULL semaphore handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    
    kfree (*sid);
    *sid = 0;

    return IX_SUCCESS;
}

/****************************
 *    Mutex
 ****************************/

PUBLIC IX_STATUS
ixOsalMutexInit (IxOsalMutex * mutex)
{
    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexInit(): NULL mutex handle \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    *mutex =(struct semaphore *) kmalloc(sizeof(struct semaphore), GFP_KERNEL);
    if (*mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexInit(): Fail to allocate for mutex \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    init_MUTEX (*mutex);
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalMutexLock (IxOsalMutex * mutex, INT32 timeout)
{
    unsigned long timeoutTime;

    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexLock(): NULL mutex handle \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if ((timeout < 0) && (timeout != IX_OSAL_WAIT_FOREVER))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexLock(): Illegal timeout value \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (timeout == IX_OSAL_WAIT_FOREVER)
    {
        down (*mutex);
    }
    else if (timeout == IX_OSAL_WAIT_NONE)
    {
        if (down_trylock (*mutex))
        {
            return IX_FAIL;
        }
    }
    else if (timeout > IX_OSAL_MAX_TIMEOUT_MS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
           IX_OSAL_LOG_DEV_STDOUT,
           "ixOsalMutexLock(): use smaller timeout value to avoid overflow \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    else
    {
        timeoutTime = jiffies + (timeout * HZ) / IX_OSAL_THOUSAND;
        while (1)
        {
            if (!down_trylock (*mutex))
            {
                break;
            }
            else
            {
                if (time_after(jiffies, timeoutTime))
                {
                    return IX_FAIL;
                }
            }

           /* Switch to next running process if not in atomic state  */
           if (!in_atomic())
           {
                set_current_state((long)TASK_INTERRUPTIBLE);
                schedule_timeout(1);
           }
        }       /* End of while loop */
    }           /* End of if */
    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalMutexUnlock (IxOsalMutex * mutex)
{
    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexUnlock(): NULL mutex handle \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    up (*mutex);
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
    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexTryLock(): NULL mutex handle \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (down_trylock (*mutex))
    {
        return IX_FAIL;
    }

    return IX_SUCCESS;
}

PUBLIC IX_STATUS
ixOsalMutexDestroy (IxOsalMutex * mutex)
{
    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalMutexDestroy(): NULL mutex handle \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    kfree (*mutex);
    *mutex = 0;

    return IX_SUCCESS;
}



#ifdef IX_OSAL_OEM_FAST_MUTEX

PUBLIC IX_STATUS
ixOsalFastMutexInit (IxOsalFastMutex * mutex)
{
    return IX_OSAL_OEM_FAST_MUTEX_INIT (mutex);
}

PUBLIC IX_STATUS
ixOsalFastMutexTryLock(IxOsalFastMutex *mutex)
{
    return IX_OSAL_OEM_FAST_MUTEX_TRYLOCK(mutex);
}

PUBLIC IX_STATUS
ixOsalFastMutexUnlock (IxOsalFastMutex * mutex)
{
    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalFastMutexUnlock(): NULL mutex handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_OSAL_OEM_FAST_MUTEX_UNLOCK(mutex);
}

PUBLIC IX_STATUS
ixOsalFastMutexDestroy (IxOsalFastMutex * mutex)
{
    if (mutex == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalFastMutexDestroy(): NULL mutex handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    return IX_OSAL_OEM_FAST_MUTEX_DESTROY(mutex);
}

#else /* ! OEM_FAST_MUTEX */

PUBLIC IX_STATUS 
ixOsalFastMutexInit (IxOsalFastMutex * mutex)
{
  if (mutex)
    {
       atomic_set(mutex,1);
       return IX_SUCCESS;
    }
    else
    {
      return IX_FAIL;
    } 
}

PUBLIC IX_STATUS  
ixOsalFastMutexTryLock(IxOsalFastMutex *mutex)
{
    if (atomic_dec_and_test(mutex))
    {
        return IX_SUCCESS;
    }
    else
    {
        atomic_inc(mutex);
        return IX_FAIL;
    }
} 

PUBLIC IX_STATUS
ixOsalFastMutexUnlock (IxOsalFastMutex * mutex)
{
    if (mutex)
    {
       atomic_inc(mutex);
       return IX_SUCCESS;
    }
    else
    { 
      return IX_FAIL;
    } 
}

PUBLIC IX_STATUS
ixOsalFastMutexDestroy (IxOsalFastMutex * mutex)
{
    atomic_set(mutex,1);
    return IX_SUCCESS;
}

#endif /* IX_OSAL_OEM_FAST_MUTEX */


PUBLIC
IX_STATUS ixOsalSemaphoreDownTimeout(
                  IxOsalSemaphore *sid,
                  INT32        timeout)
{
    IX_STATUS ixStatus;
    ixStatus = ixOsalSemaphoreWait (sid, timeout);
    return ixStatus;

} /* ixOsalSemaphoreDownTimeout */



PUBLIC
IX_STATUS ixOsalSemaphoreWaitInterruptible(
              IxOsalSemaphore *sid,
              INT32           timeout)
{
    IX_STATUS ixStatus = IX_SUCCESS;
    unsigned long timeoutTime;

    if (sid == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWaitInterruptible(): NULL handle \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    /*
     * Guard against illegal timeout values
     */
    if ((timeout < 0) && (timeout != IX_OSAL_WAIT_FOREVER))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWaitInterruptible: illegal timeout value\n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (timeout == IX_OSAL_WAIT_FOREVER)
    {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
        down_interruptible(*sid);
#else
        if (down_interruptible(*sid) < 0)
		{
	        return IX_FAIL;
		}
#endif
    }
    else if (timeout == IX_OSAL_WAIT_NONE)
    {
        if (down_trylock (*sid))
        {
            ixStatus = IX_FAIL;
        }
    }
    else if (timeout > IX_OSAL_MAX_TIMEOUT_MS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSemaphoreWaitInterruptible():use smaller timeout \
             value to avoid overflow \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    else
    {
        /* Convert timeout in milliseconds to HZ */
        timeoutTime = jiffies + (timeout * HZ) /IX_OSAL_THOUSAND;
        while (1)
        {
            if (!down_trylock (*sid))
            {
                break;
            }
            else
            {
                if (time_after(jiffies, timeoutTime))
                {
                    ixStatus = IX_FAIL;
                    break;
                }
            }
            /* Switch to next running process instantly */
            set_current_state((long)TASK_INTERRUPTIBLE);
            schedule_timeout(1);

        }  /* End of while loop */
    }      /* End of if */

    return ixStatus;

} /* ixOsalSemaphoreWaitInterruptible */


PUBLIC
IX_STATUS ixOsalSemaphorePostWakeup(IxOsalSemaphore *sid)
{
    if (sid == NULL)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                  "ixOsalSemaphorePost(): NULL semaphore handle \n",
                  0, 0, 0, 0, 0, 0);
         return IX_FAIL;
    }

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
	if( atomic_read(&((*sid)->count)) < 1)
	{
		up(*sid);
	}
	wake_up(&((*sid)->wait));
#else
	if( (*sid)->count < 1)
	{
		up(*sid);
	}
#endif
    return IX_SUCCESS;

} /* ixOsalSemaphorePostWakeup */


PUBLIC
IX_STATUS ixOsalSemaphoreFlush(IxOsalSemaphore *sid)
{
    if (sid == NULL)
    {
        ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                  "ixOsalSemaphoreFlush(): NULL semaphore handle \n",
                  0, 0, 0, 0, 0, 0);
         return IX_FAIL;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
	atomic_set(&((*sid)->count), 0);
	wake_up_all(&((*sid)->wait));
#else
	(*sid)->count = 0;
#endif
    return IX_SUCCESS;

} /* ixOsalSemaphoreFlush */

