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
 * @file lac_sym_qat_cipher.c      QAT-related support functions for Cipher
 *
 * @ingroup LacSymQat_Cipher
 *
 * @description Functions to support the QAT related operations for Cipher
 ***************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "lac_sym_qat_cipher.h"
#include "lac_mem.h"
#include "lac_common.h"
#include "cpa_cy_sym.h"
#include "lac_sym_qat.h"
#include "lac_sym_cipher_defs.h"
#include "icp_qat_hw.h"
#include "icp_qat_fw_la.h"
#include "lac_session.h"


/*****************************************************************************
 *  Internal functions
 *****************************************************************************/

STATIC void
LacSymQat_CipherHwBlockPopulate(
    const CpaCySymCipherSetupData * pCipherSetupData,
    Cpa32U targetKeyLenInBytes,
    const void * pCipherHwBlock,
    Cpa32U * pCipherHwBlockSizeBytes)
{
    icp_qat_hw_cipher_algo_t algorithm = 0;
    icp_qat_hw_cipher_mode_t mode = 0;
    icp_qat_hw_cipher_dir_t dir = 0;
    icp_qat_hw_cipher_convert_t key_convert =
        ICP_QAT_HW_CIPHER_NO_CONVERT;
    icp_qat_hw_cipher_config_t *pCipherConfig =
        (icp_qat_hw_cipher_config_t *) LAC_CONST_PTR_CAST(pCipherHwBlock);
    Cpa8U *pCipherKey = (Cpa8U *) (Cpa32U) ((const Cpa8U *) pCipherHwBlock +
                           sizeof(icp_qat_hw_cipher_config_t));
    Cpa32U actualKeyLenInBytes = pCipherSetupData->cipherKeyLenInBytes;

    /* Set the direction (encrypt/decrypt) */
    if (CPA_CY_SYM_CIPHER_DIRECTION_ENCRYPT ==
        pCipherSetupData->cipherDirection)
    {
        dir = ICP_QAT_HW_CIPHER_ENCRYPT;
    }
    else
    {
        dir = ICP_QAT_HW_CIPHER_DECRYPT;
    }

    /* Set the algorithm */
    if (LAC_CIPHER_IS_ARC4(pCipherSetupData->cipherAlgorithm))
    {
        algorithm = ICP_QAT_HW_CIPHER_ALGO_RC4;
        /* Streaming ciphers are a special case. Decrypt = encrypt */
        dir = ICP_QAT_HW_CIPHER_ENCRYPT;
    }
    else if (LAC_CIPHER_IS_DES(pCipherSetupData->cipherAlgorithm))
    {
        algorithm = ICP_QAT_HW_CIPHER_ALGO_DES;
    }
    else if (LAC_CIPHER_IS_TRIPLE_DES(pCipherSetupData->cipherAlgorithm))
    {
        algorithm = ICP_QAT_HW_CIPHER_ALGO_3DES;
    }
    else if (LAC_CIPHER_IS_AES(pCipherSetupData->cipherAlgorithm))
    {
        switch(targetKeyLenInBytes)
        {
            case ICP_QAT_HW_AES_128_KEY_SZ:
                algorithm = ICP_QAT_HW_CIPHER_ALGO_AES128;
                break;
            case ICP_QAT_HW_AES_192_KEY_SZ:
                algorithm = ICP_QAT_HW_CIPHER_ALGO_AES192;
                break;
            case ICP_QAT_HW_AES_256_KEY_SZ:
                algorithm = ICP_QAT_HW_CIPHER_ALGO_AES256;
                break;
            default:
                LAC_ENSURE(CPA_FALSE, "Invalid AES key size\n");
                break;
        }

        /* AES decrypt key needs to be reversed.  Instead of reversing the key
         * at session registration, it is instead reversed on-the-fly by
         * setting the KEY_CONVERT bit here
         */
        if (CPA_CY_SYM_CIPHER_DIRECTION_DECRYPT ==
            pCipherSetupData->cipherDirection)
        {
            key_convert = ICP_QAT_HW_CIPHER_KEY_CONVERT;
        }
    }
    else if (LAC_CIPHER_IS_NULL(pCipherSetupData->cipherAlgorithm))
    {
        algorithm = ICP_QAT_HW_CIPHER_ALGO_NULL;
    }
    else
    {
        LAC_ENSURE(CPA_FALSE, "Algorithm not supported in Cipher\n");
    }

    /* Set the mode */
    if (LAC_CIPHER_IS_CTR_MODE(pCipherSetupData->cipherAlgorithm))
    {
        mode = ICP_QAT_HW_CIPHER_CTR_MODE;
        /* Streaming ciphers are a special case. Decrypt = encrypt
         * Overriding default values previously set for AES
         */
        dir = ICP_QAT_HW_CIPHER_ENCRYPT;
        key_convert = ICP_QAT_HW_CIPHER_NO_CONVERT;
    }
    else if (LAC_CIPHER_IS_ECB_MODE(pCipherSetupData->cipherAlgorithm))
    {
        mode = ICP_QAT_HW_CIPHER_ECB_MODE;
    }
    else if (LAC_CIPHER_IS_CBC_MODE(pCipherSetupData->cipherAlgorithm))
    {
        mode = ICP_QAT_HW_CIPHER_CBC_MODE;
    }
    else if (LAC_CIPHER_IS_ARC4(pCipherSetupData->cipherAlgorithm))
    {
        mode = mode;  /* No mode for ARC4 - leave as default */
    }
    else
    {
        LAC_ENSURE(CPA_FALSE, "Mode not supported in Cipher\n");
    }

    LAC_MEM_SHARED_WRITE_32BIT(pCipherConfig->val,
                         ICP_QAT_HW_CIPHER_CONFIG_BUILD(
                             mode, algorithm, key_convert, dir));

    pCipherConfig->reserved = 0;

    *pCipherHwBlockSizeBytes = sizeof(icp_qat_hw_cipher_config_t);

    /* Key is copied into content descriptor for all cases except for
     * Arc4 and Null cipher */
    if (!(LAC_CIPHER_IS_ARC4(pCipherSetupData->cipherAlgorithm) || 
          LAC_CIPHER_IS_NULL(pCipherSetupData->cipherAlgorithm)))
    {
        /* Set the Cipher key field in the cipher block */
        memcpy (pCipherKey, pCipherSetupData->pCipherKey, actualKeyLenInBytes);
        /* Pad the key with 0's if required */
        if (0 < (targetKeyLenInBytes - actualKeyLenInBytes))
        {
            LAC_OS_BZERO (pCipherKey + actualKeyLenInBytes,
                    targetKeyLenInBytes - actualKeyLenInBytes);
        }
        *pCipherHwBlockSizeBytes += targetKeyLenInBytes;
    }
}


