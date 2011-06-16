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
 * @file lac_rsa_decrypt.c
 *
 * @ingroup LacRsa
 *
 * This file implements decrypt functions for RSA.
 *
 *****************************************************************************/

/*
********************************************************************************
* Include public/global header files
********************************************************************************
*/

/* Include API files */
#include "cpa.h"
#include "cpa_cy_rsa.h"

/* Include Osal files */
#include "IxOsal.h"

/* Include QAT header files */
#include "icp_qat_fw_mmp_ids.h"

/* Include LAC files */
#include "lac_common.h"
#include "lac_pke_qat_comms.h"
#include "lac_pke_utils.h"
#include "lac_pke_mmp.h"

/*
********************************************************************************
* Include private header files
********************************************************************************
*/

#include "lac_rsa_p.h"
#include "lac_rsa_stats_p.h"

/*
********************************************************************************
* Static Variables
********************************************************************************
*/

static const Cpa32U LAC_RSA_DP1_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
    {
        {LAC_1024_BITS, PKE_RSA_DP1_1024},
        {LAC_1536_BITS, PKE_RSA_DP1_1536},
        {LAC_2048_BITS, PKE_RSA_DP1_2048},
        {LAC_3072_BITS, PKE_RSA_DP1_3072},
        {LAC_4096_BITS, PKE_RSA_DP1_4096}
    };
/**<
 *  Maps between operation sizes and PKE function ids */

static const Cpa32U LAC_RSA_DP2_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
    {
        {LAC_1024_BITS, PKE_RSA_DP2_1024},
        {LAC_1536_BITS, PKE_RSA_DP2_1536},
        {LAC_2048_BITS, PKE_RSA_DP2_2048},
        {LAC_3072_BITS, PKE_RSA_DP2_3072},
        {LAC_4096_BITS, PKE_RSA_DP2_4096}
    };
/**<
 *  Maps between operation sizes and PKE function ids */

/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

/*
 * This function checks the parameters for an RSA decrypt operation. It returns
 * the appropriate error in the case of null and invalid params and also
 * unsupported operations.
 */
#if defined(ICP_PARAM_CHECK)
STATIC CpaStatus
LacRsa_DecryptParamsCheck(
    const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData);
#endif //ICP_PARAM_CHECK

/*
 * This function is called by the pke comms module after an RSA Encrypt
 * message has been received from the QAT.
 */
STATIC void
LacRsa_ProcessDecCb(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData);

/*
 * This function performs RSA Decrypt for type 1 private keys.
 */
STATIC CpaStatus
LacRsa_Type1Decrypt(
    const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    void *pCallbackTag,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData);

/*
 * This function performs RSA Decrypt for type 2 private keys.
 */
STATIC CpaStatus
LacRsa_Type2Decrypt(
    const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    void *pCallbackTag,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData);

/*
 * This is the LAC RSA Decrypt synchronous function.
 */
STATIC CpaStatus
LacRsa_DecryptSynch(
    const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    void *pCallbackTag,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData);

/*
********************************************************************************
* Global Variables
********************************************************************************
*/



/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

/*
********************************************************************************
* Define public/global function definitions
********************************************************************************
*/

/**
 *****************************************************************************
 * @ingroup LacRsa
 *
 *****************************************************************************/
CpaStatus
cpaCyRsaDecrypt(const CpaInstanceHandle instanceHandle,
                const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
                void *pCallbackTag,
                const CpaCyRsaDecryptOpData *pDecryptData,
                CpaFlatBuffer *pOutputData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Check if the API has been called in sync mode */
    if (NULL == pRsaDecryptCb)
    {
        return LacRsa_DecryptSynch(instanceHandle, pRsaDecryptCb,
                    pCallbackTag, pDecryptData, pOutputData);
    }

#if defined(ICP_PARAM_CHECK)
    /* Check RSA Decrypt params and return an error if invalid */
    status = LacRsa_DecryptParamsCheck(instanceHandle, pRsaDecryptCb,
        pDecryptData, pOutputData);
#endif //ICP_PARAM_CHECK

    if (CPA_STATUS_SUCCESS == status)
    {
        if (CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_1
            == pDecryptData->pRecipientPrivateKey->privateKeyRepType)
        {
             status = LacRsa_Type1Decrypt(instanceHandle, pRsaDecryptCb,
                  pCallbackTag, pDecryptData, pOutputData);
        }
        else /* Must be type2 key as param check has passed */
        {
             status = LacRsa_Type2Decrypt(instanceHandle, pRsaDecryptCb,
                  pCallbackTag, pDecryptData, pOutputData);
        }
    }

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_RSA_STAT_INC(numRsaDecryptRequests, instanceHandle);
    }
    else
    {
        LAC_RSA_STAT_INC(numRsaDecryptRequestErrors, instanceHandle);
    }

    return status;
}


