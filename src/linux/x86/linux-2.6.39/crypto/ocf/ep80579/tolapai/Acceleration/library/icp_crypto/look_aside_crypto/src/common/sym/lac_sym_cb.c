/******************************************************************************
 *
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
 *
 *****************************************************************************/

/**
 ***************************************************************************
 * @file lac_sym_cb.c      Callback handler functions for symmetric components
 *
 * @ingroup LacSym
 *
 ***************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "IxOsal.h"
#include "cpa_cy_sym.h"
#include "lac_sym.h"
#include "lac_sym_cipher.h"
#include "lac_common.h"
#include "lac_session.h"
#include "lac_sym_stats.h"
#include "lac_log.h"
#include "lac_sym_cb.h"
#include "lac_sym_qat_hash_defs_lookup.h"

#include "lac_sym_hash.h"

#include "lac_module.h"

/*
*******************************************************************************
* Define static function definitions
*******************************************************************************
*/

/**
 *****************************************************************************
 * @ingroup LacSymCb
 *      Definition of callback function for processing symmetric responses
 * 
 * @description
 *      This callback is invoked to process symmetric response messages from
 *      the QAT.  It will extract some details from the message and invoke
 *      the user's callback to complete a symmetric operation.
 *
 * @param[in] pCookie             Pointer to cookie associated with this request
 * @param[in] qatRespStatusOkFlag Boolean indicating ok/fail status from QAT
 * @param[in] status              Status variable indicating an error occurred
 *                                in sending the message (e.g. when dequeueing)
 * 
 * @return  None
 *****************************************************************************/
STATIC void
LacSymCb_ProcessCallbackInternal(
    lac_sym_bulk_cookie_t *pCookie,
    CpaBoolean qatRespStatusOkFlag,
    CpaStatus status)
{
    CpaInstanceHandle instanceHandle = CPA_INSTANCE_HANDLE_SINGLE;
    CpaCySymCbFunc pSymCb = NULL;
    void *pCallbackTag = NULL;
    lac_session_desc_t *pSessionDesc = NULL;
    CpaCySymOpData *pOpData = NULL;
    CpaBufferList *pDstBuffer = NULL;
    CpaCySymOp operationType = 0;

    /* NOTE: cookie pointer validated in previous function */
    instanceHandle = pCookie->instanceHandle;

    /* If the session descriptor is NULL cant update stats or release 
     * session descriptor */
    if (NULL == pCookie->pSessionDesc)
    {
        LAC_LOG_ERROR("Session Descriptor is NULL");
        Lac_MemPoolEntryFree(pCookie);

        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.cb.istat)
        {
            LAC_SYM_STAT_INC(numSymOpCompletedErrors, instanceHandle);
        }

        return;       
    }

    pSessionDesc = pCookie->pSessionDesc;
    pOpData = (CpaCySymOpData *)
        LAC_CONST_PTR_CAST(pCookie->pOpData);
    operationType = pSessionDesc->symOperation;

    /* For a digest verify operation - for full packet and final partial */
    if ((CPA_CY_SYM_OP_CIPHER != operationType) &&
            (CPA_TRUE == pOpData->digestVerify) && 
            ((CPA_CY_SYM_PACKET_TYPE_FULL == pOpData->packetType) ||
             (CPA_CY_SYM_PACKET_TYPE_LAST_PARTIAL == pOpData->packetType)))
    {
        *(pCookie->pVerifyResult) = qatRespStatusOkFlag;

        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.cb.istat)
        {
            if (CPA_FALSE == qatRespStatusOkFlag)
            {
                LAC_SYM_STAT_INC(numSymOpVerifyFailures, instanceHandle);
            }
        }
    }
    else
    {
        /* Most commands have no point of failure and always return
         * success. This is the default response from the QAT.
         * If status is already set to an error value, don't overwrite it
         */
        if ((CPA_STATUS_SUCCESS == status) && (CPA_TRUE != qatRespStatusOkFlag))
        {
            LAC_LOG_ERROR("Response status value not as expected");
            status = CPA_STATUS_FAIL;
        }
    }

    pSymCb = pSessionDesc->pSymCb;
    pCallbackTag = pCookie->pCallbackTag;

    /* For in-place the dst buffer will also be the source buffer */
    pDstBuffer = pCookie->pDstBuffer;

    /* State returned to the client for intermediate partials packets
     * for hash only and cipher only partial packets. Cipher update
     * allow next partial through */
    if (CPA_CY_SYM_PACKET_TYPE_PARTIAL == pOpData->packetType)
    {
        if ((CPA_CY_SYM_OP_CIPHER == operationType) &&
                (CPA_TRUE == pCookie->updateUserIvOnRecieve))
        {
            /* Update the user's IV buffer 
             * Very important to do this BEFORE dequeuing
             * subsequent partial requests, as the state buffer 
             * may get overwritten
             */
            memcpy(pCookie->pOpData->pIv,
                    pSessionDesc->cipherPartialOpState,
                    pCookie->pOpData->ivLenInBytes);
        }
    }
    if (CPA_CY_SYM_PACKET_TYPE_FULL != pOpData->packetType)
    {    
        /* There may be requests blocked pending the completion of this
         * operation
         */
        LacSymCb_PendingReqsDequeue(pSessionDesc);
    }

    /* deallocate the memory for the internal callback cookie */
    Lac_MemPoolEntryFree(pCookie);

    /* update stats */
    if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.cb.istat)
    {
        LAC_SYM_STAT_INC(numSymOpCompleted, instanceHandle);
        if (CPA_STATUS_SUCCESS != status)
        {
            LAC_SYM_STAT_INC(numSymOpCompletedErrors, instanceHandle);
        }


        ixOsalAtomicDec(&(pSessionDesc->pendingCbCount));
    }

    /* user callback function is the last thing to be called */
    LAC_ASSERT_NOT_NULL(pSymCb);
    pSymCb(pCallbackTag, status, operationType, 
            pOpData, pDstBuffer, qatRespStatusOkFlag);
}


