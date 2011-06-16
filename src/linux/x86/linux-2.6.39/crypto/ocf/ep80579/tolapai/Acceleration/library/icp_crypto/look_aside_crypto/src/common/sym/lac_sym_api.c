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
 * @file lac_sym_api.c      Implementation of the symmetric API
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
#include "cpa_cy_sym.h"
#include "lac_common.h"
#include "IxOsal.h"
#include "lac_session.h"
#include "lac_sym_cipher.h"
#include "lac_sym_hash.h"
#include "lac_sym_alg_chain.h"
#include "lac_sym_stats.h"
#include "lac_sym_partial.h"
#include "lac_log.h"
#include "lac_mem.h"
#include "lac_sym_qat_hash_defs_lookup.h"
#include "lac_sym_cb.h"
#include "lac_buffer_desc.h"
#include "lac_sync.h"

#include "lac_hooks.h"

/* Macro for checking if partial packet are supported for a given
 * symmetric operation */
#define IS_PARTIAL_ON_SYM_OP_SUPPORTED(symOp, hashMode)    \
            ((CPA_CY_SYM_OP_CIPHER == symOp) ||           \
             ((CPA_CY_SYM_OP_HASH == symOp) &&            \
              (CPA_CY_SYM_HASH_MODE_NESTED != hashMode)))

/* Identifier for pool of pre-allocated cookies for all symmetric modules */
lac_memory_pool_id_t lac_sym_cookie_memory = LAC_MEM_POOL_INIT_POOL_ID;

/*** Local functions definitions ***/
STATIC CpaStatus LacSymPerform_BufferParamCheck(
    const CpaBufferList * const pSrcBuffer,
    const CpaBufferList * const pDstBuffer,
    const CpaCySymPacketType packetType,
    const lac_session_desc_t * const pSessionDesc,
    const CpaCySymOpData * const pOpData);


/*
*******************************************************************************
* Define static function definitions
*******************************************************************************
*/


#ifdef ICP_PARAM_CHECK
/**
 * @ingroup LacSym
 * Function which perform parameter checks on session setup data
 *
 * @param[in] pSessionSetupData     Pointer to session setup data
 *
 * @retval CPA_STATUS_SUCCESS        The operation succeeded
 * @retval CPA_STATUS_INVALID_PARAM  An invalid parameter value was found
 */
