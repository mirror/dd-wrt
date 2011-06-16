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
 * @file lac_sym_hash_precomputes.c
 *
 * @ingroup LacHash
 *
 * Hash Precomputes
 ***************************************************************************/

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "icp_qat_hw.h"
#include "IxOsal.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/
#include "lac_mem.h"
#include "lac_log.h"
#include "lac_sym.h"
#include "lac_session.h"
#include "lac_sym_hash.h"
#include "lac_sym_qat.h"
#include "lac_sym_qat_hash.h"
#include "lac_sym_qat_cipher.h"
#include "lac_sym_hash_defs.h"
#include "lac_sym_qat_hash_defs_lookup.h"

#define LAC_SINGLE_BUFFER_HW_META_SIZE \
    (sizeof(icp_buffer_list_desc_t) + sizeof(icp_flat_buffer_desc_t))
/**< size of memory to allocate for the HW buffer list that is sent to the 
 * QAT */

#define LAC_SYM_HASH_PRECOMP_MAX_WORKING_BUFFER         \
    ( (sizeof(lac_sym_hash_precomp_op_data_t) * 2) +    \
       sizeof(lac_sym_hash_precomp_op_t))
/**< maximum size of the working data for the HMAC precompute operations
 * 
 * Maximum size of lac_sym_hash_precomp_op_data_t is 264 bytes. For hash 
 * precomputes there are 2 of these structrues and a further 
 * lac_sym_hash_precomp_op_t structure required. This comes to a total of 536 
 * bytes. 
 * For the asynchronous version of the precomputes, the memory for the hash 
 * state prefix buffer is used as the working memory. There are 584 bytes 
 * which are alloacted for the hash state prefix buffer which is enough to
 * carve up for the precomputes.
 */

#define LAC_SYM_HASH_PRECOMP_MAX_AES_ECB_DATA   \
    ((ICP_QAT_HW_AES_128_KEY_SZ) * (3))
/**< Maximum size for the data that an AES ECB precompute is generated on */


/**
 *****************************************************************************
 * @ingroup LacHash
 *      Precompute type enum
 * @description
 *      Enum used to distinguish between precompute types
 * 
 *****************************************************************************/
typedef enum 
{
    LAC_SYM_HASH_PRECOMP_HMAC = 1,
    /**< Hmac precompute operation. Copy state from hash state buffer */
    LAC_SYM_HASH_PRECOMP_AES_ECB,
    /**< XCBC/CGM precompute, Copy state from data buffer */
} lac_sym_hash_precomp_type_t;

/**
 *****************************************************************************
 * @ingroup LacHash
 *      overall precompute management structure
 * @description
 *      structure used to manage the precompute operations for a session
 * 
 *****************************************************************************/
typedef struct lac_sym_hash_precomp_op_s
{
    lac_hash_precompute_done_cb_t callbackFn;
    /**< Callback function to be invoked when the final precompute completes */
    void *pCallbackTag;
    /**< Opaque data to be passed back as a parameter in the callback */
    IxOsalAtomic opsPending;
    /**< counter used to determine that the current precompute is the final one
     * so that the condition above can be set */
}lac_sym_hash_precomp_op_t;

/**
 *****************************************************************************
 * @ingroup LacHash
 *      hmac precompute structure as used by the QAT
 * @description
 *      data used by the QAT for HMAC precomptes
 *
 *      Must be allocated on an 8-byte aligned memory address.
 * 
 *****************************************************************************/
typedef struct lac_sym_hash_hmac_precomp_qat_s
{
    Cpa8U data[LAC_HASH_SHA512_BLOCK_SIZE];
    /**< data to be hashed - block size of data for the algorithm */   
    /* NOTE: to save space we could have got the QAT to overwrite 
     * this with the hash state storage */
    icp_qat_fw_la_auth_req_params_t hashReqParams;
    /**< Request parameters as read in by the QAT */        
    Cpa8U bufferDesc[LAC_SINGLE_BUFFER_HW_META_SIZE];
    /**< Buffer descriptor structure */
    Cpa8U hashStateStorage[LAC_MAX_HASH_STATE_STORAGE_SIZE]; 
    /**< Internal buffer where QAT writes the intermediate partial
     * state that is used in the precompute */
} lac_sym_hash_hmac_precomp_qat_t;

