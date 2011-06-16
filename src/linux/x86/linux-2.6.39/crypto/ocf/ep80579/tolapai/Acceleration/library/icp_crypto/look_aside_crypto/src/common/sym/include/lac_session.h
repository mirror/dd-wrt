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
 *****************************************************************************
 * @file lac_session.h    
 * 
 * @defgroup LacSym_Session Session
 *
 * @ingroup LacSym
 *
 * Definition of symmetric session descriptor structure
 * 
 * @lld_start
 *
 * @lld_overview
 * A session is required for each symmetric operation. The session descriptor
 * holds information about the session from when the session is initialised to
 * when the session is removed. The session descriptor is used in the 
 * subsequent perform operations in the paths for both sending the request and
 * receiving the response. The session descriptor and any other state 
 * information required for processing responses from the QAT are stored in an
 * internal cookie. A pointer to this cookie is stored in the opaque data 
 * field of the QAT request. 
 * 
 * The user allocates the memory for the session using the size returned from
 * \ref cpaCySymSessionCtxGetSize(). Internally this memory is re-aligned on a
 * 64 byte boundary for use by the QAT engine. The aligned pointer is saved in
 * the first bytes (size of unsigned long) of the session memory. This address
 * is then dereferenced in subsequent performs to get access to the session 
 * descriptor.
 *
 * <b>LAC Session Init</b>\n The session descriptor is re-aligned and
 * populated. This includes populating the content descriptor which contains
 * the hardware setup for the QAT engine. The content descriptor is a read
 * only structure after session init and a pointer to it is sent to the QAT
 * for each perform operation.
 *
 * <b>LAC Perform </b>\n
 * The address for the session descriptor is got by dereferencing the first 
 * bytes of the session memory (size of unsigned long). For each successful 
 * request put on the ring, the pendingCbCount for the session is incremented. 
 *
 * <b>LAC Callback </b>\n
 * For each successful response the pendingCbCount for the session is 
 * decremented. See \ref LacSymCb_ProcessCallbackInternal()
 * 
 * <b>LAC Session Remove </b>\n
 * The address for the session descriptor is got by dereferencing the first 
 * bytes of the session memory (size of unsigned long). 
 * The pendingCbCount for the session is checked to see if it is 0. If it is
 * non 0 then there are requests in flight. An error is returned to the user.
 *
 * <b>Concurrency</b>\n
 * A reference count is used to prevent the descriptor being removed
 * while there are requests in flight.
 *
 * <b>Reference Count</b>\n
 * - The perform funcion increments the reference count for the session.
 * - The callback function decrements the reference count for the session.
 * - The Remove function checks the reference count to ensure that it is 0.
 *
 * @lld_dependencies
 * - \ref LacMem "Memory" - Inline memory functions
 * - Osal: logging, locking & virt to phys translations.
 * 
 * @lld_initialisation
 *
 * @lld_module_algorithms
 *
 * @lld_process_context
 *  
 * @lld_end
 * 
 *****************************************************************************/


/***************************************************************************/

#ifndef LAC_SYM_SESSION_H
#define LAC_SYM_SESSION_H


/* 
******************************************************************************
* Include public/global header files 
******************************************************************************
*/ 


#include "cpa.h"
#include "IxOsal.h"
#include "lac_sym_cipher_defs.h"
#include "lac_sym_qat.h"
#include "lac_sym_qat_hash.h" 

/* 
*******************************************************************************
* Include private header files 
*******************************************************************************
*/ 

/**
*******************************************************************************
 * @ingroup LacSym_Session
 *      Symmetric session descriptor
 * @description
 *      Maximum number of QAT slices supported for one request. Choice of size
 *      intended to be future proof (current maximum requirement is 3 for 
 *      Algorithm-Chaining, including the MemOut slice). 
 *****************************************************************************/
#define MAX_NUM_QAT_SLICES  ICP_QAT_FW_SLICE_DELIMITER


/**
*******************************************************************************
 * @ingroup LacSym_Session
 *      Symmetric session descriptor
 * @description
 *      This structure stores information about a session   
 *****************************************************************************/