/*****************************************************************************
 *  External functions
 *****************************************************************************/

Cpa8U
LacSymQat_CipherBlockSizeBytesGet(
    CpaCySymCipherAlgorithm cipherAlgorithm)
{
    if (LAC_CIPHER_IS_ARC4(cipherAlgorithm))
    {
        return LAC_CIPHER_ARC4_BLOCK_LEN_BYTES;
    }
    else if (LAC_CIPHER_IS_AES(cipherAlgorithm))
    {
        return ICP_QAT_HW_AES_128_BLK_SZ;
    }
    else if (LAC_CIPHER_IS_DES(cipherAlgorithm))
    {
        return ICP_QAT_HW_DES_BLK_SZ;
    }
    else if (LAC_CIPHER_IS_TRIPLE_DES(cipherAlgorithm))
    {
        return ICP_QAT_HW_3DES_BLK_SZ;
    }
    else if (LAC_CIPHER_IS_NULL(cipherAlgorithm))
    {
        return LAC_CIPHER_NULL_BLOCK_LEN_BYTES;
    }
    else
    {
        LAC_ENSURE(CPA_FALSE, "Algorithm not supported in Cipher");
        return 0;
    }
}


Cpa32U
LacSymQat_CipherIvSizeBytesGet(
    CpaCySymCipherAlgorithm cipherAlgorithm)
{
    if (CPA_CY_SYM_CIPHER_ARC4 == cipherAlgorithm)
    {
        return LAC_CIPHER_ARC4_STATE_LEN_BYTES;
    }
    else if (LAC_CIPHER_IS_ECB_MODE(cipherAlgorithm))
    {
        return 0;
    }
    else
    {
        return (Cpa32U)LacSymQat_CipherBlockSizeBytesGet(cipherAlgorithm);
    }
}


