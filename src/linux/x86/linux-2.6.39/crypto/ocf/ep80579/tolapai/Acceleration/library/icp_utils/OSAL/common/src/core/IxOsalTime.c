/**
 * @ingroup IxOsal
 * @file IxOsalTime.c
 *
 * @brief OS-independant implementation for timer-related 
 *        functions.
 *
 *
 * Contents:
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



/* include the OS independant implementation if native OS calls are not used */
#ifndef USE_NATIVE_OS_TIMER_API   

/* Define a warning threshhold */
#define IX_OSAL_TIMER_WARNING_THRESHOLD (IX_OSAL_MAX_TIMERS - 20)

/* Define 100 ms max callback time */
/* IX_MAX_TIMER_CALLBACK_TIME is in nsecs */
#define IX_MAX_TIMER_CALLBACK_TIME  (100000000)

/* define IX_OSAL_MAX_TIMERS */
#define IX_OSAL_MAX_TIMERS   100

/* define two semaphore values */
#define IX_OSAL_TIMER_SEM_UNAVAILABLE 0
#define IX_OSAL_TIMER_SEM_AVAILABLE   1

#define TIMER_INIT   10

/* Define Timer struct */
typedef struct
{
    BOOL inUse;
    BOOL isRepeating;
    UINT32 id;
    UINT32 priority;
    void *callbackParam;
    IxOsalTimeval period;
    IxOsalTimeval expires;
    IxOsalVoidFnVoidPtr callback;
} IxOsalTimerRec;


/* define private data structs */
PRIVATE IxOsalTimerRec ixOsalTimers[IX_OSAL_MAX_TIMERS];
PRIVATE IxOsalSemaphore ixOsalTimerRecalcSem;
PRIVATE IxOsalSemaphore ixOsalCriticalSectSem;
PRIVATE UINT32 lastTimerId = 0;
PRIVATE UINT32 ixOsalHigestTimeSlotUsed = 0;
PRIVATE BOOL ixOsalThresholdErr = FALSE;
PRIVATE UINT32 ixOsalTimerCbCnt = 0;
/* ixOsalTimerInited takes three values: TRUE, FALSE and TIMER_INIT */
PRIVATE UINT8 ixOsalTimerInited = FALSE;

/*
 * Private function definitions 
 */

PRIVATE IX_STATUS
createNewTimer (IxOsalVoidFnVoidPtr func, void *param, UINT32 priority,
    UINT32 interval, BOOL isRepeating, UINT32 * pTimerId);

PRIVATE IxOsalTimerRec *evaluateTimerPriority (IxOsalTimerRec * first,
    IxOsalTimerRec * second);

PRIVATE IxOsalTimerRec *findNextTimeout (IxOsalTimeval now);

PRIVATE void timerSleep (IxOsalTimerRec * nextTimer, IxOsalTimeval now);

PRIVATE void rescheduleTimer (IxOsalTimerRec * nextTimer);

PRIVATE void
callTimerCallback (IxOsalVoidFnVoidPtr callback, void *callbackParam);

PRIVATE int timerLoop (void);

PRIVATE IX_STATUS timerInit (void);

#endif /* !USE_NATIVE_OS_TIMER_API */

PRIVATE UINT32 MAX_UINT32 = 0xFFFFFFFF;

/* Return code if IxOsalTimavalToTicks called with too larg timeval */
#define IX_OSAL_TIMEVAL_TOO_LARGE       0
/* Public timeval functions */


/**
 * @ingroup IxOsal
 *
 * @brief Calculates the number of Ticks that correspond to
 *        the time interval provided
 *
 * @param  IxOsalTimeval tv (in)   - Time interval 
 *
 * Calculates the number of Ticks that correspond to the time
 * interval provided
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return The number of Ticks that correspond to the time
 * interval provided
 */

