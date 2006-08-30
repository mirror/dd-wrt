/**
* IxPerfProfAccXcycle.c
*
 * @par 
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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
*
* C file for Xcycle for the Perf Prof component (IxPerfProfAcc)
*
* -- Description --
* The Xcycle tool provides a measurement of available cycles during high
* load versus measurement taken during zero or minimum load.
*
* The user needs to first perform a calibration during zero or minimum load.
* A measurement of time (in APB clock cycles ) needed to perform a known
* amount of CPU work is taken. This measurement is known as the
* baseline. During baselining, the task that performs
* the work is put into highest priority and interrupts
* are disabled for that period. The time needed to perform this amount of
* CPU work is expect to be shortest possible.
*
* After calibration, users can then load program that they like to profile
* into XScale. As the programs run in steady state, the Xcycle measurement
* tool can be started. During measurement, this tool simply creates a
* task of lowest priority possible.
* User can choose up to IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS
* measurements to run. Each measurement will perform the same amount of
* CPU work as the baseline process. It is assumed that the CPU cycles given
* to this lowest priority task are leftover from other processes.
* Naturally, the time needed for each measurement will be more than the
* baseline. The percentage utilization is simply taken as the ratio of
* baseline vs time for each measurement in percentage.
*
* The results will be return in average, minimum and maximum utilization
* for the measurements.
*
*
*/

#include "IxPerfProfAccXcycle_p.h"
#include "IxPerfProfAcc_p.h"
#include "IxOsal.h"

#ifdef __linuxapp
/*
 * Linux app specific include files
 */
#include "hwprbk.h"         /* Driver to access APB timer */
#include <sys/types.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <stdio.h>

/*
 * If not Linux App
 */
#else

/*
 * Include tasklib.h in VxWorks to silence warning.
 * Note. tasklib.h is included in IxOsal.h.
 */
#ifdef __vxworks
#include <taskLib.h>
#endif

#endif /* ifdef __linuxapp */

/*
 * Static variables defined here
 */

/* Flag to send stop signal to measurement */
static BOOL ixPerfProfAccXcycleStopMeasurement = TRUE ;

/* Flag to indicate if measurement is in progress */
static BOOL ixPerfProfAccXcycleMeasurementInProgress = FALSE;

/* Record number of measurements */
static UINT32 ixPerfProfAccTotalMeasurementMade = 0 ;

/* Results storage */
static IxPerfProfAccXcycleMeasuredResults ixPerfProfAccXcycleResults
        [IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS] ;

/* Baseline value */
static UINT32 ixPerfProfAccXcycleCurrentBaseline = 0 ;

/* Remember number of loops per time slice */
static UINT32 ixNumLoopPerTimeSlice = 0 ;

#ifdef __linuxapp
/*
 * The Xcycle tools need to run on Linux Application space because it needs
 * to create user threads. In order to the APB timer, a driver needs to be
 * installed. The driver hwprb.o is required for this purpose.
 */
static int iFileHwprb = 0 ;

/* thread Id for Xcycle Start and Stop operation */
static pthread_t xcycleThreadId; 
#elif defined (__vxworks)
/* thread Id for Xcycle Start and Stop operation */    
IxOsalThread xcycleThreadId;
#endif

INLINE UINT32
ixPerfProfAccXcycleApbTimerGet (void)
{
#ifdef __linuxapp
    UINT32 timerVal;
    if(ioctl(iFileHwprb, HWPRB_GET_TIMER_TICKS, &timerVal) != HWPRB_RET_OK)
    {
        /*
         * Problem with driver encountered.
         * Terminate the application and print error.
         */
        printf ("\nHardware driver error. Cannot access APB timer !\n");
        exit(0);
    }
    return timerVal;
#else
    return (*((volatile unsigned long *)IX_OSAL_IXP400_OSTS_PHYS_BASE));
#endif
} /* end of ixPerfProfAccXcycleApbTimerGet() */

INLINE void
ixPerfProfAccXcycleLoopIter (UINT32 numLoop)
{
    /*
     * This will take approximately 2 cycles per loop
     */
    register int temp=numLoop;
    while (temp > 0 )
    {
        temp--;
    }
} /* end of ixPerfProfAccXcycleLoopIter() */

