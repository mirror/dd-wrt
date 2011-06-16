/******************************************************************************
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
 *****************************************************************************/

/**
 *****************************************************************************
 *
 * @file lac_dh_data_path.c
 *
 * @defgroup LacDh Diffie Hellman
 *
 * @ingroup LacAsym
 *
 * diffie hellman data path functions
 *
 * @lld_start
 *
 * @lld_overview
 * This is the Diffie Hellman feature implementation.  It implements 2 DH API
 * services: phase 1 and phase 2.  Statistics are maintained for each service.
 * For each service the parameters supplied by the client are checked, and then
 * input/output argument lists are constructed before calling the Mod Exp layer.
 * Diffie Hellman is really just mod exp operations. This layer simply performs
 * the appropriate param checks and then orders the input params correctly
 * for mod exp.
 *
 * In both cases the service implementations are a straightforward
 * marshalling of client-supplied parameters for the QAT. I.e. there is
 * minimal logic handled by this component.  Buffer alignment is handled by
 * the PKE QAT Comms layer.
 *
 * The user's input buffers are checked for null params, correct length, msb
 * and lsb set where necessary. The following parameter checks based on the
 * standard are also performed for Diffie Hellman
 *
 *
 * Diffie Hellman:
 * Phase 1:    y = g^x mod p   (PKCS #3)
 *
 *           Test: P must have msb set
 *           Test: P must be odd
 *           Test:  0  < g < p
 *           Test:  0  < x < p  1
 *
 * Phase 2:   s = y^x mod p  (PKCS #3)
 *
 *           Test: P must have msb set
 *           Test: P must be odd
 *           Test:  0  <  x  < p  - 1
 *
 * @lld_dependencies
 * - \ref LacAsymCommonQatComms "PKE QAT Comms" : For creating and sending
 * messages to the QAT
 * - \ref LacMem "Mem" : For memory allocation and freeing, and translating
 * between scalar and pointer types
 * - \ref LacModExp "Mod Exp" : Both Diffie Hellman phases are implemented as #
 * mod exp operations.
 * - OSAL : For atomics and logging
 *
 * @lld_initialisation
 * On initialization this component clears the stats.
 *
 * @lld_module_algorithms
 *
 * @lld_process_context
 *
 * @lld_end
 *
 *****************************************************************************/

/*
*******************************************************************************
* Include public/global header files
*******************************************************************************
*/

/* API Includes */
#include "cpa_cy_common.h"
#include "cpa_cy_dh.h"

/* OSAL Includes */
#include "IxOsal.h"
#include "cpa.h"

/* Look Aside Includes */
#include "lac_log.h"
#include "lac_common.h"
#include "lac_mod_exp.h"
#include "lac_mem.h"
#include "lac_pke_utils.h"
#include "lac_pke_qat_comms.h"
#include "lac_sync.h"

/*
*******************************************************************************
* Include private header files
*******************************************************************************
*/

#include "lac_dh_stats_p.h"
/*
*******************************************************************************
* Static Variables
*******************************************************************************
*/

#define LAC_G2_DELTA (-2)
/**< @ingroup LacDh
 *  the delta to be subtracted from the exponent when testing for
 *  g2 type operation */

STATIC const Cpa32U LAC_DH_SUPPORTED_SIZES[] =
    {LAC_768_BITS, LAC_1024_BITS, LAC_1536_BITS, LAC_2048_BITS,
        LAC_3072_BITS, LAC_4096_BITS};
/**< @ingroup LacDh
 *  the sizes supported for Diffie Hellman operations */


/*
*******************************************************************************
* Define static function definitions
*******************************************************************************
*/

/*
 * This function verifies that all the input parameters for the Diffie Hellman
 * Phase 1 operation are valid
 */
#if defined(ICP_PARAM_CHECK)
STATIC CpaStatus
LacDh_CheckPhase1KeyGenData(const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pDhPhase1Cb,
    const CpaCyDhPhase1KeyGenOpData *pPhase1KeyGenData,
    CpaFlatBuffer *pLocalOctetStringPV);
#endif //ICP_PARAM_CHECK

