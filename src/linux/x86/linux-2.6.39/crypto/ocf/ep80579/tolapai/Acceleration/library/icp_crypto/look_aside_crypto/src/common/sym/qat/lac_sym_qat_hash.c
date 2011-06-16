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
 * @file lac_sym_qat_hash.c      
 *
 * @ingroup LacSymQatHash
 * 
 * Implementation for populating QAT data structures for hash operation
 ***************************************************************************/


/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "lac_mem.h"
#include "lac_common.h"
#include "lac_sym_qat.h"
#include "lac_sym_qat_hash.h"
#include "lac_sym_qat_hash_defs_lookup.h"

/**
 * This structure contains pointers into the hash setup block of the
 * security descriptor. As the hash setup block contains fields that
 * are of variable length, pointers must be calculated to these fields
 * and the hash setup block is populated using these pointers. */
typedef struct lac_hash_blk_ptrs_s
{
    icp_qat_hw_auth_setup_t* pInHashSetup;
    /**< inner hash setup */
    Cpa8U *pInHashInitState1;
    /**< inner initial state 1 */
    Cpa8U *pInHashInitState2;
    /**< inner initial state 2 */
    icp_qat_hw_auth_setup_t* pOutHashSetup;
    /**< outer hash setup */
    Cpa8U *pOutHashInitState1;
    /**< outer hash initial state */
} lac_hash_blk_ptrs_t;


/**
 * This function calculates the pointers into the hash setup block
 * based on the control block 
 * 
 * @param[in]  pHashControlBlock    Pointer to hash control block
 * @param[in]  pHwBlockBase         pointer to base of hardware block
 * @param[out] pHashBlkPtrs         structure containing pointers to 
 *                                  various fields in the hash setup block
 *
 * @return void
 */
STATIC void
LacSymQat_HashHwBlockPtrsInit(
        icp_qat_fw_auth_hdr_t *pHashControlBlock,
        void *pHwBlockBase,
        lac_hash_blk_ptrs_t *pHashBlkPtrs);


/**
 * This function populates the hash setup block
 * 
 * @param[in]  pHashSetupData             Pointer to the hash context
 * @param[in]  pHashControlBlock    Pointer to hash control block
 * @param[in]  pHwBlockBase         pointer to base of hardware block
 * @param[in]  qatHashMode          QAT hash mode
 * @param[in]  pPrecompute          For auth mode, this is the pointer 
 *                                  to the precompute data. Otherwise this
 *                                  should be set to NULL
 * @param[in]  pHashDefs            Pointer to Hash definitions
 * @param[in]  pOuterHashDefs       Pointer to Outer Hash definitions.
 *                                  Required for nested hash mode only
 *
 * @return void
 */
STATIC void
LacSymQat_HashSetupBlockInit(
        const CpaCySymHashSetupData *pHashSetupData,
        icp_qat_fw_auth_hdr_t *pHashControlBlock,
        void *pHwBlockBase,
        icp_qat_hw_auth_mode_t qatHashMode,
        lac_sym_qat_hash_precompute_info_t *pPrecompute,
        lac_sym_qat_hash_defs_t *pHashDefs,
        lac_sym_qat_hash_defs_t *pOuterHashDefs);