void
ixPerfProfAccXcycleNumPeriodRun (UINT32 numMeasurementsRequested)
{
    UINT32 i = 0,           /* temporary variable */
           j = 0,           /* temporary variable */
           temp = 0,        /* temporary variable */
           startTime = 0,   /* record time in the begining */
           stopTime = 0,    /* record time in the end */
           totalMeasurementMade = 0 ; /* total runs made */

    ixPerfProfAccTotalMeasurementMade = 0 ;

    /*
     * This for-loop will run continuously if (numMeasurementsRequested==0).
     * Otherwise, if will run until (i == numMeasurementsRequested ).
     * The loop will also terminate if ixPerfProfAccXcycleStopMeasurement
     * is TRUE.
     */
    for (i = 0 ;
        (i < numMeasurementsRequested)||( 0 == numMeasurementsRequested);
        i++ )
    {
        /*
         * Take the initial reading of timer
         */
        startTime = ixPerfProfAccXcycleApbTimerGet ();

        /*
         * Perform IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC times of
         * ixPerfProfAccXcycleLoopIter(ixNumLoopPerTimeSlice). This
         * should take ~1 sec if load is low.
         * If load is high, measurement may take longer but will terminate
         * if more than IX_PERFPROF_ACC_XCYCLE_MAX_TIME_IN_ONE_MEASUREMENT
         * seconds has passed.
         */
        for (j = 0; IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC > j; j++)
        {
            temp = ixNumLoopPerTimeSlice;
            ixPerfProfAccXcycleLoopIter(temp);
            stopTime = ixPerfProfAccXcycleApbTimerGet ();
            /*
             * Break out if it takes more than
             * IX_PERFPROF_ACC_XCYCLE_MAX_TIME_IN_ONE_MEASUREMENT sec for
             * one measurement (one i loop is one measurement ).
             * This prevent the thread from
             * taking too long to complete each measurements. Total number
             * of time slices (one j loop is equal to one time slice )
             * will be less than IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC
             * for this particular measurement.
             */
            if ((stopTime - startTime)>
                (IX_PERFPROF_ACC_XCYCLE_MAX_TIME_IN_ONE_MEASUREMENT *
                IX_PERFPROF_ACC_XCYCLE_TIMER_FREQ))
                break;
        } /* end for (j) loop */

        /*
         * Collect run data for measurement number i
         */
        ixPerfProfAccXcycleResults[i].xcycleCountStart = startTime;
        ixPerfProfAccXcycleResults[i].xcycleCountStop = stopTime;
        ixPerfProfAccXcycleResults[i].totalTimeSlices = j;

        /*
         * Stop measurement on user requests
         */
        if (TRUE==ixPerfProfAccXcycleStopMeasurement)
            break;

        /*
         * Rotate back index i to 0 when it reaches maximum
         * This only applies to case where numMeasurementsRequested is 0
         * Set totalMeasurementMade to possible maximum.
         */
        if ((IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS <= i) &&
            (0==numMeasurementsRequested))
        {
            totalMeasurementMade = IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS;
            i = 0 ;
        }

    } /* end for i loop */

    /*
     * If totalMeasurementMade is zero, then we have not reached
     * maximum of IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS.
     * Record totalMeasurementMade.
     */
    if (0 == totalMeasurementMade )
    {
        totalMeasurementMade = i;
    }

    /*
     * Record total number of measurements performed for computation of
     * result later.
     */
    ixPerfProfAccTotalMeasurementMade = totalMeasurementMade ;

    /*
     * Set ixPerfProfAccXcycleStopMeasurement flag to TRUE so that
     * user trying to stop it gets a warning.
     */
    ixPerfProfAccXcycleStopMeasurement = TRUE;

    /*
     * Set ixPerfProfAccXcycleMeasurementInProgress flag to FALSE.
     * This allows new measurement to be taken or baseline to be re-run.
     */
    ixPerfProfAccXcycleMeasurementInProgress = FALSE ;

    /*
     * Unlock. Allow other utilities to run
     */
    ixPerfProfAccUnlock();

} /* end of ixPerfProfAccXcycleNumPeriodRun() */

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleBaselineRun(
                   UINT32 *numBaselineCycle)
{
    int i,
        priority ;      /* task priority. Value is OS dependent */
    UINT32 interruptLockKey;

    UINT32 temp,
        startTime,      /* used to store start time */
        stopTime,       /* used to store stop time */
        duration;       /* difference between stop and start time */

#ifdef __vxworks
    IxOsalThread threadId;
    IX_STATUS result;
#endif

    /*error check the parameter*/
    if (NULL == numBaselineCycle)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXcycleBaselineRun - numBaselineCycle is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    if (IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS
        == ixPerfProfAccLock())
    {
        return IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS;
    }

    /*
     * If the tool is running, then do not allow baselining to progress
     */
    if (ixPerfProfAccXcycleMeasurementInProgress)
    {
        ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS;
    }

    /*
     * Find out how many loops is needed to to complete
     * 1/IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC seconds
     */
    ixNumLoopPerTimeSlice= ixPerfProfAccXcycleComputeLoopsPerSlice();

    /*
     * Disable interrupts so that no ISR can run. We get all the CPU
     * cycles.
     */
    interruptLockKey = ixOsalIrqLock();

#ifdef __linuxapp
    priority = getpriority(PRIO_PROCESS,0);
    if(0 != setpriority( PRIO_PROCESS, 0,
            IX_PERFPROF_ACC_XCYCLE_LINUXAPP_PRIORITY_HIGHEST )
            )
    {
        ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL;
    }

#else

    result = ixOsalThreadIdGet (&threadId);
    if (IX_SUCCESS != result)
    {
        ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL;
    }
    taskPriorityGet (threadId, &priority);
    result = ixOsalThreadPrioritySet (
            &threadId,
            IX_PERFPROF_ACC_XCYCLE_VXWORKS_PRIORITY_HIGHEST );
    if (IX_SUCCESS != result)
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL;
    }
