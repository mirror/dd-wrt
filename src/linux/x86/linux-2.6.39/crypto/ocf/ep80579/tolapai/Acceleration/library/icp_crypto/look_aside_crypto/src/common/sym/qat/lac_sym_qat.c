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
 * @file lac_sym_qat.c Interfaces for populating the symmetric qat structures
 *
 * @ingroup LacSymQat
 *
 *****************************************************************************/


/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_sym.h"
#include "IxOsal.h"
#include "qat_comms.h"
#include "icp_qat_hw.h"
#include "icp_qat_fw.h"
#include "icp_qat_fw_la.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

#include "lac_sym_qat.h"
#include "lac_common.h"
#include "lac_mem.h"
#include "lac_sym_qat_hash_defs_lookup.h"

/*****************************************************************************/

/* array of response handler functions for a symmetric response ID */
static
lac_sym_qat_resp_handler_func_t RespHandlerTbl[ICP_QAT_FW_LA_CMD_DELIMITER];


STATIC void
LacSymQat_SymRespHandler(
    icp_qat_fw_la_resp_t *pRespMsg,
    icp_arch_if_request_t msgType)
{
    Cpa8U lacCmdId = 0;

    LAC_ENSURE(pRespMsg != NULL, "LacSymQat_SymRespHandler - pRespMsg NULL\n");
    LAC_MEM_SHARED_READ(pRespMsg->comn_resp.serv_cmd_id, lacCmdId);

    if (lacCmdId >= ICP_QAT_FW_LA_CMD_DELIMITER)
    {
        LAC_LOG_ERROR1("Invalid Lac Command ID 0x%x", lacCmdId);
    }
    else
    {
        lac_sym_qat_resp_handler_func_t pRespFunc = NULL;
        void *pOpaqueData;
        icp_qat_fw_comn_flags cmnRespFlags = 0;

        pRespFunc = RespHandlerTbl[lacCmdId];

        if (NULL == pRespFunc)
        {
            LAC_LOG_ERROR1("No message response handler registered for command" 
                           " ID 0x%x", lacCmdId);
        }
        /* call the response message handler registered for the command ID */
        else
        {
            LAC_MEM_SHARED_READ(pRespMsg->comn_resp.comn_resp_flags,
                                cmnRespFlags);

            LAC_MEM_SHARED_READ_TO_PTR(pRespMsg->comn_resp.opaque_data,
                                       pOpaqueData);

            if (NULL == pOpaqueData)
            {
                LAC_LOG_ERROR("Opaque data is NULL");
            }
            else
            {
                /* call the function to process the response message */
                pRespFunc(lacCmdId, pOpaqueData, cmnRespFlags);
            }
        }
    }
}


CpaStatus
LacSymQat_Init(void)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Clear the response handler table */
    LAC_OS_BZERO(&RespHandlerTbl, sizeof(RespHandlerTbl));

    status = QatComms_ResponseCbSet(
        (qat_comms_cb_func_t)LacSymQat_SymRespHandler,
        ICP_ARCH_IF_REQ_QAT_FW_LA);

    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_LOG_ERROR("Failed to register callback for " \
                      "response message handling");
        status = CPA_STATUS_FAIL;
    }

    /* initialiase the Hash lookup table */
    LacSymQat_HashLookupInit();

    return status;
}


CpaStatus
LacSymQat_Shutdown(void)
{
    /* This function does nothing at the moment but is a placeholder */

    return CPA_STATUS_SUCCESS;
}


