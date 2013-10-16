/**
 * @file    IxErrHdlAccEthNPE.c
 *
 * @author Intel Corporation
 * @date    1-Jan-2006
 *    
 * @brief   This file contains the implementation of the ixErrHdlAcc EthNPE sub-component
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
#include "IxEthAccDataPlane_p.h"
#include "IxErrHdlAccEthNPE_p.h"
#include "IxNpeDl.h"
#include "IxNpeDlImageMgr_p.h"
#include "IxParityENAcc.h"
#include "IxEthAcc.h"
#include "IxEthDB.h"
#include "IxNpeMh.h"
#include "IxQMgr.h"
#include "IxQueueAssignments.h"
#include "IxEthNpe.h"
#include "IxFeatureCtrl.h"

extern IX_STATUS ixNpeDlNpeMgrNpeStop (IxNpeDlNpeId npeId);

/* Private APIs*/
PRIVATE void ixErrHdlAccNPEMHPollThread(void);
PRIVATE IX_STATUS ixErrHdlAccNPEMHPollThreadInit(void);
void ixErrHdlAccNPEMHPollThread(void);
PRIVATE void ixErrHdlAccNPEMHPoll(void);

/* Internal Variables*/
PRIVATE UINT32 ixErrHdlAccMBUFRXFREE[3] = {0};
PRIVATE IxOsalThread ixErrHdlAccNpeMHPollThreadId;
PRIVATE BOOL ixErrHdlAccNpePollThreadRun;

/* Ethernet Functionality ID to RXFREE Root buffer DMEM Address*/
PRIVATE IX_ERRHDLACC_ETHNPE_MBUF_INFO ixErrHdlAccEthNpeRXFREEmBufDMEMAddr[]=
{

#if defined(__ixp46X) || defined(__ixp43X)
/* The following DMEM address is obtained from the RxAHB_MbufPRootBuf 
symbol in the Ethernet NPE Image for IXP46X & IXP400 SW Version 2.3. 
This is platform dependable & IXP400 SW Version dependable. */
 {0x00, 0x1850},  /* Eth NPE Functionality ID 00 Version 2.0*/
 {0x01, 0x1c74},  /* Eth NPE Functionality ID 01 Version 2.0*/
 {0x02, 0x1f30},  /* Eth NPE Functionality ID 02 Version 2.0*/
#endif
 {0xFF, 0xFFFF}   /* Last Entry*/
};

/*
Private Function to Get NPE ID and Functional ID base on the 
NPE ID*/
PRIVATE void ixErrHdlAccNpeIdGetIndex(UINT32 npeID, UINT32* imageID, UINT32* funcID)
{
 UINT32 npeImageID = 0;
 IxNpeDlImageId npeImageStruct;

 /* Obtain the downloaded NPE Image ID*/
 ixNpeDlLoadedImageGet(npeID, &npeImageStruct);
 
 /* Construct the imageID*/
 npeImageID = (npeImageStruct.deviceId<<IX_NPEDL_IMAGEID_DEVICEID_OFFSET) +
             (npeImageStruct.npeId<<IX_NPEDL_IMAGEID_NPEID_OFFSET) + 
             (npeImageStruct.functionalityId<<IX_NPEDL_IMAGEID_FUNCTIONID_OFFSET)+
             (npeImageStruct.major<<IX_NPEDL_IMAGEID_MAJOR_OFFSET)+
             (npeImageStruct.minor<<IX_NPEDL_IMAGEID_MINOR_OFFSET);
 
 if(funcID != NULL)
 {
  *funcID = (npeImageStruct.functionalityId)&(IX_ERRHDLACC_NPEDL_FUNC_ID_MASK);
 }
 if(imageID != NULL)
 {
  *imageID = npeImageID;
 }
 return;
}

