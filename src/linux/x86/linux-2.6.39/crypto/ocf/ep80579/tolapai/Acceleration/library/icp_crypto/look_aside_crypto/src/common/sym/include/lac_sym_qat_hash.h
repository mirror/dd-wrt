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
 * @file lac_sym_qat_hash.h
 * 
 * @defgroup LacSymQatHash  Hash QAT
 *
 * @ingroup LacSymQat
 * 
 * interfaces for populating qat structures for a hash operation
 *
 *****************************************************************************/



/*****************************************************************************/

#ifndef LAC_SYM_QAT_HASH_H
#define LAC_SYM_QAT_HASH_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "icp_qat_fw_la.h"
#include "icp_qat_hw.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/
#include "lac_common.h"
#include "lac_sym_hash_defs.h"

#define LAC_SYM_QAT_HASH_STATE1_MAX_SIZE_BYTES  LAC_HASH_SHA512_BLOCK_SIZE
/**< @ingroup LacSymQatHash
 * Maximum size of state1 in the hash setup block of the content descriptor. 
 * This is set to the block size of SHA512. */

#define LAC_SYM_QAT_HASH_STATE2_MAX_SIZE_BYTES  LAC_HASH_SHA512_BLOCK_SIZE
/**< @ingroup LacSymQatHash
 * Maximum size of state2 in the hash setup block of the content descriptor.
 * This is set to the block size of SHA512. */

#define LAC_MAX_INNER_OUTER_PREFIX_SIZE_BYTES 255
/**< Maximum size of the inner and outer prefix for nested hashing operations.
 * This is got from the maximum size supported by the accelerator which stores
 * the size in an 8bit field */

#define LAC_MAX_HASH_STATE_STORAGE_SIZE \
    (sizeof(icp_qat_hw_auth_counter_t) + LAC_HASH_SHA512_STATE_SIZE)
/**< Maximum size of the hash state storage section of the hash state prefix
 * buffer */

#define LAC_MAX_HASH_STATE_BUFFER_SIZE_BYTES                                \
        LAC_MAX_HASH_STATE_STORAGE_SIZE +    \
        (LAC_ALIGN_POW2_ROUNDUP(LAC_MAX_INNER_OUTER_PREFIX_SIZE_BYTES,      \
                                LAC_QUAD_WORD_IN_BYTES) * 2)    
/**< Maximum size of the hash state prefix buffer will be for nested hash when
 * there is the maximum sized inner prefix and outer prefix */


#define IS_HMAC_ALG(algorithm)                          \
     ((algorithm == CPA_CY_SYM_HASH_MD5) ||             \
      (algorithm == CPA_CY_SYM_HASH_SHA1) ||            \
      (algorithm == CPA_CY_SYM_HASH_SHA224) ||          \
      (algorithm == CPA_CY_SYM_HASH_SHA256) ||          \
      (algorithm == CPA_CY_SYM_HASH_SHA384) ||          \
      (algorithm == CPA_CY_SYM_HASH_SHA512)) 
/**< @ingroup LacSymQatHash
 * Macro to detect if the hash algorithm is a HMAC algorithm */


#define IS_HASH_MODE_1(qatHashMode)                     \
    (ICP_QAT_HW_AUTH_MODE1 == qatHashMode)
/**< @ingroup LacSymQatHash
 * Macro to detect is qat hash mode is set to 1 (precompute mode)
 * only used with algorithms in hash mode CPA_CY_SYM_HASH_MODE_AUTH */


#define IS_HASH_MODE_2(qatHashMode)                     \
    (ICP_QAT_HW_AUTH_MODE2 == qatHashMode)
/**< @ingroup LacSymQatHash
 * Macro to detect is qat hash mode is set to 2. This is used for TLS and
 * mode 2 HMAC (no preompute mode) */   


#define IS_HASH_MODE_2_AUTH(qatHashMode, hashMode)      \
    ((IS_HASH_MODE_2(qatHashMode)) &&                   \
     (CPA_CY_SYM_HASH_MODE_AUTH == hashMode))
/**< @ingroup LacSymQatHash
 * Macro to check for qat hash mode is set to 2 and the hash mode is
 * Auth. This applies to HMAC algorithms (no pre compute). This is used 
 * to differntiate between TLS and HMAC */

#define IS_HASH_MODE_2_NESTED(qatHashMode, hashMode)    \
    ((IS_HASH_MODE_2(qatHashMode)) &&                   \
     (CPA_CY_SYM_HASH_MODE_NESTED == hashMode))