PUBLIC UINT32
ixOsalTimevalToTicks (IxOsalTimeval tv)
{
    UINT32 tickPerSecs = 0;
    UINT32 nanoSecsPerTick = 0;
    UINT32 maxSecs = 0;

    tickPerSecs = ixOsalSysClockRateGet ();
    nanoSecsPerTick = IX_OSAL_BILLION / tickPerSecs;

    /*
     * Make sure we do not overflow
     */
    maxSecs = (MAX_UINT32 / tickPerSecs) - (tv.nsecs / IX_OSAL_BILLION);

    if ( maxSecs < tv.secs )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalTimevalToTicks(): Timeval too high . Maximum value "
                "allowed in seconds is %u < %u \n",
                maxSecs , tv.secs, 0, 0, 0, 0);
        return IX_OSAL_TIMEVAL_TOO_LARGE;

    }

    return ((tv.secs * tickPerSecs) + (tv.nsecs / nanoSecsPerTick));
}



/**
 * @ingroup IxOsal
 *
 * @brief Calculates the time interval correspond to
 *        the number of Ticks provided provided
 *
 * @param  UINT32 ticks (in) - Number of Ticks
 * @param IxOsalTimeval * pTv(in/out)   - Time interval 
 *
* Calculates the time interval correspond to
 *        the number of Ticks provided provided
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return Nothing
 */


PUBLIC void
ixOsalTicksToTimeval (UINT32 ticks, IxOsalTimeval * pTv)
{
    UINT32 tickPerSecs = 0;
    UINT32 nanoSecsPerTick = 0;
    /*
     * Reset the time value 
     */
    pTv->secs = 0;
    pTv->nsecs = 0;

    tickPerSecs = ixOsalSysClockRateGet ();
    nanoSecsPerTick = IX_OSAL_BILLION / tickPerSecs;

    /*
     * value less than 1 sec 
     */
    if (tickPerSecs > ticks)    /* value less then 1 sec */
    {
        pTv->nsecs = ticks * nanoSecsPerTick;
    }
    else
    {
        pTv->secs = ticks / tickPerSecs;
        pTv->nsecs = (ticks % tickPerSecs) * nanoSecsPerTick;
    }
}

/* include the OS independant implementation if native OS calls are not used */
#ifndef USE_NATIVE_OS_TIMER_API   

/*
 * Public timer functions  
 */

/**
 * @ingroup IxOsal
 *
 * @brief Schedules a single shot timer
 *
 * @param  IxOsalTimer * timer (in)   - Timer id
 *                 UINT32 period - Period of timer
 *                  UINT32 priority - Priority of the scheduled timer
 *                  IxOsalVoidFnVoidPtr callback - Callback function for the scheduled timer
 *                  void *param - Parameters of the callback function
 *
 * Schedules a single shot timer setting its priority and callback function and arguments
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return IX_SUCCESS on successfully scheduling the Timer, IX_FAIL otherwise
 */

PUBLIC IX_STATUS
ixOsalSingleShotTimerSchedule (IxOsalTimer * timer,
    UINT32 period, UINT32 priority, IxOsalVoidFnVoidPtr callback, void *param)
{
    IX_STATUS ixStatus = IX_FAIL;

    if (ixOsalTimerInited == FALSE)
    {
        /*
         * Init timer 
         */
        ixStatus = timerInit ();
        if (ixStatus != IX_SUCCESS)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalSingleShotTimerSchedule: fail to init timer \n",
                0, 0, 0, 0, 0, 0);
            ixOsalTimerInited = FALSE;
            return IX_FAIL;
        }
    }

    if (ixOsalTimerInited == TIMER_INIT)
    {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalSingleShotTimerSchedule:Timer Init in Progress \n",
                0, 0, 0, 0, 0, 0);
            return IX_FAIL;
    }

    /* Call createNewTimer() with FALSE as we want a non-repeating timer */
    ixStatus =
        createNewTimer (callback, param, priority, period, FALSE, timer);

    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalSingleShotTimerSchedule: fail to create timer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    return IX_SUCCESS;
}