/*
 Private Function to Restore the mBUF (RXFREE)
 to the RXFREE Queue
*/
PRIVATE IX_STATUS ixErrHdlAccMbufTryRestore(UINT32 npeID)
{
 UINT32 retryLoopCount, mBufAddr, irqlock, rxFreeId;
 IX_STATUS status = IX_FAIL;
 static UINT32 npeIDToRXQueueID[]=
 {
  IX_ETH_ACC_RX_FREE_NPEA_Q,
  IX_ETH_ACC_RX_FREE_NPEB_Q,
  IX_ETH_ACC_RX_FREE_NPEC_Q
 };
 irqlock = ixOsalIrqLock(); 

 rxFreeId = npeIDToRXQueueID[npeID];
 /* Check for validity of mBUF pointer that
   was succesfully retrieved from the NPE DMEM*/
 if(ixErrHdlAccMBUFRXFREE[npeID]!=0 
 && (ixErrHdlAccMBUFRXFREE[npeID]&IX_ERRHDLACC_OUT_OF_BOUND_ADDRESS_VALIDITYMASK)
 ==0)
 {
  mBufAddr = (ixErrHdlAccMBUFRXFREE[npeID]&IX_ETHNPE_QM_Q_RXENET_ADDR_MASK);

  /* Retry to write to the QM*/
  for (retryLoopCount = 0; 
       retryLoopCount<IX_ERRHDLACC_MAX_QMGR_RETRY_WRITE; 
       retryLoopCount++)
   {
    /* Write to Queue ID*/
    if(IX_SUCCESS == ixQMgrQWrite(rxFreeId, &mBufAddr))
    {
      status = IX_SUCCESS;
      break;     
    }
   }
 }
 ixOsalIrqUnlock(irqlock);
 return status;
}

/* 
  Private function to retrieve mBUF (RXFREE) from
  the DMEM of the NPE
*/
PRIVATE IX_STATUS ixErrHdlAccMbufTryRetrieve(UINT32 npeID)
{
 IxParityENAccParityErrorContextMessage pENContext;
 IxParityENAccStatus pENStatus;
 UINT32 data=0, funcId, rxDMemByteAddr, loopCount = 0;
 /* 
 We should not try to read DMEM if
 DMEM Parity error occurs
 */
 static UINT32 errorInvalidCheckTable[] =
 {
  IX_PARITYENACC_NPE_A_DMEM,
  IX_PARITYENACC_NPE_B_DMEM,
  IX_PARITYENACC_NPE_C_DMEM
 };
    
 ixErrHdlAccMBUFRXFREE[npeID] = 0;
 
 /* 
 Get the NPE error type source 
 (AHB Error, DMEM or IMEM parity error)
 */
 pENStatus = ixParityENAccNPEParityErrorCheck (npeID, &pENContext);
 if (IX_PARITYENACC_SUCCESS != pENStatus
 || errorInvalidCheckTable[npeID] == pENContext.pecParitySource
 || IX_FAIL == ixNpeDlNpeMgrNpeStop(npeID))
 {
  
  return IX_FAIL;
 }
 
/* Obtain the functional ID*/
 ixErrHdlAccNpeIdGetIndex(npeID, NULL, &funcId);
 while(1)
 {
    /* Obtain the DMEM Address for the RXFREE mBUF
       base on the functional ID*/
    if( (ixErrHdlAccEthNpeRXFREEmBufDMEMAddr[loopCount].funcId) 
      == 0xFF)
    {
      /*Invalid DMEM Address found. 
       Clear the parity Error interrupt condition*/
      ixParityENAccParityErrorInterruptClear (&pENContext);
      return IX_FAIL;
    }
    
    if( (ixErrHdlAccEthNpeRXFREEmBufDMEMAddr[loopCount].funcId) 
      == funcId)
    {
      /* Valid DMEM Address found*/
      break;
    }
    loopCount++;
 }  
 
 rxDMemByteAddr = ixErrHdlAccEthNpeRXFREEmBufDMEMAddr[loopCount].mBufRXRootAddr;
  /* Read DMEM*/
 data  = (UINT32)ixNpeDlDataMemRead(npeID, rxDMemByteAddr>>2);
 
 /*Store the value of the mBUF pointer address*/
 ixErrHdlAccMBUFRXFREE[npeID] = data;
 
 /* Clear the ixParityENAcc interrupt error status*/
 ixParityENAccParityErrorInterruptClear (&pENContext);
 return IX_SUCCESS;
}

