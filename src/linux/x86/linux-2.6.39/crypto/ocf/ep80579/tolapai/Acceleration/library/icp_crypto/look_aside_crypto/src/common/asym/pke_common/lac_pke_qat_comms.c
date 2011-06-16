/*
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
 */

/**
 ***************************************************************************
 * @file lac_pke_qat_comms.c
 *
 * @ingroup LacAsymCommonQatComms
 *
 * This file implements the API for creating PKE QAT messages and sending
 * these to the QAT. It implements an API for creating a PKE QAT request, and
 * for sending a PKE message to the QAT.
 *
 ***************************************************************************/

/*
****************************************************************************
* Include public/global header files
****************************************************************************
*/

#include "cpa.h"

#include "icp_qat_fw_pke.h"
#include "icp_qat_fw_mmp.h"
#include "qat_comms.h"

#include "lac_common.h"
#include "lac_mem.h"
#include "lac_mem_pools.h"

#include "lac_pke_qat_comms.h"
#include "lac_pke_mmp.h"

/*
****************************************************************************
* Include private header files
****************************************************************************
*/

/*
****************************************************************************
* Static Variables
****************************************************************************
*/
/* Contains the data for a pke op callback */
typedef struct lac_pke_qat_req_data_cb_info_s
{
    lac_pke_op_cb_func_t cbFunc; /**< Callback function */
    lac_pke_op_cb_data_t cbData; /**< Callback function data */
    CpaInstanceHandle instanceHandle; /**< Acceleration engine for request */
} lac_pke_qat_req_data_cb_info_t;

/* Contains the input and output buffer data */
typedef struct lac_pke_qat_req_data_param_info_s
{
    CpaFlatBuffer *clientInputParams[LAC_MAX_MMP_INPUT_PARAMS];
        /**< the client input parameters (unaligned flat buffers) */
    CpaFlatBuffer *clientOutputParams[LAC_MAX_MMP_OUTPUT_PARAMS];
        /**< the client output parameters (unaligned flat buffers) */

    Cpa8U *pkeInputParams[LAC_MAX_MMP_INPUT_PARAMS];
        /* the PKE input parameters (aligned data pointers) */
    Cpa8U *pkeOutputParams[LAC_MAX_MMP_OUTPUT_PARAMS];
        /* the PKE output parameters (aligned data pointers) */
        
    Cpa32U inArgSizeList[LAC_MAX_MMP_INPUT_PARAMS];
    /* Array of input arguments sizes */
    Cpa32U outArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS];
    /* Array of output arguments sizes */

} lac_pke_qat_req_data_param_info_t;

/* Identifier for pool of aligned buffers */
static lac_memory_pool_id_t lac_pke_req_pool = LAC_MEM_POOL_INIT_POOL_ID;

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      Request data for QAT messages
 *
 * @description
 *      This structure defines the request data for PKE QAT messages. This is
 * used to store data which is known when the message is sent and which we wish
 * to retrieve when the response message is processed.
 **************************************************************************/
typedef struct lac_pke_qat_req_data_s {

    /* use union to ensure optimal alignment */
    union lac_pke_qat_req_data_request_u
    {
        icp_qat_fw_pke_request_t request; /**< the PKE request */
        Cpa8U padding[1 << LAC_OPTIMAL_ALIGNMENT_SHIFT];
    } u1;

    /* use union to ensure optimal alignment */
    union lac_pke_qat_req_data_in_args_u
    {
        icp_qat_fw_mmp_input_param_t inArgList; /**< msg input arg list */
        Cpa8U padding[1 << LAC_OPTIMAL_ALIGNMENT_SHIFT];
    } u2;

    /* use union to ensure optimal alignment */
    union lac_pke_qat_req_data_out_args_u
    {
        icp_qat_fw_mmp_output_param_t outArgList; /**< msg output arg list */
        Cpa8U padding[1 << LAC_OPTIMAL_ALIGNMENT_SHIFT];
    } u3;

    lac_pke_qat_req_data_cb_info_t cbInfo; /**< Callback info */
    lac_pke_qat_req_data_param_info_t paramInfo; /**< Parameter info */

    struct lac_pke_qat_req_data_s *pNextReqData; /**< next req data ptr */
    struct lac_pke_qat_req_data_s *pHeadReqData; /**< head req data ptr */
    struct lac_pke_qat_req_data_s *pTailReqData; /**< tail req data ptr */
    

} lac_pke_qat_req_data_t;


