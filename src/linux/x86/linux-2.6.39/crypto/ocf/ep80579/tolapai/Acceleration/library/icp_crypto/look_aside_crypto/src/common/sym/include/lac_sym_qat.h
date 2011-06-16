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
 * @file lac_sym_qat.h
 *
 * @defgroup LacSymQat  Symmetric QAT
 *
 * @ingroup LacSym 
 * 
 * Interfaces for populating the qat structures for a symmetric operation
 * 
 * @lld_start
 * 
 * @lld_overview
 * This file documents the interfaces for populating the qat structures
 * that are common for all symmetric operations.
 *
 * @lld_dependencies
 * - \ref LacSymQatHash "Hash QAT Comms" Sym Qat commons for Hash
 * - \ref LacSymQat_Cipher "Cipher QAT Comms" Sym Qat commons for Cipher
 * - OSAL: logging
 * - \ref LacMem "Memory" - Inline memory functions
 *
 * @lld_initialisation
 * This component is initialied during the LAC initialisation sequence. It
 * is called by the Symmetric Initialisation function. 
 *
 * @lld_module_algorithms
 *
 * @lld_process_context
 * Refer to \ref LacHash "Hash" and \ref LacCipher "Cipher" for sequence
 * diagrams to see their interactions with this code. 
 *
 * 
 * @lld_end
 *
 *****************************************************************************/


/*****************************************************************************/

#ifndef LAC_SYM_QAT_H
#define LAC_SYM_QAT_H


/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "icp_qat_fw_la.h"
#include "icp_qat_hw.h"
#include "lac_common.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

/* The RC4 key will not be stored in the content descriptor so we only need to
 * reserve enough space for the next biggest key size.*/
#define LAC_SYM_QAT_MAX_CIPHER_SETUP_BLK_SZ \
        (sizeof(icp_qat_hw_cipher_config_t) + ICP_QAT_HW_AES_256_KEY_SZ) 
/**< @ingroup LacSymQat
 * Maximum size for the cipher setup block of the content descriptor */

#define LAC_SYM_QAT_MAX_HASH_SETUP_BLK_SZ   sizeof(icp_qat_hw_auth_algo_blk_t)
/**< @ingroup LacSymQat
 * Maximum size for the hash setup block of the content descriptor */

#define LAC_SYM_QAT_HASH_CONTENT_DESC_SIZE \
        (sizeof(icp_qat_fw_auth_hdr_t) + LAC_SYM_QAT_MAX_HASH_SETUP_BLK_SZ)
/**< @ingroup LacSymQat
 * Maximum size for a hash-only content descriptor */

#define LAC_SYM_QAT_CIPHER_CONTENT_DESC_SIZE \
        (sizeof(icp_qat_fw_cipher_hdr_t) + LAC_SYM_QAT_MAX_CIPHER_SETUP_BLK_SZ)
/**< @ingroup LacSymQat
 * Maximum size for a cipher-only content descriptor */

#define LAC_SYM_QAT_CONTENT_DESC_MAX_SIZE   LAC_ALIGN_POW2_ROUNDUP( \
        LAC_SYM_QAT_HASH_CONTENT_DESC_SIZE +                        \
        LAC_SYM_QAT_CIPHER_CONTENT_DESC_SIZE,                       \
        (1 << LAC_64BYTE_ALIGNMENT_SHIFT)) 
/**< @ingroup LacSymQat
 *  Maximum size of content descriptor. This is incremented to the next multiple
 * of 64 so that it can be 64 byte aligned */

/**
 ******************************************************************************
 * @ingroup LacSymQat
 *      content descriptor info structure
 *
 * @description
 *      This structure contains information on the content descriptor
 *
 *****************************************************************************/
typedef struct lac_sym_qat_content_desc_info_s
{
    Cpa64U pDataPhys;
    /**< Physical Pointer to Content Descriptor */
    void *pData;
    /**< Virtual Pointer to Content Descriptor */
    Cpa8U hdrSzQuadWords;
    /**< Content desc header size in quad words */
    Cpa8U hwBlkSzQuadWords;
    /**< Hardware Setup Block size in quad words */
} lac_sym_qat_content_desc_info_t;


/**
 ******************************************************************************
 * @ingroup LacSymQat
 *      buffer type enumeration
 *
 * @description
 *      This enumeration lists the buffer types supported
 *
 *****************************************************************************/
typedef enum{
    LAC_SYM_QAT_START_DELIMITER = 0,
    /**< Enum start Delimiter */
    LAC_SYM_QAT_BUFFER_DESC,
    /**< Buffer descriptor */    
    LAC_SYM_QAT_FLAT_BUFFER,
    /**< Flat Buffer */
    LAC_SYM_QAT_BUFFER_LIST,
    /** Buffer List */
    LAC_SYM_QAT_END_DELIMITER
    /**< Enum end Delimiter */
}lac_sym_qat_buffer_type_t;


/**
 *******************************************************************************
 * @ingroup LacSymQat
 *      Initialise the Symmetric QAT code
 *
 * @description
 *      This function initialises the symmetric QAT code
 *
 * @return CPA_STATUS_SUCCESS       Operation successful
 * @return CPA_STATUS_FAIL           Initialisation Failed
 * 
 *****************************************************************************/
CpaStatus
LacSymQat_Init(void);

/**
 *******************************************************************************
 * @ingroup LacSymQat
 *      Shutdown the Symmetric QAT code
 *
 * @description
 *      This function shuts down the symmetric QAT code. Note that this
 *      function is currently only a placeholder. It only returns a status.
 *
 * @return CPA_STATUS_SUCCESS       Operation successful
 * 
 *****************************************************************************/