/**< @ingroup LacSymQatHash
 * Macro to check for qat hash mode is set to 2 and the LAC hash mode is 
 * Nested. This applies to TLS. This is used to differntiate between 
 * TLS and HMAC */

/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      hash precomputes
 *
 * @description
 *      This structure contains infomation on the hash precomputes
 *
 *****************************************************************************/
typedef struct lac_sym_qat_hash_precompute_info_s
{
    Cpa8U *pState1;
    /**< state1 pointer */
    Cpa32U state1Size;
    /**< state1 size */
    Cpa8U *pState2;
    /**< state2 pointer */
    Cpa32U state2Size;
    /**< state2 size */
}lac_sym_qat_hash_precompute_info_t;

/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      hash state prefix buffer info
 *
 * @description
 *      This structure contains infomation on the hash state prefix aad buffer 
 *
 *****************************************************************************/
typedef struct lac_sym_qat_hash_state_buffer_info_s
{
    Cpa64U pDataPhys;
    /**< Physical pointer to the hash state prefix buffer */
    Cpa8U *pData;
    /**< Virtual pointer to the hash state prefix buffer */
    Cpa8U stateStorageSzQuadWords;
    /**< hash state storage size in quad words */
    Cpa8U prefixAadSzQuadWords;
    /**< inner prefix/aad and outer prefix size in quad words */
}lac_sym_qat_hash_state_buffer_info_t;

/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      Init the hash specific part of the content descriptor.  
 *
 * @description
 *      This function populates the hash specific fields of the control block 
 *      and the hardware setup block for a digest session. This function sets 
 *      the size param to hold the size of the hash setup block. 
 * 
 *      In the case of hash only, the content descriptor will contain just a 
 *      hash control block and hash setup block. In the case of chaining it 
 *      will contain the hash control block and setup block along with the
 *      control block and setup blocks of additional services. 
 *
 *      Note: The memory for the content descriptor MUST be allocated prior to 
 *      calling this function. The memory for the hash control block and hash 
 *      setup block MUST be set to 0 prior to calling this function.        
 *  
 * @image html contentDescriptor.png "Content Descriptor"       
 * 
 * @param[in] pHashSetupData                  Pointer to the hash setup data as
 *                                      defined in the LAC API.
 *
 * @param[in] pHashControlBlock         Pointer to the hash control Block
 * 
 * @param[in] pHwBlockBase              Pointer to the base of the hardware
 *                                      setup block 
 *
 * @param[in] hashBlkOffsetInHwBlock    Offset in bytes from the base of
 *                                      the hardware setup block where the
 *                                      hash block will start. This offset 
 *                                      is stored in the control block. It 
 *                                      is used to figure out where to write
 *                                      that hash setup block.
 *
 * @param[in] nextSlice                 SliceID for next control block 
 *                                      entry This value is known only by 
 *                                      the calling component
 * 
 * @param[in] qatHashMode               QAT hash mode
 *  
 * @param[in] pPrecompute               For auth mode, this is the pointer 
 *                                      to the precompute data. Otherwise this
 *                                      should be set to NULL
 *
 * @param[out] pHashBlkSizeInBytes      size in bytes of hash setup block 
 * 
 * @return void
 *
 *****************************************************************************/
void
LacSymQat_HashContentDescInit(
        const CpaCySymHashSetupData *pHashSetupData,
        icp_qat_fw_auth_hdr_t *pHashControlBlock,
        void *pHwBlockBase,
        Cpa32U hashBlkOffsetInHwBlock,
        icp_qat_fw_slice_t nextSlice,
        icp_qat_hw_auth_mode_t qatHashMode,
        lac_sym_qat_hash_precompute_info_t *pPrecompute,
        Cpa32U *pHashBlkSizeInBytes);


/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      Calculate the size of the hash state prefix aad buffer and create it
 *
 * @description
 *      This function inspects the hash control block and based on the values 
 *      in the fields, it calculates the size of the hash state prefix aad 
 *      buffer. 
 *
 *      A partial packet processing request is possible at any stage during a 
 *      hash session. In this case, there will always be space for the hash 
 *      state storage field of the hash state prefix buffer. When there is 
 *      AAD data just the inner prefix AAD data field is used. 
 *
 * @param[in]  pHashSetupData             Pointer to the hash setup data
 * 
 * @param[in]  pHashControlBlock    Pointer to the control block of the
 *                                  content descriptor
 * 
 * @param[in]  qatHashMode          QAT hash mode
 * 
 * @param[out] pHashStateBuf        Pointer to hash state prefix buffer info 
 *                                  structure. 
 *
 * @return None
 *
 *****************************************************************************/
