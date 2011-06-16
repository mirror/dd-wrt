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
 * @file lac_sym_hash.c
 *
 * @ingroup LacHash
 *
 * Hash specific functionality
 ***************************************************************************/


/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

#include "lac_common.h"
#include "lac_mem.h"
#include "lac_session.h"
#include "lac_sym_hash.h"
#include "lac_log.h"
#include "lac_sym_qat_hash.h"
#include "lac_sym_qat_hash_defs_lookup.h"
#include "lac_sym_cb.h"
#include "lac_sync.h"

#define LAC_HASH_ALG_MODE_NOT_SUPPORTED(alg, mode) \
     (((CPA_CY_SYM_HASH_AES_XCBC == (alg)) ||     \
       (CPA_CY_SYM_HASH_AES_CCM == (alg)) ||      \
       (CPA_CY_SYM_HASH_AES_GCM == (alg))) &&     \
       (CPA_CY_SYM_HASH_MODE_AUTH != (mode)))
/**< Macro to check for valid algorithm-mode combination */
      

/**
 * @ingroup LacHash 
 * This callback function will be invoked whenever a synchronous
 * hash precompute operation completes.  It will set the wait
 * queue flag for the synchronous operation.
 * 
 * @param[in] pCallbackTag  Opaque value provided by user. This will
 *                         be a pointer to a wait queue flag.
 * 
 * @retval
 *     None
 *
 */
STATIC void LacHash_SyncPrecomputeDoneCb(void *pCallbackTag)
{
    LacSync_GenWakeupSyncCaller(pCallbackTag, CPA_STATUS_SUCCESS);
}

/** @ingroup LacHash */
void
LacHash_StatePrefixAadBufferInit(
    const CpaCySymHashSetupData *pHashSetupData,
    const icp_qat_fw_auth_hdr_t *pHashControlBlock,
    icp_qat_hw_auth_mode_t qatHashMode,
    Cpa8U *pHashStateBuffer,
    lac_sym_qat_hash_state_buffer_info_t *pHashStateBufferInfo)
{
   /* set up the hash state prefix buffer info structure */
    pHashStateBufferInfo->pData = pHashStateBuffer;
    pHashStateBufferInfo->pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(pHashStateBuffer));

    /* Create the hash state prefix aad buffer */
    LacSymQat_HashStatePrefixAadBufferSizeGet(
        pHashSetupData, pHashControlBlock, qatHashMode, pHashStateBufferInfo);

    /* Prefix data gets copied to the hash state buffer for nested mode */
    if (CPA_CY_SYM_HASH_MODE_NESTED == pHashSetupData->hashMode)
    {
        LacSymQat_HashStatePrefixAadBufferPopulate(
            pHashStateBufferInfo,
            pHashControlBlock,
            pHashSetupData->nestedModeSetupData.pInnerPrefixData,
            pHashSetupData->nestedModeSetupData.innerPrefixLenInBytes,
            pHashSetupData->nestedModeSetupData.pOuterPrefixData,
            pHashSetupData->nestedModeSetupData.outerPrefixLenInBytes);
    }
    /* For mode2 HMAC the key gets copied into both the inner and 
     * outer prefix fields */
    else if ( IS_HASH_MODE_2_AUTH(qatHashMode, pHashSetupData->hashMode) )
    {
        LacSymQat_HashStatePrefixAadBufferPopulate(
            pHashStateBufferInfo,
            pHashControlBlock,
            pHashSetupData->authModeSetupData.authKey,
            pHashSetupData->authModeSetupData.authKeyLenInBytes,
            pHashSetupData->authModeSetupData.authKey,
            pHashSetupData->authModeSetupData.authKeyLenInBytes);
    }
    /* else do nothing for the other cases */ 
}


