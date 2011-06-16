/**
 * @file IxOsalOsTimer.c (linux)
 *
 * @brief Implementation for Timer API's.
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

#include <linux/timer.h>


static void callback_repeat_timer(unsigned long ptr)
{
    IxOsalTimerRec *nTimerPtr;
 
    nTimerPtr = (IxOsalTimerRec *)ptr;
    /* if timer inUse then only reregister the timer */
    if(nTimerPtr->inUse == TRUE)
    {
        /* call callback function registered by user */
        nTimerPtr->callback(nTimerPtr->callbackParam);
        nTimerPtr->timer.expires = jiffies + nTimerPtr->period;
        /* add timer to call the callback after period */
        add_timer(&nTimerPtr->timer);
    }
}

/**
 * @ingroup IxOsal
 *
 * @brief Schedules a repeating timer
 *
 * @param timer - handle of the timer object
 * @param period - timer trigger period, in milliseconds
 * @param priority - timer priority (0 being the highest)
 * @param callback - user callback to invoke when the timer triggers
 * @param param - custom parameter passed to the callback
 *
 * Schedules a timer to be called every period milliseconds. The timer
 * will invoke the specified callback function possibly in interrupt
 * context, passing the given parameter. If several timers trigger at the
 * same time contention issues are dealt according to the specified timer
 * priorities.
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalRepeatingTimerSchedule (IxOsalTimer *timer,
                                               UINT32 period,
                                               UINT32 priority,
                                               IxOsalVoidFnVoidPtr callback,
                                               void *param)
{
    IxOsalTimerRec *timerPtr;
    IX_OSAL_LOCAL_ENSURE(timer, 
            "ixOsalRepeatingTimerSchedule(): Null IxOsalTimer pointer",
            IX_FAIL);

    IX_OSAL_LOCAL_ENSURE(callback, 
            "ixOsalRepeatingTimerSchedule(): NULL callback function pointer", 
            IX_FAIL);

    *timer = kmalloc (sizeof (IxOsalTimerRec), GFP_KERNEL);
    if (!(*timer))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalRepeatingTimerSchedule():  "
            "Fail to allocate for IxOsalTimer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    
    timerPtr = *timer;
    timerPtr->inUse = TRUE;
    init_timer(&(timerPtr->timer));
    timerPtr->timer.function = callback_repeat_timer;
    timerPtr->timer.data = (unsigned long)timerPtr;
    timerPtr->timer.expires = jiffies + \
                          ((period*HZ)/IX_OSAL_THOUSAND);

    /* store period to call the callback at regular intervals in jiffies*/
    timerPtr->period = ((period*HZ)/IX_OSAL_THOUSAND);
    timerPtr->callback = callback;
    timerPtr->callbackParam = param;
    timerPtr->isRepeating = TRUE;
    add_timer(&(timerPtr->timer));

    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Schedules a single-shot timer
 *
 * @param timer - handle of the timer object
 * @param period - timer trigger period, in milliseconds
 * @param priority - timer priority (0 being the highest)
 * @param callback - user callback to invoke when the timer triggers
 * @param param - custom parameter passed to the callback
 *
 * Schedules a timer to be called after period milliseconds. The timer
 * will cease to function past its first trigger. The timer will invoke
 * the specified callback function, possibly in interrupt context, passing
 * the given parameter. If several timers trigger at the same time contention
 * issues are dealt according to the specified timer priorities.
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS
ixOsalSingleShotTimerSchedule (IxOsalTimer *timer,
                               UINT32 period,
                               UINT32 priority,
                               IxOsalVoidFnVoidPtr callback, void *param)
{
    IxOsalTimerRec *timerPtr;

    IX_OSAL_LOCAL_ENSURE(timer, 
            "ixOsalSingleShotTimerSchedule(): NULL IxOsalTimer pointer", 
            IX_FAIL);

    IX_OSAL_LOCAL_ENSURE(callback, 
            "ixOsalSingleShotTimerSchedule(): NULL callback function pointer", 
            IX_FAIL);

    *timer = kmalloc (sizeof (IxOsalTimerRec), GFP_KERNEL);
    if (!(*timer))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSingleShotTimerSchedule() Fail to allocate IxOsalTimer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    
    timerPtr = *timer;
    timerPtr->inUse = TRUE;
    init_timer(&timerPtr->timer);
    timerPtr->timer.function = (voidFnULongPtr)callback;
    timerPtr->timer.data =  (unsigned long)param;
    timerPtr->timer.expires = jiffies + \
                          ((period*HZ)/IX_OSAL_THOUSAND);
    timerPtr->isRepeating = FALSE;
    add_timer(&timerPtr->timer);

    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief Cancels a running timer
 *
 * @param timer - handle of the timer object
 *
 * Cancels a single-shot or repeating timer.
 *
 * @li Reentrant: no
 * @li IRQ safe:  yes
 *
 * @return - IX_SUCCESS/IX_FAIL
 */
PUBLIC IX_STATUS ixOsalTimerCancel (IxOsalTimer * timer)
{
    IxOsalTimerRec *timerPtr;
    IX_OSAL_LOCAL_ENSURE(timer, 
            "ixOsalTimerCancel():  Null IxOsalTimer pointer\n", 
            IX_FAIL);

    timerPtr = *timer;
    if(timerPtr->inUse == FALSE)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel():  Timer is already deleted\n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    timerPtr->inUse = FALSE;
    /* free & call del_timer in callback fn for repeat timer */
    del_timer(&(timerPtr->timer));
    kfree (*timer);

    return IX_SUCCESS;
}

/**
 * @ingroup IxOsal
 *
 * @brief displays all the running timers
 *
 * Displays a list with all the running timers and their parameters (handle,
 * period, type, priority, callback and user parameter)
 *
 * @li Reentrant: no
 * @li IRQ safe:  no
 *
 * @return - none
 */
PUBLIC void ixOsalTimersShow (void)
{
    ixOsalLog (IX_OSAL_LOG_LVL_MESSAGE, IX_OSAL_LOG_DEV_STDOUT,
        "ixOsalTimersShow not Supported in linux native mode implemenattion\n",
         0, 0, 0, 0, 0, 0);
 
   return ;
}