#endif

    /*
     * Perform a measure of time needed for one measurement without
     * load.
     */
    startTime = ixPerfProfAccXcycleApbTimerGet();
    for (i = 0; IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC > i; i++)
    {
        temp = ixNumLoopPerTimeSlice;
        ixPerfProfAccXcycleLoopIter(temp);
    }
    stopTime = ixPerfProfAccXcycleApbTimerGet();

    /*
     * Rollover situation is handled through the fact that the different
     * between start and stop time is a fraction of 32bit storage
     * The duration of time stored in 32 bits is 65 sec
     * We expect difference between start and stop time to be
     * ~ 1 sec
     */
    duration = stopTime - startTime;

    ixOsalIrqUnlock(interruptLockKey);

#ifdef __linuxapp
    if(setpriority(PRIO_PROCESS, 0, priority) != 0)
    {
        ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_RESTORE_FAIL;
    }
#else /* ifdef __linuxapp */
    /*
     * Restore the calling thread to previous priority
     */
    result = ixOsalThreadPrioritySet (&threadId, priority);

    if (IX_SUCCESS != result)
    {
        ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_RESTORE_FAIL;
    }

#endif /* ifdef __linuxapp */

    ixPerfProfAccXcycleCurrentBaseline = duration;
    *numBaselineCycle = duration;
    ixPerfProfAccUnlock();
    return IX_PERFPROF_ACC_STATUS_SUCCESS ;
} /* end of ixPerfProfAccXcycleBaselineRun() */

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleStart (UINT32 numMeasurementsRequested)
{

#ifdef __linuxapp
    UINT32 result;      /* result of thread creation */
    int priority;       /* used to remember priority of current process */
    pthread_attr_t threadAttribute; /* used to setup thread attribute */
#elif defined (__vxworks)
    IxOsalThreadAttr threadAttr;
    IX_STATUS retStatus;
#endif /* ifdef __linuxapp */

    ixPerfProfAccXcycleStopMeasurement = FALSE;

    /*
     * Check if baseline had been run. If not, terminate.
     */
    if (0 == ixPerfProfAccXcycleCurrentBaseline)
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE;
    }

    /*
     * Range checking for numMeasurementsRequested
     */
    if (IX_PERFPROF_ACC_XCYCLE_MAX_NUM_OF_MEASUREMENTS < numMeasurementsRequested)
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_REQUEST_OUT_OF_RANGE;
    }

    /*
     * Check if Xcycle is already running, do not allow another start
     * if Xcycle is running.
     */
    if (ixPerfProfAccXcycleMeasurementInProgress)
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS;
    }

    /*
     * Xcycle is not running, set the flag to indicate that
     * a measurement is in progress
     */
    else
    {
        ixPerfProfAccXcycleMeasurementInProgress = TRUE ;
    }

    /*
     * Check to see if any other PMU utilities besides Xcycle is running
     * If yes, we abort to avoid inaccurate results due to load introduce
     * by other tools.
     */
    if (IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS
        == ixPerfProfAccLock())
    {
        return IX_PERFPROF_ACC_STATUS_ANOTHER_UTIL_IN_PROGRESS;
    }