/** @ingroup LacSymQatHash */
void
LacSymQat_HashContentDescInit(
        const CpaCySymHashSetupData *pHashSetupData,
        icp_qat_fw_auth_hdr_t *pHashControlBlock,
        void *pHwBlockBase,
        Cpa32U hashBlkOffsetInHwBlock,
        icp_qat_fw_slice_t nextSlice,
        icp_qat_hw_auth_mode_t qatHashMode,
        lac_sym_qat_hash_precompute_info_t *pPrecompute,
        Cpa32U *pHashBlkSizeInBytes)
{
    lac_sym_qat_hash_defs_t *pHashDefs = NULL;
    lac_sym_qat_hash_defs_t *pOuterHashDefs = NULL;
    Cpa32U hashSetupBlkSize = 0;

    LAC_ENSURE_NOT_NULL(pHashSetupData);
    LAC_ENSURE_NOT_NULL(pHashControlBlock);
    LAC_ENSURE_NOT_NULL(pHwBlockBase);
    LAC_ENSURE_NOT_NULL(pHashBlkSizeInBytes);

    LacSymQat_HashDefsLookupGet(pHashSetupData->hashAlgorithm, &pHashDefs);

    pHashControlBlock->next_id = nextSlice;
    pHashControlBlock->curr_id = ICP_QAT_FW_SLICE_AUTH;
    pHashControlBlock->offset = LAC_BYTES_TO_QUADWORDS(hashBlkOffsetInHwBlock);
    pHashControlBlock->reserved = 0;

    /* Hmac in mode 2 TLS */
    if (IS_HASH_MODE_2(qatHashMode))
    {
        /* set bit for nested hashing */
        pHashControlBlock->hash_flags = ICP_QAT_FW_AUTH_HDR_FLAG_DO_NESTED;

        /* Inner and outer prefixes are the block length */
        pHashControlBlock->u.inner_prefix_sz = pHashDefs->algInfo->blockLength;
        pHashControlBlock->outer_prefix_sz = pHashDefs->algInfo->blockLength;
    }
    /* Nested hash in mode 0 */
    else if (CPA_CY_SYM_HASH_MODE_NESTED == pHashSetupData->hashMode)
    {
        /* set bit for nested hashing */
        pHashControlBlock->hash_flags = ICP_QAT_FW_AUTH_HDR_FLAG_DO_NESTED;

        /* set inner and outer prefixes */
        pHashControlBlock->u.inner_prefix_sz = 
            pHashSetupData->nestedModeSetupData.innerPrefixLenInBytes;
        pHashControlBlock->outer_prefix_sz = 
            pHashSetupData->nestedModeSetupData.outerPrefixLenInBytes;
    }
    /* mode0 - plain or mode1 - auth */
    else
    {
        pHashControlBlock->hash_flags = ICP_QAT_FW_AUTH_HDR_FLAG_NO_NESTED;

        /* For Auth Encrypt set the aad size */
        if ((CPA_CY_SYM_HASH_AES_CCM == pHashSetupData->hashAlgorithm) ||
            (CPA_CY_SYM_HASH_AES_GCM == pHashSetupData->hashAlgorithm)) 
        {
            /* round the aad size to the multiple of CCM/GCM common 
            *  hash block size. Block size for CCM & GCM is the same */
            pHashControlBlock->u.aad_sz = LAC_ALIGN_POW2_ROUNDUP(
                pHashSetupData->authModeSetupData.aadLenInBytes,
                LAC_HASH_AES_CCM_BLOCK_SIZE);
        }
        else
        {
            pHashControlBlock->u.inner_prefix_sz = 0;
        }

        pHashControlBlock->outer_prefix_sz = 0;
    }

    /* set the final digest size */
    pHashControlBlock->final_sz = pHashSetupData->digestResultLenInBytes;

    /* set the state1 size */
    pHashControlBlock->inner_state1_sz = LAC_ALIGN_POW2_ROUNDUP(
        pHashDefs->qatInfo->state1Length, LAC_QUAD_WORD_IN_BYTES);

    /* set the inner result size to the digest length */
    pHashControlBlock->inner_res_sz = pHashDefs->algInfo->digestLength;

    /* set the state2 size - only for mode 1 Auth algos */
    if (IS_HASH_MODE_1(qatHashMode))
    {
        pHashControlBlock->inner_state2_sz = LAC_ALIGN_POW2_ROUNDUP(
            pHashDefs->qatInfo->state2Length, LAC_QUAD_WORD_IN_BYTES);
    }
    else
    {
        pHashControlBlock->inner_state2_sz = 0;
    }

    pHashControlBlock->inner_state2_off = pHashControlBlock->offset + 
        LAC_BYTES_TO_QUADWORDS(sizeof(icp_qat_hw_auth_setup_t) + 
                               pHashControlBlock->inner_state1_sz);

    /* size of inner part of hash setup block */
    hashSetupBlkSize = sizeof(icp_qat_hw_auth_setup_t) +
            pHashControlBlock->inner_state1_sz +
            pHashControlBlock->inner_state2_sz;


    /* For nested hashing - Fill in the outer fields */
    if (CPA_CY_SYM_HASH_MODE_NESTED == pHashSetupData->hashMode || 
        IS_HASH_MODE_2(qatHashMode) )
    {
        /* For nested - use the outer algorithm. This covers TLS and 
         * nested hash. For HMAC mode2 use inner algorithm again */
        CpaCySymHashAlgorithm outerAlg = 
            (CPA_CY_SYM_HASH_MODE_NESTED == pHashSetupData->hashMode) ?
            pHashSetupData->nestedModeSetupData.outerHashAlgorithm : 
            pHashSetupData->hashAlgorithm; 

        LacSymQat_HashDefsLookupGet(outerAlg, &pOuterHashDefs);

        /* outer config offset */
        pHashControlBlock->outer_config_off = 
            pHashControlBlock->inner_state2_off + 
            LAC_BYTES_TO_QUADWORDS(pHashControlBlock->inner_state2_sz); 

        /* state size of outer hash algorithm */
        pHashControlBlock->outer_state1_sz = LAC_ALIGN_POW2_ROUNDUP(
            pOuterHashDefs->algInfo->stateSize, LAC_QUAD_WORD_IN_BYTES);

        /* outer result size */
        pHashControlBlock->outer_res_sz = pOuterHashDefs->algInfo->digestLength;

        /* outer prefix offset will be the size of the inner prefix data 
         * plus the hash state storage size. */
        pHashControlBlock->outer_prefix_off = LAC_BYTES_TO_QUADWORDS(
            LAC_ALIGN_POW2_ROUNDUP(
                pHashControlBlock->u.inner_prefix_sz,
                LAC_QUAD_WORD_IN_BYTES));

        /* size of outer part of hash block */
        hashSetupBlkSize += sizeof(icp_qat_hw_auth_setup_t) +
                pHashControlBlock->outer_state1_sz;
    }
    else
    {
        pHashControlBlock->outer_config_off = 0;
        pHashControlBlock->outer_state1_sz = 0;
        pHashControlBlock->outer_res_sz = 0;
        pHashControlBlock->outer_prefix_off = 0;
    }

    *pHashBlkSizeInBytes = hashSetupBlkSize;

/*****************************************************************************
 *                                                 Populate Hash Setup block *
 *****************************************************************************/

    LacSymQat_HashSetupBlockInit(pHashSetupData,
                                 pHashControlBlock,
                                 pHwBlockBase,
                                 qatHashMode,
                                 pPrecompute,
                                 pHashDefs,
                                 pOuterHashDefs);
}


