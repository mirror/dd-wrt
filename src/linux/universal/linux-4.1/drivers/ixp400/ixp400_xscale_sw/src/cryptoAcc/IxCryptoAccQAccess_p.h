/**
 * @file IxCryptoAccQAccess_p.h
 *
 * @date October-03-2002
 *
 * @brief Private header file for Queue Access module. The only interface  
 *        to QMgr.
 *
 *
 * Design Notes:
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.4
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


#ifndef IxCryptoAccQAccess_p_H
#define IxCryptoAccQAccess_p_H


/*
 * Os/System dependancies.
 */
#include "IxOsal.h"
#include "IxQMgr.h"
#include "IxQueueAssignments.h"
#include "IxCryptoAcc_p.h"


/*
 * #defines for function return types, etc.
 */
                     
/* Global variables */
extern IxCryptoAccStats ixCryptoStats;                     

/**
 * @typedef ixCryptoQCfgInfo
 * @brief Data structure for queue configuration
 *
 */
typedef struct 
{
   IxQMgrQId qId;
   char *qName;
   IxQMgrCallback qCallback;
   IxQMgrCallbackId callbackTag;
   IxQMgrQSizeInWords qSize;
   IxQMgrQEntrySizeInWords qWords; 
   BOOL qNotificationEnableAtStartup;
   IxQMgrSourceId qConditionSource; 
   IxQMgrWMLevel AlmostEmptyThreshold;
   IxQMgrWMLevel AlmostFullThreshold;
} IxCryptoQCfgInfo;


/**
 * @fn ixCryptoQAccessInit
 *
 * @brief Queue configuration and callback registration function
 *
 * @param IxCryptoAccCfg [in] compCfg - IxCryptoAcc component configuration
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 * Reentrant     : no
 * ISR Callable  : no
 *
 */
IxCryptoAccStatus 
ixCryptoQAccessInit (IxCryptoAccCfg compCfg);

/**
 * @fn ixCryptoQAccessUninit
 *
 * @brief Queue configuration and callback registration un-initialization
 *
 * @param none
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoQAccessUninit (void);

/**
 * @fn ixCryptoQAccessReqDoneQMgrCallback
 *
 * @brief Notification callback registered with QManager for crypto hardware
 *        acclerator service. Once the threshold of the interrupt source is 
 *        reached, the notification callback is triggered and is executed.
 *
 * @param IxQMgrId [in] queueId - Queue ID
 * @param IxQMgrCallbackId [in] callbackId - Notification callback ID
 *
 * @return none
 *
 */
void 
ixCryptoQAccessReqDoneQMgrCallback (
    IxQMgrQId queueId, 
    IxQMgrCallbackId callbackId);


/**
 * @fn      ixCryptoQAccessQueueWrite
 *
 * @brief   Write queue entry to hardware queue through QMgr
 *
 * @param IxQMgrId [in] queueId - Queue ID
 * @param UINT32* [out] pQEntry - Queue entry message
 *                                  31    - NPE operation status 
 *                                         (authentication Pass / Fail)
 *                                  30:28 - reserved
 *                                  27:0  - Q descriptor address
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_QUEUE_FULL
 *
 * Remove INLINE keyword to resolve cannot inline warning (vxWorks). SCR1421
 *
 */
IxCryptoAccStatus
ixCryptoQAccessQueueWrite (
    IxQMgrQId queueId,
    UINT32 *pQEntry);


/**
 * @fn      ixCryptoQAccessQueueStatusShow
 *
 * @brief   Show queue status
 *
 * @param IxQMgrId [in] queueId - Queue ID 
 *
 * @return IxCryptoAccStatus
 *             - IX_CRYPTO_ACC_STATUS_SUCCESS
 *             - IX_CRYPTO_ACC_STATUS_FAIL
 *
 */
IxCryptoAccStatus
ixCryptoQAccessQueueStatusShow(IxQMgrQId queueId);


/**
 * @fn ixCryptoQAccessWepReqDoneQMgrCallback
 *
 * @brief Notification callback registered with QManager for WEP service. Once  
 *        the threshold of the interrupt source is reached, the notification
 *        callback is triggered and is executed.
 *
 * @param IxQMgrId [in] queueId - Queue ID
 * @param IxQMgrCallbackId [in] callbackId - Notification callback ID
 *
 * @return none
 *
 */
void 
ixCryptoQAccessWepReqDoneQMgrCallback (
    IxQMgrQId queueId, 
    IxQMgrCallbackId callbackId);


#endif /* ndef IxCryptoAccQAccess_p_H */