/**
 * @ingroup IxOsal
 *
 * @brief Schedules a repeating timer
 *
 * @param  IxOsalTimer * timer (in)   - Timer id
 *                 UINT32 period - Period of timer
 *                  UINT32 priority - Priority of the scheduled timer
 *                  IxOsalVoidFnVoidPtr callback - Callback function for the scheduled timer
 *                  void *param - Parameters of the callback function
 *
 * Schedules a repeating timer setting its priority and callback function and arguments
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return IX_SUCCESS on success, IX_FAIL otherwise
 */

PUBLIC IX_STATUS
ixOsalRepeatingTimerSchedule (IxOsalTimer * timer,
    UINT32 period, UINT32 priority, IxOsalVoidFnVoidPtr callback, void *param)
{
    IX_STATUS ixStatus = IX_FAIL;

    if (ixOsalTimerInited == FALSE)
    {
        /*
         * Init timer 
         */
        ixStatus = timerInit ();
        if (ixStatus != IX_SUCCESS)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalRepeatingTimerSchedule: fail to init timer \n",
                0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }
    }

    if (ixOsalTimerInited == TIMER_INIT)
    {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "ixOsalSingleShotTimerSchedule:Timer Init in Progress \n",
                0, 0, 0, 0, 0, 0);
            return IX_FAIL;
    }

    /* Call createNewTimer() with TRUE as we want a repeating timer */
    ixStatus =
        createNewTimer (callback, param, priority, period, TRUE, timer);

    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalRepeatingTimerSchedule: fail to create timer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    return IX_SUCCESS;
}


/**
 * @ingroup IxOsal
 *
 * @brief Cancels a timer
 *
 * @param  IxOsalTimer * timer (in)   - Timer id
 *
 * Cancels a timer 
 *
 * @li Reentrant: yes
 * @li IRQ safe:  yes
 *
 * @return IX_SUCCESS on successfully canceling the timer, IX_FAIL otherwise
 */

PUBLIC IX_STATUS
ixOsalTimerCancel (IxOsalTimer * timer)
{
    UINT32 id;
    UINT32 i;
    IX_STATUS status = IX_FAIL;

    if (ixOsalTimerInited != TRUE)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel: call schedule APIs first to start timer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    id = *timer;

    status =
        ixOsalSemaphoreWait (&ixOsalCriticalSectSem, IX_OSAL_WAIT_FOREVER);
    OSAL_ENSURE_CHECK_SUCCESS(status, "fail to get ixOsalCriticalSectSem ");
    /*
     *  NOTE : there is no need to get the timer callback thread to wake
     *  up and recalculate.  If this timer is the next to expire, then
     *  the timer callback thread will wake up but not find any timers to
     *  callback and will just go back to sleep again anyway.
     */

    /*
     *  NOTE2 : We cancel a timer by doing a linear search for the timer id.
     *  This is not terribly efficient but is the safest way to ensure that
     *  cancellations do not cancel the wrong timer by mistake.  Also we
     *  assume timer cancellation does not occur often.  If the timer to
     *  be cancelled is not found, an error is logged.
     */

    for (i = 0; i < ixOsalHigestTimeSlotUsed; i++)
    {
        if (ixOsalTimers[i].inUse && ixOsalTimers[i].id == id)
        {
            ixOsalTimers[i].inUse = FALSE;
            break;
        }
    }

    status = ixOsalSemaphorePost (&ixOsalCriticalSectSem);
    OSAL_ENSURE_CHECK_SUCCESS(status, "fail to  ixOsalCriticalSectSem ");

    /* If i == ixOsalHigestTimeSlotUsed then we have exausted the ixOsalTimers[] array */
    if (i == ixOsalHigestTimeSlotUsed)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel: Timer not found\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
    return IX_SUCCESS;
}



/**
 * @ingroup IxOsal
 *
 * @brief Displays a list with all the running timers and their parameters
 *
 * Displays a list with all the running timers and their parameters (handle, period, type, priority, callback and user parameter)
 *
 * @li Reentrant: yes
 * @li IRQ safe:  No
 *
 * @return Nothing
 */


