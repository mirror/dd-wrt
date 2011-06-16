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
 * @file lac_rsa_keygen.c
 *
 * @ingroup LacRsa
 *
 * This file implements keygen functions for RSA.
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

/*Include LAC files */
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

#define LAC_RSA_KEYGEN_MIN_EXP -3
/**<
 *  Exponenent for key gen ops must be >= 3 */

static const Cpa32U LAC_RSA_KP1_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
    {
        {LAC_1024_BITS, PKE_RSA_KP1_1024},
        {LAC_1536_BITS, PKE_RSA_KP1_1536},
        {LAC_2048_BITS, PKE_RSA_KP1_2048},
        {LAC_3072_BITS, PKE_RSA_KP1_3072},
        {LAC_4096_BITS, PKE_RSA_KP1_4096}
    };
/**<
 *  Maps between operation sizes and PKE function ids */

static const Cpa32U LAC_RSA_KP2_SIZE_ID_MAP[][LAC_PKE_NUM_COLUMNS] =
    {
        {LAC_1024_BITS, PKE_RSA_KP2_1024},
        {LAC_1536_BITS, PKE_RSA_KP2_1536},
        {LAC_2048_BITS, PKE_RSA_KP2_2048},
        {LAC_3072_BITS, PKE_RSA_KP2_3072},
        {LAC_4096_BITS, PKE_RSA_KP2_4096}
    };
/**<
 *  Maps between operation sizes and PKE function ids */

/*
********************************************************************************
* Define static function definitions
********************************************************************************
*/

/*
 * This function performs synchronious version of the RSA Key Gen.
 */
STATIC CpaStatus
LacRsa_KeyGenSync(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    void *pCallbackTag,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey);

/*
 * This function is the synchronious callback function.
 */
STATIC
void LacRsa_KeyGenSyncCb(void *pCallbackTag, CpaStatus status,
                         void *pOpData, 
                         CpaCyRsaPrivateKey *pPrivateKey,
                         CpaCyRsaPublicKey *pPublicKey);

/*
 * This function checks the parameters for an RSA encrypt operation. It returns
 * the appropriate error in the case of null and invalid params and also
 * unsupported operations.
 */
#if defined(ICP_PARAM_CHECK)
STATIC CpaStatus
LacRsa_KeyGenParamsCheck(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey);
#endif //ICP_PARAM_CHECK

/*
 * This function is called by the pke comms module after an RSA Encrypt
 * message has been received from the QAT.
 */
STATIC void
LacRsa_ProcessKeyCb(
    CpaStatus status,
    CpaBoolean pass,
    CpaInstanceHandle instanceHandle,
    lac_pke_op_cb_data_t *pCbData);

/*
 * This function performs RSA Decrypt for type 1 private keys.
 */
STATIC CpaStatus
LacRsa_Type1KeyGen(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    void *pCallbackTag,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey);

/*
 * This function performs RSA Decrypt for type 2 private keys.
 */
STATIC CpaStatus
LacRsa_Type2KeyGen(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    void *pCallbackTag,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey);

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
cpaCyRsaGenKey(const CpaInstanceHandle instanceHandle,
               const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
               void *pCallbackTag,
               const CpaCyRsaKeyGenOpData *pKeyGenData,
               CpaCyRsaPrivateKey *pPrivateKey,
               CpaCyRsaPublicKey *pPublicKey)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Check if the API has been called in sync mode */
    if (NULL == pRsaKeyGenCb)
    {
        return LacRsa_KeyGenSync(instanceHandle, pRsaKeyGenCb,
                    pCallbackTag, pKeyGenData, pPrivateKey,
                    pPublicKey);
    }
    
#ifdef ICP_PARAM_CHECK
    /* Check RSA KeyGen params and return an error if invalid */
    status = LacRsa_KeyGenParamsCheck(instanceHandle, pRsaKeyGenCb,
        pKeyGenData, pPrivateKey, pPublicKey);
#endif //ICP_PARAM_CHECK

    if (CPA_STATUS_SUCCESS == status)
    {
        CpaCyRsaPrivateKeyRepType keyType
            = pPrivateKey->privateKeyRepType;
        if (CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_1 == keyType)
        {
             status = LacRsa_Type1KeyGen(instanceHandle, pRsaKeyGenCb,
                  pCallbackTag, pKeyGenData, pPrivateKey, pPublicKey);
        }
        else /* Must be type2 key as param check has passed */
        {
             status = LacRsa_Type2KeyGen(instanceHandle, pRsaKeyGenCb,
                  pCallbackTag, pKeyGenData, pPrivateKey, pPublicKey);
        }
    }

    /* increment stats */
    if (CPA_STATUS_SUCCESS == status)
    {
        LAC_RSA_STAT_INC(numRsaKeyGenRequests, instanceHandle);
    }
    else
    {
        LAC_RSA_STAT_INC(numRsaKeyGenRequestErrors, instanceHandle);
    }

    return status;
}