/**
 *****************************************************************************
 * @ingroup LacHash
 *      AES ECB precompute structure as used by the QAT
 * @description
 *      data used by the QAT for AES ECB precomptes
 *
 *      Must be allocated on an 8-byte aligned memory address.
 * 
 *****************************************************************************/
typedef struct lac_sym_hash_aes_precomp_qat_s
{
    Cpa8U contentDesc[LAC_SYM_QAT_CIPHER_CONTENT_DESC_SIZE];
    /**< Content descriptor for a cipher operation */
    Cpa8U data[LAC_SYM_HASH_PRECOMP_MAX_AES_ECB_DATA];
    /**< The data to be ciphered is conatined here and the result is 
     * written in place back into this buffer */
    icp_qat_fw_la_cipher_req_params_t cipherReqParams;
    /**< Request parameters as read in by the QAT */
    Cpa8U bufferDesc[LAC_SINGLE_BUFFER_HW_META_SIZE];
    /**< Buffer descriptor structure */
} lac_sym_hash_aes_precomp_qat_t;

/**
 *****************************************************************************
 * @ingroup LacHash
 *      overall structure for managing a single precompute operation
 * @description
 *      overall structure for managing a single precompute operation
 *
 *      Must be allocated on an 8-byte aligned memory address.
 * 
 *****************************************************************************/
typedef struct lac_sym_hash_precomp_op_data_s
{
    lac_sym_hash_precomp_type_t opType;
    /**< operation type to determine the precompute type in the callback */ 
    lac_sym_hash_precomp_op_t *pOpStatus;
    /**< structure containing the counter and the condition for the overall
     * precompute operation. This is a pointer because the memory structure
     * may be shared between precomputes when there are more than 1 as in the
     * case of HMAC */
    union 
    {
        lac_sym_hash_hmac_precomp_qat_t hmacQatData;
        /**< Data sent to the QAT for hmac precomputes */
        lac_sym_hash_aes_precomp_qat_t aesQatData;    
        /**< Data sent to the QAT for AES ECB precomputes */ 
    } u;    
    /**< ASSUMPTION: The above structures are 8 byte aligned if the overall
     * struct is 8 byte aligned, as there are two 4 byte fields before this
     * union */
    Cpa32U stateSize;
    /**< Size of the state to be copied into the state pointer in the content 
     * descriptor */ 
    Cpa8U *pState;
    /**< pointer to the state in the content descriptor where the result of
     * the precompute should be copied to */
} lac_sym_hash_precomp_op_data_t;


STATIC Cpa8U *pHmacContentDesc[CPA_CY_HASH_ALG_END + 1] = {0};
/**< ingroup LacHash
 * array of pointers to the hash content desciptors for the HMAC operations */

/*** Local functions prototypes ***/
STATIC CpaStatus
LacSymHash_HmacPreCompute(CpaInstanceHandle instanceHandle,
                          CpaCySymHashAlgorithm hashAlgorithm,
                          lac_sym_hash_precomp_op_data_t *pHashOpData);

STATIC void 
LacSymHash_PrecompCbFunc(icp_qat_fw_la_cmd_id_t lacCmdId,
                         void *pOpaqueData,
                         icp_qat_fw_comn_flags cmnRespFlags);



CpaStatus
icp_LacHashPrecomputeSynchronous(
    CpaCySymHashAlgorithm hashAlgorithm,
    CpaCySymHashAuthModeSetupData authModeSetupData,
    Cpa8U *pState1,
    Cpa8U *pState2)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa8U *pWorkingBuffer = NULL;

