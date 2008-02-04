/**
 * @file IxOsalTime.c
 *
 * @brief OS-independant implementation for timer-related 
 *        functions.
 *
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

#include "IxOsal.h"

/* Define a warning threshhold */
#define IX_OSAL_TIMER_WARNING_THRESHOLD (IX_OSAL_MAX_TIMERS - 20)

/* Define 100 ms max callback time */
#define IX_MAX_TIMER_CALLBACK_TIME  (100000000)

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

/* define IX_OSAL_MAX_TIMERS */
#define IX_OSAL_MAX_TIMERS	100

/* define two semaphore valures */
#define IX_OSAL_TIMER_SEM_UNAVAILABLE 0
#define IX_OSAL_TIMER_SEM_AVAILABLE   1

/* define private date structs */
PRIVATE IxOsalTimerRec ixOsalTimers[IX_OSAL_MAX_TIMERS];
PRIVATE IxOsalSemaphore ixOsalTimerRecalcSem;
PRIVATE IxOsalSemaphore ixOsalCriticalSectSem;
PRIVATE char *ixOsalTimerThreadName = "tOSALTimer";
PRIVATE UINT32 lastTimerId = 0;
PRIVATE UINT32 ixOsalHigestTimeSlotUsed = 0;
PRIVATE BOOL ixOsalThresholdErr = FALSE;
PRIVATE UINT32 ixOsalTimerCbCnt = 0;
PRIVATE BOOL ixOsalTimerInited = FALSE;

/*
 * Private function definitions 
 */

PRIVATE IX_STATUS
createNewTimer (IxOsalVoidFnVoidPtr func, void *param, UINT32 priority,
    UINT32 interval, BOOL isRepeating, IxOsalTimer * pTimerId);

PRIVATE IxOsalTimerRec *evaluateTimerPriority (IxOsalTimerRec * first,
    IxOsalTimerRec * second);

PRIVATE IxOsalTimerRec *findNextTimeout (IxOsalTimeval now);

PRIVATE void timerSleep (IxOsalTimerRec * nextTimer, IxOsalTimeval now);

PRIVATE void rescheduleTimer (IxOsalTimerRec * nextTimer);

PRIVATE void
callTimerCallback (IxOsalVoidFnVoidPtr callback, void *callbackParam);

int timerLoop (void);

PRIVATE IX_STATUS timerInit (void);

/* Public timeval functions */

PUBLIC UINT32
ixOsalTimevalToTicks (IxOsalTimeval tv)
{
    UINT32 tickPerSecs = 0;
    UINT32 nanoSecsPerTick = 0;

    tickPerSecs = ixOsalSysClockRateGet ();
    nanoSecsPerTick = IX_OSAL_BILLION / tickPerSecs;
    return ((tv.secs * tickPerSecs) + (tv.nsecs / nanoSecsPerTick));
}

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

/*
 * Public timer functions  
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
            return IX_FAIL;
        }
        ixOsalTimerInited = TRUE;
    }

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
        ixOsalTimerInited = TRUE;
    }

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

PUBLIC IX_STATUS
ixOsalTimerCancel (IxOsalTimer * timer)
{
    UINT32 id;
    UINT32 i;
    IX_STATUS status = IX_FAIL;

    if (ixOsalTimerInited == FALSE)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel: call schedule APIs first to start timer \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    id = *timer;

    status =
        ixOsalSemaphoreWait (&ixOsalCriticalSectSem, IX_OSAL_WAIT_FOREVER);

    if (status != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel: fail to get semaphore \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }
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
    if (status != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel: fail to free semaphore \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    if (i >= ixOsalHigestTimeSlotUsed)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "ixOsalTimerCancel: Timer not found\n", 0, 0, 0, 0, 0, 0);
	return IX_FAIL;
    }
    return IX_SUCCESS;
}

void
ixOsalTimersShow (void)
{
    UINT32 i = 0;
    UINT32 count = 0;

    if (ixOsalTimerInited == FALSE)
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

PRIVATE IX_STATUS
timerInit (void)
{
    IxOsalThread taskId;
    IX_STATUS ixStatus;
    IxOsalThreadAttr timerThreadAttr;

    ixStatus =
        ixOsalSemaphoreInit (&ixOsalCriticalSectSem,
        IX_OSAL_TIMER_SEM_AVAILABLE);
    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "Error creating critical section semaphore\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    ixStatus =
        ixOsalSemaphoreInit (&ixOsalTimerRecalcSem,
        IX_OSAL_TIMER_SEM_AVAILABLE);

    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "Error creating timer recalc semaphore\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    timerThreadAttr.stackSize = 10 * 1024;  
    timerThreadAttr.priority = IX_OSAL_OS_DEFAULT_THREAD_PRIORITY; /*90*/
    timerThreadAttr.name = ixOsalTimerThreadName;

    ixStatus = ixOsalThreadCreate (&taskId, &timerThreadAttr, (IxOsalVoidFnVoidPtr) timerLoop, NULL);
    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR,
            IX_OSAL_LOG_DEV_STDOUT,
            "timerInit: fail to create timer thread. \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    ixStatus = ixOsalThreadStart (&taskId);

    if (ixStatus != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "Error creating timerLoop task\n", 0, 0, 0, 0, 0, 0);

        return IX_FAIL;
    }

    return IX_SUCCESS;
}