/*
****************************************************************************
* Define static function definitions
****************************************************************************
*/

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      Aligns parameters for a PKE request
 *
 * @description
 *      This function aligns the flat buffer parameters for a PKE request, by
 * calling LacMem_BufferAlign for each input/output flat buffer parameter in
 * the request data structure.  LacPke_RestoreParams is the corresponding
 * function for undoing the buffer alignment.
 *
 * @param pParamInfo        IN  The data pointers of the flat buffers from the
 *                              clientInputParams and clientOutputParams
 *                              arrays are aligned and stored in the
 *                              pkeInputParams and pkeOutputParams arrays
 *                              respectively.  The client...Params arrays are
 *                              processed one-by-one from the start, and
 *                              processing ends once a NULL parameter is
 *                              encountered.  Consequently, the pke...Params
 *                              arrays should be initialized to zero as NULL
 *                              inputs won't be written as NULL outputs.
 *
 *
 * @retval CPA_STATUS_SUCCESS     No error
 * @retval CPA_STATUS_RESOURCE    Resource error (e.g. failed memory allocation)
 *
 * @see LacPke_RestoreParams()
 * @see LacMem_BufferAlign()
 ***************************************************************************/
STATIC
CpaStatus
LacPke_AlignParams(struct lac_pke_qat_req_data_param_info_s *pParamInfo)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U i = 0;

    /* align input parameter flat buffers (end if NULL encountered) */
    for (i = 0; (i < LAC_MAX_MMP_INPUT_PARAMS) &&
        (NULL != pParamInfo->clientInputParams[i]); i++)
    {
        /* align buffer, and round length up to whole quadwords */
        /* pad left with zeroes when aligning an input buffer */        
        Cpa32U dataLen = pParamInfo->clientInputParams[i]->dataLenInBytes;
        Cpa32U dataRoundLen = dataLen;
        Cpa32U offset = 0;
        if(pParamInfo->inArgSizeList[i])
        {
            dataRoundLen = pParamInfo->inArgSizeList[i];
            offset = (dataLen > pParamInfo->inArgSizeList[i]) ?
            (dataLen - pParamInfo->inArgSizeList[i]) : 0;
        }
        pParamInfo->pkeInputParams[i] = LacMem_BufferAlign(
            pParamInfo->clientInputParams[i]->pData + offset,
            dataLen - offset,
            LAC_ALIGN_POW2_ROUNDUP(
                dataRoundLen,
                LAC_QUAD_WORD_IN_BYTES),
                ICP_LAC_MEM_PAD_LEFT | ICP_LAC_MEM_PAD_ZEROES);
        status =
            (!pParamInfo->pkeInputParams[i] ? CPA_STATUS_RESOURCE : status);
        LAC_CHECK_STATUS(status);
    }

    /* align output parameter flat buffers (end if NULL encountered) */
    for (i = 0; (i < LAC_MAX_MMP_OUTPUT_PARAMS) &&
        (NULL != pParamInfo->clientOutputParams[i]); i++)
    {
        /* align buffer, and round length up to whole quadwords */
        /* don't copy when aligning an output buffer */
        Cpa32U dataLen = pParamInfo->clientOutputParams[i]->dataLenInBytes;
        Cpa32U dataRoundLen = dataLen;
        Cpa32U offset = 0;
        if(pParamInfo->outArgSizeList[i])
        {
            dataRoundLen = pParamInfo->outArgSizeList[i];
            offset = (dataLen > pParamInfo->outArgSizeList[i]) ?
            (dataLen - pParamInfo->outArgSizeList[i]) : 0;
        }
        pParamInfo->pkeOutputParams[i] = LacMem_BufferAlign(
            pParamInfo->clientOutputParams[i]->pData + offset,
            dataLen - offset,
            LAC_ALIGN_POW2_ROUNDUP(
                dataRoundLen,
                LAC_QUAD_WORD_IN_BYTES),
                ICP_LAC_MEM_NO_COPY);

        status =
            (!pParamInfo->pkeOutputParams[i] ? CPA_STATUS_RESOURCE : status);
        LAC_CHECK_STATUS(status);
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      Restores parameters for a PKE request
 *
 * @description
 *      This function restores the flat buffer parameters for a PKE request, by
 * calling LacMem_BufferRestore for each input/output flat buffer parameter in
 * the request data structure.  LacPke_AlignParams is the corresponding
 * function for doing the buffer alignment.
 *
 * @param pParamInfo        IN  The data pointers from the pkeInputParams and
 *                              pkeOutputParams arrays are restored and stored
 *                              in the data pointers of the flat buffers in
 *                              the clientInputParams and clientOutputParams
 *                              arrays respectively.  The pke...Params arrays
 *                              are processed one-by-one from the start, and
 *                              processing ends once a NULL parameter is
 *                              encountered.  Consequently, the
 *                              client...Params arrays should be initialized
 *                              to zero as NULL inputs won't be written as
 *                              NULL outputs.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_RESOURCE       Resource error (e.g. failed memory free)
 *
 * @see LacPke_AlignParams()
 * @see LacMem_BufferRestore()
 ***************************************************************************/
STATIC
CpaStatus
LacPke_RestoreParams(struct lac_pke_qat_req_data_param_info_s *pParamInfo)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U i = 0;

    /* restore input parameter flat buffers (end if NULL encountered) */
    for (i = 0; (i < LAC_MAX_MMP_INPUT_PARAMS) &&
        (NULL != pParamInfo->pkeInputParams[i]); i++)
    {
        /* restore buffer */
        /* don't copy when restoring an input buffer */
        Cpa32U dataLen = pParamInfo->clientInputParams[i]->dataLenInBytes;
        Cpa32U dataRoundLen = dataLen;
        Cpa32U offset = 0;
        if(pParamInfo->inArgSizeList[i])
        {
            dataRoundLen = pParamInfo->inArgSizeList[i];
            offset = (dataLen > pParamInfo->inArgSizeList[i]) ?
            (dataLen - pParamInfo->inArgSizeList[i]) : 0;
        }
        status = LacMem_BufferRestore(
            pParamInfo->clientInputParams[i]->pData + offset,
            dataLen - offset,
            pParamInfo->pkeInputParams[i],
            LAC_ALIGN_POW2_ROUNDUP(
                dataRoundLen,
                LAC_QUAD_WORD_IN_BYTES),
                ICP_LAC_MEM_NO_COPY);
        LAC_CHECK_STATUS(status);
    }

    /* restore output parameter flat buffers (end if NULL encountered) */
    for (i = 0; (i < LAC_MAX_MMP_OUTPUT_PARAMS) &&
        (NULL != pParamInfo->pkeOutputParams[i]); i++)
    {
        /* restore buffer */
        /* consider buffer left padded when restoring an output buffer */
        Cpa32U dataLen = pParamInfo->clientOutputParams[i]->dataLenInBytes;
        Cpa32U dataRoundLen = dataLen;
        Cpa32U offset = 0;
        if(pParamInfo->outArgSizeList[i])
        {
            dataRoundLen = pParamInfo->outArgSizeList[i];
            offset = (dataLen > pParamInfo->outArgSizeList[i]) ?
            (dataLen - pParamInfo->outArgSizeList[i]) : 0;
        }
        status = LacMem_BufferRestore(
            pParamInfo->clientOutputParams[i]->pData + offset,
            dataLen - offset,
            pParamInfo->pkeOutputParams[i],
            LAC_ALIGN_POW2_ROUNDUP(
                dataRoundLen,
                LAC_QUAD_WORD_IN_BYTES),
                ICP_LAC_MEM_PAD_LEFT);
        LAC_CHECK_STATUS(status);
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      Destroys a PKE request
 *
 * @description
 *      This function destroys a PKE request that was created using
 * LacPke_CreateRequest().  It should be called if an error occurs during
 * request create or request send, or else as part of the response callback.
 *
 * @param pRequestHandle    IN  Pointer to the request handle that identifies
 *                              the request to be destroyed.  The request
 *                              handle will get set to LAC_PKE_INVALID_HANDLE.
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_RESOURCE       Resource error (e.g. failed memory free)
 *
 * @see LacPke_CreateRequest()
 ***************************************************************************/
STATIC
CpaStatus
LacPke_DestroyRequest(lac_pke_request_handle_t *pRequestHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_pke_qat_req_data_t *pReqData = NULL;

    /* extract head request data pointer from the request handle */
    pReqData = *pRequestHandle;

    /* invalidate the request handle */
    *pRequestHandle = LAC_PKE_INVALID_HANDLE;

    /* free all request data structures in the chain - continue even in
       the case of errors */
    while (NULL != pReqData)
    {
        lac_pke_qat_req_data_t *pNextReqData =
            pReqData->pNextReqData;

        /* restore parameters (i.e. undo alignment) */
        if (CPA_STATUS_SUCCESS != LacPke_RestoreParams(&pReqData->paramInfo))
        {
            status = CPA_STATUS_RESOURCE;
        }

        Lac_MemPoolEntryFree(pReqData);
        pReqData = pNextReqData;
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      Handles a PKE response
 *
 * @description
 *      This function is called by the QAT comms module when a response is
 * received from the QAT to a PKE request. The pPkeRespMsg is the message read
 * off the ring and the msgType is ICP_ARCH_IF_REQ_QAT_FW_PKE. This function
 * is registered to handle QAT PKE responses in LacPke_CommsInit().
 *
 * @param pRespMsg          IN  pointer to the response message
 * @param msgType           IN  should be ICP_ARCH_IF_REQ_QAT_FW_PKE
 *
 * @retval CPA_STATUS_SUCCESS       No error
 * @retval CPA_STATUS_RESOURCE      Resource error (e.g. failed memory free)
 *
 * @see LacPke_CommsInit()
 ***************************************************************************/
STATIC
void
LacPke_MsgCallback(void *pRespMsg, icp_arch_if_request_t msgType)
{
    icp_qat_fw_pke_response_t *pPkeRespMsg = NULL;
    lac_pke_request_handle_t requestHandle = LAC_PKE_INVALID_HANDLE;
    CpaStatus status = CPA_STATUS_SUCCESS;
    CpaBoolean pass = CPA_TRUE;
    lac_pke_qat_req_data_t *pReqData = NULL;
    lac_pke_op_cb_func_t pCbFunc = NULL;
    lac_pke_op_cb_data_t cbData = {0};
    CpaInstanceHandle instanceHandle = CPA_INSTANCE_HANDLE_SINGLE;
    Cpa16U pkeFlags = 0;

    LAC_ASSERT((msgType == ICP_ARCH_IF_REQ_QAT_FW_PKE), "Bad msgType");

    /* cast response message to PKE response message type */

    pPkeRespMsg = (icp_qat_fw_pke_response_t *)pRespMsg;

    LAC_MEM_SHARED_READ(pPkeRespMsg->flags, pkeFlags);
    /* check QAT response status */
    pass = (ICP_QAT_FW_PKE_STATUS_CLEARED ==
        ICP_QAT_FW_PKE_STATUS_FLAG_GET(pkeFlags));

    /* extract request data pointer from the opaque data */
    LAC_MEM_SHARED_READ_TO_PTR(pPkeRespMsg->opaque_data, pReqData);
    LAC_ASSERT_NOT_NULL(pReqData);

    /* extract fields from request data struct */
    pCbFunc = pReqData->cbInfo.cbFunc;
    cbData = pReqData->cbInfo.cbData;
    instanceHandle = pReqData->cbInfo.instanceHandle;

    /* destroy the request */
    requestHandle = (lac_pke_request_handle_t)pReqData->pHeadReqData;
    status = LacPke_DestroyRequest(&requestHandle);

    /* call the client callback */
    LAC_ASSERT_NOT_NULL(pCbFunc);
    (*pCbFunc)(status, pass, instanceHandle, &cbData);
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      PKE QAT Comms initialization
 ***************************************************************************/
CpaStatus
LacPke_CommsInit(Cpa64U numAsymConcurrentReq)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    status = Lac_MemPoolCreate(&lac_pke_req_pool, "PKE REQUESTS",
        numAsymConcurrentReq, sizeof(lac_pke_qat_req_data_t),
        LAC_64BYTE_ALIGNMENT, CPA_FALSE);

    if (CPA_STATUS_SUCCESS == status)
    {
        status = QatComms_ResponseCbSet(LacPke_MsgCallback,
                                  ICP_ARCH_IF_REQ_QAT_FW_PKE);
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      PKE QAT Comms initialization
 ***************************************************************************/
void
LacPke_CommsShutdown(void)
{
    Lac_MemPoolDestroy(lac_pke_req_pool);
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      PKE request creation
 ***************************************************************************/
CpaStatus
LacPke_CreateRequest(
    lac_pke_request_handle_t *pRequestHandle,
    Cpa32U functionalityId,
    Cpa32U *pInArgSizeList,
    Cpa32U *pOutArgSizeList,
    icp_qat_fw_mmp_input_param_t *pInArgList,
    icp_qat_fw_mmp_output_param_t *pOutArgList,
    lac_pke_op_cb_func_t pPkeOpCbFunc,
    lac_pke_op_cb_data_t *pCbData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_pke_qat_req_data_t *pReqData = NULL;
    Cpa32U i = 0;

    /** @mem_usage : currently 324B is allocated here per request
        (assuming 32-bit pointers) */

    /** @performance : having a pool of pre-allocated requests would
        improve performance here */

    /* allocate request data */
    pReqData = Lac_MemPoolEntryAlloc(lac_pke_req_pool);
    if (NULL == pReqData)
    {
        LAC_LOG_ERROR("Cannot get a mem pool entry"); 
        status = CPA_STATUS_RESOURCE;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_ASSERT_NOT_NULL(pReqData);

        /* ensure correct request structure alignment */
        LAC_ASSERT(
            LAC_ADDRESS_ALIGNED(
                &pReqData->u1.request, LAC_OPTIMAL_ALIGNMENT_SHIFT),
            "request structure not correctly aligned");

        /* ensure correct input argument list structure alignment */
        LAC_ASSERT(
            LAC_ADDRESS_ALIGNED(
                &pReqData->u2.inArgList, LAC_OPTIMAL_ALIGNMENT_SHIFT),
            "inArgList structure not correctly aligned");

        /* ensure correct output argument list structure alignment */
        LAC_ASSERT(
            LAC_ADDRESS_ALIGNED(
                &pReqData->u3.outArgList, LAC_OPTIMAL_ALIGNMENT_SHIFT),
            "outArgList structure not correctly aligned");

        /* initialize request data structure */
        LAC_OS_BZERO(pReqData, sizeof(*pReqData));

        /* initialize handle for single request, or first in a chain */
        if (*pRequestHandle == LAC_PKE_INVALID_HANDLE)
        {
            /* store request data pointer in the request handle */
            *pRequestHandle =  (lac_pke_request_handle_t)pReqData;

            /* initialize next, head, and tail request data pointers */
            pReqData->pNextReqData = NULL;
            pReqData->pHeadReqData = pReqData;
            /* note: tail pointer is only valid in head request data struct */
            pReqData->pTailReqData = pReqData;
        }
        else /* handle second or subsequent request in a chain */
        {
            lac_pke_qat_req_data_t *pHeadReqData = NULL;
            lac_pke_qat_req_data_t *pTailReqData = NULL;

            /* extract head request data pointer from the request handle */
            pHeadReqData = *pRequestHandle;
            LAC_ASSERT_NOT_NULL(pHeadReqData);

            /* get tail request data pointer from head request data pointer */
            pTailReqData = pHeadReqData->pTailReqData;
            LAC_ASSERT_NOT_NULL(pTailReqData);

            /* chain the two requests */
            LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
                pTailReqData->u1.request.next_descriptor,
                &(pReqData->u1.request));

            /* chain the request data structures */
            pTailReqData->pNextReqData = pReqData;
            pHeadReqData->pTailReqData = pReqData;
            pReqData->pNextReqData = NULL;
            pReqData->pHeadReqData = pHeadReqData;
            /* note: tail pointer not stored here as it changes (unlike head) */
        }

        /* populate request data structure */
        pReqData->cbInfo.cbFunc = pPkeOpCbFunc;
        pReqData->cbInfo.cbData = *pCbData;
        pReqData->pNextReqData = NULL;
        
        /* initialise the input/output size list with zero sizes */
        LAC_OS_BZERO(&pReqData->paramInfo.inArgSizeList,
            sizeof(Cpa32U)*LAC_MAX_MMP_INPUT_PARAMS);
        LAC_OS_BZERO(&pReqData->paramInfo.outArgSizeList,
            sizeof(Cpa32U)*LAC_MAX_MMP_OUTPUT_PARAMS);
        
        /* if the list is passed by the user, store it in prealocated memory */
        if(NULL != pInArgSizeList)
        {
            memcpy(&pReqData->paramInfo.inArgSizeList, pInArgSizeList,
                sizeof(Cpa32U)*LAC_MAX_MMP_INPUT_PARAMS);         
        }
        if(NULL != pOutArgSizeList)
        {
            memcpy(&pReqData->paramInfo.outArgSizeList, pOutArgSizeList,
                sizeof(Cpa32U)*LAC_MAX_MMP_OUTPUT_PARAMS);         
        }       

        /** @performance : the caller's input/output parameter lists are copied
           here into internal structures.  it would be more efficient, if
           possible, to have the caller populate the internal structure
           directly. */

        /* store input parameters in req struct (end if NULL encountered) */
        for (i = 0; (i < LAC_MAX_MMP_INPUT_PARAMS) &&
            (0 != pInArgList->flat_array[i]); i++)
        {
            LAC_MEM_SHARED_READ_TO_PTR(
                pInArgList->flat_array[i],
                pReqData->paramInfo.clientInputParams[i]);
        }

        /* store output parameters in req struct (end if NULL encountered) */
        for (i = 0; (i < LAC_MAX_MMP_OUTPUT_PARAMS) &&
            (0 != pOutArgList->flat_array[i]); i++)
        {
            LAC_MEM_SHARED_READ_TO_PTR(
                pOutArgList->flat_array[i],
                pReqData->paramInfo.clientOutputParams[i]);
        }

        /* align parameters */
        status = LacPke_AlignParams(
            &pReqData->paramInfo);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate request */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            pReqData->u1.request.opaque_data, pReqData);
        LAC_MEM_SHARED_WRITE_32BIT(
            pReqData->u1.request.functionalityId, functionalityId);
        LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
            pReqData->u1.request.input_params, &pReqData->u2.inArgList);
        LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
            pReqData->u1.request.output_params, &pReqData->u3.outArgList);

        /* populate request header */
        status = QatComms_ReqHdrCreate(
            &(pReqData->u1.request), ICP_ARCH_IF_REQ_QAT_FW_PKE);
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        Cpa8U numInputParams = 0;
        Cpa8U numOutputParams = 0;

        /* store aligned in params in QAT struct (end if NULL encountered) */
        for (i = 0; (i < LAC_MAX_MMP_INPUT_PARAMS) &&
            (NULL != pReqData->paramInfo.pkeInputParams[i]); i++)
        {
            LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
                pReqData->u2.inArgList.flat_array[i],
                pReqData->paramInfo.pkeInputParams[i]);
            numInputParams++;
        }

        /* store aligned out params in QAT struct (end if NULL encountered) */
        for (i = 0; (i < LAC_MAX_MMP_OUTPUT_PARAMS) &&
            (NULL != pReqData->paramInfo.pkeOutputParams[i]); i++)
        {
            LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(
                pReqData->u3.outArgList.flat_array[i],
                pReqData->paramInfo.pkeOutputParams[i]);
            numOutputParams++;
        }

        LAC_ASSERT(((numInputParams + numOutputParams) <= LAC_MAX_MMP_PARAMS),
            "number of input/output parameters exceeds maximum allowed");
            
        /* store input/output parameter counts in the request */
        LAC_MEM_SHARED_WRITE_8BIT(
            pReqData->u1.request.input_param_count, numInputParams);
        LAC_MEM_SHARED_WRITE_8BIT(
            pReqData->u1.request.output_param_count, numOutputParams);
    }

    /* clean up in the event of an error */
    if (CPA_STATUS_SUCCESS != status)
    {
        /* destroy the request (chain) */
        (void) LacPke_DestroyRequest(pRequestHandle);
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      PKE request send to QAT
 ***************************************************************************/
CpaStatus
LacPke_SendRequest(
    lac_pke_request_handle_t *pRequestHandle,
    CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_pke_qat_req_data_t *pHeadReqData = NULL;

    LAC_ASSERT_NOT_NULL(pRequestHandle);

    /* extract head request data pointer from the request handle */
    pHeadReqData = *pRequestHandle;
    LAC_ASSERT_NOT_NULL(pHeadReqData);

    /* store the acceleration handle in the head request */
    pHeadReqData->cbInfo.instanceHandle = instanceHandle;

    /* send the request (chain) */
    status = QatComms_MsgSend(
        &(pHeadReqData->u1.request), ICP_ARCH_IF_REQ_QAT_FW_PKE,
        QAT_COMMS_PRIORITY_NORMAL, instanceHandle);

    if (CPA_STATUS_SUCCESS != status)
    {
        /* destroy the request (chain) */
        (void) LacPke_DestroyRequest(pRequestHandle);
    }

    return status;
}

/**
 ***************************************************************************
 * @ingroup LacAsymCommonQatComms
 *      PKE request create and send to QAT
 ***************************************************************************/
CpaStatus
LacPke_SendSingleRequest(
    Cpa32U functionalityId,
    Cpa32U *pInArgSizeList,
    Cpa32U *pOutArgSizeList,
    icp_qat_fw_mmp_input_param_t *pInArgList,
    icp_qat_fw_mmp_output_param_t *pOutArgList,
    lac_pke_op_cb_func_t pPkeOpCbFunc,
    lac_pke_op_cb_data_t *pCbData,
    CpaInstanceHandle instanceHandle)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_pke_request_handle_t requestHandle = LAC_PKE_INVALID_HANDLE;

    /* prepare the request */
    status = LacPke_CreateRequest(
        &requestHandle, functionalityId,
        pInArgSizeList, pOutArgSizeList,
        pInArgList, pOutArgList,
        pPkeOpCbFunc, pCbData);

    if (CPA_STATUS_SUCCESS == status)
    {
        /* send the request */
        status = LacPke_SendRequest(&requestHandle, instanceHandle);
    }

    return status;
}