STATIC CpaStatus
LacRsa_DecryptSynch(const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
        void *pCallbackTag,
        const CpaCyRsaDecryptOpData *pDecryptData,
        CpaFlatBuffer *pOutputData)
{
    CpaStatus status = CPA_STATUS_FAIL;
    lac_sync_op_data_t *pSyncCallbackData = NULL;

    status = LacSync_CreateSyncCookie(&pSyncCallbackData);
    /*
     * Call the async version of the function
     * with the sync callback function as a parameter.
     */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = cpaCyRsaDecrypt(instanceHandle, 
                    LacSync_GenFlatBufCb, pSyncCallbackData, 
                    pDecryptData, pOutputData);
    }
    else
    {
        LAC_RSA_STAT_INC(numRsaDecryptRequestErrors, instanceHandle);
        return status;
    }
    if (CPA_STATUS_SUCCESS == status)
    {
        CpaStatus wCbStatus = CPA_STATUS_FAIL;
        wCbStatus = LacSync_WaitForCallback(pSyncCallbackData, 
                        LAC_PKE_SYNC_CALLBACK_TIMEOUT, 
                        &status, NULL);
        if (CPA_STATUS_SUCCESS != wCbStatus)
        {
            /*
             * Inc stats only if the wait for callback failed.
             */
            LAC_RSA_STAT_INC(numRsaDecryptCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}


#if defined(ICP_PARAM_CHECK)
STATIC CpaStatus
LacRsa_DecryptParamsCheck(
    const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U opSizeInBytes = 0;

    LAC_INITIALISED_CHECK();
    LAC_PKE_INITIALISED_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pRsaDecryptCb);

    /* Check user parameters */
    LAC_CHECK_NULL_PARAM(pDecryptData);

    status = LacRsa_CheckPrivateKeyParam(
        pDecryptData->pRecipientPrivateKey, 0, CPA_TRUE, CPA_TRUE);
    LAC_CHECK_STATUS(status);

    opSizeInBytes
        = LacRsa_GetPrivateKeyOpSize(pDecryptData->pRecipientPrivateKey);

    if (CPA_FALSE == LacRsa_IsValidRsaSize(opSizeInBytes))
    {
        LAC_INVALID_PARAM_LOG(
            "Invalid Private Key Size - pDecryptData->pRecipientPrivateKey");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Check message and ciphertext buffers */
    LAC_CHECK_FLAT_BUFFER_PARAM(&(pDecryptData->inputData),
        CHECK_LESS_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);
    LAC_CHECK_FLAT_BUFFER_PARAM(pOutputData,
        CHECK_GREATER_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);
    pOutputData->dataLenInBytes = opSizeInBytes;

    if (CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_1
        == pDecryptData->pRecipientPrivateKey->privateKeyRepType)
    {
        /* Standards based check: 0 < c < n */
        LAC_CHECK_NON_ZERO_PARAM(&(pDecryptData->inputData));
        if (LacPke_Compare(&(pDecryptData->inputData), 0,
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep1.modulusN), 0)
            >= 0)
        {
            LAC_INVALID_PARAM_LOG("inputData must be < modulusN");
            return CPA_STATUS_INVALID_PARAM;
        }
    }
    else
    {
        status = LacRsa_Type2StdsCheck(
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep2));
        LAC_CHECK_STATUS(status);
    }

    return status;
}
#endif //ICP_PARAM_CHECK