void
LacSymQat_HashHwBlockPtrsInit(
        icp_qat_fw_auth_hdr_t *pHashControlBlock,
        void *pHwBlockBase,
        lac_hash_blk_ptrs_t *pHashBlkPtrs) 
{
    LAC_ENSURE_NOT_NULL(pHashControlBlock);
    LAC_ENSURE_NOT_NULL(pHwBlockBase);
    LAC_ENSURE_NOT_NULL(pHashBlkPtrs);

    /* encoded offset for inner config is converted to a byte offset. */
    pHashBlkPtrs->pInHashSetup = (icp_qat_hw_auth_setup_t *)( 
        (Cpa8U *)pHwBlockBase + 
        (pHashControlBlock->offset * LAC_QUAD_WORD_IN_BYTES) );

    pHashBlkPtrs->pInHashInitState1 = (Cpa8U *)pHashBlkPtrs->pInHashSetup 
        + sizeof(icp_qat_hw_auth_setup_t);

    pHashBlkPtrs->pInHashInitState2 = 
        (Cpa8U *)(pHashBlkPtrs->pInHashInitState1) +
        pHashControlBlock->inner_state1_sz;

    pHashBlkPtrs->pOutHashSetup = (icp_qat_hw_auth_setup_t *)(
        (Cpa8U *)pHashBlkPtrs->pInHashInitState2 +
        pHashControlBlock->inner_state2_sz);

    pHashBlkPtrs->pOutHashInitState1 = (Cpa8U *)pHashBlkPtrs->pOutHashSetup 
        + sizeof(icp_qat_hw_auth_setup_t);
}


