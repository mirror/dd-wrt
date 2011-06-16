/***************************************************************************
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
 ***************************************************************************/

/**
 ***************************************************************************
 * @file lac_sym_alg_chain.c      Algorithm Chaining Perform
 *
 * @ingroup LacAlgChain
 ***************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "icp_qat_fw.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

#include "lac_mem.h"
#include "lac_log.h"
#include "lac_sym.h"
#include "lac_sym_alg_chain.h"
#include "lac_sym_cipher.h"
#include "lac_sym_cipher_defs.h"
#include "lac_sym_hash.h"
#include "lac_sym_hash_defs.h"
#include "lac_sym_qat_cipher.h"
#include "lac_sym_qat_hash.h"
#include "lac_sym_stats.h"
#include "lac_sym_queue.h"
#include "lac_sym_cb.h"

#include "lac_module.h"


/* These #defines are used index to a slice array to build a chain of qat 
 * requests. */
#define LAC_ALG_CHAIN_QAT_SLICE_INDEX0            0
#define LAC_ALG_CHAIN_QAT_SLICE_INDEX1            1
#define LAC_ALG_CHAIN_QAT_SLICE_INDEX2            2


/**
 * @ingroup LacAlgChain 
 * This callback function will be invoked whenever a hash precompute
 * operation completes.  It will dequeue and send any QAT requests
 * which were queued up while the precompute was in progress.
 * 
 * @param[in] callbackTag  Opaque value provided by user. This will
 *                         be a pointer to the session descriptor.
 * 
 * @retval
 *     None
 *
 */
STATIC void LacSymAlgChain_HashPrecomputeDoneCb(void *callbackTag)
{
    LacSymCb_PendingReqsDequeue((lac_session_desc_t *)callbackTag);
}


/**
 * @ingroup LacAlgChain 
 * Walk the buffer list and locate the address within a buffer. A return 
 * value of True indicates that the address is within the buffer. False if
 * the address is outside the buffer. When the address is inside the buffer
 * list the calculated offset from the beginning of the buffer list to the 
 * address is returned 
 * 
 * @param[in] pBufferList   Buffer List
 * @param[in] pAddr         Address to find in the buffer list
 * @param[out] pOffset      Offset within the buffer list if address is
 *                          inside the buffer list
 * 
 * @retval CPA_TRUE     address is within the buffer list
 * @retval CPA_FALSE    address is outside the buffer list
 * 
 */
STATIC CpaBoolean
LacSymAlgChain_OffsetFromPtrGet(const CpaBufferList *pBufferList,
                                const Cpa8U *pAddr,
                                Cpa32U *pOffset)
{
    Cpa32U offset = 0;
    Cpa32U i = 0;
    
    for (i = 0; i < pBufferList->numBuffers; i++)
    {
        Cpa8U *pCurrData = pBufferList->pBuffers[i].pData;
        Cpa32U currDataSize = pBufferList->pBuffers[i].dataLenInBytes;

        /* If the address is within the address space of the current buffer */ 
        if ((pAddr >= pCurrData) && (pAddr < (pCurrData + currDataSize)))
        {
            /* incmrement by offset of the address in the current buffer */
            *pOffset = offset + (pAddr - pCurrData);
            return CPA_TRUE; 
        }

        /* Increment by the size of the buffer */
        offset += currDataSize;    
    }

    return CPA_FALSE;
}

