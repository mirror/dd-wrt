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
 * @defgroup LacAsym Asymmetric
 *
 * @ingroup Lac
 *
 * Asymmetric component includes Diffie Hellman, Rsa, Dsa, Random and Prime.
 **************************************************************************/

/**
 ***************************************************************************
 * @defgroup LacAsymCommon Asymmetric Common
 *
 * @ingroup LacAsym
 *
 * Asymmetric common includes pke utils, mmp and qat communication layer
 **************************************************************************/

/**
 ***************************************************************************
 * @file lac_pke_qat_comms.h
 *
 * @defgroup LacAsymCommonQatComms QAT Communication Layer
 *
 * @ingroup LacAsymCommon
 *
 * Asymmetric QAT Communication Layer
 *
 * @lld_start
 *
 * @lld_overview
 * This is the LAC PKE QAT Comms component.  It takes care of the creation
 * of PKE messages in the format the QAT expects, and also the sending of
 * these messages to the QAT.  As part of PKE message creation, flat buffers
 * are internally aligned by this component if necessary.  Also PKE request
 * chaining, where multiple requests are linked together and just the head
 * request is sent to the QAT, is supported by this component.  This component
 * also takes care of the allocation and freeing of request data structures
 * to minimize the work required by calling code.
 *
 * The expected usage is that clients will create input/output parameter
 * lists, with flat buffer pointers stored in the correct order and location
 * within these lists.  They will then call the function to create and send
 * a PKE request to the QAT.  If the client wishes to chain requests, then
 * they will create multiple requests using the same handle, and then send
 * the request chain as a normal request.
 *
 * The clients call the asynchronous function to send the message to the QAT.
 * They pass in a callback function and callback data when creating the
 * message. When the response is received from the QAT the callback function
 * is invoked with the LAC status, the QAT pass/fail status, and the callback
 * data as params.
 *
 * In the case of request chaining, the QAT will abort the execution of
 * requests in the chain if any request fails.  The response message will
 * correspond to the last executed request in the chain.  As each request
 * has its own (potentially unique) callback data, a client could in theory
 * determine which request in the chain failed if this info is needed.
 *
 * @lld_dependencies
 * - \ref QatComms "QAT Comms" : For sending messages to the QAT, and for
 *   setting the response callback
 * - \ref LacMem "Mem" : For memory allocation and freeing, virtual/physical
 *   address translation, and translating between scalar and pointer types
 *
 * @lld_initialisation
 * On initialization this component sets the response callback for messages
 * of type PKE, so that they can be handled by this component.
 *
 * @lld_module_algorithms
 * To support request chaining, this component builds up a linked list of
 * requests, and maintains the head of the list in the handle that gets
 * returned to the caller.  Each element of the list has a pointer to the
 * head of the list so that the entire list can be freed by knowing any
 * element of the list.  Each element of the list maintains a next pointer
 * to facilitate list traversal.  Only the head element of the list maintains
 * a tail pointer, to facilitate adding to the end of the list.  The head
 * element is static, so can readily be stored in each element, but as the
 * tail element is dynamic (it changes as the list grows) its not convenient
 * to store and maintain it in each element.  Instead to get the tail of the
 * list you must first get the head of the list to get the tail pointer.
 *
 * @lld_process_context
 *
 * @lld_end
 ****************************************************************************/

/******************************************************************************/

#ifndef _LAC_PKE_QAT_COMMS_H_
#define _LAC_PKE_QAT_COMMS_H_

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

#include "cpa.h"
#include "cpa_types.h"
#include "cpa_cy_common.h"
#include "lac_pke_mmp.h"
#include "lac_sync.h"

#include "icp_qat_fw_pke.h"
#include "icp_qat_fw_mmp.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/

/**
 *****************************************************************************
 * @ingroup LacPkeQatComms
 *
 * @description
 *      PKE request callback data structure
 *
 *****************************************************************************/