PUBLIC void
ixOsalTimersShow (void)
{
    UINT32 i = 0;
    UINT32 count = 0;

    if (ixOsalTimerInited != TRUE )
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimersShow: call schedule APIs first to start timer \n",
            0, 0, 0, 0, 0, 0);
        return;
    }

    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Timers\n", 0, 0, 0, 0, 0, 0);

    for (i = 0; i < ixOsalHigestTimeSlotUsed; i++)
    {
        if (ixOsalTimers[i].inUse == TRUE)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
                "id=%d, repeat=%d, priority=%d\n",
                ixOsalTimers[i].id, ixOsalTimers[i].isRepeating,
                ixOsalTimers[i].priority, 0, 0, 0);
            count++;
        }
    }

    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "total=%d\n", count, 0, 0, 0, 0, 0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "num called=%u\n", ixOsalTimerCbCnt, 0, 0, 0, 0, 0);

    ixOsalLog (IX_OSAL_LOG_LVL_USER, IX_OSAL_LOG_DEV_STDOUT,
        "Timer threshold reached Error Flag: %d\n", ixOsalThresholdErr,
        0, 0, 0, 0, 0);

}

/* Private functions only used in this file */

/**
 * @ingroup Private
 *
 * @brief Initializes the timer API
 *
 * @param  Nothing
 *
 * Initializes the timer API
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return Nothing
 */

PRIVATE IX_STATUS
timerInit (void)
{
    IxOsalThread taskId;
    IX_STATUS ixStatus;
    IxOsalThreadAttr timerThreadAttr;

    /* Make sure that the API is not initialized or being initialized */
    if (ixOsalTimerInited != FALSE) {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "timerInit: API currently not to uninitialised state\n",
            0, 0, 0, 0, 0, 0);

       return IX_FAIL;
    }
       /* Set the flag that the API is being initialised */
       ixOsalTimerInited = TIMER_INIT;

    /* Initialize the semaphores */
       ixStatus =
           ixOsalSemaphoreInit (&ixOsalCriticalSectSem,
           IX_OSAL_TIMER_SEM_AVAILABLE);
       OSAL_ENSURE_CHECK_SUCCESS(ixStatus, "fail to Init ixOsalCriticalSectSem ");
       ixStatus =
           ixOsalSemaphoreInit (&ixOsalTimerRecalcSem,
           IX_OSAL_TIMER_SEM_AVAILABLE);
       OSAL_ENSURE_CHECK_SUCCESS(ixStatus, "fail to Init ixOsalTimerRecalcSem");

    /* Set the attributes of the timerThreadAttr in order to define the characteristics of the timer thread */

       timerThreadAttr.stackSize = IX_OSAL_THREAD_DEFAULT_STACK_SIZE;  
       timerThreadAttr.priority = IX_OSAL_DEFAULT_THREAD_PRIORITY; 
       timerThreadAttr.name = "tOSALTimer";

    /* Create the timer thread */
       ixStatus = ixOsalThreadCreate (&taskId, &timerThreadAttr, \
                                   (IxOsalVoidFnVoidPtr) timerLoop, NULL);


       if (ixStatus != IX_SUCCESS)
       {
           ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
               "timerInit: fail to create thread \n",
               0, 0, 0, 0, 0, 0);
           ixOsalTimerInited = FALSE;
           return IX_FAIL;
       }

       /* Start the thread now */
       ixStatus = ixOsalThreadStart (&taskId);

       OSAL_ENSURE_CHECK_SUCCESS(ixStatus, "fail to start Thread");

       ixOsalTimerInited = TRUE;

       return IX_SUCCESS;
}

/**
 * @ingroup Private
 *
 * @brief Allocates a new timer
 *
 * @param   IxOsalVoidFnVoidPtr func (in) - Callback function for the scheduled timer
 *                  void *param (in) - Parameters of the callback function
 *                  UINT32 priority (in) - Priority of the scheduled timer
 *                  UINT32 interval (in) - Interval of the scheduled timer
 *                  BOOL isRepeating (in) - Flag if a repeating timer
 *                  UINT32 * timerId (in)   - Timer id
 *
 * Allocates a new timer.  It is used by Schedule and ScheduleRepeating. 
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return IX_SUCCESS on successfully allocating a new timer, IX_FAIL otherwise
 */