void
LacSymQat_HashStatePrefixAadBufferSizeGet(
        const CpaCySymHashSetupData *pHashSetupData,
        const icp_qat_fw_auth_hdr_t *pHashControlBlock,
        icp_qat_hw_auth_mode_t qatHashMode,
        lac_sym_qat_hash_state_buffer_info_t *pHashStateBuf);
 
/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      Populate the fields of the hash state prefix buffer
 *
 * @description
 *      This function populates the inner prefix/aad fields and/or the outer
 *      prefix field of the hash state prefix buffer.
 *
 * @param[in] pHashStateBuf         Pointer to hash state prefix buffer info 
 *                                  structure.
 * 
 * @param[in] pHashControlBlock     Pointer to the hash control block
 *  
 * @param[in] pInnerPrefixAad       Pointer to the Inner Prefix or Aad data
 *                                  This is NULL where if the data size is 0
 *
 * @param[in] innerPrefixSize       Size of inner prefix/aad data
 * 
 * @param[in] pOuterPrefix          Pointer to the Outer Prefix data. This is
 *                                  NULL where the data size is 0.
 *
 * @param[in] outerPrefixSize       Size of the outer prefix data
 *
 * @return void
 *
 *****************************************************************************/
void
LacSymQat_HashStatePrefixAadBufferPopulate(
    lac_sym_qat_hash_state_buffer_info_t *pHashStateBuf,
    const icp_qat_fw_auth_hdr_t *pHashControlBlock, 
    Cpa8U *pInnerPrefixAad,
    Cpa8U innerPrefixSize,
    Cpa8U *pOuterPrefix,
    Cpa8U outerPrefixSize);


/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      Populate the hash request params structure
 *
 * @description
 *      This function is passed a pointer to the hash request params block. 
 *      (This memory must be allocated prior to calling this function). It 
 *      populates the fields of this block using the parameters as described 
 *      below. It is also expected that this structure has been set to 0
 *      prior to calling this function. 
 * 
 *
 * @param[out] pHashReqParams       Pointer to hash request params block.
 *
 * @param[in] nextSlice             Id of next slice. For a hash only request 
 *                                  this will be NULL which indicates that the
 *                                  data will not be passed to any more slices.
 *                                  For Other services using hash functionality 
 *                                  it is their resposibility to indicate what 
 *                                  the next slice is, as they know the next
 *                                  slice.                                 
 *
 * @param[in] pHashStateBuf         Pointer to hash state buffer info. This 
 *                                  structure contains the pointers and sizes.
 *                                  If there is no hash state prefix buffer
 *                                  required, this parameter can be set to NULL 
 *                                     
 * @param[in] qatPacketType         Packet type using QAT macros. The hash 
 *                                  state buffer pointer and state size will be
 *                                  different depending on the packet type 
 *
 * @param[in] hashResultSize        Size of the final hash result in bytes.   
 *      
 * @param[in] authOffsetInBytes     start offset of data that the digest is to
 *                                  be computed on. 
 *
 * @param[in] authLenInBytes        Length of data digest calculated on
 * 
 * @param[in] digestVerify          Indicates if verify is enabled or not
 *      
 * @param[in] pAuthResult           Virtual pointer to digest 
 *
 * @return void
 *
 *****************************************************************************/
void
LacSymQat_HashRequestParamsPopulate(
    icp_qat_fw_la_auth_req_params_t *pHashReqParams,
    icp_qat_fw_slice_t nextSlice,
    lac_sym_qat_hash_state_buffer_info_t *pHashStateBuf,
    Cpa32U qatPacketType,
    Cpa32U hashResultSize,
    Cpa32U authOffsetInBytes,
    Cpa32U authLenInBytes,
    CpaBoolean digestVerify,
    Cpa8U *pAuthResult); 


/**
 ******************************************************************************
 * @ingroup LacSymQatHash
 *      Populate the hash command flags
 *
 * @description
 *      This function populates the hash command flags for returning the auth
 *      result and for setting the compare command flag.
 *                        
 * @param[in] qatPacketType         Packet type using QAT macros
 *
 * @param[in] hashVerify            Boolean value indicating if the digest is
 *                                  requested to be verified
 *      
 * @param[in] pLaCommandFlags       Command Flags
 *
 * @return void
 *
 *****************************************************************************/
void
LacSymQat_HashLaCommandFlagsSet(
    Cpa32U qatPacketType,
    CpaBoolean hashVerify,
    Cpa16U *pLaCommandFlags); 

#endif /* LAC_SYM_QAT_HASH_H */