CpaStatus
LacSymQat_Shutdown(void);


/**
 *******************************************************************************
 * @ingroup LacSymQat
 *      Symmetric response handler function type
 *
 * @description
 *      This type definition specifies the function prototype for handling the
 *      response messages for a specific symmetric operation
 *
 * @param[in] lacCmdId      Look Aside Command ID
 *
 * @param[in] pOpaqueData   Pointer to Opaque Data
 * 
 * @param[in] cmnRespFlags  Common Response flags
 *
 * @return void
 * 
 *****************************************************************************/
typedef void (*lac_sym_qat_resp_handler_func_t)(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    void *pOpaqueData,
    icp_qat_fw_comn_flags cmnRespFlags);


/**
 *******************************************************************************
 * @ingroup LacSymQat
 *      Register a response handler function for a symmetric command ID
 *
 * @description
 *      This function registers a response handler function for a symmetric 
 *      operation.
 *
 *      Note: This operation should only be performed once by the init function 
 *      of a component. There is no corresponding deregister function, but 
 *      registering a NULL function pointer will have the same effect. There 
 *      MUST not be any requests in flight when calling this function.   
 *
 * @param[in] lacCmdId      Command Id of operation 
 *
 * @param[in] pCbHandler    callback handler function
 *
 * @return None
 * 
 *****************************************************************************/
void
LacSymQat_RespHandlerRegister(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    lac_sym_qat_resp_handler_func_t pCbHandler);

/**
 ******************************************************************************
 * @ingroup LacSymQat
 *      Populate the fields of a bulk request message
 *
 * @description
 *      This function populates the QAT symmetric bulk message.  All fields
 * are initialised to 0 before the message is populated.
 *
 * @param[out] pBulkRq              Pointer to a bulk request structure
 * @param[in] pContentDescInfo      Pointer to content descriptor info structure
 * @param[in] pCookie               Pointer to cookie
 * @param[in] srcBuffer             source buffer handle
 * @param[in] dstBuffer             destination buffer handle
 * @param[in] laCmdId               Command ID for bulk message
 * @param[in] pReqParams            Virtual Pointer to request params structure
 * @param[in] reqParamBlkSizeBytes  Size of request params structure
 * @param[in] laCmdFlags            LA command flags
 *
 * @return none
 *
 *****************************************************************************/
void
LacSymQat_BulkReqPopulate(
    icp_qat_fw_la_bulk_req_t * pBulkRq,
    const lac_sym_qat_content_desc_info_t * pContentDescInfo,
    const void * pCookie, 
    Cpa64U srcBuffer,
    Cpa64U dstBuffer,
    icp_qat_fw_la_cmd_id_t laCmdId,
    void * pReqParams,
    Cpa8U reqParamBlkSizeBytes,
    Cpa16U laCmdFlags);


/**
 ******************************************************************************
 * @ingroup LacSymQat
 *      Populate the fields of the common request message
 *
 * @description
 *      This function populates the QAT common request message. 
 *
 *      Note: Memory must be allocated for the message prior to calling this
 *      function. As the contents of the message is copied onto the ring it can 
 *      be allocated as a variable on the stack.
 *
 *
 * @param[out] pComnReq             Pointer to a common request structure
 * @param[in] pContentDescInfo      Pointer to content descriptor info structure
 * @param[in] pCookie               Pointer to cookie
 * @param[in] srcBuffer             source buffer. Physical pointer for
 *                                 a flat buffer otherwise a buffer handle.  
 * @param[in] dstBuffer             destination buffer. Physical pointer
 *                                 for a flat buffer otherwise a buffer handle.
 * @param[in] ordering              Ordering of messages
 * @param[in] bufferType            type of buffer
 *
 * @return none
 *
 *****************************************************************************/
void 
LacSymQat_ComnReqHdrPopulate(
    icp_qat_fw_comn_req_hdr_t * pComnReq,
    const lac_sym_qat_content_desc_info_t * pContentDescInfo,
    const void * pCookie,
    Cpa64U srcBuffer,
    Cpa64U dstBuffer,
    CpaBoolean ordering,
    lac_sym_qat_buffer_type_t bufferType);


/**
 ******************************************************************************
 * @ingroup LacSymQat
 *      Populate the command flags based on the packet type
 *
 * @description
 *      This function populates the packet type and update state LA command Flag
 *      based on looking at the packet type.
 *                        
 * @param[in] qatPacketType          Packet type (use QAT Macros for this)
 * @param[out] pLaCommandFlags       Command Flags
 *
 * @return none
 *
 *****************************************************************************/
void
LacSymQat_LaPacketCommandFlagSet(
    Cpa32U qatPacketType,
    Cpa16U *pLaCommandFlags);


/**
 ******************************************************************************
 * @ingroup LacSymQat
 *      get the QAT packet type 
 *
 * @description
 *      This function returns the QAT packet type for a LAC packet type. The 
 *      LAC packet type does not indicate a first partial. therefore for a 
 *      partial request, the previous packet type needs to be looked at to
 *      figure out if the current partial request is a first partial. 
 *      
 *
 * @param[in] packetType          LAC Packet type
 * @param[in] packetState         LAC Previous Packet state 
 * @param[out] pQatPacketType     Packet type using the QAT macros
 *
 * @return none
 *
 *****************************************************************************/
void
LacSymQat_packetTypeGet(
    CpaCySymPacketType packetType,
    CpaCySymPacketType packetState,
    Cpa32U *pQatPacketType);
    
#endif /* LAC_SYM_QAT_H */