STATIC void
LacSymQat_HashSetupBlockInit(
        const CpaCySymHashSetupData *pHashSetupData,
        icp_qat_fw_auth_hdr_t *pHashControlBlock,
        void *pHwBlockBase,
        icp_qat_hw_auth_mode_t qatHashMode,
        lac_sym_qat_hash_precompute_info_t *pPrecompute,
        lac_sym_qat_hash_defs_t *pHashDefs,
        lac_sym_qat_hash_defs_t *pOuterHashDefs)
{
    Cpa32U innerConfig = 0;
    lac_hash_blk_ptrs_t hashBlkPtrs = {0};

    LacSymQat_HashHwBlockPtrsInit(
        pHashControlBlock, pHwBlockBase, &hashBlkPtrs);

    innerConfig = ICP_QAT_HW_AUTH_CONFIG_BUILD(qatHashMode, 
                    pHashDefs->qatInfo->algoEnc, 
                    pHashSetupData->digestResultLenInBytes);

    /* Set the Inner hash configuration */
    LAC_MEM_SHARED_WRITE_32BIT(hashBlkPtrs.pInHashSetup->auth_config.config, 
                         innerConfig);
    hashBlkPtrs.pInHashSetup->auth_config.reserved = 0;

    /* For mode 1 pre-computes for auth algorithms */
    if (IS_HASH_MODE_1(qatHashMode))
    {
        LAC_ENSURE_NOT_NULL(pPrecompute);

        /* for HMAC in mode 1 counter the block size
         * for xcbc, ccm & gcm counter is 0  */
        LAC_MEM_SHARED_WRITE_32BIT(
                             hashBlkPtrs.pInHashSetup->auth_counter.counter,  
                             pHashDefs->qatInfo->authCounter);

        /* state 1 is set to 0 for the following algorithms */
        if ((CPA_CY_SYM_HASH_AES_XCBC == pHashSetupData->hashAlgorithm) ||
            (CPA_CY_SYM_HASH_AES_CCM == pHashSetupData->hashAlgorithm) ||
            (CPA_CY_SYM_HASH_AES_GCM == pHashSetupData->hashAlgorithm))
        {  
            LAC_OS_BZERO(hashBlkPtrs.pInHashInitState1,
                   pHashDefs->qatInfo->state1Length);
        }

        /* Pad remaining bytes of sha1 precomputes */
        if (CPA_CY_SYM_HASH_SHA1 == pHashSetupData->hashAlgorithm)
        {
            Cpa32U state1PadLen = pHashControlBlock->inner_state1_sz -
                                    pHashDefs->algInfo->stateSize;
            Cpa32U state2PadLen = pHashControlBlock->inner_state2_sz -
                                    pHashDefs->algInfo->stateSize;
            
            if (state1PadLen > 0)
            {
                LAC_OS_BZERO(hashBlkPtrs.pInHashInitState1 + 
                        pHashDefs->algInfo->stateSize, state1PadLen);
            }

            if (state2PadLen > 0)
            {
                LAC_OS_BZERO(hashBlkPtrs.pInHashInitState2 +
                        pHashDefs->algInfo->stateSize, state2PadLen);
            }
        }

        pPrecompute->state1Size = pHashDefs->qatInfo->state1Length;
        pPrecompute->state2Size = pHashDefs->qatInfo->state2Length;

        /* Set the destination for pre-compute state1 data to be written */
        pPrecompute->pState1 = hashBlkPtrs.pInHashInitState1;

        /* Set the destination for pre-compute state1 data to be written */
        pPrecompute->pState2 = hashBlkPtrs.pInHashInitState2;
    }
    /* For digest and nested digest */
    else
    {
        Cpa32S padLen = pHashControlBlock->inner_state1_sz -  
                            pHashDefs->algInfo->stateSize;


        /* counter set to 0 */
        hashBlkPtrs.pInHashSetup->auth_counter.counter = 0;

        /* set the inner hash state 1 */
        memcpy(hashBlkPtrs.pInHashInitState1,
               pHashDefs->algInfo->initState,
               pHashDefs->algInfo->stateSize );

        if (padLen > 0)
        {
            LAC_OS_BZERO(hashBlkPtrs.pInHashInitState1 + 
                    pHashDefs->algInfo->stateSize, padLen);
        }
    }

    hashBlkPtrs.pInHashSetup->auth_counter.reserved = 0;

    /* Fill in the outer part of the hash setup block */
    if (CPA_CY_SYM_HASH_MODE_NESTED == pHashSetupData->hashMode || 
        IS_HASH_MODE_2(qatHashMode))
    {
        Cpa32U outerConfig = ICP_QAT_HW_AUTH_CONFIG_BUILD(
            qatHashMode,
            pOuterHashDefs->qatInfo->algoEnc,
            pHashSetupData->digestResultLenInBytes);

        Cpa32U padLen = pHashControlBlock->outer_state1_sz - 
                            pOuterHashDefs->algInfo->stateSize; 

        /* populate the auth config */
        LAC_MEM_SHARED_WRITE_32BIT(
                             hashBlkPtrs.pOutHashSetup->auth_config.config, 
                             outerConfig);
        hashBlkPtrs.pOutHashSetup->auth_config.reserved = 0;

        /* outer Counter set to 0 */
        hashBlkPtrs.pOutHashSetup->auth_counter.counter = 0;
        hashBlkPtrs.pOutHashSetup->auth_counter.reserved = 0;

        /* set outer hash state 1 */
        memcpy(hashBlkPtrs.pOutHashInitState1,
               pOuterHashDefs->algInfo->initState,
               pOuterHashDefs->algInfo->stateSize);

        if (padLen > 0)
        {
            LAC_OS_BZERO(hashBlkPtrs.pOutHashInitState1 +
                    pOuterHashDefs->algInfo->stateSize, padLen);
        }
    }
}


