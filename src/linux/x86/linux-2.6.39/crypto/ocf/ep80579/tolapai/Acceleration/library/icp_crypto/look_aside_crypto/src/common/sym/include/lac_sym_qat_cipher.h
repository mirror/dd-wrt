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
 *****************************************************************************
 * @file lac_sym_qat_cipher.h
 *
 * @defgroup LacSymQat_Cipher  Cipher QAT
 *
 * @ingroup LacSymQat
 *
 * external interfaces for populating QAT structures for cipher operations.
 *
 *****************************************************************************/


/*****************************************************************************/

#ifndef LAC_SYM_QAT_CIPHER_H
#define LAC_SYM_QAT_CIPHER_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa_cy_sym.h"
#include "icp_qat_fw_la.h"
#include "lac_session.h"


/**
 ******************************************************************************
 * @ingroup LacSymQat_Cipher
 *      Retrieve the cipher block size in bytes for a given algorithm
 *
 * @description
 *      This function returns a hard-coded block size for the specific cipher
 *      algorithm
 *
 * @param[in] cipherAlgorithm   Cipher algorithm for the current session
 *
 * @retval The block size, in bytes, for the given cipher algorithm
 *
 *****************************************************************************/
Cpa8U
LacSymQat_CipherBlockSizeBytesGet(
    CpaCySymCipherAlgorithm cipherAlgorithm);

/**
 ******************************************************************************
 * @ingroup LacSymQat_Cipher
 *      Retrieve the cipher IV/state size in bytes for a given algorithm
 *
 * @description
 *      This function returns a hard-coded IV/state size for the specific cipher
 *      algorithm
 *
 * @param[in] cipherAlgorithm   Cipher algorithm for the current session
 *
 * @retval The IV/state size, in bytes, for the given cipher algorithm
 *
 *****************************************************************************/
Cpa32U
LacSymQat_CipherIvSizeBytesGet(
    CpaCySymCipherAlgorithm cipherAlgorithm);

/**
 ******************************************************************************
 * @ingroup LacSymQat_Cipher
 *      Populate the Cipher specific part of the content descriptor.
 *
 * @description
 *      This function populates the Cipher specific fields of the control block
 *      and the hardware setup block for a cipher session. This function sets
 *      the size param to hold the size of the cipher hardware setup block.
 *
 *      In the case of cipher only, the content descriptor will contain just a
 *      cipher control block and cipher setup block. In the case of chaining it
 *      will contain the cipher control block and setup block along with the
 *      control block and setup blocks of additional services.
 *
 *      Note: The memory for the content descriptor MUST be allocated prior to
 *      calling this function. The memory for the cipher control block and 
 *      cipher setup block MUST be set to 0 prior to calling this function.
 *
 *      Note: The cipher key length is ignored in the case of ARC4 and NULL
 *      algorithms, as no key will be passed to the QAT in those cases.
 *
 * @image html contentDescriptor.png "Content Descriptor"
 *
 * @param[in] pCipherSetupData          Pointer to cipher setup data 
 *
 * @param[in] targetKeyLenInBytes       Target key length.  If key length given
 *                                      in cipher setup data is less that this,
 *                                      the key will be "rounded up" to this
 *                                      target length by padding it with 0's.
 *                                      In normal no-padding case, the target
 *                                      key length MUST match the key length
 *                                      in the cipher setup data.
 *
 * @param[in] pCipherControlBlock       Pointer to the cipher control Block
 *
 * @param[in] pHwBlockBase              Pointer to the base of the hardware
 *                                      setup block
 *
 * @param[in] cipherBlkOffsetInHwBlock  Offset in bytes from the base of
 *                                      the hardware setup block where the
 *                                      cipher block will start. This offset
 *                                      is stored in the control block. It
 *                                      is used to figure out where to write
 *                                      that cipher setup block.
 *
 * @param[in] nextSlice                 SliceID for next control block
 *                                      entry.  This value is known only by
 *                                      the calling component
 *
 * @param[out] pCipherHwBlockSizeBytes   size in bytes of cipher setup block
 *
 *
 * @retval void
 *
 *****************************************************************************/
void
LacSymQat_CipherContentDescPopulate(
    const CpaCySymCipherSetupData * pCipherSetupData,
    Cpa32U targetKeyLenInBytes,
    icp_qat_fw_cipher_hdr_t * pCipherControlBlock,
    void * pHwBlockBase,
    Cpa32U cipherBlkOffsetInHwBlock,
    icp_qat_fw_slice_t nextSlice,
    Cpa32U * pCipherHwBlockSizeBytes);

/**
 ******************************************************************************
 * @ingroup LacSymQat_Cipher
 *      Populate the cipher request params structure
 *
 * @description
 *      This function is passed a pointer to the cipher request params block.
 *      (This memory must be allocated prior to calling this function). It
 *      populates the fields of this block using the parameters as described
 *      below. It is also expected that this structure has been set to 0
 *      prior to calling this function.
 *
 *
 * @param[in] pCipherReqParams    Pointer to cipher request params block.
 *
 * @param[in] nextSlice           Id of next slice. For cipher-only requests
 *                                    this will be MEM_OUT which indicates that
 *                                    the data will not be passed to any more
 *                                    slices.  For other services using 
 *                                    functions, it is their resposibility to
 *                                    indicate what the next slice is, as they
 *                                    know the next slice.
 *
 * @param[in] cipherAlgorithm     Cipher algorithm for the current session
 *
 * @param[in] cipherOffsetInBytes Offset to cipher data in user data buffer
 *
 * @param[in] cipherLenInBytes    Length of cipher data in buffer
 *
 * @param[in] pAlignedIvBuffer    Pointer to 8-byte aligned IV/state buffer
 *
 * @retval void
 *
 *****************************************************************************/
void
LacSymQat_CipherRequestParamsPopulate(
    icp_qat_fw_la_cipher_req_params_t * pCipherReqParams,
    icp_qat_fw_slice_t nextSlice,
    CpaCySymCipherAlgorithm cipherAlgorithm,
    Cpa32U cipherOffsetInBytes,
    Cpa32U cipherLenInBytes,
    const Cpa8U * pAlignedIvBuffer);

/**
 ******************************************************************************
 * @ingroup LacSymQat_Cipher
 *       Derive initial ARC4 cipher state from a base key
 *
 * @description
 *       An initial state for an ARC4 cipher session is derived from the base
 *       key provided by the user, using the ARC4 Key Scheduling Algorithm (KSA)
 *
 * @param[in] pKey              The base key provided by the user  
 *
 * @param[in] keyLenInBytes     The length of the base key provided.
 *                              The range of valid values is 1-256 bytes
 *
 * @param[out] pArc4CipherState The initial state is written to this buffer,
 *                              including i and j values, and 6 bytes of padding
 *                              so 264 bytes must be allocated for this buffer
 *                              by the caller
 *
 * @retval void
 *
 *****************************************************************************/
void
LacSymQat_CipherArc4StateInit (
    const Cpa8U *pKey,
    Cpa32U keyLenInBytes,
    Cpa8U *pArc4CipherState);

#endif  /* LAC_SYM_QAT_CIPHER_H */