/*
 This function must be called in the ISR
*/
PUBLIC IX_STATUS ixErrHdlAccEthNPEHandleISR(ixErrHdlAccEvent* eventStruct)
{
 UINT32 npeID;

 npeID = eventStruct->priv;
 if( (eventStruct->lastEvtStatus <IX_ERRHDLACC_EVT_IDLE))
 {
  /*Error, 2 concurrent NPE Error events occuring*/
  IX_ERRHDLACC_FATAL_LOG("\n ixErrHdlAccEthNPEHandleISR: Error due to:"
                        " Illegal Call, 2 or more request on an"
                        " already serviced NPE HW or Call-back not called!",
                        0,0,0,0,0,0);
  return IX_FAIL;
 }
 
/* Try to retrieve missing mBUFs that may fail*/
 ixErrHdlAccMbufTryRetrieve(npeID);
         
 /* Reset the NPE core to clear the NPE Error condition*/
 ixErrHdlAccNPEReset(npeID);     

 /* Request to stop Eternet data path
    Ethernet AQM Read/write Halt*/
 ixEthAccStopRequest();

 /* Make sure the ixEthDB Event processing thread is 
    disabled*/
 ixEthDBEventProcessorPauseModeSet(TRUE);

 /* Next Recovery state is in Wait Mode*/
 eventStruct->lastEvtStatus = IX_ERRHDLACC_EVT_WAIT;

 return IX_SUCCESS; 
}

