/**
 * @file    IxErrHdlAccControl.c
 *
 * @author Intel Corporation
 * @date    1-Jan-2006
 *
 * @brief   This file contains the implementation of the ixErrHdlAcc control sub-component
 *
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
#include "IxErrHdlAcc.h"
#include "IxErrHdlAcc_p.h"
#include "IxErrHdlAccControl_p.h"
#include "IxNpeDl.h"
#include "IxFeatureCtrl.h"
#include "IxErrHdlAccEthNPE_p.h"

/* Internal Variables*/
PRIVATE UINT32 ixErrHdlAccJobManagerState;
PRIVATE volatile UINT32 ixErrHdlAccThreadKill;
PRIVATE volatile UINT32 ixErrHdlAccJobQueueReadPtr;
PRIVATE volatile UINT32 ixErrHdlAccJobQueueWritePtr;
PRIVATE UINT32 ixErrHdlAccJobTotalQueueEntries;
PRIVATE IxErrHdlAccRecoveryDoneCallback ixErrHdlAccRecoveryDoneCB;
PRIVATE ixErrHdlAccJobQueue ixErrHdlAccJobQueueArray[IX_ERRHDLACC_MAX_QUEUE_JOBS];
PRIVATE IxOsalSemaphore ixErrHdlAccSemaphoreJobQueue;
PRIVATE UINT32 ixErrHdlAccHWCompErrorStatus;
PRIVATE IxOsalThread ErrorHandlerThread;
PRIVATE ixErrHdlAccEvent ixErrHdlAccEventTable[IX_ERRHDLACC_MAX_TOTAL_EVENT];

/* Private Function prototypes*/
PRIVATE void ixErrHdlAccStatsUpdate(IxErrHdlAccRecoveryStatistics* statsDataPtr);
PRIVATE void ixErrHdlAccJobsManager(void);
PRIVATE IX_STATUS ixErrHdlAccJobEnqueue(IxErrHdlAccErrorJob job, ixErrHdlAccEvent* data);
PRIVATE IX_STATUS ixErrHdlAccJobDequeue(IxErrHdlAccErrorJob* job, ixErrHdlAccEvent** data);
PRIVATE IX_STATUS ixErrHdlAccRecoveryJobInit(void);
PRIVATE void ixErrHdlAccInvokeCompletionDone(ixErrHdlAccEvent* eventStruct);
PRIVATE IX_STATUS ixErrHdlAccNPEEventHandler(UINT32 npeID, ixErrHdlAccEvent* eventStruct);

/*
* Index read access table
*/
PRIVATE ixErrHdlAccEvent ixErrHdlAccInitEventTable[]=
{
 {
    IX_ERRHDLACC_NPEA_ERROR,
    &ixErrHdlAccNPEAEventHandler,
    IX_ERRHDLACC_EVT_DISABLE,
    IX_ERRHDLACC_EVT_IDLE,
    0, 0, 0, 0, 0
 }, /* NPEA Error handler*/

 {
    IX_ERRHDLACC_NPEB_ERROR,
    &ixErrHdlAccNPEBEventHandler,
    IX_ERRHDLACC_EVT_DISABLE,
    IX_ERRHDLACC_EVT_IDLE,
    0, 0, 0, 0, 0
 }, /* NPEB Error handler*/

 {
    IX_ERRHDLACC_NPEC_ERROR,
    &ixErrHdlAccNPECEventHandler,
    IX_ERRHDLACC_EVT_DISABLE,
    IX_ERRHDLACC_EVT_IDLE,
    0, 0, 0, 0, 0
 }, /* NPEC Error handler*/

 {
    IX_ERRHDLACC_AQM_ERROR,
    &ixErrHdlAccQMEventHandler,
    IX_ERRHDLACC_EVT_DISABLE,
    IX_ERRHDLACC_EVT_IDLE,
    0, 0, 0, 0, 0
 }, /* AQM Error handler*/

 {
    IX_ERRHDLACC_NO_ERROR,
    NULL,
    IX_ERRHDLACC_EVT_DISABLE,
    IX_ERRHDLACC_EVT_IDLE,
    0, 0, 0, 0, 0
 } /* End of Entry */
};

