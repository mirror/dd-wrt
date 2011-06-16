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
 * @file lac_sym_alg_chain.h
 *
 * @defgroup  LacAlgChain  Algorithm Chaining
 *
 * @ingroup LacSym
 *
 * Interfaces exposed by the Algorithm Chaining Component
 * 
 * @lld_start
 *
 * @lld_overview
 * This is the LAC Algorithm-Chaining feature component.  This component
 * implements session registration and cleanup functions, and a perform
 * function.  Statistics are maintained to track requests issued and completed,
 * errors incurred, and  authentication verification failures.  For each
 * function the parameters  supplied by the client are checked, and then the
 * function proceeds if all the parameters are valid.  This component also 
 * incorporates support for Authenticated-Encryption (CCM and GCM) which
 * essentially comprises of a cipher operation and a hash operation combined.
 *
 * This component can combine a cipher operation with a hash operation or just
 * simply create a hash only or cipher only operation and is called from the 
 * LAC Symmetric API component. In turn it calls the LAC Cipher, LAC Hash, and
 * LAC Symmetric QAT components.  The goal here is to duplicate as little code
 * as possible from the Cipher and Hash components.
 *
 * The cipher and hash operations can be combined in either order, i.e. cipher
 * first then hash or hash first then cipher.  The client specifies this via
 * the algChainOrder field in the session context.  This ordering choice is
 * stored as part of the session descriptor, so that it is known when a
 * perform request is issued.  In the case of Authenticated-Encryption, the
 * ordering is an implicit part of the CCM or GCM protocol.
 *
 * When building a content descriptor, as part of session registration, this
 * component asks the Cipher and Hash components to build their respective
 * parts of the session descriptor.  The key aspect here is to provide the
 * correct offsets to the Cipher and Hash components for where in the content
 * descriptor to write their Config and Hardware Setup blocks.  Also the
 * Config block in each case must specify the appropriate next slice.
 *
 * When building request parameters, as part of a perform operation, this
 * component asks the Cipher and Hash components to build their respective
 * parts of the request parameters block.  Again the key aspect here is to
 * provide the correct offsets to the Cipher and Hash components for where in
 * the request parameters block to write their parameters.  Also the request
 * parameters block in each case must specify the appropriate next slice.
 *
 * Parameter checking for session registration and for operation perform is
 * mostly delegated to the Cipher and Hash components.  There are a few
 * extra checks that this component must perform: check the algChainOrder
 * parameter, ensure that CCM/GCM are specified for hash/cipher algorithms
 * as appropriate, and ensure that requests are for full packets (partial
 * packets are not supported for Algorithm-Chaining).
 *
 * The perform operation allocates a cookie to capture information required
 * in the request callback.  This cookie is then freed in the callback.
 *
 * @lld_dependencies
 * - \ref LacCipher "Cipher" : For taking care of the cipher aspects of
 *   session registration and operation perform
 * - \ref LacHash "Hash" : For taking care of the hash aspects of session
 *   registration and operation perform
 * - \ref LacSymCommon "Symmetric Common" : statistics.
 * - \ref LacSymQat "Symmetric QAT": To build the QAT request message,
 *   request param structure, and populate the content descriptor.  Also
 *   for registering a callback function to process the QAT response.
 * - \ref QatComms "QAT Comms" : For sending messages to the QAT, and for
 *   setting the response callback
 * - \ref LacMem "Mem" : For memory allocation and freeing, virtual/physical
 *   address translation, and translating between scalar and pointer types
 * - OSAL : For atomics and locking
 * 
 * @lld_module_algorithms
 * This component builds up a chain of slices at session init time 
 * and stores it in the session descriptor. This is used for building up the
 * content descriptor at session init time and the request parameters structure
 * in the perform operation. 
 * 
 * The offsets for the first slice are updated so that the second slice adds 
 * its configuration information after that of the first slice. The first
 * slice also configures the next slice appropriately.
 *
 * This component is very much hard-coded to just support cipher+hash or
 * hash+cipher.  It should be quite possible to extend this idea to support
 * an arbitrary chain of commands, by building up a command chain that can
 * be traversed in order to build up the appropriate configuration for the
 * QAT.  This notion should be looked at in the future if other forms of
 * Algorithm-Chaining are desired.
 *
 * @lld_process_context 
 * 
 * @lld_end
 *
 *****************************************************************************/

/*****************************************************************************/

#ifndef LAC_SYM_ALG_CHAIN_H
#define LAC_SYM_ALG_CHAIN_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "lac_session.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/


/**
*******************************************************************************
 * @ingroup LacAlgChain
 *      This function registers a session for an Algorithm-Chaining operation.
 *
 * @description
 *      This function is called from the LAC session register API function for
 *      Algorithm-Chaining operations. It validates all input parameters. If
 *      an invalid parameter is passed, an error is returned to the calling
 *      function. If all parameters are valid an Algorithm-Chaining session is
 *      registered.
 *
 * @param[in] instanceHandle    Instance Handle
 *
 * @param[in] pSessionCtx       Pointer to session context which contains
 *                              parameters which are static for a given
 *                              cryptographic session such as operation type,
 *                              mechanisms, and keys for cipher and/or digest
 *                              operations.
 * @param[out] pSessionDesc     Pointer to session descriptor
 * 
 * @retval CPA_STATUS_SUCCESS       Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resources.
 *
 * @see cpaCySymInitSession()
 *
 *****************************************************************************/
CpaStatus
LacAlgChain_SessionInit(
    const CpaInstanceHandle instanceHandle,
    const CpaCySymSessionSetupData *pSessionCtx,
    lac_session_desc_t *pSessionDesc);
 


/**
*******************************************************************************
 * @ingroup LacAlgChain
 *      Data path function for the Algorithm-Chaining component
 *
 * @description
 *      This function gets called from cpaCySymPerformOp() which is the
 *      symmetric LAC API function. It is the data path function for the
 *      Algorithm-Chaining component. It does the parameter checking on the
 *      client supplied parameters and if the parameters are valid, the
 *      operation is performed and a request sent to the QAT, otherwise an
 *      error is returned to the client.
 *
 * @param[in] instanceHandle    Instance Handle
 * 
 * @param[in] pSessionDesc  Pointer to session descriptor
 * @param[in] pCallbackTag    The application's context for this call
 * @param[in] pOpData       Pointer to a structure containing request
 *                          parameters. The client code allocates the memory for
 *                          this structure. This component takes ownership of
 *                          the memory until it is returned in the callback.
 *
 * @param[in] pSrcBuffer        Source Buffer List 
 * @param[out] pDstBuffer       Destination Buffer List 
 * @param[out] pVerifyResult    Verify Result  
 *
 * @retval CPA_STATUS_SUCCESS        Function executed successfully.
 * @retval CPA_STATUS_FAIL           Function failed.
 * @retval CPA_STATUS_INVALID_PARAM  Invalid parameter passed in.
 * @retval CPA_STATUS_RESOURCE       Error related to system resource.
 *
 * @see cpaCySymPerformOp()
 *
 *****************************************************************************/
CpaStatus
LacAlgChain_Perform(
    const CpaInstanceHandle instanceHandle,
    lac_session_desc_t *pSessionDesc,
    void *pCallbackTag,
    const CpaCySymOpData *pOpData,
    const CpaBufferList *pSrcBuffer,
    CpaBufferList *pDstBuffer,
    CpaBoolean *pVerifyResult
    );

#endif /* LAC_SYM_ALG_CHAIN_H */

