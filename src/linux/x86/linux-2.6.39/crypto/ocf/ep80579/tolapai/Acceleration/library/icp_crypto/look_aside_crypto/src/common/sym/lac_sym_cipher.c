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
 * @file lac_sym_cipher.c   Cipher 
 *
 * @ingroup LacCipher
 *
 * @description Functions specific to cipher
 ***************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/
#include "cpa.h"
#include "cpa_cy_sym.h"
#include "icp_qat_fw_la.h"
#include "icp_qat_hw.h"

#include "lac_sym_cipher.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/
#include "lac_session.h"
#include "lac_mem.h"
#include "lac_common.h"
#include "lac_sym_cipher_defs.h"
#include "lac_sym_cipher.h"
#include "lac_sym_stats.h"
#include "lac_sym.h"
#include "lac_sym_qat_cipher.h"
#include "lac_log.h"
#include "lac_buffer_desc.h"

/*
*******************************************************************************
* Static Variables
*******************************************************************************
*/

CpaStatus
LacCipher_PerformIvCheckAndAlign(
    lac_sym_bulk_cookie_t * pCbCookie,
    Cpa32U qatPacketType,
    Cpa8U **ppAlignedIvBuffer)
{
    lac_session_desc_t *pSessionDesc = pCbCookie->pSessionDesc;
    const CpaCySymOpData * pOpData = pCbCookie->pOpData;
    CpaCySymCipherAlgorithm algorithm = 
        pSessionDesc->cipherAlgorithm;
        
/* Perform IV check.  Re-align if necessary */
    if (LAC_CIPHER_IS_CTR_MODE(algorithm) || LAC_CIPHER_IS_CBC_MODE(algorithm))
    {
        unsigned ivLenInBytes = LacSymQat_CipherIvSizeBytesGet(algorithm);

#ifdef ICP_PARAM_CHECK
        LAC_CHECK_NULL_PARAM(pOpData->pIv);
        if (pOpData->ivLenInBytes != ivLenInBytes)
        {
            LAC_INVALID_PARAM_LOG("invalid cipher IV size");
            return CPA_STATUS_INVALID_PARAM;
        }
#endif //ICP_PARAM_CHECK


        /* Always copy the user's IV into another cipher state buffer if
         * (i) alignment is not correct for access by the QAT engine, or
         * (ii) the request is part of a partial packet sequence
         *      (ensures that pipelined partial requests use same buffer)
         */
        if (ICP_QAT_FW_LA_PARTIAL_NONE == qatPacketType)
        {
            Cpa64U physAddr =
                LAC_OS_VIRT_TO_PHYS((Cpa32U) pOpData->pIv);

            if (LAC_ADDRESS_ALIGNED(physAddr, LAC_8BYTE_ALIGNMENT_SHIFT))
                /* User IV address aligned on 8-byte boundary */
            {
                *ppAlignedIvBuffer = pOpData->pIv;
            }
            else
                /* User IV address not aligned */
            {
                /* For full packets, we use a per-request buffer to re-align
                 * the IV.  This allows us to support multiple concurrent
                 * full operations on a given session
                 */ 
                *ppAlignedIvBuffer = pCbCookie->alignedIvBuffer;
                /* Copy in the user's IV to the temporary aligned buffer */
                memcpy(*ppAlignedIvBuffer, pOpData->pIv, ivLenInBytes);
            }
        }
        else
        {
            /* For partial packets, we use a per-session buffer to maintain
             * the IV.  This allows us to easily pass the updated IV forward
             * to the next partial in the sequence.  This makes internal
             * buffering of partials easier to implement.
             */ 
            *ppAlignedIvBuffer = pSessionDesc->cipherPartialOpState;
            
            /* Ensure that the user's IV buffer gets updated between partial
             * requests so that they may also see the residue from the
             * previous partial.  Not needed for final partials though.
             */
            if ((ICP_QAT_FW_LA_PARTIAL_START == qatPacketType) ||
                (ICP_QAT_FW_LA_PARTIAL_MID == qatPacketType))
            {               
                pCbCookie->updateUserIvOnRecieve = CPA_TRUE;
                
                if (ICP_QAT_FW_LA_PARTIAL_START == qatPacketType)
                {
                    /* if the previous partial state was full, then this is
                     * the first partial in the sequence so we need to copy
                     * in the user's IV. But, we have to be very careful
                     * here not to overwrite the cipherPartialOpState just
                     * yet in case there's a previous partial sequence in
                     * flight, so we defer the copy for now.  This will be
                     * completed in the LacSymQueue_RequestSend() function.
                     */
                    pCbCookie->updateSessionIvOnSend = CPA_TRUE;
                }
                /* For subsequent partials in a sequence, we'll re-use the
                 * IV that was written back by the QAT, using internal
                 * request queueing if necessary to ensure that the next
                 * partial request isn't issued to the QAT until the 
                 * previous one completes
                 */
            }
        }
    }
    else if(LAC_CIPHER_IS_ARC4(algorithm))
    {
        if (ICP_QAT_FW_LA_PARTIAL_NONE == qatPacketType)
        {
            /* For full packets, the initial RC4 state is stored in the 
             * session descriptor.  Use it directly.
             */
             *ppAlignedIvBuffer = pSessionDesc->cipherARC4InitialState;
        }
        else
        {
            /* For partial packets, we maintain the running RC4 state in 
             * dedicated buffer in the session descriptor
             */
            *ppAlignedIvBuffer = pSessionDesc->cipherPartialOpState;

            if (ICP_QAT_FW_LA_PARTIAL_START == qatPacketType)
            {
                /* if the previous partial state was full, then this is the
                 * first partial in the sequence so we need to (re-)initialise
                 * the contents of the state buffer using the initial state
                 * that is stored in the session descriptor. But, we have to be
                 * very careful here not to overwrite the cipherPartialOpState
                 * just yet in case there's a previous partial sequence in
                 * flight, so we defer the copy for now. This will be completed
                 * in the LacSymQueue_RequestSend() function when clear to send.
                 */
                pCbCookie->updateSessionIvOnSend = CPA_TRUE;
            }
        }
    }
    else
    {
        *ppAlignedIvBuffer = NULL;
    }
    
    return CPA_STATUS_SUCCESS;
}