#ifdef ICP_PARAM_CHECK
/*
 * This function verifies that all the input parameters for the Diffie Hellman
 * Phase 2 operation are valid
 */
STATIC CpaStatus
LacDh_CheckPhase2KeyGenData(const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pDhPhase2Cb,
    const CpaCyDhPhase2SecretKeyGenOpData *pPhase2SecretKeyGenData,
    CpaFlatBuffer *pOctetStringSecretKey);
#endif //ICP_PARAM_CHECK

/*
 * This function is called by the mod exp module after a Diffie Hellman
 * Phase1 Mod Exp message has been received from the QAT.
 */
STATIC void
LacDh_ProcessPhase1Cb(CpaStatus status, CpaBoolean pass,
    CpaInstanceHandle instanceHandle, lac_pke_op_cb_data_t *pCbData);

/*
 * This function is called by the mod exp module after a Diffie Hellman
 * Phase2 Mod Exp message has been received from the QAT.
 */
STATIC void
LacDh_ProcessPhase2Cb(CpaStatus status, CpaBoolean pass,
    CpaInstanceHandle instanceHandle, lac_pke_op_cb_data_t *pCbData);

/*
 * This function is the synchronous version of cpaCyDhKeyGenPhase1
 */
STATIC CpaStatus
LacDh_SynKeyGenPhase1(const CpaInstanceHandle instanceHandle,
                      const CpaCyGenFlatBufCbFunc pDhPhase1Cb,
                      void *pCallbackTag,
                      const CpaCyDhPhase1KeyGenOpData *pPhase1KeyGenData,
                      CpaFlatBuffer *pLocalOctetStringPV);

/*
 * This function is the synchronous version of cpaCyDhKeyGenPhase2Secret
 */
STATIC CpaStatus
LacDh_SynKeyGenPhase2Secret(
        const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pDhPhase2Cb,
        void *pCallbackTag,
        const CpaCyDhPhase2SecretKeyGenOpData *pPhase2SecretKeyGenData,
        CpaFlatBuffer *pOctetStringSecretKey);

/*
*******************************************************************************
* Global Variables
*******************************************************************************
*/

/*
*******************************************************************************
* Define public/global function definitions
*******************************************************************************
*/

#ifdef ICP_PARAM_CHECK
STATIC CpaBoolean
LacDh_IsValidDhSize(Cpa32U opSizeInBytes)
{
    Cpa32U opSizeInBits = LAC_BYTES_TO_BITS(opSizeInBytes);

    if ((LAC_768_BITS != opSizeInBits)
        && (LAC_1024_BITS != opSizeInBits)
        && (LAC_1536_BITS != opSizeInBits)
        && (LAC_2048_BITS != opSizeInBits)
        && (LAC_3072_BITS != opSizeInBits)
        && (LAC_4096_BITS != opSizeInBits))
    {
        LAC_INVALID_PARAM_LOG("Invalid operation size. Valid op sizes for "
            "DH are 768, 1024, 1536, 2048, 3072 and 4096 bits.");
        return CPA_FALSE;
    }

    return CPA_TRUE;
}
#endif //ICP_PARAM_CHECK

STATIC lac_mod_exp_op_type_t
LacDh_GetModExpOpType(const CpaFlatBuffer *pExp)
{
    LAC_ASSERT_NOT_NULL(pExp);

    /* if exp is equal to 2 then return op type G2, otherwise normal mod exp */
    if (0 == LacPke_CompareZero(pExp, LAC_G2_DELTA))
    {
        return LAC_MOD_EXP_G2_OP;
    }
    return LAC_MOD_EXP_OP;
}