STATIC
void LacRsa_KeyGenSyncCb(void *pCallbackTag, CpaStatus status,
                         void *pOpData, 
                         CpaCyRsaPrivateKey *pPrivateKey,
                         CpaCyRsaPublicKey *pPublicKey)
{
    return LacSync_GenWakeupSyncCaller(pCallbackTag, status);
}


STATIC CpaStatus
LacRsa_KeyGenSync(const CpaInstanceHandle instanceHandle,
                  const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
                  void *pCallbackTag,
                  const CpaCyRsaKeyGenOpData *pKeyGenData,
                  CpaCyRsaPrivateKey *pPrivateKey,
                  CpaCyRsaPublicKey *pPublicKey)
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
        status = cpaCyRsaGenKey(instanceHandle, 
                    LacRsa_KeyGenSyncCb, pSyncCallbackData,
                    pKeyGenData, pPrivateKey, pPublicKey);
    }
    else
    {
        LAC_RSA_STAT_INC(numRsaKeyGenRequestErrors, instanceHandle);
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
            LAC_RSA_STAT_INC(numRsaKeyGenCompletedErrors,
                            instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}


#if defined(ICP_PARAM_CHECK)
CpaStatus
LacRsa_KeyGenParamsCheck(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey)
{
    CpaStatus status = CPA_STATUS_SUCCESS;
    Cpa32U opSizeInBytes = 0;

    LAC_INITIALISED_CHECK();
    LAC_PKE_INITIALISED_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pRsaKeyGenCb);

    /* Check user parameters */
    LAC_CHECK_NULL_PARAM(pKeyGenData);
    LAC_CHECK_NULL_PARAM(pPrivateKey);
    LAC_CHECK_NULL_PARAM(pPublicKey);

    opSizeInBytes  = pKeyGenData->modulusLenInBytes;

    if (CPA_FALSE == LacRsa_IsValidRsaSize(opSizeInBytes))
    {
        LAC_INVALID_PARAM_LOG("Invalid pKeyGenData->modulusLenInBytes Size. ");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* Check p and q buffers which should be half op size */
    LAC_CHECK_FLAT_BUFFER_PARAM(&(pKeyGenData->prime1P), CHECK_EQUALS,
        LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes), CPA_TRUE, CPA_TRUE);
    LAC_CHECK_FLAT_BUFFER_PARAM(&(pKeyGenData->prime2Q), CHECK_EQUALS,
        LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes), CPA_TRUE, CPA_TRUE);

    /* If type 2 key we need to set p and q correctly in the key struct.
     * We also have to check param d in the type 1 key  */
    pPrivateKey->version = pKeyGenData->version;
    pPrivateKey->privateKeyRepType = pKeyGenData->privateKeyRepType;

    LAC_CHECK_FLAT_BUFFER_PARAM(&(pKeyGenData->publicExponentE),
        CHECK_LESS_EQUALS, opSizeInBytes, CPA_FALSE, CPA_TRUE);
    pPublicKey->publicExponentE.pData =
        pKeyGenData->publicExponentE.pData;
    pPublicKey->publicExponentE.dataLenInBytes =
        pKeyGenData->publicExponentE.dataLenInBytes;

    LAC_CHECK_RSA_BUFFER_PARAM(&(pPublicKey->modulusN),
        CHECK_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);

    if (CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_2
        == pPrivateKey->privateKeyRepType)
    {
        pPrivateKey->privateKeyRep2.prime1P.pData =
            pKeyGenData->prime1P.pData;
        pPrivateKey->privateKeyRep2.prime2Q.pData =
            pKeyGenData->prime2Q.pData;

        pPrivateKey->privateKeyRep2.prime1P.dataLenInBytes =
            pKeyGenData->prime1P.dataLenInBytes;
        pPrivateKey->privateKeyRep2.prime2Q.dataLenInBytes =
            pKeyGenData->prime2Q.dataLenInBytes;

        LAC_CHECK_FLAT_BUFFER_PARAM(
            &(pPrivateKey->privateKeyRep1.privateExponentD),
                CHECK_LESS_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);
    }

    status = LacRsa_CheckPrivateKeyParam(
        pPrivateKey, opSizeInBytes, CPA_FALSE, CPA_FALSE);
    LAC_CHECK_STATUS(status);

    /* Standards based check: e >= 3 */
    if(LacPke_CompareZero(&(pPublicKey->publicExponentE),
        LAC_RSA_KEYGEN_MIN_EXP) < 0)
    {
        LAC_INVALID_PARAM_LOG("publicExponentE must be >= 3");
        return CPA_STATUS_INVALID_PARAM;
    }

    return status;
}
#endif //ICP_PARAM_CHECK