/** @ingroup LacAlgChain */
CpaStatus
LacAlgChain_SessionInit(
    const CpaInstanceHandle instanceHandle,
    const CpaCySymSessionSetupData *pSessionSetupData,
    lac_session_desc_t *pSessionDesc)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_sym_qat_content_desc_info_t *pCdInfo = NULL;
    Cpa32U ctrlBlockHdrSz = 0;

    Cpa8U *pCtrlBlock = NULL;
    Cpa8U *pHwBlockBase = NULL;
    Cpa32U ctrlBlockOffset = 0;
    Cpa32U hwBlockOffset = 0;
    Cpa32U i = 0;

    /* check parameters */
    LAC_ENSURE_NOT_NULL(pSessionSetupData);
    LAC_ENSURE_NOT_NULL(pSessionDesc);

    switch(pSessionSetupData->symOperation)
    {
        case CPA_CY_SYM_OP_CIPHER:
        {
            pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX0] = 
                ICP_QAT_FW_SLICE_CIPHER;
            pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX1] = 
                ICP_QAT_FW_SLICE_MEM_OUT;
            ctrlBlockHdrSz = sizeof(icp_qat_fw_cipher_hdr_t);
            break;
        }
        case CPA_CY_SYM_OP_HASH:
        {
            pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX0] = 
                ICP_QAT_FW_SLICE_AUTH;
            pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX1] = 
                ICP_QAT_FW_SLICE_MEM_OUT; 
            ctrlBlockHdrSz = sizeof(icp_qat_fw_auth_hdr_t);
            break;
        }
        case CPA_CY_SYM_OP_ALGORITHM_CHAINING:
        {
            CpaCySymAlgChainOrder chainOrder = pSessionSetupData->algChainOrder;

            if (pSessionDesc->isAuthEncryptOp)
            {
                /* Ensure algChainOrder is set correctly for CCM/GCM ops */
                CpaCySymCipherAlgorithm cipherAlgorithm = 
                    pSessionSetupData->cipherSetupData.cipherAlgorithm;
                CpaCySymCipherDirection cipherDir = 
                    pSessionSetupData->cipherSetupData.cipherDirection;

                if ((LAC_CIPHER_IS_CCM(cipherAlgorithm) &&
                     (CPA_CY_SYM_CIPHER_DIRECTION_DECRYPT == cipherDir)) ||
                    (LAC_CIPHER_IS_GCM(cipherAlgorithm) &&
                     (CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT == cipherDir)))
                {
                    chainOrder = CPA_CY_SYM_ALG_CHAIN_ORDER_CIPHER_THEN_HASH;
                }
                else
                {
                    chainOrder = CPA_CY_SYM_ALG_CHAIN_ORDER_HASH_THEN_CIPHER;
                }
            }

            if (CPA_CY_SYM_ALG_CHAIN_ORDER_CIPHER_THEN_HASH == chainOrder)
            {
                pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX0] = 
                    ICP_QAT_FW_SLICE_CIPHER;
                pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX1] = 
                    ICP_QAT_FW_SLICE_AUTH;
            }
            else if (CPA_CY_SYM_ALG_CHAIN_ORDER_HASH_THEN_CIPHER == chainOrder)
            {
                pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX0] = 
                    ICP_QAT_FW_SLICE_AUTH;
                pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX1] = 
                    ICP_QAT_FW_SLICE_CIPHER;
            }
            else
            {
                LAC_INVALID_PARAM_LOG("algChainOrder");
                status = CPA_STATUS_INVALID_PARAM;
                break;
            }
            pSessionDesc->qatSlices[LAC_ALG_CHAIN_QAT_SLICE_INDEX2] = 
                ICP_QAT_FW_SLICE_MEM_OUT;
            ctrlBlockHdrSz = sizeof(icp_qat_fw_cipher_hdr_t) +
                sizeof(icp_qat_fw_auth_hdr_t);
            break;
        }
        /* Unsupported operation. Return error */
        default:
        {
            LAC_INVALID_PARAM_LOG("symOperation");
            status = CPA_STATUS_INVALID_PARAM;
            break;
        }
    }

    /* setup some convenience pointers */
    pCdInfo = &(pSessionDesc->contentDescInfo);
    pCtrlBlock = (Cpa8U *)pCdInfo->pData;
    pHwBlockBase = pCtrlBlock + ctrlBlockHdrSz;
 
    /* Initialise Request Queue */
    LAC_SPINLOCK_INIT(&pSessionDesc->requestQueueLock);
    pSessionDesc->validSessionDesc = CPA_TRUE;
    pSessionDesc->pRequestQueueHead = NULL;
    pSessionDesc->pRequestQueueTail = NULL;
    pSessionDesc->nonBlockingOpsInProgress = CPA_TRUE;

    /* Reset the pending callback counter */
    ixOsalAtomicSet(0, &pSessionDesc->pendingCbCount);

    /* Partial state must be set to full, to indicate that next packet
     * expected on the session is a full packet or the start of a 
     * partial packet. */
    pSessionDesc->partialState = CPA_CY_SYM_PACKET_TYPE_FULL;

    /* walk the QAT slice chain, excluding last (terminating) entry */
    for (i = 0; (i < MAX_NUM_QAT_SLICES - 1) &&
                (CPA_STATUS_SUCCESS == status); i++)
    {
        if (ICP_QAT_FW_SLICE_CIPHER == pSessionDesc->qatSlices[i])
        {
            const CpaCySymCipherSetupData 
                        *pCipherSetupData = &pSessionSetupData->cipherSetupData;
            icp_qat_fw_cipher_hdr_t *pCipherControlBlock =
                (icp_qat_fw_cipher_hdr_t *)(pCtrlBlock + ctrlBlockOffset);
            Cpa32U cipherHwBlkSizeBytes = 0;

#ifdef ICP_PARAM_CHECK
            status = LacCipher_SessionSetupDataCheck(pCipherSetupData);
#endif //ICP_PARAM_CHECK

            if (CPA_STATUS_SUCCESS == status)
            {
                /* set the cipher session information */
                pSessionDesc->cipherAlgorithm = 
                                            pCipherSetupData->cipherAlgorithm;
                pSessionDesc->cipherKeyLenInBytes = 
                                          pCipherSetupData->cipherKeyLenInBytes;
                pSessionDesc->cipherDirection = 
                                            pCipherSetupData->cipherDirection;

                /* populate the cipher section of the content descriptor
                 * - the key length from cipher setup data is also used as 
                 *   target key length.  No key padding is required.
                 */
                LacSymQat_CipherContentDescPopulate(
                    pCipherSetupData,
                    pCipherSetupData->cipherKeyLenInBytes,
                    pCipherControlBlock,
                    pHwBlockBase,
                    hwBlockOffset,
                    pSessionDesc->qatSlices[i + 1],
                    &cipherHwBlkSizeBytes);
                
                /* update offsets */
                ctrlBlockOffset += sizeof(*pCipherControlBlock);
                hwBlockOffset += cipherHwBlkSizeBytes;

                /* ARC4 base key isn't added to the content descriptor, because
                 * we don't need to pass it directly to the QAT engine. Instead
                 * an initial cipher state & key matrix is derived from the
                 * base key and provided to the QAT through the state pointer
                 * in the request params. We'll store this initial state in 
                 * the session descriptor. */
                if (LAC_CIPHER_IS_ARC4(pCipherSetupData->cipherAlgorithm))
                {
                    LacSymQat_CipherArc4StateInit (
                        pCipherSetupData->pCipherKey,
                        pCipherSetupData->cipherKeyLenInBytes,
                        pSessionDesc->cipherARC4InitialState);
                }
            }
        }
        else if (ICP_QAT_FW_SLICE_AUTH == pSessionDesc->qatSlices[i])
        {
            const CpaCySymHashSetupData *pHashSetupData = 
                                        &pSessionSetupData->hashSetupData;
            lac_sym_qat_hash_precompute_info_t precomputeData = {0};
            icp_qat_fw_auth_hdr_t *pHashControlBlock =
                (icp_qat_fw_auth_hdr_t *)(pCtrlBlock + ctrlBlockOffset);
            Cpa32U hashHwBlockSizeBytes = 0;
            icp_qat_hw_auth_mode_t qatHashMode = 0; 
            lac_sym_qat_hash_state_buffer_info_t *pHashStateBufferInfo = 
                 &(pSessionDesc->hashStateBufferInfo);  
 
#ifdef ICP_PARAM_CHECK
            status = LacHash_HashContextCheck(pHashSetupData);
#endif
            if (CPA_STATUS_SUCCESS == status)
            {
                /* set the hash session information */
                pSessionDesc->hashResultSize = 
                    pHashSetupData->digestResultLenInBytes;
                pSessionDesc->hashMode = pHashSetupData->hashMode;
                pSessionDesc->hashAlgorithm = pHashSetupData->hashAlgorithm;
   
                if (CPA_TRUE == pSessionDesc->isAuthEncryptOp)
                {
                    pSessionDesc->aadLenInBytes = 
                        pHashSetupData->authModeSetupData.aadLenInBytes;
                }

                /* Set the QAT hash mode */ 
                if ((pHashSetupData->hashMode == CPA_CY_SYM_HASH_MODE_NESTED) ||
                    (pHashSetupData->hashMode == CPA_CY_SYM_HASH_MODE_PLAIN))
                {
                    qatHashMode = ICP_QAT_HW_AUTH_MODE0;
                }
                else
                {
                    /* HMAC Hash mode is determined by the Kernel module 
                     * parameter */
                    if (IS_HMAC_ALG(pHashSetupData->hashAlgorithm))
                    {
                        qatHashMode = Lac_GetQatHmacMode();
                    }
                    /* XCBC MAC & GCM must be done in mode1 */     
                    else 
                    {
                        qatHashMode = ICP_QAT_HW_AUTH_MODE1;
                    }
                }

                /* populate the hash section of the content descriptor */
                LacSymQat_HashContentDescInit(
                    pHashSetupData,
                    pHashControlBlock,
                    pHwBlockBase,
                    hwBlockOffset,
                    pSessionDesc->qatSlices[i + 1],
                    qatHashMode,
                    &precomputeData,
                    &hashHwBlockSizeBytes);
            
                /* update offsets */
                ctrlBlockOffset += sizeof(*pHashControlBlock);
                hwBlockOffset += hashHwBlockSizeBytes;

                /* populate the hash state prefix buffer info structure & the 
                 * buffer itself. For CCM/GCM the buffer is stored in the 
                 * cookie and is not initialised here */
                if (CPA_FALSE == pSessionDesc->isAuthEncryptOp)
                {
                    LacHash_StatePrefixAadBufferInit(
                        pHashSetupData, pHashControlBlock, qatHashMode,
                        pSessionDesc->hashStatePrefixBuffer,
                        pHashStateBufferInfo);
                }
               
                if (IS_HASH_MODE_1(qatHashMode))
                {
                    /* Block messages until precompute is completed */
                    pSessionDesc->nonBlockingOpsInProgress = CPA_FALSE;

                    status = LacHash_PrecomputeDataCreate(
                                instanceHandle,
                                pHashSetupData->hashAlgorithm,
                                pHashSetupData->authModeSetupData,
                                LacSymAlgChain_HashPrecomputeDoneCb,
                                pSessionDesc,
                                pSessionDesc->hashStatePrefixBuffer,
                                precomputeData.pState1,
                                precomputeData.pState2); 
                }
            }
        }
        /* chain terminators */
        else if (ICP_QAT_FW_SLICE_MEM_OUT == pSessionDesc->qatSlices[i])
        {
            /* end of chain */
            break;
        }
    } /* for (i = 0; ... */

    if (CPA_STATUS_SUCCESS == status)
    {
        /* configure the common fields of the content descriptor */
        pCdInfo->hdrSzQuadWords = LAC_BYTES_TO_QUADWORDS(ctrlBlockOffset);
        pCdInfo->hwBlkSzQuadWords = LAC_BYTES_TO_QUADWORDS(hwBlockOffset);
 
        /* create the request header */
        status = QatComms_ReqHdrCreate(&pSessionDesc->reqHdr, 
                    ICP_ARCH_IF_REQ_QAT_FW_LA);
   }

    return status;
}