STATIC CpaStatus
LacDh_SynKeyGenPhase1(const CpaInstanceHandle instanceHandle,
                      const CpaCyGenFlatBufCbFunc pDhPhase1Cb,
                      void *pCallbackTag,
                      const CpaCyDhPhase1KeyGenOpData *pPhase1KeyGenData,
                      CpaFlatBuffer *pLocalOctetStringPV)
{
    CpaStatus status = CPA_STATUS_FAIL;
    lac_sync_op_data_t *pSyncCallbackData = NULL;

    status = LacSync_CreateSyncCookie(&pSyncCallbackData);
    /*
     * Call the asynchronous version of the function 
     * with the synchronous callback function as a parameter.
     */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = cpaCyDhKeyGenPhase1(instanceHandle,
        LacSync_GenFlatBufCb, pSyncCallbackData,
                pPhase1KeyGenData, pLocalOctetStringPV);
    }
    else
    {
        LAC_DH_STAT_INC(numDhPhase1KeyGenRequestErrors, instanceHandle);
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
            LAC_DH_STAT_INC(numDhPhase1KeyGenCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}

STATIC CpaStatus
LacDh_SynKeyGenPhase2Secret(
        const CpaInstanceHandle instanceHandle,
        const CpaCyGenFlatBufCbFunc pDhPhase2Cb,
        void *pCallbackTag,
        const CpaCyDhPhase2SecretKeyGenOpData *pPhase2SecretKeyGenData,
        CpaFlatBuffer *pOctetStringSecretKey)
{
    CpaStatus status = CPA_STATUS_FAIL;
    lac_sync_op_data_t *pSyncCallbackData = NULL;

    status = LacSync_CreateSyncCookie(&pSyncCallbackData);
    /*
     * Call the asynchronous version of the function
     * with the synchronous callback function as a parameter.
     */
    if (CPA_STATUS_SUCCESS == status)
    {
        status = cpaCyDhKeyGenPhase2Secret(instanceHandle, 
        LacSync_GenFlatBufCb, pSyncCallbackData,
                pPhase2SecretKeyGenData,
                pOctetStringSecretKey);
    }
    else
    {
        LAC_DH_STAT_INC(numDhPhase2KeyGenRequestErrors, instanceHandle);
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
            LAC_DH_STAT_INC(numDhPhase2KeyGenCompletedErrors,
                    instanceHandle);
            status = wCbStatus;
        }
    }
    LacSync_DestroySyncCookie(&pSyncCallbackData);
    return status;
}


/**
 *****************************************************************************
 * @ingroup LacDh
 *
 *****************************************************************************/
CpaStatus
cpaCyDhKeyGenPhase1(const CpaInstanceHandle instanceHandle,
                    const CpaCyGenFlatBufCbFunc pDhPhase1Cb,
                    void *pCallbackTag,
                    const CpaCyDhPhase1KeyGenOpData *pPhase1KeyGenData,
                    CpaFlatBuffer *pLocalOctetStringPV)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Check if the API has been called in synchronous mode */
    if (NULL == pDhPhase1Cb)
    {
        return LacDh_SynKeyGenPhase1(instanceHandle, pDhPhase1Cb, 
                pCallbackTag, pPhase1KeyGenData, pLocalOctetStringPV);
    }
    
#ifdef ICP_PARAM_CHECK
    /* Check the members of the key gen data struct */
    status = LacDh_CheckPhase1KeyGenData(instanceHandle, pDhPhase1Cb,
        pPhase1KeyGenData, pLocalOctetStringPV);
#endif //ICP_PARAM_CHECK

    if (CPA_STATUS_SUCCESS == status)
    {
        lac_pke_op_cb_data_t pkeOpData = {0};
        lac_mod_exp_op_data_t modExpOpData = {0};

        pkeOpData.pCallbackTag = pCallbackTag;
        pkeOpData.pClientOpData = pPhase1KeyGenData;
        pkeOpData.pClientCb = pDhPhase1Cb;
        pkeOpData.pOpaqueData = NULL;
        pkeOpData.pOutputData1 = pLocalOctetStringPV;

        
        /* Fill in parameters for mod exp function from session
         * data. We are calculating PV = g^x % p so set parameters to be
         * base  = g, exponent = x, modulus = p, result = PV */

         /* Invoke the mod exp API */
        modExpOpData.pPkeOpCbFunc = LacDh_ProcessPhase1Cb;
        modExpOpData.pPkeOpData = &pkeOpData;
    
        modExpOpData.sizeInBits = 
            LAC_BYTES_TO_BITS(pPhase1KeyGenData->primeP.dataLenInBytes);
        modExpOpData.pBase =
            (const CpaFlatBuffer *) &(pPhase1KeyGenData->baseG);
        modExpOpData.pExponent = 
            (const CpaFlatBuffer *) &(pPhase1KeyGenData->privateValueX);
        modExpOpData.pModulus =
            (const CpaFlatBuffer *) &(pPhase1KeyGenData->primeP);
        modExpOpData.pResult = pLocalOctetStringPV;
        modExpOpData.instanceHandle = instanceHandle;

        modExpOpData.opType = LacDh_GetModExpOpType(
            (const CpaFlatBuffer *)&(pPhase1KeyGenData->baseG));

        status = LacModExp_Perform(&modExpOpData);
    }

    /* if any of the preceding steps failed then we need to perform clean up */
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_DH_STAT_INC(numDhPhase1KeyGenRequestErrors, instanceHandle);
    }
    else
    {
        LAC_DH_STAT_INC(numDhPhase1KeyGenRequests, instanceHandle);
    }
    return status;
}