/** @ingroup LacHash */
CpaStatus
LacHash_PrecomputeDataCreate(
    const CpaInstanceHandle instanceHandle,
    CpaCySymHashAlgorithm hashAlgorithm,
    CpaCySymHashAuthModeSetupData authModeSetupData,
    lac_hash_precompute_done_cb_t callbackFn,
    void *pCallbackTag,
    Cpa8U *pWorkingBuffer,
    Cpa8U *pState1,
    Cpa8U *pState2)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa8U *pAuthKey = NULL;
    Cpa32U authKeyLenInBytes = 0;

    LAC_ENSURE_NOT_NULL(pWorkingBuffer);

    /* synchronous operation */
    if (NULL == callbackFn)
    {
        lac_sync_op_data_t *pSyncCallbackData = NULL;
    
        status = LacSync_CreateSyncCookie(&pSyncCallbackData);

        if (CPA_STATUS_SUCCESS == status)
        { 
            status = LacHash_PrecomputeDataCreate(
                        instanceHandle,
                        hashAlgorithm,
                        authModeSetupData,
                        LacHash_SyncPrecomputeDoneCb,
                        /* wait queue condition from sync cookie */
                        pSyncCallbackData,
                        pWorkingBuffer,
                        pState1,
                        pState2);
        }

        if (CPA_STATUS_SUCCESS == status)
        {
            CpaStatus syncStatus = CPA_STATUS_SUCCESS;
            
            syncStatus = LacSync_WaitForCallback(pSyncCallbackData,
                            LAC_SYM_SYNC_CALLBACK_TIMEOUT,
                            &status,
                            NULL);
    
            /* If callback doesn't come back */
            if (CPA_STATUS_SUCCESS != syncStatus)
            {
                LAC_LOG_ERROR(
                    "callback functions for precomputes did not return");
                status = syncStatus;
            }
        }

        LacSync_DestroySyncCookie(&pSyncCallbackData);

        return status;
    }

    /* set up convenience pointers */
    pAuthKey = authModeSetupData.authKey;
    authKeyLenInBytes = authModeSetupData.authKeyLenInBytes;

    /* Pre-compute data state pointers must already be set up
     * by LacSymQat_HashSetupBlockInit() 
     */

    /* state1 is not allocated for AES XCBC/CCM/GCM */
    if ((CPA_CY_SYM_HASH_AES_XCBC == hashAlgorithm) ||
        (CPA_CY_SYM_HASH_AES_CCM == hashAlgorithm) ||
        (CPA_CY_SYM_HASH_AES_GCM == hashAlgorithm))
    {
        if (CPA_CY_SYM_HASH_AES_XCBC == hashAlgorithm)
        {
            status = LacSymHash_AesECBPreCompute(instanceHandle,
                            hashAlgorithm,
                            authKeyLenInBytes,
                            pAuthKey,
                            pWorkingBuffer,
                            pState2,
                            callbackFn,
                            pCallbackTag);
        }
        else if (CPA_CY_SYM_HASH_AES_CCM == hashAlgorithm)
        {
             /*
             * The Inner Hash Initial State2 block must contain K 
             * (the cipher key) and 16 zeroes which will be replaced with 
             * EK(Ctr0) by the QAT-ME.
             */

            /* write the auth key */
            memcpy(pState2, pAuthKey, authKeyLenInBytes);

            /* initialize remaining buffer space to all zeroes */
           LAC_OS_BZERO(pState2 + authKeyLenInBytes, 
           ICP_QAT_HW_AES_CCM_CBC_E_CTR0_SZ);

            /* There is no request sent to the QAT for this operation,
             * so just invoke the user's callback directly to signal
             * completion of the precompute
             */
            callbackFn(pCallbackTag);
        }
        else if (CPA_CY_SYM_HASH_AES_GCM == hashAlgorithm)
        {
            /*
             * The Inner Hash Initial State2 block contains the following 
             *      H (the Galois Hash Multiplier) 
             *      len(A) (the length of A), (length before padding)
             *      16 zeroes which will be replaced with EK(Ctr0) by the QAT.
             */
           
            /* Memset state2 to 0 */ 
            LAC_OS_BZERO(pState2, ICP_QAT_HW_GALOIS_H_SZ + 
                    ICP_QAT_HW_GALOIS_LEN_A_SZ + ICP_QAT_HW_GALOIS_E_CTR0_SZ);
 
            /* write H (the Galois Hash Multiplier) where H = E(K, 0...0)
             * This will only write bytes 0-15 of pState2
             */
            status = LacSymHash_AesECBPreCompute(instanceHandle,
                         hashAlgorithm,
                         authKeyLenInBytes,
                         pAuthKey,
                         pWorkingBuffer,
                         pState2,
                         callbackFn,
                         pCallbackTag);
    
            if (CPA_STATUS_SUCCESS == status)
            {
                /* write len(A) (the length of A) into bytes 16-19 of pState2
                 * in big-endian format. This field is 8 bytes */
                *(Cpa32U *)&pState2[ICP_QAT_HW_GALOIS_H_SZ] =
                    LAC_MEM_WR_32(authModeSetupData.aadLenInBytes);
            }
        }
    }
    else /* For Hmac Precomputes */
    {
        status =
        LacSymHash_HmacPreComputes(instanceHandle,
                            hashAlgorithm,
                            authKeyLenInBytes,
                            pAuthKey,
                            pWorkingBuffer,
                            pState1,
                            pState2,
                            callbackFn,
                            pCallbackTag);
    }

    return status;
}