#ifdef __linuxapp
    /*
     * Set current priority to lowest possible
     */
    priority = getpriority(PRIO_PROCESS,0);
    if(0 != setpriority (PRIO_PROCESS,
                         0,
                         IX_PERFPROF_ACC_XCYCLE_LINUXAPP_PRIORITY_LOWEST))
    {
        ixPerfProfAccUnlock();
        ixPerfProfAccXcycleMeasurementInProgress = FALSE ;
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL;
    }

    /*
     * Preparing to create a new thread of ixPerfProfAccXcycleNumPeriodRun()
     * Start with setting thread attribute with state DETACHED
     */
    result = pthread_attr_init (&threadAttribute);
    result |= pthread_attr_setdetachstate (&threadAttribute,
                                            PTHREAD_CREATE_DETACHED);
    result |= pthread_create (&xcycleThreadId,
                            &threadAttribute,
                            (void *) &ixPerfProfAccXcycleNumPeriodRun ,
                            (void *)numMeasurementsRequested);
    if (0 != result)
    {
        ixPerfProfAccUnlock();
        ixPerfProfAccXcycleMeasurementInProgress = FALSE ;
        return  IX_PERFPROF_ACC_STATUS_XCYCLE_THREAD_CREATE_FAIL;
    }/* end of if (result) */

    /*
     * Successful in creating a new thread with lowest priority.
     * Restore priority of calling thread to original level
     */
    if (0 != setpriority (PRIO_PROCESS, 0, priority))
    {
        ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_XCYCLE_PRIORITY_SET_FAIL;
    }
#else /* ifdef __linuxapp */

    /*
     * Create a new thread of ixPerfProfAccXcycleNumPeriodRun()
     * Set the priority level of new thread to lowest possible
     * If fail, set ixPerfProfAccXcycleMeasurementInProgress
     * back to FALSE and return error.
     */
    threadAttr.name = "PerfProf Xcycle thread";
    threadAttr.stackSize = 0;
    threadAttr.priority = IX_PERFPROF_ACC_XCYCLE_VXWORKS_PRIORITY_LOWEST;
    if ((retStatus = ixOsalThreadCreate(
	           &xcycleThreadId,
		   &threadAttr, 
		   (IxOsalVoidFnVoidPtr) ixPerfProfAccXcycleNumPeriodRun,
		   (void*) numMeasurementsRequested)) ==
	  IX_SUCCESS)

    {
	if ((retStatus = ixOsalThreadStart(&xcycleThreadId)) != IX_SUCCESS)
	{
	    ixPerfProfAccUnlock();
	    ixPerfProfAccXcycleMeasurementInProgress = FALSE;
	    return  IX_PERFPROF_ACC_STATUS_XCYCLE_THREAD_CREATE_FAIL;
	}
    }	
    else
    {
        ixPerfProfAccUnlock();
        ixPerfProfAccXcycleMeasurementInProgress = FALSE ;
        return  IX_PERFPROF_ACC_STATUS_XCYCLE_THREAD_CREATE_FAIL;
    } /* end of if (result) */

#endif /* ifdef __linuxapp */

    return IX_PERFPROF_ACC_STATUS_SUCCESS;
} /* end of ixPerfProfAccXcycleStart() */

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleStop (void)
{
    if (ixPerfProfAccXcycleStopMeasurement == FALSE )
    {
        ixPerfProfAccXcycleStopMeasurement = TRUE;
		ixPerfProfAccUnlock();
        return IX_PERFPROF_ACC_STATUS_SUCCESS;
    }
    else
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_NOT_RUNNING;
    }

} /* end of ixPerfProfAccXcycleStop() */