typedef struct lac_session_desc_s
{
    Cpa8U contentDescriptor[LAC_SYM_QAT_CONTENT_DESC_MAX_SIZE];
    /**< QAT Content Descriptor for this session. 
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    Cpa8U hashStatePrefixBuffer[LAC_MAX_HASH_STATE_BUFFER_SIZE_BYTES];
    /**< hash state prefix buffer used for hash operations 
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    Cpa8U cipherPartialOpState[LAC_CIPHER_STATE_SIZE_MAX];
    /**< Buffer to hold the cipher state for the session (for partial ops). 
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    Cpa8U cipherARC4InitialState[LAC_CIPHER_ARC4_STATE_LEN_BYTES];
    /**< Buffer to hold the initial ARC4 cipher state for the session, which 
     * is derived from the user-supplied base key during session registration.
     * NOTE: Field must be correctly aligned in memory for access by QAT engine
     */
    CpaCySymOp symOperation;
    /**< type of command to be performed */
    CpaBoolean isAuthEncryptOp;
    /**< if the algorithm chaining operation is auth encrypt */
    qat_comms_priority_t qatSessionPriority;
    /**< session priority */
    lac_sym_qat_content_desc_info_t contentDescInfo;
    /**< info on the content descriptor */
    lac_sym_qat_hash_state_buffer_info_t hashStateBufferInfo;
    /**< info on the hash state prefix buffer */
    CpaCySymHashAlgorithm hashAlgorithm;
    /**< hash algorithm */
    CpaCySymHashMode hashMode;
    /**< Mode of the hash operation. plain, auth or nested */
    Cpa32U hashResultSize;
    /**< size of the digest produced/compared in bytes */
    CpaCySymCipherAlgorithm cipherAlgorithm;
    /**< Cipher algorithm and mode */
    Cpa32U cipherKeyLenInBytes;
    /**< Cipher key length in bytes */
    CpaCySymCipherDirection cipherDirection;
    /**< This parameter determines if the cipher operation is an encrypt or
     * a decrypt operation. */
    Cpa32U aadLenInBytes;
    /**< For CCM/GCM case, this parameter holds the AAD size, otherwise it
     *  is set to zero */
    icp_qat_fw_slice_t qatSlices[MAX_NUM_QAT_SLICES];
    /**< Ordered list of QAT slices that are to be invoked */
    CpaCySymPacketType partialState;
    /**< state of the partial packet. This can be written to by the perform
     * because the SpinLock pPartialInFlightSpinlock guarantees that that the 
     * state is accessible in only one place at a time. */
    icp_arch_if_req_hdr_t reqHdr;
    /* Request header that is the same for every request for LAC */
    CpaCySymCbFunc pSymCb;
    /**< symmetric function callback pointer */
    IxOsalAtomic pendingCbCount;     
    /**< Keeps track of number of pending requests.  */
    struct lac_sym_bulk_cookie_s *pRequestQueueHead;
    /**< A fifo list of queued QAT requests. Head points to first queue entry */
    struct lac_sym_bulk_cookie_s *pRequestQueueTail;
    /**< A fifo list of queued QAT requests. Tail points to last queue entry */
    lac_lock_t requestQueueLock;
    /**< A lock to protect accesses to the above request queue  */
    CpaBoolean nonBlockingOpsInProgress;
    /**< Flag is set if a non blocking operation is in progress for a session. 
     * If set to false, new requests will be queued until the condition is 
     * cleared.
     * ASSUMPTION: Only one blocking condition per session can exist at any time
     */
    CpaBoolean validSessionDesc;
    /**< Flag which is set iff the session is valid. */
} lac_session_desc_t;

#define LAC_SYM_SESSION_SIZE \
    (sizeof(lac_session_desc_t) + LAC_64BYTE_ALIGNMENT + sizeof(unsigned long))
/**< @ingroup LacSym_Session
 * Size of the memory that the client has to allocate for a session. Extra 
 * memory is needed to internally re-align the data. The pointer to the algined 
 * data is stored at the start of the user allocated memory hence the extra 
 * space for an unsigned long */ 

#define LAC_SYM_SESSION_DESC_FROM_CTX_GET(pSession) \
    (lac_session_desc_t *) (*(unsigned long *)pSession)
/**< @ingroup LacSym_Session
 * Retrieve the session descriptor pointer from the session context structure
 * that the user allocates. The pointer to the internally realigned address 
 * is stored at the start of the session context that the user allocates */

#endif /* LAC_SYM_SESSION_H */