PRIVATE IX_STATUS
createNewTimer (IxOsalVoidFnVoidPtr func, void *param, UINT32 priority,
    UINT32 interval, BOOL isRepeating, UINT32 * timerId)
{
    UINT32 i;
    IX_STATUS status = IX_SUCCESS;
    int osTicks;
    IxOsalTimeval timeVal;

    osTicks = (int)ixOsalSysClockRateGet ();
    /*
     * Figure out how many milisecond per tick and compare against interval 
     */
    if (interval < (UINT32) (IX_OSAL_THOUSAND/ osTicks) || (interval == 0 ))
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "client requested time interval (%d) finer than clock ticks\n",
            interval, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    /*
     * Increment timerId 
     */
    *timerId = ++lastTimerId;

    status =
        ixOsalSemaphoreWait (&ixOsalCriticalSectSem, IX_OSAL_WAIT_FOREVER);
    OSAL_ENSURE_CHECK_SUCCESS(status, "fail to start Thread");

    for (i = 0; i < IX_OSAL_MAX_TIMERS; i++)
    {
        if (!ixOsalTimers[i].inUse)
        {
            break;
        }
    }

    if ((i >= IX_OSAL_TIMER_WARNING_THRESHOLD)
        && (ixOsalThresholdErr == FALSE))
    {
        /*
         * This error serves as an early indication that the number of
         * available timer slots will need to be increased. This is done
         * by increasing IX_OSAL_MAX_TIMERS
         */
        ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
            "Timer threshold reached. Only %d timer slots now available\n",
            IX_OSAL_MAX_TIMERS - i, 0, 0, 0, 0, 0);
        ixOsalThresholdErr = TRUE;
    }

    if (i == IX_OSAL_MAX_TIMERS)
    {
        /*
         *  If you get this error, increase MAX_TIMERS above 
         */
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "Out of timer slots %d used - request ignored\n",
            i, 0, 0, 0, 0, 0);
        status = ixOsalSemaphorePost (&ixOsalCriticalSectSem);
        OSAL_ENSURE_CHECK_SUCCESS(status, "fail to post Semaphore");
        status = IX_FAIL;
    }
    else
    {
        ixOsalTimers[i].inUse = TRUE;

        IX_OSAL_MS_TO_TIMEVAL (interval, &timeVal);

        ixOsalTimers[i].period = timeVal;
        ixOsalTimers[i].isRepeating = isRepeating;
        ixOsalTimeGet (&(ixOsalTimers[i].expires));
        IX_OSAL_TIME_ADD ((ixOsalTimers[i].expires), (ixOsalTimers[i].period));
            ixOsalTimers[i].priority = priority;
        ixOsalTimers[i].callback = func;
        ixOsalTimers[i].callbackParam = param;
        ixOsalTimers[i].id = *timerId;

        if ( i >= ixOsalHigestTimeSlotUsed)
        {
            ixOsalHigestTimeSlotUsed = i + 1;
        }

        status = ixOsalSemaphorePost (&ixOsalTimerRecalcSem);
        OSAL_ENSURE_CHECK_SUCCESS(status, "fail to post Semaphore");
        status = ixOsalSemaphorePost (&ixOsalCriticalSectSem);
        OSAL_ENSURE_CHECK_SUCCESS(status, "fail to post Semaphore");
    }

    return status;
}

/**
 * @ingroup Private
 *
 * @brief Evaluates the priority between two timer
 *
 * @param   IxOsalTimerRec * first (in) - Timer record
 *          IxOsalTimerRec * second (in)
 *
 * Evaluates the priority between two timers
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return The timer record of the two that has higher priority
 */