void
LacSymQat_HashStatePrefixAadBufferSizeGet(
        const CpaCySymHashSetupData *pHashSetupData,        
        const icp_qat_fw_auth_hdr_t *pHashControlBlock,
        icp_qat_hw_auth_mode_t qatHashMode,
        lac_sym_qat_hash_state_buffer_info_t *pHashStateBuf)
{
    Cpa32U prefixAadSize = 0;

    LAC_ENSURE_NOT_NULL(pHashSetupData);
    LAC_ENSURE_NOT_NULL(pHashControlBlock);
    LAC_ENSURE_NOT_NULL(pHashStateBuf);

    /* Nested hashing and mode2 HMAC will use both prefix data */
    if ((ICP_QAT_FW_AUTH_HDR_FLAG_DO_NESTED == pHashControlBlock->hash_flags) 
            || IS_HASH_MODE_2(qatHashMode))
    {
        /* Round prefixes to multiples of quad words */
        prefixAadSize = 
            LAC_ALIGN_POW2_ROUNDUP(pHashControlBlock->u.inner_prefix_sz, 
                                   LAC_QUAD_WORD_IN_BYTES) + 
            LAC_ALIGN_POW2_ROUNDUP(pHashControlBlock->outer_prefix_sz, 
                                   LAC_QUAD_WORD_IN_BYTES);
    }

    /* hash state storage needed to support partial packets. Space reserved
     * for this in all cases */
    pHashStateBuf->stateStorageSzQuadWords = LAC_BYTES_TO_QUADWORDS( 
            sizeof(icp_qat_hw_auth_counter_t) + 
            pHashControlBlock->inner_state1_sz);

    pHashStateBuf->prefixAadSzQuadWords = 
        LAC_BYTES_TO_QUADWORDS(prefixAadSize);
}