#ifdef ICP_PARAM_CHECK
    lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;

    if( (hashAlgorithm < CPA_CY_HASH_ALG_START) ||
        (hashAlgorithm > CPA_CY_HASH_ALG_END) )
    {
        LAC_INVALID_PARAM_LOG("hashAlgorithm");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* state1 is not allocated for AES XCBC/CCM/GCM */
    if ((CPA_CY_SYM_HASH_AES_XCBC == hashAlgorithm) ||
        (CPA_CY_SYM_HASH_AES_CCM == hashAlgorithm) ||
        (CPA_CY_SYM_HASH_AES_GCM == hashAlgorithm))
    {
        if (NULL != pState1)
        {
            LAC_INVALID_PARAM_LOG(
                "pState1 must be NULL for selected algorithm");
            return CPA_STATUS_INVALID_PARAM;
        }
    }
    else
    {
        if (NULL == pState1)
        {
            LAC_INVALID_PARAM_LOG(
                "pState1 cannot be NULL");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    if (NULL == pState2)
    {
        LAC_INVALID_PARAM_LOG("pState2 cannot be NULL");
        return CPA_STATUS_INVALID_PARAM;
    }

    LacSymQat_HashAlgLookupGet(hashAlgorithm, &pHashAlgInfo);

    /* The key size must be less than or equal the block length */
    if (authModeSetupData.authKeyLenInBytes >
        pHashAlgInfo->blockLength)
    {
        LAC_INVALID_PARAM_LOG("authKeyLenInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* when the key size is greater than 0 check pointer is not null */
    if (authModeSetupData.authKeyLenInBytes > 0)
    {
        LAC_CHECK_NULL_PARAM(authModeSetupData.authKey);
    }

#endif //ICP_PARAM_CHECK
    
    status = LAC_OS_MALLOC_ALIGNED(&pWorkingBuffer, 
                LAC_SYM_HASH_PRECOMP_MAX_WORKING_BUFFER,
                LAC_8BYTE_ALIGNMENT);
    
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_LOG_ERROR("Failed to allocate working memory");
        return status;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        status = LacHash_PrecomputeDataCreate(
                    CPA_INSTANCE_HANDLE_SINGLE,
                    hashAlgorithm,
                    authModeSetupData,
                    NULL, /* NULL selects synchronous behaviour */
                    NULL, /* Ignored if previous param is NULL */
                    pWorkingBuffer,
                    pState1,
                    pState2);
    }

    LAC_OS_FREE_ALIGNED(pWorkingBuffer);
       
    return status;
}


/**
*******************************************************************************
 * @ingroup LacHash
 *      Perform single hash precompute operation for HMAC      
 *
 * @description
 *      This function builds up a request and sends it to the QAT for a single
 *      hmac precompute operation. 
 * 
 * @param[in]  instanceHandle       Instance Handle
 * @param[in]  hashAlgorithm        Hash Algorithm
 * @param[in]  pHashOpData          Operation data used for a single hmac 
 *                                  precompute operation for a session
 *
 * @retval CPA_STATUS_SUCCESS       Success
 * @retval CPA_STATUS_RETRY         Retry the operation.
 * @retval CPA_STATUS_FAIL          Operation Failed 
 *
 *****************************************************************************/
STATIC CpaStatus
LacSymHash_HmacPreCompute(CpaInstanceHandle instanceHandle,
                          CpaCySymHashAlgorithm hashAlgorithm,
                          lac_sym_hash_precomp_op_data_t *pHashOpData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    icp_buffer_list_desc_t *pBufferListDesc = NULL;
    icp_flat_buffer_desc_t *pCurrFlatBufDesc = NULL;
    lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;
    Cpa64U srcAddrPhys = 0;
    Cpa16U laCmdFlags = 0;
    lac_sym_qat_hash_state_buffer_info_t hashStateBufferInfo = {0};
    lac_sym_qat_content_desc_info_t contentDescInfo = {0};
    icp_qat_fw_la_bulk_req_t bulkMsg;
    
    /* Convenience pointer */
    lac_sym_hash_hmac_precomp_qat_t *pHmacQatData =
        &pHashOpData->u.hmacQatData;

    LacSymQat_HashAlgLookupGet(hashAlgorithm, &pHashAlgInfo);

    hashStateBufferInfo.pData = pHmacQatData->hashStateStorage; 
    hashStateBufferInfo.pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(pHmacQatData->hashStateStorage)); 
    hashStateBufferInfo.stateStorageSzQuadWords = LAC_BYTES_TO_QUADWORDS(
        sizeof(icp_qat_hw_auth_counter_t) + LAC_ALIGN_POW2_ROUNDUP(
        pHashAlgInfo->stateSize, LAC_QUAD_WORD_IN_BYTES));
    hashStateBufferInfo.prefixAadSzQuadWords = 0;

    contentDescInfo.pData = pHmacContentDesc[hashAlgorithm];
    contentDescInfo.pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(contentDescInfo.pData));
    contentDescInfo.hdrSzQuadWords = LAC_BYTES_TO_QUADWORDS(
        sizeof(icp_qat_fw_auth_hdr_t));
    contentDescInfo.hwBlkSzQuadWords = LAC_BYTES_TO_QUADWORDS(
        LAC_SYM_QAT_MAX_HASH_SETUP_BLK_SZ); 

    /* populate the hash request parameters */
    LacSymQat_HashRequestParamsPopulate(
        &(pHmacQatData->hashReqParams),
        ICP_QAT_FW_SLICE_NULL,
        &(hashStateBufferInfo),
        ICP_QAT_FW_LA_PARTIAL_START,
        pHashAlgInfo->digestLength,
        0,      /* Auth offset */
        pHashAlgInfo->blockLength, /* Auth Length */
        FALSE,  /* Verify disabled */
        NULL);  /* QAT doesnt write the result for first partial */

    pBufferListDesc = (icp_buffer_list_desc_t *)pHmacQatData->bufferDesc; 
    pBufferListDesc->numBuffers = 1;

    pCurrFlatBufDesc = (icp_flat_buffer_desc_t *)(pBufferListDesc->phyBuffers);
    pCurrFlatBufDesc->dataLenInBytes = pHashAlgInfo->blockLength;
    pCurrFlatBufDesc->phyBuffer = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(pHmacQatData->data));        

    srcAddrPhys = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(pBufferListDesc));

    /* proto, cmp_auth, ret_auth, apd_auth, wr_state, partial */
    laCmdFlags = ICP_QAT_FW_LA_FLAGS_BUILD(
                    ICP_QAT_FW_LA_NO_PROTO, 
                    ICP_QAT_FW_LA_NO_CMP_AUTH_RES, 
                    ICP_QAT_FW_LA_NO_RET_AUTH_RES, 
                    ICP_QAT_FW_LA_NO_APD_AUTH_RES,
                    ICP_QAT_FW_LA_UPDATE_STATE, 
                    ICP_QAT_FW_LA_PARTIAL_START);

    /* populate the bulk request */
    LacSymQat_BulkReqPopulate(
        &bulkMsg,
        &contentDescInfo,   
        pHashOpData,    /* put in the opaque data field */
        srcAddrPhys,
        0,
        ICP_QAT_FW_LA_CMD_AUTH_PRE_COMP,
        &(pHmacQatData->hashReqParams),
        sizeof(icp_qat_fw_la_auth_req_params_t),
        laCmdFlags);

    /* create the request header */
    status = QatComms_ReqHdrCreate(&bulkMsg, ICP_ARCH_IF_REQ_QAT_FW_LA);

    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatComms_MsgSend(
                    &bulkMsg,
                    ICP_ARCH_IF_REQ_QAT_FW_LA,
                    QAT_COMMS_PRIORITY_HIGH,
                    instanceHandle);
    }

    return status;
}


