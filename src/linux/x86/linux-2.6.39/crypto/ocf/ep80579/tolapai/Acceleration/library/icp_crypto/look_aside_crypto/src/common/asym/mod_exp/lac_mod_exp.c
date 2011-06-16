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
 * @file lac_mod_exp.c
 *
 * @ingroup LacModExp
 *
 * This file implements the mod exp feature.
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

#include "cpa.h"
#include "cpa_cy_common.h"
#include "IxOsal.h"

#include "icp_qat_fw_pke.h"
#include "icp_qat_fw_mmp.h"
#include "icp_qat_fw_mmp_ids.h"

#include "lac_common.h"
#include "lac_mod_exp.h"
#include "lac_mem.h"

#include "lac_pke_utils.h"
#include "lac_pke_mmp.h"
#include "lac_pke_qat_comms.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

/*
*******************************************************************************
* Static Variables
*******************************************************************************
*/

/* Maps between operation sizes and PKE function ids */
static const Cpa32U LAC_MOD_EXP_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
    {
        {LAC_160_BITS, PKE_MODEXP_160},
        {LAC_512_BITS, PKE_MODEXP_512},
        {LAC_768_BITS, PKE_MODEXP_768},
        {LAC_1024_BITS, PKE_MODEXP_1024},
        {LAC_1536_BITS, PKE_MODEXP_1536},
        {LAC_2048_BITS, PKE_MODEXP_2048},
        {LAC_3072_BITS, PKE_MODEXP_3072},
        {LAC_4096_BITS, PKE_MODEXP_4096}
    };

/* Maps between operation sizes and PKE function ids */
static const Cpa32U LAC_MOD_EXP_G2_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
    {
        {LAC_160_BITS, PKE_MODEXP_G2_160},
        {LAC_512_BITS, PKE_MODEXP_G2_512},
        {LAC_768_BITS, PKE_MODEXP_G2_768},
        {LAC_1024_BITS, PKE_MODEXP_G2_1024},
        {LAC_1536_BITS, PKE_MODEXP_G2_1536},
        {LAC_2048_BITS, PKE_MODEXP_G2_2048},
        {LAC_3072_BITS, PKE_MODEXP_G2_3072},
        {LAC_4096_BITS, PKE_MODEXP_G2_4096}
    };

/*
*******************************************************************************
* Define static function definitions
*******************************************************************************
*/

/*
*******************************************************************************
* Global Variables
*******************************************************************************
*/

/*
*******************************************************************************
* Enumerations
*******************************************************************************
*/

/*
*******************************************************************************
* Functions
*******************************************************************************
*/

CpaStatus
LacModExp_Perform(lac_mod_exp_op_data_t *pData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U pInArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U pOutArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};
    lac_mmp_input_param_t inArgList = {.inputParam.flat_array = {0}};
    lac_mmp_output_param_t outArgList = {.outputParam.flat_array = {0}};
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    Cpa32U sizeInBytes = 0;

    LAC_ASSERT_NOT_NULL(pData);
    sizeInBytes = LAC_BITS_TO_BYTES(pData->sizeInBits);

    if (LAC_MOD_EXP_OP == pData->opType)
    {
        functionalityId = LacPke_GetMmpId(pData->sizeInBits,
            LAC_MOD_EXP_SIZE_ID_MAP, LAC_ARRAY_LEN(LAC_MOD_EXP_SIZE_ID_MAP));

        /* Fill out Input List */
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.modexpInput.g, pData->pBase);
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_input_t, g)]
             = sizeInBytes;
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            inArgList.modexpInput.e, pData->pExponent);
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_input_t, e)]
            = sizeInBytes;
        LAC_MEM_SHARED_WRITE_FROM_PTR(inArgList.modexpInput.m, pData->pModulus);
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_input_t, m)]
            = sizeInBytes;

        /* Fill out Output List */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            outArgList.modexpOutput.r, pData->pResult);
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_output_t, r)]
            = sizeInBytes;
    }
    else if (LAC_MOD_EXP_G2_OP == pData->opType)
    {
        functionalityId = LacPke_GetMmpId(pData->sizeInBits,
            LAC_MOD_EXP_G2_SIZE_ID_MAP,
            LAC_ARRAY_LEN(LAC_MOD_EXP_G2_SIZE_ID_MAP));

        /* Fill out Input List */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            inArgList.modexpG2Input.e, pData->pExponent);
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_g2_input_t, e)]
            = sizeInBytes;
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            inArgList.modexpG2Input.m, pData->pModulus);
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_g2_input_t, m)]
            = sizeInBytes;

        /* Fill out Output List */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            outArgList.modexpG2Output.r, pData->pResult);
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_modexp_g2_output_t, r)]
            = sizeInBytes;
    }
    else
    {   
        LAC_LOG_ERROR("Invalid mod exp operation type");
        return CPA_STATUS_FAIL;
    }

    if (LAC_PKE_INVALID_FUNC_ID == functionalityId)
    {
        LAC_LOG_ERROR("Invalid pke func ID");
        status =  CPA_STATUS_FAIL;
    }
    else
    {
        status = LacPke_SendSingleRequest(functionalityId, pInArgSizeList,
            pOutArgSizeList, &(inArgList.inputParam), &(outArgList.outputParam),
            pData->pPkeOpCbFunc, pData->pPkeOpData, pData->instanceHandle);
    }

    return status;
}