PRIVATE IxErrHdlAccNPEImageRecoveryInfo
ixErrHdlAccNPEImageRecoveryInfoTable[]= {
/* NPEID, Functional ID  Major_version Minor_Version Recovery_ID*/
#if defined(__ixp46X) || defined(__ixp43X)
/* The following Etherrnet NPE Image are for IXP46X Platform 
& IXP400 SW Version 2.3. This is platform dependable & 
IXP400 SW Version dependable. */
 /* NPE A*/
 {0x00, 0x80, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 {0x00, 0x81, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 {0x00, 0x82, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 /* NPE B*/
 {0x01, 0x00, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 {0x01, 0x01, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 {0x01, 0x02, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 /* NPE C*/
 {0x02, 0x00, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 {0x02, 0x01, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
 {0x02, 0x02, 0x02, 0x01, IX_ERRHDLACC_ETHERNET_NPE},
#endif
 {0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

PRIVATE UINT32 ixErrHdlAccNPEIdToNPEFUSEMask[]=
{
 IX_ERRHDLACC_EXPBUS_FUSE_NPE_A_RESET,
 IX_ERRHDLACC_EXPBUS_FUSE_NPE_B_RESET,
 IX_ERRHDLACC_EXPBUS_FUSE_NPE_C_RESET
};

/*
 * ixErrHdlAccQMEventHandler
 * Brief: AQM Event handler
 */
PUBLIC IX_STATUS ixErrHdlAccQMEventHandler(void)
{
 UINT32 irqLock;
 irqLock = ixOsalIrqLock();
 ixErrHdlAccEventTable[IX_ERRHDLACC_AQM_ERROR].numRequest++;

 if(IX_ERRHDLACC_EVT_DISABLE ==
   ixErrHdlAccEventTable[IX_ERRHDLACC_AQM_ERROR].enable)
 {
  ixOsalIrqUnlock(irqLock);
  IX_ERRHDLACC_WARNING_LOG("\n ixErrHdlAccQMEventHandler: Warning:"
                            "Error Handler is disabled.\n ",
                            0, 0, 0, 0, 0, 0);
  return IX_FAIL;
 }
 ixOsalIrqUnlock(irqLock);
 IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccQMEventHandler:AQM Error handler called. \n");
 
 return IX_SUCCESS;
}

/*
 * ixErrHdlAccQMEventHandler
 * Brief: AQM Event handler
 */
PUBLIC IX_STATUS ixErrHdlAccNPEAEventHandler(void)
{
 IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccNPEAEventHandler:"
                            "NPE A Error handler called. \n");
 return ixErrHdlAccNPEEventHandler(IX_ERRHDLACC_NPEA,
                            &ixErrHdlAccEventTable[IX_ERRHDLACC_NPEA_ERROR]);
}

/*
 * ixErrHdlAccQMEventHandler
 * Brief: NPE B Event handler
 */
PUBLIC IX_STATUS ixErrHdlAccNPEBEventHandler(void)
{
 IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccNPEBEventHandler:"
                            "NPE B Error handler called. \n");
 return ixErrHdlAccNPEEventHandler(IX_ERRHDLACC_NPEB,
                            &ixErrHdlAccEventTable[IX_ERRHDLACC_NPEB_ERROR]);
}

/*
 * ixErrHdlAccNPECEventHandler
 * Brief: NPE C Event handler
 */
PUBLIC IX_STATUS ixErrHdlAccNPECEventHandler(void)
{
 IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccNPECEventHandler:"
                            "NPE C Error handler called. \n");
 return ixErrHdlAccNPEEventHandler(IX_ERRHDLACC_NPEC,
                            &ixErrHdlAccEventTable[IX_ERRHDLACC_NPEC_ERROR]);
}

/*
 * ixErrHdlAccRecoveryTypeGet
 * Brief: To get the recovery type.
 */
PRIVATE UINT32 ixErrHdlAccRecoveryTypeGet(UINT32 npeID)
{
 UINT32  i = 0;
 IxNpeDlImageId imageID;
 UINT32 recoveryId = 0xFF;
 IX_ERRHDLACC_CHECK_NPEID_VALIDITY(npeID, 0xFF);
 if(ixNpeDlLoadedImageGet(npeID, &imageID) == IX_FAIL)
 {
   return 0xFF;
 }

 while(i < IX_ERRHDLACC_MAX_TOTAL_RECOVERY_TYPE )
 {
   if(ixErrHdlAccNPEImageRecoveryInfoTable[i].npeId == 0xFF)
   {
     /* Last entry*/
     recoveryId = 0xFF; /* No recovery ID found*/
     IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccNPEEventHandler:"
                            "Unknown NPE Image \n");
     break;
   }
   /* Check the ImageID to determine if there is 
      an existing recovery handler*/
   if (ixErrHdlAccNPEImageRecoveryInfoTable[i].npeId == imageID.npeId)
   {
      if(imageID.functionalityId
         ==ixErrHdlAccNPEImageRecoveryInfoTable[i].functionalId &&
         imageID.major
         ==ixErrHdlAccNPEImageRecoveryInfoTable[i].majorVersion &&
         imageID.minor
         ==ixErrHdlAccNPEImageRecoveryInfoTable[i].minorVersion
        )
      {
        recoveryId = ixErrHdlAccNPEImageRecoveryInfoTable[i].recoveryId;
        break;
      }
   }
     i++;
 }

 return recoveryId;
}

/*
 * ixErrHdlAccNPEEventHandler
 * Brief: NPE Event handler
 */
PRIVATE IX_STATUS ixErrHdlAccNPEEventHandler(UINT32 npeID, ixErrHdlAccEvent* eventStruct)
{
 UINT32 irqlock;
 IX_STATUS status = IX_SUCCESS;

 if(eventStruct->enable == FALSE)
 {
    IX_ERRHDLACC_WARNING_LOG("\n ixErrHdlAccNPEEventHandler: Warning:"
                            "Error Handler is disabled.\n ",
                            0, 0, 0, 0, 0, 0);
   return IX_FAIL;
 }
 
 /* Invoke call-back if previous event (task) was pre-empted 
    by an interrupt*/
 ixErrHdlAccInvokeCompletionDone(eventStruct);

 irqlock = ixOsalIrqLock();
 eventStruct->numRequest++;
 eventStruct->priv = npeID;
 {
  UINT32 errorCondWord;
  ixErrHdlAccStatusGet(&errorCondWord);
  /* Set the Error condition Flag*/
  errorCondWord |= (0x01<<eventStruct->eventType);
  ixErrHdlAccStatusSet(errorCondWord);
 }
 ixOsalIrqUnlock(irqlock);
 /*Customize error handling specific base on NPE Type */
 switch(ixErrHdlAccRecoveryTypeGet(npeID))
 {
  case IX_ERRHDLACC_ETHERNET_NPE:
       if(IX_SUCCESS == ixErrHdlAccEthNPEHandleISR(eventStruct))
       {
        /* Enqueue the recovery job in the pending queue list*/
        ixErrHdlAccJobEnqueue(ixErrHdlAccEthNPERecoveryJob, eventStruct);
       }
       else
       {
         status = IX_FAIL;
       }
       break;

  default:
       /* Default Reset the NPE */
       ixErrHdlAccNPEReset(npeID);
       IX_ERRHDLACC_WARNING_LOG("\n ixErrHdlAccNPEEventHandler: Warning:"
                               " No Recovery Job handler available.\n"
                               "\n NPE ID = %d has been reset!",                              
                               0, 0, 0, 0, 0, 0);
       status = IX_FAIL;
      break;

 }
 return status;
}

/*
 * ixErrHdlAccInvokeCompletionDone
 * Brief: Invoke the completion done call-back
 */
PRIVATE void ixErrHdlAccInvokeCompletionDone(ixErrHdlAccEvent* eventStructPtr)
{
   UINT32 irqLock;  
  if(eventStructPtr->lastEvtStatus == IX_ERRHDLACC_EVT_WAIT
   ||eventStructPtr->lastEvtStatus == IX_ERRHDLACC_EVT_RUN
   ||eventStructPtr->lastEvtStatus == IX_ERRHDLACC_EVT_IDLE)
  {
    return;
  }

  /* Lock Interrupt*/
  irqLock = ixOsalIrqLock();

 if(IX_ERRHDLACC_EVT_STOP_SUCCESS ==
   eventStructPtr->lastEvtStatus)
 {

   UINT32 status;
   ixErrHdlAccStatusGet(&status);
   /* Clear the Error condition Flag*/
   status&=~(0x01<<eventStructPtr->eventType);
   ixErrHdlAccStatusSet(status);
  }
  /* Call-back sent and end of event job indication*/
  eventStructPtr->lastEvtStatus = IX_ERRHDLACC_EVT_IDLE;
  /* Unlock interrupt*/
  ixOsalIrqUnlock(irqLock);
  /* Always Sent Completion done
     to indicate end of task*/
  if(ixErrHdlAccRecoveryDoneCB!=NULL)
  {
    (*ixErrHdlAccRecoveryDoneCB)(eventStructPtr->eventType);
  }
 return;
}

/*
 * ixErrHdlAccJobsManager
 * Brief: Job manager
 */
PRIVATE void ixErrHdlAccJobsManager(void)
{
 IxErrHdlAccErrorJob jobFunc = NULL;
 ixErrHdlAccEvent* eventStructPtr = NULL;

 while(1)
 {
  /* Blocking function , will wait till there is a job
    enqueued*/
   ixErrHdlAccJobManagerState = IX_ERRHDLACC_JOBMANAGER_WAITING;
   ixOsalSemaphoreWait (&ixErrHdlAccSemaphoreJobQueue, IX_OSAL_WAIT_FOREVER);
   ixErrHdlAccJobDequeue(&jobFunc, &eventStructPtr);
   if(jobFunc!=NULL)
   {
    ixErrHdlAccJobManagerState = IX_ERRHDLACC_JOBMANAGER_RUN;
    (*(jobFunc))(eventStructPtr);
     
     ixErrHdlAccInvokeCompletionDone(eventStructPtr);
   }
   else
   {
     if(IX_ERRHDL_THREAD_KILL==ixErrHdlAccThreadKill)
     { /* Exit Thread*/
       ixErrHdlAccJobManagerState = IX_ERRHDLACC_JOBMANAGER_KILLED;
       return;
     }
     else
     {
      IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccJobsManager:"
                            "Error, Illegal NULL Job Function.\n");
     }
   }
 }
 return;
}

/*
 * ixErrHdlAccRecoveryTaskInit
 * Brief: To initialize the Job manager and create the thread
 */
IX_STATUS ixErrHdlAccRecoveryTaskInit(void)
{
 IxOsalThreadAttr threadAttr;
 char *pThreadName = "Error handler Main thread";
 ixErrHdlAccJobManagerState = IX_ERRHDLACC_JOBMANAGER_INIT;

 if(IX_FAIL == ixErrHdlAccRecoveryJobInit())
 {
    IX_ERRHDLACC_FATAL_LOG("\n ixErrHdlAccRecoveryTaskInit: Fatal Error"
                           " ixErrHdlAccRecoveryJobInit call returned Fail!",
                           0, 0, 0, 0, 0, 0);
   return IX_FAIL;
 }

 /* Set attribute of thread */
 threadAttr.name      = pThreadName;
 threadAttr.stackSize = 0; /* Forced to use Default stack size*/
 threadAttr.priority  = IX_ERRHDLACC_JOBMANAGER_THREAD_PRIORITY;

 if (IX_SUCCESS !=
     ixOsalThreadCreate (
                        &ErrorHandlerThread,
                        &threadAttr,
                        (IxOsalVoidFnVoidPtr) ixErrHdlAccJobsManager,
                        NULL))
 {
     IX_ERRHDLACC_FATAL_LOG("\n ixErrHdlAccRecoveryTaskInit: Fatal Error"
                           " Thread Create Failed!",
                           0, 0, 0, 0, 0, 0);
    return IX_FAIL;
 }

 /* HW Component Error Status*/
 ixErrHdlAccHWCompErrorStatus = 0;

 /* Initialize all events*/
 {
   UINT32 eventCount = 0;
   BOOL lastEventEntryDone = FALSE;
   for (eventCount = 0; eventCount < IX_ERRHDLACC_MAX_TOTAL_EVENT; eventCount++)
   {
    if(FALSE == lastEventEntryDone )
    {
     if(NULL == ixErrHdlAccInitEventTable[eventCount].eventHandler)
     {
      lastEventEntryDone = TRUE;
     }
    ixErrHdlAccEventTable[eventCount] = ixErrHdlAccInitEventTable[eventCount];

    }
    else
    {
      /* Initialize the Event Data structure*/
      ixErrHdlAccEventTable[eventCount].eventType = eventCount;
      ixErrHdlAccEventTable[eventCount].numRequest=
      ixErrHdlAccEventTable[eventCount].numFail   =
      ixErrHdlAccEventTable[eventCount].numPass   =0;
      ixErrHdlAccEventTable[eventCount].eventHandler = NULL;
      ixErrHdlAccEventTable[eventCount].enable = IX_ERRHDLACC_EVT_DISABLE;
      ixErrHdlAccEventTable[eventCount].priv = 0;
      ixErrHdlAccEventTable[eventCount].numRxMBufLost = 0;
    }
   }
 }
 ixErrHdlAccStatisticsClear();
 /* Start the Thread*/
 return ixOsalThreadStart(&ErrorHandlerThread);
}

/*
 * ixErrHdlAccRecoveryTaskUnInit
 * Brief: To unload the task and kill the thread
 */
IX_STATUS ixErrHdlAccRecoveryTaskUnInit(void)
{
 UINT32 tryKillcnt = 0;
 IX_STATUS status = IX_SUCCESS;
 /* Stop the Job Manager Thread*/
 ixErrHdlAccThreadKill = IX_ERRHDL_THREAD_KILL;
 ixOsalSemaphorePost(&ixErrHdlAccSemaphoreJobQueue);

 /* Make sure thread is exit before destroying
    the semaphore*/
 while(tryKillcnt < IX_ERRHDLACC_MAX_TRY_THREADKILL_COUNT)
 {
   if(IX_ERRHDLACC_JOBMANAGER_KILLED == ixErrHdlAccJobManagerState)
   {
     break;
   }
   tryKillcnt++;
   ixOsalSleep(50);
 }
 if(tryKillcnt >= IX_ERRHDLACC_MAX_TRY_THREADKILL_COUNT)
 {
   status = IX_FAIL;
 }else
 {
  /* Destroy semaphores*/
  return ixOsalSemaphoreDestroy(&ixErrHdlAccSemaphoreJobQueue);
 }
 return status;
}

/*
 * ixErrHdlAccJobEnqueue
 * Brief: To enqueue the pending jobs
 */
PRIVATE IX_STATUS ixErrHdlAccJobEnqueue(IxErrHdlAccErrorJob job, ixErrHdlAccEvent* data)
{
 UINT32 intlock;
 intlock = ixOsalIrqLock();
 if(ixErrHdlAccJobTotalQueueEntries >= IX_ERRHDLACC_MAX_QUEUE_JOBS)
 {
   /* Queue Full*/
   IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccJobEnqueue:"
                          "Job Queue Overflow!\n");
   ixOsalIrqUnlock(intlock);
   return IX_FAIL;
 }

 ixErrHdlAccJobQueueArray[ixErrHdlAccJobQueueWritePtr].job = job;
 ixErrHdlAccJobQueueArray[ixErrHdlAccJobQueueWritePtr].data = (void*) data;
 ixErrHdlAccJobQueueWritePtr++;
 ixErrHdlAccJobTotalQueueEntries++;

 if(ixErrHdlAccJobQueueWritePtr >= IX_ERRHDLACC_MAX_QUEUE_JOBS)
 {
  ixErrHdlAccJobQueueWritePtr = 0;
 }
 ixOsalIrqUnlock(intlock);
 /*Semaphore post - starts the thread recovery*/
 ixOsalSemaphorePost(&ixErrHdlAccSemaphoreJobQueue);
 
 return IX_SUCCESS;
}

/*
 * ixErrHdlAccJobDequeue
 * Brief: To Dequeue the pending jobs
 */
PRIVATE IX_STATUS ixErrHdlAccJobDequeue(IxErrHdlAccErrorJob* job, ixErrHdlAccEvent** data)
{
 UINT32 intlock;
 IX_ERRHDLACC_CHECK_NULL_PTR(job);
 IX_ERRHDLACC_CHECK_NULL_PTR(data);

 intlock = ixOsalIrqLock();
 if(ixErrHdlAccJobTotalQueueEntries<=0)
 {
  /* Queue Empty*/
  *job = 0;
  ixOsalIrqUnlock(intlock);
  IX_ERRHDLACC_DEBUG_MSG("\n ixErrHdlAccJobDequeue:"
                         "Job Queue Empty!\n");
  return IX_FAIL;
 }
 *job = ixErrHdlAccJobQueueArray[ixErrHdlAccJobQueueReadPtr].job;
 *data =ixErrHdlAccJobQueueArray[ixErrHdlAccJobQueueReadPtr].data;
 ixErrHdlAccJobQueueArray[ixErrHdlAccJobQueueReadPtr].job = 0;
 ixErrHdlAccJobQueueArray[ixErrHdlAccJobQueueReadPtr].data = 0;

 ixErrHdlAccJobQueueReadPtr++;

 if(ixErrHdlAccJobQueueReadPtr >= IX_ERRHDLACC_MAX_QUEUE_JOBS)
 {
   ixErrHdlAccJobQueueReadPtr = 0;
 }

 ixErrHdlAccJobTotalQueueEntries--;
 ixOsalIrqUnlock(intlock);
 return IX_SUCCESS;
}

/*
 * ixErrHdlAccRecoveryJobInit
 * Brief: To initialize the Job Queue
 */
PRIVATE IX_STATUS ixErrHdlAccRecoveryJobInit(void)
{
 /* Initialize Job Queue data*/
 ixErrHdlAccJobQueueReadPtr = 0;
 ixErrHdlAccJobQueueWritePtr = 0;
 ixErrHdlAccJobTotalQueueEntries = 0;

 ixErrHdlAccThreadKill = IX_ERRHDL_THREAD_NOKILL;

 /* Initialize and create a Semaphore object*/
 return ixOsalSemaphoreInit (&ixErrHdlAccSemaphoreJobQueue, 0);

}

/*
 * ixErrHdlAccErrorHandlerGet
 * Brief: To get the soft-error handlers
 */
PUBLIC IX_STATUS ixErrHdlAccErrorHandlerGet(IxErrHdlAccErrorEventType event, IxErrHdlAccFuncHandler* handler)
{
 IxErrHdlAccFuncHandler funcHandler = NULL;

 IX_ERRHDLACC_CHECK_INIT_DONE();
 IX_ERRHDLACC_CHECK_NULL_PTR(handler);
 IX_ERRHDLACC_EVENT_TYPE_CHECK_VALIDITY(event);
 funcHandler = ixErrHdlAccEventTable[event].eventHandler;
 if(handler!=NULL)
 {
  *handler = funcHandler;
  return (funcHandler!=NULL)?IX_SUCCESS:IX_FAIL;
 }

  return IX_FAIL;
}

/*
Reset the NPE Core
*/
PUBLIC IX_STATUS ixErrHdlAccNPEReset(IxErrHdlAccNPEId npeID)
{
 UINT32 doneCnt = 0, maskResetBit;
 BOOL   isDone = FALSE;
 IxFeatureCtrlReg regVal;
 IX_ERRHDLACC_CHECK_NPEID_VALIDITY(npeID, IX_FAIL);
 maskResetBit = ixErrHdlAccNPEIdToNPEFUSEMask[npeID];
 regVal = ixFeatureCtrlRead();
 /* FUSE On to RESET*/
 regVal|=maskResetBit;
 ixFeatureCtrlWrite(regVal);

 while (++doneCnt < IX_ERRHDLACC_HWRESET_POLL_WAIT_COUNT_MAX)
 {
   regVal = ixFeatureCtrlRead();
   isDone = ( maskResetBit == (regVal & maskResetBit) );

   if(isDone)
   {
     break;
   }
   ixOsalYield();
 }
 doneCnt = 0;
 /* turn OFF the reset of the NPE(s) */
 do {
    regVal = ixFeatureCtrlRead();
    regVal &= ~maskResetBit;
    ixFeatureCtrlWrite  (regVal);
    regVal = ixFeatureCtrlRead();
    if(0 == (regVal & maskResetBit))
    {
     break;
    }
    ixOsalYield();
    doneCnt++;
  }while(doneCnt < IX_ERRHDLACC_HWRESET_POLL_WAIT_COUNT_MAX);

 return IX_SUCCESS;
}
/*
 * ixErrHdlAccStatusSet
 * Brief: To set the status
 */
PUBLIC IX_STATUS ixErrHdlAccStatusSet(UINT32 status)
{
 IX_ERRHDLACC_CHECK_INIT_DONE();
 ixErrHdlAccHWCompErrorStatus = status;
 return IX_SUCCESS;
}
/*
 * ixErrHdlAccStatusGet
 * Brief: To get the status
 */
PUBLIC IX_STATUS ixErrHdlAccStatusGet(UINT32* status)
{
 IX_ERRHDLACC_CHECK_INIT_DONE();
 IX_ERRHDLACC_CHECK_NULL_PTR(status);
 *status = ixErrHdlAccHWCompErrorStatus;
 return IX_SUCCESS;
}
/*
 * ixErrHdlAccCallbackRegister
 * Brief: To register call-back
 */
PUBLIC IX_STATUS ixErrHdlAccCallbackRegister(IxErrHdlAccRecoveryDoneCallback callback)
{
 IX_ERRHDLACC_CHECK_INIT_DONE();
 IX_ERRHDLACC_CHECK_NULL_PTR(callback);
 ixErrHdlAccRecoveryDoneCB = callback;
 return IX_SUCCESS;
}
/*
 * ixErrHdlAccStatisticsGet
 * Brief: To get statistics
 */
PUBLIC IX_STATUS ixErrHdlAccStatisticsGet(IxErrHdlAccRecoveryStatistics* statistics)
{
 IX_ERRHDLACC_CHECK_INIT_DONE();
 IX_ERRHDLACC_CHECK_NULL_PTR(statistics);
 ixErrHdlAccStatsUpdate(statistics);
 return IX_SUCCESS;
}
/*
 * ixErrHdlAccEnableConfigSet
 * Brief: To set configuration
 */
PUBLIC IX_STATUS ixErrHdlAccEnableConfigSet(UINT32 maskWord)
{
 UINT32 shiftCnt, shiftWord;
 IX_ERRHDLACC_CHECK_INIT_DONE();
 for(shiftCnt = 0; shiftCnt<IX_ERRHDLACC_MAX_TOTAL_EVENT;shiftCnt++)
 {
   shiftWord = maskWord;
   if((shiftWord>>shiftCnt)&0x01)
   {
     ixErrHdlAccEventTable[shiftCnt].enable = IX_ERRHDLACC_EVT_ENABLE;
   } 
   else
   {
     ixErrHdlAccEventTable[shiftCnt].enable = IX_ERRHDLACC_EVT_DISABLE;
   }
 }
 return IX_SUCCESS;
}

/*
 * ixErrHdlAccEnableConfigGet
 * Brief: To get configuration
 */
PUBLIC IX_STATUS ixErrHdlAccEnableConfigGet(UINT32* maskWord)
{
 UINT32 numErrorHdl;
 IX_ERRHDLACC_CHECK_INIT_DONE();
 IX_ERRHDLACC_CHECK_NULL_PTR(maskWord);
 
 *maskWord = 0;
 for(numErrorHdl = 0; numErrorHdl < IX_ERRHDLACC_MAX_TOTAL_EVENT; numErrorHdl++)
 {
   if(ixErrHdlAccEventTable[numErrorHdl].enable == IX_ERRHDLACC_EVT_ENABLE)
   {
    *maskWord|=(0x01<<numErrorHdl);
   }
 }
 return IX_SUCCESS;
}

/*
 * ixErrHdlAccStatsUpdate
 * Brief: To Update the statistics
 */
PRIVATE void ixErrHdlAccStatsUpdate(IxErrHdlAccRecoveryStatistics* statsDataPtr)
{
 statsDataPtr->totalNPEAEvent
 = ixErrHdlAccEventTable[IX_ERRHDLACC_NPEA_ERROR].numRequest;

 statsDataPtr->totalNPEBEvent
 = ixErrHdlAccEventTable[IX_ERRHDLACC_NPEB_ERROR].numRequest;

 statsDataPtr->totalNPECEvent
 = ixErrHdlAccEventTable[IX_ERRHDLACC_NPEC_ERROR].numRequest;

 statsDataPtr->totalAQMEvent
 = ixErrHdlAccEventTable[IX_ERRHDLACC_AQM_ERROR].numRequest;

 statsDataPtr->totalNPEARecoveryPass
 = ixErrHdlAccEventTable[IX_ERRHDLACC_NPEA_ERROR].numPass;

 statsDataPtr->totalNPEBRecoveryPass
 = ixErrHdlAccEventTable[IX_ERRHDLACC_NPEB_ERROR].numPass;

 statsDataPtr->totalNPECRecoveryPass
 = ixErrHdlAccEventTable[IX_ERRHDLACC_NPEC_ERROR].numPass;

  statsDataPtr->totalAQMRecoveryPass
 = ixErrHdlAccEventTable[IX_ERRHDLACC_AQM_ERROR].numPass;
 return;
}
/*
 * ixErrHdlAccStatisticsClear
 * Brief: To Clear the statistics
 */
PUBLIC void ixErrHdlAccStatisticsClear(void)
{
 /* Initialize Statistics*/
 ixErrHdlAccEventTable[IX_ERRHDLACC_NPEA_ERROR].numRequest = 0;
 ixErrHdlAccEventTable[IX_ERRHDLACC_NPEB_ERROR].numRequest = 0;
 ixErrHdlAccEventTable[IX_ERRHDLACC_NPEC_ERROR].numRequest = 0;
 ixErrHdlAccEventTable[IX_ERRHDLACC_NPEA_ERROR].numPass    = 0;
 ixErrHdlAccEventTable[IX_ERRHDLACC_NPEB_ERROR].numPass    = 0;
 ixErrHdlAccEventTable[IX_ERRHDLACC_NPEC_ERROR].numPass    = 0;
 return;
}
/*
 * ixErrHdlAccStatsShow
 * Brief: To display the statistics
 */
PUBLIC void ixErrHdlAccStatisticsShow(void)
{
 IxErrHdlAccRecoveryStatistics stats;
 ixErrHdlAccStatsUpdate(&stats);

 printf("\n\n ixErrHdlAccControlStatsShow:Events Statistics"
        "\n----------------------------------------------"
        "\n Total requested NPEA Error event   = %d"
        "\n Total requested NPEB Error event   = %d"
        "\n Total requested NPEC Error event   = %d"
        "\n Total requested AQM Error event    = %d"
        "\n Total passed NPEA Error event      = %d"
        "\n Total passed NPEB Error event      = %d"
        "\n Total passed NPEC Error event      = %d",
        stats.totalNPEAEvent,
        stats.totalNPEBEvent,
        stats.totalNPECEvent,
        stats.totalAQMEvent,
        stats.totalNPEARecoveryPass,
        stats.totalNPEBRecoveryPass,
        stats.totalNPECRecoveryPass);

#ifdef IX_ERRHDLACC_DEBUG
 printf ("\n Total NPEA Recovery Fail  = %d"
         "\n Total NPEB Recovery Fail  = %d"
         "\n  Total NPEC Recovery Fail = %d"
         "\n\n",
        ixErrHdlAccEventTable[IX_ERRHDLACC_NPEA_ERROR].numFail,
        ixErrHdlAccEventTable[IX_ERRHDLACC_NPEB_ERROR].numFail,
        ixErrHdlAccEventTable[IX_ERRHDLACC_NPEC_ERROR].numFail);

 {
   
   UINT32 eventCount;
   printf("\n Event Data Structure Print - out "
          "\n --------------------------------");
   for (eventCount = 0; eventCount < IX_ERRHDLACC_MAX_ERROR_EVENT_SUPPORTED; eventCount++)
   {
     printf ("\n Data Field 1(eventType)     = %d"
             "\n Data Field 2(eventHandler)  = 0x%x"
             "\n Data Field 3(enable)        = %d"
             "\n Data Field 4(lastEvtStatus) = %d"
             "\n Data Field 5(numRequest)    = %d"
             "\n Data Field 6(numFail)       = %d"
             "\n Data Field 7(numPass)       = %d"
             "\n Data Field 8(priv)          = %d"
             "\n Data Field 9(RX mbuf lost)  = %d\n",
             ixErrHdlAccEventTable[eventCount].eventType,
             ixErrHdlAccEventTable[eventCount].eventHandler,
             ixErrHdlAccEventTable[eventCount].enable,
             ixErrHdlAccEventTable[eventCount].lastEvtStatus,
             ixErrHdlAccEventTable[eventCount].numRequest,
             ixErrHdlAccEventTable[eventCount].numFail,
             ixErrHdlAccEventTable[eventCount].numPass,
             ixErrHdlAccEventTable[eventCount].priv,
             ixErrHdlAccEventTable[eventCount].numRxMBufLost);
   }
 }
#endif
 return;
}