CpaStatus
LacSymHash_HmacPreComputes( CpaInstanceHandle instanceHandle,
                            CpaCySymHashAlgorithm hashAlgorithm,
                            Cpa32U authKeyLenInBytes,
                            Cpa8U *pAuthKey,
                            Cpa8U *pWorkingMemory,
                            Cpa8U *pState1,
                            Cpa8U *pState2,
                            lac_hash_precompute_done_cb_t callbackFn,
                            void *pCallbackTag)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Memory is carved up for pHmacIpadOpData and for pHmacIpadOpData
     * one after another. As the structure size is a multiple of 8, if the
     * first one is aligned on an 8 byte boundary, so too will the second 
     * structure. The pOpStatus structure is carved up just after these two
     * structures and has no alignment constraints. Pointer arithemtic is
     * used to carve the memory up. */
    lac_sym_hash_precomp_op_data_t *pHmacIpadOpData = 
        (lac_sym_hash_precomp_op_data_t *)pWorkingMemory;
    lac_sym_hash_precomp_op_data_t *pHmacOpadOpData = 
        pHmacIpadOpData + 1;
    lac_sym_hash_precomp_op_t *pOpStatus=
        (lac_sym_hash_precomp_op_t *)(pHmacOpadOpData + 1);

    /* Convenience pointers */
    lac_sym_hash_hmac_precomp_qat_t *pHmacIpadQatData = 
        &pHmacIpadOpData->u.hmacQatData;
    lac_sym_hash_hmac_precomp_qat_t *pHmacOpadQatData = 
        &pHmacOpadOpData->u.hmacQatData;

    lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;
    Cpa32U wordIndex = 0;
    Cpa32U padLenBytes = 0;

    LacSymQat_HashAlgLookupGet(hashAlgorithm, &pHashAlgInfo);

    /* Initialise opsPending to the number of operations
     * needed to complete this precompute
     */
    ixOsalAtomicSet(2, &(pOpStatus->opsPending));
    pOpStatus->callbackFn = callbackFn;
    pOpStatus->pCallbackTag = pCallbackTag;

    pHmacIpadOpData->opType = LAC_SYM_HASH_PRECOMP_HMAC;
    pHmacIpadOpData->pOpStatus = pOpStatus;
    pHmacIpadOpData->pState = pState1;
    pHmacIpadOpData->stateSize = pHashAlgInfo->stateSize;

    pHmacOpadOpData->opType = LAC_SYM_HASH_PRECOMP_HMAC;
    pHmacOpadOpData->pOpStatus = pOpStatus;
    pHmacOpadOpData->pState = pState2;
    pHmacOpadOpData->stateSize = pHashAlgInfo->stateSize;

    /* Copy HMAC key into buffers */
    if (authKeyLenInBytes > 0)
    {
        memcpy(pHmacIpadQatData->data, pAuthKey, authKeyLenInBytes);
        memcpy(pHmacOpadQatData->data, pAuthKey, authKeyLenInBytes);
    }

    padLenBytes = pHashAlgInfo->blockLength - authKeyLenInBytes;

    /* Clear the remaining buffer space */
    if (padLenBytes > 0)
    {
        LAC_OS_BZERO(pHmacIpadQatData->data + authKeyLenInBytes, padLenBytes);
        LAC_OS_BZERO(pHmacOpadQatData->data + authKeyLenInBytes, padLenBytes);
    }

    /* XOR Key with IPAD at 4-byte level */
    for (wordIndex = 0; 
         wordIndex < LAC_BYTES_TO_LONGWORDS(pHashAlgInfo->blockLength);
         wordIndex ++)
    {
        Cpa32U *pIpadData = ((Cpa32U *)pHmacIpadQatData->data) + wordIndex;
        Cpa32U *pOpadData = ((Cpa32U *)pHmacOpadQatData->data) + wordIndex;

        LAC_MEM_WR_32(*pIpadData ^= LAC_HASH_IPAD_4_BYTES);
        LAC_MEM_WR_32(*pOpadData ^= LAC_HASH_OPAD_4_BYTES);
    }

    status = LacSymHash_HmacPreCompute(instanceHandle, 
                hashAlgorithm, pHmacIpadOpData);

    if (CPA_STATUS_SUCCESS == status)
    {
        status = LacSymHash_HmacPreCompute(instanceHandle, 
                    hashAlgorithm, pHmacOpadOpData);
    }

    return status;
}