UINT32
ixPerfProfAccXcycleComputeLoopsPerSlice (void)
{
    UINT32  startTime,          /* used to store start time */
            stopTime,           /* used to store stop time */
            ulLoopsPerTimeSlice,/* unsigned long to return results */
            fixNumLoop = IX_PERFPROF_ACC_XCYCLE_SEED_FOR_BASELINE;

    float   timeInSeconds,
            numLoopsPerSecond,
            numLoopsPerTimeSlice;

    startTime = ixPerfProfAccXcycleApbTimerGet ();

    /*
     * Run for IX_PERFPROF_ACC_XCYCLE_SEED_FOR_BASELINE times
     */
    ixPerfProfAccXcycleLoopIter (fixNumLoop);

    stopTime = ixPerfProfAccXcycleApbTimerGet ();

    /*
     * It takes (stopTime-startTime) to complete
     * IX_PERFPROF_ACC_XCYCLE_SEED_FOR_BASELINE iterations
     * Compute number of loops needed for 1 sec divided by
     * IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC
     */
    timeInSeconds = ((float) (stopTime - startTime))/
                    ((float)IX_PERFPROF_ACC_XCYCLE_TIMER_FREQ);
    numLoopsPerSecond = ((float)fixNumLoop)/timeInSeconds;
    numLoopsPerTimeSlice = numLoopsPerSecond /
                        IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC;
    ulLoopsPerTimeSlice = (UINT32) numLoopsPerTimeSlice;
    return (ulLoopsPerTimeSlice);
} /* end of ixPerfProfAccXcycleComputeLoopsPerSlice */

PUBLIC IxPerfProfAccStatus
ixPerfProfAccXcycleResultsGet (
                   IxPerfProfAccXcycleResults *xcycleResult)
{
    UINT32 i ;
    UINT32 duration ;
    float fPercentage,
          fPercentageMax = 0 ,
          fPercentageMin= 1000,
          fPercentageTotal=0 ;

    /*error check the parameter*/
    if (NULL == xcycleResult)
    {
	/* report the error */ 
        IX_PERFPROF_ACC_LOG(
            IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDERR,
            "ixPerfProfAccXcycleResultsGet - xcycleResult is invalid\n",
            0, 0, 0, 0, 0, 0);
	/* return error */
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    if (0 == ixPerfProfAccXcycleCurrentBaseline)
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_NO_BASELINE;
    }

    if (ixPerfProfAccXcycleMeasurementInProgress)
    {
        return IX_PERFPROF_ACC_STATUS_XCYCLE_MEASUREMENT_IN_PROGRESS;
    }

    /*
     * No measurement has been performed
     */
    if (0 == ixPerfProfAccTotalMeasurementMade)
    {
        return IX_PERFPROF_ACC_STATUS_FAIL;
    }

    /*
     * Compute average, minimum and maximum percentage from measurement
     * made.
     */
    for (i = 0 ; i < ixPerfProfAccTotalMeasurementMade ; i ++)
    {
        duration = ixPerfProfAccXcycleResults[i].xcycleCountStop -
            ixPerfProfAccXcycleResults[i].xcycleCountStart;

        fPercentage = ((float)ixPerfProfAccXcycleCurrentBaseline /
            (float)IX_PERFPROF_ACC_XCYCLE_TIME_SLICES_PER_SEC) /
            ((float)duration/ ixPerfProfAccXcycleResults[i].totalTimeSlices)
            * 100.00f;  /* covert to percentage by multiplying with 100 */

        fPercentageTotal += fPercentage;

        if ( fPercentage > fPercentageMax)
        {
            fPercentageMax = fPercentage;
        }

        if (fPercentage < fPercentageMin)
        {
            fPercentageMin = fPercentage;
        }
    } /* end for i loop */

    xcycleResult->maxIdlePercentage = fPercentageMax;
    xcycleResult->minIdlePercentage = fPercentageMin;
    xcycleResult->aveIdlePercentage = fPercentageTotal/
        (float)ixPerfProfAccTotalMeasurementMade;
    xcycleResult->totalMeasurements = ixPerfProfAccTotalMeasurementMade;
    return IX_PERFPROF_ACC_STATUS_SUCCESS;
} /* end of ixPerfProfAccXcycleResultsGet() */

PUBLIC BOOL
ixPerfProfAccXcycleInProgress (void)
{
    return (ixPerfProfAccXcycleMeasurementInProgress);
} /* end of ixPerfProfAccXcycleInProgress() */