/** @ingroup LacAlgChain */
CpaStatus
LacAlgChain_Perform(
    const CpaInstanceHandle instanceHandle,
    lac_session_desc_t *pSessionDesc,
    void *pCallbackTag,
    const CpaCySymOpData *pOpData,
    const CpaBufferList *pSrcBuffer,
    CpaBufferList *pDstBuffer,
    CpaBoolean *pVerifyResult
    )
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_sym_bulk_cookie_t * pCookie = NULL;
    lac_sym_qat_content_desc_info_t *pCdInfo = NULL;
    Cpa8U *pCtrlBlock = NULL;
    Cpa32U ctrlBlockOffset = 0;
    Cpa8U *pReqParams = NULL;
    Cpa32U reqParamsOffset = 0;
    icp_qat_fw_la_bulk_req_t *pMsg = NULL;
    Cpa16U laCmdFlags = 0;
    Cpa32U qatPacketType = 0;
    icp_qat_fw_la_cmd_id_t laCmdId = ICP_QAT_FW_LA_CMD_CIPHER;
    Cpa32U i = 0;

    LAC_ENSURE_NOT_NULL(pSessionDesc);
    LAC_ENSURE_NOT_NULL(pOpData);

    /* Select the command id */
    switch(pSessionDesc->symOperation)
    {
        case CPA_CY_SYM_OP_CIPHER:
        {
            laCmdId = ICP_QAT_FW_LA_CMD_CIPHER;
            break;
        }
        case CPA_CY_SYM_OP_HASH:
        {
            laCmdId = ICP_QAT_FW_LA_CMD_AUTH;
            break;
        }
        case CPA_CY_SYM_OP_ALGORITHM_CHAINING:
        {
            laCmdId = ICP_QAT_FW_LA_CMD_CIPHER_HASH;
            break;
        }
        /* Unsupported operation. Return error */
        default:
        {
            LAC_INVALID_PARAM_LOG("symOperation");
            status = CPA_STATUS_INVALID_PARAM;
            break;
        }
    }

    /* setup some convenience pointers */
    pCdInfo = &(pSessionDesc->contentDescInfo);
    pCtrlBlock = (Cpa8U *)pCdInfo->pData;

    /* allocate cookie (used by callback function) */
    if (CPA_STATUS_SUCCESS == status)
    {
        if((pCookie = 
            (lac_sym_bulk_cookie_t *)
            Lac_MemPoolEntryAlloc(lac_sym_cookie_memory)) == NULL)
        {
            LAC_LOG_ERROR("Cannot allocate cookie");
            status = CPA_STATUS_RESOURCE;
        }
    }

    /* allocate memory for cipher and hash request parameters */
    if (CPA_STATUS_SUCCESS == status)
    {
#ifdef ICP_PARAM_CHECK
        Cpa32U srcPktSize = 0;
#endif

        /* populate the cookie */
        pCookie->pCallbackTag = pCallbackTag;
        pCookie->pSessionDesc = pSessionDesc;
        pCookie->pOpData = (const CpaCySymOpData *) pOpData;
        pCookie->pSrcBuffer = (const CpaBufferList *) pSrcBuffer;   
        pCookie->pDstBuffer = pDstBuffer;
        pCookie->pVerifyResult = pVerifyResult;
        pCookie->updateSessionIvOnSend = CPA_FALSE;
        pCookie->updateUserIvOnRecieve = CPA_FALSE;
        pCookie->pNext = NULL;
        /* Cookie contains space for request params */
        pReqParams = pCookie->reqParamsBuffer;
        pMsg = &(pCookie->qatMsg);

        /* get the qat packet type for LAC packet type */
        LacSymQat_packetTypeGet(pOpData->packetType,
                                pSessionDesc->partialState,
                                &qatPacketType);

        /* set the command flags based on the packet type */
        LacSymQat_LaPacketCommandFlagSet(
            qatPacketType,
            &laCmdFlags);

#ifdef ICP_PARAM_CHECK
        LacBuffDesc_BufferListTotalSizeGet(pSrcBuffer, &srcPktSize);    
#endif
        /* walk the QAT slice chain, excluding last (terminating) entry */
        for (i = 0; (i < MAX_NUM_QAT_SLICES - 1) && 
                               (CPA_STATUS_SUCCESS == status); i++)
        {
            if (ICP_QAT_FW_SLICE_CIPHER == pSessionDesc->qatSlices[i])
            {
                icp_qat_fw_cipher_hdr_t *pCipherControlBlock =
                    (icp_qat_fw_cipher_hdr_t *)(pCtrlBlock + ctrlBlockOffset);
                icp_qat_fw_la_cipher_req_params_t *pCipherReqParams =
                    (icp_qat_fw_la_cipher_req_params_t *)
                    (pReqParams + reqParamsOffset);
                Cpa32U cipherOffsetInBytes = 
                    pOpData->cryptoStartSrcOffsetInBytes;
                Cpa32U cipherLenInBytes = pOpData->messageLenToCipherInBytes;
                Cpa8U *pAlignedIvBuffer = NULL;

                LAC_ENSURE_NOT_NULL(pCipherControlBlock);
                LAC_ENSURE_NOT_NULL(pCipherReqParams);

#ifdef ICP_PARAM_CHECK
                status = LacCipher_PerformParamCheck(
                            pSessionDesc->cipherAlgorithm, 
                            pOpData, 
                            srcPktSize);
#endif
                if (CPA_STATUS_SUCCESS == status)
                {
                    /* align cipher IV */
                    status = LacCipher_PerformIvCheckAndAlign(
                        pCookie, qatPacketType, &pAlignedIvBuffer);
                }

                if (CPA_STATUS_SUCCESS == status)
                {
                    /* populate the cipher request parameters */
                    LacSymQat_CipherRequestParamsPopulate(
                        pCipherReqParams,
                        pSessionDesc->qatSlices[i + 1],
                        pSessionDesc->cipherAlgorithm,
                        cipherOffsetInBytes,
                        cipherLenInBytes,
                        pAlignedIvBuffer);

                    /* update offsets */
                    ctrlBlockOffset += sizeof(*pCipherControlBlock);
                    reqParamsOffset += sizeof(*pCipherReqParams);

                    /* For ECB-mode ciphers, IV is NULL so update-state flag
                     * must be unset (normally set for all partial packets)
                     */
                    if ((CPA_CY_SYM_PACKET_TYPE_PARTIAL == pOpData->packetType)
                        &&
                        LAC_CIPHER_IS_ECB_MODE(pSessionDesc->cipherAlgorithm))
                    {
                        ICP_QAT_FW_LA_WR_STATE_SET(
                            laCmdFlags, ICP_QAT_FW_LA_NO_UPDATE_STATE);
                    }
                }
            }
            else if (ICP_QAT_FW_SLICE_AUTH == pSessionDesc->qatSlices[i])
            {
                icp_qat_fw_auth_hdr_t *pHashControlBlock =
                    (icp_qat_fw_auth_hdr_t *)(pCtrlBlock + ctrlBlockOffset);
                icp_qat_fw_la_auth_req_params_t *pHashReqParams =
                    (icp_qat_fw_la_auth_req_params_t *)
                    (pReqParams + reqParamsOffset);
                Cpa32U authOffsetInBytes = pOpData->hashStartSrcOffsetInBytes;
                Cpa32U authLenInBytes = pOpData->messageLenToHashInBytes;

#ifdef ICP_PARAM_CHECK
                status = LacHash_PerformParamCheck(pSessionDesc, pOpData,
                                                   srcPktSize, pVerifyResult);
#endif
                if (CPA_STATUS_SUCCESS == status)
                {
                    /* performance optimisation for in place 
                     * auth only operation */
                    icp_qat_fw_slice_t nextSlice = 
                        ((ICP_QAT_FW_LA_CMD_AUTH == laCmdId) && 
                         (pSrcBuffer == pDstBuffer)) ? ICP_QAT_FW_SLICE_NULL :
                             pSessionDesc->qatSlices[i + 1];
                    /* Info structure for CCM/GCM */
                    lac_sym_qat_hash_state_buffer_info_t 
                        hashStateBufferInfo = {0};
                    lac_sym_qat_hash_state_buffer_info_t *pHashStateBufferInfo 
                         = &(pSessionDesc->hashStateBufferInfo);

                    if (CPA_TRUE == pSessionDesc->isAuthEncryptOp)
                    {
                        /* AAD buffer is stored in the cookie for CCM/GCM */ 
                        hashStateBufferInfo.pData = pCookie->aadData;
                        hashStateBufferInfo.pDataPhys = 
                            LAC_MEM_CAST_PTR_TO_UINT64(
                              LAC_OS_VIRT_TO_PHYS(hashStateBufferInfo.pData));
                        hashStateBufferInfo.stateStorageSzQuadWords = 0;
                        hashStateBufferInfo.prefixAadSzQuadWords =
                            LAC_BYTES_TO_QUADWORDS(pHashControlBlock->u.aad_sz);

                        /* populate AAD buffer */
                        LacSymQat_HashStatePrefixAadBufferPopulate(
                            &hashStateBufferInfo,
                            pHashControlBlock,
                            pOpData->pAdditionalAuthData, 
                            pSessionDesc->aadLenInBytes, 
                            NULL,
                            0);
                   
                        /* Overwrite hash state buffer info structure pointer
                         * with the one created for CCM/GCM */
                        pHashStateBufferInfo = &hashStateBufferInfo;
 
                        /* for CCM/GCM the hash and cipher data regions 
                         * are equal */
                        authOffsetInBytes = 
                                   pOpData->cryptoStartSrcOffsetInBytes;
                        authLenInBytes = pOpData->messageLenToCipherInBytes;
                    }

                    /* populate the hash request parameters */
                    LacSymQat_HashRequestParamsPopulate(
                        pHashReqParams,
                        nextSlice,
                        pHashStateBufferInfo,
                        qatPacketType,
                        pSessionDesc->hashResultSize,
                        authOffsetInBytes,
                        authLenInBytes,
                        pOpData->digestVerify,
                        pOpData->pDigestResult);

                    /* update offsets */
                    ctrlBlockOffset += sizeof(*pHashControlBlock);
                    reqParamsOffset += sizeof(*pHashReqParams);

                    /* setup the hash command flags */
                    LacSymQat_HashLaCommandFlagsSet(
                        qatPacketType,
                        pOpData->digestVerify,
                        &laCmdFlags);
                }
            }
            else if (ICP_QAT_FW_SLICE_MEM_OUT == pSessionDesc->qatSlices[i])
            {
                /* end of chain */
                break;
            }
        } /* for (i = 0; ... */
    }
    
    /* set command flags */
    if (CPA_STATUS_SUCCESS == status)
    {
        Cpa16U proto = ICP_QAT_FW_LA_NO_PROTO; /* no CCM/GCM */ 
        Cpa16U apdAuthResult = ICP_QAT_FW_LA_NO_APD_AUTH_RES; 

        /* check for CCM/GCM protocols */
        if (CPA_TRUE == pSessionDesc->isAuthEncryptOp)
        {
            if (LAC_CIPHER_IS_CCM(pSessionDesc->cipherAlgorithm))
            {
                proto = ICP_QAT_FW_LA_CCM_PROTO;
            }
            else
            {
                proto = ICP_QAT_FW_LA_GCM_PROTO;
            }
        }
        else
        {
            /* if hash then encrypt operation - a flag needs to be set if hash
             * result is written inside the cipher region */
            if ((ICP_QAT_FW_SLICE_AUTH == pSessionDesc->qatSlices[0]) &&
                (ICP_QAT_FW_SLICE_CIPHER == pSessionDesc->qatSlices[1]) &&
                (CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT == 
                 pSessionDesc->cipherDirection))
            {
                Cpa32U hashResultOffset = 0;

                /* check if digest pointer is inside the packet and if so get 
                 * the corresponding hash result offset */
                if (CPA_TRUE == LacSymAlgChain_OffsetFromPtrGet(pDstBuffer, 
                                    pOpData->pDigestResult, &hashResultOffset))
                {
                    /* Check if hash offset is inside the cipher region. The 
                     * auth result append flag must be set */ 
                    if ((hashResultOffset >= 
                            pOpData->cryptoStartSrcOffsetInBytes) &&
                        (hashResultOffset < 
                            (pOpData->cryptoStartSrcOffsetInBytes + 
                             pOpData->messageLenToCipherInBytes)))
                    {
                        apdAuthResult = ICP_QAT_FW_LA_APD_AUTH_RES;
#ifdef ICP_PARAM_CHECK
                        /* The hash result must be specified at the end of
                         *  the hash region */
                        if (hashResultOffset != 
                                (pOpData->hashStartSrcOffsetInBytes +
                                 pOpData->messageLenToHashInBytes))
                        {
                            LAC_INVALID_PARAM_LOG(
                                "As hash result pointer is inside cipher region"
                                " it must point to the end of the hash region");
                            status = CPA_STATUS_INVALID_PARAM;
                        }
#endif
                    }
                }
            }
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            ICP_QAT_FW_LA_APD_AUTH_SET(laCmdFlags, apdAuthResult);
            ICP_QAT_FW_LA_PROTO_SET(laCmdFlags, proto);
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        Cpa64U srcAddrPhys = 0;
        Cpa64U dstAddrPhys = 0;

        /* write the buffer descriptors */
        LacBuffDesc_BufferListDescWrite((const CpaBufferList *) pSrcBuffer, 
                                            &srcAddrPhys);

        /* For out of place operations */
        if (pSrcBuffer != pDstBuffer)
        {
            LacBuffDesc_BufferListDescWrite(pDstBuffer, &dstAddrPhys);
        }

        /* populate the bulk request */
        LacSymQat_BulkReqPopulate(
            pMsg,
            &(pSessionDesc->contentDescInfo),
            pCookie,
            srcAddrPhys,
            dstAddrPhys,
            laCmdId,
            pReqParams,
            reqParamsOffset,
            laCmdFlags);

        memcpy(pMsg, &pSessionDesc->reqHdr, sizeof(icp_arch_if_req_hdr_t));
        
        /* send the message to the QAT */
        status = LacSymQueue_RequestSend(
            instanceHandle,
            pCookie,
            pSessionDesc);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        if(ICP_CRYPTO_STATISTIC_ON == icp_crypto.statistics.alg_chain.istat) {
            ixOsalAtomicInc(&(pSessionDesc->pendingCbCount));
        }
    }
    /* Case that will catch all error status's for this function */
    else
    {
        /* free the cookie */
        if (NULL != pCookie)
        {
            Lac_MemPoolEntryFree(pCookie);
        }
    }

    return status;
}