PRIVATE IxOsalTimerRec *
evaluateTimerPriority (IxOsalTimerRec * first, IxOsalTimerRec * second)
{
    IX_OSAL_ASSERT (first != NULL || second != NULL);

    if (first == NULL)
    {
        return second;
    }

    if (second == NULL)
    {
        return first;
    }

    /*
     *  For now we just compare the values of the priority with lower
     *  values getting higher priority.
     *
     *  If someone needs to change priorities of different purposes without
     *  modifying the order of the enums, then more code will be required
     *  here.
     */

    return ((first->priority <= second->priority) ? first : second);
}


/**
 * @ingroup Private
 *
 * @brief Evaluates the next timeout
 *
 * @param   IxOsalTimeval now(in) - Timer record
 *
 *  Evaluates the next timer about to go off. We do this by starting with a time 
 *  infinitely far in the future and successively finding better timers
 *  (ones that go off sooner).
 *
 *  If a timer is found that has already expired, the flag bestTimerExpired is
 *  set.  If another timer is found that has also expired, their respective 
 *  priorities are compared using the function evaluateTimerPriority()
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return The record of the next expiring timer, or NULL if there is no next expiring timer
 */

PRIVATE IxOsalTimerRec *
findNextTimeout (IxOsalTimeval now)
{
    IxOsalTimeval timeoutAt = { ULONG_MAX, LONG_MAX };
    UINT32 i;
    IxOsalTimerRec *bestTimer = NULL;
    BOOL bestTimerExpired = FALSE;
    BOOL thisTimerExpired = FALSE;

    for (i = 0; i < ixOsalHigestTimeSlotUsed; i++)
    {
        if (ixOsalTimers[i].inUse)
        {
            thisTimerExpired = FALSE;

            /*
             * Check if this timer has expired,
             * i.e. 'now' must be greater than ixOsalTimers[i].expired 
             */

            if (!IX_OSAL_TIME_GT (ixOsalTimers[i].expires, now))
            {
                thisTimerExpired = TRUE;
            }

            /*
             * If more than one timer has expired, determine
             * which callback to call first based on a priority scheme
             * i.e. the bestTimer
             */

            if ((bestTimerExpired && thisTimerExpired) ||
                IX_OSAL_TIME_EQ (ixOsalTimers[i].expires, timeoutAt))
            {
                bestTimer =
                    evaluateTimerPriority (bestTimer, &ixOsalTimers[i]);
                timeoutAt = bestTimer->expires;
            }
            else if (IX_OSAL_TIME_LT (ixOsalTimers[i].expires, timeoutAt))
            {
                bestTimer = &ixOsalTimers[i];
                timeoutAt = ixOsalTimers[i].expires;
            }

            /*
             *  bestTimer can not be NULL here because any timer will
             *  have a shorter timeout than the default.
             */
            if(bestTimer != NULL)
            {
                if ((IX_OSAL_TIME_GT (bestTimer->expires, now)) != TRUE)
                {
                    bestTimerExpired = TRUE;
                }
            }
        }
    }
    return bestTimer;
}


/**
 * @ingroup Private
 *
 * @brief Puts a timer to sleep
 *
 * @param   IxOsalTimerRec * nextTimer (in) - Timer record
 *          IxOsalTimeval now(in) - Timeval
 *
 * Puts a timer to sleep
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return Nothing
 */

PRIVATE void
timerSleep (IxOsalTimerRec * nextTimer, IxOsalTimeval now)
{
    UINT32 milliseconds;
    IxOsalTimeval temp;

    if (nextTimer == NULL)
    {
        ixOsalSemaphoreWait (&ixOsalTimerRecalcSem, IX_OSAL_WAIT_FOREVER);
    }
    else
    {
        temp.secs = nextTimer->expires.secs;
        temp.nsecs = nextTimer->expires.nsecs;

        IX_OSAL_TIME_SUB (temp, now);

        milliseconds = IX_OSAL_TIMEVAL_TO_MS (temp);

        /*
         * We should sleep but the period is less than a tick
         * * away, rounding up. 
         */
        if (milliseconds == 0)
        {
            milliseconds = 1;
        }

        /* The post hapens in the timerCreate function. So far this has not caused problems but
           maybe there is a clearer way to do this*/
       ixOsalSemaphoreWait (&ixOsalTimerRecalcSem, milliseconds);
    }
}