CpaStatus
LacSymHash_AesECBPreCompute(CpaInstanceHandle instanceHandle,
                            CpaCySymHashAlgorithm hashAlgorithm,
                            Cpa32U authKeyLenInBytes,
                            Cpa8U *pAuthKey,
                            Cpa8U *pWorkingMemory,
                            Cpa8U *pState,
                            lac_hash_precompute_done_cb_t callbackFn,
                            void *pCallbackTag)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    /* Carve up memory for the following structures: */
    lac_sym_hash_precomp_op_data_t *pAesOpData =
        (lac_sym_hash_precomp_op_data_t *)pWorkingMemory;
    lac_sym_hash_precomp_op_t *pOpStatus=
        (lac_sym_hash_precomp_op_t *)(pAesOpData + 1);
    /* Convenience pointer */
    lac_sym_hash_aes_precomp_qat_t *pAesQatData = 
         &pAesOpData->u.aesQatData;
    icp_buffer_list_desc_t *pBufferListDesc = NULL;
    icp_flat_buffer_desc_t *pCurrFlatBufDesc = NULL;
    CpaCySymCipherSetupData cipherSetupData = {0};
    Cpa32U cipherHwBlockSizeBytes = 0;
    lac_sym_qat_content_desc_info_t contentDescInfo = {0};
    icp_qat_fw_la_bulk_req_t bulkMsg;
    Cpa64U srcAddrPhys = 0;
    Cpa16U laCmdFlags = 0;
    Cpa32U stateSize = 0;
    Cpa32U targetCipherKeyLen = 0;

    if (CPA_CY_SYM_HASH_AES_XCBC == hashAlgorithm)
    {
        lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;

        LacSymQat_HashAlgLookupGet(hashAlgorithm, &pHashAlgInfo);
        stateSize = pHashAlgInfo->stateSize;

        memcpy(pAesQatData->data, pHashAlgInfo->initState, stateSize);
    }
    else if (CPA_CY_SYM_HASH_AES_GCM == hashAlgorithm)
    {
        stateSize = ICP_QAT_HW_GALOIS_H_SZ;
        LAC_OS_BZERO(pAesQatData->data, stateSize);
    }
    else
    {
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Initialise opsPending to the number of operations
     * needed to complete this precompute
     */
    ixOsalAtomicSet(1, &(pOpStatus->opsPending));
    pOpStatus->callbackFn = callbackFn;
    pOpStatus->pCallbackTag = pCallbackTag;

    pAesOpData->opType = LAC_SYM_HASH_PRECOMP_AES_ECB;
    pAesOpData->pOpStatus = pOpStatus;
    pAesOpData->stateSize = stateSize;
    pAesOpData->pState = pState;

    cipherSetupData.cipherAlgorithm = CPA_CY_SYM_CIPHER_AES_ECB;
    cipherSetupData.cipherKeyLenInBytes = authKeyLenInBytes;
    cipherSetupData.pCipherKey = pAuthKey;
    cipherSetupData.cipherDirection = CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT;

    /* Key length must be rounded up to nearest supported AES key length */
    if (authKeyLenInBytes <= ICP_QAT_HW_AES_128_KEY_SZ)
    {
        targetCipherKeyLen = ICP_QAT_HW_AES_128_KEY_SZ;
    }
    else if (authKeyLenInBytes <= ICP_QAT_HW_AES_192_KEY_SZ)
    {
        targetCipherKeyLen = ICP_QAT_HW_AES_192_KEY_SZ;
    }
    else if (authKeyLenInBytes <= ICP_QAT_HW_AES_256_KEY_SZ)
    {
        targetCipherKeyLen = ICP_QAT_HW_AES_256_KEY_SZ;
    }
    else
    {
        LAC_INVALID_PARAM_LOG("authKeyLenInBytes exceeds max supported size");
        return CPA_STATUS_INVALID_PARAM;
    }        

    LacSymQat_CipherContentDescPopulate(
        &cipherSetupData,
        targetCipherKeyLen,
        (icp_qat_fw_cipher_hdr_t *)pAesQatData->contentDesc,
        pAesQatData->contentDesc + sizeof(icp_qat_fw_cipher_hdr_t),
        0,
        ICP_QAT_FW_SLICE_MEM_OUT, 
        &cipherHwBlockSizeBytes);

    contentDescInfo.pData = pAesQatData->contentDesc;
    contentDescInfo.pDataPhys = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(contentDescInfo.pData));
    contentDescInfo.hdrSzQuadWords = LAC_BYTES_TO_QUADWORDS(
        sizeof(icp_qat_fw_cipher_hdr_t));
    contentDescInfo.hwBlkSzQuadWords = LAC_BYTES_TO_QUADWORDS(
        cipherHwBlockSizeBytes);

    LacSymQat_CipherRequestParamsPopulate(
        &pAesQatData->cipherReqParams,
        ICP_QAT_FW_SLICE_MEM_OUT,
        CPA_CY_SYM_CIPHER_AES_ECB,
        0,
        stateSize,
        NULL);  /* IV of 0's */

    pBufferListDesc = (icp_buffer_list_desc_t *)pAesQatData->bufferDesc;
    pBufferListDesc->numBuffers = 1;
    pCurrFlatBufDesc = (icp_flat_buffer_desc_t *)(pBufferListDesc->phyBuffers);

    pCurrFlatBufDesc->dataLenInBytes = stateSize;
    pCurrFlatBufDesc->phyBuffer = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(pAesQatData->data));

    srcAddrPhys = LAC_MEM_CAST_PTR_TO_UINT64(
        LAC_OS_VIRT_TO_PHYS(pBufferListDesc));

    /* proto, cmp_auth, ret_auth, apd_auth, wr_state, partial */
    laCmdFlags = ICP_QAT_FW_LA_FLAGS_BUILD(
                    ICP_QAT_FW_LA_NO_PROTO,
                    ICP_QAT_FW_LA_NO_CMP_AUTH_RES,
                    ICP_QAT_FW_LA_NO_RET_AUTH_RES,
                    ICP_QAT_FW_LA_NO_APD_AUTH_RES,
                    ICP_QAT_FW_LA_NO_UPDATE_STATE,
                    ICP_QAT_FW_LA_PARTIAL_NONE);

    /* populate the bulk request */
    LacSymQat_BulkReqPopulate(
        &bulkMsg,
        &contentDescInfo,
        pAesOpData,
        srcAddrPhys,
        0,
        ICP_QAT_FW_LA_CMD_AUTH_PRE_COMP,
        &pAesQatData->cipherReqParams,
        sizeof(icp_qat_fw_la_cipher_req_params_t),
        laCmdFlags);

    /* create the request header */
    status = QatComms_ReqHdrCreate(&bulkMsg, ICP_ARCH_IF_REQ_QAT_FW_LA);
    
    if (CPA_STATUS_SUCCESS == status)
    {
        /* send the message to the QAT */
        status = QatComms_MsgSend(
                    &bulkMsg,
                    ICP_ARCH_IF_REQ_QAT_FW_LA,
                    QAT_COMMS_PRIORITY_HIGH,
                    instanceHandle);
    }

    return status;
}

