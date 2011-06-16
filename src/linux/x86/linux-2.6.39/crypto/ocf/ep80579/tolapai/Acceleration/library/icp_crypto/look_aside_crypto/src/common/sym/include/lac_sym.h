/*****************************************************************************
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
 ****************************************************************************/

/**
 ***************************************************************************
 * @file lac_sym.h
 *
 * @defgroup LacSym    Symmetric
 *
 * @ingroup Lac
 *
 * Symmetric component includes cipher, Hash, chained cipher & hash, 
 * authenticated encryption and key generation.
 * 
 * @lld_start
 * @lld_overview
 * 
 * The symmetric component demuliplexes the following crypto operations to 
 * the appropriate sub-components: cipher, hash, algorithm chaining and 
 * authentication encryption. It is a common layer between the above 
 * mentioned components where common resources are allocated and paramater 
 * checks are done. The operation specific resource allocation and parameter 
 * checks are done in the sub-component itself.
 * 
 * The symmetric component demultiplexes the session register/deregister
 * and perform functions to the appropriate subcomponents. 
 *
 * @lld_dependencies
 * - \ref LacSymPartial "Partial Packet Code":  This code manages the partial
 *    packet state for a session.
 * - \ref LacBufferDesc  "Common Buffer Code" : This code traverses a buffer
 *   chain to ensure it is valid. 
 * - \ref LacSymStats "Statistics": Manages statistics for symmetric
 * - \ref LacSymQat "Symmetric QAT": The symmetric qat component is
 *   initialiased by the symmetric component. 
 * - \ref LacCipher "Cipher" : demultiplex cipher opertions to this component.
 * - \ref LacHash "Hash" : demultiplex hash opertions to this component.
 *   to this component.
 * - \ref LacAlgChain "Algorithm Chaining": The algorithm chaining component
 * - OSAL : Memory allocation, Mutex's, atomics
 * 
 * @lld_initialisation
 * This component is initialied during the LAC initialisation sequence. It 
 * initialises the session table, statistics, symmetric QAT, initialises the
 * hash definitions lookup table, the hash alg supported lookup table and
 * registers a callback function with the symmetric response handler to process
 * response messages for Cipher, Hash and Algorithm-Chaining requests.
 *
 * @lld_module_algorithms
 *
 * @lld_process_context
 * Refer to \ref LacHash "Hash" and \ref LacCipher "Cipher" for sequence 
 * diagrams from the symmetric component through the sub components.
 *
 * @lld_end
 *
 ***************************************************************************/

/***************************************************************************/

#ifndef LAC_SYM_H
#define LAC_SYM_H

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "lac_common.h"
#include "lac_session.h"
#include "cpa_cy_rand.h"
#include "lac_mem_pools.h"
#include "lac_sym_cipher_defs.h"
#include "icp_qat_fw_la.h"


#define LAC_SYM_REQ_PARAMS_SIZE_MAX              \
    (sizeof(icp_qat_fw_la_cipher_req_params_t) + \
     sizeof(icp_qat_fw_la_auth_req_params_t))
/**< Reserve enough space for cipher and authentication request params */

#define LAC_SYM_REQ_PARAMS_SIZE_PADDED                  \
    LAC_ALIGN_POW2_ROUNDUP(LAC_SYM_REQ_PARAMS_SIZE_MAX, \
                           LAC_OPTIMAL_ALIGNMENT_SHIFT)
/**< Pad out to 64-byte multiple to ensure optimal alignment of next field */

#define LAC_SYM_KEY_TLS_PREFIX_SIZE 64
/**< Prefix size in bytes for TLS */

#define LAC_SYM_KEY_MAX_HASH_STATE_BUFFER   (LAC_SYM_KEY_TLS_PREFIX_SIZE * 2)
/**< hash state prefix buffer structure that holds the maximum sized secret */

/**
 *******************************************************************************
 * @ingroup LacSym
 *      Symmetric cookie
 *
 * @description
 *      This cookie stores information for a particular symmetric perform op.
 *      This includes the request params, re-aligned Cipher IV, the request
 *      message sent to the QAT engine, and various user-supplied parameters
 *      for the operation which will be needed in our callback function.
 *      A pointer to this cookie is stored in the opaque data field of the QAT
 *      message so that it can be accessed in the asynchronous callback.
 *      Cookies for multiple operations on a given session can be linked
 *      together to allow queuing of requests using the pNext field.
 *****************************************************************************/