/**
 *****************************************************************************
 * @ingroup LacSymCb
 *      Definition of callback function for processing symmetric responses
 * 
 * @description
 *      This callback, which is registered with the common symmetric response
 *      message handler,  is invoked to process symmetric response messages from
 *      the QAT.  It will extract the response status from the cmnRespFlags set
 *      by the QAT, and then will pass it to @ref
 *      LacSymCb_ProcessCallbackInternal to complete the response processing.
 *
 * @param[in] lacCmdId          ID of the symmetric QAT command of the request
 *                              message
 * @param[in] pOpaqueData       pointer to opaque data in the request message
 * @param[in] cmnRespFlags      Flags set by QAT to indicate response status
 * 
 * @return  None
 *****************************************************************************/
STATIC void
LacSymCb_ProcessCallback(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pOpaqueData,
    icp_qat_fw_comn_flags cmnRespFlags)
{
    CpaBoolean qatRespStatusOkFlag = CPA_TRUE;

    /* Response status is true by default, but if the status is not OK 
     * then overwrite the status with false */
    if (ICP_QAT_FW_COMN_STATUS_FLAG_OK != 
        ICP_QAT_FW_COMN_STATUS_GET(cmnRespFlags))
    {
        qatRespStatusOkFlag = CPA_FALSE;
    }

    LacSymCb_ProcessCallbackInternal((lac_sym_bulk_cookie_t *)pOpaqueData,
                                     qatRespStatusOkFlag, 
                                     CPA_STATUS_SUCCESS);
}


/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

/**
 * @ingroup LacSymCb
 */