/*
 *  This function allocates a new timer.  It is used by Schedule and 
 *  ScheduleRepeating. 
 */

PRIVATE IX_STATUS
createNewTimer (IxOsalVoidFnVoidPtr func, void *param, UINT32 priority,
    UINT32 interval, BOOL isRepeating, UINT32 * timerId)
{
    UINT32 i;
    IX_STATUS status = IX_SUCCESS;
    int osTicks;
    IxOsalTimeval timeVal;

    /*
     * Check if callback is NULL 
     */
    if (func == NULL)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "client registered a NULL callback function\n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    osTicks = ixOsalSysClockRateGet ();
    /*
     * Figure out how many milisecond per tick and compare against interval 
     */
    if (interval < (UINT32) (1000 / osTicks))
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
    if (status != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "createNewTimer  fail to get semaphore \n", 0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

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
        status = IX_FAIL;
    }
    else
    {
        ixOsalTimers[i].inUse = TRUE;

        IX_OSAL_MS_TO_TIMEVAL (interval, &timeVal);

        ixOsalTimers[i].period = timeVal;
        ixOsalTimers[i].isRepeating = isRepeating;
        ixOsalTimeGet (&(ixOsalTimers[i].expires));
        IX_OSAL_TIME_ADD ((ixOsalTimers[i].expires), (ixOsalTimers[i].period))
            ixOsalTimers[i].priority = priority;
        ixOsalTimers[i].callback = func;
        ixOsalTimers[i].callbackParam = param;
        ixOsalTimers[i].id = *timerId;

        if ((i) >= ixOsalHigestTimeSlotUsed)
        {
            ixOsalHigestTimeSlotUsed = i + 1;
        }

        status = ixOsalSemaphorePost (&ixOsalTimerRecalcSem);
        if (status != IX_SUCCESS)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "createNewTimer:  fail to release semaphore \n",
                0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

    }

    status = ixOsalSemaphorePost (&ixOsalCriticalSectSem);
    if (status != IX_SUCCESS)
    {
        ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
            "createNewTimer:  fail to release semaphore: ixOsalCriticalSectSem \n",
            0, 0, 0, 0, 0, 0);
        return IX_FAIL;
    }

    return status;
}

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

    if (first->priority <= second->priority)
    {
        return first;
    }

    return second;
}

/*
 *  Find the next timer about to go off.  Do this by starting with a time infinitely
 *  far in the future and successively finding better timers (ones that go off 
 *  sooner).
 *
 *  If a timer is found that has already expired, the flag bestTimerExpired is
 *  set.  If another timer is found that has also expired, their respective 
 *  priorities are compared using the function evaluateTimerPriority()
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
            if (!IX_OSAL_TIME_GT (bestTimer->expires, now))
            {
                bestTimerExpired = TRUE;
            }
        }
    }
    return bestTimer;
}

PRIVATE void
timerSleep (IxOsalTimerRec * nextTimer, IxOsalTimeval now)
{
    UINT32 milliseconds;
    IxOsalTimeval temp;

    if (nextTimer == NULL)
    {
        milliseconds = IX_OSAL_WAIT_FOREVER;
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
    }

    /*
     * Note: Status is ignored here, wait intentionally 
     */
    ixOsalSemaphoreWait (&ixOsalTimerRecalcSem, milliseconds);

}

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

/*  
 *  Maximum time spent in a call-back is defined as 100 millisec.  (one 
 *  tenth of a second).
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

int
timerLoop (void)
{
    IxOsalTimeval now;
    IxOsalVoidFnVoidPtr callback = NULL;
    void *callbackParam = NULL;
    IxOsalTimerRec *nextTimer;
    IX_STATUS status;

    while (1)
    {
        /*
         *  This loop catches all cases in a simple way.  If multiple
         *  timers expire together, then lowest will be <=0 until all
         *  have been processed and the queue get won't get invoked.
         */

        status =
            ixOsalSemaphoreWait (&ixOsalCriticalSectSem,
            IX_OSAL_WAIT_FOREVER);
        if (status != IX_SUCCESS)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "timerLoop  fail to get semaphore \n", 0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

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
        if (status != IX_SUCCESS)
        {
            ixOsalLog (IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT,
                "timerLoop  fail to release semaphore \n", 0, 0, 0, 0, 0, 0);
            return IX_FAIL;
        }

        if (callback != NULL)
        {
            callTimerCallback (callback, callbackParam);
        }
        else
        {
            timerSleep (nextTimer, now);
        }
    }
}