void
LacSymQat_RespHandlerRegister(
    icp_qat_fw_la_cmd_id_t lacCmdId,
    lac_sym_qat_resp_handler_func_t pCbHandler)
{
    LAC_ENSURE((lacCmdId < ICP_QAT_FW_LA_CMD_DELIMITER), "Invalid Command ID");

    /* set the response handler for the command ID */
    RespHandlerTbl[lacCmdId] = pCbHandler;
}


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
    Cpa16U laCmdFlags)
{
    /**************************************************************************
     *                                            Common QAT FW request header
     *************************************************************************/

    LAC_ENSURE(pBulkRq != NULL,
               "LacSymQat_BulkReqPopulate - pBulkRq is NULL\n");

    LacSymQat_ComnReqHdrPopulate(
        &(pBulkRq->comn_req),
        pContentDescInfo,
        pCookie,
        srcBuffer,
        dstBuffer,
        CPA_TRUE, /* Ordering */
        LAC_SYM_QAT_BUFFER_LIST);

    /*************************************************************************/

    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pBulkRq->req_params_addr, pReqParams);


    /**************************************************************************
     *                                            Common LA request parameters
     *************************************************************************/

    /* set the LAC command ID */
    LAC_MEM_SHARED_WRITE_8BIT(pBulkRq->comn_la_req.la_cmd_id, laCmdId);

    /* set the request params block size in quad words */
    LAC_MEM_SHARED_WRITE_8BIT(pBulkRq->comn_la_req.u1.req_params_blk_sz,
        reqParamBlkSizeBytes / LAC_QUAD_WORD_IN_BYTES);

    /* set the LA command flags */
    LAC_MEM_SHARED_WRITE_16BIT(pBulkRq->comn_la_req.u2.la_flags, laCmdFlags);

    /**************************************************************************
     *                                                          Reserved area
     *************************************************************************/

    /* Clear the reserved area contents in the request descriptor */
    LAC_OS_BZERO(pBulkRq->resrvd, sizeof(pBulkRq->resrvd));
}


void
LacSymQat_ComnReqHdrPopulate(
    icp_qat_fw_comn_req_hdr_t * pComnReq,
    const lac_sym_qat_content_desc_info_t * pContentDescInfo,
    const void * pCookie,
    Cpa64U srcBuffer,
    Cpa64U dstBuffer,
    CpaBoolean ordering,
    lac_sym_qat_buffer_type_t bufferType)
{
    icp_qat_fw_comn_flags cmnReqFlags = 0;

    LAC_ENSURE(pComnReq != NULL,
               "LacSymQat_ComnReqHdrPopulate - pComnReq is NULL\n");

    /* QAT comms will write the Common arch fields of the message */

    LAC_MEM_SHARED_WRITE_8BIT(pComnReq->content_desc_hdr_sz,
                         pContentDescInfo->hdrSzQuadWords);

    LAC_MEM_SHARED_WRITE_8BIT(pComnReq->content_desc_params_sz,
                         pContentDescInfo->hwBlkSzQuadWords);

    /* === set the common request flags  ==== */

    if (CPA_TRUE == ordering)
    {
        ICP_QAT_FW_COMN_ORD_SET(cmnReqFlags,
                                ICP_QAT_FW_COMN_ORD_FLAG_STRICT);
    }
    else
    {
        ICP_QAT_FW_COMN_ORD_SET(cmnReqFlags,
                                ICP_QAT_FW_COMN_ORD_FLAG_NONE);
    }

    /* Buffer Descriptors */
    if (LAC_SYM_QAT_BUFFER_DESC == bufferType)
    {
        ICP_QAT_FW_COMN_DEST_TYPE_SET(cmnReqFlags,
                                      ICP_QAT_FW_COMN_DEST_FLAG_ICP_BUF);

        ICP_QAT_FW_COMN_SRC_TYPE_SET(cmnReqFlags,
                                     ICP_QAT_FW_COMN_SRC_FLAG_ICP_BUF);
    }
    /* Flat Buffers */
    else if (LAC_SYM_QAT_FLAT_BUFFER == bufferType)
    {
        ICP_QAT_FW_COMN_DEST_TYPE_SET(cmnReqFlags,
                                      ICP_QAT_FW_COMN_DEST_FLAG_FLAT);

        ICP_QAT_FW_COMN_SRC_TYPE_SET(cmnReqFlags,
                                     ICP_QAT_FW_COMN_SRC_FLAG_FLAT);
    }
    /* Buffer List */
    else if (LAC_SYM_QAT_BUFFER_LIST == bufferType)
    {
        ICP_QAT_FW_COMN_SRC_TYPE_SET(cmnReqFlags, 
                                     ICP_QAT_FW_COMN_SRC_FLAG_SGL_BUF);

        ICP_QAT_FW_COMN_DEST_TYPE_SET(cmnReqFlags, 
                                      ICP_QAT_FW_COMN_DEST_FLAG_SGL_BUF);
    }
    else
    {
        LAC_ENSURE(CPA_FALSE, "LacSymQat_ComnReqHdrPopulate - "
                   "Invalid bufferType parameter\n");
    }

    LAC_MEM_SHARED_WRITE_64BIT(pComnReq->src_data_addr, srcBuffer);
    LAC_MEM_SHARED_WRITE_64BIT(pComnReq->dest_data_addr, dstBuffer);

/* Direction : in place */
    if (0 == dstBuffer)
    {
        /* set to this value if the destination is unused by the request */
        ICP_QAT_FW_COMN_DEST_TYPE_SET(cmnReqFlags,
                                      ICP_QAT_FW_COMN_DEST_FLAG_FLAT);

        ICP_QAT_FW_COMN_DIR_SET(cmnReqFlags,
                                ICP_QAT_FW_COMN_DIR_FLAG_INPLACE);
    }
    /* Direction : out of place */
    else
    {
        ICP_QAT_FW_COMN_DIR_SET(cmnReqFlags,
                                ICP_QAT_FW_COMN_DIR_FLAG_OUT_PLACE);
    }

    ICP_QAT_FW_COMN_OWN_SET(cmnReqFlags, ICP_QAT_FW_COMN_OWN_NONE);

    /* write local variable for the common request flags into the message */
    LAC_MEM_SHARED_WRITE_16BIT(pComnReq->comn_req_flags, cmnReqFlags);

    LAC_MEM_SHARED_WRITE_FROM_PTR(pComnReq->opaque_data, pCookie);

    LAC_MEM_SHARED_WRITE_64BIT(pComnReq->content_desc_addr,
                         pContentDescInfo->pDataPhys);
}