CpaStatus
LacRsa_Type1Decrypt(const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    void *pCallbackTag,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData)
{
    Cpa32U opSizeInBytes = 0;
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    Cpa32U pInArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U pOutArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};
    CpaStatus status = CPA_STATUS_FAIL;
    lac_pke_op_cb_data_t cbData = {0};
    lac_mmp_input_param_t in = {.inputParam.flat_array = {0}};
    lac_mmp_output_param_t out = {.outputParam.flat_array = {0}};

    LAC_ASSERT_NOT_NULL(pDecryptData);
    LAC_ASSERT_NOT_NULL(pOutputData);

    opSizeInBytes
        = LacRsa_GetPrivateKeyOpSize(pDecryptData->pRecipientPrivateKey);

    functionalityId = LacPke_GetMmpId(LAC_BYTES_TO_BITS(opSizeInBytes),
        LAC_RSA_DP1_SIZE_ID_MAP, LAC_ARRAY_LEN(LAC_RSA_DP1_SIZE_ID_MAP));
    if (LAC_PKE_INVALID_FUNC_ID == functionalityId)
    {
        LAC_INVALID_PARAM_LOG(
            "Invalid Private Key Size - pDecryptData->pRecipientPrivateKey");
        status = CPA_STATUS_INVALID_PARAM;
    }
    else
    {
       /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.rsaDp1Input.c, &(pDecryptData->inputData));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp1_input_t, c)]
             = opSizeInBytes;

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp1Input.d,
        &(pDecryptData->pRecipientPrivateKey->privateKeyRep1.privateExponentD));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp1_input_t, d)]
             = opSizeInBytes;

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp1Input.n,
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep1.modulusN));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp1_input_t, n)]
             = opSizeInBytes;

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaDp1Output.m, pOutputData);
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp1_output_t, m)]
             = opSizeInBytes;

        /* populate callback data */
        cbData.pClientCb = pRsaDecryptCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pDecryptData;
        cbData.pOutputData1 = pOutputData;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(functionalityId, pInArgSizeList,
            pOutArgSizeList, &(in.inputParam), &(out.outputParam),
            LacRsa_ProcessDecCb, &cbData, instanceHandle);
    }

    return status;
}

CpaStatus
LacRsa_Type2Decrypt(const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pRsaDecryptCb,
    void *pCallbackTag,
    const CpaCyRsaDecryptOpData *pDecryptData,
    CpaFlatBuffer *pOutputData)
{
    Cpa32U opSizeInBytes = 0;
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    CpaStatus status = CPA_STATUS_FAIL;
    Cpa32U pInArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U pOutArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};
    lac_pke_op_cb_data_t cbData = {0};
    lac_mmp_input_param_t in = {.inputParam.flat_array = {0}};
    lac_mmp_output_param_t out = {.outputParam.flat_array = {0}};

    LAC_ASSERT_NOT_NULL(pDecryptData);
    LAC_ASSERT_NOT_NULL(pOutputData);

    opSizeInBytes
        = LacRsa_GetPrivateKeyOpSize(pDecryptData->pRecipientPrivateKey);

    functionalityId = LacPke_GetMmpId(LAC_BYTES_TO_BITS(opSizeInBytes),
        LAC_RSA_DP2_SIZE_ID_MAP, LAC_ARRAY_LEN(LAC_RSA_DP2_SIZE_ID_MAP));
    if (LAC_PKE_INVALID_FUNC_ID == functionalityId)
    {
        LAC_INVALID_PARAM_LOG(
            "Invalid Private Key Size - pDecryptData->pRecipientPrivateKey");
        status = CPA_STATUS_INVALID_PARAM;
    }
    else
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.rsaDp1Input.c, &(pDecryptData->inputData));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_input_t, c)]
            = opSizeInBytes;

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp2Input.p,
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep2.prime1P));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_input_t, p)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp2Input.q,
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep2.prime2Q));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_input_t, q)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp2Input.dp,
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep2.exponent1Dp));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_input_t, dp)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp2Input.dq,
            &(pDecryptData->pRecipientPrivateKey->privateKeyRep2.exponent2Dq));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_input_t, dq)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaDp2Input.qinv,
         &(pDecryptData->pRecipientPrivateKey->privateKeyRep2.coefficientQInv));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_input_t, qinv)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaDp2Output.m, pOutputData);
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_dp2_output_t, m)]
             = opSizeInBytes;

        /* populate callback data */
        cbData.pClientCb = pRsaDecryptCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pDecryptData;
        cbData.pOutputData1 = pOutputData;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(functionalityId, pInArgSizeList,
            pOutArgSizeList, &(in.inputParam), &(out.outputParam),
            LacRsa_ProcessDecCb, &cbData, instanceHandle);
    }

    return status;
}

void
LacRsa_ProcessDecCb(CpaStatus status, CpaBoolean pass,
    CpaInstanceHandle instanceHandle, lac_pke_op_cb_data_t *pCbData)
{
    CpaCyGenFlatBufCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyRsaDecryptOpData *pOpData = NULL;
    CpaFlatBuffer *pOutputData = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCallbackTag = (void *)pCbData->pCallbackTag;

    pOpData = (CpaCyRsaDecryptOpData *) 
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    LAC_ASSERT_NOT_NULL(pOpData);

    pCb = (CpaCyGenFlatBufCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    LAC_ASSERT_NOT_NULL(pCb);

    pOutputData = pCbData->pOutputData1;
    LAC_ASSERT_NOT_NULL(pOutputData);

    /* increment stats */
    LAC_RSA_STAT_INC(numRsaDecryptCompleted, instanceHandle);
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_RSA_STAT_INC(numRsaDecryptCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pOutputData);
}