void
LacSymQat_CipherContentDescPopulate(
    const CpaCySymCipherSetupData * pCipherSetupData,
    Cpa32U targetKeyLenInBytes,
    icp_qat_fw_cipher_hdr_t * pCipherControlBlock,
    void * pHwBlockBase,
    Cpa32U cipherBlkOffsetInHwBlock,
    icp_qat_fw_slice_t nextSlice,
    Cpa32U * pCipherHwBlockSizeBytes)
{
    CpaCySymCipherAlgorithm algorithm;

    LAC_ENSURE(NULL != pCipherSetupData,
               "LacSymQat_CipherContentDescPopulate - "
               "pCipherSetupData is NULL\n");
    LAC_ENSURE(NULL != pCipherControlBlock,
               "LacSymQat_CipherContentDescPopulate - "
               "pCipherControlBlock is NULL\n");
    LAC_ENSURE(NULL != pHwBlockBase,
               "LacSymQat_CipherContentDescPopulate - "
               "pHwBlockBase is NULL\n");
    LAC_ENSURE(NULL != pCipherHwBlockSizeBytes,
               "LacSymQat_CipherContentDescPopulate - "
               "pCipherHwBlockSizeBytes is NULL\n");

    algorithm = pCipherSetupData->cipherAlgorithm;

/********************************************
 *  Populate Cipher Content Descriptor header
 */
    /* Base Key is not passed down to QAT in the case of ARC4 or NULL */
    if (LAC_CIPHER_IS_ARC4(pCipherSetupData->cipherAlgorithm) ||
        LAC_CIPHER_IS_NULL(pCipherSetupData->cipherAlgorithm))
    {
        /* No need to endian-swap 0, hence no need for LAC_MEM_SHARED_WRITE */
        pCipherControlBlock->key_sz = 0;
    }
    else
    {
        LAC_MEM_SHARED_WRITE_8BIT(pCipherControlBlock->key_sz,
                             LAC_BYTES_TO_QUADWORDS(targetKeyLenInBytes));
    }

    LAC_MEM_SHARED_WRITE_8BIT(pCipherControlBlock->block_sz,
        LacSymQat_CipherBlockSizeBytesGet(algorithm));
    LAC_MEM_SHARED_WRITE_8BIT(pCipherControlBlock->state_sz,
        LAC_BYTES_TO_QUADWORDS(LacSymQat_CipherIvSizeBytesGet(algorithm)));

    LAC_MEM_SHARED_WRITE_8BIT(pCipherControlBlock->next_id, nextSlice);
    LAC_MEM_SHARED_WRITE_8BIT(pCipherControlBlock->curr_id,
        ICP_QAT_FW_SLICE_CIPHER);
    LAC_MEM_SHARED_WRITE_8BIT(pCipherControlBlock->offset,
        LAC_BYTES_TO_QUADWORDS(cipherBlkOffsetInHwBlock));

    pCipherControlBlock->reserved1 = 0;
    pCipherControlBlock->reserved2 = 0;

/***************************************
 *  Populate Cipher Hardware Setup Block
 */
    LacSymQat_CipherHwBlockPopulate(pCipherSetupData,
                                    targetKeyLenInBytes,
                                    ((Cpa8U *)pHwBlockBase +
                                     cipherBlkOffsetInHwBlock),
                                    pCipherHwBlockSizeBytes);
}



void
LacSymQat_CipherRequestParamsPopulate(
    icp_qat_fw_la_cipher_req_params_t * pCipherReqParams,
    icp_qat_fw_slice_t nextSlice,
    CpaCySymCipherAlgorithm cipherAlgorithm,
    Cpa32U cipherOffsetInBytes,
    Cpa32U cipherLenInBytes,
    const Cpa8U * pAlignedIvBuffer)
{
    LAC_ENSURE(NULL != pCipherReqParams,
               "LacSymQat_CipherRequestParamsPopulate - "
               "pCipherReqParams is NULL\n");

    LAC_MEM_SHARED_WRITE_8BIT(pCipherReqParams->next_id, nextSlice);
    LAC_MEM_SHARED_WRITE_8BIT(pCipherReqParams->curr_id, 
                              ICP_QAT_FW_SLICE_CIPHER);

    LAC_MEM_SHARED_WRITE_8BIT(pCipherReqParams->cipher_state_sz,
        LAC_BYTES_TO_QUADWORDS(
            LacSymQat_CipherIvSizeBytesGet(cipherAlgorithm)));

    LAC_MEM_SHARED_WRITE_16BIT(pCipherReqParams->cipher_len, 
                               cipherLenInBytes);
    LAC_MEM_SHARED_WRITE_16BIT(pCipherReqParams->cipher_off, 
                               cipherOffsetInBytes);

    if (pAlignedIvBuffer != NULL)
    {
        unsigned long physAddr = 0;
        physAddr = LAC_OS_VIRT_TO_PHYS((unsigned long)pAlignedIvBuffer);
        LAC_MEM_SHARED_WRITE_64BIT(pCipherReqParams->state_address,
                             (Cpa64U)physAddr);
    }
    else
    {
        pCipherReqParams->state_address = 0;
    }
    pCipherReqParams->resrvd = 0;
}


void
LacSymQat_CipherArc4StateInit (
    const Cpa8U *pKey,
    Cpa32U keyLenInBytes,
    Cpa8U *pArc4CipherState)
{
    Cpa32U i = 0;
    Cpa32U j = 0;

    for (i = 0; i < LAC_CIPHER_ARC4_KEY_MATRIX_LEN_BYTES; i ++)
    {
        pArc4CipherState[i] = i;
    }

    for (i = 0; i < LAC_CIPHER_ARC4_KEY_MATRIX_LEN_BYTES; i ++)
    {
        Cpa8U swap;
    
        j = (j + pArc4CipherState[i] + pKey[i % keyLenInBytes]) %
            LAC_CIPHER_ARC4_KEY_MATRIX_LEN_BYTES;

        /* Swap state[i] & state[j] */
        swap = pArc4CipherState[i];
        pArc4CipherState[i] = pArc4CipherState[j];
        pArc4CipherState[j] = swap;
    }

    /* Initialise i & j values for QAT */
    pArc4CipherState[LAC_CIPHER_ARC4_KEY_MATRIX_LEN_BYTES] = 0;
    pArc4CipherState[LAC_CIPHER_ARC4_KEY_MATRIX_LEN_BYTES + 1] = 0;
}