CpaStatus
LacRsa_Type1KeyGen(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    void *pCallbackTag,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey)
{
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    Cpa32U pInArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U pOutArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};
    Cpa32U opSizeInBytes = 0;
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_pke_op_cb_data_t cbData = {0};
    lac_mmp_input_param_t in = {.inputParam.flat_array = {0}};
    lac_mmp_output_param_t out = {.outputParam.flat_array = {0}};

    LAC_ASSERT_NOT_NULL(pKeyGenData);
    opSizeInBytes  = pKeyGenData->modulusLenInBytes;

    functionalityId = LacPke_GetMmpId(
        LAC_BYTES_TO_BITS(opSizeInBytes),
        LAC_RSA_KP1_SIZE_ID_MAP, LAC_ARRAY_LEN(LAC_RSA_KP1_SIZE_ID_MAP));
    if (LAC_PKE_INVALID_FUNC_ID == functionalityId)
    {
        status = CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.rsaKp1Input.p, &(pKeyGenData->prime1P));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp1_input_t, p)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.rsaKp1Input.q, &(pKeyGenData->prime2Q));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp1_input_t, q)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaKp1Input.e,
            &(pKeyGenData->publicExponentE));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp1_input_t, e)]
            = opSizeInBytes;

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.rsaKp1Output.n, &(pPublicKey->modulusN));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp1_output_t, n)]
            = opSizeInBytes;

        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaKp1Output.d,
            &(pPrivateKey->privateKeyRep1.privateExponentD));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp1_output_t, d)]
            = opSizeInBytes;

        /* populate callback data */
        cbData.pClientCb = pRsaKeyGenCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pKeyGenData;
        cbData.pOutputData1 = pPrivateKey;
        cbData.pOutputData2 = pPublicKey;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(functionalityId, pInArgSizeList,
            pOutArgSizeList, &(in.inputParam), &(out.outputParam),
            LacRsa_ProcessKeyCb, &cbData, instanceHandle);

        /* @performance
         * the in and out structures are allocated on the stack. This forces #
         * the underlying function to allocate and copy. why not allocate
         * nicely aligned in and out structures ? (this is a general comment
         * for all functions)*/
    }

    return status;
}