#ifdef ICP_PARAM_CHECK

CpaStatus
LacCipher_SessionSetupDataCheck(
    const CpaCySymCipherSetupData * pCipherSetupData)
{   
    /* No key required for NULL algorithm */
    if (!LAC_CIPHER_IS_NULL(pCipherSetupData->cipherAlgorithm))
    {
        LAC_CHECK_NULL_PARAM(pCipherSetupData->pCipherKey);

        /* Check that algorithm and keys passed in are correct size */
        if (LAC_CIPHER_IS_ARC4(pCipherSetupData->cipherAlgorithm))
        {
            if (pCipherSetupData->cipherKeyLenInBytes > ICP_QAT_HW_RC4_KEY_SZ)
            {
                LAC_INVALID_PARAM_LOG("Invalid RC4 cipher key length");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        else if (LAC_CIPHER_IS_AES(pCipherSetupData->cipherAlgorithm))
        {
            if((pCipherSetupData->cipherKeyLenInBytes != 
                                                    ICP_QAT_HW_AES_128_KEY_SZ)
               && (pCipherSetupData->cipherKeyLenInBytes !=
                                                    ICP_QAT_HW_AES_192_KEY_SZ)
               && (pCipherSetupData->cipherKeyLenInBytes != 
                                                    ICP_QAT_HW_AES_256_KEY_SZ))
            {
                LAC_INVALID_PARAM_LOG("Invalid AES cipher key length");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        else if (LAC_CIPHER_IS_DES(pCipherSetupData->cipherAlgorithm))
        {
            if(pCipherSetupData->cipherKeyLenInBytes != ICP_QAT_HW_DES_KEY_SZ)
            {
                LAC_INVALID_PARAM_LOG("Invalid DES cipher key length");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        else if (LAC_CIPHER_IS_TRIPLE_DES(pCipherSetupData->cipherAlgorithm))
        {
            if(pCipherSetupData->cipherKeyLenInBytes != ICP_QAT_HW_3DES_KEY_SZ)
            {
                LAC_INVALID_PARAM_LOG("Invalid Triple-DES cipher key length");
                return CPA_STATUS_INVALID_PARAM;
            }
        }
        else
        {
            LAC_INVALID_PARAM_LOG("Invalid cipher algorithm");
            return CPA_STATUS_INVALID_PARAM;
        }
    }

    return CPA_STATUS_SUCCESS;
}


CpaStatus
LacCipher_PerformParamCheck(
    CpaCySymCipherAlgorithm algorithm,
    const CpaCySymOpData *pOpData,
    const Cpa32U packetLen)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    if (pOpData->messageLenToCipherInBytes > LAC_MAX_16_BIT_VALUE)
    {
        LAC_INVALID_PARAM_LOG("messageLenToCipherInBytes must be "
                "a 16 bit number");
            return CPA_STATUS_INVALID_PARAM;
    }

    /* The following check will cover the dstBuffer as well, since
     * the dstBuffer cannot be smaller than the srcBuffer (checked in 
     * LacSymPerform_BufferParamCheck() called from LacSym_Perform())
    */
    if ((pOpData->messageLenToCipherInBytes +
         pOpData->cryptoStartSrcOffsetInBytes) > packetLen)
    {
        LAC_INVALID_PARAM_LOG("cipher len + offset greater than "
                            "srcBuffer packet len");
        status = CPA_STATUS_INVALID_PARAM;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Perform algorithm-specific checks */
        if (!(LAC_CIPHER_IS_ARC4(algorithm) ||
              LAC_CIPHER_IS_CTR_MODE(algorithm)))
        {
/* Mask & check below is based on assumption that block size is a power of 2.
 * If data size is not a multiple of the block size, the "remainder" bits 
 * selected by the mask be non-zero
 */
            if (pOpData->messageLenToCipherInBytes & 
                (LacSymQat_CipherBlockSizeBytesGet(algorithm) - 1))
            {
                LAC_INVALID_PARAM_LOG("data size must be block size multiple");
                status = CPA_STATUS_INVALID_PARAM;
            }
        }

    }

    return status;
}

#endif //ICP_PARAM_CHECK