STATIC CpaStatus
LacSymSession_ParamCheck(const CpaCySymSessionSetupData *pSessionSetupData)
{
    /* initialize convenient pointers to cipher and hash contexts */
    const CpaCySymCipherSetupData * const pCipherSetupData =
        &pSessionSetupData->cipherSetupData;
    const CpaCySymHashSetupData * const pHashSetupData =
        &pSessionSetupData->hashSetupData;

    /* ensure CCM/GCM cipher and hash algorithms are selected together for
     * Algorithm Chaining */
    if (CPA_CY_SYM_OP_ALGORITHM_CHAINING == pSessionSetupData->symOperation)
    {
        /* ensure both hash and cipher algorithms are CCM */
        if (((CPA_CY_SYM_CIPHER_AES_CCM == pCipherSetupData->cipherAlgorithm) &&
                (CPA_CY_SYM_HASH_AES_CCM != pHashSetupData->hashAlgorithm)) ||
                ((CPA_CY_SYM_HASH_AES_CCM == pHashSetupData->hashAlgorithm) &&
              (CPA_CY_SYM_CIPHER_AES_CCM != pCipherSetupData->cipherAlgorithm)))
        {
            LAC_INVALID_PARAM_LOG(
                "Invalid combination of Cipher/Hash Algorithms for CCM");
            return CPA_STATUS_INVALID_PARAM;
        }

        /* ensure both hash and cipher algorithms are GCM */
        if (((CPA_CY_SYM_CIPHER_AES_GCM == pCipherSetupData->cipherAlgorithm) &&
             (CPA_CY_SYM_HASH_AES_GCM != pHashSetupData->hashAlgorithm)) ||
            ((CPA_CY_SYM_HASH_AES_GCM == pHashSetupData->hashAlgorithm) &&
             (CPA_CY_SYM_CIPHER_AES_GCM != pCipherSetupData->cipherAlgorithm)))
        {
            LAC_INVALID_PARAM_LOG(
                "Invalid combination of Cipher/Hash Algorithms for GCM");
            return CPA_STATUS_INVALID_PARAM;
        }
    }
    /* not Algorithm Chaining so prevent CCM/GCM being selected */
    else if (CPA_CY_SYM_OP_CIPHER == pSessionSetupData->symOperation)
    {
        /* ensure cipher algorithm is not CCM or GCM */
        if ((CPA_CY_SYM_CIPHER_AES_CCM == pCipherSetupData->cipherAlgorithm) ||
               (CPA_CY_SYM_CIPHER_AES_GCM == pCipherSetupData->cipherAlgorithm))
        {
            LAC_INVALID_PARAM_LOG(
              "Invalid Cipher Algorithm for non-Algorithm Chaining operation");
            return CPA_STATUS_INVALID_PARAM;
        }
    }
    else if (CPA_CY_SYM_OP_HASH == pSessionSetupData->symOperation)
    {
        /* ensure hash algorithm is not CCM or GCM */
        if ((CPA_CY_SYM_HASH_AES_CCM == pHashSetupData->hashAlgorithm) ||
                (CPA_CY_SYM_HASH_AES_GCM == pHashSetupData->hashAlgorithm))
        {
            LAC_INVALID_PARAM_LOG(
                "Invalid Hash Algorithm for non-Algorithm Chaining operation");
            return CPA_STATUS_INVALID_PARAM;
        }
    }
    /* Unsupported operation. Return error */
    else
    {
        LAC_INVALID_PARAM_LOG("symOperation");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* ensure that cipher direction param is
     * valid for cipher and algchain ops */
    if(CPA_CY_SYM_OP_HASH != pSessionSetupData->symOperation)
    {
        if((pCipherSetupData->cipherDirection !=
                                CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT) &&
            (pCipherSetupData->cipherDirection !=
                                CPA_CY_SYM_CIPHER_DIRECTION_DECRYPT))
        {
            LAC_INVALID_PARAM_LOG("Invalid Cipher Direction");
            return CPA_STATUS_INVALID_PARAM;

        }
    }

    return CPA_STATUS_SUCCESS;
}
#endif //ICP_PARAM_CHECK


#ifdef ICP_PARAM_CHECK

/**
 * @ingroup LacSym
 * Function which perform parameter checks on data buffers for symmetric
 * crypto operations
 *
 * @param[in] pSrcBuffer     Pointer to source buffer list
 * @param[in] pDstBuffer     Pointer to destination buffer list
 * @param[in] packetType     Packet type (full/partial/last partial)
 * @param[in] pSessionDesc   Pointer to session descriptor
 *
 * @retval CPA_STATUS_SUCCESS        The operation succeeded
 * @retval CPA_STATUS_INVALID_PARAM  An invalid parameter value was found
 */

STATIC CpaStatus LacSymPerform_BufferParamCheck(
    const CpaBufferList * const pSrcBuffer,
    const CpaBufferList * const pDstBuffer,
    const CpaCySymPacketType packetType,
    const lac_session_desc_t * const pSessionDesc,
    const CpaCySymOpData * const pOpData)
{
    Cpa32U srcBufferLen = 0, dstBufferLen = 0;

    /* verify packet type is in correct range */
    switch (packetType)
    {
        case CPA_CY_SYM_PACKET_TYPE_FULL:
            break;
        case CPA_CY_SYM_PACKET_TYPE_PARTIAL:
        case CPA_CY_SYM_PACKET_TYPE_LAST_PARTIAL:
        {
            /* out of place not supported for partials */
            if (pSrcBuffer != pDstBuffer)
            {
                LAC_INVALID_PARAM_LOG(
                    "partial packets not supported for out of place");
                return CPA_STATUS_INVALID_PARAM;
            }
            break;
        }
        default:
        {
            LAC_INVALID_PARAM_LOG("packetType");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* Verify buffer(s) for source packet & return packet length 
     * The exception to this is for zero length hash requests in 
     * which the BufferList pointer must be valid.*/
    if (!(CPA_CY_SYM_OP_HASH == pSessionDesc->symOperation &&
          CPA_CY_SYM_HASH_MODE_PLAIN == pSessionDesc->hashMode &&
          0 == pOpData->messageLenToHashInBytes))
    {
        if (CPA_STATUS_SUCCESS !=
                LacBuffDesc_BufferListVerify(pSrcBuffer, &srcBufferLen))
        {
            LAC_INVALID_PARAM_LOG("Source buffer invalid");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* out of place checks */
    if (pSrcBuffer != pDstBuffer)
    {
        /* Verify buffer(s) for dest packet & return packet length */
        if (CPA_STATUS_SUCCESS !=  LacBuffDesc_BufferListVerify(pDstBuffer,
                                            &dstBufferLen))
        {
            LAC_INVALID_PARAM_LOG(
                "Destination buffer invalid");
            return CPA_STATUS_INVALID_PARAM;

        }

        /* verify that the destination packet is big enough to hold the data
         * as indicated by the packet length field in the source buffer
         * descriptor.*/
        if (srcBufferLen > dstBufferLen)
        {
            LAC_INVALID_PARAM_LOG(
                "Destination buffer not big enough to hold source packet");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* check for partial packet suport for the session operation */
    if (CPA_CY_SYM_PACKET_TYPE_FULL != packetType)
    {
        if (!(IS_PARTIAL_ON_SYM_OP_SUPPORTED(pSessionDesc->symOperation,
                                             pSessionDesc->hashMode)))
        {
            /* return out here to simplify cleanup */
            LAC_INVALID_PARAM_LOG(
                "partial packets not supported for operation");
            return CPA_STATUS_INVALID_PARAM;
        }
        else
        {
            /* This function checks to see if the partial packet sequence
             * is correct */
            if (CPA_STATUS_SUCCESS != LacSym_PartialPacketStateCheck(packetType,
                    pSessionDesc->partialState))
            {
                LAC_INVALID_PARAM_LOG("Partial packet Type");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
    }

    return CPA_STATUS_SUCCESS;
}

#endif //ICP_PARAM_CHECK


/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/


/* @ingroup LacSym
 * This function is called by cpaCyInitInstance() which ensures consistency
 * between the Lac states. A shutdown cannot be called when LAC has not been
 * initialiased etc.. */
CpaStatus
LacSym_Init(Cpa64U numSymConcurrentReq)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = Lac_MemPoolCreate(&lac_sym_cookie_memory,
                               "SYM COOKIES",
                               numSymConcurrentReq,
                               sizeof(lac_sym_cookie_t),
                               LAC_64BYTE_ALIGNMENT,
                               CPA_TRUE);
    LAC_CHECK_STATUS(status);

    LacSym_StatsInit();

    status = LacSymQat_Init();
    LAC_CHECK_STATUS(status);

    /*** HASH ***/
    LacSymQat_HashLookupInit();

    status = LacSymHash_HmacPrecompInit();
    
    LAC_CHECK_STATUS(status);

    /* Register callbacks for the symmetric services
     * (Hash, Cipher, Algorithm-Chaining) */
    LacSymCb_CallbacksRegister();

    return CPA_STATUS_SUCCESS;
}

/** @ingroup LacSym */
CpaStatus
LacSym_Shutdown(void)
{
    CpaStatus status = LacSymQat_Shutdown();

    LacSym_StatsShutdown();

    Lac_MemPoolDestroy(lac_sym_cookie_memory);

    LacSymHash_HmacPrecompShutdown();

    return status;
}

/** @ingroup LacSym */
CpaStatus
cpaCySymInitSession (const CpaInstanceHandle instanceHandle,
                     const CpaCySymCbFunc pSymCb,
                     const CpaCySymSessionSetupData *pSessionSetupData,
                     CpaCySymSessionCtx pSessionCtx)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_session_desc_t *pSessionDesc = NULL;
    unsigned long physAddress = 0;
    unsigned long physAddressAligned = 0;

#ifdef ICP_PARAM_CHECK
    /* check if LAC is initialised otherwise return an error */
    LAC_INITIALISED_CHECK();

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pSessionSetupData);
    LAC_CHECK_NULL_PARAM(pSessionCtx);

    status = LacSymSession_ParamCheck(pSessionSetupData);
    LAC_CHECK_STATUS(status);

#endif //ICP_PARAM_CHECK

    /* Re-align the session structure to 64 byte alignment */
    physAddress = (unsigned long)LAC_OS_VIRT_TO_PHYS(
                      (Cpa8U *)pSessionCtx + sizeof(void *));

    physAddressAligned = (unsigned long)LAC_ALIGN_POW2_ROUNDUP(
                             physAddress, LAC_64BYTE_ALIGNMENT);

    pSessionDesc = (lac_session_desc_t *)
                    /* Move the session pointer by the physical offset
                    between aligned and unaligned memory */
                    ((Cpa8U *) pSessionCtx + sizeof(void *)
                    + (physAddressAligned-physAddress));

    /* Assume the session desc shall not be created succesfully */
    pSessionDesc->validSessionDesc = CPA_FALSE;

    /* save the aligned pointer in the first bytes (size of unsigned long)
     * of the session memory */
    *((unsigned long *)pSessionCtx) = (unsigned long)pSessionDesc;

    /* Setup content descriptor info structure
     * assumption that content descriptor is the first field in
     * in the session descriptor */
    pSessionDesc->contentDescInfo.pData = (Cpa8U *)pSessionDesc;
    pSessionDesc->contentDescInfo.pDataPhys = physAddressAligned;

    /* Set the Common Session Information */
    pSessionDesc->symOperation = pSessionSetupData->symOperation;

    /* For ansynchronous - use the user supplied callback
     * for synchronous - use the internal synchronous callback */
    pSessionDesc->pSymCb =
        ((void*) NULL != (void *) pSymCb) ? 
          pSymCb : 
          LacSync_GenBufListVerifyCb;

    /* set the session priority for QAT AL*/
    if (CPA_CY_PRIORITY_HIGH == pSessionSetupData->sessionPriority)
    {
        pSessionDesc->qatSessionPriority = QAT_COMMS_PRIORITY_HIGH;
    }
    else if (CPA_CY_PRIORITY_NORMAL == pSessionSetupData->sessionPriority)
    {
        pSessionDesc->qatSessionPriority = QAT_COMMS_PRIORITY_NORMAL;
    }
    else
    {
        LAC_INVALID_PARAM_LOG("sessionPriority");
        status = CPA_STATUS_INVALID_PARAM;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* initialize convenient pointers to cipher and hash contexts */
        const CpaCySymCipherSetupData * const pCipherSetupData =
            (const CpaCySymCipherSetupData * const)
            &pSessionSetupData->cipherSetupData;
        const CpaCySymHashSetupData * const pHashSetupData =
            (const CpaCySymHashSetupData * const)
            &pSessionSetupData->hashSetupData;

        if (((CPA_CY_SYM_CIPHER_AES_CCM == pCipherSetupData->cipherAlgorithm) &&
             (CPA_CY_SYM_HASH_AES_CCM == pHashSetupData->hashAlgorithm)) ||
            ((CPA_CY_SYM_CIPHER_AES_GCM == pCipherSetupData->cipherAlgorithm) &&
             (CPA_CY_SYM_HASH_AES_GCM == pHashSetupData->hashAlgorithm)))
        {
            pSessionDesc->isAuthEncryptOp = CPA_TRUE;
        }
        else
        {
            pSessionDesc->isAuthEncryptOp = CPA_FALSE;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = LacAlgChain_SessionInit(
                     instanceHandle, pSessionSetupData, pSessionDesc);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Increment the stats for a session registered successfully */
        LAC_SYM_STAT_INC(numSessionsInitialized, instanceHandle);

    }
    else /* if there was an error */
    {
        LAC_SYM_STAT_INC(numSessionErrors, instanceHandle);
    }

    return status;
}


/** @ingroup LacSym */
CpaStatus
cpaCySymRemoveSession (const CpaInstanceHandle instanceHandle,
                       CpaCySymSessionCtx pSessionCtx)
{
    lac_session_desc_t *pSessionDesc = NULL;
    CpaStatus status = CPA_STATUS_SUCCESS;

#ifdef ICP_PARAM_CHECK
    /* check if LAC is initialised otherwise return an error */
    LAC_INITIALISED_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pSessionCtx);
#endif //ICP_PARAM_CHECK

    pSessionDesc = LAC_SYM_SESSION_DESC_FROM_CTX_GET(pSessionCtx);

#ifdef ICP_PARAM_CHECK
    if (NULL == pSessionDesc)
    {
        LAC_LOG_ERROR("Session Data not as expected");
        return CPA_STATUS_RESOURCE;
    }
    if (CPA_FALSE == pSessionDesc->validSessionDesc)
    {
        LAC_LOG_ERROR("Session Data not as expected");
        return CPA_STATUS_RESOURCE;
    }
#endif //ICP_PARAM_CHECK

    /* If there are pending requests */
    if (0 != ixOsalAtomicGet(&(pSessionDesc->pendingCbCount)))
    {
        LAC_LOG_ERROR1("There are %d requests pending",
            ixOsalAtomicGet(&(pSessionDesc->pendingCbCount)));
        status = CPA_STATUS_RETRY;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_SPINLOCK_DESTROY(&pSessionDesc->requestQueueLock);
        pSessionDesc->validSessionDesc = CPA_FALSE;
        LAC_SYM_STAT_INC(numSessionsRemoved, instanceHandle);
    }
    else
    {
        LAC_SYM_STAT_INC(numSessionErrors, instanceHandle);
    }


    return status;
}

/** @ingroup LacSym */
STATIC CpaStatus
LacSym_Perform (const CpaInstanceHandle instanceHandle,
                void *callbackTag,
                const CpaCySymOpData *pOpData,
                const CpaBufferList *pSrcBuffer,
                CpaBufferList *pDstBuffer,
                CpaBoolean *pVerifyResult,
                CpaBoolean isAsyncMode)
{
    lac_session_desc_t *pSessionDesc = NULL;
    CpaStatus status = CPA_STATUS_SUCCESS;

#ifdef ICP_PARAM_CHECK
    /* check if LAC is initialised otherwise return an error */
    LAC_INITIALISED_CHECK();

    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pOpData);
    LAC_CHECK_NULL_PARAM(pOpData->pSessionCtx);
    LAC_CHECK_NULL_PARAM(pSrcBuffer);
    LAC_CHECK_NULL_PARAM(pDstBuffer);
#endif //ICP_PARAM_CHECK

    pSessionDesc = LAC_SYM_SESSION_DESC_FROM_CTX_GET(pOpData->pSessionCtx);

#ifdef ICP_PARAM_CHECK
    if (NULL == pSessionDesc)
    {
        LAC_LOG_ERROR("Session Data not as expected");
        return CPA_STATUS_RESOURCE;
    }
#endif //ICP_PARAM_CHECK

    /* If synchronous Operation - Callback function stored in the session
     * descriptor so a flag is set in the perform to indicate that
     * the perform is being re-called for the synchronous operation */
    if ((LacSync_GenBufListVerifyCb == pSessionDesc->pSymCb)
            && isAsyncMode == CPA_TRUE)
    {
        CpaBoolean opResult = CPA_FALSE;
        lac_sync_op_data_t *pSyncCallbackData = NULL;

        status = LacSync_CreateSyncCookie(&pSyncCallbackData);

        if (CPA_STATUS_SUCCESS == status)
        {
            status = LacSym_Perform( instanceHandle,
                        pSyncCallbackData,
                        pOpData,
                        pSrcBuffer,
                        pDstBuffer,
                        pVerifyResult,
                        CPA_FALSE);
        }
        else
        {
            /* Failure allocating sync cookie */
            LAC_SYM_STAT_INC(numSymOpRequestErrors, instanceHandle);
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            CpaStatus syncStatus = CPA_STATUS_SUCCESS;

            syncStatus = LacSync_WaitForCallback(pSyncCallbackData,
                            LAC_SYM_SYNC_CALLBACK_TIMEOUT,
                            &status,
                            &opResult);

            /* If callback doesn't come back */
            if (CPA_STATUS_SUCCESS != syncStatus)
            {
                LAC_SYM_STAT_INC(numSymOpCompletedErrors, instanceHandle);
                LAC_LOG_ERROR("Callback timed out");
                status = syncStatus;
            }
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            if (NULL != pVerifyResult)
            {
                *pVerifyResult = opResult;
            }
        }

        LacSync_DestroySyncCookie(&pSyncCallbackData);

        return status;
    }

#ifdef ICP_PARAM_CHECK
    status = LacSymPerform_BufferParamCheck(
                (const CpaBufferList * const) pSrcBuffer,
                pDstBuffer,
                pOpData->packetType,
                pSessionDesc,
                pOpData);

    LAC_CHECK_STATUS(status);
#endif //ICP_PARAM_CHECK

        status = LacAlgChain_Perform(
                     instanceHandle,
                     pSessionDesc,
                     callbackTag,
                     pOpData,
                     pSrcBuffer,
                     pDstBuffer,
                     pVerifyResult);

    if (CPA_STATUS_SUCCESS == status)
    {
        /* check for partial packet suport for the session operation */
        if (CPA_CY_SYM_PACKET_TYPE_FULL != pOpData->packetType)
        {
            LacSym_PartialPacketStateUpdate(pOpData->packetType,
                                            &pSessionDesc->partialState);
        }

        /* increment #requests stat */
        LAC_SYM_STAT_INC(numSymOpRequests, instanceHandle);
    }
    /* Retry also results in the errors stat been incremented */
    else
    {
        /* increment #errors stat */
        LAC_SYM_STAT_INC(numSymOpRequestErrors, instanceHandle);
    }

    return status;
}

/** @ingroup LacSym */
CpaStatus
cpaCySymPerformOp (const CpaInstanceHandle instanceHandle,
                   void *callbackTag,
                   const CpaCySymOpData *pOpData,
                   const CpaBufferList *pSrcBuffer,
                   CpaBufferList *pDstBuffer,
                   CpaBoolean *pVerifyResult)
{
    return LacSym_Perform (instanceHandle, callbackTag, pOpData,
                           pSrcBuffer, pDstBuffer, pVerifyResult, CPA_TRUE);
}


/** @ingroup LacSym */
CpaStatus
cpaCySymQueryStats(const CpaInstanceHandle instanceHandle,
                   CpaCySymStats *pSymStats)
{
#ifdef ICP_PARAM_CHECK
    /* check if LAC is initialised otherwise return an error */
    LAC_RUNNING_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pSymStats);
#endif //ICP_PARAM_CHECK

    /* copy the fields from the internal structure into the api defined
     * structure */
    LacSym_StatsCopyGet(instanceHandle, pSymStats);

    return CPA_STATUS_SUCCESS;
}

/** @ingroup LacSym */
CpaStatus
cpaCySymSessionCtxGetSize(const CpaInstanceHandle instanceHandle,
                          const CpaCySymSessionSetupData *pSessionSetupData,
                          Cpa32U *pSessionCtxSizeInBytes)
{
#ifdef ICP_PARAM_CHECK
    LAC_RUNNING_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pSessionSetupData);
    LAC_CHECK_NULL_PARAM(pSessionCtxSizeInBytes);
#endif //ICP_PARAM_CHECK

    *pSessionCtxSizeInBytes = LAC_SYM_SESSION_SIZE;

    return CPA_STATUS_SUCCESS;
}