CpaStatus
LacRsa_Type2KeyGen(
    const CpaInstanceHandle instanceHandle,
    const CpaCyRsaKeyGenCbFunc pRsaKeyGenCb,
    void *pCallbackTag,
    const CpaCyRsaKeyGenOpData *pKeyGenData,
    CpaCyRsaPrivateKey *pPrivateKey,
    CpaCyRsaPublicKey *pPublicKey)
{
    Cpa32U functionalityId = LAC_PKE_INVALID_FUNC_ID;
    Cpa32U pInArgSizeList[LAC_MAX_MMP_INPUT_PARAMS] = {0};
    Cpa32U pOutArgSizeList[LAC_MAX_MMP_OUTPUT_PARAMS] = {0};
    Cpa32U opSizeInBytes = 0;
    CpaStatus status = CPA_STATUS_SUCCESS;
    lac_pke_op_cb_data_t cbData = {0};
    lac_mmp_input_param_t in = {.inputParam.flat_array = {0}};
    lac_mmp_output_param_t out = {.outputParam.flat_array = {0}};

    LAC_ASSERT_NOT_NULL(pKeyGenData);
    opSizeInBytes  = pKeyGenData->modulusLenInBytes;

    functionalityId = LacPke_GetMmpId(
        LAC_BYTES_TO_BITS(opSizeInBytes),
        LAC_RSA_KP2_SIZE_ID_MAP, LAC_ARRAY_LEN(LAC_RSA_KP2_SIZE_ID_MAP));
    if (LAC_PKE_INVALID_FUNC_ID == functionalityId)
    {
        status = CPA_STATUS_FAIL;
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* populate input parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.rsaKp2Input.p, &(pKeyGenData->prime1P));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_input_t, p)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(
            in.rsaKp2Input.q, &(pKeyGenData->prime2Q));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_input_t, q)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(in.rsaKp2Input.e,
            &(pKeyGenData->publicExponentE));
        pInArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_input_t, e)]
            = opSizeInBytes;

        /* populate output parameters */
        LAC_MEM_SHARED_WRITE_FROM_PTR(
            out.rsaKp2Output.n, &(pPublicKey->modulusN));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_output_t, n)]
            = opSizeInBytes;

        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaKp2Output.d,
            &(pPrivateKey->privateKeyRep1.privateExponentD));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_output_t, d)]
            = opSizeInBytes;

        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaKp2Output.dp,
            &(pPrivateKey->privateKeyRep2.exponent1Dp));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_output_t, dp)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaKp2Output.dq,
            &(pPrivateKey->privateKeyRep2.exponent2Dq));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_output_t, dq)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        LAC_MEM_SHARED_WRITE_FROM_PTR(out.rsaKp2Output.qinv,
            &(pPrivateKey->privateKeyRep2.coefficientQInv));
        pOutArgSizeList[LAC_IDX_OF(icp_qat_fw_mmp_rsa_kp2_output_t, qinv)]
            = LAC_RSA_TYPE_2_BUF_SIZE_GET(opSizeInBytes);

        /* populate callback data */
        cbData.pClientCb = pRsaKeyGenCb;
        cbData.pCallbackTag = pCallbackTag;
        cbData.pClientOpData = pKeyGenData;
        cbData.pOutputData1 = pPrivateKey;
        cbData.pOutputData2 = pPublicKey;

        /* send a PKE request to the QAT */
        status = LacPke_SendSingleRequest(functionalityId, pInArgSizeList,
            pOutArgSizeList, &(in.inputParam), &(out.outputParam),
            LacRsa_ProcessKeyCb, &cbData, instanceHandle);
    }

    return status;
}


void
LacRsa_ProcessKeyCb(CpaStatus status, CpaBoolean pass,
    CpaInstanceHandle instanceHandle, lac_pke_op_cb_data_t *pCbData)
{
    CpaCyRsaKeyGenCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyRsaKeyGenOpData *pOpData = NULL;
    CpaCyRsaPrivateKeyRepType keyType = 0;
    CpaCyRsaPrivateKey *pPrivateKey = NULL;
    CpaCyRsaPublicKey *pPublicKey = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCallbackTag = pCbData->pCallbackTag;

    pOpData = (CpaCyRsaKeyGenOpData *) 
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    LAC_ASSERT_NOT_NULL(pOpData);

    pCb = (CpaCyRsaKeyGenCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);

    pPrivateKey = (CpaCyRsaPrivateKey *)pCbData->pOutputData1;
    pPublicKey = (CpaCyRsaPublicKey *)pCbData->pOutputData2;
    LAC_ASSERT_NOT_NULL(pPrivateKey);
    LAC_ASSERT_NOT_NULL(pPublicKey);

    keyType = pPrivateKey->privateKeyRepType;

    if (CPA_STATUS_SUCCESS == status)
    {
        if (CPA_FALSE == pass)
        {
            LAC_LOG_ERROR(
                "Cannot generate a valid RSA public key from provided "
                "e, p, and q input parameters");
            status = CPA_STATUS_FAIL;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        /* Standards based checks */
        /* n must have one of the two most sig bits set and e < n */
        if (!(pPublicKey->modulusN.pData[0] & 0xC0))
        {
            LAC_LOG_ERROR("The number n = p * q is out of range or invalid");
            status = CPA_STATUS_FAIL;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        if(LacPke_Compare(&(pPublicKey->publicExponentE), 0,
            &(pPublicKey->modulusN), 0) >= 0)
        {
            LAC_INVALID_PARAM_LOG("(e,n) is not a valid RSA public key n <= e");
            status = CPA_STATUS_FAIL;
        }
    }

    if (CPA_STATUS_SUCCESS == status)
    {
        if (CPA_CY_RSA_PRIVATE_KEY_REP_TYPE_1 == keyType)
        {
             memcpy(pPrivateKey->privateKeyRep1.modulusN.pData,
                pPublicKey->modulusN.pData,
                pPublicKey->modulusN.dataLenInBytes);

        }
    }

    /* increment stats */
    LAC_RSA_STAT_INC(numRsaKeyGenCompleted, instanceHandle);
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_RSA_STAT_INC(numRsaKeyGenCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pPrivateKey, pPublicKey);
}
