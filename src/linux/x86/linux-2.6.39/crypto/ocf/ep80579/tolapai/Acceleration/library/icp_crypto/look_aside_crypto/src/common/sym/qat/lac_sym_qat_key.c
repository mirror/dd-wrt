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
 * @file lac_sym_qat_key.c Interfaces for populating the symmetric qat key 
 *  structures
 *
 * @ingroup LacSymQatKey
 *
 *****************************************************************************/


#include "cpa.h"
#include "lac_mem.h"
#include "icp_qat_fw_la.h"
#include "lac_sym_qat_key.h"
#include "lac_sym_hash_defs.h"

void
LacSymQat_KeySslRequestPopulate(
    icp_qat_fw_la_key_gen_req_t *pKeyGenReq,
    Cpa32U generatedKeyLenInBytes,
    Cpa32U labelLenInBytes,
    Cpa32U secretLenInBytes,
    Cpa32U iterations)
{
    /* Rounded to nearest 8 byte boundary */
    Cpa8U outLenRounded = LAC_ALIGN_POW2_ROUNDUP(
        generatedKeyLenInBytes, LAC_QUAD_WORD_IN_BYTES);

    pKeyGenReq->req_params_addr = 0;

    /* Set up the common LA flags */
    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->comn_la_req.la_cmd_id,
        ICP_QAT_FW_LA_CMD_SSL3_KEY_DERIVE);

    pKeyGenReq->comn_la_req.u1.resrvd = 0;

    pKeyGenReq->comn_la_req.u2.resrvd2 = 0;
        
    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->u1.label_len, labelLenInBytes);

    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->u2.out_len, outLenRounded); 
    
    LAC_MEM_SHARED_WRITE_16BIT(
        pKeyGenReq->u3.secret_len, secretLenInBytes);

    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->u4.iter_count, iterations);

    /* Set reserved fields to 0 */
    LAC_OS_BZERO(&pKeyGenReq->resrvd2, QAT_FW_LA_KEY_GEN_REQ_RSVD2_SZ);
    pKeyGenReq->resrvd3 = 0;
}


void
LacSymQat_KeyTlsRequestPopulate(
    icp_qat_fw_la_key_gen_req_t *pKeyGenReq,
    icp_qat_fw_la_auth_req_params_t *pHashReqParams,
    Cpa32U generatedKeyLenInBytes,
    Cpa32U labelLenInBytes,
    Cpa32U secretLenInBytes)
{
    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pKeyGenReq->req_params_addr,
                                          pHashReqParams);
  
    /* Setup the common LA request fields */
    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->comn_la_req.la_cmd_id,
        ICP_QAT_FW_LA_CMD_TLS_KEY_DERIVE);

    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->comn_la_req.u1.req_params_blk_sz,
        LAC_BYTES_TO_QUADWORDS(sizeof(icp_qat_fw_la_auth_req_params_t)));

    pKeyGenReq->comn_la_req.u2.resrvd2 = 0;
  
    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->u1.label_len,
        labelLenInBytes);

    LAC_MEM_SHARED_WRITE_8BIT(
        pKeyGenReq->u2.out_len,
        LAC_ALIGN_POW2_ROUNDUP(generatedKeyLenInBytes, 
            LAC_QUAD_WORD_IN_BYTES));

    LAC_MEM_SHARED_WRITE_16BIT(
        pKeyGenReq->u3.secret_len, secretLenInBytes);

    /* Set reserved fields to 0 */
    LAC_OS_BZERO(&pKeyGenReq->resrvd2, QAT_FW_LA_KEY_GEN_REQ_RSVD2_SZ);
    pKeyGenReq->resrvd3 = 0;
    pKeyGenReq->u4.resrvd4 = 0;
}



void
LacSymQat_KeyMgfRequestPopulate(
    icp_qat_fw_la_key_gen_req_t *pKeyGenReq,
    Cpa8U seedLenInBytes,
    Cpa16U maskLenInBytes,
    Cpa8U hashLenInBytes)
{
    LAC_MEM_SHARED_WRITE_8BIT(pKeyGenReq->comn_la_req.la_cmd_id,
                         ICP_QAT_FW_LA_CMD_MGF1);

    pKeyGenReq->comn_la_req.u1.resrvd = 0;

    pKeyGenReq->comn_la_req.u2.resrvd2 = 0;
    
    LAC_MEM_SHARED_WRITE_8BIT(pKeyGenReq->u1.seed_len, seedLenInBytes);
    
    LAC_MEM_SHARED_WRITE_8BIT(pKeyGenReq->u2.hash_len, hashLenInBytes);
    
    LAC_MEM_SHARED_WRITE_16BIT(pKeyGenReq->u3.mask_len, 
        LAC_ALIGN_POW2_ROUNDUP(maskLenInBytes, LAC_QUAD_WORD_IN_BYTES));

    /* Set reserved fields to 0 */
    LAC_OS_BZERO(&pKeyGenReq->resrvd2, QAT_FW_LA_KEY_GEN_REQ_RSVD2_SZ);
    pKeyGenReq->resrvd3 = 0;
    pKeyGenReq->u4.resrvd4 = 0;
}


void 
LacSymQat_KeySslKeyMaterialInputPopulate(
    icp_qat_fw_la_ssl_key_material_input_t *pSslKeyMaterialInput, 
    void *pSeed,
    void *pLabel,
    void *pSecret)
{
    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pSslKeyMaterialInput->seed_addr, 
                                          pSeed);

    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pSslKeyMaterialInput->label_addr, 
                                          pLabel);

    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pSslKeyMaterialInput->secret_addr,
                                          pSecret);
}


void 
LacSymQat_KeyTlsKeyMaterialInputPopulate(
    icp_qat_fw_la_tls_key_material_input_t *pTlsKeyMaterialInput,
    void *pSeed,
    void *pLabel)
{
    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pTlsKeyMaterialInput->seed_addr, 
                                          pSeed);

    LAC_MEM_SHARED_WRITE_VIRT_TO_PHYS_PTR(pTlsKeyMaterialInput->label_addr,
                                          pLabel);
}
