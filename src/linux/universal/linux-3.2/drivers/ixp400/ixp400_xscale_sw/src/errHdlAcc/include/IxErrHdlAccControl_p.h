/**
 * @file    IxErrHdlAccControl_p.h
 *
 * @author Intel Corporation
 * @date    1-Jan-2006
 *    
 * @brief   This file contains the implementation 
 * of the ixErrHdlAcc Control and Events sub-component
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
#ifndef IxErrHdlAccControl_p_H
#define IxErrHdlAccControl_p_H

typedef struct{
  IxErrHdlAccErrorEventType eventType;
  IxErrHdlAccFuncHandler eventHandler;
  BOOL enable;  /* enable - TRUE, error handler is enabled*/
  IX_ERRHDLACC_EVT_STATUS lastEvtStatus;
  
  /*Statistical data used for measurement*/
  UINT32 numRequest; /* Number of times the handler is called*/
  UINT32 numFail;    /* Number of failures*/
  UINT32 numPass;   /* Number of Pass*/
  UINT32 numRxMBufLost; /* Number of mBUF RX lost*/
  
  /* Private data*/
  UINT32 priv;       
}ixErrHdlAccEvent;

/* Function pointer definition for Job Function handler*/
typedef void (*IxErrHdlAccErrorJob)(ixErrHdlAccEvent*);

/* Binary 
   NPE functional types defines*/
typedef enum{
 IX_ERRHDLACC_ETHERNET_NPE,
 IX_ERRHDLACC_ATM_NPE,
 IX_ERRHDLACC_HSS_NPE, 
 IX_ERRHDLACC_CRYPTO_NPE,
 IX_ERRHDLACC_ETHERNET_HSS_NPE,
 IX_ERRHDLACC_UNKNOWN_NPE = 0xFF
}IxErrHdlAccTargetRecoveryType;

/* 
   Enum definition
   of the Job Manager State
*/
typedef enum{
 IX_ERRHDLACC_JOBMANAGER_INIT,
 IX_ERRHDLACC_JOBMANAGER_WAITING,
 IX_ERRHDLACC_JOBMANAGER_RUN,
 IX_ERRHDLACC_JOBMANAGER_KILLED
}IxErrHdlAccJobManagerState;


#define  IX_ERRHDL_THREAD_NOKILL                 (0)
#define  IX_ERRHDL_THREAD_KILL                   (1)

#define IX_ERRHDLACC_MAX_ERROR_EVENT_SUPPORTED   (4)
#define IX_ERRHDLACC_MAX_TRY_THREADKILL_COUNT    (1000) /* Maximum tries to kill the thread*/
#define IX_ERRHDLACC_MAX_TOTAL_RECOVERY_TYPE     (32)
#define IX_ERRHDLACC_MAX_TOTAL_EVENT             (32) /* Maximum Total Error events supported*/
#define IX_ERRHDLACC_MAX_QUEUE_JOBS              (IX_ERRHDLACC_MAX_ERROR_EVENT_SUPPORTED) /* Maximum Job queue depth*/
#define IX_ERRHDLACC_HWRESET_POLL_WAIT_COUNT_MAX (1000) /* Maximum Polling Loop counts*/
#define IX_ERRHDLACC_CHECK_INIT_DONE() if (ixErrHdlAccInitDone == FALSE) {return IX_FAIL;}

#define IX_ERRHDLACC_JOBMANAGER_THREAD_PRIORITY  (30)          
#define IX_ERRHDLACC_EXPBUS_FUSE_NPE_A_RESET  (0x00000001 << 11) /* Fuse NPE A Mask Bit*/
#define IX_ERRHDLACC_EXPBUS_FUSE_NPE_B_RESET  (0x00000001 << 12) /* Fuse NPE B Mask Bit*/
#define IX_ERRHDLACC_EXPBUS_FUSE_NPE_C_RESET  (0x00000001 << 13) /* Fuse NPE C Mask Bit*/

/*
 Job Queue Data Structure
*/
typedef struct{
  ixErrHdlAccEvent* data;
  IxErrHdlAccErrorJob job;
}ixErrHdlAccJobQueue;

/* 
 NPE Image ID to Recovery ID Data Structure
*/
typedef struct{
UINT8 npeId;
UINT8 functionalId;
UINT8 majorVersion;
UINT8 minorVersion;
UINT32 recoveryId;
}IxErrHdlAccNPEImageRecoveryInfo;

/* Public APIs*/
PUBLIC IX_STATUS ixErrHdlAccRecoveryTaskInit(void);
PUBLIC IX_STATUS ixErrHdlAccRecoveryTaskUnInit(void);
PUBLIC IX_STATUS ixErrHdlAccQMEventHandler(void);
PUBLIC IX_STATUS ixErrHdlAccNPEAEventHandler(void);
PUBLIC IX_STATUS ixErrHdlAccNPEBEventHandler(void);
PUBLIC IX_STATUS ixErrHdlAccNPECEventHandler(void);

extern IxErrHdlAccRecoveryStatistics ixErrHdlAccDebugStatistics;

#endif