PUBLIC void ixErrHdlAccEthNPERecoveryJob(ixErrHdlAccEvent* eventStruct)
{

 UINT32 npeID, npeImageID = 0, ethPort, irqlock;
 
 irqlock = ixOsalIrqLock();

 npeID = eventStruct->priv;

 /* Obtain Ethernet Port ID from the NPE Id*/
 ethPort = IX_ETHNPE_NODE_AND_PORT_TO_PHYSICAL_ID(npeID, 0);

 /* Recovery is in Run mode*/
 eventStruct->lastEvtStatus = IX_ERRHDLACC_EVT_RUN;
 
 /* Make sure the ixEthDB Event processing thread is 
    disabled*/
 ixEthDBEventProcessorPauseModeSet(TRUE);

 ixOsalIrqUnlock(irqlock);
 /* Restore mBUF to the RX Queue*/

 if(ixErrHdlAccMbufTryRestore(npeID) == IX_FAIL)
 {
  eventStruct->numRxMBufLost++;
 }

 /* Obtain NPE Image ID base on the NPE ID*/
 ixErrHdlAccNpeIdGetIndex(npeID, &npeImageID, NULL);


 /* Begin NPE Polling task*/
 ixErrHdlAccNPEMHPollThreadInit();

 /* Download and start the NPE with the previous
    downloaded image*/
 
 if(ixNpeDlNpeInitAndStart(npeImageID) != IX_SUCCESS)
 {
   IX_ERRHDLACC_ETH_RELOAD_ABORT(eventStruct, 
   IX_ERRHDLACC_EVT_NPE_DOWNLOAD_FAIL);
   IX_ERRHDLACC_FATAL_LOG("ixErrHdlAccEthNPERecoveryJob:" 
                          "ixNpeDlNpeInitAndStart returns Failure.",
                          0, 0, 0, 0, 0, 0);
   return;
 }
 /* Update NPE Core OUTFIFO interrupt enabling*/
 ixNpeMhConfigStateRestore(npeID);

 /*Restore the Priority Table to the NPE, restore dynamic RX configuration
   to the NPE after Reset*/
 ixErrHdlAccNPEMHPoll();

 if(IX_ETH_DB_SUCCESS!=ixEthDBPriorityMappingTableUpdate(ethPort))
 {
     IX_ERRHDLACC_ETH_RELOAD_ABORT(eventStruct, 
     IX_ERRHDLACC_EVT_NPE_ETH_PRIORITYTABLE_UPDATE_FAIL);
     IX_ERRHDLACC_FATAL_LOG("\n ixErrHdlAccEthNPERecoveryJob:"
                            "ixEthDBPriorityMappingTableUpdate returns Failure.",
                            0, 0, 0, 0, 0, 0);
     return;
 }  
 ixErrHdlAccNPEMHPoll();

 /* Restore ixEthDB Features to the reset NPE*/
 if( IX_ETH_DB_SUCCESS != ixEthDBFeatureStatesRestore(ethPort))
 {
   IX_ERRHDLACC_ETH_RELOAD_ABORT(eventStruct, 
   IX_ERRHDLACC_EVT_NPE_ETH_FEATUREUPDATE_FAIL);
   IX_ERRHDLACC_FATAL_LOG("ixErrHdlAccEthNPERecoveryJob:"
                          "ixEthDBFeatureStatesRestore returns Failure.",
                          0, 0, 0, 0, 0, 0);
   return;
 }
 ixErrHdlAccNPEMHPoll();

 irqlock = ixOsalIrqLock();
 /* Force AQM Conditonal Event flag update*/
 if( IX_ETH_ACC_SUCCESS != ixEthAccQMStatusUpdate(ethPort))
 {
   ixOsalIrqUnlock(irqlock);
   IX_ERRHDLACC_ETH_RELOAD_ABORT(eventStruct, 
   IX_ERRHDLACC_EVT_NPE_ETH_AQMQUEUEUPDATE_FAIL);
   IX_ERRHDLACC_FATAL_LOG("ixErrHdlAccEthNPERecoveryJob:"
                          "ixEthAccQMStatusUpdate returns Failure.",
                          0, 0, 0, 0, 0, 0);
   return;    
 }
 /* Restore MAC configurations*/
 ixOsalIrqUnlock(irqlock);
 
 if(IX_ETH_ACC_SUCCESS != ixEthAccMacStateRestore(ethPort))
 {
   IX_ERRHDLACC_ETH_RELOAD_ABORT(eventStruct, 
   IX_ERRHDLACC_EVT_NPE_ETHMAC_REUPDATE_FAIL);
   IX_ERRHDLACC_FATAL_LOG("ixErrHdlAccEthNPERecoveryJob:"
                          "ixEthAccMacStateRestore returns Failure.",
                          0, 0, 0, 0, 0, 0);
   return;    
 }
 
 irqlock = ixOsalIrqLock();
 
 /* Re-establish TX and RX of the Ethernet data path*/
 IX_ERRHDLACC_ETH_RELOAD_DONE(eventStruct);

 /* Restore Parity Control enable bit*/
 ixParityENAccParityNPEConfigReUpdate(npeID);
 ixOsalIrqUnlock(irqlock);
 return;
}
PRIVATE IX_STATUS ixErrHdlAccNPEMHPollThreadInit(void)
{
 IxOsalThreadAttr threadAttr;
 char *pThreadName = "NPEMH Polling thread(Recovery)";
 ixErrHdlAccNpePollThreadRun = TRUE;
 /* Set attribute of thread */
 threadAttr.name      = pThreadName;
 threadAttr.stackSize = 0; /* Forced to use Default stack size*/
 threadAttr.priority  = IX_ERRHDLACC_NPEMHPOLL_THREAD_PRIORITY;

 if (IX_SUCCESS !=
     ixOsalThreadCreate (
                        &ixErrHdlAccNpeMHPollThreadId,
                        &threadAttr,
                        (IxOsalVoidFnVoidPtr) ixErrHdlAccNPEMHPollThread,
                        NULL))
 {
    return IX_FAIL;
 }
 return ixOsalThreadStart(&ixErrHdlAccNpeMHPollThreadId);
}
/*
Fast NPEMH polling Thread during recovery
*/
PRIVATE void ixErrHdlAccNPEMHPoll(void)
{
  UINT32 irqlock;
  irqlock = ixOsalIrqLock();
  /* NPE Message OutFIFO Poll (receive) for all NPE's */
  ixNpeMhMessagesReceive(IX_NPEDL_NPEID_NPEA);
  /* NPE B is not present in IXP43X */
  if (IX_FEATURE_CTRL_DEVICE_TYPE_IXP43X != ixFeatureCtrlDeviceRead())
  {
      ixNpeMhMessagesReceive(IX_NPEDL_NPEID_NPEB);
  }
  ixNpeMhMessagesReceive(IX_NPEDL_NPEID_NPEC);
  ixOsalIrqUnlock(irqlock);
  return;
}
/*
Fast NPEMH polling Thread during recovery
*/
PRIVATE void ixErrHdlAccNPEMHPollThread(void)
{
  while(ixErrHdlAccNpePollThreadRun)
  {
   /* npeMH Polling of Out FIFO (Receive)*/
		 ixErrHdlAccNPEMHPoll();
   /* De-schedule*/
   ixOsalYield();
  }
 return;
}