typedef struct lac_sym_bulk_cookie_s
{
    Cpa8U reqParamsBuffer[LAC_SYM_REQ_PARAMS_SIZE_PADDED];
    /**< memory block reserved for request params
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    Cpa8U alignedIvBuffer[LAC_CIPHER_IV_SIZE_MAX];
    /**< Buffer to hold a re-aligned IV for a single request (for full ops only)
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    Cpa8U aadData[LAC_MAX_INNER_OUTER_PREFIX_SIZE_BYTES]; 
    /**< AAD Data varies per request and must be stored in the cookie */
    icp_qat_fw_la_bulk_req_t qatMsg;
    /**< QAT request message */
    CpaInstanceHandle instanceHandle;
    /**< Instance handle for the operation */
    void *pCallbackTag;
    /**< correlator supplied by the client */
    lac_session_desc_t *pSessionDesc;
    /**< pointer to the session descriptor */
    const CpaCySymOpData *pOpData;
    /**< pointer to the op data structure that the user supplied in the perform
     * operation. The op data is modified in the process callback function
     * and the pointer is returned to the user in their callback function */
    CpaBoolean updateSessionIvOnSend;
    /**< Boolean flag to indicate if the session cipher IV buffer should be
     * updated prior to sending the request */
    CpaBoolean updateUserIvOnRecieve;
    /**< Boolean flag to indicate if the user's cipher IV buffer should be
     * updated after recieving the response from the QAT */
    const CpaBufferList *pSrcBuffer;
    /**< Pointer to source buffer with the data to be processed */
    CpaBufferList *pDstBuffer;
    /**< Pointer to destination buffer to hold the data output */
    CpaBoolean *pVerifyResult;
    /**< In synchronous mode, this parameter is returned 
     *   when the digestVerify option is set in the 
     *   CpaCySymOpData structure. A value of CPA_TRUE 
     *   indicates that the compare succeeded. A value of
     *   CPA_FALSE indicates that the compare failed for
     *   an unspecified reason.  */

    struct lac_sym_bulk_cookie_s *pNext;
    /**< Pointer to next node in linked list (if request is queued) */
} lac_sym_bulk_cookie_t;

/**
*******************************************************************************
 * @ingroup LacSymKey
 *      symmetric Key cookie
 * @description
 *      This cookie stores information for a particular keygen perform op.
 *      This includes a hash content descriptor, request params, hash state
 *      buffer, and various user-supplied parameters for the operation which
 *      will be needed in our callback function.
 *      A pointer to this cookie is stored in the opaque data field of the QAT
 *      message so that it can be accessed in the asynchronous callback.
 *****************************************************************************/
typedef struct lac_sym_key_cookie_s
{
    Cpa8U contentDesc[LAC_SYM_QAT_HASH_CONTENT_DESC_SIZE];
    /**< Content descriptor. 
     **< NOTE: Field must be correctly aligned in memory for access by QAT
     * engine */
    icp_qat_fw_la_auth_req_params_t hashRequestParams;
    /**< Hash Request param structure
     **< NOTE: Field must be correctly aligned in memory for access by QAT
     * engine */
    union {
        icp_qat_fw_la_ssl_key_material_input_t  sslKeyInput;
        /**< SSL key material input structure */  
        icp_qat_fw_la_tls_key_material_input_t  tlsKeyInput;
        /**< TLS key material input structure */        
    } u;
    /**< NOTE: Field must be correctly aligned in memory for access by QAT
     * engine */
    Cpa8U hashStateBuffer[LAC_SYM_KEY_MAX_HASH_STATE_BUFFER];
    /**< hash state prefix buffer 
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    CpaInstanceHandle instanceHandle;
    /**< QAT device id supplied by the client */
    CpaCyGenFlatBufCbFunc pKeyGenCb;
    /**< callback function supplied by the client */
    void *pCallbackTag;
    /**< Mechanism used. TLS, SSL or MGF */
    void *pKeyGenOpData;
    /**< pointer to the (SSL/TLS) or MGF op data structure that the user
     * supplied in the perform operation */
} lac_sym_key_cookie_t;


/**
*******************************************************************************
 * @ingroup LacSymRandom
 *      symmetric Random cookie
 * @description
 *      This cookie stores information for a particular random perform op.
 *      This includes various user-supplied parameters for the operation which
 *      will be needed in our callback function.
 *      A pointer to this cookie is stored in the opaque data field of the QAT
 *      message so that it can be accessed in the asynchronous callback.
 *****************************************************************************/
typedef struct lac_sym_random_cookie_s
{
    CpaCyGenFlatBufCbFunc pRandCb;
    /**< Callback function supplied by the client */
    CpaInstanceHandle instanceHandle;
    /**< QAT device id supplied by the client */
    void *pCallbackTag;
    /**< Opaque data supplied by the client */
    const CpaCyRandGenOpData *pOpData;
    /**< Op data pointer supplied by the client */
    CpaFlatBuffer *pRandData;
    /**< Rand data pointer supplied by the client */
} lac_sym_random_cookie_t;

/**
*******************************************************************************
 * @ingroup LacSym
 *      symmetric cookie
 * @description
 *      used to determine the amount of memory to allocate for the symmetric 
 *      cookie pool. As symmetric, random and key generation shared the same
 *      pool 
 *****************************************************************************/
typedef union
{
    lac_sym_bulk_cookie_t bulkCookie;
    /**< symmetric bulk cookie */
    lac_sym_key_cookie_t keyCookie;
    /**< symmetric key cookie */
    lac_sym_random_cookie_t randomCookie;
    /**< symmetric random cookie */
} lac_sym_cookie_t;

/**
 *******************************************************************************
 * @ingroup LacSym
 *      Symmetric cookie pool ID
 *
 * @description
 *      Identifier for pool of pre-allocated cookies for all symmetric modules.
 *****************************************************************************/
extern lac_memory_pool_id_t lac_sym_cookie_memory;

#endif /* LAC_SYM_H */