void
LacSymQat_HashStatePrefixAadBufferPopulate(
    lac_sym_qat_hash_state_buffer_info_t *pHashStateBuf,
    const icp_qat_fw_auth_hdr_t *pHashControlBlock,    
    Cpa8U *pInnerPrefixAad,
    Cpa8U innerPrefixSize,
    Cpa8U *pOuterPrefix,
    Cpa8U outerPrefixSize)
{
    LAC_ENSURE_NOT_NULL(pHashStateBuf);
    LAC_ENSURE_NOT_NULL(pHashControlBlock);

    if (NULL != pInnerPrefixAad)
    {
        Cpa8U *pLocalInnerPrefix = (Cpa8U *)pHashStateBuf->pData +
            LAC_QUADWORDS_TO_BYTES(pHashStateBuf->stateStorageSzQuadWords);
        Cpa8U padding = pHashControlBlock->u.inner_prefix_sz - innerPrefixSize;

        /* copy the inner prefix or aad data */
        memcpy(pLocalInnerPrefix, pInnerPrefixAad, innerPrefixSize);
        
        /* Reset with zeroes any area reserved for padding in this block */
        if (0 < padding)
        {
            LAC_OS_BZERO(pLocalInnerPrefix + innerPrefixSize, padding);
        }
    }

    if (NULL != pOuterPrefix)
    {
        Cpa8U *pLocalOuterPrefix = (Cpa8U *)pHashStateBuf->pData + 
                LAC_QUADWORDS_TO_BYTES(pHashStateBuf->stateStorageSzQuadWords + 
                                        pHashControlBlock->outer_prefix_off);
        Cpa8U padding = pHashControlBlock->outer_prefix_sz - outerPrefixSize;

        /* copy the outer prefix */
        memcpy(pLocalOuterPrefix, pOuterPrefix, outerPrefixSize);
        
        /* Reset with zeroes any area reserved for padding in this block */
        if (0 < padding)
        {
            LAC_OS_BZERO(pLocalOuterPrefix + outerPrefixSize, padding);
        }
    }
}


void
LacSymQat_HashRequestParamsPopulate(
    icp_qat_fw_la_auth_req_params_t *pHashReqParams,
    icp_qat_fw_slice_t nextSlice,
    lac_sym_qat_hash_state_buffer_info_t *pHashStateBuf,
    Cpa32U packetType,
    Cpa32U hashResultSize,
    Cpa32U authOffsetInBytes,
    Cpa32U authLenInBytes,
    CpaBoolean digestVerify,
    Cpa8U *pAuthResult)
{
    Cpa32U authResSize = 0;
    Cpa64U authResultPhys = 0;

    LAC_ENSURE_NOT_NULL(pHashReqParams);

    LAC_MEM_SHARED_WRITE_8BIT(pHashReqParams->next_id, nextSlice);

    LAC_MEM_SHARED_WRITE_8BIT(pHashReqParams->curr_id, ICP_QAT_FW_SLICE_AUTH);

    /* For a Full packet or last partial need to set the digest result pointer
     * and the auth result field */
    if (NULL != pAuthResult)
    {
        authResultPhys = LAC_OS_VIRT_TO_PHYS((unsigned long)pAuthResult);

        /* Figure out how many bytes the QAT needs to read from the nearest
         * quad word aligned address, to read entire digest to be verified */
        if (CPA_TRUE == digestVerify)
        {
            authResSize = LAC_ALIGN_POW2_ROUNDUP( (authResultPhys & 0x7) +
                            hashResultSize, LAC_QUAD_WORD_IN_BYTES);

            /* auth result size in quad words to be read in for a verify
             *  operation */
            LAC_MEM_SHARED_WRITE_8BIT(pHashReqParams->auth_res_sz,
                LAC_BYTES_TO_QUADWORDS(authResSize));
        }
        else
        {
            pHashReqParams->auth_res_sz = 0;
        }

        /* auth result size in quad words to be read in for a verify operation*/
        LAC_MEM_SHARED_WRITE_8BIT(pHashReqParams->auth_res_sz,
            LAC_BYTES_TO_QUADWORDS(authResSize));

        LAC_MEM_SHARED_WRITE_64BIT(pHashReqParams->auth_res_address, 
                                   authResultPhys);
    }
    else
    {
        pHashReqParams->auth_res_sz = 0;
        pHashReqParams->auth_res_address = 0;
    }

    LAC_MEM_SHARED_WRITE_16BIT(pHashReqParams->auth_len, authLenInBytes);

    LAC_MEM_SHARED_WRITE_16BIT(pHashReqParams->auth_off, authOffsetInBytes);
    
    /* If there is a hash state prefix buffer */
    if (NULL != pHashStateBuf)
    {
        /* Only write the pointer to the buffer if the size is greater than 0 
         * this will be the case for plain and auth mode as due to the 
         * state storage required for partial packets and for nested mode (when
         * the prefix data is > 0) */
        if ( (pHashStateBuf->stateStorageSzQuadWords + 
              pHashStateBuf->prefixAadSzQuadWords) > 0)
        {
            /* For the first partial packet, the QAT expects the pointer to the 
             * inner prefix even if there is no memory allocated for this. The 
             * QAT will internally calculate where to write the state back. */
            if ((ICP_QAT_FW_LA_PARTIAL_START == packetType) ||
                (ICP_QAT_FW_LA_PARTIAL_NONE == packetType))
            {
                LAC_MEM_SHARED_WRITE_64BIT(pHashReqParams->u.prefix_addr,
                    (pHashStateBuf->pDataPhys) + LAC_QUADWORDS_TO_BYTES(
                    pHashStateBuf->stateStorageSzQuadWords)); 
            }
            else
            {
                LAC_MEM_SHARED_WRITE_64BIT(pHashReqParams->u.prefix_addr, 
                                     pHashStateBuf->pDataPhys);
            }
        }
        /* nested mode when the prefix data is 0 */
        else
        {
            pHashReqParams->u.prefix_addr = 0; 
        }

        /* For middle & last partial, state size is the hash state storage
         * if hash mode 2 this will include the prefix data */
        if ((ICP_QAT_FW_LA_PARTIAL_MID == packetType) ||
            (ICP_QAT_FW_LA_PARTIAL_END == packetType))
        {
            LAC_MEM_SHARED_WRITE_8BIT(pHashReqParams->hash_state_sz, 
                                 pHashStateBuf->stateStorageSzQuadWords + 
                                 pHashStateBuf->prefixAadSzQuadWords);
        }
        /* For full packets and first partials set the state size to that of
         * the prefix/aad. prefix includes both the inner and  outer prefix */
        else
        {
            LAC_MEM_SHARED_WRITE_8BIT(pHashReqParams->hash_state_sz, 
                                 pHashStateBuf->prefixAadSzQuadWords);
        }
    }
    else
    {
        pHashReqParams->u.prefix_addr = 0;
        pHashReqParams->hash_state_sz = 0;
    }
}