#ifdef ICP_PARAM_CHECK

/** @ingroup LacHash */
CpaStatus
LacHash_HashContextCheck(const CpaCySymHashSetupData *pHashSetupData)
{
    lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;
    lac_sym_qat_hash_alg_info_t *pOuterHashAlgInfo = NULL;

    LAC_ENSURE_NOT_NULL(pHashSetupData);

    if( (pHashSetupData->hashAlgorithm < CPA_CY_HASH_ALG_START) ||
        (pHashSetupData->hashAlgorithm > CPA_CY_HASH_ALG_END) )
    {
        LAC_INVALID_PARAM_LOG("hashAlgorithm");
        return CPA_STATUS_INVALID_PARAM;
    }

    switch (pHashSetupData->hashMode)
    {
        case CPA_CY_SYM_HASH_MODE_PLAIN:
        case CPA_CY_SYM_HASH_MODE_AUTH:
        case CPA_CY_SYM_HASH_MODE_NESTED:
            break;

        default:
        {
            LAC_INVALID_PARAM_LOG("hashMode");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    if (LAC_HASH_ALG_MODE_NOT_SUPPORTED(pHashSetupData->hashAlgorithm,
                                        pHashSetupData->hashMode))
    {
        LAC_INVALID_PARAM_LOG("hashAlgorithm and hashMode combination");
        return CPA_STATUS_INVALID_PARAM;
    }

    LacSymQat_HashAlgLookupGet(pHashSetupData->hashAlgorithm, &pHashAlgInfo);

    /* note: nested hash mode checks digest length against outer algorithm */
    if ((CPA_CY_SYM_HASH_MODE_PLAIN == pHashSetupData->hashMode) ||
        (CPA_CY_SYM_HASH_MODE_AUTH == pHashSetupData->hashMode))
    {
        /* Check Digest Length is permitted by the algorithm  */
        if ( (0 == pHashSetupData->digestResultLenInBytes) ||
             (pHashSetupData->digestResultLenInBytes >
                                                pHashAlgInfo->digestLength))
        {
            LAC_INVALID_PARAM_LOG("digestResultLenInBytes");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    if (CPA_CY_SYM_HASH_MODE_AUTH == pHashSetupData->hashMode)
    {
        if (CPA_CY_SYM_HASH_AES_GCM == pHashSetupData->hashAlgorithm)
        {
            /* ensure auth key len is valid (128/192/256-bit keys supported) */
            if ((pHashSetupData->authModeSetupData.authKeyLenInBytes !=
                     ICP_QAT_HW_AES_128_KEY_SZ) &&
                (pHashSetupData->authModeSetupData.authKeyLenInBytes !=
                     ICP_QAT_HW_AES_192_KEY_SZ) &&
                (pHashSetupData->authModeSetupData.authKeyLenInBytes !=
                     ICP_QAT_HW_AES_256_KEY_SZ))
            {
                LAC_INVALID_PARAM_LOG("authKeyLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }

            /* RFC 4106: Implementations MUST support a full-length 16-octet 
             * ICV, and MAY support 8 or 12 octet ICVs, and MUST NOT support 
             * other ICV lengths. */ 
            if ((pHashSetupData->digestResultLenInBytes !=
                     LAC_HASH_AES_GCM_ICV_SIZE_8) &&
                (pHashSetupData->digestResultLenInBytes !=
                     LAC_HASH_AES_GCM_ICV_SIZE_12) &&
                (pHashSetupData->digestResultLenInBytes !=
                     LAC_HASH_AES_GCM_ICV_SIZE_16))
            {
                LAC_INVALID_PARAM_LOG("digestResultLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }

            /* ensure aadLen is within maximum limit imposed by QAT */
            if (pHashSetupData->authModeSetupData.aadLenInBytes > 
                    ICP_QAT_FW_CCM_GCM_AAD_SZ_MAX)
            {
                LAC_INVALID_PARAM_LOG("aadLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        else if (CPA_CY_SYM_HASH_AES_CCM == pHashSetupData->hashAlgorithm)
        {
            /* QAT-FW only supports 128 bit keys for AES-CBC-MAC part of CCM */
            if (pHashSetupData->authModeSetupData.authKeyLenInBytes !=
                    ICP_QAT_HW_AES_128_KEY_SZ)
            {
                LAC_INVALID_PARAM_LOG("authKeyLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }

            /* RFC 3610: Valid values are 4, 6, 8, 10, 12, 14, and 16 octets */
            if ((pHashSetupData->digestResultLenInBytes >= 
                 LAC_HASH_AES_CCM_ICV_SIZE_MIN) &&
                (pHashSetupData->digestResultLenInBytes <= 
                 LAC_HASH_AES_CCM_ICV_SIZE_MAX))
            {
                if ((pHashSetupData->digestResultLenInBytes & 0x01) != 0)
                {
                    LAC_INVALID_PARAM_LOG(
                        "digestResultLenInBytes must be a multiple of 2");
                    return CPA_STATUS_INVALID_PARAM;
                }
            }
            else
            {
                LAC_INVALID_PARAM_LOG("digestResultLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        else
        {
            /* The key size must be less than or equal the block length */
            if (pHashSetupData->authModeSetupData.authKeyLenInBytes > 
                pHashAlgInfo->blockLength)
            {
                LAC_INVALID_PARAM_LOG("authKeyLenInBytes");
                return CPA_STATUS_INVALID_PARAM;
            }
        }

        /* when the key size is greater than 0 check pointer is not null */
        if (pHashSetupData->authModeSetupData.authKeyLenInBytes > 0)
        {
            LAC_CHECK_NULL_PARAM(pHashSetupData->authModeSetupData.authKey);
        }
    }
    else if (CPA_CY_SYM_HASH_MODE_NESTED == pHashSetupData->hashMode)
    {
        if( (pHashSetupData->nestedModeSetupData.outerHashAlgorithm 
                < CPA_CY_HASH_ALG_START) ||
            (pHashSetupData->nestedModeSetupData.outerHashAlgorithm 
                > CPA_CY_HASH_ALG_END) )
        {
            LAC_INVALID_PARAM_LOG("outerHashAlgorithm");
            return CPA_STATUS_INVALID_PARAM;
        }

        if (LAC_HASH_ALG_MODE_NOT_SUPPORTED(
                pHashSetupData->nestedModeSetupData.outerHashAlgorithm,
                pHashSetupData->hashMode))
        {
            LAC_INVALID_PARAM_LOG(
                "outerHashAlgorithm and hashMode combination");
            return CPA_STATUS_INVALID_PARAM;
        }

        LacSymQat_HashAlgLookupGet(
                         pHashSetupData->nestedModeSetupData.outerHashAlgorithm,
                         &pOuterHashAlgInfo);

        /* Check Digest Length is permitted by the algorithm  */
        if ( (0 == pHashSetupData->digestResultLenInBytes) ||
             (pHashSetupData->digestResultLenInBytes > 
              pOuterHashAlgInfo->digestLength))
        {
            LAC_INVALID_PARAM_LOG("digestResultLenInBytes");
            return CPA_STATUS_INVALID_PARAM;
        }

        if (pHashSetupData->nestedModeSetupData.innerPrefixLenInBytes >
                LAC_MAX_INNER_OUTER_PREFIX_SIZE_BYTES)
        {
            LAC_INVALID_PARAM_LOG("innerPrefixLenInBytes");
            return CPA_STATUS_INVALID_PARAM;
        }

        if (pHashSetupData->nestedModeSetupData.innerPrefixLenInBytes > 0)
        {
            LAC_CHECK_NULL_PARAM(
                          pHashSetupData->nestedModeSetupData.pInnerPrefixData);
        }

        if (pHashSetupData->nestedModeSetupData.outerPrefixLenInBytes >
                LAC_MAX_INNER_OUTER_PREFIX_SIZE_BYTES)
        {
            LAC_INVALID_PARAM_LOG("outerPrefixLenInBytes");
            return CPA_STATUS_INVALID_PARAM;
        }

        if (pHashSetupData->nestedModeSetupData.outerPrefixLenInBytes > 0)
        {
            LAC_CHECK_NULL_PARAM(
                          pHashSetupData->nestedModeSetupData.pOuterPrefixData);
        }
    }

    return CPA_STATUS_SUCCESS;
}



/** @ingroup LacHash */
CpaStatus
LacHash_PerformParamCheck(
    lac_session_desc_t *pSessionDesc,
    const CpaCySymOpData *pOpData,
    Cpa32U srcPktSize,
    const CpaBoolean *pVerifyResult)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;

    /* check the digest result pointer */
    if ((CPA_CY_SYM_PACKET_TYPE_PARTIAL != pOpData->packetType) &&
        (NULL == pOpData->pDigestResult))
    {
        LAC_INVALID_PARAM_LOG("pDigestResult is NULL");
        return CPA_STATUS_INVALID_PARAM;
    } 

    /*
     * Check if the pVerifyResult pointer is not null for hash operation when 
     * the packet is the last one and user set digestVerify flag
     */ 
    if ((CPA_TRUE == pOpData->digestVerify) && 
        (CPA_CY_SYM_PACKET_TYPE_PARTIAL != pOpData->packetType)) 
    {
        if( NULL == pVerifyResult ) 
        {
            LAC_INVALID_PARAM_LOG("Null pointer pVerifyResult for hash op");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* verify start offset + messageLenToDigest is inside the source packet.
     * this also verifies that the start offset is inside the packet
     * Note: digest is specified as a pointer therefore it can be 
     * written anywhere so we cannot check for this been inside a buffer 
     * CCM/GCM specify the auth region using just the cipher params as this
     * region is the same for auth and cipher. It is not checked here */
    if ((CPA_CY_SYM_HASH_AES_CCM == pSessionDesc->hashAlgorithm) ||
        (CPA_CY_SYM_HASH_AES_GCM == pSessionDesc->hashAlgorithm))
    {
        /* ensure AAD data pointer is non-NULL if AAD len > 0 */
        if ((pSessionDesc->aadLenInBytes > 0) &&
            (NULL == pOpData->pAdditionalAuthData))
        {
            LAC_INVALID_PARAM_LOG("pAdditionalAuthData is NULL");
            return CPA_STATUS_INVALID_PARAM;
        }
    }
    else
    {
        if (pOpData->messageLenToHashInBytes > LAC_MAX_16_BIT_VALUE)
        {
            LAC_INVALID_PARAM_LOG("messageLenToHashInBytes must be "
                "a 16 bit number");
            return CPA_STATUS_INVALID_PARAM; 
        }

        if ((pOpData->hashStartSrcOffsetInBytes +
             pOpData->messageLenToHashInBytes) > srcPktSize)
        {
            LAC_INVALID_PARAM_LOG("hashStartSrcOffsetInBytes + "
                "messageLenToHashInBytes > Src Buffer Packet Length");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    /* partial packets need to be multiples of the algorithm block size
     * (except for final partial packet) */
    if (CPA_CY_SYM_PACKET_TYPE_PARTIAL == pOpData->packetType)
    {
        LacSymQat_HashAlgLookupGet(pSessionDesc->hashAlgorithm, &pHashAlgInfo);

        /* check if the message is a multiple of the block size. A mask is
         * used for this seeing that the block size is a power of 2 */
        if ((pOpData->messageLenToHashInBytes &
            (pHashAlgInfo->blockLength - 1)) != 0)
        {
            LAC_INVALID_PARAM_LOG("messageLenToHashInBytes not block size");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    return status;
}

#endif //ICP_PARAM_CHECK