void
LacSymCb_PendingReqsDequeue(lac_session_desc_t *pSessionDesc)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Need to protect access to queue head and tail pointers, which may 
     * be accessed by multiple contexts simultaneously for enqueue and
     * dequeue operations
     */
    LAC_SPINLOCK(&pSessionDesc->requestQueueLock);

    /* Clear the blocking flag in the session descriptor */
    pSessionDesc->nonBlockingOpsInProgress = CPA_TRUE;

    while ((NULL != pSessionDesc->pRequestQueueHead) &&
           (CPA_TRUE == pSessionDesc->nonBlockingOpsInProgress))
    {
        /* If we send a partial packet request, set the blockingOpsInProgress
         * flag for the session to indicate that subsequent requests must be
         * queued up until this request completes
         */
        if (CPA_CY_SYM_PACKET_TYPE_FULL != 
            pSessionDesc->pRequestQueueHead->pOpData->packetType)
        {
            pSessionDesc->nonBlockingOpsInProgress = CPA_FALSE;
        }

        /* At this point, we're clear to send the request.  For cipher requests,
         * we need to check if the session IV needs to be updated.  This can
         * only be done when no other partials are in flight for this session,
         * to ensure the cipherPartialOpState buffer in the session descriptor
         * is not currently in use
         */
        if (CPA_TRUE == pSessionDesc->pRequestQueueHead->updateSessionIvOnSend)
        {
            if(LAC_CIPHER_IS_ARC4(pSessionDesc->cipherAlgorithm))
            {
                memcpy(pSessionDesc->cipherPartialOpState,
                       pSessionDesc->cipherARC4InitialState,
                       LAC_CIPHER_ARC4_STATE_LEN_BYTES);
            }
            else
            {
                memcpy(pSessionDesc->cipherPartialOpState, 
                       pSessionDesc->pRequestQueueHead->pOpData->pIv,
                       pSessionDesc->pRequestQueueHead->pOpData->ivLenInBytes);
            }
        }

        /* Send the message */
        status = QatComms_MsgSend(
            &(pSessionDesc->pRequestQueueHead->qatMsg),
            ICP_ARCH_IF_REQ_QAT_FW_LA,
            pSessionDesc->qatSessionPriority,
            pSessionDesc->pRequestQueueHead->instanceHandle);

        if (CPA_STATUS_SUCCESS != status)
        {
            /* The callback function can only return CPA_STATUS_SUCCESS or 
             * CPA_STATUS_FAIL following the API definition.
             */
            status = CPA_STATUS_FAIL;

            /* Invoke the callback for this request, with 
             * status param set to FAIL.  This should free resources,
             * dequeue any pending messages, and invoke the user's callback
             * with the status set to FAIL. 
             */
            LacSymCb_ProcessCallbackInternal(pSessionDesc->pRequestQueueHead,
                                             CPA_FALSE,
                                             status);
        }

       pSessionDesc->pRequestQueueHead = pSessionDesc->pRequestQueueHead->pNext;
    }

    /* If we've drained the queue, ensure the tail pointer is set to NULL */ 
    if (NULL == pSessionDesc->pRequestQueueHead)
    {
        pSessionDesc->pRequestQueueTail = NULL;
    }

    LAC_SPINUNLOCK(&pSessionDesc->requestQueueLock);
}


/**
 * @ingroup LacSymCb
 */
void
LacSymCb_CallbacksRegister(void)
{
    /*** HASH ***/
    LacSymQat_RespHandlerRegister(ICP_QAT_FW_LA_CMD_AUTH,
                                  LacSymCb_ProcessCallback);
    /*** ALGORITHM-CHAINING ***/
    LacSymQat_RespHandlerRegister(ICP_QAT_FW_LA_CMD_CIPHER_HASH,
                                  LacSymCb_ProcessCallback);

    /*** CIPHER ***/
    LacSymQat_RespHandlerRegister(ICP_QAT_FW_LA_CMD_CIPHER,
                                  LacSymCb_ProcessCallback);
}