/**
 * @ingroup Private
 *
 * @brief Reschedules a timer
 *
 * @param   IxOsalTimerRec * nextTimer (in) - Timer record
 *
 * Reschedules a timer
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return Nothing
 */

PRIVATE void
rescheduleTimer (IxOsalTimerRec * nextTimer)
{
    if (nextTimer->isRepeating)
    {
        IX_OSAL_TIME_ADD (nextTimer->expires, nextTimer->period);
    }
    else
    {
        nextTimer->inUse = FALSE;
    }
}

/**
 * @ingroup Private
 *
 * @brief Calls the timer callback function
 *
 * @param  IxOsalVoidFnVoidPtr callback (in) - Pointer to the callback function
 * @param  void *callbackParam (in) - Parameters of the callback function
 *
 * Calls the timer callback function
 * 
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return Nothing
 */

PRIVATE void
callTimerCallback (IxOsalVoidFnVoidPtr callback, void *callbackParam)
{
    IxOsalTimeval timevalCbBegin, timevalCbFinish;
    static BOOL errorReported = FALSE;

    ixOsalTimerCbCnt++;
    ixOsalTimeGet (&timevalCbBegin);

    callback (callbackParam);

    ixOsalTimeGet (&timevalCbFinish);

    IX_OSAL_TIME_SUB (timevalCbFinish, timevalCbBegin);

   /*
    *  Maximum time spent in a call-back is defined as 100 millisec.  (one
    *  tenth of a second).
    */


    if ((timevalCbFinish.nsecs > IX_MAX_TIMER_CALLBACK_TIME)
        || (timevalCbFinish.secs > 0))
    {
        if (!errorReported)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_WARNING, IX_OSAL_LOG_DEV_STDOUT,
                "Slow timer callback - %d Secs, %d nSecs \n",
                timevalCbFinish.secs, timevalCbFinish.nsecs, 0, 0, 0, 0);
            errorReported = TRUE;
        }
    }
}



/**
 * @ingroup Private
 *
 * @brief Main timer thread
 *
 * @param   Nothing
 * 
 * This is a thread that is being spawned by OSAL for the timer API. It is an infinite loop that
 * constantly monitors timers if they go off and if so it calls their callback functions. 
 *
 * @li Reentrant: Yes
 * @li IRQ safe:  No
 *
 * @return IX_FAIL if failed, otherwise it will loop for ever
 */

PRIVATE int
timerLoop (void)
{
    IxOsalTimeval now;
    IxOsalVoidFnVoidPtr callback = NULL;
    void *callbackParam = NULL;
    IxOsalTimerRec *nextTimer;
    IX_STATUS status=IX_SUCCESS;

    while (status==IX_SUCCESS)
    {
        /*
         *  This loop catches all cases in a simple way.  If multiple
         *  timers expire together, then lowest will be <=0 until all
         *  have been processed and the queue get won't get invoked.
         */

        status =
            ixOsalSemaphoreWait (&ixOsalCriticalSectSem,
            IX_OSAL_WAIT_FOREVER);
        OSAL_ENSURE_CHECK_SUCCESS(status, "fail to get Semaphore");

        ixOsalTimeGet (&now);
        nextTimer = findNextTimeout (now);

        if ((nextTimer == NULL)
            || IX_OSAL_TIME_GT ((nextTimer->expires), (now)))
        {
            callback = NULL;
        }
        else
        {
            rescheduleTimer (nextTimer);
            callback = nextTimer->callback;
            callbackParam = nextTimer->callbackParam;
        }

        status = ixOsalSemaphorePost (&ixOsalCriticalSectSem);
        OSAL_ENSURE_CHECK_SUCCESS(status, "fail to post Semaphore");

        if (callback != NULL)
        {
            callTimerCallback (callback, callbackParam);
        }
        else
        {
            timerSleep (nextTimer, now);
        }
    }
    return IX_FAIL;
}

#endif /* !USE_NATIVE_OS_TIMER_API */