void
LacSymQat_HashLaCommandFlagsSet(
    Cpa32U qatPacketType,
    CpaBoolean hashVerify,
    Cpa16U *pLaCommandFlags)
{
    LAC_ENSURE_NOT_NULL(pLaCommandFlags);

    /* verify or return auth result only set for a full or final partial */
    if ((ICP_QAT_FW_LA_PARTIAL_NONE == qatPacketType) ||
        (ICP_QAT_FW_LA_PARTIAL_END == qatPacketType))
    {
        /* For a verify operation set the compare auth result command flag */
        if (CPA_TRUE == hashVerify)
        {
            ICP_QAT_FW_LA_CMP_AUTH_SET(
                *pLaCommandFlags, ICP_QAT_FW_LA_CMP_AUTH_RES);
            
            ICP_QAT_FW_LA_RET_AUTH_SET(
                *pLaCommandFlags, ICP_QAT_FW_LA_NO_RET_AUTH_RES);
        } 
        /* else return the auth result */
        else
        {
            ICP_QAT_FW_LA_RET_AUTH_SET(
                *pLaCommandFlags, ICP_QAT_FW_LA_RET_AUTH_RES);
            
            ICP_QAT_FW_LA_CMP_AUTH_SET(
                *pLaCommandFlags, ICP_QAT_FW_LA_NO_CMP_AUTH_RES);
        }
    }
    /* For partial - verify and return auth result are disabled */
    else
    {
        ICP_QAT_FW_LA_RET_AUTH_SET(
                *pLaCommandFlags, ICP_QAT_FW_LA_NO_RET_AUTH_RES);

        ICP_QAT_FW_LA_CMP_AUTH_SET(
                *pLaCommandFlags, ICP_QAT_FW_LA_NO_CMP_AUTH_RES);
    }
}