/**
 *****************************************************************************
 * @ingroup LacHash
 *      Definition of the callback function for processing responses for 
 *      precompute operations
 * 
 * @description
 *      This callback, which is registered with the common symmetric response
 *      message handler, It is invoked to process precompute response messages 
 *      from the QAT.  
 *
 * @param[in] lacCmdId          ID of the symmetric QAT command of the request
 *                              message
 * @param[in] pOpaqueData       pointer to opaque data in the request message
 * @param[in] cmnRespFlags      Flags set by QAT to indicate response status
 * 
 * @return  None
 *****************************************************************************/
STATIC void 
LacSymHash_PrecompCbFunc(icp_qat_fw_la_cmd_id_t lacCmdId,
                         void *pOpaqueData,
                         icp_qat_fw_comn_flags cmnRespFlags)
{
    lac_sym_hash_precomp_op_data_t *pPrecompOpData = 
        (lac_sym_hash_precomp_op_data_t *)pOpaqueData;
    lac_sym_hash_precomp_op_t *pOpStatus = NULL;
    
    if (NULL == pPrecompOpData)
    {
        LAC_LOG_ERROR("Opaque data for precompute is NULL");
        return;
    }

    pOpStatus = pPrecompOpData->pOpStatus;

    if (LAC_SYM_HASH_PRECOMP_HMAC == pPrecompOpData->opType)
    {
        lac_sym_hash_hmac_precomp_qat_t *pHmacQatData = 
            &pPrecompOpData->u.hmacQatData;

        /* Copy the hash state */
        memcpy(pPrecompOpData->pState,
                (Cpa8U *)pHmacQatData->hashStateStorage + 
                    sizeof(icp_qat_hw_auth_counter_t), 
                pPrecompOpData->stateSize);
    }
    else
    {
        lac_sym_hash_aes_precomp_qat_t *pAesQatData = 
            &pPrecompOpData->u.aesQatData;

        memcpy(pPrecompOpData->pState,
               pAesQatData->data,
               pPrecompOpData->stateSize); 
    }

    /* Check if there are any more pending requests by testing for opsPending
     * for 0. If there arent then we can signal to the user that we're done
     */
    if (CPA_FALSE != ixOsalAtomicDecAndTest(&(pOpStatus->opsPending)))
    {
        pOpStatus->callbackFn(pOpStatus->pCallbackTag);
    }
}