typedef struct lac_pke_op_cb_data_s
{
    const void *pClientCb; 
    /**< client callback function pointer */
    void *pCallbackTag; 
    /**< client callback correlator */
    const void *pClientOpData; 
    /**< client callback operation data pointer */
    void *pOpaqueData; 
    /**< generic opaque data pointer */
    /* Output data */
    void *pOutputData1; 
    /**< Output data pointer 1 */
    void *pOutputData2; 
    /**< Output data pointer 2 */
    lac_sync_op_data_t *pSyncCookie;
    /**< synchronous cookie */
} lac_pke_op_cb_data_t;


/**
 *****************************************************************************
 * @ingroup LacPkeQatComms
 *
 * @description
 * This is the callback prototype for the non-blocking PKE operations. It
 * takes a status, pass flag, acceleration handle, and callback data pointer
 * as parameters and returns void. This function will be invoked when a
 * PKE response is received from the QAT to a previously issued PKE request.
 *
 * @param[in] status            status of the operation
 * @param[in] pass              result of the operation. For operations such as
 *                              verify which require the QAT to perform a test
 *                              then this parameter shall be set to CPA_TRUE if
 *                              the test succeeds or CPA_FALSE if the test
 *                              fails.
 *                              For messages which do not perform a test e.g.
 *                              encrypt, decrypt then this parameter shall
 *                              always be CPA_TRUE.
 * @param[in] instanceHandle    Acceleration engine to which the message was
 *                              sent.
 * @param[in] pCbData           this field contains a copy of the callback data 
 *                              passed when the request was created.
 *****************************************************************************/
typedef void (*lac_pke_op_cb_func_t)(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData);


typedef void *lac_pke_request_handle_t;
/**< ingroup LacPkeQatComms
 * Handle to a PKE request. The handle is created using  LacPke_CreateRequest()
 * and is subsequently used to send the request using LacPke_SendRequest(). */


#define LAC_PKE_INVALID_HANDLE ((lac_pke_request_handle_t)0)
/**< @ingroup LacPkeQatComms
 * Invalid PKE request handle. */

/**
 *******************************************************************************
 * @ingroup LacPkeQatComms
 *      Inits the PKE QAT interface component.
 *
 * @description
 *      Initialise this component and register callback with the QAT comms
 * module. Create the mem pool for pke requests.
 *
 * @retval CPA_STATUS_SUCCESS   No error
 * @retval CPA_STATUS_FAIL      General error
 *
 *****************************************************************************/
CpaStatus
LacPke_CommsInit(Cpa64U numAsymConcurrentReq);

/**
 *******************************************************************************
 * @ingroup LacPkeQatComms
 *      Shuts down the PKE QAT interface component.
 *
 * @description
 *      Destroy the mem pool for pke requests.
 *****************************************************************************/
void
LacPke_CommsShutdown(void);

/**
 *******************************************************************************
 * @ingroup LacPkeQatComms
 *      Creates a PKE request for the QAT.
 *
 * @description
 *      This function takes the parameters for a PKE QAT request, creates the
 * request, aligns the input & output buffer parameters, and fills in the PKE
 * fields.  The request can subsequently be sent to the QAT using
 * LacPke_SendRequest(). In the event of an error this function will tidy up
 * any resources associated with the request handle and set it to
 * PKE_INVALID_HANDLE.
 *
 * @param[in,out] pRequestHandle    Pointer to hold the handle for the request
 *                                  created by this call.  If the incoming value
 *                                  is non-zero then the new request is appended
 *                                  to the request (chain) already associated
 *                                  with the handle.  For a single request, or
 *                                  the first request in a chain, the passed in
 *                                  handle value must be zero
 *                                  (i.e. PKE_INVALID_HANDLE).
 *
 * @param[in] functionalityId   the PKE functionality id.
 * @param[in] pInArgList        pointer to the list of input params. This 
 *                              should contain the client-provided flat buffer
 *                              pointers. Any entries in the list which are not
 *                              used must be set to 0.
 * @param[in] pOutArgList       pointer to the list of output params. This
 *                              should contain the client-provided flat buffer
 *                              pointers. Any entries in the list which are not
 *                              used must be set to 0.
 * @param[in] pPkeOpCbFunc      this function is invoked when the response is
 *                              received from the QAT
 * @param[in] pCbData           callback data to be returned (by copy) 
 *                              unchanged in the callback.
 *
 * @retval CPA_STATUS_SUCCESS   No error
 * @retval CPA_STATUS_RESOURCE  Resource error (e.g. failed memory allocation)
 *
 * @pre @ref LacPke_CommsInit() should have been called already.
 ******************************************************************************/