#ifdef ICP_PARAM_CHECK
CpaStatus
LacDh_CheckPhase1KeyGenData(const CpaInstanceHandle instanceHandle,
                      const CpaCyGenFlatBufCbFunc pDhPhase1Cb,
                      const CpaCyDhPhase1KeyGenOpData *pPhase1KeyGenData,
                      CpaFlatBuffer *pLocalOctetStringPV)
{
    Cpa32U opSizeInBytes = 0;

    LAC_INITIALISED_CHECK();
    LAC_PKE_INITIALISED_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pDhPhase1Cb);

    LAC_CHECK_NULL_PARAM(pPhase1KeyGenData);
    LAC_CHECK_NULL_PARAM(pLocalOctetStringPV);

    /* Check members of pPhase1KeyGenData are valid. Check pPrimeP, pBaseG,
     * pLocalOctetStringPV and pPrivateValueX */
    opSizeInBytes = pPhase1KeyGenData->primeP.dataLenInBytes;

    if (CPA_FALSE == LacDh_IsValidDhSize(opSizeInBytes))
    {
        LAC_INVALID_PARAM_LOG("Invalid size for opSizeInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* input params */
    LAC_CHECK_FLAT_BUFFER_PARAM((&pPhase1KeyGenData->primeP), CHECK_EQUALS,
        opSizeInBytes, CPA_TRUE, CPA_TRUE);

    LAC_CHECK_FLAT_BUFFER_PARAM((&pPhase1KeyGenData->baseG), CHECK_LESS_EQUALS,
        opSizeInBytes, CPA_FALSE, CPA_FALSE);

    LAC_CHECK_FLAT_BUFFER_PARAM((&pPhase1KeyGenData->privateValueX),
        CHECK_LESS_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);

    /* output params */
    LAC_CHECK_FLAT_BUFFER_PARAM(pLocalOctetStringPV,
        CHECK_GREATER_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);
    pLocalOctetStringPV->dataLenInBytes = opSizeInBytes;

    /* Standards based checks */
    /* 0 < g < p */
    LAC_CHECK_NON_ZERO_PARAM(&(pPhase1KeyGenData->baseG));
    if(LacPke_Compare(
        &(pPhase1KeyGenData->baseG), 0, &(pPhase1KeyGenData->primeP), 0) >= 0)
    {
        LAC_INVALID_PARAM_LOG("baseG must be < primeP");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* 0 < x < p-1 */
    LAC_CHECK_NON_ZERO_PARAM(&(pPhase1KeyGenData->privateValueX));
    if(LacPke_Compare(&(pPhase1KeyGenData->privateValueX), 0,
        &(pPhase1KeyGenData->primeP), -1) >= 0)
    {
        LAC_INVALID_PARAM_LOG("privateValueX must be < primeP - 1");
        return CPA_STATUS_INVALID_PARAM;
    }

    return CPA_STATUS_SUCCESS;
}
#endif //ICP_PARAM_CHECK

/**
 *****************************************************************************
 * @ingroup LacDh
 *
 *****************************************************************************/
CpaStatus
cpaCyDhKeyGenPhase2Secret(
    const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pDhPhase2Cb,
    void *pCallbackTag,
    const CpaCyDhPhase2SecretKeyGenOpData *pPhase2SecretKeyGenData,
    CpaFlatBuffer *pOctetStringSecretKey)
{
    CpaStatus status = CPA_STATUS_SUCCESS;

    /* Check if the API has been called in synchronous mode */
    if (NULL == pDhPhase2Cb)
    {
        return LacDh_SynKeyGenPhase2Secret(instanceHandle,
                pDhPhase2Cb, pCallbackTag, pPhase2SecretKeyGenData,
                pOctetStringSecretKey);
    }

#if defined(ICP_PARAM_CHECK)
    /* Check the members of the key gen data struct */
    status = LacDh_CheckPhase2KeyGenData(instanceHandle,
        pDhPhase2Cb, pPhase2SecretKeyGenData, pOctetStringSecretKey);
#endif //ICP_PARAM_CHECK

    if (CPA_STATUS_SUCCESS == status)
    {
        lac_pke_op_cb_data_t pkeOpData = {0};
        lac_mod_exp_op_data_t modExpOpData = {0};

        pkeOpData.pCallbackTag = pCallbackTag;
        pkeOpData.pClientOpData = pPhase2SecretKeyGenData;
        pkeOpData.pClientCb = pDhPhase2Cb;
        pkeOpData.pOpaqueData = NULL;
        pkeOpData.pOutputData1 = pOctetStringSecretKey;

        /* Fill in parameters for the mod exp call.
         * We are calculating Secret = (PV')^x % p so set parameters to be
         * base  = PV' exponent = x, modulus = p, result = secret */

        /* Invoke the mod exp API */
        modExpOpData.pPkeOpCbFunc = (lac_pke_op_cb_func_t)LacDh_ProcessPhase2Cb;
        modExpOpData.pPkeOpData = &pkeOpData;

        modExpOpData.sizeInBits = 
            LAC_BYTES_TO_BITS(pPhase2SecretKeyGenData->primeP.dataLenInBytes);
        modExpOpData.pBase =
            (const CpaFlatBuffer *)
            &(pPhase2SecretKeyGenData->remoteOctetStringPV);
        modExpOpData.pExponent =
            (const CpaFlatBuffer *)&(pPhase2SecretKeyGenData->privateValueX);
        modExpOpData.pModulus =
            (const CpaFlatBuffer *)&(pPhase2SecretKeyGenData->primeP);
        modExpOpData.pResult = pOctetStringSecretKey;
        modExpOpData.instanceHandle = instanceHandle;
        modExpOpData.opType = LAC_MOD_EXP_OP;

        status = LacModExp_Perform(&modExpOpData);
    }

    /* if any of the preceding steps failed then we need to perform clean up */
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_DH_STAT_INC(numDhPhase2KeyGenRequestErrors, instanceHandle);
    }
    else
    {
        LAC_DH_STAT_INC(numDhPhase2KeyGenRequests, instanceHandle);
    }
    return status;
}

#if defined(ICP_PARAM_CHECK)
STATIC CpaStatus
LacDh_CheckPhase2KeyGenData(const CpaInstanceHandle instanceHandle,
    const CpaCyGenFlatBufCbFunc pDhPhase2Cb,
    const CpaCyDhPhase2SecretKeyGenOpData *pPhase2SecretKeyGenData,
    CpaFlatBuffer *pOctetStringSecretKey)
{
    Cpa32U opSizeInBytes = 0;

    LAC_INITIALISED_CHECK();
    LAC_PKE_INITIALISED_CHECK();
    LAC_CHECK_INSTANCE_HANDLE(instanceHandle);
    LAC_CHECK_NULL_PARAM(pDhPhase2Cb);

    LAC_CHECK_NULL_PARAM(pPhase2SecretKeyGenData);
    LAC_CHECK_NULL_PARAM(pOctetStringSecretKey);

    /* Check members of pPhase2SecretKeyGenData are valid. Check pPrimeP,
     * pOctetStringSecretKey, pPrivateValueX and pRemoteOctetStringPV */
    opSizeInBytes = pPhase2SecretKeyGenData->primeP.dataLenInBytes;

    if (CPA_FALSE == LacDh_IsValidDhSize(opSizeInBytes))
    {
        LAC_INVALID_PARAM_LOG("Invalid size for opSizeInBytes");
        return CPA_STATUS_INVALID_PARAM;
    }

    /* input params */
    LAC_CHECK_FLAT_BUFFER_PARAM(&(pPhase2SecretKeyGenData->primeP),
        CHECK_EQUALS, opSizeInBytes, CPA_TRUE, CPA_TRUE);

    LAC_CHECK_FLAT_BUFFER_PARAM(&(pPhase2SecretKeyGenData->remoteOctetStringPV),
        CHECK_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);

    LAC_CHECK_FLAT_BUFFER_PARAM(&(pPhase2SecretKeyGenData->privateValueX),
        CHECK_LESS_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);

    /* output params */
    LAC_CHECK_FLAT_BUFFER_PARAM(pOctetStringSecretKey,
        CHECK_GREATER_EQUALS, opSizeInBytes, CPA_FALSE, CPA_FALSE);
    pOctetStringSecretKey->dataLenInBytes = opSizeInBytes;

    /* Standards based checks */
    /* 0 < x < p-1 */
    LAC_CHECK_NON_ZERO_PARAM(&(pPhase2SecretKeyGenData->privateValueX));
    if(LacPke_Compare(&(pPhase2SecretKeyGenData->privateValueX), 0,
        &(pPhase2SecretKeyGenData->primeP), -1) >= 0)
    {
        LAC_INVALID_PARAM_LOG("privateValueX must be < primeP - 1");
        return CPA_STATUS_INVALID_PARAM;
    }

    return CPA_STATUS_SUCCESS;
}
#endif //ICP_PARAM_CHECK

void
LacDh_ProcessPhase1Cb(CpaStatus status, CpaBoolean pass,
    CpaInstanceHandle instanceHandle, lac_pke_op_cb_data_t *pCbData)
{
    CpaCyGenFlatBufCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDhPhase1KeyGenOpData *pOpData = NULL;
    CpaFlatBuffer *pLocalOctetStringPV = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDhPhase1KeyGenOpData *) 
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pCb = (CpaCyGenFlatBufCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pLocalOctetStringPV = pCbData->pOutputData1;
    LAC_ASSERT_NOT_NULL(pLocalOctetStringPV);

    /* increment stats */
    LAC_DH_STAT_INC(numDhPhase1KeyGenCompleted, instanceHandle);
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_DH_STAT_INC(numDhPhase1KeyGenCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pLocalOctetStringPV);
}

void
LacDh_ProcessPhase2Cb(CpaStatus status, CpaBoolean pass,
    CpaInstanceHandle instanceHandle, lac_pke_op_cb_data_t *pCbData)
{
    CpaCyGenFlatBufCbFunc pCb = NULL;
    void *pCallbackTag = NULL;
    CpaCyDhPhase2SecretKeyGenOpData *pOpData = NULL;
    CpaFlatBuffer *pOctetStringSecretKey = NULL;

    /* extract info from callback data structure */
    LAC_ASSERT_NOT_NULL(pCbData);
    pCallbackTag = pCbData->pCallbackTag;
    pOpData = (CpaCyDhPhase2SecretKeyGenOpData *)
                  LAC_CONST_PTR_CAST(pCbData->pClientOpData);
    pCb = (CpaCyGenFlatBufCbFunc) LAC_CONST_PTR_CAST(pCbData->pClientCb);
    pOctetStringSecretKey = pCbData->pOutputData1;
    LAC_ASSERT_NOT_NULL(pOctetStringSecretKey);

    /* increment stats */
    LAC_DH_STAT_INC(numDhPhase2KeyGenCompleted, instanceHandle);
    if (CPA_STATUS_SUCCESS != status)
    {
        LAC_DH_STAT_INC(numDhPhase2KeyGenCompletedErrors, instanceHandle);
    }

    /* invoke the user callback */
    pCb(pCallbackTag, status, pOpData, pOctetStringSecretKey);
}