CpaStatus
LacSymHash_HmacPrecompInit(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaCySymHashSetupData hashSetupData = {0};
    Cpa32U hashBlkSizeInBytes = 0; 
    Cpa32U i = 0;

    LacSymQat_RespHandlerRegister(ICP_QAT_FW_LA_CMD_AUTH_PRE_COMP,
                                  LacSymHash_PrecompCbFunc);

    for (i = CPA_CY_SYM_HASH_MD5; i <= CPA_CY_SYM_HASH_SHA512; i++) 
    {
        Cpa8U *pContentDesc = NULL;
        lac_sym_qat_hash_alg_info_t *pHashAlgInfo = NULL;

        LacSymQat_HashAlgLookupGet(i, &pHashAlgInfo);

        status = LAC_OS_CAMALLOC(&pContentDesc, 
                    LAC_SYM_QAT_HASH_CONTENT_DESC_SIZE, 
                    LAC_64BYTE_ALIGNMENT);
        
        if (CPA_STATUS_SUCCESS == status)
        {
            hashSetupData.hashAlgorithm = i;
            hashSetupData.hashMode = CPA_CY_SYM_HASH_MODE_PLAIN;
            hashSetupData.digestResultLenInBytes = pHashAlgInfo->digestLength;

            /* Populate the Control block of the content descriptor */
            LacSymQat_HashContentDescInit(
                &hashSetupData,
                /* control block first in content descriptor */
                (icp_qat_fw_auth_hdr_t *)pContentDesc,
                /* point to base of hw setup block */
                (Cpa8U *)pContentDesc + sizeof(icp_qat_fw_auth_hdr_t),
                0,  /* offset of hash setup block */
                ICP_QAT_FW_SLICE_MEM_OUT,
                ICP_QAT_HW_AUTH_MODE0,  /* just a plain hash */
                NULL,
                &hashBlkSizeInBytes);
        
            pHmacContentDesc[i] = pContentDesc;         
        }
        else 
        {
            break;
        }
    }

    if (CPA_STATUS_SUCCESS != status)
    {
        for (i = CPA_CY_SYM_HASH_MD5; i <= CPA_CY_SYM_HASH_SHA512; i++)
        {
            LAC_OS_CAFREE(pHmacContentDesc[i],
                    LAC_SYM_QAT_HASH_CONTENT_DESC_SIZE);
        } 
    }

    return status;
}

void
LacSymHash_HmacPrecompShutdown(void)
{
    Cpa32U i = 0;

    for (i = CPA_CY_SYM_HASH_MD5; i <= CPA_CY_SYM_HASH_SHA512; i++)
    {
        LAC_OS_CAFREE(pHmacContentDesc[i], LAC_SYM_QAT_HASH_CONTENT_DESC_SIZE);
    }
}