void
LacSymQat_LaPacketCommandFlagSet(
    Cpa32U qatPacketType,
    Cpa16U *pLaCommandFlags)
{
    ICP_QAT_FW_LA_PARTIAL_SET(
        *pLaCommandFlags, qatPacketType);

    /* update state disabled for full packets and final partials */
    if ((ICP_QAT_FW_LA_PARTIAL_NONE == qatPacketType) ||
        (ICP_QAT_FW_LA_PARTIAL_END == qatPacketType))
    {
        ICP_QAT_FW_LA_WR_STATE_SET(
            *pLaCommandFlags, ICP_QAT_FW_LA_NO_UPDATE_STATE);
    }
    /* For first or middle partials set the update state command flag */
    else
    {
        ICP_QAT_FW_LA_WR_STATE_SET(
            *pLaCommandFlags, ICP_QAT_FW_LA_UPDATE_STATE);
    }
}


void
LacSymQat_packetTypeGet(
    CpaCySymPacketType packetType,
    CpaCySymPacketType packetState,
    Cpa32U *pQatPacketType)
{
    /* partial */
    if (CPA_CY_SYM_PACKET_TYPE_PARTIAL == packetType)
    {
        /* if the previous state was full, then this is the first packet */
        if (CPA_CY_SYM_PACKET_TYPE_FULL == packetState)
        {
            *pQatPacketType = ICP_QAT_FW_LA_PARTIAL_START;
        }
        else
        {
            *pQatPacketType = ICP_QAT_FW_LA_PARTIAL_MID;
        }
    }
    /* final partial */
    else if (CPA_CY_SYM_PACKET_TYPE_LAST_PARTIAL == packetType)
    {
        *pQatPacketType = ICP_QAT_FW_LA_PARTIAL_END;
    }
    /* full packet - CPA_CY_SYM_PACKET_TYPE_FULL */
    else
    {
        *pQatPacketType = ICP_QAT_FW_LA_PARTIAL_NONE;
    }
}