CpaStatus
LacPke_CreateRequest(
    lac_pke_request_handle_t *pRequestHandle,
    Cpa32U functionalityId,
    Cpa32U *pInArgSizeList,
    Cpa32U *pOutArgSizeList,
    icp_qat_fw_mmp_input_param_t *pInArgList,
    icp_qat_fw_mmp_output_param_t *pOutArgList,
    lac_pke_op_cb_func_t pPkeOpCbFunc,
    lac_pke_op_cb_data_t *pCbData);

/**
 *******************************************************************************
 * @ingroup LacPkeQatComms
 *      Sends a PKE request to the QAT.
 *
 * @description
 *      This function sends a PKE request, previously created using
 * LacPke_CreateRequest(), to the QAT. It does not block waiting for a
 * response. Instead the callback function is invoked when the response from
 * the QAT has been processed.
 *
 * @param[in,out] pRequestHandle    the handle of the PKE request (chain) to be
 *                                  sent.  Will be set to CPA_INVALID_HANDLE in
 *                                  the case of any error.
 * @param[in] instanceHandle        Acceleration engine to which the message
 *                                  will be sent.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_RESOURCE      Resource error (e.g. failed memory
 *                                  allocation)
 *
 * @pre @ref LacPke_CommsInit() should have been called already.
 ******************************************************************************/
CpaStatus
LacPke_SendRequest(
    lac_pke_request_handle_t *pRequestHandle,
    CpaInstanceHandle instanceHandle);

/**
 *******************************************************************************
 * @ingroup LacPkeQatComms
 *      Sends a single (unchained) PKE request to the QAT.
 *
 * @description
 *      This function takes the parameters for a PKE QAT request, creates the
 * request, fills in the PKE fields and sends it to the QAT. It does not block
 * waiting for a response. Instead the callback function is invoked when the
 * response from the QAT has been processed.
 *
 * @param[in] functionalityId   the PKE functionality id.
 * @param[in] pInArgList        pointer to the list of input params. This should
 *                              contain the client-provided flat buffer
 *                              pointers. Any entries in the list which are not
 *                              used must be set to 0.
 * @param[in] pOutArgList]      pointer to the list of output params. This
 *                              should contain the client-provided flat buffer
 *                              pointers. Any entries in the list which are not
 *                              used must be set to 0.
 * @param[in] pPkeOpCbFunc]     this function is invoked when the response is
 *                              received from the QAT
 * @param[in] pCbData           callback data to be returned (by copy)
 *                              unchanged in the callback.
 * @param[in] instanceHandle    Acceleration engine to which the message will
 *                              be sent.
 *
 * @retval CPA_STATUS_SUCCESS   No error
 * @retval CPA_STATUS_RESOURCE  Resource error (e.g. failed memory allocation)
 *
 * @pre @ref LacPke_CommsInit() should have been called already.
 ******************************************************************************/
CpaStatus
LacPke_SendSingleRequest(
    Cpa32U functionalityId,
    Cpa32U *pInArgSizeList,
    Cpa32U *pOutArgSizeList,
    icp_qat_fw_mmp_input_param_t *pInArgList,
    icp_qat_fw_mmp_output_param_t *pOutArgList,
    lac_pke_op_cb_func_t pPkeOpCbFunc,
    lac_pke_op_cb_data_t *pCbData,
    CpaInstanceHandle instanceHandle);

#endif /* _LAC_PKE_QAT_COMMS_H_ */

